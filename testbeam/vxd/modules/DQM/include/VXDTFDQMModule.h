#pragma once

#undef DQM
#ifndef DQM
#include <framework/core/HistoModule.h>
#else
#include <daq/dqm/modules/DqmHistoManagerModule.h>
#endif

// fw:
#include <vxd/dataobjects/VxdID.h>
#include <svd/geometry/SensorInfo.h>
#include <vxd/geometry/GeoCache.h>

// root:
// 1D:
#include "TH1F.h"
#include "TH1I.h"
#include "TH1D.h"
// 2D:
#include "TH2F.h"
#include "TH2I.h"
// #include "TH1D.h"
// others:
#include "TGraphAsymmErrors.h"

// Tobistuff:
#include "TH1.h"
#include "TH2.h"
#include "TProfile.h"


namespace Belle2 {

  /** VXDTF DQM Module */
  class VXDTFDQMModule : public HistoModule {  // <- derived from HistoModule class

  public:

    /** Number of SVD planes and their numbers.
     * The actual (layer, ladder, sensor numbers are (i,1,i), i = 3,4,5,6
     */
    enum {
      c_nSVDPlanes = 4,
      c_firstSVDPlane = 3,
      c_lastSVDPlane = 6,
    };

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


    /** returns sensorInfo for current index */
    inline const SVD::SensorInfo& getInfo(int index) const { // VXDTFDQMModule::getInfo
      int iPlane = indexToPlane(index);
      VxdID sensorID(iPlane, 1, iPlane);
      return dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(sensorID));
    }

  protected:
    // Allocate all histos through these, and the destructor will take
    // care of deleting them, they will be reset on a new run, maybe
    // other stuff. REMARK: devilishly stolen from Tobias Schlüter
    /** returns 1D histogram */
    TH1* getHist(const char* name, const char* title,
                 int nBins, double x0, double x1) {
      TH1* h = new TH1D(name, title, nBins, x0, x1);
//       m_allHistos.push_back(h);
      return h;
    }

    /** returns TProfile */
    TProfile* getHistProfile(const char* name, const char* title,
                             int nBins, double x0, double x1) {
      TProfile* h = new TProfile(name, title, nBins, x0, x1);
//       m_allHistos.push_back(h);
      return h;
    }

    /** returns 2D histogram */
    TH2* getHist(const char* name, const char* title,
                 int nBinsX, double x0, double x1,
                 int nBinsY, double y0, double y1) {
      TH2* h = new TH2D(name, title, nBinsX, x0, x1, nBinsY, y0, y1);
//       m_allHistos.push_back(h);
      return h;
    }

    /** returns 1D histogram */
    TH1* getHist(const std::string& name, const std::string& title,
                 int nBins, double x0, double x1) {
      return getHist(name.c_str(), title.c_str(), nBins, x0, x1);
    }

    /** returns TProfile */
    TProfile* getHistProfile(const std::string& name, const std::string& title,
                             int nBins, double x0, double x1) {
      return getHistProfile(name.c_str(), title.c_str(), nBins, x0, x1);
    }

    /** returns 2D histogram */
    TH2* getHist(const std::string& name, const std::string& title,
                 int nBinsX, double x0, double x1,
                 int nBinsY, double y0, double y1) {
      return getHist(name.c_str(), title.c_str(), nBinsX, x0, x1, nBinsY, y0, y1);
    }



    /** returns plane for current index */
    inline int indexToPlane(int index) const {
      return c_firstSVDPlane + index;
    }

    /** resetting counters */
    inline void resetCounters() {
      for (auto & entry : m_countTCHitsForEachSensor) {
        entry.second = 0;
      }

      for (auto & entry : m_countTotalHitsForEachSensor) {
        entry.second = 0;
      }
    }

    /** prepare map of 2DHistograms */
    void Prepare2DHistMap(std::string headLine, std::string title, std::map<uint, TH2F*>& thisMap);

    /** prepare map of 1DHistograms */
    void Prepare1DHistMap(std::string headLine, std::string title, std::map<uint, TH1F*>& thisMap, bool isU);


    std::string m_storeTrackCandsName;      /**< genfit::trackCand StoreArray name */
    std::string m_storeSvdClustersName;    /**< SVDClusters StoreArray name */
    std::string m_storePxdClustersName;    /**< PXDClusters StoreArray name */
    std::string m_storeTelClustersName;    /**< TELClusters StoreArray name */
    std::string m_histogramDirectoryName;   /**< subdir where to place the histograms */
    std::string m_storeReferenceTCsColName;      /**< genfit::trackCand reference TCs StoreArray name */

//     std::vector<TH1*> m_allHistos;          /**< All histograms for easy deletion */

    TH1F* m_histoMomentum;          /**< Total momentum estimated */
    TH1F* m_histoPT;                /**< Transverse momentum estimated */
    TH1F* m_histoMomentumX;         /**< Momentum in X estimated */
    TH1F* m_histoMomentumY;         /**< Momentum in Y  estimated */
    TH1F* m_histoMomentumZ;         /**< Momentum in Z  estimated */
    TH1I* m_histoNumHitsUsed;       /**< Num of hits used for TC */
    TH1I* m_histoNumHitsIgnored;    /**< Total num of Hits - num of hits used for TC */
    TH1I* m_histoNumTCsPerEvent;    /**< Total num of TCs per event */
    TH1D* m_histoHitsIgnoredRatio;    /**< Num of hits used for TC / Total num of Hits */

    TH2F* m_correlationHitSignalStrength[c_nSVDPlanes]; /**< Correlation in signal strength/energy deposit per cluster. Total vs used for TCs - info for each layer independently */
    TH2I* m_correlationNHitUsed[c_nSVDPlanes]; /**< Correlation in number of Hits used at a layer vs total number of hits. Total vs used for TCs - info for each layer independently */
    TH2F* m_correlationHitPositionU[c_nSVDPlanes]; /**< Correlation of position (U) of hit at a layer vs total position distribution. Total vs used for TCs - info for each layer independently */
    TH2F* m_correlationHitPositionV[c_nSVDPlanes]; /**< Correlation of position (V) of hit at a layer vs total position distribution. Total vs used for TCs - info for each layer independently */

    TH1D* m_histHitPosFoundU; /**< Position (U) of hit at layer 5 of all TCs produced */
    TH1D* m_histHitPosTotalU; /**< Position (U) of hit at layer 5 of all hits produced  */
    TGraphAsymmErrors* m_hitPosUsedVshitPosTotalU; /**< Position (U) of hit at a layer vs total position distribution. Total vs used for TCs, Layer5 */

    TH1I* m_histoNHitsAtPXD;         /**< Counted number of hits at the PXD per event added to the TCs  */
    TH1I* m_histoNHitsAtSVD;         /**< Counted number of hits at the SVD per event added to the TCs */
    TH1I* m_histoNHitsAtTEL;         /**< Counted number of hits at the TEL per event added to the TCs */
    TH1I* m_histoNHitsAtLayer0;      /**< Counted number of hits at layer 0 per event added to the TCs */
    TH1I* m_histoNHitsAtLayer1;      /**< Counted number of hits at layer 1 per event added to the TCs */
    TH1I* m_histoNHitsAtLayer2;      /**< Counted number of hits at layer 2 per event added to the TCs */
    TH1I* m_histoNHitsAtLayer3;      /**< Counted number of hits at layer 3 per event added to the TCs */
    TH1I* m_histoNHitsAtLayer4;      /**< Counted number of hits at layer 4 per event added to the TCs */
    TH1I* m_histoNHitsAtLayer5;      /**< Counted number of hits at layer 5 per event added to the TCs */
    TH1I* m_histoNHitsAtLayer6;      /**< Counted number of hits at layer 6 per event added to the TCs */
    TH1I* m_histoNHitsAtLayer7;      /**< Counted number of hits at layer 7 per event added to the TCs */

    std::map<uint, TH1F*> m_histoTCvsTotalHitsForEachSensor; /**< for each sensor which occured, total number of hits added to TCs divided by total number of hits, is 0 if there were no TCs-hits, -1 (by definition) if there was no hit at all */

    std::map<uint, uint> m_countTCHitsForEachSensor; /**< counts the hits per sensor of current event which were found by the TF */
    std::map<uint, uint> m_countTotalHitsForEachSensor; /**< counts the hits per sensor of current event */

    int m_countNoSensorFound; /**< counts cases when sensorID was not part of histogram-map */
    int m_countAllHits; /**< counts all hits */
    std::vector<uint> m_badSensorIDs; /**< collects all sensorIDs which weren't found */

    std::vector<uint> m_sensorIDs; /**< collects all sensorIDs */
    std::map<uint, TH2F*> m_hitMapHitsTotal; /**< Hit maps for each sensor, local uv coordinates - all hits which were there */
    std::map<uint, TH2F*> m_hitMapTCsOnly; /**< Hit maps for each sensor, local uv coordinates - all hits which were found by the TF */
    std::map<uint, TH2F*> m_hitMapReferenceTCs; /**< Hit maps for each sensor, local uv coordinates - all hits which were found by a reference TF */

    std::map<uint, TH1F*> m_hitMapHitsTotalSVDOnlyU; /**< Hit maps for each sensor, local uv coordinates - all hits which were there - SVDOnly */
    std::map<uint, TH1F*> m_hitMapTCsOnlySVDOnlyU; /**< Hit maps for each sensor, local uv coordinates - all hits which were found by the TF - SVDOnly */
    std::map<uint, TH1F*> m_hitMapReferenceTCsSVDOnlyU; /**< Hit maps for each sensor, local uv coordinates - all hits which were found by a reference TF - SVDOnly */
    std::map<uint, TH1F*> m_hitMapHitsTotalSVDOnlyV; /**< Hit maps for each sensor, local uv coordinates - all hits which were there - SVDOnly */
    std::map<uint, TH1F*> m_hitMapTCsOnlySVDOnlyV; /**< Hit maps for each sensor, local uv coordinates - all hits which were found by the TF - SVDOnly */
    std::map<uint, TH1F*> m_hitMapReferenceTCsSVDOnlyV; /**< Hit maps for each sensor, local uv coordinates - all hits which were found by a reference TF - SVDOnly */
  };

}
