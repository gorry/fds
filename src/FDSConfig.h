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

#include "FDSCommon.h"
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
	const std::string& fdxViewCmd() const { return mFdxViewCmd; }
	const std::string& fdDumpCmd() const { return mFdDumpCmd; }
	const std::string& fdRestoreCmd() const { return mFdRestoreCmd; }
	const std::string& fdxConvCmd() const { return mFdxConvCmd; }

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

	const std::string& cfgOtherImageExt(int num) const { return mVecOtherImages[num]; }
	const std::vector<std::string>& cfgOtherImageExts() const { return mVecOtherImages; }
	int numOtherImages() const { return mVecOtherImages.size(); }
	int fromOtherImageNo() const { return mFromOtherImageNo; }
	void setFromOtherImageNo(int num) { mFromOtherImageNo = num; }
	int toOtherImageNo() const { return mToOtherImageNo; }
	void setToOtherImageNo(int num) { mToOtherImageNo = num; }

	std::string makeDumpOpt(int machineno, int driveno, int dumpno) const;
	std::string makeDumpOpt(int dumpno) const { return makeDumpOpt(machineNo(), driveNo(), dumpno); }
	std::string makeDumpTrackOpt(int machineno, int driveno, int dumpno, int trackno, int retrycount) const;
	std::string makeDumpTrackOpt(int dumpno, int trackno, int retrycount) const { return makeDumpTrackOpt(machineNo(), driveNo(), dumpno, trackno, retrycount); }
	std::string makeRestoreOpt(int machineno, int driveno) const;
	std::string makeRestoreOpt() const { return makeRestoreOpt(machineNo(), driveNo()); }
	std::string makeRestoreTrackOpt(int machineno, int driveno, int trackno) const;
	std::string makeRestoreTrackOpt(int trackno) const { return makeRestoreTrackOpt(machineNo(), driveNo(), trackno); }

private:
	std::string mCmdDir;
	std::string mFddEmuCmd;
	std::string mFdxConvCmd;
	std::string mFdxToolCmd;
	std::string mFdxViewCmd;
	std::string mFdDumpCmd;
	std::string mFdRestoreCmd;

	IniFile mIniFile;

	std::string mDrivesDir;
	std::vector<FDSDrive> mVecDrives;
	int mDriveNo;

	std::string mMachinesDir;
	std::vector<FDSMachine> mVecMachines;
	int mMachineNo;

	std::vector<std::string> mVecOtherImages;
	int mFromOtherImageNo;
	int mToOtherImageNo;

};


#endif  // __FDSCONFIG_H__
// =====================================================================
// [EOF]
