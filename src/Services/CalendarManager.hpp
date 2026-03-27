#pragma once
#include <vector>
#include "Data/Task.hpp"
#include "Data/UserBalcony.hpp"
#include "Services/DatabaseManager.hpp"

class CalendarManager {
private:
    std::vector<Task> m_currentTasks;
    int m_currentMonth; // De 1 à 12

public:
    CalendarManager();
    
    // La fonction magique qui analyse ton balcon
    void generateTasks(const UserBalcony& balcony, const DatabaseManager& db, int currentMonth);
    
    std::vector<Task>& getTasks() { return m_currentTasks; }
    int getCurrentMonth() const { return m_currentMonth; }
};