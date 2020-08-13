/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/dataobjects/PXDDAQStatus.h>

using namespace std;
using namespace Belle2;


const PXDDAQDHEStatus* PXDDAQStatus::findDHE(const VxdID& id) const
{
  for (auto& pkt : m_pxdPacket) {
    for (auto it_dhc = pkt.cbegin(); it_dhc != pkt.cend(); ++it_dhc) {
      for (auto it_dhe = it_dhc->cbegin(); it_dhe != it_dhc->cend(); ++it_dhe) {
        if (it_dhe->getSensorID() == id) {
          return &(*it_dhe);
        }
      }
    }
  }
  return nullptr;
}

std::map <VxdID , bool> PXDDAQStatus::getUsable() const
{
  // This function assumes that each DHE is only present ONCE
  // The check for that must be done before!
  std::map <VxdID , bool> usemap;
  for (auto& pkt : m_pxdPacket) {
    for (auto it_dhc = pkt.cbegin(); it_dhc != pkt.cend(); ++it_dhc) {
      for (auto it_dhe = it_dhc->cbegin(); it_dhe != it_dhc->cend(); ++it_dhe) {
        usemap[it_dhe->getSensorID()] = it_dhe->isUsable();
      }
    }
  }
  return usemap;
}
