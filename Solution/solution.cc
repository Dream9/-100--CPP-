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
	show(figure, question_name_);
}
void Solution::show(void* figure, const string& str) {
	__show_without_destroy(str, figure);

	cv::waitKey(0);
	cv::destroyAllWindows();
}

//brief:同时展示2幅
void Solution::show(void* figure, void* figure_2) {
	show(figure, figure_2, question_name_);
}
void Solution::show(void* figure, void* figure_2, const string& str) {
	cv::Mat* first = static_cast<cv::Mat*>(figure);
	cv::Mat* second = static_cast<cv::Mat*>(figure_2);

	//brief:类型不一致时
	if (first->type() != second->type()) {
		__show_without_destroy("Orignal",first);
		__show_without_destroy("Answer",second);
		cv::waitKey(0);
		cv::destroyAllWindows();
		return;
	}

	//brief：一致时，拼接在一起显示
	cv::Mat out;
	__merger(*first, *second, out);

	cv::namedWindow(str, cv::WINDOW_NORMAL);
	cv::imshow(str, out);
	cv::waitKey(0);
	cv::destroyAllWindows();
}

//brief:展示多幅数据
//parameter:figs：指向Mat*的指针数组
//          len：数组长度
void Solution::show(void** figs, int len) {
	show(figs, len, question_name_);
}
void Solution::show(void** figs, int len, const string& str) {
	int i = 0;
	cv::Size size(0, 0);
	auto type = static_cast<cv::Mat*>(figs[0])->type();
	while (i < len) {

#ifndef NDEBUG
		//debug时检查所有的类型匹配
		cv::Mat* cur = static_cast<cv::Mat*>(figs[i]);
		if (cur->type() != type) {
			dealException(kParameterNotMatch);
			exit(EXIT_FAILURE);
		}
#endif

		size.height = MAX(size.height, static_cast<cv::Mat*>(figs[i])->rows);
		size.width += static_cast<cv::Mat*>(figs[i])->cols + 5;

		++i;
	}
	cv::Mat dst(size, type);

	int last_cols = 0;
	i = 0;
	while (i < len) {
		cv::Mat* src = static_cast<cv::Mat*>(figs[i]);
		src->copyTo(dst(cv::Rect(last_cols, 0, src->cols, src->rows)));
		last_cols += src->cols + 5;
		++i;
	}
	
	cv::namedWindow(str, cv::WINDOW_NORMAL);
	cv::imshow(str, dst);
	cv::waitKey(0);
	cv::destroyAllWindows();
}

//brief:统一的方案解决接口
void Solve(Solution& solution) {
	solution();
}

//brief:处理错误
void dealException(ErrorCode code) {
	switch (code) {
	case kNone:
	case kCodeSize:
		return;
		break;
	case kFileError:
		fprintf(stderr, "File may be broken or not exist.\r\n");
		break;
	case kParameterNotMatch:
		fprintf(stderr, "Parameter does not match.\r\n");
		break;
	case kFatal:
		fprintf(stderr, "Inner fatal error occured.\r\n");
		exit(EXIT_FAILURE);
		break;
	case kFatalSys:
		perror("System error.\r\n");
		exit(EXIT_FAILURE);
		break;
	default:
		printf("Unknown error.\n");
	}
}

//brief:just for test , never be used outside
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

//brief:生成展示
void __show_without_destroy(const string& name, void* figure) {
	cv::Mat* out = static_cast<cv::Mat*>(figure);
	
	//or AUTOSIZE?
	cv::namedWindow(name, cv::WINDOW_NORMAL);

	//about imshow:If the window was not created before this function, it is assumed creating a window with cv::WINDOW_AUTOSIZE.
	cv::imshow(name, *out);
}

}//!namespace

