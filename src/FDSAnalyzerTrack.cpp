// ◇
// fds: fdx68 selector
// FDSAnalyzer: FDSアナライザ:トラック
// Copyright: (C)2020 Hiroaki GOTO as GORRY.
// License: see readme.txt
// =====================================================================

#include "FDSAnalyzer.h"

// =====================================================================
// FDSAnalyzer: FDSアナライザ:ディスク
// =====================================================================

// -------------------------------------------------------------
// ビュー作成
// -------------------------------------------------------------
void
FDSAnalyzer::trackViewCreateWindow()
{
	if (mwTrackView) {
		trackViewDestroyWindow();
	}
	mwTrackView = newwin(
	  mTrackViewXYWH.h(),
	  mTrackViewXYWH.w(),
	  mTrackViewXYWH.y(),
	  mTrackViewXYWH.x()
	);
	keypad(mwTrackView, TRUE);

	mTrackViewInnerW = mTrackViewXYWH.w()-2;
	mTrackViewInnerH = mTrackViewXYWH.h()-2-3;
	mTrackViewOfsY = 0;
	mTrackViewCsrY = 0;
	mTrackViewWindowOfsX = 1;
	mTrackViewWindowOfsY = 1+3;
}

// -------------------------------------------------------------
// ビュー破棄
// -------------------------------------------------------------
void
FDSAnalyzer::trackViewDestroyWindow()
{
	if (mwTrackView) {
		delwin(mwTrackView);
		mwTrackView = nullptr;
	}
}

// -------------------------------------------------------------
// ビュー再描画
// -------------------------------------------------------------
void
FDSAnalyzer::trackViewRedraw()
{
	redrawwin(mwTrackView);
	trackViewRefresh();
}

// -------------------------------------------------------------
// ビューの枠だけ再描画
// -------------------------------------------------------------
void
FDSAnalyzer::trackViewRedrawBorder()
{
	wborder(mwTrackView, 0,0,0,0,ACS_LTEE,0,0,0);
	diskViewRedrawBorder();
}

// -------------------------------------------------------------
// ビュー更新
// -------------------------------------------------------------
void
FDSAnalyzer::trackViewRefresh()
{
	char line[256];

	FdxView::DiskInfo& disk = mFdxView.diskInfo();
	mTrackViewTrackNo = mTrackViewCylinderNo * disk.mFdxInfo.mHeads + mTrackViewHeadNo;
	if (mTrackViewReqLoad && !mTrackViewClear) {
		mTrackViewReqLoad = false;
		std::string cmd = mConfig.fdxViewCmd();
		mFdxView.readFDXTrack(cmd, mFilename, mTrackViewTrackNo);
	}
	FdxView::TrackInfo* track = nullptr;

	// ビューのクリア
	werase(mwTrackView);

	// トラック情報
	std::string disktype = fds::getDiskType(disk.mFdxInfo);
	wattron(mwTrackView, COLOR_PAIR(fds::ColorPair::TrackStatus));
	if (mFdxView.trackInfo().TrackSize() /* && !mTrackViewClear */) {
		track = &(mFdxView.trackInfo().Track(0));
		sprintf(line, "[MFM]Rate=%6.2f Gap4a=%-4dIAM=%-4dGap1=%-4dGap4b=%-4d[%s]",
		  track->mDRate, track->mSizeGap4a, track->mCellIAM, track->mSizeGap1, track->mSizeGap4b, disktype.c_str()
		);
		mvwaddstr(mwTrackView,  1, 2, line);
		if (track->mStatus.InfoIMIX()) {
			wattron(mwTrackView, COLOR_PAIR(fds::ColorPair::TrackStatusFM));
			mvwaddstr(mwTrackView,  1, 2, "[MIX]");
			wattroff(mwTrackView, COLOR_PAIR(fds::ColorPair::TrackStatusFM));
		} else if (track->mStatus.InfoIFM()) {
			wattron(mwTrackView, COLOR_PAIR(fds::ColorPair::TrackStatusFM));
			mvwaddstr(mwTrackView,  1, 2, "[ FM]");
			wattroff(mwTrackView, COLOR_PAIR(fds::ColorPair::TrackStatusFM));
		}
		if ((track->mDRate < 99.0) || (101.0 < track->mDRate)) {
			wattron(mwTrackView, COLOR_PAIR(fds::ColorPair::TrackAbnormalItem)|A_BOLD);
			sprintf(line, "%6.2f", track->mDRate);
			mvwaddstr(mwTrackView,  1, 12, line);
			wattroff(mwTrackView, COLOR_PAIR(fds::ColorPair::TrackAbnormalItem)|A_BOLD);
		}
	}
	wattroff(mwTrackView, COLOR_PAIR(fds::ColorPair::TrackStatus));

	// ゲージ
	wattron(mwTrackView, COLOR_PAIR(fds::ColorPair::TrackGauge));
	//                          01234567890123456789012345678901234567890123456789012345678901234567890
	mvwaddstr(mwTrackView,  2, 2, " Sec  Cell   Time   C  H  R  N  CRC GAP2 DAM  DATA CRC GAP3 D-Rate");
	mvwaddstr(mwTrackView,  3, 2, "-------------------------------------------------------------------");
	wattroff(mwTrackView, COLOR_PAIR(fds::ColorPair::TrackGauge));

	std::string buf(mTrackViewInnerW, ' ');
	int y = mTrackViewOfsY;
	for (int i=0; i<mTrackViewInnerH; i++) {
		// 選択肢カーソルを表示
		bool sel = false;
		fds::ColorPair col = fds::ColorPair::Normal;
		if (track && !mTrackViewClear) {
			if (y < (int)track->SectorSize()) {
				FdxView::SectorInfo& sector = track->Sector(y);
				col = fds::ColorPair::TrackNormalSec;
				if (sector.mStatus.Err()) {
					col = fds::ColorPair::TrackErrorSec;
				}
				if (sector.SecF5F6F7()) {
					col = fds::ColorPair::TrackF5F6F7Sec;
				}
			}
		}
		if (i == mTrackViewCsrY && !mTrackViewClear) {
			col = (fds::ColorPair)((int)col + (int)fds::ColorPair::TrackNormalSecCsr - (int)fds::ColorPair::TrackNormalSec);
			wattron(mwTrackView, COLOR_PAIR(col)|A_BOLD);
			sel = true;
		} else {
			wattron(mwTrackView, COLOR_PAIR(col));
		}
		mvwaddstr(mwTrackView, mTrackViewWindowOfsY+i, mTrackViewWindowOfsX+1, buf.c_str());

		// アイテムを表示
		if (track && !mTrackViewClear) {
			if (y < (int)(track->SectorSize())) {
				FdxView::SectorInfo& sector = track->Sector(y);

				int secfm = 'M';
				if (sector.mStatus.InfoIFM()) secfm = 'F';
				if (sector.mStatus.InfoIMIX()) secfm = 'X';

				int diskrate = disk.mFdxInfo.mRate;
				int trackcellsize = disk.mFdxInfo.mTrackSize / 2 / 8;
				if (disk.mFdxInfo.mType == 9) {  // RAW
					diskrate /= 8;
					trackcellsize /= 8;
				}
				if (track->mStatus.InfoIFM()) {
					diskrate /= 2;
				}
				// int seccells = sector.mCellEnd - sector.mCellStart - sector.mGap2 - sector.mGap3;
				int nextseccells = 0;
				if (y < (int)track->SectorSize()-1) {
					nextseccells = track->Sector(y+1).mCellStart;
					if (nextseccells < sector.mCellStart) {
						nextseccells += trackcellsize;
					}
				} else {
					nextseccells = track->Sector(0).mCellStart;
					nextseccells += trackcellsize;
				}
				int cellend = sector.mCellEnd;
				if (cellend < sector.mCellStart) {
					cellend += trackcellsize;
				}
				int seccells = cellend - sector.mCellStart;
				if (cellend >= nextseccells) {
					seccells = nextseccells - sector.mCellStart;
				}
				if (seccells < 64) {
					seccells = 64;
				}
				int secsize = sector.mSecSize;
				if (secsize < 1) {
					secsize = 1;
				}
				double secsizerate = (double)(seccells-44) / (double)secsize * 1.000 * 100.0;
				FDS_LOG("Track=%d, Sector=%d, CellStart=%d, CellEnd=%d, seccells=%d, Time=%d, CHRN=%08x, SecSize=%d, Gap2=%d, Gap3=%d, secsizerate=%f, diskrate=%d\n", mTrackViewTrackNo, y+1, sector.mCellStart, sector.mCellEnd, seccells, sector.mTime, sector.mCHRN, sector.mSecSize, sector.mGap2, sector.mGap3, secsizerate, diskrate);

				wmove(mwTrackView, mTrackViewWindowOfsY+i, mTrackViewWindowOfsX+2);
				//                     1         2         3         4         5         6         7
				//             34567890123456789012345678901234567890123456789012345678901234567890
				//             Sec  Cell   Time   C  H  R  N  ICR GAP2 DAM  DATA DCR GAP3 D-Rate
				//             M99 99999 999999 [00:00:00:00] --- 9999 --- 99999 --- 9999 100.00
				sprintf(line, "%c%2d %5d %6d [%02X:%02X:%02X:%02X]  OK %4d  OK %5d  OK %4d %6.2f", 
				  secfm,
				  y+1, 
				  sector.mCellStart, 
				  sector.mTime,
				  sector.CHRN_C(),
				  sector.CHRN_H(),
				  sector.CHRN_R(),
				  sector.CHRN_N(),
				  sector.mGap2,
				  sector.mSecSize,
				  sector.mGap3,
				  sector.mDRate
				);
				waddstr(mwTrackView, line);
				wattron(mwTrackView, COLOR_PAIR(fds::ColorPair::TrackAbnormalItem)|A_BOLD);

				// 前セクタのmCellEnd > 今セクタのmCellStartなら、前セクタの長さがおかしい
				cellend = 0;
				if (y > 0) {
					cellend = track->Sector(y-1).mCellEnd;
					if (cellend < track->Sector(y - 1).mCellStart) {
						cellend += trackcellsize;
					}
				}
				if (cellend >= sector.mCellStart) {
					sprintf(line, "%5d", sector.mCellStart);
					mvwaddstr(mwTrackView, mTrackViewWindowOfsY+i, mTrackViewWindowOfsX+6, line);
				}

				// Cとシリンダ番号が合わないとおかしい
				if (mTrackViewCylinderNo != sector.CHRN_C()) {
					sprintf(line, "%02X", sector.CHRN_C());
					mvwaddstr(mwTrackView, mTrackViewWindowOfsY+i, mTrackViewWindowOfsX+20, line);
				}

				// Hとヘッド番号が合わないとおかしい
				if (mTrackViewHeadNo != sector.CHRN_H()) {
					sprintf(line, "%02X", sector.CHRN_H());
					mvwaddstr(mwTrackView, mTrackViewWindowOfsY+i, mTrackViewWindowOfsX+23, line);
				}

				// 時分秒チェック
				if (sector.SecF5F6F7()) {
					sprintf(line, "%02X", sector.CHRN_R());
					mvwaddstr(mwTrackView, mTrackViewWindowOfsY+i, mTrackViewWindowOfsX+26, line);
				}

				// ID-CRCチェック
				if (sector.mStatus.ErrInfoCRC()) {
					sprintf(line, "ERR");
					mvwaddstr(mwTrackView, mTrackViewWindowOfsY+i, mTrackViewWindowOfsX+33, line);
				}

				// DAMチェック
				if (sector.mStatus.ErrDataDeleted()) {
					sprintf(line, "DEL");
					mvwaddstr(mwTrackView, mTrackViewWindowOfsY+i, mTrackViewWindowOfsX+42, line);
				} else if (sector.mStatus.ErrDataNothing()) {
					sprintf(line, "---");
					mvwaddstr(mwTrackView, mTrackViewWindowOfsY+i, mTrackViewWindowOfsX+42, line);
				}

				// セクタ長チェック
				if ((secsizerate < 80.0) || (120.0 < secsizerate)) {
					sprintf(line, "%02X", sector.CHRN_N());
					mvwaddstr(mwTrackView, mTrackViewWindowOfsY+i, mTrackViewWindowOfsX+29, line);
					sprintf(line, "%5d", sector.mSecSize);
					mvwaddstr(mwTrackView, mTrackViewWindowOfsY+i, mTrackViewWindowOfsX+46, line);
				}

				// DATA-CRCチェック
				if (sector.mStatus.ErrDataNothing()) {
					sprintf(line, "---");
					mvwaddstr(mwTrackView, mTrackViewWindowOfsY+i, mTrackViewWindowOfsX+52, line);
				} else if (sector.mStatus.ErrDataCRC()) {
					sprintf(line, "ERR");
					mvwaddstr(mwTrackView, mTrackViewWindowOfsY+i, mTrackViewWindowOfsX+52, line);
				}

				// 回転速度チェック
				if ((sector.mDRate < 99.0) || (101.0 < sector.mDRate)) {
					sprintf(line, "%6.2f", sector.mDRate);
					mvwaddstr(mwTrackView, mTrackViewWindowOfsY+i, mTrackViewWindowOfsX+61, line);
				}

				wattroff(mwTrackView, COLOR_PAIR(fds::ColorPair::TrackAbnormalItem)|A_BOLD);
			}
		}
		if (sel) {
			wattroff(mwTrackView, COLOR_PAIR(col)|A_BOLD);
		} else {
			wattroff(mwTrackView, COLOR_PAIR(col));
		}
		y++;
	}
	mTrackViewClear = false;

	// 枠を追加して更新
	trackViewRedrawBorder();
	wrefresh(mwTrackView);

}

// -------------------------------------------------------------
// トラックビュー表示開始位置をファイルリストidxで設定
// -------------------------------------------------------------
void
FDSAnalyzer::trackViewSetViewOfsY(int idx)
{
	// idxがファイルリスト先頭より前を差さないよう調整
	if (idx < 0) {
		idx = 0;
	}

	// 表示開始位置が（ファイルリスト末尾－ビュー表示幅）より後を差さないよう調整
	FdxView::TrackInfo& track = mFdxView.trackInfo().Track();
	int h = track.SectorSize() - mTrackViewInnerH;
	if (h < 0) {
		h = 0;
	}
	if (idx > h) {
		idx = h;
	}

	mTrackViewOfsY = idx;
}

// -------------------------------------------------------------
// トラックビュー表示開始位置を取得
// -------------------------------------------------------------
int
FDSAnalyzer::trackViewGetViewOfsY()
{
	return mTrackViewOfsY;
}

// -------------------------------------------------------------
// トラックビューカーソル位置をビュー位置yで設定
// -------------------------------------------------------------
void
FDSAnalyzer::trackViewSetCsrY(int y)
{
	// カーソルがビュー先頭より前を差さないよう調整
	if (y < 0) {
		y = 0;
	}

	// カーソルがビュー最終位置として差せる位置を算出
	FdxView::TrackInfo& track = mFdxView.trackInfo().Track();
	int h = track.SectorSize()-mTrackViewOfsY;
	if (h > mTrackViewInnerH) {
		h = mTrackViewInnerH;
	}

	// カーソルがビュー最終位置より後を差さないよう調整
	if (y > h-1) {
		y = h-1;
	}

	mTrackViewCsrY = y;
}

// -------------------------------------------------------------
// ファイルリストidxから表示開始位置とカーソル位置を設定
// -------------------------------------------------------------
void
FDSAnalyzer::trackViewSetIdx(int idx)
{
	// idxがファイルリスト先頭より前を差さないよう調整
	if (idx < 0) {
		idx = 0;
	}

	// idxがファイルリスト末尾より後を差さないよう調整
	FdxView::TrackInfo& track = mFdxView.trackInfo().Track();
	int h = track.SectorSize()-1;
	if (idx > h) {
		idx = h;
	}

	// 現在見えているビュー内にあれば、カーソルを移動して終了
	if (mTrackViewOfsY <= (int)track.SectorSize()-mTrackViewInnerH) {
		int y = idx-mTrackViewOfsY;
		if ((0 <= y) && (y < mTrackViewInnerH)) {
			mTrackViewCsrY = y;
			return;
		}
	}

	// 表示開始位置とカーソルを設定
	trackViewSetViewOfsY(idx);
	idx -= mTrackViewOfsY;
	trackViewSetCsrY(idx);
}

// -------------------------------------------------------------
// カーソルを上へ
// -------------------------------------------------------------
void
FDSAnalyzer::trackViewUpCursor()
{
	mTrackViewCsrY--;

	// カーソルがビュー先頭より前を差さないよう調整
	if (mTrackViewCsrY < 0) {
		mTrackViewCsrY = 0;
		mTrackViewOfsY--;

		// 表示開始位置がファイルリスト先頭より前を差さないよう調整
		if (mTrackViewOfsY < 0) {
			mTrackViewOfsY = 0;
		}
	}
}

// -------------------------------------------------------------
// カーソルを下へ
// -------------------------------------------------------------
void
FDSAnalyzer::trackViewDownCursor()
{
	mTrackViewCsrY++;

	// カーソルがファイルリスト末尾より後を差さないよう調整
	FdxView::TrackInfo& track = mFdxView.trackInfo().Track();
	int idx = track.SectorSize()-1;
	if (mTrackViewCsrY > idx) {
		mTrackViewCsrY = idx;
	}

	// カーソルがビュー最下段より後を差さないよう調整
	int h = mTrackViewInnerH-1;
	if (mTrackViewCsrY > h) {
		mTrackViewCsrY = h;
		trackViewSetViewOfsY(mTrackViewOfsY+1);
	}
}

// -------------------------------------------------------------
// ビューを１ページ前へ
// -------------------------------------------------------------
void
FDSAnalyzer::trackViewPageUpCursor()
{
	// カーソルがビュー最上段でなければ最上段へ移動
	if (mTrackViewCsrY > 0) {
		mTrackViewCsrY = 0;
	} else {
		// カーソルがビュー最上段なら表示開始位置を１ページ前へ
		mTrackViewOfsY -= mTrackViewInnerH;
		if (mTrackViewOfsY < 0) {
			mTrackViewOfsY = 0;
		}
	}
}

// -------------------------------------------------------------
// ビューを１ページ後へ
// -------------------------------------------------------------
void
FDSAnalyzer::trackViewPageDownCursor()
{
	FdxView::TrackInfo& track = mFdxView.trackInfo().Track();
	int h = mTrackViewInnerH-1;
	if (mTrackViewCsrY < h) {
		// カーソルがビュー最下段でなければ最下段へ移動
		mTrackViewCsrY = h;

		// カーソルがファイルリスト末尾を超えないよう調整
		h = track.SectorSize()-1;
		if (mTrackViewCsrY > h) {
			mTrackViewCsrY = h;
		}
	} else {
		// カーソルがビュー最下段なら表示開始位置を１ページ後へ
		mTrackViewCsrY = h;
		mTrackViewOfsY += mTrackViewInnerH;

		// 表示開始位置が（ファイルリスト末尾－ビュー表示幅）より後を差さないよう調整
		int h2 = track.SectorSize()-mTrackViewInnerH;
		if (h2 < 0) {
			h2 = 0;
		}
		if (mTrackViewOfsY >= h2) {
			mTrackViewOfsY = h2;
		}
	}
}

// -------------------------------------------------------------
// ビューをファイルリスト先頭へ
// -------------------------------------------------------------
void
FDSAnalyzer::trackViewPageTopCursor()
{
	mTrackViewOfsY = 0;
	mTrackViewCsrY = 0;
}

// -------------------------------------------------------------
// ビューをファイルリスト末尾へ
// -------------------------------------------------------------
void
FDSAnalyzer::trackViewPageBottomCursor()
{
	FdxView::TrackInfo& track = mFdxView.trackInfo().Track();
	mTrackViewOfsY = track.SectorSize()-mTrackViewInnerH;
	if (mTrackViewOfsY < 0) {
		mTrackViewOfsY = 0;
	}
	mTrackViewCsrY = track.SectorSize()-mTrackViewOfsY-1;
}

// -------------------------------------------------------------
// ファイルを選択
// -------------------------------------------------------------
void
FDSAnalyzer::trackViewSelectEntry()
{
#if 0
	FdxView::TrackInfo& track = mFdxView.trackInfo().Track();

	// 選択位置をチェック
	int idx = trackViewGetIdx();
	if ((idx < 0) || (idx >= (int)track->SectorSize())) {
		return;
	}

	// ".."を選択
	if (mFiles[idx].isParentDir()) {
		// 移動先ディレクトリをスタックからポップ
		DirStack::Stack stack = mDirStack.pop();
		mTrackViewOfsY = stack.mTrackViewOfsY;
		mTrackViewCsrY = stack.mTrackViewCsrY;
		mRootDir = stack.mRootDir;
		mCurDir = stack.mCurDir;

		// 新しいファイルリストを取得
		mFiles.setPath(mRootDir+mCurDir);
		mFiles.getFiles(mCurDir.empty());
		mFiles.sortFiles();

		// パスビューを更新
		pathViewRefresh();
		return;
	}

	// ディレクトリを選択
	if (mFiles[idx].isDir()) {
		// 現在のディレクトリをスタックへプッシュ
		DirStack::Stack stack;
		stack.mTrackViewOfsY = mTrackViewOfsY;
		stack.mTrackViewCsrY = mTrackViewCsrY;
		stack.mRootDir = mRootDir;
		stack.mCurDir = mCurDir;
		mDirStack.push(stack);

		// 新しいディレクトリへ移動
		setCurDir(mCurDir+mFiles[idx].filename());

		// 新しいファイルリストを取得
		mFiles.setPath(mRootDir+mCurDir);
		mFiles.getFiles(false);
		mFiles.sortFiles();

		// パスビューを更新
		pathViewRefresh();

		// カーソル位置をファイルリストの先頭に設定
		trackViewSetIdx(0);
		return;
	}

	// FDXファイルを選択
	if (mFiles[idx].isFdxFile()) {
		// イメージの挿入先ドライブを選択
		int sel = 0;
		DlgSelect::ItemsVec items;
		for (int i=0; i<mFddEmu.Drives; i++) {
			items.push_back(getDriveName(i));
		}
		{
			DlgSelect dlg;
			dlg.setItemsVec(items);
			dlg.setHeader("[Insert Disk to Drive]");
			dlg.setCanEscape(true);
			sel = dlg.start(sel);
			if (sel < 0) {
				return;
			}
		}

		// イメージをドライブへ挿入
		std::string path = mRootDir + mCurDir + mFiles[idx].filename();
		mFddEmu.setImage(sel, path);

		// FDDビューを更新
		fddViewRefresh();
		return;
	}
#endif
}

// -------------------------------------------------------------
// ひとつ前のディレクトリへ戻る
// -------------------------------------------------------------
void
FDSAnalyzer::trackViewBackDisk()
{
#if 0
	// ディレクトリスタックが残ってなければ終了
	if (mDirStack.level() <= 0) {
		return;
	}

	// ディレクトリスタックからポップ
	DirStack::Stack stack = mDirStack.pop();
	mTrackViewOfsY = stack.mTrackViewOfsY;
	mTrackViewCsrY = stack.mTrackViewCsrY;
	mRootDir = stack.mRootDir;
	mCurDir = stack.mCurDir;

	// 新しいファイルリストを取得
	mFiles.setPath(mRootDir+mCurDir);
	mFiles.getFiles(mCurDir.empty());
	mFiles.sortFiles();

	// パスビューを更新
	pathViewRefresh();
#endif
}

// -------------------------------------------------------------
// トラックビューのカーソル位置を得る
// -------------------------------------------------------------
int
FDSAnalyzer::trackViewGetCsrY()
{
	return mTrackViewCsrY;
}

// -------------------------------------------------------------
// トラックビューで選択したエントリのidxを得る
// -------------------------------------------------------------
int
FDSAnalyzer::trackViewGetIdx()
{
	int idx = mTrackViewOfsY + mTrackViewCsrY;
	FdxView::TrackInfo& track = mFdxView.trackInfo().Track();
	int h = (int)track.SectorSize()-1;
	if (idx > h) {
		idx = h;
	}
	return idx;
}

// -------------------------------------------------------------
// トラックビューで選択したセクタの情報をセクタビューに表示する
// -------------------------------------------------------------
void
FDSAnalyzer::trackViewShowSector()
{
#if 0
	// 選択位置をチェック
	int idx = trackViewGetIdx();
	FdxView::TrackInfo& track = mFdxView.trackInfo().Track();
	if (idx >= (int)track.SectorSize()) {
		return;
	}

	// 選択したFDXファイルで情報ビューを更新
	std::string path = mRootDir + mCurDir + mFiles[idx].filename();
	infoViewSetFile(path);
	infoViewRefresh();
#endif
}

// -------------------------------------------------------------
// ビューで表示するトラック情報をセットする
// -------------------------------------------------------------
void
FDSAnalyzer::trackViewSetCylinder(int cylinder)
{
	mTrackViewCylinderNo = cylinder;
}

void
FDSAnalyzer::trackViewSetHead(int head)
{
	mTrackViewHeadNo = head;
}

void
FDSAnalyzer::trackViewSetLoad(void)
{
	mTrackViewReqLoad = true;
}


// =====================================================================
// [EOF]
