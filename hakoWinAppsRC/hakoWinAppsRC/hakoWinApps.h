
// hakoWinApps.h : PROJECT_NAME アプリケーションのメイン ヘッダー ファイルです
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH に対してこのファイルをインクルードする前に 'pch.h' をインクルードしてください"
#endif

#include "resource.h"		// メイン シンボル


// ChakoWinAppsApp:
// このクラスの実装については、hakoWinApps.cpp を参照してください
//

class ChakoWinAppsApp : public CWinApp
{
public:
	ChakoWinAppsApp();

// オーバーライド
public:
	virtual BOOL InitInstance();

// 実装

	DECLARE_MESSAGE_MAP()
};

extern ChakoWinAppsApp theApp;
