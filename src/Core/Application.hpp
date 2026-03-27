#pragma once
#include <SFML/Graphics.hpp>
#include "StateMachine.hpp"
#include "VFX/BackgroundEngine.hpp"
#include "States/StateMainMenu.hpp"
#include "Services/DatabaseManager.hpp"
#include "Data/UserBalcony.hpp"
#include "Services/CalendarManager.hpp"

class Application {
private:
    std::unique_ptr<BackgroundEngine> m_background;
    sf::RenderWindow m_window;
    StateMachine m_stateMachine;
    sf::Clock m_clock;
    DatabaseManager m_database;
    UserBalcony m_userBalcony;
    CalendarManager m_calendar;
    
    void initStyleImGui(); // Notre fameux style vert

public:
    Application();
    ~Application();

    void run();

    // Getters pour que les "States" puissent manipuler l'appli
    sf::RenderWindow& getWindow() { return m_window; }
    StateMachine& getStateMachine() { return m_stateMachine; }
    DatabaseManager& getDatabase() { return m_database; }
    UserBalcony& getUserBalcony() { return m_userBalcony; }
    CalendarManager& getCalendar() { return m_calendar; }
};