/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef SVD_CLUSTEREVALUATION_H_
#define SVD_CLUSTEREVALUATION_H_

#include <framework/core/Module.h>
#include <vxd/geometry/GeoCache.h>
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
    TFile* m_rootFilePtr = nullptr; /**< pointer at root file used for storing histograms */
    TTree* m_tree = nullptr; /**<pointer at tree containing the parameters */
    TTree* m_treeSummary = nullptr; /**<pointer at tree containing the summary parameters */

    double m_UbinWidth; /**< histogram u-bin width in microns*/
    double m_VbinWidth;  /**< histogram v-bin width in microns*/

    //branches
    TBranch* b_experiment = nullptr; /**< experiment number*/
    TBranch* b_run = nullptr; /**< run number*/
    TBranch* b_ladder = nullptr; /**< ladder number*/
    TBranch* b_layer = nullptr; /**< layer number*/
    TBranch* b_sensor = nullptr; /**< sensor number*/
    TBranch* b_interU = nullptr; /**< intercept U position*/
    TBranch* b_interV = nullptr; /**< intercept V position*/
    TBranch* b_interErrU = nullptr; /**< intercept U position error*/
    TBranch* b_interErrV = nullptr; /**< intercept V position error*/
    TBranch* b_interUprime = nullptr; /**< intercept U prime*/
    TBranch* b_interVprime = nullptr; /**< intercept V prime*/
    TBranch* b_interErrUprime = nullptr; /**< intercept U prime error*/
    TBranch* b_interErrVprime = nullptr; /**< intercept V prime error*/
    TBranch* b_residU = nullptr; /**< U residual*/
    TBranch* b_residV = nullptr; /**< V residual*/
    TBranch* b_clUpos = nullptr; /**< cluster U position*/
    TBranch* b_clVpos = nullptr; /**< cluster V position*/
    TBranch* b_clUcharge = nullptr; /**< cluster U charge*/
    TBranch* b_clVcharge = nullptr; /**< cluster V charge*/
    TBranch* b_clUsnr = nullptr; /**< cluster U snr*/
    TBranch* b_clVsnr = nullptr; /**< cluster V snr*/
    TBranch* b_clUsize = nullptr; /**< cluster U size*/
    TBranch* b_clVsize = nullptr; /**< cluster V size*/
    TBranch* b_clUtime = nullptr; /**< cluster U time*/
    TBranch* b_clVtime = nullptr; /**< cluster V time*/

    //branch variables
    int m_experiment = -1; /**< experiment number*/
    int m_run = -1; /**< run number*/
    int m_ladder = -1; /**< ladder number*/
    int m_layer = -1; /**< layer number*/
    int m_sensor = -1; /**< sensor number*/
    float m_interU = -1; /**< intercept U position*/
    float m_interV = -1; /**< intercept V position*/
    float m_interErrU = -1; /**< intercept U position error*/
    float m_interErrV = -1; /**< intercept V position error*/
    float m_interUprime = -1; /**< intercept U prime*/
    float m_interVprime = -1; /**< intercept V prime*/
    float m_interErrUprime = -1; /**< intercept U prime error*/
    float m_interErrVprime = -1; /**< intercept V prime error*/
    float m_residU = -1; /**< U residual*/
    float m_residV = -1; /**< V residual*/
    float m_clUpos = -1; /**< cluster U position*/
    float m_clVpos = -1; /**< cluster V position*/
    float m_clUcharge = -1; /**< cluster U charge*/
    float m_clVcharge = -1; /**< cluster V charge*/
    float m_clUsnr = -1; /**< cluster U snr*/
    float m_clVsnr = -1; /**< cluster V snr*/
    int m_clUsize = -1; /**< cluster U size*/
    int m_clVsize = -1; /**< cluster V size*/
    float m_clUtime = -1; /**< cluster U time*/
    float m_clVtime = -1; /**< cluster V time*/

    //branches summary variables
    TBranch* bs_experiment = nullptr; /**< experiment number*/
    TBranch* bs_run = nullptr; /**< run number*/
    TBranch* bs_ladder = nullptr; /**< ladder number*/
    TBranch* bs_layer = nullptr; /**< layer number*/
    TBranch* bs_sensor = nullptr; /**< sensor number*/
    TBranch* bs_effU = nullptr; /**< efficiency U*/
    TBranch* bs_effV = nullptr; /**< efficiency V*/
    TBranch* bs_effErrU = nullptr; /**< efficiency error U*/
    TBranch* bs_effErrV = nullptr; /**< efficiency error V*/
    TBranch* bs_nIntercepts = nullptr; /**< number of intercepts*/
    TBranch* bs_residU = nullptr; /**< residual U*/
    TBranch* bs_residV = nullptr; /**< residual V*/
    TBranch* bs_misU = nullptr; /**< misalignment U*/
    TBranch* bs_misV = nullptr; /**< misalignment V*/
    TBranch* bs_statU = nullptr; /**< intercept stat error U*/
    TBranch* bs_statV = nullptr; /**< intercept stat error V*/

    //branch variables
    int ms_experiment = -1; /**< experiment number*/
    int ms_run = -1; /**< run number*/
    int ms_ladder = -1; /**< ladder number*/
    int ms_layer = -1; /**< layer number*/
    int ms_sensor = -1; /**< sensor number*/
    float ms_effU = -1; /**< efficiency U*/
    float ms_effV = -1; /**< efficiency V*/
    float ms_effErrU = -1; /**< efficiency error U*/
    float ms_effErrV = -1; /**< efficiency error V*/
    int ms_nIntercepts = -1; /**< number of intercepts*/
    float ms_residU = -1; /**< residual U*/
    float ms_residV = -1; /**< residual V*/
    float ms_misU = -1; /**< misalignment U*/
    float ms_misV = -1; /**< misalignment V*/
    float ms_statU = -1; /**< intercept stat error U*/
    float ms_statV = -1; /**< intercept stat error V*/

  private:

    double m_cmTomicron = 10000; /**<factor cm -> micron*/
    int m_theLayer = -1;  /**< layer under study */
    double m_interSigmaMax = -1;  /**< max of the histo of the intercept stat error */
    double m_uFiducial = -1;  /**< fiducial length u*/
    double m_vFiducial = -1;  /**< fiducial length v*/
    float m_nSigma = -1;  /**< number of sigmas for efficiency computation*/
    float m_halfWidth = -1;  /**< window half width for efficiency computation*/
    int m_groupNstrips = -1; /**< number of strip in the group in 2D resid vs position*/

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

    double getOneSigma(TH1F* h); /**<returns one sigma using quantiles */

    bool isRelatedToTrack(SVDIntercept* inter); /**< is the intercept related to a track */

  };
}

#endif /* SVDClusterEvaluationModule_H_ */

