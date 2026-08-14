#include "nodisconnect.hpp"
#include <bedrocktools/memory/Signatures.hpp>
#include <bedrocktools/sdk/Memory.hpp>
#include "core/memory/Hooks.hpp"

static bool (*original_isInEDUMultiplayerSession)(void* _this) = nullptr;
static void (*original_nativeSuspendGameplayUpdates)(void* env, void* activity, bool suspend) = nullptr;
static NoDisconnectModule* g_noDisconnectMod = nullptr;

static bool isInEDUMultiplayerSession_hook(void* _this) {
    if (g_noDisconnectMod && g_noDisconnectMod->enabled) {
        return true;
    }

    if (original_isInEDUMultiplayerSession) {
        return original_isInEDUMultiplayerSession(_this);
    }
    return false;
}

// Android/Bedrock calls this JNI entry point when the Activity is being
// backgrounded or resumed. When No Disconnect is enabled, keep the Bedrock
// gameplay-update suspension flag cleared. This is deliberately scoped to
// Minecraft's own suspend flag; it does not create a background thread and
// does not interfere with process termination (e.g. swiping the app away).
static void nativeSuspendGameplayUpdates_hook(void* env, void* activity, bool suspend) {
    (void)suspend;

    if (g_noDisconnectMod && g_noDisconnectMod->enabled) {
        if (original_nativeSuspendGameplayUpdates) {
            original_nativeSuspendGameplayUpdates(env, activity, false);
        }
        return;
    }

    if (original_nativeSuspendGameplayUpdates) {
        original_nativeSuspendGameplayUpdates(env, activity, suspend);
    }
}

NoDisconnectModule::NoDisconnectModule()
    : Module("No Disconnect", "Keeps Bedrock gameplay updates active while the app is backgrounded.") {
    g_noDisconnectMod = this;
}

void NoDisconnectModule::onInit() {
    if (m_patched) return;

    const auto eduAddr =
        bedrocktools::memory::resolve(bedrocktools::memory::SignatureId::EduMultiplayer);
    if (eduAddr != 0) {
        auto handle = bedrocktools::hooks::install(
            reinterpret_cast<void*>(eduAddr),
            reinterpret_cast<void*>(isInEDUMultiplayerSession_hook),
            reinterpret_cast<void**>(&original_isInEDUMultiplayerSession)
        );
        if (handle) {
            m_eduHook = handle;
        }
    }

    const auto suspendAddr =
        bedrocktools::memory::resolve(
            bedrocktools::memory::SignatureId::NativeSuspendGameplayUpdates
        );
    if (suspendAddr != 0) {
        auto handle = bedrocktools::hooks::install(
            reinterpret_cast<void*>(suspendAddr),
            reinterpret_cast<void*>(nativeSuspendGameplayUpdates_hook),
            reinterpret_cast<void**>(&original_nativeSuspendGameplayUpdates)
        );
        if (handle) {
            m_suspendHook = handle;
        }
    }

    m_patched = (m_eduHook != nullptr || m_suspendHook != nullptr);
}

void NoDisconnectModule::onEnable() {
}

void NoDisconnectModule::onDisable() {
}

