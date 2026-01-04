using HakonAppsInstaller.Helper;
using HakoIniFile.Helpers;
using IWshRuntimeLibrary;
using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Configuration.Install;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Security.Cryptography;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace CustomAction
{
    [RunInstaller(true)]
    public partial class Installer1 : System.Configuration.Install.Installer
    {
        public override void Install(System.Collections.IDictionary stateSaver)
        {
            // Install後の動作
            base.Install(stateSaver);

            // 環境変数PATHの追加
            string currentPath;
            currentPath = System.Environment.GetEnvironmentVariable("path", System.EnvironmentVariableTarget.User);
            string installPath = this.Context.Parameters["InstallPath"];
            string path = installPath + @"\hakoSim\bin;";
                        
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
                currentPath += ";"+path;
            }

#if DEBUG
            System.Windows.Forms.MessageBox.Show(currentPath);
            System.Windows.Forms.MessageBox.Show(path);
#endif
            // PYTHONPATHの設定
            string pythonPath;

            pythonPath = System.Environment.GetEnvironmentVariable("PYTHONPATH", System.EnvironmentVariableTarget.User);

            string[] subPaths = new[]
            {
              @"\hakoSim\bin\drone_api\rc;",
              @"\hakoSim\bin\drone_api\pymavlink;",
              @"\hakoSim\bin\drone_api\libs;",
              @"\hakoSim\bin\drone_api\mavsdk;"
            };

            foreach (string subPath in subPaths)
            {
                string fullPath = installPath + subPath;

#if DEBUG
        MessageBox.Show($"追加するPYTHONPATH: {fullPath}");
#endif
                SetUserEnvironmentPathVariable("PYTHONPATH", fullPath);

#if DEBUG
        string updated = Environment.GetEnvironmentVariable("PYTHONPATH", EnvironmentVariableTarget.User);
        MessageBox.Show($"更新後PYTHONPATH: {updated}");
#endif
            }

#if DEBUG
                MessageBox.Show($"InstallPath: {installPath}");
#endif
             // HakoWinAppsAPIのiniファイルの更新
             string iniPath = Path.Combine(installPath, @"hakoWinAppsAPI\hakoapi.ini");

#if DEBUG
    System.Windows.Forms.MessageBox.Show(iniPath);
#endif

             var updates = new Dictionary<string, string>
             {
              { "HakoWinPath", installPath + @"\hakosim\bin" },
              { "HakoAvatarApp", installPath + @"\hakoAvatar" },
              { "HakoPyPath", installPath + @"\hakoSim\bin\drone_api\rc" },
              { "HakoPyConf", installPath + @"\hakoSim\bin\config\pdudef\webavatar.json" },
              { "HakoWinScripts", installPath + @"\hakoWinScripts" },
             };

             IniUpdater.UpdateIniFile(iniPath, updates);

             // HakoWinAppsAPIのiniファイルの更新
             iniPath = Path.Combine(installPath, @"hakoWinAppsRC\hakorc.ini");

#if DEBUG
    System.Windows.Forms.MessageBox.Show(iniPath);
#endif

             updates = new Dictionary<string, string>
             {
              { "HakoWinPath", installPath + @"\hakosim\bin" },
              { "HakoAvatarApp", installPath + @"\hakoAvatar" },
              { "HakoRCPath", installPath + @"\hakoSim\bin\drone_api\rc" },
              { "HakoRCConf", installPath + @"\hakoSim\bin\drone_api\rc\rc_config" },
              { "HakoRCAvatar", installPath + @"\hakoSim\bin\config\pdudef\webavatar.json" },
              { "HakoWinScripts", installPath + @"\hakoWinScripts" },
             };

             // HakoWinAppsAPIのiniファイルの更新
             IniUpdater.UpdateIniFile(iniPath, updates);

            // インストール場所のフォルダを開くショートカットをデスクトップに作成
            string desktop = Environment.GetFolderPath(Environment.SpecialFolder.DesktopDirectory);
            string desktopFolder = System.IO.Path.Combine(desktop, "hakoApps-win");
            string shortcutLocation = System.IO.Path.Combine(desktopFolder, "インストールフォルダを開く.lnk");

            WshShell shell = new WshShell();
            IWshShortcut shortcut = (IWshShortcut)shell.CreateShortcut(shortcutLocation);
            shortcut.TargetPath = System.IO.Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.Windows), "explorer.exe");
            shortcut.Arguments = Context.Parameters["InstallPath"]; // インストール先フォルダを開く
            shortcut.Description = "インストールフォルダを開く";
            shortcut.Save();
    }
    private void SetUserEnvironmentPathVariable(string variableName, string newPath)
    {
      string currentValue = System.Environment.GetEnvironmentVariable(variableName, System.EnvironmentVariableTarget.User);

      if (string.IsNullOrEmpty(currentValue))
      {
        currentValue = newPath;
      }
      else if (currentValue.EndsWith(";"))
      {
        currentValue += newPath;
      }
      else
      {
        currentValue += ";" + newPath;
      }

      System.Environment.SetEnvironmentVariable(variableName, currentValue, System.EnvironmentVariableTarget.User);
    }

    private void UpdateIniFile(string iniPath, Dictionary<string, string> updates)
    {
      var lines = System.IO.File.ReadAllLines(iniPath);
      for (int i = 0; i < lines.Length; i++)
      {
        foreach (var kvp in updates)
        {
          if (lines[i].StartsWith(kvp.Key + "="))
          {
            lines[i] = kvp.Key + "=" + kvp.Value;
          }
        }
      }
      System.IO.File.WriteAllLines(iniPath, lines);
    }



    //public override void Commit(System.Collections.IDictionary savedState)
    //{
    //    //コミット動作
    //    base.Commit(savedState);
    //    System.Windows.Forms.MessageBox.Show(“Commit”);
    //}



    public override void Rollback(System.Collections.IDictionary savedState)
    {
        //失敗時のロールバック動作
        base.Rollback(savedState);

        // 環境変数、ショートカットのクリーンアップ
        string installPath = this.Context.Parameters["InstallPath"];
        HakoAppsCleanup.PerformCleanup(installPath);
    }

    public override void Uninstall(System.Collections.IDictionary savedState)
    {
        //Un-install動作
        base.Uninstall(savedState);


        // 環境変数、ショートカットのクリーンアップ
        string installPath = this.Context.Parameters["InstallPath"];
        HakoAppsCleanup.PerformCleanup(installPath);
    }
  }
}
