Set WshShell = CreateObject("WScript.Shell")
Set FSO = CreateObject("Scripting.FileSystemObject")

' 脚本所在目录 = 项目根目录
projectDir = FSO.GetParentFolderName(WScript.ScriptFullName)
batPath = projectDir & "\一键启动.bat"

If Not FSO.FileExists(batPath) Then
    MsgBox "未找到 一键启动.bat" & vbCrLf & batPath, vbCritical, "路径规划 - 启动失败"
    WScript.Quit 1
End If

' 在项目目录下启动，保留一个控制台窗口显示状态
WshShell.CurrentDirectory = projectDir
WshShell.Run """" & batPath & """", 1, False
