// ◇
// fds: fdx68 selector
// FDSSystem: FDSシステム
// Copyright: (C)2020 Hiroaki GOTO as GORRY.
// License: see readme.txt
// =====================================================================

#include "FDSSystem.h"

// =====================================================================
// FDSSystem: FDSシステム
// =====================================================================

// -------------------------------------------------------------
// ルートディレクトリの設定
// -------------------------------------------------------------
void
FDSSystem::setRootDir(const std::string& rootDir)
{
	// 空の指定ならカレントに
	mRootDir = rootDir;
	if (mRootDir.empty()) {
		mRootDir = "./";
	}

	// 末尾に"/"を付けておく
	if (mRootDir[mRootDir.length()-1] != '/') {
		mRootDir += "/";
	}

	// カレントディレクトリは初期化しておく
	mCurDir = "";
}

// -------------------------------------------------------------
// カレントディレクトリの設定
// -------------------------------------------------------------
void
FDSSystem::setCurDir(const std::string& curDir)
{
	mCurDir = curDir;

	// 空でなければ末尾に"/"を付けておく
	if (!mCurDir.empty()) {
		if (mCurDir[mCurDir.length()-1] != '/') {
			mCurDir += "/";
		}
	}
}

// -------------------------------------------------------------
// No rootモードの設定
// -------------------------------------------------------------
void
FDSSystem::setNoRoot(bool sw)
{
	mNoRoot = sw;
	mFddEmu.setNoRoot(sw);
}

// -------------------------------------------------------------
// ウィンドウ表示開始
// -------------------------------------------------------------
void
FDSSystem::start()
{
#if !defined(FDS_WINDOWS)
	// fddemuチェック
	if (!mNoRoot) {
		FILE *fin = popen("pgrep fddemu", "r");
		char buf[FDX_FILENAME_MAX];
		fgets(buf, sizeof(buf), fin);
		pclose(fin);
		FDS_LOG("pgrep: %s\n", buf);
		if (atoi(buf) > 1) {
			FDS_ERROR("fddemu is already running. stop.\n");
			fprintf(stderr, "fddemu is already running. stop.\n");
			return;
		}
	}
#endif

	// 設定ファイル読み込み
	int err = loadIniFile();
	if (err < 0) {
		return;
	}

	// ウィンドウ位置算出
	initView();
	setViewLayout();
	drawHeader();

	// マシン選択肢択準備
	DlgSelect::ItemsVec items;
	int n = mConfig.numMachines();
	for (int i=0; i<n; i++) {
		items.push_back(mConfig.cfgMachine(i).name());
	}
	items.push_back("[ Quit ]");

	// マシン選択肢
	int sel = mConfig.machineNo();
	while (!0) {
		// ダイアログ表示
		DlgSelect dlg;
		dlg.setItemsVec(items);
		dlg.setHeader("[Select Machine]");
		sel = dlg.start(sel);

		// [Quit]を選んだら終了
		if (sel == (int)items.size()-1) {
			break;
		}

		// マシンを選択
		mConfig.setMachineNo(sel);

		// 選択した設定を読み込む
		setRootDir(mConfig.cfgMachine().rootDir());
		{
			// マシンのルートディレクトリが存在するか調べる
			std::string path = mRootDir+".";
#if defined(FDS_WINDOWS)
			struct _stat st;
			int ret = _stat(path.c_str(), &st);
#else
			struct stat st;
			int ret = lstat(path.c_str(), &st);
#endif
			if (ret < 0) {
				if (errno == ENOENT) {
					// ルートディレクトリが存在しなければ作成するか尋ねる
					FDS_ERROR("start: No System Root Directory!\n");
					FDS_ERROR(" path=[%s]\n", path.c_str());
					DlgSelect::ItemsVec items2;
					items2.push_back("Make "+mRootDir);
					items2.push_back("[ Quit ]");
					DlgSelect dlg2;
					dlg2.setItemsVec(items2);
					dlg2.setHeader("No System Root Directory!");
					dlg2.setCanEscape(true);
					int sel2 = dlg2.start();
					if (sel2 == 0) {
						// ルートディレクトリを作成する
#if defined(FDS_WINDOWS)
						std::wstring wrootdir = WStrUtil::str2wstr(mRootDir);
						wrootdir = WStrUtil::pathSlash2Backslash(wrootdir);
						std::wstring cmd = L"mkdir " + wrootdir;
						_wsystem(cmd.c_str());
#else
						std::string cmd = "mkdir -p ";
						if (!mNoRoot) {
							cmd += "-m777 ";
						}
						cmd += mRootDir;
						system(cmd.c_str());
#endif
					} else {
						continue;
					}
				}
			}
		}

		// FddEmuを起動
		std::string cmd = mConfig.fddEmuCmd();
		std::string option = mConfig.cfgMachine().fddEmuOpt();
		mFddEmu.setCmd(cmd);
		mFddEmu.setOption(option);
		mFddEmu.run();

		// FDSメイン起動
		mainLoop();

		// FddEmuを終了
		mFddEmu.kill();
	}

}

// -------------------------------------------------------------
// FDSメインループ
// -------------------------------------------------------------
void
FDSSystem::mainLoop()
{
	// ファイラーのパス設定
	mFiles.setPath(mRootDir+mCurDir);
	mFiles.getFiles(mCurDir.empty());
	mFiles.sortFiles();

	// 各ビューを起動
	pathViewCreateWindow();
	pathViewRefresh();
	infoViewCreateWindow();
	infoViewRefresh();
	fddViewCreateWindow();
	fddViewRefresh();
	helpViewCreateWindow();
	helpViewRefresh();
	filerViewCreateWindow();
	filerViewSetIdx(0);

	// キー入力設定
	nodelay(mwFilerView, true);
	wtimeout(mwFilerView, FddViewRefreshInterval);

	// ファイラービューのキー入力ループ
	bool finish = false;
	bool filerRefresh = true;
	bool infoRefresh = true;
	while (!finish) {
		// ファイラービューの更新依頼があれば更新
		if (filerRefresh) {
			filerViewRefresh();
		}
		filerRefresh = true;

		// キー入力
		int key = wgetch(mwFilerView);
		if (key < 256) {
			key = toupper(key);
		}
		switch (key) {
		  case ERR:
			// キー入力がない場合、一定間隔でFDDビューを更新
			fddViewRefresh();

			// 情報ビューの更新依頼があれば更新
			if (infoRefresh) {
				int idx = filerViewGetIdx();
				if (idx >= (int)mFiles.size()) {
					break;
				}
				if (mFiles[idx].isFdxFile() || mFiles[idx].isNormalDir()) {
					std::string path = mRootDir + mCurDir + mFiles[idx].filename();
					if (mFiles[idx].isDir()) {
						path += "/";
					}
					infoViewSetFile(path);
					infoViewRefresh();
				}
				infoRefresh = false;
			}

			filerRefresh = false;
			break;
		  case 0x1b: // ESC
			wtimeout(mwFilerView, 0);
			if (fds::doEscKey(mwFilerView)) {
				finish = true;
			}
			wtimeout(mwFilerView, FddViewRefreshInterval);
			break;
		  case KEY_UP:
#if defined(KEY_A2)
		  case KEY_A2:
#endif
			filerViewUpCursor();
			infoRefresh = true;
			goto clearInfoView;
		  case KEY_DOWN:
#if defined(KEY_C2)
		  case KEY_C2:
#endif
			filerViewDownCursor();
			infoRefresh = true;
			goto clearInfoView;
		  case 10: // ENTER
			filerViewSelectEntry();
			break;
		  case 8: // BS
		  case KEY_BACKSPACE:
			filerViewBackDir();
			infoRefresh = true;
			goto clearInfoView;
		  case KEY_PPAGE:
			filerViewPageUpCursor();
			infoRefresh = true;
			goto clearInfoView;
		  case KEY_NPAGE:
			filerViewPageDownCursor();
			infoRefresh = true;
			goto clearInfoView;
			break;
		  case KEY_HOME:
			filerViewPageTopCursor();
			infoViewSetFile("");
			infoRefresh = true;
			goto clearInfoView;
		  case KEY_END:
			filerViewPageBottomCursor();
			infoRefresh = true;
			goto clearInfoView;
		  case '\\':
		  case '/':
			filerViewRootDir();
			break;

		  case 'A':
		  case ' ':
			cmdAutoSet();
			goto refreshScreen;
		  case 'C':
			cmdCreateDisk();
			goto refreshScreen;
		  case 'D':
		  case '=':
			cmdDupDisk();
			goto refreshScreen;
		  case 'E':
			cmdEjectDrive();
			goto refreshScreen;
		  case 'I':
			filerViewShowFileInfo();
			filerRefresh = false;
			break;
		  case 'K':
		  case '+':
			cmdMakeDirectory();
			goto refreshScreen;
		  case 'N':
			cmdEditName();
			goto refreshScreen;
		  case 'O':
			cmdEditProtect();
			goto refreshScreen;
		  case 'P':
			cmdProtectDrive();
			goto refreshScreen;
		  case 'Q':
			cmdAnalyzeDisk();
			goto refreshScreen;
		  case 'R':
		  case '?':
			cmdRename();
			goto refreshScreen;
		  case 'X':
		  case KEY_DC:
			cmdDelete();
			goto refreshScreen;
		  case 'W':
			cmdProtectDisk();
			goto refreshScreen;
		  case '#':
			cmdShell();
			break;

		  case '1':
			cmdSetDrive(0);
			break;
		  case '2':
			cmdSetDrive(1);
			break;
		  case '0':
			cmdEjectAllDrive();
			goto refreshScreen;

		  case '<':
			cmdDumpDisk();
			break;
		  case '>':
			cmdRestoreDisk();
			break;
		  default:
			FDS_LOG("mainLoop: key=%d\n", key);
			break;

		  clearInfoView:;
			infoViewSetFile("");
			infoViewRefresh();
			break;

		  refreshScreen:;
			pathViewRefresh();
			infoViewRefresh();
			fddViewRefresh();
			helpViewRefresh();
			filerRefresh = true;
			break;
		}
	}

	// 各ビューを終了
	filerViewDestroyWindow();
	helpViewDestroyWindow();
	fddViewDestroyWindow();
	infoViewDestroyWindow();
	pathViewDestroyWindow();
}

// -------------------------------------------------------------
// ウィンドウ表示終了
// -------------------------------------------------------------
void
FDSSystem::end()
{
	// メインビュー終了
	destroyView();
}

// -------------------------------------------------------------
// 各ビューのための初期化
// -------------------------------------------------------------
void
FDSSystem::initView()
{
	// Cursesの設定
	initscr();
	start_color();
	cbreak();
	noecho();
	curs_set(0);

	fds::initColorPair();
}

// -------------------------------------------------------------
// 各ビューのための終了処理
// -------------------------------------------------------------
void
FDSSystem::destroyView()
{
	move(LINES-1, 0);
	endwin();
}

// -------------------------------------------------------------
// 画面全体をリフレッシュ
// -------------------------------------------------------------
void
FDSSystem::refreshAllView()
{
	drawHeader();
	pathViewRefresh();
	infoViewRefresh();
	fddViewRefresh();
	helpViewRefresh();
	filerViewRefresh();

}

// -------------------------------------------------------------
// ヘッダー描画
// -------------------------------------------------------------
void
FDSSystem::drawHeader()
{
	char buf[FDX_STRING_MAX];

	// ヘッダー描画
	clear();
	attron(COLOR_PAIR(fds::ColorPair::Header));
	memset(buf, ' ', _countof(buf));
	const char* s1 = "FDS: FDX68 File Selector";
	int l1 = strlen(s1);
	const char* s2 = FDS_VERSION " GORRY";
	int l2 = strlen(s2);
	memcpy(buf+COLS/2-l1/2, s1, l1);
	memcpy(buf+COLS-1-l2, s2, l2);
	buf[COLS] = '\0';
	mvaddstr(0, 0, buf);
	attroff(COLOR_PAIR(fds::ColorPair::Header));
	refresh();
}

// -------------------------------------------------------------
// 設定ファイルの読み込み
// -------------------------------------------------------------
int
FDSSystem::loadIniFile()
{
	int err = mConfig.load("fds2.ini");
	if (err < 0) {
		return err;
	}

	return 0;
}

// -------------------------------------------------------------
// 各ビューの位置を設定
// -------------------------------------------------------------
void
FDSSystem::setViewLayout()
{
	// 画面全体
	const fds::XYWH fullViewXYWH = {
	  0,
	  1,
	  COLS,
	  LINES-1,
	};

	// 画面全体の右端19桁
	mHelpViewXYWH.W = 19;
	mHelpViewXYWH.H = fullViewXYWH.h();
	mHelpViewXYWH.X = fullViewXYWH.r()-mHelpViewXYWH.w();
	mHelpViewXYWH.Y = fullViewXYWH.y();

	// 残りの上辺4行
	mFddViewXYWH.W = fullViewXYWH.w() - mHelpViewXYWH.w();
	mFddViewXYWH.H = 4;
	mFddViewXYWH.X = fullViewXYWH.x();
	mFddViewXYWH.Y = fullViewXYWH.y();

	// 残りの上辺4行（上辺重なり）
	mPathViewXYWH.W = mFddViewXYWH.w();
	mPathViewXYWH.H = 4;
	mPathViewXYWH.X = fullViewXYWH.x();
	mPathViewXYWH.Y = fullViewXYWH.y() + mFddViewXYWH.h() - 1;

	// 残りの下辺6行（上辺重なり）
	mInfoViewXYWH.W = mFddViewXYWH.w();
	mInfoViewXYWH.H = 6;
	mInfoViewXYWH.X = fullViewXYWH.x();
	mInfoViewXYWH.Y = fullViewXYWH.b() - mInfoViewXYWH.h();

	// 残り（上辺・下辺重なり）
	mFilerViewXYWH.W = mFddViewXYWH.w();
	mFilerViewXYWH.H = mInfoViewXYWH.y() - mPathViewXYWH.b() + 2;
	mFilerViewXYWH.X = fullViewXYWH.x();
	mFilerViewXYWH.Y = mPathViewXYWH.b() - 1;

	// ダンプビューは中央
	mDumpViewXYWH.W = 52;
	mDumpViewXYWH.H = 17;
	mDumpViewXYWH.X = (fullViewXYWH.w()-mDumpViewXYWH.w())/2;
	mDumpViewXYWH.Y = (fullViewXYWH.h()-mDumpViewXYWH.h())/2;
	mDumpViewXYWH.Y -= 2;

	// リストアビューは中央
	mRestoreViewXYWH.W = 52;
	mRestoreViewXYWH.H = 17;
	mRestoreViewXYWH.X = (fullViewXYWH.w()-mRestoreViewXYWH.w())/2;
	mRestoreViewXYWH.Y = (fullViewXYWH.h()-mRestoreViewXYWH.h())/2;
	mRestoreViewXYWH.Y -= 2;

}

// -------------------------------------------------------------
// 各マシン用のドライブ名の取得
// -------------------------------------------------------------
const std::string&
FDSSystem::getDriveName(int id)
{
	return mConfig.cfgMachine().driveName(id);
}

// =====================================================================
// [EOF]
