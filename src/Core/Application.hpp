#pragma once
#include <SFML/Graphics.hpp>
#include "StateMachine.hpp"
#include "VFX/BackgroundEngine.hpp"
#include "States/StateMainMenu.hpp"
#include "Services/DataBaseManager.hpp"
#include "Services/DataBaseManagerV2.hpp"
#include "Data/UserBalcony.hpp"
#include "Services/CalendarManager.hpp"

class Application {
private:
    std::unique_ptr<BackgroundEngine> m_background;
    sf::RenderWindow m_window;
    StateMachine     m_stateMachine;
    sf::Clock        m_clock;
    DatabaseManager  m_database;
    DatabaseManagerV2 m_databaseV2;
    UserBalcony      m_userBalcony;
    CalendarManager  m_calendar;

    void initStyleImGui();

public:
    Application();
    ~Application();

    void run();

    // Getters pour les States
    sf::RenderWindow& getWindow()          { return m_window; }
    StateMachine&     getStateMachine()    { return m_stateMachine; }
    DatabaseManager&  getDatabase()        { return m_database; }
    UserBalcony&      getUserBalcony()     { return m_userBalcony; }
    CalendarManager&  getCalendar()        { return m_calendar; }

    // Accès au background engine depuis les States
    // Exemple d'usage dans un State :
    //   m_app->getBackground().triggerWindBurst();
    //   m_app->getBackground().applyPreset(BackgroundPreset::ZEN);
    BackgroundEngine& getBackground()      { return *m_background; }
};
