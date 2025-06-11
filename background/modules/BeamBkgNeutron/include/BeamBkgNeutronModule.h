/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

#include <string>

#include <Math/Vector3D.h>
#include <TFile.h>
#include <TTree.h>

namespace Belle2 {

  /**
   * BeamBkgNeutron module.
   */
  class BeamBkgNeutronModule : public Module {
  public:

    //! Constructor.
    BeamBkgNeutronModule();

    //! Destructor.
    virtual ~BeamBkgNeutronModule();

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

    std::string m_filename; /**< Output file name.   */

    std::vector<Float_t> trj_x; /**< X component of trajectory position */
    std::vector<Float_t> trj_y; /**< Y component of trajectory position */
    std::vector<Float_t> trj_z; /**< Z component of trajectory position */
    std::vector<Float_t> trj_px; /**< X component of trajectory momentum */
    std::vector<Float_t> trj_py; /**< Y component of trajectory momentum */
    std::vector<Float_t> trj_pz; /**< Z component of trajectory momentum */
    Float_t vtxProdX; /**< McParticle prod. vertex position X */
    Float_t vtxProdY; /**< McParticle prod. vertex position Y */
    Float_t vtxProdZ; /**< McParticle prod. vertex position Z */
    Float_t E_init;   /**< McParticle energy [GeV] */
    Float_t mass;   /**< McParticle mass [GeV] */
    Float_t lifeTime;   /**< McParticle lifetime [ns] */
    Int_t PDG; /**< PDG */
    Int subDet; /**< Subdetector */
    Int iden; /**< iden */
    Int trackID; /**< TrackID */
    Int iEvent; /**< Event identifier */
    Int iEntry; /**< Entry identifier */
    Int nSimHits[13]; /**< Array with number of SimHits */
    Int hitPDG[13]; /**< Array with PDG hits */
    Int momPDG[13]; /**< Array with PDG momentum */
    Float_t E_start; /**< Starting energy */
    Float_t E_end; /**< Ending energy */
    Float_t eDep; /**< Deposited energy */
    Float_t trackLength; /**< Track length */
    Float_t momentumX; /**< X component of momentum */
    Float_t momentumY; /**< Y component of momentum */
    Float_t momentumZ; /**< Z component of momentum */
    Float_t positionX; /**< X component of position */
    Float_t positionY; /**< Y component of position */
    Float_t positionZ; /**< Z component of position */
    Float_t nWeight; /**< Weight */

    TFile* ff;    /**< Output root file.   */
    TTree* tree1;   /**< Output tree */
    TTree* tree2;   /**< Output tree */
  };
} // Belle2 namespace
