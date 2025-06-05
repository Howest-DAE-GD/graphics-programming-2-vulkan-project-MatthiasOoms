This is a Vulkan renderer with the goal of presenting the Sponza Atrium scene.
To load in this scene I use a .gltf file with provided textures, normal maps and specular/roughness maps.
As of right now this Vulkan renderer does a depth prepass, followed by a deferred pass rendering to 3 output images, which it then combines into 1 final output (thoug currently the 3rd output image goes unused, but it will support specular/roughness).
After this final output, it does one final transparency pass which renders any objects that have an alpha mask in the scene.
To showcase the normal maps there is currently a hardcoded directional light in the combined fragment shader.
Matthias Ooms
https://github.com/Howest-DAE-GD/graphics-programming-2-vulkan-project-MatthiasOoms 