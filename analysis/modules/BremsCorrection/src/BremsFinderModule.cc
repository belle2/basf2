/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own include
#include <analysis/modules/BremsCorrection/BremsFinderModule.h>

// framework aux
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationVector.h>

// dataobjects
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/ECLCluster.h>

// utilities
#include <analysis/DecayDescriptor/ParticleListName.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>

// variables
#include <analysis/variables/ECLVariables.h>

#include <cmath>
#include <algorithm>
#include <TMatrixFSym.h>

using namespace std;

namespace Belle2 {
//-----------------------------------------------------------------
//                 Register module
//-----------------------------------------------------------------

  REG_MODULE(BremsFinder)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

  BremsFinderModule::BremsFinderModule() :
    Module(), m_pdgCode(0)
  {
    // set module description (e.g. insert text)
    setDescription(R"DOC(
    This module copies each particle in the ``inputList`` to the ``outputList`` and uses
    the results of the **eclTrackBremFinder** module to look for possible bremsstrahlung photons; if these 
    photons exist, it adds their four momentum to the particle in the ``outputList``.
    It also adds the original particle and these photons as daughters of the new, corrected particle. 
    Track and PID information of the original particle are copied onto the new one to facilitate their access 
    in the analysis scripts.

    The **eclTrackBremFinder** module uses the lepton track PXD and SVD hits and extrapolates them to the ECL; 
    then looks for ECL clusters with energies between 0.2 and 1 times the track energy and without associated 
    tracks, and checks if the distance between each of these clusters 
    and the extrapolated hit is smaller than 0.5 mm. If it is, a *Bremsstrahlung* weighted relation 
    between said cluster and the track is established. The weight is determined as

    .. math:: 
                   
       \text{max}\left(\frac{\left|\phi_{\text{cluster}}-\phi_{\text{hit}}\right|}{\Delta\phi_{\text{cluster}}+\Delta\phi_{\text{hit}}}, \, \frac{\left|\theta_{\text{cluster}}-\theta_{\text{hit}}\right|}{\Delta\theta_{\text{cluster}}+\Delta\theta_{\text{hit}}}\right)

    where :math:`\phi_i` and :math:`\theta_i` are the azimuthal and polar angles of the ECL cluster and the
    extrapolated hit, and :math:`\Delta x` represents the uncertainty of the value :math:`x`. The details of the
    calculation of these quantities are `here`_. By default, only relations with a weight smaller than 3.0 are stored.
    The user can further reduce the maximally allowed value of this weight to remove unwanted photons from the
    bremsstrahlung correction.

    This module looks for photons in the ``gammaList`` whose clusters have a *Bremsstrahlung* relation with the track 
    of one of the particles in the ``inputList``, and adds their 4-momentum to the particle's one. It also stores the value
    of each relation weight as ``extraInfo`` of the corrected particle, under the name ``"bremsWeightWithPhotonN"``, where
    N is the index of the photon as daughter of the corrected particle; thus ``"bremsWeightWithPhoton0"`` gives the weight
    of the Bremsstrahlung relation between the new, corrected particle, and the first photon daughter.

    Warning:
      Even in the event of no bremsstrahlung photons found, a new particle is still created, and the original one is still 
      added as its daughter.

    Warning:
      Studies have shown that the requirements that the energy of the photon must be between 0.2 and 1 times the track energy and
      that only track-photon relations with a weight below three are considered, are too tight. Until these are relaxed and a new
      processing is done (MC15 and proc 13) it might be better to use the alternative `BelleBremRecovery` module.
                  
    See also:
      `eclTrackBremFinder module`_
                   
    .. _eclTrackBremFinder module: https://stash.desy.de/projects/B2/repos/basf2/browse/ecl/modules/eclTrackBremFinder
    .. _here: https://stash.desy.de/projects/B2/repos/basf2/browse/ecl/modules/eclTrackBremFinder/src/BremFindingMatchCompute.cc)DOC");
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("outputList", m_outputListName, "The output particle list name.");
    addParam("inputList", m_inputListName,
             R"DOC(The initial particle list name containing the particles to correct. *It should already exist* and *the particles must have an associated track.*)DOC");
    addParam("gammaList", m_gammaListName,
             R"DOC(The photon list containing the preselected bremsstrahlung candidates. *It should already exist* and *the particles in the list must be photons*)DOC");
    addParam("maximumAcceptance", m_maximumAcceptance,
             "The maximum value of the relation weight between a bremsstrahlung cluster and a particle track",
             m_maximumAcceptance);
    addParam("multiplePhotons", m_addMultiplePhotons, "If true, use all possible photons to correct the particle's 4-momentum",
             m_addMultiplePhotons);
    addParam("usePhotonOnlyOnce", m_usePhotonOnlyOnce,
             R"DOC(If true, each brems candidate is used to correct maximum 1 particle (the one with the lowest relation weight among all in the ``inputList``).)DOC",
             m_usePhotonOnlyOnce);
    addParam("writeOut", m_writeOut,
             R"DOC(If true, the output ``ParticleList`` will be saved by `RootOutput`. If false, it will be ignored when writing the file.)DOC",
             m_writeOut);
  }

  void BremsFinderModule::initialize()
  {
    //Get input lepton particle list
    m_inputList.isRequired(m_inputListName);

    DecayDescriptor decDes;
    decDes.init(m_inputListName);

    m_pdgCode  = decDes.getMother()->getPDGCode();
    //Check for the particles in the lepton list to have a track associated
    if (Const::chargedStableSet.find(abs(m_pdgCode)) == Const::invalidParticle)
      B2ERROR("[BremsFinderModule] Invalid particle list. the particles in " << m_inputListName << " should have an associated track.");

    //Get input photon particle list
    m_gammaList.isRequired(m_gammaListName);

    decDes.init(m_gammaListName);
    int temp_pdg = decDes.getMother()->getPDGCode();

    //Check that this is a gamma list
    if (temp_pdg != Const::photon.getPDGCode())
      B2ERROR("[BremsFinderModule] Invalid particle list. the particles in " << m_gammaListName << " should be photons!");

    decDes.init(m_outputListName);
    temp_pdg = decDes.getMother()->getPDGCode();
    // check the validity of output ParticleList name
    if (m_inputListName == m_outputListName)
      B2ERROR("[BremsFinderModule] Input and output particle list names are the same: " << m_inputListName);
    else if (temp_pdg != m_pdgCode) {
      B2ERROR("[BremsFinderModule] The input and output particle list correspond to different particles: " << m_inputListName << " --> "
              << m_outputListName);
    }

    // output particle
    m_outputAntiListName = ParticleListName::antiParticleListName(m_outputListName);

    // make output lists
    DataStore::EStoreFlags flags = m_writeOut ? DataStore::c_WriteOut : DataStore::c_DontWriteOut;
    m_outputList.registerInDataStore(m_outputListName, flags);
    m_outputAntiList.registerInDataStore(m_outputAntiListName, flags);

    m_particles.registerRelationTo(m_pidlikelihoods);
  }

  void BremsFinderModule::event()
  {
    RelationArray particlesToMCParticles(m_particles, m_mcParticles);

    // new output particle list
    m_outputList.create();
    m_outputList->initialize(m_pdgCode, m_outputListName);

    m_outputAntiList.create();
    m_outputAntiList->initialize(-1 * m_pdgCode, m_outputAntiListName);
    m_outputAntiList->bindAntiParticleList(*(m_outputList));

    // Number of photons (calculate it here only once)
    const unsigned int nGamma = m_gammaList->getListSize();

    // Number of leptons (calculate it here only once)
    const unsigned int nLep = m_inputList->getListSize();

    const std::string relationName = "Bremsstrahlung";

    //In the case of only one track per photon
    if (m_usePhotonOnlyOnce) {
      for (unsigned n = 0; n < nGamma; n++) {
        Particle* gamma = m_gammaList->getParticle(n);
        //Skip this if it the best match has already been assigned (pathological case: happens only if you use the same gamma list
        //to correct more than once. Performance studies, in which the same list is used with different options, are an example
        if (gamma->hasExtraInfo("bestMatchIndex")) continue;

        auto cluster = gamma->getECLCluster();
        //Get the tracks related to each photon...
        RelationVector<Track> relatedTracks = cluster->getRelationsTo<Track>("", relationName);
        double bestWeight = m_maximumAcceptance;
        unsigned bestMatchIndex = 0;
        unsigned trkIndex = 0;
        //Loop over the related tracks...
        for (auto trk = relatedTracks.begin(); trk != relatedTracks.end(); trk++, trkIndex++) {
          //... and over the input particles' tracks...
          for (unsigned i = 0; i < nLep; i++) {
            const Particle* lepton = m_inputList->getParticle(i);
            auto leptonTrack = lepton->getTrack();
            //... check that the particle track corresponds to the related track....
            if (leptonTrack->getArrayIndex() == trk->getArrayIndex()) {
              double weight = relatedTracks.weight(trkIndex);
              if (weight < bestWeight) {
                bestWeight = weight;
                //... and only select the best match among the tracks in the input list
                bestMatchIndex = trk->getArrayIndex();
              }
              break; //If the particle corresponding to the related track is found, break the loop over the particles and go for the next related track
            }
          }
        }
        //... finally, add the best match index as an extra info for the photon
        gamma->addExtraInfo("bestMatchIndex", bestMatchIndex);
      }
    }

    // loop over charged particles, correct them and add them to the output list

    for (unsigned i = 0; i < nLep; i++) {
      const Particle* lepton = m_inputList->getParticle(i);

      //Get the track of this lepton...
      auto track = lepton->getTrack();

      //... and get the bremsstrahlung clusters related to this track
      RelationVector<ECLCluster> bremClusters = track->getRelationsFrom<ECLCluster>("", relationName);

      std::vector<std::pair <double, Particle*> > selectedGammas;

      unsigned j = 0;
      for (auto bremCluster = bremClusters.begin(); bremCluster != bremClusters.end(); bremCluster++, j++) {
        double weight = bremClusters.weight(j);

        if (weight > m_maximumAcceptance) continue;

        for (unsigned k = 0; k < nGamma; k++) {

          Particle* gamma = m_gammaList->getParticle(k);
          auto cluster = gamma->getECLCluster();

          if (bremCluster->getClusterId() == cluster->getClusterId()) {
            if (m_usePhotonOnlyOnce) { //If only one track per photon should be used...
              if (track->getArrayIndex() == static_cast<int>
                  (gamma->getExtraInfo("bestMatchIndex")))      //... check if this track is the best match ...
                selectedGammas.push_back(std::make_pair(weight, gamma)); //... and if it is, add it to the selected gammas
            } else {
              selectedGammas.push_back(std::make_pair(weight, gamma));
            }
          }

        } // Closes for loop on gammas

      } // Closes for loop on brem clusters

      //The 4-momentum of the new lepton in the output particle list
      TLorentzVector new4Vec = lepton->get4Vector();

      //Sort weight-particle pairs by weight. Smaller weights go first
      std::sort(selectedGammas.begin(), selectedGammas.end());

      //Add to this 4-momentum those of the selected photon(s)
      for (auto const& bremsPair : selectedGammas) {
        Particle* g = bremsPair.second;
        new4Vec += g->get4Vector();
        if (! m_addMultiplePhotons) break; //stop after adding the first photon
      }

      //Create the new particle with the 4-momentum calculated before
      Particle correctedLepton(new4Vec, lepton->getPDGCode(), Particle::EFlavorType::c_Flavored, Particle::c_Track,
                               track->getArrayIndex());

      //And add the original lepton as its daughter
      correctedLepton.appendDaughter(lepton, false);

      const TMatrixFSym& lepErrorMatrix = lepton->getMomentumVertexErrorMatrix();
      TMatrixFSym corLepMatrix(lepErrorMatrix);

      double bremsGammaEnergySum = 0.0;
      //Now, if there are any, add the brems photons as daughters as well. As before, we distinguish between the multiple and only one brems photon cases
      int photonIndex = 0;
      for (auto const& bremsPair : selectedGammas) {
        //Add the weights as extra info of the mother
        Particle* bremsGamma = bremsPair.second;
        std::string extraInfoName = "bremsWeightWithPhoton" + std::to_string(photonIndex);
        correctedLepton.addExtraInfo(extraInfoName, bremsPair.first);
        photonIndex++;
        bremsGammaEnergySum += Variable::eclClusterE(bremsGamma);

        const TMatrixFSym& gammaErrorMatrix = bremsGamma->getMomentumVertexErrorMatrix();
        for (int irow = 0; irow <= 3; irow++) {
          for (int icol = irow; icol <= 3; icol++) corLepMatrix(irow, icol) += gammaErrorMatrix(irow, icol);
        }
        correctedLepton.appendDaughter(bremsGamma, false);
        B2DEBUG(10, "[BremsFinderModule] Found a bremsstrahlung gamma and added its 4-vector to the charged particle");
        if (! m_addMultiplePhotons) break; //stop after adding the first photon
      }

      correctedLepton.setMomentumVertexErrorMatrix(corLepMatrix);

      // add the info from original lepton to the new lepton
      correctedLepton.setVertex(lepton->getVertex());
      correctedLepton.setPValue(lepton->getPValue());
      correctedLepton.addExtraInfo("bremsCorrected", float(selectedGammas.size() > 0));
      correctedLepton.addExtraInfo("bremsCorrectedPhotonEnergy", bremsGammaEnergySum);

      // add the mc relation
      Particle* newLepton = m_particles.appendNew(correctedLepton);
      const MCParticle* mcLepton = lepton->getRelated<MCParticle>();
      const PIDLikelihood* pid = lepton->getPIDLikelihood();

      if (pid) newLepton->addRelationTo(pid);

      if (mcLepton != nullptr) newLepton->addRelationTo(mcLepton);

      m_outputList->addParticle(newLepton);

    } //Closes for loop on leptons

  } //Close event()


} // end Belle2 namespace

