#pragma once
#include "math3d.h"

namespace cg {
inline constexpr Vec3 LAMP_BULB{-1.8f,5.8f,4.1f};
inline constexpr float LAMP_RADIUS=.32f;
inline float lampAttenuation(float distance){return 1/std::max(distance*distance,LAMP_RADIUS*LAMP_RADIUS);}
struct Lighting {
    const char* name;
    Vec3 direction,keyColor,lampPosition,lampColor,sky,ground,background;
    float exposure, softness;
};
// Studio/night: the visible lamp is the only direct source. Day/sunset add
// a distant sun. Sky/ground are low ambient terms, never specular rim lights.
// Lamp color is radiant intensity in scene units; irradiance falls as 1/r^2.
inline Lighting lightingPreset(int mode) {
    static const Lighting presets[]={
        {"STUDIO", {0,1,0}, {0,0,0}, LAMP_BULB, {85,73,55},
            {.20f,.22f,.25f}, {.065f,.07f,.08f}, {.018f,.028f,.042f}, 1.20f,1.0f},
        {"DAYLIGHT", {.65f,1.4f,.6f}, {1.8f,1.72f,1.55f}, LAMP_BULB, {26,23,18},
            {.20f,.25f,.32f}, {.045f,.04f,.035f}, {.11f,.16f,.21f}, 1.05f,1.0f},
        {"SUNSET", {-1.3f,.65f,.55f}, {2.1f,.85f,.28f}, LAMP_BULB, {42,34,24},
            {.09f,.08f,.12f}, {.024f,.016f,.02f}, {.055f,.025f,.035f}, 1.2f,1.0f},
        {"NIGHT", {0,1,0}, {0,0,0}, LAMP_BULB, {48,31,15},
            {.04f,.055f,.095f}, {.009f,.014f,.023f}, {.006f,.012f,.026f}, 1.25f,1.0f}
    };
    return presets[std::clamp(mode,0,3)];
}
}
