// ◇
// fds: fdx68 selector
// FdDump: FdDumpの操作
// Copyright: (C)2020 Hiroaki GOTO as GORRY.
// License: see readme.txt
// =====================================================================

#include "FdDump.h"

// =====================================================================
// FdDumpの操作
// =====================================================================

#define FDDUMP_LOGFILENAME "fddump.log"

// -------------------------------------------------------------
// コンストラクタ
// -------------------------------------------------------------
FdDump::FdDump()
{
	mCallbackFunc = nullptr;
}

// -------------------------------------------------------------
// FdDumpの起動コマンドライン設定
// -------------------------------------------------------------
void
FdDump::setCmd(const std::string& cmd)
{
	mCmd = cmd;
}

// -------------------------------------------------------------
// FdDumpの起動コマンドラインオプション設定
// -------------------------------------------------------------
void
FdDump::setOption(const std::string& option)
{
	mOption = option;
}

// -------------------------------------------------------------
// FdDumpのディスク名設定
// -------------------------------------------------------------
void
FdDump::setDiskName(const std::string& name)
{
	mStatus.mName = name;
}

// -------------------------------------------------------------
// FdDumpのフォーマット名設定
// -------------------------------------------------------------
void
FdDump::setFormatName(const std::string& name)
{
	mStatus.mFormat = name;
}

// -------------------------------------------------------------
// FdDumpの入力ファイル名設定
// -------------------------------------------------------------
void
FdDump::setFileName(const std::string& name)
{
	mStatus.mFileName = name;
}

// -------------------------------------------------------------
// FdDumpのシリンダ数設定
// -------------------------------------------------------------
void
FdDump::setCylinders(int num)
{
	mStatus.mCylinders = num;
}

// -------------------------------------------------------------
// FdDumpのヘッド数設定
// -------------------------------------------------------------
void
FdDump::setHeads(int num)
{
	mStatus.mHeads = num;
}

// -------------------------------------------------------------
// FdDumpのステップ数設定
// -------------------------------------------------------------
void
FdDump::setSteps(int num)
{
	mStatus.mSteps = num;
}

// -------------------------------------------------------------
// FdDumpのコールバック設定
// -------------------------------------------------------------
void
FdDump::setCallback(Callback* func, void* param)
{
	mCallbackFunc = func;
	mCallbackParam = param;
}

// -------------------------------------------------------------
// FdDumpの起動
// -------------------------------------------------------------
int
FdDump::run()
{
	bool escape = false;

#if 0
	size_t pos;
	// オプションからC/H/Sを得ておく
	pos = mOption.find("-c");
	if (pos != std::string::npos) {
		mStatus.mCylinders = atoi(mOption.c_str()+pos+2);
	}
	pos = mOption.find("-h");
	if (pos != std::string::npos) {
		mStatus.mHeads = atoi(mOption.c_str()+pos+2);
	}
	pos = mOption.find("-s");
	if (pos != std::string::npos) {
		mStatus.mSteps = atoi(mOption.c_str()+pos+2);
	}
#endif
	mStatus.mTracks = mStatus.mCylinders * mStatus.mHeads;
	if (mStatus.mTracks > MAX_TRACKS) {
		mStatus.mTracks = MAX_TRACKS;
	}

	// ステータスクリア
	for (int i=0; i<mStatus.mTracks; i++) {
		mStatus.mStatus[i] = TrackStatus::None;
		mStatus.mChanged[i] = 0;
	}
	mStatus.mNowTrack = -1;
	mStatus.mFinished = false;
	clearLogLine();

	// 最初のコールバック発行
	if (mCallbackFunc) {
		mCallbackFunc(mStatus, mCallbackParam);
	}

	// FdDumpコマンドライン作成
	char opt[FDX_FILENAME_MAX];
	sprintf(opt, "%s -c%d -h%d -s%d \"%s\"", mOption.c_str(), mStatus.mCylinders, mStatus.mHeads, mStatus.mSteps, mStatus.mFileName.c_str());
	std::vector<const char*> argv = makeArgv(mCmd, opt, mArgv);
	FDS_LOG("cmd=[%s], option=[%s]\n", mCmd.c_str(), opt);

#if !defined(FDS_WINDOWS)
	// 出力をパイプに差し替える準備
	int err = pipe2(mPipe, O_NONBLOCK);
	if (err < 0) {
		FDS_ERROR("cannot open pipe");
		return -1;
	}
	FDS_LOG("pipe: 0=%d, 1=%d\n", mPipe[0], mPipe[1]);

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
		close(mPipe[0]);
		dup2(mPipe[1], STDOUT_FILENO);
		dup2(mPipe[1], STDERR_FILENO);
		close(mPipe[1]);

		// FdDumpを起動
		FDS_LOG("child: run cmd=[%s], option=[%s]\n", mCmd.c_str(), opt);
		err = execvp(mCmd.c_str(), (char* const*)&argv[0]);
		if (err != 0) {
			FDS_ERROR("child: error %d: cmd=[%s], option=[%s]\n", mCmd.c_str(), opt);
		}

		// 起動
		FDS_LOG("child: exit\n");
		exit(1);
		return 0;
	}

	// 生成元スレッドの実行内容

	// 不要なハンドルは閉じる
	close(mPipe[1]);

	// pidを覚えておく
	mPid = pid;

	// 起動したFdDumpの終了まで待つ
	FDS_LOG("parent: wait finished: pid=%d\n", mPid);
	while (!mStatus.mFinished) {
		// 経過のコールバック発行
		bool b = updateStatus();
		FDS_LOG("updateStatus: ret=%d\n", (int)b);
		if (!b) break;
		if (mCallbackFunc) {
			int ret = mCallbackFunc(mStatus, mCallbackParam);
			if (ret < 0) {
				// 中止
				escape = true;
				::kill(mPid, SIGHUP);
				// mStatus.mFinished = true;
			}
		}

		// ウェイト
		usleep(200*1000);
	}

#else

	// ダミーのダンプ
	for (int i=0; i<mStatus.mTracks; i++) {
		// 経過のコールバック発行
		mStatus.mStatus[i] = TrackStatus::Finish;
		mStatus.mChanged[i] = 1;
		if (mCallbackFunc) {
			int ret = mCallbackFunc(mStatus, mCallbackParam);
			if (ret < 0) {
				// 中止
				escape = true;
				mStatus.mFinished = true;
				break;
			}
		}

		// ウェイト
		Sleep(200);
	}

	// ダミーのダンプ終了
	mStatus.mFinished = true;

#endif  // !defined(FDS_WINDOWS)

	if (escape) {
		return -1;
	}

	// エラートラック数を勘定して返す
	int errorcount = 0;
	for (int i=0; i<mStatus.mTracks; i++) {
		if (mStatus.mStatus[i] == TrackStatus::Error) {
			errorcount++;
		}
	}

	return errorcount;
}

// -------------------------------------------------------------
// FdDumpのステータス確認
// -------------------------------------------------------------
bool
FdDump::updateStatus()
{
	bool ret = recvStatus();
	return ret;
}

// -------------------------------------------------------------
// FdDump出力の解析
// -------------------------------------------------------------
void
FdDump::analyzeLogLine(void)
{
	FDS_LOG("analyze: [%s]\n", mLineBuf.c_str());
	size_t pos1;

	// "Done"のときの処理
	pos1 = mLineBuf.find("Done");
	if (std::string::npos != pos1) {
		FDS_LOG("analyze: found [Done] on %d\n", (int)pos1);
		if (mStatus.mNowTrack >= 0) {
			if (mStatus.mStatus[mStatus.mNowTrack] == TrackStatus::None) {
				// 前のトラックが完了前なら完了にする
				FDS_LOG("analyze: Processing Track %d is Finished\n", mStatus.mNowTrack);
				mStatus.mStatus[mStatus.mNowTrack] = TrackStatus::Finish;
				mStatus.mChanged[mStatus.mNowTrack] = 1;
			}
		}
	}

	// "Processing"のときの処理
	pos1 = mLineBuf.find("Processing");
	if (std::string::npos != pos1) {
		FDS_LOG("analyze: found [Processing] on %d\n", (int)pos1);
		size_t pos2 = mLineBuf.find("(T", pos1);
		if (std::string::npos != pos2) {
			// Processing         :   0%(T  0 C 0 H0)
			// トラック番号を得る
			int nowtrack = atoi(mLineBuf.c_str()+pos2+2);
			FDS_LOG("analyze: found [Processing] Track on %d: %d\n", (int)pos2+2, nowtrack);

			// 前のトラックのステータスが処理中なら完了にする
			if (mStatus.mNowTrack != nowtrack) {
				if (mStatus.mNowTrack >= 0) {
					if (mStatus.mStatus[mStatus.mNowTrack] == TrackStatus::None) {
						FDS_LOG("analyze: Processing Track %d is Finished\n", mStatus.mNowTrack);
						mStatus.mStatus[mStatus.mNowTrack] = TrackStatus::Finish;
						mStatus.mChanged[mStatus.mNowTrack] = 1;
					}
				}
			}

			// 処理トラック番号を更新
			mStatus.mNowTrack = nowtrack;

			// アンフォーマットが出力されたときのステータス処理
			size_t pos3 = mLineBuf.find("Warning : Damaged or unformated", pos2);
			if (std::string::npos != pos3) {
				FDS_LOG("analyze: Processing Track %d is Unformatted\n", mStatus.mNowTrack);
				mStatus.mStatus[mStatus.mNowTrack] = TrackStatus::Unformat;
				mStatus.mChanged[mStatus.mNowTrack] = 1;
				return;
			}

			// エラーが出力されたときのステータス処理
			pos3 = mLineBuf.find("Warning : Damaged or protected", pos2);
			if (std::string::npos != pos3) {
				FDS_LOG("analyze: Processing Track %d is Error\n", mStatus.mNowTrack);
				mStatus.mStatus[mStatus.mNowTrack] = TrackStatus::Error;
				mStatus.mChanged[mStatus.mNowTrack] = 1;
				return;
			}
		}
		return;
	}

	// "Retry"のときの処理
	pos1 = mLineBuf.find("Retry");
	if (std::string::npos != pos1) {
		//  Retry   1/  3     :  95%(T155 C77 H1)
		// リトライ回数を得る
		int nowretry = atoi(mLineBuf.c_str()+pos1+5);
		FDS_LOG("analyze: found [Retry] on %d: count=%d\n", (int)pos1, nowretry);

		// リトライ回数が1～9ならその回数をステータスに
		if ((nowretry > 0) && (nowretry < 10)) {
			mStatus.mStatus[mStatus.mNowTrack] = (TrackStatus)((int)TrackStatus::None+nowretry);
			mStatus.mChanged[mStatus.mNowTrack] = 1;
		}

		// アンフォーマットが出力されたときのステータス処理
		size_t pos2 = mLineBuf.find("Warning : Damaged or unformated", pos1);
		if (std::string::npos != pos2) {
			FDS_LOG("analyze: Retry Track %d is Unformatted\n", mStatus.mNowTrack);
			mStatus.mStatus[mStatus.mNowTrack] = TrackStatus::Unformat;
			mStatus.mChanged[mStatus.mNowTrack] = 1;
			return;
		}

		// エラーが出力されたときのステータス処理
		pos2 = mLineBuf.find("Warning : Damaged or protected", pos1);
		if (std::string::npos != pos2) {
			FDS_LOG("analyze: Retry Track %d is Error\n", mStatus.mNowTrack);
			mStatus.mStatus[mStatus.mNowTrack] = TrackStatus::Error;
			mStatus.mChanged[mStatus.mNowTrack] = 1;
			return;
		}

		return;
	}
}

// -------------------------------------------------------------
// FdDump出力のログ行出力
// -------------------------------------------------------------
void
FdDump::putLogLine(void)
{
	// fddumpの出力をfddump.logへ出力
	FDS_LOG("putlog: [%s]\n", mLineBuf.c_str());
	if (FDSLog::getOutputLog()) {
		FILE* flog = fopen(FDDUMP_LOGFILENAME, "a+");
		if (flog) {
			fprintf(flog, "%s\n", mLineBuf.c_str());
			fclose(flog);
		}
	}
}

// -------------------------------------------------------------
// FdDump出力のログ行クリア
// -------------------------------------------------------------
void
FdDump::clearLogLine(void)
{
	FDS_LOG("clearlog: [%s]\n", mLineBuf.c_str());
	mLineBuf.clear();
	mLogMode = LogMode::None;
}

// -------------------------------------------------------------
// FdDumpに対するステータスの受け取り
// -------------------------------------------------------------
bool
FdDump::recvStatus(void)
{
	FDS_LOG("recvStatus: start\n");
#if !defined(FDS_WINDOWS)
#if 0
	if (mPid == 0) {
		FDS_LOG("sendCommand: not fork FdDump\n");
		return false;
	}
#endif

	while (!0) {

		// fddumpの出力を読み込み
		char buf[1024];
		memset(buf, 0, sizeof(buf));
		ssize_t size = read(mPipe[0], &buf, sizeof(buf));

		// fddumpが終了していたらquit
		if (size <= 0) {
			int st;
			if (waitpid(mPid, &st, WNOHANG)) {
				if (WIFEXITED(st) || WIFSIGNALED(st)) {
					break;
				}
			}
		}

		// 出力がなければ繰り返し
		if (size == 0) {
			usleep(1000*100);
			continue;
		}

		// エラーなら適宜処理
		if (size < 0) {
			switch (errno) {
			  case EAGAIN:
				usleep(1000*100);
				continue;

			  default:
				return false;
			}
		}

		// fddumpの出力を解析
		for (ssize_t i=0; i<size; i++) {
		  retry:;
			int c = (int)(unsigned char)buf[i];
			switch (mLogMode) {
			  default:
			  case LogMode::None:
				switch (c) {
				  default:
					goto output;
				  case 0x1b:
					mLogMode = LogMode::ESC1;
					continue;
				  case 0x0d:
					mLogMode = LogMode::CR;
					continue;
				}
				continue;

			  case LogMode::ESC1:
				mLogMode = LogMode::ESC2;
				continue;

			  case LogMode::ESC2:
				mLogMode = LogMode::ESC3;
				continue;

			  case LogMode::ESC3:
				if (c == 'G') {
					c = 0x0a;
					goto output;
				}
				mLogMode = LogMode::None;
				continue;

			  case LogMode::CR:
				analyzeLogLine();
				putLogLine();
				clearLogLine();
				if (c == 0x0d) {
					continue;
				}
				goto retry;
			}

		  output:;
			mLogMode = LogMode::None;
			if (c == 0x0a) {
				analyzeLogLine();
				putLogLine();
				clearLogLine();
				continue;
			}
			mLineBuf.push_back(c);
		}

		return true;
	}

#else

#endif  // !defined(FDS_WINDOWS)

	mStatus.mFinished = true;

	return true;
}

// =====================================================================
// [EOF]
