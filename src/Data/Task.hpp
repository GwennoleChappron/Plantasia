#pragma once
#include <string>

enum class TaskType {
    ARROSAGE,
    REMPOTAGE,
    TAILLE,
    RECOLTE,
    INFO
};

struct Task {
    std::string titre;
    std::string description;
    std::string nomPlante;
    TaskType type;
    int jour = 1;
    bool isCompleted = false; // Pour pouvoir cocher la case !
};