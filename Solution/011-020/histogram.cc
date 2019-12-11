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

	//����Ҫ�²�һ�£�����ӿ���Ƶļ���ķ�������Ҫ����������õ�һ�Ľӿ����̫������񣬾͵�����Ƴ�������
	//���Ҫ�õĻ������Ǹ���ʵ������ڷ�װһ�¸���
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

	//brief:�Թ���������ʽ��ʾ
	int value = 250;
	const string& name = "hist";
	cv::namedWindow(name);
	cv::createTrackbar("bins", name, &value, 256, trackbarCallback_, &data);

	cv::waitKey(0);
	cv::destroyAllWindows();
}

//brief:���CV_8U����Mat���ݼ���pdf
//becare:opencv���������ʵ�����ػ���������ȣ�They all should have the same depth, CV_8U, CV_16U or CV_32F , and the same
//       size. Each of them can have an arbitrary number of channels.
//
//       ����Դ��
//       if (depth == CV_8U)
//       calcHist_8u(ptrs, deltas, imsize, ihist, dims, ranges, _uniranges, uniform);
//       else if (depth == CV_16U)
//       calcHist_<ushort>(ptrs, deltas, imsize, ihist, dims, ranges, _uniranges, uniform);
//       else if (depth == CV_32F)
//       calcHist_<float>(ptrs, deltas, imsize, ihist, dims, ranges, _uniranges, uniform);
//       else
//       CV_Error(CV_StsUnsupportedFormat, "");
//����c++ȱ�������ڵ����ͷ�����ƣ�opencv������if-else�ַ��ķ�ʽʵ�ַ���
void Histogram::getHistogram_(void* src, void* dst, int min, int max, int bins) {
	//
	cv::Mat& data = *static_cast<cv::Mat*>(src);
	cv::Mat& hist = *static_cast<cv::Mat*>(dst);

	detail::calcHistogram(data, hist, min, max, bins);


	//brief:��Ϊpdf���㻹�ᱻ������Ŀ�õ������ת�Ƶ�base.cc��ʵ��
	//assert(bins > 0);
	//assert(data.depth() == CV_8U);

	//hist = cv::Mat::zeros(bins, 1, CV_32F);

	//int ranges = max - min + 1;
	//double ratio = 1. * bins / ranges;

	//float* arr = hist.ptr<float>(0, 0);

	////�������ж�
	//cv::Size size = data.size();
	//int channels = data.channels();
	//int step = static_cast<int>(data.step);
	//if (data.isContinuous()) {
	//	size.width *= size.height;
	//	size.height = 1;
	//}
	//size.width *= channels;

	////���ݻҶ�ֵ�ַ�����
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