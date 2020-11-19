// ◇
// fds: fdx68 selector
// DlgSystem: 選択肢ダイアログ
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

	// 準備
	mSelect = sel;

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
	if (y < 0) {
		y = (LINES-h)/2;
	}

	// 表示準備
	mwFrame = newwin(h, w, y+mOfsY, x+mOfsX);
	wborder(mwFrame, 0,0,0,0,0,0,0,0);
	wattron(mwFrame, COLOR_PAIR(FDSSystem::ColorPair::SelectHeader)|A_BOLD);
	mvwaddstr(mwFrame, 1, 2, mHeader.c_str());
	wattroff(mwFrame, COLOR_PAIR(FDSSystem::ColorPair::SelectHeader)|A_BOLD);

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
			mSelect--;
			if (mSelect < 0) {
				mSelect = (int)mSelectTxt.size()-1;
			}
			break;
		  case KEY_DOWN:
#if defined(KEY_C2)
		  case KEY_C2:
#endif
			mSelect++;
			if (mSelect > (int)mSelectTxt.size()-1) {
				mSelect = 0;
			}
			break;
		  case 10: // ENTER
			  menuRet = mSelect;
			  finish = true;
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
	std::wstring buf(mInnerWidth, L' ');

	for (int i=0; i<(int)mSelectTxt.size(); i++) {
		// 選択肢カーソルを表示
		if (i == mSelect) {
			wattron(mwFrame, COLOR_PAIR(FDSSystem::ColorPair::SelectItemCursor));
		}
		mvwaddwstr(mwFrame, mInnerOfsY+i, mInnerOfsX, buf.c_str());
		if (i == mSelect) {
			wattroff(mwFrame, COLOR_PAIR(FDSSystem::ColorPair::SelectItemCursor));
		}

		// 選択肢を表示
		wmove(mwFrame, mInnerOfsY+i, mInnerOfsX+1);
		if (i == mSelect) {
			wattron(mwFrame, COLOR_PAIR(FDSSystem::ColorPair::SelectItemCursor)|A_BOLD);
			waddstr(mwFrame, mSelectTxt[i].c_str());
			wattroff(mwFrame, COLOR_PAIR(FDSSystem::ColorPair::SelectItemCursor)|A_BOLD);
		} else {
			wattron(mwFrame, COLOR_PAIR(FDSSystem::ColorPair::SelectItem));
			waddstr(mwFrame, mSelectTxt[i].c_str());
			wattroff(mwFrame, COLOR_PAIR(FDSSystem::ColorPair::SelectItem));
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
