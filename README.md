## Getting Started

Details of the Pi Pico support in wolfSSL can be found in the
`wolfcrypt/src/port/pi_pico/README.md`.

Hi.

### Prerequisites

You of course need the Pico 2W (RP2350) board we received from Guillermo.

You need to have the [Raspberry Pi Pico SDK GitHub repository](https://github.com/raspberrypi/pico-sdk)
somewhere on your system. You also need the ARM compiler and CMake installed,
in Debian / Ubuntu you can do this using:

```
sudo apt install cmake gcc-arm-none-eabi libnewlib-arm-none-eabi libstdc++-arm-none-eabi-newlib
```

### 1. Set an export to the wolfSSL source directory.

```
export WOLFSSL_ROOT=/path/to/wolfssl/source
export WOLFSSL_ROOT=~/bwsi/pico/wolfssl
```

### 2. Setup pico-sdk and set `PICO_SDK_PATH`

```
export PICO_SDK_PATH=/path/to/pico-sdk
export PICO_SDK_PATH=~/pico/pico-sdk
export PICO_PLATFORM=rp2350 
```

### 3. cmake and make

The following CMAKE options are available:

* `PICO_BOARD` - This should be set to `pico` for a Pi Pico, `pico_w` for a Pi Pico with WiFi or `pico2` for a Pi Pico 2. A full list of boards for this option can be found [here](https://github.com/raspberrypi/pico-sdk/tree/master/src/boards/include/boards), just ignore the `.h` at the end.
* `USE_WIFI` - Build the tests that use WiFi, only works when `PICO_BOARD` defined has a CYW43 WiFi chip.
* `USE_UART` - Output to UART instead of USB, for the Pi Debug Probe.
* `WIFI_SSID` - The SSID to connect to (if `USE_WIFI` is set).
* `WIFI_PASSWORD` - The password used for the WiFi network (if `USE_WIFI` is set).
* `TEST_TCP_SERVER_IP` - The test server to connect to for the TCP client test (if `USE_WIFI` is set).

To use the RP2350 in RISC-V mode, add `-DPICO_PLATFORM=rp2350-riscv`.

```
$ cd RPi-Pico
$ mkdir build
$ cd build
$ cmake -DPICO_BOARD=pico2_w -DPICO_PLATFORM=rp2350 ..
$ make
```

### 4. Upload to the Pico

Hold the boot button and plug the Pico into your computer, you can then
drag/drop a `.uf2` to the Pico. It will stop becoming a USB mass storage device
and run immediately once the upload is complete.

### 5. Serial output

Because we have not set `USE_UART`, once rebooted the USB port will turn into an
"Abstract Control Module" serial port. This means our board is visible as the familiar
`/dev/ttyACM0` or `/dev/tty.usbmodemXXXX`. The baud rate of this port
is 115200.

Either use `minicom` (`minicom -b 115200 -o -D /dev/ttyACM0`) or VSCode's built-in serial monitor to communicate with the board.

## Appendix A: Intellisense include paths

[placeholder]