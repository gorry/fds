// ◇
// fds: fdx68 selector
// FDSAnalyzer: FDSアナライザ:ヘルプ
// Copyright: (C)2020 Hiroaki GOTO as GORRY.
// License: see readme.txt
// =====================================================================

#include "FDSAnalyzer.h"

// =====================================================================
// FDSAnalyzer: FDSアナライザ:ヘルプ
// =====================================================================

// -------------------------------------------------------------
// ビュー作成
// -------------------------------------------------------------
void
FDSAnalyzer::helpViewCreateWindow()
{
	if (mwHelpView) {
		helpViewDestroyWindow();
	}
	mwHelpView = newwin(
	  mHelpViewXYWH.h(),
	  mHelpViewXYWH.w(),
	  mHelpViewXYWH.y(),
	  mHelpViewXYWH.x()
	);
	keypad(mwHelpView, TRUE);

	mHelpViewMode = HelpViewMode::Disk;
}

// -------------------------------------------------------------
// ビュー破棄
// -------------------------------------------------------------
void
FDSAnalyzer::helpViewDestroyWindow()
{
	if (mwHelpView) {
		delwin(mwHelpView);
		mwHelpView = nullptr;
	}
}

// -------------------------------------------------------------
// ビュー再描画
// -------------------------------------------------------------
void
FDSAnalyzer::helpViewRedraw()
{
	redrawwin(mwHelpView);
	helpViewRefresh();
}

// -------------------------------------------------------------
// ビュー更新
// -------------------------------------------------------------
void
FDSAnalyzer::helpViewRefresh()
{
	// ビューのクリア
	werase(mwHelpView);

	wattron(mwHelpView, COLOR_PAIR(fds::ColorPair::HelpHeader)|A_BOLD);
	mvwaddstr(mwHelpView,  1, 1, "[KEY INSTRUCTION]");
	wattroff(mwHelpView, COLOR_PAIR(fds::ColorPair::HelpHeader)|A_BOLD);

	mvwaddstr(mwHelpView,  3, 1, (1 ? "Arrow: Move Track" : ""));
	mvwaddstr(mwHelpView,  4, 1, (1 ? "AZSX: Move Sector" : ""));
	mvwaddstr(mwHelpView,  5, 1, (1 ? "Q: View Sector" : ""));
	mvwaddstr(mwHelpView,  6, 1, (1 ? "<: Dump Track" : ""));
	mvwaddstr(mwHelpView,  7, 1, (1 ? ">: Restore Track" : ""));

#if 0
	// 選択したヘルプを表示
	switch (mHelpViewMode) {
	  default:
	  case HelpViewMode::Dir:
		mvwaddstr(mwHelpView,  3, 1, (1 ? "A: Auto Set Disk" : ""));
		mvwaddstr(mwHelpView,  4, 1, (1 ? "C: Create Disk" : ""));
		mvwaddstr(mwHelpView,  5, 1, (0 ? "D: Dup Disk" : ""));
		mvwaddstr(mwHelpView,  6, 1, (1 ? "E: Eject Drive" : ""));
		mvwaddstr(mwHelpView,  7, 1, (1 ? "K: Make Directory" : ""));
		mvwaddstr(mwHelpView,  8, 1, (0 ? "N: Edit Disk Name" : ""));
		mvwaddstr(mwHelpView,  9, 1, (0 ? "O: Edit Protect" : ""));
		mvwaddstr(mwHelpView, 10, 1, (1 ? "P: Protect Drive" : ""));
		mvwaddstr(mwHelpView, 11, 1, (1 ? "R: Rename Dir" : ""));
		mvwaddstr(mwHelpView, 12, 1, (0 ? "W: Protect Disk" : ""));
		mvwaddstr(mwHelpView, 13, 1, (1 ? "X: Delete Dir" : ""));
		mvwaddstr(mwHelpView, 14, 1, (0 ? "Z: Analyze Disk" : ""));
		mvwaddstr(mwHelpView, 15, 1, (1 ? "#: Shell" : ""));
		mvwaddstr(mwHelpView, 17, 1, (0 ? "1: Set 1st Drive" : ""));
		mvwaddstr(mwHelpView, 18, 1, (0 ? "2: Set 2nd Drive" : ""));
		mvwaddstr(mwHelpView, 19, 1, (1 ? "0: Eject All" : ""));
		mvwaddstr(mwHelpView, 21, 1, (1 ? "<: Dump from FDD" : ""));
		mvwaddstr(mwHelpView, 22, 1, (0 ? ">: Restore to FDD" : ""));
		break;
	  case HelpViewMode::ParentDir:
		mvwaddstr(mwHelpView,  3, 1, (0 ? "A: Auto Set Disk" : ""));
		mvwaddstr(mwHelpView,  4, 1, (1 ? "C: Create Disk" : ""));
		mvwaddstr(mwHelpView,  5, 1, (0 ? "D: Dup Disk" : ""));
		mvwaddstr(mwHelpView,  6, 1, (1 ? "E: Eject Drive" : ""));
		mvwaddstr(mwHelpView,  7, 1, (1 ? "K: Make Directory" : ""));
		mvwaddstr(mwHelpView,  8, 1, (0 ? "N: Edit Disk Name" : ""));
		mvwaddstr(mwHelpView,  9, 1, (0 ? "O: Edit Protect" : ""));
		mvwaddstr(mwHelpView, 10, 1, (1 ? "P: Protect Drive" : ""));
		mvwaddstr(mwHelpView, 11, 1, (0 ? "R: Rename Dir" : ""));
		mvwaddstr(mwHelpView, 12, 1, (0 ? "W: Protect Disk" : ""));
		mvwaddstr(mwHelpView, 13, 1, (0 ? "X: Delete Dir" : ""));
		mvwaddstr(mwHelpView, 14, 1, (0 ? "Z: Analyze Disk" : ""));
		mvwaddstr(mwHelpView, 15, 1, (1 ? "#: Shell" : ""));
		mvwaddstr(mwHelpView, 17, 1, (0 ? "1: Set 1st Drive" : ""));
		mvwaddstr(mwHelpView, 18, 1, (0 ? "2: Set 2nd Drive" : ""));
		mvwaddstr(mwHelpView, 19, 1, (1 ? "0: Eject All" : ""));
		mvwaddstr(mwHelpView, 21, 1, (1 ? "<: Dump from FDD" : ""));
		mvwaddstr(mwHelpView, 22, 1, (0 ? ">: Restore to FDD" : ""));
		break;
	  case HelpViewMode::FdxFile:
		mvwaddstr(mwHelpView,  3, 1, (1 ? "A: Auto Set" : ""));
		mvwaddstr(mwHelpView,  4, 1, (1 ? "C: Create Disk" : ""));
		mvwaddstr(mwHelpView,  5, 1, (1 ? "D: Dup Disk" : ""));
		mvwaddstr(mwHelpView,  6, 1, (1 ? "E: Eject Drive" : ""));
		mvwaddstr(mwHelpView,  7, 1, (1 ? "K: Make Directory" : ""));
		mvwaddstr(mwHelpView,  8, 1, (1 ? "N: Edit Disk Name" : ""));
		mvwaddstr(mwHelpView,  9, 1, (1 ? "O: Edit Protect" : ""));
		mvwaddstr(mwHelpView, 10, 1, (1 ? "P: Protect Drive" : ""));
		mvwaddstr(mwHelpView, 11, 1, (1 ? "R: Rename Disk" : ""));
		mvwaddstr(mwHelpView, 12, 1, (1 ? "W: Protect Disk" : ""));
		mvwaddstr(mwHelpView, 13, 1, (1 ? "X: Delete Disk" : ""));
		mvwaddstr(mwHelpView, 14, 1, (1 ? "Z: Analyze Disk" : ""));
		mvwaddstr(mwHelpView, 15, 1, (1 ? "#: Shell" : ""));
		mvwaddstr(mwHelpView, 17, 1, (1 ? "1: Set 1st Drive" : ""));
		mvwaddstr(mwHelpView, 18, 1, (1 ? "2: Set 2nd Drive" : ""));
		mvwaddstr(mwHelpView, 19, 1, (1 ? "0: Eject All" : ""));
		mvwaddstr(mwHelpView, 21, 1, (1 ? "<: Dump from FDD" : ""));
		mvwaddstr(mwHelpView, 22, 1, (1 ? ">: Restore to FDD" : ""));
		break;
	  case HelpViewMode::OtherFile:
		mvwaddstr(mwHelpView,  3, 1, (1 ? "A: Auto Set" : ""));
		mvwaddstr(mwHelpView,  4, 1, (1 ? "C: Create Disk" : ""));
		mvwaddstr(mwHelpView,  5, 1, (0 ? "D: Dup Disk" : ""));
		mvwaddstr(mwHelpView,  6, 1, (1 ? "E: Eject Drive" : ""));
		mvwaddstr(mwHelpView,  7, 1, (1 ? "K: Make Directory" : ""));
		mvwaddstr(mwHelpView,  8, 1, (0 ? "N: Edit Disk Name" : ""));
		mvwaddstr(mwHelpView,  9, 1, (0 ? "O: Edit Protect" : ""));
		mvwaddstr(mwHelpView, 10, 1, (0 ? "P: Protect Drive" : ""));
		mvwaddstr(mwHelpView, 11, 1, (0 ? "R: Rename Disk" : ""));
		mvwaddstr(mwHelpView, 12, 1, (1 ? "W: Protect Disk" : ""));
		mvwaddstr(mwHelpView, 13, 1, (0 ? "X: Delete Disk" : ""));
		mvwaddstr(mwHelpView, 14, 1, (0 ? "Z: Analyze Disk" : ""));
		mvwaddstr(mwHelpView, 15, 1, (1 ? "#: Shell" : ""));
		mvwaddstr(mwHelpView, 17, 1, (0 ? "1: Set 1st Drive" : ""));
		mvwaddstr(mwHelpView, 18, 1, (0 ? "2: Set 2nd Drive" : ""));
		mvwaddstr(mwHelpView, 19, 1, (1 ? "0: Eject All" : ""));
		mvwaddstr(mwHelpView, 21, 1, (1 ? "<: Dump from FDD" : ""));
		mvwaddstr(mwHelpView, 22, 1, (0 ? ">: Restore to FDD" : ""));
		break;
	}
#endif

	// 枠を追加して更新
	wborder(mwHelpView, 0,0,0,0,0,0,0,0);
	wrefresh(mwHelpView);
}

// -------------------------------------------------------------
// ヘルプビューのモードを設定
// -------------------------------------------------------------
void
FDSAnalyzer::helpViewSetMode(FDSAnalyzer::HelpViewMode md)
{
	mHelpViewMode = md;
}


// [EOF]
