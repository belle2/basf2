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
    struct Plane_t {
      G4ThreeVector n;// Normal unit vector (a,b,c)
      double d;       // offset (d)
      // => n.x*x + n.y*y + n.z*z + d = 0
    };

    struct Point_t {
      double x, y;
    };

    /** a Belle crystal in Geant4 */
    class BelleCrystal : public G4CSGSolid {
    public:
      /** Constructor for "nominal" BelleCrystal */
      explicit BelleCrystal(const G4String& pName);
      /** Constructor */
      BelleCrystal(const G4String& pName, int, const G4ThreeVector*);

      /** Destructor */
      virtual ~BelleCrystal() ;

      Plane_t GetSidePlane(G4int n) const {return fPlanes[n];}

      // Methods for solid

      G4double GetCubicVolume(); /**< get the Cubic volume */
      G4double GetSurfaceArea(); /**< get the surface area */

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

      G4VSolid* Clone() const;

      std::ostream& StreamInfo(std::ostream& os) const;

      // Visualisation functions

      void          DescribeYourselfTo(G4VGraphicsScene& scene) const;
      G4Polyhedron* CreatePolyhedron() const;

      /** Two vectors define an axis-parallel bounding box for the shape */
      void BoundingLimits(G4ThreeVector& pMin, G4ThreeVector& pMax) const;

    public:  // without description

      /**
       * Fake default constructor for usage restricted to direct object
       * persistency for clients requiring preallocation of memory for
       * persistifiable objects.
       */
      explicit BelleCrystal(__void__&);

      /** copy constructor */
      BelleCrystal(const BelleCrystal& rhs);
      /** assignment operator */
      BelleCrystal& operator=(const BelleCrystal& rhs);

      G4ThreeVector vertex(unsigned int i) const;

    protected:  // with description

      G4bool MakePlane(const G4ThreeVector& p1,
                       const G4ThreeVector& p2,
                       const G4ThreeVector& p3,
                       const G4ThreeVector& p4,
                       Plane_t& plane) const;

    private:
      G4ThreeVector GetPointOnTriangle(int) const; /**< Returns a random point on the surface of one of the faces */
      double area(int, double&) const; /**< triangle area */
      double getvolarea() const;
      const unsigned int* ivertx(unsigned int i) const; /**< get the ith vertex */
    private:
      unsigned int nsides; /**< the number of sides */
      double fDz;
      std::vector<Plane_t> fPlanes;
      std::vector<Point_t> fx;

      mutable std::vector<double> fareas;
    };

    /** Belle crystal in polyhedron */
    class PolyhedronBelleCrystal: public G4Polyhedron {
    public:
      PolyhedronBelleCrystal(int, const G4ThreeVector*); /**< constructor */
      virtual ~PolyhedronBelleCrystal();                 /**< destructor */
    };

  }
}
