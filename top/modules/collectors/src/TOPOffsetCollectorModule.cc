/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/modules/collectors/TOPOffsetCollectorModule.h>
#include <framework/logging/Logger.h>
#include <TH1F.h>


using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  ///                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(TOPOffsetCollector);

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPOffsetCollectorModule::TOPOffsetCollectorModule()
  {
    // set module description and processing properties
    setDescription("A collector for eventT0 and fill pattern offset calibrations");
    setPropertyFlags(c_ParallelProcessingCertified);

  }


  void TOPOffsetCollectorModule::prepare()
  {
    m_recBunch.isRequired();
    m_eventT0.isRequired();

    m_names[Const::SVD] = "svdOffset";
    m_names[Const::CDC] = "cdcOffset";
    for (const auto& x : m_names) {
      registerObject<TH1F>(x.second, new TH1F(x.second.c_str(), "Event T0 difference w.r.t TOP; #Delta T_{0} [ns]",
                                              500, -50, 50));
    }

    int RFBuckets = m_bunchStructure->getRFBucketsPerRevolution();
    registerObject<TH1F>("fillPattern", new TH1F("fillPattern", "Fill pattern from DB; bucket number",
                                                 RFBuckets, 0, RFBuckets));
    registerObject<TH1F>("recBuckets", new TH1F("recBuckets", "Reconstructed buckets; bucket number",
                                                RFBuckets, 0, RFBuckets));
  }


  void TOPOffsetCollectorModule::collect()
  {
    if (m_firstEvent) {
      m_firstEvent = false;
      if (m_bunchStructure->isSet()) {
        auto h = getObjectPtr<TH1F>("fillPattern");
        int RFBuckets = m_bunchStructure->getRFBucketsPerRevolution();
        for (int i = 0; i < RFBuckets; i++) {
          if (m_bunchStructure->getBucket(i)) h->SetBinContent(i + 1, 1);
        }
      }
    }

    if (not m_recBunch->isReconstructed()) return;

    for (const auto& x : m_names) {
      const auto& detector = x.first;
      if (m_eventT0->hasTemporaryEventT0(detector)) {
        auto eventT0s = m_eventT0->getTemporaryEventT0s(detector);
        if (eventT0s.empty()) continue;
        if (detector == Const::CDC and eventT0s.back().algorithm != "chi2") continue;
        double t0 = eventT0s.back().eventT0;
        auto h = getObjectPtr<TH1F>(x.second);
        h->Fill(t0 - m_recBunch->getTime());
      }
    }

    auto h = getObjectPtr<TH1F>("recBuckets");
    h->Fill(m_recBunch->getBucketNumber(0, m_bunchStructure->getRFBucketsPerRevolution()));
  }

}
