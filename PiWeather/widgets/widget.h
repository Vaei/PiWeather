#pragma once

#include <string>
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <memory>
#include <utility>
#include <vector>
#include <cmath>
#include <SDL2/SDL.h>

// Abstract Widget Base Class
class Widget {
protected:
    std::string name;
    float xPercentage;
    float yPercentage;

public:
    Widget(const std::string& name, float xPercentage, float yPercentage)
        : name(name), xPercentage(xPercentage), yPercentage(yPercentage) {}

    virtual ~Widget() = default;

    virtual void render(SDL_Renderer* renderer) = 0;

    virtual void save(std::ofstream& outFile) const = 0;

    virtual void load(std::ifstream& inFile) = 0;
};
