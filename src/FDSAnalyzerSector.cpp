// ◇
// fds: fdx68 selector
// FDSAnalyzer: FDSアナライザ:セクタ
// Copyright: (C)2020 Hiroaki GOTO as GORRY.
// License: see readme.txt
// =====================================================================

#include "FDSAnalyzer.h"

// =====================================================================
// FDSAnalyzer: FDSアナライザ:セクタ
// =====================================================================

// -------------------------------------------------------------
// ビュー作成
// -------------------------------------------------------------
void
FDSAnalyzer::sectorViewCreateWindow()
{
	if (mwTrackView) {
		sectorViewDestroyWindow();
	}
	mwSectorView = newwin(
	  mSectorViewXYWH.h(),
	  mSectorViewXYWH.w(),
	  mSectorViewXYWH.y(),
	  mSectorViewXYWH.x()
	);
	keypad(mwSectorView, TRUE);

	mSectorViewInnerW = mSectorViewXYWH.w()-2;
	mSectorViewInnerH = mSectorViewXYWH.h()-2-2;
	mSectorViewOfsY = 0;
	mSectorViewCsrY = 0;
	mSectorViewWindowOfsX = 1;
	mSectorViewWindowOfsY = 1+2;
}

// -------------------------------------------------------------
// ビュー破棄
// -------------------------------------------------------------
void
FDSAnalyzer::sectorViewDestroyWindow()
{
	if (mwSectorView) {
		delwin(mwSectorView);
		mwSectorView = nullptr;
	}
}

// -------------------------------------------------------------
// ビュー再描画
// -------------------------------------------------------------
void
FDSAnalyzer::sectorViewRedraw()
{
	redrawwin(mwSectorView);
	sectorViewRefresh();
}

// -------------------------------------------------------------
// ビューの枠だけ再描画
// -------------------------------------------------------------
void
FDSAnalyzer::sectorViewRedrawBorder()
{
	wborder(mwSectorView, 0,0,0,0,0,0,0,0);
}

// -------------------------------------------------------------
// ビュー更新
// -------------------------------------------------------------
static wchar_t sjis2wchar(char* chr) {
	return (wchar_t)'.';
}

static wchar_t utf82wchar(char* chr) {
	return (wchar_t)'.';
}

void
FDSAnalyzer::sectorViewRefresh()
{
	char line[256];

	if (mSectorViewReqLoad && !mSectorViewClear) {
		mSectorViewReqLoad = false;
		std::string cmd = mConfig.fdxViewCmd();
		mFdxView.readFDXSector(cmd, mFilename, mSectorViewTrackNo, mSectorViewSectorNo);
	}
	FdxView::SectorInfo* sector = nullptr;
	if (mFdxView.isSectorReady(mSectorViewTrackNo, mSectorViewSectorNo)) {
		sector = &(mFdxView.Sector(mSectorViewTrackNo, mSectorViewSectorNo));
		mSectorViewListLines = sector->mSecSize/mSectorViewListColumns;
	}

	// ビューのクリア
	werase(mwSectorView);

	// ゲージ
	wattron(mwSectorView, COLOR_PAIR(fds::ColorPair::SectorGauge));
	switch (mSectorViewMode) {
	  case SectorViewMode::Encode:
		//                          01234567890123456789012345678901234567890123456789012345678901234567890
		mvwaddstr(mwSectorView,  1, 2, "Ofs|   0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F");
		mvwaddstr(mwSectorView,  2, 2, "---+----------------------------------------------------------------");
		break;
	  default:
	  case SectorViewMode::Data:
		//                          01234567890123456789012345678901234567890123456789012345678901234567890
		mvwaddstr(mwSectorView,  1, 2, "Ofs| 0 1 2 3 4 5 6 7 8 9 A B C D E F|0123456789ABCDEF");
		mvwaddstr(mwSectorView,  2, 2, "---+--------------------------------+----------------");
		break;
	}
	{
		FdxView::SectorInfo& sec2 = mFdxView.Sector(mSectorViewTrackNo, mSectorViewSectorNo);
		int secfm = 'M';
		if (sec2.mStatus.InfoIFM()) secfm = 'F';
		if (sec2.mStatus.InfoIMIX()) secfm = 'X';
		sprintf(line, "%c%02d", secfm, mSectorViewSectorNo+1);
		mvwaddstr(mwSectorView,  1, 2, line);
	}
	wattroff(mwSectorView, COLOR_PAIR(fds::ColorPair::SectorGauge));

	std::string buf(mSectorViewInnerW, ' ');
	int y = mSectorViewOfsY;
	for (int i=0; i<mSectorViewInnerH; i++) {
		// 選択肢カーソルを表示
		bool sel = false;
		fds::ColorPair col = fds::ColorPair::Normal;
		if (sector && !mSectorViewClear) {
			if (y < mSectorViewListLines) {
				col = fds::ColorPair::SectorNormal;
			}
		}
		if (i == mSectorViewCsrY && !mSectorViewClear) {
			col = (fds::ColorPair)((int)col + (int)fds::ColorPair::SectorNormalCsr - (int)fds::ColorPair::SectorNormal);
			wattron(mwSectorView, COLOR_PAIR(col)|A_BOLD);
			sel = true;
		} else {
			wattron(mwSectorView, COLOR_PAIR(col));
		}
		mvwaddstr(mwSectorView, mSectorViewWindowOfsY+i, mSectorViewWindowOfsX+1, buf.c_str());

		// アイテムを表示
		if (sector && !mSectorViewClear) {
			if (y < mSectorViewListLines) {
				std::vector<uint8_t>& data = sector->Data();
				std::vector<uint8_t>& encode = sector->Encode();
				int ofs = y*mSectorViewListColumns;
				sprintf(line, "%03X|%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X|",
				  ofs,
				  data[ofs+0x00], data[ofs+0x01], data[ofs+0x02], data[ofs+0x03],
				  data[ofs+0x04], data[ofs+0x05], data[ofs+0x06], data[ofs+0x07],
				  data[ofs+0x08], data[ofs+0x09], data[ofs+0x0a], data[ofs+0x0b],
				  data[ofs+0x0c], data[ofs+0x0d], data[ofs+0x0e], data[ofs+0x0f]
				);
				wmove(mwSectorView, mSectorViewWindowOfsY+i, mSectorViewWindowOfsX+1);
				waddstr(mwSectorView, line);
				wchar_t wbuf[256];
				char chr[256];
				int jx = 0;
				for (int j=0; j<mSectorViewListColumns; j++) {
					int c = data[ofs+j];
					switch (mSectorViewStringEncode) {
					  default:
					  case SectorViewStringEncode::Ascii:
						if (c < ' ') c = '.';
						if (0x7f <= c) c = '.';
						wbuf[jx++] = (wchar_t)c;
						break;
					  case SectorViewStringEncode::Sjis:
						if (c < ' ') c = '.';
						if (c == 0x7f) c = '.';
						if (((0x81 <= c) && (c <= 0x9f)) || ((0xe0 <= c) && (c <= 0xef))) {
							int c2 = data[ofs+j+1];
							if ((0x40 <= c) && (c <= 0xfc)) {
								chr[0] = (char)c;
								chr[1] = (char)c2;
								chr[2] = '\0';
								wbuf[jx++] = sjis2wchar(chr);
								j++;
								continue;
							}
							wbuf[jx++] = (wchar_t)'.';
							j++;
							c = '.';
						}
						wbuf[jx++] = (wchar_t)c;
						break;
					  case SectorViewStringEncode::Utf16:
						if (c < ' ') c = '.';
						if (c == 0x7f) c = '.';
						if (c < 0x100) {
							wbuf[jx++] = (wchar_t)c;
							c = ' ';
						}
						wbuf[jx++] = (wchar_t)c;
						break;
					  case SectorViewStringEncode::Utf8:
						if (c < ' ') c = '.';
						if (c < 0x80) {
							wbuf[jx++] = (wchar_t)c;
						} else {
							if (c <= 0xc1) {
								wbuf[jx++] = (wchar_t)'.';
								continue;
							} else if (c <= 0xdf) {
								int c2 = data[ofs+j+1];
								if ((0x80 <= c2) && (c2 <= 0xbf)) {
									chr[0] = (char)c;
									j++;
									chr[1] = (char)c2;
									chr[2] = '\0';
									wbuf[jx++] = utf82wchar(chr);
									continue;
								}
								wbuf[jx++] = (wchar_t)'.';
								j++;
								wbuf[jx++] = (wchar_t)'.';
								continue;
							} else if (c <= 0xef) {
								int c2 = data[ofs+j+1];
								int c3 = data[ofs+j+2];
								if ((0x80 <= c2) && (c2 <= 0xbf)) {
									if ((0x80 <= c3) && (c3 <= 0xbf)) {
										chr[0] = (char)c;
										j++;
										chr[1] = (char)c2;
										j++;
										chr[2] = (char)c3;
										chr[3] = '\0';
										wbuf[jx++] = utf82wchar(chr);
										continue;
									}
								}
								wbuf[jx++] = (wchar_t)'.';
								j++;
								wbuf[jx++] = (wchar_t)'.';
								j++;
								wbuf[jx++] = (wchar_t)'.';
								continue;
							} else if (c <= 0xf4) {
								int c2 = data[ofs+j+1];
								int c3 = data[ofs+j+2];
								int c4 = data[ofs+j+2];
								if ((0x80 <= c2) && (c2 <= 0xbf)) {
									if ((0x80 <= c3) && (c3 <= 0xbf)) {
										if ((0x80 <= c4) && (c4 <= 0xbf)) {
											chr[0] = (char)c;
											j++;
											chr[1] = (char)c2;
											j++;
											chr[2] = (char)c3;
											j++;
											chr[3] = (char)c4;
											chr[4] = '\0';
											wbuf[jx++] = utf82wchar(chr);
											continue;
										}
									}
								}
								wbuf[jx++] = (wchar_t)'.';
								j++;
								wbuf[jx++] = (wchar_t)'.';
								j++;
								wbuf[jx++] = (wchar_t)'.';
								j++;
								wbuf[jx++] = (wchar_t)'.';
								continue;
							}
						}
						break;
					}
				}
				wbuf[jx] = 0;
				waddwstr(mwSectorView, wbuf);

				// ビット構成チェック
				for (int j=0; j<mSectorViewListColumns; j++) {
					fds::ColorPair col2 = fds::ColorPair::Normal;
					uint8_t c1 = encode[(ofs+j)*2+0];
					uint8_t c2 = encode[(ofs+j)*2+1];
					uint8_t c3 = (c1>>2)&0x0f;
					uint8_t c4 = (c2>>2)&0x0f;
					uint8_t c5 = (c1>>4)&0x0f;
					uint8_t c6 = (c2>>4)&0x0f;
					c1 &= 0x0f;
					c2 &= 0x0f;
					bool b;
					b = (c1 == 0x00)||(c2 == 0x00)||(c3 == 0x00)||(c4 == 0x00)||(c5 == 0x00)||(c6 == 0x00);
					if (b) {
						col2 = fds::ColorPair::SectorBit0;
					}
					b = (c1 == 0x03)||(c2 == 0x03)||(c3 == 0x03)||(c4 == 0x03)||(c5 == 0x03)||(c6 == 0x03);
					if (b) {
						col2 = fds::ColorPair::SectorBit3;
					}
					b = (c1 == 0x06)||(c2 == 0x06)||(c3 == 0x06)||(c4 == 0x06)||(c5 == 0x06)||(c6 == 0x06);
					if (b) {
						col2 = fds::ColorPair::SectorBit6;
					}
					b = (c1 == 0x07)||(c2 == 0x07)||(c3 == 0x07)||(c4 == 0x07)||(c5 == 0x07)||(c6 == 0x07);
					if (b) {
						col2 = fds::ColorPair::SectorBit7;
					}
					b = (c1 == 0x08)||(c2 == 0x08)||(c3 == 0x08)||(c4 == 0x08)||(c5 == 0x08)||(c6 == 0x08);
					if (b) {
						col2 = fds::ColorPair::SectorBit8;
					}
					b = (c1 >= 0x0b)||(c2 >= 0x0b)||(c3 >= 0x0b)||(c4 >= 0x0b)||(c5 >= 0x0b)||(c6 >= 0x0b);
					if (b) {
						col2 = fds::ColorPair::SectorBitB;
					}
					if (col2 != fds::ColorPair::Normal) {
						wattron(mwSectorView, COLOR_PAIR(col2)|A_BOLD);
						sprintf(line, "%02X", data[ofs+j]);
						wmove(mwSectorView, mSectorViewWindowOfsY+i, mSectorViewWindowOfsX+5+j*2);
						waddstr(mwSectorView, line);
						wattroff(mwSectorView, COLOR_PAIR(col2)|A_BOLD);
					}
				}

			}
		}
		if (sel) {
			wattroff(mwSectorView, COLOR_PAIR(col)|A_BOLD);
		} else {
			wattroff(mwSectorView, COLOR_PAIR(col));
		}
		y++;
	}
	mSectorViewClear = false;

	// 枠を追加して更新
	sectorViewRedrawBorder();
	wrefresh(mwSectorView);

}

// -------------------------------------------------------------
// セクタビュー表示開始位置をファイルリストidxで設定
// -------------------------------------------------------------
void
FDSAnalyzer::sectorViewSetViewOfsY(int idx)
{
	// idxがファイルリスト先頭より前を差さないよう調整
	if (idx < 0) {
		idx = 0;
	}

	// 表示開始位置が（ファイルリスト末尾－ビュー表示幅）より後を差さないよう調整
	int h = mSectorViewListLines - mSectorViewInnerH;
	if (h < 0) {
		h = 0;
	}
	if (idx > h) {
		idx = h;
	}

	mSectorViewOfsY = idx;
}

// -------------------------------------------------------------
// セクタビュー表示開始位置を取得
// -------------------------------------------------------------
int
FDSAnalyzer::sectorViewGetViewOfsY()
{
	return mSectorViewOfsY;
}

// -------------------------------------------------------------
// セクタビューカーソル位置をビュー位置yで設定
// -------------------------------------------------------------
void
FDSAnalyzer::sectorViewSetCsrY(int y)
{
	// カーソルがビュー先頭より前を差さないよう調整
	if (y < 0) {
		y = 0;
	}

	// カーソルがビュー最終位置として差せる位置を算出
	int h = mSectorViewListLines-mSectorViewOfsY;
	if (h > mSectorViewInnerH) {
		h = mSectorViewInnerH;
	}

	// カーソルがビュー最終位置より後を差さないよう調整
	if (y > h-1) {
		y = h-1;
	}

	mSectorViewCsrY = y;
}

// -------------------------------------------------------------
// ファイルリストidxから表示開始位置とカーソル位置を設定
// -------------------------------------------------------------
void
FDSAnalyzer::sectorViewSetIdx(int idx)
{
	// idxがファイルリスト先頭より前を差さないよう調整
	if (idx < 0) {
		idx = 0;
	}

	// idxがファイルリスト末尾より後を差さないよう調整
	int h = mSectorViewListLines-1;
	if (idx > h) {
		idx = h;
	}

	// 現在見えているビュー内にあれば、カーソルを移動して終了
	if (mSectorViewOfsY <= mSectorViewListLines-mSectorViewInnerH) {
		int y = idx-mSectorViewOfsY;
		if ((0 <= y) && (y < mSectorViewInnerH)) {
			mSectorViewCsrY = y;
			return;
		}
	}

	// 表示開始位置とカーソルを設定
	sectorViewSetViewOfsY(idx);
	idx -= mSectorViewOfsY;
	sectorViewSetCsrY(idx);
}

// -------------------------------------------------------------
// カーソルを上へ
// -------------------------------------------------------------
void
FDSAnalyzer::sectorViewUpCursor()
{
	mSectorViewCsrY--;

	// カーソルがビュー先頭より前を差さないよう調整
	if (mSectorViewCsrY < 0) {
		mSectorViewCsrY = 0;
		mSectorViewOfsY--;

		// 表示開始位置がファイルリスト先頭より前を差さないよう調整
		if (mSectorViewOfsY < 0) {
			mSectorViewOfsY = 0;
		}
	}
}

// -------------------------------------------------------------
// カーソルを下へ
// -------------------------------------------------------------
void
FDSAnalyzer::sectorViewDownCursor()
{
	mSectorViewCsrY++;

	// カーソルがファイルリスト末尾より後を差さないよう調整
	int idx = mSectorViewListLines-1;
	if (mSectorViewCsrY > idx) {
		mSectorViewCsrY = idx;
	}

	// カーソルがビュー最下段より後を差さないよう調整
	int h = mSectorViewInnerH-1;
	if (mSectorViewCsrY > h) {
		mSectorViewCsrY = h;
		sectorViewSetViewOfsY(mSectorViewOfsY+1);
	}
}

// -------------------------------------------------------------
// ビューを１ページ前へ
// -------------------------------------------------------------
void
FDSAnalyzer::sectorViewPageUpCursor()
{
	// カーソルがビュー最上段でなければ最上段へ移動
	if (mSectorViewCsrY > 0) {
		mSectorViewCsrY = 0;
	} else {
		// カーソルがビュー最上段なら表示開始位置を１ページ前へ
		mSectorViewOfsY -= mSectorViewInnerH;
		if (mSectorViewOfsY < 0) {
			mSectorViewOfsY = 0;
		}
	}
}

// -------------------------------------------------------------
// ビューを１ページ後へ
// -------------------------------------------------------------
void
FDSAnalyzer::sectorViewPageDownCursor()
{
	int h = mSectorViewInnerH-1;
	if (mSectorViewCsrY < h) {
		// カーソルがビュー最下段でなければ最下段へ移動
		mSectorViewCsrY = h;

		// カーソルがファイルリスト末尾を超えないよう調整
		h = mSectorViewListLines-1;
		if (mSectorViewCsrY > h) {
			mSectorViewCsrY = h;
		}
	} else {
		// カーソルがビュー最下段なら表示開始位置を１ページ後へ
		mSectorViewCsrY = h;
		mSectorViewOfsY += mSectorViewInnerH;

		// 表示開始位置が（ファイルリスト末尾－ビュー表示幅）より後を差さないよう調整
		int h2 = mSectorViewListLines-mSectorViewInnerH;
		if (h2 < 0) {
			h2 = 0;
		}
		if (mSectorViewOfsY >= h2) {
			mSectorViewOfsY = h2;
		}
	}
}

// -------------------------------------------------------------
// ビューをファイルリスト先頭へ
// -------------------------------------------------------------
void
FDSAnalyzer::sectorViewPageTopCursor()
{
	mSectorViewOfsY = 0;
	mSectorViewCsrY = 0;
}

// -------------------------------------------------------------
// ビューをファイルリスト末尾へ
// -------------------------------------------------------------
void
FDSAnalyzer::sectorViewPageBottomCursor()
{
	mSectorViewOfsY = mSectorViewListLines-mSectorViewInnerH;
	if (mSectorViewOfsY < 0) {
		mSectorViewOfsY = 0;
	}
	mSectorViewCsrY = mSectorViewListLines-mSectorViewOfsY-1;
}

// -------------------------------------------------------------
// ディスクビューに戻る
// -------------------------------------------------------------
void
FDSAnalyzer::sectorViewBackDisk()
{
#if 0
	// ディレクトリスタックが残ってなければ終了
	if (mDirStack.level() <= 0) {
		return;
	}

	// ディレクトリスタックからポップ
	DirStack::Stack stack = mDirStack.pop();
	mSectorViewOfsY = stack.mSectorViewOfsY;
	mSectorViewCsrY = stack.mSectorViewCsrY;
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
// セクタビューのカーソル位置を得る
// -------------------------------------------------------------
int
FDSAnalyzer::sectorViewGetCsrY()
{
	return mSectorViewCsrY;
}

// -------------------------------------------------------------
// セクタビューで選択したエントリのidxを得る
// -------------------------------------------------------------
int
FDSAnalyzer::sectorViewGetIdx()
{
	int idx = mSectorViewOfsY + mSectorViewCsrY;
	int h = mSectorViewListLines - 1;
	if (idx > h) {
		idx = h;
	}
	return idx;
}

// -------------------------------------------------------------
// ビューで表示するセクタ情報をセットする
// -------------------------------------------------------------
void
FDSAnalyzer::sectorViewSetTrack(int track)
{
	mSectorViewTrackNo = track;
}

void
FDSAnalyzer::sectorViewSetSector(int sector)
{
	mSectorViewSectorNo = sector;
}

void
FDSAnalyzer::sectorViewSetLoad(void)
{
	mSectorViewReqLoad = true;
}

bool
FDSAnalyzer::sectorViewDataIsReady(void)
{
	FdxView::DiskInfo& disk = mFdxView.diskInfo();
	return disk.isSectorReady(mSectorViewTrackNo, mSectorViewSectorNo);
}

void
FDSAnalyzer::sectorViewSetStringEncode(SectorViewStringEncode enc)
{
	mSectorViewStringEncode = enc;
}


// =====================================================================
// [EOF]
