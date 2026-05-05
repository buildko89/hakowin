using System;
using System.Diagnostics;
using System.IO;
using System.Security.Principal;
using System.Text;
using System.Windows.Forms;

namespace SetupLauncher
{
  internal static class Program
  {
    private const string MsiFileName = "hakocore-win.msi";
    private const string AdministratorRequiredMessage =
      "管理者権限で実行してください。\n\nこのインストーラーの実行には、右クリックして「管理者として実行」を選択する必要があります。";

    [STAThread]
    private static int Main(string[] args)
    {
      Application.EnableVisualStyles();
      Application.SetCompatibleTextRenderingDefault(false);

      if (!IsRunningAsAdministrator())
      {
        MessageBox.Show(
          AdministratorRequiredMessage,
          "管理者権限が必要です",
          MessageBoxButtons.OK,
          MessageBoxIcon.Warning);
        return 1;
      }

      string executableDirectory = AppDomain.CurrentDomain.BaseDirectory;
      string msiPath = Path.Combine(executableDirectory, MsiFileName);
      if (!File.Exists(msiPath))
      {
        MessageBox.Show(
          MsiFileName + " が見つかりません。\n\nsetup.exe と同じフォルダーに " + MsiFileName + " を配置してください。",
          "インストーラーが見つかりません",
          MessageBoxButtons.OK,
          MessageBoxIcon.Error);
        return 2;
      }

      try
      {
        using (Process process = Process.Start(new ProcessStartInfo
        {
          FileName = "msiexec.exe",
          Arguments = "/i " + Quote(msiPath) + BuildForwardedArguments(args),
          UseShellExecute = false,
        }))
        {
          process.WaitForExit();
          return process.ExitCode;
        }
      }
      catch (Exception ex)
      {
        MessageBox.Show(
          "インストーラーを起動できませんでした。\n\n" + ex.Message,
          "起動エラー",
          MessageBoxButtons.OK,
          MessageBoxIcon.Error);
        return 3;
      }
    }

    private static bool IsRunningAsAdministrator()
    {
      WindowsIdentity identity = WindowsIdentity.GetCurrent();
      WindowsPrincipal principal = new WindowsPrincipal(identity);
      return principal.IsInRole(WindowsBuiltInRole.Administrator);
    }

    private static string BuildForwardedArguments(string[] args)
    {
      if (args == null || args.Length == 0)
      {
        return string.Empty;
      }

      StringBuilder builder = new StringBuilder();
      foreach (string arg in args)
      {
        builder.Append(' ');
        builder.Append(Quote(arg));
      }
      return builder.ToString();
    }

    private static string Quote(string value)
    {
      if (string.IsNullOrEmpty(value))
      {
        return "\"\"";
      }

      StringBuilder builder = new StringBuilder();
      builder.Append('"');

      int backslashCount = 0;
      foreach (char current in value)
      {
        if (current == '\\')
        {
          backslashCount++;
          continue;
        }

        if (current == '"')
        {
          builder.Append('\\', backslashCount * 2 + 1);
          builder.Append('"');
        }
        else
        {
          builder.Append('\\', backslashCount);
          builder.Append(current);
        }

        backslashCount = 0;
      }

      builder.Append('\\', backslashCount * 2);
      builder.Append('"');
      return builder.ToString();
    }
  }
}
