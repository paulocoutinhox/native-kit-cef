# Copyright (c) 2014 The Chromium Embedded Framework Authors. All rights
# reserved. Use of this source code is governed by a BSD-style license that
# can be found in the LICENSE file.

#
# Source files.
#

# project sources.
set(MY_PROJECT_NAME nkcef)

set(PROJECT_SRCS
  simple_app.cc
  simple_app.h
  simple_handler.cc
  simple_handler.h
  custom_scheme.cc
  custom_scheme.h
  )
set(PROJECT_SRCS_LINUX
  cefsimple_linux.cc
  simple_handler_linux.cc
  )
set(PROJECT_SRCS_MACOSX
  cefsimple_mac.mm
  simple_handler_mac.mm
  )
set(PROJECT_SRCS_WINDOWS
  cefsimple.exe.manifest
  cefsimple.rc
  cefsimple_win.cc
  resource.h
  simple_handler_win.cc
  )
APPEND_PLATFORM_SOURCES(PROJECT_SRCS)
source_group(${MY_PROJECT_NAME} FILES ${PROJECT_SRCS})

set(PROJECT_RES_SRCS_WINDOWS
  res/icon.ico
  res/small-icon.ico
  )
APPEND_PLATFORM_SOURCES(PROJECT_RES_SRCS)
source_group(${MY_PROJECT_NAME}\\\\res FILES ${PROJECT_RES_SRCS})

set(PROJECT_SRCS
  ${PROJECT_SRCS}
  ${PROJECT_RES_SRCS}
  )

# native-kit-cef helper sources.
set(PROJECT_HELPER_SRCS_MACOSX
  process_helper_mac.cc
  )
APPEND_PLATFORM_SOURCES(PROJECT_HELPER_SRCS)
source_group(${MY_PROJECT_NAME} FILES ${PROJECT_HELPER_SRCS})

# native-kit-cef html resources.
set(PROJECT_RESOURCES_RESOURCES_SRCS
  resources/html/index.html
  resources/html/bootstrap.bundle.js
  resources/html/favicon.png
  resources/html/polyfills.bundle.js
  resources/html/vendor.bundle.css
  resources/html/vendor.bundle.js
  resources/html/assets/images/4da1d23dc947be4f977bd54928ca7f17.png
  )
source_group(${MY_PROJECT_NAME}\\\\resources\\\\html FILES ${CEFCLIENT_RESOURCES_RESOURCES_SRCS})

# native-kit-cef resources.
set(PROJECT_RESOURCES_MAC_SRCS_MACOSX
  resources/mac/Info.plist
  resources/mac/icon.icns
  )
APPEND_PLATFORM_SOURCES(PROJECT_RESOURCES_MAC_SRCS)
source_group(${MY_PROJECT_NAME}\\\\resources\\\\mac FILES ${PROJECT_RESOURCES_MAC_SRCS})

set(PROJECT_RESOURCES_MAC_ENGLISH_LPROJ_SRCS_MACOSX
  resources/mac/English.lproj/InfoPlist.strings
  resources/mac/English.lproj/MainMenu.xib
  )
APPEND_PLATFORM_SOURCES(PROJECT_RESOURCES_MAC_ENGLISH_LPROJ_SRCS)
source_group(${MY_PROJECT_NAME}\\\\resources\\\\mac\\\\English.lproj FILES ${PROJECT_RESOURCES_MAC_ENGLISH_LPROJ_SRCS})

set(PROJECT_RESOURCES_SRCS
  ${PROJECT_RESOURCES_RESOURCES_SRCS}
  ${PROJECT_RESOURCES_MAC_SRCS}
  ${PROJECT_RESOURCES_MAC_ENGLISH_LPROJ_SRCS}
  )

#
# Shared configuration.
#

# Target executable names.
set(CEF_TARGET "${MY_PROJECT_NAME}")
if(OS_MACOSX)
  set(CEF_HELPER_TARGET "${MY_PROJECT_NAME} Helper")
endif()

# Logical target used to link the libcef library.
ADD_LOGICAL_TARGET("libcef_lib" "${CEF_LIB_DEBUG}" "${CEF_LIB_RELEASE}")

# Determine the target output directory.
SET_CEF_TARGET_OUT_DIR()


#
# Linux configuration.
#

if(OS_LINUX)
  # Executable target.
  add_executable(${CEF_TARGET} ${PROJECT_SRCS})
  SET_EXECUTABLE_TARGET_PROPERTIES(${CEF_TARGET})
  add_dependencies(${CEF_TARGET} libcef_dll_wrapper)
  target_link_libraries(${CEF_TARGET} libcef_lib libcef_dll_wrapper ${CEF_STANDARD_LIBS})

  # Set rpath so that libraries can be placed next to the executable.
  set_target_properties(${CEF_TARGET} PROPERTIES INSTALL_RPATH "$ORIGIN")
  set_target_properties(${CEF_TARGET} PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
  set_target_properties(${CEF_TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CEF_TARGET_OUT_DIR})

  # Copy binary and resource files to the target output directory.
  COPY_FILES("${CEF_TARGET}" "${CEF_BINARY_FILES}" "${CEF_BINARY_DIR}" "${CEF_TARGET_OUT_DIR}")
  COPY_FILES("${CEF_TARGET}" "${CEF_RESOURCE_FILES}" "${CEF_RESOURCE_DIR}" "${CEF_TARGET_OUT_DIR}")

  # Set SUID permissions on the chrome-sandbox target.
  SET_LINUX_SUID_PERMISSIONS("${CEF_TARGET}" "${CEF_TARGET_OUT_DIR}/chrome-sandbox")
endif()


#
# Mac OS X configuration.
#

if(OS_MACOSX)
  # Output paths for the app bundles.
  set(CEF_APP "${CEF_TARGET_OUT_DIR}/${CEF_TARGET}.app")
  set(CEF_HELPER_APP "${CEF_TARGET_OUT_DIR}/${CEF_HELPER_TARGET}.app")

  # Variable referenced from Info.plist files.
  set(PRODUCT_NAME "${CEF_TARGET}")

  # Helper executable target.
  add_executable(${CEF_HELPER_TARGET} MACOSX_BUNDLE ${PROJECT_HELPER_SRCS})
  SET_EXECUTABLE_TARGET_PROPERTIES(${CEF_HELPER_TARGET})
  add_dependencies(${CEF_HELPER_TARGET} libcef_dll_wrapper)
  target_link_libraries(${CEF_HELPER_TARGET} libcef_lib libcef_dll_wrapper ${CEF_STANDARD_LIBS})
  set_target_properties(${CEF_HELPER_TARGET} PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/resources/mac/helper-Info.plist
    )

  # Fix the framework rpath in the helper executable.
  FIX_MACOSX_HELPER_FRAMEWORK_RPATH(${CEF_HELPER_TARGET})

  # Main executable target.
  add_executable(${CEF_TARGET} MACOSX_BUNDLE ${PROJECT_RESOURCES_SRCS} ${PROJECT_SRCS})
  SET_EXECUTABLE_TARGET_PROPERTIES(${CEF_TARGET})
  add_dependencies(${CEF_TARGET} libcef_dll_wrapper "${CEF_HELPER_TARGET}")
  target_link_libraries(${CEF_TARGET} libcef_lib libcef_dll_wrapper ${CEF_STANDARD_LIBS})
  set_target_properties(${CEF_TARGET} PROPERTIES
    RESOURCE "${PROJECT_RESOURCES_SRCS}"
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/resources/mac/Info.plist
    )

  # Copy files into the main app bundle.
  add_custom_command(
    TARGET ${CEF_TARGET}
    POST_BUILD
    # Copy the helper app bundle into the Frameworks directory.
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CEF_HELPER_APP}"
            "${CEF_APP}/Contents/Frameworks/${CEF_HELPER_TARGET}.app"
    # Copy the CEF framework into the Frameworks directory.
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CEF_BINARY_DIR}/Chromium Embedded Framework.framework"
            "${CEF_APP}/Contents/Frameworks/Chromium Embedded Framework.framework"
    VERBATIM
    )

  # Fix the framework rpath in the main executable.
  FIX_MACOSX_MAIN_FRAMEWORK_RPATH(${CEF_TARGET})

  if(NOT ${CMAKE_GENERATOR} STREQUAL "Xcode")
    # Manually process and copy over resource files.
    # The Xcode generator handles this via the set_target_properties RESOURCE directive.
    set(PREFIXES "mac/")  # Remove these prefixes from input file paths.
    COPY_MACOSX_RESOURCES("${PROJECT_RESOURCES_SRCS}" "${PREFIXES}" "${CEF_TARGET}" "${CMAKE_CURRENT_SOURCE_DIR}" "${CEF_APP}")
  endif()
endif()


#
# Windows configuration.
#

if(OS_WINDOWS)
  # Executable target.
  add_executable(${CEF_TARGET} WIN32 ${PROJECT_SRCS})
  add_dependencies(${CEF_TARGET} libcef_dll_wrapper)
  SET_EXECUTABLE_TARGET_PROPERTIES(${CEF_TARGET})
  target_link_libraries(${CEF_TARGET} libcef_lib libcef_dll_wrapper ${CEF_STANDARD_LIBS})

  if(USE_SANDBOX)
    # Logical target used to link the cef_sandbox library.
    ADD_LOGICAL_TARGET("cef_sandbox_lib" "${CEF_SANDBOX_LIB_DEBUG}" "${CEF_SANDBOX_LIB_RELEASE}")
    target_link_libraries(${CEF_TARGET} cef_sandbox_lib ${CEF_SANDBOX_STANDARD_LIBS})
  endif()

  # Add the custom manifest files to the executable.
  ADD_WINDOWS_MANIFEST("${CMAKE_CURRENT_SOURCE_DIR}" "${CEF_TARGET}" "exe")

  # Copy binary and resource files to the target output directory.
  COPY_FILES("${CEF_TARGET}" "${CEF_BINARY_FILES}" "${CEF_BINARY_DIR}" "${CEF_TARGET_OUT_DIR}")
  COPY_FILES("${CEF_TARGET}" "${CEF_RESOURCE_FILES}" "${CEF_RESOURCE_DIR}" "${CEF_TARGET_OUT_DIR}")
endif()
