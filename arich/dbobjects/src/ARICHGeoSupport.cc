/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <arich/dbobjects/ARICHGeoSupport.h>
#include <iostream>
#include <iomanip>

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
