/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <arich/dbobjects/ARICHGeoSupport.h>
#include <framework/gearbox/Unit.h>
#include <math.h>
#include <iostream>
#include <iomanip>
#include <TVector2.h>

using namespace std;
using namespace Belle2;


bool ARICHGeoSupport::isConsistent() const
{
  return true;
}


void ARICHGeoSupport::print(const std::string& title) const
{
  ARICHGeoBase::print(title);

  cout << " Default material: " << getMaterial() << endl;
  cout << " Placed tubes" << endl;

  unsigned nTube = getNTubes();
  for (unsigned i = 0; i < nTube; i++) {
    cout << "  " << getTubeName(i) << ", inner R: " << getTubeInnerR(i) << " " << s_unitName << ", outer R: " << getTubeOuterR(
           i) << " " << s_unitName << ", length: " <<  getTubeLength(i) << " " << s_unitName << ", Z position: " << getTubeZPosition(
           i) <<  " " << s_unitName << ", material: " << getTubeMaterial(i) <<  endl;  ;
  }

  cout << " Parameters of wedges" << endl;

  for (unsigned i = 1; i < m_nWedgeType + 1; i++) {
    cout << "  type: " << i << " par. vector: ";
    for (auto par : getWedge(i)) cout << par << " " << s_unitName << " ";
    cout << endl;
  }

  cout << " Placed wedges" << endl;
  unsigned nWedge = getNWedges();
  for (unsigned i = 0; i < nWedge; i++) {
    cout << "  ID: " << setprecision(4) << setw(2) <<  i << ", type: " << getWedgeType(i) << ", radius: " << getWedgeR(
           i) << " " << s_unitName << ", phi: " << setw(6) << getWedgePhi(i) << ", Z position: " << getWedgeZ(
           i) << " " << s_unitName << ", material: " << getWedgeMaterial(i) << endl;
  }
}
