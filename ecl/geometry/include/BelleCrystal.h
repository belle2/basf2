#ifndef BelleCrystal_HH
#define BelleCrystal_HH

#include "G4Types.hh"

#include "G4CSGSolid.hh"
#include "G4Trap.hh"
#include "G4Polyhedron.hh"
#include <vector>

namespace Belle2 {
  namespace ECL {
    struct Plane_t {
      G4ThreeVector n;// Normal unit vector (a,b,c)
      double d;       // offset (d)
      // => n.x*x + n.y*y + n.z*z + d = 0
    };

    struct Point_t {
      double x, y;
    };

    class BelleCrystal : public G4CSGSolid {
    public:  // with description
      explicit BelleCrystal(const G4String& pName);
      // Constructor for "nominal" BelleCrystal

      BelleCrystal(const G4String& pName, int, const G4ThreeVector*);

      virtual ~BelleCrystal() ;
      // Destructor

      Plane_t GetSidePlane(G4int n) const {return fPlanes[n];}

      // Methods for solid

      G4double GetCubicVolume();
      G4double GetSurfaceArea();

      void ComputeDimensions(G4VPVParameterisation* p,
                             const G4int n,
                             const G4VPhysicalVolume* pRep);

      G4bool CalculateExtent(const EAxis pAxis,
                             const G4VoxelLimits& pVoxelLimit,
                             const G4AffineTransform& pTransform,
                             G4double& pMin, G4double& pMax) const;

      EInside Inside(const G4ThreeVector& p) const;

      G4ThreeVector SurfaceNormal(const G4ThreeVector& p) const;

      G4double DistanceToIn(const G4ThreeVector& p, const G4ThreeVector& v) const;

      G4double DistanceToIn(const G4ThreeVector& p) const;

      G4double DistanceToOut(const G4ThreeVector& p, const G4ThreeVector& v,
                             const G4bool calcNorm = false,
                             G4bool* validNorm = 0, G4ThreeVector* n = 0) const;

      G4double DistanceToOut(const G4ThreeVector& p) const;

      G4GeometryType GetEntityType() const;

      G4ThreeVector GetPointOnSurface() const;

      G4VSolid* Clone() const;

      std::ostream& StreamInfo(std::ostream& os) const;

      // Visualisation functions

      void          DescribeYourselfTo(G4VGraphicsScene& scene) const;
      G4Polyhedron* CreatePolyhedron() const;

    public:  // without description

      explicit BelleCrystal(__void__&);
      // Fake default constructor for usage restricted to direct object
      // persistency for clients requiring preallocation of memory for
      // persistifiable objects.

      BelleCrystal(const BelleCrystal& rhs);
      BelleCrystal& operator=(const BelleCrystal& rhs);
      // Copy constructor and assignment operator.

      G4ThreeVector vertex(unsigned int i) const;
    protected:  // with description

      G4bool MakePlane(const G4ThreeVector& p1,
                       const G4ThreeVector& p2,
                       const G4ThreeVector& p3,
                       const G4ThreeVector& p4,
                       Plane_t& plane) const;

    private:
      G4ThreeVector GetPointOnTriangle(int) const;
      // Returns a random point on the surface of one of the faces
      double area(int, double&) const; // triangle area
      double getvolarea() const;
      const unsigned int* ivertx(unsigned int i) const;
    private:
      unsigned int nsides;
      double fDz;
      std::vector<Plane_t> fPlanes;
      std::vector<Point_t> fx;

      mutable std::vector<double> fareas;
    };

    class PolyhedronBelleCrystal: public G4Polyhedron {
    public:
      PolyhedronBelleCrystal(int, const G4ThreeVector*);
      virtual ~PolyhedronBelleCrystal();
    };

  }
}
#endif

