/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016-2018 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Alexei Sibidanov                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include "G4Types.hh"

#include "G4CSGSolid.hh"
#include "G4Polyhedron.hh"
#include <vector>

namespace Belle2 {
  namespace ECL {

    struct zr_t {
      double z, r;
    };

    struct cachezr_t {
      double z, r;
      double dz, dr;
      double s2, is2, is;
      double zmin, zmax;
      double r2min, r2max;
      double ta;
      bool isconvex;
    };

    /** struct for a three vector */
    struct vector_t {double x, y, z;};

    /** struct for a triangle */
    struct triangle_t {int i0, i1, i2;};

    inline double dotxy(const vector_t& u, const vector_t& v)
    {
      return u.x * v.x + u.y * v.y;
    }

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

      G4double GetCubicVolume() {return fCubicVolume;}

      G4double GetSurfaceArea() {return fSurfaceArea;}

      G4VSolid* Clone() const;

      /** Two vectors define an axis-parallel bounding box for the shape */
      void BoundingLimits(G4ThreeVector& pMin, G4ThreeVector& pMax) const;

      std::ostream& StreamInfo(std::ostream& os) const;

      // Visualisation functions

      void          DescribeYourselfTo(G4VGraphicsScene& scene) const;
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
      bool insector(double, double) const;
      // double sectorside_intersection(double, const vector_t&, const vector_t&) const ;
      // double sectorside_intersection(double, const G4ThreeVector&, const G4ThreeVector&) const ;
      //  double sectorside(const G4ThreeVector&, const G4ThreeVector&, bool, bool) const;
      int    wn_poly(const zr_t&) const;
      double mindist(const zr_t&) const;
      std::vector<double> linecross(const G4ThreeVector&, const G4ThreeVector&) const;
      void eartrim() const;
      zr_t normal(const zr_t&, double&) const ;
      void getvolarea();
      void Init(const std::vector<zr_t>&, double, double);

    private:
      std::vector<zr_t> fcontour;
      std::vector<cachezr_t> fcache;
      std::vector<double> fz;
      std::vector<int> findx, fseg;
      mutable std::vector<double> farea;
      mutable std::vector<triangle_t> ftlist;

      double fphi, fdphi; // starting angle and finishing angle
      double fs0, fc0, fs1, fc1;
      double fn0x, fn0y, fn1x, fn1y;
      double frmin, frmax, fzmin, fzmax;
      bool fgtpi, ftwopi;

      G4VSolid* fshape;
      mutable std::vector<G4ThreeVector> fsurf;
    };

    /** Belle lathe polyhedron */
    class PolyhedronBelleLathe: public G4Polyhedron {
    public:
      PolyhedronBelleLathe(const std::vector<zr_t>&, const std::vector<triangle_t>&, double, double); /**< constructor */
      virtual ~PolyhedronBelleLathe(); /**< destructor */
    };
  }
}
#endif

