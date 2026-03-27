cmake_minimum_required(VERSION 3.14)
project(Plantasia)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# --- SFML ---
FetchContent_Declare(sfml GIT_REPOSITORY https://github.com/SFML/SFML.git GIT_TAG 2.6.1)
FetchContent_MakeAvailable(sfml)

# --- ImGui ---
FetchContent_Declare(imgui GIT_REPOSITORY https://github.com/ocornut/imgui.git GIT_TAG v1.90.4)
FetchContent_MakeAvailable(imgui)

# --- Sources ---
# Le _RECURSE est magique : il fouille dans src/Core, src/States, src/Data, etc.
file(GLOB_RECURSE SOURCES "src/*.cpp")

set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    "external/imgui-sfml/imgui-SFML.cpp" 
)

add_executable(Plantasia ${SOURCES} ${IMGUI_SOURCES})

# --- LES CHEMINS D'INCLUSION ---
target_include_directories(Plantasia PUBLIC 
    src                              # <--- L'AJOUT CRUCIAL pour faire #include "Core/State.hpp"
    external                         # Pour trouver nlohmann/json.hpp
    external/imgui-sfml              # Pour trouver imgui-SFML.h
    ${imgui_SOURCE_DIR}              # Pour trouver imgui.h
    ${imgui_SOURCE_DIR}/backends     # Pour certains headers imgui
)

# --- Liens ---
find_package(OpenGL REQUIRED)

# On lie d'abord toutes les librairies communes à tous les systèmes
target_link_libraries(Plantasia 
    sfml-graphics 
    sfml-window 
    sfml-audio 
    sfml-network
    sfml-system 
    OpenGL::GL
)

# On ajoute sfml-main UNIQUEMENT si on est sur Windows
if(WIN32)
    target_link_libraries(Plantasia sfml-main)
endif()