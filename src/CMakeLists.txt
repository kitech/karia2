add_executable(karia2
  ../main.cpp
  ../karia2application.cpp
  ../karia2.cpp
  ../simplelog.cpp
  ../aboutdialog.cpp
  ../abstractstorage.cpp 
  ../batchjobmandlg.cpp
  ../catmandlg.cpp
  ../catpropdlg.cpp
  ../dlrules.cpp
  ../dropzone.cpp
  ../dmstatusbar.cpp
  ../dircompletermodel.cpp
  ../hash.cpp
  ../instantspeedhistogramwnd.cpp
  ../labspace.cpp
  ../main.cpp
  ../nullcontroller.cpp
  ../karia2_embed_aria2.cpp
  ../karia2_standalone_aria2.cpp
  ../asynctask.cpp
  ../radarscanner.cpp
  ../rulesmandlg.cpp
  ../segmentlogmodel.cpp
  ../serverswitcher.cpp
  #		   skypeclientwnd.cpp
  ../sqlitecategorymodel.cpp
  ../sqlitesegmentmodel.cpp
  ../sqlitestorage.cpp
  ../sqlitetaskmodel.cpp
  ../storageexception.cpp
  ../storagefactory.cpp
  ../taskballmapwidget.cpp
  ../taskinfodisgestwnd.cpp
  ../taskinfodlg.cpp
  ../taskqueue.cpp
  ../utility.cpp
  ../walksitewnd.cpp
  ../walksitewndex.cpp
  ../webpagelinkdlg.cpp
  ../xmlstorage.cpp		 
  ../optionmanager.cpp
  ../preferencesdialog.cpp
  ../ariaman.cpp
  ../libng/html-parse.c
  ../libng/md5.c
  ../libng/qtmd5.cpp
  ../torrentpeermodel.cpp
  ../taskservermodel.cpp
  ../seedfilemodel.cpp
  ../seedfilesdialog.cpp
  ../taskitemdelegate.cpp
  ../mimetypeshash.cpp
  #		   skypetracer.cpp
  ../metauri.cpp
  #		   skypetunnel.cpp
  ../simplelog.cpp
  ../asyncdatabase.cpp
  ../databaseworker.cpp
  ../emaria2c.cpp	
  ../karia2statcalc.cpp # aria2-1.13.0/src/option_processing.cc
  ../aria2manager.cpp	
  ../aria2embedmanager.cpp
  ../aria2libaria2manager.cpp
  ../aria2rpcserver.cpp
  ../aria2rpcmanager.cpp
  ../aria2jsonmanager.cpp
  ../aria2wsjsonmanager.cpp
  ../aria2xmlmanager.cpp
  ../aria2managerfactory.cpp

  ../qtsingleapplication/qtsingleapplication.cpp
  ../qtsingleapplication/qtlocalpeer.cpp

  ../libmaia/maiaObject.cpp
  ../libmaia/maiaFault.cpp 
  ../libmaia/maiaXmlRpcClient.cpp
  ../libmaia/maiaXmlRpcServer.cpp 
  ../libmaia/maiaXmlRpcServerConnection.cpp
  
  ../qjsonrpc/src/qjsonrpcmessage.cpp  ../qjsonrpc/src/qjsonrpcservice.cpp

  ../libwebsockets/lib/base64-decode.c     ../libwebsockets/lib/extension.c 
  ../libwebsockets/lib/libwebsockets.c  ../libwebsockets/lib/server-handshake.c  
  ../libwebsockets/lib/client.c            ../libwebsockets/lib/extension-deflate-frame.c  
  ../libwebsockets/lib/sha-1.c 
  ../libwebsockets/lib/client-handshake.c  ../libwebsockets/lib/extension-deflate-stream.c 
  ../libwebsockets/lib/output.c
  ../libwebsockets/lib/client-parser.c     ../libwebsockets/lib/getifaddrs.c      
  ../libwebsockets/lib/parsers.c 
  ../libwebsockets/lib/daemonize.c         ../libwebsockets/lib/handshake.c         
  ../libwebsockets/lib/server.c

  )

qt5_use_modules(karia2 Core Network Widgets Gui Xml Sql)
target_link_libraries(karia2 -laria2 -lwslay)
target_link_libraries(karia2 -lgcrypt  -lrt -lnettle -lgmp -lz   -lcares   -lgnutls   -lsqlite3 -lxml2 -lz -lm)
