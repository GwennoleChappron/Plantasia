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
file(GLOB_RECURSE SOURCES "src/*.cpp")

set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    "external/imgui-sfml/imgui-SFML.cpp" 
)

# NOUVEAU : On n'ajoute glad.c que si on n'est PAS sur Mac
if (NOT APPLE)
    set(GLAD_SOURCES "external/glad/src/glad.c")
else()
    set(GLAD_SOURCES "")
endif()

# On compile avec GLAD seulement si la variable le contient
add_executable(Plantasia ${SOURCES} ${IMGUI_SOURCES} ${GLAD_SOURCES})

# --- LES CHEMINS D'INCLUSION ---
target_include_directories(Plantasia PUBLIC 
    src                                
    external                           
    external/imgui-sfml                
    ${imgui_SOURCE_DIR}                
    ${imgui_SOURCE_DIR}/backends       
)

if (NOT APPLE)
    message(STATUS "[SunGPU] Compute shaders activés (non-macOS)")
    target_compile_definitions(Plantasia PRIVATE SUNGPU_HAS_COMPUTE)
    # J'ai retiré "target_link_libraries(Plantasia glad)" car on compile déjà glad.c juste au-dessus !
    target_include_directories(Plantasia PUBLIC external/glad/include)
else()
    message(STATUS "[SunGPU] CPU fallback (macOS)")
endif()            

# --- Liens ---
find_package(OpenGL REQUIRED)

target_link_libraries(Plantasia 
    sfml-graphics 
    sfml-window 
    sfml-audio 
    sfml-network
    sfml-system 
    OpenGL::GL
)

if(WIN32)
    target_link_libraries(Plantasia sfml-main)
endif()