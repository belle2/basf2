/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <G4Box.hh>

#include <eklm/eklmhit/EKLMStripHit.h>
#include <eklm/eklmutils/EKLMutils.h>
#include <eklm/geoeklm/G4PVPlacementGT.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/GearDir.h>

using namespace std;
using namespace Belle2;

ClassImp(Belle2::EKLMStripHit);

EKLMStripHit::EKLMStripHit(const char * name)
{
  m_Name = name;
}

EKLMStripHit::EKLMStripHit(std::string & name)
{
  m_Name = name;
}

void EKLMStripHit::Print()
{
  std::cout << "Hit Strip:" << m_Name << "      Time:" << m_Time << std::endl;

}

double EKLMStripHit::getLightPropagationLength(CLHEP::Hep3Vector &pos)
{
  G4Box *box = (G4Box*)(m_pv->GetLogicalVolume()->GetSolid());
  double half_len = box->GetXHalfLength();
  HepGeom::Point3D<double> p(pos);
  HepGeom::Point3D<double> pt = ((G4PVPlacementGT*)m_pv)->getTransform() * p;
  m_LightPropagationLength = half_len - pt.x();
  return m_LightPropagationLength;
}

bool EKLMStripHit::doesIntersect(EKLMStripHit * hit,
                                 CLHEP::Hep3Vector & crossPoint)
{
  G4Box *box1 = (G4Box*)(hit->getPV()->GetLogicalVolume()->GetSolid());
  double max1 = 2.0 * box1->GetXHalfLength();
  HepGeom::Point3D<double> p1(0.5 * max1, 0., 0.);
  HepGeom::Point3D<double> pt1 = ((G4PVPlacementGT*)(hit->getPV()))->
                                 getTransform().inverse() * p1;

  G4Box *box2 = (G4Box*)(m_pv->GetLogicalVolume()->GetSolid());
  double max2 = 2.0 * box2->GetXHalfLength();
  HepGeom::Point3D<double> p2(0.5 * max2, 0., 0.);
  HepGeom::Point3D<double> pt2 = ((G4PVPlacementGT*)m_pv)->
                                 getTransform().inverse() * p2;

  crossPoint.setZ((pt1.z() + pt2.z()) / 2);

  if (CheckStripOrientationX(hit->getPV())) {
    if (fabs(pt1.x() - pt2.x()) <= max1 && fabs(pt1.y() - pt2.y()) <= max2) {
      crossPoint.setX(pt2.x());
      crossPoint.setY(pt1.y());
      return true;
    }
  } else {
    if (fabs(pt1.x() - pt2.x()) <= max2 && fabs(pt1.y() - pt2.y()) <= max1) {
      crossPoint.setX(pt1.x());
      crossPoint.setY(pt2.y());
      return true;
    }
  }
  return false;

}

