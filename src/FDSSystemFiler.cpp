// ◇
// fds: fdx68 selector
// FDSSystem: FDSシステム:ファイラー
// Copyright: (C)2020 Hiroaki GOTO as GORRY.
// License: see readme.txt
// =====================================================================

#include "FDSSystem.h"

// =====================================================================
// FDSSystem: FDSシステム:ファイラー
// =====================================================================

// -------------------------------------------------------------
// ビュー作成
// -------------------------------------------------------------
void
FDSSystem::filerViewCreateWindow()
{
	if (mwFilerView) {
		filerViewDestroyWindow();
	}
	mwFilerView = newwin(
	  mFilerViewXYWH.h(),
	  mFilerViewXYWH.w(),
	  mFilerViewXYWH.y(),
	  mFilerViewXYWH.x()
	);
	keypad(mwFilerView, TRUE);

	mFilerViewInnerW = mFilerViewXYWH.w()-2;
	mFilerViewInnerH = mFilerViewXYWH.h()-2;
	mFilerViewOfsY = 0;
	mFilerViewCsrY = 0;
	mFilerViewWindowOfsX = 1;
	mFilerViewWindowOfsY = 1;
}

// -------------------------------------------------------------
// ビュー破棄
// -------------------------------------------------------------
void
FDSSystem::filerViewDestroyWindow()
{
	if (mwFilerView) {
		delwin(mwFilerView);
		mwFilerView = nullptr;
	}
}

// -------------------------------------------------------------
// ビュー再描画
// -------------------------------------------------------------
void
FDSSystem::filerViewRedraw()
{
	redrawwin(mwFilerView);
	filerViewRefresh();
}

// -------------------------------------------------------------
// ビュー更新
// -------------------------------------------------------------
void
FDSSystem::filerViewRefresh()
{
	// ビューのクリア
	werase(mwFilerView);

	std::wstring buf(mFilerViewInnerW, L' ');
	FDSSystem::HelpViewMode md = FDSSystem::HelpViewMode::Dir;
	int y = mFilerViewOfsY;
	for (int i=0; i<mFilerViewInnerH; i++) {
		// 選択肢カーソルを表示
		bool sel = false;
		FDSSystem::ColorPair col = FDSSystem::ColorPair::Normal;
		if (y < (int)mFiles.size()) {
			col = (FDSSystem::ColorPair)((int)ColorPair::FilerUnknown + (int)mFiles[y].type());
			wattron(mwFilerView, COLOR_PAIR(col));
		}
		if (i == mFilerViewCsrY) {
			col = (FDSSystem::ColorPair)((int)col + (int)DirEntry::Type::Max);
			wattron(mwFilerView, COLOR_PAIR(col)|A_BOLD);
			sel = true;
		}
		mvwaddwstr(mwFilerView, mFilerViewWindowOfsY+i, mFilerViewWindowOfsX, buf.c_str());

		// アイテムを表示
		if (y < (int)mFiles.size()) {
			std::wstring line;
			wmove(mwFilerView, mFilerViewWindowOfsY+i, mFilerViewWindowOfsX+2);
			switch (mFiles[y].type()) {
			  case DirEntry::Type::ParentDir:
				WStrUtil::copyByWidth(line, mFiles[y].wfilename()+L"/", 0, mFilerViewInnerW-3);
				waddwstr(mwFilerView, line.c_str());
				if (sel) {
					md = FDSSystem::HelpViewMode::ParentDir;
				}
				break;
			  case DirEntry::Type::Dir:
				WStrUtil::copyByWidth(line, mFiles[y].wfilename()+L"/", 0, mFilerViewInnerW-3);
				waddwstr(mwFilerView, line.c_str());
				if (sel) {
					md = FDSSystem::HelpViewMode::Dir;
				}
				break;
			  case DirEntry::Type::FdxFile:
				WStrUtil::copyByWidth(line, mFiles[y].wfilename(), 0, mFilerViewInnerW-3);
				waddwstr(mwFilerView, line.c_str());
				if (sel) {
					md = FDSSystem::HelpViewMode::FdxFile;
				}
				break;
			  default:
			  case DirEntry::Type::OtherFile:
				WStrUtil::copyByWidth(line, mFiles[y].wfilename(), 0, mFilerViewInnerW-3);
				waddwstr(mwFilerView, line.c_str());
				if (sel) {
					md = FDSSystem::HelpViewMode::OtherFile;
				}
				break;
			}
			// プロテクトマークを表示
			if (mFiles[y].isProtect()) {
				if (sel) {
					wattron(mwFilerView, COLOR_PAIR(ColorPair::FilerProtectedCsr));
				} else {
					wattron(mwFilerView, COLOR_PAIR(ColorPair::FilerProtected));
				}
				wmove(mwFilerView, mFilerViewWindowOfsY+i, mFilerViewWindowOfsX+1);
				mvwaddstr(mwFilerView, mFilerViewWindowOfsY+i, mFilerViewWindowOfsX+1, "*");
				if (sel) {
					wattroff(mwFilerView, COLOR_PAIR(ColorPair::FilerProtectedCsr));
				} else {
					wattroff(mwFilerView, COLOR_PAIR(ColorPair::FilerProtected));
				}
			}
		}
		if (sel) {
			wattroff(mwFilerView, COLOR_PAIR(col)|A_BOLD);
		} else {
			wattroff(mwFilerView, COLOR_PAIR(col));
		}
		y++;
	}

	// 選択内容に応じてヘルプビューを更新
	helpViewSetMode(md);
	helpViewRefresh();

	// 枠を追加して更新
	wborder(mwFilerView, 0,0,0,0,ACS_LTEE,ACS_RTEE,ACS_LTEE,ACS_RTEE);
	wrefresh(mwFilerView);

}

// -------------------------------------------------------------
// ファイラービュー表示開始位置をファイルリストidxで設定
// -------------------------------------------------------------
void
FDSSystem::filerViewSetViewOfsY(int idx)
{
	// idxがファイルリスト先頭より前を差さないよう調整
	if (idx < 0) {
		idx = 0;
	}

	// 表示開始位置が（ファイルリスト末尾－ビュー表示幅）より後を差さないよう調整
	int h = mFiles.size() - mFilerViewInnerH;
	if (h < 0) {
		h = 0;
	}
	if (idx > h) {
		idx = h;
	}

	mFilerViewOfsY = idx;
}

// -------------------------------------------------------------
// ファイラービュー表示開始位置を取得
// -------------------------------------------------------------
int
FDSSystem::filerViewGetViewOfsY()
{
	return mFilerViewOfsY;
}

// -------------------------------------------------------------
// ファイラービューカーソル位置をビュー位置yで設定
// -------------------------------------------------------------
void
FDSSystem::filerViewSetCsrY(int y)
{
	// カーソルがビュー先頭より前を差さないよう調整
	if (y < 0) {
		y = 0;
	}

	// カーソルがビュー最終位置として差せる位置を算出
	int h = mFiles.size()-mFilerViewOfsY;
	if (h > mFilerViewInnerH) {
		h = mFilerViewInnerH;
	}

	// カーソルがビュー最終位置より後を差さないよう調整
	if (y > h-1) {
		y = h-1;
	}

	mFilerViewCsrY = y;
}

// -------------------------------------------------------------
// ファイルリストidxから表示開始位置とカーソル位置を設定
// -------------------------------------------------------------
void
FDSSystem::filerViewSetIdx(int idx)
{
	// idxがファイルリスト先頭より前を差さないよう調整
	if (idx < 0) {
		idx = 0;
	}

	// idxがファイルリスト末尾より後を差さないよう調整
	int h = mFiles.size()-1;
	if (idx > h) {
		idx = h;
	}

	// 現在見えているビュー内にあれば、カーソルを移動して終了
	if (mFilerViewOfsY <= (int)mFiles.size()-mFilerViewInnerH) {
		int y = idx-mFilerViewOfsY;
		if ((0 <= y) && (y < mFilerViewInnerH)) {
			mFilerViewCsrY = y;
			return;
		}
	}

	// 表示開始位置とカーソルを設定
	filerViewSetViewOfsY(idx);
	idx -= mFilerViewOfsY;
	filerViewSetCsrY(idx);
}

// -------------------------------------------------------------
// カーソルを上へ
// -------------------------------------------------------------
void
FDSSystem::filerViewUpCursor()
{
	mFilerViewCsrY--;

	// カーソルがビュー先頭より前を差さないよう調整
	if (mFilerViewCsrY < 0) {
		mFilerViewCsrY = 0;
		mFilerViewOfsY--;

		// 表示開始位置がファイルリスト先頭より前を差さないよう調整
		if (mFilerViewOfsY < 0) {
			mFilerViewOfsY = 0;
		}
	}
}

// -------------------------------------------------------------
// カーソルを下へ
// -------------------------------------------------------------
void
FDSSystem::filerViewDownCursor()
{
	mFilerViewCsrY++;

	// カーソルがファイルリスト末尾より後を差さないよう調整
	int idx = mFiles.size()-1;
	if (mFilerViewCsrY > idx) {
		mFilerViewCsrY = idx;
	}

	// カーソルがビュー最下段より後を差さないよう調整
	int h = mFilerViewInnerH-1;
	if (mFilerViewCsrY > h) {
		mFilerViewCsrY = h;
		filerViewSetViewOfsY(mFilerViewOfsY+1);
	}
}

// -------------------------------------------------------------
// ビューを１ページ前へ
// -------------------------------------------------------------
void
FDSSystem::filerViewPageUpCursor()
{
	// カーソルがビュー最上段でなければ最上段へ移動
	if (mFilerViewCsrY > 0) {
		mFilerViewCsrY = 0;
	} else {
		// カーソルがビュー最上段なら表示開始位置を１ページ前へ
		mFilerViewOfsY -= mFilerViewInnerH;
		if (mFilerViewOfsY < 0) {
			mFilerViewOfsY = 0;
		}
	}
}

// -------------------------------------------------------------
// ビューを１ページ後へ
// -------------------------------------------------------------
void
FDSSystem::filerViewPageDownCursor()
{
	int h = mFilerViewInnerH-1;
	if (mFilerViewCsrY < h) {
		// カーソルがビュー最下段でなければ最下段へ移動
		mFilerViewCsrY = h;

		// カーソルがファイルリスト末尾を超えないよう調整
		h = mFiles.size()-1;
		if (mFilerViewCsrY > h) {
			mFilerViewCsrY = h;
		}
	} else {
		// カーソルがビュー最下段なら表示開始位置を１ページ後へ
		mFilerViewCsrY = h;
		mFilerViewOfsY += mFilerViewInnerH;

		// 表示開始位置が（ファイルリスト末尾－ビュー表示幅）より後を差さないよう調整
		int h2 = mFiles.size()-mFilerViewInnerH;
		if (h2 < 0) {
			h2 = 0;
		}
		if (mFilerViewOfsY >= h2) {
			mFilerViewOfsY = h2;
		}
	}
}

// -------------------------------------------------------------
// ビューをファイルリスト先頭へ
// -------------------------------------------------------------
void
FDSSystem::filerViewPageTopCursor()
{
	mFilerViewOfsY = 0;
	mFilerViewCsrY = 0;
}

// -------------------------------------------------------------
// ビューをファイルリスト末尾へ
// -------------------------------------------------------------
void
FDSSystem::filerViewPageBottomCursor()
{
	mFilerViewOfsY = mFiles.size()-mFilerViewInnerH;
	if (mFilerViewOfsY < 0) {
		mFilerViewOfsY = 0;
	}
	mFilerViewCsrY = mFiles.size()-mFilerViewOfsY-1;
}

// -------------------------------------------------------------
// ファイルを選択
// -------------------------------------------------------------
void
FDSSystem::filerViewSelectEntry()
{
	// 選択位置をチェック
	int idx = filerViewGetIdx();
	if ((idx < 0) || (idx >= (int)mFiles.size())) {
		return;
	}

	// ".."を選択
	if (mFiles[idx].isParentDir()) {
		// 移動先ディレクトリをスタックからポップ
		DirStack::Stack stack = mDirStack.pop();
		mFilerViewOfsY = stack.mFilerViewOfsY;
		mFilerViewCsrY = stack.mFilerViewCsrY;
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
		stack.mFilerViewOfsY = mFilerViewOfsY;
		stack.mFilerViewCsrY = mFilerViewCsrY;
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
		filerViewSetIdx(0);
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
}

// -------------------------------------------------------------
// ひとつ前のディレクトリへ戻る
// -------------------------------------------------------------
void
FDSSystem::filerViewBackDir()
{
	// ディレクトリスタックが残ってなければ終了
	if (mDirStack.level() <= 0) {
		return;
	}

	// ディレクトリスタックからポップ
	DirStack::Stack stack = mDirStack.pop();
	mFilerViewOfsY = stack.mFilerViewOfsY;
	mFilerViewCsrY = stack.mFilerViewCsrY;
	mRootDir = stack.mRootDir;
	mCurDir = stack.mCurDir;

	// 新しいファイルリストを取得
	mFiles.setPath(mRootDir+mCurDir);
	mFiles.getFiles(mCurDir.empty());
	mFiles.sortFiles();

	// パスビューを更新
	pathViewRefresh();
}

// -------------------------------------------------------------
// システムのルートディレクトリへ戻る
// -------------------------------------------------------------
void
FDSSystem::filerViewRootDir()
{
	// ディレクトリスタックを全部掃き出し、最初に積んだ情報に更新する
	while (mDirStack.level() > 0) {
		DirStack::Stack stack = mDirStack.pop();
		mFilerViewOfsY = stack.mFilerViewOfsY;
		mFilerViewCsrY = stack.mFilerViewCsrY;
		mRootDir = stack.mRootDir;
		mCurDir = stack.mCurDir;
	}

	// 新しいファイルリストを主直
	mFiles.setPath(mRootDir+mCurDir);
	mFiles.getFiles(mCurDir.empty());
	mFiles.sortFiles();

	// パスビューを更新
	pathViewRefresh();
}

// -------------------------------------------------------------
// ファイラービューのカーソル位置を得る
// -------------------------------------------------------------
int
FDSSystem::filerViewGetCsrY()
{
	return mFilerViewCsrY;
}

// -------------------------------------------------------------
// ファイラービューで選択したエントリのidxを得る
// -------------------------------------------------------------
int
FDSSystem::filerViewGetIdx()
{
	int idx = mFilerViewOfsY + mFilerViewCsrY;
	int h = (int)mFiles.size()-1;
	if (idx > h) {
		idx = h;
	}
	return idx;
}

// -------------------------------------------------------------
// ファイラービューで選択したFDXファイルの情報を情報ビューに表示する
// -------------------------------------------------------------
void
FDSSystem::filerViewShowFileInfo()
{
	// 選択位置をチェック
	int idx = filerViewGetIdx();
	if (idx >= (int)mFiles.size()) {
		return;
	}

	// 選択したFDXファイルで情報ビューを更新
	std::string path = mRootDir + mCurDir + mFiles[idx].filename();
	infoViewSetFile(path);
	infoViewRefresh();
}

// -------------------------------------------------------------
// ファイラービューからファイルを検索し、見つかればそれを指す
// -------------------------------------------------------------
int
FDSSystem::filerViewFindEntry(const std::string& filename)
{
	// ファイルリストにあるか探す
	int idx;
	for (idx=0; idx<(int)mFiles.size(); idx++) {
		if (mFiles[idx].filename() == filename) {
			break;
		}
	}

	// 見つからなければ終了
	if (idx >= (int)mFiles.size()) {
		return -1;
	}

	// 新規にビュー位置を設定する
	filerViewSetIdx(idx);
	return idx;
}


// =====================================================================
// [EOF]
