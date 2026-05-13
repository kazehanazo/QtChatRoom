cmake_minimum_required(VERSION 3.16)

project(QTChatScoket VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Sql Network Multimedia MultimediaWidgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Sql Network Multimedia MultimediaWidgets)

set(PROJECT_SOURCES
        main.cpp
        MyMainWindow.cpp
        MyMainWindow.h
        MyMainWindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Widgets)

    qt_add_executable(QTChatScoket
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        Login.h Login.cpp Login.ui
        Icons.qrc
        Videos.qrc
        Register.h Register.cpp Register.ui
        DisplayAnimation.h DisplayAnimation.cpp
        SocketBusiness.h SocketBusiness.cpp
        UserInfoSDK.h UserInfoSDK.cpp
        MessageModel.h
        MessageBubbleWidget.h MessageBubbleWidget.cpp
        UserSearchModel.h UserSearchModel.cpp
        MessageStore.h MessageStore.cpp
        ChatBubbleWidget.h ChatBubbleWidget.cpp
        ChatStore.h ChatStore.cpp
        FloatingScrollBar.h FloatingScrollBar.cpp
        FriendApplyBubbleWidget.h FriendApplyBubbleWidget.cpp
        FriendApplyStore.h FriendApplyStore.cpp
        UserInfoBubbleWidget.h UserInfoBubbleWidget.cpp
        SettingsDialog.h SettingsDialog.cpp SettingsDialog.ui
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET AIChat APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(QTChatScoket SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(QTChatScoket
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(QTChatScoket PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
                        Qt${QT_VERSION_MAJOR}::Sql
                        Qt${QT_VERSION_MAJOR}::Network
                        Qt${QT_VERSION_MAJOR}::Multimedia
                        Qt${QT_VERSION_MAJOR}::MultimediaWidgets)
target_link_libraries(QTChatScoket PRIVATE Qt6::Core)
target_link_libraries(QTChatScoket PRIVATE Qt6::Core)
target_link_libraries(QTChatScoket PRIVATE Qt6::Widgets)
target_link_libraries(QTChatScoket PRIVATE Qt6::Widgets)
target_link_libraries(QTChatScoket PRIVATE Qt6::Core)
target_link_libraries(QTChatScoket PRIVATE Qt6::Widgets)
target_link_libraries(QTChatScoket PRIVATE Qt6::Core)
target_link_libraries(QTChatScoket PRIVATE Qt6::Core)
target_link_libraries(QTChatScoket PRIVATE Qt6::Core Qt6::Widgets)
target_link_libraries(QTChatScoket PRIVATE Qt6::Core)
target_link_libraries(QTChatScoket PRIVATE Qt6::Widgets)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.AIChat)
endif()
set_target_properties(QTChatScoket PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS QTChatScoket
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(QTChatScoket)
endif()
