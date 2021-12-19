// ◇
// fds: fdx68 selector
// FdxView: FdxViewの操作
// Copyright: (C)2020 Hiroaki GOTO as GORRY.
// License: see readme.txt

#if !defined(__FDXVIEW_H__)
#define __FDXVIEW_H__

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
#include "FdxStatus.h"
#include "Log.h"

// =====================================================================
// FdxViewの操作
// =====================================================================

class FdxView
{
public:		// struct, enum

public:		// function
	FdxView() { Clear(); }
	virtual ~FdxView() {}
	FdxView(FdxView&& o) = default;
	void Clear(void) {
	}

	bool execCmd(const std::string& cmd, const std::string& option);

	bool readFDXDiskInfo(const std::string& cmd, const std::string& filename);
	bool readFDXTrack(const std::string& cmd, const std::string& filename, int trackno);
	bool readFDXSector(const std::string& cmd, const std::string& filename, int trackno, int sectorno);

	FdxDiskInfo& diskInfo(void) { return mDiskInfo; }
	bool isTrackReady(int trackno);
	bool ReqTrackReload(int trackno);
	FdxTrackInfo& Track(int trackno) { return mDiskInfo.Track(trackno); }
	FdxSectorInfo& Sector(int trackno, int sectorno) { return mDiskInfo.Track(trackno).Sector(sectorno); }
	bool isSectorReady(int trackno, int sectorno);
	bool ReqSectorReload(int trackno, int sectorno);

private:	// function
	int readFDXDiskInfoHeader(FdxDiskInfo& diskinfo, char* s);
	int readFDXDiskInfoBody(FdxDiskInfo& diskinfo, char* s);
	int readFDXDiskInfoDump(FdxDiskInfo& diskinfo, char* s);
	bool execFdxViewAnalyze(char* tmpfilename, const std::string& cmd, const std::string& filename, const char* option);


public:		// var
	FdxDiskInfo mDiskInfo;
	// DiskInfo mTrackInfo;

private:	// var

};

#endif  // __FDXVIEW_H__
// =====================================================================
// [EOF]
