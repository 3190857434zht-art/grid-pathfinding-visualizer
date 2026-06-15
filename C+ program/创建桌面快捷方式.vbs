Set WshShell = CreateObject("WScript.Shell")
Set FSO = CreateObject("Scripting.FileSystemObject")

projectDir = FSO.GetParentFolderName(WScript.ScriptFullName)
batPath = projectDir & "\一键启动.bat"
desktop = WshShell.SpecialFolders("Desktop")
linkPath = desktop & "\路径规划可视化.lnk"

Set shortcut = WshShell.CreateShortcut(linkPath)
shortcut.TargetPath = batPath
shortcut.WorkingDirectory = projectDir
shortcut.WindowStyle = 1
shortcut.Description = "多策略路径规划可视化系统 - 一键启动"
shortcut.IconLocation = "shell32.dll,13"
shortcut.Save

MsgBox "已在桌面创建快捷方式：" & vbCrLf & "路径规划可视化.lnk" & vbCrLf & vbCrLf & "双击即可启动，无需打开代码编辑器。", vbInformation, "完成"
