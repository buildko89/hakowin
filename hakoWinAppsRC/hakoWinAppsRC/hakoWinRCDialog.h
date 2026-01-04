#pragma once
#include "afxdialogex.h"


// hakoWinRCDialog ダイアログ

class hakoWinRCDialog : public CDialogEx
{
	DECLARE_DYNAMIC(hakoWinRCDialog)

public:
	hakoWinRCDialog(const CString CurrentPath, const CString SearchPath, CWnd* pParent = nullptr);   // 標準コンストラクター
	virtual ~hakoWinRCDialog();

  CString m_HakoRCConf;
  CString m_SearchPath;
  CString m_CurrentPath;

// ダイアログ データ
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_RC_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
  DECLARE_MESSAGE_MAP()

  CComboBox m_confFile;

  virtual BOOL OnInitDialog();
  virtual void OnBnClickedOk();

  void InitConfList(void);

};
