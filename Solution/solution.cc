#include"Solution/solution.h"

#include<opencv2/highgui.hpp>

#include<cstdio>
#include<iostream>
#include<errno.h>

namespace {

void __merger(cv::Mat& src1, cv::Mat& src2, cv::Mat& dst);

inline void __show_without_destroy(const string& name, void* figure);

void __exit_failure();

void __cout_data(int ddepth, void* cursor, char delims = ' ');

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
//becare:这里隐含着要求所有图像的type一致
//TODO：允许不同type的同时显示
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
			//FIXME: 或者抛出错误？
			__exit_failure();
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

#define DIGITAL_ERROR_FORMAT "[%s:%d:%s]"
//brief:处理错误
//parameter:code:错误代码
//          file:文件名
//          line:行号
//          func：函数名
void __dealException(
	ErrorCode code,
	const char* file, 
	int line,
	const char* func) 
{
	switch (code) {
	case kNone:
	case kCodeSize:
		return;
		break;
	case kFileError:
		fprintf(stderr, DIGITAL_ERROR_FORMAT "File may be broken or not exist.\r\n", file, line, func);
		break;
	case kParameterNotMatch:
		fprintf(stderr, DIGITAL_ERROR_FORMAT "Parameter does not match.\r\n", file, line, func);
		break;
	case kImshowTypeIsNotCV8U:
		fprintf(stderr, DIGITAL_ERROR_FORMAT "Warning:you should make sure that data range is right.\r\n", file, line, func);
		break;
	case kFatal:
		fprintf(stderr, DIGITAL_ERROR_FORMAT "Inner fatal error occured.\r\n", file, line, func);
		__exit_failure();
		break;
	case kFatalSys:
		fprintf(stderr, DIGITAL_ERROR_FORMAT, file, line, func);
		perror("System error.\r\n");
		__exit_failure();
		break;
	default:
		printf(DIGITAL_ERROR_FORMAT "Unknown error.\n", file, line, func);
	}
}
#undef DIGITAL_ERROR_FORMAT

//brief:just for test , never be used outside
void __MatrixTest(void* Mat1) {
	cv::Mat& img = *static_cast<cv::Mat*>(Mat1);
	
	int rows = img.rows;
	int cols = img.cols;

	int depth = img.depth();
	int elemsize1 = static_cast<int>(img.elemSize1());
	int channels = img.channels();

	auto cur = img.data; 
	for (int i = 0; i < rows; ++i) {
		for (int j = 0; j < cols; ++j) {
			for (int c = 0; c < channels; ++c) {
				__cout_data(depth, cur);
				cur += elemsize1;
			}
		}
		std::cout << "\r\n";
		// you can check this row here
		int __i = 0;	(void)__i;
	}
}

//brief:just for test , never be used outside
void __MatrixTest(void* first, void* second) {
	cv::Mat& img = *static_cast<cv::Mat*>(first);
	cv::Mat& out = *static_cast<cv::Mat*>(second);
	
	int rows = img.rows;
	int cols = img.cols;
	
	int depth = img.depth();
	int depth_out = out.depth();
	int elemsize1 = static_cast<int>(img.elemSize1());
	int elemsize1_out = static_cast<int>(out.elemSize1());
	int channels = img.channels();

	auto cur = img.data;
	auto cur_out = out.data;

	for (int i = 0; i < rows; ++i) {
		for (int j = 0; j < cols; ++j) {
			for (int c = 0; c < channels; ++c) {
				__cout_data(depth, cur);
				__cout_data(depth_out, cur_out);
				std::cout << "\r\n";
				cur += elemsize1;
				cur_out += elemsize1_out;
			}
		}
		std::cout << "----------------\r\n";
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
	//cv::namedWindow(name, cv::WINDOW_AUTOSIZE);

	//beacare:imshow针对非8位深度的图像会做一个！！固定！！比例的伸缩！！
    //imshow:If the window was not created before this function, it is assumed creating a window with cv::WINDOW_AUTOSIZE.
	//  -If the image is 8 - bit unsigned, it is displayed as is.
	//	- If the image is 16 - bit unsigned or 32 - bit integer, the pixels are divided by 256. That is, the
	//	value range[0, 255 * 256] is mapped to[0, 255].
	//	- If the image is 32 - bit or 64 - bit floating - point, the pixel values are multiplied by 255. That is, the
    //	value range[0, 1] is mapped to[0, 255].
	cv::imshow(name, *out);
}

//becare:针对VS编译器的命令窗口会随进程退出的问题
void __exit_failure() {
#ifdef _MSC_VER
	system("pause");
#endif
	exit(EXIT_FAILURE);
}


//brief:根据不同深度输出结果
#define __CASE_DEPTH_COUT(x, type, v, delims) case x:\
                                    std::cout << *(static_cast<type *>(v)) << delims;\
                                    break;
void __cout_data(int ddepth, void* cursor, char delims) {
	switch (ddepth)
	{
		//__CASE_DEPTH_COUT(CV_8U, uint8_t, cursor, delims);
		//强制输出数字而非字符
	case CV_8U:
		std::cout << +*(static_cast<uint8_t*>(cursor)) << delims;
		break;

		__CASE_DEPTH_COUT(CV_8S, int8_t, cursor, delims);
		__CASE_DEPTH_COUT(CV_16S, int16_t, cursor, delims);
		__CASE_DEPTH_COUT(CV_16U, uint16_t, cursor, delims);
		__CASE_DEPTH_COUT(CV_32S, int32_t, cursor, delims);
		__CASE_DEPTH_COUT(CV_32F, float, cursor, delims);
		__CASE_DEPTH_COUT(CV_64F, double, cursor, delims);
	default:
		dealException(digital::kParameterNotMatch);
		break;
	}
}
#undef __CASE_DEPTH_COUT

}//!namespace

