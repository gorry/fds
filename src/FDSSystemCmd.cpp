// ◇
// fds: fdx68 selector
// FDSSystem: FDSシステム:コマンド
// Copyright: (C)2020 Hiroaki GOTO as GORRY.
// License: see readme.txt
// =====================================================================

#include "FDSSystem.h"

// =====================================================================
// FDSSystem: FDSシステム:コマンド
// =====================================================================

// -------------------------------------------------------------
// ファイラービューで選択したエントリをリネームする
// -------------------------------------------------------------
void
FDSSystem::cmdRename()
{
	// 選択位置をチェック
	int idx = filerViewGetIdx();
	if (idx >= (int)mFiles.size()) {
		return;
	}

	// ディレクトリかFDXファイルのみ受け付ける
	if (!mFiles[idx].isFdxFile() && !mFiles[idx].isNormalDir()) {
		return;
	}

	// 選択したエントリで情報ビューを更新
	std::string path = mRootDir + mCurDir + mFiles[idx].filename();
	if (mFiles[idx].isDir()) {
		path += "/";
	}
	infoViewSetFile(path);
	infoViewRefresh();
	int ret = infoViewGetResult();
	if (ret == -1) {
		return;
	}
	if (ret < -1) {
		// 正常に情報取得できなければ中止
		DlgSelect dlg;
		dlg.setItemsOk();
		dlg.setHeader("FDX File Info Error");
		dlg.setCanEscape(true);
		ret = dlg.start();
		return;
	}

	std::string filename = mFiles[idx].filename();
	while (!0) {
		// 新しいファイル名を入力
		DlgInput dlg;
		dlg.setHeader("Rename to:");
		dlg.setText(filename);
		dlg.setMaxLength(FDX_FILENAME_MAX);
		dlg.setCanEscape(true);
		ret = dlg.start(COLS/2);

		// 入力が行われなかったら中止
		if (ret < 0) {
			return;
		}

		// 入力が空だったら最初の名前でやり直し
		std::string newname = dlg.getText();
		if (newname.empty()) {
			continue;
		}

		// FDXファイル名の入力結果の拡張子が".FDX"でなかったらやり直し
		if (mFiles[idx].isFdxFile()) {
			std::wstring wnewname = WStrUtil::str2wstr(newname);
			size_t len = wnewname.length();
			if ((len < 5) || (WStrUtil::wstricmp(&wnewname[len-4], L".FDX"))) {
				FDS_ERROR("cmdRename: Use Extention \".FDX\"!\n");
				FDS_ERROR(" newname=[%s]\n", newname.c_str());
				DlgSelect dlg2;
				dlg2.setItemsOk();
				dlg2.setHeader("Use Extention \".FDX\"!");
				dlg2.setCanEscape(true);
				dlg2.start();
				dlg2.end();
				refreshAllView();
				filename = newname;
				continue;
			}
		}

		// ファイル名に"/"が含まれていたらやり直し
		if (std::string::npos != newname.find('/')) {
			FDS_ERROR("cmdRenameDisk: Can't use '/'!\n");
			FDS_ERROR(" newname=[%s]\n", newname.c_str());
			DlgSelect dlg2;
			dlg2.setItemsOk();
			dlg2.setHeader("Can't use '/'!");
			dlg2.setCanEscape(true);
			dlg2.start();
			dlg2.end();
			refreshAllView();
			filename = newname;
			continue;
		}

		// 名前が変わらなかったら中止
		if (mFiles[idx].filename() == newname) {
			return;
		}

		// 先ファイルがすでにあったらやり直し
		std::string src = mRootDir + mCurDir + mFiles[idx].filename();
		std::string dst = mRootDir + mCurDir + newname;
		FILE* fin = fopen(dst.c_str(), "rb");
		if (fin != nullptr) {
			fclose(fin);
			FDS_ERROR("cmdRenameDisk: Disk Already Exist!\n");
			FDS_ERROR(" newname=[%s]\n", newname.c_str());
			DlgSelect dlg2;
			dlg2.setItemsOk();
			dlg2.setHeader("Disk Already Exist!");
			dlg2.setCanEscape(true);
			dlg2.start();
			dlg2.end();
			refreshAllView();
			filename = newname;
			continue;
		}

		// リネーム
		int ret2 = rename(src.c_str(), dst.c_str());
		if (ret2 != 0) {
			FDS_ERROR("cmdRename: Rename Failed!\n");
			FDS_ERROR(" src=[%s], dst=[%s], result=%d, errno=%d\n", src.c_str(), dst.c_str(), ret2, errno);
			DlgSelect dlg2;
			dlg2.setItemsOk();
			dlg2.setHeader("Rename Failed!");
			dlg2.setCanEscape(true);
			dlg2.start();
			return;
		}

		// 新しいファイルリストを取得
		mFiles.getFiles(mCurDir.empty());
		mFiles.sortFiles();
		filerViewFindEntry(newname);

		infoViewSetFile(dst);
		infoViewRefresh();
		break;
	}
}

// -------------------------------------------------------------
// ディレクトリ内のイメージを自動挿入する
// ・ファイル名末尾に"1"または"A"が付いたファイルを第１ドライブへ挿入
// ・ファイル名末尾に"2"または"B"が付いたファイルを第２ドライブへ挿入
// ・条件に合うファイルが複数ある場合は、先にリストされているファイルを優先
// ディレクトリを選択している場合は、選択したディレクトリ内のファイルを対象とする
// 選択していない場合は、現在のディレクトリ内のファイルを対象とする
// -------------------------------------------------------------
void
FDSSystem::cmdAutoSet()
{
	// 選択位置をチェック
	int idx = filerViewGetIdx();
	if (idx >= (int)mFiles.size()) {
		return;
	}

	// 現在のディレクトリ内容から自動挿入ファイルを選ぶ
	Directory mydir;
	Directory* files = &mFiles;
	std::string path = mRootDir + mCurDir;
	if (mFiles[idx].isNormalDir()) {
		// ディレクトリを選んでいる場合は、選んだディレクトリ内容から自動挿入ファイルを選ぶ
		path += mFiles[idx].filename()+"/";
		mydir.setPath(path);
		mydir.getFiles(true);
		mydir.sortFiles();
		files = &mydir;
	}

	// 全ドライブからイメージを排出し、FDDビューを更新しておく
	for (int i=0; i<FddEmu::Drives; i++) {
		mFddEmu.ejectDrive(i);
	}
	fddViewRefresh();

	// ファイルを検索してドライブへ挿入する
	std::string path2;
	bool setDisk[FddEmu::Drives] = {};
	int disks = 0;
	for (size_t i=0; i<files->size(); i++) {
		if (files->entry(i).isFdxFile()) {
			const std::wstring& s = files->entry(i).wfilename();
			size_t len = s.length();
			wchar_t c = s[len-5];
			switch (c) {
			  case L'1':
			  case L'A':
			  case L'a':
				if (!setDisk[0]) {
					path2 = path + files->entry(i).filename();
					mFddEmu.setImage(0, path2);
					setDisk[0] = true;
					disks++;
				}
				break;
			  case L'2':
			  case L'B':
			  case L'b':
				if (!setDisk[1]) {
					path2 = path + files->entry(i).filename();
					mFddEmu.setImage(1, path2);
					setDisk[1] = true;
					disks++;
				}
				break;
			}
			if (disks >= FddEmu::Drives) break;
		}
	}

	// FDDビューを挿入後の状態で更新する
	fddViewRefresh();

	// １枚もディスクが選ばれなかった場合は警告ダイアログを出しておく
	if (disks == 0) {
		DlgSelect dlg;
		dlg.setItemsOk();
		dlg.setHeader("No Disk Found!");
		dlg.setCanEscape(true);
		dlg.start();
	}
}

// -------------------------------------------------------------
// イメージを挿入する
// -------------------------------------------------------------
void
FDSSystem::cmdSetDrive(int id)
{
	// 選択位置をチェック
	int idx = filerViewGetIdx();
	if (idx >= (int)mFiles.size()) {
		return;
	}

	// FDXファイルのみ受け付ける
	if (!mFiles[idx].isFdxFile()) {
		return;
	}

	// ドライブへ挿入
	std::string src = mRootDir + mCurDir + mFiles[idx].filename();
	mFddEmu.setImage(id, src);

	// FDDビューを挿入後の状態で更新する
	fddViewRefresh();
}

// -------------------------------------------------------------
// 全ドライブのイメージを排出する
// -------------------------------------------------------------
void
FDSSystem::cmdEjectAllDrive()
{
	// 全ドライブのイメージを排出
	for (int i=0; i<FddEmu::Drives; i++) {
		mFddEmu.ejectDrive(i);
	}

	// FDDビューを排出後の状態で更新する
	fddViewRefresh();
}

// -------------------------------------------------------------
// イメージを作成する
// -------------------------------------------------------------
void
FDSSystem::cmdCreateDisk()
{
	// 選択位置をチェック
	int idx = filerViewGetIdx();
	if (idx >= (int)mFiles.size()) {
		return;
	}

	// フォーマット選択肢択準備
	DlgSelect::ItemsVec items;
	int n = mConfig.cfgMachine().numFormat();
	for (int i=0; i<n; i++) {
		items.push_back(mConfig.cfgMachine().format(i).name());
	}
	items.push_back("[ Cancel ]");

	// フォーマット選択肢
	int sel = 0;
	while (!0) {
	  format:;
		// ダイアログ表示
		DlgSelect dlg;
		dlg.setItemsVec(items);
		dlg.setHeader("[Select Format]");
		dlg.setCanEscape(true);
		sel = dlg.start(sel);

		// [Cancel]を選んだら終了
		if ((sel == -1) || (sel == (int)items.size()-1)) {
			break;
		}

		// 選択した設定を読み込む
		char section[FDX_FILENAME_MAX];
		sprintf(section, "FORMAT-%d", sel+1);
		std::string name = mConfig.cfgMachine().format(sel).name();
		std::string fdxtoolopt = mConfig.cfgMachine().format(sel).fdxToolOpt();
		std::string filename = mConfig.cfgMachine().format(sel).fileName();
		filename = FDSMachine::fileNameWithTime(filename);

		while (!0) {
			// 新しいファイル名を入力
			DlgInput dlg2;
			dlg2.setHeader("Create ["+name+"] Disk:");
			dlg2.setText(filename);
			dlg2.setCanEscape(true);
			dlg2.setMaxLength(FDX_FILENAME_MAX);
			int ret = dlg2.start(COLS/2);

			// 入力が行われなかったらフォーマット選択に戻る
			if (ret < 0) {
				goto format;
			}

			// 入力が空だったら最初の名前でやり直し
			std::string newname = dlg2.getText();
			if (newname.empty()) {
				refreshAllView();
				continue;
			}
			// 入力結果の拡張子が".FDX"でなかったらやり直し
			std::wstring wnewname = WStrUtil::str2wstr(newname);
			size_t len = wnewname.length();
			if ((len < 5) || (WStrUtil::wstricmp(&wnewname[len-4], L".FDX"))) {
				FDS_ERROR("cmdCreateDisk: Use Extention \".FDX\"!\n");
				FDS_ERROR(" newname=[%s]\n", newname.c_str());
				DlgSelect dlg3;
				dlg3.setItemsOk();
				dlg3.setHeader("Use Extention \".FDX\"!");
				dlg3.setCanEscape(true);
				dlg3.start();
				dlg3.end();
				refreshAllView();
				filename = newname;
				continue;
			}

			// ファイル名に"/"が含まれていたらやり直し
			if (std::string::npos != newname.find('/')) {
				FDS_ERROR("cmdCreateDisk: Can't use '/'!\n");
				FDS_ERROR(" newname=[%s]\n", newname.c_str());
				DlgSelect dlg3;
				dlg3.setItemsOk();
				dlg3.setHeader("Can't use '/'!");
				dlg3.setCanEscape(true);
				dlg3.start();
				dlg3.end();
				refreshAllView();
				filename = newname;
				continue;
			}

			// すでにファイルがあったらやり直し
			std::string dst = mRootDir + mCurDir + newname;
			FILE* fin = fopen(dst.c_str(), "rb");
			if (fin) {
				fclose(fin);
				// 失敗
				FDS_ERROR("cmdCreateDisk: Disk Already Exist!\n");
				FDS_ERROR(" dst=[%s]\n", dst.c_str());
				DlgSelect dlg3;
				dlg3.setItemsOk();
				dlg3.setHeader("Disk Already Exist!");
				dlg3.setCanEscape(true);
				dlg3.start();
				dlg3.end();
				refreshAllView();
				filename = newname;
				continue;
			}

			// イメージを作成
			std::string option = fdxtoolopt+" \""+dst.c_str()+"\"";
			std::string cmd = mConfig.fdxToolCmd();
			int ret2 = mFdxTool.execCmd(cmd.c_str(), option.c_str());
			if (ret2 != 0) {
				// 失敗
				FDS_ERROR("cmdCreateDisk: Create Disk Failed!\n");
				FDS_ERROR(" dst=[%s], type=[%s], cmd=[%s], option=[%s], result=%d\n", dst.c_str(), name.c_str(), cmd.c_str(), option.c_str(), ret2);
				DlgSelect dlg3;
				dlg3.setItemsOk();
				dlg3.setHeader("Create Disk Failed!");
				dlg3.setCanEscape(true);
				dlg3.start();
				return;
			}
#if !defined(FDS_WINDOWS)
			if (!mNoRoot) {
				chmod(dst.c_str(), 0666);
			}
#endif

			// 新しいファイルリストを取得
			mFiles.getFiles(mCurDir.empty());
			mFiles.sortFiles();
			filerViewFindEntry(newname);

			// 作成したファイルの情報を表示
			infoViewSetFile(dst);
			infoViewRefresh();

			return;
		}
	}
}

// -------------------------------------------------------------
// イメージを複製する
// -------------------------------------------------------------
void
FDSSystem::cmdDupDisk()
{
	// 選択位置をチェック
	int idx = filerViewGetIdx();
	if (idx >= (int)mFiles.size()) {
		return;
	}

	// FDXファイルのみ受け付ける
	if (!mFiles[idx].isFdxFile()) {
		return;
	}

	// 選択したエントリで情報ビューを更新
	std::string path = mRootDir + mCurDir + mFiles[idx].filename();
	if (mFiles[idx].isDir()) {
		path += "/";
	}
	infoViewSetFile(path);
	infoViewRefresh();
	int ret = infoViewGetResult();
	if (ret == -1) {
		return;
	}
	if (ret < -1) {
		// 正常に情報取得できなければ中止
		DlgSelect dlg;
		dlg.setItemsOk();
		dlg.setHeader("FDX File Info Error");
		dlg.setCanEscape(true);
		dlg.start();
		return;
	}

	std::string filename = mFiles[idx].filename();
	std::string newfile = filename.substr(0, filename.length()-4)+"_Copy.fdx";
	while (!0) {
		// 新しいファイル名を入力
		DlgInput dlg;
		dlg.setHeader("Copy to:");
		dlg.setText(newfile);
		dlg.setMaxLength(FDX_FILENAME_MAX);
		dlg.setCanEscape(true);
		ret = dlg.start(COLS/2);

		// 入力が行われなかったら終了
		if (ret < 0) {
			return;
		}

		// 入力が空だったら最初の名前でやり直し
		std::string newname = dlg.getText();
		if (newname.empty()) {
			continue;
		}

		// 入力結果の拡張子が".FDX"でなかったらやり直し
		std::wstring wnewname = WStrUtil::str2wstr(newname);
		size_t len = wnewname.length();
		if ((len < 5) || (WStrUtil::wstricmp(&wnewname[len-4], L".FDX"))) {
			FDS_ERROR("cmdDup: Use Extention \".FDX\"!\n");
			FDS_ERROR(" newname=[%s]\n", newname.c_str());
			DlgSelect dlg3;
			dlg3.setItemsOk();
			dlg3.setHeader("Use Extention \".FDX\"!");
			dlg3.setCanEscape(true);
			dlg3.start();
			dlg3.end();
			refreshAllView();
			newfile = newname;
			continue;
		}

		// ファイル名に"/"が含まれていたらやり直し
		if (std::string::npos != newname.find('/')) {
			FDS_ERROR("cmdDupDisk: Can't use '/'!\n");
			FDS_ERROR(" newname=[%s]\n", newname.c_str());
			DlgSelect dlg3;
			dlg3.setItemsOk();
			dlg3.setHeader("Can't use '/'!");
			dlg3.setCanEscape(true);
			dlg3.start();
			dlg3.end();
			refreshAllView();
			filename = newname;
			continue;
		}

		// 先ファイルがすでにあったらやり直し
		newfile = dlg.getText();
		std::string src = mRootDir + mCurDir + filename;
		std::string dst = mRootDir + mCurDir + newfile;
		FILE* fin = fopen(dst.c_str(), "rb");
		if (fin != nullptr) {
			fclose(fin);
			FDS_ERROR("cmdDupDisk: Disk Already Exist!\n");
			FDS_ERROR(" newname=[%s]\n", newname.c_str());
			DlgSelect dlg2;
			dlg2.setItemsOk();
			dlg2.setHeader("Disk Already Exist!");
			dlg2.setCanEscape(true);
			dlg2.start();
			dlg2.end();
			refreshAllView();
			filename = newname;
			continue;
		}

		// コピー
#if defined(FDS_WINDOWS)
		int ret2 = (CopyFileA(src.c_str(), dst.c_str(), FALSE) ? 0 : -1);
#else
		std::string cmd = "cp";
		std::string option = "-pf \"" + src + "\" \"" + dst + "\"";
		int ret2 = mFdxTool.execCmd(cmd.c_str(), option.c_str());
#endif  // defined(FDS_WINDOW)
		if (ret2 != 0) {
			// 失敗
			FDS_ERROR("cmdDupDisk: Copy Failed!\n");
			FDS_ERROR(" src=[%s], dst=[%s], result=%d\n", src.c_str(), dst.c_str(), ret2);
			DlgSelect dlg3;
			dlg3.setItemsOk();
			dlg3.setHeader("Copy Failed!");
			dlg3.setCanEscape(true);
			dlg3.start();
		}
#if !defined(FDS_WINDOWS)
		if (!mNoRoot) {
			chmod(dst.c_str(), 0666);
		}
#endif

		// 新しいファイルリストを取得
		mFiles.getFiles(mCurDir.empty());
		mFiles.sortFiles();
		filerViewFindEntry(filename);

		break;
	}
}

// -------------------------------------------------------------
// ディレクトリを作成する
// -------------------------------------------------------------
void
FDSSystem::cmdMakeDirectory()
{
	// 選択位置をチェック
	int idx = filerViewGetIdx();
	if (idx >= (int)mFiles.size()) {
		return;
	}

	std::string filename = "";
	while (!0) {
		// 新しいファイル名を入力
		DlgInput dlg;
		dlg.setHeader("Make Directory:");
		dlg.setText(filename);
		dlg.setCanEscape(true);
		dlg.setMaxLength(FDX_FILENAME_MAX);
		int ret = dlg.start(COLS/2);

		// 入力が行われなかったら中止
		if (ret < 0) {
			return;
		}

		// 入力が空だったら中止
		std::string newname = dlg.getText();
		if (newname.empty()) {
			return;
		}

		// ファイル名に"/"が含まれていたらやり直し
		if (std::string::npos != newname.find('/')) {
			FDS_ERROR("cmdMakeDirectory: Can't use '/'!\n");
			FDS_ERROR(" newname=[%s]\n", newname.c_str());
			DlgSelect dlg2;
			dlg2.setItemsOk();
			dlg2.setHeader("Can't use '/'!");
			dlg2.setCanEscape(true);
			dlg2.start();
			dlg2.end();
			refreshAllView();
			filename = newname;
			continue;
		}

		// ディレクトリ作成
		std::string dst = mRootDir + mCurDir + newname;
#if defined(FDS_WINDOWS)
		int ret2 = _mkdir(dst.c_str());
#else
		int ret2 = mkdir(dst.c_str(), 0755);
		if (!mNoRoot) {
			chmod(dst.c_str(), 0777);
		}
#endif
		if (ret2 != 0) {
			FDS_ERROR("cmdMakeDirectory: Make Directory Failed!\n");
			FDS_ERROR(" dst=[%s], result=%d, errno=%d\n", dst.c_str(), ret2, errno);
			DlgSelect dlg3;
			dlg3.setItemsOk();
			dlg3.setHeader("Make Directory Failed!");
			dlg3.setCanEscape(true);
			dlg3.start();
		}

		// 新しいファイルリストを取得
		mFiles.getFiles(mCurDir.empty());
		mFiles.sortFiles();
		filerViewFindEntry(newname);

		// 新しいディレクトリの情報を表示
		infoViewSetFile(dst);
		infoViewRefresh();

		return;
	}
}

// -------------------------------------------------------------
// 指定ドライブのイメージを排出する
// -------------------------------------------------------------
void
FDSSystem::cmdEjectDrive()
{
	// 排出先ドライブを選択
	int sel = 0;
	DlgSelect::ItemsVec items;
	for (int i=0; i<mFddEmu.Drives; i++) {
		items.push_back(getDriveName(i));
	}
	{
		DlgSelect dlg;
		dlg.setItemsVec(items);
		dlg.setHeader("[Eject Drive]");
		dlg.setCanEscape(true);
		sel = dlg.start(sel);
		if (sel < 0) {
			return;
		}
	}

	// イメージをドライブへ挿入
	mFddEmu.ejectDrive(sel);

	// FDDビューを更新
	fddViewRefresh();
}

// -------------------------------------------------------------
// ディスク名を編集する
// -------------------------------------------------------------
void
FDSSystem::cmdEditName()
{
	// 選択位置をチェック
	int idx = filerViewGetIdx();
	if (idx >= (int)mFiles.size()) {
		return;
	}

	// FDXファイルのみ受け付ける
	if (!mFiles[idx].isFdxFile()) {
		return;
	}

	// 書き込み不可ならエラー
	if (mFiles[idx].isProtect()) {
		DlgSelect dlg;
		dlg.setItemsOk();
		dlg.setHeader("Write Protected!");
		dlg.setCanEscape(true);
		dlg.start();
		return;
	}

	// 選択したFDXファイルで情報ビューを更新
	std::string path = mRootDir + mCurDir + mFiles[idx].filename();
	if (mFiles[idx].isDir()) {
		path += "/";
	}
	infoViewSetFile(path);
	infoViewRefresh();
	int ret = infoViewGetResult();
	if (ret == -1) {
		return;
	}
	if (ret < -1) {
		// 正常に情報取得できなければ中止
		DlgSelect dlg;
		dlg.setItemsOk();
		dlg.setHeader("FDX File Info Error!");
		dlg.setCanEscape(true);
		dlg.start();
		return;
	}

	// ディスク名をダイアログで処理
	FdxHeader& disk = infoViewGetFdxHeader();
	std::string name = std::string((const char*)&(disk.mName[0]));
	{
		// 新しいディスク名を入力
		DlgInput dlg;
		dlg.setHeader("Disk Name:");
		dlg.setMaxLength(59);
		dlg.setText(name.c_str());
		dlg.setCanEscape(true);
		ret = dlg.start(COLS/2);

		// 入力が行われなかったら中止
		if (ret < 0) {
			return;
		}

		// 入力が行われたら変更作業（空もあり）
		std::string newname = dlg.getText();
		std::wstring wnewname = WStrUtil::str2wstr(newname);
		name = WStrUtil::wstr2strN(wnewname, 60);
		strcpy((char *)&(disk.mName[0]), name.c_str());

		// 情報を更新
		int ret2 = infoViewSaveFile();
		if (ret2 < 0) {
			// 正常に更新できなければエラー表示
			FDS_ERROR("cmdEditName: Save Error!\n");
			FDS_ERROR(" path=[%s], name=[%s], result=%d, errno=%d\n", path.c_str(), name.c_str(), ret2, errno);
			DlgSelect dlg2;
			dlg2.setItemsOk();
			dlg2.setHeader("Save Error!");
			ret = dlg2.start();
			return;
		}

		// 名前が切り詰められたら報告
		infoViewRefresh();
		if (name != newname) {
			FDS_ERROR("cmdEditName: Disk Name Trancated!\n");
			FDS_ERROR(" path=[%s], before=[%s], after=[%s]\n", path.c_str(), newname.c_str(), name.c_str());
			DlgSelect dlg2;
			dlg2.setItemsOk();
			dlg2.setHeader("Disk Name Trancated!");
			dlg2.setCanEscape(true);
			dlg2.start();
		}

		// 新しいファイルリストを取得
		mFiles.getFiles(mCurDir.empty());
		mFiles.sortFiles();
	}
}

// -------------------------------------------------------------
// ドライブのプロテクト状態を変更する
// -------------------------------------------------------------
void
FDSSystem::cmdProtectDrive()
{
	// 排出先ドライブを選択
	int sel = 0;
	DlgSelect::ItemsVec items;
	for (int i=0; i<mFddEmu.Drives; i++) {
		items.push_back(getDriveName(i));
	}
	{
		DlgSelect dlg;
		dlg.setItemsVec(items);
		dlg.setHeader("[Protect Drive]");
		dlg.setCanEscape(true);
		sel = dlg.start(sel);
		if (sel < 0) {
			return;
		}
	}

	// ドライブのプロテクトスイッチを設定
	mFddEmu.protectDrive(sel);

	// FDDビューを更新
	fddViewRefresh();
}

// -------------------------------------------------------------
// ディスクのプロテクト状態を変更する
// -------------------------------------------------------------
void
FDSSystem::cmdEditProtect()
{
	// 選択位置をチェック
	int idx = filerViewGetIdx();
	if (idx >= (int)mFiles.size()) {
		return;
	}

	// FDXファイルのみ受け付ける
	if (!mFiles[idx].isFdxFile()) {
		return;
	}

	// 書き込み不可ならエラー
	if (mFiles[idx].isProtect()) {
		DlgSelect dlg;
		dlg.setItemsOk();
		dlg.setHeader("Write Protected!");
		dlg.setCanEscape(true);
		dlg.start();
		return;
	}

	// 選択したエントリで情報ビューを更新
	std::string path = mRootDir + mCurDir + mFiles[idx].filename();
	if (mFiles[idx].isDir()) {
		path += "/";
	}
	infoViewSetFile(path);
	infoViewRefresh();
	int ret = infoViewGetResult();
	if (ret == -1) {
		return;
	}
	if (ret < -1) {
		// 正常に情報取得できなければ中止
		DlgSelect dlg;
		dlg.setItemsOk();
		dlg.setHeader("FDX File Info Error");
		dlg.setCanEscape(true);
		dlg.start();
		return;
	}

	// プロテクトの状態をダイアログで処理
	FdxHeader& disk = infoViewGetFdxHeader();
	bool protect = (disk.mWriteProtect != 0);
	{
		int sel = (protect ? 0 : 1);
		DlgSelect::ItemsArray<3> items {{ "On", "Off", "[ Cancel ]", }};
		DlgSelect dlg;
		dlg.setItemsArray(items);
		dlg.setHeader("[Edit Protect]");
		dlg.setCanEscape(true);
		sel = dlg.start(sel);
		if ((sel == 2) || (sel == -1)) {
			// キャンセルなら中止
			return;
		}
		disk.mWriteProtect = (sel == 0);
	}

	// 情報を更新
	ret = infoViewSaveFile();
	if (ret < 0) {
		// 正常に更新できなければエラー表示
		FDS_ERROR("cmdEditProtect: Save Error!\n");
		FDS_ERROR(" path=[%s], ret=%d, errno=%d\n", path.c_str(), ret, errno);
		DlgSelect dlg;
		dlg.setItemsOk();
		dlg.setHeader("Save Error!");
		dlg.setCanEscape(true);
		ret = dlg.start();
	}

	infoViewRefresh();
}

// -------------------------------------------------------------
// ディレクトリまたはディスクを削除
// -------------------------------------------------------------
void
FDSSystem::cmdDelete()
{
	// 選択位置をチェック
	int idx = filerViewGetIdx();
	if (idx >= (int)mFiles.size()) {
		return;
	}

	// ディレクトリかFDXファイルのみ受け付ける
	if (!mFiles[idx].isFdxFile() && !mFiles[idx].isNormalDir()) {
		return;
	}

	// 選択したエントリで情報ビューを更新
	std::string path = mRootDir + mCurDir + mFiles[idx].filename();
	if (mFiles[idx].isDir()) {
		path += "/";
	}
	infoViewSetFile(path);
	infoViewRefresh();
	int ret = infoViewGetResult();
	if (ret == -1) {
		return;
	}
	if (ret < -1) {
		// 正常に情報取得できなければ中止
		DlgSelect dlg;
		dlg.setItemsOk();
		dlg.setHeader("FDX File Info Error");
		dlg.setCanEscape(true);
		dlg.start();
		return;
	}

	{
		// "Yes/No"を選択
		DlgSelect dlg;
		std::string title = "Delete [" + mFiles[idx].filename() + "]. OK?";
		dlg.setHeader(title);
		dlg.setItemsYesNo();
		dlg.setCanEscape(true);
		ret = dlg.start();

		// Yesでなかったら中止
		if (ret != 0) {
			return;
		}

		// 削除
		std::string src = mRootDir + mCurDir + mFiles[idx].filename();
		int ret2;
		if (mFiles[idx].isDir()) {
#if defined(FDS_WINDOWS)
			ret2 = _rmdir(src.c_str());
#else
			ret2 = rmdir(src.c_str());
#endif  // defined(FDS_WINDOWS)
		} else {
			ret2 = remove(src.c_str());
		}
		if (ret2 != 0) {
			FDS_ERROR("cmdDelete: Delete Failed!\n");
			FDS_ERROR(" src=[%s] result=%d, errno=%d\n", src.c_str(), ret2, errno);
			DlgSelect dlg2;
			dlg2.setItemsOk();
			dlg2.setHeader("Delete Failed!");
			dlg2.setCanEscape(true);
			dlg2.start();
			return;
		}

		// 新しいファイルリストを取得
		mFiles.getFiles(mCurDir.empty());
		mFiles.sortFiles();
		filerViewSetIdx(idx);
	}
}

// -------------------------------------------------------------
// シェルを起動
// -------------------------------------------------------------
void
FDSSystem::cmdShell()
{
#if !defined(FDS_WINDOWS)

	// 選択位置をチェック
	int idx = filerViewGetIdx();
	if (idx >= (int)mFiles.size()) {
		return;
	}

	std::string filename = mFiles[idx].filename();

	// FddEmuを終了
	mFddEmu.kill();

	// シェルを起動
	move(LINES-1, 0);
	refresh();
	def_prog_mode();
	curs_set(1);
	echo();
	cbreak();
	reset_shell_mode();
	printf("\n");

	// シェル起動
	char* cwd = getcwd(nullptr, 0);
	std::string path = mRootDir + mCurDir;
	chdir(path.c_str());
	char* env = getenv("SHELL");
	system(env);
	chdir(cwd);
	free(cwd);

	// FDSの画面を復帰
	def_shell_mode();
	cbreak();
	noecho();
	curs_set(0);
	reset_prog_mode();
	drawHeader();
	pathViewRedraw();
	infoViewRedraw();
	fddViewRedraw();
	helpViewRedraw();

	// FddEmuを再開
	mFddEmu.run();

	// 新しいファイルリストを取得
	mFiles.getFiles(mCurDir.empty());
	mFiles.sortFiles();
	filerViewSetIdx(idx);

#endif
}

// -------------------------------------------------------------
// ファイルの書き込み属性を変更する
// -------------------------------------------------------------
void
FDSSystem::cmdProtectDisk()
{
	// 選択位置をチェック
	int idx = filerViewGetIdx();
	if (idx >= (int)mFiles.size()) {
		return;
	}

	// FDXファイルのみ受け付ける
	if (!mFiles[idx].isFdxFile()) {
		return;
	}

	// 選択したエントリで情報ビューを更新
	std::string path = mRootDir + mCurDir + mFiles[idx].filename();
	if (mFiles[idx].isDir()) {
		path += "/";
	}
	infoViewSetFile(path);
	infoViewRefresh();
	int ret = infoViewGetResult();
	if (ret == -1) {
		return;
	}
	if (ret < -1) {
		// 正常に情報取得できなければ中止
		DlgSelect dlg;
		dlg.setItemsOk();
		dlg.setHeader("FDX File Info Error");
		dlg.setCanEscape(true);
		dlg.start();
		return;
	}

	// プロテクトの状態をダイアログで処理
	bool protect = mFiles[idx].isProtect();
	{
		int sel = (protect ? 0 : 1);
		DlgSelect::ItemsArray<3> items {{ "On", "Off", "[ Cancel ]", }};
		DlgSelect dlg;
		dlg.setItemsArray(items);
		dlg.setHeader("[Protect Disk]");
		dlg.setCanEscape(true);
		sel = dlg.start(sel);
		if ((sel == 2) || (sel == -1)) {
			// キャンセルなら中止
			return;
		}
#if defined(FDS_WINDOWS)
		int mode = ((sel == 0) ? _S_IREAD : _S_IREAD| _S_IWRITE);
		int ret2 = _chmod(path.c_str(), mode);
#else
		int mode = ((sel == 0) ? 0444 : 0666);
		if (mNoRoot) {
			mode &= 0644;
		}
		int ret2 = chmod(path.c_str(), mode);
#endif  // defined(FDS_WINDOWS)
		if (ret2 != 0) {
			FDS_ERROR("cmdProtectDisk: Chmod Failed!\n");
			FDS_ERROR(" src=[%s] result=%d, errno=%d\n", path.c_str(), ret2, errno);
			DlgSelect dlg2;
			dlg2.setItemsOk();
			dlg2.setHeader("Chmod Failed!");
			dlg2.setCanEscape(true);
			dlg2.start();
			return;
		}
	}

	// 新しいファイルリストを取得
	mFiles.getFiles(mCurDir.empty());
	mFiles.sortFiles();
}

// -------------------------------------------------------------
// ディスクをダンプする
// -------------------------------------------------------------
void
FDSSystem::cmdDumpDisk()
{
	// ドライブ選択肢準備
	int selDrive = 0;
	int driveNo = mConfig.driveNo();
	std::vector<int> vecDrive;
	DlgSelect::ItemsVec items0;
	for (int i=0; i<mConfig.numDrives(); i++) {
		for (int j=0; j<mConfig.cfgDrive(i).numDump(); j++) {
			const std::string& type = mConfig.cfgDrive(i).dump(j).type();
			for (int k=0; k<mConfig.cfgMachine().numDump(); k++) {
				const FDSMachineDump& d = mConfig.cfgMachine().dump(k);
				if (std::string::npos != type.find(d.type())) {
					if (i == driveNo) {
						selDrive = vecDrive.size();
					}
					vecDrive.push_back(i);
					items0.push_back(mConfig.cfgDrive(i).name());
					goto nextDrive;
				}
			}
		}
	  nextDrive:;
	}
	items0.push_back("[ Cancel ]");

	// ドライブ選択肢
	while (!0) {
	  selectDrive:;
		// ダイアログ表示
		DlgSelect dlg0;
		dlg0.setItemsVec(items0);
		dlg0.setHeader("[Select Dump FDD]");
		dlg0.setCanEscape(true);
		selDrive = dlg0.start(selDrive);

		// [Cancel]を選んだら終了
		if ((selDrive == -1) || (selDrive == (int)items0.size()-1)) {
			break;
		}

		// ドライブを選択
		mConfig.setDriveNo(vecDrive[selDrive]);

		// ダンプ形式選択肢準備
		DlgSelect::ItemsVec items;
		int n = mConfig.cfgMachine().numDump();
		for (int i=0; i<n; i++) {
			items.push_back(mConfig.cfgMachine().dump(i).name());
		}
		items.push_back("[ Cancel ]");

		// ダンプ形式選択肢
		int selDump = mConfig.cfgMachine().dumpNo();
		while (!0) {
		  selectDump:;
			// ダイアログ表示
			DlgSelect dlg;
			dlg.setItemsVec(items);
			dlg.setHeader("[Select Dump Format]");
			dlg.setCanEscape(true);
			selDump = dlg.start(selDump);

			// [Cancel]を選んだらドライブ選択に戻る
			if ((selDump == -1) || (selDump == (int)items.size()-1)) {
				goto selectDrive;
			}

			// ダンプ形式を選択
			mConfig.cfgMachineW().setDumpNo(selDump);
			const std::string& type = mConfig.cfgMachine().dump().type();
			int no = mConfig.cfgDrive().findDumpNoByType(type);
			if (no < 0) {
				FDS_ERROR("cmdDumpDisk: TYPE [%s] not found in [DRIVES] Config.\n", type.c_str());
				DlgSelect dlg3;
				dlg3.setItemsOk();
				dlg3.setHeader("TYPE [%s] not found in [DRIVES] Config.");
				dlg3.setCanEscape(true);
				dlg3.start();
				dlg3.end();
				refreshAllView();
				break;
			}
			mConfig.cfgDriveW().setDumpNoByType(type);

			// 選択した設定を読み込む
			std::string name = mConfig.cfgMachine().dump(selDump).name();
			std::string fddumpopt = mConfig.makeDumpOpt(selDump);
			std::string filename = mConfig.cfgMachine().dump(selDump).fileName();
			filename = FDSMachine::fileNameWithTime(filename);

			while (!0) {
				// 新しいファイル名を入力
				DlgInput dlg2;
				dlg2.setHeader("Dump ["+name+"] Disk:");
				dlg2.setText(filename);
				dlg2.setCanEscape(true);
				dlg2.setMaxLength(FDX_FILENAME_MAX);
				int ret = dlg2.start(COLS/2);

				// 入力が行われなかったらダンプ形式選択に戻る
				if (ret < 0) {
					goto selectDump;
				}

				// 入力が空だったら最初の名前でやり直し
				std::string newname = dlg2.getText();
				if (newname.empty()) {
					refreshAllView();
					continue;
				}
				// 入力結果の拡張子が".FDX"でなかったらやり直し
				std::wstring wnewname = WStrUtil::str2wstr(newname);
				size_t len = wnewname.length();
				if ((len < 5) || (WStrUtil::wstricmp(&wnewname[len-4], L".FDX"))) {
					FDS_ERROR("cmdDumpDisk: Use Extention \".FDX\"!\n");
					FDS_ERROR(" newname=[%s]\n", newname.c_str());
					DlgSelect dlg3;
					dlg3.setItemsOk();
					dlg3.setHeader("Use Extention \".FDX\"!");
					dlg3.setCanEscape(true);
					dlg3.start();
					dlg3.end();
					refreshAllView();
					filename = newname;
					continue;
				}

				// ファイル名に"/"が含まれていたらやり直し
				if (std::string::npos != newname.find('/')) {
					FDS_ERROR("cmdDumpDisk: Can't use '/'!\n");
					FDS_ERROR(" newname=[%s]\n", newname.c_str());
					DlgSelect dlg3;
					dlg3.setItemsOk();
					dlg3.setHeader("Can't use '/'!");
					dlg3.setCanEscape(true);
					dlg3.start();
					dlg3.end();
					refreshAllView();
					filename = newname;
					continue;
				}

				// すでにファイルがあったらやり直し
				std::string dst = mRootDir + mCurDir + newname;
				FILE* fin = fopen(dst.c_str(), "rb");
				if (fin) {
					fclose(fin);
					// 失敗
					FDS_ERROR("cmdDumpDisk: Disk Already Exist!\n");
					FDS_ERROR(" dst=[%s]\n", dst.c_str());
					DlgSelect dlg3;
					dlg3.setItemsOk();
					dlg3.setHeader("Disk Already Exist!");
					dlg3.setCanEscape(true);
					dlg3.start();
					dlg3.end();
					refreshAllView();
					filename = newname;
					continue;
				}

				// FddEmuを終了
				mFddEmu.kill();

				// ダンプビューを作成
				refreshAllView();
				dumpViewCreateWindow();
				dumpViewRedraw();
				wtimeout(mwDumpView, 0);
				nodelay(mwDumpView, true);

				// C/H/Sのセット
				int cylinders = mConfig.cfgMachine().dump().cylinders();
				if (cylinders == 0) {
					cylinders = mConfig.cfgDrive().dump().cylinders();
				}
				if (cylinders) {
					mFdDump.setCylinders(cylinders);
				}
				int heads = mConfig.cfgMachine().dump().heads();
				if (heads == 0) {
					heads = mConfig.cfgDrive().dump().heads();
				}
				if (heads) {
					mFdDump.setHeads(heads);
				}
				int steps = mConfig.cfgMachine().dump().steps();
				if (steps == 0) {
					steps = mConfig.cfgDrive().dump().steps();
				}
				if (steps) {
					mFdDump.setSteps(steps);
				}

				// ディスクをダンプ
				mFdDump.setCmd(mConfig.fdDumpCmd());
				mFdDump.setOption(fddumpopt);
				mFdDump.setFormatName(name);
				mFdDump.setDiskName(newname);
				mFdDump.setFileName(dst);
				mFdDump.setCallback(&cmdDumpDiskCallback_, this);

				while (!0) {
					int ret2 = mFdDump.run();
					if (ret2 < 0) {
						// キャンセル
						FDS_ERROR("cmdDumpDisk: Dump FDD Canceled!\n");
						DlgSelect dlg3;
						dlg3.setItemsYesNo();
						dlg3.setHeader("Dump FDD Canceled! Retry?");
						dlg3.setOffset(0, 8);
						int ret3 = dlg3.start();
						dlg3.end();
						if (ret3 == 0) {
							refreshAllView();
							continue; // retry
						}
						break;
					} else if (ret2 == 0) {
						// 成功
						FDS_ERROR("cmdDumpDisk: Dump FDD Finished!\n");
						DlgSelect dlg3;
						dlg3.setItemsOk();
						dlg3.setHeader("Dump FDD Finished!");
						dlg3.setOffset(0, 8);
						dlg3.start();
						dlg3.end();
						break;
					} else if (ret2 > 0) {
						// 失敗
						FDS_ERROR("cmdDumpDisk: Dump FDD Failed!\n");
						DlgSelect dlg3;
						dlg3.setItemsYesNo();
						dlg3.setHeader("Dump FDD Failed! Retry?");
						dlg3.setOffset(0, 8);
						int ret3 = dlg3.start();
						dlg3.end();
						if (ret3 == 0) {
							refreshAllView();
							continue; // retry
						}
						break;
					}
					break;
				}
#if !defined(FDS_WINDOWS)
				if (!mNoRoot) {
					chmod(dst.c_str(), 0666);
				}
#endif

				// ダンプビューを破棄
				dumpViewDestroyWindow();
				refreshAllView();

				// FddEmuを再開
				mFddEmu.run();

				// 新しいファイルリストを取得
				mFiles.getFiles(mCurDir.empty());
				mFiles.sortFiles();
				filerViewFindEntry(newname);

				// 作成したファイルの情報を表示
				infoViewSetFile(dst);
				infoViewRefresh();

				return;
			}
		}
	}

}

// -------------------------------------------------------------
// ダンプ表示コールバックエントリ
// -------------------------------------------------------------
int
FDSSystem::cmdDumpDiskCallback_(FdDump::Status& st, void* param)
{
	FDSSystem* me = (FDSSystem*)param;
	int ret = me->cmdDumpDiskCallback(st);
	return ret;
}

// -------------------------------------------------------------
// ダンプ表示コールバック本体
// -------------------------------------------------------------
int
FDSSystem::cmdDumpDiskCallback(FdDump::Status& st)
{
	FDS_LOG("callback: Track=%d\n", st.mNowTrack);
	dumpViewUpdate(st);
	dumpViewRefresh();

	// ESCキー判定
	bool escape = false;
	int key = wgetch(mwDumpView);
	if (key == 0x20) {
		escape = true;
	}
	if (key == 0x1b) {
		if (fds::doEscKey(mwDumpView)) {
			escape = true;
		}
	}

	return ((escape == true) ? -1 : 0);
}

// =====================================================================

// -------------------------------------------------------------
// ディスクをリストアする
// -------------------------------------------------------------
void
FDSSystem::cmdRestoreDisk()
{
	// 選択位置をチェック
	int idx = filerViewGetIdx();
	if (idx >= (int)mFiles.size()) {
		return;
	}

	// FDXファイルのみ受け付ける
	if (!mFiles[idx].isFdxFile()) {
		return;
	}

	// 選択したFDXファイルで情報ビューを更新
	std::string src = mFiles[idx].filename();
	std::string path = mRootDir + mCurDir + src;
	if (mFiles[idx].isDir()) {
		path += "/";
	}
	infoViewSetFile(path);
	infoViewRefresh();
	int ret = infoViewGetResult();
	if (ret == -1) {
		return;
	}
	if (ret < -1) {
		// 正常に情報取得できなければ中止
		DlgSelect dlg;
		dlg.setItemsOk();
		dlg.setHeader("FDX File Info Error!");
		dlg.setCanEscape(true);
		dlg.start();
		return;
	}

	// ディスク名を設定
	FdxHeader& disk = infoViewGetFdxHeader();
	{
		std::string name = std::string((const char*)&(disk.mName[0]));
		mFdRestore.setDiskName(name);
		mFdRestore.setCylinders(disk.mCylinders);
	}

	// ドライブ選択肢準備
	int selDrive = 0;
	int driveNo = mConfig.driveNo();
	std::vector<int> vecDrive;
	DlgSelect::ItemsVec items;
	for (int i=0; i<mConfig.numDrives(); i++) {
		for (int j=0; j<mConfig.cfgDrive(i).numRestore(); j++) {
			const std::string& type = mConfig.cfgDrive(i).restore(j).type();
			for (int k=0; k<mConfig.cfgMachine().numRestore(); k++) {
				const FDSMachineRestore& d = mConfig.cfgMachine().restore(k);
				if (std::string::npos != type.find(d.type())) {
					if (i == driveNo) {
						selDrive = vecDrive.size();
					}
					vecDrive.push_back(i);
					items.push_back(mConfig.cfgDrive(i).name());
					goto nextDrive;
				}
			}
		}
	  nextDrive:;
	}

	items.push_back("[ Cancel ]");

	// ドライブ選択肢
	while (!0) {
	  selectDrive:;
		// ダイアログ表示
		DlgSelect dlg0;
		dlg0.setItemsVec(items);
		dlg0.setHeader("[Select Restore FDD]");
		dlg0.setCanEscape(true);
		selDrive = dlg0.start(selDrive);

		// [Cancel]を選んだら終了
		if ((selDrive == -1) || (selDrive == (int)items.size()-1)) {
			break;
		}

		// ドライブを選択
		mConfig.setDriveNo(vecDrive[selDrive]);
		std::string& driveName = items[selDrive];

		// TYPEを設定
		std::string type;
		switch (mFdxHeader.mType) {
		  case 0:
			type = "2D-250KBPS-300RPM";
			break;
		  case 1:
			type = ((mFdxHeader.mRpm == 300) ? "2DD-250KBPS-300RPM" : "2DD-250KBPS-360RPM");
			break;
		  case 2:
			type = ((mFdxHeader.mRpm == 300) ? "2HD-500KBPS-300RPM" : "2HD-500KBPS-360RPM");
			break;
		  case 9:
			if (mFdxHeader.mCylinders < 60) {
				type = "2D-250KBPS-300RPM";
			} else {
				if (mFdxHeader.mRate < 5000) {
					type = ((mFdxHeader.mRpm == 300) ? "2DD-250KBPS-300RPM" : "2DD-250KBPS-360RPM");
				} else {
					type = ((mFdxHeader.mRpm == 300) ? "2HD-500KBPS-300RPM" : "2HD-500KBPS-360RPM");
				}
			}
			break;
		  default:
			break;
		}
		if (type.empty()) {
			FDS_ERROR("cmdRestoreDisk: Restore TYPE Unknown!\n");
			DlgSelect dlg3;
			dlg3.setItemsOk();
			dlg3.setHeader("Restore TYPE Unknown!");
			dlg3.start();
			dlg3.end();
			goto selectDrive;
		}
		ret = mConfig.cfgDriveW().setDumpNoByType(type);
		if (ret < 0) {
			char buf[FDX_FILENAME_MAX];
			sprintf(buf, "Restore TYPE [%s] Unknown!\n", type.c_str());
			FDS_ERROR("cmdRestoreDisk: %s", buf);
			DlgSelect dlg3;
			dlg3.setItemsOk();
			dlg3.setHeader(buf);
			dlg3.start();
			dlg3.end();
			goto selectDrive;
		}

		// 選択した設定を読み込む
		std::string name = mConfig.cfgDrive().name();
		std::string fdrestoreopt = mConfig.makeRestoreOpt();

		{
			// "Yes/No"を選択
			DlgSelect dlg;
			std::string title = "Restore to ["+driveName+"]. OK?";
			dlg.setHeader(title);
			dlg.setItemsYesNo();
			dlg.setCanEscape(true);
			ret = dlg.start();

			// Yesでなかったら中止
			if (ret != 0) {
				goto selectDrive;
			}
		}

		// FddEmuを終了
		mFddEmu.kill();

		while (!0) {
			// Fdxファイルのアナライズ
			{
				std::string cmd = mConfig.fdxToolCmd();
				std::string option = "-a \""+path+"\"";
				mFdRestore.setAnalyzeCmd(cmd);
				mFdRestore.setAnalyzeOption(option);
				mFdRestore.analyze();
			}

			// リストアビューを作成
			refreshAllView();
			restoreViewCreateWindow();
			restoreViewRedraw();
			wtimeout(mwRestoreView, 0);
			nodelay(mwRestoreView, true);

			// C/H/Sのセット
			int cylinders = mConfig.cfgMachine().restore().cylinders();
			if (cylinders == 0) {
				cylinders = mConfig.cfgDrive().restore().cylinders();
			}
			if (cylinders) {
				mFdRestore.setCylinders(cylinders);
			}
			int heads = mConfig.cfgMachine().restore().heads();
			if (heads == 0) {
				heads = mConfig.cfgDrive().restore().heads();
			}
			if (heads) {
				mFdRestore.setHeads(heads);
			}
			int steps = mConfig.cfgMachine().restore().steps();
			if (steps == 0) {
				steps = mConfig.cfgDrive().restore().steps();
			}
			if (steps) {
				mFdRestore.setSteps(steps);
			}

			// ディスクをリストア
			mFdRestore.setCmd(mConfig.fdRestoreCmd());
			mFdRestore.setOption(fdrestoreopt);
			mFdRestore.setFormatName(name);
			mFdRestore.setFileName(path);
			mFdRestore.setCallback(&cmdRestoreDiskCallback_, this);

			while (!0) {
				int ret2 = mFdRestore.run();
				if (ret2 < 0) {
					// キャンセル
					FDS_ERROR("cmdRestoreDisk: Restore FDD Canceled!\n");
					DlgSelect dlg3;
					dlg3.setItemsYesNo();
					dlg3.setHeader("Restore FDD Canceled! Retry?");
					dlg3.setOffset(0, 8);
					int ret3 = dlg3.start();
					dlg3.end();
					if (ret3 == 0) {
						mFdRestore.revertAnalyzeStatus();
						refreshAllView();
						continue; // retry
					}
					break;
				} else if (ret2 == 0) {
					// 成功
					FDS_ERROR("cmdRestoreDisk: Restore FDD Finished!\n");
					DlgSelect dlg3;
					dlg3.setItemsOk();
					dlg3.setHeader("Restore FDD Finished!");
					dlg3.setOffset(0, 8);
					dlg3.start();
					dlg3.end();
					break;
				} else if (ret2 > 0) {
					// 失敗
					FDS_ERROR("cmdRestoreDisk: Restore FDD Failed!\n");
					DlgSelect dlg3;
					dlg3.setItemsYesNo();
					dlg3.setHeader("Restore FDD Failed! Retry?");
					dlg3.setOffset(0, 8);
					int ret3 = dlg3.start();
					dlg3.end();
					if (ret3 == 0) {
						mFdRestore.revertAnalyzeStatus();
						refreshAllView();
						continue; // retry
					}
					break;
				}
				break;
			}

			// リストアビューを破棄
			restoreViewDestroyWindow();
			refreshAllView();

			// FddEmuを再開
			mFddEmu.run();

			return;
		}
	}

}

// -------------------------------------------------------------
// リストア表示コールバックエントリ
// -------------------------------------------------------------
int
FDSSystem::cmdRestoreDiskCallback_(FdRestore::Status& st, void* param)
{
	FDSSystem* me = (FDSSystem*)param;
	int ret = me->cmdRestoreDiskCallback(st);
	return ret;
}

// -------------------------------------------------------------
// リストア表示コールバック本体
// -------------------------------------------------------------
int
FDSSystem::cmdRestoreDiskCallback(FdRestore::Status& st)
{
	FDS_LOG("callback: Track=%d\n", st.mNowTrack);
	restoreViewUpdate(st);
	restoreViewRefresh();

	// ESCキー判定
	bool escape = false;
	int key = wgetch(mwRestoreView);
	if (key == 0x20) {
		escape = true;
	}
	if (key == 0x1b) {
		if (fds::doEscKey(mwRestoreView)) {
			escape = true;
		}
	}

	return ((escape == true) ? -1 : 0);
}

// -------------------------------------------------------------
// アナライザを起動
// -------------------------------------------------------------
void
FDSSystem::cmdAnalyzeDisk()
{
	// 選択位置をチェック
	int idx = filerViewGetIdx();
	if (idx >= (int)mFiles.size()) {
		return;
	}

	std::string src = mFiles[idx].filename();
	std::string path = mRootDir + mCurDir + src;

#if 0
	// FddEmuを終了
	mFddEmu.kill();
#endif

	// アナライザを起動
	FDSAnalyzer analyzer;
	analyzer.start(path);

#if 0
	// FddEmuを再開
	mFddEmu.run();
#endif

	// 新しいファイルリストを取得
	mFiles.getFiles(mCurDir.empty());
	mFiles.sortFiles();
	filerViewSetIdx(idx);
}



// =====================================================================
// [EOF]
