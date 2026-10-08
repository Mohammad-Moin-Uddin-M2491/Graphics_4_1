# Automotive Chain Reaction - current project draft

**Mohammad Moin Uddin Moin | 2107111 | Computer Graphics**

Status: **not submitted**. The student confirmed that the instructor approved using a YouTube chain-reaction video as conceptual inspiration; an identical recreation is unnecessary. This document describes the implemented automotive project. It is not a claim of alignment with an earlier marble-machine proposal.

## Objective

Demonstrate primitive modeling, transformations, lighting, Gouraud/Phong shading, timed animation and meaningful user interaction in an OpenGL 3.3 mechanical scene. A 70-second sequence places a starter pin, winds gears, releases a rolling wheel, indexes a wiper, trips a bar, pumps a ratchet rack, withdraws a safety bolt, releases a counterweight, raises a barrier, lowers a drawbridge and cranks a starter before releasing a drivable car.

Presentation mode progresses automatically. Interactive mode activates one ready mechanism at a time with F and rejects out-of-order actions. The user can move the camera independently with mouse and keyboard, change light rigs, compare shading and inspect each object's motion. Two optional features are implemented: a primitive glove for a designated pin, and a separate CPU ray-traced snapshot.

## Requirements checklist

| Requirement | Status and evidence |
|---|---|
| Existing project stays buildable | Implemented; same MinGW/GLFW/GLAD setup and executable, no new libraries |
| Triangles, cubes, spheres | Implemented; procedural primitive geometry throughout |
| Free camera and overview | Implemented; mouse look, WASD/QE, Shift, dolly, 1 overview |
| Keyboard car after release | Implemented; arrow acceleration/reverse/steering, distance-based wheel spin |
| Five controllable moving objects | Implemented; 12 separately activated stages, dependency checks and prompt |
| At least 60 seconds before car | Implemented; 70 active seconds; real-time GPU measurement in VALIDATION.md |
| Continuous meaningful action | Implemented; winding, indexed sweeps, four rack advances, latch, weight, bridge, starter |
| Pause/reset repeatability | Implemented; deterministic poses, stage waits, full car reset |
| Genuine Gouraud and Phong | Implemented; vertex versus fragment lighting, same lights/materials, L comparison |
| Ambient/diffuse/specular and normals | Implemented; shared reflection-vector formula and inverse-transpose normals |
| Lighting button | Implemented; four presets, brightness and actual shadow-map toggle |
| Recognizable car and mechanical supports | Implemented; primitive cabin/body/wheels/windows, axles, mounts, rods, cables, rails |
| Optional glove | Implemented; one pin, local attachment during transport, socket placement |
| Optional ray tracing | Implemented as an 800-pixel-wide still; triangle intersections, shadow rays, one reflection bounce |
| Full physics / engineering accuracy | Outside scope; controlled state transitions, selected geometric constraints |
| Arbitrary glove selection / animated ray tracing | Outside scope; designated pin and offline snapshot only |

See README.md for the full timing, controls and transformation tables. See VIVA.md for the demonstration script and explanations, and VALIDATION.md for checks actually run. The earlier working source/documentation are archived in backups/before-final-requirements-2026-10-05.zip.
