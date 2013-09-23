/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa, Eugenio Paoloni                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
/* Additional Info:
* This Module is in an early stage of developement. The comments are mainly for temporal purposes
* and will be changed and corrected in later stages of developement. So please ignore them.
*/

#ifndef PXD_DATA_REDUCTION_ANALYSIS_MODULE_H_
#define PXD_DATA_REDUCTION_ANALYSIS_MODULE_H_

#include <framework/core/Module.h>
#include <string>
#include <TTree.h>
#include <TFile.h>

namespace Belle2 {

  /** The PXD Data Reduction Analysis Module
   *
   * this module performs the analysis of the PXD data redution module performances
   *
   */

  class PXDDataRedAnalysisModule : public Module {

  public:

    /**
     * Constructor of the module.
     */
    PXDDataRedAnalysisModule();

    /**
     * Destructor of the module.
     */
    virtual ~PXDDataRedAnalysisModule();

    /**
     *Initializes the Module.
     */
    virtual void initialize();

    virtual void beginRun();

    virtual void event();

    virtual void endRun();

    /**
     * Termination action.
     */
    virtual void terminate();

  private:

    std::string m_gfTrackCandsColName; /**< GFTrackCand list name*/
    std::string m_PXDInterceptListName; /**< PXDIntercept list name*/
    std::string m_ROIListName; /**< ROIid list name*/

    TFile* m_rootFilePtr; /**< pointer at root file used for storing infos for debugging and validating purposes */
    std::string m_rootFileName; /**< root file name*/
    bool m_writeToRoot; /**< if true, a rootFile named by m_rootFileName will be filled with info */

    //noHit ROOT TREE
    TTree* m_rootNoHitTreePtr; /**< pointer at root tree used for information stored once per interception */
    double m_rootNoHitCoorU; /**< U coor for intercepts in case of missing ROI*/
    double m_rootNoHitCoorV; /**< V coor for intercepts in case of missing ROI*/
    double m_rootNoHitSigmaU; /**< U stat error for intercepts in case of missing ROI*/
    double m_rootNoHitSigmaV; /**< V stat error for intercepts in case of missing ROI*/
    double m_rootNoHitLambda; /**< track length for intercepts in case of missing ROI*/
    int m_rootNoHitVxdID; /**< sensor ID for intercepts in case of missing ROI*/
    double m_rootNoHitCoorUmc; /**< true U coor of hit in case of missing ROI*/
    double m_rootNoHitCoorVmc; /**< true V coor of hit in case of missing ROI*/
    double m_rootNoHitUidmc; /**< true U id of hit in case of missing ROI*/
    double m_rootNoHitVidmc; /**< true V id of hit in case of missing ROI*/
    int m_rootNoHitVxdIDmc; /**< true sensorID of hit in case of missing ROI*/

    double m_rootGlobalTime; /**< global time of hit*/
    int m_rootNGlobalTime; /**< number of hits per intercept*/


    //hit ROOT TREE
    TTree* m_rootHitTreePtr; /**< pointer at root tree used for information stored once per interception */
    double m_rootHitCoorU; /**< U coor of a intercept relative to a PXDDigit contained in a ROI*/
    double m_rootHitCoorV; /**< V coor of a intercept relative to a PXDDigit contained in a ROI*/
    double m_rootHitSigmaU; /**< U stat error of a intercept relative to a PXDDigit contained in a ROI*/
    double m_rootHitSigmaV; /**< V stat error of a intercept relative to a PXDDigit contained in a ROI*/
    double m_rootHitLambda; /**< track length for a intercept relative to a PXDDigit contained in a ROI*/
    int m_rootHitVxdID; /**< sensorID of a intercept relative to a PXDDigit contained in a ROI*/

    //noInter ROOT TREE
    TTree* m_rootNoInterTreePtr; /**< pointer at root tree used for information stored once per interception */
    double m_rootNoInterUcoor; /**< U coor in case of no intercept found*/
    double m_rootNoInterVcoor; /**< V coor in case of no intercept found*/
    double m_rootNoInterUid; /**< U id in case of no intercept found*/
    double m_rootNoInterVid; /**< V id in case of no intercept found*/
    double m_rootNoInterVxdID; /**< sensor ID in case of no intercept found*/

    //mc truth infos
    double m_rootMomXmc; /**< X momentum from MC truth*/
    double m_rootMomYmc; /**< Y momentum from MC truth*/
    double m_rootMomZmc; /**< Z momentum from MC truth*/
    double m_rootCoorUmc; /**< U coordinate of the hit from MC truth*/
    double m_rootCoorVmc; /**< V coordinate of the hit from MC truth*/
    double m_rootUidmc; /**< U id of the hit from MC truth*/
    double m_rootVidmc; /**< V id of the hit from MC truth*/
    int m_rootVxdIDmc; /**< sensor ID of the hit from MC truth*/

    //ROI infos
    int m_rootROIminUid; /**< definition of ROI*/
    int m_rootROImaxUid; /**< definition of ROI*/
    int m_rootROIminVid; /**< definition of ROI*/
    int m_rootROImaxVid; /**< definition of ROI*/
    int m_rootROIVxdID; /**< definition of ROI*/

    int m_rootEvent;   /**<  event number*/

    unsigned int n_tracks; /**<total number of GFTrackCand*/
    unsigned int n_pxdDigit; /**< total number of PXDDigit*/
    unsigned int n_pxdDigitInROI; /**<number of PXDDigits contained in a ROI*/
    unsigned int n_noIntercept; /**<number of PXDDigits for which no intercept was found*/
    unsigned int n_noHit; /**< number of mixxing mising hits*/



  };

}

#endif
