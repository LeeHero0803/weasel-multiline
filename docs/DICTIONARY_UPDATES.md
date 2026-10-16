# 上游词库增量更新

当前基础词库来自 [iDvel/rime-ice](https://github.com/iDvel/rime-ice)，万象只提供所集成的 Lua 功能模块。

分别维护三层：

1. 小狼毫程序：本仓库的 upstream 指向 rime/weasel。合并新程序版本后重新构建。
2. 基础词库：单独获取 iDvel/rime-ice 的 Git 更新，替换其管理的词库文件。
3. 个人层：custom_phrase.txt、用户学习库、上下文库以及 custom.yaml，更新上游词库时保留。

## 更新操作

```powershell
git pull --ff-only origin multiline
.\tools\rime\update-dictionaries.ps1 -UserDir "$env:APPDATA\Rime"
# 检查变更清单后执行：
.\tools\rime\update-dictionaries.ps1 -UserDir "$env:APPDATA\Rime" -Apply
```

第一次下载上游仓库，之后使用 git fetch/pull 增量获取新提交。安装时替换发生变化的文件，保留上游删词和纠错结果。
每次更新前检查本地文件是否与上次同步记录一致；若本地直接修改过基础词库，会停止并提示先合并。
脚本备份待替换文件，只更新 cn_dicts、en_dicts、rime_ice.dict.yaml 和 melt_eng.dict.yaml。
不删除本地额外词库，也不更新 schema、Lua、OpenCC、皮肤、custom_phrase.txt 或用户数据库。

个人新增词尽量保存在自己的词库、短语或用户学习库中，不直接编辑上游文件。
运行脚本后在小狼毫菜单执行“重新部署”，编译新词库。若上游调整了 schema 或 Lua 接口，按上游说明另行评估方案升级。

## 同步新版本的小狼毫源码

```powershell
git fetch upstream --tags
git switch multiline
git merge --no-ff <需要升级的上游标签>
```

先处理合并冲突，完成编译与多行候选验证后再更新程序。这与词库更新分开进行。
