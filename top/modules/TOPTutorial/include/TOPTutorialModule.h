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
#include <arich/modules/arichReconstruction/ARICHTrack.h>

namespace Belle2 {
  namespace top {

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
       *Returns object that host the TOP Likelihoods
       */
      const TOPLikelihoods* getTOPLikelihoods(const MCParticle* particle);

      const ARICHTrack* getARICHTrack(const MCParticle* particle);

      const MCParticle* getMCParticle(const GFTrack* particle);

      double TOP_ARICH_PID(int hyp1, int hyp2, const GFTrack* particle);

      void makePi(std::vector<TLorentzVector>& pip, std::vector<TLorentzVector>& pim, double cut = 0);

      void makeK(std::vector<TLorentzVector>& kp, std::vector<TLorentzVector>& km, double cut = 0);

      void combination(std::vector<TLorentzVector>& mother, std::vector<TLorentzVector>& child1, std::vector<TLorentzVector>& child2);

    protected:
      std::string  m_mcParticlesColName;         /**< MCParticles collection name. */
      std::string  m_gfTracksColName;         /**< MCParticles collection name. */
      std::string  m_dataOutFileName;
    private:
      TTree* m_tree;
      TFile* m_rootFile;
      double m_m_bc;
      double m_deltae;
      double m_timeCPU;                /**< CPU time.     */
      int    m_nRun;                   /**< Run number.   */
      int    m_nEvent;                 /**< Event number. */


    };

  } // top namespace
} // Belle2 namespace

#endif // TOPDIGIMODULE_H
