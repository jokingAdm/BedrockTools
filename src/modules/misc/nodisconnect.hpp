#pragma once

#include "../Module.hpp"
#include "core/memory/Hooks.hpp"

class NoDisconnectModule : public Module {
public:
    NoDisconnectModule();
    void onInit() override;
    void onEnable() override;
    void onDisable() override;

private:
    bool m_patched = false;
    bedrocktools::hooks::Handle m_eduHook = nullptr;
    bedrocktools::hooks::Handle m_suspendHook = nullptr;
};
