/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <eklm/dataobjects/EKLMHit2d.h>
#include <framework/logging/Logger.h>
#include "G4TransportationManager.hh"
#include "G4Navigator.hh"
#include "G4Box.hh"
//#include <eklm/geoeklm/G4PVPlacementGT.h>


using namespace Belle2;
using namespace std;

ClassImp(Belle2::EKLMHit2d);

EKLMHit2d::EKLMHit2d()
{
  m_XStrip = NULL;
  m_YStrip = NULL;
}

const Belle2::EKLMStripHit* EKLMHit2d::getXStripHit() const
{
  return m_XStrip;
}

const Belle2::EKLMStripHit* EKLMHit2d::getYStripHit() const
{
  return m_YStrip;
}

void EKLMHit2d::setCrossPoint(TVector3& point)
{
  m_crossPoint = point;
}

TVector3 EKLMHit2d::getCrossPoint() const
{
  return m_crossPoint;
}

double EKLMHit2d::getChiSq() const
{
  return m_ChiSq;
}

EKLMHit2d::EKLMHit2d(EKLMStripHit* xStrip, EKLMStripHit* yStrip)
{
  /*
  m_XStrip = xStrip;
  m_YStrip = yStrip;
  setEndcap(xStrip->getEndcap());
  setLayer(xStrip->getLayer());
  setSector(xStrip->getSector());
  */
}

void EKLMHit2d::Print()
{
  std::cout << "------------  Hit 2d  -------------- " << std::endl;
  std::cout << "Endcap: " << getEndcap()
            << " Layer: " << getLayer()
            << " Sector: " << getSector() << "\n";
  std::cout << "X: ";
  m_XStrip->Print();
  std::cout << "Y: ";
  m_YStrip->Print();
  std::cout << "intersection:";
  m_crossPoint.Print();

  std::cout << "ChiSq: " << m_ChiSq << std::endl;
}


bool EKLMHit2d::addStripHit(const EKLMStripHit* stripHit)
{
  /*
  if (m_XStrip != NULL && m_YStrip != NULL) {
    B2FATAL("Attempt to add more than 2 strips in 2d hit!");
    exit(0);
  }
  if (m_XStrip == NULL && CheckStripOrientationX(stripHit->getVolume())) {
    m_XStrip = stripHit;
    return true;
  }
  if (m_YStrip == NULL && !CheckStripOrientationX(stripHit->getVolume())) {
    m_YStrip = stripHit;
    return true;
  }
  return false;
  */
  return true;
}

void EKLMHit2d::setChiSq()
{
  /*
  double v = 17.0;  // 17cm/ns=speed of light; should be accessible via xml!
  double tX = m_XStrip->getTime() -
              getLightPropagationLength(m_XStrip->getVolume(), m_crossPoint) / v;
  double tY = m_YStrip->getTime() -
              getLightPropagationLength(m_YStrip->getVolume(), m_crossPoint) / v;

  double sigmaT = 1.;  // ns, smearing in time ; should be accessible via xml!
  m_ChiSq = (tX - tY) * (tX - tY) / sigmaT / sigmaT;
  */
}


const  G4VPhysicalVolume*  EKLMHit2d::GetPhysicalVolumeByPoint(const G4ThreeVector& point)
{

  return G4TransportationManager::GetTransportationManager()->
         GetNavigatorForTracking()->LocateGlobalPointAndSetup(point);

}


double EKLMHit2d::getLightPropagationLength(const G4VPhysicalVolume* vol, const CLHEP::Hep3Vector&  pos)
{
  /*
  if (GetPhysicalVolumeByPoint(pos) != vol)
    B2ERROR("POINT IS NOT INSIDE IT'S VOLUME!");

  G4Box* box = (G4Box*)(vol->GetLogicalVolume()->GetSolid());
  double half_len = box->GetXHalfLength();

  HepGeom::Point3D<double> pt = ((G4PVPlacementGT*)vol)->getTransform().inverse() * HepGeom::Point3D<double> (pos);
  return half_len - pt.x();
  */
  return 0;
}

double EKLMHit2d::getLightPropagationLength(const G4VPhysicalVolume* vol, const TVector3&  pos)
{
  //  return getLightPropagationLength(vol, CLHEP::Hep3Vector(pos.X(), pos.Y(), pos.Z()));
  return 0;
}

bool EKLMHit2d::CheckStripOrientationX(const G4VPhysicalVolume* strip)
{
  /*
  G4Transform3D t = ((G4PVPlacementGT*)strip)->getTransform().inverse();
  return (fabs(sin(t.getRotation().phiX())) < 0.01);
  */
  return 0;
}


bool EKLMHit2d::doesIntersect(const EKLMStripHit* hit1, const EKLMStripHit* hit2,
                              TVector3& crossPoint)
{
  /*
  G4Box* box1 = (G4Box*)(hit1->getVolume()->GetLogicalVolume()->GetSolid());
  double max1 = 2.0 * box1->GetXHalfLength();
  HepGeom::Point3D<double> p1(0.5 * max1, 0., 0.);
  HepGeom::Point3D<double> pt1 = ((G4PVPlacementGT*)(hit1->getVolume()))->
                                 getTransform() * p1;

  G4Box* box2 = (G4Box*)(hit2->getVolume()->GetLogicalVolume()->GetSolid());
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
  */
  return true;
}
