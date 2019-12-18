#ifndef _SOLUTION_NOTCH_FILTER_H_
#define _SOLUTION_NOTCH_FILTER_H_

#include"Solution/solution.h"

namespace digital {

//brief:�ݲ��˲���������100��֮�У�����Ʒ-_-
//becare:��Ϊ�������˲���������ԭ��Գ�����������
class NotchFilter : public Solution {
public:
	//becare:��֤��detail::FrequencyFilterFlags����
	enum FrequencyFilterFlags {
		//ILPF = 0x1,
		//BLPF = 0x1 << 1,
		//GLPF = 0x1 << 2,

		//IHPF = 0x1 << 3,
		//BHPF = 0x1 << 4,
		//GHPF = 0x1 << 5,

		BP = 0x1 << 6,
		BR = 0x1 << 7,
	};

	//paramter:op:ִ�еĲ������ͣ��μ�detail::FrequencyFilterFlags
	NotchFilter(const string& path, bool flag = false, const string& name = "NotchFilter")
		:Solution(name, path, flag)
	{
		getDescriptionHandler().assign("����Ƶ�����ݲ��������ƣ��˲�");
	}
	~NotchFilter() override {}

	void operator()()override;
};

}//!namespace
#endif // !_SOLUTION_NOTCH_FILTER_H_
