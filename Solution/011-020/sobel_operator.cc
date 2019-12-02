#include"Solution/011-020/sobel_operator.h"

#include"Solution/base.h"

#include<opencv2/highgui.hpp>
#include<opencv2/imgproc.hpp>

namespace digital {

const int SobelOperator::win_ = 3;

//brief:Sobel ������������һ���Ǽ��ˮƽ��Ե�� ����һ���Ǽ�ⴹֱ��Ե�� ��
//     ��Prewitt������ȣ�Sobel���Ӷ������ص�λ�õ�Ӱ�����˼�Ȩ�����Խ��ͱ�Եģ���̶�
void SobelOperator::operator()() {
	//תΪ�ҶȲ�����
	cv::Mat data = cv::imread(getPath(), cv::IMREAD_GRAYSCALE);
	if(data.empty()) {
		dealException(kFileError);
		return;
	}
	cv::GaussianBlur(data, data, cv::Size(3, 3), 0.0, 0.0);

#ifdef USE_OPENCVLIB

	//�����ڲ����ṩһ��Scharr��ʵ�֣����ṩ���߾��ȵ�һ��΢�ֽ��ƣ��Ƕ�Sobel���Ż�
	cv::Mat img_x;
	cv::Sobel(data, img_x, CV_16S, 1, 0, 3);
	//or use cv::filter2D
	
	cv::Mat img_y;
	cv::Sobel(data, img_y, CV_16S, 0, 1, 3);

#else
	//�˴����ڸ�ֱֵ��ȡΪ0����û�����±궨
	int filter_x[win_*win_] = {
		-1, 0, 1,
		-2, 0, 2,
		-1, 0, 1 };
	cv::Mat img_x;
	detail::filter2D<CV_8U, CV_16S, int>(data, img_x, filter_x, win_, win_, false, false);
	
	int filter_y[win_*win_] = {
		-1, -2, -1,
		0, 0, 0,
		1, 2, 1 };
	cv::Mat img_y;
	detail::filter2D<CV_8U, CV_16S, int>(data, img_y, filter_y, win_, win_, false, false);

#endif

	//���±궨
	cv::convertScaleAbs(img_x, img_x, 1, 128);
	cv::convertScaleAbs(img_y, img_y, 1, 128);

	//�����ݶ�
	cv::Mat gradient = img_x + img_y;

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