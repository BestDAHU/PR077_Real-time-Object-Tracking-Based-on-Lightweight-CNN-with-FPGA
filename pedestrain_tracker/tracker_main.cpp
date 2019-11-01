// Copyright (C) 2018-2019 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "core.hpp"
#include "utils.hpp"
#include "descriptor.hpp"
#include "distance.hpp"
#include "pedestrian_tracker_demo.hpp"

#include <iostream>  // template class header file has no suffix name of .hh 
#include <utility>
#include <vector>
#include <map>
#include <memory>
#include <string>
#include <gflags/gflags.h>

#include <ctime>

#include "tracker_main.hpp"
using namespace InferenceEngine;
using ImageWithFrameIndex = std::pair<cv::Mat, int>;

std::unique_ptr<PedestrianTracker> tracker;
ObjectDetector pedestrian_detector;
TrackedObjects detections;
int current_frame_idx = 1;
std::unique_ptr<PedestrianTracker>
CreatePedestrianTracker(const std::string& reid_model,
                        const std::string& reid_weights,
                        const InferenceEngine::InferencePlugin& reid_plugin,
                        bool should_keep_tracking_info) {
    TrackerParams params;    // 定义的时候初始化参数

    if (should_keep_tracking_info) {
        params.drop_forgotten_tracks = false;
        params.max_num_objects_in_track = -1;
    }

    std::unique_ptr<PedestrianTracker> tracker(new PedestrianTracker(params));

    // 设置resize参数
    std::shared_ptr<IImageDescriptor> descriptor_fast =
        std::make_shared<ResizedImageDescriptor>(
            cv::Size(48, 96), cv::InterpolationFlags::INTER_LINEAR);

    std::shared_ptr<IDescriptorDistance> distance_fast =
        std::make_shared<MatchTemplateDistance>();

    tracker->set_descriptor_fast(descriptor_fast);
    tracker->set_distance_fast(distance_fast);

    if (!reid_model.empty() && !reid_weights.empty()) {
        CnnConfig reid_config(reid_model, reid_weights);
        reid_config.max_batch_size = 1;

        // load cnn model
        // create InterenceEngine
        std::shared_ptr<IImageDescriptor> descriptor_strong =
            std::make_shared<DescriptorIE>(reid_config, reid_plugin);   // std::make_share返回一个指定类型的shared_ptr
                                                                        // shared_ptr提供可以共享所有权的智能指针
        if (descriptor_strong == nullptr) {
            THROW_IE_EXCEPTION << "[SAMPLES] internal error - invalid descriptor";
        }
        std::shared_ptr<IDescriptorDistance> distance_strong =
            std::make_shared<CosDistance>(descriptor_strong->size());

        // openvino Inference Engine初始化后的
        tracker->set_descriptor_strong(descriptor_strong);
        tracker->set_distance_strong(distance_strong);
    } else {
        std::cout << "WARNING: Either reid model or reid weights "
            << "were not specified. "
            << "Only fast reidentification approach will be used." << std::endl;
    }

    return tracker;
}

bool ParseAndCheckCommandLine(int argc, char *argv[]) {
    // ---------------------------Parsing and validation of input args--------------------------------------

    gflags::ParseCommandLineNonHelpFlags(&argc, &argv, true);
    if (FLAGS_h) {
        showUsage();
        return false;
    }

    std::cout << "[ INFO ] Parsing input parameters" << std::endl;

    // if (FLAGS_i.empty()) {
    //     throw std::logic_error("Parameter -i is not set");
    // }

    // if (FLAGS_m_det.empty()) {
    //     throw std::logic_error("Parameter -m_det is not set");
    // }

    // if (FLAGS_m_reid.empty()) {
    //     throw std::logic_error("Parameter -m_reid is not set");
    // }

    return true;
}

int tracker_init_work(int argc, char **argv) {
    std::cout << "InferenceEngine: " << GetInferenceEngineVersion() << std::endl;

    if (!ParseAndCheckCommandLine(argc, argv)) {
        return 0;
    }

    // Reading command line parameters.
    // auto video_path = FLAGS_i;
    auto video_path = "/home/dahu/Desktop/pedestrain/pedestrain_tracker/video/double_no_cut1.mp4";

    // auto det_model = FLAGS_m_det;
    // auto det_weights = fileNameNoExt(FLAGS_m_det) + ".bin";

    // auto det_model = "/home/dahu/Desktop/pedestrain/pedestrain_tracker/model/LFFD/LFFD-fp16.xml";
    // auto det_weights = "/home/dahu/Desktop/pedestrain/pedestrain_tracker/model/LFFD/LFFD-fp16.bin";

    // auto reid_model = FLAGS_m_reid;
    // auto reid_weights = fileNameNoExt(FLAGS_m_reid) + ".bin";

    // auto reid_model = "/home/dahu/Desktop/pedestrain/pedestrain_tracker/model/DeepSort/DeepSort-fp16.xml";
    // auto reid_weights = "/home/dahu/Desktop/pedestrain/pedestrain_tracker/model/DeepSort/DeepSort-fp16.bin";

    auto detlog_out = FLAGS_out;  // default ""

    auto detector_mode = FLAGS_d_det;
    auto reid_mode = FLAGS_d_reid;
    std::string det_model,det_weights,reid_model,reid_weights;
    if(reid_mode == "CPU"){
     det_model = "/home/dahu/Desktop/pedestrain/pedestrain_tracker/model/LFFD/LFFD.xml";
     det_weights = "/home/dahu/Desktop/pedestrain/pedestrain_tracker/model/LFFD/LFFD.bin";

     reid_model = "/home/dahu/Desktop/pedestrain/pedestrain_tracker/model/DeepSort/DeepSort.xml";
     reid_weights = "/home/dahu/Desktop/pedestrain/pedestrain_tracker/model/DeepSort/DeepSort.bin";

    //  reid_model = "/home/dahu/Desktop/pedestrain/pedestrain_tracker/model/0079/person-reidentification-retail-0079.xml";
    //  reid_weights = "/home/dahu/Desktop/pedestrain/pedestrain_tracker/model/0079/person-reidentification-retail-0079.bin";
     
    //  reid_model = "/home/dahu/Desktop/pedestrain/pedestrain_tracker/model/0076/person-reidentification-retail-0076.xml";
    //  reid_weights = "/home/dahu/Desktop/pedestrain/pedestrain_tracker/model/0076/person-reidentification-retail-0076.bin";

    }else if(reid_mode == "HETERO:FPGA,CPU"){

     det_model = "/home/dahu/Desktop/pedestrain/pedestrain_tracker/model/LFFD/LFFD-fp16.xml";
     det_weights = "/home/dahu/Desktop/pedestrain/pedestrain_tracker/model/LFFD/LFFD-fp16.bin";

     reid_model = "/home/dahu/Desktop/pedestrain/pedestrain_tracker/model/DeepSort/DeepSort-fp16.xml";
     reid_weights = "/home/dahu/Desktop/pedestrain/pedestrain_tracker/model/DeepSort/DeepSort-fp16.bin";

    //  reid_model = "/home/dahu/Desktop/pedestrain/pedestrain_tracker/model/0079/person-reidentification-retail-0079-fp16.xml";
    //  reid_weights = "/home/dahu/Desktop/pedestrain/pedestrain_tracker/model/0079/person-reidentification-retail-0079-fp16.bin";

    //  reid_model = "/home/dahu/Desktop/pedestrain/pedestrain_tracker/model/0076/person-reidentification-retail-0076-fp16.xml";
    //  reid_weights = "/home/dahu/Desktop/pedestrain/pedestrain_tracker/model/0076/person-reidentification-retail-0076-fp16.bin";
    }

    auto custom_cpu_library = FLAGS_l;
    auto path_to_custom_layers = FLAGS_c;
    bool should_use_perf_counter = FLAGS_pc;   // default:false ; performance_counter_message

    bool should_print_out = FLAGS_r; //default false

    bool should_show = !FLAGS_no_show;      // default false
    int delay = FLAGS_delay;                // default 3
    if (!should_show)
        delay = -1;
    should_show = (delay >= 0);

    bool should_save_det_log = !detlog_out.empty(); // false

    std::vector<std::string> devices{detector_mode, reid_mode};
    // std::vector<std::string> devices{detector_mode};
    std::map<std::string, InferencePlugin> plugins_for_devices =
        LoadPluginForDevices(
            devices, custom_cpu_library, path_to_custom_layers,
            should_use_perf_counter);  

    DetectorConfig detector_confid(det_model, det_weights);
    auto detector_plugin = plugins_for_devices[detector_mode];

    pedestrian_detector.ObjectDetector_Create(detector_confid, detector_plugin);   

    auto reid_plugin = plugins_for_devices[reid_mode];          //  
    bool should_keep_tracking_info = should_save_det_log || should_print_out;
    // std::cout<<"part1!!!"<<std::endl;
    tracker =  CreatePedestrianTracker(reid_model, reid_weights, reid_plugin,
                                should_keep_tracking_info);     // create tracker

    // std::cout << " FPGA FPS : " << 1000.0/total.count() << std::endl;

    // if (should_keep_tracking_info) {
    //     DetectionLog log = tracker->GetDetectionLog(true);

    //     if (should_save_det_log)
    //         SaveDetectionLogToTrajFile(detlog_out, log);
    //     if (should_print_out)
    //         PrintDetectionLog(log);
    // }

    // if (should_use_perf_counter) {
    //     pedestrian_detector.PrintPerformanceCounts();
    //     tracker->PrintReidPerformanceCounts();
    // }
    return 0;
}

int tracker_work(ImageWithFrameIndex pair,cv::Rect rect,double video_fps){

        cv::Mat frame = pair.first;     // first is a image 
        cv::Mat frame1 = frame(rect); 

        int frame_idx = pair.second;    // second if a index number
        
        // std::cout << "1:" << frame.cols << "  " << frame.rows << std::endl;    //1920,1080
        pedestrian_detector.submitFrame(frame1, frame_idx);
        pedestrian_detector.waitAndFetchResults();           // the result of detection model
                                                             // wait and fetch the result

        detections = pedestrian_detector.getResults();       // get private variable
        
        // frame_idx -1 confidence 0
        // for (const auto &detection : detections) {
        //    std::cout<< detection.frame_idx<< "  " << detection.object_id << "  "<<detection.confidence<< "  "<<detection.timestamp<<std::endl; 
        // }

        // write to txt

        // timestamp in milliseconds
        uint64_t cur_timestamp = static_cast<uint64_t >(1000.0 / video_fps * frame_idx);
        tracker->Process(frame, detections, cur_timestamp);  // process the tracker  
        
        return 0;
}

int tracker_work_fw(ImageWithFrameIndex pair,cv::Rect rect,double video_fps,std::ofstream &fp){

        cv::Mat frame = pair.first;     // first is a image 
        cv::Mat frame1 = frame(rect); 

        int frame_idx = pair.second;    // second if a index number
        
        // std::cout << "1:" << frame.cols << "  " << frame.rows << std::endl;    //1920,1080
        pedestrian_detector.submitFrame(frame1, frame_idx);
        pedestrian_detector.waitAndFetchResults();           // the result of detection model
                                                             // wait and fetch the result

        detections = pedestrian_detector.getResults();       // get private variable
       
        // write to txt
        char det_out[100];
        for (const auto &detection : detections) {
            sprintf(det_out,"%d %f %d %d %d %d\n",detection.frame_idx,detection.confidence,detection.rect.x,
                                                  detection.rect.y,detection.rect.width,detection.rect.height);
            // std::cout<<det_out<<std::endl;
            fp << det_out;
        }


        // timestamp in milliseconds
        uint64_t cur_timestamp = static_cast<uint64_t >(1000.0 / video_fps * frame_idx);
        tracker->Process(frame, detections, cur_timestamp);  // process the tracker  
        
        return 0;
}

int tracker_work_fr(ImageWithFrameIndex pair,cv::Rect rect,double video_fps,std::ifstream &fp){

        cv::Mat frame = pair.first;     // first is a image 
        cv::Mat frame1 = frame(rect); 

        int frame_idx = pair.second;    // second if a index number
        
        // std::cout << "1:" << frame.cols << "  " << frame.rows << std::endl;    //1920,1080
        // pedestrian_detector.submitFrame(frame1, frame_idx);
        // pedestrian_detector.waitAndFetchResults();           // the result of detection model
        //                                                      // wait and fetch the result

        // detections = pedestrian_detector.getResults();       // get private variable

        // read tracker object from txt
        
        // write here
        detections.clear();    
        TrackedObject detection;
        while(frame_idx == current_frame_idx && current_frame_idx < 11000){
             if(frame_idx == 1){
                 fp >> current_frame_idx;
                 if(current_frame_idx == 1){
                     detection.frame_idx = current_frame_idx;
                     fp >> detection.confidence;
                     fp >> detection.rect.x;
                     fp >> detection.rect.y;
                     fp >> detection.rect.width;
                     fp >> detection.rect.height;
                     detections.emplace_back(detection);
                 }
             }else{
                     detection.frame_idx = current_frame_idx;
                     fp >> detection.confidence;
                     fp >> detection.rect.x;
                     fp >> detection.rect.y;
                     fp >> detection.rect.width;
                     fp >> detection.rect.height;
                     detections.emplace_back(detection);
                     fp >> current_frame_idx;
             }
        }

        // timestamp in milliseconds
        uint64_t cur_timestamp = static_cast<uint64_t >(1000.0 / video_fps * frame_idx);
        tracker->Process(frame, detections, cur_timestamp);  // process the tracker  
        
        return 0;
}

int tracker_init(int argc, char **argv) {
    try {
        tracker_init_work(argc, argv);
    }
    catch (const std::exception& error) {
        std::cerr << "[ ERROR ] " << error.what() << std::endl;
        return 1;
    }
    catch (...) {
        std::cerr << "[ ERROR ] Unknown/internal exception happened." << std::endl;
        return 1;
    }

    std::cout << "[ INFO ] Tracker_init successful" << std::endl;

    return 0;
}
