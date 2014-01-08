#ifndef VXDTFDQMModule_H_
#define VXDTFDQMModule_H_

#undef DQM
#ifndef DQM
#include <framework/core/HistoModule.h>
#else
#include <daq/dqm/modules/DqmHistoManagerModule.h>
#endif

#include "TH1F.h"
#include "TH1I.h"

namespace Belle2 {

  /** VXDTF DQM Module */
  class VXDTFDQMModule : public HistoModule {  // <- derived from HistoModule class

  public:

    /** Constructor */
    VXDTFDQMModule();
    /* Destructor */
    virtual ~VXDTFDQMModule();

    /** Module functions */
    virtual void initialize();
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    /**
     * Histogram definitions such as TH1(), TH2(), TNtuple(), TTree().... are supposed
     * to be placed in this function.
    */
    virtual void defineHisto();

  private:

    std::string m_storeTrackCandsName;      /**< genfit::trackCand StoreArray name */
    std::string m_storeClustersName;    /**< SVDClusters StoreArray name */

    TH1F* m_histoMomentum;          /**< Total momentum estimated */
    TH1F* m_histoPT;                /**< Transverse momentum estimated */
    TH1F* m_histoMomentumX;         /**< Momentum in X estimated */
    TH1F* m_histoMomentumY;         /**< Momentum in Y  estimated */
    TH1F* m_histoMomentumZ;         /**< Momentum in Z  estimated */
    TH1I* m_histoNumHitsUsed;       /**< Num of hits used for TC */
    TH1I* m_histoNumHitsIgnored;    /**< Total num of Hits - num of hits used for TC */
    TH1I* m_histoNumTCsPerEvent;    /**< TTotal num of TCs per event */
  };

}
#endif