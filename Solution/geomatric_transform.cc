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

//brief:以最近邻的方式进行插值
//becare:目前只针对CV_8U，其他类型未作扩展
void warpAffine_nearest(const cv::Mat& src, cv::Mat& dst, const cv::Mat& M, cv::Size size, const cv::Scalar& value) {
	assert(src.depth() == CV_8U);
	dst.create(size, src.type());

	cv::Mat M_inv = M.inv();
	int channels = src.channels();
	cv::Rect range(0, 0, src.cols, src.rows);

	//核心变换
	auto set_value = [&src,&M_inv,channels,range,value](int x, int y, uint8_t* cursor) {

		//计算出变换前的位置
		cv::Mat_<double> before = __getTransformPosition3D(x, y, M_inv);
		double* arr = before[0];
		cv::Point pos(GET_NEAREST(arr[0]), GET_NEAREST(arr[1]));

		//for test
		//digital::__printInfo("%d,%d(original)--%d,%d(warp)\r\n", pos.x, pos.y, x, y);

		if (range.contains(pos)) {
			//以最近邻的值代替
			//FIXME:ptr定位用的是行列，但坐标是列x行y!!
			//auto iter = src.ptr(pos.x, pos.y);
			auto iter = src.ptr(pos.y, pos.x);
			for (int i = 0; i < channels; ++i) {
				*cursor++ = *iter++;
			}
		}
		else {
			//填充用户指定的常数值
			for (int i = 0; i < channels; ++i) {
				*cursor++ = cv::saturate_cast<uint8_t>(value[i]);
			}
		}
	};
	
	//与遍历接口搭配在一起即可
	detail::geometricTriversal(dst, set_value);
}


//brief:以双线性插值的方式重采样
//becare:目前只针对CV_8U，其他类型未作扩展
void warpAffine_linear(const cv::Mat& src, cv::Mat& dst, const cv::Mat& M, cv::Size size, const cv::Scalar& value) {
	assert(src.depth() == CV_8U);
	dst.create(size, src.type());
	
	cv::Mat M_inv = M.inv();
	int channels = src.channels();
	cv::Rect range(0, 0, src.cols, src.rows);
	size_t step = src.step;

	//核心变换
	auto set_value = [&src, &M_inv, channels, range, value, step](int x, int y, uint8_t* cursor) {
		cv::Mat_<double> before = __getTransformPosition3D(x, y, M_inv);
		double* arr = before[0];

		cv::Point pos(GET_FLOOR(arr[0]), GET_FLOOR(arr[1]));//获取左上位置的坐标
		if (range.contains(pos)) {
			//确定四个位置的handler
			auto tl = src.ptr(pos.y, pos.x);
			auto tr = tl + 3;
			auto bl = tl + step;
			auto br = bl + 3;

			for (int i = 0; i < channels; ++i) {
#ifdef USE_ORIGINAL_IMPLEMENT
				//遵照原始逻辑的实现
				double tmp1 = __getLinearEvaluate(pos.x, arr[0], *tl++, *tr++);
				double tmp2 = __getLinearEvaluate(pos.x, arr[0], *bl++, *br++);
				*cursor++ = cv::saturate_cast<uint8_t>(__getLinearEvaluate(pos.y, arr[1], tmp1, tmp2));

#else
				//将计算展开后
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
			//填充用户指定的常数值
			for (int i = 0; i < channels; ++i) {
				*cursor++ = cv::saturate_cast<uint8_t>(value[i]);
			}
		}
	};
	
	//与遍历接口搭配在一起即可
	detail::geometricTriversal(dst, set_value);

}

//brief:以双三次插值的方式重采样
void warpAffine_cubic(const cv::Mat& src, cv::Mat& dst, const cv::Mat& M, cv::Size size, const cv::Scalar& value) {
	//
}

}//!namespace




namespace detail {

#define HOMOGENEOUS_MATRIX_SIZE 3,3

//brief:得到指定的旋转操作的放射参数
//becare:几个矩阵相乘可以合并在一起,分开只是逻辑上比较清晰
cv::Mat getRotationMatrix2D(const cv::Point& rotation_center, double angle_count_clock, double x_scale, double y_scale){
	y_scale = y_scale == -1 ? x_scale : y_scale;

	double alpha = Deg2Rad(angle_count_clock);
	double sin_a = sin(alpha);
	double cos_a = cos(alpha);

	//TODO:以下几个矩阵相乘完全可以合并在一起，从而简化运算

	//先进行中心平移
	cv::Mat_<double> out(HOMOGENEOUS_MATRIX_SIZE);
	out <<
		1, 0, -rotation_center.x,
		0, 1, -rotation_center.y,
		0, 0, 1;

	//然后进行放缩
	cv::Mat_<double> tmp(HOMOGENEOUS_MATRIX_SIZE);
	tmp <<
		x_scale, 0, 0,
		0, y_scale, 0,
		0, 0, 1;
	out = tmp * out;

	//然后进行旋转
	tmp <<
		cos_a, sin_a, 0,
		-sin_a, cos_a, 0,
		0, 0, 1;
	out = tmp * out;

	//最后平移回到原点
	tmp <<
		1, 0, rotation_center.x,
		0, 1, rotation_center.y,
		0, 0, 1;
	return tmp * out;
}
#undef HOMOGENEOUS_MATRIX_SIZE 

//brief:根据不同的插值类型进行相应的分发即可
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

//brief：逐个元素遍历，同时将位置信息传给用户
//     主要是把遍历的过程解耦出来
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

//brief:求取单个点坐标转换后的位置
//becare:计算方法为M*[x,y,1].t()
cv::Mat_<double> __getTransformPosition3D(double x, double y, cv::Mat& M) {
	double arr[] = { x,y,1. };
	cv::Mat tmp(3, 1, CV_64FC1, arr);
	return M * tmp;
}

}
