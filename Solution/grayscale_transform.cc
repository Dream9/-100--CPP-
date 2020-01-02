#include"Solution/grayscale_transfrom.h"
#include"Solution/type_extension.h"

#include<opencv2/imgproc.hpp>

#include<numeric>

namespace {

const int kCV8U_UPPER_BOUND = 1 << 8;

}

namespace detail {

//brief:计算单个Mat的指定通道的概率密度函数
void calcHistogram(const cv::Mat& data, cv::Mat& hist, int min, int max, int bins, int* select_channels, int len) {
	//assert(data.channels() == 1);
	assert(bins > 0);
	assert(data.depth() == CV_8U);
	assert(len == -1 || (len > 0 && len < data.channels()));

	hist = cv::Mat::zeros(bins, 1, CV_32F);

	int ranges = max - min + 1;
	double ratio = 1. * bins / ranges;
	float* arr = hist.ptr<float>(0, 0);
	cv::Size size = data.size();
	int channels = data.channels();	
	int step = static_cast<int>(data.step);

	if (data.isContinuous()) {
		size.width *= size.height;
		size.height = 1;
	}
	if (!select_channels || len == -1 || len == channels) {
		//以下为全部channel参与计算
		//连续性判断
		size.width *= channels;

		//根据灰度值分发即可
		auto cur = data.data;
		for (int i = 0; i < size.height; ++i) {
			auto tmp = cur;
			for (int j = 0; j < size.width; ++j) {
				int index = static_cast<int>(*tmp * ratio);
				++arr[index];
				++tmp;
			}
			cur += step;
		}
	}
	else {
		//以下为部分channels参与运算
		//先经过排序，就可以以顺序方式遍历筛选
		std::sort(select_channels, select_channels + len);
		auto cur = data.data;
		for (int i = 0; i < size.height; ++i) {
			auto tmp = cur;
			for (int j = 0; j < size.width; ++j) {
				//筛选通道
				int first = 0;
				for (int cn = 0; first < len; ++cn) {
					if (select_channels[first] != cn) {
						++tmp;
						continue;
					}

					int index = static_cast<int>(*tmp * ratio);
					++arr[index];
					++tmp;
					++first;
				}
			}
			cur += step;
		}
	}
}



//brief:根据hist数值生成对应的直方图图像Mat
//becare:期待hist为数据类型CV_32F的列向量
void fillHistogram(const cv::Mat& hist,
	cv::Mat& dst,
	const cv::Size& size,
	int type,
	const cv::Scalar& color) 
{
	assert(hist.cols == 1);
	assert(hist.depth() == CV_32F);

	dst = cv::Mat(size, type, cv::Scalar::all(255));
	//cv::Mat(size, type, cv::Scalar::all(255)).copyTo(dst);

	double step = 1. * size.width / hist.rows;//确定水平步长

	double max_val;
	cv::minMaxLoc(hist, nullptr, &max_val);
	double scale = 0.9 * size.height / max_val;//确定垂直伸缩比例，使得最大的灰度占据90%的全长

	double x_pos_d = 0;
	int x_pos = 0;
	int y = size.height;

	auto cursor = hist.ptr<float>(0, 0);
	auto end = hist.ptr<float>(hist.rows - 1, 0);
	while (cursor != end) {
		if (fabs(*cursor - 0.0) < digital::k_EPS) {
			x_pos_d += step;
			//FIXME:不要直接用int记录，否则存在舍入精度的损失,特别是step小于1时，总是被舍弃
			//x_pos = static_cast<int>(x_pos_d + step);
			++cursor;
			continue;
		}

		x_pos = static_cast<int>(x_pos_d);

#ifndef NDEBUG
		//for test
		digital::__printInfo("%d,%d--%d,%d\r\n", x_pos, y, x_pos, y - static_cast<int>(*cursor *scale));
#endif

		//brief；通过line表示出直方图分布
		cv::line(dst, cv::Point(x_pos, y), cv::Point(x_pos, y - static_cast<int>(*cursor * scale)), color, 1, 0);
		x_pos_d += step;
		++cursor;
	}
}

//brief:灰度反转
void colorInversion(cv::Mat& src, int max_value) {
	src = cv::Scalar::all(max_value) - src;
}

//brief；
//becare:用户操作可以直接修改/读取每个像素灰度值
//      本函数不提供类型的直接分发处理(数据handler都是uint8_t*)，用户提供的操作ops必须具备不同类型的反射机制，或者ops
//      能确保以何种类型处理数据
void grayscaleTransform(cv::Mat& src, const GrayScaleOperationType& ops, bool not_merge_channel) {
	assert(!src.empty());
	//assert(src.depth() == CV_8U);//已经扩展

	size_t jumpsize = not_merge_channel ? src.elemSize() : src.elemSize1();//突然发现了为什么opencv要增加这么一个接口，这真的说明c++缺少运行期的类型反射，不得不这样做
	
	cv::Size size = src.size();
	int step = static_cast<int>(src.step);
	if (src.isContinuous()) {
		size.width *= size.height * (not_merge_channel ? 1 : src.channels());
		size.height = 1;
	}

	auto cur = src.data;
	for (int i = 0; i < size.height; ++i) {
		auto tmp = cur;
		for (int j = 0; j < size.width; ++j) {
			ops(tmp);
			//++tmp;
			tmp += jumpsize;
		}
		cur += step;
	}
}

//brief；伸缩转换
void convertScaleAbs(cv::Mat& src, cv::Mat& dst, double alpha, double beta) {
	assert(!src.empty());
	assert(src.depth() == CV_8U);

	dst.create(src.size(), CV_MAKETYPE(CV_8U, src.channels()));

	uint8_t* iter = dst.data;
	auto set_value = [&](uint8_t* cursor) {
		*iter = cv::saturate_cast<uint8_t>(*cursor * alpha + beta);
		++iter;
	};
	detail::grayscaleTransform(src, set_value);
}

//brief:直方图均衡化
void equalizeHist(cv::Mat& src, cv::Mat& dst) {
	assert(!src.empty());
	assert(src.depth() == CV_8U);

	int buf[kCV8U_UPPER_BOUND];
	memset(buf, 0, sizeof buf);

	//获得图像的pdf
	auto get_pdf = [&](uint8_t* cursor) {
		++buf[*cursor];
	};
	detail::grayscaleTransform(src, get_pdf);

	//获得图像cdf
	int cdf[kCV8U_UPPER_BOUND];
	std::partial_sum(buf, buf + kCV8U_UPPER_BOUND, cdf);

	double alpha = 1.*kCV8U_UPPER_BOUND / (src.rows * src.cols * src.channels());

	//预计算映射关系
	buf[0] = 0;
	std::transform(cdf + 1, cdf + kCV8U_UPPER_BOUND, buf + 1, [=](int val) {
		 return cv::saturate_cast<uint8_t>(val * alpha - 1);
	});

	dst.create(src.size(), src.type());
	auto iter = dst.data;
	//灰度均衡化
	auto equalize_func = [&](uint8_t* cursor) {
		*iter = static_cast<uint8_t>(buf[*cursor]);
		++iter;
	};
	detail::grayscaleTransform(src, equalize_func);
}

//brief:查找表
void LUT(cv::Mat& src, cv::Mat& dst, cv::Mat& lut) {
	assert(!src.empty());
	assert(src.depth() == CV_8U);
	assert(src.data != dst.data);
	assert(lut.isContinuous());

	dst.create(src.size(), src.type());
	auto iter = dst.data;
	size_t channels = dst.channels();
	auto buncket = lut.data;

	auto set_new_value = [&](uint8_t* cursor) {
		for (int c = 0; c < channels; ++c)
			*iter++ = buncket[*cursor];
	};

	detail::grayscaleTransform(src, set_new_value);
}

//brief:获得OSTU类间最大阈值
//      参考了opencv库的实现
//becare:只针对类型为uint8_t的数据做了特化
double otsuThreshold(cv::Mat& src) {
	int step = static_cast<int>(src.step);
	cv::Size size = src.size();

	//brief:针对连续内存的优化
	if (src.isContinuous()) {
		size.width *= size.height;
		size.height = 1;
		step = size.width * sizeof(uchar);
	}

	const int N = 1 << (sizeof(uchar) * 8);
	//std::vector<int> hist(N);
	int hist[N] = { 0 };
	int i = 0;
	int j = 0;

	double global_m = 0;
	for (; i < size.height; ++i) {
		auto iter = src.ptr() + step * i;
		j = 0;

		//brief:减少不必要的内存引用
		int end = size.width;
		//brief:进行循环展开
		for (; j + 3 < end; j += 4) {
			++hist[iter[j]];
			++hist[iter[j+1]];
			++hist[iter[j+2]];
			++hist[iter[j+3]];

			global_m += iter[j];
			global_m += iter[j+1];
			global_m += iter[j+2];
			global_m += iter[j+3];
		}
		for (; j < end; ++j) {
			++hist[iter[j]];
			global_m += iter[j];
		}
	}

	double scale = 1.0 / size.width / size.height;
	global_m *= scale;//平均灰度值
	double prev_m1 = 0;
	double prev_p1 = 0;
	double max_sigma = 0;
	double out = 0;

	for (i = 0; i < N; ++i) {
		double pi = hist[i] * scale;
		double cur_p1 = prev_p1 + pi;
		double cur_m1 = (prev_m1*prev_p1 + i * pi) / cur_p1;
		
		//brief:防止除接近0的数
		if (fabs(cur_p1) < 1e-8 || fabs(1 - cur_p1) < 1e-8)
			continue;

		double cur_p2 = 1 - cur_p1;
		double cur_m2 = (global_m - cur_p1 * cur_m1) / cur_p2;

		//becare:这里采用了 P1*P2*(m1-m2)^2的计算，经过化简后与另一个公式相同
		double cur_sigma = cur_p1 * cur_p2*pow(cur_m1 - cur_m2, 2);

		if (cur_sigma > max_sigma) {
			max_sigma = cur_sigma;
			out = i;
		}

		prev_m1 = cur_m1;
		prev_p1 = cur_p1;
	}

	return out;
}

////brief:
////becare:根据Ty类型决定ddepth的类型
//template<typename Ty>
//void addAbs(cv::InputArray first, cv::InputArray second, cv::OutputArray dst) {
//	cv::Mat src1 = first.getMat();
//	cv::Mat src2 = second.getMat();
//
//	assert(src1.type() == src2.type());
//
//	int ddepth = ddepth == -1 ? src1.depth() : ddepth;
//	
//	int elemsize = src1.elemSize();
//	auto iter = src2.ptr<Ty>(0,0);
//	auto cur2 = src2.ptr<Ty>(0,0);
//
////	dst.create(src1.size(),CV_MAKETYPE(CV_8U, ddepth))
////	auto abs_add = [&](const uint8_t* cursor) {
////		Ty* cur = static_cast<Ty*>(static_cast<void*>(cursor));
////		*iter++ = std::fabs(*cur) + ;
////	}
////
////	detail::grayscaleTransform(src1,)
//
//}




}//!namespace detail