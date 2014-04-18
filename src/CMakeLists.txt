
file (GLOB Resources ../*.qrc)
QT5_ADD_RESOURCES (ResourceSources ${Resources})

# 导到progname_automoc.cpp位置不对。
# set(CMAKE_CURRENT_BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../tmp)

qt5_wrap_ui(UIS_HDRS
  ../aboutdialog.ui
  ../batchjobmandlg.ui
  ../catmandlg.ui
  ../catpropdlg.ui
  ../columnsmandlg.ui
  ../dlrules.ui
  ../dmstatusbar.ui
  ../karia2.ui
  ../rulesmandlg.ui
  # ../skypeclientwnd.ui
  ../taskinfodisgestwnd.ui
  ../taskinfodlg.ui
  ../walksitewnd.ui
  ../walksitewndex.ui
  ../webpagehostselectdlg.ui
  ../webpagelinkdlg.ui
  ../webpageurlinputdlg.ui
  ../labspace.ui
  ../preferencesdialog.ui
  ../seedfilesdialog.ui
  ../proxyinfodialog.ui

  )

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
  ../sqlitetasktreemodel.cpp

  ${ResourceSources}
  ${UIS_HDRS}  
  )


qt5_use_modules(karia2 Core Network Widgets Gui Xml Sql WebSockets)
target_link_libraries(karia2 -laria2 -lwslay)
target_link_libraries(karia2 -lcwebsockets -lqtsingleapplication -lmaia -lqjsonrpc)
target_link_libraries(karia2 -lgcrypt  -lrt -lnettle -lgmp -lz   -lcares   -lgnutls   -lsqlite3 -lxml2 -lz -lm)
