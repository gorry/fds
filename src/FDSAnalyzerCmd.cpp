// ◇
// fds: fdx68 selector
// FDSAnalyzer: FDSアナライザ:コマンド
// Copyright: (C)2020 Hiroaki GOTO as GORRY.
// License: see readme.txt
// =====================================================================

#include "FDSAnalyzer.h"

// =====================================================================
// FDSAnalyzer: FDSアナライザ:コマンド
// =====================================================================

// -------------------------------------------------------------
// トラックをダンプする
// -------------------------------------------------------------
void
FDSAnalyzer::cmdDumpTrack()
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
		dlg0.setHeader("[Select Dump Track FDD]");
		dlg0.setCanEscape(true);
		selDrive = dlg0.start(selDrive);

		// [Cancel]を選んだら終了
		if ((selDrive == -1) || (selDrive == (int)items0.size()-1)) {
			break;
		}

		// ドライブを選択
		mConfig.setDriveNo(vecDrive[selDrive]);

		// TYPEを設定
		std::string type;
		std::string format = "encode";
		const FdxHeader& fdxheader = mFdxView.diskInfo().mFdxInfo;
		switch (fdxheader.mType) {
		  case 0:
			type = "2D-250KBPS-300RPM";
			break;
		  case 1:
			type = ((fdxheader.mRpm == 300) ? "2DD-250KBPS-300RPM" : "2DD-250KBPS-360RPM");
			break;
		  case 2:
			type = ((fdxheader.mRpm == 300) ? "2HD-500KBPS-300RPM" : "2HD-500KBPS-360RPM");
			break;
		  case 9:
			format = "raw";
			if (fdxheader.mCylinders < 60) {
				type = "2D-250KBPS-300RPM";
			} else {
				if (fdxheader.mRate < 5000) {
					type = ((fdxheader.mRpm == 300) ? "2DD-250KBPS-300RPM" : "2DD-250KBPS-360RPM");
				} else {
					type = ((fdxheader.mRpm == 300) ? "2HD-500KBPS-300RPM" : "2HD-500KBPS-360RPM");
				}
			}
			break;
		  default:
			break;
		}
		if (type.empty()) {
			FDS_ERROR("cmdDumpTrack: Dump TYPE Unknown!\n");
			DlgSelect dlg3;
			dlg3.setItemsOk();
			dlg3.setHeader("Dump TYPE Unknown!");
			dlg3.start();
			dlg3.end();
			goto selectDrive;
		}

		// ダンプ形式選択肢準備
		std::vector<int> itemsVec;
		DlgSelect::ItemsVec items;
		int n = mConfig.cfgMachine().numDump();
		for (int i=0; i<n; i++) {
			const FDSMachineDump& dump = mConfig.cfgMachine().dump(i);
			if (std::string::npos != dump.type().find(type)) {
				if (dump.format() == format) {
					items.push_back(mConfig.cfgMachine().dump(i).name());
					itemsVec.push_back(i);
				}
			}
		}
		if (items.size() < 1) {
			char str[256];
			sprintf(str, "Can't Dump [TYPE=%s] [FORMAT=%s]", type.c_str(), format.c_str());
			FDS_ERROR("cmdDumpTrack: %s\n", str);
			DlgSelect dlg3;
			dlg3.setItemsOk();
			dlg3.setHeader(str);
			dlg3.start();
			dlg3.end();
			goto selectDrive;
		}
		items.push_back("[ Cancel ]");

		// ダンプ形式選択肢
		int selDump = mConfig.cfgMachine().dumpNo();
		while (!0) {
		  selectDump:;
			// selDumpをitemVec内番号に変換
			int i;
			for (i=0; i<(int)itemsVec.size(); i++) {
				if (itemsVec[i] == selDump) {
					selDump = i;
					break;
				}
			}
			if (i >= (int)itemsVec.size()) {
				selDump = 0;
			}

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

			// selDumpを元のDump形式番号に戻す
			selDump = itemsVec[selDump];

			// ダンプ形式を選択
			int no = mConfig.cfgDrive().findDumpNoByType(type);
			if (no < 0) {
				FDS_ERROR("cmdDumpTrack: TYPE [%s] not found in [DRIVES] Config.\n", type.c_str());
				DlgSelect dlg3;
				dlg3.setItemsOk();
				dlg3.setHeader("TYPE [%s] not found in [DRIVES] Config.");
				dlg3.setCanEscape(true);
				dlg3.start();
				dlg3.end();
				refreshAllView();
				break;
			}
			mConfig.cfgMachineW().setDumpNo(selDump);
			mConfig.cfgDriveW().setDumpNoByType(type);

			// 選択した設定を読み込む
			std::string name = mConfig.cfgMachine().dump(selDump).name();
			std::string filename = mFilename;

			while (!0) {
				if (mRetryCount == 0) {
					mRetryCount = mConfig.cfgDrive().retry();
				}

				// リトライ回数を入力
				DlgInput dlg2;
				char numtxt[16];
				sprintf(numtxt, "%d", mRetryCount);
				dlg2.setHeader("Retry Count:");
				dlg2.setText(numtxt);
				dlg2.setCanEscape(true);
				dlg2.setMaxLength(2);
				int ret = dlg2.start(COLS/2);

				// 入力が行われなかったらダンプ形式選択に戻る
				if (ret < 0) {
					goto selectDump;
				}

				// 入力が空だったらデフォルト値でやり直し
				int num = atoi(dlg2.getText().c_str());
				if (num <= 0) {
					refreshAllView();
					continue;
				}
				mRetryCount = num;
				std::string fddumpopt = mConfig.makeDumpTrackOpt(selDump, mTrackViewTrackNo, mRetryCount);

				// FddEmuを終了
				mpFddEmu->kill();

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
				mFdDump.setDiskName("");
				mFdDump.setFileName(mFilename);
				mFdDump.setCallback(&cmdDumpTrackCallback_, this);

				while (!0) {
					int ret2 = mFdDump.run();
					if (ret2 < 0) {
						// キャンセル
						FDS_ERROR("cmdDumpTrack: Dump Track Canceled!\n");
						DlgSelect dlg3;
						dlg3.setItemsYesNo();
						dlg3.setHeader("Dump Track Canceled! Retry?");
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
						FDS_ERROR("cmdDumpTrack: Dump Track Finished!\n");
						DlgSelect dlg3;
						dlg3.setItemsOk();
						dlg3.setHeader("Dump Track Finished!");
						dlg3.setOffset(0, 8);
						dlg3.start();
						dlg3.end();
						break;
					} else if (ret2 > 0) {
						// 失敗
						DlgSelect dlg3;
						std::string errmsg = "Dump Track Failed!";
						switch (ret2) {
						  default: break;
						  case 1: errmsg = "Not Fork!"; break;
						  case 2: errmsg = "Not Connect!"; break;
						  case 3: errmsg = "Drive Not Ready!"; break;
						  case 4: errmsg = "Write Protected!"; break;
						}
						FDS_ERROR("cmdDumpDisk: %s\n", errmsg.c_str());
						char msgbuf[256];
						sprintf(msgbuf, "%s Retry?", errmsg.c_str());
						dlg3.setItemsYesNo();
						dlg3.setHeader(msgbuf);
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
#if 0
#if !defined(FDS_WINDOWS)
				if (!mNoRoot) {
					chmod(dst.c_str(), 0666);
				}
#endif
#endif

				// FddEmuを再開
				mpFddEmu->run();

				goto quit;
			}
		}
	}

quit:;
	// ディスク状態の更新要求
	trackViewReqReload();
	sectorViewReqReload();

	// ダンプビューを破棄
	dumpViewDestroyWindow();
	refreshAllView();

}

// -------------------------------------------------------------
// ダンプ表示コールバックエントリ
// -------------------------------------------------------------
int
FDSAnalyzer::cmdDumpTrackCallback_(FdDump::Status& st, void* param)
{
	FDSAnalyzer* me = (FDSAnalyzer*)param;
	int ret = me->cmdDumpTrackCallback(st);
	return ret;
}

// -------------------------------------------------------------
// ダンプ表示コールバック本体
// -------------------------------------------------------------
int
FDSAnalyzer::cmdDumpTrackCallback(FdDump::Status& st)
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
FDSAnalyzer::cmdRestoreTrack()
{

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
		dlg0.setHeader("[Select Restore Track FDD]");
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
		std::string format = "encode";
		const FdxHeader& fdxheader = mFdxView.diskInfo().mFdxInfo;
		switch (fdxheader.mType) {
		  case 0:
			type = "2D-250KBPS-300RPM";
			break;
		  case 1:
			type = ((fdxheader.mRpm == 300) ? "2DD-250KBPS-300RPM" : "2DD-250KBPS-360RPM");
			break;
		  case 2:
			type = ((fdxheader.mRpm == 300) ? "2HD-500KBPS-300RPM" : "2HD-500KBPS-360RPM");
			break;
		  case 9:
			if (fdxheader.mCylinders < 60) {
				type = "2D-250KBPS-300RPM";
			} else {
				if (fdxheader.mRate < 5000) {
					type = ((fdxheader.mRpm == 300) ? "2DD-250KBPS-300RPM" : "2DD-250KBPS-360RPM");
				} else {
					type = ((fdxheader.mRpm == 300) ? "2HD-500KBPS-300RPM" : "2HD-500KBPS-360RPM");
				}
			}
			break;
		  default:
			break;
		}
		if (type.empty()) {
			FDS_ERROR("cmdRestoreTrack: Restore TYPE Unknown!\n");
			DlgSelect dlg3;
			dlg3.setItemsOk();
			dlg3.setHeader("Restore TYPE Unknown!");
			dlg3.start();
			dlg3.end();
			goto selectDrive;
		}

		// ダンプ形式を選択
		int ret = mConfig.cfgMachineW().setRestoreNoByType(type);
		if (ret < 0) {
			char buf[FDX_FILENAME_MAX];
			sprintf(buf, "Restore TYPE [%s] not found in [MACHINES] Config.\n", type.c_str());
			FDS_ERROR("cmdRestoreTrack: %s", buf);
			DlgSelect dlg3;
			dlg3.setItemsOk();
			dlg3.setHeader(buf);
			dlg3.start();
			dlg3.end();
			goto selectDrive;
		}
		ret = mConfig.cfgDriveW().setRestoreNoByType(type);
		if (ret < 0) {
			char buf[FDX_FILENAME_MAX];
			sprintf(buf, "Restore TYPE [%s] not found in [DRIVES] Config.\n", type.c_str());
			FDS_ERROR("cmdRestoreTrack: %s", buf);
			DlgSelect dlg3;
			dlg3.setItemsOk();
			dlg3.setHeader(buf);
			dlg3.start();
			dlg3.end();
			goto selectDrive;
		}

		// 選択した設定を読み込む
		std::string name = mConfig.cfgMachine().restore().name();
		std::string fdrestoreopt = mConfig.makeRestoreTrackOpt(mTrackViewTrackNo);

		{
			// "Yes/No"を選択
			DlgSelect dlg;
			std::string title = "Restore Track to ["+driveName+"]. OK?";
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
		mpFddEmu->kill();

		while (!0) {
			// Fdxファイルのアナライズ
			{
				std::string cmd = mConfig.fdxToolCmd();
				std::string option = "-a \""+mFilename+"\"";
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
			mFdRestore.setDiskName("");
			mFdRestore.setFileName(mFilename);
			mFdRestore.setCallback(&cmdRestoreTrackCallback_, this);

			while (!0) {
				int ret2 = mFdRestore.run();
				if (ret2 < 0) {
					// キャンセル
					FDS_ERROR("cmdRestoreTrack: Restore Track Canceled!\n");
					DlgSelect dlg3;
					dlg3.setItemsYesNo();
					dlg3.setHeader("Restore Track Canceled! Retry?");
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
					FDS_ERROR("cmdRestoreTrack: Restore Track Finished!\n");
					DlgSelect dlg3;
					dlg3.setItemsOk();
					dlg3.setHeader("Restore Track Finished!");
					dlg3.setOffset(0, 8);
					dlg3.start();
					dlg3.end();
					break;
				} else if (ret2 > 0) {
					// 失敗
					DlgSelect dlg3;
					std::string errmsg = "Restore Track Failed!";
					switch (ret2) {
					  default: break;
					  case 1: errmsg = "Not Fork!"; break;
					  case 2: errmsg = "Not Connect!"; break;
					  case 3: errmsg = "Drive Not Ready!"; break;
					  case 4: errmsg = "Write Protected!"; break;
					}
					FDS_ERROR("cmdRestoreTrack: %s\n", errmsg.c_str());
					char msgbuf[256];
					sprintf(msgbuf, "%s Retry?", errmsg.c_str());
					dlg3.setItemsYesNo();
					dlg3.setHeader(msgbuf);
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

			// FddEmuを再開
			mpFddEmu->run();

			goto quit;
		}
	}

quit:;
	// リストアビューを破棄
	restoreViewDestroyWindow();
	refreshAllView();

}

// -------------------------------------------------------------
// リストア表示コールバックエントリ
// -------------------------------------------------------------
int
FDSAnalyzer::cmdRestoreTrackCallback_(FdRestore::Status& st, void* param)
{
	FDSAnalyzer* me = (FDSAnalyzer*)param;
	int ret = me->cmdRestoreTrackCallback(st);
	return ret;
}

// -------------------------------------------------------------
// リストア表示コールバック本体
// -------------------------------------------------------------
int
FDSAnalyzer::cmdRestoreTrackCallback(FdRestore::Status& st)
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


// =====================================================================
// [EOF]
