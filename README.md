# ESP32 DLNA Render
## 描述
将ESP32作为一个Render，可以通过DLNA播放音乐
## 硬件说明
- 普通的ESP32，无PSRAM
- PCM1502A
## 实现的功能
- 通过网易云投放音乐到ESP32，获取到播放链接
- 通过网易云控制音乐播放、暂停
- 通过网易云调整音量大小
- 通过网易云切换歌曲
## 存在的问题
- 初次播放音乐时，进度条不能自动更新
- 调整音量大小时，手机上不同步显示
- 调整音量大小时，播放会出现卡顿
- 网易云从后台重新调出来时，播放会出现卡顿
- 不能调整进度
- 不能自动播放下一首
## TODO
- 分析并尽量解决上面存在的问题
- 实现event部分的代码（如subscribe，unsubscribe）
## 软件说明  
在VScode+PlatformIO环境下开发，使用Arduino框架编写  
用到的库：
- ESP32SSDP
- ESP32-audioI2S