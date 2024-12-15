#include "settingsmanager.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include "widgets/counterwidget.h"

bool SettingsManager::loadGeneralSettings(const std::string& filePath, std::string& lastUsedPreset, const std::string& version) {
    std::ifstream inFile(filePath);
    if (!inFile) {
        std::cerr << "Settings file does not exist: " << filePath << std::endl;
        lastUsedPreset = "Default";
        return false; // Don't create a new file or overwrite here
    }
    else {
        std::cout << "Settings file loaded: " << filePath << std::endl;
    }

    std::string line, section;
    while (std::getline(inFile, line)) {
        if (line.empty() || line[0] == ';' || line[0] == '#') continue;

        if (line[0] == '[') {
            section = line.substr(1, line.find(']') - 1);
            continue;
        }

        std::istringstream keyValueStream(line);
        std::string key, value;
        if (std::getline(keyValueStream, key, '=') && std::getline(keyValueStream, value)) {
            if (section == "LastUsed" && key == "Preset") {
                lastUsedPreset = value;
            } else if (section == "Version" && key == "LastUsedVersion") {
                if (value != version) {
                    std::cerr << "Version mismatch: Expected " << version << ", found " << value << std::endl;
                    return false;
                }
            }
        }
    }

    return true;
}

void SettingsManager::saveGeneralSettings(const std::string& filePath, const std::string& lastUsedPreset, const std::string& version) {
    std::ofstream outFile(filePath);
    if (!outFile) {
        std::cerr << "Failed to save settings file: " << filePath << std::endl;
        return;
    }

    outFile << "[LastUsed]\nPreset=" << lastUsedPreset << "\n";
    outFile << "\n[Version]\nLastUsedVersion=" << version << "\n";
}

bool SettingsManager::loadWidgets(const std::string& filePath, const std::string& preset, std::vector<std::unique_ptr<Widget>>& widgets) {
    std::ifstream inFile(filePath);
    if (!inFile) {
        std::cerr << "Settings file does not exist: " << filePath << std::endl;
        return false;
    }

    std::string line, currentSection;
    while (std::getline(inFile, line)) {
        if (line.empty() || line[0] == ';' || line[0] == '#') continue;

        if (line[0] == '[') {
            currentSection = line.substr(1, line.find(']') - 1);
            continue;
        }

        if (currentSection == preset) {
            std::istringstream stream(line);
            std::string widgetType, name;
            float xPercentage, yPercentage;
            int modeInt;
            long long startTime;

            if (stream >> widgetType >> name >> xPercentage >> yPercentage >> modeInt >> startTime) {
                if (widgetType == "CounterWidget") {
                    widgets.emplace_back(std::make_unique<CounterWidget>(
                        name, xPercentage, yPercentage, static_cast<CounterWidget::CounterMode>(modeInt), startTime
                    ));
                } else {
                    std::cerr << "Unknown widget type: " << widgetType << std::endl;
                }
            }
        }
    }

    return !widgets.empty();
}

void SettingsManager::saveWidgets(const std::string& filePath, const std::string& preset, const std::vector<std::unique_ptr<Widget>>& widgets) {
    std::ofstream outFile(filePath, std::ios::app); // Append mode
    if (!outFile) {
        std::cerr << "Failed to save widgets to settings file: " << filePath << std::endl;
        return;
    }

    outFile << "\n[" << preset << "]\n";
    for (const auto& widget : widgets) {
        widget->save(outFile);
    }
}
