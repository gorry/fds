// ◇
// fds: fdx68 selector
// FDSAnalyzer: FDSアナライザ
// Copyright: (C)2020 Hiroaki GOTO as GORRY.
// License: see readme.txt
// =====================================================================

#include "FDSAnalyzer.h"

// =====================================================================
// FDSAnalyzer: FDSアナライザ
// =====================================================================

// -------------------------------------------------------------
// ウィンドウ表示開始
// -------------------------------------------------------------
void
FDSAnalyzer::start(const std::string& filename)
{
	// 設定ファイル読み込み
	int err = loadIniFile();
	if (err < 0) {
		return;
	}

	mFilename = filename;

	// ウィンドウ位置算出
	initView();
	setViewLayout();
	drawHeader();

	// アナライザメイン起動
	mainLoop();

}

// -------------------------------------------------------------
// FDSメインループ
// -------------------------------------------------------------
void
FDSAnalyzer::mainLoop()
{
	// 各ビューを起動
	diskViewCreateWindow();
	trackViewCreateWindow();
	helpViewCreateWindow();
#if 0
	sectorViewCreateWindow();
#endif

	refreshAllView();

	std::string cmd = mConfig.fdxToolCmd();
	mFdxTool.readFDXDiskInfo(cmd, mFilename);
	mFdxTool.readFDXDiskInfoVerbose(cmd, mFilename);
	mFdxTool.copyFDXDiskInfoTrackStatus();

	diskViewSetIdx(0);
	refreshAllView();

	// キー入力設定
	nodelay(mwDiskView, true);
	wtimeout(mwDiskView, DiskViewRefreshInterval);

	// ディスクビューのキー入力ループ
	bool finish = false;
	bool diskRefresh = true;
	bool trackRefresh = true;
	while (!finish) {
		// ファイラービューの更新依頼があれば更新
		if (diskRefresh) {
			diskViewRefresh();
		}
		diskRefresh = true;

		// トラックビューの更新依頼があれば更新
		if (trackRefresh) {
			trackRefresh = false;
			trackViewRefresh();
		}

		// キー入力
		int key = wgetch(mwDiskView);
		if (key < 256) {
			key = toupper(key);
		}
		switch (key) {
		  case ERR:
			diskRefresh = false;
			break;
		  case 0x1b: // ESC
			wtimeout(mwDiskView, 0);
			if (fds::doEscKey(mwDiskView)) {
				finish = true;
			}
			wtimeout(mwDiskView, DiskViewRefreshInterval);
			break;
		  case KEY_UP:
#if defined(KEY_A2)
		  case KEY_A2:
#endif
			diskViewUpCursor();
			diskViewShowTrack();
			goto clearTrackView;
		  case KEY_DOWN:
#if defined(KEY_C2)
		  case KEY_C2:
#endif
			diskViewDownCursor();
			diskViewShowTrack();
			goto clearTrackView;
		  case KEY_LEFT:
#if defined(KEY_B1)
		  case KEY_B1:
#endif
			diskViewLeftCursor();
			diskViewShowTrack();
			goto clearTrackView;
		  case KEY_RIGHT:
#if defined(KEY_B3)
		  case KEY_B3:
#endif
			diskViewRightCursor();
			diskViewShowTrack();
			goto clearTrackView;
		  case 10: // ENTER
			diskViewSelectEntry();
			break;
#if 0
		  case 8: // BS
		  case KEY_BACKSPACE:
			diskViewBackDir();
			trackRefresh = true;
			goto clearTrackView;
#endif
		  case KEY_PPAGE:
			diskViewPageUpCursor();
			trackRefresh = true;
			goto clearTrackView;
		  case KEY_NPAGE:
			diskViewPageDownCursor();
			trackRefresh = true;
			goto clearTrackView;
			break;
		  case KEY_HOME:
			diskViewPageTopCursor();
			trackRefresh = true;
			goto clearTrackView;
		  case KEY_END:
			diskViewPageBottomCursor();
			trackRefresh = true;
			goto clearTrackView;
		  case 'A':
			trackViewUpCursor();
			trackRefresh = true;
			diskRefresh = false;
			goto clearTrackView;
		  case 'Z':
			trackViewDownCursor();
			trackRefresh = true;
			diskRefresh = false;
			goto clearTrackView;
		  case 'S':
			trackViewPageTopCursor();
			trackRefresh = true;
			diskRefresh = false;
			goto clearTrackView;
		  case 'X':
			trackViewPageBottomCursor();
			trackRefresh = true;
			diskRefresh = false;
			goto clearTrackView;
#if 0
		  case '\\':
		  case '/':
			diskViewRootDir();
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
			diskViewShowFileInfo();
			diskRefresh = false;
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

		  case '<':
			cmdDumpDisk();
			break;
		  case '>':
			cmdRestoreDisk();
			break;
		  default:
			FDS_LOG("mainLoop: key=%d\n", key);
			break;
#endif

		  clearTrackView:;
#if 0
			trackViewSetFile("");
			trackViewRefresh();
#endif
			break;

		  refreshScreen:;
			trackViewRefresh();
			helpViewRefresh();
#if 0
			sectorViewRefresh();
#endif
			diskRefresh = true;
			break;
		}
	}

	// 各ビューを終了
	diskViewDestroyWindow();
	trackViewDestroyWindow();
	helpViewDestroyWindow();
#if 0
	sectorViewDestroyWindow();
#endif
}

// -------------------------------------------------------------
// ウィンドウ表示終了
// -------------------------------------------------------------
void
FDSAnalyzer::end()
{
	// メインビュー終了
	destroyView();
}

// -------------------------------------------------------------
// 各ビューのための初期化
// -------------------------------------------------------------
void
FDSAnalyzer::initView()
{
}

// -------------------------------------------------------------
// 各ビューのための終了処理
// -------------------------------------------------------------
void
FDSAnalyzer::destroyView()
{
#if 0
	move(LINES-1, 0);
	endwin();
#endif
}

// -------------------------------------------------------------
// 画面全体をリフレッシュ
// -------------------------------------------------------------
void
FDSAnalyzer::refreshAllView()
{
	drawHeader();
	diskViewRefresh();
	trackViewRefresh();
	helpViewRefresh();
#if 0
	sectorViewRefresh();
#endif
}

// -------------------------------------------------------------
// ヘッダー描画
// -------------------------------------------------------------
void
FDSAnalyzer::drawHeader()
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
FDSAnalyzer::loadIniFile()
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
FDSAnalyzer::setViewLayout()
{
	// 画面全体
	const fds::XYWH fullViewXYWH = {
	  0,
	  1,
	  COLS,
	  LINES-1,
	};

	// 画面全体の右端19桁・上端9行
	mHelpViewXYWH.W = 19;
	mHelpViewXYWH.H = 11;
	mHelpViewXYWH.X = fullViewXYWH.r()-mHelpViewXYWH.w();
	mHelpViewXYWH.Y = fullViewXYWH.y();

	// 画面全体の左端残り・上端12行
	mDiskViewXYWH.W = 50;
	mDiskViewXYWH.H = 12;
	mDiskViewXYWH.X = fullViewXYWH.x();
	mDiskViewXYWH.Y = fullViewXYWH.y();

	// 残りの下端（上辺重なり）
	mTrackViewXYWH.W = fullViewXYWH.w();
	mTrackViewXYWH.H = fullViewXYWH.h() - mDiskViewXYWH.h() + 1;
	mTrackViewXYWH.X = fullViewXYWH.x();
	mTrackViewXYWH.Y = fullViewXYWH.y() + mDiskViewXYWH.h() - 1;

#if 0
	// 残りの下半分（上辺重なり）
	mSectorViewXYWH.W = mTrackViewXYWH.w();
	mSectorViewXYWH.H = fullViewXYWH.h() - mTrackViewXYWH.h() + 1;
	mSectorViewXYWH.X = mTrackViewXYWH.x();
	mSectorViewXYWH.Y = fullViewXYWH.y() + mTrackViewXYWH.h() - 1;
#endif
}

// =====================================================================
// [EOF]
