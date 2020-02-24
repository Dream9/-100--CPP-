#ifndef _SOLUTION_HOG_H_
#define _SOLUTION_HOG_H_

#include"Solution/solution.h"

namespace digital {
class Hog : public Solution {
public:
	Hog(const string& path, bool flag = false,
		const string& name = "Hog")
		:Solution(name, path, flag)
	{
		getDescriptionHandler().assign("Ã·»°HOGÃÿ’˜√Ë ˆ");
	}

	~Hog() override {}

	void operator()()override;
};

}//!namespace digital


#endif // _SOLUTION_HOG_H_
