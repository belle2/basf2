/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <pxd/modules/pxdHelper/PXDBadSensorTagModule.h>
#include <framework/core/ModuleParam.templateDetails.h>


using namespace std;
using namespace Belle2;
using namespace Belle2::PXD;
using namespace Belle2::PXD::PXDError;


REG_MODULE(PXDBadSensorTag)


PXDBadSensorTagModule::PXDBadSensorTagModule() :
  Module()
{
  //Set module properties
  setDescription("Mark bad-data PXD modules");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("zeroSuppressionCut", m_0cut, "Minimum charge for a raw hit to carry", 0);
  addParam("nrHitsCut", m_nrHitsCut, "Cut on nr hits per module [[id1,cut1],[id1,cut2],...]");
}

void PXDBadSensorTagModule::initialize()
{
  //Register output collections
  m_storeRawHits.isRequired(m_PXDRawHitsName);
  m_storeDAQEvtStats.isRequired(m_PXDDAQEvtStatsName);

  for (auto& m : m_nrHitsCut) {
    if (m.size() != 2) { B2ERROR("Wrong nr of Parameter " << m.size()); continue;}
    m_cut[VxdID(m[0])] = m[1];
  }
}

void PXDBadSensorTagModule::event()
{
  std::map <VxdID, int> freq;// count the number of RawHits per sensor
  for (auto& p : m_storeRawHits) {
    if (p.getCharge() < m_0cut) continue;// only count above some threshold
    freq[p.getSensorID()]++;
  }

  for (auto& p : *m_storeDAQEvtStats) {
    for (auto& c : p) {
      for (auto& e : c) {
        if (freq[e.getSensorID()] > m_cut[e.getSensorID()]) {
          e.markUnusable();
        }
      }
    }
  }
}
