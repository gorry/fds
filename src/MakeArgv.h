// ◇
// fds: fdx68 selector
// MakeArgv: Argvの作成
// Copyright: (C)2020 Hiroaki GOTO as GORRY.
// License: see readme.txt

#if !defined(__MAKEARGV_H__)
#define __MAKEARGV_H__

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include <vector>
#include <string>

// =====================================================================
// FddEmuの操作
// =====================================================================

std::vector<const char*> makeArgv(const std::string& cmd, const std::string& option, std::vector<std::string>& retArgs);

#endif  // __MAKEARGV_H__
// =====================================================================
// [EOF]
