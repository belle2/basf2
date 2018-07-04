/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <arich/dbobjects/ARICHCopperMapping.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>

#include <arich/dbobjects/ARICHMergerMapping.h>

#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>

using namespace std;
using namespace Belle2;

ARICHCopperMapping::ARICHCopperMapping()
{
  m_merger2copper.assign(N_MERGERS, std::make_pair(0, 0));
}

unsigned ARICHCopperMapping::getCopperID(unsigned mergerID) const
{
// if (mergerID > N_MERGERS + 1 || mergerID == 0) B2ERROR("ARICHCopperMapping::getCopperID: invalid merger ID number!");
  return (unsigned)m_merger2copper[mergerID - 1].first;
}

unsigned ARICHCopperMapping::getFinesse(unsigned mergerID) const
{
// if (mergerID > N_MERGERS + 1 || mergerID == 0) B2ERROR("ARICHCopperMapping::getFinesse: invalid merger ID number!");
  return (unsigned)m_merger2copper[mergerID - 1].second;
}

void ARICHCopperMapping::addMapping(unsigned mergerID, unsigned copperID, unsigned finesse)
{

// if (mergerID > N_MERGERS + 1 || mergerID == 0) B2ERROR("ARICHCopperMapping::addMapping: invalid merger ID number!");
  if (finesse > 3) B2ERROR("ARICHCopperMapping::addMapping: invalid finesse number (0-3)!");

  m_merger2copper[mergerID - 1] = std::make_pair(copperID, finesse);

  if (m_copper2merger.find(copperID) == m_copper2merger.end()) {
    std::vector<uint16_t> mergerList = {0, 0, 0, 0};
    mergerList[finesse] = mergerID;
    m_copper2merger.insert(std::pair<uint16_t, std::vector<uint16_t>>((uint16_t)copperID, mergerList));
  } else {
    std::vector<uint16_t>& mergerList = m_copper2merger[copperID];
    mergerList[finesse] = mergerID;
  }

  if (std::find(m_copperIDs.begin(), m_copperIDs.end(), uint16_t(copperID)) == m_copperIDs.end()) m_copperIDs.push_back(copperID);
}

unsigned ARICHCopperMapping::getMergerID(unsigned copperID, unsigned finesse) const
{
  if (finesse > 3) B2ERROR("ARICHCopperMapping::getMergerID: invalid finesse number (0-3)!");
  auto it = m_copper2merger.find(copperID);
  if (it == m_copper2merger.end()) return 0;
  return (unsigned)it->second[finesse];
}

void ARICHCopperMapping::print() const
{
  cout << endl;
  cout << "Mapping of merger to copper boards" << endl;
  cout << endl;
  for (auto copper : m_copperIDs) {
    cout << " Merger boards connected to copper board " << copper << endl;
    for (int i = 0; i < 4; i++) {
      if (getMergerID(copper, i)) cout << "  Finesse " << i << ":  " << getMergerID(copper, i) << endl;
      else cout << "  Finesse " << i << ":  not connected" << endl;
    }
  }
}
