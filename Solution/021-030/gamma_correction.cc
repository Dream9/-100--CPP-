#include"Solution/021-030/gamma_correction.h"

#ifndef USE_OPENCVLIB
#include"Solution/base.h"
#include<functional>
#endif

#include<opencv2/highgui.hpp>

#include"Solution/geometric_transform.h"

namespace digital {

//brief:٤��У����Ϊ��ӭ����ʾ���ȵ����豸������Դ�٤�����������
//becare:����0-1��Χ�ڵ����ֽ���������
void GammaCorrection::operator()() {
	cv::Mat data = cv::imread(getPath());
	if (data.empty()) {
		dealException(kFileError);
		return;
	}
	cv::Mat img;

#ifdef USE_OPENCVLIB

	//cv::multiply(data, 1./ c_, img, 1.0, CV_32F);
	//becare�����ת����0-1��Χ
	cv::multiply(data, 1./ (c_ * UINT8_MAX), img, 1.0, CV_32F);
	cv::pow(img, 1. / gamma_, img);
	img.convertTo(img, data.type(), UINT8_MAX);

#else

	//img.create(data.size(), CV_MAKETYPE(CV_32F, data.channels()));
	img.create(data.size(), data.type());

	auto iter = img.data;
	double alpha = 1. / (c_ * UINT8_MAX);
	double gamma_inv = 1. / gamma_;

	//���� lambda�������grayscaleTransform�ӿ�
	auto gamma_func = [&iter,alpha,gamma_inv](uint8_t* cursor) {
		*iter = cv::saturate_cast<uint8_t>(std::pow(*cursor * alpha, gamma_inv) * UINT8_MAX);
		++iter;
	};
	//һ�����������ת��
	detail::grayscaleTransform(data, gamma_func);
	
#endif

	//for test
	//__MatrixTest(&data, &img);


	if (needShowOriginal())
		show(&data, &img);
	else
		show(&img);
}

}