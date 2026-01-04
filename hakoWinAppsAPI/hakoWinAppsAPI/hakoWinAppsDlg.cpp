
#include "pch.h"
#include "framework.h"
#include "hakoWinApps.h"
#include "hakoWinAppsDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// アプリケーションのバージョン情報に使われる CAboutDlg ダイアログ

class CAboutDlg : public CDialogEx
{
public:
  CAboutDlg();

  // ダイアログ データ
#ifdef AFX_DESIGN_TIME
  enum { IDD = IDD_ABOUTBOX };
#endif

protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

  // 実装
protected:
  DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()




// ChakoWinAppsDlg ダイアログ

ChakoWinAppsDlg::ChakoWinAppsDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_HAKOWINAPPS_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void ChakoWinAppsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_MFCEDITBROWSE1, m_EditBrowseCtrl1);
  DDX_Control(pDX, IDC_MFCEDITBROWSE2, m_EditBrowseCtrl2);
  DDX_Control(pDX, IDC_MFCEDITBROWSE3, m_EditBrowseCtrl3);
}

BEGIN_MESSAGE_MAP(ChakoWinAppsDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
  ON_EN_CHANGE(IDC_MFCEDITBROWSE1, &ChakoWinAppsDlg::OnEnChangeMfceditbrowse1)
  ON_EN_CHANGE(IDC_MFCEDITBROWSE2, &ChakoWinAppsDlg::OnEnChangeMfceditbrowse2)
  ON_EN_CHANGE(IDC_MFCEDITBROWSE3, &ChakoWinAppsDlg::OnEnChangeMfceditbrowse3)
  ON_BN_CLICKED(IDC_BUTTON1, &ChakoWinAppsDlg::OnBnClickedButton1)
  ON_BN_CLICKED(IDC_BUTTON2, &ChakoWinAppsDlg::OnBnClickedButton2)
  ON_BN_CLICKED(IDC_BUTTON4, &ChakoWinAppsDlg::OnBnClickedButton4)
  ON_BN_CLICKED(IDC_BUTTON5, &ChakoWinAppsDlg::OnBnClickedButton5)
  ON_BN_CLICKED(IDC_BUTTON3, &ChakoWinAppsDlg::OnBnClickedButton3)
END_MESSAGE_MAP()


// ChakoWinAppsDlg メッセージ ハンドラー

BOOL ChakoWinAppsDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// "バージョン情報..." メニューをシステム メニューに追加します。

	// IDM_ABOUTBOX は、システム コマンドの範囲内になければなりません。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// このダイアログのアイコンを設定します。アプリケーションのメイン ウィンドウがダイアログでない場合、
	//  Framework は、この設定を自動的に行います。
	SetIcon(m_hIcon, TRUE);			// 大きいアイコンの設定
	SetIcon(m_hIcon, FALSE);		// 小さいアイコンの設定

	// TODO: 初期化をここに追加します。

  CString strFileName = _T("hakoapi.ini");
  CString strFilePath = _T("");
  CString currentPath = GetModuleDirectory();

  strFilePath = currentPath + strFileName;

  // INIファイルから初期値のパス情報を取得

  TCHAR szPath[MAX_PATH];


  // HakoWinPathの読み込み
  GetPrivateProfileString(_T("hakoWinAppsAPI"),
    _T("HakoWinPath"),
    _T("C:\\Initial\\Path"),
    szPath,
    MAX_PATH,
    strFilePath);

  m_HakoWinPath = szPath;
  m_EditBrowseCtrl1.SetWindowText(m_HakoWinPath);

  // HakoConfPathの読み込み
  GetPrivateProfileString(_T("hakoWinAppsAPI"),
    _T("HakoConfPath"),
    _T("C:\\Initial\\Path"),
    szPath,
    MAX_PATH,
    strFilePath);

  m_HakoConfPath = szPath;

  // HakoAvatarAppの読み込み
  GetPrivateProfileString(_T("hakoWinAppsAPI"),
    _T("HakoAvatarApp"),
    _T("C:\\Initial\\Path"),
    szPath,
    MAX_PATH,
    strFilePath);

  m_HakoAvatarApps = szPath;
  m_EditBrowseCtrl2.SetWindowText(m_HakoAvatarApps);

  // HakoAvatarConfの読み込み
  GetPrivateProfileString(_T("hakoWinAppsAPI"),
    _T("HakoAvatarConf"),
    _T("C:\\Initial\\Path"),
    szPath,
    MAX_PATH,
    strFilePath);

  m_HakoAvatarConf = szPath;

  // HakoPyPathの読み込み
  GetPrivateProfileString(_T("hakoWinAppsAPI"),
    _T("HakoPyPath"),
    _T("C:\\Initial\\Path"),
    szPath,
    MAX_PATH,
    strFilePath);

  m_HakoPyPath = szPath;

  // HakoPyConfの読み込み
  GetPrivateProfileString(_T("hakoWinAppsAPI"),
    _T("HakoPyConf"),
    _T("C:\\Initial\\Path"),
    szPath,
    MAX_PATH,
    strFilePath);

  m_HakoPyConf = szPath;

  // mmapディレクトリの作成
  // iniファイルから mmap ディレクトリパスを取得
  TCHAR szMmapDir[MAX_PATH];
  GetPrivateProfileString(_T("hakoWinAppsAPI"),
    _T("mmapDir"),
    _T("Z:\\mmap"), // デフォルト値
    szMmapDir,
    MAX_PATH,
    strFilePath);

  CString mmapDir = szMmapDir;

  // ディレクトリの存在確認と作成
  DWORD attr = GetFileAttributes(mmapDir);
  if (attr == INVALID_FILE_ATTRIBUTES || !(attr & FILE_ATTRIBUTE_DIRECTORY)) {
    if (!CreateDirectory(mmapDir, NULL)) {
      AfxMessageBox(_T("mmap ディレクトリの作成に失敗しました"));
    }
  }

  // mmap-0x100.binの作成
  // iniファイルからパスとサイズを取得
  TCHAR szFilePath[MAX_PATH];
  GetPrivateProfileString(_T("hakoWinAppsAPI"),
    _T("ZeroFilePath"),
    _T("Z:\\mmap\\mmap-0x100.bin"), // デフォルト値
    szFilePath,
    MAX_PATH,
    strFilePath);

  int fileSizeMB = GetPrivateProfileInt(_T("hakoWinAppsAPI"),
    _T("ZeroFileSizeMB"),
    5, // デフォルト値
    strFilePath);

  // Pythonプログラムのパスと引数を指定
  GetPrivateProfileString(_T("hakoWinAppsAPI"),
    _T("HakoWinScripts"),
    _T("C:\\Initial\\Path"),
    szPath,
    MAX_PATH,
    strFilePath);

  m_HakoScriptPath = szPath;

  std::string directory = std::string(CT2A(m_HakoScriptPath));

  // Pythonコマンドを構築（INIから取得した値を使用）
  std::string pycommand = "python make_zero_file.py \"" +
    std::string(CT2A(szFilePath)) + "\" " +
    std::to_string(fileSizeMB);

  std::string command =
    pycommand +
    " ; if ($LASTEXITCODE -ne 0) { "
    "Write-Host 'Error: Python execution failed with code' $LASTEXITCODE ; "
    "pause ; exit $LASTEXITCODE }";

  // PowerShellコマンドを実行
  int result = runPowerShellCommand2(command, directory);

  if (result != 0) {
    AfxMessageBox(_T("Zero Fileの作成に失敗しました！"));
    return FALSE;
  }

  // hakoniwa-pdu package upgrade
  pycommand = "pip install --upgrade hakoniwa-pdu";

  command = pycommand +
    " ; if ($LASTEXITCODE -ne 0) { "
    "Write-Host 'Error: Python execution failed with code' $LASTEXITCODE ; "
    "pause ; exit $LASTEXITCODE }";

  // PowerShellコマンドを実行
  result = runPowerShellCommand2(command, directory);

  if (result != 0) {
    AfxMessageBox(_T("hakoniwa-pduのpipアップグレードが失敗しました！"));
    return FALSE;
  }

  // Pythonプログラムの初期値設定
  m_HakoPyProg = m_HakoPyPath + _T("\\api_control_sample.py");

  return TRUE;  // フォーカスをコントロールに設定した場合を除き、TRUE を返します。
}

void ChakoWinAppsDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// ダイアログに最小化ボタンを追加する場合、アイコンを描画するための
//  下のコードが必要です。ドキュメント/ビュー モデルを使う MFC アプリケーションの場合、
//  これは、Framework によって自動的に設定されます。

void ChakoWinAppsDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 描画のデバイス コンテキスト

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// クライアントの四角形領域内の中央
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// アイコンの描画
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// ユーザーが最小化したウィンドウをドラッグしているときに表示するカーソルを取得するために、
//  システムがこの関数を呼び出します。
HCURSOR ChakoWinAppsDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void ChakoWinAppsDlg::OnEnChangeMfceditbrowse1()
{
  // TODO: これが RICHEDIT コントロールの場合、このコントロールが
  // この通知を送信するには、CDialogEx::OnInitDialog() 関数をオーバーライドし、
  // CRichEditCtrl().SetEventMask() を関数し呼び出します。
  // OR 状態の ENM_CHANGE フラグをマスクに入れて呼び出す必要があります。

  // TODO: ここにコントロール通知ハンドラー コードを追加してください。
  // 現在のパス情報を取得
  CString strPath;
  m_EditBrowseCtrl1.GetWindowText(strPath);
  m_HakoWinPath = strPath;

  CString currentPath = _T("");
  CString strFileName = _T("hakoapi.ini");
  CString strFilePath = _T("");

  currentPath = GetModuleDirectory();
  strFilePath = currentPath + strFileName;

  // INIファイルに初期値のパス情報を書き出す
  WritePrivateProfileString(
    _T("hakoWinAppsAPI"),
    _T("HakoWinPath"),
    m_HakoWinPath,
    strFilePath
  );
}

void ChakoWinAppsDlg::OnEnChangeMfceditbrowse2()
{
  // TODO: これが RICHEDIT コントロールの場合、このコントロールが
  // この通知を送信するには、CDialogEx::OnInitDialog() 関数をオーバーライドし、
  // CRichEditCtrl().SetEventMask() を関数し呼び出します。
  // OR 状態の ENM_CHANGE フラグをマスクに入れて呼び出す必要があります。

  // TODO: ここにコントロール通知ハンドラー コードを追加してください。
  // 現在のパス情報を取得
  CString strPath;
  m_EditBrowseCtrl2.GetWindowText(strPath);
  m_HakoAvatarApps = strPath;

  CString currentPath = _T("");
  CString strFileName = _T("hakoapi.ini");
  CString strFilePath = _T("");

  currentPath = GetModuleDirectory();
  strFilePath = currentPath + strFileName;

  // INIファイルに初期値のパス情報を書き出す
  WritePrivateProfileString(
    _T("hakoWinAppsAPI"),
    _T("HakoAvatarApp"),
    m_HakoAvatarApps,
    strFilePath
  );
}

void ChakoWinAppsDlg::OnBnClickedButton1()  
{  
  // TODO: ここにコントロール通知ハンドラー コードを追加します。  
  // Pythonプログラムのパスと引数を指定
  std::string directory = std::string(CT2A(m_HakoWinPath));

  // PowerShellコマンドを構築
  std::string command = ".\\hako_drone_service.exe";

  // CString → std::string へ変換して連結
  command += " " + std::string(CT2A(m_HakoConfPath));
  command += " " + std::string(CT2A(m_HakoAvatarConf));

  // PowerShellコマンドを実行
  runPowerShellCommand(command, directory);
}


// ウィンドウハンドルを取得する関数
HWND FindTargetWindow(const std::wstring& windowTitle)
{
  // 指定されたウィンドウタイトルを持つウィンドウを検索
  HWND hwnd = FindWindow(NULL, windowTitle.c_str());
  if (hwnd == NULL)
  {
    MessageBox(NULL, (L"Window with title \"" + windowTitle + L"\" not found").c_str(), L"Error", MB_OK);
  }
  return hwnd;
}

// マウスクリックイベントを送信する関数
void SendMouseClick(HWND hwnd, int x, int y)
{
  if (hwnd == NULL)
  {
    return;
  }

  // マウスの左ボタンを押すメッセージを送信
  PostMessage(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(x, y));
  // マウスの左ボタンを離すメッセージを送信
  PostMessage(hwnd, WM_LBUTTONUP, 0, MAKELPARAM(x, y));
}

void ChakoWinAppsDlg::OnBnClickedButton2()  
{  
 // TODO: ここにコントロール通知ハンドラー コードを追加します。  
 // 箱庭Unityアプリ起動ボタン  

 std::wstring workDirectory = std::wstring(CT2CW(m_HakoAvatarApps));

 std::wstring exePath = std::wstring(CT2CW(m_HakoAvatarApps));
 exePath += L"\\simulation.exe";

 STARTUPINFO si;
 PROCESS_INFORMATION pi;
 ZeroMemory(&si, sizeof(si));
 si.cb = sizeof(si);
 ZeroMemory(&pi, sizeof(pi));

 // CreateProcess関数を使用して新しいプロセスを作成
 if (!CreateProcess(NULL,   // No module name (use command line)
   &exePath[0],    // Command line
   NULL,           // Process handle not inheritable
   NULL,           // Thread handle not inheritable
   FALSE,          // Set handle inheritance to FALSE
   0,              // No creation flags
   NULL,           // Use parent's environment block
   workDirectory.c_str(),  // Use parent's starting directory 
   &si,            // Pointer to STARTUPINFO structure
   &pi)            // Pointer to PROCESS_INFORMATION structure
   )
 {
   MessageBox(L"Failed to execute the external program", L"Error", MB_OK);
   return;
 }

 // プロセスとスレッドのハンドルを閉じる
 CloseHandle(pi.hProcess);
 CloseHandle(pi.hThread);


 // 少し待ってウィンドウが描画されるのを待つ（必要に応じて調整）
 Sleep(5000); // 数秒待機（描画完了までの猶予）

 // ウィンドウタイトル（Unityアプリのウィンドウタイトルに合わせて変更）
 std::wstring targetWindowTitle = L"Simulation"; // ← 実際のタイトルに合わせてください

 // ウィンドウハンドル取得
 HWND targetHwnd = FindTargetWindow(targetWindowTitle);
 if (targetHwnd == NULL)
 {
   return; // ウィンドウが見つからなければ終了
 }

 // STOPボタンの位置（クライアント座標で推定）
 int clickX = 550; // 中央付近（仮）
 int clickY = 70;  // 上部バーの下（仮）

 // マウスクリック送信（スクリーン座標で送る場合は SendInput() を使う）
 SendMouseClick(targetHwnd, clickX, clickY);

}

void ChakoWinAppsDlg::OnBnClickedButton4()
{
  // Pythonプログラムのパスと引数を指定
  std::string directory = std::string(CT2A(m_HakoPyPath));

  // PowerShellコマンドを構築
  std::string command = "python";

  CString fileName = m_HakoPyProg;
  // バックスラッシュとスラッシュ両方で区切り位置を探す
  int pos1 = fileName.ReverseFind(_T('\\'));
  int pos2 = fileName.ReverseFind(_T('/'));
  int pos = max(pos1, pos2);
  if (pos >= 0) {
    fileName = fileName.Mid(pos + 1);
  }
  // fileName には "Pythonプログラム" が格納される
  std::string pyFileName = std::string(CT2A(fileName));

  // CString → std::string へ変換して連結
  command += " " + pyFileName;
  command += " " + std::string(CT2A(m_HakoPyConf));

  // PowerShellコマンドを実行
  runPowerShellCommand(command, directory);
}

void ChakoWinAppsDlg::OnBnClickedButton5()  
{  
 // TODO: ここにコントロール通知ハンドラー コードを追加します。  
 std::wstring directory = std::wstring(CT2W(m_HakoWinPath));  
 OpenInExplorer(directory);  
}

void ChakoWinAppsDlg::OnEnChangeMfceditbrowse3()
{
  // TODO: これが RICHEDIT コントロールの場合、このコントロールが
  // この通知を送信するには、CDialogEx::OnInitDialog() 関数をオーバーライドし、
  // CRichEditCtrl().SetEventMask() を関数し呼び出します。
  // OR 状態の ENM_CHANGE フラグをマスクに入れて呼び出す必要があります。

  // TODO: ここにコントロール通知ハンドラー コードを追加してください。
  CString strPath;
  m_EditBrowseCtrl3.GetWindowText(strPath);
  m_HakoPyProg = strPath;

  int pos1 = m_HakoPyProg.ReverseFind(_T('\\'));
  if (pos1 != -1)
  {
    CString strDir = strPath.Left(pos1);        // ディレクトリ部分
    m_HakoPyPath = strDir;
  }

}

void ChakoWinAppsDlg::OnBnClickedButton3()
{
  // TODO: ここにコントロール通知ハンドラー コードを追加します。

  // 例: m_HakoWinPath が既に設定されていることが前提

  CString srcName = L"drone_log0";
  CString dstName = L"drone_log1";

  bool success = BackupAndRenameDroneLog(srcName, dstName);

  if(!success){
    AfxMessageBox(_T("ログファイルディレクトリの変換に失敗しました"));
    return;
  }

  // Pythonプログラムのパスと引数を指定
  std::string directory = std::string(CT2A(m_HakoScriptPath));

  // replay用のjsonファイルを修正
  std::string pycommand = "python ./updatereplay_json.py ../../hakoSim/bin/drone_log1/drone_dynamics.csv ../../hakoSim/bin/config/replay/replay.json";
  std::string command = pycommand + " ; if ($LASTEXITCODE -ne 0) { "
    "Write-Host 'Error: Python execution failed with code' $LASTEXITCODE ; "
    "pause ; exit $LASTEXITCODE }";
  // PowerShellコマンドを実行
  int result = runPowerShellCommand2(command, directory);

  if (result != 0) {
    AfxMessageBox(_T("replay用jsonファイルの更新に失敗しました！"));
    return;
  }

  // Pythonプログラムのパスと引数を指定(ログリプレイの実行)
  directory = std::string(CT2A(m_HakoWinPath));

  // PowerShellコマンドを構築
  command = "python -m replay.hako_asset_replayer --replay ./config/replay/replay.json";
  // PowerShellコマンドを実行
  runPowerShellCommand(command, directory);

  // Button2の処理を実行
  OnBnClickedButton2();
}
