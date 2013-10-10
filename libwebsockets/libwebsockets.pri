####### 整合工程包含文件
QT *= 

SOURCES += \
        $$PWD/lib/base64-decode.c     $$PWD/lib/extension.c                 \
        $$PWD/lib/libwebsockets.c  $$PWD/lib/server-handshake.c  \
        $$PWD/lib/client.c            $$PWD/lib/extension-deflate-frame.c        $$PWD/lib/sha-1.c  \
        $$PWD/lib/client-handshake.c  $$PWD/lib/extension-deflate-stream.c  $$PWD/lib/output.c \
        $$PWD/lib/client-parser.c     $$PWD/lib/getifaddrs.c                $$PWD/lib/parsers.c \
        $$PWD/lib/daemonize.c         $$PWD/lib/handshake.c                 $$PWD/lib/server.c
#  $$PWD/lib/minilex.c  

HEADERS += 

INCLUDEPATH += $$PWD/lib 

DEFINES += LWS_LIBRARY_VERSION=\"\\\"1.2\\\"\" LWS_BUILD_HASH=\"\\\"1.2hash\\\"\"  \
         LWS_OPENSSL_SUPPORT=1 \
         LWS_OPENSSL_CLIENT_CERTS=\"\\\"/etc/pki/tls/certs/\\\"\"
LIBS += -lcrypto