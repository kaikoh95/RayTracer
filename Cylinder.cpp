/*----------------------------------------------------------
* COSC363  Ray Tracer
*
* The Cylinder class
* This is a subclass of Object, and hence implements the
* methods intersect() and normal().
-------------------------------------------------------------*/

#include "Cylinder.h"
#include <math.h>

float Cylinder::intersect(glm::vec3 pos, glm::vec3 dir)
{
	glm::vec3 d = pos - center;
	float a = (dir.x * dir.x) + (dir.z * dir.z);
    float b = 2 * (dir.x * d.x + dir.z * d.z);
    float c = d.x * d.x + d.z * d.z - (radius * radius);
    
    float determinant = b * b - 4 * a * c;
	
    if (determinant < 0.0) {
        return -1.0;
    }
    
    if (fabs(determinant) < 0.001) {
        return -1.0;
    }

    float root_a = (-b - sqrt(determinant))/(2 * a);
    float root_b = (-b + sqrt(determinant))/(2 * a);
    float root_small;
    float root_big;

    if (root_a < 0.01) {
        root_a=-1;
    }
    if (root_b < 0.01) {
        root_b=-1;
    }
    if (root_a <= root_b) {
        root_small = root_a;
        root_big = root_b;
    } else {
        root_small = root_b;
        root_big = root_a;
    }

    float ypos = pos.y + dir.y * root_small;

    if (ypos <= (center.y + height) && ypos >= center.y) {
        return root_small;
    } else {
        float ypos = pos.y + dir.y*root_big;
        if (ypos <= (center.y + height) && ypos >= center.y) {
            return root_big;
        } else {
            return -1.0;
        }
    }
}

/**
* Returns the unit normal vector at a given point.
*/
glm::vec3 Cylinder::normal(glm::vec3 p)
{
	glm::vec3 d = p - center;
    glm::vec3 n = glm::vec3 (d.x, 0, d.z);
    n = glm::normalize(n);
	return n;
}
