#include "bsdf.h"

#include <algorithm>
#include <iostream>
#include <utility>

#include "application/visual_debugger.h"

using std::max;
using std::min;
using std::swap;
// ./pathtracer -t 8 -s 256 -l 2 -m 10 -r 480 360 ../dae/sky/CBdragon_microfacet_au.dae
namespace CGL {

// Mirror BSDF //

Vector3D MirrorBSDF::f(const Vector3D wo, const Vector3D wi) {
  return Vector3D();
}

Vector3D MirrorBSDF::sample_f(const Vector3D wo, Vector3D* wi, double* pdf) {

  // TODO Assignment 7: Part 1
  // Implement MirrorBSDF
  return Vector3D();
}

void MirrorBSDF::render_debugger_node()
{
  if (ImGui::TreeNode(this, "Mirror BSDF"))
  {
    DragDouble3("Reflectance", &reflectance[0], 0.005);
    ImGui::TreePop();
  }
}

// Microfacet BSDF //

double MicrofacetBSDF::G(const Vector3D wo, const Vector3D wi) {
  return 1.0 / (1.0 + Lambda(wi) + Lambda(wo));
}

double MicrofacetBSDF::D(const Vector3D h) {
  // TODO Assignment 7: Part 2
  // Compute Beckmann normal distribution function (NDF) here.
  // You will need the roughness alpha.
  double fenzi = exp(-pow(sin_theta(h)/cos_theta(h), 2) / pow(alpha, 2));
  double fenmu = PI * alpha * alpha * pow(cos_theta(h), 4);
  return fenzi / fenmu;
}

Vector3D MicrofacetBSDF::F(const Vector3D wi) {
  // TODO Assignment 7: Part 2
  // Compute Fresnel term for reflection on dielectric-conductor interface.
  // You will need both eta and etaK, both of which are Vector3D.
  double val = pow(cos_theta(wi), 2);
  Vector3D term = Vector3D(val, val, val);
  Vector3D Rs_fenzi = eta*eta + k*k - 2*eta*cos_theta(wi) + term;
  Vector3D Rs_fenmu = eta*eta + k*k + 2*eta*cos_theta(wi) + term;
  Vector3D Rs = Rs_fenzi / Rs_fenmu;

  Vector3D Rp_fenzi = eta*eta + k*k - 2*eta*cos_theta(wi) + Vector3D(1, 1, 1);
  Vector3D Rp_fenmu = eta*eta + k*k + 2*eta*cos_theta(wi) + Vector3D(1, 1, 1);
  Vector3D Rp = Rs_fenzi / Rs_fenmu;
  return (Rs + Rp) / 2;
}

Vector3D MicrofacetBSDF::f(const Vector3D wo, const Vector3D wi) {
  // TODO Assignment 7: Part 2
  // Implement microfacet model here.
  if(!(wo.z > 0 && wi.z > 0)) return Vector3D();
  Vector3D h = (wo + wi).unit();
  return F(wi) * G(wo, wi) * D(h) / (4.0 * wo.z * wi.z);
}

Vector3D MicrofacetBSDF::sample_f(const Vector3D wo, Vector3D* wi, double* pdf) {
  // TODO Assignment 7: Part 2
  // *Importance* sample Beckmann normal distribution function (NDF) here.
  // Note: You should fill in the sampled direction *wi and the corresponding *pdf,
  //       and return the sampled BRDF value.
  Vector2D r = sampler.get_sample();
  float thetah = atan(sqrt(-alpha*alpha * log(1 - r[0])));
  float phih = 2 * PI * r[1];
  float pphih = 1 / (2.0 * PI);
  float pthetah = 2*sin(thetah) / (alpha*alpha*pow(cos(thetah), 3)) * exp(-pow(tan(thetah)/alpha, 2));
  float pwh = pthetah * pphih / sin(thetah);
  Vector3D h = Vector3D(sin(thetah)*cos(phih), sin(thetah)*sin(phih), cos(thetah));
	*wi = -wo + 2.0*dot(wo, h)*h;
	if (!(wi->z > 0)) {
		*pdf = 0;
		return Vector3D();
	}
  *pdf = pwh / (4.0* dot(*wi, h));
  // *wi = cosineHemisphereSampler.get_sample(pdf); // cosine sampling

  return MicrofacetBSDF::f(wo, *wi);
}

void MicrofacetBSDF::render_debugger_node()
{
  if (ImGui::TreeNode(this, "Micofacet BSDF"))
  {
    DragDouble3("eta", &eta[0], 0.005);
    DragDouble3("K", &k[0], 0.005);
    DragDouble("alpha", &alpha, 0.005);
    ImGui::TreePop();
  }
}

// Refraction BSDF //

Vector3D RefractionBSDF::f(const Vector3D wo, const Vector3D wi) {
  return Vector3D();
}

Vector3D RefractionBSDF::sample_f(const Vector3D wo, Vector3D* wi, double* pdf) {
  // TODO Assignment 7: Part 1
  // Implement RefractionBSDF
  return Vector3D();
}

void RefractionBSDF::render_debugger_node()
{
  if (ImGui::TreeNode(this, "Refraction BSDF"))
  {
    DragDouble3("Transmittance", &transmittance[0], 0.005);
    DragDouble("ior", &ior, 0.005);
    ImGui::TreePop();
  }
}

// Glass BSDF //

Vector3D GlassBSDF::f(const Vector3D wo, const Vector3D wi) {
  return Vector3D();
}

Vector3D GlassBSDF::sample_f(const Vector3D wo, Vector3D* wi, double* pdf) {

  // TODO Assignment 7: Part 1
  // Compute Fresnel coefficient and either reflect or refract based on it.

  // compute Fresnel coefficient and use it as the probability of reflection
  // - Fundamentals of Computer Graphics page 305
  return Vector3D();
}

void GlassBSDF::render_debugger_node()
{
  if (ImGui::TreeNode(this, "Refraction BSDF"))
  {
    DragDouble3("Reflectance", &reflectance[0], 0.005);
    DragDouble3("Transmittance", &transmittance[0], 0.005);
    DragDouble("ior", &ior, 0.005);
    ImGui::TreePop();
  }
}

void BSDF::reflect(const Vector3D wo, Vector3D* wi) {

  // TODO Assignment 7: Part 1
  // Implement reflection of wo about normal (0,0,1) and store result in wi.
  Vector3D n(0, 0, 1);
  *wi = -wo + 2 * dot(wo, n) * n;

}

bool BSDF::refract(const Vector3D wo, Vector3D* wi, double ior) {

  // TODO Assignment 7: Part 1
  // Use Snell's Law to refract wo surface and store result ray in wi.
  // Return false if refraction does not occur due to total internal reflection
  // and true otherwise. When dot(wo,n) is positive, then wo corresponds to a
  // ray entering the surface through vacuum.

  return true;

}

} // namespace CGL
