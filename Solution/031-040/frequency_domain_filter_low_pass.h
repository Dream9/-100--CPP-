#ifndef _SOLUTION_FREQUENCY_DOMAIN_FILTER_LOW_PASS_H_
#define _SOLUTION_FREQUENCY_DOMAIN_FILTER_LOW_PASS_H_

#include"Solution/solution.h"

namespace digital {


class FrequencyDomainFilterLowPass : public Solution {
public:
	//becare:保证和detail::FrequencyFilterFlags兼容
	enum FrequencyFilterFlags {
		ILPF = 0x1,
		BLPF = 0x1 << 1,
		GLPF = 0x1 << 2,

		//IHPF = 0x1 << 3,
		//BHPF = 0x1 << 4,
		//GHPF = 0x1 << 5,

		//BP = 0x1 << 6,
		//BR = 0x1 << 7,
	};

	//paramter:op指定滤波类型，参数参见detail::FrequencyFilterFlags
	FrequencyDomainFilterLowPass(const string& path, 
		int op = ILPF, 
		bool flag = false, 
		const string& name = "FrequencyDomainFilterLowPass")
		:Solution(name, path, flag), op_(op)
	{
		getDescriptionHandler().assign("在频率域完成低通滤波操作");
	}
	~FrequencyDomainFilterLowPass()override {}

	void operator()()override;

private:
	int op_;
};

}

#endif // !_SOLUTION_FREQUENCY_DOMAIN_FILTER_LOW_PASS_H_
