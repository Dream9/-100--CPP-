#include"Solution/solution.h"

#include<opencv2/highgui.hpp>

#include<cstdio>
#include<iostream>
#include<errno.h>

namespace {

void __merger(cv::Mat& src1, cv::Mat& src2, cv::Mat& dst);

inline void __show_without_destroy(const string& name, void* figure);

}

namespace digital {

//brief:展示数据
void Solution::show(void* figure) {
	__show_without_destroy(question_name_, figure);

	cv::waitKey(0);
	cv::destroyAllWindows();
}
//brief:同时展示2幅
void Solution::show(void* figure, void* figure_2) {
	cv::Mat* first = static_cast<cv::Mat*>(figure);
	cv::Mat* second = static_cast<cv::Mat*>(figure_2);

	if (first->type() != second->type()) {
		__show_without_destroy("Orignal",first);
		__show_without_destroy("Answer",second);
		cv::waitKey(0);
		cv::destroyAllWindows();
		return;
	}

	//brief：拼接在一起显示
	cv::Mat out;
	__merger(*first, *second, out);
	cv::imshow(question_name_, out);
	cv::waitKey(0);
	cv::destroyAllWindows();
}

//brief:增加一层间接性
void Solve(Solution& solution) {
	solution();
}

void dealException(ErrorCode code) {
	switch (code) {
	case kNone:
	case kErrorSize:
		return;
		break;
	case kFileError:
		printf("Check the file\n");
		break;
	default:
		printf("Unknown error\n");
	}
}

void __MatrixTest(void* first, void* second) {
	cv::Mat& img = *static_cast<cv::Mat*>(first);
	cv::Mat& out = *static_cast<cv::Mat*>(second);
	
	int rows = img.rows;
	int cols = img.cols;

	for (int i = 0; i < rows; ++i) {
		for (int j = 0; j < cols; ++j) {
			std::cout << img.at<cv::Vec3b>(i, j) << "---"
					  << out.at<cv::Vec3b>(i, j)
				      << "\r\n";
		}
		// you can check this row here
		int __i = 0;	(void)__i;
	}
}



}//!namespace digital

//匿名
namespace{

 //brief：将两幅图像并作一副输出
void __merger(cv::Mat& src1, cv::Mat& src2, cv::Mat& dst)
{
	assert(src1.type() == src2.type());

	const int kOffset = 5;

	int rows = std::max(src1.rows, src2.rows);
	int cols = src1.cols + kOffset + src2.cols;
	dst.create(rows, cols, src1.type());
	src1.copyTo(dst(cv::Rect(0, 0, src1.cols, src1.rows)));
	src2.copyTo(dst(cv::Rect(src1.cols + 5, 0, src2.cols, src2.rows)));
}


void __show_without_destroy(const string& name, void* figure) {
	cv::Mat* out = static_cast<cv::Mat*>(figure);
	
	cv::namedWindow(name, cv::WINDOW_NORMAL);

	//about imshow:If the window was not created before this function, it is assumed creating a window with cv::WINDOW_AUTOSIZE.
	cv::imshow(name, *out);
}

}//!namespace

