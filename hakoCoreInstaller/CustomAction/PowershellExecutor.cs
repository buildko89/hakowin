using System;
using System.Diagnostics;
using System.Windows.Forms;

namespace hakoCoreInstaller.Helpers
{
  public static class PowerShellExecutor
  {
    public static void RunCommand(string command)
    {
      try
      {
        var psi = new ProcessStartInfo
        {
          FileName = "powershell.exe",
          Arguments = $"-NoProfile -ExecutionPolicy Bypass -Command \"{command}\"",
          UseShellExecute = false,
          RedirectStandardOutput = true,
          RedirectStandardError = true,
          CreateNoWindow = true
        };

        // ユーザー環境の PATH を継承
        psi.EnvironmentVariables["PATH"] = Environment.GetEnvironmentVariable("PATH", EnvironmentVariableTarget.User);

        using (var process = Process.Start(psi))
        {
          string output = process.StandardOutput.ReadToEnd();
          string error = process.StandardError.ReadToEnd();
          process.WaitForExit();

          if (!string.IsNullOrEmpty(error))
          {
            MessageBox.Show("PowerShell エラー:\n" + error, "エラー", MessageBoxButtons.OK, MessageBoxIcon.Error);
          }
          else
          {
            MessageBox.Show("PowerShell 成功:\n" + output, "成功", MessageBoxButtons.OK, MessageBoxIcon.Information);
          }
        }
      }
      catch (Exception ex)
      {
        MessageBox.Show("PowerShell 実行例外:\n" + ex.Message, "例外", MessageBoxButtons.OK, MessageBoxIcon.Error);
      }
    }

    public static bool IsPipAvailable()
    {
      try
      {
        var psi = new ProcessStartInfo
        {
          FileName = "powershell.exe",
          Arguments = "-NoProfile -ExecutionPolicy Bypass -Command \"python -m pip --version\"",
          UseShellExecute = false,
          RedirectStandardOutput = true,
          RedirectStandardError = true,
          CreateNoWindow = true
        };

        // 現在のプロセス PATH とユーザー PATH をマージしてプロセスに渡す
        string processPath = Environment.GetEnvironmentVariable("PATH") ?? string.Empty;
        string userPath = Environment.GetEnvironmentVariable("PATH", EnvironmentVariableTarget.User) ?? string.Empty;

        if (!string.IsNullOrEmpty(userPath))
        {
          // 重複を気にせずシンプルに結合（必要なら重複除去ロジックを追加）
          if (!processPath.EndsWith(";"))
            processPath += ";";
          processPath += userPath;
        }

        psi.EnvironmentVariables["PATH"] = processPath;

        using (var process = Process.Start(psi))
        {
          string output = process.StandardOutput.ReadToEnd();
          string error = process.StandardError.ReadToEnd();
          process.WaitForExit();

          // 終了コードが0で、stdoutに何か返っており、stderrが空であれば成功とみなす
          return process.ExitCode == 0
                 && !string.IsNullOrWhiteSpace(output)
                 && string.IsNullOrWhiteSpace(error);
        }
      }
      catch
      {
        return false;
      }
    }
  }
}
