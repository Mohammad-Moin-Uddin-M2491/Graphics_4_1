#pragma once
#include "math3d.h"

namespace cg {
struct Lighting {
    const char* name;
    Vec3 direction, keyColor, fillPosition, fillColor, rimColor, sky, ground, background;
    float exposure, softness;
};
// Key light casts shadows. A weaker point fill and rear rim light reveal
// the shape of dark rubber and metal. These are authored studio light rigs.
inline Lighting lightingPreset(int mode) {
    static const Lighting presets[]={
        {"STUDIO", {-.45f,.85f,1.0f}, {2.7f,2.45f,2.1f}, {5,6,8}, {1.1f,1.55f,2.2f}, {.7f,1.1f,1.5f},
            {.12f,.17f,.23f}, {.025f,.032f,.04f}, {.018f,.028f,.042f}, 1.15f,1.5f},
        {"DAYLIGHT", {.65f,1.4f,.6f}, {3.0f,2.9f,2.65f}, {-7,7,9}, {.9f,1.2f,1.6f}, {.35f,.45f,.6f},
            {.28f,.35f,.44f}, {.065f,.06f,.05f}, {.11f,.16f,.21f}, 1.05f,1.2f},
        {"SUNSET", {-1.3f,.65f,.55f}, {3.6f,1.45f,.48f}, {6,5,8}, {.5f,.8f,1.5f}, {.9f,.35f,.15f},
            {.12f,.10f,.17f}, {.035f,.022f,.025f}, {.055f,.025f,.035f}, 1.2f,2.0f},
        {"NIGHT", {.3f,.7f,1.1f}, {.7f,1.25f,2.6f}, {-6,4,5}, {2.1f,.58f,.13f}, {.16f,.8f,1.5f},
            {.045f,.065f,.12f}, {.009f,.014f,.023f}, {.006f,.012f,.026f}, 1.4f,1.6f}
    };
    return presets[std::clamp(mode,0,3)];
}
}
