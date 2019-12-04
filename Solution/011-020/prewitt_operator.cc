#include"Solution/011-020/prewitt_operator.h"
#include"Solution/base.h"

#include<opencv2/highgui.hpp>
#include<opencv2/imgproc.hpp>

namespace digital {

static const int kWin = 3;

//brief:
void PrewittOperator::operator()() {
	cv::Mat data = cv::imread(getPath());
	if (data.empty()) {
		dealException(kFileError);
		return;
	}
	cv::cvtColor(data, data, cv::COLOR_RGB2GRAY);
	cv::GaussianBlur(data, data, cv::Size(3, 3), 0);

	cv::Mat img_x;
	cv::Mat img_y;

#ifdef USE_OPENCVLIB

	cv::Mat kernel_x = (cv::Mat_<float>(kWin, kWin) <<
		-1, 0, 1,
		-1, 0, 1,
		-1, 0, 1);
	cv::filter2D(data, img_x, CV_16S, kernel_x);
	
	cv::Mat kernel_y = (cv::Mat_<float>(kWin, kWin) <<
		-1, -1, -1,
		0, 0, 0,
		1, 1, 1);
	cv::filter2D(data, img_y, CV_16S, kernel_y);

#else

	//����ֱ�Ӳ��ö�ά����Ĳ���
	//int filter_x[kWin*kWin] = {
	//	-1, 0, 1,
	//	-1, 0, 1,
	//	-1, 0, 1
	//};
	//detail::filter2D<CV_8U, CV_16S>(data, img_x, filter_x, kWin, kWin, false, false);

	//int filter_y[kWin*kWin] = {
	//	-1, -1, -1,
	//	0, 0, 0,
	//	1, 1, 1,
	//};
	//detail::filter2D<CV_8U, CV_16S>(data, img_y, filter_y, kWin, kWin, false, false);

	//����ʵ�ֲ��ý�Prewitt���ӷ������ķ�ʽ
	prewitt_(&data, &img_x, CV_16S, 1, 0);
	prewitt_(&data, &img_y, CV_16S, 0, 1);

#endif

	//���Ե�����calibration
	cv::convertScaleAbs(img_x, img_x, 1, 128);
	cv::convertScaleAbs(img_y, img_y, 1, 128);
	cv::Mat gradient;
	cv::addWeighted(img_x, 0.5, img_y, 0.5, 0, gradient);

	//��ʾ���
	void* figs[4];
	figs[0] = &data;
	figs[1] = &img_x;
	figs[2] = &img_y;
	figs[3] = &gradient;

	if (needShowOriginal())
		show(figs, 4);
	else
		show(figs + 1, 3);
}

//brief:����Prewitt�Ŀɷ����ԣ��ֱ������������ϼ���
void PrewittOperator::prewitt_(void* src, void* dst, int ddepth, int dx, int dy, int bordertype) {
	assert((dx == 0 && dy != 0) || (dy == 0 && dx != 0));

	cv::Mat& data = *static_cast<cv::Mat*>(src);
	cv::Mat& img = *static_cast<cv::Mat*>(dst);

	//������ƽ�����Ӻ��������������
	cv::Mat smoth_kernel = (cv::Mat_<float>(3, 1) << 1, 1, 1);
	cv::Mat diff_kernel = (cv::Mat_<float>(3, 1) << -1, 0, 1);


	if (dx) {
		detail::sepConvolution2D(data, img, ddepth, diff_kernel.t(), smoth_kernel, cv::Point(-1, -1), bordertype);
		//need flip
		//cv::sepFilter2D(data, img, ddepth, diff_kernel.t(), smoth_kernel, cv::Point(-1, -1), 0.0, bordertype);
	}
	else {
		detail::sepConvolution2D(data, img, ddepth, smoth_kernel.t(), diff_kernel, cv::Point(-1, -1), bordertype);
		//need flip
		//cv::sepFilter2D(data, img, ddepth, diff_kernel, smoth_kernel.t(), cv::Point(-1, -1), 0.0, bordertype);
	}
}

}//��namespace digital