# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Users/jatan/esp/v5.2.2/esp-idf/components/bootloader/subproject"
  "C:/Users/jatan/Documents/GitHub/turn_systems/ESP32Wifi/ESP32IDF/project-name/build/bootloader"
  "C:/Users/jatan/Documents/GitHub/turn_systems/ESP32Wifi/ESP32IDF/project-name/build/bootloader-prefix"
  "C:/Users/jatan/Documents/GitHub/turn_systems/ESP32Wifi/ESP32IDF/project-name/build/bootloader-prefix/tmp"
  "C:/Users/jatan/Documents/GitHub/turn_systems/ESP32Wifi/ESP32IDF/project-name/build/bootloader-prefix/src/bootloader-stamp"
  "C:/Users/jatan/Documents/GitHub/turn_systems/ESP32Wifi/ESP32IDF/project-name/build/bootloader-prefix/src"
  "C:/Users/jatan/Documents/GitHub/turn_systems/ESP32Wifi/ESP32IDF/project-name/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Users/jatan/Documents/GitHub/turn_systems/ESP32Wifi/ESP32IDF/project-name/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Users/jatan/Documents/GitHub/turn_systems/ESP32Wifi/ESP32IDF/project-name/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
