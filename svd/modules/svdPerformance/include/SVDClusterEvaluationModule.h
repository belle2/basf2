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
    virtual void initialize() override;
    virtual void beginRun() override;
    virtual void event() override;
    virtual void endRun() override;
    virtual void terminate() override;

    /* user-defined parameters */
    std::string m_rootFileName;   /**< root file name */
    std::string m_ClusterName;   /**< SVDCluster StoreArray name */
    std::string m_InterceptName;   /**< SVDIntercept StoreArray name */
    std::string m_TrackName;   /**< Track StoreArray name */

    /* ROOT file related parameters */
    TFile* m_rootFilePtr = NULL; /**< pointer at root file used for storing histograms */

    double m_UbinWidth; /**< histogram u-bin width in microns*/
    double m_VbinWidth;  /**< histogram v-bin width in microns*/

  private:

    double m_cmTomicron = 10000;
    int m_theLayer;  /**< layer under study */
    double m_interSigmaMax;  /**< max of the histo of the intercept stat error */
    double m_uFiducial;  /**< fiducial length u*/
    double m_vFiducial;  /**< fiducial length v*/
    float m_nSigma;  /**< number of sigmas for efficiency computation*/
    int m_groupNstrips; /**< number of strip in the group in 2D resid vs position*/

    VXD::GeoCache& m_geoCache = VXD::GeoCache::getInstance(); /**< the geo cache instance*/

    StoreObjPtr<EventMetaData> m_eventMetaData; /**< event meta data */
    StoreArray<SVDCluster> m_svdClusters; /**< clusters */
    StoreArray<Track> m_tracks; /**< tracks */
    StoreArray<SVDIntercept> m_svdIntercepts; /**< intercepts */

    SVDHistograms<TH2F>* m_interCoor; /**< intercept coordinates plots*/
    SVDHistograms<TH1F>* m_interSigma; /**< intercept stat error plots*/
    SVDHistograms<TH1F>* m_clsCoor; /**< cluster coordinates plots */
    SVDHistograms<TH1F>* m_clsResid; /**< cluster resid plots */
    SVDHistograms<TH1F>* m_clsMinResid; /**< cluster minimum resid plots */
    SVDHistograms<TH2F>* m_clsResid2D; /**< 2D resid plots*/

    float m_width_LargeS_U = 5.772; /**< width large sensor U*/
    float m_width_LargeS_V = 12.290; /**< width large sensor V*/
    float m_width_SmallS_U = 3.855; /**< width small sensor U*/
    float m_width_SmallS_V = m_width_LargeS_V;  /**< width small sensor V*/

    float m_safety_margin = 0.2; /**< safety margin */

    int m_nBins_LargeS_U = 100; /**< number of bins for large sensor U*/
    int m_nBins_LargeS_V = 100;  /**< number of bins for large sensor V */
    int m_nBins_SmallS_U = 100;  /**< number of bins for small sensor U*/
    int m_nBins_SmallS_V = 100;  /**< number of bins for small sensor V*/

    float m_abs_LargeS_U = 1; /**< half width including safety margin, large sensor U */
    float m_abs_LargeS_V = 1; /**< half width including safety margin, large sensor V */
    float m_abs_SmallS_U = 1; /**< half width including safety margin, small sensor U */
    float m_abs_SmallS_V = 1;  /**< half width including safety margin, small sensor V */

    void create_SVDHistograms_interCoor(); /**< create intercept coordinates plots */
    void create_SVDHistograms_interSigma(); /**< create intercept error plots */
    void create_SVDHistograms_clsCoor(); /**< create cluster coordinates plots */
    void create_SVDHistograms_clsResid(); /**< create slucter resid plots */

    bool isRelatedToTrack(SVDIntercept* inter); /**< is the intercept related to a track */

    bool fitResiduals(TH1F* res); /**< is the fit to residuals good? */
  };
}

#endif /* SVDClusterEvaluationModule_H_ */

