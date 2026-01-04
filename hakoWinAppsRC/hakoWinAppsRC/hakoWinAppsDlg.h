
#pragma once
#include <iostream>
#include <string>
#include <windows.h> // 追加

#include <cstdlib>

#include <atlbase.h>
#include <atlconv.h>
#include <atlstr.h>
#include <afxdlgs.h> // CFileDialog
#include <afxext.h>  // CEditBrowseCtrl
#include <thread>    // 追加
#include <xlocbuf>
#include <locale>
#include <codecvt>

#include <shellapi.h> // ファイル先頭に追加


// ChakoWinAppsDlg ダイアログ
class ChakoWinAppsDlg : public CDialogEx
{
  // コンストラクション
public:
  ChakoWinAppsDlg(CWnd* pParent = nullptr); // 標準コンストラクター

  // ダイアログ データ
#ifdef AFX_DESIGN_TIME
  enum { IDD = IDD_HAKOWINAPPS_DIALOG };
#endif

protected:
  virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV サポート

  // 実装
protected:
  HICON m_hIcon;

  CString m_HakoWinPath;
  CString m_HakoConfPath;
  CString m_HakoAvatarApps;
  CString m_HakoAvatarConf;
  CString m_HakoRCPath;
  CString m_HakoRCConf;
  CString m_HakoRCAvatar;
  CString m_HakoScriptPath;

  // 追加: CMFCEditBrowseCtrl メンバ変数

  CMFCEditBrowseCtrl m_EditBrowseCtrl1;
  CMFCEditBrowseCtrl m_EditBrowseCtrl2;

  // 生成された、メッセージ割り当て関数
  virtual BOOL OnInitDialog();
  afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
  afx_msg void OnPaint();
  afx_msg HCURSOR OnQueryDragIcon();
  afx_msg void OnEnChangeMfceditbrowse1();
  afx_msg void OnEnChangeMfceditbrowse2();
  afx_msg void OnBnClickedButton1();
  afx_msg void OnBnClickedButton2();
  afx_msg void OnBnClickedButton3();
  afx_msg void OnBnClickedButton4();
  afx_msg void OnBnClickedButton5();
  DECLARE_MESSAGE_MAP()

public:
  CString GetModuleDirectory()
  {
    wchar_t path[_MAX_PATH], drive[_MAX_PATH], dir[_MAX_PATH], fname[_MAX_PATH], ext[_MAX_PATH];

    if (GetModuleFileName(NULL, path, _MAX_PATH) != 0)
    {
      _wsplitpath_s(path, drive, dir, fname, ext);
      // ドライブとディレクトリ名を結合して実行ファイルパスとする
      return CString(drive) + CString(dir);
    }
    else
    {
      // エラーハンドリング
      return _T("");
    }
  }

  void runPowerShellCommand(const std::string& command, const std::string& directory) {
    std::string fullCommand = "cd /d " + directory + " && start powershell -NoExit -Command \"" + command + "\"";

    int result = system(fullCommand.c_str());
    if (result != 0) {
      std::cerr << "Failed to execute PowerShell command: " << command << std::endl;
    }
  }

  int runPowerShellCommand2(const std::string& command, const std::string& directory) {
    std::string fullCommand =
      "cd /d " + directory +
      " && powershell -Command \"" + command + "\"";

    int result = system(fullCommand.c_str());
    if (result != 0) {
      std::cerr << "Python 実行失敗 (exit code " << result << ")" << std::endl;
    }
  	return result;
  }

  void OpenInExplorer(const std::wstring& path) {
    ShellExecute(NULL, L"open", path.c_str(), NULL, NULL, SW_SHOWNORMAL);
  }

public:
  bool BackupAndRenameDroneLog(const CString& srcName, const CString& dstName)
  {
    if (m_HakoWinPath.IsEmpty()) return false;

    CString base = m_HakoWinPath;
    if (base.GetLength() > 0 && base[base.GetLength() - 1] != L'\\' && base[base.GetLength() - 1] != L'/') {
      base += L'\\';
    }

    CString srcPath = base + srcName;
    CString dstPath = base + dstName;

    if (GetFileAttributes(dstPath) != INVALID_FILE_ATTRIBUTES) {
      int bakIdx = 1;
      CString bakPath;
      do {
        bakPath.Format(_T("%s.bak%d"), dstPath.GetString(), bakIdx);
        bakIdx++;
      } while (GetFileAttributes(bakPath) != INVALID_FILE_ATTRIBUTES);

      if (!::MoveFileW(dstPath, bakPath)) {
        DWORD err = ::GetLastError();
        OutputDebugString((CString)_T("バックアップリネーム失敗 (err=") + std::to_wstring(err).c_str() + _T("): ") + dstPath + _T(" -> ") + bakPath);
        return false;
      }
    }

    CString srcDoubleNull = srcPath + _T("\\");
    srcDoubleNull += _T('\0');
    srcDoubleNull += _T('\0');

    CString dstDoubleNull = dstPath;
    dstDoubleNull += _T('\0');
    dstDoubleNull += _T('\0');

    SHFILEOPSTRUCT fileOp = { 0 };
    fileOp.wFunc = FO_COPY;
    fileOp.pFrom = srcDoubleNull.GetBuffer();
    fileOp.pTo = dstDoubleNull.GetBuffer();
    fileOp.fFlags = FOF_NOCONFIRMATION | FOF_SILENT | FOF_NOCONFIRMMKDIR;

    int res = SHFileOperation(&fileOp);

    srcDoubleNull.ReleaseBuffer();
    dstDoubleNull.ReleaseBuffer();

    if (res != 0 || fileOp.fAnyOperationsAborted) {
      OutputDebugString((CString)_T("SHFileOperationによるコピー失敗 (err=") + std::to_wstring(res).c_str() + _T("): ") + srcPath + _T(" -> ") + dstPath);
      return false;
    }
    return true;
  }
};
