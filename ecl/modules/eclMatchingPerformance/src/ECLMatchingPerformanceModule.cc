/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Frank Meier                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/modules/eclMatchingPerformance/ECLMatchingPerformanceModule.h>

#include <ecl/geometry/ECLGeometryPar.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/datastore/RelationVector.h>

#include <root/TFile.h>
#include <root/TTree.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLMatchingPerformance)

ECLMatchingPerformanceModule::ECLMatchingPerformanceModule() :
  Module(), m_outputFile(NULL), m_dataTree(NULL)
{
  setDescription("Module to test the matching efficiency between tracks and ECLClusters. Additionally, information about the tracks are written into a ROOT file.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("outputFileName", m_outputFileName, "Name of output root file.",
           std::string("ECLMatchingPerformanceOutput.root"));
}

void ECLMatchingPerformanceModule::initialize()
{
  // Required modules
  m_recoTracks.isRequired();
  m_tracks.isRequired();
  m_trackFitResults.isRequired();
  m_eclClusters.isRequired();
  m_extHits.isRequired();
  m_eclCalDigits.isRequired();

  m_eclNeighbours1x1 = new ECL::ECLNeighbours("N", 0);
  m_eclNeighbours3x3 = new ECL::ECLNeighbours("N", 1);
  m_eclNeighbours5x5 = new ECL::ECLNeighbours("N", 2);

  m_outputFile = new TFile(m_outputFileName.c_str(), "RECREATE");
  TDirectory* oldDir = gDirectory;
  m_outputFile->cd();
  m_dataTree = new TTree("data", "data");
  oldDir->cd();

  setupTree();
}

void ECLMatchingPerformanceModule::event()
{
  StoreObjPtr<EventMetaData> eventMetaData("EventMetaData", DataStore::c_Event);
  m_iEvent = eventMetaData->getEvent();
  m_iRun = eventMetaData->getRun();
  m_iExperiment = eventMetaData->getExperiment();
  m_trackMultiplicity = m_tracks.getEntries();

  double distance;
  TVector3 pos_enter, pos_exit, pos_center;
  ECL::ECLGeometryPar* geometry = ECL::ECLGeometryPar::Instance();

  B2DEBUG(99, "Processes experiment " << m_iExperiment << " run " << m_iRun << " event " << m_iEvent);

  for (const Track& track : m_tracks) {
    setVariablesToDefaultValue();
    const RecoTrack* recoTrack = track.getRelated<RecoTrack>();
    if (recoTrack) {
      const TrackFitResult* fitResult = track.getTrackFitResult(Const::pion);
      B2ASSERT("Related Belle2 Track has no related track fit result!", fitResult);

      // write some data to the root tree
      TVector3 mom = fitResult->getMomentum();
      m_trackProperties.cosTheta = mom.CosTheta();
      m_trackProperties.phi = mom.Phi();
      m_trackProperties.ptot = mom.Mag();
      m_trackProperties.pt = mom.Pt();
      m_trackProperties.px = mom.Px();
      m_trackProperties.py = mom.Py();
      m_trackProperties.pz = mom.Pz();
      m_trackProperties.x = fitResult->getPosition().X();
      m_trackProperties.y = fitResult->getPosition().Y();
      m_trackProperties.z = fitResult->getPosition().Z();

      m_pValue = fitResult->getPValue();
      m_charge = (int)fitResult->getChargeSign();
      m_d0 = fitResult->getD0();

      ECLCluster* eclCluster = track.getRelatedTo<ECLCluster>();
      if (eclCluster != nullptr) {
        m_matchedToECLCluster = 1;
        m_hypothesisOfMatchedECLCluster = eclCluster->getHypothesisId();
      }
      bool found_first_enter = false;
      std::set<int> uniqueECLCalDigits;
      bool inserted;
      for (const auto& extHit : track.getRelationsTo<ExtHit>()) {
        int copyid =  extHit.getCopyID();
        if (copyid == -1) continue;
        if (extHit.getDetectorID() != Const::EDetector::ECL) continue;
        ECLCluster* eclClusterNear = extHit.getRelatedFrom<ECLCluster>();
        if (eclClusterNear) {
          distance = (eclClusterNear->getClusterPosition() - extHit.getPosition()).Mag();
          if (m_distance < 0 || distance < m_distance) {
            m_distance = distance;
          }
        }
        const int cell = copyid + 1;
        if (extHit.getStatus() == EXT_ENTER) {
          m_enter++;
          inserted = (uniqueECLCalDigits.insert(cell)).second;
          if (inserted) {
            for (const auto& eclCalDigit : m_eclCalDigits) {
              if (eclCalDigit.getCellId() == cell) {
                m_deposited_energy += eclCalDigit.getEnergy();
                break;
              }
            }
          }
          if (!found_first_enter) {
            pos_enter = extHit.getPosition();
            found_first_enter = true;
          }
          //Find ECLCalDigit in cell ID of ExtHit or one of its neighbours
          findECLCalDigitMatchInNeighbouringCell(m_eclNeighbours1x1, m_matchedTo1x1Neighbours, cell);
          findECLCalDigitMatchInNeighbouringCell(m_eclNeighbours3x3, m_matchedTo3x3Neighbours, cell);
          findECLCalDigitMatchInNeighbouringCell(m_eclNeighbours5x5, m_matchedTo5x5Neighbours, cell);
        } else if (extHit.getStatus() == EXT_EXIT) {
          m_exit++;
          pos_exit = extHit.getPosition();
        }
      }
      m_trackLength = (pos_enter - pos_exit).Mag();
      for (const auto& eclCalDigit : m_eclCalDigits) {
        if (eclCalDigit.getEnergy() < 0.01) continue;
        int cellid = eclCalDigit.getCellId();
        // TVector3 cvec = geometry->GetCrystalPos(cellid - 1);
        // distance = (cvec - 0.5 * (pos_enter + pos_exit)).Mag();
        // if (m_innerdistance < 0 || distance < m_innerdistance) {
        //   m_innerdistance = distance;
        // }
      }
    }
    m_dataTree->Fill(); // write data to tree
  }
}


void ECLMatchingPerformanceModule::terminate()
{
  writeData();
}

void ECLMatchingPerformanceModule::setupTree()
{
  if (m_dataTree == NULL) {
    B2FATAL("Data tree was not created.");
  }
  addVariableToTree("expNo", m_iExperiment);
  addVariableToTree("runNo", m_iRun);
  addVariableToTree("evtNo", m_iEvent);
  addVariableToTree("nTracks", m_trackMultiplicity);

  addVariableToTree("cosTheta", m_trackProperties.cosTheta);

  addVariableToTree("phi", m_trackProperties.phi);

  addVariableToTree("px", m_trackProperties.px);
  addVariableToTree("py", m_trackProperties.py);
  addVariableToTree("pz", m_trackProperties.pz);

  addVariableToTree("x", m_trackProperties.x);
  addVariableToTree("y", m_trackProperties.y);
  addVariableToTree("z", m_trackProperties.z);

  addVariableToTree("pt", m_trackProperties.pt);

  addVariableToTree("ptot", m_trackProperties.ptot);

  addVariableToTree("pValue", m_pValue);

  addVariableToTree("charge", m_charge);

  addVariableToTree("d0", m_d0);

  addVariableToTree("ECLMatch", m_matchedToECLCluster);
  addVariableToTree("HypothesisID", m_hypothesisOfMatchedECLCluster);

  addVariableToTree("MinDistance", m_distance);

  addVariableToTree("TrackToOneByOneNeighboursMatch", m_matchedTo1x1Neighbours);
  addVariableToTree("TrackToThreeByThreeNeighboursMatch", m_matchedTo3x3Neighbours);
  addVariableToTree("TrackToFiveByFiveNeighboursMatch", m_matchedTo5x5Neighbours);

  addVariableToTree("nECLEnter", m_enter);
  addVariableToTree("nECLExit", m_exit);

  addVariableToTree("DepositedEnergy", m_deposited_energy);
  addVariableToTree("TrackLengthInECL", m_trackLength);

  addVariableToTree("DistanceTo10MeV", m_innerdistance);
}

void ECLMatchingPerformanceModule::writeData()
{
  if (m_dataTree != NULL) {
    TDirectory* oldDir = gDirectory;
    if (m_outputFile)
      m_outputFile->cd();
    m_dataTree->Write();
    oldDir->cd();
  }
  if (m_outputFile != NULL) {
    m_outputFile->Close();
  }
}

void ECLMatchingPerformanceModule::setVariablesToDefaultValue()
{
  m_trackProperties = -999;

  m_pValue = -999;

  m_charge = 0;

  m_d0 = -999;

  m_matchedToECLCluster = 0;

  m_hypothesisOfMatchedECLCluster = 0;

  m_distance = -999;

  m_matchedTo1x1Neighbours = 0;
  m_matchedTo3x3Neighbours = 0;
  m_matchedTo5x5Neighbours = 0;

  m_enter = 0;
  m_exit = 0;

  m_deposited_energy = 0;
  m_trackLength = 0;

  m_innerdistance = -999;
}

void ECLMatchingPerformanceModule::addVariableToTree(const std::string& varName, double& varReference)
{
  std::stringstream leaf;
  leaf << varName << "/D";
  m_dataTree->Branch(varName.c_str(), &varReference, leaf.str().c_str());
}

void ECLMatchingPerformanceModule::addVariableToTree(const std::string& varName, int& varReference)
{
  std::stringstream leaf;
  leaf << varName << "/I";
  m_dataTree->Branch(varName.c_str(), &varReference, leaf.str().c_str());
}

void ECLMatchingPerformanceModule::findECLCalDigitMatchInNeighbouringCell(ECL::ECLNeighbours* eclneighbours,
    int& matchedToNeighbours, const int& cell)
{
  auto& vec_of_neighbouring_cells = eclneighbours->getNeighbours(cell);
  for (const auto& neighbouringcell : vec_of_neighbouring_cells) {
    const auto idigit = find_if(m_eclCalDigits.begin(), m_eclCalDigits.end(),
    [&](const ECLCalDigit & d) { return (d.getCellId() == neighbouringcell && d.getEnergy() > 0.002); }
                               );
    //Found ECLCalDigit close to ExtHit
    if (idigit != m_eclCalDigits.end()) {
      matchedToNeighbours = 1;
      break;
    }
  }
}
