// ◇
// fds: fdx68 selector
// FDSDrive: ドライブ設定
// Copyright: (C)2020 Hiroaki GOTO as GORRY.
// License: see readme.txt
// =====================================================================

#if !defined(__FDSDRIVE_H__)
#define __FDSDRIVE_H__

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <locale.h>
#include <wchar.h>
#include <string.h>

#include <string>
#include <vector>
#include <algorithm>
#include <array>

#include "FDSCommon.h"
#include "FDSMacro.h"
#include "IniFile.h"
#include "WStrUtil.h"
#include "Log.h"

// =====================================================================
// FDS ドライブ設定
// =====================================================================

class FDSDriveDump
{
	friend class FDSDrive;

public:
	FDSDriveDump() {
		mCylinders = 0;
		mHeads = 0;
		mSteps = 0;
	}
	virtual ~FDSDriveDump() {}

	const std::string& name() const { return mName; }
	const std::string& type() const { return mType; }
	const std::string& fdDumpOpt() const { return mFdDumpOpt; }
	int cylinders() const { return mCylinders; }
	int heads() const { return mHeads; }
	int steps() const { return mSteps; }

	void setName(const std::string& name) { mName = name; }
	void setType(const std::string& type) { mType = type; }
	void setFdDumpOpt(const std::string& opt) { mFdDumpOpt = opt; }

private:
	std::string mName;
	std::string mType;
	std::string mFdDumpOpt;
	int mCylinders;
	int mHeads;
	int mSteps;
};

class FDSDriveRestore
{
	friend class FDSDrive;

public:
	FDSDriveRestore() {
		mCylinders = 40;
		mHeads = 2;
		mSteps = 1;
	}
	virtual ~FDSDriveRestore() {}

	const std::string& name() const { return mName; }
	const std::string& type() const { return mType; }
	const std::string& fdRestoreOpt() const { return mFdRestoreOpt; }
	int cylinders() const { return mCylinders; }
	int heads() const { return mHeads; }
	int steps() const { return mSteps; }

	void setName(const std::string& name) { mName = name; }
	void setType(const std::string& type) { mType = type; }
	void setFdRestoreOpt(const std::string& opt) { mFdRestoreOpt = opt; }

private:
	std::string mName;
	std::string mType;
	std::string mFdRestoreOpt;
	int mCylinders;
	int mHeads;
	int mSteps;
};

class FDSDrive
{
public:
	FDSDrive() {
		mId = 0;
		mRetry = 3;
		mDumpNo = 0;
		mRestoreNo = 0;
	}
	virtual ~FDSDrive() {}

	int load(const std::string& filename);

	const std::string& name() const { return mName; }
	int id() const { return mId; }
	int retry() const { return mRetry; }

	int numDump() const { return mVecDump.size(); }
	int findDumpNoByType(const std::string& type) const;
	const FDSDriveDump& dump(int num) const { return mVecDump[num]; }
	const FDSDriveDump& dump() const { return dump(mDumpNo); }
	int dumpNo() const { return mDumpNo; }
	void setDumpNo(int num) { mDumpNo = num; }
	int setDumpNoByType(const std::string& type);

	int numRestore() const { return mVecRestore.size(); }
	int findRestoreNoByType(const std::string& type) const;
	const FDSDriveRestore& restore(int num) const { return mVecRestore[num]; }
	const FDSDriveRestore& restore() const { return restore(mRestoreNo); }
	int restoreNo() const { return mRestoreNo; }
	void setRestoreNo(int num) { mRestoreNo = num; }
	int setRestoreNoByType(const std::string& type);

private:
	IniFile mIniFile;

	std::string mName;
	int mId;
	int mRetry;

	std::vector<FDSDriveDump> mVecDump;
	int mDumpNo;

	std::vector<FDSDriveRestore> mVecRestore;
	int mRestoreNo;
};


#endif  // __FDSDRIVE_H__
// =====================================================================
// [EOF]
