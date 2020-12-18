// ◇
// fds: fdx68 selector
// FDSSystem: リストア
// Copyright: (C)2020 Hiroaki GOTO as GORRY.
// License: see readme.txt
// =====================================================================

#include "FDSSystem.h"

// =====================================================================
// FDSSystem: ダンプ
// =====================================================================

// -------------------------------------------------------------
// ビュー作成
// -------------------------------------------------------------
void
FDSSystem::restoreViewCreateWindow()
{
	if (mwRestoreView) {
		dumpViewDestroyWindow();
	}
	mwRestoreView = newwin(
	  mRestoreViewXYWH.h(),
	  mRestoreViewXYWH.w(),
	  mRestoreViewXYWH.y(),
	  mRestoreViewXYWH.x()
	);
}

// -------------------------------------------------------------
// ビュー破棄
// -------------------------------------------------------------
void
FDSSystem::restoreViewDestroyWindow()
{
	if (mwRestoreView) {
		delwin(mwRestoreView);
		mwRestoreView = nullptr;
	}
}

// -------------------------------------------------------------
// ビュー再描画
// -------------------------------------------------------------
void
FDSSystem::restoreViewRedraw()
{
	redrawwin(mwRestoreView);
	dumpViewRefresh();
}

// -------------------------------------------------------------
// ビュー更新
// -------------------------------------------------------------
void
FDSSystem::restoreViewRefresh()
{
	// ビューのクリア
	werase(mwRestoreView);

	// ヘッダ
	char buf[FDX_STRING_MAX];
	sprintf(buf, "[Restore %s : %s]", mRestoreViewStatus.mName.c_str(), mRestoreViewStatus.mFormat.c_str());
	wattron(mwRestoreView, COLOR_PAIR(ColorPair::RestoreHeader)|A_BOLD);
	std::wstring wbuf = WStrUtil::str2wstr(buf);
	mvwaddwstr(mwRestoreView, 1, 2, wbuf.c_str());
	wattroff(mwRestoreView, COLOR_PAIR(ColorPair::RestoreHeader)|A_BOLD);

	// ゲージ
	wattron(mwRestoreView, COLOR_PAIR(ColorPair::RestoreGauge));
	//                           01234567890123456789012345678901234567890123456789
	mvwaddwstr(mwRestoreView,  2, 2, L"   | 0 1 2 3 4 5 6 7 8 9 | 0 1 2 3 4 5 6 7 8 9 |");
	mvwaddwstr(mwRestoreView,  3, 2, L"---+---------------------+---------------------+");
	mvwaddwstr(mwRestoreView,  4, 2, L"  0|                     |                     |");
	mvwaddwstr(mwRestoreView,  5, 2, L" 20|                     |                     |");
	mvwaddwstr(mwRestoreView,  6, 2, L" 40|                     |                     |");
	mvwaddwstr(mwRestoreView,  7, 2, L" 60|                     |                     |");
	mvwaddwstr(mwRestoreView,  8, 2, L" 80|                     |                     |");
	mvwaddwstr(mwRestoreView,  9, 2, L"100|                     |                     |");
	mvwaddwstr(mwRestoreView, 10, 2, L"120|                     |                     |");
	mvwaddwstr(mwRestoreView, 11, 2, L"140|                     |                     |");
	mvwaddwstr(mwRestoreView, 12, 2, L"160|                     |                     |");
	mvwaddwstr(mwRestoreView, 13, 2, L"---+---------------------+---------------------+");
	mvwaddwstr(mwRestoreView, 14, 2, L"           [Push SPACE Key to Cancel]           ");
	wattroff(mwRestoreView, COLOR_PAIR(ColorPair::RestoreGauge));

	for (int i=0; i<mRestoreViewStatus.mTracks; i++) {
		if (!0) { // mRestoreViewStatus.mChanged[i]) {
			int n = i%20;
			int x =  (n+(n>=10)) * 2 + 7;
			int y = (i/20) + 4;
			switch (mRestoreViewStatus.mStatus[i]) {
			  default:
				break;
			  case FdRestore::TrackStatus::None:
				wattron(mwRestoreView, COLOR_PAIR(ColorPair::RestoreStatusNone));
				mvwaddch(mwRestoreView, y, x, '.');
				wattroff(mwRestoreView, COLOR_PAIR(ColorPair::RestoreStatusNone));
				break;
			  case FdRestore::TrackStatus::Retry1:
			  case FdRestore::TrackStatus::Retry2:
			  case FdRestore::TrackStatus::Retry3:
			  case FdRestore::TrackStatus::Retry4:
			  case FdRestore::TrackStatus::Retry5:
			  case FdRestore::TrackStatus::Retry6:
			  case FdRestore::TrackStatus::Retry7:
			  case FdRestore::TrackStatus::Retry8:
			  case FdRestore::TrackStatus::Retry9:
				wattron(mwRestoreView, COLOR_PAIR(ColorPair::RestoreStatusFinish));
				mvwaddch(mwRestoreView, y, x, '0'+(int)mRestoreViewStatus.mStatus[i]);
				wattroff(mwRestoreView, COLOR_PAIR(ColorPair::RestoreStatusFinish));
				break;
			  case FdRestore::TrackStatus::Finish:
				wattron(mwRestoreView, COLOR_PAIR(ColorPair::RestoreStatusFinish));
				mvwaddch(mwRestoreView, y, x, 'o');
				wattroff(mwRestoreView, COLOR_PAIR(ColorPair::RestoreStatusFinish));
				break;
			  case FdRestore::TrackStatus::Error:
				wattron(mwRestoreView, COLOR_PAIR(ColorPair::RestoreStatusError));
				mvwaddch(mwRestoreView, y, x, 'E');
				wattroff(mwRestoreView, COLOR_PAIR(ColorPair::RestoreStatusError));
				break;
			  case FdRestore::TrackStatus::Unformat:
				wattron(mwRestoreView, COLOR_PAIR(ColorPair::RestoreStatusUnformat));
				mvwaddch(mwRestoreView, y, x, '-');
				wattroff(mwRestoreView, COLOR_PAIR(ColorPair::RestoreStatusUnformat));
				break;
			}
		}
	}

	// 枠を追加して更新
	wborder(mwRestoreView, 0,0,0,0,0,0,0,0);
	wrefresh(mwRestoreView);
}

// -------------------------------------------------------------
// ビュー状態更新
// -------------------------------------------------------------
void
FDSSystem::restoreViewUpdate(FdRestore::Status& st)
{
	mRestoreViewStatus = st;
	memset(st.mChanged, 0, sizeof(st.mChanged));
}


// [EOF]
