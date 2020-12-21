// ◇
// fds: fdx68 selector
// FdRestore: FdRestoreの操作
// Copyright: (C)2020 Hiroaki GOTO as GORRY.
// License: see readme.txt

#if !defined(__FDRESTORE_H__)
#define __FDRESTORE_H__

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
#include <fcntl.h>
#else
#include <windows.h>
#endif

#include <vector>
#include <string>

#include "WStrUtil.h"
#include "Log.h"
#include "MakeArgv.h"

// =====================================================================
// FdRestoreの操作
// =====================================================================

#define MAX_TRACKS 256

class FdRestore
{
public:		// struct, enum
	enum class TrackStatus : char {
		None = 0,
		Retry1 = 1,
		Retry2 = 2,
		Retry3 = 3,
		Retry4 = 4,
		Retry5 = 5,
		Retry6 = 6,
		Retry7 = 7,
		Retry8 = 8,
		Retry9 = 9,
		Finish = 100,
		Error = 101,
		Unformat = 102,
	};

	struct Status {
		int mTracks = 0;
		int mNowTrack = -1;
		bool mFinished = false;
		TrackStatus mStatus[MAX_TRACKS];
		char mChanged[MAX_TRACKS];
		TrackStatus mStatus_Backup[MAX_TRACKS];
		char mChanged_Backup[MAX_TRACKS];
		std::string mName;
		std::string mFormat;
	};

	enum class LogMode : int {
		None = 0,
		ESC1 = 1,
		ESC2 = 2,
		ESC3 = 3,
		CR = 4,
	};

	enum class ErrNo : int {
		None = 0,
		NotFork,
		NotConnect,
		Max,
	};

	static const int Drives = 2;

public:		// function
	FdRestore();
	virtual ~FdRestore() {}

	void setCmd(const std::string& cmd);
	void setOption(const std::string& option);
	void setAnalyzeCmd(const std::string& cmd);
	void setAnalyzeOption(const std::string& option);
	void setDiskName(const std::string& name);
	void setFormatName(const std::string& name);
	void setCylinders(int num);
	int run();
	void kill();
	const Status& getStatus() const { return mStatus; }
	ErrNo getErrNo() const { return mErrNo; }

	typedef int Callback(Status& st, void* param);
	void setCallback(Callback* func, void* param);

	int analyze();
	void revertAnalyzeStatus(void);

private:	// function
	bool recvStatus(void);
	bool updateStatus();
	void analyzeLogLine(void);
	void putLogLine(void);
	void clearLogLine(void);
	bool recvAnalyzeStatus(void);
	bool updateAnalyzeStatus();
	void analyzeAnalyzeLogLine(void);



public:		// var

private:	// var
	std::string mCmd;
	std::string mOption;
	std::vector<std::string> mArgv;
#if !defined(FDS_WINDOWS)
	pid_t mPid = 0;
	int mPipe[2];
#endif
	Status mStatus = {};
	ErrNo mErrNo = ErrNo::None;
	LogMode mLogMode;
	std::string mLineBuf;
	Callback* mCallbackFunc;
	void* mCallbackParam;
	std::string mAnalyzeCmd;
	std::string mAnalyzeOption;

};

#endif  // __FDRESTORE_H__
// =====================================================================
// [EOF]
