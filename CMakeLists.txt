cmake_minimum_required(VERSION 3.16)

set(PROJECT_NAME cutefish-settings)
project(${PROJECT_NAME})

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找 Qt6 包
set(QT Core Gui Widgets Quick QuickControls2 DBus Xml Concurrent LinguistTools)
find_package(Qt6 REQUIRED ${QT})

find_package(X11 REQUIRED)
find_package(Freetype REQUIRED)
find_package(PkgConfig REQUIRED)

# 修正 Libcrypt 的查找方式
find_package(PkgConfig REQUIRED)
pkg_check_modules(Libcrypt REQUIRED libcrypt)
# 或者使用 find_library 替代
# find_library(Libcrypt_LIBRARIES crypt)
# if(NOT Libcrypt_LIBRARIES)
#     message(FATAL_ERROR "libcrypt library not found")
# endif()

# 更新到 KF6
find_package(KF6NetworkManagerQt REQUIRED)
find_package(KF6ModemManagerQt REQUIRED)
find_package(KF6Config REQUIRED)

pkg_search_module(FontConfig REQUIRED fontconfig IMPORTED_TARGET)
pkg_search_module(ICU REQUIRED icu-i18n)

include_directories(${ICU_INCLUDE_DIRS})

set(SRCS
    src/cscreenmanager.cpp
    src/cscreenoutput.cpp
    src/application.cpp
    src/main.cpp
    src/appearance.cpp
    src/battery.cpp
    src/brightness.cpp
    src/fontsmodel.cpp
    src/language.cpp
    src/about.cpp
    src/background.cpp
    src/notifications.cpp
    src/password.cpp
    src/batteryhistorymodel.cpp
    src/cicontheme.cpp
    src/fonts/kxftconfig.cpp
    src/fonts/fonts.cpp
    src/powermanager.cpp
    src/cursor/cursorthememodel.cpp
    src/cursor/cursortheme.cpp
    src/cursor/mouse.cpp
    src/datetime/time.cpp
    src/datetime/timezonedata.h
    src/datetime/timezonemap.cpp
    src/datetime/timedated_interface.cpp
    src/touchpad.cpp
    src/vpn/vpn.cpp
    src/vpn/nm-l2tp-service.h
    src/vpn/nm-openvpn-service.h
    src/vpn/nm-pptp-service.h
    src/networkproxy.cpp
    src/defaultapplications.cpp
    src/desktopproperties.cpp
#    src/update/updatemodel.cpp
)

set(RESOURCES
    src/resources.qrc
)

qt6_add_dbus_adaptor(DBUS_SOURCES
                    src/com.cutefish.SettingsUI.xml
                    src/application.h Application)
set_source_files_properties(${DBUS_SOURCES} PROPERTIES SKIP_AUTOGEN ON)

add_executable(${PROJECT_NAME} ${DBUS_SOURCES} ${SRCS} ${RESOURCES})

target_link_libraries(${PROJECT_NAME}
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::DBus
    Qt6::Xml
    Qt6::Concurrent
    Qt6::CorePrivate  # 这会自动处理私有头文件路径
    Qt6::GuiPrivate   # 这会自动处理私有头文件路径

    KF6::NetworkManagerQt
    KF6::ModemManagerQt
    KF6::ConfigCore

    PkgConfig::FontConfig
    ${FREETYPE_LIBRARIES}
    ${ICU_LDFLAGS}
    ${Libcrypt_LIBRARIES}  # 修改这里

    ${X11_LIBRARIES}
    X11::X11
    X11::Xi
    X11::Xcursor
)

# 如果使用 pkg_check_modules，还需要添加包含目录
if(Libcrypt_FOUND)
    target_include_directories(${PROJECT_NAME} PRIVATE ${Libcrypt_INCLUDE_DIRS})
endif()

# 设置需要扫描翻译字符串的源文件
set(TRANSLATION_SOURCES
    ${SRCS}
    ${DBUS_SOURCES}
    # 如果有其他包含翻译字符串的文件，也添加在这里
)

# 确保 TS_FILES 不为空
file(GLOB TS_FILES translations/*.ts)
if(NOT TS_FILES)
    message(WARNING "No translation files found in translations/ directory")
    # 创建一个空的 QM_FILES 变量，避免后续错误
    set(QM_FILES "")
else()
    # 使用正确的 qt6_create_translation 命令
    qt6_create_translation(QM_FILES 
        ${TRANSLATION_SOURCES}
        ${TS_FILES}
    )
endif()

# 只有当有翻译文件时才创建翻译目标
if(TS_FILES)
    add_custom_target(translations ALL DEPENDS ${QM_FILES})
    add_dependencies(${PROJECT_NAME} translations)
endif()

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(FILES
    cutefish-settings.desktop
    DESTINATION /usr/share/applications/
    COMPONENT Runtime
)

install(FILES ${QM_FILES} DESTINATION /usr/share/${PROJECT_NAME}/translations)