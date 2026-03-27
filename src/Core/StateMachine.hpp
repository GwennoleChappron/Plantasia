#pragma once
#include <memory>
#include <stack>
#include "State.hpp"

class StateMachine {
private:
    std::stack<std::unique_ptr<State>> m_states;
    std::unique_ptr<State> m_newState;
    bool m_isReplacing;
    bool m_isRemoving;
    bool m_isClearing;

public:
    StateMachine();
    
    void addState(std::unique_ptr<State> newState, bool isReplacing = true);
    void removeState();
    void clearStates();
    void processStateChanges(); // Fait les changements de façon sécurisée entre deux frames

    State* getActiveState();
};