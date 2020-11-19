// ◇
// fds: fdx68 selector
// DlgInput: １行入力ダイアログ
// Copyright: (C)2020 Hiroaki GOTO as GORRY.
// License: see readme.txt
// =====================================================================

#include "DlgInput.h"

// =====================================================================
// １行入力ダイアログ
// =====================================================================

// -------------------------------------------------------------
// コンストラクタ
// -------------------------------------------------------------
DlgInput::DlgInput()
 : mOfsX(0)
 , mOfsY(0)
{
}

// -------------------------------------------------------------
// 初期入力テキストの設定
// -------------------------------------------------------------
void 
DlgInput::setText(const std::string& text)
{
	mInputText = text;
}

// -------------------------------------------------------------
// 入力結果テキストの取得
// -------------------------------------------------------------
const std::string
DlgInput::getText()
{
	return mInputText;
}

// -------------------------------------------------------------
// ダイアログヘッダの設定
// -------------------------------------------------------------
void 
DlgInput::setHeader(const std::string& text)
{
	mHeader = text;
}

// -------------------------------------------------------------
// ダイアログに入力できる文字数（末尾'\0'を含まない）の設定
// -------------------------------------------------------------
void 
DlgInput::setMaxLength(int maxlen)
{
	mMaxTextLength = maxlen;
}

// -------------------------------------------------------------
// ESCキーでキャンセルできるかどうかを設定
// -------------------------------------------------------------
void 
DlgInput::setCanEscape(bool f)
{
	mCanEscape = f;
}

// -------------------------------------------------------------
// 表示オフセットの設定
// -------------------------------------------------------------
void 
DlgInput::setOffset(int ofsx, int ofsy)
{
	mOfsX = ofsx;
	mOfsY = ofsy;
}

// -------------------------------------------------------------
// ダイアログの表示に伴うサイズ計算
// -------------------------------------------------------------
void
DlgInput::measureSize(int &retWidth, int &retHeight)
{
	std::wstring wstr = WStrUtil::str2wstr(mHeader);
	int w = WStrUtil::widthN(wstr);
	int h = 5;
	wstr = WStrUtil::str2wstr(mInputText);
	int w2 = WStrUtil::widthN(wstr);
	if (w < w2) {
		w = w2;
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
DlgInput::start(int w)
{
	int menuRet = start(-1, -1, w);
	return menuRet;
}

// -------------------------------------------------------------
// ダイアログ処理開始（指定位置表示）
// -------------------------------------------------------------
int
DlgInput::start(int x, int y, int w)
{
	int inputRet = -1;

	// 準備
	mWInputText = WStrUtil::str2wstr(mInputText);
	mTextLength = mWInputText.length();
{
	std::string str = WStrUtil::wstr2str(mWInputText);
	FDS_LOG("start: mWInputText=[%ls], str=[%s]\n", mWInputText.c_str(), str.c_str());
}

	// 入力文字数の決定
	if (mMaxTextLength < (int)mInputText.length()) {
		mMaxTextLength = (int)mInputText.length();
		if (mMaxTextLength < 1) {
			// 最低でも１文字は入力可能
			mMaxTextLength = 1;
		}
	}

	// ダイアログ位置算出
	int w2, h2;
	measureSize(w2, h2);
	if (w2 < 6) {
		// 横幅は最低でも6
		w2 = 6;
	}
	if (w2 < w+4) {
		w2 = w+4;
	}
	mTextBoxW = w2-4;
	if (x < 0) {
		x = (COLS-w2)/2-2;
	}
	if (y < 0) {
		y = (LINES-h2)/2-1;
	}

	// 表示準備
	mwFrame = newwin(h2, w2, y+mOfsY, x+mOfsX);
	wborder(mwFrame, 0,0,0,0,0,0,0,0);
	wattron(mwFrame, COLOR_PAIR(FDSSystem::ColorPair::InputHeader)|A_BOLD);
	mvwaddstr(mwFrame, 1, 2, mHeader.c_str());
	wattroff(mwFrame, COLOR_PAIR(FDSSystem::ColorPair::InputHeader)|A_BOLD);

	// 入力準備
	keypad(mwFrame, TRUE);
	keyEnd();  // 入力末尾へカーソル移動

	// 入力処理
	mInsertMode = true;
	mFinish = false;
	while (!mFinish) {
		show();
		nodelay(mwFrame, false);
		wint_t wch;
		int key = wget_wch(mwFrame, &wch);
		// fprintf(stderr, "key=%d, wch=%d\n", key, (int)wch);
		switch (wch) {
		  case 0x1b: // ESC
			{
				nodelay(mwFrame, true);
				if (FDSSystem::doEscKeyW(mwFrame)) {
					if (mCanEscape) {
						mFinish = true;
					}
				}
				nodelay(mwFrame, false);
			}
			break;
		  case KEY_LEFT:
#if defined(KEY_B1)
		  case KEY_B1:
#endif
			keyLeft();
			break;
		  case KEY_RIGHT:
#if defined(KEY_B3)
		  case KEY_B3:
#endif
			keyRight();
			break;
		  case KEY_HOME:
			keyHome();
			break;
		  case KEY_END:
			keyEnd();
			break;
		  case 10: // ENTER
			keyEnter();
			inputRet = 0;
			mFinish = true;
			break;
		  case KEY_IC:
			keyInsert();
			break;
		  case KEY_DC:
			keyDelete();
			break;
		  case 8: // BS
		  case KEY_BACKSPACE:
			keyBackspace();
			break;
		  default:
			if ((key == 0) || ((key == KEY_CODE_YES) && (wch < 256))) {
				keyChar(wch);
				break;
			}
			FDS_LOG("DlgInput: key=%d\n", key);
			break;
		}
	}

	// 入力終了処理
	curs_set(0);

	// 入力完了なら0
	return inputRet;
}

// -------------------------------------------------------------
// ダイアログ表示
// -------------------------------------------------------------
void
DlgInput::show()
{
	std::wstring wstr;

	// 枠内に表示可能な部分を表示
	int w = WStrUtil::copyByWidth(wstr, mWInputText, mTextBoxOfsX, mTextBoxW);
	wstr += std::wstring(mTextBoxW - w, L' ');
	wattron(mwFrame, COLOR_PAIR(FDSSystem::ColorPair::InputEdit)|A_BOLD);
	mvwaddnwstr(mwFrame, 3, 2, wstr.c_str(), mTextBoxW);
	wattroff(mwFrame, COLOR_PAIR(FDSSystem::ColorPair::InputEdit) | A_BOLD);

	// カーソルを設定
	wmove(mwFrame, 3, 2+WStrUtil::widthN(wstr, 0, mTextBoxCsrX));
	curs_set((mInsertMode ? 1 : 2));

	// 更新
	wrefresh(mwFrame);
}

// -------------------------------------------------------------
// ダイアログ処理終了
// -------------------------------------------------------------
void
DlgInput::end()
{
	if (mwFrame) {
		werase(mwFrame);
		wrefresh(mwFrame);
		delwin(mwFrame);
		mwFrame = nullptr;
	}
}

// -------------------------------------------------------------
// カーソルを左へ移動
// -------------------------------------------------------------
void
DlgInput::keyLeft()
{
	// カーソルを左へ移動
	mTextBoxCsrX--;

	// カーソルが枠の左端を超えたら
	if (mTextBoxCsrX < 0) {
		// 表示開始位置を左へ移動
		mTextBoxCsrX = 0;
		mTextBoxOfsX--;

		// 表示開始位置が文字列先頭へ到達したらそこまで
		if (mTextBoxOfsX < 0) {
			mTextBoxOfsX = 0;
		}
	}

	// 入力処理位置を再設定
	mTextPosX = mTextBoxOfsX+mTextBoxCsrX;
}

// -------------------------------------------------------------
// カーソルを右へ移動
// -------------------------------------------------------------
void
DlgInput::keyRight()
{
	// 入力処理位置が末尾以外のときのみ動作する
	if (mTextPosX < mTextLength) {
		// カーソルを右へ移動
		mTextBoxCsrX++;
		while (!0) {
			// カーソルが枠内に収まっていれば終了
			int w = WStrUtil::widthN(mWInputText, mTextBoxOfsX, mTextBoxCsrX);
			if (w <= mTextBoxW-1) {
				break;
			}

			// 表示開始位置を右へ移動し、カーソル位置を調整
			mTextBoxOfsX++;
			mTextBoxCsrX--;
		}
	}

	// 入力処理位置を再設定
	mTextPosX = mTextBoxOfsX+mTextBoxCsrX;
}

// -------------------------------------------------------------
// カーソルを先頭へ移動
// -------------------------------------------------------------
void
DlgInput::keyHome()
{
	// カーソルを左端へ移動
	mTextBoxCsrX = 0;

	// 表示開始位置を左端へ移動
	mTextBoxOfsX = 0;

	// 入力処理位置を再設定
	mTextPosX = mTextBoxOfsX+mTextBoxCsrX;
}

// -------------------------------------------------------------
// カーソルを末尾へ移動
// -------------------------------------------------------------
void
DlgInput::keyEnd()
{
	// カーソルを枠の右端へ設定したときの枠内表示可能文字数を求める
	// 表示可能文字数＝カーソル位置となる
	WStrUtil::widthRightByWidth(mTextBoxCsrX, mWInputText, 0, mTextBoxW-1);

	// 表示可能文字数から表示開始位置を求める
	mTextBoxOfsX = mTextLength-mTextBoxCsrX;

	// 入力処理位置を再設定
	mTextPosX = mTextBoxOfsX+mTextBoxCsrX;
}

// -------------------------------------------------------------
// 入力を完了
// -------------------------------------------------------------
void
DlgInput::keyEnter()
{
	// 現在の入力文字列を返却用バッファへ戻す
	mInputText = WStrUtil::wstr2str(mWInputText);
	FDS_LOG("keyEnter: mWInputText=[%ls], mInputText=[%s]\n", mWInputText.c_str(), mInputText.c_str());
}

// -------------------------------------------------------------
// 挿入モード切替
// -------------------------------------------------------------
void
DlgInput::keyInsert()
{
	// 挿入モード切替
	mInsertMode = !mInsertMode;
}

// -------------------------------------------------------------
// Deleteキー処理
// -------------------------------------------------------------
void
DlgInput::keyDelete()
{
	// 入力処理位置が末尾以外のときのみ動作する
	if (mTextPosX < mTextLength) {
		// カーソル位置の文字を削除
		mWInputText.erase(mTextPosX, 1);
		mTextLength--;

		// 枠内の表示文字数が枠の幅より小さくなった時
		int w = WStrUtil::widthN(mWInputText, mTextBoxOfsX);
		if (w < mTextBoxW-1) {
			// 表示開始位置を左へ移動
			while (mTextBoxOfsX > 0) {
				int w2 = WStrUtil::widthN(mWInputText, mTextBoxOfsX);
				if (w2 > mTextBoxW-1) {
					// 溢れたらひとつ前の状態に戻して完了
					mTextBoxOfsX++;
					mTextBoxCsrX--;
					break;
				}
				// 表示開始位置を左へ移動し、カーソル位置を調整
				mTextBoxOfsX--;
				mTextBoxCsrX++;
			}
		}
	}

	// 入力処理位置を再設定
	mTextPosX = mTextBoxOfsX+mTextBoxCsrX;
}

// -------------------------------------------------------------
// Backspaceキー処理
// -------------------------------------------------------------
void
DlgInput::keyBackspace()
{
	// 先頭以外でのみ動作する
	if (mTextPosX > 0) {
		// [Backspace] = [Left][Delete]と同一
		keyLeft();
		keyDelete();
	}
}

// -------------------------------------------------------------
// １文字入力
// -------------------------------------------------------------
void
DlgInput::keyChar(wint_t wch)
{
	// 文字数が超過していたらbeepして終わり
	if (mTextLength >= mMaxTextLength) {
		beep();
		return;
	}

	// 文字を挿入
	mWInputText.insert(mTextPosX, 1, wch);
	mTextLength++;

	// カーソルを右へ移動
	mTextBoxCsrX++;
	while (!0) {
		// カーソルが枠の右端を超えないよう、表示開始位置を右へ移動
		int w = WStrUtil::widthN(mWInputText, mTextBoxOfsX, mTextBoxCsrX);
		if (w <= mTextBoxW-1) {
			break;
		}
		mTextBoxCsrX--;
		mTextBoxOfsX++;
	}

	// 入力処理位置を再設定
	mTextPosX = mTextBoxOfsX+mTextBoxCsrX;
}

// =====================================================================
// [EOF]
