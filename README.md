# RayTracer
Ray Tracer built using C++/OpenGL showcasing reflections, refractions, transparency, lightings of shapes in a 3D space

# Build Process
• Git clone the project.
• Open up the CMakeLists.txt using an IDE (eg. QtCreator). 
• Run cmake file. 
• Ensure working directory is the root directory of the repository or else images and other stuff will fail to load. 
• Run project.
• You can choose primary ray tracing or antialiasing by uncommenting either one of the respective lines in the display() method:
  - glm::vec3 materialCol = antiAliasing(eye, cellX, xp, yp); //Anti-aliasing
	- glm::vec3 materialCol = trace(ray, 1); //Trace the primary ray and get the colour value
