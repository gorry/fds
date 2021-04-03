// ◇
// fds: fdx68 selector
// FDSAnalyzer: リストア
// Copyright: (C)2020 Hiroaki GOTO as GORRY.
// License: see readme.txt
// =====================================================================

#include "FDSAnalyzer.h"

// =====================================================================
// FDSAnalyzer: ダンプ
// =====================================================================

// -------------------------------------------------------------
// ビュー作成
// -------------------------------------------------------------
void
FDSAnalyzer::restoreViewCreateWindow()
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
FDSAnalyzer::restoreViewDestroyWindow()
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
FDSAnalyzer::restoreViewRedraw()
{
	redrawwin(mwRestoreView);
	dumpViewRefresh();
}

// -------------------------------------------------------------
// ビュー更新
// -------------------------------------------------------------
void
FDSAnalyzer::restoreViewRefresh()
{
	// ビューのクリア
	werase(mwRestoreView);

	// ヘッダ
	char buf[FDX_STRING_MAX];
	wattron(mwRestoreView, COLOR_PAIR(fds::ColorPair::RestoreHeader)|A_BOLD);
	sprintf(buf, "[Restore Track : %s]", mRestoreViewStatus.mFormat.c_str());
	mvwaddstr(mwRestoreView, 1, 2, buf);
	wattroff(mwRestoreView, COLOR_PAIR(fds::ColorPair::RestoreHeader)|A_BOLD);
	sprintf(buf, "%s", mRestoreViewStatus.mName.c_str());
	mvwaddstr(mwRestoreView, 2, 2, buf);

	// ゲージ
	wattron(mwRestoreView, COLOR_PAIR(fds::ColorPair::RestoreGauge));
	//                                 01234567890123456789012345678901234567890123456789
	mvwaddstr(mwRestoreView,  3, 2, "   | 0 1 2 3 4 5 6 7 8 9 | 0 1 2 3 4 5 6 7 8 9 |");
	mvwaddstr(mwRestoreView,  4, 2, "---+---------------------+---------------------+");
	mvwaddstr(mwRestoreView,  5, 2, "  0|                     |                     |");
	mvwaddstr(mwRestoreView,  6, 2, " 20|                     |                     |");
	mvwaddstr(mwRestoreView,  7, 2, " 40|                     |                     |");
	mvwaddstr(mwRestoreView,  8, 2, " 60|                     |                     |");
	mvwaddstr(mwRestoreView,  9, 2, " 80|                     |                     |");
	mvwaddstr(mwRestoreView, 10, 2, "100|                     |                     |");
	mvwaddstr(mwRestoreView, 11, 2, "120|                     |                     |");
	mvwaddstr(mwRestoreView, 12, 2, "140|                     |                     |");
	mvwaddstr(mwRestoreView, 13, 2, "160|                     |                     |");
	mvwaddstr(mwRestoreView, 14, 2, "---+---------------------+---------------------+");
	mvwaddstr(mwRestoreView, 15, 2, "           [Push SPACE Key to Cancel]           ");
	wattroff(mwRestoreView, COLOR_PAIR(fds::ColorPair::RestoreGauge));

	for (int i=0; i<mRestoreViewStatus.mTracks; i++) {
		if (!0) { // mRestoreViewStatus.mChanged[i]) {
			int n = i%20;
			int x =  (n+(n>=10)) * 2 + 7;
			int y = (i/20) + 5;
			switch (mRestoreViewStatus.mStatus[i]) {
			  default:
				break;
			  case FdRestore::TrackStatus::None:
				wattron(mwRestoreView, COLOR_PAIR(fds::ColorPair::RestoreStatusNone));
				mvwaddch(mwRestoreView, y, x, '.');
				wattroff(mwRestoreView, COLOR_PAIR(fds::ColorPair::RestoreStatusNone));
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
			  case FdRestore::TrackStatus::Retry10:
				wattron(mwRestoreView, COLOR_PAIR(fds::ColorPair::RestoreStatusFinish));
				mvwaddch(mwRestoreView, y, x, " 1234567890"[(int)mRestoreViewStatus.mStatus[i]]);
				wattroff(mwRestoreView, COLOR_PAIR(fds::ColorPair::RestoreStatusFinish));
				break;
			  case FdRestore::TrackStatus::Finish:
				wattron(mwRestoreView, COLOR_PAIR(fds::ColorPair::RestoreStatusFinish));
				mvwaddch(mwRestoreView, y, x, 'o');
				wattroff(mwRestoreView, COLOR_PAIR(fds::ColorPair::RestoreStatusFinish));
				break;
			  case FdRestore::TrackStatus::Error:
				wattron(mwRestoreView, COLOR_PAIR(fds::ColorPair::RestoreStatusError));
				mvwaddch(mwRestoreView, y, x, 'E');
				wattroff(mwRestoreView, COLOR_PAIR(fds::ColorPair::RestoreStatusError));
				break;
			  case FdRestore::TrackStatus::Unformat:
				wattron(mwRestoreView, COLOR_PAIR(fds::ColorPair::RestoreStatusUnformat));
				mvwaddch(mwRestoreView, y, x, '-');
				wattroff(mwRestoreView, COLOR_PAIR(fds::ColorPair::RestoreStatusUnformat));
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
FDSAnalyzer::restoreViewUpdate(FdRestore::Status& st)
{
	mRestoreViewStatus = st;
	memset(st.mChanged, 0, sizeof(st.mChanged));
}


// [EOF]
