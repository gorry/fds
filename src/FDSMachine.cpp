// ◇
// fds: fdx68 selector
// FDSMachine: マシン設定
// Copyright: (C)2020 Hiroaki GOTO as GORRY.
// License: see readme.txt
// =====================================================================

#include "FDSMachine.h"

// =====================================================================
// FDS マシン設定
// =====================================================================

// -------------------------------------------------------------
// INIファイル読み込み
// -------------------------------------------------------------
int
FDSMachine::load(const std::string& filename)
{
	int num;

	// マシンINIファイルを読む
	FDS_LOG("load %s\n", filename.c_str());
	int ret = mIniFile.load(filename);
	if (ret < 0) {
		FDS_ERROR("failed: load %s\n", filename.c_str());
		fprintf(stderr, "[%s] not found.\n", filename.c_str());
		return ret;
	}

	// [SYSTEM]セクション読み込み
	mName = mIniFile.getString("SYSTEM", "NAME");
	mRootDir = mIniFile.getString("SYSTEM", "ROOTDIR");
	mFddEmuOpt = mIniFile.getString("SYSTEM", "FDDEMUOPT");
	mDriveName[0] = mIniFile.getString("SYSTEM", "DRIVE0NAME");
	mDriveName[1] = mIniFile.getString("SYSTEM", "DRIVE1NAME");

	// [FORMAT-n]セクション読み込み
	for (num=1; ; num++) {
		char section[16];
		sprintf(section, "FORMAT-%d", num);
		if (!mIniFile.hasKey(section, "NAME")) {
			break;
		}
	}
	if (num < 2) {
		FDS_ERROR("failed: no [FORMAT-n] section in file %s\n", filename.c_str());
		fprintf(stderr, "No [FORMAT-n] section in file %s\n", filename.c_str());
		return -1;
	}
	mVecFormat.resize(num-1);
	for (int i=1; i<num; i++) {
		char section[16];
		sprintf(section, "FORMAT-%d", i);
		FDSMachineFormat& format = mVecFormat[i-1];
		format.mName = mIniFile.getString(section, "NAME");
		format.mFdxToolOpt = mIniFile.getString(section, "FDXTOOLOPT");
		format.mFileName = mIniFile.getString(section, "FILENAME");
	}

	// [DUMP-n]セクション読み込み
	for (num=1; ; num++) {
		char section[16];
		sprintf(section, "DUMP-%d", num);
		if (!mIniFile.hasKey(section, "NAME")) {
			break;
		}
	}
	if (num < 2) {
		FDS_ERROR("failed: no [DUMP-n] section in file %s\n", filename.c_str());
		fprintf(stderr, "No [DUMP-n] section in file %s\n", filename.c_str());
		return -1;
	}
	mVecDump.resize(num-1);
	for (int i=1; i<num; i++) {
		char section[16];
		sprintf(section, "DUMP-%d", i);
		FDSMachineDump& dump = mVecDump[i-1];
		dump.mName = mIniFile.getString(section, "NAME");
		dump.mType = mIniFile.getString(section, "TYPE");
		dump.mFormat = mIniFile.getString(section, "FORMAT");
		dump.mCylinders = mIniFile.getInt(section, "CYLINDERS");
		dump.mFileName = mIniFile.getString(section, "FILENAME");
	}

	// [RESTORE-n]セクション読み込み
	for (num=1; ; num++) {
		char section[16];
		sprintf(section, "RESTORE-%d", num);
		if (!mIniFile.hasKey(section, "NAME")) {
			break;
		}
	}
	if (num < 2) {
		FDS_ERROR("failed: no [RESTORE-n] section in file %s\n", filename.c_str());
		fprintf(stderr, "No [RESTORE-n] section in file %s\n", filename.c_str());
		return -1;
	}
	mVecRestore.resize(num-1);
	for (int i=1; i<num; i++) {
		char section[16];
		sprintf(section, "RESTORE-%d", i);
		FDSMachineRestore& restore = mVecRestore[i-1];
		restore.mName = mIniFile.getString(section, "NAME");
		restore.mType = mIniFile.getString(section, "TYPE");
	}

	return 0;
}

// -------------------------------------------------------------
// ファイル名
// -------------------------------------------------------------

std::string
FDSMachine::fileNameWithTime(const std::string fileName)
{
	char buf[FDX_STRING_MAX];
	time_t t;
	struct tm *tp;

	t = time(NULL);
	tp = localtime(&t);
	strftime(buf, sizeof(buf), fileName.c_str(), tp);

	std::string str(buf);
	return str;
}


// [EOF]
