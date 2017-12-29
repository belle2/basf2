/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Belle II Local Maximum Finder (LMF). Two stage procedure: First it     *
 * searches a local maximum (energy higher than all neighbours and above  *
 * threshold) and second it uses a method  for different                  *
 * particle types to decide if the local maximum is a candidate. Each     *
 * candidate will result in one shower/cluster in the end.                *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (ferber@physics.ubc.ca)                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// THIS MODULE
#include <ecl/modules/eclLocalMaximumFinder/ECLLocalMaximumFinderModule.h>

// FRAMEWORK
#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>
#include <framework/datastore/RelationArray.h>
#include <framework/utilities/FileSystem.h>

// NAMESPACE(S)
using namespace Belle2;
using namespace ECL;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLLocalMaximumFinder)
REG_MODULE(ECLLocalMaximumFinderPureCsI)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------
ECLLocalMaximumFinderModule::ECLLocalMaximumFinderModule() : Module(),
  m_mcParticles(mcParticleArrayName()),
  m_eclHits(eclHitArrayName()),
  m_eclDigits(eclDigitArrayName()),
  m_eclCalDigits(eclCalDigitArrayName()),
  m_eclConnectedRegions(eclConnectedRegionArrayName()),
  m_eclLocalMaximums(eclLocalMaximumArrayName())
{
  // Set description.
  setDescription("ECLLocalMaximumFinderModule");

  // Parallel processing certification.
  setPropertyFlags(c_ParallelProcessingCertified);

  // Add module parameters.
  addParam("energyCut", m_energyCut, "Seed energy cut [MeV], minimum is 5.0 MeV.", 10.0 * Belle2::Unit::MeV);
  addParam("isTrainingMode", m_isTrainingMode,
           "Run in training mode (i.e. fill file with MVA input variables and determine MC truth of LM.).", 0);
  addParam("outfileName", m_outfileName, "Output file name for training file.", std::string("ECLLocalMaximumFinderOutput.root"));
  addParam("method", m_method, "Method to determine the LM (cut, none, fastbdt).", std::string("none"));
  addParam("truthFraction", m_truthFraction, "Minimum matched energy fraction truth/rec for the LM.", 0.51);
  addParam("cutOffset", m_cutOffset, "Cut method specific: Offset. (BaBar: 2.5, high eff: 1.40)", 2.5);
  addParam("cutSlope", m_cutSlope, "Cut method specific: Slope. (BaBar: 0.5, high eff: 3.0)", 0.5);
  addParam("cutRatioCorrection", m_cutRatioCorrection, "Cut method specific: Ratio correction.", 0.0);

}

ECLLocalMaximumFinderModule::~ECLLocalMaximumFinderModule()
{
  ;
}

void ECLLocalMaximumFinderModule::initialize()
{
  B2DEBUG(200, "ECLLocalMaximumFinderModule::initialize()");

  m_eclLocalMaximums.registerInDataStore(eclLocalMaximumArrayName());
  m_eclCalDigits.registerInDataStore(eclCalDigitArrayName());
  m_eclConnectedRegions.registerInDataStore(eclConnectedRegionArrayName());
  m_eclConnectedRegions.registerRelationTo(m_eclLocalMaximums);
  m_eclCalDigits.registerRelationTo(m_eclLocalMaximums);

  // Check user input.
  if (m_energyCut < c_minEnergyCut) {
    B2WARNING("ECLLocalMaximumFinderModule::initialize: Energy threshold too small, resetting to " << c_minEnergyCut << " GeV");
    m_energyCut = c_minEnergyCut;
  }

  if (m_truthFraction < 1e-6) {
    B2WARNING("ECLLocalMaximumFinderModule::initialize: Matching fraction must be above 1e-6, input: " << m_truthFraction <<
              " will be reset to 1e-6.");
    m_truthFraction = 1e-6;
  }

  if (m_truthFraction > 1.0) {
    B2WARNING("ECLLocalMaximumFinderModule::initialize: Matching fraction must be below 1.0, input: " << m_truthFraction <<
              " will be reset to 1.0.");
    m_truthFraction = 1.0;
  }

  // Geometry instance.
  m_geom = ECLGeometryPar::Instance();

  // Initialize neighbour map.
  m_neighbourMap = new ECLNeighbours("N", 1);

  // Reset all variables.
  resetClassifierVariables();

  // Open output files and declare branches if in training mode. Each file will hold a flat ntuple of training data.
  m_outfile = NULL;
  m_tree = NULL;
  if (m_isTrainingMode > 0) {
    const int cBufferLength = 500;
    char tmpBuffer[cBufferLength];
    int tmpBufferLength = sprintf(tmpBuffer, "%s", m_outfileName.c_str());

    if (tmpBufferLength >= cBufferLength) {
      B2FATAL("ECLLocalMaximumFinderModule::initialize(): Output training filename length too long!");
    }

    m_outfile = new TFile(tmpBuffer, "RECREATE");
    m_tree = new TTree("locmax", "locmax");
    m_tree->Branch("energyRatioNeighbour0", &m_energyRatioNeighbour[0], "energyRatioNeighbour0/F");
    m_tree->Branch("energyRatioNeighbour1", &m_energyRatioNeighbour[1], "energyRatioNeighbour1/F");
    m_tree->Branch("energyRatioNeighbour2", &m_energyRatioNeighbour[2], "energyRatioNeighbour2/F");
    m_tree->Branch("energyRatioNeighbour3", &m_energyRatioNeighbour[3], "energyRatioNeighbour3/F");
    m_tree->Branch("energyRatioNeighbour4", &m_energyRatioNeighbour[4], "energyRatioNeighbour4/F");
    m_tree->Branch("energyRatioNeighbour5", &m_energyRatioNeighbour[5], "energyRatioNeighbour5/F");
    m_tree->Branch("energyRatioNeighbour6", &m_energyRatioNeighbour[6], "energyRatioNeighbour6/F");
    m_tree->Branch("energyRatioNeighbour7", &m_energyRatioNeighbour[7], "energyRatioNeighbour7/F");
    m_tree->Branch("energyRatioNeighbour8", &m_energyRatioNeighbour[8], "energyRatioNeighbour8/F");
    m_tree->Branch("energyRatioNeighbour9", &m_energyRatioNeighbour[9], "energyRatioNeighbour9/F");
    m_tree->Branch("energyRatioNeighbour10", &m_energyRatioNeighbour[10], "energyRatioNeighbour10/F");
    m_tree->Branch("energyRatioNeighbour11", &m_energyRatioNeighbour[11], "energyRatioNeighbour11/F");
    m_tree->Branch("energy", &m_energy, "energy/F");
    m_tree->Branch("target", &m_target, "target/F");
    m_tree->Branch("targetindex", &m_targetindex, "targetindex/F");
    m_tree->Branch("targetpi0index", &m_targetpi0index, "targetpi0index/F");
    m_tree->Branch("thetaId", &m_thetaId, "thetaId/F");
    m_tree->Branch("phiId", &m_phiId, "phiId/F");
    m_tree->Branch("cellId", &m_cellId, "cellId/F");
    m_tree->Branch("maxNeighbourEnergy", &m_maxNeighbourEnergy, "maxNeighbourEnergy/F");
    m_tree->Branch("nNeighbours10", &m_nNeighbours10, "nNeighbours10/F");
    m_tree->Branch("time", &m_time, "time/F");
    m_tree->Branch("timeResolution", &m_timeResolution, "timeResolution/F");
    m_tree->Branch("timeFitFailed", &m_timeFitFailed, "timeFitFailed/F");
    m_tree->Branch("CRId", &m_CRId, "CRId/F");
    m_tree->Branch("LMId", &m_LMId, "LMId/F");
  }

  // initialize the vector that gives the relation between cellid and store array position
  m_StoreArrPosition.resize(8736 + 1);

}

void ECLLocalMaximumFinderModule::beginRun()
{
  ;
}

void ECLLocalMaximumFinderModule::event()
{
  B2DEBUG(200, "ECLLocalMaximumFinderModule::event()");

  // Fill a vector that can be used to map cellid -> store array position
  std::fill_n(m_StoreArrPosition.begin(), m_StoreArrPosition.size(), -1);
  for (int i = 0; i < m_eclCalDigits.getEntries(); i++) {
    m_StoreArrPosition[m_eclCalDigits[i]->getCellId()] = i;
  }

  // Vector with neighbour ids.
  std::vector< double > vNeighourEnergies;
  vNeighourEnergies.resize(c_nMaxNeighbours);

  // Loop over connected regions.
  for (const ECLConnectedRegion& aCR : m_eclConnectedRegions) {
    const int crId = aCR.getCRId();
    int iLM = 1;

    // Loop over all entries in this CR.
    for (const ECLCalDigit& aECLCalDigit : aCR.getRelationsTo<ECLCalDigit>()) {

      // Check seed energy cut.
      if (aECLCalDigit.getEnergy() >= m_energyCut) {

        // Clean up for this candiate (MVA is trained per LM, regardless of CR)
        std::fill_n(vNeighourEnergies.begin(), vNeighourEnergies.size(),
                    -999);   // -999 means later: this digit is just not available in this neighbour definition.
        resetTrainingVariables();
        resetClassifierVariables();

        // Check neighbours: Must be a local energy maximum.
        bool isLocMax = 1;
        int neighbourCount = 0;
        for (auto& neighbourId : m_neighbourMap->getNeighbours(aECLCalDigit.getCellId())) {
          if (neighbourId == aECLCalDigit.getCellId()) continue; // Skip the center cell to avoid possible floating point issues.

          const int pos = m_StoreArrPosition[neighbourId]; // Get position in the store array for this digit.

          double energyNeighbour = 0.0;
          if (pos >= 0) {
            energyNeighbour = m_eclCalDigits[pos]->getEnergy(); // Get the energy directly from the store array.
            vNeighourEnergies[neighbourCount] = energyNeighbour;
          } else {
            // Digit does not belong to this CR
            vNeighourEnergies[neighbourCount] = 0.0;
          }
          ++neighbourCount;

          if (energyNeighbour > aECLCalDigit.getEnergy()) {
            isLocMax = 0;
            break;
          }
        }

        // It is a local maximum. Get all variables needed for classification.
        if (isLocMax) {

          for (unsigned int npos = 0; npos < vNeighourEnergies.size(); ++npos) {
            if (vNeighourEnergies[npos] >= 0) {
              m_energyRatioNeighbour[npos] = static_cast < float >(vNeighourEnergies[npos] / aECLCalDigit.getEnergy());
              if (vNeighourEnergies[npos] > m_maxNeighbourEnergy) m_maxNeighbourEnergy = vNeighourEnergies[npos];
              if (vNeighourEnergies[npos] > 1.0 * Belle2::Unit::MeV) ++m_nNeighbours10;
            } else m_energyRatioNeighbour[npos] = 0.0;
          }
          m_time           = aECLCalDigit.getTime();
          m_maxEnergyRatio = m_maxNeighbourEnergy / aECLCalDigit.getEnergy();

          // Fill training monitoring variables and MC truth information.
          if (m_isTrainingMode > 0) {

            m_energy                  = static_cast < float >(aECLCalDigit.getEnergy());
            m_cellId                  = static_cast < float >(aECLCalDigit.getCellId());
            m_timeResolution          = static_cast < float >(aECLCalDigit.getTimeResolution());
            m_timeFitFailed           = static_cast < float >(aECLCalDigit.isFailedFit());
            m_CRId                    = static_cast < float >(crId);
            m_LMId                    = static_cast < float >(iLM);

            m_geom->Mapping(m_cellId - 1);
            m_thetaId                 = static_cast < float >(m_geom->GetThetaID());
            m_phiId                   = static_cast < float >(m_geom->GetPhiID());
          }

          if (m_isTrainingMode > 0) { // This requires MC matching before this stage!
            int motherpdg   = -1;
            int motherindex = -1;
            int pi0index    = -1;
            int maxtype     = 0;
            int maxpos      = 0;

            auto relatedParticlePairs = aECLCalDigit.getRelationsWith<MCParticle>();

            for (unsigned int irel = 0; irel < relatedParticlePairs.size(); irel++) {
              const auto particle = relatedParticlePairs.object(irel);
              const double weight = relatedParticlePairs.weight(irel);

              motherpdg = -1;
              motherindex = -1;
              pi0index = -1;
              getEnteringMother(*particle, motherpdg, motherindex, pi0index);
              addToSignalEnergy(motherpdg, motherindex, pi0index, weight);
            }

            B2DEBUG(175, "  -> digt energy: " << aECLCalDigit.getEnergy());
            B2DEBUG(175, "photon: " << m_signalEnergy[0][0] << " " << m_signalEnergy[0][1]);
            B2DEBUG(175, "pi0: " << m_signalEnergy[1][0] << " " << m_signalEnergy[1][1]);
            B2DEBUG(175, "electron: " << m_signalEnergy[2][0] << " " << m_signalEnergy[2][1]);
            B2DEBUG(175, "muon: " << m_signalEnergy[3][0] << " " << m_signalEnergy[3][1]);
            B2DEBUG(175, "neutral hadron: " << m_signalEnergy[4][0] << " " << m_signalEnergy[4][1]);
            B2DEBUG(175, "charged hadron: " << m_signalEnergy[5][0] << " " << m_signalEnergy[5][1]);
            B2DEBUG(175, "other: " << m_signalEnergy[6][0]);

            maxtype     = 0;
            maxpos      = 0;
            getMax(maxtype, maxpos);

            if (maxtype >= 0) {
              if (m_signalEnergy[maxtype][maxpos] >= m_truthFraction * aECLCalDigit.getEnergy()) {
                m_target = maxtype;
                m_targetindex = m_signalId[maxtype][maxpos];
                m_targetpi0index = pi0index;
              } else {
                m_target = 7;
              }
            } else {
              m_target = 7;
            }

            m_tree->Fill();

          } // end training

          if (m_method == "cut") {

            B2DEBUG(200, "m_cutSlope: " << m_cutSlope << ", m_nNeighbours10: " << m_nNeighbours10 << ", m_cutOffset: " << m_cutOffset <<
                    ", m_maxNeighbourEnergy: " << m_maxNeighbourEnergy << ", m_cutRatioCorrection: " << m_cutRatioCorrection <<
                    ", aECLCalDigit.getEnergy(): " << aECLCalDigit.getEnergy());
            B2DEBUG(200, "m_cutSlope * (m_nNeighbours10 - m_cutOffset): " << m_cutSlope * (m_nNeighbours10 - m_cutOffset));
            B2DEBUG(200, "(m_maxNeighbourEnergy - m_cutRatioCorrection) / (aECLCalDigit.getEnergy() - m_cutRatioCorrection)" <<
                    (m_maxNeighbourEnergy - m_cutRatioCorrection) / (aECLCalDigit.getEnergy() - m_cutRatioCorrection) << "\n");

            if (m_cutSlope * (m_nNeighbours10 - m_cutOffset) >= (m_maxNeighbourEnergy - m_cutRatioCorrection) /
                (aECLCalDigit.getEnergy() - m_cutRatioCorrection)) {
              makeLocalMaximum(aCR, aECLCalDigit.getCellId(), iLM);
              ++iLM;
            }
          } else if (m_method == "none") { // All energy local maximums will become local maximums.
            makeLocalMaximum(aCR, aECLCalDigit.getCellId(), iLM);
            ++iLM;
          }

        }

      } // end check energy

    } // end CalDigit loop

    // Check if there is at least one local maximum in the CR. If not, make the highest energetic crystal one.
    if (iLM == 1) {

      int highestEnergyCellId = -1;
      double highestEnergy = 0.0;

      // Loop over all entries in this CR.
      for (const ECLCalDigit& aECLCalDigit : aCR.getRelationsTo<ECLCalDigit>(eclCalDigitArrayName())) {
        if (aECLCalDigit.getEnergy() > highestEnergy) {
          highestEnergyCellId = aECLCalDigit.getCellId();
          highestEnergy = aECLCalDigit.getEnergy();
        }
      } // end CalDigit loop

      makeLocalMaximum(aCR, highestEnergyCellId, 0);

    }

  } // end CR loop

}


void ECLLocalMaximumFinderModule::endRun()
{
  B2DEBUG(200, "ECLLocalMaximumFinderModule::endRun()");
}


void ECLLocalMaximumFinderModule::terminate()
{
  B2DEBUG(200, "ECLLocalMaximumFinderModule::terminate()");

  // If run in trainingmode, write the training file.
  if (m_outfile and m_tree) {
    m_outfile->cd();
    m_tree->Write();
    m_outfile->Write();
    m_outfile->Close();
    delete m_outfile;
  }

  if (m_neighbourMap) delete m_neighbourMap;

}

void ECLLocalMaximumFinderModule::makeLocalMaximum(const ECLConnectedRegion& aCR, const int cellId, const int lmId)
{

  // Output Array
  StoreArray<ECLLocalMaximum> eclLocalMaximums(eclLocalMaximumArrayName());

  // Set the local maximum dataobject.
  const auto aLocalMaximum = eclLocalMaximums.appendNew();

  B2DEBUG(175, "ECLLocalMaximumFinderModule::makeLocalMaximum(): local maximum cellid: " << cellId);

  // Set the id of this local maximum.
  aLocalMaximum->setLMId(lmId);

  // Set the cell Id of the digit.
  aLocalMaximum->setCellId(cellId);

  // Add relations to ECLConnectedRegion.
  aCR.addRelationTo(aLocalMaximum);

}

void ECLLocalMaximumFinderModule::resetTrainingVariables()
{
  m_target = -1;
  m_targetindex = -1;
  m_targetpi0index = -1;

  m_totalSignalEnergy = 0.;

  for (unsigned int i = 0; i < 10; ++i) {
    for (unsigned int j = 0; j < 5; ++j) {
      m_signalEnergy[i][j] = 0.;
      m_signalId[i][j] = -1;
    }
  }

}

void ECLLocalMaximumFinderModule::resetClassifierVariables()
{

  for (unsigned i = 0; i < c_nMaxNeighbours; ++i) {
    m_energyRatioNeighbour[i] = 0.;
  }

  m_energy = 0.;
  m_cellId = -1;
  m_thetaId = -1;
  m_phiId = -1;
  m_nNeighbours10 = 0;
  m_maxNeighbourEnergy = 0.0;
  m_time = -9999;
  m_timeResolution = -9999.;
  m_timeFitFailed = -9999.;
  m_CRId = -1;
  m_LMId = -1;
  m_maxEnergyRatio = 0.0;
}


void ECLLocalMaximumFinderModule::getEnteringMother(const MCParticle& particle, int& pdg, int& arrayindex, int& pi0arrayindex)
{

  int index = particle.getArrayIndex();
  int pi0index = -1;

  while (!isEnteringECL(m_mcParticles[index]->getProductionVertex())) {
    if (m_mcParticles[index]->getMother()) index = m_mcParticles[index]->getMother()->getArrayIndex();
    else index = -1;
  };

  // For photon mother: are they from a pi0? This can be used to improved overlap/merged pi0 reconstruction.
  if (m_mcParticles[index]->getPDG() == 22) {
    if (m_mcParticles[index]->getMother()->getPDG() == 111) {
      pi0index = m_mcParticles[index]->getMother()->getArrayIndex();
    }
  }

  // Dont include mother if its energy is too low or if its a photon from a neutron interaction.
  if ((m_mcParticles[index]->getEnergy() < 5.0 * Belle2::Unit::MeV)
      or (m_mcParticles[index]->getPDG() == 22 and abs(m_mcParticles[index]->getMother()->getPDG()) == 2212)) {
    pdg = -1;
    arrayindex = -1;
    pi0arrayindex = -1;
  } else {
    pdg = m_mcParticles[index]->getPDG();
    arrayindex = index;
    pi0arrayindex = pi0index;
  }
}

bool ECLLocalMaximumFinderModule::isEnteringECL(const B2Vector3D& vertex)
{

  const double theta = vertex.Theta();

  if (theta > 0.555015 and theta < 2.26369) { //barrel
    double radius = vertex.Perp();
    if (radius < 125 * Belle2::Unit::cm) return true;
  } else if (theta <= 0.555015) { //fwd
    if (vertex.Z() < 196.16 * Belle2::Unit::cm) return true;
  } else if (theta >= 2.26369) { //bwd
    if (vertex.Z() > -102.16 * Belle2::Unit::cm) return true;
  }

  return false;
}

int ECLLocalMaximumFinderModule::getIdPosition(const int type, const int id)
{

  for (int i = 0; i < 5; i++) {
    if (m_signalId[type][i] == id) return i;
    if (m_signalId[type][i] == -1) {
      m_signalId[type][i] = id;
      return i; //next free one
    }
  }

  return -1;
}

// of all entries, get the maximum!
void ECLLocalMaximumFinderModule::getMax(int& type, int& id)
{
  double maxe = 0.;
  int maxtype = -1;
  int maxid = -1;

  for (unsigned int i = 0; i < 10; ++i) {
    for (unsigned int j = 0; j < 5; ++j) {
      if (m_signalEnergy[i][j] > maxe) {
        maxe = m_signalEnergy[i][j];
        maxtype = i;
        maxid = j;
      }
    }
  }

  type = maxtype;
  id = maxid;

}

void ECLLocalMaximumFinderModule::addToSignalEnergy(int& motherpdg, int& motherindex, int& pi0index, const double& weight)
{

  // for the LM training and CR/LM debugging
  if (motherpdg == 22) {
    if (pi0index >= 0) { // photon from pi0
      int idpos = getIdPosition(1, motherindex);
      m_signalEnergy[1][idpos] += weight;
    } else {
      int idpos = getIdPosition(0, motherindex);
      m_signalEnergy[0][idpos] += weight; // photon from another source
    }
  } else if (abs(motherpdg) == 11) { // electron
    int idpos = getIdPosition(2, motherindex);
    m_signalEnergy[2][idpos] += weight;
  } else if (abs(motherpdg) == 13) { // muon
    int idpos = getIdPosition(3, motherindex);
    m_signalEnergy[3][idpos] += weight;
  } else if (abs(motherpdg) == 130 or abs(motherpdg) == 2112) { // neutral hadron
    int idpos = getIdPosition(4, motherindex);
    m_signalEnergy[4][idpos] += weight;
  } else if (abs(motherpdg) == 211 or abs(motherpdg) == 321 or abs(motherpdg) == 2212) { // charged hadron
    int idpos = getIdPosition(5, motherindex);
    m_signalEnergy[5][idpos] += weight;
  } else { // everything else
    int idpos = getIdPosition(6, motherindex);
    m_signalEnergy[6][idpos] += weight;
  }
}
