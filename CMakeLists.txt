project(kde-thumbnailer-apk)
cmake_minimum_required(VERSION 2.8.12)

set(SHARED_MIME_INFO_MINIMUM_VERSION "0.30")

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)
include(FeatureSummary)

find_package(Qt5 REQUIRED COMPONENTS Widgets)
find_package(KF5 REQUIRED COMPONENTS KIO Archive)

find_package(SharedMimeInfo REQUIRED)

set(apkthumbnail_SRCS apkcreator.cpp)

add_library(apkthumbnail MODULE ${apkthumbnail_SRCS})
target_link_libraries(apkthumbnail PRIVATE
     Qt5::Widgets KF5::KIOWidgets KF5::Archive
)

add_executable(apkthumbnail_test test.cpp apkcreator.cpp)
target_link_libraries(apkthumbnail_test
     Qt5::Widgets KF5::KIOWidgets KF5::Archive
)

install(TARGETS apkthumbnail DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES apkthumbnail.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES apk.xml DESTINATION ${XDG_MIME_INSTALL_DIR})

update_xdg_mimetypes(${XDG_MIME_INSTALL_DIR})
