// ◇
// fds: fdx68 selector
// FDSConfig: コンフィグ設定
// Copyright: (C)2020 Hiroaki GOTO as GORRY.
// License: see readme.txt
// =====================================================================

#if !defined(__FDSCONFIG_H__)
#define __FDSCONFIG_H__

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

#include "FDSMacro.h"
#include "IniFile.h"
#include "FDSDrive.h"
#include "FDSMachine.h"
#include "WStrUtil.h"
#include "Log.h"

// =====================================================================
// FDS コンフィグ設定
// =====================================================================

class FDSConfig
{
public:
	FDSConfig() {
		mDriveNo = 0;
		mMachineNo = 0;
	}
	virtual ~FDSConfig() {}

	int load(const std::string& filename);

	const std::string& cmdDir() const { return mCmdDir; }
	const std::string& fddEmuCmd() const { return mFddEmuCmd; }
	const std::string& fdxToolCmd() const { return mFdxToolCmd; }
	const std::string& fdDumpCmd() const { return mFdDumpCmd; }
	const std::string& fdRestoreCmd() const { return mFdRestoreCmd; }

	const std::string& drivesDir() const { return mDrivesDir; }
	int numDrives() const { return mVecDrives.size(); }
	const FDSDrive& cfgDrive() const { return mVecDrives[mDriveNo]; }
	const FDSDrive& cfgDrive(int num) const { return mVecDrives[num]; }
	FDSDrive& cfgDriveW() { return mVecDrives[mDriveNo]; }
	FDSDrive& cfgDriveW(int num) { return mVecDrives[num]; }
	int driveNo() const { return mDriveNo; }
	void setDriveNo(int num) { mDriveNo = num; }

	const std::string& machinesDir() const { return mMachinesDir; }
	int numMachines() const { return mVecMachines.size(); }
	const FDSMachine& cfgMachine() const { return mVecMachines[mMachineNo]; }
	const FDSMachine& cfgMachine(int num) const { return mVecMachines[num]; }
	FDSMachine& cfgMachineW() { return mVecMachines[mMachineNo]; }
	FDSMachine& cfgMachineW(int num) { return mVecMachines[num]; }
	int machineNo() const { return mMachineNo; }
	void setMachineNo(int num) { mMachineNo = num; }

	std::string makeDumpOpt(int machineno, int driveno, int dumpno) const;
	std::string makeDumpOpt(int dumpno) const { return makeDumpOpt(machineNo(), driveNo(), dumpno); }
	std::string makeRestoreOpt(int machineno, int driveno, int restoreno) const;
	std::string makeRestoreOpt(int restoreno) const { return makeRestoreOpt(machineNo(), driveNo(), restoreno); }

private:
	std::string mCmdDir;
	std::string mFddEmuCmd;
	std::string mFdxToolCmd;
	std::string mFdDumpCmd;
	std::string mFdRestoreCmd;

	IniFile mIniFile;

	std::string mDrivesDir;
	std::vector<FDSDrive> mVecDrives;
	int mDriveNo;

	std::string mMachinesDir;
	std::vector<FDSMachine> mVecMachines;
	int mMachineNo;

};


#endif  // __FDSCONFIG_H__
// =====================================================================
// [EOF]
