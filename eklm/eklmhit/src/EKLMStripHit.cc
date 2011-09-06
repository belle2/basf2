/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <G4Box.hh>

#include <eklm/eklmhit/EKLMStripHit.h>
#include <eklm/eklmutils/EKLMutils.h>
#include <eklm/geoeklm/GeoEKLMBelleII.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/GearDir.h>

using namespace std;
using namespace Belle2 ;

ClassImp(Belle2::EKLMStripHit);


EKLMStripHit::EKLMStripHit(const char * name)
{
  m_Name = name;
}

EKLMStripHit::EKLMStripHit(std::string & name)
{
  m_Name = name;
}

void  EKLMStripHit::Print()
{
  std::cout << "Hit Strip:" << m_Name << "      Time:" << m_Time << std::endl;

}

double EKLMStripHit::getLightPropagationLength(CLHEP::Hep3Vector &pos)
{
  G4Box *box = (G4Box*)(m_pv->GetLogicalVolume()->GetSolid());
  double max = 2.0 * box->GetXHalfLength();
  HepGeom::Normal3D<double> p(pos);
  p.transform(EKLMVolumeTransforms.find((G4PVPlacement*)m_pv)->second);
  m_LightPropagationLength = max - p.x();
  return m_LightPropagationLength;
}

bool EKLMStripHit::doesIntersect(EKLMStripHit * hit,
                                 CLHEP::Hep3Vector & crossPoint)
{
  G4ThreeVector local(0., 0., 0.);
  G4Box *box1 = (G4Box*)(hit->getPV()->GetLogicalVolume()->GetSolid());
  double max1 = 2.0 * box1->GetXHalfLength();
  HepGeom::Normal3D<double> p1(local);
  p1.transform(EKLMVolumeTransforms.find((G4PVPlacement*)(hit->getPV()))->
               second.inverse());

  G4Box *box2 = (G4Box*)(m_pv->GetLogicalVolume()->GetSolid());
  double max2 = 2.0 * box2->GetXHalfLength();
  HepGeom::Normal3D<double> p2(local);
  p2.transform(EKLMVolumeTransforms.find((G4PVPlacement*)m_pv)->
               second.inverse());

  crossPoint.setZ((p1.z() + p2.z()) / 2);

  if (EKLMNameManipulator::isX(hit->getName())) {
    if (abs(p1.x() - p2.x()) <= max1 &&  abs(p1.y() - p2.y())
        <= max2) {
      crossPoint.setX(p2.x());
      crossPoint.setY(p1.y());
      return true;
    }
  } else {
    if (abs(p1.x() - p2.x()) <= max2 &&  abs(p1.y() - p2.y())
        <= max1) {
      crossPoint.setX(p1.x());
      crossPoint.setY(p2.y());
      return true;
    }
  }
  //    std::cout << "doesIntersect(EKLMStripHit * hit, CLHEP::Hep3Vector & crossPoint)" << std::endl;
  //    std::cout << "(" << global1[0] << " " << global1[1] << " " << global1[2] << ") " << max1 << "  ";
  //    if (EKLMNameManipulator::isX(hit->getName())) std::cout << " X ;"; else std::cout << " Y ;";
  //    std::cout << "(" << global2[0] << " " << global2[1] << " " << global2[2] << ") " << max2;
  //    if (EKLMNameManipulator::isX(m_Name)) std::cout << " X ;"; else std::cout << " Y ;";
  //    std::cout << std::endl;
  return false;

}

