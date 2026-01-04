using System;
using System.Diagnostics;
using System.IO;
using System.Windows.Forms;

namespace HakonAppsInstaller.Helper
{
  // アンインストール時のクリーンアップ処理をまとめたクラス
  public static class HakoAppsCleanup
  {
    public static void RemovePaths(string installPath)
    {
      // PATH から削除
      string currentPath = Environment.GetEnvironmentVariable("path", EnvironmentVariableTarget.User);
      string pathToRemove = installPath + @"\hakoSim\bin;";
      currentPath = currentPath?.Replace(pathToRemove, "");
      Environment.SetEnvironmentVariable("path", currentPath, EnvironmentVariableTarget.User);

#if DEBUG
        MessageBox.Show($"PATH削除: {pathToRemove}\n結果: {currentPath}");
#endif

      // PYTHONPATH から複数パスを削除
      string[] pythonSubPaths = new[]
      {
            @"\hakoSim\bin\drone_api\rc;",
            @"\hakoSim\bin\drone_api\pymavlink;",
            @"\hakoSim\bin\drone_api\libs;",
            @"\hakoSim\bin\drone_api\mavsdk;"
        };

      string pythonPath = Environment.GetEnvironmentVariable("PYTHONPATH", EnvironmentVariableTarget.User);

      foreach (string subPath in pythonSubPaths)
      {
        string fullPath = installPath + subPath;
        pythonPath = pythonPath?.Replace(fullPath, "");

#if DEBUG
          MessageBox.Show($"PYTHONPATH削除: {fullPath}");
#endif
      }

      Environment.SetEnvironmentVariable("PYTHONPATH", pythonPath, EnvironmentVariableTarget.User);

#if DEBUG
        MessageBox.Show($"PYTHONPATH結果: {pythonPath}");
#endif
    }

    public static void RemoveShortcut()
    {
      string desktop = Environment.GetFolderPath(Environment.SpecialFolder.DesktopDirectory);
      string shortcutPath = Path.Combine(desktop, "hakoApps-win", "インストールフォルダを開く.lnk");

      if (System.IO.File.Exists(shortcutPath))
      {
        try
        {
          System.IO.File.Delete(shortcutPath);
        }
        catch (Exception ex)
        {
          Debug.WriteLine("ショートカット削除失敗: " + ex.Message);
        }
      }
    }

    // 総合的なクリーンアップメソッド
    public static void PerformCleanup(string installPath)
    {
      RemovePaths(installPath);
      RemoveShortcut();
    }
  }
}
