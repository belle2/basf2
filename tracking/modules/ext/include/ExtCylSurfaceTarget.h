/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Leo Piilonen                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EXTCYLSURFACETARGET_H
#define EXTCYLSURFACETARGET_H

#include <globals.hh>
#include <G4ErrorSurfaceTarget.hh>
#include <G4ThreeVector.hh>
#include <G4Plane3D.hh>
#include <G4String.hh>

namespace Belle2 {

  //!  Defines a _closed_ cylinder for the geant4e "target", the surface
  //!  that encloses the volume within which track extrapolation is done
  class ExtCylSurfaceTarget : public G4ErrorSurfaceTarget {

  public:

    //! Construct the _closed_ cylindrical surface
    //! No coordinate transformations allowed: cylinder is coaxial with z axis
    ExtCylSurfaceTarget(const G4double&, const G4double&, const G4double&);

    //! Destructor
    ~ExtCylSurfaceTarget();

    //! Return the intersection of the cylinder with the line defined in
    //! local (cylinder) coordinates by point and direc
    virtual G4ThreeVector IntersectLocal(const G4ThreeVector& point,
                                         const G4ThreeVector& direc) const;

    //! Get the distance from a point to the cylinder along direc
    virtual G4double GetDistanceFromPoint(const G4ThreeVector& point,
                                          const G4ThreeVector& direc) const;

    //! Get the shortest distance from a point to the cylinder
    virtual G4double GetDistanceFromPoint(const G4ThreeVector& point) const;

    //! Get the plane tangent to the cylinder at a given point
    virtual G4Plane3D GetTangentPlane(const G4ThreeVector& point) const;

    //! Dump the cylinder parameters
    virtual void Dump(const G4String& msg) const;

  private:

    //! Cylinder radius
    G4double m_radius;

    //! Cylinder minimum-z coordinate
    G4double m_zmin;

    //! Cylinder maximum-z coordinate
    G4double m_zmax;

    //! Tolerance for distance between a point and cylinder's curved surface
    G4double m_tolerance;

  };

} // end of namespace Belle2
#endif // EXTCYLSURFACETARGET_H
