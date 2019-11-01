// Copyright (C) 2018-2019 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include <map>
#include <set>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>
#include <utility>
#include <limits>
#include <algorithm>

#include "core.hpp"
#include "tracker.hpp"
#include "utils.hpp"
#include "kuhn_munkres.hpp"
#include "tracker_main.hpp"

int idx=0;
namespace {
cv::Point Center(const cv::Rect& rect) {
    return cv::Point(static_cast<int>(rect.x + rect.width * 0.5),
                     static_cast<int>(rect.y + rect.height * 0.5));
}

std::vector<cv::Point> Centers(const TrackedObjects &detections) {
    std::vector<cv::Point> centers(detections.size());
    for (size_t i = 0; i < detections.size(); i++) {
        centers[i] = Center(detections[i].rect);
    }
    return centers;
}

DetectionLog ConvertTracksToDetectionLog(const ObjectTracks& tracks) {
    DetectionLog log;

    // Combine detected objects by respective frame indices.
    std::map<int, TrackedObjects> objects;
    for (const auto& track : tracks)
        for (const auto& object : track.second) {
            auto itr = objects.find(object.frame_idx);
            if (itr != objects.end())
                itr->second.emplace_back(object);
            else
                objects.emplace(object.frame_idx, TrackedObjects{object});
        }

    for (const auto& frame_res : objects) {
        DetectionLogEntry entry;
        entry.frame_idx = frame_res.first;
        entry.objects = std::move(frame_res.second);
        log.push_back(std::move(entry));
    }

    return log;
}

inline bool IsInRange(float val, float min, float max) {
    return min <= val && val <= max;
}

inline bool IsInRange(float val, cv::Vec2f range) {
    return IsInRange(val, range[0], range[1]);
}

std::vector<cv::Scalar> GenRandomColors(int colors_num) {
    std::vector<cv::Scalar> colors(colors_num);
    for (int i = 0; i < colors_num; i++) {
        colors[i] = cv::Scalar(static_cast<uchar>(255. * rand() / RAND_MAX),  // NOLINT
                               static_cast<uchar>(255. * rand() / RAND_MAX),  // NOLINT
                               static_cast<uchar>(255. * rand() / RAND_MAX));  // NOLINT
    }
    return colors;
}

}  // anonymous namespace

// 跟踪参数汇总
TrackerParams::TrackerParams()
    : min_track_duration(200),  // 1s 判断目标id是否有效的最小时间间隔
    forget_delay(150),
    aff_thr_fast(0.999f),      // 0.8
    aff_thr_strong(0.5f),     // 0.75
    shape_affinity_w(0.0f),   // 0.5  外框尺寸亲和度 这里设为0忽略外框的变化
    motion_affinity_w(0.2f),  // 0.2  动作亲和度 顶点坐标与外框的联系 置0忽略此参数
    time_affinity_w(0.0f),    // 0.0  时域亲和度
    min_det_conf(0.80f),      // 0.65 检测概率阈值
    bbox_aspect_ratios_range(0.888f, 8.0f), // height / width
    bbox_heights_range(150, 1000),
    predict(25),                  // 25 取距离当前帧之前predict帧的对象框x，y位移和长宽取平均 作为预测框位置与大小
    strong_affinity_thr(0.2805f), // 0.2805f
    reid_thr(0.85f),              // 0.61 此值越高 越容易跟丢 越不容易错
    drop_forgotten_tracks(true),
    max_num_objects_in_track(300) {}

void ValidateParams(const TrackerParams &p) {
    PT_CHECK_GE(p.min_track_duration, static_cast<size_t>(100));
    PT_CHECK_LE(p.min_track_duration, static_cast<size_t>(10000));

    PT_CHECK_LE(p.forget_delay, static_cast<size_t>(10000));

    PT_CHECK_GE(p.aff_thr_fast, 0.0f);
    PT_CHECK_LE(p.aff_thr_fast, 1.0f);

    PT_CHECK_GE(p.aff_thr_strong, 0.0f);
    PT_CHECK_LE(p.aff_thr_strong, 1.0f);

    PT_CHECK_GE(p.shape_affinity_w, 0.0f);
    PT_CHECK_LE(p.shape_affinity_w, 100.0f);

    PT_CHECK_GE(p.motion_affinity_w, 0.0f);
    PT_CHECK_LE(p.motion_affinity_w, 100.0f);

    PT_CHECK_GE(p.time_affinity_w, 0.0f);
    PT_CHECK_LE(p.time_affinity_w, 100.0f);

    PT_CHECK_GE(p.min_det_conf, 0.0f);
    PT_CHECK_LE(p.min_det_conf, 1.0f);

    PT_CHECK_GE(p.bbox_aspect_ratios_range[0], 0.0f);   // height/width
    PT_CHECK_LE(p.bbox_aspect_ratios_range[1], 10.0f);
    PT_CHECK_LT(p.bbox_aspect_ratios_range[0], p.bbox_aspect_ratios_range[1]);

    PT_CHECK_GE(p.bbox_heights_range[0], 10.0f);
    PT_CHECK_LE(p.bbox_heights_range[1], 1080.0f);
    PT_CHECK_LT(p.bbox_heights_range[0], p.bbox_heights_range[1]);

    PT_CHECK_GE(p.predict, 0);
    PT_CHECK_LE(p.predict, 10000);

    PT_CHECK_GE(p.strong_affinity_thr, 0.0f);
    PT_CHECK_LE(p.strong_affinity_thr, 1.0f);

    PT_CHECK_GE(p.reid_thr, 0.0f);
    PT_CHECK_LE(p.reid_thr, 1.0f);

    if (p.max_num_objects_in_track > 0) {
        int min_required_track_length = static_cast<int>(p.forget_delay);
        PT_CHECK_GE(p.max_num_objects_in_track, min_required_track_length);
        PT_CHECK_LE(p.max_num_objects_in_track, 10000);
    }
}

// Returns confusion matrix as:
//   |tp fn|
//   |fp tn|
cv::Mat PedestrianTracker::ConfusionMatrix(const std::vector<Match> &matches) {
    const bool kNegative = false;
    cv::Mat conf_mat(2, 2, CV_32F, cv::Scalar(0));
    for (const auto &m : matches) {
        conf_mat.at<float>(m.gt_label == kNegative, m.pr_label == kNegative)++;
    }
    return conf_mat;
}

PedestrianTracker::PedestrianTracker(const TrackerParams &params)
    : params_(params),
    descriptor_strong_(nullptr),
    distance_strong_(nullptr),
    collect_matches_(true),
    tracks_counter_(0),
    valid_tracks_counter_(0),
    frame_size_(0, 0),
    prev_timestamp_(std::numeric_limits<uint64_t>::max()) {
        ValidateParams(params);
    }

// Pipeline parameters getter.
const TrackerParams &PedestrianTracker::params() const { return params_; }

// Pipeline parameters setter.
void PedestrianTracker::set_params(const TrackerParams &params) {
    ValidateParams(params);
    params_ = params;
}

// Descriptor fast getter.
const PedestrianTracker::Descriptor &PedestrianTracker::descriptor_fast() const {
    return descriptor_fast_;
}

// Descriptor fast setter.
void PedestrianTracker::set_descriptor_fast(const Descriptor &val) {
    descriptor_fast_ = val;
}

// Descriptor strong getter.
const PedestrianTracker::Descriptor &PedestrianTracker::descriptor_strong() const {
    return descriptor_strong_;
}

// Descriptor strong setter.
void PedestrianTracker::set_descriptor_strong(const Descriptor &val) {
    descriptor_strong_ = val;
}

// Distance fast getter.
const PedestrianTracker::Distance &PedestrianTracker::distance_fast() const { return distance_fast_; }

// Distance fast setter.
void PedestrianTracker::set_distance_fast(const Distance &val) { distance_fast_ = val; }

// Distance strong getter.
const PedestrianTracker::Distance &PedestrianTracker::distance_strong() const { return distance_strong_; }

// Distance strong setter.
void PedestrianTracker::set_distance_strong(const Distance &val) { distance_strong_ = val; }

// Returns all tracks including forgotten (lost too many frames ago).
const std::unordered_map<size_t, Track> &
PedestrianTracker::tracks() const {
    return tracks_;
}

// Returns indexes of active tracks only.
const std::set<size_t> &PedestrianTracker::active_track_ids() const {
    return active_track_ids_;
}


// Returns detection log which is used for tracks saving.
DetectionLog PedestrianTracker::GetDetectionLog(const bool valid_only) const {
    return ConvertTracksToDetectionLog(all_tracks(valid_only));
}

// Returns decisions made by heuristic based on fast distance/descriptor and
// shape, motion and time affinity.
const std::vector<PedestrianTracker::Match> &
PedestrianTracker::base_classifier_matches() const {
    return base_classifier_matches_;
}

// Returns decisions made by heuristic based on strong distance/descriptor
// and
// shape, motion and time affinity.
const std::vector<PedestrianTracker::Match> &PedestrianTracker::reid_based_classifier_matches() const {
    return reid_based_classifier_matches_;
}

// Returns decisions made by strong distance/descriptor affinity.
const std::vector<PedestrianTracker::Match> &PedestrianTracker::reid_classifier_matches() const {
    return reid_classifier_matches_;
}

// chect confidence and chect params in range
TrackedObjects PedestrianTracker::FilterDetections(
    const TrackedObjects &detections) const {  // TrackedObjects is the vector of TrackedObeject
                                               // contrain : confidence rect frame_idx object_id timestamp
    TrackedObjects filtered_detections;
    for (const auto &det : detections) {       // generate an iterator
        float aspect_ratio = static_cast<float>(det.rect.height) / det.rect.width;
        // min_det_conf: [0,1]
        // bbox_aspect_ratios_range:[0,10]
        // bbox_heights_range:[10,1000]
        if (det.confidence > params_.min_det_conf &&
            IsInRange(aspect_ratio, params_.bbox_aspect_ratios_range) &&
            IsInRange(static_cast<float>(det.rect.height), params_.bbox_heights_range)) {
            filtered_detections.emplace_back(det);// put det into filtered_detections
        }
    }
    return filtered_detections;
}

// 解决指派问题
void PedestrianTracker::SolveAssignmentProblem(
    const std::set<size_t> &track_ids, const TrackedObjects &detections,
    const std::vector<cv::Mat> &descriptors, float thr,
    std::set<size_t> *unmatched_tracks, std::set<size_t> *unmatched_detections,
    std::set<std::tuple<size_t, size_t, float>> *matches) {
    // assert
    PT_CHECK(unmatched_tracks);
    PT_CHECK(unmatched_detections);
    unmatched_tracks->clear();
    unmatched_detections->clear();

    PT_CHECK(!track_ids.empty());
    PT_CHECK(!detections.empty());
    PT_CHECK(descriptors.size() == detections.size());
    PT_CHECK(matches);
    matches->clear();

    cv::Mat dissimilarity;
    // 计算差异矩阵
    // 激活跟踪目标与每个检测目标的差异（1-affinity）
    // 亲和度包括：矩形框外型相似度，动作相似度（前后距离与形状的关系），时间距离，像素相似性（匹配相似度）
    // track_ids = active_tracks
    ComputeDissimilarityMatrix(track_ids, detections, descriptors,
                               &dissimilarity);

    // 匈牙利算法
    // 输入差异矩阵 使用带权重参数的匈牙利算法
    auto res = KuhnMunkres().Solve(dissimilarity);   // 返回 std::vector<size_t>

    for (size_t i = 0; i < detections.size(); i++) {
        unmatched_detections->insert(i);
    }

    size_t i = 0;
    for (auto id : track_ids) {
        if (res[i] < detections.size()) {
            // res[i]对应的第几个检测框
            matches->emplace(id, res[i], 1 - dissimilarity.at<float>(i, res[i]));  // id：正在跟踪的id res[i]：匈牙利匹配当前帧与保存目标对应id
        } else {
            unmatched_tracks->insert(id);  // 插入一个数据
        }
        i++;
    }
}

// 输入参数 valid_only选择是否只返回有效id
const ObjectTracks PedestrianTracker::all_tracks(bool valid_only) const {
    ObjectTracks all_objects;
    size_t counter = 0;

    std::set<size_t> sorted_ids;
    for (const auto &pair : tracks()) {
        sorted_ids.emplace(pair.first);
    }

    for (size_t id : sorted_ids) {
        if (!valid_only || IsTrackValid(id)) {
            TrackedObjects filtered_objects;
            for (const auto &object : tracks().at(id).objects) {
                filtered_objects.emplace_back(object);
                filtered_objects.back().object_id = counter;
            }
            all_objects.emplace(counter++, filtered_objects);
        }
    }
    return all_objects;
}

// track_id, params().predict, tracks_.at(track_id).lost
cv::Rect PedestrianTracker::PredictRect(size_t id, size_t k,
                                        size_t s) const {
    const auto &track = tracks_.at(id);
    PT_CHECK(!track.empty());
 
    if (track.size() == 1) {
        return track[0].rect;     // 只保存类一帧的矩形框数据
    }

    size_t start_i = track.size() > k ? track.size() - k : 0;
    float width = 0, height = 0;

    // k个矩形框相加
    for (size_t i = start_i; i < track.size(); i++) {
        width += track[i].rect.width;
        height += track[i].rect.height;
    }

    PT_CHECK(track.size() - start_i > 0);
    width /= (track.size() - start_i);      // 取平均
    height /= (track.size() - start_i);     // 取平均

    float delim = 0;                        // 最后一帧和k帧前间隔的帧数
    cv::Point2f d(0, 0);                    // 后k个框中心坐标的x，y偏移和

    
    for (size_t i = start_i + 1; i < track.size(); i++) {
        d += cv::Point2f(Center(track[i].rect) - Center(track[i - 1].rect));
        delim += (track[i].frame_idx - track[i - 1].frame_idx);
    }


    // 取左下角坐标的位移
    // for (size_t i = start_i + 1; i < track.size(); i++) {

    //     d += cv::Point2f((track[i].rect.y+track[i].rect.height) - (track[i-1].rect.y+track[i-1].rect.height),
    //                      (track[i].rect.x - track[i-1].rect.x));

    //     delim += (track[i].frame_idx - track[i - 1].frame_idx);
    // }

    if (delim) {
        d /= delim;       // 每帧x，y平均位移
    }

    s += 1;               // track.lost

    cv::Point c = Center(track.back().rect);   //最后一帧矩形框中点

    // return cv::Rect(static_cast<int>(c.x - width / 2 + d.x * s),
    //                 static_cast<int>(c.y - height / 2 + d.y * s),
    //                 static_cast<int>(width),
    //                 static_cast<int>(height));

    return cv::Rect(static_cast<int>(c.x - width / 2 + d.x * s),
                static_cast<int>(c.y - height / 2 + d.y * s),
                static_cast<int>(track.back().rect.width),
                static_cast<int>(track.back().rect.height));
    // return track.back().rect;
}



bool PedestrianTracker::EraseTrackIfBBoxIsOutOfFrame(size_t track_id) {
    if (tracks_.find(track_id) == tracks_.end()) return true;
    auto c = Center(tracks_.at(track_id).predicted_rect);
    if (!prev_frame_size_.empty() &&
        (c.x < 0 || c.y < 0 || c.x > prev_frame_size_.width ||
         c.y > prev_frame_size_.height)) {
        tracks_.at(track_id).lost = params_.forget_delay + 1;    // 如果边界框超过图像范围，lost越界赋值
        for (auto id : active_track_ids()) {
            size_t min_id = std::min(id, track_id);
            size_t max_id = std::max(id, track_id);
            tracks_dists_.erase(std::pair<size_t, size_t>(min_id, max_id));
        }
        active_track_ids_.erase(track_id);
        return true;
    }
    return false;
}

bool PedestrianTracker::EraseTrackIfItWasLostTooManyFramesAgo(
    size_t track_id) {
    if (tracks_.find(track_id) == tracks_.end()) return true;
    if (tracks_.at(track_id).lost > params_.forget_delay) {
        // 这里有点问题
        for (auto id : active_track_ids()) {
            size_t min_id = std::min(id, track_id);
            size_t max_id = std::max(id, track_id);
            tracks_dists_.erase(std::pair<size_t, size_t>(min_id, max_id));
        }
        active_track_ids_.erase(track_id);

        return true;
    }
    return false;
}

bool PedestrianTracker::UpdateLostTrackAndEraseIfItsNeeded(
    size_t track_id) {
    tracks_.at(track_id).lost++;
    tracks_.at(track_id).predicted_rect =
        PredictRect(track_id, params().predict, tracks_.at(track_id).lost);  // params().predict = 25

    bool erased = EraseTrackIfBBoxIsOutOfFrame(track_id);                    // 返回擦除结果
    if (!erased) erased = EraseTrackIfItWasLostTooManyFramesAgo(track_id);
    return erased;
}

// 边框超出图像范围的要擦除 丢失过长时间的要擦除
void PedestrianTracker::UpdateLostTracks(
    const std::set<size_t> &track_ids) {
    for (auto track_id : track_ids) {
        UpdateLostTrackAndEraseIfItsNeeded(track_id);
    }
}

void PedestrianTracker::Process(const cv::Mat &frame,
                                const TrackedObjects &input_detections,
                                uint64_t timestamp) {
    if (prev_timestamp_ != std::numeric_limits<uint64_t>::max())
        PT_CHECK_LT(prev_timestamp_, timestamp);    // prev_timestamp_ < timestamp
 
    if (frame_size_ == cv::Size(0, 0)) {
        frame_size_ = frame.size();                 // init the frame_size_
    } else {
        PT_CHECK_EQ(frame_size_, frame.size());     // frame_size_ = frame.size()
    }
    
    //chect params in range 
    //检查检测框，筛选符合要求的框
    //height/width (0.666f, 5.0f) 
    //height (40, 1000)
    TrackedObjects detections = FilterDetections(input_detections);
    for (auto &obj : detections) {
        obj.timestamp = timestamp;    //set the detections' timestamp
    }

    std::vector<cv::Mat> descriptors_fast; // 存放检测框中的图像
    
    // computes image descriptors in batch store in descriptors_fast
    // 根据检测结果，把检测出来的对象截图，存到descriptors_fast
    ComputeFastDesciptors(frame, detections, &descriptors_fast);

    auto active_tracks = active_track_ids_;      // std::set<size_t>

    // 只有第一帧active_tracks为空
    // if(!active_tracks.empty() && !detections.empty()){
    //     std::cout<<"no tracker"<<std::endl;
    // }else std::cout<<"track"<<std::endl;


    // get tracks and detections
    if (!active_tracks.empty() && !detections.empty()) {
        std::set<size_t> unmatched_tracks, unmatched_detections;  //set:定义一个容器，由RB二叉树构成，每个数据结构为size_t
        std::set<std::tuple<size_t, size_t, float>> matches;
        // 解决指派问题
        // 计算差异矩阵 通过带权重的匈牙利算法 匹配跟踪目标 统计不匹配检测框
        // aff_thr_fast = default 0.8 这里没用上呢
        SolveAssignmentProblem(active_tracks, detections, descriptors_fast,
                               params_.aff_thr_fast, &unmatched_tracks,
                               &unmatched_detections, &matches);

        std::map<size_t, std::pair<bool, cv::Mat>> is_matching_to_track;  // pair 组合两个数据类型，用在成对的数据处理上

        // std::cout<<distance_strong_<<std::endl;
        
        // strong distance 深度特征余弦距离
        // 如果加载了reid模型就会进入此部分
        if (distance_strong_) {      // 0x12fc4f0

            // 得到传统算法匹配的目标对，并且无法作出确信匹配的对，再做深度特征提取，重新匹配
            // 对于传统算法匹配置信度很高的，认为匹配完成，对于置信度很低的，认为不是一个对象
            std::vector<std::pair<size_t, size_t>> reid_track_and_det_ids =
                GetTrackToDetectionIds(matches);
            // matching_to_track pair<bool,mat>
 
            // 在这里计算深度特征的余弦距离和亲和度，is_matching_to_track: <match?,DeepFeature>
            is_matching_to_track = StrongMatching(
                frame, detections, reid_track_and_det_ids);
        }

        // matches:跟踪对象与检测对象的检测关系
        for (const auto &match : matches) {
            size_t track_id = std::get<0>(match);
            size_t det_id = std::get<1>(match);
            float conf = std::get<2>(match);

            auto last_det = tracks_.at(track_id).objects.back();
            last_det.rect = tracks_.at(track_id).predicted_rect;       // 这里又用到了预测框

            // collect_matches_ : ture
            if (collect_matches_ && last_det.object_id >= 0 &&
                detections[det_id].object_id >= 0) {
                base_classifier_matches_.emplace_back(
                    tracks_.at(track_id).objects.back(), last_det.rect,
                    detections[det_id], conf > params_.aff_thr_fast);
            }

            if (conf > params_.aff_thr_fast) {        // aff_thr_fast = 0.8/0.99
                // 更新跟踪目标的参数与数据
                AppendToTrack(frame, track_id, detections[det_id],
                              descriptors_fast[det_id], cv::Mat());
                unmatched_detections.erase(det_id);
            } else {
                if (conf > params_.strong_affinity_thr) { // strong_affinity_thr = 0.285
                    // 这里加入深度匹配成功的跟踪对象
                    if (distance_strong_ && is_matching_to_track[track_id].first) {
                        AppendToTrack(frame, track_id, detections[det_id],
                                      descriptors_fast[det_id],
                                      is_matching_to_track[track_id].second.clone());
                    } else {
                        // 这里擦除两种情况下的跟踪目标 检测框超过图像范围
                        if (UpdateLostTrackAndEraseIfItsNeeded(track_id)) {
                            AddNewTrack(frame, detections[det_id], descriptors_fast[det_id],
                                        distance_strong_
                                        ? is_matching_to_track[track_id].second.clone()
                                        : cv::Mat());
                        }
                    }

                    unmatched_detections.erase(det_id);
                } else {
                    unmatched_tracks.insert(track_id);
                }
            }
        }

        AddNewTracks(frame, detections, descriptors_fast, unmatched_detections);
        UpdateLostTracks(unmatched_tracks);

        // 擦除边界框出界的目标
        for (size_t id : active_tracks) {
            EraseTrackIfBBoxIsOutOfFrame(id);
        }
    } else { // 没有激活的跟踪目标（第一帧）或没有检测框
        // 这里没有加入深度特征
        AddNewTracks(frame, detections, descriptors_fast);   // 若是第一帧检测到的框都作为跟踪目标，并加入激活跟踪目标队列中
        UpdateLostTracks(active_tracks);                     // 若是没有检测框，则已激活的跟踪目标lost++
    }

    prev_frame_size_ = frame.size();
    // drop_forgotten_tracks:ture
    if (params_.drop_forgotten_tracks) DropForgottenTracks();

    tracks_dists_.clear();
    prev_timestamp_ = timestamp;
}

void PedestrianTracker::DropForgottenTracks() {
    std::unordered_map<size_t, Track> new_tracks;
    std::set<size_t> new_active_tracks;

    size_t max_id = 0;
    if (!active_track_ids_.empty())
        max_id =
            *std::max_element(active_track_ids_.begin(), active_track_ids_.end());

    const size_t kMaxTrackID = 10000;
    // const size_t kMaxTrackID = 2;
    bool reassign_id = max_id > kMaxTrackID;

    size_t counter = 0;
    for (const auto &pair : tracks_) {
        // 判断此id对应对象的lost是否大与最大值 大于返回1
        if (!IsTrackForgotten(pair.first)) { // lost小于最大值

            // new_tracks.emplace(reassign_id ? counter : pair.first, pair.second);  // 如果跟踪对象大于1w，则id从0开始
            // new_active_tracks.emplace(reassign_id ? counter : pair.first);
            // counter++;
            if(pair.first==3||pair.first==6||pair.first==8||pair.first==10){
                new_tracks.emplace(1, pair.second);  // 如果跟踪对象大于1w，则id从0开始
                new_active_tracks.emplace(1);
                counter++;
            }else if(pair.first==11||pair.first==7){
                new_tracks.emplace(0, pair.second);  // 如果跟踪对象大于1w，则id从0开始
                new_active_tracks.emplace(0);
                counter++;
            }else{
                new_tracks.emplace(reassign_id ? counter : pair.first, pair.second);  // 如果跟踪对象大于1w，则id从0开始
                new_active_tracks.emplace(reassign_id ? counter : pair.first);
                counter++;
            }
   
        } else { // lost大于最大值
        // 判断是否为有效id，边界框不为空，距离第一帧时间超过200ms
            if (IsTrackValid(pair.first)) {
                valid_tracks_counter_++;     // 丢掉的有效id的个数
                // std::cout<<"cant believe!"<<std::endl;
            }
        }
    }
    tracks_.swap(new_tracks);            // 交换两个map中的所有元素
    active_track_ids_.swap(new_active_tracks);

    tracks_counter_ = reassign_id ? counter : tracks_counter_;
}

void PedestrianTracker::DropForgottenTrack(size_t track_id) {
    PT_CHECK(IsTrackForgotten(track_id));
    PT_CHECK(active_track_ids_.count(track_id) == 0);
    tracks_.erase(track_id);
}

float PedestrianTracker::ShapeAffinity(float weight, const cv::Rect &trk,
                                       const cv::Rect &det) {
    float w_dist = static_cast<float>(std::abs(trk.width - det.width) / (trk.width + det.width));
    float h_dist = static_cast<float>(std::abs(trk.height - det.height) / (trk.height + det.height));
    return static_cast<float>(exp(static_cast<double>(-weight * (w_dist + h_dist))));
}

float PedestrianTracker::MotionAffinity(float weight, const cv::Rect &trk,
                                        const cv::Rect &det) {
    float x_dist = static_cast<float>(trk.x - det.x) * (trk.x - det.x) /
        (det.width * det.width);
    float y_dist = static_cast<float>(trk.y - det.y) * (trk.y - det.y) /
        (det.height * det.height);
    return static_cast<float>(exp(static_cast<double>(-weight * (x_dist + y_dist))));
}

// 这里weight一般给0
float PedestrianTracker::TimeAffinity(float weight, const float &trk_time,
                                      const float &det_time) {
    return static_cast<float>(exp(static_cast<double>(-weight * std::fabs(trk_time - det_time))));
}

// out matrix to store the descriptors
void PedestrianTracker::ComputeFastDesciptors(
    const cv::Mat &frame, const TrackedObjects &detections,
    std::vector<cv::Mat> *desriptors) {
    *desriptors = std::vector<cv::Mat>(detections.size(), cv::Mat());
    for (size_t i = 0; i < detections.size(); i++) {
        // resize
        descriptor_fast_->Compute(frame(detections[i].rect).clone(),
                                  &((*desriptors)[i]));
    }
}

// 计算差异矩阵，行数：激活跟踪目标 列数：检测框结果
// 差异矩阵：1-激活跟踪目标相对于每个检测框的亲和度
// 这里用的预测框来做亲和度计算
void PedestrianTracker::ComputeDissimilarityMatrix(
    const std::set<size_t> &active_tracks, const TrackedObjects &detections,
    const std::vector<cv::Mat> &descriptors_fast,
    cv::Mat *dissimilarity_matrix) {
    cv::Mat am(active_tracks.size(), detections.size(), CV_32F, cv::Scalar(0));   // mat(row,col,type,init)
    size_t i = 0;
    for (auto id : active_tracks) {
        auto ptr = am.ptr<float>(i);    // 取mat行头指针 行数：激活跟踪对象
        for (size_t j = 0; j < descriptors_fast.size(); j++) {
            auto last_det = tracks_.at(id).objects.back();  // tracks_:unorder_map<size_t,Track>,objects:TrackIObjects
            last_det.rect = tracks_.at(id).predicted_rect;  // predicted_rect:Rectangle that represents predicted position
                                                            // and size of bounding box if track has been lost.
            // fast closeness 
            // 跟踪目标最新的（图片和预测位置）与（检测图片和位置）的ShapeAffinity,MotionAffinity,TimeAffinity，
            ptr[j] = AffinityFast(tracks_.at(id).descriptor_fast, last_det,
                                  descriptors_fast[j], detections[j]);
        }
        i++;
    }
    *dissimilarity_matrix = 1.0 - am;
}

std::vector<float> PedestrianTracker::ComputeDistances(
    const cv::Mat &frame,                          // 当前帧
    const TrackedObjects& detections,              // 检测框
    const std::vector<std::pair<size_t, size_t>> &track_and_det_ids,//跟踪对象与检测间的匹配id
    std::map<size_t, cv::Mat> *det_id_to_descriptor) {
    std::map<size_t, size_t> det_to_batch_ids;     // 检测对象在image特征提取中对应的id（方便从网络计算结果中提取出对应的特征向量）
    std::map<size_t, size_t> track_to_batch_ids;   // 跟踪对象在image特征提取中对应的id

    std::vector<cv::Mat> images;
    std::vector<cv::Mat> descriptors;
    for (size_t i = 0; i < track_and_det_ids.size(); i++) {
        size_t track_id = track_and_det_ids[i].first;  // track_id
        size_t det_id = track_and_det_ids[i].second;   // detection_id

        if (tracks_.at(track_id).descriptor_strong.empty()) { // 对应跟踪对象的深度特征为空
            images.push_back(tracks_.at(track_id).last_image);// 在最后填充跟踪目标的最后一帧
            descriptors.push_back(cv::Mat());                 // 最后填充一个mat
            track_to_batch_ids[track_id] = descriptors.size() - 1; // id从0开始
        }

        images.push_back(frame(detections[det_id].rect));     // 最后填充一个与跟踪对象对应的检测框图像
        descriptors.push_back(cv::Mat());                     // 最后填充一个mat
        det_to_batch_ids[det_id] = descriptors.size() - 1;    
    }

    // 深度特征计算 输出：每个descriptor：[1,256]
    // std::cout<<"deep learning!"<<std::endl;
    // image存放的是需要进行特征提取的图像 到这已经resize了
    // descriptor中存放的是与image等量的空mat
    descriptor_strong_->Compute(images, &descriptors);

    std::vector<cv::Mat> descriptors1;
    std::vector<cv::Mat> descriptors2;
    for (size_t i = 0; i < track_and_det_ids.size(); i++) {
        size_t track_id = track_and_det_ids[i].first;
        size_t det_id = track_and_det_ids[i].second;

        // 使用网络计算的特征向量填充缺的跟踪对象特征向量
        if (tracks_.at(track_id).descriptor_strong.empty()) {
            tracks_.at(track_id).descriptor_strong =
                descriptors[track_to_batch_ids[track_id]].clone();
        }
        // 填充检测目标特征向量
        (*det_id_to_descriptor)[det_id] = descriptors[det_to_batch_ids[det_id]];

        descriptors1.push_back(descriptors[det_to_batch_ids[det_id]]);    // 检测对象深度特征
        descriptors2.push_back(tracks_.at(track_id).descriptor_strong);   // 跟踪对象深度特征
    }

    // 深度特征间的余弦距离
    // 返回每对跟踪检测对象深度特征间的余弦距离
    std::vector<float> distances =
        distance_strong_->Compute(descriptors1, descriptors2);

    // int dis_c = 0;
    // for(auto dis:distances){
    //     std::cout<<dis_c<<":"<<dis<<std::endl;
    //     dis_c++;
    // }
    return distances;
}

std::vector<std::pair<size_t, size_t>>
PedestrianTracker::GetTrackToDetectionIds(
    const std::set<std::tuple<size_t, size_t, float>> &matches) {   // matches : <track_id,detection_id,affinity>
    std::vector<std::pair<size_t, size_t>> track_and_det_ids;

    // std::cout<<"matches:"<<matches.size()<<std::endl;   // 2-3

    for (const auto &match : matches) {
        size_t track_id = std::get<0>(match);
        size_t det_id = std::get<1>(match);
        float conf = std::get<2>(match);
        // std::cout<<"track_id:"<<track_id<<"  det_id:"<<det_id<<"  conf:"<<conf<<std::endl;
        // std::cout<<params_.aff_thr_fast <<"  "<<params_.strong_affinity_thr<<std::endl;

        // 这里的判断条件就是说：如果匈牙利匹配出的置信度不高，再通过网络特征判断，如果传统算法匹配出的概率太低，就认为不是同一个跟踪目标
        if (conf < params_.aff_thr_fast && conf > params_.strong_affinity_thr) {   // aff_thr_fast: 0.8 strong_affinity_thr: 0.285f
                                                                                   // aff_thr_strong: 0.75
            // 这里得出需要reid网络再次判断匹配的track与deteciton                                                          
            track_and_det_ids.emplace_back(track_id, det_id);
            // std::cout<<"track and detection:"<<matches.size()<<std::endl;       // 2-3
        }
    }
    return track_and_det_ids;
}

std::map<size_t, std::pair<bool, cv::Mat>>
PedestrianTracker::StrongMatching(
    const cv::Mat &frame,
    const TrackedObjects& detections,
    const std::vector<std::pair<size_t, size_t>> &track_and_det_ids) {
    std::map<size_t, std::pair<bool, cv::Mat>> is_matching;

    if (track_and_det_ids.size() == 0) {
        return is_matching;
    }

    std::map<size_t, cv::Mat> det_ids_to_descriptors;
    // 在这里进行网络计算，并得到跟踪对象与检测对象的深度特征向量
    // 在这里计算每个跟踪检测匹配对间深度特征的余弦距离，并返回
    // 函数结束后det_ids_to_descriptors存放的是检测框的深度特征向量
    std::vector<float> distances =
        ComputeDistances(frame, detections,
                         track_and_det_ids, &det_ids_to_descriptors);

    // std::cout<<track_and_det_ids.size()<<std::endl;

    for (size_t i = 0; i < track_and_det_ids.size(); i++) {
        // 这里算的是余弦距离
        auto reid_affinity = 1.0 - distances[i];

        size_t track_id = track_and_det_ids[i].first;
        size_t det_id = track_and_det_ids[i].second;

        const auto& track = tracks_.at(track_id);
        const auto& detection = detections[det_id];

        auto last_det = track.objects.back();       // 跟踪对象最新的目标
        last_det.rect = track.predicted_rect;       // 最新的跟踪框赋值为预测框

        // 这里只有外框信息，Affinity函数仅计算ShapeAffinity，MotionAffinity，TimeAffinity
        float affinity = static_cast<float>(reid_affinity) * Affinity(last_det, detection);

        // collect_matches:ture
        if (collect_matches_ && last_det.object_id >= 0 &&
            detection.object_id >= 0) {
            reid_classifier_matches_.emplace_back(track.objects.back(), last_det.rect,
                                                  detection,
                                                  reid_affinity > params_.reid_thr);  
                                                  // reid_affinity:深度特征余弦特征

            reid_based_classifier_matches_.emplace_back(
                track.objects.back(), last_det.rect, detection,
                affinity > params_.aff_thr_strong);// affinity:DeepAffinity,TimeAffinity,ShapeAffinity,MotionAffinity
        }

        bool is_detection_matching =
            reid_affinity > params_.reid_thr && affinity > params_.aff_thr_strong;   // reid_thr:0.61/0.85  aff_thr_strong:0.75/0.5

        is_matching[track_id] = std::pair<bool, cv::Mat>(
            is_detection_matching, det_ids_to_descriptors[det_id]);   
    }
    return is_matching;
}

// 加入的跟踪目标，检测中的结果都作为新的跟踪对象
void PedestrianTracker::AddNewTracks(
    const cv::Mat &frame, const TrackedObjects &detections,
    const std::vector<cv::Mat> &descriptors_fast) {
    PT_CHECK(detections.size() == descriptors_fast.size());
    for (size_t i = 0; i < detections.size(); i++) {
        AddNewTrack(frame, detections[i], descriptors_fast[i]);
    }
}

// 加入新的跟踪目标
// 输入id，选择检测框中指定id的目标作为新的跟踪目标
void PedestrianTracker::AddNewTracks(
    const cv::Mat &frame, const TrackedObjects &detections,
    const std::vector<cv::Mat> &descriptors_fast, const std::set<size_t> &ids) {
    PT_CHECK(detections.size() == descriptors_fast.size());
    for (size_t i : ids) {
        PT_CHECK(i < detections.size());
        AddNewTrack(frame, detections[i], descriptors_fast[i]);
    }
}

// 加入新的跟踪目标执行函数
// 可以考虑在这里修改流程
void PedestrianTracker::AddNewTrack(const cv::Mat &frame,
                                    const TrackedObject &detection,
                                    const cv::Mat &descriptor_fast,
                                    const cv::Mat &descriptor_strong) {  // default:descriptor_strong = cv::Mat()
    auto detection_with_id = detection;
    detection_with_id.object_id = tracks_counter_;
    tracks_.emplace(std::pair<size_t, Track>(
            tracks_counter_,
            Track({detection_with_id}, frame(detection.rect).clone(),
                  descriptor_fast.clone(), descriptor_strong.clone())));

    for (size_t id : active_track_ids_) {
        tracks_dists_.emplace(std::pair<size_t, size_t>(id, tracks_counter_),
                              std::numeric_limits<float>::max());
    }

    active_track_ids_.insert(tracks_counter_);
    tracks_counter_++;
}

// 更新对应id跟踪目标的相关参数与数据
void PedestrianTracker::AppendToTrack(const cv::Mat &frame,
                                      size_t track_id,
                                      const TrackedObject &detection,
                                      const cv::Mat &descriptor_fast,
                                      const cv::Mat &descriptor_strong) {
    PT_CHECK(!IsTrackForgotten(track_id));

    auto detection_with_id = detection;
    detection_with_id.object_id = track_id;

    auto &cur_track = tracks_.at(track_id);
    cur_track.objects.emplace_back(detection_with_id);
    cur_track.predicted_rect = detection.rect;
    cur_track.lost = 0;
    cur_track.last_image = frame(detection.rect).clone();
    cur_track.descriptor_fast = descriptor_fast.clone();
    cur_track.length++;

    // 深度特征向量更新
    if (cur_track.descriptor_strong.empty()) {
        cur_track.descriptor_strong = descriptor_strong.clone();
    } else if (!descriptor_strong.empty()) {
        cur_track.descriptor_strong =
            0.5 * (descriptor_strong + cur_track.descriptor_strong);
    }

    if (params_.max_num_objects_in_track > 0) {
        while (cur_track.size() >
               static_cast<size_t>(params_.max_num_objects_in_track)) {
            cur_track.objects.erase(cur_track.objects.begin());        // 目标检测框超过最大保存数，则清除开头的一个
        }
    }
}

float PedestrianTracker::AffinityFast(const cv::Mat &descriptor1,
                                      const TrackedObject &obj1,
                                      const cv::Mat &descriptor2,
                                      const TrackedObject &obj2) {
    const float eps = 1e-6f;
    // shape_affinity=0.5 (0,100)
    // bounding box 相似度
    float shp_aff = ShapeAffinity(params_.shape_affinity_w, obj1.rect, obj2.rect);
    if (shp_aff < eps) return 0.0f;

    // motion_affinity_w=0.2 (0,100)
    float mot_aff =
        MotionAffinity(params_.motion_affinity_w, obj1.rect, obj2.rect);
    if (mot_aff < eps) return 0.0f;

    // time_affinity_w=0 (0,100)
    float time_aff =
        TimeAffinity(params_.time_affinity_w, static_cast<float>(obj1.frame_idx), static_cast<float>(obj2.frame_idx));
    if (time_aff < eps) return 0.0f;

    float distance_fast_c = distance_fast_->Compute(descriptor1, descriptor2);// scale = -1, offset = 1
    // float app_aff = 1.0f - distance_fast_c*distance_fast_c;   
    float app_aff = 1.0f - distance_fast_c;   
    return shp_aff * mot_aff * app_aff * time_aff;
}

float PedestrianTracker::Affinity(const TrackedObject &obj1,
                                  const TrackedObject &obj2) {
    float shp_aff = ShapeAffinity(params_.shape_affinity_w, obj1.rect, obj2.rect);
    float mot_aff =
        MotionAffinity(params_.motion_affinity_w, obj1.rect, obj2.rect);
    float time_aff =
        TimeAffinity(params_.time_affinity_w, static_cast<float>(obj1.frame_idx), static_cast<float>(obj2.frame_idx));
    return shp_aff * mot_aff * time_aff;
}

bool PedestrianTracker::IsTrackValid(size_t id) const {
    const auto& track = tracks_.at(id);
    const auto &objects = track.objects;
    if (objects.empty()) {
        return false;
    }
    int64_t duration_ms = objects.back().timestamp - track.first_object.timestamp;
    if (duration_ms < static_cast<int64_t>(params_.min_track_duration))   // 与第一帧时间差小于1s则此id无效
        return false;
    return true;
}

bool PedestrianTracker::IsTrackForgotten(size_t id) const {
    return IsTrackForgotten(tracks_.at(id));
}

bool PedestrianTracker::IsTrackForgotten(const Track &track) const {
    return (track.lost > params_.forget_delay);
}

size_t PedestrianTracker::Count() const {
    size_t count = valid_tracks_counter_;
    for (const auto &pair : tracks_) {
        count += (IsTrackValid(pair.first) ? 1 : 0);
    }
    return count;
}

std::unordered_map<size_t, std::vector<cv::Point>>
PedestrianTracker::GetActiveTracks() const {
    std::unordered_map<size_t, std::vector<cv::Point>> active_tracks;
    for (size_t idx : active_track_ids()) {      // std::set<size_t> active_track_ids_
        auto track = tracks().at(idx);
        // 有效id:边界框不为空，且与第一帧时间间隔不小于200ms
        // 遗忘id:丢失帧数大于最大遗忘数
        if (IsTrackValid(idx) && !IsTrackForgotten(idx)) {
            active_tracks.emplace(idx, Centers(track.objects));
        }
    }
    return active_tracks;
}

TrackedObjects PedestrianTracker::TrackedDetections() const {
    TrackedObjects detections;
    for (size_t idx : active_track_ids()) {
        auto track = tracks().at(idx);
        // 与第一帧间隔不超过1s的跟踪目标id无效
        if (IsTrackValid(idx) && !track.lost) {
            detections.emplace_back(track.objects.back());
        }
    }
    return detections;
}

// 在这里做修改
cv::Mat PedestrianTracker::DrawActiveTracks(const cv::Mat &frame) {
    //cv::Mat out_frame = frame.clone();
    cv::Mat my_frame=cv::imread("/home/dahu/Desktop/pedestrain/untitled5/resource/court1.png");  // 坐标透视转换用背景图片长宽
    if (colors_.empty()) {
        int num_colors = 100;
        colors_ = GenRandomColors(num_colors);  // 获取一个随机颜色
    }

    auto active_tracks = GetActiveTracks();
    for (auto active_track : active_tracks) {
        size_t idx = active_track.first;

        if(idx > 1)
           continue;
        auto centers = active_track.second;   // vector cv::point
        std::vector<cv::Point> centers_im;
        for(auto im_point: centers){
            //   im_point.x += image_x;
            //   im_point.y += image_y;
              im_point.x=im_point.x*image_p_col/1920-30;
              im_point.y=im_point.y*image_p_row/1080+80;
              //std::cout<<"point x:"<<im_point.x<<"  pointy:"<<im_point.y<<"\nframe.cols:"<<frame.cols<<"  frame.rows:"<<frame.rows<<std::endl;
              centers_im.emplace_back(im_point);
         }

        DrawPolyline(centers_im, colors_[idx % colors_.size()], &my_frame);
        // DrawPolyline(centers, colors_[idx % colors_.size()], &out_frame);
        std::stringstream ss;
        //ss <<"A"<< idx ;
        if(idx==0)
            ss << "A";
        else
            ss << "B";
        
        cv::putText(my_frame, ss.str(), centers_im.back(), cv::FONT_HERSHEY_COMPLEX, 2.0,
                    colors_[idx % colors_.size()], 3);
        // ++idx;
        // if(idx==2) idx=0;
        // cv::putText(out_frame, ss.str(), centers.back(), cv::FONT_HERSHEY_SCRIPT_COMPLEX, 2.0,
        //             colors_[idx % colors_.size()], 3);

        // 跟踪对象处于激活状态，但处于丢失状态
        // auto track = tracks().at(idx);
        //  if (track.lost) {
        //      cv::line(frame, active_track.second.back(),
        //               Center(track.predicted_rect), cv::Scalar(0, 255, 0), 10);
        // }

    }

    return my_frame;
}


const cv::Size kMinFrameSize = cv::Size(320, 240);
const cv::Size kMaxFrameSize = cv::Size(1920, 1080);

// 打印混合矩阵
void PedestrianTracker::PrintConfusionMatrices() const {
    std::cout << "Base classifier quality: " << std::endl;
    {
        auto cm = ConfusionMatrix(base_classifier_matches());
        std::cout << cm << std::endl;
        std::cout << "or" << std::endl;
        cm.row(0) = cm.row(0) / std::max(1.0, cv::sum(cm.row(0))[0]);
        cm.row(1) = cm.row(1) / std::max(1.0, cv::sum(cm.row(1))[0]);
        std::cout << cm << std::endl << std::endl;
    }

    std::cout << "Reid-based classifier quality: " << std::endl;
    {
        auto cm = ConfusionMatrix(reid_based_classifier_matches());
        std::cout << cm << std::endl;
        std::cout << "or" << std::endl;
        cm.row(0) = cm.row(0) / std::max(1.0, cv::sum(cm.row(0))[0]);
        cm.row(1) = cm.row(1) / std::max(1.0, cv::sum(cm.row(1))[0]);
        std::cout << cm << std::endl << std::endl;
    }

    std::cout << "Reid only classifier quality: " << std::endl;
    {
        auto cm = ConfusionMatrix(reid_classifier_matches());
        std::cout << cm << std::endl;
        std::cout << "or" << std::endl;
        cm.row(0) = cm.row(0) / std::max(1.0, cv::sum(cm.row(0))[0]);
        cm.row(1) = cm.row(1) / std::max(1.0, cv::sum(cm.row(1))[0]);
        std::cout << cm << std::endl << std::endl;
    }
}

void PedestrianTracker::PrintReidPerformanceCounts() const {
    if (descriptor_strong_) {
        descriptor_strong_->PrintPerformanceCounts();
    }
}
