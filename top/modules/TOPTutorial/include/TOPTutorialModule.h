/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TOPTUTORIAL_H
#define TOPTUTORIAL_H

#include <framework/core/Module.h>

#include <string>

#include <TRandom3.h>
#include <TVector3.h>
#include <TFile.h>
#include <TH2F.h>
#include <TTree.h>
#include <vector>
#include <TLorentzVector.h>

#include <GFTrack.h>
#include <top/dataobjects/TOPTrack.h>
#include <top/dataobjects/TOPLikelihoods.h>
#include <generators/dataobjects/MCParticle.h>
#include <arich/dataobjects/ARICHLikelihoods.h>
#include <arich/dataobjects/ARICHAeroHit.h>

namespace Belle2 {
  namespace TOP {

    //! TOP Tutorial module.
    /*
     * A showcase module for analysis
     * Tutorial at B2GM spring 2012
     */

    class TOPTutorialModule : public Module {

    public:


      //! Constructor.
      TOPTutorialModule();

      //! Destructor.
      virtual ~TOPTutorialModule();

      /**
       * Initialize the Module.
       *
       * This method is called at the beginning of data processing.
       */
      virtual void initialize();

      /**
       * Called when entering a new run.
       *
       * Set run dependent things like run header parameters, alignment, etc.
       */
      virtual void beginRun();

      /**
       * Event processor.
       *
       * Convert TOPSimHits of the event to TOPHits.
       */
      virtual void event();

      /**
       * End-of-run action.
       *
       * Save run-related stuff, such as statistics.
       */
      virtual void endRun();

      /**
       * Termination action.
       *
       * Clean-up, close files, summarize statistics, etc.
       */
      virtual void terminate();

      /**
       *Prints module parameters.
       */
      void printModuleParams() const;

      /**
       *Returns object that host TOP Likelihoods
       */
      const TOPLikelihoods* getTOPLikelihoods(const MCParticle* particle);

      /**
       *Returns object that host ARICH Likelihoods
       */
      const ARICHLikelihoods* getARICHLikelihoods(const MCParticle* particle);

      /**
       *Get MCparticle associated with GFTrack
       */
      const MCParticle* getMCParticle(const GFTrack* particle);

      /**
       *Get combined PID
       */
      double TOP_ARICH_PID(int hyp1, int hyp2, const GFTrack* particle);

      /**
       *Make pion list
       */
      void makePi(std::vector<TLorentzVector>& pip, std::vector<TLorentzVector>& pim, double cut = 0);

      /**
       *Make kaon list
       */
      void makeK(std::vector<TLorentzVector>& kp, std::vector<TLorentzVector>& km, double cut = 0);

      /**
       *combination
       */
      void combination(std::vector<TLorentzVector>& mother, std::vector<TLorentzVector>& child1, std::vector<TLorentzVector>& child2);

    protected:
      std::string  m_mcParticlesColName;      /**< MCParticles collection name. */
      std::string  m_gfTracksColName;         /**< GFTrack collection name. */
      std::string  m_dataOutFileName;         /**< output file name */
    private:
      TTree* m_tree;                  /**< root tree */
      TFile* m_rootFile;              /**< root file */
      double m_m_bc;                  /**< Mbc */
      double m_deltae;                /**< deltaE */
      double m_timeCPU;                /**< CPU time.     */
      int    m_nRun;                   /**< Run number.   */
      int    m_nEvent;                 /**< Event number. */


    };

  } // TOP namespace
} // Belle2 namespace

#endif // TOPDIGIMODULE_H
