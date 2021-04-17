// ◇
// fds: fdx68 selector
// FdxConv: FdxConvの操作
// Copyright: (C)2020 Hiroaki GOTO as GORRY.
// License: see readme.txt
// =====================================================================

#include "FdxConv.h"

// =====================================================================
// FdxConvの操作
// =====================================================================

#define FDXCONV_LOGFILENAME "fdxconv.log"

// -------------------------------------------------------------
// コンストラクタ
// -------------------------------------------------------------
FdxConv::FdxConv()
{
}

// -------------------------------------------------------------
// 外部コマンドの実行
// -------------------------------------------------------------
bool
FdxConv::execCmd(const std::string& cmd, const std::string& option)
{
	// 外部コマンドを起動
	std::string cmdline = cmd + " " + option;
	cmdline += ">>" FDXCONV_LOGFILENAME " 2>&1";
	FDS_LOG("execCmd: [%s]\n", cmdline.c_str());
	int exitcode = system(cmdline.c_str());
	FDS_LOG("execCmd: exitcode=%d\n", exitcode);

	return (exitcode == 0);
}

// =====================================================================
// [EOF]
