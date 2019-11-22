

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

//brief:���������Ϣ
void dealException(ErrorCode code);

}

#endif
