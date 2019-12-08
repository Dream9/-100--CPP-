#ifndef _SOLUTION_HISTOGRAM_TRANSFORMATION_H_
#define _SOLUTION_HISTOGRAM_TRANSFORMATION_H_

#include"Solution/solution.h"

namespace digital {

//brief:通过单灰度值变换，调整最终pdf的均值和方差
class HistogramTransformation : public Solution {
public:
	HistogramTransformation(const string& path, 
		double mean,
		double sigma, 
		bool flag = false,
		const string& name = "HistogramTransformation")
		:Solution(name,path,flag), mean_(mean), sigma_(sigma)
	{
		assert(sigma >= 0);
		getDescriptionHandler().assign("使用线性变换，重新调整概率密度函数分布");
	}

	~HistogramTransformation() override {}

	void operator()()override;

private:
	double mean_;
	double sigma_;

};

}//!namespace digital


#endif // !_SOLUTION_HISTOGRAM_TRANSFORMATION_H_
