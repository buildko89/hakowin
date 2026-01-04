using System;
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
  }
}
