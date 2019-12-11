#ifndef _SOLUTION_NEAREST_NEIGHBOR_INTERPOLATION_H_
#define _SOLUTION_NEAREST_NEIGHBOR_INTERPOLATION_H_

#include"Solution/solution.h"

namespace digital {

//brief:ͼ�񼸺α任��Ϊ�������̣�ȷ��λ���Լ��ز��������������������ڷ�ʽ�ز���ͼ��
class NearestNeighborInterpolation : public Solution {
public:
	NearestNeighborInterpolation(const string& path, bool flag = false, const string& name = "NearestNeighborInterpolation")
		:Solution(name, path, flag)
	{
		getDescriptionHandler().assign("��������ڲ�ֵ�ķ�ʽ����ͼ��");
	}
	~NearestNeighborInterpolation()override {}

	void operator()()override;
};

}
#endif // !_SOLUTION_NEAREST_NEIGHBOR_INTERPOLATION_H_
