#include "settings_manager.h"

namespace SimpleGL {

std::shared_ptr<SettingsManager> SettingsManager::create() {
    auto instance = std::shared_ptr<SettingsManager>(new SettingsManager());

    return instance;
}

}