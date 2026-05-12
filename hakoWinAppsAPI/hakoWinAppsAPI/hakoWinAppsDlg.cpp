
#include "pch.h"
#include "framework.h"
#include "hakoWinApps.h"
#include "hakoWinAppsDlg.h"
#include "afxdialogex.h"

#include <filesystem>
#include <string>
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace
{
  constexpr wchar_t kIniFileName[] = L"hakoapi.ini";
  constexpr wchar_t kIniSection[] = L"hakoWinAppsAPI";

  bool PathExists(const CString& path)
  {
    return !path.IsEmpty() && ::GetFileAttributes(path) != INVALID_FILE_ATTRIBUTES;
  }

  bool FileSizeMatches(const CString& path, int expectedSizeMB)
  {
    HANDLE hFile = ::CreateFile(path, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) {
      return false;
    }

    LARGE_INTEGER size = {};
    bool ok = ::GetFileSizeEx(hFile, &size) != FALSE;
    ::CloseHandle(hFile);

    const LONGLONG expectedBytes = static_cast<LONGLONG>(expectedSizeMB) * 1024LL * 1024LL;
    return ok && size.QuadPart == expectedBytes;
  }

  CString GetIniPath(const CString& moduleDirectory)
  {
    std::filesystem::path iniPath(moduleDirectory.GetString());
    iniPath /= kIniFileName;
    return CString(iniPath.wstring().c_str());
  }

  CString ReadIniString(const CString& iniPath, const CString& key, const CString& defaultValue)
  {
    TCHAR value[MAX_PATH];
    ::GetPrivateProfileString(kIniSection, key, defaultValue, value, MAX_PATH, iniPath);
    return value;
  }

  void EnsureIniString(const CString& iniPath, const CString& key, const CString& defaultValue)
  {
    CString currentValue = ReadIniString(iniPath, key, _T(""));
    if (currentValue.IsEmpty()) {
      ::WritePrivateProfileString(kIniSection, key, defaultValue, iniPath);
    }
  }

  void EnsureIniInt(const CString& iniPath, const CString& key, int defaultValue)
  {
    TCHAR value[32];
    ::GetPrivateProfileString(kIniSection, key, _T(""), value, _countof(value), iniPath);
    if (value[0] == _T('\0')) {
      CString defaultText;
      defaultText.Format(_T("%d"), defaultValue);
      ::WritePrivateProfileString(kIniSection, key, defaultText, iniPath);
    }
  }

  void EnsureHakoCmdIniDefaults(const CString& iniPath)
  {
    CString defaultWorkDir = ReadIniString(iniPath, _T("HakoWinPath"), _T(""));
    EnsureIniString(iniPath, _T("HakoCmdPath"), _T("hako-cmd.exe"));
    EnsureIniString(iniPath, _T("HakoCmdWorkDir"), defaultWorkDir);
    EnsureIniString(iniPath, _T("HakoCmdLogPath"), _T(".\\hako-cmd.log"));
    EnsureIniString(iniPath, _T("HakoCmdStartArgs"), _T("start"));
    EnsureIniInt(iniPath, _T("HakoCmdStartRetryCount"), 25);
    EnsureIniInt(iniPath, _T("HakoCmdStartRetryIntervalMs"), 200);
    EnsureIniInt(iniPath, _T("HakoCmdStartReadyGraceMs"), 0);
    EnsureIniInt(iniPath, _T("HakoCmdStartTotalTimeoutMs"), 30000);
    EnsureIniInt(iniPath, _T("HakoCmdStartCommandTimeoutMs"), 3000);
    EnsureIniInt(iniPath, _T("HakoCmdStartRecheckReadyOnFailure"), 1);
    EnsureIniString(iniPath, _T("HakoCmdLsArgs"), _T("ls"));
    EnsureIniInt(iniPath, _T("HakoCmdLsTimeoutMs"), 3000);
    EnsureIniInt(iniPath, _T("HakoCmdReadyTimeoutMs"), 150000);
    EnsureIniInt(iniPath, _T("HakoCmdReadyPollIntervalMs"), 200);
    EnsureIniInt(iniPath, _T("HakoCmdReadyStableCount"), 3);
    EnsureIniString(iniPath, _T("HakoCmdReadyAssetNames"), _T("UnityAsset,GodotAsset"));
    EnsureIniString(iniPath, _T("HakoCmdReadyIgnoreAssetNames"), _T("drone"));
    EnsureIniInt(iniPath, _T("HakoAvatarStartupWaitMs"), 10000);
    EnsureIniInt(iniPath, _T("HakoAvatarPostStartupDelayMs"), 0);
  }

  CString GetHakoCmdLogPath(const CString& iniPath)
  {
    CString configuredPath = ReadIniString(iniPath, _T("HakoCmdLogPath"), _T(""));
    std::filesystem::path logPath;
    if (configuredPath.IsEmpty()) {
      logPath = std::filesystem::path(iniPath.GetString()).parent_path() / L"hako-cmd.log";
    }
    else {
      logPath = std::filesystem::path(configuredPath.GetString());
      if (logPath.is_relative()) {
        logPath = std::filesystem::path(iniPath.GetString()).parent_path() / logPath;
      }
    }
    return CString(logPath.wstring().c_str());
  }

  CString GetLogTimestamp()
  {
    SYSTEMTIME now = {};
    ::GetLocalTime(&now);
    CString timestamp;
    timestamp.Format(
      _T("%04u-%02u-%02u %02u:%02u:%02u.%03u"),
      now.wYear,
      now.wMonth,
      now.wDay,
      now.wHour,
      now.wMinute,
      now.wSecond,
      now.wMilliseconds);
    return timestamp;
  }

  bool AppendUtf8Text(const CString& path, const CString& text)
  {
    if (path.IsEmpty() || text.IsEmpty()) {
      return false;
    }

    try {
      std::filesystem::path logPath(path.GetString());
      std::filesystem::path parentPath = logPath.parent_path();
      if (!parentPath.empty()) {
        std::filesystem::create_directories(parentPath);
      }
    }
    catch (...) {
      return false;
    }

    int byteCount = ::WideCharToMultiByte(CP_UTF8, 0, text, text.GetLength(), nullptr, 0, nullptr, nullptr);
    if (byteCount <= 0) {
      return false;
    }

    std::string utf8(static_cast<size_t>(byteCount), '\0');
    if (::WideCharToMultiByte(CP_UTF8, 0, text, text.GetLength(), &utf8[0], byteCount, nullptr, nullptr) != byteCount) {
      return false;
    }

    HANDLE hFile = ::CreateFile(path, FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) {
      return false;
    }

    DWORD written = 0;
    bool ok = ::WriteFile(hFile, utf8.data(), static_cast<DWORD>(utf8.size()), &written, nullptr) != FALSE
      && written == utf8.size();
    ::CloseHandle(hFile);
    return ok;
  }

  void AppendHakoCmdLog(const CString& logPath, const CString& message)
  {
    CString line;
    line.Format(_T("[%s] %s\r\n"), GetLogTimestamp().GetString(), message.GetString());
    AppendUtf8Text(logPath, line);
  }

  void AppendHakoCmdOutputLog(const CString& logPath, const CString& commandName, const CString& output)
  {
    CString header;
    header.Format(_T("[%s] %s stdout/stderr begin\r\n"), GetLogTimestamp().GetString(), commandName.GetString());
    AppendUtf8Text(logPath, header);
    if (output.IsEmpty()) {
      AppendUtf8Text(logPath, _T("(no output)\r\n"));
    }
    else {
      AppendUtf8Text(logPath, output);
      if (output.Right(1) != _T("\n")) {
        AppendUtf8Text(logPath, _T("\r\n"));
      }
    }
    CString footer;
    footer.Format(_T("[%s] %s stdout/stderr end\r\n"), GetLogTimestamp().GetString(), commandName.GetString());
    AppendUtf8Text(logPath, footer);
  }

  CString QuoteArg(const CString& value)
  {
    CString quoted = _T("\"");
    quoted += value;
    quoted += _T("\"");
    return quoted;
  }

  bool RunProcess(const CString& executablePath, const CString& arguments, const CString& workingDirectory, bool waitForExit, DWORD* exitCode = nullptr)
  {
    CString commandLine = QuoteArg(executablePath);
    if (!arguments.IsEmpty()) {
      commandLine += _T(" ");
      commandLine += arguments;
    }

    std::vector<wchar_t> mutableCommandLine(commandLine.GetString(), commandLine.GetString() + commandLine.GetLength() + 1);

    STARTUPINFO si = {};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi = {};

    BOOL started = ::CreateProcess(
      nullptr,
      mutableCommandLine.data(),
      nullptr,
      nullptr,
      FALSE,
      0,
      nullptr,
      workingDirectory.IsEmpty() ? nullptr : workingDirectory.GetString(),
      &si,
      &pi);

    if (!started) {
      return false;
    }

    if (waitForExit) {
      ::WaitForSingleObject(pi.hProcess, INFINITE);
      DWORD code = 0;
      if (::GetExitCodeProcess(pi.hProcess, &code) && exitCode != nullptr) {
        *exitCode = code;
      }
    }

    ::CloseHandle(pi.hProcess);
    ::CloseHandle(pi.hThread);
    return true;
  }

  bool RunProcessCaptureOutput(const CString& executablePath, const CString& arguments, const CString& workingDirectory, DWORD timeoutMs, DWORD* exitCode, CString* output, bool* timedOutResult = nullptr)
  {
    if (timedOutResult != nullptr) {
      *timedOutResult = false;
    }

    SECURITY_ATTRIBUTES securityAttributes = {};
    securityAttributes.nLength = sizeof(securityAttributes);
    securityAttributes.bInheritHandle = TRUE;

    HANDLE readPipe = nullptr;
    HANDLE writePipe = nullptr;
    if (!::CreatePipe(&readPipe, &writePipe, &securityAttributes, 0)) {
      return false;
    }
    if (!::SetHandleInformation(readPipe, HANDLE_FLAG_INHERIT, 0)) {
      ::CloseHandle(readPipe);
      ::CloseHandle(writePipe);
      return false;
    }

    CString commandLine = QuoteArg(executablePath);
    if (!arguments.IsEmpty()) {
      commandLine += _T(" ");
      commandLine += arguments;
    }

    std::vector<wchar_t> mutableCommandLine(commandLine.GetString(), commandLine.GetString() + commandLine.GetLength() + 1);

    STARTUPINFO si = {};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdOutput = writePipe;
    si.hStdError = writePipe;
    si.hStdInput = ::GetStdHandle(STD_INPUT_HANDLE);

    PROCESS_INFORMATION pi = {};
    BOOL started = ::CreateProcess(
      nullptr,
      mutableCommandLine.data(),
      nullptr,
      nullptr,
      TRUE,
      CREATE_NO_WINDOW,
      nullptr,
      workingDirectory.IsEmpty() ? nullptr : workingDirectory.GetString(),
      &si,
      &pi);

    ::CloseHandle(writePipe);

    if (!started) {
      ::CloseHandle(readPipe);
      return false;
    }

    std::string captured;
    ULONGLONG startTick = ::GetTickCount64();
    bool timedOut = false;

    for (;;) {
      DWORD available = 0;
      while (::PeekNamedPipe(readPipe, nullptr, 0, nullptr, &available, nullptr) && available > 0) {
        char buffer[4096];
        DWORD bytesToRead = available < static_cast<DWORD>(sizeof(buffer)) ? available : static_cast<DWORD>(sizeof(buffer));
        DWORD bytesRead = 0;
        if (!::ReadFile(readPipe, buffer, bytesToRead, &bytesRead, nullptr) || bytesRead == 0) {
          break;
        }
        captured.append(buffer, buffer + bytesRead);
        available = 0;
      }

      DWORD waitResult = ::WaitForSingleObject(pi.hProcess, 50);
      if (waitResult == WAIT_OBJECT_0) {
        DWORD code = 0;
        if (::GetExitCodeProcess(pi.hProcess, &code) && exitCode != nullptr) {
          *exitCode = code;
        }
        while (::PeekNamedPipe(readPipe, nullptr, 0, nullptr, &available, nullptr) && available > 0) {
          char buffer[4096];
          DWORD bytesToRead = available < static_cast<DWORD>(sizeof(buffer)) ? available : static_cast<DWORD>(sizeof(buffer));
          DWORD bytesRead = 0;
          if (!::ReadFile(readPipe, buffer, bytesToRead, &bytesRead, nullptr) || bytesRead == 0) {
            break;
          }
          captured.append(buffer, buffer + bytesRead);
          available = 0;
        }
        break;
      }

      if (timeoutMs > 0 && (::GetTickCount64() - startTick) >= timeoutMs) {
        timedOut = true;
        ::TerminateProcess(pi.hProcess, WAIT_TIMEOUT);
        if (exitCode != nullptr) {
          *exitCode = WAIT_TIMEOUT;
        }
        break;
      }
    }

    ::CloseHandle(pi.hProcess);
    ::CloseHandle(pi.hThread);
    ::CloseHandle(readPipe);

    if (output != nullptr) {
      CStringA outputA(captured.c_str());
      *output = CString(outputA);
    }
    if (timedOutResult != nullptr) {
      *timedOutResult = timedOut;
    }

    return !timedOut;
  }

  DWORD ReadIniDword(const CString& iniPath, const CString& key, int defaultValue)
  {
    int value = ::GetPrivateProfileInt(kIniSection, key, defaultValue, iniPath);
    return value < 0 ? 0 : static_cast<DWORD>(value);
  }

  int ReadIniPositiveInt(const CString& iniPath, const CString& key, int defaultValue)
  {
    int value = ::GetPrivateProfileInt(kIniSection, key, defaultValue, iniPath);
    return value <= 0 ? defaultValue : value;
  }

  std::vector<CString> SplitCsv(const CString& value)
  {
    std::vector<CString> items;
    int start = 0;
    while (start <= value.GetLength()) {
      int comma = value.Find(_T(','), start);
      CString item = comma >= 0 ? value.Mid(start, comma - start) : value.Mid(start);
      item.Trim();
      if (!item.IsEmpty()) {
        items.push_back(item);
      }
      if (comma < 0) {
        break;
      }
      start = comma + 1;
    }
    return items;
  }

  bool ContainsName(const std::vector<CString>& names, const CString& value)
  {
    for (const CString& name : names) {
      if (name.Compare(value) == 0) {
        return true;
      }
    }
    return false;
  }

  bool IsHakoCmdLogLine(const CString& line)
  {
    return line.Find(_T("INFO:")) == 0
      || line.Find(_T("Warning:")) == 0
      || line.Find(_T("ERROR:")) == 0
      || line.Find(_T("DEBUG:")) == 0
      || line.Find(_T("Trace:")) == 0
      || line.Find(_T("TRACE:")) == 0;
  }

  bool HasReadyAsset(const CString& output, const std::vector<CString>& readyAssetNames, const std::vector<CString>& ignoreAssetNames)
  {
    int start = 0;
    while (start <= output.GetLength()) {
      int newline = output.Find(_T('\n'), start);
      CString line = newline >= 0 ? output.Mid(start, newline - start) : output.Mid(start);
      line.Trim();

      if (!line.IsEmpty() && !IsHakoCmdLogLine(line) && !ContainsName(ignoreAssetNames, line)) {
        if (readyAssetNames.empty() || ContainsName(readyAssetNames, line)) {
          return true;
        }
      }

      if (newline < 0) {
        break;
      }
      start = newline + 1;
    }
    return false;
  }

  bool CheckHakoReadyAssetOnce(
    const CString& iniPath,
    const CString& hakoCmdPath,
    const CString& hakoCmdWorkDir,
    const std::vector<CString>& readyAssetNames,
    const std::vector<CString>& ignoreAssetNames,
    bool* executed,
    bool* failed)
  {
    CString hakoCmdLogPath = GetHakoCmdLogPath(iniPath);
    CString hakoCmdLsArgs = ReadIniString(iniPath, _T("HakoCmdLsArgs"), _T("ls"));
    DWORD lsTimeoutMs = ReadIniDword(iniPath, _T("HakoCmdLsTimeoutMs"), 3000);

    DWORD exitCode = 1;
    CString output;
    bool timedOut = false;
    CString lsStartMessage;
    lsStartMessage.Format(_T("hako-cmd ls launch: exe=%s args=%s workdir=%s"), hakoCmdPath.GetString(), hakoCmdLsArgs.GetString(), hakoCmdWorkDir.GetString());
    AppendHakoCmdLog(hakoCmdLogPath, lsStartMessage);
    if (RunProcessCaptureOutput(hakoCmdPath, hakoCmdLsArgs, hakoCmdWorkDir, lsTimeoutMs, &exitCode, &output, &timedOut)) {
      if (executed != nullptr) {
        *executed = true;
      }
      CString lsFinishedMessage;
      lsFinishedMessage.Format(_T("hako-cmd ls finished: exitCode=%lu"), exitCode);
      AppendHakoCmdLog(hakoCmdLogPath, lsFinishedMessage);
      AppendHakoCmdOutputLog(hakoCmdLogPath, _T("hako-cmd ls"), output);
      return exitCode == 0 && HasReadyAsset(output, readyAssetNames, ignoreAssetNames);
    }

    if (failed != nullptr) {
      *failed = true;
    }
    CString lsFailedMessage;
    lsFailedMessage.Format(_T("hako-cmd ls failed or timed out: exitCode=%lu timedOut=%d"), exitCode, timedOut ? 1 : 0);
    AppendHakoCmdLog(hakoCmdLogPath, lsFailedMessage);
    AppendHakoCmdOutputLog(hakoCmdLogPath, _T("hako-cmd ls"), output);
    return false;
  }

  bool WaitForHakoReadyAsset(const CString& iniPath, const CString& hakoCmdPath, const CString& hakoCmdWorkDir)
  {
    CString hakoCmdLogPath = GetHakoCmdLogPath(iniPath);
    CString readyAssetNamesValue = ReadIniString(iniPath, _T("HakoCmdReadyAssetNames"), _T("UnityAsset,GodotAsset"));
    CString ignoreAssetNamesValue = ReadIniString(iniPath, _T("HakoCmdReadyIgnoreAssetNames"), _T("drone"));
    DWORD readyTimeoutMs = ReadIniDword(iniPath, _T("HakoCmdReadyTimeoutMs"), 30000);
    DWORD pollIntervalMs = ReadIniDword(iniPath, _T("HakoCmdReadyPollIntervalMs"), 200);
    int requiredStableCount = ReadIniPositiveInt(iniPath, _T("HakoCmdReadyStableCount"), 3);

    std::vector<CString> readyAssetNames = SplitCsv(readyAssetNamesValue);
    std::vector<CString> ignoreAssetNames = SplitCsv(ignoreAssetNamesValue);

    ULONGLONG startTick = ::GetTickCount64();
    bool lsExecuted = false;
    bool lsFailed = false;
    int stableCount = 0;

    while ((::GetTickCount64() - startTick) <= readyTimeoutMs) {
      if (CheckHakoReadyAssetOnce(iniPath, hakoCmdPath, hakoCmdWorkDir, readyAssetNames, ignoreAssetNames, &lsExecuted, &lsFailed)) {
        ++stableCount;
        CString stableMessage;
        stableMessage.Format(_T("hako-cmd ls detected a ready asset: stableCount=%d/%d"), stableCount, requiredStableCount);
        AppendHakoCmdLog(hakoCmdLogPath, stableMessage);
        if (stableCount >= requiredStableCount) {
          AppendHakoCmdLog(hakoCmdLogPath, _T("hako-cmd ls ready asset is stable."));
          return true;
        }
      }
      else {
        if (stableCount > 0) {
          AppendHakoCmdLog(hakoCmdLogPath, _T("hako-cmd ls ready asset was not detected. Reset stable count."));
        }
        stableCount = 0;
      }
      ::Sleep(pollIntervalMs);
    }

    if (!lsExecuted && lsFailed) {
      AfxMessageBox(_T("hako-cmd.exe ls の実行に失敗しました。\nHakoCmdPath と PATH 環境変数を確認してください。"));
    }
    else {
      AfxMessageBox(_T("hako-cmd.exe ls で Avatar アセットを確認できませんでした。\nUnityAsset または GodotAsset が表示される前にタイムアウトしました。"));
    }
    return false;
  }

  enum class AvatarProcessStartStatus
  {
    CreateProcessFailed,
    InputIdleReady,
    InputIdleTimeoutButRunning,
    InputIdleFailedButRunning,
    ExitedBeforeReady,
    ExitCodeCheckFailed
  };

  AvatarProcessStartStatus StartAvatarProcessAndObserveInputIdle(
    const CString& executablePath,
    const CString& arguments,
    const CString& workingDirectory,
    DWORD startupWaitMs,
    const CString& logPath)
  {
    CString commandLine = QuoteArg(executablePath);
    if (!arguments.IsEmpty()) {
      commandLine += _T(" ");
      commandLine += arguments;
    }

    std::vector<wchar_t> mutableCommandLine(commandLine.GetString(), commandLine.GetString() + commandLine.GetLength() + 1);

    STARTUPINFO si = {};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi = {};

    CString launchMessage;
    launchMessage.Format(_T("hakoAvatar launch begin: exe=%s workdir=%s startupWaitMs=%lu"), executablePath.GetString(), workingDirectory.GetString(), startupWaitMs);
    AppendHakoCmdLog(logPath, launchMessage);

    BOOL started = ::CreateProcess(
      nullptr,
      mutableCommandLine.data(),
      nullptr,
      nullptr,
      FALSE,
      0,
      nullptr,
      workingDirectory.IsEmpty() ? nullptr : workingDirectory.GetString(),
      &si,
      &pi);

    if (!started) {
      DWORD lastError = ::GetLastError();
      CString failedMessage;
      failedMessage.Format(_T("hakoAvatar CreateProcess failed: error=%lu"), lastError);
      AppendHakoCmdLog(logPath, failedMessage);
      return AvatarProcessStartStatus::CreateProcessFailed;
    }

    DWORD waitResult = ::WaitForInputIdle(pi.hProcess, startupWaitMs);
    DWORD waitLastError = waitResult == WAIT_FAILED ? ::GetLastError() : ERROR_SUCCESS;

    CString waitMessage;
    waitMessage.Format(_T("hakoAvatar WaitForInputIdle returned: result=%lu lastError=%lu"), waitResult, waitLastError);
    AppendHakoCmdLog(logPath, waitMessage);

    AvatarProcessStartStatus status = AvatarProcessStartStatus::InputIdleReady;
    if (waitResult == 0) {
      status = AvatarProcessStartStatus::InputIdleReady;
    }
    else {
      DWORD exitCode = 0;
      if (!::GetExitCodeProcess(pi.hProcess, &exitCode)) {
        DWORD lastError = ::GetLastError();
        CString exitCodeFailedMessage;
        exitCodeFailedMessage.Format(_T("hakoAvatar GetExitCodeProcess failed: error=%lu"), lastError);
        AppendHakoCmdLog(logPath, exitCodeFailedMessage);
        status = AvatarProcessStartStatus::ExitCodeCheckFailed;
      }
      else if (exitCode == STILL_ACTIVE) {
        CString runningMessage;
        runningMessage.Format(_T("hakoAvatar is still running after WaitForInputIdle result=%lu. Continue to hako-cmd ls ready wait."), waitResult);
        AppendHakoCmdLog(logPath, runningMessage);
        status = waitResult == WAIT_TIMEOUT
          ? AvatarProcessStartStatus::InputIdleTimeoutButRunning
          : AvatarProcessStartStatus::InputIdleFailedButRunning;
      }
      else {
        CString exitedMessage;
        exitedMessage.Format(_T("hakoAvatar exited before input idle: exitCode=%lu"), exitCode);
        AppendHakoCmdLog(logPath, exitedMessage);
        status = AvatarProcessStartStatus::ExitedBeforeReady;
      }
    }

    ::CloseHandle(pi.hProcess);
    ::CloseHandle(pi.hThread);

    return status;
  }

  bool IsAvatarProcessStartContinuable(AvatarProcessStartStatus status)
  {
    return status == AvatarProcessStartStatus::InputIdleReady
      || status == AvatarProcessStartStatus::InputIdleTimeoutButRunning
      || status == AvatarProcessStartStatus::InputIdleFailedButRunning;
  }

  bool RunHakoStartCommandWithRetry(const CString& iniPath)
  {
    CString hakoCmdLogPath = GetHakoCmdLogPath(iniPath);
    CString hakoCmdPath = ReadIniString(iniPath, _T("HakoCmdPath"), _T("hako-cmd.exe"));
    CString hakoCmdWorkDir = ReadIniString(iniPath, _T("HakoCmdWorkDir"), _T(""));
    CString hakoCmdStartArgs = ReadIniString(iniPath, _T("HakoCmdStartArgs"), _T("start"));
    int retryCount = ::GetPrivateProfileInt(kIniSection, _T("HakoCmdStartRetryCount"), 25, iniPath);
    int retryIntervalMs = ::GetPrivateProfileInt(kIniSection, _T("HakoCmdStartRetryIntervalMs"), 200, iniPath);
    DWORD startReadyGraceMs = ReadIniDword(iniPath, _T("HakoCmdStartReadyGraceMs"), 0);
    DWORD startTotalTimeoutMs = ReadIniDword(iniPath, _T("HakoCmdStartTotalTimeoutMs"), 30000);
    DWORD startCommandTimeoutMs = ReadIniDword(iniPath, _T("HakoCmdStartCommandTimeoutMs"), 3000);
    bool recheckReadyOnFailure = ::GetPrivateProfileInt(kIniSection, _T("HakoCmdStartRecheckReadyOnFailure"), 1, iniPath) != 0;

    CString sequenceStartMessage;
    sequenceStartMessage.Format(_T("hako-cmd start sequence begin: log=%s"), hakoCmdLogPath.GetString());
    AppendHakoCmdLog(hakoCmdLogPath, sequenceStartMessage);

    if (hakoCmdPath.IsEmpty()) {
      hakoCmdPath = _T("hako-cmd.exe");
    }
    const bool pathIncludesDirectory = hakoCmdPath.Find(_T('\\')) >= 0 || hakoCmdPath.Find(_T('/')) >= 0;
    if (pathIncludesDirectory && !PathExists(hakoCmdPath)) {
      AppendHakoCmdLog(hakoCmdLogPath, _T("hako-cmd start sequence failed: hako-cmd.exe was not found."));
      AfxMessageBox(_T("hako-cmd.exe が見つかりません: ") + hakoCmdPath);
      return false;
    }
    if (hakoCmdWorkDir.IsEmpty() && pathIncludesDirectory) {
      int slash = hakoCmdPath.ReverseFind(_T('\\'));
      hakoCmdWorkDir = slash >= 0 ? hakoCmdPath.Left(slash) : _T("");
    }
    if (retryCount <= 0) {
      CString retryMessage;
      retryMessage.Format(_T("HakoCmdStartRetryCount=%d is invalid. Using 1."), retryCount);
      AppendHakoCmdLog(hakoCmdLogPath, retryMessage);
      retryCount = 1;
    }
    if (retryIntervalMs < 0) {
      CString retryIntervalMessage;
      retryIntervalMessage.Format(_T("HakoCmdStartRetryIntervalMs=%d is invalid. Using 0."), retryIntervalMs);
      AppendHakoCmdLog(hakoCmdLogPath, retryIntervalMessage);
      retryIntervalMs = 0;
    }

    if (!WaitForHakoReadyAsset(iniPath, hakoCmdPath, hakoCmdWorkDir)) {
      AppendHakoCmdLog(hakoCmdLogPath, _T("hako-cmd start sequence aborted: ready asset was not detected."));
      return false;
    }

    if (startReadyGraceMs > 0) {
      CString graceMessage;
      graceMessage.Format(_T("hako-cmd start sequence: ready grace wait %lu ms."), startReadyGraceMs);
      AppendHakoCmdLog(hakoCmdLogPath, graceMessage);
      ::Sleep(startReadyGraceMs);
    }

    CString startConfigMessage;
    startConfigMessage.Format(
      _T("hako-cmd start retry config: totalTimeoutMs=%lu commandTimeoutMs=%lu retryCount=%d retryIntervalMs=%d recheckReadyOnFailure=%d"),
      startTotalTimeoutMs,
      startCommandTimeoutMs,
      retryCount,
      retryIntervalMs,
      recheckReadyOnFailure ? 1 : 0);
    AppendHakoCmdLog(hakoCmdLogPath, startConfigMessage);

    ULONGLONG startTick = ::GetTickCount64();
    int attempt = 1;
    for (;;) {
      if (startTotalTimeoutMs > 0 && (::GetTickCount64() - startTick) >= startTotalTimeoutMs) {
        break;
      }
      if (startTotalTimeoutMs == 0 && attempt > retryCount) {
        break;
      }

      DWORD exitCode = 1;
      CString output;
      bool timedOut = false;
      CString startLaunchMessage;
      startLaunchMessage.Format(_T("hako-cmd start launch: attempt=%d exe=%s args=%s workdir=%s"), attempt, hakoCmdPath.GetString(), hakoCmdStartArgs.GetString(), hakoCmdWorkDir.GetString());
      AppendHakoCmdLog(hakoCmdLogPath, startLaunchMessage);
      if (RunProcessCaptureOutput(hakoCmdPath, hakoCmdStartArgs, hakoCmdWorkDir, startCommandTimeoutMs, &exitCode, &output, &timedOut)) {
        CString startFinishedMessage;
        startFinishedMessage.Format(_T("hako-cmd start finished: attempt=%d exitCode=%lu timedOut=%d"), attempt, exitCode, timedOut ? 1 : 0);
        AppendHakoCmdLog(hakoCmdLogPath, startFinishedMessage);
        AppendHakoCmdOutputLog(hakoCmdLogPath, _T("hako-cmd start"), output);
      }
      else {
        CString startFailedMessage;
        startFailedMessage.Format(_T("hako-cmd start failed: attempt=%d exitCode=%lu timedOut=%d"), attempt, exitCode, timedOut ? 1 : 0);
        AppendHakoCmdLog(hakoCmdLogPath, startFailedMessage);
        AppendHakoCmdOutputLog(hakoCmdLogPath, _T("hako-cmd start"), output);
      }
      if (exitCode == 0) {
        return true;
      }

      if (recheckReadyOnFailure) {
        CString readyAssetNamesValue = ReadIniString(iniPath, _T("HakoCmdReadyAssetNames"), _T("UnityAsset,GodotAsset"));
        CString ignoreAssetNamesValue = ReadIniString(iniPath, _T("HakoCmdReadyIgnoreAssetNames"), _T("drone"));
        std::vector<CString> readyAssetNames = SplitCsv(readyAssetNamesValue);
        std::vector<CString> ignoreAssetNames = SplitCsv(ignoreAssetNamesValue);
        bool readyExecuted = false;
        bool readyFailed = false;
        if (!CheckHakoReadyAssetOnce(iniPath, hakoCmdPath, hakoCmdWorkDir, readyAssetNames, ignoreAssetNames, &readyExecuted, &readyFailed)) {
          AppendHakoCmdLog(hakoCmdLogPath, _T("hako-cmd start recheck: ready asset is not currently visible. Returning to ready wait."));
          if (!WaitForHakoReadyAsset(iniPath, hakoCmdPath, hakoCmdWorkDir)) {
            AppendHakoCmdLog(hakoCmdLogPath, _T("hako-cmd start sequence aborted: ready asset disappeared and did not become stable again."));
            return false;
          }
          if (startReadyGraceMs > 0) {
            CString graceMessage;
            graceMessage.Format(_T("hako-cmd start sequence: ready grace wait %lu ms after recheck."), startReadyGraceMs);
            AppendHakoCmdLog(hakoCmdLogPath, graceMessage);
            ::Sleep(startReadyGraceMs);
          }
        }
        else {
          AppendHakoCmdLog(hakoCmdLogPath, _T("hako-cmd start recheck: ready asset is still visible."));
        }
      }

      ::Sleep(retryIntervalMs);
      ++attempt;
    }

    AppendHakoCmdLog(hakoCmdLogPath, _T("hako-cmd start sequence failed: total timeout reached or all attempts returned non-zero exit code."));
    AfxMessageBox(_T("hako-cmd.exe start がタイムアウト時間内に成功しませんでした。\n詳細は hako-cmd ログを確認してください。"));
    return false;
  }
}


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

  CString currentPath = GetModuleDirectory();
  CString strFilePath = GetIniPath(currentPath);
  EnsureHakoCmdIniDefaults(strFilePath);

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

  if (!FileSizeMatches(szFilePath, fileSizeMB)) {
    std::string directory = std::string(CT2A(m_HakoScriptPath));
    std::string pycommand = "python make_zero_file.py \"" +
    std::string(CT2A(szFilePath)) + "\" " +
    std::to_string(fileSizeMB);

    std::string command =
      pycommand +
      " ; if ($LASTEXITCODE -ne 0) { "
      "Write-Host 'Error: Python execution failed with code' $LASTEXITCODE ; "
      "pause ; exit $LASTEXITCODE }";

    int result = runPowerShellCommand2(command, directory);

    if (result != 0) {
      AfxMessageBox(_T("Zero Fileの作成に失敗しました！"));
      return FALSE;
    }
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
  currentPath = GetModuleDirectory();
  CString strFilePath = GetIniPath(currentPath);

  // INIファイルに初期値のパス情報を書き出す
  WritePrivateProfileString(
    _T("hakoWinAppsAPI"),
    _T("HakoWinPath"),
    m_HakoWinPath,
    strFilePath
  );
  EnsureHakoCmdIniDefaults(strFilePath);
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
  currentPath = GetModuleDirectory();
  CString strFilePath = GetIniPath(currentPath);

  // INIファイルに初期値のパス情報を書き出す
  WritePrivateProfileString(
    _T("hakoWinAppsAPI"),
    _T("HakoAvatarApp"),
    m_HakoAvatarApps,
    strFilePath
  );
  EnsureHakoCmdIniDefaults(strFilePath);
}

void ChakoWinAppsDlg::OnBnClickedButton1()  
{  
  // TODO: ここにコントロール通知ハンドラー コードを追加します。  
  CString strFilePath = GetIniPath(GetModuleDirectory());
  CString mmapDir = ReadIniString(strFilePath, _T("mmapDir"), _T("Z:\\mmap"));
  CString cleanupScriptPath = m_HakoScriptPath;
  if (cleanupScriptPath.GetLength() > 0 && cleanupScriptPath[cleanupScriptPath.GetLength() - 1] != L'\\' && cleanupScriptPath[cleanupScriptPath.GetLength() - 1] != L'/') {
    cleanupScriptPath += L'\\';
  }
  cleanupScriptPath += _T("mmap_cleanup.py");

  auto psQuote = [](const CString& value) {
    CString escaped(value);
    escaped.Replace(_T("'"), _T("''"));
    return std::string("'") + std::string(CT2A(escaped)) + "'";
  };

  // Pythonプログラムのパスと引数を指定
  std::string directory = std::string(CT2A(m_HakoWinPath));

  // PowerShellコマンドを構築
  std::string command =
    "python " + psQuote(cleanupScriptPath) + " " + psQuote(mmapDir) +
    " ; if ($LASTEXITCODE -ne 0) { "
    "Write-Host 'Error: mmap cleanup failed with code' $LASTEXITCODE ; "
    "pause ; exit $LASTEXITCODE }"
    " ; .\\hako_drone_service.exe " + psQuote(m_HakoConfPath) + " " + psQuote(m_HakoAvatarConf);

  // PowerShellコマンドを実行
  runPowerShellCommand(command, directory);
}


void ChakoWinAppsDlg::OnBnClickedButton2()  
{  
 // TODO: ここにコントロール通知ハンドラー コードを追加します。  
 // 箱庭Unityアプリ起動ボタン  

 CString iniPath = GetIniPath(GetModuleDirectory());
 std::filesystem::path avatarDirectory(m_HakoAvatarApps.GetString());
 CString workDirectory(avatarDirectory.wstring().c_str());
 CString exePath((avatarDirectory / L"hakoAvatar.exe").wstring().c_str());
 DWORD avatarStartupWaitMs = static_cast<DWORD>(::GetPrivateProfileInt(kIniSection, _T("HakoAvatarStartupWaitMs"), 10000, iniPath));
 DWORD avatarPostStartupDelayMs = static_cast<DWORD>(::GetPrivateProfileInt(kIniSection, _T("HakoAvatarPostStartupDelayMs"), 0, iniPath));
 CString hakoCmdLogPath = GetHakoCmdLogPath(iniPath);

 AvatarProcessStartStatus avatarStartStatus = StartAvatarProcessAndObserveInputIdle(exePath, _T(""), workDirectory, avatarStartupWaitMs, hakoCmdLogPath);
 if (!IsAvatarProcessStartContinuable(avatarStartStatus))
 {
   if (avatarStartStatus == AvatarProcessStartStatus::CreateProcessFailed) {
     MessageBox(L"hakoAvatar.exe の起動に失敗しました。\n実行ファイルパスと作業ディレクトリを確認してください。", L"Error", MB_OK);
   }
   else if (avatarStartStatus == AvatarProcessStartStatus::ExitedBeforeReady) {
     MessageBox(L"hakoAvatar.exe が起動直後に終了しました。\nUnity Player のログまたは Windows イベントログを確認してください。", L"Error", MB_OK);
   }
   else {
     MessageBox(L"hakoAvatar.exe の起動状態確認に失敗しました。\n詳細は hako-cmd ログを確認してください。", L"Error", MB_OK);
   }
   return;
 }

 if (avatarPostStartupDelayMs > 0) {
   ::Sleep(avatarPostStartupDelayMs);
 }

 RunHakoStartCommandWithRetry(iniPath);

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
