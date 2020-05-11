// ◇
// fds: fdx68 selector
// Log: ログ出力
// Copyright: (C)2020 Hiroaki GOTO as GORRY.
// License: see readme.txt
// =====================================================================

#if !defined(__FDSLOG_H__)
#define __FDSLOG_H__

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>

#define FDS_LOG(...) if (FDSLog::getOutputLog()) FDSLog::log(__VA_ARGS__)

#define FDS_ERROR(...) FDSLog::log(__VA_ARGS__)

#define FDS_LOGFILENAME "fds.log"


// =====================================================================
// ログ出力
// =====================================================================

class FDSLog {

public:
	static bool sOutputLog;

public:
	static void setOutputLog(bool sw) { sOutputLog = sw; }
	static bool getOutputLog() { return sOutputLog; }
	static void log(const char* format, ...);

};

#endif  // __FDSLOG_H__
// =====================================================================
// [EOF]
