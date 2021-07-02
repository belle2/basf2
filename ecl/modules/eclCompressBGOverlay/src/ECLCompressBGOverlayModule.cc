/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//This module
#include <ecl/modules/eclCompressBGOverlay/ECLCompressBGOverlayModule.h>

//ECL
#include <ecl/dataobjects/ECLDsp.h>
#include <ecl/dataobjects/ECLTrig.h>
#include <ecl/dataobjects/ECLWaveforms.h>
#include <ecl/digitization/EclConfiguration.h>
#include <ecl/dbobjects/ECLChannelMap.h>

using namespace Belle2;
using namespace ECL;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLCompressBGOverlay)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------
ECLCompressBGOverlayModule::ECLCompressBGOverlayModule() : Module()
{
  //Set module properties
  setDescription("Compress recorded waveforms for beam backround overlay in simulation.");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("CompressionAlgorithm", m_compAlgo, "Waveform compression algorithm (0..127)", 0u);
  addParam("WithTriggerTime", m_trgTime, "Trigger time from each create", 1u);
  addParam("eclWaveformsName", m_eclWaveformsName, "Name of the collection with compressed waveforms", std::string(""));
}

ECLCompressBGOverlayModule::~ECLCompressBGOverlayModule()
{
}

void ECLCompressBGOverlayModule::initialize()
{
  m_eclDsps.registerInDataStore();
  m_eclWaveforms.registerInDataStore(m_eclWaveformsName);
  m_comp = selectAlgo(m_compAlgo & 0x7f);
  if (m_comp == nullptr)
    B2FATAL("Unknown compression algorithm: " << m_compAlgo);
}

void ECLCompressBGOverlayModule::event()
{
  setReturnValue(0);
  const EclConfiguration& ec = EclConfiguration::get();

  // check the number of waveforms
  if (m_eclDsps.getEntries() != ec.m_nch) return;

  // sort by cell id
  unsigned short indx[ec.m_nch], count = 0;
  memset(indx, 0xff, sizeof(indx));
  for (const auto& t : m_eclDsps) indx[t.getCellId() - 1] = count++;

  // check that all crystals are here
  for (auto t : indx) if (t >= ec.m_nch) return;

  int FitA[ec.m_nsmp]; // buffer for a waveform

  // compress waveforms
  BitStream out(ec.m_nch * ec.m_nsmp);
  out.putNBits((m_compAlgo & 0x7f) | (m_trgTime << 7), 8);
  if (m_trgTime) {
    if (!m_eclTrigs.isValid()) B2WARNING("Crate trigger times are asked to be saved but they are not provided.");
    // check the number of trigger times
    if (m_eclTrigs.getEntries() != ECL::ECL_CRATES) return;
    unsigned char ttime[ECL::ECL_CRATES];
    for (const auto& t : m_eclTrigs) {
      int id = t.getTrigId(), time = t.getTimeTrig();
      // unpack trigger time
      ttime[id - 1] = (time - 2 * (time / 8)) / 2; // 0<=ttime<72
    }
    for (int i = 0; i < ECL::ECL_CRATES; i++) out.putNBits(ttime[i], 7);
  }
  for (int j = 0; j < ec.m_nch; j++) {
    m_eclDsps[indx[j]]->getDspA(FitA);
    m_comp->compress(out, FitA);
  }
  out.resize();

  ECLWaveforms* wf = new ECLWaveforms;
  m_eclWaveforms.assign(wf);

  std::swap(out.getStore(), wf->getStore());
  setReturnValue(1);
}

void ECLCompressBGOverlayModule::terminate()
{
  if (m_comp) delete m_comp;
}
