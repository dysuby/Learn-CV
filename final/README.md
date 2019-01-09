# A4 纸手写数字识别



## 文件结构

- `test`，初始数据集
  - `good samples`
  - `others`
  - `bad samples`
  - `identity first`，先写身份证在写手机号码
  - `fail`，失败品
  - `labels.CSV`，打好的 label
- `tmp`，矫正后的 A4 纸图像
  - `good samples`
  - `others`
  - `bad samples`
  - `identity first`，先写身份证在写手机号码
- `digits`，分割后的数字，每张图片对应一个文件夹
  - `good samples`
  - `others`
  - `bad samples`
  - `identity first`，先写身份证在写手机号码
- `adaboost`，Adaboost 模块
  - `model`，训练后的模型
  - `test`，MNIST 测试集
  - `train`，MNIST 训练集
- `res`，最终成品 xlsx
  - `good samples.xlsx`
  - `bad samples.xlsx`
  - `others.xlsx`
  - `identity first.xlsx`
  - `all.xlxs`，所有识别结果的汇总
- `*.cpp, *.hpp`，分割数字代码
- `classify.py`，识别数字代码



## 使用说明

`g++ *.cpp -lgdi32 -O2`

运行了 exe 之后，再

`py classify.py`



默认识别 good samples 文件夹，如果需要更改，请将 `main.cpp` 中的宏 `SRC`，`TMP`，`SAVE`，以及 `classify.py` 中的 `TARGET` 改成对应的文件夹，和下面的  `open('res/good samples.csv')` 改成需要的输出文件名。

如果需要识别 `identity first`，请将 `classify.py` 下注释为 `# 身份证先` 的代码启用，并将另外的 `# 正常` 注释掉。