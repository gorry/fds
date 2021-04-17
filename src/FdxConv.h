// ◇
// fds: fdx68 selector
// FdxConv: FdxConvの操作
// Copyright: (C)2020 Hiroaki GOTO as GORRY.
// License: see readme.txt

#if !defined(__FDXCONV_H__)
#define __FDXCONV_H__

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
#else
#include <windows.h>
#include <io.h>
#endif

#include <vector>
#include <string>

#include "WStrUtil.h"
#include "FDXFile.h"
#include "Log.h"

// FdxConvのコマンドポート
#define FDDEMU_PORTNO 6869

// =====================================================================
// FdxConvの操作
// =====================================================================

class FdxConv
{
public:		// struct, enum

public:		// function
	FdxConv();
	virtual ~FdxConv() {}

	bool execCmd(const std::string& cmd, const std::string& option);

private:	// function

public:		// var

private:	// var

};

#endif  // __FDXCONV_H__
// =====================================================================
// [EOF]
