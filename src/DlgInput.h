// ◇
// fds: fdx68 selector
// DlgInput: １行入力ダイアログ
// Copyright: (C)2020 Hiroaki GOTO as GORRY.
// License: see readme.txt
// =====================================================================

#if !defined(__DLGINPUT_H__)
#define __DLGINPUT_H__

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <locale.h>
#include <wchar.h>
#include <string.h>

#include <curses.h>

#include <vector>
#include <string>

#include "FDSSystem.h"
#include "FDSMacro.h"

// =====================================================================
// １行入力ダイアログ
// =====================================================================

class DlgInput
{
public:		// struct, enum

public:		// function
	DlgInput() {}
	virtual ~DlgInput() { end(); }

	void setText(const std::string& text);
	const std::string getText();
	void setHeader(const std::string& text);
	void setMaxLength(int maxlen);
	void setCanEscape(bool f);
	int start(int w=0);
	int start(int x, int y, int w=0);
	void end();

private:	// function
	void show();
	void measureSize(int &retWidth, int &retHeight);

	void keyLeft();
	void keyRight();
	void keyHome();
	void keyEnd();
	void keyEnter();
	void keyInsert();
	void keyDelete();
	void keyBackspace();
	void keyChar(wint_t wch);

public:		// var

private:	// var
	std::string mInputText;
	std::wstring mWInputText;
	WINDOW *mwFrame = nullptr;
	int mMaxTextLength = 0;
	int mTextLength = 0;
	int mTextBoxW = 0;
	int mTextBoxOfsX = 0;
	int mTextBoxCsrX = 0;
	int mTextPosX = 0;
	bool mInsertMode = false;
	bool mFinish = false;
	bool mCanEscape = false;
	std::string mHeader = "Input";

};

#endif  // __DLGINPUT_H__
// =====================================================================
// [EOF]
