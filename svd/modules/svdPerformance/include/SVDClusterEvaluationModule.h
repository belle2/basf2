/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Giulia Casarosa                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVD_CLUSTEREVALUATION_H_
#define SVD_CLUSTEREVALUATION_H_

#include <framework/core/Module.h>
#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <vxd/dataobjects/VxdID.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <svd/dataobjects/SVDCluster.h>
#include <svd/dataobjects/SVDHistograms.h>
#include <tracking/dataobjects/SVDIntercept.h>
#include <mdst/dataobjects/Track.h>
#include <framework/dataobjects/EventMetaData.h>


#include <string>
#include <TTree.h>
#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TDirectory.h>
#include <TCollection.h>
#include <TList.h>
#include <TH3F.h>

// forward declarations
class TTree;
class TFile;

namespace Belle2 {

  /** The SVD ClusterEvaluation Module
   *
   */

  class SVDClusterEvaluationModule : public Module {

  public:

    SVDClusterEvaluationModule();

    virtual ~SVDClusterEvaluationModule();
    virtual void initialize();
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    /* user-defined parameters */
    std::string m_rootFileName;   /**< root file name */
    std::string m_ClusterName;   /**< SVDCluster StoreArray name */
    std::string m_InterceptName;   /**< SVDIntercept StoreArray name */
    std::string m_TrackName;   /**< Track StoreArray name */

    /* ROOT file related parameters */
    TFile* m_rootFilePtr; /**< pointer at root file used for storing histograms */

    double m_UbinWidth;
    double m_VbinWidth;

  private:

    double m_cmTomicron = 10000;
    int m_theLayer;
    double m_interSigmaMax;

    VXD::GeoCache& m_geoCache = VXD::GeoCache::getInstance(); /**< the geo cache instance*/

    StoreObjPtr<EventMetaData> m_eventMetaData;
    StoreArray<SVDCluster> m_svdClusters;
    StoreArray<Track> m_tracks;
    StoreArray<SVDIntercept> m_svdIntercepts;

    SVDHistograms<TH2F>* m_interCoor;
    SVDHistograms<TH1F>* m_interSigma;
    SVDHistograms<TH1F>* m_clsCoor;
    SVDHistograms<TH1F>* m_clsResid;

    float m_width_LargeS_U;
    float m_width_LargeS_V;
    float m_width_SmallS_U;
    float m_width_SmallS_V;

    float m_safety_margin;

    int m_nBins_LargeS_U;
    int m_nBins_LargeS_V;
    int m_nBins_SmallS_U;
    int m_nBins_SmallS_V;

    float m_abs_LargeS_U;
    float m_abs_LargeS_V;
    float m_abs_SmallS_U;
    float m_abs_SmallS_V;

    void create_SVDHistograms_interCoor();
    void create_SVDHistograms_interSigma();
    void create_SVDHistograms_clsCoor();
    void create_SVDHistograms_clsResid();

    bool isRelatedToTrack(SVDIntercept* inter);

    bool fitResiduals(TH1F* res);
  };
}

#endif /* SVDClusterEvaluationModule_H_ */

