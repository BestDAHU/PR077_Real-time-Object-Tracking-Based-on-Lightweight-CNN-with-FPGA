#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/types_c.h>

using namespace cv;

//坐标点转换
Point2f cvt_pos(Point2f Points, cv::Mat m_persctiveMat) {
	float u = Points.x;
	float v = Points.y;
	float x = (
		m_persctiveMat.at<double>(0, 0) * u
		+ m_persctiveMat.at<double>(0, 1) * v
		+ m_persctiveMat.at<double>(0, 2) )/ (m_persctiveMat.at<double>(2, 0) * u
		+ m_persctiveMat.at<double>(2, 0) * v + m_persctiveMat.at<double>(2, 2));
	float y = (
		m_persctiveMat.at<double>(1, 0) * u
		+ m_persctiveMat.at<double>(1, 1) * v
		+ m_persctiveMat.at<double>(1, 2)) 
		/ (m_persctiveMat.at<double>(2, 0) * u
		+ m_persctiveMat.at<double>(2, 1) * v 
		+ m_persctiveMat.at<double>(2, 2));
	Point2f Points_ok = Point2f(x, y);
	return Points_ok;
}


Point2f point_trans(Point2f point_before,Mat m_persctiveMat)
{
	//首先读入图像
	
	//求b变换后的点
	Point2f Points_after = cvt_pos(point_before, m_persctiveMat);

	
	cv::waitKey(0);

	return Points_after;

}