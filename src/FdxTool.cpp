// ◇
// fds: fdx68 selector
// FdxTool: FdxToolの操作
// Copyright: (C)2020 Hiroaki GOTO as GORRY.
// License: see readme.txt
// =====================================================================

#include "FdxTool.h"

// =====================================================================
// FdxToolの操作
// =====================================================================

#define FDXTOOL_LOGFILENAME "fdxtool.log"

// -------------------------------------------------------------
// コンストラクタ
// -------------------------------------------------------------
FdxTool::FdxTool()
{
}

// -------------------------------------------------------------
// 外部コマンドの実行
// -------------------------------------------------------------
bool
FdxTool::execCmd(const std::string& cmd, const std::string& option)
{
	// 外部コマンドを起動
	std::string cmdline = cmd + " " + option;
	cmdline += ">>" FDXTOOL_LOGFILENAME " 2>&1";
	FDS_LOG("execCmd: [%s]\n", cmdline.c_str());
	int exitcode = system(cmdline.c_str());
	FDS_LOG("execCmd: exitcode=%d\n", exitcode);

	return (exitcode == 0);
}

// -------------------------------------------------------------
// fdxtoolログのヘッダ読み取り
// -------------------------------------------------------------
int
FdxTool::readFDXDiskInfoHeader(FdxDiskInfo& diskinfo, char* s)
{
	// FDS_LOG("readFDXDiskInfoHeader: get line: %s", s);
	if (!memcmp(s, "DIsk Type          : ", 21) ||
	    !memcmp(s, "Disk Type          : ", 21)) {
		s += 21;
		if (!memcmp(s, "RAW", 3)) {
			diskinfo.mFdxInfo.mType = 9;
			return 1;
		}
		if (!memcmp(s, "2HD", 3)) {
			diskinfo.mFdxInfo.mType = 2;
			return 1;
		}
		if (!memcmp(s, "2DD", 3)) {
			diskinfo.mFdxInfo.mType = 1;
			return 1;
		}
		if (!memcmp(s, "2D", 2)) {
			diskinfo.mFdxInfo.mType = 0;
			return 1;
		}
		return 1;
	}
	if (!memcmp(s, "Cylinders          : ", 21)) {
		s += 21;
		diskinfo.mFdxInfo.mCylinders = atoi(s);
		return 1;
	}
	if (!memcmp(s, "Heads              : ", 21)) {
		s += 21;
		diskinfo.mFdxInfo.mHeads = atoi(s);
		return 1;
	}
	if (!memcmp(s, "Rate               : ", 21)) {
		s += 21;
		diskinfo.mFdxInfo.mRate = atoi(s);
		return 1;
	}
	if (!memcmp(s, "RPM                : ", 21)) {
		s += 21;
		diskinfo.mFdxInfo.mRpm = atoi(s);
		return 1;
	}
	if (!memcmp(s, "Write Protect      : ", 21)) {
		s += 21;
		if (!memcmp(s, "OFF", 3)) {
			diskinfo.mFdxInfo.mWriteProtect = 0;
			return 1;
		}
		diskinfo.mFdxInfo.mWriteProtect = 1;
		return 1;
	}
	if (!memcmp(s, "Sample length      : ", 21)) {
		s += 21;
		diskinfo.mFdxInfo.mTrackSize = atoi(s);
		return 1;
	}
	if (!memcmp(s, "Track length       : ", 21)) {
		s += 21;
		if (!diskinfo.mFdxInfo.isRaw()) {
			diskinfo.mFdxInfo.mTrackSize = atoi(s)<<3;
		}
		return 1;
	}
	if (!memcmp(s, "CYLINDER ", 9)) {
		return -1;
	}
	return 0;
}

// -------------------------------------------------------------
// fdxtoolログの本体読み取り
// -------------------------------------------------------------
int
FdxTool::readFDXDiskInfoBody(FdxDiskInfo& diskinfo, char* s)
{
	// FDS_LOG("readFDXDiskInfoBody: get line: %s", s);
	if (!memcmp(s, "CYLINDER ", 9)) {
		s += 9;
		mLastSector = 0;
		if (diskinfo.mTrack.size() == 0) {
			int tracks = diskinfo.mFdxInfo.mCylinders * diskinfo.mFdxInfo.mHeads;
			FDS_LOG("readFDXDiskInfo: tracks=%d\n", tracks);
			diskinfo.mTrack.resize(tracks);
		}
		if (s[0] == '0') {
			s++;
		}
		int cylinder = strtoul(s, &s, 10);
		s += 11;
		int head = strtoul(s, &s, 10);
		int track = cylinder * diskinfo.mFdxInfo.mHeads + head;
		mLastTrack = track;
		if (track >= (int)diskinfo.mTrack.size()) {
			FDS_LOG("readFDXDiskInfo: invalid track=%d\n", track);
			return 1;
		}
		diskinfo.mTrack[track].mCylinder = cylinder;
		diskinfo.mTrack[track].mHead = head;
		s += 3;
		if (!memcmp(s, "UNFORMAT TRACK", 14)) {
			return 1;
		}
		s += 14;
		diskinfo.mTrack[track].mLenBits = strtoul(s, &s, 10);
		s += 20;
		int sector = strtoul(s, &s, 10);
		if (sector == 0) {
			return 1;
		}
		diskinfo.mTrack[track].mSectors = sector;
		diskinfo.mTrack[track].mSector.resize(sector);
		while (s[0] >= ' ') {
			if (!memcmp(s, "/", 1)) {
				s += 1;
				continue;
			}
			if (!memcmp(s, ")", 1)) {
				s += 1;
				continue;
			}
			if (!memcmp(s, "SCT", 3)) {
				s += 3;
				continue;
			}
			if (!memcmp(s, "MIX", 3)) {
				s += 3;
				diskinfo.mTrack[track].mStatus.SetInfoIMIX(true);
				continue;
			}
			if (!memcmp(s, "MFM", 3)) {
				s += 3;
				diskinfo.mTrack[track].mStatus.SetInfoIMFM(true);
				continue;
			}
			if (!memcmp(s, "FM", 2)) {
				s += 2;
				diskinfo.mTrack[track].mStatus.SetInfoIFM(true);
				continue;
			}
			if (!memcmp(s, "IAM", 3)) {
				s += 3;
				diskinfo.mTrack[track].mStatus.SetInfoIAM(true);
				continue;
			}
			if (!memcmp(s, "ISTR", 4)) {
				s += 4;
				diskinfo.mTrack[track].mStatus.SetInfoISTR(true);
				continue;
			}
			if (!memcmp(s, "NORMAL", 6)) {
				s += 6;
				continue;
			}
			if (!memcmp(s, "ILLEGAL(", 8)) {
				s += 8;
				continue;
			}
			if (!memcmp(s, "INVALD ID", 9)) {
				s += 9;
				diskinfo.mTrack[track].mStatus.SetErrInfoInvalid(true);
				continue;
			}
			if (!memcmp(s, "ID CRC ERROR", 12)) {
				s += 12;
				diskinfo.mTrack[track].mStatus.SetErrInfoCRC(true);
				continue;
			}
			if (!memcmp(s, "INVALD DATA", 11)) {
				s += 11;
				diskinfo.mTrack[track].mStatus.SetErrDataInvalid(true);
				continue;
			}
			if (!memcmp(s, "DATA CRC ERROR", 14)) {
				s += 14;
				diskinfo.mTrack[track].mStatus.SetErrDataCRC(true);
				continue;
			}
			
		}
		return 1;
	}

	int track = mLastTrack;
	if (!memcmp(s, " GAP4a ", 7)) {
		s += 7;
		diskinfo.mTrack[track].mSizeGap4a = atoi(s);
		return 1;
	}
	if (!memcmp(s, " GAP4b ", 7)) {
		s += 7;
		diskinfo.mTrack[track].mSizeGap4a = atoi(s);
		diskinfo.mTrack[track].mSector.resize(mLastSector);
		return 1;
	}
	if (!memcmp(s, " IAM ", 5)) {
		s += 5;
		diskinfo.mTrack[track].mCellIAM = atoi(s);
		return 1;
	}
	if (!memcmp(s, " GAP1 ", 6)) {
		s += 6;
		diskinfo.mTrack[track].mSizeGap1 = atoi(s);
		return 1;
	}
	if (!memcmp(s, " M", 2) || !memcmp(s, " F", 2)) {
		int mfm = (!memcmp(s, " M", 2)) ? 1 : 0;
		s += 2;
		if (s[0] == '0') {
			s++;
		}
		int sector = strtoul(s, &s, 10);
		if (sector <= 0) {
			return 1;
		}
		mLastSector = sector;
		sector--;
		if (sector >= (int)diskinfo.mTrack[track].mSector.size()) {
			diskinfo.mTrack[track].mSector.resize(sector+10);
		}

		if (mfm) {
			diskinfo.mTrack[track].mSector[sector].mStatus.SetInfoIMFM(true);
		} else {
			diskinfo.mTrack[track].mSector[sector].mStatus.SetInfoIFM(true);
		}
		diskinfo.mTrack[track].mSector[sector].mCellStart = strtoul(s, &s, 10);
		s += 3;
		diskinfo.mTrack[track].mSector[sector].mCellEnd = strtoul(s, &s, 10);
		diskinfo.mTrack[track].mSector[sector].mTime = strtoul(s, &s, 10);
		int c = strtoul(s, &s, 16);
		int h = strtoul(s, &s, 16);
		int r = strtoul(s, &s, 16);
		int n = strtoul(s, &s, 16);
		diskinfo.mTrack[track].mSector[sector].SetCHRN(c, h, r, n);
		diskinfo.mTrack[track].mSector[sector].mSecSize = strtoul(s, &s, 10);
		diskinfo.mTrack[track].mSector[sector].mGap2 = strtoul(s, &s, 10);
		diskinfo.mTrack[track].mSector[sector].mGap3 = strtoul(s, &s, 10);
		s += 2;
		if (!memcmp(s, "ERR", 3)) {
			diskinfo.mTrack[track].mSector[sector].mStatus.SetErrInfoCRC(true);
		}
		s += 4;
		if (!memcmp(s, "---", 3)) {
			diskinfo.mTrack[track].mSector[sector].mStatus.SetErrDataNothing(true);
		}
		if (!memcmp(s, "DEL", 3)) {
			diskinfo.mTrack[track].mSector[sector].mStatus.SetErrDataDeleted(true);
		}
		s += 5;
		if (!memcmp(s, "ERR", 3)) {
			diskinfo.mTrack[track].mSector[sector].mStatus.SetErrDataCRC(true);
		}
		s += 4;
		diskinfo.mTrack[track].mSector[sector].mElapse = strtoul(s, &s, 10);

		return 1;
	}
	return 0;
}

// -------------------------------------------------------------
// fdxファイル情報の読み取り
// -------------------------------------------------------------
bool
FdxTool::execFdxToolAnalyze(char* tmpfilename, const std::string& cmd, const std::string& filename, const char* option)
{
#if !defined(FDS_WINDOWS)
	// ログファイル名を作成
	sprintf(tmpfilename, "/tmpfilename/fdxtool.log.XXXXXX");
	int tmpfd = mkstemp(tmpfilename);
	if (tmpfd < 0) {
		FDS_ERROR("cannot open temp file: errno=%d\n", errno);
		return false;
	}
	close(tmpfd);

	// 外部コマンドを起動
	std::string cmdline = cmd + " " + option + " \"" + filename + "\" >" + tmpfilename;
	FDS_LOG("readFDXDiskInfo: [%s]\n", cmdline.c_str());
	int exitcode = system(cmdline.c_str());
	if (exitcode != 0) {
		FDS_ERROR("readFDXDiskInfo: exitcode=%d\n", exitcode);
		unlink(tmpfilename);
		return false;
	}
	FDS_LOG("readFDXDiskInfo: exitcode=%d\n", exitcode);

#else
#if 0
	// ログファイルを開く
	sprintf(tmpfilename, "fdxanalyze1.log");
#else
#define unlink(a) _unlink(a)
	// ログファイル名を作成
	char tmpdir[FDX_FILENAME_MAX+1];
	const int tmpdirsiz = sizeof(tmpdir)/sizeof(tmpdir[0]);
	GetTempPathA(tmpdirsiz, tmpdir);
	sprintf(tmpfilename, "%sfdxtool.log.XXXXXX", tmpdir);
	errno_t err = _mktemp_s(tmpfilename, tmpdirsiz);
	if (err < 0) {
		FDS_ERROR("cannot open temp file: errno=%d\n", err);
		return false;
	}

	// 外部コマンドを起動
	std::string cmdline = cmd + " " + option + " \"" + filename + "\" >" + tmpfilename;
	FDS_LOG("readFDXDiskInfo: [%s]\n", cmdline.c_str());
	int exitcode = system(cmdline.c_str());
	if (exitcode != 0) {
		FDS_ERROR("readFDXDiskInfo: exitcode=%d\n", exitcode);
		unlink(tmpfilename);
		return false;
	}
	FDS_LOG("readFDXDiskInfo: exitcode=%d\n", exitcode);
#endif
#endif
	return true;
}

bool
FdxTool::readFDXDiskInfo(const std::string& cmd, const std::string& filename)
{
	bool ret = true;
	char line[1024];

	mDiskInfo.Clear();

	// ログファイルを作成する
	char tmp[FDX_FILENAME_MAX+1];
	ret = execFdxToolAnalyze(tmp, cmd, filename, "-a");
	if (!ret) {
		return false;
	}

	// ログファイルを開く
	FILE* fin = fopen(tmp, "rb");
	if (fin == nullptr) {
		FDS_ERROR("cannot open temp file [%s]\n", tmp);
		unlink(tmp);
		return false;
	}

	// ログファイルの解析
	mLastTrack = 0;
	int linestatus = 0;
	char* s = fgets(line, sizeof(line), fin);
	while (s != nullptr) {
		linestatus = readFDXDiskInfoHeader(mDiskInfo, s);
		if (linestatus < 0) break;
		s = fgets(line, sizeof(line), fin);
	}

	if (linestatus < 0) {
		while (s != nullptr) {
			linestatus = readFDXDiskInfoBody(mDiskInfo, s);
			if (linestatus < 0) break;
			s = fgets(line, sizeof(line), fin);
		}
	}

	fclose(fin);
	unlink(tmp);

	return ret;
}

// -------------------------------------------------------------
// fdxファイル詳細情報の読み取り
// -------------------------------------------------------------
bool
FdxTool::readFDXDiskInfoVerbose(const std::string& cmd, const std::string& filename)
{
	bool ret = true;
	char line[1024];

	mDiskInfoVerbose.Clear();

	// ログファイルを作成する
	char tmp[FDX_FILENAME_MAX+1];
	ret = execFdxToolAnalyze(tmp, cmd, filename, "-a -v");
	if (!ret) {
		return false;
	}


	// ログファイルを開く
	FILE* fin = fopen(tmp, "rb");
	if (fin == nullptr) {
		FDS_ERROR("cannot open temp file [%s]\n", tmp);
		return false;
	}

	// ログファイルの解析
	mLastTrack = 0;
	int linestatus = 0;
	char* s = fgets(line, sizeof(line), fin);
	while (s != nullptr) {
		linestatus = readFDXDiskInfoHeader(mDiskInfoVerbose, s);
		if (linestatus < 0) break;
		s = fgets(line, sizeof(line), fin);
	}

	if (linestatus < 0) {
		while (s != nullptr) {
			linestatus = readFDXDiskInfoBody(mDiskInfoVerbose, s);
			if (linestatus < 0) break;
			s = fgets(line, sizeof(line), fin);
		}
	}

	fclose(fin);
	unlink(tmp);

	return ret;
}

// -------------------------------------------------------------
// fdxファイル情報のトラックステータスを詳細情報側にコピーする
// -------------------------------------------------------------
bool
FdxTool::copyFDXDiskInfoTrackStatus(void)
{
	if (mDiskInfoVerbose.TrackSize() != mDiskInfo.TrackSize()) {
		return false;
	}

	for (int i=0; i<(int)mDiskInfoVerbose.TrackSize(); i++) {
		mDiskInfoVerbose.Track(i).mStatus = mDiskInfo.Track(i).mStatus;
	}

	return true;
}

// =====================================================================
// [EOF]
