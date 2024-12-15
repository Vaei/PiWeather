#include "counterwidget.h"
#include <chrono>
#include <iostream>
#include <iomanip>

CounterWidget::CounterWidget(const std::string& name, float x, float y, CounterMode mode, long long startTime)
    : Widget(name, x, y), mode(mode), startTime(startTime) {}

void CounterWidget::render(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_Rect rect = {
        static_cast<int>(xPercentage * 1024),
        static_cast<int>(yPercentage * 600),
        150,
        50
    };
    SDL_RenderFillRect(renderer, &rect);

    int count = calculateElapsedCount();
    std::string modeStr;
    switch (mode) {
        case CounterMode::Seconds: modeStr = "Seconds"; break;
        case CounterMode::Minutes: modeStr = "Minutes"; break;
        case CounterMode::Hours:   modeStr = "Hours"; break;
        case CounterMode::Days:    modeStr = "Days"; break;
    }

    std::cout << name << " : " << count << " " << modeStr << " elapsed.\n";
}

long long CounterWidget::calculateElapsedMilliseconds() const {
    auto now = std::chrono::system_clock::now();
    auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    return nowMs - startTime;
}

int CounterWidget::calculateElapsedCount() const {
    long long elapsedMs = calculateElapsedMilliseconds();
    switch (mode) {
        case CounterMode::Seconds: return elapsedMs / 1000;
        case CounterMode::Minutes: return elapsedMs / (1000 * 60);
        case CounterMode::Hours:   return elapsedMs / (1000 * 60 * 60);
        case CounterMode::Days:    return elapsedMs / (1000 * 60 * 60 * 24);
    }
    return 0;
}

void CounterWidget::save(std::ofstream& outFile) const {
    outFile << "CounterWidget " << name << " " << xPercentage << " " << yPercentage << " "
            << static_cast<int>(mode) << " " << startTime << "\n";
}

void CounterWidget::load(std::ifstream& inFile) {
    int modeInt;
    inFile >> xPercentage >> yPercentage >> modeInt >> startTime;
    mode = static_cast<CounterMode>(modeInt);
}
