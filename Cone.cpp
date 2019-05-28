/*----------------------------------------------------------
* COSC363  Ray Tracer
*
* The Cone class
* This is a subclass of Object, and hence implements the
* methods intersect() and normal().
-------------------------------------------------------------*/

#include "Cone.h"
#include <math.h>

float Cone::intersect(glm::vec3 pos, glm::vec3 dir)
{   
    glm::vec3 d = pos - center;
    float y_change = height - pos.y + center.y;
    float tangent = (radius / height) * (radius / height);

    float a = (dir.x * dir.x) + (dir.z * dir.z) - (tangent * (dir.y * dir.y));
    float b = 2 * (d.x * dir.x + d.z * dir.z + tangent * y_change * dir.y);
    float c = (d.x * d.x) + (d.z* d.z) - (tangent * (y_change* y_change));
    float determinant = b * b - 4 * (a * c);
	
    if (fabs(determinant) < 0.001) {
		return -1.0;
	}	 
    if (determinant < 0.0) {
		 return -1.0;
    }
    
    float root_a = (-b - sqrt(determinant))/(2 * a);
    float root_b = (-b + sqrt(determinant))/(2 * a);
    float root_small;
    float root_big;
    
    if (root_a < 0.01) {
        root_a = -1;
	}
    if (root_b < 0.01) {
        root_b=-1;
	}
 
    if (root_a > root_b) {
        root_small = root_b;
        root_big = root_a;
    } else {
        root_small = root_a;
        root_big=root_b;
	}	
    					
    float ypos = pos.y + dir.y * root_small;
    if(ypos >= center.y && ypos <= (center.y + height)){
        return root_small;
    } else {
        float ypos = pos.y + dir.y*root_big;
        if(ypos >= center.y && ypos <= (center.y + height)){
            return root_big;
        }
        return -1.0;
	}
}

/**
* Returns the unit normal vector at a given point.
*/
glm::vec3 Cone::normal(glm::vec3 p)
{
	glm::vec3 d = p-center;
    float r = sqrt(d.x * d.x + d.z * d.z);
    glm::vec3 n= glm::vec3 (d.x, r*(radius/height), d.z);
    n=glm::normalize(n);
    return n;
}
