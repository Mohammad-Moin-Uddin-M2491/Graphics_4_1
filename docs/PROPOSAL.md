# 3D Automotive Chain Reaction ? Inspired by Honda Cog

**Mohammad Moin Uddin Moin | Roll 2107111 | Computer Graphics**

## Objective

Demonstrate 3D transformations, moving objects, lighting and shading through a
continuous mechanical chain reaction. The scene is built from cubes, spheres
and triangles. Curved gears, wheels, hubs, pulleys and wheel arches use generated
triangle geometry. No imported model or physics engine is used.

## Current implementation

The previous project already supplied working window/input handling, primitive
meshes, a timeline, camera controls and lighting. These were retained and extended.
Three repetitive domino panels were replaced by mechanically different assemblies,
and the previously stationary car now has a complete release and departure.

The updated machine has a consistent workbench, dark mounts, restrained steel and
brass mechanisms, and one blue car. Normal presentation shows a small title and
stage indicator. Press H for explanatory labels and C for detailed controls.

## Cause-and-effect sequence

1. Two gears rotate in opposite directions. A pin on the large gear pushes a lever.
2. The lever retracts a stop through a slot in the wheel track.
3. The wheel rolls down the track, then pushes the wiper's input paddle.
4. The wiper sweeps into a tab on a tipping bar.
5. The bar strikes an off-centre crank paddle, starting a flywheel stroke.
6. The crank and fixed-length connecting rod advance a guided piston.
7. The piston head pushes a catch tab. The supporting finger rotates clear.
8. A counterweight drops in its guide. Its cable turns a pulley and raises the car barrier.
9. Once the barrier clears the roof, the car moves forward along its exit bay.
10. The completed scene holds for inspection. Space replays after completion; R resets.

The three new assemblies are the crank/piston, hinged catch, and pulley/counterweight.
They have different motion and each is necessary to trigger the next stage.

## Timeline

| Time | Trigger |
|---:|---|
| 0.00 | Gear pair starts |
| 3.05 | Lever begins releasing wheel gate |
| 4.50 | Wheel begins rolling |
| 8.05 | Wheel reaches paddle; wiper begins sweeping |
| 10.15 | Wiper pushes bar |
| 11.20 | Bar starts crank stroke |
| 13.20 | Piston head starts pushing catch |
| 14.55 | Catch is clear; counterweight descends and barrier rises |
| 16.65 | Barrier finishes rising |
| 17.30 | Car starts moving |
| 22.00 | Car finishes its four-unit movement |
| 25.00 | Final hold completes; replay available |

Every pose is calculated from elapsed simulation time. A clamped progress value
and smoothstep easing give continuous motion. Intervals overlap so one component
can finish while its successor starts. These are scripted contact cues rather
than collision-detection results.

## Transformations to explain

Matrices use column vectors. `T * R * S * vertex` scales in local space, rotates,
and finally positions the result in the world. Reversing translation and rotation
can make a part orbit the origin instead of spinning around its own axle.

| Part | Explanation |
|---|---|
| Gears | Local Z rotation; small gear angle = -1.5 times large gear angle |
| Lever, wiper, bar, catch | Build relative to hinge, then use T(hinge) * R(angle) |
| Wheel | Translate its center, then rotate by negative travel/radius |
| Crank | Rotate about its fixed center through a 150-degree stroke |
| Connecting rod | Position between two moving joints; rotate/scale a cube along their connecting vector |
| Piston | Translate on a fixed horizontal axis, calculated from rod length |
| Counterweight/barrier | Equal and opposite vertical translation |
| Pulley | Rotate by cable travel divided by pulley radius |
| Car | Parent rotation aligns local X with world +Z; parent translation moves all children |
| Car wheels | Child Z rotations from car travel divided by wheel radius |

For a pivot not at the mesh origin, use `T(pivot) * R(angle) * T(-pivot)`.
Scaled cubes create bars, rails, supports and panels; inverse-transpose normal
transformation keeps their lighting correct under non-uniform scaling.

## Three useful equations

**Crank-slider:** let r be crank radius, L rod length, and theta crank angle.
The crank pin is `(cx + r*cos(theta), cy + r*sin(theta))`. The slider stays at
height cy, so its X coordinate is:

```text
sliderX = cx + r*cos(theta) + sqrt(L*L - (r*sin(theta))^2)
```

This comes from a right triangle and keeps the connecting rod rigid.

**Cable:** the counterweight drops by d while the barrier rises by d. One vertical
cable segment lengthens by d and the other shortens by d. Fixed guide arcs do not
change length. Pulley angle equals d/radius.

**Rolling:** wheel angle in radians equals traveled distance divided by radius.
Both the loose wheel and all four car wheels use this relationship.

## Triangle geometry

Sample circle points with cosine and sine. A solid disk uses a triangle fan for
each cap and two triangles per side segment. A hollow ring uses inner and outer
rings joined by triangles, including the inner wall. A half-ring gives a simple
car wheel arch. Gear teeth and spokes are scaled cubes placed around the ring.

This keeps the shapes explainable while providing real thickness and openings.
The cabin is a small triangulated prism; the car also has windows, pillars,
mirrors, grille, lights, hubs and wheel arches.

## Lighting and shading

The default mode is correctly labeled **Blinn-Phong**: it computes specular light
using the halfway vector between the light and viewer directions. Lighting is
evaluated per fragment. Flat mode uses each triangle's face normal; Gouraud mode
evaluates lighting at vertices and interpolates the result.

Ambient sky/ground light prevents unlit surfaces from becoming entirely black.
Lambert diffuse light depends on the surface normal and light direction. Specular
highlights depend on the viewer and light directions, material strength and shininess.
A directional main light, a point fill with distance attenuation, and a rear light
make metal and rubber distinguishable. The four presets change direction/color,
ambient illumination and exposure without changing the animation.

The Shadows control switches a real shadow-map pass: render depth from the main
light's view, then compare surface depth against that texture. A 5x5 filter softens
edges; slope correction reduces self-shadow artifacts. Only the main light casts
shadows. Exposure/tone mapping/gamma are used for display; the shader is not a full
physically based renderer. Windows are opaque and lamps are modeled shapes.

## Two-minute viva demonstration

1. Run cog_demo.exe. Explain the full sequence as it plays (25 seconds).
2. Press 2 for the closer presentation view and H for numbered mechanisms.
3. Seek around 11?14 seconds using the bottom line; step with arrow keys.
   Explain the crank-slider equation and the rigid connecting rod.
4. Press 4 and seek around 15?18 seconds. Show the catch, falling weight,
   rising barrier and clearance before the car starts.
5. Click Light to compare presets. Open Tools, toggle Shadows and cycle Shading.
   Explain diffuse/specular terms and the inverse-transpose normal matrix.
6. Press 1 to show the final car position. Press R to prove all mechanisms reset.
   Press Space to play again.

## Scope and limitations

This is a controlled graphics animation. It does not simulate rigid-body forces,
gear tooth contact or cable tension. Gear teeth are simplified blocks. The crank
uses an authored impulse/stroke after the bar's strike, and car acceleration is
scripted. Rod length, cable travel, roof clearance and wheel rotation are checked
mathematically. This scope is intentional so the graphics concepts remain clear.

The older working source is archived in backups/before-unique-mechanisms.zip.
