#include "StateMachine.hpp"

StateMachine::StateMachine() : m_isReplacing(false), m_isRemoving(false), m_isClearing(false) {}

void StateMachine::addState(std::unique_ptr<State> newState, bool isReplacing) {
    m_isReplacing = isReplacing;
    m_newState = std::move(newState);
}

void StateMachine::removeState() { m_isRemoving = true; }
void StateMachine::clearStates() { m_isClearing = true; }

void StateMachine::processStateChanges() {
    if (m_isRemoving && !m_states.empty()) {
        m_states.top()->onExit();
        m_states.pop();
        if (!m_states.empty()) m_states.top()->onEnter();
        m_isRemoving = false;
    }

    if (m_isClearing) {
        while (!m_states.empty()) {
            m_states.top()->onExit();
            m_states.pop();
        }
        m_isClearing = false;
    }

    if (m_newState) {
        if (m_isReplacing && !m_states.empty()) {
            m_states.top()->onExit();
            m_states.pop();
        } else if (!m_states.empty()) {
            m_states.top()->onExit(); // Met en pause l'état précédent
        }
        m_states.push(std::move(m_newState));
        m_states.top()->onEnter();
        m_isReplacing = false;
    }
}

State* StateMachine::getActiveState() {
    if (m_states.empty()) return nullptr;
    return m_states.top().get();
}