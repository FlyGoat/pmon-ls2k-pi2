# pmon-ls2k-pi2
PMON For Loongson2K Pi2

## Build Instructions

Step by step:

```
cd zloader
make board=ls2k cfg #Generate config files
make board=ls2k tgt=rom #Generate PMON binary
make board=ls2k dtb #Generate env and DeviceTree dtb
./fill.sh #Fill the remaining flash space
```
Then you'll get flash.bin
Have fun!
