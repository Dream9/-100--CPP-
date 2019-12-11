#include"Solution/021-030/equalize_histogram.h"
#include"Solution/grayscale_transfrom.h"

#include<opencv2/highgui.hpp>
#ifdef USE_OPENCVLIB
#include<opencv2/imgproc.hpp>
#endif

#include<numeric>
#include<functional>


namespace {

const int kN = 256;

}

namespace digital {

//brief:使用具有均衡分布pdf特征的cdf作为转换函数，则匹配后的pdf将近似具有均衡的特征
//     由于cdf是固定的，因此结果是恒定的
void EqualizeHistogram::operator()() {
	cv::Mat data = cv::imread(getPath());
	if (data.empty()) {
		dealException(kFileError);
		return;
	}

	cv::Mat img;
#ifdef USE_OPENCVLIB

	//becare:  @param src Source 8-bit single channel image.
	//         @param dst Destination image of the same size and type as src .
	//       因此需要调整一下通道
	cv::equalizeHist(data.reshape(1), img);
	img = img.reshape(3);

#else

	detail::equalizeHist(data, img);

#endif

	if (needShowOriginal())
		show(&data, &img);
	else
		show(&img);

	//变换后的直方图
	if (show_pdf_) {
		cv::Mat hist;
		cv::Mat hist_graph;
		detail::calcHistogram(img, hist, 0, 255, 256);
		detail::fillHistogram(hist, hist_graph, cv::Size(200, 200));
		show(&hist_graph);
	}
}

}//!namespace digital