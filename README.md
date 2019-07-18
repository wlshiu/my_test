# my_test

+ master
+ test_avi
+ test_bslot
+ test_c_wrap_py
+ test_combination
+ test_crc32
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
    - [reference](https://github.com/engina/uip-1.0-win)
    - [offical](https://github.com/adamdunkels/uip)

# Broadcast (ubuntu)

+ `socat`

```
$ sudo apt-get install socat
$ echo "hello" | socat - UDP4-DATAGRAM:255.255.255.255:9999,broadcast
$ echo "hello" | socat - UDP4-LISTEN:4321,reuseaddr,fork UDP4:192.168.0.100:4321
```

    - `binary fiel`

    ```
    # on host 1:
    $ socat -u open:rom.img,binary tcp-listen:999

    # on host 2:
    $ socat -u tcp:host1:999 open:rom.img,create,binary

    這個命令把文件 rom.img 用二進制的方式, 從host 1 傳到host 2。
    -u 表示數據單向流動, 從第一個參數到第二個參數,
    -U 表示從第二個到第一個。文件傳完了, 自動退出。
    ```

+ `netcat` or `nc`

```
    nc [-hlnruz][-g<網關…>][-G<指向器數目>][-i<延遲秒數>][-o<輸出文件>]
        [-p<通信端口>][-s<來源位址>][-v…][-w<超時秒數>][主機名稱][通信端口…]

        -g <網關> 設置路由器躍程通信網關, 最丟哦可設置8個。
        -G <指向器數目> 設置來源路由指向器, 其數值為4的倍數。
        -h 在線幫助。
        -i <延遲秒數> 設置時間間隔, 以便傳送信息及掃瞄通信端口。
        -l 使用監聽模式, 管控傳入的資料。
        -n 直接使用IP地址, 而不通過域名服務器。
        -o <輸出文件> 指定文件名稱, 把往來傳輸的數據以16進制字碼傾倒成該文件保存。
        -p <通信端口> 設置本地主機使用的通信端口。
        -r 亂數指定本地與遠端主機的通信端口。
        -s <來源位址> 設置本地主機送出數據包的IP地址。
        -u 使用UDP傳輸協議。
        -v 顯示指令執行過程。(用兩個v可得到更詳細的內容)
        -w <超時秒數> 設置等待連線的時間。
        -z 使用0輸入/輸出模式, 只在掃瞄通信端口時使用。
```

    - example

    ```
    $ echo -n "hello" | nc -u 255.255.255.255 9999
    $ echo -n "hello" | nc -u 192.168.1.255 8080
    ```

+ `tcpdump` catch packets

```
　　tcpdump [ -adeflnNOpqStvx ] [ -c 數量 ] [ -F 文件名 ]
　　　　　　　　　　[ -i 網絡接口 ] [ -r 文件名] [ -s snaplen ]
　　　　　　　　　　[ -T 類型 ] [ -w 文件名 ] [表達式 ]
　　1. tcpdump的選項介紹
　　　-a 　　　將網絡地址和廣播地址轉變成名字；
　　　-d 　　　將匹配信息包的代碼以人們能夠理解的彙編格式給出；
　　　-dd 　　　將匹配信息包的代碼以c語言程序段的格式給出；
　　　-ddd 　　　將匹配信息包的代碼以十進制的形式給出；
　　　-e 　　　在輸出行打印出數據鏈路層的頭部信息；
　　　-f 　　　將外部的Internet地址以數字的形式打印出來；
　　　-l 　　　使標準輸出變為緩衝行形式；
　　　-n 　　　不把網絡地址轉換成名字；
　　　-t 　　　在輸出的每一行不打印時間戳；
　　　-v 　　　輸出一個稍微詳細的信息, 例如在ip包中可以包括ttl和服務類型的信息；
　　　-vv 　　　輸出詳細的報文信息；
　　　-c 　　　在收到指定的包的數目後, tcpdump就會停止；
　　　-F 　　　從指定的文件中讀取表達式,忽略其它的表達式；
　　　-i 　　　指定監聽的網絡接口；
　　　-r 　　　從指定的文件中讀取包(這些包一般通過-w選項產生)；
　　　-w 　　　直接將包寫入文件中, 並不分析和打印出來；
　　　-T 　　　將監聽到的包直接解釋為指定的類型的報文, 常見的類型有rpc(遠程過程調用)
                和 snmp(簡單網絡管理協議；)
```

# DHCP

+ install

    ```
    $ sudo apt install isc-dhcp-server
    ```

+ configure setting

    - set interface

        ```
        $ sudo vi /etc/default/isc-dhcp-server
        ...
        INTERFACES=""  => INTERFACES="eth0" or "enp0s8"
        ```

    - set dhcp config
        ```
        $ sudo vim /etc/dhcp/dhcpd.conf
        ...
        # add to tail
        subnet 192.168.56.0 netmask 255.255.255.0 {
            range 192.168.56.5 192.168.56.10;
            option domain-name-servers 8.8.8.8;
            option domain-name "my-cloud.orz";
            option subnet-mask 255.255.255.0;
            option routers 196.168.56.3;  # dhcp server ip address
            option broadcast-address 192.168.56.254;
            default-lease-time 600;
            max-lease-time 7200;
            filename "test.elf";
        }
        ```

+ run dhcp server

    ```
    $ sudo systemctl enable isc-dhcp-server
    $ sudo systemctl restart isc-dhcp-server
    ```

+ check dhcp server status

    ```
    $ sudo systemctl status isc-dhcp-server
    ```

+ list assigned addresses

    ```
    $ dhcp-lease-list
    ```

+ misc

    - error msg `Can't open /var/lib/dhcp/dhcpd.leases for append.`
        > change permission

    ```
    $ sudo chmod -R 777 /var/lib/dhcp/dhcpd.leases
    ```

    - [dhcptest](https://github.com/CyberShadow/dhcptest)

    ```
    # windows console
    $ dhcptest-0.7-win64.exe --quiet --query --wait --tries 5 --timeout 10
    op=BOOTREPLY chaddr=E4:97:67:C9:E3:E0 hops=0 xid=45B7FC3D secs=0 flags=0000
    ciaddr=0.0.0.0 yiaddr=192.168.56.104 siaddr=0.0.0.0 giaddr=0.0.0.0 sname= file=
    4 options:
     54 (Server Identifier): 192.168.56.100
     53 (DHCP Message Type): offer
      1 (Subnet Mask): 255.255.255.0
     51 (IP Address Lease Time): 1200 (20 minutes)
    op=BOOTREPLY chaddr=E4:97:67:C9:E3:E0 hops=0 xid=45B7FC3D secs=0 flags=8000
    ciaddr=0.0.0.0 yiaddr=192.168.56.7 siaddr=192.168.56.3 giaddr=0.0.0.0 sname= file=
    8 options:
     53 (DHCP Message Type): offer
     54 (Server Identifier): 192.168.56.3
     51 (IP Address Lease Time): 600 (10 minutes)
      1 (Subnet Mask): 255.255.255.0
      3 (Router Option): 196.168.56.2
      6 (Domain Name Server Option): 8.8.8.8
     28 (Broadcast Address Option): 192.168.56.254
     15 (Domain Name): my-cloud.orz
    ```

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
        > 提供建立和管理 Windows 圖形介面的功能, 例如桌面, 視窗和功能表.裡面的函式可以讓應用程式建立及管理視窗, 接收 Windows 訊息((諸如使用者的輸入或系統的通知), 在視窗中顯示文字, 及顯示一個訊息視窗.
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
