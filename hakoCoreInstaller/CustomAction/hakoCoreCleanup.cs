using System;
using System.Diagnostics;
using System.IO;
using System.Windows.Forms;

namespace hakoCoreInstaller.Helpers
{
  public static class hakoCoreEnvCleanup
  {
    public static void RemoveHakoniwaEnvironmentVariables(string installPath)
    {
      string pathVar = Environment.GetEnvironmentVariable("path", EnvironmentVariableTarget.User);
      string binPath = installPath + @"\bin;";
      if (pathVar != null && pathVar.Contains(binPath))
      {
        pathVar = pathVar.Replace(binPath, "");
        Environment.SetEnvironmentVariable("path", pathVar, EnvironmentVariableTarget.User);

#if DEBUG
        MessageBox.Show($"PATHから削除: {binPath}\n結果: {pathVar}");
#endif
      }

      Environment.SetEnvironmentVariable("HAKOCORE_LIB_PATH", "", EnvironmentVariableTarget.User);
      Environment.SetEnvironmentVariable("HAKO_CONFIG_PATH", "", EnvironmentVariableTarget.User);

#if DEBUG
      MessageBox.Show("HAKOCORE_LIB_PATH / HAKO_CONFIG_PATH を削除");
#endif

      string pythonPath = Environment.GetEnvironmentVariable("PYTHONPATH", EnvironmentVariableTarget.User);
      string hakopyPath = installPath + @"\lib\py;";
      if (pythonPath != null && pythonPath.Contains(hakopyPath))
      {
        pythonPath = pythonPath.Replace(hakopyPath, "");
        Environment.SetEnvironmentVariable("PYTHONPATH", pythonPath, EnvironmentVariableTarget.User);

#if DEBUG
        MessageBox.Show($"PYTHONPATHから削除: {hakopyPath}\n結果: {pythonPath}");
#endif
      }
    }

    public static void RemoveDesktopExamples()
    {
      string desktop = Environment.GetFolderPath(Environment.SpecialFolder.DesktopDirectory);
      string examplesPath = Path.Combine(desktop, "hakoCore-win", "examples");

      if (Directory.Exists(examplesPath))
      {
        try
        {
          Directory.Delete(examplesPath, true);
        }
        catch (Exception ex)
        {
          Debug.WriteLine("examplesフォルダ削除失敗: " + ex.Message);
        }
      }
    }

    public static void RemoveShortcut()
    {
      RemoveShortcutFromDesktopFolder("hakoCore-win");
      RemoveShortcutFromDesktopFolder("hakoApps-win");
    }

    private static void RemoveShortcutFromDesktopFolder(string folderName)
    {
      string desktop = Environment.GetFolderPath(Environment.SpecialFolder.DesktopDirectory);
      string shortcutPath = Path.Combine(desktop, folderName, "hakoCore-winフォルダを開く.lnk");

      if (File.Exists(shortcutPath))
      {
        try
        {
          File.Delete(shortcutPath);
        }
        catch (Exception ex)
        {
          Debug.WriteLine("ショートカット削除失敗: " + ex.Message);
        }
      }
    }
  }
}
