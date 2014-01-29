/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tobias Schlüter                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef TrackfitDQMModule_H_
#define TrackfitDQMModule_H_

#undef DQM
#ifndef DQM
#include <framework/core/HistoModule.h>
#else
#include <daq/dqm/modules/DqmHistoManagerModule.h>
#endif

#include <vxd/dataobjects/VxdID.h>
#include <svd/geometry/SensorInfo.h>
#include <vxd/geometry/GeoCache.h>
#include <vxd/dataobjects/VxdID.h>
#include <tracking/pxdDataReductionClasses/ROIDetPlane.h>

#include <string>

#include "TH1.h"
#include "TH2.h"
#include "TProfile.h"

namespace genfit {
  class Track;
}

namespace Belle2 {

  /** Trackfit DQM Module */
  class TrackfitDQMModule : public HistoModule {  // <- derived from HistoModule class

  public:

    /** Number of SVD planes and their numbers.
     * The actual (layer, ladder, sensor numbers are (i,1,i), i = 3,4,5,6
     */
    enum {
      c_nSVDPlanes = 4,
      c_firstSVDPlane = 3,
      c_lastSVDPlane = 6,

      c_nPXDPlanes = 1,
      c_firstPXDPlane = 2,
      c_lastPXDPlane = 2,

      c_nPlanes = c_nPXDPlanes + c_nSVDPlanes,
    };

    /** Constructor */
    TrackfitDQMModule();
    /* Destructor */
    virtual ~TrackfitDQMModule();

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

    /** Utility functions to convert indices to plane numbers and v.v.,
     * and to protect against range errors.
     */
    inline int indexToPlane(int index) const {
      return c_firstPXDPlane + index;
    }
    inline int planeToIndex(int iPlane) const {
      return iPlane - c_firstPXDPlane;
    }

    /** This is a shortcut to getting SVD::SensorInfo from the GeoCache.
     * @param index Index of the sensor (0,1,2,3), _not_ layer number!
     * @return SensorInfo object for the desired plane.
     */
    const VXD::SensorInfoBase& getInfo(int index) const;

    std::string m_storeTrackName;           /**< genfit::Track StoreArray name */
    std::string m_storeTrackCandName;      /**< genfit::TrackCand StoreArray name */
    std::string m_relTrackCandTrackName;    /**< Relation between the two */

    std::string m_histogramDirectoryName;   /**< subdir where to place the histograms */

    std::vector<TH1*> m_allHistos;          /**< All histograms for easy deletion */

    // Allocate all histos through these, and the destructor will take
    // care of deleting them, they will be reset on a new run, maybe
    // other stuff.
    TH1* getHist(const char* name, const char* title,
                 int nBins, double x0, double x1) {
      TH1* h = new TH1D(name, title, nBins, x0, x1);
      m_allHistos.push_back(h);
      return h;
    }
    TProfile* getHistProfile(const char* name, const char* title,
                             int nBins, double x0, double x1) {
      TProfile* h = new TProfile(name, title, nBins, x0, x1);
      m_allHistos.push_back(h);
      return h;
    }
    TH2* getHist(const char* name, const char* title,
                 int nBinsX, double x0, double x1,
                 int nBinsY, double y0, double y1) {
      TH2* h = new TH2D(name, title, nBinsX, x0, x1, nBinsY, y0, y1);
      m_allHistos.push_back(h);
      return h;
    }
    TH1* getHist(const std::string& name, const std::string& title,
                 int nBins, double x0, double x1) {
      return getHist(name.c_str(), title.c_str(), nBins, x0, x1);
    }
    TProfile* getHistProfile(const std::string& name, const std::string& title,
                             int nBins, double x0, double x1) {
      return getHistProfile(name.c_str(), title.c_str(), nBins, x0, x1);
    }
    TH2* getHist(const std::string& name, const std::string& title,
                 int nBinsX, double x0, double x1,
                 int nBinsY, double y0, double y1) {
      return getHist(name.c_str(), title.c_str(), nBinsX, x0, x1, nBinsY, y0, y1);
    }

    TH1* m_hNTracks;
    TH1* m_hNDF;
    TH1* m_hChi2;
    TH1* m_hPval;
    TH2* m_hNDFChi2;
    TH2* m_hNDFPval;

    /** Residuals in cm.  */
    TH1* m_hResidualU[c_nPlanes];
    TH1* m_hResidualV[c_nPlanes];
    TH2* m_hNDFResidualU[c_nPlanes];
    TH2* m_hNDFResidualV[c_nPlanes];

    TH2* m_hResidualUvsV[c_nPlanes];
    TH2* m_hResidualVvsU[c_nPlanes];
    TH2* m_hResidualUvsU[c_nPlanes];
    TH2* m_hResidualVvsV[c_nPlanes];

    /** Residuals normalized with tracking error.  */
    TH1* m_hNormalizedResidualU[c_nPlanes];
    TH1* m_hNormalizedResidualV[c_nPlanes];
    TH2* m_hNDFNormalizedResidualU[c_nPlanes];
    TH2* m_hNDFNormalizedResidualV[c_nPlanes];

    /** The pseudo efficiencies := number of total reco tracks hitting the
    plane (y = 0), number of reco tracks with associated hits in this
    plane (y = 2, because denominator double-counts).  */
    TH1* m_hPseudoEfficienciesU;
    TH1* m_hPseudoEfficienciesV;

    TH1* m_hMomentum;

    /** Correlation between seed and fitted track for five parameters in the first plane.  */
    TH2* m_hSeedQuality[6];

    void findPlanes();
    std::vector<ROIDetPlane> m_vPlanes;

    void plotResiduals(const genfit::Track* track);
    void plotPseudoEfficiencies(const genfit::Track* track);
    void plotSeedQuality(const genfit::Track* track);

    bool m_fillExpertHistos;
  };

}
#endif
