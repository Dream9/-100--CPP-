#ifndef _SOLUTION_THIN_H_
#define _SOLUTION_THIN_H_

#include"Solution/solution.h"

namespace digital {

//brief:
class Thin : public Solution {
public :
	Thin(const string& path, bool flag = false, const string& name = "Thin")
		:Solution(name, path, flag)
	{
		getDescriptionHandler().assign("对轮廓线进行细化");
	}
	~Thin()override {}

	void operator()()override;
};

}//!namespace digital

#endif // !_SOLUTION_THIN_H_
