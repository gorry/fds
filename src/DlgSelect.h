﻿// ◇
// fds: fdx68 selector
// DlgSelect: 選択肢ダイアログ
// Copyright: (C)2020 Hiroaki GOTO as GORRY.
// License: see readme.txt
// =====================================================================

#if !defined(__DLGSELECT_H__)
#define __DLGSELECT_H__

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <locale.h>
#include <wchar.h>
#include <string.h>

#include <curses.h>

#include <vector>
#include <string>
#include <array>

#include "FDSCommon.h"
#include "FDSMacro.h"
#include "FDSSystem.h"

// =====================================================================
// 選択肢ダイアログ
// =====================================================================

class DlgSelect
{
public:		// struct, enum
	using ItemsVec = std::vector<std::string>;

	template <size_t N>
	using ItemsArray = std::array<std::string, N>;

public:		// function
	DlgSelect();
	virtual ~DlgSelect() { end(); }

	template <size_t N>
	void setItemsArray(const ItemsArray<N>& items)
	{
		mSelectTxt.clear();
		for (int i=0; i<(int)items.size(); i++) {
			mSelectTxt.push_back(items[i]);
		}
	}

	void setItemsVec(const ItemsVec& items);
	void setItemsOk();
	void setItemsOkCancel();
	void setItemsYesNo();
	void setItemsYesNoCancel();
	void setCanEscape(bool f);
	void setHeader(const std::string& text);
	void setOffset(int ofsx, int ofsy);
	int start(int sel=0);
	int start(int x, int y, int sel=0);
	void end();

private:	// function
	void show();
	void measureSize(int &retWidth, int &retHeight);

public:		// var

private:	// var
	std::vector<std::string> mSelectTxt;
	WINDOW *mwFrame = nullptr;
	std::string mHeader;
	std::vector<std::string> mHeaders;
	int mInnerOfsX = 0;
	int mInnerOfsY = 0;
	int mInnerWidth = 0;
	int mInnerHeight = 0;
	int mSelect = 0;
	bool mCanEscape = false;
	int mOfsX = 0;
	int mOfsY = 0;
	int mViewOfsY = 0;
	bool mYesNo = false;
	bool mDisableEnter;

};

#endif  // __DLGSELECT_H__
// =====================================================================
// [EOF]
