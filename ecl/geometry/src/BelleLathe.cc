#include "ecl/geometry/BelleLathe.h"

#include "globals.hh"

#include "G4VoxelLimits.hh"
#include "G4AffineTransform.hh"

#include "G4VPVParameterisation.hh"

#include "Randomize.hh"

#include "G4VGraphicsScene.hh"

#include "G4GenericPolycone.hh"

#include <map>

using namespace std;
using namespace Belle2;
using namespace ECL;

#define COMPARE 0
#define PERFCOUNTER 0
#if PERFCOUNTER==1
typedef int counter_t[6];
map<string, counter_t> counterl;
#define COUNTER(x) counterl[GetName()][x]++
//#define MATCHOUT(x) //if(GetName().find("sv_crystalcontainersolid")==0) cout<<x<<endl;
#else
#define COUNTER(x)
//
#endif

//#define MATCHOUT(x) G4cout<<GetName()<<" "<<x<<G4endl;
#define MATCHOUT(x)

struct Plane_t {
  G4ThreeVector n;// Normal unit vector (x,y,z)
  double d;       // offset (d)
  // => n.x*x + n.y*y + n.z*z + d = 0
};

namespace Belle2 {
  namespace ECL {
    inline double dotxy(const G4ThreeVector& p, const G4ThreeVector& n)
    {
      return p.x() * n.x() + p.y() * n.y();
    }
  }
}

ostream& operator <<(ostream& o, const zr_t& v)
{
  return o << "{" << v.z << ",  " << v.r << "}";
}

struct curl_t {
  G4ThreeVector v;
  explicit curl_t(const G4ThreeVector& _v): v(_v) {}
};

ostream& operator <<(ostream& o, const curl_t& c)
{
  return o << "{" << c.v.x() << ", " << c.v.y() << ", " << c.v.z() << "}, ";
}


BelleLathe::BelleLathe(const G4String& pName, double phi0, double dphi, const vector<zr_t>& c)
  : G4CSGSolid(pName)
{
  Init(c, phi0, dphi);
}

BelleLathe::BelleLathe(const G4String& pName, double phi0, double dphi, int n, double* z, double* rin, double* rout)
  : G4CSGSolid(pName)
{
  vector<zr_t> contour;
  for (int i = 0; i < n; i++) {
    zr_t t = {z[i], rin[i]};
    contour.push_back(t);
  }
  for (int i = n - 1; i >= 0; i--) {
    zr_t t = {z[i], rout[i]};
    contour.push_back(t);
  }

  Init(contour, phi0, dphi);
}

void BelleLathe::Init(const vector<zr_t>& c, double phi0, double dphi)
{
  vector<zr_t> contour = c;
  // remove duplicated vertices
  do {
    vector<zr_t>::iterator it0 = contour.begin(), it1 = it0 + 1;
    for (; it1 != contour.end();) {
      const zr_t& s0 = *it0, &s1 = *it1;
      if (abs(s0.z - s1.z) < kCarTolerance && abs(s0.r - s1.r) < kCarTolerance)
        it1 = contour.erase(it1);
      else {
        ++it0; ++it1;
      }
    }
    const zr_t& s0 = *it0, &s1 = contour[0];
    if (abs(s0.z - s1.z) < kCarTolerance && abs(s0.r - s1.r) < kCarTolerance) contour.erase(it0);
  } while (0);

  // remove vertices on the same line
  do {
    auto inc = [&contour](vector<zr_t>::iterator & it) -> void {
      if (++it >= contour.end()) it = contour.begin();
    };
    auto dec = [&contour](vector<zr_t>::iterator & it) -> void {
      if (--it < contour.begin()) it = (++contour.rbegin()).base();
    };
    vector<zr_t>::iterator it0 = contour.begin(), it1 = it0 + 1, it2 = it1 + 1;
    for (; it0 != contour.end();) {
      const zr_t& s0 = *it0, &s1 = *it1, &s2 = *it2;
      double dr2 = s2.r - s0.r, dz2 = s2.z - s0.z;
      double d = (s1.z - s0.z) * dr2 - (s1.r - s0.r) * dz2;
      if (d * d < kCarTolerance * kCarTolerance * (dr2 * dr2 + dz2 * dz2)) {
        it1 = contour.erase(it1); it2 = it1; inc(it2); it0 = it1; dec(it0);
      } else {
        ++it0; inc(it1); inc(it2);
      }
    }
  } while (0);

  auto isClockwise = [&contour]() -> bool {
    double sum = 0;
    zr_t p0 = contour[0];
    for (int i = 1, imax = contour.size(); i < imax; i++)
    {
      zr_t p1 = contour[i]; sum += (p1.z - p0.z) * (p1.r + p0.r);
      p0 = p1;
    }
    zr_t p1 = contour[0]; sum += (p1.z - p0.z) * (p1.r + p0.r);
    return sum > 0;
  };

  if (isClockwise()) {
    // std::ostringstream message;
    // message << "Polygon is not in anti-clockwise order: " << GetName() << "\nReversing order...";
    // G4Exception("BelleLathe::BelleLathe()", "BelleLathe", JustWarning, message);
    std::reverse(contour.begin(), contour.end());
  }
  fcontour = contour;

  auto convexside = [this](cachezr_t& s, double eps) -> void {
    s.isconvex = false;
    if (s.dz > 0) return;
    vector<zr_t>::const_iterator it = fcontour.begin();
    double a = s.dz * s.is, b = s.dr * s.is, cc = b * s.z - a * s.r;
    bool dp = false, dm = false;
    s.isconvex = true;
    do {
      const zr_t& p = *it;
      double d = a * p.r - b * p.z + cc; // distance to line
      dm = dm || (d < -eps);
      dp = dp || (d >  eps);
      if (dm && dp) {s.isconvex = false; return;}
    } while (++it != fcontour.end());
  };

  frmin =  kInfinity;
  frmax = -kInfinity;
  fzmin =  kInfinity;
  fzmax = -kInfinity;
  fcache.reserve(fcontour.size());
  for (int i = 0, n = fcontour.size(); i < n; i++) {
    const zr_t& s0 = fcontour[i], &s1 = fcontour[(i + 1) % n];
    cachezr_t t;
    t.z = s0.z;
    t.r = s0.r;
    t.dz = s1.z - s0.z;
    t.dr = s1.r - s0.r;
    t.s2 = t.dz * t.dz + t.dr * t.dr;
    t.is2 = 1 / t.s2;
    t.is = sqrt(t.is2);
    t.zmin = min(s0.z, s1.z);
    t.zmax = max(s0.z, s1.z);
    t.r2min = pow(min(s0.r, s1.r), 2);
    t.r2max = pow(max(s0.r, s1.r), 2);
    t.ta = (s1.r - s0.r) / (s1.z - s0.z);
    convexside(t, kCarTolerance);
    fcache.push_back(t);

    frmax = max(frmax, s0.r);
    frmin = min(frmin, s0.r);
    fzmax = max(fzmax, s0.z);
    fzmin = min(fzmin, s0.z);
  }

  fphi = phi0;
  fdphi = dphi;

  fdphi = std::min(2 * M_PI, fdphi);
  fdphi = std::max(0.0, fdphi);
  fc0 = cos(fphi);
  fs0 = sin(fphi);
  fc1 = cos(fphi + fdphi);
  fs1 = sin(fphi + fdphi);

  fn0x =  fs0;
  fn0y = -fc0;
  fn1x = -fs1;
  fn1y =  fc1;
  fgtpi = fdphi > M_PI;
  ftwopi = abs(fdphi - 2 * M_PI) < kCarTolerance;

  //  cout << ftwopi << " " << fgtpi << " " << fn0y << " " << fn0x << " " << fn1y << " " << fn1x << endl;

  for (int i = 0, n = fcontour.size(); i < n; i++) {
    const zr_t& s = fcontour[i];
    fz.push_back(s.z);
  }
  sort(fz.begin(), fz.end());
  fz.erase(std::unique(fz.begin(), fz.end()), fz.end());

  for (int i = 1, ni = fz.size(); i < ni; i++) {
    double a = fz[i - 1], b = fz[i];
    findx.push_back(fseg.size());
    for (int j = 0, nj = fcache.size(); j < nj; j++) {
      const cachezr_t& sj = fcache[j];
      double cc = sj.zmin, d = sj.zmax;
      if (cc != d and b > cc and d > a) { // overlap
        fseg.push_back(j);
      }
    }
  }
  findx.push_back(fseg.size());

  getvolarea();

#if COMPARE>0
  auto getpolycone = [](const G4String & pName, double phi0, double dphi, const vector<zr_t>& c) -> G4GenericPolycone* {
    vector<double> r, z;
    r.reserve(c.size());
    z.reserve(c.size());
    for (int i = 0, imax = c.size(); i < imax; i++)
    {
      r.push_back(c[i].r);
      z.push_back(c[i].z);
    }
    return new G4GenericPolycone(pName, phi0, dphi, c.size(), r.data(), z.data());
  };
  fshape = getpolycone(GetName(), phi0, dphi, fcontour);
#else
  fshape = NULL;
#endif
//  StreamInfo(G4cout);
}

// Nominal constructor for BelleLathe whose parameters are to be set by
// a G4VParamaterisation later.  Check and set half-widths as well as
// angles: final check of coplanarity
BelleLathe::BelleLathe(const G4String& pName)
  : G4CSGSolid(pName)
{
  vector<zr_t> a;
  Init(a, 0, 2 * M_PI);
}

// Fake default constructor - sets only member data and allocates memory
//                            for usage restricted to object persistency.
BelleLathe::BelleLathe(__void__& a)
  : G4CSGSolid(a)
{
  vector<zr_t> b;
  Init(b, 0, 2 * M_PI);
}

// Destructor
BelleLathe::~BelleLathe()
{
#if PERFCOUNTER==1
  cout << GetName() << " ";
  for (int i = 0; i < 6; i++) cout << counterl[GetName()][i] << " "; cout << endl;
#endif
}

// Copy constructor
BelleLathe::BelleLathe(const BelleLathe& rhs)
  : G4CSGSolid(rhs), fcontour(rhs.fcontour), fcache(rhs.fcache), fz(rhs.fz),
    findx(rhs.findx), fseg(rhs.fseg), farea(rhs.farea), ftlist(rhs.ftlist),
    fphi(rhs.fphi), fdphi(rhs.fdphi), fs0(rhs.fs0), fc0(rhs.fc0), fs1(rhs.fs1),
    fc1(rhs.fc1), fn0x(rhs.fn0x), fn0y(rhs.fn0y), fn1x(rhs.fn1x), fn1y(rhs.fn1y),
    frmin(rhs.frmin), frmax(rhs.frmax), fzmin(rhs.fzmin), fzmax(rhs.fzmax),
    fgtpi(rhs.fgtpi), ftwopi(rhs.ftwopi), fshape(rhs.fshape), fsurf(rhs.fsurf)
{
}

// Assignment operator
BelleLathe& BelleLathe::operator = (const BelleLathe& rhs)
{
  // Check assignment to self
  if (this == &rhs)  { return *this; }

  // Copy base class data
  G4CSGSolid::operator=(rhs);

  // Copy data
  fcontour = rhs.fcontour;
  fcache = rhs.fcache;
  fz = rhs.fz;
  findx = rhs.findx;
  fseg = rhs.fseg;
  farea = rhs.farea;
  ftlist = rhs.ftlist;
  fphi = rhs.fphi;
  fdphi = rhs.fdphi;
  fs0 = rhs.fs0;
  fc0 = rhs.fc0;
  fs1 = rhs.fs1;
  fc1 = rhs.fc1;
  fn0x = rhs.fn0x;
  fn0y = rhs.fn0y;
  fn1x = rhs.fn1x;
  fn1y = rhs.fn1y;
  frmin = rhs.frmin;
  frmax = rhs.frmax;
  fzmin = rhs.fzmin;
  fzmax = rhs.fzmax;
  fgtpi = rhs.fgtpi;
  ftwopi = rhs.ftwopi;
  fshape = rhs.fshape;
  fsurf = rhs.fsurf;
  return *this;
}


// Dispatch to parameterisation for replication mechanism dimension
// computation & modification.
void BelleLathe::ComputeDimensions(G4VPVParameterisation*,
                                   const G4int,
                                   const G4VPhysicalVolume*)
{
  G4Exception("BelleLathe::ComputeDimensions()",
              "GeomSolids0001", FatalException,
              "BelleLathe does not support Parameterisation.");
  // std::cout<<"ComputeDimensions"<<std::endl;
  // p->ComputeDimensions(*this,n,pRep);
}

vector<double> quadsolve(double a, double b, double c)
{
  // solve equation a*t^2 + b*t + c = 0 taking care intermediate rounding errors
  vector<double> t(2);
  b *= 0.5;
  double D = b * b - a * c;
  if (D >= 0) {
    double sD = sqrt(D);
    double sum = b + ((b > 0) ? sD : -sD);
    double t0 = -c / sum;
    double t1 = -sum / a;
    t[0] = t0;
    t[1] = t1;
  } else {
    t.clear();
  }

  return t;
}

inline int quadsolve(double a, double b, double c, double& t0, double& t1)
{
  // solve equation a*t^2 + b*t + c = 0 taking care intermediate rounding errors
  b *= 0.5;
  double D = b * b - a * c;
  if (D >= 0) {
    double sD = sqrt(D);
    double sum = b + ((b > 0) ? sD : -sD);
    t0 = -c / sum;
    t1 = -sum / a;
    return 2;
  }
  return 0;
}

struct solution_t {double t, s;};
vector<solution_t> extremum(double A, double B, double C, double D, double E, double F)
{
  // extremum of Fun(t,s) = A*t*t + B*t*s + C*s*s + D*t + E*s + F => dFun/ds = 0
  vector<solution_t> res;
  if (abs(B) < abs(A)) {
    double a =    4 * A * C - B * B;
    double b = 2 * (2 * A * E - B * D);
    double c =    4 * A * F - D * D;
    vector<double> ss = quadsolve(a, b, c);
    for (auto s : ss) {
      if (fpclassify(s) == FP_INFINITE) continue;
      double t = -(s * B + D) / (2 * A);
      solution_t r = {t, s};
      res.push_back(r);
    }
  } else {
    double B2 = B * B, CD = C * D, BE = B * E;
    double a =   A * (4 * A * C - B2);
    double b = 2 * A * (2 * CD - BE);
    double c =   D * (CD - BE) + B2 * F;
    vector<double> ts = quadsolve(a, b, c);
    for (auto t : ts) {
      if (fpclassify(t) == FP_INFINITE) continue;
      double s = -(2 * t * A + D) / B;
      solution_t r = {t, s};
      res.push_back(r);
    }
  }
  return res;
}

// calculate all ray solid's surface intersection return ordered vector
vector<double> BelleLathe::linecross(const G4ThreeVector& p, const G4ThreeVector& n) const
{
  auto hitside = [this, &p, &n](double t, double zmin, double zmax) -> bool {
    double z = p.z() + n.z() * t;
    bool k = zmin < z && z <= zmax;
    if (k && !ftwopi)
    {
      double x = p.x() + n.x() * t;
      double y = p.y() + n.y() * t;
      k = k && insector(x, y);
    }
    return k;
  };

  auto hitzside = [this, &p, &n](double t, double r2min, double r2max) -> bool {
    double x = p.x() + n.x() * t;
    double y = p.y() + n.y() * t;
    double r2 = x * x + y * y;
    bool k = r2min <= r2 && r2 < r2max;
    if (k && !ftwopi)
    {
      k = k && insector(x, y);
    }
    return k;
  };

  vector<double> tc;
  double inz = 1 / n.z();
  double nn = Belle2::ECL::dotxy(n, n), np = dotxy(n, p), pp = dotxy(p, p);
  for (const cachezr_t& s : fcache) { // loop over sides
    if (s.dz == 0.0) { // z-plane
      double t = (s.z - p.z()) * inz;
      if (hitzside(t, s.r2min, s.r2max)) { tc.push_back(t); }
    } else {
      double ta = s.ta;
      double A, B, R2;
      if (s.dr == 0.0) { // cylinder
        double R = s.r;
        R2 = R * R;

        A = -nn;
        B = np;
      } else { // cone
        double taz = ta * (p.z() - s.z);
        double R = taz + s.r;
        R2 = R * R;

        double nzta = n.z() * ta;
        A = nzta * nzta - nn;
        B = np - nzta * R;
      }
      double D = B * B + (pp - R2) * A;
      if (D > 0) {
        double sD = sqrt(D), iA = 1 / A;
        double t0 = (B + sD) * iA, t1 = (B - sD) * iA;
        if (hitside(t0, s.zmin, s.zmax)) tc.push_back(t0);
        if (hitside(t1, s.zmin, s.zmax)) tc.push_back(t1);
      }
    }
  }

  if (!ftwopi) {
    do { // side at phi0
      double d  = fn0x * p.x() + fn0y * p.y();
      double vn = fn0x * n.x() + fn0y * n.y();
      double t  = -d / vn;
      G4ThreeVector r = p + n * t;
      zr_t zr = {r.z(), fc0 * r.x() + fs0 * r.y()};
      if (vn != 0 && wn_poly(zr) == 2) tc.push_back(t);
    } while (0);

    do { // side at phi0+dphi
      double d  = fn1x * p.x() + fn1y * p.y();
      double vn = fn1x * n.x() + fn1y * n.y();
      double t  = -d / vn;
      G4ThreeVector r = p + n * t;
      zr_t zr = {r.z(), fc1 * r.x() + fs1 * r.y()};
      if (vn != 0 && wn_poly(zr) == 2) tc.push_back(t);
    } while (0);
  }

  sort(tc.begin(), tc.end());
  return tc;
}

// Calculate extent under transform and specified limit
G4bool BelleLathe::CalculateExtent(const EAxis A,
                                   const G4VoxelLimits& bb,
                                   const G4AffineTransform& T,
                                   G4double& pMin, G4double& pMax) const
{
  auto maxdist = [this](const G4ThreeVector & n) -> G4ThreeVector {
    G4ThreeVector r;
    int i = 0, nsize = fcache.size();
    if (ftwopi || insector(n.x(), n.y())) // n in sector
    {
      double nr = hypot(n.x(), n.y()), nz = n.z();
      double dmax = -kInfinity, R = 0, Z = 0;
      do {
        const cachezr_t& s = fcache[i];
        double d1 = nz * s.z + nr * s.r;
        if (dmax < d1) { R = s.r; Z = s.z; dmax = d1;}
      } while (++i < nsize);
      if (nr > 0) {
        R /= nr;
        r.set(R * n.x(), R * n.y(), Z);
      } else {
        double phi = fphi + 0.5 * fdphi;
        r.set(R * cos(phi), R * sin(phi), Z);
      }
    } else {
      double dmax = -kInfinity;
      do {
        const cachezr_t& s = fcache[i];
        // check both sides
        G4ThreeVector rf(-fn0y * s.r, fn0x * s.r, s.z), rl(fn1y * s.r, -fn1x * s.r, s.z);
        double d0 = rf * n, d1 = rl * n;
        //  cout<<rf<<" "<<rl<<endl;
        if (dmax < d0) { r = rf; dmax = d0;}
        if (dmax < d1) { r = rl; dmax = d1;}
      } while (++i < nsize);
    }
    return r;
  };

  struct seg_t {int i0, i1;};
  auto clip = [](vector<G4ThreeVector>& vlist, vector<seg_t>& slist, const G4ThreeVector & n, double dist) {
    vector<seg_t> snew;
    vector<int> lone;

    vector<double> d;
    for (const G4ThreeVector& v : vlist) d.push_back(v * n + dist);

    for (seg_t s : slist) {
      double prod = d[s.i0] * d[s.i1];
      //      cout<<d[s.i0]<<" "<<d[s.i1]<<endl;
      if (prod < 0) { // segment crosses plane - break it
        G4ThreeVector rn = (vlist[s.i0] * d[s.i1] - vlist[s.i1] * d[s.i0]) * (1 / (d[s.i1] - d[s.i0]));
        lone.push_back(vlist.size()); vlist.push_back(rn);
        if (d[s.i0] < 0) {
          s = {lone.back(), s.i1};
        } else {
          s = {s.i0, lone.back()};
        }
      } else if (prod == 0) { // segment end on plane
        if (d[s.i0] == 0 && d[s.i1] > 0) {
          lone.push_back(s.i0);
        } else if (d[s.i0] > 0 && d[s.i1] == 0) {
          lone.push_back(s.i1);
        } else continue;
      } else {
        if (d[s.i0] < 0) continue; // segment below plane
      }
      snew.push_back(s);
    }

    double dmax = -1e99;
    int imax = -1, jmax = -1;
    // search for the most distant points on the clipping plane
    for (unsigned int i = 0; i < lone.size(); i++) {
      for (unsigned int j = i + 1; j < lone.size(); j++) {
        double d2 = (vlist[lone[i]] - vlist[lone[j]]).mag2();
        if (d2 > dmax) { imax = lone[i]; jmax = lone[j];}
      }
    }

    // close the new polygon by creating new segments
    if (imax >= 0) {
      G4ThreeVector k = vlist[jmax] - vlist[imax];
      sort(lone.begin(), lone.end(), [&k, &vlist, &imax](int i, int j) {return k * (vlist[i] - vlist[imax]) < k * (vlist[j] - vlist[imax]);});

      for (unsigned int i = 0; i < lone.size(); i += 2) {
        seg_t t = {lone[i], lone[i + 1]};
        for (const seg_t& s : snew) {
          if (t.i1 == s.i0) { snew.push_back(t); break;}
          if (t.i0 == s.i0) { swap(t.i0, t.i1); snew.push_back(t); break;}
        }
      }
    }
    swap(slist, snew);
  };

  auto PhiCrossN = [this, clip](const vector<Plane_t>& planes) {
    // unordered clipped phi-sides vertices within
    // limiting planes
    vector<G4ThreeVector> vlist; // vertex list
    vector<seg_t> slist; // segment list
    vector<G4ThreeVector> res;

    int nsize = fcache.size();
    vlist.reserve(nsize);
    slist.reserve(nsize);
    for (int iphi = 0; iphi < 2; iphi++) {
      vlist.clear();
      slist.clear();
      // phi-side directional vector is (kx,ky)
      double kx = iphi ? -fn0y : fn1y, ky = iphi ? fn0x : -fn1x;
      do {
        int i = 0;
        do {
          const cachezr_t& s = fcache[i];
          G4ThreeVector r(kx * s.r, ky * s.r, s.z);
          vlist.push_back(r);
          seg_t t = {i, i + 1};
          slist.push_back(t);
        } while (++i < nsize - 1);
        const cachezr_t& s = fcache[nsize - 1];
        G4ThreeVector r(kx * s.r, ky * s.r, s.z);
        vlist.push_back(r);
        seg_t t = {nsize - 1, 0};
        slist.push_back(t);
      } while (0);

      // clip phi-side polygon by limiting planes
      for (const Plane_t& p : planes) {
        //  cout<<p.n<<" "<<p.d<<endl;
        clip(vlist, slist,  p.n, p.d);
        //  for(auto t:vlist) cout<<t<<" "; cout<<endl;
      }
      vector<bool> bv(vlist.size(), false);

      for (vector<seg_t>::const_iterator it = slist.begin(); it != slist.end(); ++it) {
        bv[(*it).i0] = true;
        bv[(*it).i1] = true;
      }

      for (unsigned int i = 0; i < vlist.size(); i++) {
        if (!bv[i]) continue;
        res.push_back(vlist[i]);
      }
    }
    return res;
  };

  auto RCross = [this](const G4ThreeVector & op, const G4ThreeVector & k, const G4ThreeVector & u) {
    // plane with origin at op and normal vector n = [k x u], k and u are orthogonal k*u = 0
    // plane equation r = t*k + s*u + op
    vector<solution_t> ts;
    int nsize = fcache.size();
    int i = 0;
    do {
      const cachezr_t& seg = fcache[i];
      // r0 -- cone radius at z0, c -- cone axis
      // cone equation is (r0 + tg * ((r-c0)*c))^2 = (r-c0)^2 - ((r-c0)*c)^2
      double r0 = seg.r, z0 = seg.z, tg = seg.ta, tg2 = tg * tg;
      double rtg = r0 * tg;

      G4ThreeVector o(op.x(), op.y(), op.z() - z0);

      double ko = k * o, uo = u * o, ck = k.z(), cu = u.z(), co = o.z();
      double k2 = 1, u2 = 1, o2 = o * o;
      double ck2 = ck * ck, cu2 = cu * cu, co2 = co * co;
      double dr2 = r0 * r0 - o2;
      if (seg.dz != 0.0) {
        double q0 = 1 + tg2;
        double q1 = co * q0 + rtg;

        double F00 = co2 * q0 + 2 * co * rtg + dr2;
        double F10 = 2 * (ck * q1 - ko);
        double F20 = ck2 * q0 - k2;
        double F01 = 2 * (cu * q1 - uo);
        double F11 = 2 * ck * cu * q0;
        double F02 = cu2 * q0 - u2;

        vector<solution_t> res = extremum(F02, F11, F20, F01, F10, F00);
        for (const solution_t& r : res) {
          double t = r.s, s = r.t;
          G4ThreeVector p = t * k + s * u + op;
          if (seg.zmin < p.z() && p.z() < seg.zmax) {
            solution_t e = {t, s};
            if (ftwopi || insector(p.x(), p.y()))
              ts.push_back(e);
          }
        }
      }
      double a = -(ck2 * u2 + cu2 * k2);
      if (a != 0) {
        if (abs(cu) > abs(ck)) {
          double b = 2 * (ck * (cu * uo - co * u2) - cu2 * ko);
          double c =  co * (2 * cu * uo - co * u2) + cu2 * dr2;
          vector<double> tv = quadsolve(a, b, c);
          for (double t : tv) {
            double s = -(co + ck * t) / cu;
            G4ThreeVector p = t * k + s * u + op;
            if (ftwopi || insector(p.x(), p.y())) {
              solution_t e = {t, s};
              ts.push_back(e);
            }
          }
        } else {
          double b = 2 * (cu * (ck * ko - co * k2) - ck2 * uo);
          double c =  co * (2 * ck * ko - co * k2) + ck2 * dr2;
          vector<double> sv = quadsolve(a, b, c);
          for (double s : sv) {
            double t = -(co + cu * s) / ck;
            G4ThreeVector p = t * k + s * u + op;
            if (ftwopi || insector(p.x(), p.y())) {
              solution_t e = {t, s};
              ts.push_back(e);
            }
          }
        }
      }
    } while (++i < nsize);
    return ts;
  };

  bool b1 = false, b2 = false;
  G4ThreeVector n0, n1, n2;
  switch (A) {
    case kXAxis: n0.set(1, 0, 0); n1.set(0, 1, 0); n2.set(0, 0, 1); b1 = bb.IsYLimited(); b2 = bb.IsZLimited(); break;
    case kYAxis: n0.set(0, 1, 0); n1.set(1, 0, 0); n2.set(0, 0, 1); b1 = bb.IsXLimited(); b2 = bb.IsZLimited(); break;
    case kZAxis: n0.set(0, 0, 1); n1.set(1, 0, 0); n2.set(0, 1, 0); b1 = bb.IsXLimited(); b2 = bb.IsYLimited(); break;
    default:                   break;
  }

  double dmin1 = -kInfinity, dmax1 = kInfinity;
  if (b1) {
    switch (A) {
      case kXAxis: dmin1 = bb.GetMinYExtent(); dmax1 = bb.GetMaxYExtent(); break;
      case kYAxis: dmin1 = bb.GetMinXExtent(); dmax1 = bb.GetMaxXExtent(); break;
      case kZAxis: dmin1 = bb.GetMinXExtent(); dmax1 = bb.GetMaxXExtent(); break;
      default:                   break;
    }
  }

  double dmin2 = -kInfinity, dmax2 = kInfinity;
  if (b2) {
    switch (A) {
      case kXAxis: dmin2 = bb.GetMinZExtent(); dmax2 = bb.GetMaxZExtent(); break;
      case kYAxis: dmin2 = bb.GetMinZExtent(); dmax2 = bb.GetMaxZExtent(); break;
      case kZAxis: dmin2 = bb.GetMinYExtent(); dmax2 = bb.GetMaxYExtent(); break;
      default:                   break;
    }
  }

  G4AffineTransform iT = T.Inverse();
  // axis to solid coordinates
  G4ThreeVector n0t = iT.TransformAxis(n0);
  G4ThreeVector smin = n0t * kInfinity, smax = (-kInfinity) * n0t; // extremum points in solid coordinate system
  double pmin = kInfinity, pmax = -pmin;
  if (b1 && b2) {
    G4ThreeVector corners[] = {n1* dmin1 + n2 * dmin2, n1* dmax1 + n2 * dmin2, n1* dmax1 + n2 * dmax2, n1* dmin1 + n2 * dmax2};
    for (G4ThreeVector& c : corners) iT.ApplyPointTransform(c); // to solid coordinates

    vector<Plane_t> planes;
    for (int i = 0; i < 4; i++) {
      const G4ThreeVector& c0 = corners[i], &c1 = corners[(i + 1) % 4];
      vector<double> dists = linecross(c0, n0t);
      //      cout<<"c0 "<<c0<<endl;
      for (double t : dists) {
        G4ThreeVector p = n0t * t + c0;
        double tt = t + c0 * n0t;
        //  cout<<p<<" "<<tt<<endl;
        if (pmax < tt) { pmax = tt; smax = p;}
        if (pmin > tt) { pmin = tt; smin = p;}
      }

      G4ThreeVector u = c1 - c0, un = u.unit();
      vector<solution_t> ts = RCross(c0, n0t, un);
      double umax = u.mag();
      for (solution_t r : ts) {
        if (0 < r.s && r.s < umax) {
          double tt = r.t + c0 * n0t;
          G4ThreeVector p = n0t * r.t + un * r.s + c0;
          //      cout<<r.t<<" "<<r.s<<" "<<smax<<endl;
          if (pmax < tt) { pmax = tt; smax = p;}
          if (pmin > tt) { pmin = tt; smin = p;}
        }
      }
      planes.push_back({ -un, un * c1});
    }

    vector<G4ThreeVector> vside = PhiCrossN(planes);
    for (G4ThreeVector& p : vside) {
      //      cout<<p<<endl;
      double tt = n0t * p;
      if (pmax < tt) { pmax = tt; smax = p;}
      if (pmin > tt) { pmin = tt; smin = p;}
    }

  } else if (b1 || b2) {
    G4ThreeVector limits[2], u;
    if (b1) {
      limits[0] = n1 * dmin1;
      limits[1] = n1 * dmax1;
      u = iT.TransformAxis(n2);
    } else {
      limits[0] = n2 * dmin2;
      limits[1] = n2 * dmax2;
      u = iT.TransformAxis(n1);
    }

    for (G4ThreeVector& c : limits) iT.ApplyPointTransform(c); // to solid coordinates
    for (int i = 0; i < 2; i++) {
      vector<solution_t> ts = RCross(limits[i], n0t, u);
      for (solution_t r : ts) {
        double tt = r.t + limits[i] * n0t;
        G4ThreeVector p = n0t * r.t + u * r.s + limits[i];
        //  cout<<r.t<<" "<<r.s<<" "<<endl;
        if (pmax < tt) { pmax = tt; smax = p;}
        if (pmin > tt) { pmin = tt; smin = p;}
      }
    }

    vector<Plane_t> planes(2);
    G4ThreeVector n;
    if (b1) {
      n = iT.TransformAxis(n1);
    } else {
      n = iT.TransformAxis(n2);
    }
    planes[0] = { n, -limits[0]* n};
    planes[1] = { -n, limits[1]* n};
    vector<G4ThreeVector> vside = PhiCrossN(planes);

    for (G4ThreeVector& p : vside) {
      //      double t = n0t*(p-limits[0]);
      double tt = n0t * p;
      //      cout<<tt<<" "<<p<<" "<<endl;
      if (pmax < tt) { pmax = tt; smax = p;}
      if (pmin > tt) { pmin = tt; smin = p;}
    }
  }
  // maximal distance in +- directions
  G4ThreeVector rp = maxdist(n0t), rm = maxdist(-n0t);
  if (bb.Inside(T.TransformPoint(rm))) {
    double tt = rm * n0t;
    if (pmin > tt) {pmin = tt; smin = rm;}
  }
  if (bb.Inside(T.TransformPoint(rp))) {
    double tt = rp * n0t;
    if (pmax < tt) {pmax = tt; smax = rp;}
  }

  // to mother volume coordinate system
  T.ApplyPointTransform(smin);
  T.ApplyPointTransform(smax);
  pmin = n0 * smin;
  pmax = n0 * smax;

  pmin -= kCarTolerance;
  pmax += kCarTolerance;
  //  bool hit = pmin > -kInfinity && pmax < kInfinity;
  bool hit = pmin < pmax;

#if COMPARE==10
  auto surfhit = [this, &bb, &T, &n0, &n0t](double & pmin, double & pmax, bool print = false)->bool {
    const int N = 1000 * 1000;
    if (fsurf.size() == 0) for (int i = 0; i < N; i++) fsurf.push_back(GetPointOnSurface());

    int umin = -1, umax = -1;
    double wmin = 1e99, wmax = -1e99;
    for (int i = 0; i < N; i++)
    {
      if (bb.Inside(T.TransformPoint(fsurf[i]))) {
        double w = n0t * fsurf[i];
        if (wmin > w) {wmin = w; umin = i;}
        if (wmax < w) {wmax = w; umax = i;}
      }
    }
    if (print)cout << umin << " " << umax << " " << wmin << " " << wmax << endl;
    if (umin >= 0 && umax >= 0)
    {
      G4ThreeVector qmin = fsurf[umin], qmax = fsurf[umax];
      T.ApplyPointTransform(qmin);
      T.ApplyPointTransform(qmax);
      pmin = n0 * qmin, pmax = n0 * qmax;
      return true;
    }
    return false;
  };

  bool res = fshape->CalculateExtent(A, bb, T, pMin, pMax);
  double srfmin = kInfinity, srfmax = -srfmin;
  bool shit = surfhit(srfmin, srfmax);
  double diff = kCarTolerance;
  diff = 10;
  //  if (abs(pmin - pMin) > diff || abs(pmax - pMax) > diff || hit != res) {
  if ((abs(pmin - srfmin) > diff || abs(pmax - srfmax) > diff) && shit) {
    cout << "===================================\n";
    cout << GetName() << " " << fcache.size() << " " << fphi << " " << fdphi << " " << ftwopi << "\n";
    cout << hit << " " << res << " " << b1 << " " << b2 << "\n";
    if (shit) {
      cout << "ss " << srfmin << " " << srfmax << "\n";
    } else {
      cout << "ss : not in bounding box" << "\n";
    }
    cout << "my " << pmin << " " << pmax << "\n";
    cout << "tc " << pMin << " " << pMax << "\n";
    cout << "df " << pmin - pMin << " " << pmax - pMax << "\n";
    G4ThreeVector bmin(bb.GetMinXExtent(), bb.GetMinYExtent(), bb.GetMinZExtent());
    G4ThreeVector bmax(bb.GetMaxXExtent(), bb.GetMaxYExtent(), bb.GetMaxZExtent());
    cout << "Axis=" << A << " " << bmin << " " << bmax << " " << T << "\n";
    cout << rp << " " << rm << "\n";
    cout << smin << " " << smax << "\n";
    cout << flush;
    //      _exit(0);
  }
  //    cout<<endl;
#endif
  pMin = pmin;
  pMax = pmax;

  return hit;
}

inline bool BelleLathe::insector(double x, double y) const
{
  double d0 = fn0x * x + fn0y * y;
  double d1 = fn1x * x + fn1y * y;
  bool b0 = d0 < 0, b1 = d1 < 0;
  return fgtpi ? b0 || b1 : b0 && b1;
}

int BelleLathe::wn_poly(const zr_t& r) const
{
  int wn = 0;
  vector<double>::const_iterator it = upper_bound(fz.begin(), fz.end(), r.z);
  //  cout<<r<<" "<<fz.size()<<" "<<it-fz.begin()<<endl;
  if (it != fz.begin() && it != fz.end()) {
    int k = it - fz.begin();
    for (int i = findx[k - 1]; i != findx[k]; i++) {
      const cachezr_t& s = fcache[fseg[i]];
      double dz = r.z - s.z, dr = r.r - s.r;
      double crs = s.dr * dz - s.dz * dr;
      wn -= (crs > 0) - (crs < 0);
    }
  }
  return wn;
}

double BelleLathe::mindist(const zr_t& r) const
{
  double d = kInfinity;
  int i = 0, n = fcache.size();
  do {
    const cachezr_t& s = fcache[i];
    double dz = r.z - s.z, dr = r.r - s.r;
    double dot = s.dz * dz + s.dr * dr; // projection of the point on the segement
    if (dot <   0) {
      d = min(d, dz * dz + dr * dr); // distance to the first point of the segement
    } else if (dot <= s.s2) { // point should be within the segment
      double crs = s.dr * dz - s.dz * dr;
      d = min(d, crs * crs * s.is2);
    }
  } while (++i < n);
  d = sqrt(d);
  d = (wn_poly(r) == 2) ? -d : d;
  return d;
}

// Return whether point inside/outside/on surface, using tolerance
EInside BelleLathe::Inside(const G4ThreeVector& p) const
{
  COUNTER(0);
  const double delta = 0.5 * kCarTolerance;
  EInside res = kInside;
  if (!ftwopi) {
    double d0 = fn0x * p.x() + fn0y * p.y();
    double d1 = fn1x * p.x() + fn1y * p.y();
    if (fgtpi) {
      if (d0 > delta && d1 > delta) { res = kOutside;}
      else if (d0 > -delta && d1 > -delta) { res = kSurface;}
    } else {
      if (d0 > delta || d1 > delta) { res = kOutside;}
      else if (d0 > -delta || d1 > -delta) { res = kSurface;}
    }
  }
  if (res != kOutside) {
    zr_t r = {p.z(), p.perp()};
    double d = mindist(r);
    if (res == kSurface && d < delta) res = kSurface;
    else if (d > delta) res = kOutside;
    else if (d > -delta) res = kSurface;
    else              res = kInside;
  }

#if COMPARE==1
  EInside dd = fshape->Inside(p);
  if (1 || dd != res) {
    double d0 = fn0x * p.x() + fn0y * p.y();
    double d1 = fn1x * p.x() + fn1y * p.y();
    //    if (abs(d0) > kCarTolerance && abs(d1) > kCarTolerance) {
    int oldprec = cout.precision(16);
    zr_t r = {p.z(), p.perp()};
    cout << GetName() << " Inside(p) " << p << " " << r << " my=" << res << " tc=" << dd <<
         " dist=" << mindist(r) << " " << d0 << " " << d1 << endl;
    cout.precision(oldprec);
    //    }
  }
#endif
  MATCHOUT("BelleLathe::Inside(p) " << p << " res= " << res);
  return res;
}

zr_t BelleLathe::normal(const zr_t& r, double& d2) const
{
  double d = std::numeric_limits<double>::infinity(), t = 0;
  int iseg = -1;
  for (int i = 0, imax = fcache.size(); i < imax; i++) {
    const cachezr_t& s = fcache[i];
    double dz = r.z - s.z, dr = r.r - s.r;
    double dot = s.dz * dz + s.dr * dr; // projection of the point on the segement
    if (dot <   0) {
      double dist = dz * dz + dr * dr; // distance to the first point of the segement
      if (dist < d) { d = dist; t = dot * s.is2; iseg = i;}
    } else if (dot <= s.s2) { // point should be within the segment
      double crs = s.dr * dz - s.dz * dr;
      double dist = crs * crs * s.is2;
      if (dist < d) { d = dist; t = dot * s.is2; iseg = i;}
    }
  }
  d2 = d;

  auto getn = [this](int i)->zr_t{
    int imax = fcache.size();
    int i0 = i;
    if (i == -1) i0 = imax;
    const cachezr_t& s = fcache[i0];
    double is = sqrt(s.is2);
    return {s.dr * is, -s.dz * is};
  };
  return getn(iseg);

  if (t < 0.0) {
    const cachezr_t& s = fcache[iseg];
    zr_t dist = {r.z - s.z, r.r - s.r};
    double dist2 = dist.z * dist.z + dist.r * dist.r;
    if (dist2 > 1e-18) {
      double q = 1 / sqrt(dist2);
      if (wn_poly(r) == 2) q = -q;
      return {dist.z * q, dist.r * q};
    } else {
      zr_t n = getn(iseg), np = getn(iseg - 1);
      n.z += np.z; n.r += np.r;
      double n2 = n.z * n.z + n.r * n.r;
      double q = 1 / sqrt(n2);
      n.z *= q, n.r *= q;
      return n;
    }
  }
  return getn(iseg);
}

// Calculate side nearest to p, and return normal
// If 2+ sides equidistant, first side's normal returned (arbitrarily)
G4ThreeVector BelleLathe::SurfaceNormal(const G4ThreeVector& p) const
{
  COUNTER(1);

  auto side = [this, &p](const zr_t & r, double d, int iside) {
    double nx = (iside) ? fn1x : fn0x, ny = (iside) ? fn1y : fn0y;
    if (wn_poly(r) == 2) return G4ThreeVector(nx, ny, 0);
    double cphi = (iside) ? fc1 : fc0, sphi = (iside) ? fs1 : fc0;

    double d2; zr_t n = normal(r, d2);
    double x = cphi * n.r, y = sphi * n.r;
    double u = sqrt(d2);
    d2 += d * d;
    G4ThreeVector res;
    if (d2 > 0) {
      double q = 1 / sqrt(d2);
      double cpsi = u * q, spsi = d * q;
      res.set(x * cpsi - y * spsi, x * spsi + y * cpsi, n.z);
    }
    res.set(x, y, n.z);
    return res;
  };

  G4ThreeVector res;
  zr_t r = {p.z(), p.perp()};
  double d2; zr_t n = normal(r, d2);
  double d = sqrt(d2);
  double pt = hypot(p.x(), p.y());

  if (pt > 0) {
    double ir = n.r / pt;
    res = G4ThreeVector(ir * p.x(), ir * p.y(), n.z);
  } else
    res = G4ThreeVector(n.r, 0, n.z);

  if (!ftwopi) {
    double d0 = fn0x * p.x() + fn0y * p.y();
    double d1 = fn1x * p.x() + fn1y * p.y();
    zr_t r0 = {p.z(), fc0 * p.x() + fs0 * p.y()}; // projection on plane phi
    zr_t r1 = {p.z(), fc1 * p.x() + fs1 * p.y()}; // projection on plane phi+dphi
    if (fgtpi) {
      if (d0 > 0 && d1 > 0) { // outside sector
        if (d0 < d1) {
          res = side(r0, d0, 0); goto exit;
        } else {
          res = side(r1, -d1, 1); goto exit;
        }
      } else {// inside sector
        if (wn_poly(r) == 2) { // point p inside the solid
          if (abs(d0) < d && abs(d0) < abs(d1)) { res = G4ThreeVector(fn0x, fn0y, 0); goto exit;}
          if (abs(d1) < d && abs(d1) < abs(d0)) { res = G4ThreeVector(fn1x, fn1y, 0); goto exit;}
        }
      }
    } else {
      if (d0 > 0 || d1 > 0) { // outside sector
        if (d0 < 0) {
          res = side(r1, -d1, 1); goto exit;
        } else {
          res = side(r0, d0, 0); goto exit;
        }
      } else {
        if (wn_poly(r) == 2) { // point p inside the solid
          if (abs(d0) < d && abs(d0) < abs(d1)) { res = G4ThreeVector(fn0x, fn0y, 0); goto exit;}
          if (abs(d1) < d && abs(d1) < abs(d0)) { res = G4ThreeVector(fn1x, fn1y, 0); goto exit;}
        }
      }
    }
  }
exit:
#if COMPARE==1
  G4ThreeVector dd = fshape->SurfaceNormal(p);
  if ((res - dd).mag() > 1e-11) {
    int oldprec = cout.precision(16);
    EInside inside = fshape->Inside(p);
    cout << GetName() << " SurfaceNormal(p) " << p << " " << res << " " << dd << " " << res - dd << " " << inside << endl;
    cout.precision(oldprec);
    //    _exit(0);
  }
#endif
  MATCHOUT("BelleLathe::SurfaceNormal(p,n) " << p << " res= " << res);
  return res;
}

// Calculate exact shortest distance to any boundary from outside
// This is the best fast estimation of the shortest distance to trap
// - Returns 0 is ThreeVector inside
G4double BelleLathe::DistanceToIn(const G4ThreeVector& p) const
{
  COUNTER(2);
  double d = 0;
  //  int sector = 0, plane = 0;
  if (ftwopi) {
    zr_t r = {p.z(), p.perp()};
    d = max(mindist(r), 0.0);
  } else {
    double d0 = fn0x * p.x() + fn0y * p.y();
    double d1 = fn1x * p.x() + fn1y * p.y();

    if (fgtpi) {
      if (d0 > 0 && d1 > 0) { // outside sector
        if (d0 < d1) {
          zr_t r = {p.z(), -fn0y * p.x() + fn0x * p.y()}; // projection on plane
          d = sqrt(pow(max(mindist(r), 0.0), 2) + d0 * d0);
        } else {
          zr_t r = {p.z(),  fn1y * p.x() - fn1x * p.y()}; // projection on plane
          d = sqrt(pow(max(mindist(r), 0.0), 2) + d1 * d1);
        }
      } else {
        zr_t r = {p.z(), p.perp()};
        d = max(mindist(r), 0.0);
      }
    } else {
      if (d0 > 0 || d1 > 0) { // outside sector
        if (d0 < 0) {
          zr_t r = {p.z(),  fn1y * p.x() - fn1x * p.y()}; // projection on plane
          d = sqrt(pow(max(mindist(r), 0.0), 2) + d1 * d1);
        } else {
          zr_t r = {p.z(), -fn0y * p.x() + fn0x * p.y()}; // projection on plane
          d = sqrt(pow(max(mindist(r), 0.0), 2) + d0 * d0);
        }
      } else {
        zr_t r = {p.z(), p.perp()};
        d = max(mindist(r), 0.0);
      }
    }
  }
#if COMPARE==1
  //  double dd = fshape->Inside(p) == 2 ? 0.0 : fshape->DistanceToIn(p);
  double dd = fshape->DistanceToIn(p);
  //  if (abs(d - dd) > kCarTolerance) {
  if (dd > d && abs(d - dd) > kCarTolerance) {
    int oldprec = cout.precision(16);
    EInside inside = fshape->Inside(p);
    zr_t r = {p.z(), p.perp()};
    cout << GetName() << " DistanceToIn(p) " << p << " " << r << " " << d << " " << dd << " " << d - dd << " " << inside << endl;
    cout.precision(oldprec);
    //    exit(0);
  }
#endif
  MATCHOUT("BelleLathe::DistanceToIn(p) " << p << " res= " << d);
  return d;
}

// Calculate exact shortest distance to any boundary from inside
// - Returns 0 is ThreeVector outside
G4double BelleLathe::DistanceToOut(const G4ThreeVector& p) const
{
  //  return ref->DistanceToOut(p);
  COUNTER(3);
  zr_t r = {p.z(), p.perp()};
  double d = mindist(r);
  if (!ftwopi) {
    double d0 = fn0x * p.x() + fn0y * p.y();
    double d1 = fn1x * p.x() + fn1y * p.y();
    if (fgtpi) {
      d = max(d, min(d0, d1));
    } else {
      d = max(d, max(d0, d1));
    }
  }
  d = max(-d, 0.0);
#if COMPARE==1
  double dd = fshape->Inside(p) == 0 ? 0.0 : fshape->DistanceToOut(p);
  if (abs(d - dd) > kCarTolerance) {
    int oldprec = cout.precision(16);
    zr_t r = {p.z(), p.perp()};
    //    cout<<r<<endl;
    cout << GetName() << " DistanceToOut(p) " << p << " " << r << " " << d << " " << dd << " " << d - dd << endl;
    cout.precision(oldprec);
  }
#endif
  MATCHOUT("BelleLathe::DistanceToOut(p) " << p.x() << " " << p.y() << " " << p.z() << " res= " << d);
  return d;
}

// Calculate distance to shape from outside - return kInfinity if no intersection
G4double BelleLathe::DistanceToIn(const G4ThreeVector& p, const G4ThreeVector& n) const
{
  //  return fshape->DistanceToIn(p, n);
  auto getnormal = [this, &p, &n](int i, double t) ->G4ThreeVector{
    const int imax = fcache.size();
    G4ThreeVector o;
    if (i < 0)
    {
    } else if (i < imax)
    {
      const cachezr_t& s = fcache[i];
      if (s.dz == 0.0) {
        o.setZ(copysign(1, s.dr));
      } else {
        double x = p.x() + n.x() * t;
        double y = p.y() + n.y() * t;
        double sth = s.dr * s.is, cth = -s.dz * s.is;
        double ir = cth / sqrt(x * x + y * y);
        o.set(x * ir, y * ir, sth);
      }
    } else if (i == imax)
    {
      o.setX(fn0x), o.setY(fn0y);
    } else {
      o.setX(fn1x), o.setY(fn1y);
    }
    return o;
  };

  auto hitside = [this, &p, &n](double t, const cachezr_t& s) -> bool {
    double z = p.z() + n.z() * t;
    //    cout<<t<<" "<<x<<" "<<y<<" "<<z<<endl;
    bool k = s.zmin < z && z <= s.zmax;
    if (k && !ftwopi)
    {
      double x = p.x() + n.x() * t;
      double y = p.y() + n.y() * t;
      k = k && insector(x, y);
    }
    return k;
  };

  auto hitzside = [this, &p, &n](double t, const cachezr_t& s) -> bool {
    double x = p.x() + n.x() * t;
    double y = p.y() + n.y() * t;
    double r2 = x * x + y * y;
    bool k = s.r2min <= r2 && r2 < s.r2max;
    if (k && !ftwopi)
    {
      k = k && insector(x, y);
    }
    return k;
  };

  auto hitphi0side = [this, &p, &n](double t) -> bool {
    double x = p.x() + n.x() * t;
    double y = p.y() + n.y() * t;
    double r = x * fc0 + y * fs0;
    if (r >= frmin)
    {
      double z = p.z() + n.z() * t;
      zr_t zr = {z, r};
      return wn_poly(zr) == 2;
    }
    return false;
  };

  auto hitphi1side = [this, &p, &n](double t) -> bool {
    double x = p.x() + n.x() * t;
    double y = p.y() + n.y() * t;
    double r = x * fc1 + y * fs1;
    if (r >= frmin)
    {
      double z = p.z() + n.z() * t;
      zr_t zr = {z, r};
      return wn_poly(zr) == 2;
    }
    return false;
  };

  double tmin = kInfinity;
  const int imax = fcache.size();
  int iseg = -1, isurface = -1;
  const double delta = 0.5 * kCarTolerance;
  double inz = 1 / n.z();
  double nn = dotxy(n, n), np = dotxy(n, p), pp = dotxy(p, p);
  double pz = p.z(), pr = sqrt(pp);
  for (int i = 0; i < imax; i++) { // loop over sides
    const cachezr_t& s = fcache[i];
    double dz = pz - s.z, dr = pr - s.r;
    double d = dz * s.dr - dr * s.dz;
    bool surface = false;
    if (abs(d * s.is) < delta) {
      double dot = dz * s.dz + dr * s.dr;
      if (dot >= 0 && dot <= s.s2) {
        surface = true;
        isurface = i;
      }
    }
    if (s.dz == 0.0) { // z-plane
      if (!surface) {
        double t = -dz * inz;
        if (0 < t && t < tmin && hitzside(t, s)) { tmin = t; iseg = i;}
      }
    } else {
      double A, B, R2;
      if (s.dr == 0.0) { // cylinder
        double R = s.r;
        R2 = R * R;

        A = -nn;
        B = np;
      } else { // cone
        double taz = s.ta * dz;
        double R = taz + s.r;
        R2 = R * R;

        double nzta = n.z() * s.ta;
        A = nzta * nzta - nn;
        B = np - nzta * R;
      }
      double C = pp - R2;
      double D = B * B + C * A;
      if (D > 0) {
        // double sD = sqrt(D), iA = 1 / A;
        // double t0 = (B + sD) * iA, t1 = (B - sD) * iA;
        double sD = sqrt(D), sum = B + copysign(sD, B);
        double t0 = -C / sum, t1 = sum / A;
        if (surface) { // exclude solution on surface
          if (abs(t0) > abs(t1)) {
            if (t0 > 0 && t0 < tmin && hitside(t0, s)) { tmin = t0; iseg = i;}
          } else {
            if (t1 > 0 && t1 < tmin && hitside(t1, s)) { tmin = t1; iseg = i;}
          }
        } else {
          if (t0 > 0 && t0 < tmin && hitside(t0, s)) { tmin = t0; iseg = i;}
          if (t1 > 0 && t1 < tmin && hitside(t1, s)) { tmin = t1; iseg = i;}
        }
      }
    }
  }

  if (!ftwopi) {
    do { // side at phi0
      double vn = fn0x * n.x() + fn0y * n.y();
      if (vn < 0) {
        double d = fn0x * p.x() + fn0y * p.y();
        double t = -d / vn;
        if (hitphi0side(t)) {
          bool surface = std::abs(d) < delta;
          if (surface) {
            tmin = 0; iseg = imax + 0;
          } else {
            if (0 < t && t < tmin) {tmin = t; iseg = imax + 0;}

          }
        }
      }
    } while (0);

    do { // side at phi0+dphi
      double vn = fn1x * n.x() + fn1y * n.y();
      if (vn < 0) {
        double d = fn1x * p.x() + fn1y * p.y();
        double t = -d / vn;
        if (hitphi1side(t)) {
          bool surface = std::abs(d) < delta;
          if (surface) {
            tmin = 0; iseg = imax + 1;
          } else {
            if (0 < t && t < tmin) { tmin = t; iseg = imax + 1;}
          }
        }
      }
    } while (0);
  }

  if (iseg >= 0) {
    if (getnormal(iseg, tmin)*n > 0) tmin = 0;
    //    if (getnormal(iseg, tmin)*n > 0) tmin = kInfinity; // mimic genericpolycone
  }

  auto convex = [this, imax](int i) -> bool{
    if (i < imax)
      return fcache[i].isconvex;
    else
      return !fgtpi;
  };

  if (tmin >= 0 && tmin < kInfinity) {
    if (isurface >= 0) if (convex(isurface) && getnormal(isurface, 0)*n >= 0) tmin = kInfinity;
  } else {
    if (Inside(p) == kSurface) {
      if (isurface >= 0) {
        tmin = (getnormal(isurface, 0) * n >= 0) ? kInfinity : 0; // mimic genericpolycone
      }
    }
  }

#if COMPARE==1
  //  double dd = fshape->Inside(p) == 2 ? 0.0 : fshape->DistanceToIn(p, n);
  double dd = fshape->DistanceToIn(p, n);
  if (abs(tmin - dd) > 1e-10) {
    int oldprec = cout.precision(16);
    EInside inside = fshape->Inside(p);
    cout << GetName() << " DistanceToIn(p,v) " << p << " " << n << " " << tmin << " " << dd << " " << tmin - dd << " " << inside << " "
         << Inside(p) << " iseg = " << iseg << " " << isurface << endl;
    if (isurface >= 0) cout << getnormal(isurface, 0) << endl;
    cout.precision(oldprec);
  }
#endif
  tmin = max(0.0, tmin);
  MATCHOUT("BelleLathe::DistanceToIn(p,n) " << p << " " << n << " res= " << tmin);
  return tmin;
}

// Calculate distance to surface of shape from inside
G4double BelleLathe::DistanceToOut(const G4ThreeVector& p, const G4ThreeVector& n,
                                   const G4bool calcNorm, G4bool* IsValid, G4ThreeVector* _n) const
{
  //  return fshape->DistanceToOut(p, n, calcNorm, IsValid, _n);
  auto getnormal = [this, &p, &n](int i, double t)->G4ThreeVector{
    const int imax = fcache.size();
    G4ThreeVector o;
    if (i < 0)
    {
    } else if (i < imax)
    {
      const cachezr_t& s = fcache[i];
      if (s.dz == 0.0) {
        o.setZ(copysign(1, s.dr));
      } else {
        double x = p.x() + n.x() * t;
        double y = p.y() + n.y() * t;
        double sth = s.dr * s.is, cth = -s.dz * s.is;
        double ir = cth / sqrt(x * x + y * y);
        o.set(x * ir, y * ir, sth);
      }
    } else if (i == imax)
    {
      o.setX(fn0x), o.setY(fn0y);
    } else {
      o.setX(fn1x), o.setY(fn1y);
    }
    return o;
  };

  double nn = dotxy(n, n), np = dotxy(n, p), pp = dotxy(p, p);
  auto hitside = [this, &p, &n, nn, np, pp](double t, const cachezr_t& s) -> bool {
    double z = p.z() + n.z() * t;
    //    cout<<t<<" "<<x<<" "<<y<<" "<<z<<endl;
    double dot = n.z() * s.dr * sqrt(pp + ((np + np) + nn * t) * t) - s.dz * (np + nn * t);
    bool k = s.zmin < z && z <= s.zmax && dot > 0;
    if (k && !ftwopi)
    {
      double x = p.x() + n.x() * t;
      double y = p.y() + n.y() * t;

      k = k && insector(x, y);
    }
    return k;
  };

  auto hitzside = [this, &p, &n](double t, const cachezr_t& s) -> bool {
    double x = p.x() + n.x() * t;
    double y = p.y() + n.y() * t;
    double r2 = x * x + y * y;
    bool k = s.dr * n.z() > 0 && s.r2min <= r2 && r2 < s.r2max;
    if (k && !ftwopi)
    {
      k = k && insector(x, y);
    }
    return k;
  };

  auto hitphi0side = [this, &p, &n](double t) -> bool {
    double x = p.x() + n.x() * t;
    double y = p.y() + n.y() * t;
    double r = x * fc0 + y * fs0;
    if (r >= frmin)
    {
      double z = p.z() + n.z() * t;
      zr_t zr = {z, r};
      return wn_poly(zr) == 2;
    }
    return false;
  };

  auto hitphi1side = [this, &p, &n](double t) -> bool {
    double x = p.x() + n.x() * t;
    double y = p.y() + n.y() * t;
    double r = x * fc1 + y * fs1;
    if (r >= frmin)
    {
      double z = p.z() + n.z() * t;
      zr_t zr = {z, r};
      return wn_poly(zr) == 2;
    }
    return false;
  };

  COUNTER(5);
  double tmin = kInfinity;

  const int imax = fcache.size();
  int iseg = -1, isurface = -1;

  const double delta = 0.5 * kCarTolerance;
  double inz = 1 / n.z();
  double pz = p.z(), pr = sqrt(pp);

  for (int i = 0; i < imax; i++) {
    const cachezr_t& s = fcache[i];

    double d = (pz - s.z) * s.dr - (pr - s.r) * s.dz;
    bool surface = abs(d * s.is) < delta;
    if (surface) isurface = i;
    if (s.dz == 0.0) {
      double t = (s.z - p.z()) * inz;
      if (surface) {
        if (hitzside(t, s)) {tmin = 0; iseg = i; break;}
      } else {
        if (0 < t && t < tmin && hitzside(t, s)) {tmin = t; iseg = i;}
      }
    } else {
      double A, B, R2;
      if (s.dr == 0.0) { // cylinder
        double R = s.r;
        R2 = R * R;

        A = -nn;
        B = np;
      } else { // cone
        double taz = s.ta * (p.z() - s.z);
        double R = taz + s.r;
        R2 = R * R;

        double nzta = n.z() * s.ta;
        A = nzta * nzta - nn;
        B = np - nzta * R;
      }
      double C = pp - R2;
      double D = B * B + C * A;
      if (D > 0) {
        double sD = sqrt(D);
        double sum = B + copysign(sD, B);
        double t0 = -C / sum, t1 = sum / A;
        //    cout<<t0<<" "<<t1<<" "<<endl;
        if (surface) { //exclude solution on surface
          if (abs(t0) < abs(t1)) {
            if (hitside(t0, s)) { tmin = 0; iseg = i; break;}
            if (0 < t1 && t1 < tmin && hitside(t1, s)) { tmin = t1; iseg = i;}
          } else {
            if (hitside(t1, s)) { tmin = 0; iseg = i; break;}
            if (0 < t0 && t0 < tmin && hitside(t0, s)) { tmin = t0; iseg = i;}
          }
        } else { // check both solutions
          if (0 < t0 && t0 < tmin && hitside(t0, s)) { tmin = t0; iseg = i;}
          if (0 < t1 && t1 < tmin && hitside(t1, s)) { tmin = t1; iseg = i;}
        }
      }
    }
    //      cout<<i<<" "<<iseg<<" "<<sqrt(d*d*s.is2)<<" "<<tmin<<" "<<s.zmin<<" "<<s.zmax<<endl;
  }

  if (!ftwopi) {
    do { // side at phi0
      double vn = fn0x * n.x() + fn0y * n.y();
      if (vn > 0) {
        double d = fn0x * p.x() + fn0y * p.y();
        double t = -d / vn;
        if (hitphi0side(t)) {
          bool surface = std::abs(d) < delta;
          if (surface) {
            tmin = 0; iseg = imax + 0;
          } else {
            if (0 < t && t < tmin) {tmin = t; iseg = imax + 0;}

          }
        }
      }
    } while (0);

    do { // side at phi0+dphi
      double vn = fn1x * n.x() + fn1y * n.y();
      if (vn > 0) {
        double d = fn1x * p.x() + fn1y * p.y();
        double t = -d / vn;
        if (hitphi1side(t)) {
          bool surface = std::abs(d) < delta;
          if (surface) {
            tmin = 0; iseg = imax + 1;
          } else {
            if (0 < t && t < tmin) { tmin = t; iseg = imax + 1;}
          }
        }
      }
    } while (0);
  }

  auto convex = [this, imax](int i) -> bool{
    if (i < imax)
      return fcache[i].isconvex;
    else
      return !fgtpi;
  };

  if (calcNorm) {
    if (tmin >= 0 && tmin < kInfinity) {
      *_n = getnormal(iseg, tmin);
      *IsValid = convex(iseg);
    } else {
      if (Inside(p) == kSurface) {
        if (isurface >= 0) {
          *_n = getnormal(isurface, tmin);
          *IsValid = convex(isurface);
          tmin = 0;
        }
      } else {
        *IsValid = false;
      }
    }
  }
  //  cout<<"tmin "<<tmin<<endl;
#if COMPARE==1
  do {
    // G4ThreeVector p0(1210.555046, -292.9578965, -36.71671492);
    // if ((p - p0).mag() > 1e-2)continue;
    bool isvalid;
    G4ThreeVector norm;
    double dd = fshape->DistanceToOut(p, n, calcNorm, &isvalid, &norm);
    if (abs(tmin - dd) > 1e-10 || (calcNorm && *IsValid != isvalid)) {
      int oldprec = cout.precision(16);
      cout << GetName() << " DistanceToOut(p,v) p,n =" << curl_t(p) << curl_t(n) << " calcNorm=" << calcNorm
           << " myInside=" << Inside(p) << " tmin=" << tmin << " dd=" << dd << " d=" << tmin - dd << " iseg=" << iseg << " isurf=" << isurface
           << " ";
      if (calcNorm) cout << "myIsValid = " << *IsValid << " tIsValid=" << isvalid << " myn=" << (*_n) << " tn=" << (norm);
      cout << endl;
      cout.precision(oldprec);
      //      _exit(0);
    }
  } while (0);
#endif
  MATCHOUT("BelleLathe::DistanceToOut(p,n) " << p << " " << n << " res= " << tmin);
  return tmin;
}

void BelleLathe::eartrim() const
{
  ftlist.clear();
  unsigned int n = fcontour.size();
  vector<int> indx;
  for (unsigned int i = 0; i < n; i++) indx.push_back(i);
  int count = 0;
  while (indx.size() > 3 && ++count < 200) {
    unsigned int ni = indx.size();
    for (unsigned int i = 0; i < ni; i++) {
      int i0 = indx[i], i1 = indx[(i + 1) % ni], i2 = indx[(i + 2) % ni];
      double nx = fcontour[i2].z - fcontour[i0].z;
      double ny = fcontour[i2].r - fcontour[i0].r;
      double d1 = nx * (fcontour[i1].r - fcontour[i0].r) - ny * (fcontour[i1].z - fcontour[i0].z);
      bool ear = true;
      for (unsigned int j = 0; j < ni - 3; j++) {
        int k = indx[(i + 3 + j) % ni];
        double d = nx * (fcontour[k].r - fcontour[i0].r) - ny * (fcontour[k].z - fcontour[i0].z);
        if (d * d1 > 0) {
          ear = false;
          break;
        }
      }
      if (ear) {
        triangle_t t = {i0, i1, i2};
        ftlist.push_back(t);
        indx.erase(indx.begin() + (i + 1) % ni);
        break;
      }
    }
  }
  if (indx.size() == 3) {
    triangle_t t = {indx[0], indx[1], indx[2]};
    ftlist.push_back(t);
  }
}

void BelleLathe::getvolarea()
{
  double vol = 0;
  for (const cachezr_t& s : fcache) vol += s.dz * ((3 * s.r) * (s.r + s.dr) + s.dr * s.dr);
  fCubicVolume = -fdphi * vol / 6;

  double totalarea = 0;
  if (!ftwopi) {
    eartrim();
    for (const triangle_t& t : ftlist) {
      const zr_t& p0 = fcontour[t.i0], &p1 = fcontour[t.i1], &p2 = fcontour[t.i2];
      double area = (p1.z - p0.z) * (p2.r - p0.r) - (p1.r - p0.r) * (p2.z - p0.z);
      totalarea += abs(area);
      farea.push_back(totalarea);
    }
  }

  for (const cachezr_t& s : fcache) {
    double area = fdphi * (s.r + 0.5 * s.dr) * sqrt(s.dz * s.dz + s.dr * s.dr);
    totalarea += area;
    farea.push_back(totalarea);
  }
  fSurfaceArea = farea.back();
}

G4ThreeVector BelleLathe::GetPointOnSurface() const
{
  auto GetPointOnTriangle = [this](const triangle_t& t)-> G4ThreeVector{
    // barycentric coordinates
    double a1 = CLHEP::RandFlat::shoot(0., 1.), a2 = CLHEP::RandFlat::shoot(0., 1.);
    if (a1 + a2 > 1) { a1 = 1 - a1; a2 = 1 - a2;}
    double a0 = 1 - (a1 + a2);
    const zr_t& p0 = fcontour[t.i0], &p1 = fcontour[t.i1], &p2 = fcontour[t.i2];
    zr_t p = {p0.z* a0 + p1.z* a1 + p2.z * a2, p0.r* a0 + p1.r* a1 + p2.r * a2};
    double c, s;
    if (CLHEP::RandFlat::shoot(0., 1.) > 0.5) // select phi side
    {
      c = -fn0y; s = fn0x;
    } else {
      c = fn1y; s = -fn1x;
    }
    G4ThreeVector r1(p.r * c, p.r * s, p.z);
    return r1;
  };

  double rnd = CLHEP::RandFlat::shoot(0., farea.back());
  std::vector<double>::const_iterator it = std::lower_bound(farea.begin(), farea.end(), rnd);
  unsigned int i = it - farea.begin();

  if (!ftwopi) {
    if (i < ftlist.size()) {
      return GetPointOnTriangle(ftlist[i]);
    } else {
      i -= ftlist.size();
    }
  }

  const cachezr_t& s = fcache[i];
  double I = 2 * s.r + s.dr;
  double Iw = CLHEP::RandFlat::shoot(0., I);
  double q = sqrt(Iw * s.dr + s.r * s.r);
  double t = Iw / (q + s.r);
  double z = s.z + s.dz * t;
  double r = s.r + s.dr * t;
  double phi = CLHEP::RandFlat::shoot(fphi, fphi + fdphi);
  double x = r * cos(phi), y = r * sin(phi);
  return G4ThreeVector(x, y, z);
}

// GetEntityType
G4GeometryType BelleLathe::GetEntityType() const
{
  return G4String("BelleLathe");
}

// Make a clone of the object
G4VSolid* BelleLathe::Clone() const
{
  return new BelleLathe(*this);
}

// Stream object contents to an output stream
std::ostream& BelleLathe::StreamInfo(std::ostream& os) const
{
  G4int oldprc = os.precision(16);
  os << "-----------------------------------------------------------\n"
     << "    *** Dump for solid - " << GetName() << " ***\n"
     << "    ===================================================\n"
     << " Solid type: BelleLathe\n"
     << "    Contour: " << fcontour.size() << " sides, {z, r} points \n";
  for (int i = 0, imax = fcontour.size(); i < imax; i++) {
    os << fcontour[i] << ", ";
  }
  os << "\n";
  for (int i = 0, imax = fcontour.size(); i < imax; i++) {
    os << fcache[i].isconvex << ", ";
  }
  os << "\n";
  os << "phi0 = " << fphi << ", dphi = " << fdphi << ", Full Circle = " << (ftwopi ? "yes" : "no") << "\n";
  double xmin = fzmin - 0.05 * (fzmax - fzmin), xmax = fzmax + 0.05 * (fzmax - fzmin);
  double ymin = frmin - 0.05 * (frmax - frmin), ymax = frmax + 0.05 * (frmax - frmin);
  os << " BB: " << xmin << ", " << xmax << ", " << ymin << ", " << ymax << endl;
  os << "-----------------------------------------------------------\n";
  os.precision(oldprc);

  return os;
}

// Methods for visualisation
void BelleLathe::DescribeYourselfTo(G4VGraphicsScene& scene) const
{
  scene.AddSolid(*this);
}


void takePolyhedron(const HepPolyhedron& p)
{
  int i, nnode, iNodes[5], iVis[4], iFaces[4];

  for (int iface = 1; iface <= p.GetNoFacets(); iface++) {
    p.GetFacet(iface, nnode, iNodes, iVis, iFaces);
    for (i = 0; i < nnode; i++) {
      if (iNodes[i] < 1 || iNodes[i] > p.GetNoVertices()) { //G.Barrand
        //        processor_error = 1;
        G4cerr
            << "BooleanProcessor::takePolyhedron : problem 1."
            << G4endl;
      }
      if (iFaces[i] < 1 || iFaces[i] > p.GetNoFacets()) { //G.Barrand
        //        processor_error = 1;
        G4cerr
            << "BooleanProcessor::takePolyhedron : problem 2. "
            << i << " " << iFaces[i] << " " << p.GetNoFacets() << G4endl;
      }
    }
  }
}

PolyhedronBelleLathe::PolyhedronBelleLathe(const std::vector<zr_t>& v, const std::vector<triangle_t>& t, double phi, double dphi)
{
  int nphi = GetNumberOfRotationSteps();
  bool twopi = abs(dphi - 2 * M_PI) < 1e-6;
  int n = v.size();
  if (twopi) {
    int nv = n * nphi;
    int nf = nv;
    AllocateMemory(nv, nf);

    auto vnum = [nphi, n](int iphi, int ip) {
      return (iphi % nphi) * n + (ip % n) + 1;
    };

    int fcount = 1;
    double dfi = dphi / nphi;
    for (int i = 0; i < nphi; i++) {
      double fi = phi + i * dfi;
      double cf = cos(fi), sf = sin(fi);
      for (int j = 0; j < n; j++) pV[vnum(i, j)].set(v[j].r * cf, v[j].r * sf, v[j].z);
      for (int j = 0; j < n; j++) pF[fcount++ ] = G4Facet(vnum(i, j), 0,  vnum(i, j + 1), 0, vnum(i + 1, j + 1), 0, vnum(i + 1, j), 0);
    }
  } else {
    //    cout<<"NPHI = "<<nphi<<" "<<phi<<" "<<dphi<<endl;
    nphi = int(nphi * (dphi / (2 * M_PI)) + 0.5);
    nphi = nphi > 3 ? nphi : 3;

    //    cout<<"NPHI = "<<nphi<<endl;

    int nv = n * nphi;
    int nf = n * (nphi - 1) + 2 * t.size();
    AllocateMemory(nv, nf);

    auto vnum = [nphi, n](int iphi, int ip) {
      return iphi * n + (ip % n) + 1;
    };

    int fcount = 1;
    double dfi = dphi / (nphi - 1);
    for (int i = 0; i < nphi; i++) {
      double fi = phi + i * dfi;
      double cf = cos(fi), sf = sin(fi);
      for (int j = 0; j < n; j++) pV[vnum(i, j)].set(v[j].r * cf, v[j].r * sf, v[j].z);
      if (i == nphi - 1) break;
      for (int j = 0; j < n; j++) pF[fcount++] = G4Facet(vnum(i, j), 0,  vnum(i, j + 1), 0, vnum(i + 1, j + 1), 0, vnum(i + 1, j), 0);
    }

    for (const triangle_t& k : t) pF[fcount++] = G4Facet(vnum(0, k.i0), 0,  vnum(0, k.i2), 0, vnum(0, k.i1), 0, 0, 0);
    int i = nphi - 1;
    for (const triangle_t& k : t) pF[fcount++] = G4Facet(vnum(i, k.i0), 0,  vnum(i, k.i1), 0, vnum(i, k.i2), 0, 0, 0);

  }
  SetReferences();
  //  takePolyhedron(*this);
}

PolyhedronBelleLathe::~PolyhedronBelleLathe() {}

G4Polyhedron* BelleLathe::CreatePolyhedron() const
{
  eartrim();
  return new PolyhedronBelleLathe(fcontour, ftlist, fphi, fdphi);
}

#if 0
#include <immintrin.h>
double mindistsimd(const zr_t& r, const vector<cachezr_t>& contour)
{
  double d = kInfinity;
  int i = 0, n = contour.size();
  __m128d zero = _mm_set_sd(0);
  __m128d one = _mm_set_sd(1);
  double wn = 0;
  do {
    const cachezr_t& s = contour[i];
    double dz = r.z - s.z, dr = r.r - s.r;
    double crs = s.dr * dz - s.dz * dr;
    double dot = s.dz * dz + s.dr * dr; // projection of the point on the segement
    //    if(s.zmin<=r.z&&r.z<s.zmax) wn -= (crs>0) - (crs<0);
    __m128d crssd =  _mm_set_sd(crs);
    __m128d maskgt = _mm_cmpgt_sd(crssd, zero);
    __m128d masklt = _mm_cmplt_sd(crssd, zero);
    __m128d left   = _mm_sub_sd(_mm_and_pd(maskgt, one), _mm_and_pd(masklt, one));
    __m128d z = _mm_set_sd(s.z);
    __m128d mask = _mm_and_pd(_mm_cmple_sd(_mm_set_sd(s.zmin), z), _mm_cmplt_sd(z, _mm_set_sd(s.zmax)));
    left  = _mm_and_pd(mask, left);
    double du = dz * dz + dr * dr;
    double dv = crs * crs * s.is2;

    masklt = _mm_cmplt_sd(_mm_set_sd(dot), zero);
    maskgt = _mm_cmpgt_sd(_mm_set_sd(dot), _mm_set_sd(s.s2));

    __m128d uu = _mm_or_pd(_mm_and_pd(maskgt, _mm_set_sd(kInfinity)), _mm_andnot_pd(maskgt, _mm_set_sd(dv)));
    __m128d vv = _mm_or_pd(_mm_and_pd(masklt, _mm_set_sd(min(d, du))), _mm_andnot_pd(masklt, _mm_set_sd(min(d, uu[0]))));
    wn -= left[0];
    d = vv[0];
  } while (++i < n);
  d = sqrt(d);
  d = (wn == 2) ? -d : d;
  //  cout<<wn<<" "<<d<<endl;
  //  cout<<sqrt(dp)<<" "<<sqrt(dm)<<endl;
  return d;
}
inline int left(const zr_t& r0, const zr_t& r1, const zr_t& r)
{
  double d = (r1.z - r0.z) * (r.r - r0.r) - (r.z - r0.z) * (r1.r - r0.r);
  return (d > 0) - (d < 0);
}

inline int checkside(const zr_t& s0, const zr_t& s1, const zr_t& r)
{
  double zmin = min(s0.z, s1.z), zmax = max(s0.z, s1.z);
  if (zmin <= r.z && r.z < zmax) return left(s0, s1, r);
  return 0;
}

int wn_poly(const zr_t& r, const vector<zr_t>& contour)
{
  int wn = 0; // the winding number counter
  int i = 0, n = contour.size() - 1;
  do {
    wn += checkside(contour[i], contour[i + 1], r);
  } while (++i < n);
  wn += checkside(contour[n], contour[0], r);
  return wn;
}

double mindist(const zr_t& r, const vector<zr_t>& contour)
{
  int wn = 0;
  double d = kInfinity;
  auto dist = [&contour, &d, &r, &wn](int i0, int i1)->void {
    const zr_t& s0 = contour[i0], &s1 = contour[i1];
    double zmin = min(s0.z, s1.z), zmax = max(s0.z, s1.z);
    double sz = s1.z - s0.z, sr = s1.r - s0.r;
    double dz = r.z - s0.z, dr = r.r - s0.r;
    double crs = dz * sr - sz * dr;
    if (zmin <= r.z && r.z < zmax) wn -= (crs > 0) - (crs < 0);
    double dot = sz * dz + sr * dr; // projection of the point on the segement
    double s2 = sz * sz + sr * sr;
    if (dot > s2) return; // point should be within the segment
    if (dot < 0)
    {
      double d2 = dz * dz + dr * dr; // distance to the first point of the segement
      d = min(d, d2);
    } else{
      d = min(d, crs* crs / s2);
    }
    //    cout<<i0<<" "<<s0.z<<" "<<s0.r<<" "<<d<<" "<<wn<<endl;
  };
  int i = 0, n = contour.size() - 1;
  do {dist(i, i + 1);} while (++i < n);
  dist(n, 0);
  d = sqrt(d);
  d = (wn == 2) ? -d : d;
  //  cout<<wn<<" "<<d<<endl;
  //  cout<<sqrt(dp)<<" "<<sqrt(dm)<<endl;
  return d;
}

double mindist(const zr_t& r, const vector<cachezr_t>& contour)
{
  double d = kInfinity;
  int wn = 0, i = 0, n = contour.size();
  do {
    const cachezr_t& s = contour[i];
    double dz = r.z - s.z, dr = r.r - s.r;
    double crs = s.dr * dz - s.dz * dr;
    double dot = s.dz * dz + s.dr * dr; // projection of the point on the segement
    if (s.zmin <= r.z && r.z < s.zmax) wn -= (crs > 0) - (crs < 0);
    if (dot > s.s2) continue; // point should be within the segment
    if (dot <   0) {
      d = min(d, dz * dz + dr * dr); // distance to the first point of the segement
    } else {
      d = min(d, crs * crs * s.is2);
    }
    //    cout<<i<<" "<<s.z<<" "<<s.r<<" "<<d<<" "<<wn<<endl;
    //    cout<<i<<" "<<d<<" "<<wn<<endl;
  } while (++i < n);
  d = sqrt(d);
  d = (wn == 2) ? -d : d;
  //  cout<<wn<<" "<<d<<endl;
  return d;
}

#endif
