/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Stefan Ferstl                                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingVXD/displayInterfaceTF/HitTFInfo.h>

using namespace std;
using namespace Belle2;

/**
 * Implementation of Methodes of HitTFInfo:
 *   changeUseCounterCell: returns counter of all Cell-Change-Counters, changes hit counts for the inner or outer cells
 *   isOverlappedByCellsOrTcs: returns true if Hit is overlapped due connected Cells and/or connected TCs
 *   push_back_UseCounterTCIDs: add new connected TC-ID (-1 = no new member)
 *
 **/

ClassImp(HitTFInfo)

HitTFInfo::HitTFInfo()
{
  m_sectorID = 0;
  m_useCounter = 0;
  m_maxCounter = 0;
  m_useCounterCell.clear();
  m_useTCCounterIDs.clear();
  m_isReal = 0;
  m_hitPos.SetXYZ(0., 0., 0.);
  m_hitSigma.SetXYZ(0., 0., 0.);
};

HitTFInfo::HitTFInfo(int parPassIndex, unsigned int parSecId, TVector3 parPosition, TVector3 parHitsigma): BaseTFInfo(parPassIndex)
{
  m_sectorID = parSecId;
  m_useCounter = 0;
  m_maxCounter = 0;
  m_useCounterCell.clear();
  m_useTCCounterIDs.clear();
  m_isReal = 0;
  m_hitPos = parPosition;
  m_hitSigma = parHitsigma;
};


// outer hit  = Index 0, inner hit = Index 1
// hit counts for the inner or outer hit => counter for each one
int HitTFInfo::changeUseCounterCell(std::vector<int> value)
{

  if (value.size() < m_useCounterCell.size()) {
    B2DEBUG(10, "CollectorTFInfo: changeUseCounterCell too few input-values");
  }

  // if the vector is too small (standard at start => to size 2)
  while (value.size() > m_useCounterCell.size()) {
    push_back_UseCounterCell(0);
  }

  int cellChange = 0;
  for (uint i = 0; i < value.size() ; i++) {
    m_useCounterCell.at(i) +=  value.at(i);
    cellChange += value.at(i);
  }

  // Sum of delta changes to cells (could be negative)
  return cellChange;
}



/** if the Hit is overlaped */
bool HitTFInfo::isOverlappedByCellsOrTCs()
{
  bool isOverlapped = false;

  // CELLS
  int allOkFoundCells = 0;

  // Search if hit is overlapped because of the cell
  for (auto & currentCell : m_useCounterCell) {

    //B2DEBUG(100,"Akt_cell" << currentCell);

    //if hit is used once per cell (inner, outer) => no overlap
    // if hit is used more then once per cell => overlap
    if (currentCell > 1) {
      isOverlapped = true;
      // Don't have to check other ones

      B2DEBUG(100, "HitTFInfo: isOverlappedByCellsOrTCs - Cell overlapped, currentCell: " <<  currentCell);

      return isOverlapped;
    }

    // Counter for ok used cells (once)
    else if (currentCell == 1) {
      allOkFoundCells ++;
    }
  }

  // TC

  //if there is a inner an a outer cell use / but there are more TCs => overlap
  // same TC => no overlap
  if (allOkFoundCells > 1 && m_useTCCounterIDs.size() > 1) {
    isOverlapped = true;

    B2DEBUG(100, "HitTFInfo: isOverlappedByCellsOrTCs - TC overlapped ");

  }

  return isOverlapped;
}


/** add new int to UseCounterTCIDs */
void HitTFInfo::push_back_UseCounterTCIDs(int newMember)
{

  // -1 => no new Member
  if (newMember != -1) {
    auto found = find(m_useTCCounterIDs.begin(), m_useTCCounterIDs.end(), newMember);

    // Only add the new TCID if not allready in the vector
    if (found == m_useTCCounterIDs.end()) {
      m_useTCCounterIDs.push_back(newMember);

      B2DEBUG(100, "push_back_UseCounterTCIDs newMember " << newMember << " *******");
    }


  }
}


/** add new int to AllCounterTCIDs */
void HitTFInfo::push_back_AllCounterTCIDs(int newMember)
{

  // -1 => no new Member
  if (newMember != -1) {
    auto found = find(m_allTCCounterIDs.begin(), m_allTCCounterIDs.end(), newMember);

    // Only add the new TCID if not allready in the vector
    if (found == m_allTCCounterIDs.end()) {
      m_allTCCounterIDs.push_back(newMember);

      B2DEBUG(100, "push_back_AllCounterTCIDs newMember " << newMember << " *******");
    }

  }
}


/** remove int from UseCounterTCIDs */
void HitTFInfo::remove_UseCounterTCIDs(int oldMember)
{

  // -1 => no old Member
  if (oldMember != -1) {
    auto found = find(m_useTCCounterIDs.begin(), m_useTCCounterIDs.end(), oldMember);

    // Only add the new TCID if not allready in the vector
    if (found != m_useTCCounterIDs.end()) {
      m_useTCCounterIDs.erase(found);

      B2DEBUG(100, "removed from m_useTCCounterIDs oldMember " << oldMember << " *******");
    }
  }
}

