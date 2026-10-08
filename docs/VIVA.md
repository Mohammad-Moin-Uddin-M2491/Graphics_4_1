ï»¿# Viva guide

**Mohammad Moin Uddin Moin, 2107111**

## A short demonstration

1. Run cog_demo.exe. Presentation mode begins automatically. Explain that the chained actions take 70 active seconds before the car can move.
2. Hold right mouse and use WASD/QE to approach an object. Release the mouse to click UI. Press 1 to recover the overview.
3. Press 3 and Space. Press L to compare Gouraud and Phong without changing the camera, material, light or pose. Click Light to demonstrate another light direction/color. Resume with Space.
4. Press 2 to explain the single crank compression, rigid connecting rod, rack and ball-stop cable. Press 5 for the full new row, then 6 to show the cup, striker, rocker and pin contacts. The low cable returns to the catch on the right.
5. Press 4 as the bridge lowers. At 70 seconds the starter disconnects and the car unlocks. Use Up/Down and Left/Right; camera movement works independently. Drive straight across the bridge before turning on the apron.
6. Press I to reset into interactive mode. Hold G for the glove, tap Z four times for four quarter turns, hold X for the cam, press F for the wheel, hold V for the wiper, press F for the bar and hold P for the crank. Release a held key midway to demonstrate a partial pose. F then rolls the ball; U tips the cup, J releases the striker, K rocks the arm, O lifts the pin, and F releases each remaining ready mechanism. Future keys cannot skip stages.
7. Click Guide or press C. Show its two tabs, current mechanism, next available action and Close button. The manual controls are explicitly marked interactive only.
8. Press T for the optional ray-traced still. It uses actual triangle intersections, shadow rays and one reflection bounce. Press T, Esc or left click to return without advancing the simulation during tracing.
9. Press R to demonstrate deterministic reset. Space starts another presentation, or the displayed individual key operates the first interactive action.

## Explanations to learn

**Gouraud versus Phong:** Both use the same lighting equation in this project. Gouraud computes lighting at the triangle vertices and interpolates their colors. Phong interpolates normals and positions and computes lighting for each fragment. This makes small specular highlights more accurate. The lighting model uses the reflected light vector, not the Blinn halfway-vector approximation.

**Ambient, diffuse, specular:** Ambient is a simple sky/ground approximation for background light. Diffuse uses max(dot(normal, lightDirection), 0). Specular uses pow(max(dot(viewDirection, reflectedLight), 0), shininess). A point fill light gets weaker with distance. These are a practical teaching model, not a full physically based renderer.

**Transformation order:** Matrices multiply column vectors: world = T * R * S * local. Scale first, rotate about the local origin, then translate into the scene. A hinge at a mesh-local origin uses T(hinge) * R(angle). If the pivot is elsewhere, use T(pivot) * R * T(-pivot). Car wheels inherit the chassis transform and then add their own steering and rolling rotations.

**Normals:** Nonuniform scaling distorts normals if we use the ordinary position transform. We use normalize(transpose(inverse(mat3(model))) * normal). Phong renormalizes after interpolation.

**Rolling:** angle = signedDistance / radius. Reverse movement reverses the wheel rotation. All car wheels use the same chassis distance; the project does not model an automotive differential.

**Crank and connecting rod:** With crank radius r, rod length L and crank angle a, the slider is constrained to the crank center's height:

```text
pinX = cx + r*cos(a)
pinY = cy + r*sin(a)
sliderX = cx + r*cos(a) + sqrt(L*L - (r*sin(a))^2)
```

The square root comes from Pythagoras and preserves the connecting rod length. A cube is oriented along the vector joining its two joints.

**Cable:** Counterweight drop and gate lift are equal. One cable segment gets longer by d while the other gets shorter by d. Pulley rotation is d/radius. The fixed cable guide arcs preserve their lengths.

**Animation state:** STAGE_TIME stores 18 boundaries for 17 actions totaling 70 seconds. GLFW supplies actual elapsed time. Each pose is a deterministic function of clamped stage progress. In interactive mode, held controls advance only while their specific key is held; releasing preserves a partial pose. Z budgets one quarter turn per press and ignores OS repeat. Passive actions run after release. Every stage clamps at its boundary and requires fresh input for the next mechanism. Reset also clears held input and queued gear turns.

**Cause and effect:** Contacts and springs are authored, deterministic animation cues rather than rigid-body forces. The design demonstrates transformations and state dependencies. The code does not solve gear-tooth forces, cable tension, hydraulics or real engine mechanics.

**Glove attachment:** One known pin starts on a tray. During transport its world position equals hand position plus a fixed local offset. At placement, it detaches and stays at the socket. Finger cubes rotate toward it; sphere joints join the arm. This is a designated-component pickup, not arbitrary object selection or inverse kinematics.

**Ray tracing versus shadow mapping:** Live OpenGL rasterizes triangles and compares depths in a shadow map. The optional snapshot instead sends rays from the eye through pixels, tests their intersections with triangles, casts visibility rays to lights and follows one reflection bounce. A binary bounding-volume tree skips triangles that cannot be hit. The result is a still image; rendering it to the screen with a texture does not change how it was computed.

**Primitive construction:** All geometry is triangles. Cubes provide planar parts; sampled sphere triangles provide rounded joints. Cylinders, annuli and wheel arches are circle samples joined with triangles. Gear teeth are repeated cubes. There are no imported mesh assets.

**New front row:** The ball translates down the chute and rotates by distance/radius. The cup rotates 32 degrees about its right hinge and carries the ball as a child, withdrawing the pendulum stop. The pendulum rotates from +45 to -45 degrees with fixed rod length. The rocker rotates -25 degrees to press the pin collar. The pin translates upward 0.65; the freed drum rotates by cable take-up/radius to pull the catch bolt. Cubes, spheres and triangle-built pulleys are sufficient for every part.

The complete sequence, durations, transformations and controls chart are in README.md.
