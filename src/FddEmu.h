﻿// ◇
// fds: fdx68 selector
// FddEmu: FddEmuの操作
// Copyright: (C)2020 Hiroaki GOTO as GORRY.
// License: see readme.txt

#if !defined(__FDDEMU_H__)
#define __FDDEMU_H__

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
#include "MakeArgv.h"

// FddEmuのコマンドポート
#define FDDEMU_PORTNO 6869

// =====================================================================
// FddEmuの操作
// =====================================================================

class FddEmu
{
public:		// struct, enum
	struct Status {
		int mId = 0;
		bool mInsert = false;
		bool mProtect = false;
		int mCluster = 0;
		std::string mFileName;
	};

	enum class Command : int {
		List = 0,
		Insert,
		Eject,
		Protect,
	};

	enum class ErrNo : int {
		None = 0,
		NotFork,
		NotConnect,
		Max,
	};

	static const int Drives = 2;

public:		// function
	FddEmu();
	virtual ~FddEmu() {}

	void setNoRoot(bool sw);
	void setCmd(const std::string& cmd);
	void setOption(const std::string& option);
	int run();
	void kill();
	bool updateStatus();
	const Status& getStatus(int id) const { return mStatus[id]; }
	bool setImage(int id, const std::string& filename);
	bool ejectDrive(int id);
	bool protectDrive(int id);
	bool execCmd(const std::string& cmd, const std::string& option);
	ErrNo getErrNo() const { return mErrNo; }

private:	// function
	bool sendCommand(const std::string& command, bool getStatus);

public:		// var

private:	// var
	std::string mCmd;
	std::string mOption;
	std::vector<std::string> mArgv;
#if !defined(FDS_WINDOWS)
	pid_t mPid = 0;
#endif
	Status mStatus[2] = {};
	std::string mDiskPath[2];
	ErrNo mErrNo = ErrNo::None;
	bool mNoRoot = false;

};

#endif  // __FDDEMU_H__
// =====================================================================
// [EOF]
