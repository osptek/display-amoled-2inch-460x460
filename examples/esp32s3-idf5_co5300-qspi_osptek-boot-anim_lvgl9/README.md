# 生成efc文件的python指令

> 将普通的 GIF 或视频转为 .efc 文件

```
python pc/video_to_efc.py input.gif output.efc
```

> 使用调色板量化（推荐，文件更小、画质稳定）
通过 --via-gif 参数进行 GIF 风格的全局调色板量化再进行 EFC 编码，非常适合用于表情或 UI 动画：

```
python pc/video_to_efc.py input.mp4 output.efc --via-gif
```

> 无损转换
跳过所有去噪、调色板缩减等处理，完全保留源像素：

```
python pc/video_to_efc.py input.mp4 output.efc --lossless
```

> 限制目标文件大小（按需压缩）

```
python pc/video_to_efc.py input.mp4 output.efc --size 300K
```

> 针对复杂源进行 H.264 预处理平滑（减小体积）

```
python pc/video_to_efc.py input.gif output.efc --h264-crf 18 --cmp /tmp/cmp.png
```