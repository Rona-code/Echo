cmake_minimum_required(VERSION 3.16)

project(Echo
    VERSION 2.0.0
    LANGUAGES CXX
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -------------------------
# Version appli (unique)
# -------------------------
set(APP_VERSION "2.0.1")
string(REPLACE "." "," APP_VERSION_COMMA "${APP_VERSION}")
add_definitions(-DAPP_VERSION="${APP_VERSION}")

# ------------------------- 
# Génération du fichier RC 
# ------------------------- 
if (WIN32) 
    string(REPLACE "\\" "/" CMAKE_SOURCE_DIR_FIXED "${CMAKE_SOURCE_DIR}")
    configure_file( 
        ${CMAKE_SOURCE_DIR}/app.rc.in 
        ${CMAKE_BINARY_DIR}/app.rc 
        @ONLY 
    ) 
    # Copie l'icône dans le build dir pour que le .rc la trouve
    configure_file(
        ${CMAKE_SOURCE_DIR}/resources/Icone.ico
        ${CMAKE_BINARY_DIR}/resources/Icone.ico
        COPYONLY
    )
endif()

# -------------------------
# Qt Widgets (statique)
# -------------------------
find_package(Qt6 REQUIRED COMPONENTS Widgets)

# -------------------------
# Sources / Headers
# -------------------------
file(GLOB_RECURSE SOURCES
    "${CMAKE_SOURCE_DIR}/src/commun/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/commun/security/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/ui/components/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/ui/dialogs/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/ui/windows/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/engine/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/managers/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/physics/*.cpp"
)

file(GLOB_RECURSE HEADERS
    "${CMAKE_SOURCE_DIR}/src/commun/*.h"
    "${CMAKE_SOURCE_DIR}/src/commun/security/*.h"
    "${CMAKE_SOURCE_DIR}/src/ui/components/*.h"
    "${CMAKE_SOURCE_DIR}/src/ui/dialogs/*.h"
    "${CMAKE_SOURCE_DIR}/src/ui/windows/*.h"
    "${CMAKE_SOURCE_DIR}/src/core/engine/*.h"
    "${CMAKE_SOURCE_DIR}/src/core/managers/*.h"
    "${CMAKE_SOURCE_DIR}/src/core/physics/*.h"
)

# -------------------------
# Exécutable
# -------------------------
qt_add_executable(${PROJECT_NAME}
    src/main.cpp
    ${SOURCES}
    ${HEADERS}
)

# -------------------------
# Copie Asset
# -------------------------
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/resources
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources
)

# -------------------------
# Ressources Qt (Windows + Linux)
# -------------------------
qt_add_resources(${PROJECT_NAME} "app_resources"
    PREFIX "/"
    FILES
        resources/Icone.png
        resources/style/menu.qss
        resources/translations/echo_fr.qm
        resources/translations/echo_en.qm
        resources/translations/echo_ja.qm
        resources/Renard/renards_Idle0.png
        resources/Renard/renards_Idle1.png
        resources/Renard/renards_sleep0.png
        resources/Renard/renards_sleep1.png
        resources/Renard/renards_walk0.png
        resources/Renard/renards_walk1.png
)

# -------------------------
# Ressources Windows (.rc)
# -------------------------
if (WIN32)
    # Force le compilateur de ressources
    set_source_files_properties(${CMAKE_BINARY_DIR}/app.rc
        PROPERTIES
            LANGUAGE RC
    )
    target_sources(${PROJECT_NAME} PRIVATE app.rc "src/commun/security/LockManager.cpp" "src/commun/security/LockManager.h")
    set_target_properties(${PROJECT_NAME}
        PROPERTIES
            WIN32_EXECUTABLE TRUE
    )
endif()

# -------------------------
# Includes
# -------------------------
target_include_directories(${PROJECT_NAME}
    PRIVATE
        
)

# -------------------------
# Link
# -------------------------
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt6::Widgets
        
)

# -------------------------
# Link Windows
# -------------------------
if (WIN32)
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
        dwmapi
    )
endif()

# -------------------------
# Link Linux
# -------------------------
if (UNIX AND NOT APPLE)
    find_package(X11 REQUIRED)
    target_link_libraries(${PROJECT_NAME} 
        PRIVATE 
            ${X11_LIBRARIES}
    )
endif()