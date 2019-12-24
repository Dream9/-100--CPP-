#include"Solution/041-050/morphology.h"
#include"Solution/morphology_transform.h"

#include<opencv2/highgui.hpp>
#include<opencv2/imgproc.hpp>

namespace digital {

#define SHOW_IMG(x,y,str) if(needShowOriginal())\
						show(&x, &y, str);\
				  else\
						show(&y);
void Morphology::operator()() {
	static_assert(cv::MORPH_CLOSE == detail::MORPH_CLOSE, "compability error");
	static_assert(cv::MORPH_OPEN== detail::MORPH_OPEN, "compability error");
	static_assert(cv::MORPH_ERODE == detail::MORPH_ERODE, "compability error");
	static_assert(cv::MORPH_DILATE == detail::MORPH_DILATE, "compability error");
	static_assert(cv::MORPH_GRADIENT == detail::MORPH_GRADIENT, "compability error");
	static_assert(cv::MORPH_TOPHAT == detail::MORPH_TOPHAT, "compability error");

	cv::Mat data = cv::imread(getPath());
	if (data.empty()) {
		dealException(kFileError);
		return;
	}
	cv::Mat img;
	cv::Mat kernel;
	string title;

	kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));

	if (op_ & DILATE) {
		title = "ÅòÕÍ²Ù×÷";
#ifdef USE_OPENCVLIB
		cv::dilate(data, img, kernel, cv::Point(-1,-1), iter_, cv::BORDER_DEFAULT);
#else
		//cv::Mat test;
		//cv::dilate(data, test, kernel, cv::Point(-1,-1), iter_, cv::BORDER_DEFAULT);
		detail::dilate(data, img, kernel,cv::Point(-1,-1), iter_);
		//__MatrixTest(&test, &img);//it should be same
#endif
		SHOW_IMG(data, img, title);
	}

	if (op_ & ERODE) {
		title = "¸¯Ê´²Ù×÷";
#ifdef USE_OPENCVLIB
		cv::erode(data, img, kernel, cv::Point(-1, -1), iter_,cv::BORDER_DEFAULT);
#else
		//cv::Mat test;
		//cv::erode(data, test, kernel, cv::Point(-1, -1), iter_,cv::BORDER_DEFAULT);
		detail::erode(data, img, kernel,cv::Point(-1,-1), iter_);
		//__MatrixTest(&test, &img);//it should be same
#endif

		SHOW_IMG(data, img, title);
	}

	if (op_ & OPEN) {
		title = "¿ª²Ù×÷";
#ifdef USE_OPENCVLIB
		cv::morphologyEx(data, img, cv::MORPH_OPEN, kernel, cv::Point(-1, -1), iter_, cv::BORDER_DEFAULT);//same answer
#else
		detail::morphologyEx(data, img, cv::MORPH_OPEN, kernel, cv::Point(-1, -1), iter_);
#endif
		SHOW_IMG(data, img, title);
	}

	if (op_ & CLOSE) {
		title = "±Õ²Ù×÷";
#ifdef USE_OPENCVLIB
		cv::morphologyEx(data, img, cv::MORPH_CLOSE, kernel, cv::Point(-1, -1), iter_, cv::BORDER_DEFAULT);//same answer
#else
		detail::morphologyEx(data, img, cv::MORPH_CLOSE, kernel, cv::Point(-1, -1), iter_);
#endif
		SHOW_IMG(data, img, title);
	}

}





#undef SHOW_IMG


}//£¡namespace digital


