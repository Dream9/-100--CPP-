#include"Solution/geometric_transform.h"

namespace detail {

#define HOMOGENEOUS_MATRIX_SIZE 3,3

//brief:�õ�ָ������ת�����ķ������
cv::Mat getRotationMatrix2D(cv::Point rotation_center, double angle_count_clock, double x_scale, double y_scale){
	y_scale = y_scale == -1 ? x_scale : y_scale;

	double alpha = Deg2Rad(angle_count_clock);
	double sin_a = sin(alpha);
	double cos_a = cos(alpha);

	//TODO:���¼������������ȫ���Ժϲ���һ�𣬴Ӷ�������
	//double a11 = x_scale * cos_a;
	//double a12 = x_scale * sin_a;

	//cv::Mat_<double> tmp(HOMOGENEOUS_MATRIX_SIZE);
	//tmp <<
	//	a11, a12, (1 - a11)*rotation_center.x - a12 * rotation_center.y;
	//return tmp;

	//�Ƚ�������ƽ��
	cv::Mat_<double> out(HOMOGENEOUS_MATRIX_SIZE);
	out <<
		1, 0, -rotation_center.x,
		0, 1, -rotation_center.y,
		0, 0, 1;

	//Ȼ����з���
	cv::Mat_<double> tmp(HOMOGENEOUS_MATRIX_SIZE);
	tmp <<
		x_scale, 0, 0,
		0, y_scale, 0,
		0, 0, 1;
	out = tmp * out;

	//Ȼ�������ת
	tmp <<
		cos_a, sin_a, 0,
		-sin_a, cos_a, 0,
		0, 0, 1;
	out = tmp * out;

	//���ƽ�ƻص�ԭ��
	tmp <<
		1, 0, rotation_center.x,
		0, 1, rotation_center.y,
		0, 0, 1;
	return tmp * out;
}
#undef HOMOGENEOUS_MATRIX_SIZE 

}//!namespace detail
