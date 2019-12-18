#ifndef _SOLUTION_FREQUENCY_DOMAIN_FILTER_HIGH_PASS_H_
#define _SOLUTION_FREQUENCY_DOMAIN_FILTER_HIGH_PASS_H_

#include"Solution/solution.h"

namespace digital {


class FrequencyDomainFilterHighPass : public Solution {
public:
	//becare:保证和detail::FrequencyFilterFlags兼容
	enum FrequencyFilterFlags {
		//ILPF = 0x1,
		//BLPF = 0x1 << 1,
		//GLPF = 0x1 << 2,

		IHPF = 0x1 << 3,
		BHPF = 0x1 << 4,
		GHPF = 0x1 << 5,

		//BP = 0x1 << 6,
		//BR = 0x1 << 7,
	};

	//paramter:op:执行的操作类型，参见detail::FrequencyFilterFlags
	FrequencyDomainFilterHighPass(const string& path, int op, bool flag = false, const string& name = "FrequencyDomainFilterHighPass")
		:Solution(name, path, flag), op_(op)
	{
		getDescriptionHandler().assign("进行频率域高通滤波");
	}
	~FrequencyDomainFilterHighPass() override {}

	void operator()()override;
private:
	int op_;
};
}


#endif