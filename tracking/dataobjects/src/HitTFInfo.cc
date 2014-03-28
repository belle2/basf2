/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Stefan Ferstl                                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/dataobjects/HitTFInfo.h>

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
  m_sectorID = -1;
  m_use_counter = 0;
  m_max_counter = 0;
  m_use_counter_cell.clear();
  m_use_TC_counter_IDs.clear();
  m_is_real = 0;
  m_hitPos.SetXYZ(0., 0., 0.);
  m_hitSigma.SetXYZ(0., 0., 0.);
};

HitTFInfo::HitTFInfo(int par_pass_index, int par_sec_id, TVector3 par_position, TVector3 par_hitsigma): BaseTFInfo(par_pass_index)
{
  m_sectorID = par_sec_id;
  m_use_counter = 0;
  m_max_counter = 0;
  m_use_counter_cell.clear();
  m_use_TC_counter_IDs.clear();
  m_is_real = 0;
  m_hitPos = par_position;
  m_hitSigma = par_hitsigma;
};


// outer hit  = Index 0, inner hit = Index 1
// hit counts for the inner or outer hit => counter for each one
int HitTFInfo::changeUseCounterCell(std::vector<int> value)
{

  if (value.size() < m_use_counter_cell.size()) {
    B2DEBUG(10, "CollectorTFInfo: changeUseCounterCell too few input-values");
  }

  // if the vector is too small (standard at start => to size 2)
  while (value.size() > m_use_counter_cell.size()) {
    push_back_UseCounterCell(0);
  }

  int cell_change = 0;
  for (uint i = 0; i < value.size() ; i++) {
    m_use_counter_cell.at(i) +=  value.at(i);
    cell_change += value.at(i);
  }

  // Sum of delta changes to cells (could be negative)
  return cell_change;
}



/** if the Hit is overlaped */
bool HitTFInfo::isOverlappedByCellsOrTCs()
{
  bool is_overlapped = false;

  // CELLS
  int all_ok_found_cells = 0;

  // Search if hit is overlapped because of the cell
  for (auto & akt_cell : m_use_counter_cell) {

    //B2DEBUG(100,"Akt_cell" << akt_cell);

    //if hit is used once per cell (inner, outer) => no overlap
    // if hit is used more then once per cell => overlap
    if (akt_cell > 1) {
      is_overlapped = true;
      // Don't have to check other ones

      B2DEBUG(100, "HitTFInfo: isOverlappedByCellsOrTCs - Cell overlapped, akt_cell: " <<  akt_cell);

      return is_overlapped;
    }

    // Counter for ok used cells (once)
    else if (akt_cell == 1) {
      all_ok_found_cells ++;
    }
  }

  // TC

  //if there is a inner an a outer cell use / but there are more TCs => overlap
  // same TC => no overlap
  if (all_ok_found_cells > 1 && m_use_TC_counter_IDs.size() > 1) {
    is_overlapped = true;

    B2DEBUG(100, "HitTFInfo: isOverlappedByCellsOrTCs - TC overlapped ");

  }

  return is_overlapped;
}


/** add new int to UseCounterTCIDs */
void HitTFInfo::push_back_UseCounterTCIDs(int newMember)
{

  // -1 => no new Member
  if (newMember != -1) {
    std::vector<int>::iterator found = find(m_use_TC_counter_IDs.begin(), m_use_TC_counter_IDs.end(), newMember);

    // Only add the new TCID if not allready in the vector
    if (found == m_use_TC_counter_IDs.end()) {
      m_use_TC_counter_IDs.push_back(newMember);

      B2DEBUG(100, "push_back_UseCounterTCIDs newMember " << newMember << " *******");
    }


  }
}


/** add new int to AllCounterTCIDs */
void HitTFInfo::push_back_AllCounterTCIDs(int newMember)
{

  // -1 => no new Member
  if (newMember != -1) {
    std::vector<int>::iterator found = find(m_all_TC_counter_IDs.begin(), m_all_TC_counter_IDs.end(), newMember);

    // Only add the new TCID if not allready in the vector
    if (found == m_all_TC_counter_IDs.end()) {
      m_all_TC_counter_IDs.push_back(newMember);

      B2DEBUG(100, "push_back_AllCounterTCIDs newMember " << newMember << " *******");
    }

  }
}


/** remove int from UseCounterTCIDs */
void HitTFInfo::remove_UseCounterTCIDs(int oldMember)
{

  // -1 => no old Member
  if (oldMember != -1) {
    std::vector<int>::iterator found = find(m_use_TC_counter_IDs.begin(), m_use_TC_counter_IDs.end(), oldMember);

    // Only add the new TCID if not allready in the vector
    if (found != m_use_TC_counter_IDs.end()) {
      m_use_TC_counter_IDs.erase(found);

      B2DEBUG(100, "removed from m_use_TC_counter_IDs oldMember " << oldMember << " *******");
    }
  }
}

