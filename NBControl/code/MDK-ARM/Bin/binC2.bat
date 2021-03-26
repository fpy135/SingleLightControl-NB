
copy Output\SmartBox.hex .\Bin\SmartBox.hex
copy "..\..\SmartBox_01B - boot\MDK-ARM\Output\SmartBox.hex" ".\Bin\SmartBox-Boot.hex"

::功能:hex转bin 生成远程升级固件
srec_cat .\Bin\SmartBox.hex -Intel -offset -0x8020000 -o .\Bin\SmartBox.bin -Binary
srec_cat .\Bin\SmartBox-Boot.hex -Intel -offset -0x8000000 -o .\Bin\SmartBox-Boot.bin -Binary

::功能:合并bin 生成离线烧写固件
srec_cat .\Bin\SmartBox-Boot.bin -binary -fill 0xFF 0x0000 0x20000 .\Bin\SmartBox.bin -binary -offset 0x20000 -o .\Bin\SmartBox-Merge.bin -Binary
::srec_cat Bin\SmartBox.bin -binary -offset 0x20000 -o Bin\SmartBox-Merge.bin -Binary

::功能:合并hex 用于KEIL下载
srec_cat Bin\SmartBox-Boot.hex -Intel Bin\SmartBox.hex -Intel -o Bin\SmartBox-Merge.hex -Intel

copy Bin\SmartBox-Merge.hex .\output\SmartBox-Merge.hex

exit
