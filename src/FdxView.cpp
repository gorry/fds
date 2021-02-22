// ◇
// fds: fdx68 selector
// FdxView: FdxViewの操作
// Copyright: (C)2020 Hiroaki GOTO as GORRY.
// License: see readme.txt
// =====================================================================

#include "FdxView.h"

// =====================================================================
// FdxViewの操作
// =====================================================================

#define FDXVIEW_LOGFILENAME "fdxview.log"

// -------------------------------------------------------------
// コンストラクタ
// -------------------------------------------------------------
FdxView::FdxView()
{
}

// -------------------------------------------------------------
// 外部コマンドの実行
// -------------------------------------------------------------
bool
FdxView::execCmd(const std::string& cmd, const std::string& option)
{
	// 外部コマンドを起動
	std::string cmdline = cmd + " " + option;
	cmdline += ">>" FDXVIEW_LOGFILENAME " 2>&1";
	FDS_LOG("execCmd: [%s]\n", cmdline.c_str());
	int exitcode = system(cmdline.c_str());
	FDS_LOG("execCmd: exitcode=%d\n", exitcode);

	return (exitcode == 0);
}

// -------------------------------------------------------------
// fdxviewログのヘッダ読み取り
// -------------------------------------------------------------
int
FdxView::readFDXDiskInfoHeader(DiskInfo& diskinfo, char* s)
{
	FDS_LOG("readFDXDiskInfoHeader: get line: %s", s);
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
		diskinfo.mFdxInfo.mTrackSize = diskinfo.mFdxInfo.mRate * 1000 * 60 / (double)diskinfo.mFdxInfo.mRpm;
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
	if (!memcmp(s, "TRACK ", 5)) {
		return -1;
	}
	return 0;
}

// -------------------------------------------------------------
// fdxviewログの本体読み取り
// -------------------------------------------------------------
int
FdxView::readFDXDiskInfoBody(DiskInfo& diskinfo, char* s, int mode)
{
	FDS_LOG("readFDXDiskInfoBody: get line: %s", s);
	TrackInfo* track = mLastTrackInfo;

	if (!memcmp(s, "TRACK ERRORS", 12)) {
		return 1;
	}
	if (!memcmp(s, "TRACK ", 5)) {
		s += 5;
		mLastSectorNo = 0;
		int trackno = strtoul(s, &s, 10);
		s += 3;
		if (s[0] == '0') {
			s++;
		}
		int cylinder = strtoul(s, &s, 10);
		s += 3;
		if (s[0] == '0') {
			s++;
		}
		int head = strtoul(s, &s, 10);
		switch (mode) {
		  default:
			FDS_LOG("readFDXDiskInfoBody: invalid mode=%d\n", mode);
			return 1;
		  case 0:
			if (diskinfo.mTrack.size() == 0) {
				int tracks = diskinfo.mFdxInfo.mCylinders * diskinfo.mFdxInfo.mHeads;
				FDS_LOG("readFDXDiskInfo: tracks=%d\n", tracks);
				diskinfo.mTrack.resize(tracks);
			}
			mLastTrackNo = trackno;
			if (trackno >= (int)diskinfo.mTrack.size()) {
				FDS_LOG("readFDXDiskInfoBody: invalid track=%d\n", track);
				return 1;
			}
			track = &(diskinfo.mTrack[trackno]);
			break;
		  case 1:
			if (diskinfo.mTrack.size() == 0) {
				diskinfo.mTrack.resize(1);
			}
			track = &(diskinfo.mTrack[0]);
		}
		mLastTrackInfo = track;
		track->mCylinder = cylinder;
		track->mHead = head;
		s += 1;
		while (s[0] == ' ') {
			s++;
		}
		if (!memcmp(s, "UNFORMAT TRACK", 14)) {
			return 1;
		}
		track->mLenBits = strtoul(s, &s, 10);
		s += 4;
		int lenbytes = strtoul(s, &s, 10);
		s += 5;
		(void)lenbytes;
		int lencells = strtoul(s, &s, 10);
		s += 6;
		(void)lencells;
		track->mDRate = strtod(s, &s);
		s += 2;
		int sector = strtoul(s, &s, 10);
		if (sector == 0) {
			return 1;
		}
		track->mSectors = sector;
		track->mSector.resize(sector);
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
				track->mStatus.SetInfoIMIX(true);
				continue;
			}
			if (!memcmp(s, "MFM", 3)) {
				s += 3;
				track->mStatus.SetInfoIMFM(true);
				continue;
			}
			if (!memcmp(s, "FM", 2)) {
				s += 2;
				track->mStatus.SetInfoIFM(true);
				continue;
			}
			if (!memcmp(s, "IAM", 3)) {
				s += 3;
				track->mStatus.SetInfoIAM(true);
				continue;
			}
			if (!memcmp(s, "ISTR", 4)) {
				s += 4;
				track->mStatus.SetInfoISTR(true);
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
				track->mStatus.SetErrInfoInvalid(true);
				continue;
			}
			if (!memcmp(s, "ID CRC ERROR", 12)) {
				s += 12;
				track->mStatus.SetErrInfoCRC(true);
				continue;
			}
			if (!memcmp(s, "INVALD DATA", 11)) {
				s += 11;
				track->mStatus.SetErrDataInvalid(true);
				continue;
			}
			if (!memcmp(s, "DATA CRC ERROR", 14)) {
				s += 14;
				track->mStatus.SetErrDataCRC(true);
				continue;
			}
			
		}
		return 1;
	}

	if (track == nullptr) {
		FDS_LOG("readFDXDiskInfoBody: mLastTrackInfo == nullptr\n");
		return 1;
	}

	if (!memcmp(s, " GAP4a ", 7)) {
		s += 7;
		track->mSizeGap4a = atoi(s);
		return 1;
	}
	if (!memcmp(s, " GAP4b ", 7)) {
		s += 7;
		track->mSizeGap4b = atoi(s);
		track->mSector.resize(mLastSectorNo);
		return 1;
	}
	if (!memcmp(s, " IAM ", 5)) {
		s += 5;
		track->mCellIAM = atoi(s);
		return 1;
	}
	if (!memcmp(s, " GAP1 ", 6)) {
		s += 6;
		track->mSizeGap1 = atoi(s);
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
		mLastSectorNo = sector;
		sector--;
		if (sector >= (int)track->mSector.size()) {
			track->mSector.resize(sector+10);
		}

		if (mfm) {
			track->mSector[sector].mStatus.SetInfoIMFM(true);
		} else {
			track->mSector[sector].mStatus.SetInfoIFM(true);
		}
		track->mSector[sector].mCellStart = strtoul(s, &s, 10);
		s += 3;
		track->mSector[sector].mCellEnd = strtoul(s, &s, 10);
		track->mSector[sector].mTime = strtoul(s, &s, 10);
		int c = strtoul(s, &s, 16);
		int h = strtoul(s, &s, 16);
		int r = strtoul(s, &s, 16);
		int n = strtoul(s, &s, 16);
		track->mSector[sector].SetCHRN(c, h, r, n);
		track->mSector[sector].mSecSize = strtoul(s, &s, 10);
		track->mSector[sector].mGap2 = strtoul(s, &s, 10);
		track->mSector[sector].mGap3 = strtoul(s, &s, 10);
		s += 2;
		if (!memcmp(s, "ERR", 3)) {
			track->mSector[sector].mStatus.SetErrInfoCRC(true);
		}
		s += 4;
		if (!memcmp(s, "---", 3)) {
			track->mSector[sector].mStatus.SetErrDataNothing(true);
		}
		if (!memcmp(s, "DEL", 3)) {
			track->mSector[sector].mStatus.SetErrDataDeleted(true);
		}
		s += 5;
		if (!memcmp(s, "ERR", 3)) {
			track->mSector[sector].mStatus.SetErrDataCRC(true);
		}
		s += 3;
		track->mSector[sector].mDRate = strtod(s, &s);

		return 1;
	}
	return 0;
}

// -------------------------------------------------------------
// fdxファイル情報の読み取り
// -------------------------------------------------------------
bool
FdxView::execFdxViewAnalyze(char* tmpfilename, const std::string& cmd, const std::string& filename, const char* option)
{
#if !defined(FDS_WINDOWS)
	// ログファイル名を作成
	sprintf(tmpfilename, "/tmp/fdxview.log.XXXXXX");
	int tmpfd = mkstemp(tmpfilename);
	if (tmpfd < 0) {
		FDS_ERROR("cannot open temp file: errno=%d", errno);
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
	sprintf(tmpfilename, "%sfdxview.log.XXXXXX", tmpdir);
	errno_t err = _mktemp_s(tmpfilename, tmpdirsiz);
	if (err < 0) {
		FDS_ERROR("cannot open temp file: errno=%d", err);
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
FdxView::readFDXDiskInfo(const std::string& cmd, const std::string& filename)
{
	bool ret = true;
	char line[1024];

	mDiskInfo.Clear();

	// ログファイルを作成する
	char tmp[FDX_FILENAME_MAX+1];
	ret = execFdxViewAnalyze(tmp, cmd, filename, "-v track");
	if (!ret) {
		return false;
	}

	// ログファイルを開く
	FILE* fin = fopen(tmp, "rb");
	if (fin == nullptr) {
		FDS_ERROR("cannot open temp file [%s]", tmp);
		unlink(tmp);
		return false;
	}

	// ログファイルの解析
	mLastTrackNo = 0;
	int linestatus = 0;
	char* s = fgets(line, sizeof(line), fin);
	while (s != nullptr) {
		linestatus = readFDXDiskInfoHeader(mDiskInfo, s);
		if (linestatus < 0) break;
		s = fgets(line, sizeof(line), fin);
	}

	if (linestatus < 0) {
		while (s != nullptr) {
			linestatus = readFDXDiskInfoBody(mDiskInfo, s, 0);
			if (linestatus < 0) break;
			s = fgets(line, sizeof(line), fin);
		}
	}

	fclose(fin);
	unlink(tmp);

	return ret;
}

// -------------------------------------------------------------
// fdxトラック情報の読み取り
// -------------------------------------------------------------
bool
FdxView::readFDXTrack(const std::string& cmd, const std::string& filename, int track)
{
	bool ret = true;
	char line[1024];

	mTrackInfo.Clear();

	// ログファイルを作成する
	char tmp[FDX_FILENAME_MAX+1];
	char opt[256];
	int heads = mDiskInfo.mFdxInfo.mHeads;
	if (heads == 0) heads = 2;
	int cylinder = track / heads;
	int head = track % heads;
	sprintf(opt, "-v sector -c %d -h %d", cylinder, head);
	ret = execFdxViewAnalyze(tmp, cmd, filename, opt);
	if (!ret) {
		return false;
	}


	// ログファイルを開く
	FILE* fin = fopen(tmp, "rb");
	if (fin == nullptr) {
		FDS_ERROR("cannot open temp file [%s]", tmp);
		return false;
	}

	// ログファイルの解析
	mLastTrackNo = 0;
	int linestatus = 0;
	char* s = fgets(line, sizeof(line), fin);
	while (s != nullptr) {
		linestatus = readFDXDiskInfoHeader(mTrackInfo, s);
		if (linestatus < 0) break;
		s = fgets(line, sizeof(line), fin);
	}

	if (linestatus < 0) {
		while (s != nullptr) {
			linestatus = readFDXDiskInfoBody(mTrackInfo, s, 1);
			if (linestatus < 0) break;
			s = fgets(line, sizeof(line), fin);
		}
	}

	fclose(fin);
	unlink(tmp);

	return ret;
}

// =====================================================================
// [EOF]
