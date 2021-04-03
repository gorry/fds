// ◇
// fds: fdx68 selector
// FDSAnalyzer: ダンプ
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
FDSAnalyzer::dumpViewCreateWindow()
{
	if (mwDumpView) {
		dumpViewDestroyWindow();
	}
	mwDumpView = newwin(
	  mDumpViewXYWH.h(),
	  mDumpViewXYWH.w(),
	  mDumpViewXYWH.y(),
	  mDumpViewXYWH.x()
	);
}

// -------------------------------------------------------------
// ビュー破棄
// -------------------------------------------------------------
void
FDSAnalyzer::dumpViewDestroyWindow()
{
	if (mwDumpView) {
		delwin(mwDumpView);
		mwDumpView = nullptr;
	}
}

// -------------------------------------------------------------
// ビュー再描画
// -------------------------------------------------------------
void
FDSAnalyzer::dumpViewRedraw()
{
	redrawwin(mwDumpView);
	dumpViewRefresh();
}

// -------------------------------------------------------------
// ビュー更新
// -------------------------------------------------------------
void
FDSAnalyzer::dumpViewRefresh()
{
	// ビューのクリア
	werase(mwDumpView);

	// ヘッダ
	char buf[FDX_STRING_MAX];
	wattron(mwDumpView, COLOR_PAIR(fds::ColorPair::DumpHeader)|A_BOLD);
	sprintf(buf, "[Dump Track : %s]", mDumpViewStatus.mFormat.c_str());
	mvwaddstr(mwDumpView, 1, 2, buf);
	wattroff(mwDumpView, COLOR_PAIR(fds::ColorPair::DumpHeader)|A_BOLD);
	sprintf(buf, "%s", mDumpViewStatus.mName.c_str());
	mvwaddstr(mwDumpView, 2, 2, buf);

	// ゲージ
	wattron(mwDumpView, COLOR_PAIR(fds::ColorPair::DumpGauge));
	//                              01234567890123456789012345678901234567890123456789
	mvwaddstr(mwDumpView,  3, 2, "   | 0 1 2 3 4 5 6 7 8 9 | 0 1 2 3 4 5 6 7 8 9 |");
	mvwaddstr(mwDumpView,  4, 2, "---+---------------------+---------------------+");
	mvwaddstr(mwDumpView,  5, 2, "  0|                     |                     |");
	mvwaddstr(mwDumpView,  6, 2, " 20|                     |                     |");
	mvwaddstr(mwDumpView,  7, 2, " 40|                     |                     |");
	mvwaddstr(mwDumpView,  8, 2, " 60|                     |                     |");
	mvwaddstr(mwDumpView,  9, 2, " 80|                     |                     |");
	mvwaddstr(mwDumpView, 10, 2, "100|                     |                     |");
	mvwaddstr(mwDumpView, 11, 2, "120|                     |                     |");
	mvwaddstr(mwDumpView, 12, 2, "140|                     |                     |");
	mvwaddstr(mwDumpView, 13, 2, "160|                     |                     |");
	mvwaddstr(mwDumpView, 14, 2, "---+---------------------+---------------------+");
	mvwaddstr(mwDumpView, 15, 2, "           [Push SPACE Key to Cancel]           ");
	wattroff(mwDumpView, COLOR_PAIR(fds::ColorPair::DumpGauge));

	for (int i=0; i<mDumpViewStatus.mTracks; i++) {
		if (!0) { // mDumpViewStatus.mChanged[i]) {
			int n = i%20;
			int x =  (n+(n>=10)) * 2 + 7;
			int y = (i/20) + 5;
			switch (mDumpViewStatus.mStatus[i]) {
			  default:
				break;
			  case FdDump::TrackStatus::None:
				wattron(mwDumpView, COLOR_PAIR(fds::ColorPair::DumpStatusNone));
				mvwaddch(mwDumpView, y, x, '.');
				wattroff(mwDumpView, COLOR_PAIR(fds::ColorPair::DumpStatusNone));
				break;
			  case FdDump::TrackStatus::Retry1:
			  case FdDump::TrackStatus::Retry2:
			  case FdDump::TrackStatus::Retry3:
			  case FdDump::TrackStatus::Retry4:
			  case FdDump::TrackStatus::Retry5:
			  case FdDump::TrackStatus::Retry6:
			  case FdDump::TrackStatus::Retry7:
			  case FdDump::TrackStatus::Retry8:
			  case FdDump::TrackStatus::Retry9:
			  case FdDump::TrackStatus::Retry10:
				wattron(mwDumpView, COLOR_PAIR(fds::ColorPair::DumpStatusFinish));
				mvwaddch(mwDumpView, y, x, " 1234567890"[(int)mDumpViewStatus.mStatus[i]]);
				wattroff(mwDumpView, COLOR_PAIR(fds::ColorPair::DumpStatusFinish));
				break;
			  case FdDump::TrackStatus::Finish:
				wattron(mwDumpView, COLOR_PAIR(fds::ColorPair::DumpStatusFinish));
				mvwaddch(mwDumpView, y, x, 'o');
				wattroff(mwDumpView, COLOR_PAIR(fds::ColorPair::DumpStatusFinish));
				break;
			  case FdDump::TrackStatus::Error:
				wattron(mwDumpView, COLOR_PAIR(fds::ColorPair::DumpStatusError));
				mvwaddch(mwDumpView, y, x, 'E');
				wattroff(mwDumpView, COLOR_PAIR(fds::ColorPair::DumpStatusError));
				break;
			  case FdDump::TrackStatus::Unformat:
				wattron(mwDumpView, COLOR_PAIR(fds::ColorPair::DumpStatusUnformat));
				mvwaddch(mwDumpView, y, x, '-');
				wattroff(mwDumpView, COLOR_PAIR(fds::ColorPair::DumpStatusUnformat));
				break;
			}
		}
	}

	// 枠を追加して更新
	wborder(mwDumpView, 0,0,0,0,0,0,0,0);
	wrefresh(mwDumpView);
}

// -------------------------------------------------------------
// ビュー状態更新
// -------------------------------------------------------------
void
FDSAnalyzer::dumpViewUpdate(FdDump::Status& st)
{
	mDumpViewStatus = st;
	memset(st.mChanged, 0, sizeof(st.mChanged));
}


// [EOF]
