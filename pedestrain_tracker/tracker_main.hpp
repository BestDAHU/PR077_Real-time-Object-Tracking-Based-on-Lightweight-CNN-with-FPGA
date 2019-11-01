#include <image_reader.hpp>
#include <opencv2/core.hpp>
#include "detector.hpp"
#include "tracker.hpp"
#include "point_trans.hpp"
int tracker_init(int argc, char **argv);
int tracker_work(ImageWithFrameIndex pair,cv::Rect rect,double video_fps);
int tracker_work_fw(ImageWithFrameIndex pair,cv::Rect rect,double video_fps,std::ofstream &fp);
int tracker_work_fr(ImageWithFrameIndex pair,cv::Rect rect,double video_fps,std::ifstream &fp);

extern std::unique_ptr<PedestrianTracker> tracker;
extern ObjectDetector pedestrian_detector;
extern TrackedObjects detections;

extern int image_x;
extern int image_y;

extern int image_p_row;
extern int image_p_col;