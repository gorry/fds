// ◇
// fds: fdx68 selector
// FdxTrkCpy: FdxTrkCpyの操作
// Copyright: (C)2020 Hiroaki GOTO as GORRY.
// License: see readme.txt

#if !defined(__FDXTRKCPY_H__)
#define __FDXTRKCPY_H__

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

// =====================================================================
// FdxTrkCpyの操作
// =====================================================================

class FdxTrkCpy
{
public:		// struct, enum

public:		// function
	FdxTrkCpy();
	virtual ~FdxTrkCpy() {}

	bool execCmd(const std::string& cmd, const std::string& option);

private:	// function

public:		// var

private:	// var

};

#endif  // __FDXTRKCPY_H__
// =====================================================================
// [EOF]
