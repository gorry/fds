// ◇
// fds: fdx68 selector
// Log: ログ出力
// Copyright: (C)2020 Hiroaki GOTO as GORRY.
// License: see readme.txt
// =====================================================================

#include "Log.h"

// =====================================================================
// ログ出力
// =====================================================================

bool FDSLog::sOutputLog;

// -------------------------------------------------------------
// ログ出力
// -------------------------------------------------------------
void
FDSLog::log(const char* format, ...)
{
	va_list arg;
	va_start(arg, format);
	if (sOutputLog) {
		FILE* fout = fopen(FDS_LOGFILENAME, "a+");
		if (fout) {
			vfprintf(fout, format, arg);
			fclose(fout);
		}
	}
	va_end(arg);
}


// =====================================================================
// [EOF]
