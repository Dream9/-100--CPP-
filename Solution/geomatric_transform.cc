#include"Solution/geometric_transform.h"
#include"Solution/type_extension.h"
#include"Solution/grayscale_transfrom.h"

namespace {

const double kCubic_a = -1.;//���β�ֵ�Ƽ�aȡֵ
const int kCubicWinSize = 4;

//brief:����任ǰλ��
cv::Mat_<double> __getTransformPosition3D(double x, double y, const cv::Mat& M);

//brief:ͬ������ڵ�Ȩ�غ���Ϊ return 1;

//brief:���Բ�ֵȨ�غ���
inline double __getLinearWeight(double position, double target) {
	return std::fabs(position - target);
}
//brief:ֱ�ӹ������Բ�ֵ
inline double __getLinearEvaluate(double left, double medium, double val_left, double val_right) {
	return __getLinearWeight(medium, left) * (val_right - val_left) + val_left;
}

//brief:���β�ֵȨ�غ���
inline double __getCubicWeight(double position, double target) {
	double distance = std::fabs(target - position);
	double tmp2 = distance * distance;
	double tmp3 = tmp2 * distance;

	return distance < 1 ? 
		(kCubic_a + 2)*tmp3 - (kCubic_a + 3)*tmp2 + 1 
		: kCubic_a * tmp3 - 5 * kCubic_a*tmp2 + 8 * kCubic_a*distance - 4 * kCubic_a;
}

void __getCompleteIncludeMatrix(cv::Mat& M, cv::Size& size);

}//!namespace

namespace {

#define GET_NEAREST(x) static_cast<int>(round(x))
#define GET_FLOOR(x) static_cast<int>(floor(x))
#define GET_CEIL(x) static_cast<int>(ceil(x))
#define GET_CUBIC_TOP_LEFT(x) static_cast<int>(floor(x) - 1)

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
//     ˫���Բ�ֵ�ĵ�Ȩ�غ���ΪL1������||x||1
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
		cv::Point pos_br(pos.x + 1, pos.y + 1);//��������
		if (range.contains(pos) && range.contains(pos_br)) {
			//ȷ���ĸ�λ�õ�handler
			auto tl = src.ptr(pos.y, pos.x);
			auto tr = tl + 3;
			auto bl = tl + step;
			auto br = bl + 3;

			for (int i = 0; i < channels; ++i) {
#ifdef USE_ORIGINAL_IMPLEMENT
				//����ԭʼ�߼���ʵ��
				//becare:��ԭʼ���߼�Ӧ���Ǳ���ÿ���㣬Ȼ������ӦȨֵ���ۺ�
				double x
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
//      ���ֲ�����ʽ�Ƕ������������αƽ�������������������Ѳ�ֵ��������Ȼ������ֱ����ʵ����ʹ�ã�
void warpAffine_cubic(const cv::Mat& src, cv::Mat& dst, const cv::Mat& M, cv::Size size, const cv::Scalar& value) {
	assert(src.depth() == CV_8U);
	assert(src.channels() <= 4);

	const int offset = 2;

	cv::Mat src_border;
	cv::copyMakeBorder(src, src_border, offset, offset, offset, offset, cv::BORDER_REPLICATE);

	dst.create(size, src.type());	
	cv::Mat M_inv = M.inv();
	int channels = src.channels();
	cv::Rect range(0, 0, src_border.cols, src_border.rows);
	size_t step = src_border.step;

	//���ı任
	auto set_value = [&src_border, &M_inv, channels, range, value, step, offset](int x, int y, uint8_t* cursor) {
		cv::Mat_<double> before = __getTransformPosition3D(x, y, M_inv);
		double* arr = before[0];

		cv::Point pos(GET_CUBIC_TOP_LEFT(arr[0]) + offset, GET_CUBIC_TOP_LEFT(arr[1]) + offset);//��ȡ����λ�õ�����
		cv::Point pos_br(pos.x + 3, pos.y + 3);//��������
		if (range.contains(pos) && range.contains(pos_br)) {
			cv::Scalar gbra;
			double wight_sum = 0.;
			auto iter = src_border.ptr(pos.y, pos.x);

			//���������ڵ����ݣ�����Ȩֵ���ۺ�
			for (int i = 0; i < kCubicWinSize; ++i) {//��
				auto tmp = iter;
				for (int j = 0; j < kCubicWinSize; ++j) {//��
					double weight_y = __getCubicWeight(pos.y + i, arr[1] + offset);
					double weight_x = __getCubicWeight(pos.x + j, arr[0] + offset);
					double w_x_y = weight_x * weight_y;
					wight_sum += w_x_y;

					for (int c = 0; c < channels; ++c) {//ͨ��
						gbra[c] += w_x_y * *tmp++;
					}
				}
				iter += step;
			}

			//��Ȩƽ��
			for (int c = 0; c < channels; ++c) {
				*cursor++ = cv::saturate_cast<uint8_t>(gbra[c] / wight_sum);
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

#ifndef USE_ORIGINAL_IMPLEMENT
	double a11 = x_scale * cos_a;
	double a12 = y_scale * sin_a;
	double a21 = -x_scale * sin_a;
	double a22 = y_scale * cos_a;

	cv::Mat res(HOMOGENEOUS_MATRIX_SIZE, CV_64FC1);
	double* iter = res.ptr<double>(0, 0);
	*iter++ = a11;
	*iter++ = a12;
	*iter++ = (1 - a11)*x_scale - a12 * y_scale;
	*iter++ = a21;
	*iter++ = a22;
	*iter++ = (1 - a22)*y_scale - a21 * x_scale;
	*iter++ = 0;//FIXME:or not add this?
	*iter++ = 0;
	*iter++ = 1;

	return res;
#else
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
#endif
}
#undef HOMOGENEOUS_MATRIX_SIZE 

//brief:ͨ������ԭʼͼ���������ȷ���µ�ͼ����С�Ĳ���ʧ��Χ
cv::Size getSizeAfterWarpAffine(const cv::Mat& M, const cv::Size& original_size, double* ptr_minx,double* ptr_miny) {
	auto right = __getTransformPosition3D(original_size.width, original_size.height, M);
	auto left = __getTransformPosition3D(0.,0., M);
	auto top = __getTransformPosition3D(original_size.width, 0., M);
	auto bottom = __getTransformPosition3D(0., original_size.height, M);

	double xmax = right[0][0], ymax = right[0][1];
	double xmin = xmax, ymin = ymax;
	xmax = std::max(xmax, left[0][0]);
	xmax = std::max(xmax, top[0][0]);
	xmax = std::max(xmax, bottom[0][0]);

	xmin = std::min(xmin, left[0][0]);
	xmin = std::min(xmin, top[0][0]);
	xmin = std::min(xmin, bottom[0][0]);

	ymax = std::max(ymax, left[1][0]);
	ymax = std::max(ymax, top[1][0]);
	ymax = std::max(ymax, bottom[1][0]);

	ymin = std::min(ymin, left[1][0]);
	ymin = std::min(ymin, top[1][0]);
	ymin = std::min(ymin, bottom[1][0]);

	if (ptr_minx)
		*ptr_minx = xmin;
	if (ptr_miny)
		*ptr_miny = ymin;

	return cv::Size(GET_CEIL(fabs(xmax - xmin)), GET_CEIL(fabs(ymax - ymin)));
}


//brief:���ݲ�ͬ�Ĳ�ֵ���ͽ�����Ӧ�ķַ�����
//
void warpAffine(const cv::Mat& src, cv::Mat& dst, cv::Mat& M, cv::Size size,
	int interpolation_type, const cv::Scalar& value, bool need_complete_include) {

	assert(src.channels() <= 4);
	assert(src.depth() == CV_8U);
	
	if (need_complete_include) {
		//��Ҫ����������Ч����
		size = src.size();
		__getCompleteIncludeMatrix(M, size);
	}

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
#undef GET_CUBIC_TOP_LEFT

}//!namespace detail



namespace {

//brief:��ȡ����������ת�����λ��
//becare:���㷽��ΪM*[x,y,1].t()
cv::Mat_<double> __getTransformPosition3D(double x, double y, const cv::Mat& M) {
	double arr[] = { x,y,1. };
	cv::Mat tmp(3, 1, CV_64FC1, arr);
	return M * tmp;
}

//brief:ͨ��ƽ�Ʋ���ʹ�����е���Ч���ݶ����Ա�չʾ����
void __getCompleteIncludeMatrix(cv::Mat& M, cv::Size& size) {
	double xmin = 0;
	double ymin = 0;

	size = detail::getSizeAfterWarpAffine(M, size, &xmin, &ymin);
	
	cv::Mat_<double> tmp(3,3);
	tmp << 1, 0, -xmin,
		0, 1, -ymin,
		0, 0, 1;
	M = tmp * M;
}

}
