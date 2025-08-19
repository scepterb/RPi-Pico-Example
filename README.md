## What is this?

I created this repository as a basic example of how to get WolfSSL working on the Raspberry Pi Pico. This project can be built either from the CLI, as described in step 3, or with the VSCode RPi Pico extension. The resulting `testwolfcrypt` binary will test wolfcrypt's functionality (shocking) and output the return value over USB UART.

### Prerequisites

You need to have the [Raspberry Pi Pico SDK GitHub repository](https://github.com/raspberrypi/pico-sdk) and the [WolfSSL Github Repository](https://github.com/wolfSSL/wolfssl)
somewhere on your system. You also need the ARM compiler (gcc-arm-none-eabi) and CMake installed. I mean, it would be pretty impressive if you haven't had those installed at this point.

## Setup

#### 1. Set you env variables to your wolfSSL and pico-sdk directories.

In the original project, you'd do this by typing something like `export WOLFSSL_ROOT=/path/to/wolfssl/source` in your terminal. However, that means CMake will only function when ran in that terminal session. Instead, just update the two lines at the top of your `CMakeLists.txt` with the appropriate paths
```
set(ENV{PICO_SDK_PATH} "/Users/bsquared/pico/pico-sdk")
set(ENV{WOLFSSL_ROOT} "/Users/bsquared/bwsi/pico/wolfssl")
```

#### 2. cmake and make

To generate the buildsystem and build the project, run `cmake` and `make`:
```
$ cd RPi-Pico-Example
$ mkdir build
$ cd build
$ cmake -DPICO_BOARD=pico2_w -DPICO_PLATFORM=rp2350 ..
$ make
```

> [!TIP]
> Use `make -j4` to spread the compiling process across 4 cores of your CPU, speeding up the build.

> [!NOTE]
> **You shouldn't need these**, but the original project had the following CMake options available:
> * `PICO_BOARD` - A full list of boards for this option can be found [here](https://github.com/raspberrypi/pico-sdk/tree/master/src/boards/include/boards), just ignore the ".h" at the end.
> * `USE_UART` - Output to UART instead of USB, for the Pi Debug Probe.
> * `USE_WIFI`, `WIFI_SSID`, `WIFI_PASSWORD`, `TEST_TCP_SERVER_IP` are also all options, but I doubt the functionality of WiFi & TCP/IP.

#### 3. Upload to the Pico

Hold the boot button and plug the Pico into your computer. You can then
drag/drop a `.uf2` to the Pico. It will stop becoming a USB mass storage device
and run immediately once the upload is complete.

Alternatively, use `picotool load -f <path-to-uf2>` to force the Pico into BOOTSEL mode and load the firmware.

### 4. Serial output

Because we have not set `USE_UART`, once rebooted the USB port will turn into an
"Abstract Control Module" serial port. This means our board is visible as the familiar
`/dev/ttyACM0` or `/dev/tty.usbmodemXXXX`. The baud rate of this port
is 115200.

Either use `minicom` (`minicom -b 115200 -o -D /dev/tty...`) or VSCode's built-in serial monitor to communicate with the board.

If all tests pass, the Pico will output "End: 0".

## Appendix A: Intellisense include paths

You can use your CMakeLists.txt to tell VSCode's IntelliSense what your include paths are. 

For example, in `${WOLFSSL_ROOT}/wolfcrypt/src/random.c`, you might notice that VSCode can not resolve the `#include <pico/rand.h>` line. To remedy this, I have a line in CMakeLists that exports a `compile_commands.json` file, which can then be inputted into your `c_cpp_properties.json` file to create an include path. 

Read my comment explaining this in CMakeLists.txt for more info.

## Appendix B: Additional resources

Surprisingly, WolfSSL does include a bit of documentation about using the library on the Pico. This can be found in your WolfSSL directory, at `wolfcrypt/src/port/pi_pico/README.md`.

## Appendix C: Using WolfSSL in your own scripts

Let's say you have this `bootloader.c` script from the attack phase that has the following WolfSSL imports:
```
// Cryptography Imports
#include "wolfssl/wolfcrypt/settings.h"
#include "wolfssl/wolfcrypt/aes.h"
#include "wolfssl/wolfcrypt/sha.h"
#include "wolfssl/wolfcrypt/rsa.h"
```

Your CMakeLists should include these modules when your compiling your new bootloader target:
```
add_executable(bootloader
    src/bootloader.c
)

# Include WolfSSL during linking phase
target_link_libraries(bootloader
    wolfssl
    pico_stdlib
    pico_rand
)

# Include WolfSSL in include path
target_include_directories(testwolfcrypt PRIVATE ${WOLFSSL_ROOT})
```

Now, in your `bootloader.c`, you can import WolfSSL modules like normal!
```
#include "wolfssl/wolfcrypt/settings.h"
#include "wolfssl/wolfcrypt/aes.h"
...etc
```

best of luck

\- ben b