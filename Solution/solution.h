#ifndef _SOLUTION_H_
#define _SOLUTION_H_

#include"Solution/type_extension.h"

namespace digital {

//brief:��������ͨ�ýӿ�
class Solution {
public:
	Solution(const string& name, const string& path, bool flag)
		: question_name_(name),
		path_(path), 
		show_old_(flag),
		description_()
	{
		assert(!name.empty());
		assert(!path.empty());
	}

	virtual ~Solution() {
		;
	}
	virtual void operator()() = 0;

	string getDescription()const {
		return description_;
	}
	string& getDescriptionHandler() {
		return description_;
	}

	const string& getPath()const {
		return path_;
	}
	const string& getName()const {
		return question_name_;
	}
	
	void show(void *);
	void show(void *, void *);

	bool needShowOriginal()const {
		return show_old_;
	}
	

private:

	string question_name_;
	string path_;
	bool show_old_;
	
	string description_;
};

//ͳһ�Ľӿ�
void Solve(Solution& solution);
//brief:�����ڲ���
void __MatrixTest(void* Mat1, void* Mat2);

}


#endif
