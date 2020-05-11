// ◇
// fds: fdx68 selector
// DirStack: ディレクトリスタック
// Copyright: (C)2020 Hiroaki GOTO as GORRY.
// License: see readme.txt
// =====================================================================

#if !defined(__DIRSTACK_H__)
#define __DIRSTACK_H__

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <vector>
#include <string>

// =====================================================================
// ディレクトリスタック
// =====================================================================

class DirStack
{
public:		// struct, enum
	struct Stack {
		int mFilerViewOfsY = 0;
		int mFilerViewCsrY = 0;
		std::string mRootDir;
		std::string mCurDir;
	};

public:		// function
	DirStack() {}
	virtual ~DirStack() {}

	void clear() { mDirStack.clear(); }
	int push(const Stack& stack);
	Stack pop();
	const Stack& top() const { return mDirStack.back(); }
	int level() const { return mDirStack.size(); }

private:	// function

public:		// var

private:	// var
	std::vector<Stack> mDirStack;

};

#endif  // __DIRSTACK_H__
// =====================================================================
// [EOF]
