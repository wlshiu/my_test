microhttpd
---

```
$ cd microhttpd
$ ./configure --enable-static --disable-cookie --disable-curl --disable-doc
```

+ example

    - http-server
        1. at `libmicrohttpd-0.9.26/src/examples/fileserver_example`

            ```
            λ .\fileserver_example.exe 36969
            ```

        1. at `libmicrohttpd-1.0.2/src/examples/demo`

            ```
            λ .\demo.exe 36969
            ```

    - browser
        > url: `http://localhost:36969/`


