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

ClassImp(Belle2::EKLMStripHit);

void EKLMStripHit::setNumberPhotoElectrons(const double &npe)
{
  m_NumberPhotoElectrons = npe;
}

void EKLMStripHit::setTime(const double &time)
{
  m_Time = time;
}

void EKLMStripHit::setLeadingParticlePDGCode(const int &pdg)
{
  m_LeadingParticlePDGCode = pdg;
}

double EKLMStripHit::getTime()
{
  return m_Time;
}

double EKLMStripHit::getNumberPhotoElectrons()
{
  return m_NumberPhotoElectrons;
}

int EKLMStripHit::getLeadingParticlePDGCode()
{
  return m_LeadingParticlePDGCode;
}

G4VPhysicalVolume *EKLMStripHit::getPV()
{
  return m_pv;
}

void EKLMStripHit::setPV(G4VPhysicalVolume *pv)
{
  m_pv = pv;
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

double EKLMStripHit::getLightPropagationLength(CLHEP::Hep3Vector &pos)
{
  G4Box *box = (G4Box*)(m_pv->GetLogicalVolume()->GetSolid());
  double half_len = box->GetXHalfLength();
  HepGeom::Point3D<double> p(pos);
  HepGeom::Point3D<double> pt = ((G4PVPlacementGT*)m_pv)->getTransform()
                                .inverse() * p;
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
                                 getTransform() * p1;

  G4Box *box2 = (G4Box*)(m_pv->GetLogicalVolume()->GetSolid());
  double max2 = 2.0 * box2->GetXHalfLength();
  HepGeom::Point3D<double> p2(0.5 * max2, 0., 0.);
  HepGeom::Point3D<double> pt2 = ((G4PVPlacementGT*)m_pv)->
                                 getTransform() * p2;

  crossPoint.setZ((pt1.z() + pt2.z()) / 2);

  if (CheckStripOrientationX(hit->getPV())) {
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

