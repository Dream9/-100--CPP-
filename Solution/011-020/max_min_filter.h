#ifndef _SOLUTION_MAX_MIN_FILTER_H_

#include"Solution/solution.h"

namespace digital {

//brief:�����Сֵ�˲�
class MaxMinFilter :public Solution {
public:
	MaxMinFilter(const string&path, int win, bool flag = false, const string& name = "MaxMinFilter")
		:Solution(name, path, flag), win_(win)
	{
		assert((win_ & 0x1) == 0x1);
		getDescriptionHandler().assign("ͨ�������ڵ���ֵ��ֵ�����߽���Ϣ");
	}
	~MaxMinFilter() override {}

	void operator()() override;

private:

	//template<int N>
	//class SubtrationMaxMin {
	//	void operator()
	//};

	int win_;
};

}

#endif // !_SOLUTION_MAX_MIN_FILTER_H_
