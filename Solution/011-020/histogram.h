#ifndef _SOLUTION_HISTOGRAM_H_
#define _SOLUTION_HISTOGRAM_H_

#include"Solution/solution.h"

namespace digital {

//brief:
class Histogram :public Solution {
public:
	//parameter: bins:ֱ��ͼ�ķָ����
	Histogram(const string& path, int bins = 256, bool flag = false, const string& name = "Histogram")
		:Solution(name, path, flag), bins_(bins)
	{
		assert(bins_ > 0 && bins_ < 1024);

		getDescriptionHandler().assign("ͳ��ͼ��ĻҶ�ֱ��ͼ");
	}
	~Histogram() override {}

	void operator()()override;

	//brief:����CV_8U����Mat���ݵ�ֱ��ͼ
	static void getHistogram_(void* src, void* dst, int min, int max, int bins );

private:
	int bins_;
};

void trackbarCallback_(int pos, void* userdata);

}//!namespace digital

#endif // !_SOLUTION_HISTOGRAM_H_
