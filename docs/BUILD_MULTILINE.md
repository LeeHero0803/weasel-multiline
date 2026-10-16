# 构建与验证

保留上游的 INSTALL.md 和 build.bat。以下列出此定制版实际使用的环境：

- Windows、Visual Studio 2022 C++、v143、ATL/MFC。
- Windows SDK 10.0.26100.0。
- Boost 1.84.0，分别构建 x86/x64 的静态、多线程、静态运行库 Release 库。
- 官方 librime 1.13.1 的 x86/x64 开发包，提交 1c23358。

从 [librime 官方发布页](https://github.com/rime/librime/releases/tag/1.13.1) 获取开发包。
保存的原安装使用 x64 WeaselServer、x64 rime.dll 和 x64 WinSparkle.dll；客户端 DLL/IME 分别支持 x86/x64 应用。
服务端与这两个依赖的架构必须一致。

在 VS 2022 Developer PowerShell 中，先准备 Boost 及 librime 开发包，然后运行：

```powershell
.\tools\multiline\prepare-build.ps1 -BoostRoot D:\deps\boost_1_84_0 `
  -LibrimeX86 D:\deps\rime-x86\dist -LibrimeX64 D:\deps\rime-x64\dist

MSBuild.exe weasel.sln /m /t:WeaselServer,WeaselTSF,WeaselIME /p:Configuration=Release /p:Platform=x64 /p:WindowsTargetPlatformVersion=10.0.26100.0 /p:PlatformToolset=v143 /p:PLATFORM_TOOLSET=v143
MSBuild.exe weasel.sln /m /t:WeaselTSF,WeaselIME /p:Configuration=Release /p:Platform=Win32 /p:WindowsTargetPlatformVersion=10.0.26100.0 /p:PlatformToolset=v143 /p:PLATFORM_TOOLSET=v143

.\tools\multiline\test.ps1 -BoostRoot D:\deps\boost_1_84_0
```

测试脚本使用 x64 Developer PowerShell。`-Navigation` 另外检查正在运行的定制版服务；它创建自己的输入会话，不向应用输入文字。

程序输出：`output/WeaselServer.exe`、`weasel.dll`、`weaselx64.dll`、`weasel.ime`、`weaselx64.ime`。
保留原安装中的 Rime/WinSparkle 依赖。首次启用多行功能须同时更新服务端和对应客户端，保持原注册路径。
正在运行的应用持有旧 DLL 时，完全退出再重新打开应用。

## 代码来源

小狼毫源文件和上游历史保留 GPLv3 许可证。万象可选 Lua 模块固定于
`d814d6f7695e46973a7a9a2730d7727ddcbdd111`，原 CC BY 4.0 许可证与署名见 THIRD_PARTY_NOTICES.md。
