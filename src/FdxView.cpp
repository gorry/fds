﻿// ◇
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
// トラックが読み込み済みか調べる
// -------------------------------------------------------------
bool
FdxView::isTrackReady(int trackno)
{
	return mDiskInfo.isTrackReady(trackno);
}

// -------------------------------------------------------------
// トラックのリロードを要求
// -------------------------------------------------------------
bool
FdxView::ReqTrackReload(int trackno)
{
	return mDiskInfo.ReqTrackReload(trackno);
}

// -------------------------------------------------------------
// セクタが読み込み済みか調べる
// -------------------------------------------------------------
bool
FdxView::isSectorReady(int trackno, int sectorno)
{
	return mDiskInfo.isSectorReady(trackno, sectorno);
}

// -------------------------------------------------------------
// セクタのリロードを要求
// -------------------------------------------------------------
bool
FdxView::ReqSectorReload(int trackno, int sectorno)
{
	return mDiskInfo.ReqSectorReload(trackno, sectorno);
}

// -------------------------------------------------------------
// fdxviewログのヘッダ読み取り
// -------------------------------------------------------------
int
FdxView::readFDXDiskInfoHeader(FdxDiskInfo& diskinfo, char* s)
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
		diskinfo.mFdxInfo.mTrackSize = (int)(diskinfo.mFdxInfo.mRate * 1000 * 60 / (double)diskinfo.mFdxInfo.mRpm);
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
FdxView::readFDXDiskInfoBody(FdxDiskInfo& diskinfo, char* s)
{
	FDS_LOG("readFDXDiskInfoBody: get line: %s", s);

	if (!memcmp(s, "SECTOR ", 7)) {
		return -1;
	}
	if (!memcmp(s, "TRACK ERRORS", 12)) {
		return 1;
	}
	if (!memcmp(s, "TRACK ", 5)) {
		s += 5;
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
		if (diskinfo.mTrack.size() == 0) {
			int tracks = diskinfo.mFdxInfo.mCylinders * diskinfo.mFdxInfo.mHeads;
			FDS_LOG("readFDXDiskInfoBody: tracks=%d\n", tracks);
			diskinfo.mTrack.resize(tracks);
		}
		if (trackno >= (int)diskinfo.TrackSize()) {
			FDS_ERROR("readFDXDiskInfoBody: invalid track=%d\n", trackno);
			return 1;
		}
		diskinfo.mLastTrackNo = trackno;
		FdxTrackInfo& trk = diskinfo.Track(diskinfo.mLastTrackNo);
		trk.mCylinder = cylinder;
		trk.mHead = head;
		s += 1;
		while (s[0] == ' ') {
			s++;
		}
		if (!memcmp(s, "UNFORMAT TRACK", 14)) {
			return 1;
		}
		trk.mLenBits = strtoul(s, &s, 10);
		s += 4;
		int lenbytes = strtoul(s, &s, 10);
		s += 5;
		(void)lenbytes;
		int lencells = strtoul(s, &s, 10);
		s += 6;
		(void)lencells;
		trk.mDRate = strtod(s, &s);
		s += 2;
		int sector = strtoul(s, &s, 10);
		if (sector == 0) {
			return 1;
		}
		trk.mSectors = sector;
		trk.mSector.resize(sector);
		trk.mStatus.Clear();
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
				trk.mStatus.SetInfoIMIX(true);
				continue;
			}
			if (!memcmp(s, "MFM", 3)) {
				s += 3;
				trk.mStatus.SetInfoIMFM(true);
				continue;
			}
			if (!memcmp(s, "FM", 2)) {
				s += 2;
				trk.mStatus.SetInfoIFM(true);
				continue;
			}
			if (!memcmp(s, "IAM", 3)) {
				s += 3;
				trk.mStatus.SetInfoIAM(true);
				continue;
			}
			if (!memcmp(s, "ISTR", 4)) {
				s += 4;
				trk.mStatus.SetInfoISTR(true);
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
			if (!memcmp(s, "MISING IDAM", 11)) {
				s += 11;
				trk.mStatus.SetWarnMissingIDAM(true);
				continue;
			}
			if (!memcmp(s, "INVALD ID", 9)) {
				s += 9;
				trk.mStatus.SetErrInfoInvalid(true);
				continue;
			}
			if (!memcmp(s, "ID CRC ERROR", 12)) {
				s += 12;
				trk.mStatus.SetErrInfoCRC(true);
				continue;
			}
			if (!memcmp(s, "INVALD DATA", 11)) {
				s += 11;
				trk.mStatus.SetErrDataInvalid(true);
				continue;
			}
			if (!memcmp(s, "DATA CRC ERROR", 14)) {
				s += 14;
				trk.mStatus.SetErrDataCRC(true);
				continue;
			}
			if (!memcmp(s, "OVERLAP", 7)) {
				s += 7;
				trk.mStatus.SetErrOverlap(true);
				continue;
			}
			
		}
		return 1;
	}

	if ((diskinfo.mLastTrackNo < 0) || ((int)diskinfo.TrackSize() <= diskinfo.mLastTrackNo)) {
		FDS_LOG("readFDXDiskInfoBody: invalid trackno=%d\n", diskinfo.mLastTrackNo);
		return 1;
	}
	FdxTrackInfo& trk = diskinfo.Track(diskinfo.mLastTrackNo);

	if (!memcmp(s, " GAP4a ", 7)) {
		s += 7;
		trk.mSizeGap4a = atoi(s);
		return 1;
	}
	if (!memcmp(s, " GAP4b ", 7)) {
		s += 7;
		trk.mSizeGap4b = atoi(s);
		return 1;
	}
	if (!memcmp(s, " IAM ", 5)) {
		s += 5;
		trk.mCellIAM = atoi(s);
		return 1;
	}
	if (!memcmp(s, " GAP1 ", 6)) {
		s += 6;
		trk.mSizeGap1 = atoi(s);
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
		trk.mLastSectorNo = sector;
		sector--;
		if (sector >= (int)trk.mSector.size()) {
			FDS_ERROR("readFDXDiskInfoBody: invalid sector=%d\n", sector);
			return 1;
		}

		FdxSectorInfo& sct = trk.mSector[sector];
		sct.mStatus.Clear();
		if (mfm) {
			sct.mStatus.SetInfoIMFM(true);
		} else {
			sct.mStatus.SetInfoIFM(true);
		}
		sct.mCellStart = strtoul(s, &s, 10);
		s += 3;
		sct.mCellEnd = strtoul(s, &s, 10);
		sct.mTime = strtoul(s, &s, 10);
		int c = strtoul(s, &s, 16);
		int h = strtoul(s, &s, 16);
		int r = strtoul(s, &s, 16);
		int n = strtoul(s, &s, 16);
		sct.SetCHRN(c, h, r, n);
		sct.mSecSize = strtoul(s, &s, 10);
		sct.mGap2 = strtoul(s, &s, 10);
		sct.mGap3 = strtoul(s, &s, 10);
		s += 2;
		if (!memcmp(s, "ERR", 3)) {
			sct.mStatus.SetErrInfoCRC(true);
		}
		s += 4;
#if 0
		if (!memcmp(s, "---", 3)) {
			sct.mStatus.SetErrDataNothing(true);
		}
#endif
		if (!memcmp(s, "DEL", 3)) {
			sct.mStatus.SetErrDataDeleted(true);
		}
		s += 5;
		if (!memcmp(s, "ERR", 3)) {
			sct.mStatus.SetErrDataCRC(true);
		}
		s += 3;
		sct.mDRate = strtod(s, &s);

		return 1;
	}
	return 0;
}

// -------------------------------------------------------------
// fdxviewログのダンプ読み取り
// -------------------------------------------------------------
int
FdxView::readFDXDiskInfoDump(FdxDiskInfo& diskinfo, char* s)
{
	FDS_LOG("readFDXDiskInfoDump: get line: %s", s);

	if ((diskinfo.mLastTrackNo < 0) || ((int)diskinfo.TrackSize() <= diskinfo.mLastTrackNo)) {
		FDS_ERROR("readFDXDiskInfoDump: track=%d\n", diskinfo.mLastTrackNo);
		return 1;
	}
	FdxTrackInfo& track = diskinfo.Track(diskinfo.mLastTrackNo);

	if (!memcmp(s, "TRACK ERRORS", 12)) {
		return 1;
	}
	if (!memcmp(s, "SECTOR ", 7)) {
		s += 7;
		if (s[0] == '0') {
			s++;
		}
		int sectorno = strtoul(s, &s, 10);
#if 0
		s += 4;
		int c = strtoul(s, &s, 16);
		s += 3;
		int h = strtoul(s, &s, 16);
		s += 3;
		int r = strtoul(s, &s, 16);
		s += 3;
		int n = strtoul(s, &s, 16);
		s += 6;
		if (!memcmp(s, "ERR", 3)) {
			// track->mStatus.SetErrInfoCRC(true);
		}
		s += 6;
		if (!memcmp(s, "ERR", 3)) {
			// track->mStatus.SetErrDataCRC(true);
		}
#endif

		if (sectorno > (int)track.SectorSize()) {
			FDS_ERROR("readFDXDiskInfoDump: sector=%d\n", sectorno);
			return 1;
		}
		track.mLastSectorNo = sectorno-1;
		FdxSectorInfo& sector = track.Sector(track.mLastSectorNo);
		FdxSectorData& data = sector.mSectorData;
		if (data.mData.size() == 0) {
			data.mData.resize(sector.mSecSize+16, 0);
			data.mEncode.resize((sector.mSecSize+16)*2, 0);
		}
		data.mOffset = 0;

		return 1;
	}

	if ((track.mLastSectorNo < 0) || ((int)track.SectorSize() <= track.mLastSectorNo)) {
		FDS_ERROR("readFDXDiskInfoDump: sector=%d\n", track.mLastSectorNo);
		return 1;
	}
	FdxSectorInfo& sector = track.Sector(track.mLastSectorNo);
	FdxSectorData& data = sector.mSectorData;

	char buf[16];
	sprintf(buf, "%04X", data.mOffset);
	if (!memcmp(s, buf, 4)) {
		s += 5;
		for (int i=0; i<16*2; i++) {
			buf[0] = s[0];
			buf[1] = s[1];
			buf[2] = '\0';
			s += 2;
			data.mEncode[data.mOffset*2+i] = (uint8_t)strtoul(buf, nullptr, 16);
		}
		s += 1;
		for (int i=0; i<16; i++) {
			buf[0] = s[0];
			buf[1] = s[1];
			buf[2] = '\0';
			s += 2;
			data.mData[data.mOffset+i] = (uint8_t)strtoul(buf, nullptr, 16);
		}
		data.mOffset += 0x10;
		return 1;
	}
	return 0;
}

// -------------------------------------------------------------
// fdxviewを起動してログを得る
// -------------------------------------------------------------
bool
FdxView::execFdxViewAnalyze(char* tmpfilename, const std::string& cmd, const std::string& filename, const char* option)
{
#if !defined(FDS_WINDOWS)
	// ログファイル名を作成
	sprintf(tmpfilename, "/tmp/fdxview.log.XXXXXX");
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
		FDS_ERROR("execFdxViewAnalyze: exitcode=%d\n", exitcode);
		unlink(tmpfilename);
		return false;
	}
	FDS_LOG("execFdxViewAnalyze: exitcode=%d\n", exitcode);

#else

#define unlink(a) _unlink(a)
	// ログファイル名を作成
	char tmpdir[FDX_FILENAME_MAX+1];
	const int tmpdirsiz = sizeof(tmpdir)/sizeof(tmpdir[0]);
	GetTempPathA(tmpdirsiz, tmpdir);
	sprintf(tmpfilename, "%sfdxview.log.XXXXXX", tmpdir);
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
	return true;
}

// -------------------------------------------------------------
// fdxファイル情報の読み取り
// -------------------------------------------------------------
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
// fdxviewを起動してログを得る（経過表示付き）
// -------------------------------------------------------------
bool
FdxView::execFdxViewAnalyze2(char* tmpfilename, const std::string& cmd, const std::string& filename, const char* option)
{
	// 外部コマンドを作成
	std::string dummy;
	std::string cmdline = dummy + option + " \"" + filename + "\"";

#if !defined(FDS_WINDOWS)
	// ログファイル名を作成
	sprintf(tmpfilename, "/tmp/fdxview.log.XXXXXX");
	int tmpfd = mkstemp(tmpfilename);
	if (tmpfd < 0) {
		FDS_ERROR("cannot open temp file: errno=%d\n", errno);
		return false;
	}
	close(tmpfd);

	// 出力をパイプに差し替える準備
	int pipebuf[2];
	int err = pipe2(pipebuf, O_NONBLOCK);
	if (err < 0) {
		FDS_ERROR("cannot open pipe");
		return -1;
	}
	FDS_LOG("pipe: 0=%d, 1=%d\n", pipebuf[0], pipebuf[1]);

	// スレッドの生成
	pid_t pid = fork();
	if (pid < 0) {
		FDS_ERROR("cannot fork()");
		return -1;
	}

	// 生成先スレッドの実行内容
	if (pid == 0) {
		// 起動したFdDumpのstdoutとstderrを親へリダイレクト
		FDS_LOG("child: dup to pipe\n");
		close(pipebuf[0]);
		dup2(pipebuf[1], STDOUT_FILENO);
		dup2(pipebuf[1], STDERR_FILENO);
		close(pipebuf[1]);

		// FdDumpを起動
		FDS_LOG("child: run cmd=[%s], option=[%s]\n", cmd.c_str(), option);
		std::vector<std::string> argvs;
		std::vector<const char*> argv = makeArgv(cmd, option, argvs);
		err = execvp(cmd.c_str(), (char* const*)&argv[0]);
		if (err != 0) {
			FDS_ERROR("child: error %d: cmd=[%s], option=[%s]\n", cmd.c_str(), option);
		}

		// 起動
		FDS_LOG("child: exit\n");
		exit(1);
		return 0;
	}

	// 生成元スレッドの実行内容

	// 不要なハンドルは閉じる
	close(pipebuf[1]);

	FILE *fout;
	fout = fopen(tmpfilename, "wb");
	if (fout == NULL) {
		FDS_ERROR("execFdxViewAnalyze2: cannot create [%s]\n", tmpfilename);
	}

	// パイプからの入力処理
	std::string line;
	while (!0) {

		// 出力を読み込み
		char buf[1024];
		memset(buf, 0, sizeof(buf));
		ssize_t readsize = read(pipebuf[0], &buf, sizeof(buf));

		// プロセスが終了していたらquit
		if (readsize <= 0) {
			int st;
			if (waitpid(pid, &st, WNOHANG)) {
				if (WIFEXITED(st) || WIFSIGNALED(st)) {
					break;
				}
			}
		}

		// 出力がなければ繰り返し
		if (readsize == 0) {
			usleep(1000*100);
			continue;
		}

		// 出力をログへ
		fwrite(buf, 1, readsize, fout);

		// 出力を解析
		for (ssize_t i=0; i<readsize; i++) {
			char c = buf[i];
			if ((unsigned char)c < 0x20) {
				size_t pos = line.find("TRACK");
				if (pos != std::string::npos) {
					int track = atoi(line.c_str()+pos);
					FDS_LOG("analyze: track %d\n", track);
#if 0
					if (mCallbackFunc) {
						int ret = mCallbackFunc(mStatus, mCallbackParam);
						if (ret < 0) {
							// 中止
							escape = true;
							::kill(pid, SIGHUP);
						}
					}
#endif
				}
				line.clear();
				continue;
			}
			line.push_back(buf[i]);
		}

		// ウェイト
		usleep(200*1000);
	}

#else

	// ログファイル名を作成
	char tmpdir[FDX_FILENAME_MAX+1];
	const int tmpdirsiz = sizeof(tmpdir)/sizeof(tmpdir[0]);
	GetTempPathA(tmpdirsiz, tmpdir);
	sprintf(tmpfilename, "%sfdxview.log.XXXXXX", tmpdir);
	errno_t err = _mktemp_s(tmpfilename, tmpdirsiz);
	if (err < 0) {
		FDS_ERROR("cannot open temp file: errno=%d\n", err);
		return false;
	}

	// パイプの作成
	HANDLE pipe_in_read = NULL;
	HANDLE pipe_in_write = NULL;
	HANDLE pipe_out_read = NULL;
	HANDLE pipe_out_write = NULL;
	SECURITY_ATTRIBUTES sa; 
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;
	BOOL ret = CreatePipe(&pipe_out_read, &pipe_out_write, &sa, 0);
	if (!ret) {
		FDS_ERROR("execFdxViewAnalyze2: cannot create pipe_out\n");
		return false;
	}
	ret = SetHandleInformation(pipe_out_read, HANDLE_FLAG_INHERIT, 0);
	if (!ret) {
		FDS_ERROR("execFdxViewAnalyze2: set pipe_out_read\n");
		return false;
	}

	ret = CreatePipe(&pipe_in_read, &pipe_in_write, &sa, 0);
	if (!ret) {
		FDS_ERROR("execFdxViewAnalyze2: cannot create pipe_in\n");
		return false;
	}
	ret = SetHandleInformation(pipe_in_write, HANDLE_FLAG_INHERIT, 0);
	if (!ret) {
		FDS_ERROR("execFdxViewAnalyze2: set pipe_in_write\n");
		return false;
	}

	// パイプを繋いで子プロセスを起動
	PROCESS_INFORMATION pi = {0};
	STARTUPINFO si = {0};

	si.cb = sizeof(STARTUPINFO); 
	si.hStdError = pipe_out_write;
	si.hStdOutput = pipe_out_write;
	si.hStdInput = pipe_in_read;
	si.dwFlags |= STARTF_USESTDHANDLES;

	ret = CreateProcess(
	  (LPCTSTR)cmd.c_str(), (LPTSTR)cmdline.c_str(),
	  NULL, NULL, TRUE, 0,
	  NULL, NULL, &si,&pi
	);
	if (!ret) {
		FDS_ERROR("execFdxViewAnalyze2: cannot exec [%s]\n", cmdline.c_str());
		CloseHandle(pipe_in_read);
		CloseHandle(pipe_in_write);
		CloseHandle(pipe_out_read);
		CloseHandle(pipe_out_write);
		return false;
	}

	FILE *fout;
	fout = fopen(tmpfilename, "wb");
	if (fout == NULL) {
		FDS_ERROR("execFdxViewAnalyze2: cannot create [%s]\n", tmpfilename);
	}

	// パイプからの入力処理
	std::string line;
	while (!0) {

		// 出力を読み込み
		char buf[1024];
		memset(buf, 0, sizeof(buf));
		DWORD readsize = 0;
		ret = ReadFile(pipe_out_read, buf, sizeof(buf), &readsize, NULL);

		// プロセスが終了していたらquit
		if (!ret) {
			break;
		}
		if (readsize == 0) {
			DWORD w = WaitForSingleObject(pi.hProcess, 0);
			if (w != WAIT_TIMEOUT) {
				break;
			}
		}

		// 出力がなければ繰り返し
		if (readsize == 0) {
			Sleep(100);
			continue;
		}

		// 出力をログへ
		fwrite(buf, 1, readsize, fout);

		// 出力を解析
		for (DWORD i=0; i<readsize; i++) {
			char c = buf[i];
			if ((unsigned char)c < 0x20) {
				size_t pos = line.find("TRACK");
				if (pos != std::string::npos) {
					int track = atoi(line.c_str()+pos);
					FDS_LOG("analyze: track %d\n", track);
#if 0
					if (mCallbackFunc) {
						int ret = mCallbackFunc(mStatus, mCallbackParam);
#if 0
						if (ret < 0) {
							// 中止
							escape = true;
							::kill(mPid, SIGHUP);
							// mStatus.mFinished = true;
						}
#endif
					}
#endif
				}
				line.clear();
				continue;
			}
			line.push_back(buf[i]);
		}

		// ウェイト
		Sleep(200);
	}

	// ハンドルを閉じる
	fclose(fout);
	CloseHandle(pipe_in_read);
	CloseHandle(pipe_in_write);
	CloseHandle(pipe_out_read);
	CloseHandle(pipe_out_write);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

#endif

	return true;
}

// -------------------------------------------------------------
// fdxトラック情報の読み取り
// -------------------------------------------------------------
bool
FdxView::readFDXTrack(const std::string& cmd, const std::string& filename, int trackno)
{
	bool ret = true;
	char line[1024];

	// ディスク情報が読み込み済みでなければまず読みに行く
	if (mDiskInfo.mLastTrackNo < 0) {
		ret = readFDXDiskInfo(cmd, filename);
		if (!ret) {
			return false;
		}
	}

	// トラック範囲チェック
	if ((trackno < 0) || ((int)mDiskInfo.TrackSize() <= trackno)) {
		FDS_ERROR("invalid trackno number %d\n", trackno);
		return false;
	}

	// トラックが読み込み済みなら即終了
	if (mDiskInfo.isTrackReady(trackno)) {
		mDiskInfo.mLastTrackNo = trackno;
		return true;
	}

	// ログファイルを作成する
	char tmp[FDX_FILENAME_MAX+1];
	char opt[256];
	int heads = mDiskInfo.mFdxInfo.mHeads;
	if (heads == 0) heads = 2;
	int cylinder = trackno / heads;
	int head = trackno % heads;
	sprintf(opt, "-v sector -c %d -h %d", cylinder, head);
	ret = execFdxViewAnalyze(tmp, cmd, filename, opt);
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
// fdxセクタ情報の読み取り
// -------------------------------------------------------------
bool
FdxView::readFDXSector(const std::string& cmd, const std::string& filename, int trackno, int sectorno)
{
	bool ret = true;
	char line[1024];

	// トラック範囲チェック
	if ((trackno < 0) || ((int)mDiskInfo.TrackSize() <= trackno)) {
		FDS_ERROR("invalid trackno=%d\n", trackno);
		return false;
	}

	// トラックが読み込み済みでなければまず読みに行く
	if (mDiskInfo.Track(trackno).mLastSectorNo < 0) {
		ret = readFDXTrack(cmd, filename, trackno);
		if (!ret) {
			return false;
		}
	}

	// セクタ範囲チェック
	if ((sectorno < 0) || ((int)(mDiskInfo.Track(trackno).SectorSize()) <= sectorno)) {
		FDS_ERROR("invalid trackno=%d, sectorno=%d\n", trackno, sectorno);
		return false;
	}

	// セクタが読み込み済みなら即終了
	if (mDiskInfo.isSectorReady(trackno, sectorno)) {
		mDiskInfo.mLastTrackNo = trackno;
		mDiskInfo.Track(trackno).mLastSectorNo = sectorno;
		return true;
	}

	// ログファイルを作成する
	char tmp[FDX_FILENAME_MAX+1];
	char opt[256];
	int heads = mDiskInfo.mFdxInfo.mHeads;
	if (heads == 0) heads = 2;
	int cylinder = trackno / heads;
	int head = trackno % heads;
	sprintf(opt, "-v data -c %d -h %d -s %d", cylinder, head, sectorno+1);
	ret = execFdxViewAnalyze(tmp, cmd, filename, opt);
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
	int linestatus = 0;
	char* s = fgets(line, sizeof(line), fin);
	while (s != nullptr) {
		linestatus = readFDXDiskInfoHeader(mDiskInfo, s);
		if (linestatus < 0) break;
		s = fgets(line, sizeof(line), fin);
	}

	if (linestatus < 0) {
		while (s != nullptr) {
			linestatus = readFDXDiskInfoDump(mDiskInfo, s);
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
