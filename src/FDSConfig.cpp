// ◇
// fds: fdx68 selector
// FDSConfig: コンフィグ設定
// Copyright: (C)2020 Hiroaki GOTO as GORRY.
// License: see readme.txt
// =====================================================================

#include "FDSConfig.h"

// =====================================================================
// FDS コンフィグ設定
// =====================================================================

// -------------------------------------------------------------
// INIファイル読み込み
// -------------------------------------------------------------
int
FDSConfig::load(const std::string& filename)
{
	int num;

	// コンフィグINIファイルを読む
	FDS_LOG("load %s\n", filename.c_str());
	int ret = mIniFile.load(filename);
	if (ret < 0) {
		FDS_ERROR("failed: load %s\n", filename.c_str());
		fprintf(stderr, "[%s] not found. Please copy [fds2_sample.ini] to [%s].\n", filename.c_str(), filename.c_str());
		return ret;
	}

	// SYSTEMセクション読み込み
#if defined(FDS_WINDOWS)
	mCmdDir = mIniFile.getString("SYSTEM", "CMDDIRWIN", "fdxbinwin");
	std::string separator = "\\";
#else
	mCmdDir = mIniFile.getString("SYSTEM", "CMDDIR", "fdxbin");
	std::string separator = "/";
#endif
	mFddEmuCmd = mCmdDir + separator + mIniFile.getString("SYSTEM", "FDDEMUCMD", "fddemu");
	mFdxConvCmd = mCmdDir + separator + mIniFile.getString("SYSTEM", "FDXCONVCMD", "fdxconv");
	mFdxToolCmd = mCmdDir + separator + mIniFile.getString("SYSTEM", "FDXTOOLCMD", "fdxtool");
	mFdxTrkCpyCmd = mCmdDir + separator + mIniFile.getString("SYSTEM", "FDXTRKCPYCMD", "fdxtrkcpy");
	mFdxViewCmd = mCmdDir + separator + mIniFile.getString("SYSTEM", "FDXVIEWCMD", "fdxview");
	mFdDumpCmd = mCmdDir + separator + mIniFile.getString("SYSTEM", "FDDUMPCMD", "fddump");
	mFdRestoreCmd = mCmdDir + separator + mIniFile.getString("SYSTEM", "FDRESTORECMD", "fdrestore");

	// [DRIVES]セクション読み込み
	for (num=1; ; num++) {
		char key[FDX_FILENAME_MAX];
		sprintf(key, "DRIVE-%d", num);
		if (!mIniFile.hasKey("DRIVES", key)) {
			break;
		}
	}
	if (num < 2) {
		FDS_ERROR("failed: no DRIVE entry in [DRIVES] section in file %s\n", filename.c_str());
		fprintf(stderr, "No DRIVE entry in [DRIVES] section in file %s\n", filename.c_str());
		return -1;
	}
	mVecDrives.resize(num-1);
	mDrivesDir = mIniFile.getString("DRIVES", "DIR", "drives");
	for (int i=1; i<num; i++) {
		char key[FDX_FILENAME_MAX];
		sprintf(key, "DRIVE-%d", i);
		std::string vfilename = mIniFile.getString("DRIVES", key);
		std::string path = mDrivesDir + "/" + vfilename;
		FDSDrive& drive = mVecDrives[i-1];
		ret = drive.load(path);
		if (ret < 0) {
			return ret;
		}
	}

	// [MACHINES]セクション読み込み
	for (num=1; ; num++) {
		char key[FDX_FILENAME_MAX];
		sprintf(key, "MACHINE-%d", num);
		if (!mIniFile.hasKey("MACHINES", key)) {
			break;
		}
	}
	if (num < 2) {
		FDS_ERROR("failed: no MACHINE entry in [MACHINES] section in file %s\n", filename.c_str());
		fprintf(stderr, "No MACHINE entry in [MACHINES] section in file %s\n", filename.c_str());
		return -1;
	}
	mVecMachines.resize(num-1);
	mMachinesDir = mIniFile.getString("MACHINES", "DIR", "machines");
	for (int i=1; i<num; i++) {
		char key[FDX_FILENAME_MAX];
		sprintf(key, "MACHINE-%d", i);
		std::string vfilename = mIniFile.getString("MACHINES", key);
		std::string path = mMachinesDir + "/" + vfilename;
		FDSMachine& machine = mVecMachines[i-1];
		ret = machine.load(path);
		if (ret < 0) {
			return ret;
		}
	}

	// [OTHERIMAGES]セクション読み込み
	for (num=1; ; num++) {
		char key[FDX_FILENAME_MAX];
		sprintf(key, "OTHERIMAGE-%d", num);
		if (!mIniFile.hasKey("OTHERIMAGES", key)) {
			break;
		}
	}
	mVecOtherImages.resize(num-1);
	for (int i=1; i<num; i++) {
		char key[FDX_FILENAME_MAX];
		sprintf(key, "OTHERIMAGE-%d", i);
		std::string ext = mIniFile.getString("OTHERIMAGES", key);
		mVecOtherImages[i-1] = ext;
	}

	return 0;
}

// -------------------------------------------------------------
// ダンプオプション文字列作成
// -------------------------------------------------------------
std::string
FDSConfig::makeDumpOpt(int machineno, int driveno, int dumpno) const
{
	char buf[FDX_STRING_MAX];

	const std::string& type = cfgMachine(machineno).dump(dumpno).type();
	int no = cfgDrive(driveno).findDumpNoByType(type);
	if (no < 0) {
		FDS_ERROR("failed: machineno=%d, driveno=%d, dumpno=%d, type=[%s], no=%d\n", machineno, driveno, dumpno, type.c_str(), no);
		fprintf(stderr, "failed: machineno=%d, driveno=%d, dumpno=%d, type=[%s], no=%d\n", machineno, dumpno, driveno, type.c_str(), no);
		std::string str;
		return str;
	}

	sprintf(buf, "-i%d -r%d %s -f %s",
	  cfgDrive(driveno).id(),
	  cfgDrive(driveno).retry(),
	  cfgDrive(driveno).dump(no).fdDumpOpt().c_str(),
	  cfgMachine(machineno).dump(dumpno).format().c_str()
	);
	std::string str(buf);
	return str;
}


// -------------------------------------------------------------
// トラックダンプオプション文字列作成
// -------------------------------------------------------------
std::string
FDSConfig::makeDumpTrackOpt(int machineno, int driveno, int dumpno, int trackno, int retrycount) const
{
	char buf[FDX_STRING_MAX];

	const std::string& type = cfgMachine(machineno).dump(dumpno).type();
	int no = cfgDrive(driveno).findDumpNoByType(type);
	if (no < 0) {
		FDS_ERROR("failed: machineno=%d, driveno=%d, dumpno=%d, type=[%s], no=%d\n", machineno,driveno, dumpno, type.c_str(), no);
		fprintf(stderr, "failed: machineno=%d, driveno=%d, dumpno=%d, type=[%s], no=%d\n", machineno, driveno, dumpno, type.c_str(), no);
		std::string str;
		return str;
	}

	sprintf(buf, "-i%d -r%d -t%d %s -f %s",
	  cfgDrive(driveno).id(),
	  retrycount,
	  trackno,
	  cfgDrive(driveno).dump(no).fdDumpOpt().c_str(),
	  cfgMachine(machineno).dump(dumpno).format().c_str()
	);
	std::string str(buf);
	return str;
}


// -------------------------------------------------------------
// リストアオプション文字列作成
// -------------------------------------------------------------
std::string
FDSConfig::makeRestoreOpt(int machineno, int driveno, int restoreno) const
{
	char buf[FDX_STRING_MAX];

	const std::string& type = cfgMachine(machineno).restore(restoreno).type();
	int no = cfgDrive(driveno).findRestoreNoByType(type);
	if (no < 0) {
		FDS_ERROR("failed: machineno=%d, driveno=%d, restoreno=%d, type=[%s], no=%d\n", machineno, driveno, restoreno, type.c_str(), no);
		fprintf(stderr, "failed: machineno=%d, driveno=%d, restoreno=%d, type=[%s], no=%d\n", machineno, driveno, restoreno, type.c_str(), no);
		std::string str;
		return str;
	}

	sprintf(buf, "-i%d %s",
	  cfgDrive(driveno).id(),
	  cfgDrive(driveno).restore(no).fdRestoreOpt().c_str()
	);
	std::string str(buf);
	return str;
}


// -------------------------------------------------------------
// リストアトラックオプション文字列作成
// -------------------------------------------------------------
std::string
FDSConfig::makeRestoreTrackOpt(int machineno, int driveno, int restoreno, int trackno) const
{
	char buf[FDX_STRING_MAX];

	const std::string& type = cfgMachine(machineno).restore(restoreno).type();
	int no = cfgDrive(driveno).findRestoreNoByType(type);
	if (no < 0) {
		FDS_ERROR("failed: machineno=%d, driveno=%d, restoreno=%d, type=[%s], no=%d\n", machineno, driveno, restoreno, type.c_str(), no);
		fprintf(stderr, "failed: machineno=%d, driveno=%d, restoreno=%d, type=[%s], no=%d\n", machineno, driveno, restoreno, type.c_str(), no);
		std::string str;
		return str;
	}

	sprintf(buf, "-i%d -t%d %s",
	  cfgDrive(driveno).id(),
	  trackno,
	  cfgDrive(driveno).restore(no).fdRestoreOpt().c_str()
	);
	std::string str(buf);
	return str;
}


// [EOF]
