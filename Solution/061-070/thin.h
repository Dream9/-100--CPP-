#ifndef _SOLUTION_THIN_H_
#define _SOLUTION_THIN_H_

#include"Solution/solution.h"

namespace digital {

//brief:
class Thin : public Solution {
public :
	//ThinType:
	enum {
		Hilditch = 1,
		ZhangSuen =2,
	};

	Thin(const string& path, int op = Hilditch, bool flag = false, const string& name = "Thin")
		:Solution(name, path, flag),op_(op)
	{
		getDescriptionHandler().assign("对轮廓线进行细化");
	}
	~Thin()override {}

	void operator()()override;
private:
	int op_;
};

}//!namespace digital

#endif // !_SOLUTION_THIN_H_
