# Install script for directory: C:/Users/FuatAras/Desktop/Server/ClientLibGUI/source

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/SRO_DevKit")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "RelWithDebInfo")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("C:/Users/FuatAras/Desktop/Server/ClientLibGUI/cmake-build-default/source/libs/JMX_Library/BSLib/cmake_install.cmake")
  include("C:/Users/FuatAras/Desktop/Server/ClientLibGUI/cmake-build-default/source/libs/JMX_Library/GFX/GFX3DFunction/cmake_install.cmake")
  include("C:/Users/FuatAras/Desktop/Server/ClientLibGUI/cmake-build-default/source/libs/JMX_Library/GFX/FM/cmake_install.cmake")
  include("C:/Users/FuatAras/Desktop/Server/ClientLibGUI/cmake-build-default/source/libs/JMX_Library/GFX/GFXMainFrame/cmake_install.cmake")
  include("C:/Users/FuatAras/Desktop/Server/ClientLibGUI/cmake-build-default/source/libs/JMX_Library/SROInterfaceLib/cmake_install.cmake")
  include("C:/Users/FuatAras/Desktop/Server/ClientLibGUI/cmake-build-default/source/libs/ClientNet/cmake_install.cmake")
  include("C:/Users/FuatAras/Desktop/Server/ClientLibGUI/cmake-build-default/source/libs/ClientLib/cmake_install.cmake")
  include("C:/Users/FuatAras/Desktop/Server/ClientLibGUI/cmake-build-default/source/libs/NavMesh/cmake_install.cmake")
  include("C:/Users/FuatAras/Desktop/Server/ClientLibGUI/cmake-build-default/source/libs/SimpleViewer/cmake_install.cmake")
  include("C:/Users/FuatAras/Desktop/Server/ClientLibGUI/cmake-build-default/source/libs/MathHelpers/cmake_install.cmake")
  include("C:/Users/FuatAras/Desktop/Server/ClientLibGUI/cmake-build-default/source/libs/TypeId/cmake_install.cmake")
  include("C:/Users/FuatAras/Desktop/Server/ClientLibGUI/cmake-build-default/source/tests/cmake_install.cmake")
  include("C:/Users/FuatAras/Desktop/Server/ClientLibGUI/cmake-build-default/source/third-party/remodel/cmake_install.cmake")
  include("C:/Users/FuatAras/Desktop/Server/ClientLibGUI/cmake-build-default/source/third-party/memory/cmake_install.cmake")
  include("C:/Users/FuatAras/Desktop/Server/ClientLibGUI/cmake-build-default/source/third-party/imgui/cmake_install.cmake")
  include("C:/Users/FuatAras/Desktop/Server/ClientLibGUI/cmake-build-default/source/third-party/abi-testing/cmake_install.cmake")
  include("C:/Users/FuatAras/Desktop/Server/ClientLibGUI/cmake-build-default/source/third-party/ghidra/cmake_install.cmake")
  include("C:/Users/FuatAras/Desktop/Server/ClientLibGUI/cmake-build-default/source/DevKit_DLL/cmake_install.cmake")

endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "C:/Users/FuatAras/Desktop/Server/ClientLibGUI/cmake-build-default/source/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
