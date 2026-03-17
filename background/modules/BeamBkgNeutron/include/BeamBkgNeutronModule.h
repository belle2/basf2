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

  private:

    std::string m_filename; /**< Output file name.   */

    std::vector<Float_t> m_trj_x; /**< X component of trajectory position */
    std::vector<Float_t> m_trj_y; /**< Y component of trajectory position */
    std::vector<Float_t> m_trj_z; /**< Z component of trajectory position */
    std::vector<Float_t> m_trj_px; /**< X component of trajectory momentum */
    std::vector<Float_t> m_trj_py; /**< Y component of trajectory momentum */
    std::vector<Float_t> m_trj_pz; /**< Z component of trajectory momentum */
    Float_t m_vtxProdX; /**< McParticle prod. vertex position X */
    Float_t m_vtxProdY; /**< McParticle prod. vertex position Y */
    Float_t m_vtxProdZ; /**< McParticle prod. vertex position Z */
    Float_t m_E_init;   /**< McParticle energy [GeV] */
    Float_t m_mass;   /**< McParticle mass [GeV] */
    Float_t m_lifeTime;   /**< McParticle lifetime [ns] */
    Int_t m_PDG; /**< PDG */
    Int_t m_subDet; /**< Subdetector */
    Int_t m_iden; /**< iden */
    Int_t m_trackID; /**< TrackID */
    Int_t m_iEvent; /**< Event identifier */
    Int_t m_iEntry; /**< Entry identifier */
    Int_t m_nSimHits[13]; /**< Array with number of SimHits */
    Int_t m_hitPDG[13]; /**< Array with PDG hits */
    Int_t m_momPDG[13]; /**< Array with PDG momentum */
    Float_t m_E_start; /**< Starting energy */
    Float_t m_E_end; /**< Ending energy */
    Float_t m_eDep; /**< Deposited energy */
    Float_t m_trackLength; /**< Track length */
    Float_t m_momentumX; /**< X component of momentum */
    Float_t m_momentumY; /**< Y component of momentum */
    Float_t m_momentumZ; /**< Z component of momentum */
    Float_t m_positionX; /**< X component of position */
    Float_t m_positionY; /**< Y component of position */
    Float_t m_positionZ; /**< Z component of position */
    Float_t m_nWeight; /**< Weight */

    TFile* m_ff;    /**< Output root file.   */
    TTree* m_tree1;   /**< Output tree */
    TTree* m_tree2;   /**< Output tree */
  };
} // Belle2 namespace
