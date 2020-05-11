// ◇
// fds: fdx68 selector
// FddEmu: FddEmuの操作
// Copyright: (C)2020 Hiroaki GOTO as GORRY.
// License: see readme.txt
// =====================================================================

#include "FddEmu.h"

// =====================================================================
// FddEmuの操作
// =====================================================================

#define FDDEMU_LOGFILENAME "fddemu.log"

// -------------------------------------------------------------
// コンストラクタ
// -------------------------------------------------------------
FddEmu::FddEmu()
{
	for (int i=0; i<Drives; i++) {
		mStatus[i] = {};
		mStatus[i].mFileName = "EMPTY";
	}
}

// -------------------------------------------------------------
// FddEmuの起動コマンドライン設定
// -------------------------------------------------------------
void
FddEmu::setFddEmuCmd(const std::string& cmd)
{
	mFddEmuCmd = cmd;
}

// -------------------------------------------------------------
// FddEmuの起動コマンドラインオプション設定
// -------------------------------------------------------------
void
FddEmu::setFddEmuOption(const std::string& option)
{
	mFddEmuOption = option;
}

// -------------------------------------------------------------
// No rootモードの設定
// -------------------------------------------------------------
void
FddEmu::setNoRoot(bool sw)
{
	mNoRoot = sw;
}

// -------------------------------------------------------------
// FddEmuの起動
// -------------------------------------------------------------
int
FddEmu::run()
{
#if !defined(FDS_NOROOT)

	// NoRootモードならFddEmuの起動は外部に任せる
	FDS_LOG("noRoot=%d\n", (int)mNoRoot);
	if (mNoRoot) {
		return 0;
	}

	// スレッドの生成
	pid_t pid = fork();
	if (pid < 0) {
		fprintf(stderr, "cannot fork()");
		return -1;
	}

	// 生成先スレッドの実行内容
	if (pid == 0) {
		if (FDSLog::getOutputLog()) {
			// 起動したFddEmuのstdoutとstderrをfddemu.logへリダイレクト
			FILE* flog = fopen(FDDEMU_LOGFILENAME, "a+");
			if (flog) {
				dup2(fileno(flog), STDOUT_FILENO);
				dup2(fileno(flog), STDERR_FILENO);
				fclose(flog);
			}
		} else {
			close(STDOUT_FILENO);
			close(STDERR_FILENO);
		}

		// FddEmuを起動
		std::vector<const char*> argv = makeArgv(mFddEmuCmd, mFddEmuOption);
		FDS_LOG("cmd=[%s], option=[%s]\n", mFddEmuCmd.c_str(), mFddEmuOption.c_str());
		execvp(mFddEmuCmd.c_str(), (char* const*)&argv[0]);

		// 起動
		exit(1);
		return 0;
	}

	// pidを覚えておく
	mPid = pid;

	// 起動したFddEmuから情報取得ができるようになるまで待つ
	FDS_LOG("pid=%d, wait...\n", mPid);
	while (!0) {
		bool ret = updateStatus();
		if (ret) break;
		sleep(1);
	}
	FDS_LOG("pid=%d, connected\n", mPid);

#endif  // !defined(FDS_NOROOT)

	return 0;
}

// -------------------------------------------------------------
// FddEmuの終了
// -------------------------------------------------------------
void
FddEmu::kill()
{
#if !defined(FDS_NOROOT)
	// NoRootモードならFddEmuのKillは外部に任せる
	if (mNoRoot) {
		return;
	}

	if (mPid == 0) {
		// fprintf(stderr, "not fork()");
		return;
	}
	::kill(mPid, SIGHUP);
	mPid = 0;
#endif  // !defined(FDS_NOROOT)
}

// -------------------------------------------------------------
// 起動コマンドライン文字列からArgvを生成する
// -------------------------------------------------------------
std::vector<const char*>
FddEmu::makeArgv(const std::string& cmd, const std::string& option)
{
	mArgv.clear();
	mArgv.push_back(cmd.c_str());

	int mode = 0;
	char c = 0;
	char c2 = 0;
	const char* p = option.c_str();
	std::string arg;
	while ((c = *(p++)) != '\0') {
		switch (mode) {
		  default:
		  case 0:
			if ((c == ' ') || (c == '\t')) {
				continue;
			}
			if (c == '\\') {
				c = *(p++);
				if (c == '\0') goto finish;
				arg.push_back(c);
				mode = 1;
				continue;
			}
			if ((c == '\'') || (c == '\"')) {
				arg.push_back(c);
				c2 = c;
				mode = 2;
				continue;
			}
			arg.push_back(c);
			mode = 1;
			continue;
		  case 1:
			if ((c == ' ') || (c == '\t')) {
				mArgv.push_back(arg);
				arg.clear();
				mode = 0;
				continue;
			}
			if (c == '\\') {
				c = *(p++);
				if (c == '\0') goto finish;
				arg.push_back(c);
				continue;
			}
			if ((c == '\'') || (c == '\"')) {
				arg.push_back(c);
				c2 = c;
				mode = 2;
				continue;
			}
			mode = 1;
			arg.push_back(c);
			continue;
		  case 2:
			if (c == '\\') {
				c = *(p++);
				if (c == '\0') goto finish;
				arg.push_back(c);
				continue;
			}
			if (c == c2) {
				arg.push_back(c);
				mode = 1;
				continue;
			}
			arg.push_back(c);
			continue;
		}

	  finish:;
		break;
	}
	if (!arg.empty()) {
		mArgv.push_back(arg);
	}

	std::vector<const char*> argv;
	for (int i=0; i<(int)mArgv.size(); i++) {
		argv.push_back(mArgv[i].c_str());
	}
	argv.push_back(nullptr);

	return argv;
}

// -------------------------------------------------------------
// FddEmuステータス表示コマンドの実行
// -------------------------------------------------------------
bool
FddEmu::updateStatus()
{
	bool ret = sendCommand("list\n", true);
	return ret;
}

// -------------------------------------------------------------
// ディスクイメージ挿入コマンドの実行
// -------------------------------------------------------------
bool
FddEmu::setImage(int id, const std::string& filename)
{
#if !defined(FDS_WINDOWS)
	char buf[FDX_FILENAME_MAX];
	sprintf(buf, "%d %d %s\n", id, (int)Command::Insert, filename.c_str());
	FDS_LOG("setImage: send [%s]\n", buf);
	bool ret = sendCommand(buf, false);
	if (ret == 0) {
		mDiskPath[id] = filename;
	}
	return ret;
#else
	mStatus[id].mInsert = true;
	std::wstring wfilename = WStrUtil::str2wstr(filename);
	int pos = (int)wfilename.find_last_of(L'/');
	std::wstring wfilename2;
	WStrUtil::copyN(wfilename2, wfilename, pos);
	mStatus[id].mFileName = WStrUtil::wstr2str(wfilename2);
	mDiskPath[id] = filename;
	return true;
#endif  // !defined(FDS_WINDOWS)
}

// -------------------------------------------------------------
// ディスク排出コマンドの実行
// -------------------------------------------------------------
bool
FddEmu::ejectDrive(int id)
{
#if !defined(FDS_WINDOWS)
	char buf[FDX_STRING_MAX];
	sprintf(buf, "%d %d %s\n", id, (int)Command::Eject, "-");
	FDS_LOG("ejectDrive: send [%s]\n", buf);
	bool ret = sendCommand(buf, false);
	mDiskPath[id].clear();
	return ret;
#else
	mStatus[id].mInsert = false;
	mDiskPath[id].clear();
	return true;
#endif  // !defined(FDS_WINDOWS)
}

// -------------------------------------------------------------
// プロテクトスイッチ設定コマンドの実行
// -------------------------------------------------------------
bool
FddEmu::protectDrive(int id)
{
#if !defined(FDS_WINDOWS)
	char buf[FDX_STRING_MAX];
	sprintf(buf, "%d %d %s\n", id, (int)Command::Protect, "-");
	FDS_LOG("protectDrive: send [%s]\n", buf);
	bool ret = sendCommand(buf, false);
	return ret;
#else
	mStatus[id].mProtect = !mStatus[id].mProtect;
	return true;
#endif  // !defined(FDS_WINDOWS)
}

// -------------------------------------------------------------
// 外部コマンドの実行
// -------------------------------------------------------------
bool
FddEmu::execCmd(const std::string& cmd, const std::string& option)
{
	// 外部コマンドを起動
	std::string cmdline = cmd + " " + option;
	cmdline += ">>" FDS_LOGFILENAME " 2>&1";
	FDS_LOG("execCmd: [%s]\n", cmdline.c_str());
	int ret = system(cmdline.c_str());
	FDS_LOG("execCmd: ret=%d\n", ret);

	return ret;
}

// -------------------------------------------------------------
// FddEmuに対するコマンドの実行
// -------------------------------------------------------------
bool
FddEmu::sendCommand(const std::string& command, bool getStatus)
{
#if !defined(FDS_WINDOWS)
#if 0
	if (mPid == 0) {
		FDS_LOG("sendCommand: not fork FddEmu\n");
		mErrNo = ErrNo::NotFork;
		return false;
	}
#endif

	int fd;
	struct sockaddr_in server;
	FILE *fp;

	fd = socket(PF_INET, SOCK_STREAM, 0);
	memset(&server, 0, sizeof(server));
	server.sin_family = PF_INET;
	server.sin_port   = htons(FDDEMU_PORTNO);
	server.sin_addr.s_addr = htonl(INADDR_LOOPBACK); 

	if (connect(fd, (struct sockaddr *)&server,
		sizeof(struct sockaddr_in)) < 0) {
		if (!getStatus) {
			FDS_LOG("sendCommand: Can't connect to FddEmu process\n");
		}
		mErrNo = ErrNo::NotConnect;
		return false;
	}

	fp = fdopen(fd, "r+");
	setvbuf(fp, NULL, _IONBF, 0);
	fprintf(fp, "%s", command.c_str());

	while (1) {
		char buf[1024];
		if (fgets((char *)buf, sizeof(buf), fp) == NULL) {
			break;
		}
		if (getStatus) {
			if ((buf[2] == '0') || (buf[2] == '1')) {
				int id = atoi(&buf[2]);
				mStatus[id].mProtect = (buf[6] == 'O');
				mStatus[id].mCluster = atoi(&buf[11]);
				buf[strlen(buf)-1] = '\0';
				mStatus[id].mFileName = &buf[16];
				mStatus[id].mInsert = true;
				if (mStatus[id].mFileName == "EMPTY") {
					mStatus[id].mInsert = false;
				}
				// FDS_LOG("sendCommand: ID=%d, WP=%d, CL=%02d, NAME=[%s]\n", id, (mStatus[id].mProtect ? 1 : 0), mStatus[id].mCluster, mStatus[id].mFileName.c_str());
			}
		}
	}

	fclose(fp);
	close(fd);

#else
	(void)command;
	(void)getStatus;
#endif  // !defined(FDS_WINDOWS)

	return true;
}

// =====================================================================
// [EOF]
