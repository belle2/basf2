/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
