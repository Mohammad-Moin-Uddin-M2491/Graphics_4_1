# Validation of the contact and presentation revision

Date: 9 October 2026 (Asia/Dhaka)

**Later lighting correction:** see [LIGHTING_REVIEW.md](LIGHTING_REVIEW.md) for current lighting/projection tests and render evidence. The contact-revision counts below describe the earlier checkpoint.

## Build and focused checks

`mingw32-make test` passed with C++17, -O2, -Wall and -Wextra, without compiler warnings.

- **17 nominal and 17 invalid handoff fixtures** exercise actual boundary guards. Invalid fixtures include a ball short of its cup, an obstructing finger, a short striker swing, a missed collar, a pin still blocking the drum, a raised bridge and an uncleared starter/shoe. Each failed guard stops the production Simulation state, rejects subsequent input and clears on reset.
- **293 focused geometry/controller/UI assertions** include those fixtures, measured contact gaps, routed cable length and tangent continuity, socket engagement, roof/tire clearance, stage controls, held-key release, quarter turns, pause/reset, camera/car controllers, honest button labels/actions and six guide tabs.
- **9,241 time samples / 189,137 repeated numeric checks** cover continuity, gear ratio, rod length, cable travel, rolling distance and parent attachment. These numbers are reported separately. They are not distinct visual tests.
- Timings in poseAt() come from STAGE_TIME and relative substage fractions. Every runtime boundary is checked even when a long frame crosses multiple stages.

## GPU and actual-time results

Intel Iris Xe Graphics; OpenGL 3.3.0; driver 31.0.101.4502.

`--verify-interactive` rendered all mechanisms and keyboard-driven exit in **420 frames**. It exercises registered callbacks, pauses held operations midway, resumes them, and performs four Z taps with key-repeat rejection. Active release time: **70.0000 seconds** under synthetic verification steps.

`--verify-timing` uses actual GLFW elapsed time, without an assumed frame rate:

```text
RELEASE: active=70.0078 s, wall=70.0078 s, frame=10272
PASS: full 17-action / 70-second chain + 7-second car exit; 11153 GPU frames; mode, stage, camera, pause/reset, guide and shading handlers
```

The authored threshold is exactly 70.000 seconds; 70.0078 is the first rendered frame at or beyond it. Every runtime guard passed and the car completed its departure at 77 seconds. No OpenGL errors were reported.

## Actual rendered review

**68 framebuffer captures**, including 51 before/at/after handoff frames, were generated from the timeline. Every handoff trio and all six larger guide pages were inspected. Full-size PNGs, four review sheets, filenames/times/cameras in frames.csv, and individual observations are in [HANDOFF_REVIEW.md](HANDOFF_REVIEW.md).

Gouraud/Phong/Flat frames share the same pose and lighting. The optional ray tracer is retained; its rendering algorithm was not changed in this revision. The current geometry is shared with it. This revision's reported GPU timing and visual coverage refer to live OpenGL, not a new ray-tracing performance benchmark.

## Scope and packaging

The evidence combines focused faults, repeated invariants, direct registered-handler exercises, complete rendering and sampled visual inspection. It does not claim general collision response, dynamic spring forces, rope tension, full engine physics or human testing of every gesture. Projector legibility was checked at the normal window resolution, not on the actual classroom projector.

The clean submission is assembled from an explicit allowlist: one executable, GLFW and required MinGW runtime DLLs, source/headers/link libraries, build files, current documents and the current review set. No old main.exe, backup ZIP or previous preview generation is included. START_PROJECT.bat launches without rebuilding; run.bat is for rebuilding with MinGW. SHA256SUMS.txt records packaged file hashes. See SUBMISSION_CHECK.txt in the submission for the isolated runtime test result.
