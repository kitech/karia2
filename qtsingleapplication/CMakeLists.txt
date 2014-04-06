add_library(qtsingleapplication
  ./qtsingleapplication.cpp
  ./qtlocalpeer.cpp
  )

qt5_use_modules(qtsingleapplication Core Network Widgets)
