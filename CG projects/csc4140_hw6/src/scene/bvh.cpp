#include "bvh.h"

#include "CGL/CGL.h"
#include "triangle.h"

#include <iostream>
#include <stack>

using namespace std;

namespace CGL {
namespace SceneObjects {

BVHAccel::BVHAccel(const std::vector<Primitive *> &_primitives,
                   size_t max_leaf_size) {

  // primitives = std::vector<Primitive *>(_primitives);
  root = construct_bvh(_primitives, max_leaf_size);
}

BVHAccel::~BVHAccel() {
  if (root)
    delete root;
  primitives.clear();
}

BBox BVHAccel::get_bbox() const { return root->bb; }

void BVHAccel::draw(BVHNode *node, const Color &c, float alpha) const {
  if (node->isLeaf()) {
    for (Primitive *p : *(node->prim))
      p->draw(c, alpha);
    
  } else {
    draw(node->l, c, alpha);
    draw(node->r, c, alpha);
  }
}

void BVHAccel::drawOutline(BVHNode *node, const Color &c, float alpha) const {
  if (node->isLeaf()) {
    for (Primitive *p : *(node->prim))
      p->drawOutline(c, alpha);
  } else {
    drawOutline(node->l, c, alpha);
    drawOutline(node->r, c, alpha);
  }
}
int get_axis(Vector3D extent){
  double maxn = max({extent.x, extent.y, extent.z });
  if(extent.x == maxn) return 0;
  return extent.y == maxn ? 1 : 2;
}
BVHNode *BVHAccel::construct_bvh(const std::vector<Primitive *>&prim,
                                 size_t max_leaf_size) {

  // TODO (Part 2.1):
  // Construct a BVH from the given vector of primitives and maximum leaf
  // size configuration. The starter code build a BVH aggregate with a
  // single leaf node (which is also the root) that encloses all the
  // primitives.

  BBox bbox;

  for (Primitive *p : prim) {
    BBox bb = p->get_bbox();
    bbox.expand(bb);
  }

  BVHNode *node = new BVHNode(bbox);
  node->prim = new vector<Primitive *>(prim);
  stack<BVHNode*> stk;
  stk.push(node);
  while(!stk.empty()){
    BVHNode *now = stk.top();
    stk.pop();
    int size = now->prim->size();
    if(size <= max_leaf_size) continue;
    int axis = get_axis(now->bb.extent);
    Vector3D tot = Vector3D();
    for (Primitive *p : *(now->prim)) {
      Vector3D c = p->get_bbox().centroid();
      tot += c;
    }
    tot /= size;
    std::vector<Primitive *> ls_prim, rs_prim;
    BBox ls_box, rs_box;
    for(Primitive *p: *(now->prim)){
      Vector3D c = p->get_bbox().centroid();
      if(c[axis] < tot[axis]) {
        ls_box.expand(p->get_bbox());
        ls_prim.push_back(p);
      }
      else {
        rs_box.expand(p->get_bbox());
        rs_prim.push_back(p);
      }
	 
    }
	now->prim->clear();
	delete now->prim;
    BVHNode *ls = new BVHNode(ls_box);
    ls->prim = new vector<Primitive *>(ls_prim);
    BVHNode *rs = new BVHNode(rs_box);
    rs->prim = new vector<Primitive *>(rs_prim);
    now->l = ls;
    now->r = rs;
    stk.push(ls);
    stk.push(rs);
  }
  return node;


}

bool BVHAccel::has_intersection(const Ray &ray, BVHNode *node) const {
  // TODO (Part 2.3):
  // Fill in the intersect function.
  // Take note that this function has a short-circuit that the
  // Intersection version cannot, since it returns as soon as it finds
  // a hit, it doesn't actually have to find the closest hit.



  // for (auto p : primitives) {
  //   total_isects++;
  //   if (p->has_intersection(ray))
  //     return true;
  // }
  // return false;
	return intersect(ray, nullptr, node);



}
// int kk;
bool BVHAccel::intersect(const Ray &ray, Intersection *i, BVHNode *node) const {
  // TODO (Part 2.3):
  // Fill in the intersect function.
  double t1 = ray.min_t, t2 = ray.max_t;
  if(node == nullptr || !node->bb.intersect(ray, t1, t2)) return false;
  // kk++;
  // if(kk%100 == 0)cout<<kk<<endl;
  bool hit = false;
  if(node->isLeaf()){
    for(Primitive *p: *(node->prim)){
      total_isects++;
      if(p->intersect(ray, i)) hit = true;
    }
    return hit;
  }

  hit |= intersect(ray, i, node->l);
  hit |= intersect(ray, i, node->r);
  return hit;


}

} // namespace SceneObjects
} // namespace CGL
