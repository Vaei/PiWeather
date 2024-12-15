#pragma once

#include "widget.h"
#include <chrono>
#include <string>

class CounterWidget : public Widget {
public:
    enum class CounterMode { Seconds, Minutes, Hours, Days };

private:
    CounterMode mode;
    long long startTime; // Unix timestamp in milliseconds

public:
    CounterWidget(const std::string& name, float xPercentage, float yPercentage, CounterMode mode, long long startTime);

    void render(SDL_Renderer* renderer) override;
    void save(std::ofstream& outFile) const override;
    void load(std::ifstream& inFile) override;

    long long calculateElapsedMilliseconds() const;
    int calculateElapsedCount() const;
};
