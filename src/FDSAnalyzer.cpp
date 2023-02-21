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
// コンストラクタ
// -------------------------------------------------------------
FDSAnalyzer::FDSAnalyzer()
 : mpFddEmu(nullptr)
{
}

// -------------------------------------------------------------
// IsWindowsの設定
// -------------------------------------------------------------
void
FDSAnalyzer::setIsWindows(bool b)
{
	mIsWindows = b;
}

// -------------------------------------------------------------
// FddEmuの設定
// -------------------------------------------------------------
void
FDSAnalyzer::setFddEmu(FddEmu* pfddemu)
{
	mpFddEmu = pfddemu;
}

// -------------------------------------------------------------
// ウィンドウ表示開始
// -------------------------------------------------------------
void
FDSAnalyzer::start(const std::string& filename, int machineNo)
{
	// 設定ファイル読み込み
	int err = loadIniFile();
	if (err < 0) {
		return;
	}
	mConfig.setMachineNo(machineNo);

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
	mAnalyzerMode = AnalyzerMode::Disk;

	// 各ビューを起動
	diskViewCreateWindow();
	trackViewCreateWindow();
	helpViewCreateWindow();
	sectorViewCreateWindow();

	mTrackViewClear = true;
	refreshAllView();

	std::string cmd = mConfig.fdxViewCmd();
	mFdxView.readFDXDiskInfo(cmd, mFilename);
	diskViewShowTrack();

	diskViewSetIdx(0);
	diskViewRefresh();
	trackViewRefresh();

	// キー入力設定
	nodelay(mwDiskView, true);
	wtimeout(mwDiskView, DiskViewRefreshInterval);
	nodelay(mwSectorView, true);
	wtimeout(mwSectorView, DiskViewRefreshInterval);

	// ディスクビューのキー入力ループ
	bool finish = false;
	bool diskRefresh = true;
	bool trackRefreshDelay = true;
	bool sectorRefresh = true;
	bool sectorRefreshDelay = true;
	while (!finish) {
		if (mAnalyzerMode == AnalyzerMode::Disk) {
			// ディスクビューの更新依頼があれば更新
			if (diskRefresh) {
				diskRefresh = false;
				diskViewRefresh();
			}
		} else {
			// セクタビューの更新依頼があれば更新
			if (sectorRefresh) {
				sectorRefresh = false;
				if (trackRefreshDelay) {
					// 先にトラックビューの更新依頼があれば更新
					trackRefreshDelay = false;
					trackViewRefresh();
				}
				sectorViewRefresh();
			}
		}

		// キー入力
		int key = 0;
		if (mAnalyzerMode == AnalyzerMode::Disk) {
			key = wgetch(mwDiskView);
		} else {
			key = wgetch(mwSectorView);
		}
		if (key < 256) {
			key = toupper(key);
		}
		switch (key) {
		  case ERR:
			diskRefresh = false;
			// トラックビューの更新依頼があれば更新
			if (trackRefreshDelay) {
				trackRefreshDelay = false;
				trackViewRefresh();
			}
			if (mAnalyzerMode == AnalyzerMode::Sector) {
				// セクタビューの更新依頼があれば更新
				if (sectorRefreshDelay) {
					sectorRefreshDelay = false;
					sectorViewRefresh();
				}
			}
			break;
		  case 0x1b: // ESC
			if (mAnalyzerMode == AnalyzerMode::Disk) {
				wtimeout(mwDiskView, 0);
				if (fds::doEscKey(mwDiskView)) {
					// exit
					// finish = true;
				}
				wtimeout(mwDiskView, DiskViewRefreshInterval);
			} else {
				wtimeout(mwSectorView, 0);
				if (fds::doEscKey(mwSectorView)) {
					// ディスクビューに切り替え
					mAnalyzerMode = AnalyzerMode::Disk;
					helpViewSetMode(FDSAnalyzer::HelpViewMode::Disk);
					diskRefresh = true;
				}
				wtimeout(mwSectorView, DiskViewRefreshInterval);
			}
			break;
		  case KEY_UP:
#if defined(KEY_A2)
		  case KEY_A2:
#endif
			if (mAnalyzerMode == AnalyzerMode::Disk) {
				diskViewUpCursor();
				diskRefresh = true;
			} else {
				sectorViewUpCursor();
			}
			goto updateView;
		  case KEY_DOWN:
#if defined(KEY_C2)
		  case KEY_C2:
#endif
			if (mAnalyzerMode == AnalyzerMode::Disk) {
				diskViewDownCursor();
				diskRefresh = true;
			} else {
				sectorViewDownCursor();
			}
			goto updateView;
		  case KEY_LEFT:
#if defined(KEY_B1)
		  case KEY_B1:
#endif
			if (mAnalyzerMode == AnalyzerMode::Disk) {
				diskViewLeftCursor();
				diskRefresh = true;
			}
			goto updateView;
		  case KEY_RIGHT:
#if defined(KEY_B3)
		  case KEY_B3:
#endif
			if (mAnalyzerMode == AnalyzerMode::Disk) {
				diskViewRightCursor();
				diskRefresh = true;
			}
			goto updateView;
		  case 10: // ENTER
			if (mAnalyzerMode == AnalyzerMode::Disk) {
				// セクタビューに切り替え
				mAnalyzerMode = AnalyzerMode::Sector;
				helpViewSetMode(FDSAnalyzer::HelpViewMode::Data);
				sectorRefresh = true;
			} else {
				// ディスクビューに切り替え
				mAnalyzerMode = AnalyzerMode::Disk;
				helpViewSetMode(FDSAnalyzer::HelpViewMode::Disk);
				diskRefresh = true;
			}
			goto updateView;
#if 0
		  case 8: // BS
		  case KEY_BACKSPACE:
			diskViewBackDir();
			trackRefreshDelay = true;
			break;
#endif
		  case KEY_PPAGE:
			if (mAnalyzerMode == AnalyzerMode::Disk) {
				diskViewPageUpCursor();
				diskRefresh = true;
			} else {
				sectorViewPageUpCursor();
			}
			goto updateView;
		  case KEY_NPAGE:
			if (mAnalyzerMode == AnalyzerMode::Disk) {
				diskViewPageDownCursor();
				diskRefresh = true;
			} else {
				sectorViewPageDownCursor();
			}
			goto updateView;
		  case KEY_HOME:
			if (mAnalyzerMode == AnalyzerMode::Disk) {
				diskViewPageTopCursor();
				diskRefresh = true;
			} else {
				sectorViewPageTopCursor();
			}
			goto updateView;
		  case KEY_END:
			if (mAnalyzerMode == AnalyzerMode::Disk) {
				diskViewPageBottomCursor();
				diskRefresh = true;
			} else {
				sectorViewPageBottomCursor();
			}
			goto updateView;
		  case 'A':
			trackViewUpCursor();
			goto updateView;
		  case 'Z':
			trackViewDownCursor();
			goto updateView;
		  case 'S':
			trackViewPageTopCursor();
			goto updateView;
		  case 'X':
			trackViewPageBottomCursor();
			goto updateView;

		  case 'Q':
			finish = true;
			break;

		  case 'V':
			if (mAnalyzerMode == AnalyzerMode::Sector) {
				if (mSectorViewMode == SectorViewMode::Encode) {
					mSectorViewMode = SectorViewMode::Data;
					helpViewSetMode(FDSAnalyzer::HelpViewMode::Data);
				} else {
					mSectorViewMode = SectorViewMode::Encode;
					helpViewSetMode(FDSAnalyzer::HelpViewMode::Encode);
				}
				sectorRefresh = true;
			}
			break;


		  case '<':
			cmdDumpTrack();
			goto updateView;
		  case '>':
			cmdRestoreTrack();
			goto updateView;

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

#endif

		  default:
			FDS_LOG("mainLoop: key=%d\n", key);
			break;

		  updateView:;
			diskViewShowTrack();
			if (trackViewDataIsReady()) {
				trackViewRefresh();
			} else {
				trackRefreshDelay = true;
				mTrackViewClear = true;
			}
			trackViewRefresh();
			if (mAnalyzerMode == AnalyzerMode::Sector) {
				trackViewShowSector();
				if (sectorViewDataIsReady()) {
					sectorViewRefresh();
				} else {
					sectorRefreshDelay = true;
					mSectorViewClear = true;
				}
				sectorViewRefresh();
			}
			break;
		}
	}

	// 各ビューを終了
	diskViewDestroyWindow();
	trackViewDestroyWindow();
	helpViewDestroyWindow();
	sectorViewDestroyWindow();
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
	trackViewRefresh();
	helpViewRefresh();
	if (mAnalyzerMode == AnalyzerMode::Disk) {
		diskViewRefresh();
	} else {
		sectorViewRefresh();
	}
}

// -------------------------------------------------------------
// ヘッダー描画
// -------------------------------------------------------------
void
FDSAnalyzer::drawHeader()
{
	char buf[FDX_STRING_MAX];

	// ヘッダー描画
	// clear();
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

	// 画面全体の右端19桁・上端12行
	mHelpViewXYWH.W = 19;
	mHelpViewXYWH.H = 12;
	mHelpViewXYWH.X = fullViewXYWH.r() - mHelpViewXYWH.w();
	mHelpViewXYWH.Y = fullViewXYWH.y();

	// 画面全体の左端残り・上端12行
	mDiskViewXYWH.W = fullViewXYWH.w() - mHelpViewXYWH.w();
	mDiskViewXYWH.H = mHelpViewXYWH.h();
	mDiskViewXYWH.X = fullViewXYWH.x();
	mDiskViewXYWH.Y = fullViewXYWH.y();

	// 残りの下端
	mTrackViewXYWH.W = fullViewXYWH.w();
	mTrackViewXYWH.H = fullViewXYWH.h() - mDiskViewXYWH.h();
	mTrackViewXYWH.X = fullViewXYWH.x();
	mTrackViewXYWH.Y = fullViewXYWH.y() + mDiskViewXYWH.h();

	// 画面全体の左端残り・上端12行
	mSectorViewXYWH.W = mDiskViewXYWH.w();
	mSectorViewXYWH.H = mDiskViewXYWH.h();
	mSectorViewXYWH.X = fullViewXYWH.x();
	mSectorViewXYWH.Y = fullViewXYWH.y();

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

// =====================================================================
// [EOF]
