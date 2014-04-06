add_library(maia STATIC
  ./maiaObject.cpp
  ./maiaFault.cpp 
  ./maiaXmlRpcClient.cpp
  ./maiaXmlRpcServer.cpp 
  ./maiaXmlRpcServerConnection.cpp
)

qt5_use_modules(maia Core Network Xml)


