# Capture and verification notes

Images were rendered at 1600 x 1000 through the unchanged project code by a
report-only C++ harness. BMP output was converted losslessly to PNG. Shading
comparison crops use LaTeX clipping of the original matched screenshots.

Manifest enumerations:
- lighting: 0 Studio, 1 Daylight, 2 Sunset, 3 Night.
- shading: 0 Phong, 1 Flat/face-normal, 2 Gouraud.
- shadows: 1 enabled, 0 disabled.
- camera: application preset selected before any explicit override below.
- time: active simulation seconds, using poseAt(time).

Explicit camera overrides (eye -> target, in scene units):
- glove_pickup, glove_place: (-10.5,4.3,7.4) -> (-10.6,1.8,1.0).
- gear: (-8.8,5.4,8.2) -> (-8.8,3.0,0).
- crank: (7.5,5.2,9.1) -> (5.1,1.95,0.7).
- drum: (-6.2,3.2,8.4) -> (-7.45,1.5,3.65).
- catch: (9.7,5.1,7.3) -> (7.1,1.8,0.5).
- starter_engaged, starter_clear: (13,3.5,5.8) -> (9.8,0.9,1.4).

Additional captures made after the manifest's standard save calls:
- ray_snapshot: same t=70, camera 2, Studio, shadows enabled as ray_raster.
  CPU image 800 x 500, displayed at 1600 x 1000; 165980 world triangles;
  one observed render duration 1893.63 ms, including scene preparation.
- car_model: original Scene::car geometry isolated at position (0,0,0), yaw=0,
  with a dark base; eye (4.8,3.1,5.7), target (0,0.7,0), Daylight, Phong.

overview_ui and guide_* include the original HUD. Guide captures are in
interactive waiting state. Other report images omit the UI for clarity.

Source hashes cover the application files used to generate these images.
pixel_example.json is a chosen arithmetic example, not a measured image pixel.
The report's verification table distinguishes assertion counts, simulated-time
handler checks and the single local snapshot measurement. It does not claim
a repeated performance benchmark.
