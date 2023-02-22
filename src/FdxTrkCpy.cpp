// ◇
// fds: fdx68 selector
// FdxTrkCpy: FdxTrkCpyの操作
// Copyright: (C)2020 Hiroaki GOTO as GORRY.
// License: see readme.txt
// =====================================================================

#include "FdxTrkCpy.h"

// =====================================================================
// FdxTrkCpyの操作
// =====================================================================

#define FDXTRKCPY_LOGFILENAME "fdxtrkcpy.log"

// -------------------------------------------------------------
// コンストラクタ
// -------------------------------------------------------------
FdxTrkCpy::FdxTrkCpy()
{
}

// -------------------------------------------------------------
// 外部コマンドの実行
// -------------------------------------------------------------
bool
FdxTrkCpy::execCmd(const std::string& cmd, const std::string& option)
{
	// 外部コマンドを起動
	std::string cmdline = cmd + " " + option;
	cmdline += ">>" FDXTRKCPY_LOGFILENAME " 2>&1";
	FDS_LOG("execCmd: [%s]\n", cmdline.c_str());
	int exitcode = system(cmdline.c_str());
	FDS_LOG("execCmd: exitcode=%d\n", exitcode);

	return (exitcode == 0);
}

// =====================================================================
// [EOF]
