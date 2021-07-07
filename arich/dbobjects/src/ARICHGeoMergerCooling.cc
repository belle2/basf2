/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//basf2
#include <arich/dbobjects/ARICHGeoMergerCooling.h>
#include <arich/dbobjects/tessellatedSolidStr.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <math.h>
#include <iostream>
#include <iomanip>

//root
#include <TVector3.h>

using namespace std;
using namespace Belle2;

void ARICHGeoMergerCooling::print(const std::string& title) const
{

  ARICHGeoBase::print(title);

  cout << " Merger cooling bodies info" << endl;
  cout << " Merger cooling bodies material name : " << getMergerCoolingBodiesMaterialName() << endl;
  cout << " m_mergerCoolingBodiesInfo.size() = " << m_mergerCoolingBodiesInfo.size() << endl;

  for (unsigned i = 0; i < m_mergerCoolingBodiesInfo.size(); i++) {
    tessellatedSolidStr mergerCoolingBodiesStr = getMergerCoolingBodiesInfo(i + 1);
    mergerCoolingBodiesStr.printInfo();
    //mergerCoolingBodiesStr.printInfo(1);
  }

  cout << "mergerCoolingBodiesv2/mergerCoolingPositionID" << endl;
  for (unsigned i = 0; i < getMergerCoolingPositionID().size(); i++) {
    cout << setw(20) << i + 1
         << setw(25) << getMergerCoolingPositionID().at(i) << endl;
  }

}

void ARICHGeoMergerCooling::checkMergerCoolingSystemDataConsistency() const
{

  B2ASSERT("Data of the cooling system V2 is inconsisten : getMergerCoolingPositionID().size() != 72",
           getMergerCoolingPositionID().size() == 72);
  for (unsigned i = 0; i < m_mergerCoolingBodiesInfo.size(); i++) {
    tessellatedSolidStr mergerCoolingBodiesStr = getMergerCoolingBodiesInfo(i + 1);
  }
}

const tessellatedSolidStr ARICHGeoMergerCooling::getMergerCoolingBodiesInfo_globalMergerID(unsigned int iMergerSlot) const
{
  B2ASSERT("iMergerSlot <=0 ", iMergerSlot > 0);
  B2ASSERT("iMergerSlot > 72", iMergerSlot <= 72);
  int positionID = (int)getMergerCoolingPositionID().at(iMergerSlot - 1);
  B2ASSERT("positionID <=0 ", positionID > 0);
  B2ASSERT("positionID > 12", positionID <= 12);
  return getMergerCoolingBodiesInfo(positionID);
}
