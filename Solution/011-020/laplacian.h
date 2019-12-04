#ifndef _SOLUTION_LAPLACIAN_H_
#define _SOLUTION_LAPLACIAN_H_

#include"Solution/solution.h"

namespace digital {

//brief:
class Laplacian : public Solution {
public:
	Laplacian(const string& path, bool flag = false, const string& name = "Laplacian")
		:Solution(name,path,flag)
	{
		getDescriptionHandler().assign("使用Laplacian提取图像边界");
	}
	~Laplacian() override {}

	void operator()()override;

private:
	void laplacian_(void* src, void* dst, int ddepth, int bordertype = BORDER_DEFAULT);


};
}

#endif // !_SOLUTION_LAPLACIAN_H_
