using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Configuration.Install;
using System.Linq;
using System.Runtime.Remoting.Metadata.W3cXsd2001;
using System.Threading.Tasks;
using System.Windows.Forms;
using hakoCoreInstaller.Helpers;


namespace CustomAction
{
  [RunInstaller(true)]
  public partial class Installer1 : System.Configuration.Install.Installer
  {
    // インストール時の動作関数
    public override void Install(System.Collections.IDictionary stateSaver)
    {
      // Install後の動作
      base.Install(stateSaver);

      // 環境変数PATHの追加
      string currentPath;
      currentPath = System.Environment.GetEnvironmentVariable("path", System.EnvironmentVariableTarget.User);
      string installPath = this.Context.Parameters["InstallPath"];
      string path = installPath + @"\bin;";

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
      string configpath = installPath + @"\config\cpp_core_config.json";
      System.Environment.SetEnvironmentVariable("HAKO_CONFIG_PATH", configpath, System.EnvironmentVariableTarget.User);

      //hakoniwa core Library pathの設定
      string libpath = installPath + @"\lib";
      System.Environment.SetEnvironmentVariable("HAKOCORE_LIB_PATH", libpath, System.EnvironmentVariableTarget.User);

      //hakoniwa core Python pathの設定
      string pythonPath;

      pythonPath = System.Environment.GetEnvironmentVariable("PYTHONPATH", System.EnvironmentVariableTarget.User);
      string hakopypath = installPath + @"\lib\py;";
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

#if DEBUG
      System.Windows.Forms.MessageBox.Show("Install");
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

      string installPath = this.Context.Parameters["InstallPath"];
      hakoCoreEnvCleanup.RemoveHakoniwaEnvironmentVariables(installPath);



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
      string installPath = this.Context.Parameters["InstallPath"];
      hakoCoreEnvCleanup.RemoveHakoniwaEnvironmentVariables(installPath);

#if DEBUG
      System.Windows.Forms.MessageBox.Show("Uninstall");
#endif
    }
  }
}
