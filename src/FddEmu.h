// ◇
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

// FddEmuのコマンドポート
#define FDDEMU_PORTNO 6869

// =====================================================================
// FddEmuの操作
// =====================================================================

class FddEmu
{
public:		// struct, enum
	struct status {
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
	void setFddEmuCmd(const std::string& cmd);
	void setFddEmuOption(const std::string& option);
	int run();
	void kill();
	bool updateStatus();
	const status& getStatus(int id) const { return mStatus[id]; }
	bool setImage(int id, const std::string& filename);
	bool ejectDrive(int id);
	bool protectDrive(int id);
	bool execCmd(const std::string& cmd, const std::string& option);
	ErrNo getErrNo() const { return mErrNo; }

private:	// function
	std::vector<const char*> makeArgv(const std::string& cmd, const std::string& option);
	bool sendCommand(const std::string& command, bool getStatus);

public:		// var

private:	// var
	std::string mFddEmuCmd;
	std::string mFddEmuOption;
	std::vector<std::string> mArgv;
#if !defined(FDS_WINDOWS)
	pid_t mPid = 0;
#endif
	status mStatus[2] = {};
	std::string mDiskPath[2];
	ErrNo mErrNo = ErrNo::None;
	bool mNoRoot = false;

};

#endif  // __FDDEMU_H__
// =====================================================================
// [EOF]
