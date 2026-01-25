#pragma once

#include <memory>

namespace SimpleGL {

class SettingsManager : public std::enable_shared_from_this<SettingsManager> {
public:
    static std::shared_ptr<SettingsManager> create();
};

}
