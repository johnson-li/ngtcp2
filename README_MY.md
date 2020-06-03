# FOR HTML APPLICATION

## how to install ``lexbor`` library 
- LINK: http://lexbor.com/docs/lexbor/
- Download Lexbor signing key used for our repositories and packages and add it to apt’s keyring:
    - curl -O https://lexbor.com/keys/lexbor_signing.key
    - apt-key add lexbor_signing.key

- To configure Lexbor repository, create the following file named ``/etc/apt/sources.list.d/lexbor.list``:
    - Ubuntu 18.04:
        - deb https://packages.lexbor.com/ubuntu/ bionic liblexbor
        - deb-src https://packages.lexbor.com/ubuntu/ bionic liblexbor

- Install Lexbor base package and additional packages you would like to use.
    - apt update
    - apt install liblexbor
    - apt install liblexbor-dev


## how to add the ``lexbor`` to dynamic library link
- in configure running:
  - ./configure PKG_CONFIG_PATH=$PWD/../../openssl/build/lib/pkgconfig LDFLAGS="-Wl,-rpath,$PWD/../../openssl/build/lib,-llexbor" --host=arm
  - this means you need add ``llexbor`` and ``--host=arm``


## how to get the inner element ``streams_`` in ``Client`` Class
- 通过增加了一个messages的全局变量数组，数据类型是message
- message struct 主要包括了client类中的conn_, streams_, 还有HTML的content信息。
- message中有一个``message_complete_cb_called``，用来控制收到data是否需要http解析
