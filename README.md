## What is this?

I created this repository as a basic example of how to get WolfSSL working on the Raspberry Pi Pico. This project can be built either from the CLI, as described in step 3, or with the VSCode RPi Pico extension. After building, you'll be left with a `testwolfcrypt` binary that will test wolfcrypt's functionality and output the return value over USB UART.

### Prerequisites

You need to have the [Raspberry Pi Pico SDK GitHub repository](https://github.com/raspberrypi/pico-sdk) and the [WolfSSL Github Repository](https://github.com/wolfSSL/wolfssl)
somewhere on your system. You also need the ARM compiler (gcc-arm-none-eabi) and CMake installed. (You should already have those installed if you went to Beaver Works.)

## Setup

#### 1. Set your env variables to your wolfSSL and pico-sdk directories.

In the original project, you'd do this by typing something like `export WOLFSSL_ROOT=/path/to/wolfssl/source` in your terminal. I found that it's cleaner to set these in `CMakeLists.txt` instead; just update the two lines at the top of CMakeLists with the appropriate paths:
```
set(ENV{PICO_SDK_PATH} "/Users/bsquared/pico/pico-sdk")
set(ENV{WOLFSSL_ROOT} "/Users/bsquared/bwsi/pico/wolfssl")
```

#### 2. Configure your WolfSSL library
You must go into your WolfSSL library to define your own RNG function. In the root directory, navigate to `./wolfcrypt/src/random.c`. Add the following import to the top:
```
#include <pico/rand.h>
```
(This will resolve because of the `${PICO_SDK_PATH}/src/rp2_common/pico_rand/include` include path in CMakeLists.txt)

Then add the following function around line 3710:
```
#elif defined(CUSTOM_RAND_GENERATE_BLOCK)
    extern int wc_pico_rng_gen_block(byte* output, word32 sz)
    {
        uint64_t rand64;
        while (sz > 0) {
            word32 len = sizeof(rand64);
            if (sz < len)
                len = sz;
            /* Get one random 64-bit int from hw RNG */
            rand64 = get_rand_64();
            XMEMCPY(output, &rand, len);
            output += len;
            sz -= len;
        }

        return 0;
    }
```

#### 3. cmake and make

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

#### 4. Upload to the Pico

Hold the boot button and plug the Pico into your computer. You can then
drag/drop a `.uf2` to the Pico. It will stop becoming a USB mass storage device
and run immediately once the upload is complete.

Alternatively, use `picotool load -f <path-to-uf2>` to force the Pico into BOOTSEL mode and load the firmware.

#### 5. Serial output

Because we have not set `USE_UART`, once rebooted the USB port will turn into an
"Abstract Control Module" serial port. This means our board is visible as the familiar
`/dev/ttyACM0` or `/dev/tty.usbmodemXXXX`. The baud rate of this port is 115200.

Either use `minicom` (`minicom -b 115200 -o -D /dev/tty...`) or VSCode's built-in serial monitor to communicate with the board.

If all tests pass, the Pico will output "End: 0".

## Appendix A: Intellisense include paths

VSCode's IntelliSense will not automatically detect your include paths. For example, IntelliSense can not resolve the `#include <pico/rand.h>` line in `${WOLFSSL_ROOT}/wolfcrypt/src/random.c`. 

To remedy this, we'll modify CMake's build process slightly. By adding the line `set(CMAKE_EXPORT_COMPILE_COMMANDS ON)` to CMakeLists.txt, Cmake will create a file on build that specifies include path. This file, `compile_commands.json`, can then be inputted into `.vscode/c_cpp_properties.json` file using a custom VSCode configuration. By doing this, VSCode IntelliSense will use that `compile_commands.json` to set include paths.

```
{
    "configurations": [
        {
            "name": "CMake",
            "compileCommands": "${config:cmake.buildDirectory}/compile_commands.json"
        }
    ],
    "version": 4
}
```

## Appendix B: Additional resources

Surprisingly, the WolfSSL library includes some documentation regarding the RasPi Pico. You can find this in your project's WolfSSL directory at `wolfcrypt/src/port/pi_pico/README.md`.

## Appendix C: Using WolfSSL in your own scripts

Let's say you have this `bootloader.c` script that has the following WolfSSL imports:
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
target_include_directories(bootloader PRIVATE ${WOLFSSL_ROOT})
```

Now, in your `bootloader.c`, you can import WolfSSL modules like normal!
```
#include "wolfssl/wolfcrypt/settings.h"
#include "wolfssl/wolfcrypt/aes.h"
...etc
```

best of luck

\- ben b
