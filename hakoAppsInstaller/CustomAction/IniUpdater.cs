using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using System.Windows.Forms;

namespace HakoIniFile.Helpers
{
  public static class IniUpdater
  {
    /// <summary>
    /// 指定した INI ファイルにキーと値を更新する
    /// </summary>
    /// <param name="iniPath">INI ファイルのフルパス</param>
    /// <param name="updates">更新するキーと値の辞書</param>
    public static void UpdateIniFile(string iniPath, Dictionary<string, string> updates)
    {
      if (!File.Exists(iniPath))
      {
#if DEBUG
                MessageBox.Show($"INI ファイルが存在しません: {iniPath}");
#endif
        return;
      }

      var lines = new List<string>(File.ReadAllLines(iniPath, Encoding.UTF8));
      var updatedKeys = new HashSet<string>();

      for (int i = 0; i < lines.Count; i++)
      {
        foreach (var kvp in updates)
        {
          if (lines[i].StartsWith(kvp.Key + "=", StringComparison.OrdinalIgnoreCase))
          {
            lines[i] = kvp.Key + "=" + kvp.Value;
            updatedKeys.Add(kvp.Key);
#if DEBUG
                        MessageBox.Show($"更新: {kvp.Key}={kvp.Value}");
#endif
          }
        }
      }

      // 存在しなかったキーは末尾に追加
      foreach (var kvp in updates)
      {
        if (!updatedKeys.Contains(kvp.Key))
        {
          lines.Add(kvp.Key + "=" + kvp.Value);
#if DEBUG
                    MessageBox.Show($"追加: {kvp.Key}={kvp.Value}");
#endif
        }
      }

      // BOM を出力しない UTF-8 で保存
      File.WriteAllLines(iniPath, lines, new UTF8Encoding(encoderShouldEmitUTF8Identifier: false));
    }
  }
}
