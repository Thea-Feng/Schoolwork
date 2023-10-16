#include "sphere.h"

#include <cmath>

#include "pathtracer/bsdf.h"
#include "util/sphere_drawing.h"

namespace CGL {
namespace SceneObjects {

bool Sphere::test(const Ray &r, double &t1, double &t2) const {

  // TODO (Part 1.4):
  // Implement ray - sphere intersection test.
  // Return true if there are intersections and writing the
  // smaller of the two intersection times in t1 and the larger in t2.


  return false;

}

bool Sphere::has_intersection(const Ray &r) const {

  // TODO (Part 1.4):
  // Implement ray - sphere intersection.
  // Note that you might want to use the the Sphere::test helper here.
  return intersect(r, nullptr);
}

bool Sphere::intersect(const Ray &r, Intersection *i) const {

  // TODO (Part 1.4):
  // Implement ray - sphere intersection.
  // Note again that you might want to use the the Sphere::test helper here.
  // When an intersection takes place, the Intersection data should be updated
  // correspondingly.
  Vector3D o_minus_c = r.o - o;
  double a = dot(r.d, r.d), b = 2 * dot(o_minus_c, r.d), c = dot(o_minus_c, o_minus_c) - r2;
  if(b*b - 4*a*c < 0) return false;
  double sqrt_val = sqrt(b*b - 4*a*c);
  double t1 = (-b - sqrt_val) / (2*a), t2 = (-b + sqrt_val) / (2*a);
  double minn = min(t1, t2);
  if(minn < r.min_t || minn > r.max_t) return false;
  r.max_t = minn;

  Vector3D n = (r.o + minn * r.d) - o;
  n.normalize();

  if(i != nullptr){
    i->t = minn;
    i->n = n;
    i->primitive = this;
    i->bsdf = this->get_bsdf();
  } 
  return true;
}

void Sphere::draw(const Color &c, float alpha) const {
  Misc::draw_sphere_opengl(o, r, c);
}

void Sphere::drawOutline(const Color &c, float alpha) const {
  // Misc::draw_sphere_opengl(o, r, c);
}

} // namespace SceneObjects
} // namespace CGL
