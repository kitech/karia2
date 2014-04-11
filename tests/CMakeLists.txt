
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/..")

### tasktreeview
qt5_wrap_ui (tasktreeview_UIS_HDRS  tasktreeview/tasktreeview.ui)
add_executable(tasktreeview
  tasktreeview/main.cpp
  tasktreeview/tasktreeview.cpp
  ${tasktreeview_UIS_HDRS}

  ## dev
  ../sqlitetasktreemodel.cpp
  ../sqlitetaskmodel.cpp
  
  ## depends
  ../sqlitestorage.cpp
  ../abstractstorage.cpp
  ../asyncdatabase.cpp
  ../databaseworker.cpp
  )
qt5_use_modules(tasktreeview Core Widgets Gui Xml Sql)

### dummy
qt5_wrap_ui (dummy_UIS_HDRS dummy/dummy.ui)
add_executable(dummy
  dummy/main.cpp
  dummy/dummy.cpp
  ${dummy_UIS_HDRS}

  ## dev

  ## depends
  
  )
qt5_use_modules(dummy Core Widgets Gui Xml Sql)
