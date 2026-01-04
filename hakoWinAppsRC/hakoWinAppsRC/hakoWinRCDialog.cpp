// hakoWinRCDialog.cpp : 実装ファイル
//

#include "pch.h"
#include "afxdialogex.h"
#include "hakoWinRCDialog.h"
#include "resource.h"

#include <filesystem>
#include <map>
#include <fstream>
#include <sstream>
#include <locale>
#include <windows.h>

// UTF-8 → CString 変換関数
static CString Utf8ToCStr(const std::string& utf8)
{
  int len = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.length(), NULL, 0);
  CString result;
  if (len > 0) {
    wchar_t* buf = result.GetBuffer(len);
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.length(), buf, len);
    result.ReleaseBuffer(len);
  }
  return result;
}

// hakoWinRCDialog ダイアログ
IMPLEMENT_DYNAMIC(hakoWinRCDialog, CDialogEx)

hakoWinRCDialog::hakoWinRCDialog(const CString CurrentPath, const CString SearchPath, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_RC_DIALOG, pParent), m_CurrentPath(CurrentPath), m_SearchPath(SearchPath)
{

}

hakoWinRCDialog::~hakoWinRCDialog()
{
}

void hakoWinRCDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(hakoWinRCDialog, CDialogEx)
  ON_BN_CLICKED(IDOK, &hakoWinRCDialog::OnBnClickedOk)
END_MESSAGE_MAP()


// hakoWinRCDialog メッセージ ハンドラー


BOOL hakoWinRCDialog::OnInitDialog()
{
  CDialogEx::OnInitDialog();

  // コンボボックスの初期化
  InitConfList();


  return TRUE;  // フォーカスをコントロールに設定した場合を除き、TRUE を返します。
}

void hakoWinRCDialog::OnBnClickedOk()
{
  // TODO: ここにコントロール通知ハンドラー コードを追加します。
  int sel = m_confFile.GetCurSel();
  if (sel != CB_ERR)
  {
    // 表示ラベルではなく、ItemDataPtr に保持してあるファイル名を取得
    CString* pFilename = (CString*)m_confFile.GetItemDataPtr(sel);
    if (pFilename != nullptr)
    {
      m_HakoRCConf = *pFilename;  // ファイル名を格納
    }
  }

  CDialogEx::OnOK();
}

std::map<CString, CString> LoadLabelMapFromConf(const CString& confPath)
{
  std::map<CString, CString> labelMap;

  std::ifstream file(CT2A(confPath), std::ios::binary);
#ifdef _DEBUG
  AfxMessageBox(confPath);
#endif

  if (!file.is_open()) {
#ifdef _DEBUG
    AfxMessageBox(_T("confファイルが開けない…"));
#endif
    return labelMap;
  }

#ifdef _DEBUG
  AfxMessageBox(_T("confファイルが開けた！"));
#endif

  std::string line;

  while (std::getline(file, line))
  {
    size_t commaPos = line.find(',');
    if (commaPos != std::string::npos)
    {
#ifdef _DEBUG
      CString dbg = Utf8ToCStr(line);
      AfxMessageBox(dbg);
#endif
      std::string key = line.substr(0, commaPos);
      std::string value = line.substr(commaPos + 1);

      CString ckey = Utf8ToCStr(key);
      CString cvalue = Utf8ToCStr(value);

      labelMap[ckey.Trim()] = cvalue.Trim();
    }
  }

  return labelMap;
}

void hakoWinRCDialog::InitConfList(void)
{
  m_confFile.SubclassDlgItem(IDC_COMBO1, this);

  // confファイルからラベルマップを読み込む
  CString confPath = m_CurrentPath + _T("\\controller_map.conf");
#ifdef _DEBUG
  AfxMessageBox(m_CurrentPath);
  AfxMessageBox(confPath);
#endif
  std::map<CString, CString> labelMap = LoadLabelMapFromConf(confPath);

  CString searchPath = m_SearchPath + _T("\\*.json");
#ifdef _DEBUG
  AfxMessageBox(m_SearchPath);
  for (const auto& pair : labelMap) {
    AfxMessageBox(_T("confキー: ") + pair.first);
  }
#endif

  CFileFind finder;
#ifdef _DEBUG
  AfxMessageBox(_T("searchPath: ") + searchPath);
#endif
  BOOL bWorking = finder.FindFile(searchPath);
  if (!bWorking) {
    AfxMessageBox(_T("ファイルが見つかりません"));
  }  int ps4Index = -1;

  while (bWorking)
  {
#ifdef _DEBUG
    AfxMessageBox(_T("ｂWorkingでwhile実行中"));
#endif
    bWorking = finder.FindNextFile();
    if (!finder.IsDots() && !finder.IsDirectory())
    {
      CString filename = finder.GetFileName().Trim();
#ifdef _DEBUG
      AfxMessageBox(_T("ファイル名: ") + filename);
#endif

      auto it = labelMap.find(filename);
      if (it != labelMap.end())
      {
        CString label = it->second.Trim();
        int index = m_confFile.AddString(label);
        m_confFile.SetItemDataPtr(index, new CString(filename));

        if (filename.CompareNoCase(_T("ps4-control.json")) == 0)
          ps4Index = index;
      }
    }
  }

  if (ps4Index >= 0)
    m_confFile.SetCurSel(ps4Index);
}
