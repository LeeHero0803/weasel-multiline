# Rime 配置与基础词库

此目录用于雾凇拼音方案。小狼毫程序和 Rime 输入方案分别维护。

- `rime_ice.custom.yaml`：万象上下文调频、默认量词排序和回头码；不挂接 predictor。
- `lua/wanxiang/`：三个固定版本的万象 Lua 文件及原始许可证。
- `weasel.custom.yaml`：仅设置矩形窗口和直角高亮，沿用用户已有字体与配色。
- `dictionaries/`：雾凇基础词库的 cn_dicts、en_dicts 和主词典文本。

将 Lua 文件放到用户目录的 `lua/wanxiang`。已有 custom.yaml 时合并 patch 项，
不直接覆盖自己的配置。基础雾凇方案仍使用原来的 schema、Lua 和 OpenCC 文件。
词典快照可按原相对目录恢复。

个人词库、自定义短语及上下文学习记录不在此公开仓库中。
基础词库增量更新保留本地个人数据，见 [词库增量更新说明](../docs/DICTIONARY_UPDATES.md)。
