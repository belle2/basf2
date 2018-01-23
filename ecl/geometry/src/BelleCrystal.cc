#include "ecl/geometry/BelleCrystal.h"

#include "globals.hh"

#include "G4VoxelLimits.hh"
#include "G4AffineTransform.hh"

#include "G4VPVParameterisation.hh"

#include "Randomize.hh"

#include "G4VGraphicsScene.hh"

//#include "G4Trap.hh"
#include <map>
using namespace Belle2;
using namespace std;
using namespace ECL;

#define DO_QUOTE(X)        #X
#define QUOTE(X)           DO_QUOTE(X)

#define PERFCOUNTER 0
//#define MATCHNAME sv_fwd_crystal2

#if PERFCOUNTER==1
int counter[6];
#  define COUNTER(x) counter[x]++
#else
#  define COUNTER(x)
#endif

#ifdef MATCHNAME
#  define MATCHOUT(x) if(fmatch) cout<<x<<"\n";
#else
#  define MATCHOUT(x) {}
#endif

// Accuracy of coplanarity
const G4double kCoplanar_Tolerance = 1E-4;

BelleCrystal::BelleCrystal(const G4String& pName, int n,
                           const G4ThreeVector* pt)
  : G4CSGSolid(pName), nsides(n), fPlanes(nsides), fx(2 * nsides)
{
#if PERFCOUNTER==1
  memset(fcounter, 0, sizeof(fcounter));
#endif
#ifdef MATCHNAME
  fmatch = GetName() == QUOTE(MATCHNAME);
  cout.precision(17);
#endif
  //  ref = new G4Trap("dummy",pt);
  fDz = abs(pt[2 * nsides - 1].z());
  for (unsigned int i = 0; i < 2 * nsides; i++) {
    fx[i].x = pt[i].x();
    fx[i].y = pt[i].y();
  }
  //  cout<<pName<<" "<<fx.data()<<endl;
  //  for(int i=0; i<2*nsides + 2*(nsides-2); i++) ivertx(i);

  auto isConvex = [](std::vector<Point_t>::const_iterator begin, std::vector<Point_t>::const_iterator end) -> bool {
    bool sign = false;
    int np = end - begin;
    for (int i0 = 0; i0 < np; i0++)
    {
      int i1 = (i0 + 1) % np, i2 = (i0 + 2) % np;
      const Point_t& r2 = *(begin + i2), &r1 = *(begin + i1), &r0 = *(begin + i0);
      double dx1 = r2.x - r1.x, dy1 = r2.y - r1.y;
      double dx2 = r0.x - r1.x, dy2 = r0.y - r1.y;
      double x = dx1 * dy2 - dy1 * dx2;
      if (i0 == 0) sign = x > 0;
      else {
        if (sign != (x > 0)) return false;
      }
    }
    return true;
  };

  auto isClockwise = [](std::vector<Point_t>::const_iterator begin, std::vector<Point_t>::const_iterator end) -> bool {
    std::vector<Point_t>::const_iterator it = begin;
    Point_t r0 = *it++;
    double sum = 0;
    for (; it != end;)
    {
      Point_t r1 = *it++; sum += (r1.x - r0.x) * (r1.y + r0.y);
      r0 = r1;
    }
    Point_t r1 = *begin; sum += (r1.x - r0.x) * (r1.y + r0.y);
    return sum > 0;
  };

  if (!isConvex(fx.begin(), fx.begin() + nsides)) {
    std::ostringstream message;
    message << "At -z polygon is not convex: " << GetName();
    G4Exception("BelleCrystal::BelleCrystal()", "BelleCrystal", FatalException, message);
  }
  if (!isConvex(fx.begin() + nsides, fx.end())) {
    std::ostringstream message;
    message << "At +z polygon is not convex: " << GetName();
    G4Exception("BelleCrystal::BelleCrystal()", "BelleCrystal", FatalException, message);
  }

  if (isClockwise(fx.begin(), fx.begin() + nsides)) {
    std::ostringstream message;
    message << "At -z polygon is not in anti-clockwise order: " << GetName();
    G4Exception("BelleCrystal::BelleCrystal()", "BelleCrystal", FatalException, message);
  }
  if (isClockwise(fx.begin() + nsides, fx.end())) {
    std::ostringstream message;
    message << "At +z polygon is not in anti-clockwise order: " << GetName();
    G4Exception("BelleCrystal::BelleCrystal()", "BelleCrystal", FatalException, message);
  }

  // sanity check
  const G4ThreeVector* pm = pt, *pp = pt + nsides;
  bool zm = pm[0].z() < 0, zp = pp[0].z() > 0;
  for (unsigned int i = 1; i < nsides; i++) {
    zm = zm && (pm[i - 1].z() - pm[i].z()) < kCarTolerance;
    zp = zp && (pp[i - 1].z() - pp[i].z()) < kCarTolerance;
  }
  bool zpm = abs(pm[0].z() + pp[0].z()) < kCarTolerance;
  if (!(zm && zp && zpm)) {
    std::ostringstream message;
    message << "Invalid vertice coordinates for Solid: " << GetName() << " " << zp << " " << zm << " " << zpm;
    G4Exception("BelleCrystal::BelleCrystal()", "BelleCrystal", FatalException, message);
  }
  if (nsides > 3) {
    for (unsigned int i = 0; i < nsides; i++)
      MakePlane(pt[i], pt[i + nsides], pt[((i + 1) % (nsides)) + nsides], pt[(i + 1) % nsides], fPlanes[i]);
  } else {
    std::ostringstream message;
    message << "Wrong number of sides for Belle Crystal: " << GetName() << " nsides = " << nsides;
    G4Exception("BelleCrystal::BelleCrystal()", "BelleCrystal", FatalException, message);
  }
}

// Nominal constructor for BelleCrystal
BelleCrystal::BelleCrystal(const G4String& pName)
  : G4CSGSolid(pName), nsides(0), fDz(0)
{
}

// Fake default constructor - sets only member data and allocates memory
//                            for usage restricted to object persistency.
BelleCrystal::BelleCrystal(__void__& a)
  : G4CSGSolid(a), nsides(0), fDz(0)
{
}

// Destructor
BelleCrystal::~BelleCrystal()
{
#if PERFCOUNTER==1
  cout << GetName() << " ";
  for (int i = 0; i < 6; i++) cout << counter[i] << " "; cout << endl;
  exit(0);
#endif
  //  delete ref;
}

// Copy constructor
BelleCrystal::BelleCrystal(const BelleCrystal& rhs)
  : G4CSGSolid(rhs), nsides(rhs.nsides), fDz(rhs.fDz), fPlanes(rhs.fPlanes), fx(rhs.fx)
{
}

// Assignment operator
BelleCrystal& BelleCrystal::operator = (const BelleCrystal& rhs)
{
  // Check assignment to self
  if (this == &rhs)  { return *this; }

  // Copy base class data
  G4CSGSolid::operator=(rhs);

  // Copy data
  nsides = rhs.nsides;
  fDz = rhs.fDz;
  fx = rhs.fx;
  fPlanes = rhs.fPlanes;

  return *this;
}

// Calculate the coef's of the plane p1->p2->p3->p4->p1
// where the ThreeVectors 1-4 are in anti-clockwise order when viewed from
// infront of the plane (i.e. from normal direction).
//
// Return true if the ThreeVectors are coplanar + set coef;s
//        false if ThreeVectors are not coplanar
G4bool BelleCrystal::MakePlane(const G4ThreeVector& p1, const G4ThreeVector& p2,
                               const G4ThreeVector& p3, const G4ThreeVector& p4,
                               Plane_t& plane) const
{
  G4ThreeVector v12    = p2 - p1;
  G4ThreeVector v13    = p3 - p1;
  G4ThreeVector v14    = p4 - p1;
  G4ThreeVector Vcross = v12.cross(v13);

  if (std::fabs(Vcross.dot(v14) / (Vcross.mag()*v14.mag())) > kCoplanar_Tolerance) {
    std::ostringstream message;
    message << "Verticies are not in the same plane: " << GetName() << " volume =" << Vcross.dot(v14);
    G4Exception("BelleCrystal::MakePlane()", "BelleCrystal", FatalException, message);
  }

  double a = +(p4.y() - p2.y()) * (p3.z() - p1.z()) - (p3.y() - p1.y()) * (p4.z() - p2.z());
  double b = -(p4.x() - p2.x()) * (p3.z() - p1.z()) + (p3.x() - p1.x()) * (p4.z() - p2.z());
  double c = +(p4.x() - p2.x()) * (p3.y() - p1.y()) - (p3.x() - p1.x()) * (p4.y() - p2.y());
  double sd = sqrt(a * a + b * b + c * c); // so now vector plane.(a,b,c) is unit
  a /= sd;
  b /= sd;
  c /= sd;
  plane.n = G4ThreeVector(a, b, c);
  plane.d = -(a * p1.x() + b * p1.y() + c * p1.z());

  // plane.n = (p4-p2).cross(p3-p1).unit();
  // plane.d =-(plane.n*p1);
  return true;
}

// Dispatch to parameterisation for replication mechanism dimension
// computation & modification.
void BelleCrystal::ComputeDimensions(G4VPVParameterisation*,
                                     const G4int,
                                     const G4VPhysicalVolume*)
{
  G4Exception("BelleCrystal::ComputeDimensions()",
              "GeomSolids0001", FatalException,
              "BelleCrystal does not support Parameterisation.");
  // std::cout<<"ComputeDimensions"<<std::endl;
  // p->ComputeDimensions(*this,n,pRep);
}

// by component min
G4ThreeVector min(const G4ThreeVector& a, const G4ThreeVector& b)
{
  return G4ThreeVector(std::min(a.x(), b.x()), std::min(a.y(), b.y()), std::min(a.z(), b.z()));
}

// by component max
G4ThreeVector max(const G4ThreeVector& a, const G4ThreeVector& b)
{
  return G4ThreeVector(std::max(a.x(), b.x()), std::max(a.y(), b.y()), std::max(a.z(), b.z()));
}

// Calculate extent under transform and specified limit
G4bool BelleCrystal::CalculateExtent(const EAxis pAxis,
                                     const G4VoxelLimits& bb,
                                     const G4AffineTransform& pTransform,
                                     G4double& pMin, G4double& pMax) const
{
  const double inf = std::numeric_limits<double>::infinity();
  G4ThreeVector v0(inf, inf, inf), v1(-inf, -inf, -inf);
  for (unsigned int i = 0; i < 2 * nsides; i++) {
    G4ThreeVector v = pTransform.TransformPoint(vertex(i));
    v0 = min(v0, v);
    v1 = max(v1, v);
  }
  G4ThreeVector b0(bb.GetMinXExtent(), bb.GetMinYExtent(), bb.GetMinZExtent());
  G4ThreeVector b1(bb.GetMaxXExtent(), bb.GetMaxYExtent(), bb.GetMaxZExtent());

  v0 = max(v0, b0);
  v1 = min(v1, b1);

  switch (pAxis) {
    case kXAxis: pMin = v0.x(); pMax = v1.x(); break;
    case kYAxis: pMin = v0.y(); pMax = v1.y(); break;
    case kZAxis: pMin = v0.z(); pMax = v1.z(); break;
    default:                               break;
  }

  G4bool flag = false;
  if ((pMin < inf) || (pMax > -inf)) {
    flag = true;
    // Add tolerance to avoid precision troubles
    pMin -= kCarTolerance ;
    pMax += kCarTolerance ;
  }
  // do {
  //   double t_pMin, t_pMax;
  //   bool t_flag = ref->CalculateExtent(pAxis, bb, pTransform, t_pMin, t_pMax);
  //   if(flag!=t_flag||abs(t_pMin-pMin)>kCarTolerance||abs(t_pMax-pMax)>kCarTolerance){
  //     cout<<GetName()<<" "<<pAxis<<" "<<bb<<" "<<pTransform<<t_pMin-pMin<<" "<<t_pMax-pMax<<endl;
  //     exit(0);
  //   }
  // } while(0);
  return flag;
}

// Return whether point inside/outside/on surface, using tolerance
EInside BelleCrystal::Inside(const G4ThreeVector& p) const
{
  COUNTER(0);
  MATCHOUT("Inside(p) " << p.x() << " " << p.y() << " " << p.z());
  //  return ref->Inside(p);
  double d = std::fabs(p.z()) - fDz;
  unsigned int i = 0;
  do {
    const Plane_t& t = fPlanes[i++];
    d = max(t.n * p + t.d, d);
  } while (i < nsides);
  const G4double delta = 0.5 * kCarTolerance;
  int in = 0;
  in += d <= delta;
  in += d <= -delta;

  EInside res = EInside(in);
  // if(res != ref->Inside(p)){
  //   cout<<GetName()<<" "<<p<<endl;
  //   exit(0);
  // }
  return res;
}

// Calculate side nearest to p, and return normal
// If 2+ sides equidistant, first side's normal returned (arbitrarily)
G4ThreeVector BelleCrystal::SurfaceNormal(const G4ThreeVector& p) const
{
  //  return ref->SurfaceNormal(p);
  COUNTER(1);
  MATCHOUT("SurfaceNormal(p) " << p.x() << " " << p.y() << " " << p.z());
  const G4double delta = 0.5 * kCarTolerance;
  G4double safe = kInfinity;
  unsigned int iside = 0, kside = 0;
  vector<double> adist(nsides + 2);
  auto dist = [delta, &safe, &iside, &kside, &adist](double d, unsigned int i) -> void {
    d = std::abs(d);
    adist[i] = d;
    iside = (d < safe) ? i : iside;
    safe = min(d, safe);
    kside += d < delta;
  };

  unsigned int i = 0;
  do {
    const Plane_t& t = fPlanes[i];
    dist(t.n * p + t.d, i++);
  } while (i < nsides);
  do {
    dist(p.z() - fDz, i++);
    dist(p.z() + fDz, i++);
  } while (0);

  G4ThreeVector res;
  if (kside > 1) {
    i = 0;
    do {
      if (adist[i++] < delta) res += fPlanes[i].n;
    } while (i < nsides);
    do {
      if (adist[i++] < delta) res.setZ(res.z() + 1.0);
      if (adist[i++] < delta) res.setZ(res.z() - 1.0);
    } while (0);
    res = res.unit();
  } else {
    if (iside < nsides) {
      res = fPlanes[iside].n;
    } else if (iside == nsides) {
      res = G4ThreeVector(0, 0, 1);
    } else {
      res = G4ThreeVector(0, 0, -1);
    }
  }
  // if((res- ref->SurfaceNormal(p)).mag()>kCarTolerance){
  //   cout<<GetName()<<" "<<p<<" "<<res-ref->SurfaceNormal(p)<<endl;
  //   exit(0);
  // }

  return res;
}

// Calculate exact shortest distance to any boundary from outside
// This is the best fast estimation of the shortest distance to trap
// - Returns 0 is ThreeVector inside
G4double BelleCrystal::DistanceToIn(const G4ThreeVector& p) const
{
  //  return ref->DistanceToIn(p);
  COUNTER(2);
  MATCHOUT("DistanceToIn(p) " << p.x() << " " << p.y() << " " << p.z());
  G4double d = std::fabs(p.z()) - fDz;
  unsigned int i = 0;
  do {
    const Plane_t& t = fPlanes[i++];
    d = max(t.n * p + t.d, d);
  } while (i < nsides);
  d = max(d, 0.0);

  // if(abs(d - ref->DistanceToIn(p))>kCarTolerance){
  //   cout<<GetName()<<" "<<p<<" "<<d-ref->DistanceToIn(p)<<endl;
  //   exit(0);
  // }
  //  if(fmatch&&d==0.0) cout<<"inside\n";
  return d;
}

// Calculate exact shortest distance to any boundary from inside
// - Returns 0 is ThreeVector outside
G4double BelleCrystal::DistanceToOut(const G4ThreeVector& p) const
{
  //  return ref->DistanceToOut(p);
  COUNTER(3);
  MATCHOUT("DistanceToOut(p) " << p.x() << " " << p.y() << " " << p.z());
  G4double d = fDz - std::fabs(p.z());
  unsigned int i = 0;
  do {
    const Plane_t& t = fPlanes[i++];
    d = min(-(t.n * p + t.d), d);
  } while (i < nsides);
  d = max(d, 0.0);
  // if(abs(d - ref->DistanceToOut(p))>0){
  //   cout<<GetName()<<" "<<p<<" "<<d<<" "<<ref->DistanceToOut(p)<<" "<<fDz-ref->GetZHalfLength()<<endl;
  //   exit(0);
  // }
  return d;
}

// Calculate distance to shape from outside - return kInfinity if no intersection
//
// ALGORITHM:
// For each component, calculate pair of minimum and maximum intersection
// values for which the particle is in the extent of the shape
// - The smallest (MAX minimum) allowed distance of the pairs is intersect
G4double BelleCrystal::DistanceToIn(const G4ThreeVector& p,
                                    const G4ThreeVector& v) const
{
  //  return ref->DistanceToIn(p,v);
  COUNTER(4);
  MATCHOUT("DistanceToIn(p,v) " << p.x() << " " << p.y() << " " << p.z() << " " << v.x() << " " << v.y() << " " << v.z());
  G4double delta = 0.5 * kCarTolerance;
  double tin = 0, tout = kInfinity;
  auto outside = [delta, &tin, &tout](double d, double nv) -> bool {
    double t = -d / nv;
    if (nv < 0)
      tin  = max(tin , t);
    else {
      if (d >= -delta) return true;
      tout = min(tout, t);
    }
    return false;
  };

  unsigned int i = 0;
  do {
    const Plane_t& t = fPlanes[i++];
    if (outside(t.n * p + t.d, t.n * v)) {tin = kInfinity; goto exit;}
  } while (i < nsides);
  do {
    if (outside(p.z() - fDz, v.z())) {tin = kInfinity; goto exit;}
    if (outside(-p.z() - fDz, -v.z())) {tin = kInfinity; goto exit;}
  } while (0);
exit:
  double res = (tin > tout) ? kInfinity : tin;
  // G4int oldprc = cout.precision(16);
  // if(nsides==5) cout<<GetName()<<" "<<p.x()<<" "<<p.y()<<" "<<p.z()<<" "<<v.x()<<" "<<v.y()<<" "<<v.z()<<" "<<res<<endl;
  // cout.precision(oldprc);

  // if(abs(res - ref->DistanceToIn(p,v))>kCarTolerance){
  //   cout<<GetName()<<" "<<p<<" "<<v<<" "<<res-ref->DistanceToIn(p,v)<<endl;
  //   exit(0);
  // }
  return res;
}

// Calculate distance to surface of shape from inside
// Calculate distance to x/y/z planes - smallest is exiting distance
G4double BelleCrystal::DistanceToOut(const G4ThreeVector& p, const G4ThreeVector& v,
                                     const G4bool calcNorm, G4bool* IsValid, G4ThreeVector* n) const
{
  //  return ref->DistanceToOut(p,v,calcNorm,IsValid,n);
  COUNTER(5);
  MATCHOUT("DistanceToOut(p,v) " << p.x() << " " << p.y() << " " << p.z() << " " << v.x() << " " << v.y() << " " << v.z() << " " <<
           calcNorm);
  const G4double delta = 0.5 * kCarTolerance;
  unsigned int iside = 10;
  G4double lmin = kInfinity;

  auto outside = [delta, &lmin, &iside](double d, double nv, unsigned int i) -> bool {
    if (d > delta) // definitly outside
    {
      lmin = 0; iside = i; return true;
    } else {
      bool c = nv > 0;
      if (d < -delta) // definitly inside
      {
        if (c) { // has to point to the same direction
          d = -d;
          if (d < nv * lmin) {lmin = d / nv; iside = i;}
        }
      } else { // surface
        if (c) // points outside
        {
          lmin = 0; iside = i; return true;
        }
      }
    }
    return false;
  };

  unsigned int i = 0;
  do {
    const Plane_t& t = fPlanes[i];
    if (outside(t.n * p + t.d, t.n * v, i++)) goto exit;
  } while (i < nsides);
  do {
    if (outside(p.z() - fDz, v.z(), i++)) goto exit;
    if (outside(-p.z() - fDz, -v.z(), i++)) goto exit;
  } while (0);
exit:
  if (calcNorm) {
    *IsValid = true;
    if (iside < nsides) {
      *n = fPlanes[iside].n;
    } else if (iside == nsides) {
      *n = G4ThreeVector(0, 0, 1);
    } else {
      *n = G4ThreeVector(0, 0, -1);
    }
  }
  // G4int oldprc = cout.precision(16);
  // if(nsides==5) cout<<GetName()<<" "<<p.x()<<" "<<p.y()<<" "<<p.z()<<" "<<v.x()<<" "<<v.y()<<" "<<v.z()<<" "<<calcNorm<<" "<<lmin<<endl;
  // cout.precision(oldprc);
  // do {
  //   G4bool t_IsValid; G4ThreeVector t_n;
  //   double t_lmin = ref->DistanceToOut(p,v,calcNorm,&t_IsValid,&t_n);
  //   if(abs(lmin - t_lmin)>kCarTolerance||*IsValid!=t_IsValid||(*n - t_n).mag()>kCarTolerance){
  //     cout<<GetName()<<" "<<p<<" "<<v<<" "<<lmin-t_lmin<<(*n-t_n)<<endl;
  //     exit(0);
  //   }
  // } while(0);
  return lmin;
}

// GetEntityType
G4GeometryType BelleCrystal::GetEntityType() const
{
  return G4String("BelleCrystal");
}

// Make a clone of the object
G4VSolid* BelleCrystal::Clone() const
{
  return new BelleCrystal(*this);
}

// Stream object contents to an output stream
std::ostream& BelleCrystal::StreamInfo(std::ostream& os) const
{
  G4int oldprc = os.precision(16);
  os << "-----------------------------------------------------------\n"
     << "    *** Dump for solid - " << GetName() << " ***\n"
     << "    ===================================================\n"
     << " Solid type: BelleCrystal\n"
     << " Parameters: \n"
     << "    crystal side plane equations:\n"
     << "-----------------------------------------------------------\n";
  os.precision(oldprc);

  return os;
}

// triangle vertices are ordered in anti-clockwise order looking from outside the solid
unsigned int ivertx4[][3] = {{0, 5, 4}, {0, 1, 5}, {1, 7, 5}, {1, 3, 7}, {3, 6, 7}, {3, 2, 6}, {2, 4, 6}, {2, 0, 4}, {0, 3, 1}, {0, 2, 3}, {4, 5, 7}, {4, 7, 6}};
unsigned int ivertx5[][3] = {{0, 6, 5}, {0, 1, 6}, {1, 7, 6}, {1, 2, 7}, {2, 8, 7}, {2, 3, 8}, {3, 9, 8}, {3, 4, 9}, {4, 5, 9}, {4, 0, 5}, {0, 2, 1}, {0, 3, 2}, {0, 4, 3}, {5, 6, 7}, {5, 7, 8}, {5, 8, 9}};

const unsigned int* BelleCrystal::ivertx(unsigned int it) const
{
  static unsigned int vert[3];
  //  if(nsides==4) return ivertx4[it];
  if (nsides == 0) return ivertx4[it];
  //  else if(nsides==5) { vert[0] = ivertx5[it][0], vert[1] = ivertx5[it][1], vert[2] = ivertx5[it][2];}
  else {
    if (it < 2 * nsides) {
      int j = it / 2;
      if ((it & 1) == 0) {
        vert[0] = j, vert[2] = j + nsides, vert[1] = ((j + 1) % nsides) + nsides;
      } else {
        vert[0] = j, vert[2] = ((j + 1) % nsides) + nsides, vert[1] = (j + 1) % nsides;
      }
    } else if (it < 2 * nsides + (nsides - 2)) {
      int j = it - 2 * nsides;
      vert[0] = 0, vert[2] = 1 + j, vert[1] = 2 + j;
    } else {
      int j = it - (2 * nsides + (nsides - 2));
      vert[0] = nsides, vert[2] = nsides + 2 + j, vert[1] = nsides + 1 + j;
    }
  }
  //  cout<<it<<" {"<<vert[0]<<", "<<vert[1]<<", "<<vert[2]<<"}"<<endl;
  return vert;
}

G4ThreeVector crossplanes(const Plane_t& p0, const Plane_t& p1, double Dz)
{
  const G4ThreeVector& n0 = p0.n;
  const G4ThreeVector& n1 = p1.n;
  double A = n0.z() * Dz + p0.d;
  double B = n1.z() * Dz + p1.d;
  double iD = 1 / (n0.y() * n1.x() - n0.x() * n1.y());
  double x = (A * n1.y() - B * n0.y()) * iD;
  double y = (B * n0.x() - A * n1.x()) * iD;
  double z =  Dz;
  return G4ThreeVector(x, y, z);
}

#define PACK(k0,k1,w,n) ((((k0)<<(0))+((k1)<<(w)))<<((2*w)*n))
G4ThreeVector BelleCrystal::vertex(unsigned int i) const
{
  double Dz = (i < nsides) ? -fDz : fDz;

  // if(nsides==4) {
  //   const unsigned char w = 2, mask = ((1<<w)-1);
  //   unsigned int map = PACK(0,2,w,0)+PACK(0,3,w,1)+PACK(1,2,w,2)+PACK(1,3,w,3); // 20 bits
  //   map >>= (2*w)*(i%4);
  //   int k0 = map&mask, k1 = (map>>2)&mask;
  //   return crossplanes(fPlanes[k0],fPlanes[k1],Dz);
  // } else if(nsides==5){
  //   const unsigned char w = 3, mask = ((1<<w)-1);
  //   unsigned int map = PACK(0,4,w,0)+PACK(1,0,w,1)+PACK(2,1,w,2)+PACK(3,2,w,3)+PACK(4,3,w,4); // 30 bits
  //   map >>= (2*w)*(i%5);
  //   int k0 = map&mask, k1 = (map>>w)&mask;
  //   return crossplanes(fPlanes[k0],fPlanes[k1],Dz);
  // }
  return G4ThreeVector(fx[i].x, fx[i].y, Dz);
}
#undef PACK

// uniformly sampled point over triangle
G4ThreeVector BelleCrystal::GetPointOnTriangle(int it) const
{
  // barycentric coordinates
  double a1 = CLHEP::RandFlat::shoot(0., 1.), a2 = CLHEP::RandFlat::shoot(0., 1.);
  if (a1 + a2 > 1) { a1 = 1 - a1; a2 = 1 - a2;}
  double a0 = 1 - (a1 + a2);
  const unsigned int* iv = ivertx(it);
  G4ThreeVector p0 = vertex(iv[0]), p1 = vertex(iv[1]), p2 = vertex(iv[2]);
  G4ThreeVector r1(p0.x()*a0 + p1.x()*a1 + p2.x()*a2,
                   p0.y()*a0 + p1.y()*a1 + p2.y()*a2,
                   p0.z()*a0 + p1.z()*a1 + p2.z()*a2);
  return r1;
}

double BelleCrystal::area(int it, double& vol) const
{
  const unsigned int* iv = ivertx(it);
  G4ThreeVector p0 = vertex(iv[0]), p1 = vertex(iv[1]), p2 = vertex(iv[2]);
  //  std::cout<<it<<" "<<p0<<" "<<p1<<" "<<p2<<std::endl;
  G4ThreeVector n = (p1 - p0).cross(p2 - p0);
  vol = n * p0;
  return 0.5 * n.mag();
}

double BelleCrystal::getvolarea() const
{
  fareas.clear();
  int nt = 2 * nsides + 2 * (nsides - 2); // total number of triangles
  fareas.reserve(nt);
  double totarea = 0, totvol = 0;
  for (int i = 0; i < nt; i++) {
    double v, s = area(i, v);
    totarea += s;
    totvol  += v;
    fareas.push_back(totarea);
    //    cout<<i<<" "<<s<<" "<<v<<endl;
  }
  return totvol / 6;
}

G4double BelleCrystal::GetCubicVolume()
{
  //  cout<<GetName()<<" GetCubicVolume "<<fCubicVolume<<endl;
  if (fCubicVolume == 0.0) {
    fCubicVolume = getvolarea();
    fSurfaceArea = fareas.back();
  }
  return fCubicVolume;
}

G4double BelleCrystal::GetSurfaceArea()
{
  if (fSurfaceArea == 0.0) {
    fCubicVolume = getvolarea();
    fSurfaceArea = fareas.back();
  }
  return fSurfaceArea;
}

// GetPointOnSurface
G4ThreeVector BelleCrystal::GetPointOnSurface() const
{
  if (fareas.size() == 0) getvolarea();
  double r = CLHEP::RandFlat::shoot(0., fareas.back());
  std::vector<double>::const_iterator it = std::lower_bound(fareas.begin(), fareas.end(), r);
  return (it != fareas.end()) ? GetPointOnTriangle(it - fareas.begin()) : GetPointOnSurface();
}

// Methods for visualisation
void BelleCrystal::DescribeYourselfTo(G4VGraphicsScene& scene) const
{
  scene.AddSolid(*this);
}

PolyhedronBelleCrystal::PolyhedronBelleCrystal(int n, const G4ThreeVector* pt)
{
  int nsides = n / 2;
  AllocateMemory(n, nsides + 2 * (nsides - 2));
  for (int i = 0; i < n; i++) pV[i + 1] = pt[i];

  int count = 1;
  for (int j = 0; j < nsides; j++)
    pF[count++] = G4Facet(1 + j, 0,  1 + j + nsides, 0, 1 + ((j + 1) % nsides) + nsides, 0, 1 + (j + 1) % nsides, 0);
  for (int j = 0; j < nsides - 2; j++) pF[count++] = G4Facet(1, 0,  2 + j, 0, 3 + j, 0, 0, 0);
  for (int j = 0; j < nsides - 2; j++) pF[count++] = G4Facet(1 + nsides, 0,  3 + j + nsides, 0, 2 + j + nsides, 0, 0, 0);
  SetReferences();
}

PolyhedronBelleCrystal::~PolyhedronBelleCrystal() {}

G4Polyhedron* BelleCrystal::CreatePolyhedron() const
{
  int np = 2 * nsides;
  vector<G4ThreeVector> pt(np);
  for (int i = 0; i < np; i++) pt[i] = vertex(i);
  return new PolyhedronBelleCrystal(np, pt.data());
}
