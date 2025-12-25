cmake_minimum_required(VERSION 3.20)
project(DevPlanner VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui Network)

set(DevPlanner_SOURCES
    src/main.cpp
    src/core/storage.cpp
    src/ai/ai_action_registry.cpp
    src/ui/glassmorphism_widget.cpp
    src/ui/modern_button.cpp
    src/ui/task_node.cpp
    src/ui/node_canvas.cpp
    src/ui/ai_chat_panel.cpp
    src/ui/main_window.cpp
    src/ui/live_background.cpp
)

set(DevPlanner_HEADERS
    src/core/config.hpp
    src/core/storage.hpp
    src/ai/ai_action.hpp
    src/ai/ai_action_registry.hpp
    src/ai/actions/create_task_action.hpp
    src/ai/actions/create_chain_action.hpp
    src/ai/actions/connect_action.hpp
    src/ai/actions/connect_many_action.hpp
    src/ai/actions/disconnect_action.hpp
    src/ai/actions/set_status_action.hpp
    src/ai/actions/set_many_status_action.hpp
    src/ai/actions/rename_action.hpp
    src/ai/actions/set_description_action.hpp
    src/ai/actions/delete_action.hpp
    src/ai/actions/delete_many_action.hpp
    src/ai/actions/clear_all_action.hpp
    src/ai/actions/arrange_action.hpp
    src/ui/glassmorphism_widget.hpp
    src/ui/modern_button.hpp
    src/ui/task_node.hpp
    src/ui/node_canvas.hpp
    src/ui/ai_chat_panel.hpp
    src/ui/main_window.hpp
    src/ui/live_background.hpp
)

add_executable(DevPlanner ${DevPlanner_SOURCES} ${DevPlanner_HEADERS})

target_include_directories(DevPlanner PRIVATE ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(DevPlanner PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    Qt6::Network
)

# macOS specific settings
if(APPLE)
    set(MACOSX_BUNDLE_ICON_FILE app.icns)
    set(APP_ICON_MACOSX ${CMAKE_SOURCE_DIR}/resources/app.icns)
    set_source_files_properties(${APP_ICON_MACOSX} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    target_sources(DevPlanner PRIVATE ${APP_ICON_MACOSX})
    set_target_properties(DevPlanner PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.devplanner.app"
        MACOSX_BUNDLE_BUNDLE_NAME "Dev Planner"
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
        MACOSX_BUNDLE_ICON_FILE app.icns
    )
endif()

# Windows specific settings
if(WIN32)
    set_target_properties(DevPlanner PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()
