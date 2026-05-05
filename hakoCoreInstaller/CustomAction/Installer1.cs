using System;
using System.Collections;
using System.ComponentModel;
using System.Configuration.Install;
using System.IO;
using hakoCoreInstaller.Helpers;
using IWshRuntimeLibrary;

namespace CustomAction
{
  [RunInstaller(true)]
  public partial class Installer1 : System.Configuration.Install.Installer
  {
    // インストール前の動作
    protected override void OnBeforeInstall(IDictionary savedState)
    {
      base.OnBeforeInstall(savedState);
    }

    // インストール時の動作関数
    public override void Install(System.Collections.IDictionary stateSaver)
    {
      // Install後の動作
      base.Install(stateSaver);

      // 環境変数PATHの追加
      string currentPath;
      currentPath = System.Environment.GetEnvironmentVariable("path", System.EnvironmentVariableTarget.User);
      string installPath = GetInstallPath(true);
      string path = Path.Combine(installPath, "bin") + ";";

      // Python環境の確認とhakoniwa-pduのインストール
      if (PowerShellExecutor.IsPipAvailable())
      {
        PowerShellExecutor.RunCommand("python -m pip install hakoniwa-pdu");
      }
      else
      {
        // 例外をスローしてインストールを失敗させる
        throw new InstallException("pythonまたはpipがインストールされていません。Python環境を確認してください！！");
      }


#if DEBUG
      System.Windows.Forms.MessageBox.Show(installPath);
#endif

      if (currentPath == null)
      {
        currentPath = path;
      }
      else if (currentPath.EndsWith(";"))
      {
        currentPath += path;
      }
      else
      {
        currentPath += ";" + path;
      }

      // 環境変数PATHを設定する
      System.Environment.SetEnvironmentVariable("path", currentPath, System.EnvironmentVariableTarget.User);

      //hakoniwa core config pathの設定
      string configpath = Path.Combine(installPath, "config", "cpp_core_config.json");
      System.Environment.SetEnvironmentVariable("HAKO_CONFIG_PATH", configpath, System.EnvironmentVariableTarget.User);

      //hakoniwa core Library pathの設定
      string libpath = Path.Combine(installPath, "lib");
      System.Environment.SetEnvironmentVariable("HAKOCORE_LIB_PATH", libpath, System.EnvironmentVariableTarget.User);

      //hakoniwa core Python pathの設定
      string pythonPath;

      pythonPath = System.Environment.GetEnvironmentVariable("PYTHONPATH", System.EnvironmentVariableTarget.User);
      string hakopypath = Path.Combine(installPath, "lib", "py") + ";";
#if DEBUG
      System.Windows.Forms.MessageBox.Show(hakopypath);
#endif

      if (pythonPath == null)
      {
        pythonPath = hakopypath;
      }
      else if (pythonPath.EndsWith(";"))
      {
        pythonPath += hakopypath;
      }
      else
      {
        pythonPath += ";" + hakopypath;
      }

      // 環境変数PYTHONPATHを設定する
      System.Environment.SetEnvironmentVariable("PYTHONPATH", pythonPath, System.EnvironmentVariableTarget.User);

      CreateDesktopContents(installPath);

#if DEBUG
      System.Windows.Forms.MessageBox.Show("Install End!!");
#endif
    }

    // インストールの状態を変更する動作関数
    public override void Commit(System.Collections.IDictionary savedState)
    {
      //変更時の動作
      base.Commit(savedState);
#if DEBUG
      System.Windows.Forms.MessageBox.Show("Commit");
#endif
    }

    // インストール失敗時の修復動作関数
    public override void Rollback(System.Collections.IDictionary savedState)
    {
      //修復動作
      base.Rollback(savedState);

      string installPath = GetInstallPath(false);
      if (!string.IsNullOrWhiteSpace(installPath))
      {
        hakoCoreEnvCleanup.RemoveHakoniwaEnvironmentVariables(installPath);
      }
      hakoCoreEnvCleanup.RemoveDesktopExamples();
      hakoCoreEnvCleanup.RemoveShortcut();

#if DEBUG
      System.Windows.Forms.MessageBox.Show("Rollback");
#endif
    }

    // アンインストール時の動作関数
    public override void Uninstall(System.Collections.IDictionary savedState)
    {
      //Un-install動作
      base.Uninstall(savedState);

      // 環境変数PATHを編集
      string installPath = GetInstallPath(false);
      if (!string.IsNullOrWhiteSpace(installPath))
      {
        hakoCoreEnvCleanup.RemoveHakoniwaEnvironmentVariables(installPath);
      }
      hakoCoreEnvCleanup.RemoveDesktopExamples();
      hakoCoreEnvCleanup.RemoveShortcut();

#if DEBUG
      System.Windows.Forms.MessageBox.Show("Uninstall");
#endif
    }

    private void CreateDesktopContents(string installPath)
    {
      try
      {
        // examplesフォルダをデスクトップのhakoCore-winフォルダへコピー
        ExamplesCopier.CopyExamplesToDesktop(installPath);

        // インストール場所のフォルダを開くショートカットをデスクトップに作成
        string desktop = Environment.GetFolderPath(Environment.SpecialFolder.DesktopDirectory);
        string desktopFolder = Path.Combine(desktop, "hakoCore-win");
        Directory.CreateDirectory(desktopFolder);
        string shortcutLocation = Path.Combine(desktopFolder, "インストールフォルダを開く.lnk");

        WshShell shell = new WshShell();
        IWshShortcut shortcut = (IWshShortcut)shell.CreateShortcut(shortcutLocation);
        shortcut.TargetPath = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.Windows), "explorer.exe");
        shortcut.Arguments = QuotePath(installPath); // インストール先フォルダを開く
        shortcut.Description = "インストールフォルダを開く";
        shortcut.Save();
      }
      catch (Exception ex)
      {
        WriteInstallLog("デスクトップへのexamplesコピーまたはショートカット作成に失敗しました。", ex);
      }
    }

    private string GetInstallPath(bool throwIfMissing)
    {
      string installPath = GetParameter("installpath");
      if (!string.IsNullOrWhiteSpace(installPath))
      {
        return TrimTrailingDirectorySeparator(installPath);
      }

      string targetDir = GetParameter("targetdir");
      if (!string.IsNullOrWhiteSpace(targetDir))
      {
        return TrimTrailingDirectorySeparator(targetDir);
      }

      string assemblyPath = GetParameter("assemblypath");
      if (!string.IsNullOrWhiteSpace(assemblyPath))
      {
        string assemblyDirectory = Path.GetDirectoryName(assemblyPath);
        if (!string.IsNullOrWhiteSpace(assemblyDirectory))
        {
          return TrimTrailingDirectorySeparator(assemblyDirectory);
        }
      }

      if (!throwIfMissing)
      {
        return string.Empty;
      }

      throw new InstallException("インストール先フォルダを取得できませんでした。");
    }

    private string GetParameter(string name)
    {
      if (this.Context == null || this.Context.Parameters == null)
      {
        return string.Empty;
      }

      return this.Context.Parameters[name] ?? string.Empty;
    }

    private static string TrimTrailingDirectorySeparator(string path)
    {
      return path.TrimEnd(Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar);
    }

    private static string QuotePath(string path)
    {
      return "\"" + path + "\"";
    }

    private static void WriteInstallLog(string message, Exception ex)
    {
      try
      {
        string logPath = Path.Combine(Path.GetTempPath(), "hakoCore-win-install.log");
        System.IO.File.AppendAllText(logPath, DateTime.Now + " " + message + Environment.NewLine + ex + Environment.NewLine);
      }
      catch
      {
      }
    }
  }
}
