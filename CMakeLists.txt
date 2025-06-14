cmake_minimum_required(VERSION 3.13)
cmake_policy(SET CMP0048 NEW)
cmake_policy(SET CMP0042 NEW)
cmake_policy(SET CMP0028 NEW)
cmake_policy(SET CMP0077 NEW)

enable_language(C)
enable_language(CXX)
set(CMAKE_CXX_STANDARD 11)
if(APPLE)
set (CMAKE_OSX_ARCHITECTURE arm64)
endif()

if (MSVC)
  option(LITESQL_MSVC_MT "link with /MT instead of /MD (use dlls versus static linked)" ON) 
  # see http://msdn.microsoft.com/en-us/library/2kzt1wy3%28VS.71%29.aspx   
  if (LITESQL_MSVC_MT)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  else(LITESQL_MSVC_MT)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL$<$<CONFIG:Debug>:Debug>")
  endif (LITESQL_MSVC_MT)
  
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /INCREMENTAL:NO")
  add_compile_options(/W4) # for now disabled /WX
  set_property(DIRECTORY APPEND PROPERTY
               COMPILE_DEFINITIONS _CRT_SECURE_NO_WARNINGS)
else(MSVC)
  add_compile_options(-Wall -Wextra -Wpedantic -Wunused-variable -Wno-long-long)
endif(MSVC)
if(APPLE)
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -arch arm64")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -arch arm64")
endif(APPLE)
if(APPLE)
  set(CPACK_GENERATOR DragNDrop ${CPACK_GENERATOR})
endif(APPLE)
if (UNIX AND NOT APPLE AND NOT WIN32)
  set(CPACK_GENERATOR DEB ${CPACK_GENERATOR})
endif (UNIX AND NOT APPLE AND NOT WIN32)
set(CPACK_SOURCE_GENERATOR TGZ ZIP)
set(CPACK_PACKAGE_CONTACT gulliver@users.sourceforge.net)
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_NAME})
set(CPACK_PACKAGE_VENDOR "litesql Development Team")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_SOURCE_IGNORE_FILES "${CMAKE_CURRENT_BINARY_DIR}/;/docs/html;/autom4te.cache;/build/*;/win32/;/.svn/;/.git/;~$;${CPACK_SOURCE_IGNORE_FILES}")
set(CPACK_PACKAGE_EXECUTABLES Visual-Litesql;Visual-Litesql;${PROJECT_NAME}-gen;${PROJECT_NAME}-gen)
include(CPack)
# Include all the necessary files for macros
include(FetchContent)
include (${CMAKE_ROOT}/Modules/CheckFunctionExists.cmake)
include (${CMAKE_ROOT}/Modules/CheckIncludeFile.cmake)
include (${CMAKE_ROOT}/Modules/CheckIncludeFileCXX.cmake)
include (${CMAKE_ROOT}/Modules/CheckIncludeFiles.cmake)
include (${CMAKE_ROOT}/Modules/CheckLibraryExists.cmake)
include (${CMAKE_ROOT}/Modules/CheckSymbolExists.cmake)

set(PROJECT_NAME "litesql" VERSION 0.3.19)

include(cmake/FindMySQL.cmake)
include(cmake/FindPostgreSQL.cmake)
include(cmake/FindSQLite3.cmake)
include(cmake/FindOCILib.cmake)

option(LITESQL_WITH_BACKEND_PLUGINS "build with database plugins in shared libraries" ON)
option(LITESQL_WITH_MYSQL "build with mysql Backend" OFF)
option(LITESQL_WITH_SQLITE3 "build with sqlite3 backend(sources integrated)" ON)
option(LITESQL_WITH_POSTGRES "build with postgres backend" OFF)
option(LITESQL_WITH_OCILIB "build with ocilib Backend" OFF)
option(LITESQL_WITH_ODBC "build with ODBC Backend" OFF)
option(LITESQL_WITH_DOCS "generate Documentation" OFF)
option(LITESQL_WITH_UI "build also GUI-Version of generator" OFF)
option(LITESQL_WITH_TESTS "enable testing" ON)
option(LITESQL_USE_SYSTEM_EXPAT "use expat lib provided by system" ON)
option(CMAKE_SUPPRESS_REGENERATION "suppress rules to re-run cmake on changes (warning: no dependency checks!)?" OFF)

list(APPEND BACKEND_LIBRARIES ${CMAKE_DL_LIBS})
set(BACKEND_DL_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/src/library)

if (LITESQL_WITH_SQLITE3)
  set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS HAVE_LIBSQLITE3)
  option(LITESQL_USE_SYSTEM_SQLITE "use sqlite3 lib provided by system" ON)
  list(APPEND BACKEND_LIBRARIES litesql_sqlite)
endif (LITESQL_WITH_SQLITE3)

if (LITESQL_WITH_MYSQL AND MYSQL_FOUND)
  set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS HAVE_LIBMYSQLCLIENT)
  list(APPEND BACKEND_INCLUDES ${MYSQL_INCLUDE_DIR})
  list(APPEND BACKEND_LIBRARIES litesql_mysql ${MYSQL_LIBRARIES})
  list(APPEND BACKEND_DEFINITIONS ${MYSQL_DEFINITIONS})
  
  if (MSVC)
    # the precompiled mysql client libraries are compiled with libcmt 
    # - this causes duplicate symbols
    # if MySQL Connector is older than this then LIBCMT will be disabled
    # C Connector 6.1 works fine with VS 2013 WITH LIBCMT, 
    # I haven't tested older versions
    set(deprecated_connector_threshold "MySQL Connector C 6.0")
    # now to figure out the mysql C connector version number
    string(REGEX MATCH "MySQL Connector C [0-9]+\\.[0-9]+" curr_version ${MYSQL_INCLUDE_DIR})
    if(${curr_version} STRGREATER ${deprecated_connector_threshold})
      message("MySQL backend: LIBCMT enabled")
    else(${curr_version} STRGREATER ${deprecated_connector_threshold})
      message("MySQL backend: LIBCMT disabled")
      set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:LIBCMT")
    endif(${curr_version} STRGREATER ${deprecated_connector_threshold})
  endif (MSVC)
  
else(LITESQL_WITH_MYSQL AND MYSQL_FOUND)
  message("can not use mysql")
  set(LITESQL_WITH_MYSQL OFF)
endif (LITESQL_WITH_MYSQL AND MYSQL_FOUND)

if (LITESQL_WITH_OCILIB AND OCILIB_FOUND)
  set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS HAVE_OCILIB)
  list(APPEND BACKEND_INCLUDES ${OCILIB_INCLUDE_DIR})
  list(APPEND BACKEND_LIBRARIES litesql_ocilib ${OCILIB_LIBRARIES})
  list(APPEND BACKEND_DEFINITIONS ${OCILIB_DEFINITIONS})
  # the precompiled ocilib client libraries are compiled with libcmt - this causes duplicate symbols
  if (MSVC)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:LIBCMT")
  endif(MSVC)
else(LITESQL_WITH_OCILIB AND OCILIB_FOUND)
  message("can not use ocilib")
  set(LITESQL_WITH_OCILIB OFF)
endif (LITESQL_WITH_OCILIB AND OCILIB_FOUND)

if (LITESQL_WITH_POSTGRES AND POSTGRESQL_FOUND)
  set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS HAVE_LIBPQ)
  list(APPEND BACKEND_INCLUDES ${POSTGRESQL_INCLUDE_DIR})
  list(APPEND BACKEND_LIBRARIES litesql_postgresql ${POSTGRESQL_LIBRARIES})
  list(APPEND BACKEND_DEFINITIONS ${POSTGRESQL_DEFINITIONS})
else (LITESQL_WITH_POSTGRES AND POSTGRESQL_FOUND)
  message("can not use postgres")
  set(LITESQL_WITH_POSTGRES OFF)
endif (LITESQL_WITH_POSTGRES AND POSTGRESQL_FOUND)

if (LITESQL_WITH_ODBC)
  set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS HAVE_ODBC)
  message(ODBC is not supported yet)
endif (LITESQL_WITH_ODBC)

if (LITESQL_WITH_DOCS)
  add_subdirectory(docs/doxygen)
endif(LITESQL_WITH_DOCS)

if(MSVC)
  set(CMAKE_FIND_LIBRARY_PREFIXES "")
  set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")
  set(CMAKE_STATIC_LIBRARY_PREFIX "")
  set(CMAKE_STATIC_LIBRARY_SUFFIX ".lib")
  set(CMAKE_SHARED_LIBRARY_PREFIX "")         # lib
  set(CMAKE_SHARED_LIBRARY_SUFFIX ".dll")         # .so
  set(CMAKE_IMPORT_LIBRARY_PREFIX "")
  set(CMAKE_IMPORT_LIBRARY_SUFFIX ".lib")
  set(CMAKE_EXECUTABLE_SUFFIX ".exe")         # .exe
  set(CMAKE_LINK_LIBRARY_SUFFIX ".lib")
  set(CMAKE_DL_LIBS "")
  set(CMAKE_DEBUG_POSTFIX "d") 
    link_directories(${CMAKE_CURRENT_SOURCE_DIR}/win32/lib/)
endif(MSVC)

check_include_files(dlfcn.h HAVE_DLFCN_H)
check_include_files(inttypes.h HAVE_INTTYPES_H)
check_include_files(memory.h HAVE_MEMORY_H)
check_include_files(stdint.h HAVE_STDINT_H)
check_include_files(stdlib.h HAVE_STDLIB_H)
check_include_files(strings.h HAVE_STRINGS_H)
check_include_files(string.h HAVE_STRING_H)
check_include_files(sys/stat.h HAVE_SYS_STAT_H)
check_include_files(sys/types.h HAVE_SYS_TYPES_H)
check_include_files(unistd.h HAVE_UNISTD_H)


check_function_exists(malloc HAVE_MALLOC)
check_function_exists(gettimeofday HAVE_GETTIMEOFDAY)
check_function_exists(atof HAVE_ATOF)
check_function_exists(strtoll HAVE_STRTOLL)
check_function_exists(strtof  HAVE_STRTOF)
check_function_exists(localtime_r HAVE_LOCALTIME_R)


# create config.h
configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake 
    ${CMAKE_CURRENT_BINARY_DIR}/config.h)

# set include directories
get_filename_component(CONFIG_INCLUDE ${CMAKE_CURRENT_BINARY_DIR}/config.h PATH)
include_directories(include ${CONFIG_INCLUDE})

install(DIRECTORY include/ DESTINATION include 
                          COMPONENT devel 
                          FILES_MATCHING PATTERN "*.hpp"
                          PATTERN .svn EXCLUDE 
)

install(DIRECTORY include/ DESTINATION include 
                          COMPONENT devel 
                          FILES_MATCHING PATTERN "*.h"
                          PATTERN .svn EXCLUDE 
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/config.h DESTINATION include/litesql
          COMPONENT devel)

add_subdirectory(src/library)
add_subdirectory(src/generator)

add_subdirectory(src/examples)

if(LITESQL_WITH_TESTS)
  enable_testing()
  add_subdirectory(src/tests)
endif(LITESQL_WITH_TESTS)
 
project( ${PROJECT_NAME} )

export(EXPORT litesql-util)
install(EXPORT litesql DESTINATION litesql.cmake)
export(EXPORT model)
install(EXPORT model DESTINATION model.cmake)

include(InstallRequiredSystemLibraries)

set(CPACK_GENERATOR TGZ ZIP)
if(WIN32 AND NOT CYGWIN)
  set(CPACK_GENERATOR NSIS ${CPACK_GENERATOR})
endif(WIN32 AND NOT CYGWIN)
if(APPLE)
  set(CPACK_GENERATOR DragNDrop ${CPACK_GENERATOR})
endif(APPLE)
if (UNIX AND NOT APPLE AND NOT WIN32)
  set(CPACK_GENERATOR DEB ${CPACK_GENERATOR})
endif (UNIX AND NOT APPLE AND NOT WIN32)

set(CPACK_SOURCE_PACKAGE_FILE_NAME ${PROJECT_NAME}-src-${PROJECT_VERSION})
set(CPACK_SOURCE_GENERATOR TGZ ZIP)
set(CPACK_PACKAGE_CONTACT gulliver@users.sourceforge.net) 
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_NAME})
set(CPACK_PACKAGE_VENDOR "litesql Development Team")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_SOURCE_IGNORE_FILES "${CMAKE_CURRENT_BINARY_DIR}/;/docs/html;/autom4te.cache;/build/*;/win32/;/.svn/;/.git/;~$;${CPACK_SOURCE_IGNORE_FILES}")

if(WIN32 AND NOT UNIX)
  set(CPACK_PACKAGE_INSTALL_DIRECTORY "${PROJECT_NAME}\\\\${PROJECT_VERSION}")
  # There is a bug in NSI that does not handle full unix paths properly. Make
  # sure there is at least one set of four (4) backlashes.
  # set(CPACK_PACKAGE_ICON "${CMake_SOURCE_DIR}/Utilities/Release\\\\InstallIcon.bmp")
  set(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\${PROJECT_NAME}.exe")
  set(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} ${PROJECT_NAME}")
  set(CPACK_NSIS_HELP_LINK "http://litesql.sourceforge.net/")
  set(CPACK_NSIS_URL_INFO_ABOUT "http://litesql.sourceforge.net/")
  set(CPACK_NSIS_CONTACT "gulliver@users.sourceforge.net")
  set(CPACK_NSIS_MODIFY_PATH ON)
  set(CPACK_NSIS_MENU_LINKS "doc/html/index.html;Documentation")
else(WIN32 AND NOT UNIX)
  set(CPACK_PACKAGE_INSTALL_DIRECTORY "${PROJECT_NAME}/${PROJECT_VERSION}")
  set(CPACK_STRIP_FILES "bin/${PROJECT_NAME}-gen")
  set(CPACK_SOURCE_STRIP_FILES "")
endif(WIN32 AND NOT UNIX)
set(CPACK_PACKAGE_EXECUTABLES Visual-Litesql;Visual-Litesql;${PROJECT_NAME}-gen;${PROJECT_NAME}-gen)
include(CPack)

set(ARCHIVE_NAME ${CMAKE_PROJECT_NAME}-src-${PROJECT_VERSION})
add_custom_target(dist
    COMMAND git archive --prefix=${ARCHIVE_NAME}/ HEAD
        | bzip2 > ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.tar.bz2
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
