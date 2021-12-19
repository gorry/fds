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
#else
#include <windows.h>
#include <io.h>
#endif

#include <vector>
#include <string>

#include "WStrUtil.h"
#include "FDXFile.h"
#include "FDXStatus.h"
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

	bool readFDXDiskInfo(const std::string& cmd, const std::string& filename);
	bool readFDXDiskInfoVerbose(const std::string& cmd, const std::string& filename);

	FdxDiskInfo& diskInfo(void) { return mDiskInfo; }
	FdxDiskInfo& diskInfoVerbose(void) { return mDiskInfoVerbose; }

	bool copyFDXDiskInfoTrackStatus(void);

private:	// function
	int readFDXDiskInfoHeader(FdxDiskInfo& diskinfo, char* s);
	int readFDXDiskInfoBody(FdxDiskInfo& diskinfo, char* s);
	bool execFdxToolAnalyze(char* tmpfilename, const std::string& cmd, const std::string& filename, const char* option);


public:		// var
	FdxDiskInfo mDiskInfo;
	FdxDiskInfo mDiskInfoVerbose;
	int mLastTrack;
	int mLastSector;

private:	// var

};

#endif  // __FDXTOOL_H__
// =====================================================================
// [EOF]
