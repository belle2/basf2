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

    std::string m_gfTrackCandsColName;
    std::string m_PXDInterceptListName;
    std::string m_ROIListName;

    TFile* m_rootFilePtr; /**< pointer at root file used for storing infos for debugging and validating purposes */
    std::string m_rootFileName;
    bool m_writeToRoot; /**< if true, a rootFile named by m_rootFileName will be filled with info */

    //noHit ROOT TREE
    TTree* m_rootNoHitTreePtr; /**< pointer at root tree used for information stored once per interception */
    double m_rootNoHitCoorU;
    double m_rootNoHitCoorV;
    double m_rootNoHitSigmaU;
    double m_rootNoHitSigmaV;
    double m_rootNoHitLambda;
    int m_rootNoHitVxdID;
    double m_rootNoHitCoorUmc;
    double m_rootNoHitCoorVmc;
    double m_rootNoHitUidmc;
    double m_rootNoHitVidmc;
    int m_rootNoHitVxdIDmc;

    double m_rootGlobalTime;
    int m_rootNGlobalTime;


    //hit ROOT TREE
    TTree* m_rootHitTreePtr; /**< pointer at root tree used for information stored once per interception */
    double m_rootHitCoorU;
    double m_rootHitCoorV;
    double m_rootHitSigmaU;
    double m_rootHitSigmaV;
    double m_rootHitLambda;
    int m_rootHitVxdID;

    //noInter ROOT TREE
    TTree* m_rootNoInterTreePtr; /**< pointer at root tree used for information stored once per interception */
    double m_rootNoInterUcoor;
    double m_rootNoInterVcoor;
    double m_rootNoInterUid;
    double m_rootNoInterVid;
    double m_rootNoInterVxdID;

    //mc truth infos
    double m_rootMomXmc;
    double m_rootMomYmc;
    double m_rootMomZmc;
    double m_rootCoorUmc;
    double m_rootCoorVmc;
    double m_rootUidmc;
    double m_rootVidmc;
    int m_rootVxdIDmc;

    //ROI infos
    int m_rootROIminUid;
    int m_rootROImaxUid;
    int m_rootROIminVid;
    int m_rootROImaxVid;
    int m_rootROIVxdID;

    int m_rootEvent;   /**<  event number*/

    unsigned int n_tracks;
    unsigned int n_pxdDigit;
    unsigned int n_pxdDigitInROI;
    unsigned int n_noIntercept;
    unsigned int n_noHit;



  };

}

#endif
