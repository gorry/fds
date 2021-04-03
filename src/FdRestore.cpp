// ◇
// fds: fdx68 selector
// FdRestore: FdRestoreの操作
// Copyright: (C)2020 Hiroaki GOTO as GORRY.
// License: see readme.txt
// =====================================================================

#include "FdRestore.h"

// =====================================================================
// FdRestoreの操作
// =====================================================================

#define FDRESTORE_LOGFILENAME "fdrestore.log"

// -------------------------------------------------------------
// コンストラクタ
// -------------------------------------------------------------
FdRestore::FdRestore()
{
	mCallbackFunc = nullptr;
}

// -------------------------------------------------------------
// FdRestoreの起動コマンドライン設定
// -------------------------------------------------------------
void
FdRestore::setCmd(const std::string& cmd)
{
	mCmd = cmd;
}

// -------------------------------------------------------------
// FdRestoreの起動コマンドラインオプション設定
// -------------------------------------------------------------
void
FdRestore::setOption(const std::string& option)
{
	mOption = option;
}

// -------------------------------------------------------------
// FdRestoreの起動コマンドライン設定
// -------------------------------------------------------------
void
FdRestore::setAnalyzeCmd(const std::string& cmd)
{
	mAnalyzeCmd = cmd;
}

// -------------------------------------------------------------
// FdRestoreの起動コマンドラインオプション設定
// -------------------------------------------------------------
void
FdRestore::setAnalyzeOption(const std::string& option)
{
	mAnalyzeOption = option;
}

// -------------------------------------------------------------
// FdRestoreのディスク名設定
// -------------------------------------------------------------
void
FdRestore::setDiskName(const std::string& name)
{
	mStatus.mName = name;
}

// -------------------------------------------------------------
// FdRestoreのフォーマット名設定
// -------------------------------------------------------------
void
FdRestore::setFormatName(const std::string& name)
{
	mStatus.mFormat = name;
}

// -------------------------------------------------------------
// FdRestoreの入力ファイル名設定
// -------------------------------------------------------------
void
FdRestore::setFileName(const std::string& name)
{
	mStatus.mFileName = name;
}

// -------------------------------------------------------------
// FdRestoreのシリンダ数設定
// -------------------------------------------------------------
void
FdRestore::setCylinders(int num)
{
	mStatus.mCylinders = num;
}

// -------------------------------------------------------------
// FdRestoreのヘッド数設定
// -------------------------------------------------------------
void
FdRestore::setHeads(int num)
{
	mStatus.mHeads = num;
}

// -------------------------------------------------------------
// FdRestoreのステップ数設定
// -------------------------------------------------------------
void
FdRestore::setSteps(int num)
{
	mStatus.mSteps = num;
}

// -------------------------------------------------------------
// FdRestoreのコールバック設定
// -------------------------------------------------------------
void
FdRestore::setCallback(Callback* func, void* param)
{
	mCallbackFunc = func;
	mCallbackParam = param;
}

// -------------------------------------------------------------
// FdRestoreの起動
// -------------------------------------------------------------
int
FdRestore::run()
{
	bool escape = false;
	mErrNo = ErrNo::None;

	// オプションからC/H/Sを得ておく
#if 0
	size_t pos;
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
	mStatus.mNowTrack = -1;
	mStatus.mFinished = false;
	clearLogLine();

	// 最初のコールバック発行
	if (mCallbackFunc) {
		mCallbackFunc(mStatus, mCallbackParam);
	}

	// FdRestoreコマンドライン作成
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
		// 起動したFdRestoreのstdoutとstderrを親へリダイレクト
		FDS_LOG("child: dup to pipe\n");
		close(mPipe[0]);
		dup2(mPipe[1], STDOUT_FILENO);
		dup2(mPipe[1], STDERR_FILENO);
		close(mPipe[1]);

		// FdRestoreを起動
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

	// 起動したFdRestoreの終了まで待つ
	FDS_LOG("parent: wait finished\n");
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

	// ダミーのステータスをセット
	for (int i=0; i<mStatus.mTracks; i++) {
		mStatus.mStatus[i] = TrackStatus::Finish;
		mStatus.mChanged[i] = 1;
	}

	// ダミーのリストア
	for (int i=0; i<mStatus.mTracks; i++) {
		// 経過のコールバック発行
		mStatus.mStatus[i] = TrackStatus::None;
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

	// ダミーのリストア終了
	mStatus.mFinished = true;

#endif  // !defined(FDS_WINDOWS)

	if (escape) {
		return -1;
	}
	if (mErrNo != ErrNo::None) {
		return (int)mErrNo;
	}

	return 0;
}

// -------------------------------------------------------------
// FdRestoreのステータス確認
// -------------------------------------------------------------
bool
FdRestore::updateStatus()
{
	bool ret = recvStatus();
	return ret;
}

// -------------------------------------------------------------
// FdRestore出力の解析
// -------------------------------------------------------------
void
FdRestore::analyzeLogLine(void)
{
	FDS_LOG("analyze: [%s]\n", mLineBuf.c_str());
	size_t pos1;

	// "Done"のときの処理
	pos1 = mLineBuf.find("Done");
	if (0 == pos1) {
		FDS_LOG("analyze: found [Done] on %d\n", (int)pos1);
		if (mStatus.mNowTrack >= 0) {
			if (mStatus.mStatus[mStatus.mNowTrack] != TrackStatus::None) {
				// 前のトラックが完了前なら完了にする
				FDS_LOG("analyze: Processing Track %d is Finished\n", mStatus.mNowTrack);
				mStatus.mStatus[mStatus.mNowTrack] = TrackStatus::None;
				mStatus.mChanged[mStatus.mNowTrack] = 1;
			}
		}
	}

	// "Processing"のときの処理
	pos1 = mLineBuf.find("Processing");
	if (0 == pos1) {
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
					if (mStatus.mStatus[mStatus.mNowTrack] != TrackStatus::None) {
						FDS_LOG("analyze: Processing Track %d is Finished\n", mStatus.mNowTrack);
						mStatus.mStatus[mStatus.mNowTrack] = TrackStatus::None;
						mStatus.mChanged[mStatus.mNowTrack] = 1;
					}
				}
			}

			// 処理トラック番号を更新
			mStatus.mNowTrack = nowtrack;
		}
		return;
	}

	pos1 = mLineBuf.find("Write protect");
	if (0 == pos1) {
		FDS_LOG("analyze: found [Write protect] on %d\n", (int)pos1);
		size_t pos2 = mLineBuf.find("Error", pos1);
		if (std::string::npos != pos2) {
			// Write protect      : Error : Disk is protected
			mErrNo = ErrNo::WriteProtect;
			return;
		}
	}

	pos1 = mLineBuf.find("Index period");
	if (0 == pos1) {
		FDS_LOG("analyze: found [Index period] on %d\n", (int)pos1);
		size_t pos2 = mLineBuf.find("Error", pos1);
		if (std::string::npos != pos2) {
			// Index period       : Error : Drive not ready
			mErrNo = ErrNo::NotReady;
			return;
		}
	}
}

// -------------------------------------------------------------
// FdRestore出力のログ行出力
// -------------------------------------------------------------
void
FdRestore::putLogLine(void)
{
	// fddumpの出力をfddump.logへ出力
	FDS_LOG("putlog: [%s]\n", mLineBuf.c_str());
	if (FDSLog::getOutputLog()) {
		FILE* flog = fopen(FDRESTORE_LOGFILENAME, "a+");
		if (flog) {
			fprintf(flog, "%s\n", mLineBuf.c_str());
			fclose(flog);
		}
	}
}

// -------------------------------------------------------------
// FdRestore出力のログ行クリア
// -------------------------------------------------------------
void
FdRestore::clearLogLine(void)
{
	FDS_LOG("clearlog: [%s]\n", mLineBuf.c_str());
	mLineBuf.clear();
	mLogMode = LogMode::None;
}

// -------------------------------------------------------------
// FdRestoreに対するステータスの受け取り
// -------------------------------------------------------------
bool
FdRestore::recvStatus(void)
{
#if !defined(FDS_WINDOWS)
#if 0
	if (mPid == 0) {
		FDS_LOG("sendCommand: not fork FdRestore\n");
		return false;
	}
#endif

	while (!0) {

		// fdrestoreの出力を読み込み
		char buf[1024];
		memset(buf, 0, sizeof(buf));
		ssize_t size = read(mPipe[0], &buf, sizeof(buf));

		// fdrestoreが終了していたらquit
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

		// fdrestoreの出力を解析
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
		if (mErrNo != ErrNo::None) {
			return false;
		}

		return true;
	}

#else

#endif  // !defined(FDS_WINDOWS)

	mStatus.mFinished = true;

	return true;
}

// =====================================================================

// -------------------------------------------------------------
// Fdxファイルの事前解析
// -------------------------------------------------------------
int
FdRestore::analyze()
{
	bool escape = false;

	// ステータスクリア
	for (int i=0; i<mStatus.mTracks; i++) {
		mStatus.mStatus[i] = TrackStatus::None;
		mStatus.mChanged[i] = 0;
	}
	mStatus.mNowTrack = -1;
	mStatus.mFinished = false;
	clearLogLine();

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
		// 起動したFdRestoreのstdoutとstderrを親へリダイレクト
		FDS_LOG("child: dup to pipe\n");
		close(mPipe[0]);
		dup2(mPipe[1], STDOUT_FILENO);
		dup2(mPipe[1], STDERR_FILENO);
		close(mPipe[1]);

		// FdRestoreを起動
		std::vector<const char*> argv = makeArgv(mAnalyzeCmd, mAnalyzeOption, mArgv);
		FDS_LOG("child: run cmd=[%s], option=[%s]\n", mAnalyzeCmd.c_str(), mAnalyzeOption.c_str());
		execvp(mAnalyzeCmd.c_str(), (char* const*)&argv[0]);

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

	// 起動したFdRestoreの終了まで待つ
	FDS_LOG("parent: wait finished\n");
	while (!mStatus.mFinished) {
		// 経過のコールバック発行
		bool b = updateAnalyzeStatus();
		FDS_LOG("updateAnalyzeStatus: ret=%d\n", (int)b);
		if (!b) break;
	}

#else

	// ダミーのリストア
	for (int i=0; i<mStatus.mTracks; i++) {
		// 経過のコールバック発行
		mStatus.mStatus[i] = TrackStatus::Finish;
		mStatus.mChanged[i] = 1;
	}

	// ダミーのリストア終了
	mStatus.mFinished = true;

#endif  // !defined(FDS_WINDOWS)

	if (escape) {
		return -1;
	}

	// ステータスのバックアップをとっておく
	for (int i=0; i<mStatus.mTracks; i++) {
		mStatus.mStatus_Backup[i] = mStatus.mStatus[i];
		mStatus.mChanged_Backup[i] = mStatus.mChanged[i];
	}

	return 0;
}

// -------------------------------------------------------------
// FdxAnalyzeのステータスの変更を戻す
// -------------------------------------------------------------
void
FdRestore::revertAnalyzeStatus()
{
	// ステータスのバックアップをとっておく
	for (int i=0; i<mStatus.mTracks; i++) {
		mStatus.mStatus[i] = mStatus.mStatus_Backup[i];
		mStatus.mChanged[i] = mStatus.mChanged_Backup[i];
	}
}

// -------------------------------------------------------------
// FdxAnalyzeのステータス確認
// -------------------------------------------------------------
bool
FdRestore::updateAnalyzeStatus()
{
	bool ret = recvAnalyzeStatus();
	return ret;
}

// -------------------------------------------------------------
// FdxAnalyze出力の解析
// -------------------------------------------------------------
void
FdRestore::analyzeAnalyzeLogLine(void)
{
	FDS_LOG("analyze: [%s]\n", mLineBuf.c_str());
	size_t pos1;

	// "CYLINDER"のときの処理
	pos1 = mLineBuf.find("CYLINDER");
	if (std::string::npos != pos1) {
		FDS_LOG("analyze: found [CYLINDER] on %d\n", (int)pos1);
		int nowcylinder = atoi(mLineBuf.c_str()+pos1+8);
		size_t pos2 = mLineBuf.find("HEAD", pos1);
		if (std::string::npos != pos2) {
			// CYLINDER 00 HEAD 
			FDS_LOG("analyze: found [HEAD] on %d\n", (int)pos2);
			int nowhead = atoi(mLineBuf.c_str()+pos2+4);
			mStatus.mNowTrack = nowcylinder*2 + nowhead;
			size_t pos3 = mLineBuf.find("NORMAL", pos2);
			if (std::string::npos != pos3) {
				// CYLINDER 00 HEAD ... NORMAL
				mStatus.mStatus[mStatus.mNowTrack] = TrackStatus::Finish;
				mStatus.mChanged[mStatus.mNowTrack] = 1;
				return;
			}
			pos3 = mLineBuf.find("ILLEGAL", pos2);
			if (std::string::npos != pos3) {
				// CYLINDER 00 HEAD ... ILLEGAL
				mStatus.mStatus[mStatus.mNowTrack] = TrackStatus::Error;
				mStatus.mChanged[mStatus.mNowTrack] = 1;
				return;
			}
			pos3 = mLineBuf.find("UNFORMAT", pos2);
			if (std::string::npos != pos3) {
				// CYLINDER 00 HEAD ... UNFORMAT
				mStatus.mStatus[mStatus.mNowTrack] = TrackStatus::Unformat;
				mStatus.mChanged[mStatus.mNowTrack] = 1;
				return;
			}
		}
	}
}

// -------------------------------------------------------------
// FdxAnalyzeに対するステータスの受け取り
// -------------------------------------------------------------
bool
FdRestore::recvAnalyzeStatus(void)
{
#if !defined(FDS_WINDOWS)
#if 0
	if (mPid == 0) {
		FDS_LOG("sendCommand: not fork FdRestore\n");
		return false;
	}
#endif

	while (!0) {

		// FdxAnalyzeの出力を読み込み
		char buf[1024];
		memset(buf, 0, sizeof(buf));
		ssize_t size = read(mPipe[0], &buf, sizeof(buf));

		// FdxAnalyzeが終了していたらquit
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

		// FdxAnalyzeの出力を解析
		for (ssize_t i=0; i<size; i++) {
		  retry:;
			int c = (int)(unsigned char)buf[i];
			switch (mLogMode) {
			  default:
			  case LogMode::None:
				switch (c) {
				  default:
					goto output;
				  case 0x0d:
					mLogMode = LogMode::CR;
					continue;
				}
				continue;

			  case LogMode::CR:
				analyzeAnalyzeLogLine();
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
				analyzeAnalyzeLogLine();
				putLogLine();
				clearLogLine();
				continue;
			}
			mLineBuf.push_back(c);
		}
		if (mErrNo != ErrNo::None) {
			return false;
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
