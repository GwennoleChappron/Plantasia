cmake_minimum_required(VERSION 3.10)

# Nom du projet et version
project(Plantasia VERSION 1.0 LANGUAGES CXX)

# On demande du C++17 (minimum pour les enum class modernes et le futur JSON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Inclusion du dossier où se trouvent tes fichiers .hpp
include_directories(include)

# Liste des fichiers sources
# On sépare le main du reste pour plus de clarté
set(SOURCES 
    src/Plante.cpp 
    main.cpp
)

# Création de l'exécutable
add_executable(${PROJECT_NAME} ${SOURCES})

# Optionnel : Afficher un message de succès à la fin de la config
message(STATUS "Configuration terminée pour ${PROJECT_NAME}")