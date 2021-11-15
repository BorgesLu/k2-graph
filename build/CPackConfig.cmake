# This file will be configured to contain variables for CPack. These variables
# should be set in the CMake list file of the project before CPack module is
# included. The list of available CPACK_xxx variables and their associated
# documentation may be obtained using
#  cpack --help-variable-list
#
# Some variables are common to all generators (e.g. CPACK_PACKAGE_NAME)
# and some are specific to a generator
# (e.g. CPACK_NSIS_EXTRA_INSTALL_COMMANDS). The generator specific variables
# usually begin with CPACK_<GENNAME>_xxxx.


set(CPACK_BINARY_7Z "")
set(CPACK_BINARY_BUNDLE "")
set(CPACK_BINARY_CYGWIN "")
set(CPACK_BINARY_DEB "OFF")
set(CPACK_BINARY_DRAGNDROP "")
set(CPACK_BINARY_FREEBSD "OFF")
set(CPACK_BINARY_IFW "OFF")
set(CPACK_BINARY_NSIS "OFF")
set(CPACK_BINARY_NUGET "")
set(CPACK_BINARY_OSXX11 "")
set(CPACK_BINARY_PACKAGEMAKER "")
set(CPACK_BINARY_PRODUCTBUILD "")
set(CPACK_BINARY_RPM "OFF")
set(CPACK_BINARY_STGZ "ON")
set(CPACK_BINARY_TBZ2 "OFF")
set(CPACK_BINARY_TGZ "ON")
set(CPACK_BINARY_TXZ "OFF")
set(CPACK_BINARY_TZ "ON")
set(CPACK_BINARY_WIX "")
set(CPACK_BINARY_ZIP "")
set(CPACK_BUILD_SOURCE_DIRS "/home/kvgroup/lmx/k2-graph;/home/kvgroup/lmx/k2-graph/build")
set(CPACK_CMAKE_GENERATOR "Unix Makefiles")
set(CPACK_COMPONENTS_ALL "common;console;graph;meta;storage;tool")
set(CPACK_COMPONENTS_GROUPING "ALL_COMPONENTS_IN_ONE")
set(CPACK_COMPONENT_UNSPECIFIED_HIDDEN "TRUE")
set(CPACK_COMPONENT_UNSPECIFIED_REQUIRED "TRUE")
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "/home/kvgroup/lmx/k2-graph/package/postinst")
set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "A distributed, scalable, lightning-fast graph database.")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/vesoft-inc/nebula/releases")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "vesoft inc")
set(CPACK_DEB_COMPONENT_INSTALL "YES")
set(CPACK_DEFAULT_PACKAGE_DESCRIPTION_FILE "/usr/share/cmake-3.16/Templates/CPack.GenericDescription.txt")
set(CPACK_GENERATOR "STGZ;TGZ;TZ")
set(CPACK_INSTALL_CMAKE_PROJECTS "/home/kvgroup/lmx/k2-graph/build;Nebula Graph;ALL;/")
set(CPACK_INSTALL_PREFIX "/usr/local/nebula")
set(CPACK_MODULE_PATH "/home/kvgroup/lmx/k2-graph/cmake;/usr/local/lib/cmake/Seastar")
set(CPACK_NSIS_DISPLAY_NAME "nebula-graph 0.1.1")
set(CPACK_NSIS_INSTALLER_ICON_CODE "")
set(CPACK_NSIS_INSTALLER_MUI_ICON_CODE "")
set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES")
set(CPACK_NSIS_PACKAGE_NAME "nebula-graph 0.1.1")
set(CPACK_OUTPUT_CONFIG_FILE "/home/kvgroup/lmx/k2-graph/build/CPackConfig.cmake")
set(CPACK_PACKAGE_CONTACT "Nebula Graph")
set(CPACK_PACKAGE_DEFAULT_LOCATION "/")
set(CPACK_PACKAGE_DESCRIPTION "Nebula Graph")
set(CPACK_PACKAGE_DESCRIPTION_FILE "/usr/share/cmake-3.16/Templates/CPack.GenericDescription.txt")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Nebula Graph built using CMake")
set(CPACK_PACKAGE_FILE_NAME "nebula-graph-0.1.1-Linux")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "nebula-graph 0.1.1")
set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "nebula-graph 0.1.1")
set(CPACK_PACKAGE_NAME "nebula-graph")
set(CPACK_PACKAGE_RELOCATABLE "true")
set(CPACK_PACKAGE_VENDOR "vesoft inc")
set(CPACK_PACKAGE_VERSION "0.1.1")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "1")
set(CPACK_PACKAGE_VERSION_PATCH "1")
set(CPACK_PACKAGING_INSTALL_PREFIX "/usr/local/nebula")
set(CPACK_RESOURCE_FILE_LICENSE "/usr/share/cmake-3.16/Templates/CPack.GenericLicense.txt")
set(CPACK_RESOURCE_FILE_README "/usr/share/cmake-3.16/Templates/CPack.GenericDescription.txt")
set(CPACK_RESOURCE_FILE_WELCOME "/usr/share/cmake-3.16/Templates/CPack.GenericWelcome.txt")
set(CPACK_RPM_COMPONENT_INSTALL "YES")
set(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION "/usr/local")
set(CPACK_RPM_PACKAGE_ARCHITECTURE "x86_64")
set(CPACK_RPM_PACKAGE_DESCRIPTION "A distributed, scalable, lightning-fast graph database.")
set(CPACK_RPM_PACKAGE_GROUP "vesoft inc")
set(CPACK_RPM_PACKAGE_LICENSE "Apache 2.0 + Common Clause 1.0")
set(CPACK_RPM_PACKAGE_RELOCATABLE "ON")
set(CPACK_RPM_PACKAGE_URL "https://github.com/vesoft-inc/nebula/releases")
set(CPACK_RPM_POST_INSTALL_SCRIPT_FILE "/home/kvgroup/lmx/k2-graph/package/rpm_postinst")
set(CPACK_RPM_SPEC_MORE_DEFINE "%define debug_package %{nil}
            %define __os_install_post %{nil}")
set(CPACK_SET_DESTDIR "OFF")
set(CPACK_SOURCE_7Z "")
set(CPACK_SOURCE_CYGWIN "")
set(CPACK_SOURCE_GENERATOR "TBZ2;TGZ;TXZ;TZ")
set(CPACK_SOURCE_OUTPUT_CONFIG_FILE "/home/kvgroup/lmx/k2-graph/build/CPackSourceConfig.cmake")
set(CPACK_SOURCE_RPM "OFF")
set(CPACK_SOURCE_TBZ2 "ON")
set(CPACK_SOURCE_TGZ "ON")
set(CPACK_SOURCE_TXZ "ON")
set(CPACK_SOURCE_TZ "ON")
set(CPACK_SOURCE_ZIP "OFF")
set(CPACK_SYSTEM_NAME "Linux")
set(CPACK_TOPLEVEL_TAG "Linux")
set(CPACK_WIX_SIZEOF_VOID_P "8")

if(NOT CPACK_PROPERTIES_FILE)
  set(CPACK_PROPERTIES_FILE "/home/kvgroup/lmx/k2-graph/build/CPackProperties.cmake")
endif()

if(EXISTS ${CPACK_PROPERTIES_FILE})
  include(${CPACK_PROPERTIES_FILE})
endif()

# Configuration for component "common"

SET(CPACK_COMPONENTS_ALL common console graph meta storage tool)
set(CPACK_COMPONENT_COMMON_GROUP common)

# Configuration for component "graph"

SET(CPACK_COMPONENTS_ALL common console graph meta storage tool)
set(CPACK_COMPONENT_GRAPH_GROUP graph)
set(CPACK_COMPONENT_GRAPH_DEPENDS common)

# Configuration for component "storage"

SET(CPACK_COMPONENTS_ALL common console graph meta storage tool)
set(CPACK_COMPONENT_STORAGE_GROUP storage)
set(CPACK_COMPONENT_STORAGE_DEPENDS common)

# Configuration for component "meta"

SET(CPACK_COMPONENTS_ALL common console graph meta storage tool)
set(CPACK_COMPONENT_META_GROUP meta)
set(CPACK_COMPONENT_META_DEPENDS common)

# Configuration for component "console"

SET(CPACK_COMPONENTS_ALL common console graph meta storage tool)
set(CPACK_COMPONENT_CONSOLE_GROUP console)

# Configuration for component "tool"

SET(CPACK_COMPONENTS_ALL common console graph meta storage tool)
set(CPACK_COMPONENT_TOOL_GROUP tool)

# Configuration for component group "common"

# Configuration for component group "graph"

# Configuration for component group "storage"

# Configuration for component group "meta"

# Configuration for component group "console"

# Configuration for component group "tool"
