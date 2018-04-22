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
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventT0.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <genfit/Track.h>
#include <tracking/dataobjects/RecoTrack.h>
#include "TTree.h"
#include "TFile.h"

namespace Belle2 {

  namespace CDC {



    /**
     * Create QAM histograms.
     */
    void createQAMHist(TTree*);

    /**
     * Analysis module for CDC CR data.
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
      StoreObjPtr<EventT0> m_eventTimeStoreObject; /**<Event t0 */
      const Belle2::TrackFitResult* fitresult; /**< Track fit result. */
      std::string m_trackArrayName;           /**< Belle2::Track StoreArray name. */
      std::string m_recoTrackArrayName ;       /**< Belle2::RecoTrack StoreArray nam.e */
      std::string m_trackFitResultArrayName;  /**< Belle2::TrackFitResult StoreArray name. */
      std::string m_relRecoTrackTrackName;     /**< Releation between RecoTrack and Belle2:Track. */
      std::string m_OutputFileName;           /**< xt output file name. */
      std::string m_treeName;    /**< output tree name. */


      TFile* tfile;  /**< Output file. */
      TTree* tree;             /**< output tree, save info of each hit. */
      double ndf1;             /**< degree of freedom of 1st track. */
      double ndf2;             /**< degree of freedom of 2nd track. */
      double Pval1;            /**< P-value of 1st track. */
      double Pval2;            /**< P-value of 2nd track. */
      double D01;              /**< D0 of 1st track. */
      double D02;              /**< D0 of 2nd track. */
      double Phi01;            /**< Phi0 of 1st track. */
      double Phi02;            /**< Phi0 of 2nd track. */
      double Om1;              /**< Omega of 1st track. */
      double Om2;              /**< Omega of 2nd track. */
      double Z01;              /**< Z0 of 1st track. */
      double Z02;              /**< Z0 of 2nd track. */
      double tanLambda1;       /**< TanLambda of 1st track. */
      double tanLambda2;       /**< Tanlambda of 2nd track. */
      double eD01;             /**< error on D0 of 1st track. */
      double eD02;             /**< error on D0 of 2nd track. */
      double eOm1;             /**< error on Omega of 1st track. */
      double eOm2;             /**< error on Omega of 2nd track. */
      double ePhi01;           /**< error on Phi0 of 1st track. */
      double ePhi02;           /**< error on Phi0 of 2nd track. */
      double eZ01;             /**< error on Z0 of 1st track. */
      double eZ02;             /**< error on Z0 of 2nd track. */
      double etanL1;           /**< error on TanLambda of 1st track. */
      double etanL2;           /**< error on TanLambda of 2nd track. */
      double Pt1;              /**< Pt of 1st track. */
      double Pt2;              /**< Pt of 2nd track. */
      int m_fitstatus;         /**< fit status, 0 - nofit;1 - fit but not convergence;2-fit and convergence.*/
      TVector3 posSeed1;                /**< seed position of first track. */
      TVector3 posSeed2;                /**< seed position of second track. */

      TVector3 Mom1;                    /**< Momentum of 1st track. */
      TVector3 Mom2;                    /**< Momentum of 2nd track. */
      double Omega1;                    /**< omega of 1st track. */
      double Omega2;                    /**< omega of 2nd track. */
      double evtT0;                       /**< event t0. */
      short charge;                     /**< charge of track */

      bool m_noBFit; /**< fit incase no magnetic Field of not, if true, NDF=4 in cal P-value */
      bool m_EventT0Extraction;/**< run with event t0 extraction*/
      bool m_phi0InRad; /**< Unit of phi0, true: radian, false: degree */
      bool m_qam; /**< Enable output of QAM histogram */
    };
  }
}

