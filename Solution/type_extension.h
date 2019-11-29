//brief:提供基本类型和数据的扩展

#ifndef _SOLUTION_TYPES_EXTENSION_H_
#define _SOLUTION_TYPES_EXTENSION_H_

#include<string.h>
#include<string>

#include<assert.h>

using std::string;

namespace digital {

enum ErrorCode {
	kNone,
	kFileError,
	kParameterNotMatch,
	kFatal,
	kFatalSys,
	kCodeSize,
};

const double k_PI = 3.141592653;

//brief:处理出错信息
void dealException(ErrorCode code);

}

#endif
