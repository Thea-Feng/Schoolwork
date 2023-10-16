#include "camera.h"

#include <iostream>
#include <sstream>
#include <fstream>

#include "CGL/misc.h"
#include "CGL/vector2D.h"
#include "CGL/vector3D.h"

using std::cout;
using std::endl;
using std::max;
using std::min;
using std::ifstream;
using std::ofstream;

namespace CGL {

using Collada::CameraInfo;

Ray Camera::generate_ray_for_thin_lens(double x, double y, double rndR, double rndTheta) const {

  // TODO Assignment 7: Part 4
  // compute position and direction of ray from the input sensor sample coordinate.
  // Note: use rndR and rndTheta to uniformly sample a unit disk.
  double xx = (x - 0.5) / 0.5 * tan(radians(hFov)/2.0);
  double yy = (y - 0.5) / 0.5 * tan(radians(vFov)/2.0);
  Vector3D pLens_cam = Vector3D(lensRadius*sqrt(rndR)*cos(rndTheta), lensRadius*sqrt(rndR)*sin(rndTheta), 0);
  Vector3D pLens_world = c2w * pLens_cam;
  
  Vector3D senor_cam(xx, yy, -1);
  Vector3D pFocus_cam = senor_cam * focalDistance;
  Vector3D pFocus_world = c2w * pFocus_cam;
  

  Vector3D dir = (pFocus_world - pLens_world).unit();

  Ray r(pLens_world + pos, dir);
  r.min_t = nClip;
  r.max_t = fClip;


  return r;
  return Ray(pos, Vector3D(0, 0, -1));
}


} // namespace CGL
