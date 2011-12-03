/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "G4TransportationManager.hh"
#include "G4Navigator.hh"
#include "G4Box.hh"
#include <framework/logging/Logger.h>

#include <eklm/geoeklm/G4PVPlacementGT.h>


#include <eklm/eklmutils/EKLMutils.h>

using namespace CLHEP;
namespace Belle2 {

  const  G4VPhysicalVolume *GetPhysicalVolumeByPoint(const G4ThreeVector &point)
  {
    return G4TransportationManager::GetTransportationManager()->
           GetNavigatorForTracking()->LocateGlobalPointAndSetup(point);
  }

  bool CheckStripOrientationX(const G4VPhysicalVolume *strip)
  {
    G4Transform3D t = ((G4PVPlacementGT*)strip)->getTransform().inverse();
    return (fabs(sin(t.getRotation().phiX())) < 0.01);
  }

  double getLightPropagationLength(const G4VPhysicalVolume * vol, const Hep3Vector  &pos)
  {
    if (GetPhysicalVolumeByPoint(pos) != vol)
      B2ERROR("POINT IS NOT INSIDE IT'S VOLUME!");

    G4Box *box = (G4Box*)(vol->GetLogicalVolume()->GetSolid());
    double half_len = box->GetXHalfLength();

    HepGeom::Point3D<double> pt = ((G4PVPlacementGT*)vol)->getTransform().inverse() * HepGeom::Point3D<double> (pos);
    return half_len - pt.x();
  }

  double getLightPropagationLength(const G4VPhysicalVolume * vol, const TVector3  &pos)
  {
    return getLightPropagationLength(vol, Hep3Vector(pos.X(), pos.Y(), pos.Z()));
  }



  bool doesIntersect(const EKLMStripHit * hit1, const EKLMStripHit * hit2,
                     TVector3 & crossPoint)
  {
    G4Box *box1 = (G4Box*)(hit1->getVolume()->GetLogicalVolume()->GetSolid());
    double max1 = 2.0 * box1->GetXHalfLength();
    HepGeom::Point3D<double> p1(0.5 * max1, 0., 0.);
    HepGeom::Point3D<double> pt1 = ((G4PVPlacementGT*)(hit1->getVolume()))->
                                   getTransform() * p1;

    G4Box *box2 = (G4Box*)(hit2->getVolume()->GetLogicalVolume()->GetSolid());
    double max2 = 2.0 * box2->GetXHalfLength();
    HepGeom::Point3D<double> p2(0.5 * max2, 0., 0.);
    HepGeom::Point3D<double> pt2 = ((G4PVPlacementGT*)(hit2->getVolume()))->
                                   getTransform() * p2;

    crossPoint.SetZ((pt1.z() + pt2.z()) / 2);

    if (CheckStripOrientationX(hit1->getVolume())) {
      if (fabs(pt1.x() - pt2.x()) <= max1 && fabs(pt1.y() - pt2.y()) <= max2 &&
          fabs(pt2.x()) <= fabs(pt1.x()) && fabs(pt1.y()) <= fabs(pt2.y())) {
        crossPoint.SetX(pt2.x());
        crossPoint.SetY(pt1.y());
        return true;
      }
    } else {
      if (fabs(pt1.x() - pt2.x()) <= max2 && fabs(pt1.y() - pt2.y()) <= max1 &&
          fabs(pt1.x()) <= fabs(pt2.x()) && fabs(pt2.y()) <= fabs(pt1.y())) {
        crossPoint.SetX(pt1.x());
        crossPoint.SetY(pt2.y());
        return true;
      }
    }
    crossPoint = TVector3(0., 0., 0.);
    return false;
  }







}
