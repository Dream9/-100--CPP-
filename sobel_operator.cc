#include"Solution/011-020/sobel_operator.h"
#include"Solution/base.h"

#include<opencv2/highgui.hpp>
#include<opencv2/imgproc.hpp>

#ifndef NDEBUG
//for test
#include<vector>
#endif

namespace digital {

//brief:Sobel ������������һ���Ǽ��ˮƽ��Ե�� ����һ���Ǽ�ⴹֱ��Ե�� ��
//     ��Prewitt������ȣ�Sobel���Ӷ������ص�λ�õ�Ӱ�����˼�Ȩ�����Խ��ͱ�Եģ���̶�
//becare:ʹ�ø���Ĵ��ڣ���ζ�Ų�ȡ���ߵ�ģ��
void SobelOperator::operator()() {
    //תΪ�ҶȲ�����
	cv::Mat data = cv::imread(getPath(), cv::IMREAD_GRAYSCALE);
	if(data.empty()) {
		dealException(kFileError);
		return;
	}
	cv::GaussianBlur(data, data, cv::Size(3, 3), 0.0, 0.0);
	cv::Mat img_x;
	cv::Mat img_y;

#ifdef USE_OPENCVLIB

	//�����ڲ����ṩһ��Scharr��ʵ�֣����ṩ���߾��ȵ�һ��΢�ֽ��ƣ��Ƕ�Sobel���Ż�
	cv::Sobel(data, img_x, CV_16S, 1, 0, win_);
	//or use cv::filter2D
	
	cv::Sobel(data, img_y, CV_16S, 0, 1, win_);

#else
	//������ù̶��Ŀռ��˲��˵�ʵ�ַ�ʽ
	//int filter_x[] = {
	//	-1, 0, 1,
	//	-2, 0, 2,
	//	-1, 0, 1 };
	//detail::filter2D<CV_8U, CV_16S, int>(data, img_x, filter_x, 3, 3, false, false);

	//int filter_y[] = {
	//	-1, -2, -1,
	//	0, 1, 0,
	//	1, 2, 1 };
	//detail::filter2D<CV_8U, CV_16S, int>(data, img_y, filter_y, 3, 3, false, true);

	detail::Sobel(data, img_x, CV_16S, 1, 0, win_);
	detail::Sobel(data, img_y, CV_16S, 0, 1, win_);
#endif

	//���±궨
	cv::convertScaleAbs(img_x, img_x, 1, 128);
	cv::convertScaleAbs(img_y, img_y, 1, 128);

	//�����ݶ�
	cv::Mat gradient;
	cv::addWeighted(img_x, 0.5, img_y, 0.5, 0, gradient);

	assert_imshow_type(gradient.depth());

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


}//!namespace digital