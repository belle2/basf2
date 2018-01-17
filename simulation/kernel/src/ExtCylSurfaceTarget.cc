/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <simulation/kernel/ExtCylSurfaceTarget.h>
#include <G4GeometryTolerance.hh>
#include <geomdefs.hh>
#include <G4Plane3D.hh>
#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::Simulation;

ExtCylSurfaceTarget::ExtCylSurfaceTarget(const G4double& radius,
                                         const G4double& zmin,
                                         const G4double& zmax) :
  m_radius(radius),
  m_zmin(zmin),
  m_zmax(zmax)
{
  theType = G4ErrorTarget_CylindricalSurface;
  m_tolerance = 1000.0 * G4GeometryTolerance::GetInstance()->GetSurfaceTolerance();
  B2DEBUG(1, "Simulation::ExtCylSurfaceTarget created with radius "
          << m_radius << " zmin " << zmin << " zmax " << zmax);
  // Force a reference to unused functions to avoid cppcheck remarks. (always skipped)
  if (m_tolerance < 0.0) {
    GetTangentPlane(G4ThreeVector(m_radius, 0.0, 0.0));
    Dump("");
  }
}

ExtCylSurfaceTarget::~ExtCylSurfaceTarget()
{
}

G4double ExtCylSurfaceTarget::GetDistanceFromPoint(const G4ThreeVector& point,
                                                   const G4ThreeVector& dir) const
{
  if (fabs(dir.mag() - 1.0) > 1.0E-10) {
    B2FATAL("Simulation::ExtCylSurfaceTarget::GetDistanceFromPoint() direction is not a unit vector: " << dir);
  }

  // Get distance to intersection point with the cylinder's curved surface
  // should be negative if outside!  G4double dist = (point - IntersectLocal(point, dir)).mag();
  G4double dist = (IntersectLocal(point, dir) - point) * dir;

  // Get intersection point with the plane at either zmin or zmax
  G4double dirz = dir.z();
  if (dirz < -1.0E-10) {
    dist = fmin(dist, (m_zmin - point.z()) / dirz);
  } else if (dirz > 1.0E-10) {
    dist = fmin(dist, (m_zmax - point.z()) / dirz);
  }

  B2DEBUG(300, "Simulation::ExtCylSurfaceTarget::GetDistanceFromPoint():  Global point "
          << point << " dir " << dir << " distance " << dist);

  return dist;
}

G4double ExtCylSurfaceTarget::GetDistanceFromPoint(const G4ThreeVector& point) const
{

  G4double dist = m_radius - point.perp();
  dist = fmin(dist, point.z() - m_zmin);
  dist = fmin(dist, m_zmax - point.z());

  B2DEBUG(300, "Simulation::ExtCylSurfaceTarget::GetDistanceFromPoint():  Global point "
          << point << " minimum distance " << dist);

  return dist;
}

G4ThreeVector ExtCylSurfaceTarget::IntersectLocal(const G4ThreeVector& localPoint,
                                                  const G4ThreeVector& localDir) const
{
  // localDir has already been verified to be a unit vector
  G4double eqa = localDir.x() * localDir.x() + localDir.y() * localDir.y();
  G4double eqb = 2.0 * (localPoint.x() * localDir.x() + localPoint.y() * localDir.y());
  G4double eqc = localPoint.x() * localPoint.x() + localPoint.y() * localPoint.y()
                 - m_radius * m_radius;
  G4double eqaInside = (localPoint.perp() > m_radius) ? -eqa : eqa;

  G4ThreeVector intersection = localPoint;
  if (eqaInside > 0.0) {
    intersection += localDir * ((-eqb + sqrt(eqb * eqb - 4.0 * eqa * eqc)) / (2.0 * eqa));
  } else if (eqaInside < 0.0) {
    intersection += localDir * ((-eqb - sqrt(eqb * eqb - 4.0 * eqa * eqc)) / (2.0 * eqa));
  } else {
    if (eqb != 0.0) {
      intersection -= localDir * (eqc / eqb);
    } else {
      B2WARNING("Simulation::ExtCylSurfaceTarget::IntersectLocal():  localPoint "
                << localPoint << " localDir " << localDir << " does not intersect with cylinder");
      intersection = localDir * kInfinity;
    }
  }

  B2DEBUG(300, "Simulation::ExtCylSurfaceTarget::IntersectLocal(): localPoint "
          << localPoint << " localDir " << localDir << " radial intersection " << intersection.perp());

  return intersection;
}

G4Plane3D ExtCylSurfaceTarget::GetTangentPlane(const G4ThreeVector& point) const
{
  // check that point is at the cylinder's curved surface
  if (fabs(point.perp() - m_radius) > m_tolerance) {
    B2ERROR("Simulation::ExtCylSurfaceTarget::GetTangentPlane(): point "
            << point << " is not at surface; radial distance is " << point.perp() - m_radius);
  }

  G4Normal3D normal(point);

  return G4Plane3D(normal, point);
}

//! Dump the cylinder parameters: **EMPTY STUB**
void ExtCylSurfaceTarget::Dump(const G4String&) const
{
}

