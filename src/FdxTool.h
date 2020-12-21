// ◇
// fds: fdx68 selector
// FdxTool: FdxToolの操作
// Copyright: (C)2020 Hiroaki GOTO as GORRY.
// License: see readme.txt

#if !defined(__FDXTOOL_H__)
#define __FDXTOOL_H__

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>

#if !defined(FDS_WINDOWS)
#include <unistd.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif

#include <vector>
#include <string>

#include "WStrUtil.h"
#include "Log.h"

// FdxToolのコマンドポート
#define FDDEMU_PORTNO 6869

// =====================================================================
// FdxToolの操作
// =====================================================================

class FdxTool
{
public:		// struct, enum

public:		// function
	FdxTool();
	virtual ~FdxTool() {}

	bool execCmd(const std::string& cmd, const std::string& option);

private:	// function

public:		// var

private:	// var

};

#endif  // __FDXTOOL_H__
// =====================================================================
// [EOF]
