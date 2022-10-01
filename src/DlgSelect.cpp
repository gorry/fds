// ◇
// fds: fdx68 selector
// DlgSelect: 選択肢ダイアログ
// Copyright: (C)2020 Hiroaki GOTO as GORRY.
// License: see readme.txt
// =====================================================================

#include "DlgSelect.h"

// =====================================================================
// 選択肢ダイアログ
// =====================================================================

// -------------------------------------------------------------
// コンストラクタ
// -------------------------------------------------------------
DlgSelect::DlgSelect()
 : mOfsX(0)
 , mOfsY(0)
{
	mViewOfsY = 0;
}

// -------------------------------------------------------------
// 選択肢の設定（vector）
// -------------------------------------------------------------
void 
DlgSelect::setItemsVec(const ItemsVec& items)
{
	mSelectTxt.clear();
	for (int i=0; i<(int)items.size(); i++) {
		mSelectTxt.push_back(items[i]);
	}
}

// -------------------------------------------------------------
// 選択肢の設定（固定：OKのみ）
// -------------------------------------------------------------
void 
DlgSelect::setItemsOk()
{
	mSelectTxt.clear();
	mSelectTxt.push_back("OK");
	setCanEscape(false);
}

// -------------------------------------------------------------
// 選択肢の設定（固定：OK/Cancelのみ）
// -------------------------------------------------------------
void 
DlgSelect::setItemsOkCancel()
{
	mSelectTxt.clear();
	mSelectTxt.push_back("OK");
	mSelectTxt.push_back("Cancel");
	setCanEscape(false);
}

// -------------------------------------------------------------
// 選択肢の設定（固定：Yes/Noのみ）
// -------------------------------------------------------------
void 
DlgSelect::setItemsYesNo()
{
	mSelectTxt.clear();
	mSelectTxt.push_back("Yes");
	mSelectTxt.push_back("No");
	setCanEscape(false);
	mYesNo = true;
}

// -------------------------------------------------------------
// 選択肢の設定（固定：Yes/No/Cancelのみ）
// -------------------------------------------------------------
void 
DlgSelect::setItemsYesNoCancel()
{
	mSelectTxt.clear();
	mSelectTxt.push_back("Yes");
	mSelectTxt.push_back("No");
	mSelectTxt.push_back("Cancel");
	setCanEscape(false);
	mYesNo = true;
}

// -------------------------------------------------------------
// ESCキーでキャンセルできるかどうかを設定
// -------------------------------------------------------------
void 
DlgSelect::setCanEscape(bool f)
{
	mCanEscape = f;
}

// -------------------------------------------------------------
// ダイアログヘッダの設定
// -------------------------------------------------------------
void 
DlgSelect::setHeader(const std::string& text)
{
	mHeader = text;
}

// -------------------------------------------------------------
// 表示オフセットの設定
// -------------------------------------------------------------
void 
DlgSelect::setOffset(int ofsx, int ofsy)
{
	mOfsX = ofsx;
	mOfsY = ofsy;
}

// -------------------------------------------------------------
// ダイアログの表示に伴うサイズ計算
// -------------------------------------------------------------
void
DlgSelect::measureSize(int &retWidth, int &retHeight)
{
	std::wstring wstr = WStrUtil::str2wstr(mHeader);
	int w = WStrUtil::widthN(wstr);
	int h = mSelectTxt.size()+4;
	for (int i=0; i<(int)mSelectTxt.size(); i++) {
		wstr = WStrUtil::str2wstr(mSelectTxt[i]);
		int w2 = WStrUtil::widthN(wstr);
		if (w < w2) {
			w = w2;
		}
	}
	w += 4;
	if (w > COLS-4) {
		w = COLS-4;
	}
	if (h > LINES-2) {
		h = LINES-2;
	}

	retWidth = w;
	retHeight = h;
}

// -------------------------------------------------------------
// ダイアログ処理開始（中央表示）
// -------------------------------------------------------------
int
DlgSelect::start(int sel)
{
	int w, h;
	measureSize(w, h);

	int menuRet = start(-1, -1, sel);
	return menuRet;
}

// -------------------------------------------------------------
// ダイアログ処理開始（指定位置表示）
// -------------------------------------------------------------
int
DlgSelect::start(int x, int y, int sel)
{
	int menuRet = -1;
	mDisableEnter = mYesNo;

	// 準備
	mSelect = sel;
	if (mSelect > (int)mSelectTxt.size()-1) {
		mSelect = (int)mSelectTxt.size()-1;
	}

	// ダイアログ位置算出
	int w, h;
	measureSize(w, h);
	if (w < 6) {
		// 横幅は最低でも6
		w = 6;
	}
	mInnerOfsX = 2;
	mInnerOfsY = 3;
	mInnerWidth = w-4;
	if (x < 0) {
		x = (COLS-w)/2;
	}
	mInnerHeight = h-4;
	if (y < 0) {
		y = (LINES-h)/2;
	}
	if (mSelect > mInnerHeight-1) {
		mViewOfsY = mSelect - (mInnerHeight-1);
		mSelect -= mViewOfsY;
	}

	// 表示準備
	mwFrame = newwin(h, w, y+mOfsY, x+mOfsX);
	wborder(mwFrame, 0,0,0,0,0,0,0,0);
	wattron(mwFrame, COLOR_PAIR(fds::ColorPair::SelectHeader)|A_BOLD);
	mvwaddstr(mwFrame, 1, 2, mHeader.c_str());
	wattroff(mwFrame, COLOR_PAIR(fds::ColorPair::SelectHeader)|A_BOLD);

	// 入力準備
	keypad(mwFrame, TRUE);

	// 入力処理
	bool finish = false;
	while (!finish) {
		show();
		int key = wgetch(mwFrame);
		switch (key) {
		  case 0x1b: // ESC
			if (mCanEscape) {
				finish = true;
			}
			break;
		  case KEY_UP:
#if defined(KEY_A2)
		  case KEY_A2:
#endif
			if (mDisableEnter) {
				mDisableEnter = false;
				break;
			}
			mSelect--;
			if (mSelect < 0) {
				mSelect++;
				mViewOfsY--;
				if (mSelect+mViewOfsY < 0) {
					mSelect = (int)mSelectTxt.size()-1;
					mViewOfsY = mSelect+1 - mInnerHeight;
					if (mViewOfsY < 0) {
						mViewOfsY = 0;
					} else {
						mSelect -= mViewOfsY;
					}
				}
			}
			break;
		  case KEY_DOWN:
#if defined(KEY_C2)
		  case KEY_C2:
#endif
			if (mDisableEnter) {
				mDisableEnter = false;
				break;
			}
			mSelect++;
			if (mSelect >= mInnerHeight) {
				mSelect--;
				mViewOfsY++;
				if (mSelect+mViewOfsY > (int)mSelectTxt.size()-1) {
					mViewOfsY = 0;
					mSelect = 0;
				}
			}
			break;
		  case 10: // ENTER
			if (!mDisableEnter) {
				menuRet = mSelect + mViewOfsY;
				finish = true;
			}
			break;
		  case 'Y':
		  case 'y':
			if (mYesNo) {
				menuRet = 0;
				finish = true;
			}
			break;
		  case 'N':
		  case 'n':
			if (mYesNo) {
				menuRet = 1;
				finish = true;
			}
			break;

		  default:
			FDS_LOG("DlgSelect: key=%d\n", key);
			break;

		}
	}

	return menuRet;
}

// -------------------------------------------------------------
// ダイアログ表示
// -------------------------------------------------------------
void
DlgSelect::show()
{
	std::string buf(mInnerWidth, ' ');

	int h = mSelectTxt.size();
	if (h > mInnerHeight) {
		h = mInnerHeight;
	}
	for (int i=0; i<h; i++) {
		int y = i + mViewOfsY;
		// 選択肢カーソルを表示
		if ((i == mSelect) && (!mDisableEnter)) {
			wattron(mwFrame, COLOR_PAIR(fds::ColorPair::SelectItemCursor));
		}
		mvwaddstr(mwFrame, mInnerOfsY+i, mInnerOfsX, buf.c_str());
		if ((i == mSelect) && (!mDisableEnter)) {
			wattroff(mwFrame, COLOR_PAIR(fds::ColorPair::SelectItemCursor));
		}

		// 選択肢を表示
		wmove(mwFrame, mInnerOfsY+i, mInnerOfsX+1);
		if ((i == mSelect) && (!mDisableEnter)) {
			wattron(mwFrame, COLOR_PAIR(fds::ColorPair::SelectItemCursor)|A_BOLD);
			waddstr(mwFrame, mSelectTxt[y].c_str());
			wattroff(mwFrame, COLOR_PAIR(fds::ColorPair::SelectItemCursor)|A_BOLD);
		} else {
			wattron(mwFrame, COLOR_PAIR(fds::ColorPair::SelectItem));
			waddstr(mwFrame, mSelectTxt[y].c_str());
			wattroff(mwFrame, COLOR_PAIR(fds::ColorPair::SelectItem));
		}

	}
	wrefresh(mwFrame);
}

// -------------------------------------------------------------
// ダイアログ処理終了
// -------------------------------------------------------------
void
DlgSelect::end()
{
	if (mwFrame) {
		werase(mwFrame);
		wrefresh(mwFrame);
		delwin(mwFrame);
		mwFrame = nullptr;
	}
}

// =====================================================================
// [EOF]
