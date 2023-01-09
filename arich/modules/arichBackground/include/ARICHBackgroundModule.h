/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/database/DBObjPtr.h>

#include <arich/dbobjects/ARICHGeometryConfig.h>

#include <string>

#include <TFile.h>
#include <TTree.h>

namespace Belle2 {
  class MCParticle;
  class ARICHSimHit;
  class BeamBackHit;

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

      std::string m_filename;          /**< Output file name.   */
      int m_bkgTag;           /**< background source.  */

      TVector3 m_phpos;                  /**< hit position */
      TVector3 m_phmom;                  /**< hit momentum */
      TVector3 m_phVtx;                  /**< hit particle vertex position */
      TVector3 m_phMmom;                 /**< hit particle mother momentum */
      TVector3 m_phMvtx;                 /**< hit particle mother vertex */
      TVector3 m_phPvtx;                 /**< primary particle vertex */
      TVector3 m_phPmom;                 /**< primary particle momentum */
      TVector3 m_phGMvtx;                /**< hit particle grand mother vertex */
      TVector3 m_phGMmom;                /**< hit particle grand mother momentum */
      TVector3 m_modOrig;                /**< HAPD module position */
      int m_source;                      /**< hit source (RBB_HER, ...) */
      int m_phPDG;                       /**< hit particle PDG code */
      int m_phMPDG;                      /**< hit particle mother PDG code */
      int m_phPPDG;                      /**< hit particle primary PDG code */
      int m_phGMPDG;                     /**< hit particle grand mother PDG code */
      int m_type;                        /**< hit particle type; 0 hit in board, 1 hit in HAPD bottom, 2 photon hit */
      double m_edep;                     /**< hit deposited energy */
      double m_ttime;                    /**< hit global time */
      int m_moduleID;                    /**< hit module ID */
      double m_phnw;                     /**< neutron 1MeV equiv. weight */
      double m_trackLength;              /**< particle track lenght in hit volume */
      double m_energy;                 /**< energy of particle */
      TFile* m_outputFile;               /**< Output root file.   */
      TTree* m_outputTree;               /**< Output tree */
      DBObjPtr<ARICHGeometryConfig> m_arichgp; /**< Geometry parametrization */

      StoreArray<MCParticle>  m_MCParticles;  /**< StoreArray for MCParticles*/
      StoreArray<ARICHSimHit> m_ARICHSimHits; /**< StoreArray for ARICHSimHits*/
      StoreArray<BeamBackHit> m_BeamBackHits; /**< StoreArray for BeamBackHits*/

    };

  } // arich namespace
} // Belle2 namespace