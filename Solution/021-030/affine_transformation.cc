#include"Solution/021-030/affine_transformation.h"
#include"Solution/geometric_transform.h"

#include<opencv2/highgui.hpp>

namespace {

const double kOffset_x = 30;
const double kOffset_y = -30;
const double kScale_x = 1.3;
const double kScale_y = 0.8;
const double kLean_x = 60;//��x��нǣ�90�ȱ�ʾû����бʱ��ֵ
const double kLean_y = 80;//��y��н�

//brief:����б�Ƕ�ת��������任�������
inline double __getLean(double angle) {
	//FIXME:���ø�����Ĳ���ɸѡ�б�
	assert(angle <= 90);
	assert(angle >= -90);

	return (angle == 90 || angle == -90) ? 0 : std::tan(((90 - angle) / 180 * digital::k_PI));
}

}

namespace digital {

//brief:���ʲ��䣬����ʵ��λ��geometric_transformģ��
void AffineTransformation::operator()() {
	cv::Mat data = cv::imread(getPath());
	if (data.empty()) {
		dealException(kFileError);
		return;
	}

	cv::Mat img;
	cv::Size size = data.size();

	if (ops_ & TRANSLATION) {
		//brief:ƽ��
		cv::Mat M_translation = (cv::Mat_<double>(3, 3) <<
			1, 0, kOffset_x,
			0, 1, kOffset_y,
			0, 0, 1);
#ifdef USE_OPENCVLIB
		cv::warpAffine(data, img, M_translation, size, cv::INTER_CUBIC);
#else
		detail::warpAffine(data, img, M_translation, size, cv::INTER_CUBIC);
#endif
		if (needShowOriginal())
			show(&data, &img, "Translate");
		else
			show(&img, "Translate");
	}

	if (ops_ & SCALE) {
		//brief:����
		size.width = static_cast<int>(size.width*kScale_x);
		size.height = static_cast<int>(size.height*kScale_y);
#ifdef USE_OPENCVLIB
		coutInfo("getRotationMatrix2Dֻ�ṩ��x-y��ͬ�������߶�");
		cv::Mat M_scale = cv::getRotationMatrix2D(cv::Point(), 0, kScale_x);
		cv::warpAffine(data, img, M_scale, size, cv::INTER_CUBIC);
#else
		cv::Mat M_scale = detail::getRotationMatrix2D(cv::Point(), 0, kScale_x, kScale_y);
		detail::warpAffine(data, img, M_scale, size, cv::INTER_CUBIC);
#endif
		if (needShowOriginal())
			show(&data, &img, "Scale");
		else
			show(&img, "Scale");
	}

	if (ops_ & ROTATION) {
		//brief:��ת

#ifdef USE_OPENCVLIB
		cv::Mat M_rotation = cv::getRotationMatrix2D(cv::Point(), 30, 1);
		//opencv û��ֱ���ṩУ�������ĵĹ��ܣ���Ҫ�û����е����任����
		double x = 0., y = 0.;
		size = detail::getSizeAfterWarpAffine(M_rotation, data.size(), &x, &y);
		cv::Mat_<double> tmp(3, 3);
		tmp << 1, 0, -x,
			0, 1, -y,
			0, 0, 1;
		M_rotation = tmp * M_rotation;

		cv::warpAffine(data, img, M_rotation, size, cv::INTER_CUBIC);

		cv::Mat M_rotation = detail::getRotationMatrix2D(cv::Point(), 30, 1);
		detail::warpAffine(data, img, M_rotation, size, cv::INTER_CUBIC, cv::Scalar(), true);
#else
		cv::Mat M_rotation = detail::getRotationMatrix2D(cv::Point(), 30, 1);
		detail::warpAffine(data, img, M_rotation, size, cv::INTER_CUBIC, cv::Scalar(), true);
#endif
		if (needShowOriginal())
			show(&data, &img);
		else
			show(&img);
	}

	if (ops_ & LEAN) {
		if (kLean_x + kLean_y == 90 || kLean_x + kLean_y == -90) {
			//��б��Ϊ90/-90�ľ��������ԭͼ�����ñ����һ���ߣ��������ڲ��õı߽����䷽ʽ���ƣ����ز���ʱ
			//���ܻᱻ�ж�Ϊ������Ч�����Ӷ�����ͼ��ȫ��Ϊ����ֵ
			coutInfo("Warning! it should be a line, but it can be ignored by our border_type!");
		}
		double lean_x = __getLean(kLean_x);
		double lean_y = __getLean(kLean_y);

		cv::Mat M_lean = (cv::Mat_<double>(3, 3) <<
			1, lean_x, 0,
			lean_y, 1, 0,
			0, 0, 1);
		detail::warpAffine(data, img, M_lean, size, cv::INTER_CUBIC, cv::Scalar(), true);
		if (needShowOriginal())
			show(&data, &img);
		else
			show(&img);

	}
}


}