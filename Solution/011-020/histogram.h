#ifndef _SOLUTION_HISTOGRAM_H_
#define _SOLUTION_HISTOGRAM_H_

#include"Solution/solution.h"

namespace digital {

//brief:
class Histogram :public Solution {
public:
	//parameter: bins:直方图的分割块数
	Histogram(const string& path, int bins = 256, bool flag = false, const string& name = "Histogram")
		:Solution(name, path, flag), bins_(bins)
	{
		assert(bins_ > 0 && bins_ < 1024);

		getDescriptionHandler().assign("统计图像的灰度直方图");
	}
	~Histogram() override {}

	void operator()()override;

	//brief:计算CV_8U类型Mat数据的直方图
	static void getHistogram_(void* src, void* dst, int min, int max, int bins );

private:
	int bins_;
};

void trackbarCallback_(int pos, void* userdata);

}//!namespace digital

#endif // !_SOLUTION_HISTOGRAM_H_
