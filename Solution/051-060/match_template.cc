#include"Solution/051-060/match_template.h"
#include"Solution/geometry_match.h"

#include<opencv2/highgui.hpp>
#include<opencv2/imgproc.hpp>

namespace digital {

void MatchTemplate::operator()() {
	//兼容性判断
	static_assert(TM_SQDIFF == detail::TM_SQDIFF, "compability error");
	static_assert(TM_SQDIFF_NORMED == detail::TM_SQDIFF_NORMED, "compability error");
	static_assert(TM_CCORR == detail::TM_CCORR, "compability error");
	static_assert(TM_CCORR_NORMED == detail::TM_CCORR_NORMED, "compability error");
	static_assert(TM_CCOEFF== detail::TM_CCOEFF, "compability error");
	static_assert(TM_CCOEFF_NORMED== detail::TM_CCOEFF_NORMED, "compability error");
	static_assert(TM_ABSDIFF== detail::TM_ABSDIFF, "compability error");
	static_assert(TM_ABSDIFF_NORMED== detail::TM_ABSDIFF_NORMED, "compability error");

	cv::Mat data = cv::imread(getPath(), cv::IMREAD_GRAYSCALE);
	cv::Mat templ = cv::imread(template_path_, cv::IMREAD_GRAYSCALE);
	if (data.empty() || templ.empty()) {
		dealException(kFileError);
		return;
	}
	cv::Mat img;
	//show(&data, &templ);

	if (op_ == TM_SQDIFF || op_ == TM_SQDIFF_NORMED) {
		//基于L2距离（差的平方和）的判断
#ifdef USE_OPENCVLIB
		cv::Mat test;
#ifdef WITHOUT_NORMED
		cv::matchTemplate(data, templ, test, TM_SQDIFF);
#else
		cv::matchTemplate(data, templ, test, TM_SQDIFF_NORMED);
#endif
		img = test;
#else
#ifdef WITHOUT_NORMED
		detail::matchTemplate(data, templ, img, TM_SQDIFF);
#else
		detail::matchTemplate(data, templ, img, TM_SQDIFF_NORMED);
#endif
#endif
		//for test
		//__MatrixTest(&img, &test);//it should be the same
	}
	else if (op_ == TM_CCORR || op_ == TM_CCORR_NORMED) {
		//基于相关的判断
#ifdef USE_OPENCVLIB
		cv::Mat test;
#ifdef WITHOUT_NORMED
		cv::matchTemplate(data, templ, test, TM_CCORR);
#else
		cv::matchTemplate(data, templ, test, TM_CCORR_NORMED);
#endif
		img = test;
#else
#ifdef WITHOUT_NORMED
		detail::matchTemplate(data, templ, img, TM_CCORR);
#else
		detail::matchTemplate(data, templ, img, TM_CCORR_NORMED);
#endif
#endif
		//for test
		//__MatrixTest(&img, &test);//it should be the same
	}
	else if (op_ == TM_CCOEFF || op_ == TM_CCOEFF_NORMED) {
		//基于相关系数（未归一化时，采用的是协方差）的判断
#ifndef USE_OPENCVLIB
		cv::Mat test;
#ifdef WITHOUT_NORMED
		cv::matchTemplate(data, templ, test, TM_CCOEFF);
#else
		cv::matchTemplate(data, templ, test, TM_CCOEFF_NORMED);
#endif
		img = test;
#else
#ifdef WITHOUT_NORMED
		detail::matchTemplate(data, templ, img, TM_CCOEFF);
		cv::Mat test;
		cv::matchTemplate(data, templ, test, TM_CCOEFF);
#else
		detail::matchTemplate(data, templ, img, TM_CCOEFF_NORMED);
		cv::Mat test;
		cv::matchTemplate(data, templ, test, TM_CCOEFF_NORMED);
#endif
#endif
		//for test
		//__MatrixTest(&img, &test);//it should be the same
	}
	else if (op_ == TM_ABSDIFF || op_ == TM_ABSDIFF_NORMED) {
		//基于L1范数进行相似性衡量
#ifndef WITHOUT_NORMED
		detail::matchTemplate(data, templ, img, TM_ABSDIFF);
#else
		detail::matchTemplate(data, templ, img, TM_ABSDIFF_NORMED);
#endif
	}

	//定位最相似的图像位置
	double maxval;
	double minval;
	cv::Point maxP;
	cv::Point minP;
	cv::normalize(img, img, 0, 1, cv::NORM_MINMAX);
	cv::minMaxLoc(img, &minval, &maxval, &minP, &maxP);

	cv::Point matchLoc;
	matchLoc = op_ ==TM_SQDIFF || op_==TM_SQDIFF_NORMED || op_ == TM_ABSDIFF ? minP : maxP;
	cv::rectangle(data, matchLoc, cv::Point(matchLoc.x + templ.cols, matchLoc.y + templ.rows), cv::Scalar::all(0));
    cv::circle(img, matchLoc, 2,op_ ==TM_SQDIFF || op_==TM_SQDIFF_NORMED || op_ == TM_ABSDIFF \
		? cv::Scalar::all(255):cv::Scalar::all(0));

	if (needShowOriginal()) {
		void* figs[3];
		figs[0] = &data;
		figs[1] = &templ;
		figs[2] = &img;
		show(figs, sizeof figs / sizeof(void*));
	}
	else
		show(&data, &img);
}

}//!namespace digital