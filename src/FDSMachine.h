// ◇
// fds: fdx68 selector
// FDSMachine: マシン設定
// Copyright: (C)2020 Hiroaki GOTO as GORRY.
// License: see readme.txt
// =====================================================================

#if !defined(__FDSMACHINE_H__)
#define __FDSMACHINE_H__

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <locale.h>
#include <wchar.h>
#include <string.h>
#include <time.h>

#include <string>
#include <vector>
#include <algorithm>
#include <array>

#include "FDSCommon.h"
#include "FDSMacro.h"
#include "IniFile.h"
#include "FDSDrive.h"
#include "WStrUtil.h"
#include "Log.h"

// =====================================================================
// FDS マシン設定
// =====================================================================

class FDSMachineFormat
{
	friend class FDSMachine;

public:
	FDSMachineFormat() {}
	virtual ~FDSMachineFormat() {}

	const std::string& name() const { return mName; }
	const std::string& fdxToolOpt() const { return mFdxToolOpt; }
	const std::string& fileName() const{ return mFileName; }

private:
	std::string mName;
	std::string mFdxToolOpt;
	std::string mFileName;
};

class FDSMachineDump
{
	friend class FDSMachine;

public:
	FDSMachineDump() {
		mCylinders = 0;
		mHeads = 0;
		mSteps = 0;
	}
	virtual ~FDSMachineDump() {}

	const std::string& name() const { return mName; }
	const std::string& type() const { return mType; }
	const std::string& format() const { return mFormat; }
	int cylinders() const { return mCylinders; }
	int heads() const { return mHeads; }
	int steps() const { return mSteps; }
	const std::string& fileName() const{ return mFileName; }

private:
	std::string mName;
	std::string mType;
	std::string mFormat;
	int mCylinders;
	int mHeads;
	int mSteps;
	std::string mFileName;
};

class FDSMachineRestore
{
	friend class FDSMachine;

public:
	FDSMachineRestore() {
		mCylinders = 0;
		mHeads = 0;
		mSteps = 0;
	}
	virtual ~FDSMachineRestore() {}

	const std::string& name() const { return mName; }
	const std::string& type() const { return mType; }

	void setName(const std::string& name) { mName = name; }
	void setType(const std::string& type) { mType = type; }
	int cylinders() const { return mCylinders; }
	int heads() const { return mHeads; }
	int steps() const { return mSteps; }

private:
	std::string mName;
	std::string mType;
	int mCylinders;
	int mHeads;
	int mSteps;
};

class FDSMachine
{
public:
	FDSMachine() {
		mFormatNo = 0;
		mDumpNo = 0;
		mRestoreNo = 0;
	}
	virtual ~FDSMachine() {}

	int load(const std::string& filename);

	const std::string& name() const { return mName; }
	const std::string& rootDir() const { return mRootDir; }
	const std::string& fddEmuOpt() const { return mFddEmuOpt; }
	const std::string& driveName(int id) const { return mDriveName[id]; }

	int numFormat() const { return mVecFormat.size(); }
	const FDSMachineFormat& format(int num) const { return mVecFormat[num]; }
	const FDSMachineFormat& format() const { return format(mFormatNo); }
	int formatNo() const { return mFormatNo; }
	void setFormatNo(int num) { mFormatNo = num; }

	int numDump() const { return mVecDump.size(); }
	int findDumpNoByType(const std::string& type) const;
	const FDSMachineDump& dump(int num) const { return mVecDump[num]; }
	const FDSMachineDump& dump() const { return dump(mDumpNo); }
	int dumpNo() const { return mDumpNo; }
	void setDumpNo(int num) { mDumpNo = num; }
	int setDumpNoByType(const std::string& type);

	int numRestore() const { return mVecRestore.size(); }
	int findRestoreNoByType(const std::string& type) const;
	const FDSMachineRestore& restore(int num) const { return mVecRestore[num]; }
	const FDSMachineRestore& restore() const { return restore(mRestoreNo); }
	int restoreNo() const { return mRestoreNo; }
	void setRestoreNo(int num) { mRestoreNo = num; }
	int setRestoreNoByType(const std::string& type);

	static std::string fileNameWithTime(const std::string fileName);

private:
	IniFile mIniFile;

	std::string mName;
	std::string mRootDir;
	std::string mFddEmuOpt;
	std::string mDriveName[2];

	std::vector<FDSMachineFormat> mVecFormat;
	int mFormatNo;
	std::vector<FDSMachineDump> mVecDump;
	int mDumpNo;
	std::vector<FDSMachineRestore> mVecRestore;
	int mRestoreNo;
};


#endif  // __FDSMACHINE_H__
// =====================================================================
// [EOF]
