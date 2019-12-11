#include"Solution/011-020/histogram.h"
#include"Solution/grayscale_transfrom.h"

#include<opencv2/highgui.hpp>
#include<opencv2/imgproc.hpp>

#include<functional>
#include<iostream>

namespace digital {

//brief:
void Histogram::operator()() {
	cv::Mat data = cv::imread(getPath());
	if (data.empty()) {
		dealException(kFileError);
		return;
	}

	cv::Mat hist;

#ifdef USE_OPENCVLIB

	int histSize[] = { bins_ };
	float single_ranges[] = { 0,255 };
	const float *ranges[] = { single_ranges };
	int channels[] = { 0 };

	//必须要吐槽一下，这个接口设计的极其的繁琐，主要是设计者想用单一的接口完成太多的任务，就导致设计成这样了
	//如果要用的话，还是根据实际情况在封装一下更好
	calcHist(&data, 1, channels, cv::Mat(), hist, 1, histSize, ranges);

#else

	getHistogram_(&data, &hist, 0, 255, bins_);

#endif

	//for test
	//__MatrixTest(&hist);

	cv::Mat img;
	detail::fillHistogram(hist, img, cv::Size(255, 255),CV_8U,cv::Scalar(0));

	if (needShowOriginal())
		show(&data, &img);
	else
		show(&img);

	//brief:以滚动条的形式显示
	int value = 250;
	const string& name = "hist";
	cv::namedWindow(name);
	cv::createTrackbar("bins", name, &value, 256, trackbarCallback_, &data);

	cv::waitKey(0);
	cv::destroyAllWindows();
}

//brief:针对CV_8U类型Mat数据计算pdf
//becare:opencv关于这里的实现是特化了三种深度：They all should have the same depth, CV_8U, CV_16U or CV_32F , and the same
//       size. Each of them can have an arbitrary number of channels.
//
//       部分源码
//       if (depth == CV_8U)
//       calcHist_8u(ptrs, deltas, imsize, ihist, dims, ranges, _uniranges, uniform);
//       else if (depth == CV_16U)
//       calcHist_<ushort>(ptrs, deltas, imsize, ihist, dims, ranges, _uniranges, uniform);
//       else if (depth == CV_32F)
//       calcHist_<float>(ptrs, deltas, imsize, ihist, dims, ranges, _uniranges, uniform);
//       else
//       CV_Error(CV_StsUnsupportedFormat, "");
//由于c++缺少运行期的类型反射机制，opencv采用了if-else分发的方式实现反射
void Histogram::getHistogram_(void* src, void* dst, int min, int max, int bins) {
	//
	cv::Mat& data = *static_cast<cv::Mat*>(src);
	cv::Mat& hist = *static_cast<cv::Mat*>(dst);

	detail::calcHistogram(data, hist, min, max, bins);


	//brief:因为pdf计算还会被其他项目用到，因此转移到base.cc中实现
	//assert(bins > 0);
	//assert(data.depth() == CV_8U);

	//hist = cv::Mat::zeros(bins, 1, CV_32F);

	//int ranges = max - min + 1;
	//double ratio = 1. * bins / ranges;

	//float* arr = hist.ptr<float>(0, 0);

	////连续性判断
	//cv::Size size = data.size();
	//int channels = data.channels();
	//int step = static_cast<int>(data.step);
	//if (data.isContinuous()) {
	//	size.width *= size.height;
	//	size.height = 1;
	//}
	//size.width *= channels;

	////根据灰度值分发即可
	//auto cur = data.data;
	//for (int i = 0; i < size.height; ++i) {
	//	auto tmp = cur;
	//	for (int j = 0; j < size.width; ++j) {
	//		int index = static_cast<int>(*tmp * ratio);
	//		++arr[index];
	//		++tmp;
	//	}
	//	cur += step;
	//}
}

//brief:
void trackbarCallback_(int pos, void* userdata) {
	pos = pos == 0 ? 1 : pos;
	cv::Mat& data = *static_cast<cv::Mat*>(userdata);
	
	cv::Mat hist;
	Histogram::getHistogram_(&data, &hist, 0, 255, pos);

	__MatrixTest(&hist);
	
	cv::Mat img;
	detail::fillHistogram(hist, img, cv::Size(255, 255),CV_8U,cv::Scalar(0));

	cv::imshow("hist", img);
}

}//!namespace digital