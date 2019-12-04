#ifndef _SOLUTION_EMBOSS_FILTER_H_
#define _SOLUTION_EMBOSS_FILTER_H_

#include"Solution/solution.h"

namespace digital {

//brief:原理就是将soble计算结果的负值当成浮雕的阴影，正值当成浮雕的光线照射
class EmbossFilter : public Solution {
public:
	EmbossFilter(const string& path, bool flag = false, const string& name = "EmbossFilter")
		:Solution(name, path, flag)
	{
		getDescriptionHandler().assign("通过叠加Soble边缘得到浮雕效果");
	}
	~EmbossFilter()override {}

	void operator()()override;

private:
	static const int kWin;
};
}

#endif // !_SOLUTION_EMBOSS_FILTER_H_
