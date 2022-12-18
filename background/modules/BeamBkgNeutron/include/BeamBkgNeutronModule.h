#pragma once

#include <framework/core/Module.h>

#include <string>

#include <Math/Vector3D.h>
#include <TFile.h>
#include <TTree.h>

namespace Belle2 {
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

    std::vector<Float_t> trj_x;
    std::vector<Float_t> trj_y;
    std::vector<Float_t> trj_z;
    std::vector<Float_t> trj_px;
    std::vector<Float_t> trj_py;
    std::vector<Float_t> trj_pz;
    Float_t vtxProdX; /**< McParticle prod. vertex position X */
    Float_t vtxProdY; /**< McParticle prod. vertex position Y */
    Float_t vtxProdZ; /**< McParticle prod. vertex position Z */
    Float_t E_init;   /**< McParticle energy [GeV] */
    Float_t mass;   /**< McParticle mass [GeV] */
    Float_t lifeTime;   /**< McParticle lifetime [ns] */
    Int_t PDG, subDet, iden, trackID, iEvent, iEntry, nSimHits[13];
    Float_t E_start, E_end, eDep, trackLength, momentumX, momentumY, momentumZ, positionX, positionY, positionZ, nWeight;

    TFile* ff;    /**< Output root file.   */
    TTree* tree1;   /**< Output tree */
    TTree* tree2;   /**< Output tree */
  };
} // Belle2 namespace
