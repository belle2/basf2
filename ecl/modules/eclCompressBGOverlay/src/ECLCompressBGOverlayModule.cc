/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: Alexei Sibidanov                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

//This module
#include <ecl/modules/eclCompressBGOverlay/ECLCompressBGOverlayModule.h>

//ECL
#include <ecl/dataobjects/ECLDsp.h>
#include <ecl/dataobjects/ECLWaveforms.h>
#include <ecl/digitization/EclConfiguration.h>

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
  addParam("CompressionAlgorithm", m_compAlgo, "Waveform compression algorithm", 0u);
  addParam("eclWaveformsName", m_eclWaveformsName, "Name of the collection with compressed waveforms", std::string(""));
}

ECLCompressBGOverlayModule::~ECLCompressBGOverlayModule()
{
}

void ECLCompressBGOverlayModule::initialize()
{
  m_eclDsps.registerInDataStore();
  m_eclWaveforms.registerInDataStore(m_eclWaveformsName);
  m_comp = selectAlgo(m_compAlgo & 0xff);
  if (m_comp == NULL)
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
  BitStream out(ec.m_nch * ec.m_nsmp * 18 / 32);
  out.putNBits(m_compAlgo & 0xff, 8);
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
