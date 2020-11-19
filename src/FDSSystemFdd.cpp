// ◇
// fds: fdx68 selector
// FDSSystem: FDSシステム:FDDビュー
// Copyright: (C)2020 Hiroaki GOTO as GORRY.
// License: see readme.txt
// =====================================================================

#include "FDSSystem.h"

// =====================================================================
// FDSSystem: FDSシステム:FDDビュー
// =====================================================================

// -------------------------------------------------------------
// ビュー作成
// -------------------------------------------------------------
void
FDSSystem::fddViewCreateWindow()
{
	if (mwFddView) {
		fddViewDestroyWindow();
	}
	mwFddView = newwin(
	  mFddViewXYWH.h(),
	  mFddViewXYWH.w(),
	  mFddViewXYWH.y(),
	  mFddViewXYWH.x()
	);
	keypad(mwFddView, TRUE);
}

// -------------------------------------------------------------
// ビュー破棄
// -------------------------------------------------------------
void
FDSSystem::fddViewDestroyWindow()
{
	if (mwFddView) {
		delwin(mwFddView);
		mwFddView = nullptr;
	}
}

// -------------------------------------------------------------
// ビュー再描画
// -------------------------------------------------------------
void
FDSSystem::fddViewRedraw()
{
	redrawwin(mwFddView);
	fddViewRefresh();
}

// -------------------------------------------------------------
// ビュー更新
// -------------------------------------------------------------
void
FDSSystem::fddViewRefresh()
{
	// ビューのクリア
	werase(mwFddView);

	// FddEmuステータス更新
	mFddEmu.updateStatus();

	for (int i=0; i<mFddEmu.Drives; i++) {
		char buf[1024];
		const std::string& drive = getDriveName(i);
		const FddEmu::Status& st = mFddEmu.getStatus(i); 

		if (!st.mInsert) {
			// ドライブは空
			wattron(mwFddView, COLOR_PAIR(FDSSystem::ColorPair::FddHeaderOff));
			sprintf(buf, " %s ", drive.c_str());
			mvwaddstr(mwFddView, 1+i, 1, buf);
			wattroff(mwFddView, COLOR_PAIR(FDSSystem::ColorPair::FddHeaderOff));
			waddstr(mwFddView, "      -- [EMPTY] --");
		} else {
			// ドライブの情報を表示
			wattron(mwFddView, COLOR_PAIR(FDSSystem::ColorPair::FddHeaderOn)|A_BOLD);
			sprintf(buf, " %s ", drive.c_str());
			mvwaddstr(mwFddView, 1+i, 1, buf);
			wattroff(mwFddView, COLOR_PAIR(FDSSystem::ColorPair::FddHeaderOn)|A_BOLD);
			wattron(mwFddView, COLOR_PAIR(FDSSystem::ColorPair::FddCluster)|A_BOLD);
			sprintf(buf, " %02d", st.mCluster);
			waddstr(mwFddView, buf);
			wattroff(mwFddView, COLOR_PAIR(FDSSystem::ColorPair::FddCluster)|A_BOLD);
			if (st.mProtect) {
				wattron(mwFddView, COLOR_PAIR(FDSSystem::ColorPair::FddProtect)|A_BOLD);
				waddstr(mwFddView, "* ");
				wattroff(mwFddView, COLOR_PAIR(FDSSystem::ColorPair::FddProtect)|A_BOLD);
			} else {
				waddstr(mwFddView, ": ");
			}
			std::wstring str = WStrUtil::str2wstr(st.mFileName);
			waddwstr(mwFddView, str.c_str());
			mvwaddstr(mwFddView, 1+i, mFddViewXYWH.w()-2, " ");
		}
	}

	// 枠を追加して更新
	wborder(mwFddView, 0,0,0,0,0,0,ACS_LTEE,ACS_RTEE);
	wrefresh(mwFddView);
}


// [EOF]
