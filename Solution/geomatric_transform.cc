#include"Solution/geometric_transform.h"
#include"Solution/type_extension.h"
#include"Solution/grayscale_transfrom.h"

namespace {

cv::Mat_<double> __getTransformPosition3D(double x, double y, cv::Mat& M);

inline double __getLinearEvaluate(double left, double medium, double val_left, double val_right) {
	return (medium - left) * (val_right - val_left) + val_left;
}

}

namespace {

#define GET_NEAREST(x) static_cast<int>(x+0.5)
#define GET_FLOOR(x) static_cast<int>(floor(x))

//brief:������ڵķ�ʽ���в�ֵ
//becare:Ŀǰֻ���CV_8U����������δ����չ
void warpAffine_nearest(const cv::Mat& src, cv::Mat& dst, const cv::Mat& M, cv::Size size, const cv::Scalar& value) {
	assert(src.depth() == CV_8U);
	dst.create(size, src.type());

	cv::Mat M_inv = M.inv();
	int channels = src.channels();
	cv::Rect range(0, 0, src.cols, src.rows);

	//���ı任
	auto set_value = [&src,&M_inv,channels,range,value](int x, int y, uint8_t* cursor) {

		//������任ǰ��λ��
		cv::Mat_<double> before = __getTransformPosition3D(x, y, M_inv);
		double* arr = before[0];
		cv::Point pos(GET_NEAREST(arr[0]), GET_NEAREST(arr[1]));

		//for test
		//digital::__printInfo("%d,%d(original)--%d,%d(warp)\r\n", pos.x, pos.y, x, y);

		if (range.contains(pos)) {
			//������ڵ�ֵ����
			//FIXME:ptr��λ�õ������У�����������x��y!!
			//auto iter = src.ptr(pos.x, pos.y);
			auto iter = src.ptr(pos.y, pos.x);
			for (int i = 0; i < channels; ++i) {
				*cursor++ = *iter++;
			}
		}
		else {
			//����û�ָ���ĳ���ֵ
			for (int i = 0; i < channels; ++i) {
				*cursor++ = cv::saturate_cast<uint8_t>(value[i]);
			}
		}
	};
	
	//������ӿڴ�����һ�𼴿�
	detail::geometricTriversal(dst, set_value);
}


//brief:��˫���Բ�ֵ�ķ�ʽ�ز���
//becare:Ŀǰֻ���CV_8U����������δ����չ
void warpAffine_linear(const cv::Mat& src, cv::Mat& dst, const cv::Mat& M, cv::Size size, const cv::Scalar& value) {
	assert(src.depth() == CV_8U);
	dst.create(size, src.type());
	
	cv::Mat M_inv = M.inv();
	int channels = src.channels();
	cv::Rect range(0, 0, src.cols, src.rows);
	size_t step = src.step;

	//���ı任
	auto set_value = [&src, &M_inv, channels, range, value, step](int x, int y, uint8_t* cursor) {
		cv::Mat_<double> before = __getTransformPosition3D(x, y, M_inv);
		double* arr = before[0];

		cv::Point pos(GET_FLOOR(arr[0]), GET_FLOOR(arr[1]));//��ȡ����λ�õ�����
		if (range.contains(pos)) {
			//ȷ���ĸ�λ�õ�handler
			auto tl = src.ptr(pos.y, pos.x);
			auto tr = tl + 3;
			auto bl = tl + step;
			auto br = bl + 3;

			for (int i = 0; i < channels; ++i) {
#ifdef USE_ORIGINAL_IMPLEMENT
				//����ԭʼ�߼���ʵ��
				double tmp1 = __getLinearEvaluate(pos.x, arr[0], *tl++, *tr++);
				double tmp2 = __getLinearEvaluate(pos.x, arr[0], *bl++, *br++);
				*cursor++ = cv::saturate_cast<uint8_t>(__getLinearEvaluate(pos.y, arr[1], tmp1, tmp2));

#else
				//������չ����
				double alpha_x = arr[0] - pos.x;
				double alpha_y = arr[1] - pos.y;
				*cursor++ = static_cast<uint8_t>(alpha_x * alpha_y * ( *tl + *br - *tr - *bl) + alpha_x * (*tr - *tl) +
					alpha_y * (*bl - *tl) + *tl);
				++tl;
				++tr;
				++bl;
				++br;
#endif
			}
		}
		else {
			//����û�ָ���ĳ���ֵ
			for (int i = 0; i < channels; ++i) {
				*cursor++ = cv::saturate_cast<uint8_t>(value[i]);
			}
		}
	};
	
	//������ӿڴ�����һ�𼴿�
	detail::geometricTriversal(dst, set_value);

}

//brief:��˫���β�ֵ�ķ�ʽ�ز���
void warpAffine_cubic(const cv::Mat& src, cv::Mat& dst, const cv::Mat& M, cv::Size size, const cv::Scalar& value) {
	//
}

}//!namespace




namespace detail {

#define HOMOGENEOUS_MATRIX_SIZE 3,3

//brief:�õ�ָ������ת�����ķ������
//becare:����������˿��Ժϲ���һ��,�ֿ�ֻ���߼��ϱȽ�����
cv::Mat getRotationMatrix2D(const cv::Point& rotation_center, double angle_count_clock, double x_scale, double y_scale){
	y_scale = y_scale == -1 ? x_scale : y_scale;

	double alpha = Deg2Rad(angle_count_clock);
	double sin_a = sin(alpha);
	double cos_a = cos(alpha);

	//TODO:���¼������������ȫ���Ժϲ���һ�𣬴Ӷ�������

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

//brief:���ݲ�ͬ�Ĳ�ֵ���ͽ�����Ӧ�ķַ�����
//
void warpAffine(const cv::Mat& src, cv::Mat& dst, const cv::Mat& M, cv::Size size,
	int interpolation_type, const cv::Scalar& value) {

	assert(src.channels() <= 4);
	assert(src.depth() == CV_8U);

	switch (interpolation_type)
	{
	case INTER_NEAREST:
		warpAffine_nearest(src, dst, M, size, value);
		break;
	case INTER_LINEAR:
		warpAffine_linear(src, dst, M, size, value);
		break;
	case INTER_CUBIC:
		warpAffine_cubic(src, dst, M, size, value);
		break;
	case INTER_AREA:
	case INTER_LANCZOS4:
	case INTER_LINEAR_EXACT:
	case WARP_FILL_OUTLIERS:
	case  WARP_INVERSE_MAP:
		coutInfo("It's on my TODO list...");
		dealException(digital::kFatal);
		break;

	default:
		coutInfo("Unknown interporate type");
		dealException(digital::kFatal);
		break;
	}

}

//brief�����Ԫ�ر�����ͬʱ��λ����Ϣ�����û�
//     ��Ҫ�ǰѱ����Ĺ��̽������
void geometricTriversal(cv::Mat& src, const TriversalOperatorType& op) {
	cv::Size size = src.size();
	int step = static_cast<int>(src.step);

	uint8_t* cursor = src.data;
	int channels = src.channels();
	for (int y = 0; y < size.height; ++y) {
		auto tmp = cursor;
		for (int x = 0; x < size.width; ++x) {
			op(x, y, tmp);
			//++tmp;
			tmp += channels;
		}
		cursor += step;
	}
}

#undef GET_NEAREST
#undef GET_FLOOR

}//!namespace detail



namespace {

//brief:��ȡ����������ת�����λ��
//becare:���㷽��ΪM*[x,y,1].t()
cv::Mat_<double> __getTransformPosition3D(double x, double y, cv::Mat& M) {
	double arr[] = { x,y,1. };
	cv::Mat tmp(3, 1, CV_64FC1, arr);
	return M * tmp;
}

}
