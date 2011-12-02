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

#include <eklm/dataobjects/EKLMStripHit.h>
#include <eklm/eklmutils/EKLMutils.h>
#include <eklm/geoeklm/G4PVPlacementGT.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/GearDir.h>

using namespace std;
using namespace Belle2;
using namespace CLHEP;

ClassImp(Belle2::EKLMStripHit);


EKLMStripHit::EKLMStripHit(const EKLMSimHit * hit)
    : EKLMHitBase((EKLMHitBase)(*hit)),
    m_Plane(hit->getPlane()),
    m_Strip(hit->getStrip()),
    m_NumberPhotoElectrons(-1),
    m_LightPropagationLength(0),
    m_pv(hit->getVolume())
{}



double EKLMStripHit::getNumberPhotoElectrons() const
{
  return m_NumberPhotoElectrons;
}

void EKLMStripHit::setNumberPhotoElectrons(double npe)
{
  m_NumberPhotoElectrons = npe;
}


int EKLMStripHit::getPlane() const
{
  return m_Plane;
}
void EKLMStripHit::setPlane(int plane)
{
  m_Plane = plane;
}
int EKLMStripHit::getStrip() const
{
  return m_Strip;
}
void EKLMStripHit::setStrip(int strip)
{
  m_Strip = strip;
}




const G4VPhysicalVolume* EKLMStripHit::getVolume() const
{
  return m_pv;
}

void EKLMStripHit::setVolume(const G4VPhysicalVolume *pv)
{
  m_pv = pv;
}



bool EKLMStripHit::doesIntersect(EKLMStripHit * hit,
                                 Hep3Vector & crossPoint)
{
  G4Box *box1 = (G4Box*)(hit->getVolume()->GetLogicalVolume()->GetSolid());
  double max1 = 2.0 * box1->GetXHalfLength();
  HepGeom::Point3D<double> p1(0.5 * max1, 0., 0.);
  HepGeom::Point3D<double> pt1 = ((G4PVPlacementGT*)(hit->getVolume()))->
                                 getTransform() * p1;

  G4Box *box2 = (G4Box*)(m_pv->GetLogicalVolume()->GetSolid());
  double max2 = 2.0 * box2->GetXHalfLength();
  HepGeom::Point3D<double> p2(0.5 * max2, 0., 0.);
  HepGeom::Point3D<double> pt2 = ((G4PVPlacementGT*)m_pv)->
                                 getTransform() * p2;

  crossPoint.setZ((pt1.z() + pt2.z()) / 2);

  if (CheckStripOrientationX(hit->getVolume())) {
    if (fabs(pt1.x() - pt2.x()) <= max1 && fabs(pt1.y() - pt2.y()) <= max2 &&
        fabs(pt2.x()) <= fabs(pt1.x()) && fabs(pt1.y()) <= fabs(pt2.y())) {
      crossPoint.setX(pt2.x());
      crossPoint.setY(pt1.y());
      return true;
    }
  } else {
    if (fabs(pt1.x() - pt2.x()) <= max2 && fabs(pt1.y() - pt2.y()) <= max1 &&
        fabs(pt1.x()) <= fabs(pt2.x()) && fabs(pt2.y()) <= fabs(pt1.y())) {
      crossPoint.setX(pt1.x());
      crossPoint.setY(pt2.y());
      return true;
    }
  }
  return false;
}


double EKLMStripHit::getLightPropagationLength(Hep3Vector &pos)
{
  G4Box *box = (G4Box*)(m_pv->GetLogicalVolume()->GetSolid());
  double half_len = box->GetXHalfLength();
  HepGeom::Point3D<double> p(pos);
  HepGeom::Point3D<double> pt = ((G4PVPlacementGT*)m_pv)->getTransform()
                                .inverse() * p;
  m_LightPropagationLength = half_len - pt.x();
  return m_LightPropagationLength;
}





void EKLMStripHit::Print()
{
  std::cout << "Endcap: " << getEndcap()
            << " Layer: " << getLayer()
            << " Sector: " << getSector()
            << " Plane: " << getPlane()
            << " Strip: " << getStrip()
            << " # Time: " << m_Time << "\n";
}

