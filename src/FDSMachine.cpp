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
		char section[FDX_FILENAME_MAX];
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
		char section[FDX_FILENAME_MAX];
		sprintf(section, "FORMAT-%d", i);
		FDSMachineFormat& format = mVecFormat[i-1];
		format.mName = mIniFile.getString(section, "NAME");
		format.mFdxToolOpt = mIniFile.getString(section, "FDXTOOLOPT");
		format.mFileName = mIniFile.getString(section, "FILENAME");
	}

	// [DUMP-n]セクション読み込み
	for (num=1; ; num++) {
		char section[FDX_FILENAME_MAX];
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
		char section[FDX_FILENAME_MAX];
		sprintf(section, "DUMP-%d", i);
		FDSMachineDump& dump = mVecDump[i-1];
		dump.mName = mIniFile.getString(section, "NAME");
		dump.mType = mIniFile.getString(section, "TYPE");
		dump.mFormat = mIniFile.getString(section, "FORMAT");
		dump.mCylinders = mIniFile.getInt(section, "CYLINDERS");
		dump.mHeads = mIniFile.getInt(section, "HEADS");
		dump.mSteps = mIniFile.getInt(section, "STEPS");
		dump.mFileName = mIniFile.getString(section, "FILENAME");
	}

	// [RESTORE-n]セクション読み込み
	for (num=1; ; num++) {
		char section[FDX_FILENAME_MAX];
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
		char section[FDX_FILENAME_MAX];
		sprintf(section, "RESTORE-%d", i);
		FDSMachineRestore& restore = mVecRestore[i-1];
		restore.mName = mIniFile.getString(section, "NAME");
		restore.mType = mIniFile.getString(section, "TYPE");
		restore.mCylinders = mIniFile.getInt(section, "CYLINDERS");
		restore.mHeads = mIniFile.getInt(section, "HEADS");
		restore.mSteps = mIniFile.getInt(section, "STEPS");
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


// -------------------------------------------------------------
// TypeからDUMPエントリを求める
// -------------------------------------------------------------
int 
FDSMachine::findDumpNoByType(const std::string& type) const
{
	for (int i=0; i<(int)mVecDump.size(); i++) {
		const std::string& listtype = mVecDump[i].type();
		if (std::string::npos != listtype.find(type)) {
			return i;
		}
		std::string listtype2 = "3.5-"+listtype;
		if (std::string::npos != listtype2.find(type)) {
			return i;
		}
		listtype2 = "5.25-"+listtype;
		if (std::string::npos != listtype2.find(type)) {
			return i;
		}
		listtype2 = "8-"+listtype;
		if (std::string::npos != listtype2.find(type)) {
			return i;
		}
	}
	return -1;
}

// -------------------------------------------------------------
// TypeからDUMPエントリを設定する
// -------------------------------------------------------------
int 
FDSMachine::setDumpNoByType(const std::string& type)
{
	int no = findDumpNoByType(type);
	if (no < 0) {
		return no;
	}
	mDumpNo = no;
	return 0;
}

// -------------------------------------------------------------
// TypeからRESTOREエントリを求める
// -------------------------------------------------------------
int 
FDSMachine::findRestoreNoByType(const std::string& type) const
{
	for (int i=0; i< (int)mVecRestore.size(); i++) {
		const std::string& listtype = mVecRestore[i].type();
		if (std::string::npos != listtype.find(type)) {
			return i;
		}
		std::string listtype2 = "3.5-"+listtype;
		if (std::string::npos != listtype2.find(type)) {
			return i;
		}
		listtype2 = "5.25-"+listtype;
		if (std::string::npos != listtype2.find(type)) {
			return i;
		}
		listtype2 = "8-"+listtype;
		if (std::string::npos != listtype2.find(type)) {
			return i;
		}
	}
	return -1;
}

// -------------------------------------------------------------
// TypeからRESTOREエントリを設定する
// -------------------------------------------------------------
int 
FDSMachine::setRestoreNoByType(const std::string& type)
{
	int no = findRestoreNoByType(type);
	if (no < 0) {
		return no;
	}
	mRestoreNo = no;
	return 0;
}


// [EOF]
