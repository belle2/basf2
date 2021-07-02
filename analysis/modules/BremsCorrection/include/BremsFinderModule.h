/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/PIDLikelihood.h>

#include <string>

namespace Belle2 {
  class Particle;

  /**
   * Bremsstrahlung finder correction module
   * For each lepton in the input particle list, this module copies it to the output particle list and uses the results of the eclTrackBremFinder module to look
   * for a possible bremsstrahlung photon; if this photon exists, adds its four momentum to the leptons in the output list. It also adds the original lepton and
   * this photon as daughters of the corrected lepton. Track and PID information of the original lepton are copied to the new one to facilitate their access in the
   * analysis scripts.
   *
   * The eclTrackBremFinder module uses the lepton track PXD and SVD hits and extrapolates them to the ECL; then looks for ECL clusters with energies between 0.2 and 1
   * times the track energy and without associated tracks, and checks if the distance between these clusters and the extrapolated tracks is smaller than 0.05.
   * If it is, the cluster is marked as a bremsstrahlung, and a weighted relation between the track and the cluster is established. The weight is determined by the maximum
   * between two values:
   *
   *    1. The difference in phi divided by the sum of the errors in the measurements of the cluster and the hit phi coordinate
   *    2. The difference in theta divided by the sum of the errors in the measurements of the cluster and the hit theta coordinate
   *
   * The user can determine the minimum value of this weight required in order to perform the Bremsstrahlung correction
   */
  class BremsFinderModule : public Module {

  public:

    /** Constructor, for setting module description and parameters. */
    BremsFinderModule();

    /** Use this to initialize resources or memory your module needs.
     *
     *  Also register any outputs of your module (StoreArrays, RelationArrays,
     *  StoreObjPtrs) here, see the respective class documentation for details.
     */
    virtual void initialize() override;

    /** Called once for each event.
     *
     * This is most likely where your module will actually do anything.
     */
    virtual void event() override;

    enum {c_DimMatrix = 7};

  private:
    std::string m_inputListName; /**< input particle list name */
    std::string m_gammaListName; /**< input gamma list name */
    std::string m_outputListName; /**< output particle list name */
    std::string m_outputAntiListName;   /**< output anti-particle list name */
    int m_pdgCode;                /**< PDG code of the particle to be corrected */
    StoreObjPtr<ParticleList>  m_inputList; /**<StoreObjptr for input charged particle list */
    StoreObjPtr<ParticleList>  m_gammaList; /**<StoreObjptr for gamma list */
    StoreObjPtr<ParticleList>  m_outputList; /**<StoreObjptr for output particlelist */
    StoreObjPtr<ParticleList>  m_outputAntiList; /**<StoreObjptr for output antiparticlelist */
    StoreArray<Particle> m_particles; /**< StoreArray of Particle objects */
    StoreArray<MCParticle> m_mcParticles; /**< StoreArray of MCParticle objects */
    StoreArray<PIDLikelihood> m_pidlikelihoods; /**< StoreArray of PIDLikelihood objects */
    double m_maximumAcceptance =
      3.0f;  /**< photons whose clusters have relation weights higher than this will not be used for bremsstrahlung correction of the track */
    bool m_addMultiplePhotons =
      false; /**<In case there is more than one brems photon, use only the best one (based on the weight of the relation), or all of them for correcting the lepton momentum */
    bool m_usePhotonOnlyOnce =
      false; /**< Each brems photon can be used to correct only one particle (the one with the smallest relation weight) */
    bool m_writeOut = false;  /**< Write the output particle list in the final file? */

  };

} // Belle2 namespace

