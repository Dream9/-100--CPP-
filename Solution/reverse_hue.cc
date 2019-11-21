#include"Solution/reverse_hue.h"

#include<opencv2/highgui.hpp>

static const double kPI = 3.141592653;
static const double k2PI = kPI * 2;

namespace digital {

void ReverseHue::operator()() {
	cv::Mat data = cv::imread(getPath(), 1);

	cv::Size size = data.size();
	int step = static_cast<int>(data.step);
	if (data.isContinuous()) {
		size.width *= size.height;
		size.height = 1;
		step = size.width * sizeof(uchar) * 3;
	}

	cv::Mat img = needShowOriginal() ? cv::Mat(data.size(), data.type()) : data;

	for (int i = 0; i < size.height; ++i) {
		auto iter = data.data + step * i;
		auto cur = img.data + step * i;

		int end = size.width;
		for (int j = 0; j < end; ++j) {
			uchar r = *(iter + 2);
			uchar g = *(iter + 1);
			uchar b = *iter;
			double h;
			double s;
			double i;

			rgb2hsi_(r, g, b, h, s, i);
			h += kPI;
			h = h > k2PI ? h - k2PI : h;
			hsi2rgb_(h, s, i, r, g, b);

			cur[0] = b;
			cur[1] = g;
			cur[2] = r;

			cur += 3;
			iter += 3;
		}
	}

	if (needShowOriginal())
		show(&data, &img);
	else
		show(&img);

}

//brief:从rgb转换到hsi
//becare:这里采用了Bajon近似变换，存在一定的误差
void ReverseHue::rgb2hsi_(uint8_t r, uint8_t g, uint8_t b, double& h, double& s, double& i) {
	int sum = r + g + b;
	uchar m = MIN(r, MIN(g, b));

	if (m == b) {
		h = (g - b) / 3.0 / (sum - 3 * b);
	}
	else if (m == r) {
		h = (b - r) / 3.0 / (sum - 3 * r) + 1.0 / 3;
	}
	else {
		h = (r - g) / 3.0 / (sum - 3 * g) + 2.0 / 3;
	}
	h *= k2PI;

	s = 1 - 3.0*m / sum;
	i = sum / 3.0;
}

//brief:hsi转到rgb
void ReverseHue::hsi2rgb_(double h, double s, double i, uint8_t& r, uint8_t& g, uint8_t& b) {
	if (h < k2PI / 3) {
		b = i * (1 - s);
		r = i * (1 + s * cos(h) / cos(kPI / 3 - h));
		g = 3 * i - r - b;
	}
	else if (h < k2PI * 2 / 3) {
		h -= k2PI / 3;
		r = i * (1 - s);
		g = i * (1 + s * cos(h) / cos(kPI / 3 - h));
		b = 3 * i - r - g;
	}
	else{
		h -= k2PI * 2 / 3;
		g = i * (1 - s);
		b = i * (1 + s * cos(h) / cos(kPI / 3 - h));
		r = 3 * i - g - b;
	}
}


}