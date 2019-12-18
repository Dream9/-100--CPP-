#ifndef _SOLUTION_FREQUENCY_DOMAIN_FILTER_BAND_H_
#define _SOLUTION_FREQUENCY_DOMAIN_FILTER_BAND_H_

#include"Solution/solution.h"

namespace digital {


class FrequencyDomainFilterBand : public Solution {
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
	FrequencyDomainFilterBand(const string& path, int op, bool flag = false, const string& name = "FrequencyDomainFilterBand")
		:Solution(name, path, flag), op_(op)
	{
		getDescriptionHandler().assign("����Ƶ�����ͨ/�����˲�");
	}
	~FrequencyDomainFilterBand() override {}

	void operator()()override;
private:
	int op_;
};

}//!namespace digital


#endif // !_SOLUTION_FREQUENCY_DOMAIN_FILTER_BAND_H_
