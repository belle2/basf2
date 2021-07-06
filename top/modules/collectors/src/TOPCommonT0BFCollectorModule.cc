/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/modules/collectors/TOPCommonT0BFCollectorModule.h>
#include <top/geometry/TOPGeometryPar.h>

// framework aux
#include <TH1F.h>

using namespace std;

namespace Belle2 {

  using namespace TOP;

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(TOPCommonT0BFCollector)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPCommonT0BFCollectorModule::TOPCommonT0BFCollectorModule()
  {
    // set module description and processing properties
    setDescription("A collector for common T0 calibration with a fit of bunch finder residuals (method BF)");
    setPropertyFlags(c_ParallelProcessingCertified);

    // module parameters
    addParam("bunchesPerSSTclk", m_bunchesPerSSTclk,
             "number of bunches per SST clock period", 24);
    addParam("nx", m_nx, "number of histogram bins", 200);

  }


  void TOPCommonT0BFCollectorModule::prepare()
  {

    m_recBunch.isRequired();

    const auto* geo = TOPGeometryPar::Instance()->getGeometry();
    m_bunchTimeSep = geo->getNominalTDC().getSyncTimeBase() / m_bunchesPerSSTclk;

    auto h1a = new TH1F("offset_a", "current offset; offset [ns]",
                        m_nx, -m_bunchTimeSep / 2, m_bunchTimeSep / 2);
    registerObject<TH1F>("offset_a", h1a);

    auto h1b = new TH1F("offset_b", "current offset; offset [ns]",
                        m_nx, 0.0, m_bunchTimeSep);
    registerObject<TH1F>("offset_b", h1b);

  }


  void TOPCommonT0BFCollectorModule::collect()
  {

    if (not m_recBunch.isValid()) return;
    if (not m_recBunch->isReconstructed()) return;
    if (m_recBunch->getNumTracks() != 2) return;

    auto h1a = getObjectPtr<TH1F>("offset_a");
    auto h1b = getObjectPtr<TH1F>("offset_b");
    auto offset = m_recBunch->getCurrentOffset();
    if (m_commonT0->isCalibrated()) offset += m_commonT0->getT0();

    // wrap-around into [-1/2, 1/2] of bunch cycle
    auto a = offset - round(offset / m_bunchTimeSep) * m_bunchTimeSep;
    h1a->Fill(a);

    // wrap-around into [0, 1] of bunch cycle
    auto b = offset - round(offset / m_bunchTimeSep - 0.5) * m_bunchTimeSep;
    h1b->Fill(b);

  }

}
