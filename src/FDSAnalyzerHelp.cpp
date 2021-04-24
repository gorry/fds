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
	bool P = true;
	if (mIsWindows) {
		P = false;
	}

	// ビューのクリア
	werase(mwHelpView);

	wattron(mwHelpView, COLOR_PAIR(fds::ColorPair::HelpHeader)|A_BOLD);
	mvwaddstr(mwHelpView,  1, 1, "[KEY INSTRUCTION]");
	wattroff(mwHelpView, COLOR_PAIR(fds::ColorPair::HelpHeader)|A_BOLD);


	// 選択したヘルプを表示
	switch (mHelpViewMode) {
	  default:
	  case HelpViewMode::Disk:
		mvwaddstr(mwHelpView,  3, 1, (1 ? "Arrow:Move Track " : ""));
		mvwaddstr(mwHelpView,  4, 1, (1 ? "AZSX :Move Sector" : ""));
		mvwaddstr(mwHelpView,  5, 1, (1 ? "Enter:View Data  " : ""));
		mvwaddstr(mwHelpView,  6, 1, (1 ? "Q: Quit          " : ""));
		mvwaddstr(mwHelpView,  7, 1, (P ? "<: Dump Track    " : ""));
		mvwaddstr(mwHelpView,  8, 1, (P ? ">: Restore Track " : ""));
		break;
	  case HelpViewMode::Data:
		mvwaddstr(mwHelpView,  3, 1, (1 ? "Arrow:Move Data  " : ""));
		mvwaddstr(mwHelpView,  4, 1, (1 ? "AZSX :Move Sector" : ""));
		mvwaddstr(mwHelpView,  5, 1, (1 ? "Enter:View Track " : ""));
		mvwaddstr(mwHelpView,  6, 1, (1 ? "Q: Quit          " : ""));
		mvwaddstr(mwHelpView,  7, 1, (0 ? "<: Dump Track    " : ""));
		mvwaddstr(mwHelpView,  8, 1, (0 ? ">: Restore Track " : ""));
		break;
	  case HelpViewMode::Encode:
		  mvwaddstr(mwHelpView, 3, 1, (1 ? "Arrow:Move Data  " : ""));
		  mvwaddstr(mwHelpView, 4, 1, (1 ? "AZSX :Move Sector" : ""));
		  mvwaddstr(mwHelpView, 5, 1, (1 ? "Enter:View Track " : ""));
		  mvwaddstr(mwHelpView, 6, 1, (1 ? "Q: Quit          " : ""));
		  mvwaddstr(mwHelpView, 7, 1, (0 ? "<: Dump Track    " : ""));
		  mvwaddstr(mwHelpView, 8, 1, (0 ? ">: Restore Track " : ""));
		  break;
	}

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
