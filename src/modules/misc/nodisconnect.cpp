#include "nodisconnect.hpp"
#include <bedrocktools/memory/Signatures.hpp>
#include <bedrocktools/sdk/Memory.hpp>
#include "core/memory/Hooks.hpp"

static bool (*original_isInEDUMultiplayerSession)(void* _this) = nullptr;
static void (*original_nativeSuspendGameplayUpdates)(void* env, void* activity, bool suspend) = nullptr;
static NoDisconnectModule* g_noDisconnectMod = nullptr;

static void nativeSuspendGameplayUpdates_hook(void* env, void* activity, bool suspend) {
    // Bedrock calls this JNI entry point when Android backgrounds the activity.
    // Keep gameplay/network update scheduling enabled while No Disconnect is active.
    if (g_noDisconnectMod && g_noDisconnectMod->enabled) {
        suspend = false;
    }
    if (original_nativeSuspendGameplayUpdates) {
        original_nativeSuspendGameplayUpdates(env, activity, suspend);
    }
}

static bool isInEDUMultiplayerSession_hook(void* _this) {
    if (g_noDisconnectMod && g_noDisconnectMod->enabled) {
        return true;
    }
    
    if (original_isInEDUMultiplayerSession) {
        return original_isInEDUMultiplayerSession(_this);
    }
    return false;
}

NoDisconnectModule::NoDisconnectModule()
    : Module("No Disconnect", "Prevents you from being disconnected when you minimize the app.") {
    g_noDisconnectMod = this;
}

void NoDisconnectModule::onInit() {
    if (m_patchTarget) return;
    
    uintptr_t addr = bedrocktools::memory::resolve(bedrocktools::memory::SignatureId::EduMultiplayer);
    if (addr != 0) {
        m_patchTarget = (void*)addr;
        bedrocktools::hooks::install(m_patchTarget, (void*)isInEDUMultiplayerSession_hook, (void**)&original_isInEDUMultiplayerSession);
        m_patched = true;
    }

    // Also intercept the Android lifecycle callback that explicitly suspends
    // Minecraft gameplay updates while the Activity is backgrounded.
    uintptr_t suspendAddr = bedrocktools::memory::resolve(bedrocktools::memory::SignatureId::NativeSuspendGameplayUpdates);
    if (suspendAddr != 0) {
        bedrocktools::hooks::install(
            (void*)suspendAddr,
            (void*)nativeSuspendGameplayUpdates_hook,
            (void**)&original_nativeSuspendGameplayUpdates);
    }
}

void NoDisconnectModule::onEnable() {
}

void NoDisconnectModule::onDisable() {
}

