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
	char buf[FDX_STRING_MAX];

	// ウィンドウ位置算出
	initView();
	setViewLayout();
	drawHeader();

	// 設定ファイル読み込み
	loadIniFile();

	// システム選択肢択準備
	DlgSelect::ItemsVec items;
	int n = mIniFile.getInt("GLOBAL", "SYSTEMS");
	for (int i=0; i<n; i++) {
		sprintf(buf, "SYSTEM-%d", i+1);
		items.push_back(mIniFile.getString(buf, "NAME"));
	}
	items.push_back("[ Quit ]");

	// システム選択肢
	int sel = 0;
	while (!0) {
		// ダイアログ表示
		DlgSelect dlg;
		dlg.setItemsVec(items);
		dlg.setHeader("[Select System]");
		sel = dlg.start(sel);

		// [Quit]を選んだら終了
		if (sel == (int)items.size()-1) {
			break;
		}

		// システムを選択
		sprintf(buf, "SYSTEM-%d", sel+1);
		mIniSecSystem = buf;

		// 選択した設定を読み込む
		setRootDir(mIniFile.getString(mIniSecSystem, "ROOTDIR"));
		{
			// システムのルートディレクトリが存在するか調べる
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
						std::string cmd = "mkdir -p "+mRootDir;
						system(cmd.c_str());
#endif
					} else {
						continue;
					}
				}
			}
		}

		// FddEmuを起動
		std::string cmd = mIniFile.getString("GLOBAL", "FDDEMUCMD");
		std::string option = mIniFile.getString(mIniSecSystem, "FDDEMUOPT");
		mFddEmu.setFddEmuCmd(cmd);
		mFddEmu.setFddEmuOption(option);
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
			if (doEscKey(mwFilerView)) {
				finish = true;
			}
			wtimeout(mwFilerView, FddViewRefreshInterval);
			break;
		  case KEY_UP:
		  case KEY_A2:
			filerViewUpCursor();
			infoRefresh = true;
			goto clearInfoView;
		  case KEY_DOWN:
		  case KEY_C2:
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

	// カラー設定
#if defined(FDS_WINDOWS)
	char* env = "";
#else
	char* env = getenv("TERM");
#endif
	if (env && strstr(env, "256color")) {
		use_default_colors();
		#define MyColor(r,g,b) (((r)*36)+((g)*6)+(b)+16)
		init_pair((short)ColorPair::Normal,             MyColor(5,5,5), MyColor(0,0,0));
		init_pair((short)ColorPair::FilerUnknown,       MyColor(3,3,3), MyColor(0,0,1));
		init_pair((short)ColorPair::FilerParentDir,     MyColor(0,5,5), MyColor(0,0,1));
		init_pair((short)ColorPair::FilerDir,           MyColor(0,5,0), MyColor(0,0,1));
		init_pair((short)ColorPair::FilerFdxFile,       MyColor(5,5,5), MyColor(0,0,1));
		init_pair((short)ColorPair::FilerOtherFile,     MyColor(3,3,3), MyColor(0,0,1));
		init_pair((short)ColorPair::FilerUnknownCsr,    MyColor(3,3,3), MyColor(0,0,5));
		init_pair((short)ColorPair::FilerParentDirCsr,  MyColor(0,5,5), MyColor(0,0,5));
		init_pair((short)ColorPair::FilerDirCsr,        MyColor(0,5,0), MyColor(0,0,5));
		init_pair((short)ColorPair::FilerFdxFileCsr,    MyColor(5,5,5), MyColor(0,0,5));
		init_pair((short)ColorPair::FilerOtherFileCsr,  MyColor(3,3,3), MyColor(0,0,5));
		init_pair((short)ColorPair::FilerProtected,     MyColor(5,0,0), MyColor(0,0,1));
		init_pair((short)ColorPair::FilerProtectedCsr,  MyColor(5,0,0), MyColor(0,0,5));
		init_pair((short)ColorPair::Header,             MyColor(0,0,0), MyColor(5,3,0));
		init_pair((short)ColorPair::PathHeader,         MyColor(0,5,5), MyColor(0,0,0));
		init_pair((short)ColorPair::PathRoot,           MyColor(3,5,0), MyColor(0,0,0));
		init_pair((short)ColorPair::PathCurrent,        MyColor(0,5,3), MyColor(0,0,0));
		init_pair((short)ColorPair::SelectHeader,       MyColor(0,5,5), MyColor(0,0,0));
		init_pair((short)ColorPair::SelectItem,         MyColor(5,5,5), MyColor(0,0,0));
		init_pair((short)ColorPair::SelectItemCursor,   MyColor(5,5,5), MyColor(0,0,5));
		init_pair((short)ColorPair::InputHeader,        MyColor(0,5,5), MyColor(0,0,0));
		init_pair((short)ColorPair::InputEdit,          MyColor(5,5,5), MyColor(0,0,5));
		init_pair((short)ColorPair::FddHeaderOff,       MyColor(0,0,0), MyColor(2,2,2));
		init_pair((short)ColorPair::FddHeaderOn,        MyColor(0,5,0), MyColor(2,2,2));
		init_pair((short)ColorPair::FddProtect,         MyColor(5,0,0), MyColor(0,0,0));
		init_pair((short)ColorPair::FddCluster,         MyColor(0,5,3), MyColor(0,0,0));
		init_pair((short)ColorPair::HelpHeader,         MyColor(0,5,5), MyColor(0,0,0));
		init_pair((short)ColorPair::InfoHeader,         MyColor(0,5,5), MyColor(0,0,0));

	} else {
		init_pair((short)ColorPair::Normal,             COLOR_WHITE, COLOR_BLACK);
		init_pair((short)ColorPair::FilerUnknown,       COLOR_WHITE, COLOR_BLACK);
		init_pair((short)ColorPair::FilerParentDir,     COLOR_YELLOW, COLOR_BLACK);
		init_pair((short)ColorPair::FilerDir,           COLOR_GREEN, COLOR_BLACK);
		init_pair((short)ColorPair::FilerFdxFile,       COLOR_WHITE, COLOR_BLACK);
		init_pair((short)ColorPair::FilerOtherFile,     COLOR_WHITE, COLOR_BLACK);
		init_pair((short)ColorPair::FilerUnknownCsr,    COLOR_WHITE, COLOR_BLUE);
		init_pair((short)ColorPair::FilerParentDirCsr,  COLOR_YELLOW, COLOR_BLUE);
		init_pair((short)ColorPair::FilerDirCsr,        COLOR_GREEN, COLOR_BLUE);
		init_pair((short)ColorPair::FilerFdxFileCsr,    COLOR_WHITE, COLOR_BLUE);
		init_pair((short)ColorPair::FilerOtherFileCsr,  COLOR_WHITE, COLOR_BLUE);
		init_pair((short)ColorPair::FilerProtected,     COLOR_RED, COLOR_BLACK);
		init_pair((short)ColorPair::FilerProtectedCsr,  COLOR_RED, COLOR_BLUE);
		init_pair((short)ColorPair::Header,             COLOR_BLACK, COLOR_YELLOW);
		init_pair((short)ColorPair::PathHeader,         COLOR_CYAN, COLOR_BLACK);
		init_pair((short)ColorPair::PathRoot,           COLOR_WHITE, COLOR_BLACK);
		init_pair((short)ColorPair::PathCurrent,        COLOR_GREEN, COLOR_BLACK);
		init_pair((short)ColorPair::SelectHeader,       COLOR_CYAN, COLOR_BLACK);
		init_pair((short)ColorPair::SelectItem,         COLOR_WHITE, COLOR_BLACK);
		init_pair((short)ColorPair::SelectItemCursor,   COLOR_WHITE, COLOR_BLUE);
		init_pair((short)ColorPair::InputHeader,        COLOR_CYAN, COLOR_BLACK);
		init_pair((short)ColorPair::InputEdit,          COLOR_WHITE, COLOR_BLUE);
		init_pair((short)ColorPair::FddHeaderOff,       COLOR_BLACK, COLOR_WHITE);
		init_pair((short)ColorPair::FddHeaderOn,        COLOR_GREEN, COLOR_WHITE);
		init_pair((short)ColorPair::FddProtect,         COLOR_RED, COLOR_BLACK);
		init_pair((short)ColorPair::FddCluster,         COLOR_GREEN, COLOR_BLACK);
		init_pair((short)ColorPair::HelpHeader,         COLOR_CYAN, COLOR_BLACK);
		init_pair((short)ColorPair::InfoHeader,         COLOR_CYAN, COLOR_BLACK);
	}
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
	attron(COLOR_PAIR(ColorPair::Header));
	memset(buf, ' ', _countof(buf));
	const char* s1 = "FDS: FDX68 File Selector";
	int l1 = strlen(s1);
	const char* s2 = FDS_VERSION " GORRY";
	int l2 = strlen(s2);
	memcpy(buf+COLS/2-l1/2, s1, l1);
	memcpy(buf+COLS-1-l2, s2, l2);
	buf[COLS] = '\0';
	mvaddstr(0, 0, buf);
	attroff(COLOR_PAIR(ColorPair::Header));
	refresh();
}

// -------------------------------------------------------------
// 設定ファイルの読み込み
// -------------------------------------------------------------
void
FDSSystem::loadIniFile()
{
	mIniFile.load("fds.ini");
}

// -------------------------------------------------------------
// 各ビューの位置を設定
// -------------------------------------------------------------
void
FDSSystem::setViewLayout()
{
	// 画面全体
	const XYWH fullViewXYWH = {
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
}

// -------------------------------------------------------------
// 各システム用のドライブ名の取得
// -------------------------------------------------------------
const std::string&
FDSSystem::getDriveName(int id)
{
	static std::string sDriveName[2];

	if (sDriveName[id].empty()) {
		for (int i=0; i<mFddEmu.Drives; i++) {
			char buf[64];
			sprintf(buf, "DRIVE%dNAME", i);
			sDriveName[i] = mIniFile.getString(mIniSecSystem, buf);
		}
	}

	return sDriveName[id];
}

// -------------------------------------------------------------
// コマンドキー設定
// -------------------------------------------------------------
const FDSSystem::ESCKEYMAP FDSSystem::sEscKeyMap[] = {
	{ "[11~", 265 },
	{ "[12~", 266 },
	{ "[13~", 267 },
	{ "[14~", 268 },
	{ "[1~", KEY_HOME },
	{ "[4~", KEY_END },
	{ "Ol", '+' },
	{ "On", '.' },
	{ "Op", '0' },
	{ "Oq", '1' },
	{ "Or", '2' },
	{ "Os", '3' },
	{ "Ot", '4' },
	{ "Ou", '5' },
	{ "Ov", '6' },
	{ "Ow", '7' },
	{ "Ox", '8' },
	{ "Oy", '9' },
};

// -------------------------------------------------------------
// ESCキー（と各コマンドキー）入力時の処理
// -------------------------------------------------------------
bool
FDSSystem::doEscKey(WINDOW* window)
{
	std::string keys;
	int key2 = wgetch(window);
	if (key2 == ERR) {
		return true;
	} else {
		keys.push_back((char)key2);
		while ((key2 = wgetch(window)) != ERR) {
			keys.push_back((char)key2);
		}
		for (int i=0; i<(int)_countof(sEscKeyMap); i++) {
			if (keys == sEscKeyMap[i].keys) {
				ungetch(sEscKeyMap[i].newkey);
				return false;
			}
		}
		FDS_LOG("doEscKey: keys=[%s]\n", keys.c_str());
	}
	return false;
}

// -------------------------------------------------------------
// ESCキー（と各コマンドキー）入力時の処理: ワイド用
// -------------------------------------------------------------
bool
FDSSystem::doEscKeyW(WINDOW* window)
{
	std::wstring wkeys;
	wint_t wch;
	int wkey = wget_wch(window, &wch);
	if (wkey == ERR) {
		return true;
	} else {
		wkeys.push_back((wchar_t)wch);
		while ((wkey = wget_wch(window, &wch)) != ERR) {
			wkeys.push_back((wchar_t)wch);
		}
		std::string keys = WStrUtil::wstr2str(wkeys);
		for (int i=0; i<(int)_countof(sEscKeyMap); i++) {
			if (keys == sEscKeyMap[i].keys) {
				unget_wch((wchar_t)sEscKeyMap[i].newkey);
				return false;
			}
		}
		FDS_LOG("doEscKeyW: wkeys=[%ls]\n", wkeys.c_str());
	}
	return false;
}


// =====================================================================
// [EOF]
