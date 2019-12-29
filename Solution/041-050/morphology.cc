#include"Solution/041-050/morphology.h"
#include"Solution/morphology_transform.h"
#include"Solution/grayscale_transfrom.h"

#include<opencv2/highgui.hpp>
#include<opencv2/imgproc.hpp>



#include"Solution/geometric_transform.h"

namespace digital {

//for test, no meaning
void test(cv::OutputArray laji) {
	laji.create(cv::Size(1, 20),CV_32SC2);
	cv::Mat data = laji.getMat();
	data.at<int>(0, 0) = 15;

	//data = cv::Mat::zeros(cv::Size(10, 10), CV_8U);
	//return;
}

#define SHOW_IMG(x,y,str) if(needShowOriginal())\
						show(&x, &y, str);\
				  else\
						show(&y);
void Morphology::operator()() {
	//�����Զ���
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
		title = "���Ͳ���";
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
		title = "��ʴ����";
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
		title = "������";
#ifdef USE_OPENCVLIB
		cv::morphologyEx(data, img, cv::MORPH_OPEN, kernel, cv::Point(-1, -1), iter_, cv::BORDER_DEFAULT);//same answer
#else
		detail::morphologyEx(data, img, cv::MORPH_OPEN, kernel, cv::Point(-1, -1), iter_);
#endif
		SHOW_IMG(data, img, title);
	}

	if (op_ & CLOSE) {
		title = "�ղ���";
#ifdef USE_OPENCVLIB
		cv::morphologyEx(data, img, cv::MORPH_CLOSE, kernel, cv::Point(-1, -1), iter_, cv::BORDER_DEFAULT);//same answer
#else
		detail::morphologyEx(data, img, cv::MORPH_CLOSE, kernel, cv::Point(-1, -1), iter_);
#endif
		SHOW_IMG(data, img, title);
	}

	if (op_ & GRADIENT) {
		title = "��̬ѧ�ݶ�";
#ifdef USE_OPENCVLIB
		cv::morphologyEx(data, img, cv::MORPH_GRADIENT, kernel, cv::Point(-1, -1), iter_, cv::BORDER_DEFAULT);//same answer
#else
		detail::morphologyEx(data, img, cv::MORPH_GRADIENT, kernel, cv::Point(-1, -1), iter_);
#endif
		SHOW_IMG(data, img, title);
	}

	if (op_ & TOPHAT) {
		title = "�׶�ñ����������㹻���SE";
		kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(30, 30));
#ifdef USE_OPENCVLIB
		cv::morphologyEx(data, img, cv::MORPH_TOPHAT, kernel, cv::Point(-1, -1), iter_, cv::BORDER_DEFAULT);//same answer
#else
		detail::morphologyEx(data, img, cv::MORPH_TOPHAT, kernel, cv::Point(-1, -1), iter_);
#endif
		SHOW_IMG(data, img, title);

		//brief:һ����֤����һ�㣬ԭͼ��ı��������ǲ����ȵģ�ֱ��OTSU��ֵ������һЩλ�û����
		double th = detail::otsuThreshold(data);
		cv::Mat a;
		cv::threshold(data, a, th, 255, cv::THRESH_BINARY);
		
		double tb = detail::otsuThreshold(img);
		cv::Mat b;
		cv::threshold(img, b, tb, 255, cv::THRESH_BINARY);

		show(&a, &b,"��ͼ�����׶�ñ�������պ��ֵ���Ľ�����ӽӽ�����ʵֵ");
	}

	if (op_ & BLACKHAT) {
		title = "�ڵ�ñ�� ��������㹻���SE";
		kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(30, 30));
#ifdef USE_OPENCVLIB
		cv::morphologyEx(data, img, cv::MORPH_BLACKHAT, kernel, cv::Point(-1, -1), iter_, cv::BORDER_DEFAULT);//same answer
#else
		detail::morphologyEx(data, img, cv::MORPH_BLACKHAT, kernel, cv::Point(-1, -1), iter_);
#endif
		SHOW_IMG(data, img, title);
	}

}





#undef SHOW_IMG


}//��namespace digital


