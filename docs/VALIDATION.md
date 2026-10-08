# Expanded machine validation

Date: 8 October 2026 (Asia/Dhaka)

## Build and controller checks

`mingw32-make build` and `mingw32-make test` passed with C++17, -O2, -Wall and -Wextra; no compiler warnings.

**189,551 invariant checks passed.** They cover all 17 stage boundaries, exact 70-second release threshold, frame partition independence, glove attachment, gear ratio, one winding turn, one crank stroke, fixed crank and rack-link lengths, both cable travel relationships, ball spin and cup parenting, contact locations, pose continuity, sequential interlocks, partial held-key operations, quarter-turn input, pause/reset, car steering/reverse/rolling, free camera, guide tabs/close/tools and UI coordinate scaling. The ray tracer's geometric self-checks also pass.

`--verify-interactive` rendered **441 GPU frames** through all 17 mechanisms and the car exit. It uses the registered key callback, releases each held operation midway for several frames, resumes it, and taps Z four times. It verifies that OS key repeat does not add winding turns and that unrelated or future operation types cannot bypass the order. Release occurred at **70.0000 active seconds** in this synthetic-time run.

`--verify-playback` rendered the full presentation using synthetic time steps. The actual-time run below repeats the complete presentation with real elapsed time.

## Actual elapsed-time playback

Device: Intel Iris Xe Graphics, OpenGL 3.3.0, driver 31.0.101.4502.

```text
RELEASE: active=70.0085 s, wall=70.0085 s, frame=8879
PASS: full 17-action / 70-second chain + 7-second car exit; 9725 GPU frames; mode, stage, camera, pause/reset, guide and shading handlers
```

The release threshold is exactly 70.000 seconds. 70.0085 is the first rendered frame that reached it. The complete sequence and car departure rendered through 77 seconds. Capture and ray tracing ran concurrently during part of this run; animation still followed GLFW elapsed time.

## Rendered review

- **37 framebuffer captures** in expanded-preview cover every action, arrivals/contact poses, the car exit, labels, Phong/Gouraud/Flat, all four lighting presets, and both guide tabs. PNG copies and three contact sheets accompany the BMP captures.
- During review, the new row was spread across the bench, the cup pivot/entry changed so it tips clear of the chute, a supporting chute bed was added, and a closer contact camera was added as key 6.
- Camera 1 frames the complete chain; camera 5 shows the ball's full route; camera 6 makes the cup, striker, rocker and sliding-pin contacts large enough to explain.
- Identical-pose/view/light captures 29 and 30 confirm different Phong and Gouraud outputs (mean absolute RGB differences approximately 2.70 / 3.14 / 3.55). The lighting formula remains evaluated in the vertex shader for Gouraud and fragment shader for Phong.
- GLFW context creation, shader compilation, the depth-map framebuffer and complete rendering reported no OpenGL errors.
- The guide lists current handlers, mode scope, held operations and quarter turns, live status, two tabs and an explicit close button. Shared UiLayout scaling supports normal, 16:9 and wide windows.

## Ray-traced snapshot

`--capture-ray docs/expanded-ray` traced **78,326 triangles** into an **800 x 500** image in **4,685.8 ms** during concurrent timed playback. The output was visually inspected. It uses actual triangle intersections, shadow rays and one reflection bounce. The return callback preserves the earlier play/pause state; tracing time is excluded from the simulation clock.

## Scope of verification

The evidence combines complete GPU playback, real elapsed-time measurement, sampled rendered inspection, registered input-handler exercises and controller invariants. It is not a recording of a human performing every mouse/keyboard gesture. Contacts and stored spring energy are authored cues; no force or collision engine has been added. The car retains bounded kinematic driving, and CPU ray tracing remains a frozen single-sample image.
