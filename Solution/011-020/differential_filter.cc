#include"Solution/011-020/differential_filter.h"

#include<opencv2/highgui.hpp>

#ifdef USE_OPENCVLIB
#include<opencv2/imgproc.hpp>
#else
#include"Solution/base.h"
#endif

namespace digital {

//Ϊ�˴ճ�����
const int DifferentialFilter::kWin = 3;

#define _SHOW(x,y) if(needShowOriginal())\
	{\
		show(&##x,&##y);\
	}\
	else{\
		show(&##y);\
	}

//becare:��x�󵼵õ�����y����ı��أ���y�󵼵õ�����x����ı���
void DifferentialFilter::operator()() {
	cv::Mat data = cv::imread(getPath(), cv::IMREAD_GRAYSCALE);
	if (data.empty()) {
		dealException(kFileError);
		return;
	}

#ifdef USE_OPENCVLIB

	cv::Mat img;
	cv::Mat dx = (cv::Mat_<float>(3, 3) <<
		0, 0, 0,
		0, -1, 1,
		0, 0, 0);
	cv::filter2D(data, img, data.depth(), dx);
	//_SHOW(data, img);

	cv::Mat img_y;
	cv::Mat dy = (cv::Mat_<float>(3, 3) << 
		0, 0, 0,
		0, -1, 0,
		0, 1, 0);
	cv::filter2D(data, img_y, data.depth(), dy);
	//_SHOW(data, img);

	//�����ݶ�
	cv::Mat gradient;
	cv::addWeighted(img, 1.0, img_y, 1.0, 0.0, gradient);

#else

	//����x�����΢��
	cv::Mat img;

	int filter_x[kWin * kWin] = {
		0, 0, 0,
		0,-1, 1,
		0, 0, 0 };
	//���������ֵ��Ȼ�����±궨��ͼ��Ӧ��λ�ڻ�ɫ��
	detail::filter2D<CV_8U, CV_16S, int>(data, img, filter_x, kWin, kWin, false, false);
	//����calibration��Χ
	img.convertTo(img, CV_8UC1, 1.0, 128);
	//_SHOW(data, img);
	
	//����y�����΢��
	cv::Mat img_y;
	int filter_y[kWin * kWin] = {
		0, 0, 0,
		0,-1, 0,
		0, 1, 0 };
	//�����������ֵ��ͼ��Ӧ��λ�ں�ɫ��
	detail::filter2D<CV_8U, CV_8U, int>(data, img_y, filter_y, kWin, kWin, false, false);
	//_SHOW(data, img_y);


	//���Ƽ����ݶ�
	cv::Mat gradient = img + img_y;

#endif

	void*figs[4];
	figs[0] = &data;
	figs[1] = &img;
	figs[2] = &img_y;
	figs[3] = &gradient;
	if (needShowOriginal())
		show(figs, 4);
	else
		show(figs + 1, 3);
}

#undef _SHOW

}//!namespace digital
