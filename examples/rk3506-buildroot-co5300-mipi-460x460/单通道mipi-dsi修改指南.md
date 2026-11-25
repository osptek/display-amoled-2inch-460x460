# 给mipi-dsi打补丁


> 如果你在 SDK 根目录（当前目录下能看到 kernel-6.1/ 文件夹）：
```
# 先测试补丁能否干净打上（不会修改文件）
git apply --check 0001-drm-rockchip-dw-mipi-dsi-derive-rk3506-lane-enables.patch

# 确认无报错后正式应用
git apply 0001-drm-rockchip-dw-mipi-dsi-derive-rk3506-lane-enables.patch
```

# 添加设备树

```
cd /home/jin/rk3506-work/rk3506_linux6.1_sdk_v1.2.0_evb/kernel-6.1/arch/arm/boot/dts
```

```
nano vanxoak-hd-rk3506-evb-mipi-460x460.dtsi
```

```
# 返回 SDK 根目录
cd /home/jin/rk3506-work/rk3506_linux6.1_sdk_v1.2.0_evb

# 全部编译镜像
./build.sh
```

```
# Loader 模式固件更新
cd /home/jin/rk3506-work/rk3506_linux6.1_sdk_v1.2.0_evb/output/firmware/

# 烧录完整固件并自动重启设备
sudo upgrade_tool uf update.img
```

# 查看屏幕设备节点

```
# 查看 DRM / KMS 设备节点（核心图形节点）

ls -l /dev/dri/
```

**输出解读**：

- **`/dev/dri/card0`**：DRM 主设备节点，LVGL、Wayland 或 Modetest 都会通过打开这个节点来进行硬件渲染和图像输出。
- **`/dev/dri/renderD128`**：DRM 渲染节点（如果 RK3506 内核启用了 GPU / 2D 硬件加速支持）。


# 显示测试

彩条测试指令
```
#竖条测试
modetest -M rockchip -s 74@71:460x460

#斜条测试
modetest -M rockchip -s 74@71:460x460 -Ftiles
```