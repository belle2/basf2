/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (torben.ferber@desy.de)                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
//This module
#include <ecl/modules/eclTRGInformation/ECLTRGInformationModule.h>

//Framework
#include <framework/logging/Logger.h>

//ECL
#include <ecl/dataobjects/ECLCalDigit.h>

//TRG
#include <trg/ecl/dataobjects/TRGECLUnpackerStore.h>
#include <trg/ecl/TrgEclMapping.h>

//Analysis
#include <analysis/dataobjects/ECLTRGInformation.h>

using namespace Belle2;
using namespace std;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLTRGInformation)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ECLTRGInformationModule::ECLTRGInformationModule() : Module()
{
  // Set module properties
  setDescription("Get ECL TRG energy information");
  setPropertyFlags(c_ParallelProcessingCertified);
}

void ECLTRGInformationModule::initialize()
{

  /** Required dataobjects */
  m_eclCalDigits.isRequired();
  m_trgUnpackerStore.isRequired();

  /** output dataobjects */
  m_eclTRGInformation.registerInDataStore();

  /** map to have direct access to ecl caldigits based on cellid */
  m_calDigitStoreArrPosition.resize(8737);

  /** ecl cell ids per TC */
  trgmap = new TrgEclMapping();

}

void ECLTRGInformationModule::event()
{
  // Fill a vector that can be used to map cellid -> store array position
  memset(&m_calDigitStoreArrPosition[0], -1, m_calDigitStoreArrPosition.size() * sizeof m_calDigitStoreArrPosition[0]);
  for (int i = 0; i < m_eclCalDigits.getEntries(); i++) {
    m_calDigitStoreArrPosition[m_eclCalDigits[i]->getCellId()] = i;
  }

  // create the dataobject
  if (!m_eclTRGInformation) {
    m_eclTRGInformation.create();
  }

  // get the actual TC information (energy, timing, ...)
  for (const auto& trg : m_trgUnpackerStore) {
    m_eclTRGInformation->setEnergyTC(trg.getTCId() - 1, trg.getTCEnergy());
    m_eclTRGInformation->setTimingTC(trg.getTCId() - 1, trg.getTCTime());
    m_eclTRGInformation->setRevoGDLTC(trg.getTCId() - 1, trg.getRevoGDL());
    m_eclTRGInformation->setRevoFAMTC(trg.getTCId() - 1, trg.getRevoFAM());
    B2DEBUG(10, "TC: " << trg.getTCId() << " E="  << trg.getTCEnergy() << ",  t=" << trg.getTCTime());
  }

  // loop over all possible TCs and fill the 'offline' ECLCalDigit information
  for (unsigned idx = 1; idx <= 576; idx++) {
    m_listOfXtalIds = trgmap->getXtalIdFromTCId(idx);
    float energySum = 0.;
    float highestEnergy = -1.;
    float time = std::numeric_limits<float>::quiet_NaN();

    for (const auto& id : m_listOfXtalIds) {
      // the mapping returns fixed size vectors with '0' to indicate empty positions
      if (id > 0) {
        const int pos = m_calDigitStoreArrPosition[id - 1];
        if (pos > 0) {
          energySum += m_eclCalDigits[pos]->getEnergy();

          if (m_eclCalDigits[pos]->getEnergy() > highestEnergy) {
            highestEnergy = m_eclCalDigits[pos]->getEnergy();
            time = m_eclCalDigits[pos]->getTime();
          }
        }
      }
    }

    m_eclTRGInformation->setEnergyTCECLCalDigit(idx - 1, energySum);
    m_eclTRGInformation->setTimingTCECLCalDigit(idx - 1, time);

    m_eclTRGInformation->setThetaIdTC(idx - 1, trgmap->getTCThetaIdFromTCId(idx));
    m_eclTRGInformation->setPhiIdTC(idx - 1, trgmap->getTCPhiIdFromTCId(idx));

    if (energySum > 0) B2DEBUG(10, "ECLCalDigits: " << idx << " Esum="  << energySum << ", t=" << time);

  }


}

void ECLTRGInformationModule::terminate()
{
  if (trgmap) delete trgmap;
}

