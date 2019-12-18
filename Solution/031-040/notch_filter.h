#ifndef _SOLUTION_NOTCH_FILTER_H_
#define _SOLUTION_NOTCH_FILTER_H_

#include"Solution/solution.h"

namespace digital {

//brief:陷波滤波器，不在100题之中，附赠品-_-
//becare:作为零相移滤波器，关于原点对称是其充分条件
class NotchFilter : public Solution {
public:
	//becare:保证和detail::FrequencyFilterFlags兼容
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

	//paramter:op:执行的操作类型，参见detail::FrequencyFilterFlags
	NotchFilter(const string& path, bool flag = false, const string& name = "NotchFilter")
		:Solution(name, path, flag)
	{
		getDescriptionHandler().assign("进行频率域陷波（零相移）滤波");
	}
	~NotchFilter() override {}

	void operator()()override;
};

}//!namespace
#endif // !_SOLUTION_NOTCH_FILTER_H_
