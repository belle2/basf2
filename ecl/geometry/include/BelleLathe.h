/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include "G4Types.hh"

#include "G4CSGSolid.hh"
#include "G4Polyhedron.hh"
#include <vector>

namespace Belle2 {
  namespace ECL {

    /** simple struct with z and r coordinates */
    struct zr_t {
      double z; /**< z coordinate */
      double r; /**< r coordinate */
    };

    /** cached z-r struct */
    struct cachezr_t {
      double z; /**< z coordinate */
      double r; /**< r coordinate */
      double dz; /**< difference in z */
      double dr; /**< difference in r */
      double s2; /**< squared distance */
      double is2; /**< inverted distance squared */
      double is; /**< distance */
      double zmin; /**< minimal z value */
      double zmax; /**< maximal z value */
      double r2min; /**< minimal r value squared */
      double r2max; /**< maximal r value squared */
      double ta; /**< ratio of dr over dz*/
      bool isconvex; /**< is shape convex? */
    };

    /** struct for a three vector */
    struct vector_t {
      double x; /**< x coordinate */
      double y; /**< y coordinate */
      double z; /**< z coordinate */
    };

    /** struct for a triangle */
    struct triangle_t {
      int i0; /**< first side */
      int i1; /**< second side */
      int i2; /**< third side */
    };

    inline double dotxy(const vector_t& u, const vector_t& v)
    {
      return u.x * v.x + u.y * v.y;
    }

    /** BelleLathe class */
    class BelleLathe : public G4CSGSolid {
    public:  // with description
      /**
       * Constructor for "nominal" BelleLathe whose parameters are to be set
       * by a G4VPVParamaterisation later
       */
      explicit BelleLathe(const G4String& pName);

      /** explicit constructor */
      BelleLathe(const G4String& pName, double phi0, double dphi, int n, double* z, double* rin, double* rout);
      /** explicit constructor */
      BelleLathe(const G4String& pName, double, double, const std::vector<zr_t>&);

      /** Destructor */
      virtual ~BelleLathe();

      // Methods for solid

      /** compute the dimensions */
      void ComputeDimensions(G4VPVParameterisation* p,
                             const G4int n,
                             const G4VPhysicalVolume* pRep);

      /** calculate the extent of the volume */
      G4bool CalculateExtent(const EAxis pAxis,
                             const G4VoxelLimits& pVoxelLimit,
                             const G4AffineTransform& pTransform,
                             G4double& pMin, G4double& pMax) const;

      /** Return whether point inside/outside/on surface, using tolerance */
      EInside Inside(const G4ThreeVector& p) const;

      /** Calculate side nearest to p, and return normal */
      G4ThreeVector SurfaceNormal(const G4ThreeVector& p) const;

      /** Calculate distance to shape from outside - return kInfinity if no intersection */
      G4double DistanceToIn(const G4ThreeVector& p, const G4ThreeVector& v) const;

      /** Calculate exact shortest distance to any boundary from outside */
      G4double DistanceToIn(const G4ThreeVector& p) const;

      /** Calculate distance to surface of shape from inside */
      G4double DistanceToOut(const G4ThreeVector& p, const G4ThreeVector& v,
                             const G4bool calcNorm = false,
                             G4bool* validNorm = 0, G4ThreeVector* n = 0) const;

      /** Calculate exact shortest distance to any boundary from inside */
      G4double DistanceToOut(const G4ThreeVector& p) const;

      /** Get entity type */
      G4GeometryType GetEntityType() const;

      /** Get point on surface */
      G4ThreeVector GetPointOnSurface() const;

      /** Get cubic volume */
      G4double GetCubicVolume() {return fCubicVolume;}

      /** Get surface area */
      G4double GetSurfaceArea() {return fSurfaceArea;}

      /** Make a clone of the object */
      G4VSolid* Clone() const;

      /** Two vectors define an axis-parallel bounding box for the shape */
      void BoundingLimits(G4ThreeVector& pMin, G4ThreeVector& pMax) const;

      /** Stream object contents to an output stream */
      std::ostream& StreamInfo(std::ostream& os) const;

      /** Visualisation function */
      void          DescribeYourselfTo(G4VGraphicsScene& scene) const;
      /** create polyhedron */
      G4Polyhedron* CreatePolyhedron() const;

    public:  // without description

      /**
       * Fake default constructor for usage restricted to direct object
       * persistency for clients requiring preallocation of memory for
       * persistifiable objects.
       */
      explicit BelleLathe(__void__&);

      /** copy constructor */
      BelleLathe(const BelleLathe& rhs);
      /** assignment operator */
      BelleLathe& operator=(const BelleLathe& rhs);

    protected:  // with description
      bool insector(double, double) const; /**< True if (x,y) is within the shape rotation */
      // double sectorside_intersection(double, const vector_t&, const vector_t&) const ;
      // double sectorside_intersection(double, const G4ThreeVector&, const G4ThreeVector&) const ;
      //  double sectorside(const G4ThreeVector&, const G4ThreeVector&, bool, bool) const;
      int    wn_poly(const zr_t&) const; /**< wn_poly */
      double mindist(const zr_t&) const; /**< minimal distance */
      /** calculate all ray solid's surface intersection return ordered vector */
      std::vector<double> linecross(const G4ThreeVector&, const G4ThreeVector&) const;
      void eartrim() const; /**< ear trim */
      zr_t normal(const zr_t&, double&) const ; /**< return normal */
      void getvolarea(); /**< get volume area */
      void Init(const std::vector<zr_t>&, double, double); /**< initialize */

    private:
      std::vector<zr_t> fcontour; /**< vector of zr structs */
      std::vector<cachezr_t> fcache; /**< vector of cached zr structs */
      std::vector<double> fz; /**< vector of z values */
      std::vector<int> findx; /**< vector of indices */
      std::vector<int> fseg; /**< vector of segments */
      mutable std::vector<double> farea; /**< vector of area values */
      mutable std::vector<triangle_t> ftlist; /**< vector of triangle structs */

      double fphi; /**< starting angle */
      double fdphi; /**< finishing angle */
      double fs0; /**< fs0 */
      double fc0; /**< fc0 */
      double fs1; /**< fs1 */
      double fc1; /**< fc1 */
      double fn0x; /**< fn0x */
      double fn0y; /**< fn0y */
      double fn1x; /**< fn1x */
      double fn1y; /**< fn1y */
      double frmin; /**< minimal r value */
      double frmax; /**< maximal r value */
      double fzmin; /**< minimal z value */
      double fzmax; /**< maximal z value */
      bool fgtpi; /**< greater than pi? */
      bool ftwopi; /**< bound within +- 2pi? */

      G4VSolid* fshape; /**< shape */
      mutable std::vector<G4ThreeVector> fsurf; /**< vector of surfaces */
    };

    /** Belle lathe polyhedron */
    class PolyhedronBelleLathe: public G4Polyhedron {
    public:
      PolyhedronBelleLathe(const std::vector<zr_t>&, const std::vector<triangle_t>&, double, double); /**< constructor */
      virtual ~PolyhedronBelleLathe(); /**< destructor */
    };
  }
}
