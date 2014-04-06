add_library(qjsonrpc STATIC
  ./src/qjsonrpcmessage.cpp  ./src/qjsonrpcservice.cpp
)

qt5_use_modules(qjsonrpc Core Network)
