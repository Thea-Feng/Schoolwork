#include "student_code.h"
#include "mutablePriorityQueue.h"

using namespace std;

namespace CGL
{

  /**
   * Evaluates one step of the de Casteljau's algorithm using the given points and
   * the scalar parameter t (class member).
   *
   * @param points A vector of points in 2D
   * @return A vector containing intermediate points or the final interpolated vector
   */
  std::vector<Vector2D> BezierCurve::evaluateStep(std::vector<Vector2D> const &points)
  { 
    // TODO Task 1.
    if(points.size() == 1) return points;
    std::vector<Vector2D> intermidpoints = std::vector<Vector2D>();
    for (int i = 1; i < points.size(); i++)
      intermidpoints.push_back((1 - t) * points[i - 1] + t * points[i]);
    return intermidpoints;
  }

  /**
   * Evaluates one step of the de Casteljau's algorithm using the given points and
   * the scalar parameter t (function parameter).
   *
   * @param points    A vector of points in 3D
   * @param t         Scalar interpolation parameter
   * @return A vector containing intermediate points or the final interpolated vector
   */
  std::vector<Vector3D> BezierPatch::evaluateStep(std::vector<Vector3D> const &points, double t) const
  {
    // TODO Task 2.
    if(points.size() == 1) return points;
    std::vector<Vector3D> intermidpoints = std::vector<Vector3D>();
    for (int i = 1; i < points.size(); i++)
      intermidpoints.push_back((1 - t) * points[i - 1] + t * points[i]);
    return intermidpoints;
  }

  /**
   * Fully evaluates de Casteljau's algorithm for a vector of points at scalar parameter t
   *
   * @param points    A vector of points in 3D
   * @param t         Scalar interpolation parameter
   * @return Final interpolated vector
   */
  Vector3D BezierPatch::evaluate1D(std::vector<Vector3D> const &points, double t) const
  {
    // TODO Task 2.
    std::vector<Vector3D>intermidpoints = points;
    while(intermidpoints.size() > 1)
      intermidpoints = evaluateStep(intermidpoints, t);
    return intermidpoints[0];
  }

  /**
   * Evaluates the Bezier patch at parameter (u, v)
   *
   * @param u         Scalar interpolation parameter
   * @param v         Scalar interpolation parameter (along the other axis)
   * @return Final interpolated vector
   */
  Vector3D BezierPatch::evaluate(double u, double v) const 
  {  
    // TODO Task 2.
    std::vector<Vector3D> point_u = std::vector<Vector3D>();
    for(int i = 0; i < controlPoints.size(); i++)
      point_u.push_back(evaluate1D(controlPoints[i], u));
    Vector3D point_v = evaluate1D(point_u, v);
    return point_v;
  }

  Vector3D Vertex::normal( void ) const
  {
    // TODO Task 3.
    // Returns an approximate unit normal at this vertex, computed by
    // taking the area-weighted average of the normals of neighboring
    // triangles, then normalizing.
    HalfedgeCIter h = halfedge();
    FaceCIter f = h->face();
    Vector3D Norm;
    do{
      Vector3D vec1 = h->vertex()->position - h->next()->vertex()->position;
      Vector3D vec2 = h->next()->vertex()->position - h->next()->next()->vertex()->position;
      float Area = 0.5 * abs(cross(vec1, vec2).norm());
      Norm += Area * f->normal();
      h = h->twin()->next();
      f = h->face();
    }while(h != halfedge());
    Norm.normalize();
    return Norm;
  }

  EdgeIter HalfedgeMesh::flipEdge( EdgeIter e0 )
  {
    // TODO Task 4.
    // This method should flip the given edge and return an iterator to the flipped edge.
    if(e0->isBoundary()) return e0;
	  // This part refers to http://15462.courses.cs.cmu.edu/fall2015content/misc/HalfedgeEdgeOpImplementationGuide.pdf 
    // given in the requirement of the assignment
    HalfedgeIter h0 = e0->halfedge();
    HalfedgeIter h1 = h0->next();
    HalfedgeIter h2 = h1->next();
    HalfedgeIter h3 = h0->twin();
    HalfedgeIter h4 = h3->next();
    HalfedgeIter h5 = h4->next();
    HalfedgeIter h6 = h1->twin();
    HalfedgeIter h7 = h2->twin();
    HalfedgeIter h8 = h4->twin();
    HalfedgeIter h9 = h5->twin();
    
    VertexIter v0 = h0->vertex();
    VertexIter v1 = h3->vertex();
    VertexIter v2 = h2->vertex();
    VertexIter v3 = h5->vertex();
    
    EdgeIter e1 = h1->edge();
    EdgeIter e2 = h2->edge();
    EdgeIter e3 = h4->edge();
    EdgeIter e4 = h5->edge();
    
    FaceIter f0 = h0->face();
    FaceIter f1 = h3->face();

    // void setNeighbors( HalfedgeIter next,
    //                         HalfedgeIter twin,
    //                         VertexIter vertex,
    //                         EdgeIter edge,
    //                         FaceIter face )

    h0->setNeighbors(h1, h3, v3, e0, f0);
    h1->setNeighbors(h2, h7, v2, e2, f0);
    h2->setNeighbors(h0, h8, v0, e3, f0);
    h3->setNeighbors(h4, h0, v2, e0, f1);
    h4->setNeighbors(h5, h9, v3, e4, f1);
    h5->setNeighbors(h3, h6, v1, e1, f1);
    h6->setNeighbors(h6->next(), h5, v2, e1, h6->face());
    h7->setNeighbors(h7->next(), h1, v0, e2, h7->face());
    h8->setNeighbors(h8->next(), h2, v3, e3, h8->face());
    h9->setNeighbors(h9->next(), h4, v1, e4, h9->face());

    v0->halfedge() = h2;
    v3->halfedge() = h0;
    v2->halfedge() = h3;
    v1->halfedge() = h5;

    e0->halfedge() = h0;
    e1->halfedge() = h5;
    e2->halfedge() = h1;
    e3->halfedge() = h2;
    e4->halfedge() = h4;

    f0->halfedge() = h0;
    f1->halfedge() = h3;

    return e0;
  }

  VertexIter HalfedgeMesh::splitEdge( EdgeIter e0 ){
  //   // TODO Task 5.
  //   // This method should split the given edge and return an iterator to the newly inserted vertex.
  //   // The halfedge of this vertex should point along the edge that was split, rather than the new edges.
    if(e0->isBoundary()){
    return e0->halfedge()->vertex();
}else{
    HalfedgeIter h0 = e0->halfedge();
    HalfedgeIter h1 = h0->next();
    HalfedgeIter h2 = h1->next();
    HalfedgeIter h3 = h0->twin();
    HalfedgeIter h4 = h3->next();
    HalfedgeIter h5 = h4->next();
    HalfedgeIter h6 = h1->twin();
    HalfedgeIter h7 = h2->twin();
    HalfedgeIter h8 = h4->twin();
    HalfedgeIter h9 = h5->twin();
    
    VertexIter v0 = h0->vertex();
    VertexIter v1 = h3->vertex();
    VertexIter v2 = h2->vertex();
    VertexIter v3 = h5->vertex();
    
    EdgeIter e1 = h1->edge();
    EdgeIter e2 = h2->edge();
    EdgeIter e3 = h4->edge();
    EdgeIter e4 = h5->edge();
    
    FaceIter f0 = h0->face();
    FaceIter f1 = h3->face();

    HalfedgeIter hmd = newHalfedge();
    HalfedgeIter hdm = newHalfedge();
    HalfedgeIter ham = newHalfedge();
    HalfedgeIter hma = newHalfedge();
    HalfedgeIter hmc = newHalfedge();
    HalfedgeIter hcm = newHalfedge();

    VertexIter v4 = newVertex();
    v4->position = (v0->position + v1->position)/2.0;
    v4->isNew = true;

    EdgeIter emd = newEdge();
    emd->isNew = true;
    EdgeIter emc = newEdge();
    emc->isNew = false;
    EdgeIter ema = newEdge();
    ema->isNew = true;

    FaceIter f2 = newFace();
	  FaceIter f3 = newFace();

    h0->setNeighbors(hmd, h3, v0, e0, f0);
    hmd->setNeighbors(h2, hdm, v4, emd, f0);
    h2->setNeighbors(h0, h7, v2, e2, f0);
    h7->setNeighbors(h7->next(), h2, v0, e2, h7->face());

    hmc->setNeighbors(h1, hcm, v4, emc, f2);
    h1->setNeighbors(hdm, h6, v1, e1, f2);
    hdm->setNeighbors(hmc, hmd, v2, emd, f2);
    h6->setNeighbors(h6->next(), h1, v2, e1, h6->face());

    hcm->setNeighbors(hma, hmc, v1, emc, f3);
    hma->setNeighbors(h5, ham, v4, ema, f3);
    h5->setNeighbors(hcm, h9, v3, e4, f3);
    h9->setNeighbors(h9->next(), h5, v1, e4, h9->face());

    h3->setNeighbors(h4, h0, v4, e0, f1);
    h4->setNeighbors(ham, h8, v0, e3, f1);
    ham->setNeighbors(h3, hma, v3, ema, f1);
    h8->setNeighbors(h8->next(), h4, v3, e3, h8->face());
    
    v0->halfedge() = h0;
    v1->halfedge() = h1;
    v2->halfedge() = h2;
    v3->halfedge() = h5;
    v4->halfedge() = h3;
    
    e0->halfedge() = h0;
    e1->halfedge() = h1;
    e2->halfedge() = h2;
    e3->halfedge() = h4;
    e4->halfedge() = h5;
    emc->halfedge() = hmc;
    emd->halfedge() = hdm;
    ema->halfedge() = hma;
    
    // faces
    f0->halfedge() = h0;
    f1->halfedge() = h3;
    f2->halfedge() = h1;
    f3->halfedge() = h5;
    
    return v4;
  }
  }

  void MeshResampler::upsample( HalfedgeMesh& mesh )
  {
    // TODO Task 6.
    // This routine should increase the number of triangles in the mesh using Loop subdivision.
    // One possible solution is to break up the method as listed below.

    // 1. Compute new positions for all the vertices in the input mesh, using the Loop subdivision rule,
    // and store them in Vertex::newPosition. At this point, we also want to mark each vertex as being
    // a vertex of the original mesh.
    
    for(VertexIter v = mesh.verticesBegin(); v != mesh.verticesEnd(); v++){
      v->isNew = false;
      std::vector<Vector3D>neighbor = std::vector<Vector3D>();
      Vector3D original_neighbor_position_sum = Vector3D();
      HalfedgeIter h = v->halfedge();
      do{
        neighbor.push_back(h->twin()->vertex()->position);
        h = h->twin()->next();
      }while(h != v->halfedge());
      int n = neighbor.size();
      float u = (n == 3) ? 3.0 / 16 : 3.0 / (8 * n);
      for(int i = 0; i < n; i++) original_neighbor_position_sum += neighbor[i];
      v->newPosition = (1 - n * u) * v->position + u * original_neighbor_position_sum;

    }
    // 2. Compute the updated vertex positions associated with edges, and store it in Edge::newPosition.
    for(EdgeIter e = mesh.edgesBegin(); e != mesh.edgesEnd(); e++){
      VertexIter a = e->halfedge()->vertex();
      VertexIter b = e->halfedge()->twin()->vertex();
      VertexIter c = e->halfedge()->next()->next()->vertex();
      VertexIter d = e->halfedge()->twin()->next()->next()->vertex();
      e->isNew = false;
      e->newPosition = 3.0 / 8 * (a->position + b->position) + 1.0 / 8 * (c->position + d->position);

    }
    // 3. Split every edge in the mesh, in any order. For future reference, we're also going to store some
    // information about which subdivide edges come from splitting an edge in the original mesh, and which edges
    // are new, by setting the flat Edge::isNew. Note that in this loop, we only want to iterate over edges of
    // the original mesh---otherwise, we'll end up splitting edges that we just split (and the loop will never end!)
    for(EdgeIter e = mesh.edgesBegin(); e != mesh.edgesEnd(); e++){
      if(e->isNew == false && e->halfedge()->vertex()->isNew == false && e->halfedge()->twin()->vertex()->isNew == false){
        VertexIter v = mesh.splitEdge(e);
        v->newPosition = e->newPosition;
      }
    }
    // 4. Flip any new edge that connects an old and new vertex.
    // int cnt=0;
    vector<EdgeIter> edges;
    for(EdgeIter e = mesh.edgesBegin(); e != mesh.edgesEnd(); e++){
      if(e->isNew && (e->halfedge()->vertex()->isNew != e->halfedge()->twin()->vertex()->isNew)){
        mesh.flipEdge(e);
        // cnt++;
        // edges.push_back(e);
      }
    }
    // printf("%d\n", cnt);
    // 5. Copy the new vertex positions into final Vertex::position.
    for(VertexIter v = mesh.verticesBegin(); v != mesh.verticesEnd(); v++)
      v->position = v->newPosition;
    
  }
}
