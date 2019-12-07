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
const double k_EPS = 1e-6;

enum {
	BORDER_DEFAULT = 4,
};

//brief:处理出错信息
#define dealException(x) digital::__dealException(x,__FILE__, __LINE__, __FUNCTION__)
void __dealException(ErrorCode code, const char* file, int line, const char* func);

//brief:输出提示信息
#define coutInfo(x) digital::__coutInfo(x,__FILE__, __LINE__, __FUNCTION__)
inline void __coutInfo(const char* str, const char* file, int line, const char* func) {
	fprintf(stderr, "[%s-%d-%s]:%s", file, line, func, str);
}

//brief:该函数仅为了调试方便，release时不会出现
#ifndef NDEBUG
template<typename... Args>
void __printInfo(const Args&... args) {
	fprintf(stderr, args...);
}
#endif

}

#endif
