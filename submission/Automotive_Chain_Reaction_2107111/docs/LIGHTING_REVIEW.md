# Lighting, shading and projection correction

Reference: `2.png`, Studio/Gouraud at about 60.4 active seconds.

## What was wrong

The visible bulb supplied an unshadowed fill term. A stronger invisible directional light cast the dominant shadows, including a shadow of the lamp itself. An additional unshadowed rim light produced unexplained highlights. Gouraud lighting on large two-triangle surfaces also poorly sampled the nearby source. Some sphere and ring triangle orientations disagreed with their normals.

## Current lighting model

- The bulb center and point-light origin both equal **(-1.8, 5.8, 4.1)** in world coordinates. The lamp is mounted near the front edge so the visible mechanism faces receive direct light.
- **Studio and Night:** one direct point light at the bulb, plus low sky/ground ambient illumination.
- **Daylight and Sunset:** the same lamp plus a directional sun. Two shadow directions can therefore be visible. There is no extra rim source.
- At a surface point P, L = normalize(bulb - P), V = normalize(eye - P), and N is the normalized inverse-transpose transformed normal.
- Lamp irradiance is `lampColor / max(distance^2, 0.32^2)`. The radius clamp only regularizes points inside the visible bulb.
- Diffuse is `kd * max(dot(N,L),0)`. Specular is `ks * pow(max(dot(V,reflect(-L,N)),0),shininess)`, gated off when N dot L is nonpositive. Existing material colors and metallic tint control kd/ks. The old extra shininess-dependent boost and extra N dot L factor on the specular lobe were removed.
- The bulb's emissive color is independent of shadow visibility. Its luminous envelope is excluded from its own lamp shadow map; the opaque guard and housing still cast shadows.
- All lighting calculations use linear values. Exposure, Reinhard tone mapping and display gamma are applied once. The former camera-distance haze was removed so moving the camera does not change diffuse surface colors.

## Shadow and shading methods

The lamp uses six 1024-square depth faces, 90-degree perspective projection, and radial distance divided by an 80-unit shadow range. It casts shadows in all presets. Daylight/Sunset use a separate 2048-square orthographic sun shadow map. Shadows affect each source's diffuse/specular contribution individually, preserving ambient and emission.

Nine point-shadow samples smooth texel edges. Each sample's receiving-plane depth is calculated along the **actual cubemap texel-center ray**, preventing striped self-shadowing on the lamp pole and distant floor. A small depth bias handles floating-point error.

Phong computes lighting per fragment using interpolated, renormalized normals. Gouraud computes the same lighting terms per vertex and interpolates them. Shadow visibility is per fragment in both modes. Large cubes use 16 subdivisions per face; the floor uses 64. This preserves true Gouraud shading while resolving the point light's falloff across the bench. Flat mode uses the triangle face normal. Fine highlights can still differ between Gouraud and Phong; this is expected.

## Projection and normals

Rendering, picking and CPU primary rays share a **43-degree vertical perspective FOV**, aspect ratio, and **0.05/220** near/far planes. At oblique pixels, ray clipping converts camera-space plane distances to distance along the ray. Cubemap perspective and sun orthographic projections serve shadow generation only.

Normals use inverse-transpose transforms for nonuniform scales. Sphere pole degeneracies were removed; sphere, inner ring and half-ring end-cap winding now agree with outward normals.

## Verification

- `mingw32-make test`: 472 focused assertions; 9,241 sampled poses / 189,430 repeated numerical invariants.
- Projection round trips: 6 camera presets, 3 aspect ratios, 9 image locations per combination; near/far clip-plane endpoints checked.
- Nonuniform normal transformation checked for perpendicularity to two transformed surface tangents. Lamp falloff checked for a fourfold change when distance doubles.
- `--verify-lighting`: 54,188 mesh triangle orientations checked. Each of six shadow faces sees a box face exactly 1.5 units from the bulb; measured radial depth must agree within 0.002 units. The bulb does not block its own source.
- GPU receiver test: a blocker on the bulb-to-receiver segment darkens that receiver in Phong, Flat and Gouraud; moving it away restores illumination.
- Matched captures at 60.4 seconds cover all 4 presets and 3 shading modes. See [Studio/Phong](lighting-correction/lighting_0_shading_0.png), [Studio/Gouraud](lighting-correction/lighting_0_shading_2.png), [Daylight](lighting-correction/lighting_1_shading_0.png), and [Night](lighting-correction/lighting_3_shading_2.png).

This remains a teaching renderer: ambient is an approximation, the bulb is represented as a point emitter, and shadow filtering is not a physical area-light penumbra. The CPU snapshot shares direct-light equations and projection but additionally traces one reflection bounce and uses exact shadow rays, so it need not match the OpenGL image pixel for pixel.
