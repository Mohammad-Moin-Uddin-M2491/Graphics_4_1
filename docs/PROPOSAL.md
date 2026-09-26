# 3D Automotive Chain Reaction — Inspired by Honda Cog

**Student:** Mohammad Moin Uddin Moin  
**Roll:** 2107111  
**Course:** Computer Graphics

## Project proposal

The project demonstrates a continuous sequence of automotive and mechanical
parts in a 3D OpenGL scene. A gear pair operates a release lever, allowing a wheel
to roll down a track. The wheel drives a windshield wiper, which pushes a tipping
bar. The bar starts a three-panel domino sequence, ending at a receiver beside
a stylized car. In the final version, the receiver will release the car for its
forward movement and ending shot.

The Honda Cog advertisement is an inspiration for sequential cause and effect,
not a scene to reproduce exactly. No Honda branding, downloaded car model,
imported mesh or rigid-body physics engine is used.

The purpose is to demonstrate transformations, viewing, animation, lighting,
shading and normals using geometry that one student can construct and explain.

## Current milestone (approximately 70% of the planned project)

The executable already renders all ten main objects and animates the chain
through the third domino and receiver. The car is modeled and parked. The
remaining central feature is the car's release and forward movement, followed
by final contact tuning, presentation polish and the final report.

This percentage is a planning estimate. Demonstrate the implemented behavior
and identify the remaining work explicitly; do not claim the ending is complete.

## Layout

The scene uses a right-handed coordinate system: X is the direction of the chain,
Y is vertical and Z provides depth. Axles generally lie along Z. Parts are placed
on an exhibit platform and seen through a perspective orbit camera.

```text
X increases to the right; Y points up; axles run along Z

  small gear -> large gear -> release lever
                                   |
                                   v
                              rolling wheel
                                    \____ -> wiper -> bar -> panel 1 -> 2 -> 3
                                                                             |
                                                                         receiver
                                                                             |
                                                                     parked car
                                                               [final stage pending]
```

## Mechanism and graphics mapping

| Object | Geometry | Graphics concept | Current behavior |
|---|---|---|---|
| Small gear | Triangle disk + cube teeth | Local rotation | Drives large gear |
| Large gear | Triangle disk + cube teeth + cam pin | Opposite rotation, radius ratio | Pushes lever |
| Lever assembly | Scaled cubes + axle | Pivot rotation, linked translation | Retracts wheel gate |
| Wheel | Triangle circular solids + cube spokes/treads | Translation with distance-based rotation | Rolls and pushes paddle |
| Wiper assembly | Cube arm/blade + circular hub | Local pivot, composite transforms | Sweeps into bar tab |
| Bar | Scaled cube + triangle axle | Bottom-pivot rotation | Starts panel sequence |
| Panel 1 | Scaled cube | Bottom-edge pivot | Tips into panel 2 |
| Panel 2 | Scaled cube | Delayed overlapping rotation | Tips into panel 3 |
| Panel 3 | Scaled cube | Delayed rotation | Depresses receiver |
| Car | Cubes, triangular cabin and wheels, sphere mirrors | Parent/child modeling | Modeled; movement pending |

## Animation design

The application advances one simulation clock by elapsed frame time. When paused,
the clock stays unchanged. Each mechanism derives its pose from that clock, so
resetting or seeking gives a consistent scene without accumulated numerical drift.

Each stage uses a clamped progress value and smooth interpolation:

```text
u = clamp((simulationTime - startTime) / duration, 0, 1)
smooth(u) = u*u*(3 - 2*u)
```

| Time (seconds) | Action begins |
|---:|---|
| 0.00 | Gear pair rotates |
| 3.05 | Lever pivots and gate retracts |
| 4.50 | Wheel rolls |
| 8.05 | Wheel reaches paddle; wiper sweeps |
| 10.15 | Bar tips |
| 11.28 | First panel tips |
| 12.05 | Second panel tips |
| 12.82 | Third panel tips |
| 14.30 | Receiver depresses |
| 17.50 | Final hold ends; playback loops |

Adjacent stages can overlap. These times are authored contact cues, not results
of collision detection. A falling panel or gear does not abruptly stop when the
status display changes to the next stage.

## Viva explanations

**Transformation order.** Matrices use column vectors. `T * R * S * vertex`
scales the mesh in local space, rotates it, then places it in the scene.
Reversing translation and rotation can make an object orbit the world origin
instead of spinning at its own location.

**Local pivots.** Levers, bars and panels are built relative to a local hinge.
Their model matrix begins with `T(hinge) * R(angle)`. For a pivot that is not the
local origin, use `T(pivot) * R(angle) * T(-pivot)`.

**Hierarchy.** Car wheels, body, cabin and mirrors inherit a shared car-root
matrix. Changing that parent transform will move the entire car in the final
stage. Each wheel can then have its own child rotation.

**Wheel rolling.** The wheel's angle in radians is traveled distance divided
by radius. Its sign depends on the travel direction and axle orientation.

**Gear ratio.** The pitch radii are 0.70 and 1.05 units. Therefore the small
gear turns 1.5 times as far, in the opposite direction. Teeth are simplified
rectangular blocks rather than engineering-accurate involute profiles.

**Curved geometry.** Circle points use cosine and sine. A disk consists of
triangles from its center to adjacent rim points. Front/back rings are joined
with two triangles per side segment to form a thick wheel or axle.

**Normals.** Cube faces have constant normals. Curved sides use interpolated
radial normals. The shader transforms normals with
`transpose(inverse(mat3(model)))` and normalizes them; simply applying the model
matrix would give incorrect lighting under non-uniform scaling.

**Lighting and shading.** The scene combines ambient, diffuse and specular terms.
The default is per-fragment Blinn-Phong, a Phong-style model using a halfway
vector. Flat mode uses a triangle face normal. Gouraud mode evaluates lighting
at vertices and interpolates the resulting color. Press L to compare them on
curved wheels and gears. Material shininess and specular strength distinguish
metal, rubber, supports and the car body.

**Shadows (an extra feature).** A depth map renders the scene from the light's
view. The main pass compares each surface with that depth map to determine
whether the surface is shaded. This is separate from the lighting model.

## Two-minute demonstration

1. Open `cog_demo.exe`. Explain the ten numbered main objects and the primitive rule.
2. Playback starts automatically. Let the full chain run in the overview (about 18 seconds).
3. Press R, then 3, then Space. Show the gears, cam, lever and rolling wheel.
4. Press Space to pause. Drag to orbit and use the wheel to zoom.
5. Click Light to compare Studio, Daylight, Sunset and Night. Adjust brightness or shadows, then click Shading to compare the shading models.
6. Press 4 and seek using the bottom timeline to show the wiper and dominoes.
7. Press 1 and R to restore the overview. State that car release/movement,
   final contact tuning and the final report are the remaining work.

## Suggested verbal introduction

"My project is a simplified 3D automotive chain reaction inspired by Honda Cog.
I constructed the objects using basic shapes and triangle-based circular meshes.
The current version animates the gears, lever, wheel, wiper, tipping bar and
three domino panels. It also demonstrates perspective viewing, hierarchical
transformations, lighting and three shading modes. I use a controlled timeline
instead of a physics engine. The car model is complete, and its release and
movement are the main remaining animation task."

## Lighting controls added

The Light button selects four rigs. Each rig changes the direction and color of the main directional light, the color of a point fill light, the rear rim light, ambient sky/ground colors and exposure. Point light attenuation reduces its contribution with distance. Only the main light casts a shadow map. A 5 by 5 filter softens its edges, with slope correction to prevent the floor from shadowing itself. Tone mapping compresses bright highlights before gamma correction. Metal uses a tinted specular highlight; this remains a Phong-style teaching model, not a full physically based renderer. The on-screen brightness buttons control exposure, independently of animation speed.

