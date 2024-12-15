#pragma once

#include <string>
#include <vector>
#include <memory>
#include "widgets/widget.h"

class SettingsManager {
public:
    // Load general settings
    static bool loadGeneralSettings(const std::string& filePath, std::string& lastUsedPreset, const std::string& version);

    // Save general settings
    static void saveGeneralSettings(const std::string& filePath, const std::string& lastUsedPreset, const std::string& version);

    // Load widgets
    static bool loadWidgets(const std::string& filePath, const std::string& preset, std::vector<std::unique_ptr<Widget>>& widgets);

    // Save widgets
    static void saveWidgets(const std::string& filePath, const std::string& preset, const std::vector<std::unique_ptr<Widget>>& widgets);
};
