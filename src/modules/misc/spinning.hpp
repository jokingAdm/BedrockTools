#pragma once

#include "../Module.hpp"
#include <string>

class SpinningModule : public Module {
public:
    SpinningModule();
    ~SpinningModule() override;

    void onInit() override;
    void onEnable() override;
    void onDisable() override;
    void onFrame() override;

    void loadConfig(const nlohmann::json& j) override;
    void saveConfig(nlohmann::json& j) override;

private:
    bool m_leftSpinning = false;
    bool m_rightSpinning = false;

    int m_uinputFd = -1;
    bool m_deviceReady = false;
    bool m_leftDown = false;
    bool m_rightDown = false;

    bool createInputDevice();
    void destroyInputDevice();
    void syncKeys();
    bool emitKey(int code, bool down);
    void releaseKeys();

    static bool keyLeftSelected(bool left, bool right) { return left && !right; }
    static bool keyRightSelected(bool left, bool right) { return right && !left; }
};
