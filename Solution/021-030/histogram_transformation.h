#ifndef _SOLUTION_HISTOGRAM_TRANSFORMATION_H_
#define _SOLUTION_HISTOGRAM_TRANSFORMATION_H_

#include"Solution/solution.h"

namespace digital {

//brief:ͨ�����Ҷ�ֵ�任����������pdf�ľ�ֵ�ͷ���
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
		getDescriptionHandler().assign("ʹ�����Ա任�����µ��������ܶȺ����ֲ�");
	}

	~HistogramTransformation() override {}

	void operator()()override;

private:
	double mean_;
	double sigma_;

};

}//!namespace digital


#endif // !_SOLUTION_HISTOGRAM_TRANSFORMATION_H_
