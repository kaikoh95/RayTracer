/*========================================================================
* COSC 363  Computer Graphics (2019)
* Ray Tracer
*=========================================================================
*/

#include <iostream>
#include <cmath>
#include <vector>
#include <glm/glm.hpp>
#include <GL/glut.h>

#include "Sphere.h"
#include "SceneObject.h"
#include "Ray.h"
#include "Plane.h"
#include "Cylinder.h"
#include "Cone.h"
#include "TextureBMP.h"

using namespace std;

const float WIDTH = 20.0;
const float HEIGHT = 20.0;
const float EDIST = 40.0;
const int NUMDIV = 500;
const int MAX_STEPS = 5;
const float XMIN = -WIDTH * 0.5;
const float XMAX =  WIDTH * 0.5;
const float YMIN = -HEIGHT * 0.5;
const float YMAX =  HEIGHT * 0.5;
const float ETA = 1.01;
const int SHEAR = 2;

TextureBMP textureWall;
TextureBMP textureSphere;
vector<SceneObject*> sceneObjects;  //A global list containing pointers to objects in the scene


//---The most important function in a ray tracer! ----------------------------------
//   Computes the colour value obtained by tracing a ray and finding its
//     closest point of intersection with objects in the scene.
//----------------------------------------------------------------------------------
glm::vec3 trace(Ray ray, int step)
{
    glm::vec3 backgroundCol(0);
    glm::vec3 lightFirst(-10.0, 15, -5.0);
    glm::vec3 lightSecond(10, 40, -3);
    glm::vec3 ambientLight(0.2);
    glm::vec3 whiteColor(1);
    glm::vec3 specular(0);
    glm::vec3 specular2(0);
    glm::vec3 colorSum(0);

    float lightIntensity = 0.6;
    float floatNum = 20.0;
    float transparency = 0.2;

    ray.closestPt(sceneObjects);		//Compute the closest point of intersetion of objects with the ray

    if(ray.xindex == -1) return backgroundCol;      //If there is no intersection return background colour

    glm::vec3 materialCol = sceneObjects[ray.xindex]->getColor(); //else return object's colour

    glm::vec3 normalVector = sceneObjects[ray.xindex]->normal(ray.xpt);
    glm::vec3 lightVector = lightFirst - ray.xpt;
    glm::vec3 lightVector2 = lightSecond - ray.xpt;


    float lightDistance = glm::length(lightVector);
    float lightDistance2 = glm::length(lightVector2);

    lightVector = glm::normalize(lightVector);
    lightVector2 = glm::normalize(lightVector2);

    float lightDotProduct = glm::dot(lightVector, normalVector);
    float lightDotProduct2 = glm::dot(lightVector2, normalVector);

    glm::vec3 reflectionVector = glm::reflect(-lightVector, normalVector);
    glm::vec3 reflectionVector2 = glm::reflect(-lightVector2, normalVector);

    // For fog-like light shining from background
    float reflectionDotProduct = glm::dot(reflectionVector,-ray.dir);
    float reflectionDotProduct2 = glm::dot(reflectionVector2,-ray.dir);
    reflectionDotProduct = max(reflectionDotProduct, 0.0f);
    reflectionDotProduct2 = max(reflectionDotProduct, 0.0f);

    // Sphere
    if (ray.xindex == 1) {
        glm::vec3 center(-7, 5.0, -70.0);
        glm::vec3 d = glm::normalize(ray.xpt-center);
        float u = (0.5 - atan2(d.z,d.x) + M_PI) /(2 * M_PI);
        float v = 0.5 + asin(d.y) / M_PI;
        materialCol = textureSphere.getColorAt(u, v);
    }

   // Checkered Ground
   if (ray.xindex == 3) {
        int x_val = int((ray.xpt.x + 50) / 8) % 2;
        int z_val = int((ray.xpt.z + 50) / 8) % 2;

       if (x_val && z_val || !x_val && !z_val) {
           materialCol = glm::vec3(0,1,1);
       }
       else {
           materialCol = glm::vec3(1,0,0);
       }
    }

    // Background
    if (ray.xindex == 4) {
        float s = ray.xpt.x / 100;
        float t = ray.xpt.y / 100;
        materialCol = textureWall.getColorAt(s,t);
    }

    // Cylinder Procedural Pattern
    if (ray.xindex == 5) {
        if (int(ray.xpt.x - ray.xpt.y) % 2 == 0) {
            materialCol = glm::vec3(0.2, 0.2, 0);
        } else {
            materialCol = glm::vec3(1,1,1);
        }
   }

    Ray shadow(ray.xpt,lightVector);
    Ray shadow2(ray.xpt,lightVector2);
    shadow.closestPt(sceneObjects);
    shadow2.closestPt(sceneObjects);

    // First light source shadow
    if (reflectionDotProduct >= 0) {
        specular = pow(reflectionDotProduct, floatNum) * whiteColor;
    }
    if ((shadow.xdist < lightDistance && shadow.xindex > -1) || lightDotProduct <= 0) {
        colorSum = ambientLight * materialCol;
        if (shadow.xindex == 2) {
            colorSum += (specular + lightDotProduct * materialCol) * glm::vec3(0.2) + sceneObjects[2]->getColor() * glm::vec3(0.02);
        }
    } else {
        colorSum = ambientLight * materialCol + lightIntensity * (specular + lightDotProduct * materialCol);
    }

    // Second light source shadow
    if (reflectionDotProduct2 >=0) {
        specular2 = pow(reflectionDotProduct2, floatNum) * whiteColor;
    }
    if ((shadow2.xdist < lightDistance2 && shadow2.xindex > -1) || lightDotProduct2 <= 0) {
        colorSum += ambientLight * materialCol;
        if (shadow2.xindex == 2) {
            colorSum += (specular2 + lightDotProduct2 * materialCol) * glm::vec3(0.15) + sceneObjects[2]->getColor() * glm::vec3(0.015);
        }
    } else {
        colorSum += ambientLight * materialCol + (1 - lightIntensity) * (specular2 + lightDotProduct2 * materialCol);
    }

    // Reflection
    if((ray.xindex == 0||ray.xindex ==4) && (step < MAX_STEPS)) {
        glm::vec3 reflectedDirection = glm::reflect(ray.dir, normalVector);
        Ray reflectedRay(ray.xpt, reflectedDirection);
        glm::vec3 reflectedColor = trace(reflectedRay, step++);
        colorSum = colorSum + (0.9f * reflectedColor);
    }

    // Refraction
    if ((ray.xindex == 2) && (step < MAX_STEPS)) {
        glm::vec3 refractionDirection1 = glm::refract(ray.dir, normalVector, 1.0f/ETA);
        Ray refractionRay1(ray.xpt, refractionDirection1);
        refractionRay1.closestPt(sceneObjects);
        if (refractionRay1.xindex == -1) {
            return backgroundCol;
        }
        glm::vec3 normalVector2 = sceneObjects[refractionRay1.xindex]->normal(refractionRay1.xpt);
        glm::vec3 refractionDirection2 = glm::refract(refractionDirection1, -normalVector2, ETA);
        Ray refractionRay2(refractionRay1.xpt, refractionDirection2);
        refractionRay2.closestPt(sceneObjects);
        if (refractionRay2.xindex == -1 ){
            return backgroundCol;
        }
        glm::vec3 refractionColor = trace(refractionRay2, step++);
        colorSum = colorSum * transparency + refractionColor * (1 - transparency);
        return colorSum;
    }
    return colorSum;
}

/**
 * Anti-aliasing via Super Sampling
 */
glm::vec3 antiAliasing(glm::vec3 eye, float pixel_size, float xp, float yp){
    float smallPixel = pixel_size * 0.25;
    float largePixel = pixel_size * 0.75;
    glm::vec3 colorSum(0);
    glm::vec3 average(0.25);

    Ray ray = Ray(eye, glm::vec3(xp+smallPixel, yp+smallPixel, -EDIST));
    ray.normalize();
    colorSum += trace(ray,1);

    ray = Ray(eye, glm::vec3(xp+largePixel, yp+smallPixel, -EDIST));
    ray.normalize();
    colorSum += trace(ray,1);

    ray = Ray(eye, glm::vec3(xp+smallPixel, yp+largePixel, -EDIST));
    ray.normalize();
    colorSum += trace(ray,1);

    ray = Ray(eye, glm::vec3(xp+largePixel, yp+largePixel, -EDIST));
    ray.normalize();
    colorSum += trace(ray,1);

    // get average
    colorSum *= average;

    return colorSum;
}

/**
 * Function to draw a Cube and set its color
 */
void drawCube(float x, float y, float z, float length, float width, float height, glm::vec3 color, int shear)
{
    glm::vec3 A = glm::vec3(x, y, z);
    glm::vec3 B = glm::vec3(x+length, y, z);
    glm::vec3 C = glm::vec3(x+length+shear, y+height, z);
    glm::vec3 D = glm::vec3(x+shear, y+height, z);
    glm::vec3 E = glm::vec3(x+length, y, z-width);
    glm::vec3 F = glm::vec3(x+length+shear, y+height, z-width);
    glm::vec3 G = glm::vec3(x+shear, y+height, z-width);
    glm::vec3 H = glm::vec3(x, y, z-width);

    Plane *face1 = new Plane(A, B, C, D, color);
    Plane *face2 = new Plane(B, E, F, C, color);
    Plane *face3 = new Plane(E, H, G, F, color);
    Plane *face4 = new Plane(D, G, H, A, color);
    Plane *face5 = new Plane(D, C, F, G, color);
    Plane *face6 = new Plane(H, E, B, A, color);

    sceneObjects.push_back(face1);
    sceneObjects.push_back(face2);
    sceneObjects.push_back(face3);
    sceneObjects.push_back(face4);
    sceneObjects.push_back(face5);
    sceneObjects.push_back(face6);
}

/**
 * Function to draw a Tetrahedraon and set its color
 */
void drawTetrahedron(float x, float y, float z, float length, glm::vec3 color1, glm::vec3 color2)
{
    glm::vec3 A = glm::vec3(x, y, z);
    glm::vec3 B = glm::vec3(x+length, y, z);
    glm::vec3 C = glm::vec3(x+length*0.5, y, z + sqrt(3.0f) * 0.5 * length);
    glm::vec3 D = glm::vec3(x+length*0.5, y+sqrt(6.0f)/3.0f * length, z + sqrt(3.0f) * 0.25 * length);
    glm::vec3 E = glm::vec3(x+length/2, y, z);
    glm::vec3 MID = glm::vec3((C.x+D.x)/2, (C.y+D.y)/2, (C.z+D.z)/2);

    Plane *face1 = new Plane(E, B, C, A, color1);
    Plane *face2 = new Plane(B, D, MID, C, color2);
    Plane *face3 = new Plane(D, A, C, MID, color1);
    Plane *face4 = new Plane(A, D, B, E, color2);

    sceneObjects.push_back(face1);
    sceneObjects.push_back(face2);
    sceneObjects.push_back(face3);
    sceneObjects.push_back(face4);
}

//---The main display module -----------------------------------------------------------
// In a ray tracing application, it just displays the ray traced image by drawing
// each cell as a quad.
//---------------------------------------------------------------------------------------
void display()
{
    float xp, yp;  //grid point
    float cellX = (XMAX-XMIN) / NUMDIV;  //cell width
    float cellY = (YMAX-YMIN) / NUMDIV;  //cell height

    glm::vec3 eye(0., 0., 0.);  //The eye position (source of primary rays) is the origin

    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glBegin(GL_QUADS);  //Each cell is a quad.

    for(int i = 0; i < NUMDIV; i++)  	//For each grid point xp, yp
    {
        xp = XMIN + i*cellX;
        for(int j = 0; j < NUMDIV; j++)
        {
            yp = YMIN + j*cellY;

            glm::vec3 dir(xp+0.5*cellX, yp+0.5*cellY, -EDIST); //direction of the primary ray

            Ray ray = Ray(eye, dir); //Create a ray originating from the camera in the direction 'dir'
            ray.normalize(); //Normalize the direction of the ray to a unit vector

            glm::vec3 materialCol = antiAliasing(eye, cellX, xp, yp); //Anti-aliasing
            //glm::vec3 materialCol = trace(ray, 1); //Trace the primary ray and get the colour value

            glColor3f(materialCol.r, materialCol.g, materialCol.b);
            glVertex2f(xp, yp);				//Draw each cell with its color value
            glVertex2f(xp+cellX, yp);
            glVertex2f(xp+cellX, yp+cellY);
            glVertex2f(xp, yp+cellY);
        }
    }

    glEnd();
    glFlush();
}

//---This function initializes the scene -------------------------------------------
//   Specifically, it creates scene objects (spheres, planes, cones, cylinders etc)
//     and add them to the list of scene objects.
//   It also initializes the OpenGL orthographc projection matrix for drawing the
//     the ray traced image.
//----------------------------------------------------------------------------------
void initialize()
{
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(XMIN, XMAX, YMIN, YMAX);
    glClearColor(0, 0, 0, 1);

    textureWall = TextureBMP((char*)"Background.bmp");
    textureSphere = TextureBMP((char*)"AntarcticaSphere.bmp");

    Plane *ground = new Plane (
                glm::vec3(-50., -20, -40),
                glm::vec3(50., -20, -40),
                glm::vec3(50., -20, -200),
                glm::vec3(-50., -20, -200),
                glm::vec3(0));

    Plane *background = new Plane(
                glm::vec3(-50., -20, -200),
                glm::vec3(50., -20, -200),
                glm::vec3(50., 50, -200),
                glm::vec3(-50., 50, -200),
                glm::vec3(0,0,0));

    Sphere *antarcticaSphere = new Sphere(glm::vec3(-7, 5.0, -70.0), 3, glm::vec3(0, 0, 0));
    Sphere *largeSphere = new Sphere(glm::vec3(-5.0, -5.0, -105.0), 12.0, glm::vec3(0.2, 0.2, 0.2));
    Sphere *smallSphere = new Sphere(glm::vec3(5, -10.0, -68.0), 3, glm::vec3(0, 0, 0));

    Cylinder *cylinder = new Cylinder(glm::vec3(14, -12, -70), 2, 10.0, glm::vec3(1,0,0));

    Cone *cone = new Cone(glm::vec3(-10, -12.0, -70.0), 3, 14.0, glm::vec3(1, 0.5, 0.2));

    //--Add the above to the list of scene objects.
    sceneObjects.push_back(largeSphere);
    sceneObjects.push_back(antarcticaSphere);
    sceneObjects.push_back(smallSphere);
    sceneObjects.push_back(ground);
    sceneObjects.push_back(background);
    sceneObjects.push_back(cylinder);
    sceneObjects.push_back(cone);

    drawTetrahedron(-6, -10, -70, 5, glm::vec3(0.8, 0.3, 0.3), glm::vec3(0.5, 0.8, 0.8));
    drawCube(5, 0, -50, 3, 3, 3, glm::vec3(1, 1, 0), SHEAR);
}

int main(int argc, char *argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(600, 600);
    glutInitWindowPosition(20, 20);
    glutCreateWindow("Ray Tracer");

    glutDisplayFunc(display);
    initialize();

    glutMainLoop();
    return 0;
}
