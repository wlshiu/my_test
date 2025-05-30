# my_test

+ master
+ test_avi
+ test_bslot
+ test_c_wrap_py
+ test_c_call_py
+ test_combination
+ test_crc32
+ test_crc16
+ test_fatfs
+ test_frame_buf
+ test_libevent
+ test_llist
+ test_log2mem
+ test_lwip
+ test_msg_box
+ test_ring_log
+ test_sim_spifc
+ test_string_reverse
+ test_timer
+ test_trace_malloc
+ test_wsdiscovery
+ test_xml
+ test_tftp
    - [reference](https://github.com/wlshiu/tftpx)
+ test_uip
+ test_elf2
+ test_elf_axf
+ test_shell
    - It MUST be on Windows 10 platform
        > support `ENABLE_VIRTUAL_TERMINAL_PROCESSING`
+ test_backtrace
    > backtrace for MCU system

    - [reference](https://github.com/armink/CmBacktrace)
+ test_rbuf
    - ring buffer template
        > if end_idx catches start_idx, replace the data and force start_idx forward
+ test_protothreads
    > Protothreads function as stackless, lightweight threads, or coroutines, providing a blocking context cheaply using minimal memory per protothread (on the order of single bytes).
+ test_uart_term
    > base on raspberry pi
+ test_test_nodeq
    > node queue base on share memory pool between 2 tasks
+ test_rsa
    > Asymmetric Encryption: use Public and Private Key-pair
+ test_version_info
    > import key-pair for authentication
+ test_auto_script
    > configure regiter of H/w module with script
+ test_sw_time_event
    > software time event trigger
+ test_lwmesh
    > mesh route [Atmel Lightweight Mesh](https://www.microchip.com/DevelopmentTools/ProductDetails/PartNO/Atmel%20Lightweight%20Mesh)
+ test_print
    > [Tiny print](https://github.com/mpaland/printf)
+ test_script
    > generate bash script
+ test_upgrade
+ test_serial_port_win
+ test_serial_port_linux
+ test_float_bits
+ test_skb
    > analyze socket buffer of linux net sub-system
+ test_fat
    > [fat_io_lib](https://github.com/ultraembedded/fat_io_lib)
+ test_ext2
    > [EXT2 Linux File System](https://github.com/yoonje/ext2-linux-file-system)
+ test_lwext4
    > [lwext4](https://github.com/gkostka/lwext4)
+ test_fdt
    > linux device tree
+ test_lzma
    > test lzma with lzma920
+ test_backtrace_nds32
    > implement backtrace in nds32
+ test_mem_watermark
    > the stack usage detection
+ test_tlsf
    > Two-Level Segregated Fit memory allocator [TLSF](https://github.com/mattconte/tlsf)
+ test_xlsxio
    > read xlsx file
+ test_lsh
    > [LSH](https://github.com/brenns10/lsh) is a simple implementation of a shell in C
+ test_gen_pin_table
    > generate a pin table with JSON file.
+ test_aes
+ test_aes_KDF
+ test_convolution-2D
    > [convolution-2D](https://github.com/attilach/convolution-2D)
+ test_imgui
    > [Dear ImGui](https://github.com/ocornut/imgui)
+ test_glfw
    > [glfw](https://www.glfw.org/)
+ test_rc5
    > RC5 Encrypt/Decrypt
+ test_xmodem
    > simulate xmodem transmission

    - [reference](https://www.menie.org/georges/embedded/)

+ test_winsocket
+ test_winthread
+ test_ymodem
+ test_tinyUSB
    > Host and device

+ teset_cpu_pipeline_sim
+ test_ARM_CortexM_Sim
+ ARM cpu simulator
    - test_arm_sim
    - test_ARMSim
    - test_armv8_sim

+ test_basic_window
    > basic win32 window system
+ test_mingw_wmain
    > mingw not support `wmain()`, this is a wrap

+ test_flash_fs
    > use LittleFS and Spiffs

+ test_freertos
    > windows simulation

+ test_combination_lite
+ test_tiny_regex
    > 只支援 `1 local variable`, 故無法同時宣告多個 pattern 來使用
+ test_mini_scanf
    > Only supported control characters

    ```
    %c  - matches a character or a sequence of characters
    %s  - matches a sequence of non-whitespace characters (a string)
    %[] - matches a non-empty sequence of character from set of characters.
    %d  - matches a decimal integer.
    %u  - same as %d (You can enable ignoring the sign. nostd)
    ```

+ test_PocketSphinx
    > A small speech recognizer

+ test_arm_kws
    > Keyword spotting for MCU

+ test_rand_mac
    > random MAC address

+ test_rw_csv
    > read `wav` file and convert to `csv` file

+ test_findfile
    > recursively find files on windows platform

+ test_TinyMaix
    > use [TinyMaix](https://github.com/sipeed/TinyMaix/tree/main)

+ test_my_malloc
    > SMM (Static Memory Management) is implemented to manage a static memory.
    > This lib only support to allocate memory and record the pulse of memory usage.
    > ps. free memory and merge memory leakage are NOT supported,
    > and the memory fragmentation MUST be happened.

+ test_overlay
    > Keil example of overlay section of scatter

+ test_CherryUSB
    > simulate CherryUSB v0.9.0 (Only USB protocol layer)

+ test_win32_mutex
    > mutex of win32 api

+ test_libusb
    > use static lib on win32

+ test_recursive_combination
    > 遞迴方式來搜尋 Pin 的組合

+ test_gen_bin_pattern
    > generate a binary file and insert other file

+ test_stm32
    > ARM development environment with CodeBlocks

+ test_rsqrt
    > fast the reciprocal of square root

+ test_pt
    > Protothreads

+ test_pthread_win32
+ test_miniz
    > Miniz is a lossless, high performance data compression library in a single source file that implements the zlib (RFC 1950) and Deflate (RFC 1951) compressed data format specification standards.
    >> [richgel999/miniz](https://github.com/richgel999/miniz/tree/master)

+ test_qt5_cli
    > only use unicode of QT5 to implement CLI

+ test_py_call_c
    > python call C API with share object

+ test_SystemC
    > [systemc-2.3.3](https://github.com/accellera-official/systemc)


+ test_lexical_analyzer
    > lexical analyzer (e,g, flex)

+ test_SimpleAudioDenoise
    > [A simple audio De-Noise](https://github.com/cpuimage/SimpleAudioDenoise)

+ test_lru
    > LRU (Least Recently Used) Cache
    >> [umer7/LRU-Cache-using-C](https://github.com/umer7/LRU-Cache-using-C/tree/master)

+ test_ppm
    > read ppm file (color image) and convert

+ test_tinySSL
+ test_easytlv

+ test_Qfplib
    > for arm cortex-m0+

+ test_flashDB
+ test_mempool
    > handle malloc/free in a mempool

+ test_makefile
    > makefele example

+ test_sqrt
    > implement sqrt version

+ test_sw_cordic
    > ref. [Simple C source for CORDIC](https://www.dcs.gla.ac.uk/~jhw/cordic/)

+ test_riscv_isa_sim
    > ref. [Build RISC-V Instruction Set Simulator from scratch](file:///C:/Users/wl_hsu/AppData/Local/Temp/7zO0CB1E7A8/Build%20RISC-V%20Instruction%20Set%20Simulator%20from%20scratch%20-%20HackMD.html)

+ test_tim2pwm
    > generate PWM waveform depend on the register values of timer

+ test_sw_cordic_arm
    > use DSP-lib of ARM

# MS Visual Studio

+ include libs
    - `shell32.lib`
        > 用於打開網頁和文件, 建立文件時的默認文件名的設置等大量功能.嚴格來講, 它只是代碼的合集, 真正執行這些功能的是操作系統的相關程序, dll文件只是根據設置調用這些程序的相關功能罷了.
    - `ws2_32.lib`
        > Windows Sockets應用程序接口, 用於支持Internet和網絡應用程序
        >>　bsd socket api, WSAxxx api

    ```
    #include <winsock2.h>
    #pragma comment (lib, "ws2_32.lib")  //加載 ws2_32.dll
    ```

    - `comctl32.lib`
        > 提供各種標準視窗介面元件.它提供對話框如開啟檔案, 存檔及另存新檔, 或視窗元件如按鈕和進度列.它倚靠 user32.dll 和 Gdi32.dll 來建立及管理這些介面元素
    - `iphlpapi.lib`
        > 用來獲取, 設置網絡相關參數, 比如設置IP地址, 獲取網卡是否被禁用, 還有一些和ARP, 路由表相關的參數, 比如nmap的windows port版本就利用了這個ip helper api.
    - `mswsock.lib`
        > 提供 Winsock 網絡服務相關元件
    - `kernel32.lib`
        > 提供應用程式一些Win32下的基底API, 包括記憶體管理, 輸入/輸出操作和同步函式
    - `user32.lib`
        > 提供建立和管理 Windows 圖形介面的功能, 例如桌面, 視窗和功能表.裡面的函式可以讓應用程式建立及管理視窗, 接收 Windows 訊息（(諸如使用者的輸入或系統的通知), 在視窗中顯示文字, 及顯示一個訊息視窗.
          這個函式庫裡面大部份函式也需要倚靠 Gdi32.dll 提供的繪圖功能, 來對使用者介面進行彩現.有些程式還會直接使用GDI函式, 來對先前由 User32.dll 所建立的視窗進行底層繪圖.
    - `gdi32.lib`
        > 提供跟圖形裝置介面有關的函式, 例如輸出到顯示卡和列印機的原生繪圖功能
    - `winspool.lib`
        > 提供列印功能接口
    - `comdlg32.lib`
    - `advapi32.lib`
    - `ole32.lib`
    - `oleaut32.lib`
    - `uuid.lib`
    - `odbc32.lib`
    - `odbccp32.lib`
    - `libntdll`

