// ◇
// fds: fdx68 selector
// FDSSystem: パス
// Copyright: (C)2020 Hiroaki GOTO as GORRY.
// License: see readme.txt
// =====================================================================

#include "FDSSystem.h"

// =====================================================================
// FDSSystem: パス
// =====================================================================

// -------------------------------------------------------------
// ビュー作成
// -------------------------------------------------------------
void
FDSSystem::pathViewCreateWindow()
{
	if (mwPathView) {
		pathViewDestroyWindow();
	}
	mwPathView = newwin(
	  mPathViewXYWH.h(),
	  mPathViewXYWH.w(),
	  mPathViewXYWH.y(),
	  mPathViewXYWH.x()
	);
	keypad(mwPathView, TRUE);
}

// -------------------------------------------------------------
// ビュー破棄
// -------------------------------------------------------------
void
FDSSystem::pathViewDestroyWindow()
{
	if (mwPathView) {
		delwin(mwPathView);
		mwPathView = nullptr;
	}
}

// -------------------------------------------------------------
// ビュー再描画
// -------------------------------------------------------------
void
FDSSystem::pathViewRedraw()
{
	redrawwin(mwPathView);
	pathViewRefresh();
}

// -------------------------------------------------------------
// ビュー更新
// -------------------------------------------------------------
void
FDSSystem::pathViewRefresh()
{
	// ビューのクリア
	werase(mwPathView);

	// ヘッダ
	wattron(mwPathView, COLOR_PAIR(ColorPair::PathHeader)|A_BOLD);
	mvwaddstr(mwPathView, 1, 1, "[DIR] ");
	wattroff(mwPathView, COLOR_PAIR(ColorPair::PathHeader)|A_BOLD);

	// ルートディレクトリ
	wattron(mwPathView, COLOR_PAIR(ColorPair::PathRoot));
	std::wstring wroot = WStrUtil::str2wstr(mRootDir);
	waddwstr(mwPathView, wroot.c_str());
	wattroff(mwPathView, COLOR_PAIR(ColorPair::PathRoot));

	// ファイラー操作ディレクトリ
	wattron(mwPathView, COLOR_PAIR(ColorPair::PathCurrent)|A_BOLD);
	std::wstring wcurrent = WStrUtil::str2wstr(mCurDir);
	if (wcurrent.empty()) {
		wcurrent = L"/";
	}
	mvwaddwstr(mwPathView, 2, 7, wcurrent.c_str());
	wattroff(mwPathView, COLOR_PAIR(ColorPair::PathCurrent)|A_BOLD);

	mvwaddstr(mwPathView, 2, mPathViewXYWH.w()-2, " ");

	// 枠を追加して更新
	wborder(mwPathView, 0,0,0,0,ACS_LTEE,ACS_RTEE,ACS_LTEE,ACS_RTEE);
	wrefresh(mwPathView);
}


// [EOF]
