#pragma once

#include <Rgb.h>
#include <Point.h>

#include <cstdint>
#include <functional>

class GridComponent {
public:
    template<typename T>
    void set_hsv(const Point<T>& hsv);
public:
    Point<size_t> top_left_high = {0,0,0};
    Point<size_t> size = {1,1,1};
    Point<float> position_velocity = {0,0,0};
    Point<float> rgb_velocity = {0,0,0};
    bool kill_when_dead = true;
    bool visible = false;
    Rgb center_color = {0, 0, 0};
    std::function<Rgb (const Point&)> color_getter = [=]{return center_color;};
};