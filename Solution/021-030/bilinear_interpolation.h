#ifndef _SOLUTION_BILINEAR_INTERPOLATION_H_
#define _SOLUTION_BILINEAR_INTERPOLATION_H_

#include"Solution/solution.h"

namespace digital {

//brief:˫�����ڲ�ķ�ʽ������ڲ����Ľ��Ҫƽ����ʧ�����
class BilinearInterpolation : public Solution {
public:
	BilinearInterpolation(const string& path, bool flag = false, const string& name = "BilinearInterpolation")
		:Solution(name, path, flag)
	{
		getDescriptionHandler().assign("����˫�����ڲ�Ĳ�����ʽ�����任���ͼ��");
	}
	~BilinearInterpolation()override {}

	void operator()()override ;
};

}


#endif // !_SOLUTION_BILINEAR_INTERPOLATION_H_
