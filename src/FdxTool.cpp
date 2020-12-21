// ◇
// fds: fdx68 selector
// FdxTool: FdxToolの操作
// Copyright: (C)2020 Hiroaki GOTO as GORRY.
// License: see readme.txt
// =====================================================================

#include "FdxTool.h"

// =====================================================================
// FdxToolの操作
// =====================================================================

#define FDXTOOL_LOGFILENAME "fdxtool.log"

// -------------------------------------------------------------
// コンストラクタ
// -------------------------------------------------------------
FdxTool::FdxTool()
{
}

// -------------------------------------------------------------
// 外部コマンドの実行
// -------------------------------------------------------------
bool
FdxTool::execCmd(const std::string& cmd, const std::string& option)
{
	// 外部コマンドを起動
	std::string cmdline = cmd + " " + option;
	cmdline += ">>" FDXTOOL_LOGFILENAME " 2>&1";
	FDS_LOG("execCmd: [%s]\n", cmdline.c_str());
	int ret = system(cmdline.c_str());
	FDS_LOG("execCmd: ret=%d\n", ret);

	return ret;
}

// =====================================================================
// [EOF]
