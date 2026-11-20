# 固件并包bin文件

```
/home/jin/.espressif/tools/python/v6.1/venv/bin/python -m esptool --chip esp32p4 merge-bin \
  --output esp32p4-idf6_co5300-mipi-460x460_osptek-boot-anim_lvgl9_tear-resistant_0x0.bin \
  --flash-mode dio \
  --flash-freq 80m \
  --flash-size 16MB \
  0x2000 bootloader/bootloader.bin \
  0x8000 partition_table/partition-table.bin \
  0x10000 esp32p4-idf6_co5300-mipi-460x460_osptek-boot-anim_lvgl9_tear-resistant.bin \
  0x810000 mmap_build/assets/efc/efc.bin
```
