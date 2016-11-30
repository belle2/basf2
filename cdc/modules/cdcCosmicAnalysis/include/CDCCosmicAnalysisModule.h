/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Dong Van Thanh                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <string>

#include <framework/datastore/RelationIndex.h>
#include <framework/datastore/RelationVector.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <genfit/Track.h>
#include <tracking/dataobjects/RecoTrack.h>
#include "TTree.h"
#include "TFile.h"

namespace Belle2 {

  namespace CDC {

    /**
     * Analysis data of CDC-TOP test two tracks case.
     */

    class CDCCosmicAnalysisModule : public Module {

    public:
      /**
       * Constructor.
       */
      CDCCosmicAnalysisModule();

      /**
       * Destructor.
       */
      virtual ~CDCCosmicAnalysisModule();


      /**
       * Initializes the Module.
       */
      virtual void initialize();

      /**
       * Begin run action.
       */
      virtual void beginRun();
      /**
       * Event action (main routine).
       */
      virtual void event();
      /**
       * End run action.
       */
      virtual void endRun();
      /**
       * Termination action.
       */
      virtual void terminate();


    private:
      const Belle2::TrackFitResult* fitresult; /**< Track fit result. */
      std::string m_trackArrayName;           /**< Belle2::Track StoreArray name. */
      std::string m_recoTrackArrayName ;       /**< Belle2::RecoTrack StoreArray nam.e */
      std::string m_trackFitResultArrayName;  /**< Belle2::TrackFitResult StoreArray name. */
      std::string m_relRecoTrackTrackName;     /**< Releation between RecoTrack and Belle2:Track. */
      std::string m_OutputFileName;           /**< xt output file name. */


      TFile* tfile;  /**< Output file. */
      TTree* tree;                          /**< output tree, save info of each hit. */
      double ndf1;                    /**< degree of freedom of first track. */
      double ndf2;                    /**< degree of freedom of secon track. */
      double Pval1;                   /**< Pvalue of 1st track. */
      double Pval2;                    /**< Pvalue of 2nd track. */
      double Phi01;                    /**< Phi0 of 1st track. */
      double Phi02;                    /**< Phi0 of 2nd track. */
      double  tanLambda1;              /**< TanLambda of 1st track. */
      double  tanLambda2;              /**< Tanlambda of 2nd track. */
      double D01;                        /**< D0 of 1st track. */
      double D02;                        /**< D0 of 2nd track. */
      double Z01;                        /**< Z0 of 1st track. */
      double Z02;                        /**< Z0 of 2nd track. */
      int m_fitstatus;                  /**< fit status, 0 - nofit;1 - fit but not convergence;2-fit and convergence.*/
      TVector3 posSeed1;                /**< seed position of first track. */
      TVector3 posSeed2;                /**< seed position of second track. */
      TVector3 trigHitPos; /**< Trigger position. */
      bool m_noBFit; /**< fit incase no magnetic Field of not, if true, NDF=4 in cal P-value */

    };
  }
}

