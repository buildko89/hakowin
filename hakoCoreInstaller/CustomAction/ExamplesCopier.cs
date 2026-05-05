using System;
using System.IO;

namespace hakoCoreInstaller.Helpers
{
  public static class ExamplesCopier
  {
    public static void CopyExamplesToDesktop(string installPath)
    {
      if (string.IsNullOrWhiteSpace(installPath))
      {
        return;
      }

      string source = Path.Combine(installPath, "examples");
      if (!Directory.Exists(source))
      {
        return;
      }

      string desktop = Environment.GetFolderPath(Environment.SpecialFolder.DesktopDirectory);
      string destination = Path.Combine(desktop, "hakoCore-win", "examples");

      CopyDirectory(source, destination);
    }

    private static void CopyDirectory(string sourceDir, string destinationDir)
    {
      Directory.CreateDirectory(destinationDir);

      foreach (string file in Directory.GetFiles(sourceDir))
      {
        string destinationFile = Path.Combine(destinationDir, Path.GetFileName(file));
        File.Copy(file, destinationFile, true);
      }

      foreach (string directory in Directory.GetDirectories(sourceDir))
      {
        string destinationSubDir = Path.Combine(destinationDir, Path.GetFileName(directory));
        CopyDirectory(directory, destinationSubDir);
      }
    }
  }
}
