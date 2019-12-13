#ifndef _SOLUTION_AFFINE_TRANSFORMATION_H_
#define _SOLUTION_AFFINE_TRANSFORMATION_H_

#include"Solution/solution.h"

namespace digital {

//brief:Ĭ�ϲ���˫���β�ֵ�ķ�ʽ����ͼ��ı任
//becare:����NO_028,NO_029,NO_030,NO_031����Ŀ���Ƕ�warpAffine�Ĳ�ͬ����Ĳ��ԣ����ͳһ�ڴ����
class AffineTransformation : public Solution {
public:
	//��ʶ���еĲ���
	enum {
		TRANSLATION = 0x1,  //ƽ��
		SCALE = 0x1 << 1,   //����
		ROTATION = 0x1 << 2,//��ת
		LEAN = 0x1 << 3,    //��б
	};

	//parameter: opָ�����ԵĲ���
	AffineTransformation(const string& path, int op, bool flag = false, const string& name = "AffineTransformation")
		:Solution(name, path, flag),ops_(op)
	{
		getDescriptionHandler().assign("��ͼ��������·���任��ƽ�ƣ����ţ���ת");
	}

	~AffineTransformation()override {}

	void operator()()override;

private:

	int ops_;
};

}

#endif // !_SOLUTION_AFFINE_TRANSFORMATION_H_
