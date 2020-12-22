// ◇
// fds: fdx68 selector
// FDSDrive: ドライブ設定
// Copyright: (C)2020 Hiroaki GOTO as GORRY.
// License: see readme.txt
// =====================================================================

#include "FDSDrive.h"

// =====================================================================
// FDS ドライブ設定
// =====================================================================

// -------------------------------------------------------------
// INIファイル読み込み
// -------------------------------------------------------------
int
FDSDrive::load(const std::string& filename)
{
	int num;

	// ドライブINIファイルを読む
	FDS_LOG("load %s\n", filename.c_str());
	int ret = mIniFile.load(filename);
	if (ret < 0) {
		FDS_ERROR("failed: load %s\n", filename.c_str());
		fprintf(stderr, "[%s] not found.\n", filename.c_str());
		return ret;
	}

	// [SYSTEM]セクション読み込み
	mName = mIniFile.getString("SYSTEM", "NAME");
	mId = mIniFile.getInt("SYSTEM", "ID");
	mRetry = mIniFile.getInt("SYSTEM", "RETRY");

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
		FDSDriveDump& dump = mVecDump[i-1];
		dump.mName = mIniFile.getString(section, "NAME");
		dump.mType = mIniFile.getString(section, "TYPE");
		dump.mFdDumpOpt = mIniFile.getString(section, "FDDUMPOPT");
		dump.mCylinders = mIniFile.getInt(section, "CYLINDERS");
		dump.mHeads = mIniFile.getInt(section, "HEADS");
		dump.mSteps = mIniFile.getInt(section, "STEPS");
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
		FDSDriveRestore& restore = mVecRestore[i-1];
		restore.mName = mIniFile.getString(section, "NAME");
		restore.mType = mIniFile.getString(section, "TYPE");
		restore.mFdRestoreOpt = mIniFile.getString(section, "FDRESTOREOPT");
		restore.mCylinders = mIniFile.getInt(section, "CYLINDERS");
		restore.mHeads = mIniFile.getInt(section, "HEADS");
		restore.mSteps = mIniFile.getInt(section, "STEPS");
	}

	return 0;
}

// -------------------------------------------------------------
// TypeからDUMPエントリを求める
// -------------------------------------------------------------
int 
FDSDrive::findDumpNoByType(const std::string& type) const
{
	for (int i=0; i<(int)mVecDump.size(); i++) {
		if (std::string::npos != mVecDump[i].type().find(type)) {
			return i;
		}
	}
	return -1;
}

// -------------------------------------------------------------
// TypeからDUMPエントリを設定する
// -------------------------------------------------------------
int 
FDSDrive::setDumpNoByType(const std::string& type)
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
FDSDrive::findRestoreNoByType(const std::string& type) const
{
	for (int i=0; i< (int)mVecRestore.size(); i++) {
		if (std::string::npos != mVecRestore[i].type().find(type)) {
			return i;
		}
	}
	return -1;
}

// -------------------------------------------------------------
// TypeからRESTOREエントリを設定する
// -------------------------------------------------------------
int 
FDSDrive::setRestoreNoByType(const std::string& type)
{
	int no = findRestoreNoByType(type);
	if (no < 0) {
		return no;
	}
	mRestoreNo = no;
	return 0;
}


// [EOF]
