// Copyright (C) 2018-2019 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "cnn.hpp"

#include <string>
#include <vector>
#include <algorithm>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <inference_engine.hpp>

using namespace InferenceEngine;

CnnBase::CnnBase(const Config& config,
                 const InferenceEngine::InferencePlugin& plugin) :
    config_(config), net_plugin_(plugin) {}
void CnnBase::Load() {

    std::cout<<"load reid model here!" << std::endl;

    CNNNetReader net_reader;
    net_reader.ReadNetwork(config_.path_to_model);
    net_reader.ReadWeights(config_.path_to_weights);

    if (!net_reader.isParseSuccess()) {
        THROW_IE_EXCEPTION << "Cannot load model";
    }

    const int currentBatchSize = net_reader.getNetwork().getBatchSize();
    if (currentBatchSize != config_.max_batch_size)
        net_reader.getNetwork().setBatchSize(config_.max_batch_size);

    InferenceEngine::InputsDataMap in;
    in = net_reader.getNetwork().getInputsInfo();


    if (in.size() != 1) {
        THROW_IE_EXCEPTION << "Network should have only one input";
    }

    SizeVector inputDims = in.begin()->second->getTensorDesc().getDims();
    in.begin()->second->setInputPrecision(Precision::U8);
    input_blob_ = make_shared_blob<uint8_t>(TensorDesc(Precision::U8, inputDims, Layout::NCHW));
    input_blob_->allocate();
    BlobMap inputs;
    inputs[in.begin()->first] = input_blob_;
    outInfo_ = net_reader.getNetwork().getOutputsInfo();
    
    // std::cout<< inputDims.size() << std::endl;     // 4
    
    for (auto&& item : outInfo_) {
        SizeVector outputDims = item.second->getTensorDesc().getDims();
        // std::cout<< outputDims.size() << std::endl;   // 4
        item.second->setPrecision(Precision::FP32);
        // create output buffer note that "Layout"
        TBlob<float>::Ptr output;
        if(outputDims.size()==4)
            output = make_shared_blob<float>(TensorDesc(Precision::FP32, outputDims, Layout::NCHW));  
        else if(outputDims.size()==2)
            output = make_shared_blob<float>(TensorDesc(Precision::FP32, outputDims, Layout::NC));   
        output->allocate();
        outputs_[item.first] = output;
    }

    executable_network_ = net_plugin_.LoadNetwork(net_reader.getNetwork(), {});
    infer_request_ = executable_network_.CreateInferRequest();
    infer_request_.SetInput(inputs);
    infer_request_.SetOutput(outputs_);
}

void CnnBase::InferBatch(
    const std::vector<cv::Mat>& frames,
    std::function<void(const InferenceEngine::BlobMap&, size_t)> fetch_results) const {
    const size_t batch_size = input_blob_->getTensorDesc().getDims()[0];    // input->getDim() [1,3,96,48]

    // 
    size_t num_imgs = frames.size();

    // 根据网络设置的输入batch
    for (size_t batch_i = 0; batch_i < num_imgs; batch_i += batch_size) {

        const size_t current_batch_size = std::min(batch_size, num_imgs - batch_i); // 网络设置的输入batch与计算剩batch的小值作为本次计算的实际batch
        // std::cout<<input_blob_->getTensorDesc().getDims().size()<<std::endl;     // 4
        // 将数据一个一个的送入blob
        // 在赋值之前会将输入图片resize为网络设置的大小
        for (size_t b = 0; b < current_batch_size; b++) {
            matU8ToBlob<uint8_t>(frames[batch_i + b], input_blob_, b);
        }

        // std::cout<<"inferBatch:"<<current_batch_size<<std::endl;    // 1
         
        // infer_request_.SetBatch(current_batch_size);
        infer_request_.Infer();
        // infer_request_.Wait(InferenceEngine::IInferRequest::WaitMode::RESULT_READY);
        fetch_results(outputs_, current_batch_size);
    }
}

void CnnBase::PrintPerformanceCounts() const {
    std::cout << "Performance counts for " << config_.path_to_model << std::endl << std::endl;
    ::printPerformanceCounts(infer_request_.GetPerformanceCounts(), std::cout, false);
}

void CnnBase::Infer(const cv::Mat& frame,
                    std::function<void(const InferenceEngine::BlobMap&, size_t)> fetch_results) const {
    InferBatch({frame}, fetch_results);
}

VectorCNN::VectorCNN(const Config& config,
                     const InferenceEngine::InferencePlugin& plugin)
    : CnnBase(config, plugin) {
    Load();

    if (outputs_.size() != 1) {
        THROW_IE_EXCEPTION << "Demo supports topologies only with 1 output";
    }
    InferenceEngine::SizeVector dims = outInfo_.begin()->second->dims;
    int num_dims = static_cast<int>(dims.size());

    result_size_ = 1;
    for (int i = 0; i < num_dims - 1; ++i) {
        result_size_ *= dims[i];
    }
}

void VectorCNN::Compute(const cv::Mat& frame,
                        cv::Mat* vector, cv::Size outp_shape) const {
    std::vector<cv::Mat> output;
    Compute({frame}, &output, outp_shape);
    *vector = output[0];
}

// 最终调用的深度特征提取函数
// 输入 images需要计算深度特征的图片集合，vectors输出集合
// cv::Size outp_shape = cv::Size() default
void VectorCNN::Compute(const std::vector<cv::Mat>& images, std::vector<cv::Mat>* vectors,
                        cv::Size outp_shape) const {
    if (images.empty()) {
        return;
    }
    vectors->clear();
    // std::cout<<images.size()<<std::endl;
    // std::cout<<"reid model comput!"<<std::endl;

    // 这里定义一个函数，用来对网络结果做处理，并将函数头作为参数传入推理函数
    // input:output_ current_batch_size
    auto results_fetcher = [vectors, outp_shape](const InferenceEngine::BlobMap& outputs, size_t batch_size) {

        // std::cout<<"outputs:"<<outputs.size();  // 1
        // 可能有多个输出，但是这里只有一个输出，包含了多个维度，一个输入图片就对应一个维度的输出
        for (auto&& item : outputs) {
            InferenceEngine::Blob::Ptr blob = item.second;
            if (blob == nullptr) {
                THROW_IE_EXCEPTION << "VectorCNN::Compute() Invalid blob '" << item.first << "'";
            }
            InferenceEngine::SizeVector ie_output_dims = blob->getTensorDesc().getDims();  
            // std::cout<<ie_output_dims.size()<<std::endl;            // 2
            std::vector<int> blob_sizes(ie_output_dims.size(), 0);     // 向量初始化 一维向量 大小 初值

            // std::cout<<" blob_sizes:"<<blob_sizes.size();  // 2维输出 
            for (size_t i = 0; i < blob_sizes.size(); ++i) {
                blob_sizes[i] = ie_output_dims[i];
            }
            cv::Mat out_blob(blob_sizes, CV_32F, blob->buffer());

            // std::cout<<"out_blob:"<<out_blob<<std::endl;   // 正负均有 计算余弦距离
            for (size_t b = 0; b < batch_size; b++) {
                cv::Mat blob_wrapper(out_blob.size[1], 1, CV_32F,
                                     reinterpret_cast<void*>((out_blob.ptr<float>(0) + b * out_blob.size[1]))); // mat初始化 （rows,cols）

                vectors->emplace_back();                    // vector.emplace_back()在最后添加数据 相较于vector.push_back() 直接原地构造，但让内存重新分配
                                                            // 造成之前的引用失效

                // std::cout<<"  cv::size():"<<cv::Size();     // [0x0]
                // std::cout<<"  outp_shape:"<<outp_shape;     // [0x0]
                
                if (outp_shape != cv::Size())
                    blob_wrapper = blob_wrapper.reshape(1, {outp_shape.height, outp_shape.width}); // wapper包装
                blob_wrapper.copyTo(vectors->back());        // vector.back() 传回最后一个成员的引用
            }
        }
        // std::cout<<std::endl;
        // for(auto vector:vectors)
        // std::cout<<"vevtor:"<<*vectors<<std::endl;
    };
    InferBatch(images, results_fetcher);
}
