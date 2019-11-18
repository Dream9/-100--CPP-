#ifndef _SOLUTION_GRAYSCALE_H_
#define _SOLUTION_GRAYSCALE_H_ 

#include"Solution/solution.h"

#include<opencv2/highgui.hpp>

namespace digital {

//brief:
class Grayscale :public Solution {
public:
	Grayscale(const string& path, bool show_old, const string& name = "Grayscale")
		:Solution(name, path, show_old) 
	{
		getDescriptionHandler().assign("彩色图像灰度化");
	}

	virtual ~Grayscale() {  }

	virtual void operator()()
	{
		cv::Mat data = cv::imread(getPath(), 1);

		if (data.empty()) {
			dealException(kFileError);
			return;
		}
		
		int rows = data.rows;
		int cols = data.cols;

		cv::Mat img(rows, cols, CV_8UC1);

		for (int i = 0; i < rows; ++i) {
			auto ptr = data.ptr<uchar>(i);
			auto target = img.ptr<uchar>(i);
			for (int j = 0; j < cols; ++j) {
				//按照加权平均获得灰度值，rgb:0.3,0.59,0.11
				*target = static_cast<uint8_t>(0.11*(*ptr) + 0.59*(*(ptr + 1)) + 0.3*(*(ptr + 2)));

				++target;
				ptr += 3;
			}
		}

		if (needShowOriginal())
			show(&data, &img);
		else
			show(&img);
	}
};
}





#endif