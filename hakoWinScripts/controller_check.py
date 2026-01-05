import sys
import pygame
# pygame を初期化
pygame.init()
pygame.joystick.init()
# 接続されているジョイスティックの数を取得
joystick_count = pygame.joystick.get_count()
if joystick_count == 0:
    print("ゲームコントローラが接続されていません！！")
    print("ゲームコントローラを接続して、コントローラ起動をやりなおしてください！！")
    sys.exit(1)  # エラーコード
else:
    print(f"{joystick_count} 台のコントローラが接続されています。")
    # 1台目のコントローラ情報を表示
    joystick = pygame.joystick.Joystick(0)
    joystick.init()
    print("コントローラ名:", joystick.get_name())
    print("ボタン数:", joystick.get_numbuttons())





