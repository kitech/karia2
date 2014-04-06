add_library(websockets STATIC
  lib/base64-decode.c     lib/extension.c 
  lib/libwebsockets.c  lib/server-handshake.c  
  lib/client.c            lib/extension-deflate-frame.c  
  lib/sha-1.c 
  lib/client-handshake.c  lib/extension-deflate-stream.c 
  lib/output.c
  lib/client-parser.c     lib/getifaddrs.c      
  lib/parsers.c 
  lib/daemonize.c         lib/handshake.c         
  lib/server.c
)

target_link_libraries(websockets -lcrypto)
