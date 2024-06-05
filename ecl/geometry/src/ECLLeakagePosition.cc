/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//..ECL
#include <ecl/geometry/ECLLeakagePosition.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>
#include <ecl/geometry/ECLNeighbours.h>

//..Other
#include <iostream>
#include <TMath.h>

using namespace Belle2;
using namespace ECL;

//..Constructor.
ECLLeakagePosition::ECLLeakagePosition() :
  m_ECLCrystalThetaEdge("ECLCrystalThetaEdge"),
  m_ECLCrystalPhiEdge("ECLCrystalPhiEdge"),
  m_ECLCrystalThetaWidth("ECLCrystalThetaWidth"),
  m_ECLCrystalPhiWidth("ECLCrystalPhiWidth")
{

  //..Obtain the vectors of crystal edges and widths from the database
  if (m_ECLCrystalThetaEdge.hasChanged()) {
    m_thetaEdge = m_ECLCrystalThetaEdge->getCalibVector();
  }
  if (m_ECLCrystalPhiEdge.hasChanged()) {
    m_phiEdge = m_ECLCrystalPhiEdge->getCalibVector();
  }
  if (m_ECLCrystalThetaWidth.hasChanged()) {
    m_thetaWidth = m_ECLCrystalThetaWidth->getCalibVector();
  }
  if (m_ECLCrystalPhiWidth.hasChanged()) {
    m_phiWidth = m_ECLCrystalPhiWidth->getCalibVector();
  }

  //..Eight nearest neighbours, plus crystal itself. Uses cellID, 1--8736
  m_neighbours = new ECLNeighbours("N", 1);

  //..Record the thetaID and phiID of each cellID
  for (int thID = 0; thID < 69; thID++) {
    for (int phID = 0; phID < m_neighbours->getCrystalsPerRing(thID); phID++) {
      m_thetaIDofCrysID.push_back(thID);
      m_phiIDofCrysID.push_back(phID);
    }
  }

  //..Crystals between mechanical structure for each thetaID
  for (int thID = 0; thID < m_firstBarrelThetaID; thID++) {
    int nCrys = m_neighbours->getCrystalsPerRing(thID) / 16;
    m_crysBetweenMech.push_back(nCrys);
  }
  for (int thID = m_firstBarrelThetaID; thID <= m_lastBarrelThetaID; thID++) {
    m_crysBetweenMech.push_back(2);
  }
  for (int thID = m_lastBarrelThetaID + 1; thID < 69; thID++) {
    int nCrys = m_neighbours->getCrystalsPerRing(thID) / 16;
    m_crysBetweenMech.push_back(nCrys);
  }
}

ECLLeakagePosition::~ECLLeakagePosition()
{
  delete m_neighbours;
}

std::vector<int> ECLLeakagePosition::getLeakagePosition(const int cellIDFromEnergy, const float theta, const float phi,
                                                        const int nPositions)
{

  //..Start by checking if the crystal with the most energy is the correct one
  int crysID = cellIDFromEnergy - 1;
  int iStatus = -1;

  //..theta and phi need to be within the crystal
  float dTheta = theta - m_thetaEdge[crysID];
  float dPhi = phi - m_phiEdge[crysID];
  if (dPhi > TMath::Pi()) {dPhi -= 2.*TMath::Pi();}
  if (dPhi < -TMath::Pi()) {dPhi += 2.*TMath::Pi();}
  if (dTheta >= 0 and dTheta <= m_thetaWidth[crysID] and dPhi >= 0 and dPhi <= m_phiWidth[crysID]) {
    iStatus = 0;
  }

  //..Theta and phi are not located in the maximum energy crystal. Check the
  //  eight nearest neighbours.
  if (iStatus == -1) {

    //..Nearest neighbours (plus the central crystal)
    for (const auto& tempCellID : m_neighbours->getNeighbours(cellIDFromEnergy)) {
      int tempCrysID = tempCellID - 1;
      dTheta = theta - m_thetaEdge[tempCrysID];
      dPhi = phi - m_phiEdge[tempCrysID];
      if (dPhi > TMath::Pi()) {dPhi -= 2.*TMath::Pi();}
      if (dPhi < -TMath::Pi()) {dPhi += 2.*TMath::Pi();}

      //..This one is correct
      if (dTheta >= 0 and dTheta <= m_thetaWidth[tempCrysID] and dPhi >= 0 and dPhi <= m_phiWidth[tempCrysID]) {
        iStatus = 1;
        crysID = tempCrysID;
        break;
      }
    }
  }

  //..Need to do this one by brute force
  if (iStatus == -1) {

    //..Start at the last crystal and work backwards
    for (int crys = 8735; crys >= 0; crys--) {
      dTheta = theta - m_thetaEdge[crys];
      dPhi = phi - m_phiEdge[crys];
      if (dPhi > TMath::Pi()) {dPhi -= 2.*TMath::Pi();}
      if (dPhi < -TMath::Pi()) {dPhi += 2.*TMath::Pi();}
      if (dPhi >= 0 and dPhi <= m_phiWidth[crys] and dTheta >= 0) {
        iStatus = 2;
        crysID = crys;
        break;
      }
    }
  }

  //..Above should cover all cases, but set sensible values if we missed something
  if (iStatus == -1) {
    iStatus = 3;
    crysID = cellIDFromEnergy - 1;
    dTheta = 0.5;
    dPhi = 0.5;
  }

  //..Return values
  int cellID = crysID + 1;
  int thetaID = m_thetaIDofCrysID[crysID];
  int iRegion = 0;
  if (thetaID >= m_firstBarrelThetaID and thetaID <= m_lastBarrelThetaID) {iRegion = 1;}
  if (thetaID > m_lastBarrelThetaID) {iRegion = 2;}

  //..Mechanical structure is on lower edge of phiID 0, and every 2 (barrel) or n (endcap)
  //  crystals thereafter
  int iPhiMech = m_phiIDofCrysID[crysID] % m_crysBetweenMech[thetaID];
  bool reversePhi;

  //..Mechanical structure on lower edge
  if ((iPhiMech == 0 and iRegion != 1) or (iPhiMech == m_crysBetweenMech[thetaID] - 1 and iRegion == 1)) {
    iPhiMech = 0;
    reversePhi = false;

    //..Mechanical structure on upper edge
  } else if ((iPhiMech == m_crysBetweenMech[thetaID] - 1 and iRegion != 1) or (iPhiMech == 0 and iRegion == 1)) {
    iPhiMech = 0;
    reversePhi = true;

    //..No mechanical structure adjacent
  } else {
    iPhiMech = 1;
    reversePhi = false;
  }

  //..Divide crystal into nPositions in width, starting from lower edge
  int iLocalTheta = (int)(nPositions * dTheta / m_thetaWidth[crysID]);
  if (iLocalTheta < 0) {iLocalTheta = 0;}
  if (iLocalTheta >= nPositions) {iLocalTheta = nPositions - 1;}

  int iLocalPhi = (int)(nPositions * dPhi / m_phiWidth[crysID]);
  if (iLocalPhi < 0) {iLocalPhi = 0;}
  if (iLocalPhi >= nPositions) {iLocalPhi = nPositions - 1;}

  //..iLocalPhi is measured from edge with mechanical structure, if present,
  //  so reverse order of iLocalPhi if mech structure is on the upper edge
  if (reversePhi) {iLocalPhi = nPositions - iLocalPhi - 1;}

  //..Return as a std::vector
  std::vector<int> position;
  position.push_back(cellID);
  position.push_back(thetaID);
  position.push_back(iRegion);
  position.push_back(iLocalTheta);
  position.push_back(iLocalPhi);
  position.push_back(iPhiMech);
  position.push_back(iStatus);
  return position;
}
