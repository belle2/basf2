/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <eklm/eklmhit/EKLMStripHit.h>
#include <eklm/eklmutils/EKLMutils.h>
#include <eklm/geoeklm/GeoEKLMBelleII.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/GearDir.h>
//#include <framework/datastore/Units.h>



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
  double global[3] = {pos.getX(), pos.getY(), pos.getZ()};
  double local[3] = {0, 0, 0};
  double min = 0;
  double max = 0;

  gGeoManager->cd((EKLMNameManipulator::getNodePath(m_Name)).c_str());
  gGeoManager->MasterToLocal(global, local);
  gGeoManager->GetCurrentVolume()->GetShape()->GetAxisRange(1, min, max);
  m_LightPropagationLength = max - local[0];
  return m_LightPropagationLength;
}

bool EKLMStripHit::doesIntersect(EKLMStripHit * hit, CLHEP::Hep3Vector & crossPoint)
{

  double local[3] = {0, 0, 0};
  double min = 0;
  double max1 = 0;
  double max2 = 0;


  double global1[3] = {0, 0, 0};
  gGeoManager->cd((EKLMNameManipulator::getNodePath(hit->getName())).c_str());
  gGeoManager->LocalToMaster(local, global1);
  gGeoManager->GetCurrentVolume()->GetShape()->GetAxisRange(1, min, max1);

  double global2[3] = {0, 0, 0};
  gGeoManager->cd((EKLMNameManipulator::getNodePath(m_Name)).c_str());
  gGeoManager->LocalToMaster(local, global2);
  gGeoManager->GetCurrentVolume()->GetShape()->GetAxisRange(1, min, max2);

  crossPoint.setZ((global1[2] + global2[2]) / 2);

  if (EKLMNameManipulator::isX(hit->getName())) {
    if (abs(global1[0] - global2[0]) <= max1 &&  abs(global1[1] - global2[1]) <= max2) {
      crossPoint.setX(global2[0]);
      crossPoint.setY(global1[1]);
      return true;
    }
  } else {
    if (abs(global1[0] - global2[0]) <= max2 &&  abs(global1[1] - global2[1]) <= max1) {
      crossPoint.setX(global1[0]);
      crossPoint.setY(global2[1]);
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

