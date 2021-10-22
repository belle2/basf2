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
    /** struct for plane */
    struct Plane_t {
      G4ThreeVector n; /**< Normal unit vector (a,b,c) */
      double d;       /**< offset (d) */
      // => n.x*x + n.y*y + n.z*z + d = 0
    };

    /** struct for Point */
    struct Point_t {
      double x; /**< x coordinate */
      double y; /**< y coordinate */
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

      /** Get side plane */
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

      /** Return whether point inside/outside/on surface, using tolerance */
      EInside Inside(const G4ThreeVector& p) const;

      /** Calculate side nearest to p, and return normal */
      G4ThreeVector SurfaceNormal(const G4ThreeVector& p) const;

      /** Calculate exact shortest distance to any boundary from outside */
      G4double DistanceToIn(const G4ThreeVector& p, const G4ThreeVector& v) const;

      /** Calculate exact shortest distance to any boundary from outside */
      G4double DistanceToIn(const G4ThreeVector& p) const;

      /** Calculate exact shortest distance to any boundary from inside */
      G4double DistanceToOut(const G4ThreeVector& p, const G4ThreeVector& v,
                             const G4bool calcNorm = false,
                             G4bool* validNorm = 0, G4ThreeVector* n = 0) const;

      /** Calculate exact shortest distance to any boundary from inside */
      G4double DistanceToOut(const G4ThreeVector& p) const;

      /** Get entity type */
      G4GeometryType GetEntityType() const;

      /** get point on surface*/
      G4ThreeVector GetPointOnSurface() const;

      /** Make a clone of the object */
      G4VSolid* Clone() const;

      /** Stream object contents to an output stream */
      std::ostream& StreamInfo(std::ostream& os) const;

      /** Visualisation functions */
      void          DescribeYourselfTo(G4VGraphicsScene& scene) const;
      /** create polyhedron*/
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
      /** return ith vertex */
      G4ThreeVector vertex(unsigned int i) const;

    protected:  // with description

      /**
       * Calculate the coef's of the plane p1->p2->p3->p4->p1
       * where the ThreeVectors 1-4 are in anti-clockwise order when viewed from
       * in front of the plane (i.e. from normal direction).
       *
       * Return true if the ThreeVectors are coplanar + set coef;s
       *        false if ThreeVectors are not coplanar
       */
      G4bool MakePlane(const G4ThreeVector& p1,
                       const G4ThreeVector& p2,
                       const G4ThreeVector& p3,
                       const G4ThreeVector& p4,
                       Plane_t& plane) const;

    private:
      G4ThreeVector GetPointOnTriangle(int) const; /**< Returns a random point on the surface of one of the faces */
      double area(int, double&) const; /**< triangle area */
      double getvolarea() const; /**< get volume area */
      const unsigned int* ivertx(unsigned int i) const; /**< get the ith vertex */
    private:
      unsigned int nsides; /**< the number of sides */
      double fDz; /**< Dz */
      std::vector<Plane_t> fPlanes; /**< vector of planes */
      std::vector<Point_t> fx; /**< vector of points */

      mutable std::vector<double> fareas; /**< vector of area values */
    };

    /** Belle crystal in polyhedron */
    class PolyhedronBelleCrystal: public G4Polyhedron {
    public:
      PolyhedronBelleCrystal(int, const G4ThreeVector*); /**< constructor */
      virtual ~PolyhedronBelleCrystal();                 /**< destructor */
    };

  }
}
