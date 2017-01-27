project(kde-thumbnailer-apk)
cmake_minimum_required(VERSION 2.6)

find_package(KDE4 REQUIRED)
set(SHARED_MIME_INFO_MINIMUM_VERSION "0.30")
find_package(SharedMimeInfo REQUIRED)

include(KDE4Defaults)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(${QT_INCLUDES} ${KDE4_INCLUDES})

set(apkthumbnail_SRCS apkcreator.cpp)

kde4_add_plugin(apkthumbnail ${apkthumbnail_SRCS})
target_link_libraries(apkthumbnail
    ${KDE4_KIO_LIBS}
)

install(TARGETS apkthumbnail DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES apkthumbnail.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES apk.xml DESTINATION ${XDG_MIME_INSTALL_DIR})
update_xdg_mimetypes(${XDG_MIME_INSTALL_DIR})
