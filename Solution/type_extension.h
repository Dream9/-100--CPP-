//brief:提供基本类型和数据的扩展

#ifndef _SOLUTION_TYPES_EXTENSION_H_
#define _SOLUTION_TYPES_EXTENSION_H_

#include<string.h>
#include<string>

#include<assert.h>

using std::string;

namespace digital {

//brief:错误代码
enum ErrorCode {
	kNone,
	kFileError,
	kParameterNotMatch,
	kImshowTypeIsNotCV8U,
	kFatal,
	kFatalSys,
	kCodeSize,
};

const double k_PI = 3.141592653;

enum {
	BORDER_DEFAULT = 4,
};

//brief:处理出错信息
#define dealException(x) __dealException(x,__FILE__, __LINE__, __FUNCTION__)
void __dealException(ErrorCode code, const char* file, int line, const char* func);

}

#endif
