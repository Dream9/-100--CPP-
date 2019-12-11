#include"Solution/grayscale_transfrom.h"
#include"Solution/type_extension.h"

#include<opencv2/imgproc.hpp>

#include<numeric>

namespace {

const int kCV8U_UPPER_BOUND = 1 << 8;

}

namespace detail {

//brief:���㵥��Mat��ָ��ͨ���ĸ����ܶȺ���
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
		//����Ϊȫ��channel�������
		//�������ж�
		size.width *= channels;

		//���ݻҶ�ֵ�ַ�����
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
		//����Ϊ����channels��������
		//�Ⱦ������򣬾Ϳ�����˳��ʽ����ɸѡ
		std::sort(select_channels, select_channels + len);
		auto cur = data.data;
		for (int i = 0; i < size.height; ++i) {
			auto tmp = cur;
			for (int j = 0; j < size.width; ++j) {
				//ɸѡͨ��
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



//brief:����hist��ֵ���ɶ�Ӧ��ֱ��ͼͼ��Mat
//becare:�ڴ�histΪ��������CV_32F��������
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

	double step = 1. * size.width / hist.rows;//ȷ��ˮƽ����

	double max_val;
	cv::minMaxLoc(hist, nullptr, &max_val);
	double scale = 0.9 * size.height / max_val;//ȷ����ֱ����������ʹ�����ĻҶ�ռ��90%��ȫ��

	double x_pos_d = 0;
	int x_pos = 0;
	int y = size.height;

	auto cursor = hist.ptr<float>(0, 0);
	auto end = hist.ptr<float>(hist.rows - 1, 0);
	while (cursor != end) {
		if (fabs(*cursor - 0.0) < digital::k_EPS) {
			x_pos_d += step;
			//FIXME:��Ҫֱ����int��¼������������뾫�ȵ���ʧ,�ر���stepС��1ʱ�����Ǳ�����
			//x_pos = static_cast<int>(x_pos_d + step);
			++cursor;
			continue;
		}

		x_pos = static_cast<int>(x_pos_d);

#ifndef NDEBUG

		//for test
		digital::__printInfo("%d,%d--%d,%d\r\n", x_pos, y, x_pos, y - static_cast<int>(*cursor *scale));

#endif

		//brief��ͨ��line��ʾ��ֱ��ͼ�ֲ�
		cv::line(dst, cv::Point(x_pos, y), cv::Point(x_pos, y - static_cast<int>(*cursor * scale)), color, 1, 0);
		x_pos_d += step;
		++cursor;
	}
}

//brief:�Ҷȷ�ת
void colorInversion(cv::Mat& src, int max_value) {
	src = max_value - src;
}

//brief��
//becare:�û���������ֱ���޸�/��ȡÿ�����ػҶ�ֵ
//      ֻ���CV_8U���͵����������ػ���Ҳ���Կ��ǲ���detail::filter2D����ʽ��չ�������ͣ�����if-else�ַ�
void grayscaleTransform(cv::Mat& src, const GrayScaleOperationType& ops) {
	assert(!src.empty());
	assert(src.depth() == CV_8U);
	
	cv::Size size = src.size();
	int step = static_cast<int>(src.step);
	if (src.isContinuous()) {
		size.width *= size.height * src.channels();
		size.height = 1;
	}

	auto cur = src.data;
	for (int i = 0; i < size.height; ++i) {
		auto tmp = cur;
		for (int j = 0; j < size.width; ++j) {
			ops(tmp);
			++tmp;
		}
		cur += step;
	}
}

//brief������ת��
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

//brief:ֱ��ͼ���⻯
void equalizeHist(cv::Mat& src, cv::Mat& dst) {
	assert(!src.empty());
	assert(src.depth() == CV_8U);

	int buf[kCV8U_UPPER_BOUND];
	memset(buf, 0, sizeof buf);

	//���ͼ���pdf
	auto get_pdf = [&](uint8_t* cursor) {
		++buf[*cursor];
	};
	detail::grayscaleTransform(src, get_pdf);

	//���ͼ��cdf
	int cdf[kCV8U_UPPER_BOUND];
	std::partial_sum(buf, buf + kCV8U_UPPER_BOUND, cdf);

	double alpha = 1.*kCV8U_UPPER_BOUND / (src.rows * src.cols * src.channels());

	//Ԥ����ӳ���ϵ
	buf[0] = 0;
	std::transform(cdf + 1, cdf + kCV8U_UPPER_BOUND, buf + 1, [=](int val) {
		 return cv::saturate_cast<uint8_t>(val * alpha - 1);
	});

	dst.create(src.size(), src.type());
	auto iter = dst.data;
	//�ҶȾ��⻯
	auto equalize_func = [&](uint8_t* cursor) {
		*iter = static_cast<uint8_t>(buf[*cursor]);
		++iter;
	};
	detail::grayscaleTransform(src, equalize_func);
}





}//!namespace detail