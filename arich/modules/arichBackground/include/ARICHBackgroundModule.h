/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ARICHBACKGROUND_H
#define ARICHBACKGROUND_H

#include <framework/core/Module.h>
#include <arich/geometry/ARICHGeometryPar.h>

#include <string>

#include <TRandom3.h>
#include <TFile.h>
#include <TH2F.h>
#include <TTree.h>

namespace Belle2 {
  namespace arich {

    //! ARICH digitizer module.
    /*!
      This module extracts information relevant for arich background estimation
      from BeamBackHits and produces TTree files, then used to generate plots.
    */
    class ARICHBackgroundModule : public Module {

    public:


      //! Constructor.
      ARICHBackgroundModule();

      //! Destructor.
      virtual ~ARICHBackgroundModule();

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
       * Convert reads information from BeamBackHits and writes tree file.
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

    private:

      TFile* ff;                     /**< Output root file.   */

      std::string m_filename;          /**< Output file name.   */
      std::string m_bkgType;           /**< background source.  */

      ARICHGeometryPar* m_arichgp;     /**< Geometry parametrization */
      TTree* TrHits;                   /**< Output tree */
      TVector3 phpos;                  /**< hit position */
      TVector3 phmom;                  /**< hit momentum */
      TVector3 phVtx;                  /**< hit particle vertex position */
      TVector3 phMmom;                 /**< hit particle mother momentum */
      TVector3 phMvtx;                 /**< hit particle mother vertex */
      TVector3 phPvtx;                 /**< primary particle vertex */
      TVector3 phPmom;                 /**< primary particle momentum */
      TVector3 phGMvtx;                /**< hit particle grand mother vertex */
      TVector3 phGMmom;                /**< hit particle grand mother momentum */
      TVector3 modOrig;                /**< HAPD module position */
      int phPDG;                       /**< hit particle PDG code */
      int phMPDG;                      /**< hit particle mother PDG code */
      int phPPDG;                      /**< hit particle primary PDG code */
      int phGMPDG;                     /**< hit particle grand mother PDG code */
      int type;                        /**< hit particle type; 0 hit in board, 1 hit in HAPD bottom, 2 photon hit */
      int moduleID;                    /**< hit module ID */
      int source;                      /**< hit source (RBB_HER, ...) */
      double edep;                     /**< hit deposited energy */
      double ttime;                    /**< hit global time */
      double phnw;                     /**< neutron 1MeV equiv. weight */
      double en;                       /**< energy of particle */
      double trlen;                    /**< particle track lenght in hit volume */

    };

  } // arich namespace
} // Belle2 namespace

#endif // ARICHDIGIMODULE_H
