#include <background/modules/EclBackgroundStudy/EclBackgroundStudyModule.h>

using namespace std;
using namespace boost;

namespace Belle2 {
  //-----------------------------------------------------------------
  ///                Register the Module
  //-----------------------------------------------------------------

  REG_MODULE(EclBackgroundStudy);


  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  EclBackgroundStudyModule::EclBackgroundStudyModule() : Module(), m_iEntry(0), m_ff(NULL), m_tree1(NULL), m_tree2(NULL),
    m_TCMap(NULL)
  {
    // Set description()
    setDescription("EclBackgroundStudy module. Used to extract information relevant for the ECL background from background files");

    // Add parameters
    addParam("FileName", m_filename, "output file name", string("mytree.root"));
  }

  EclBackgroundStudyModule::~EclBackgroundStudyModule()
  {
  }

  void EclBackgroundStudyModule::initialize()
  {
    // Print set parameters
    printModuleParams();

    // create/open the file
    m_ff = new TFile(m_filename.c_str(), "RECREATE");

    // create trees
    m_tree1 = new TTree("tree1", "ECLHits data");
    m_tree2 = new TTree("tree2", "ECLSimHits data");

    // create branches
    m_tree1->Branch("CellId", &m_CellId, "CellId/I");
    m_tree1->Branch("TcId", &m_TcId, "TcId/I");
    m_tree1->Branch("Edep", &m_Edep, "Edep/D");
    m_tree1->Branch("TimeAve", &m_TimeAve, "TimeAve/D");

    m_tree2->Branch("CellId", &m_CellId, "CellId/I");
    m_tree2->Branch("TcId", &m_TcId, "TcId/I");
    m_tree2->Branch("Pdg", &m_Pdg, "Pdg/I");
    m_tree2->Branch("FlightTime", &m_FlightTime, "FlightTime/D");
    m_tree2->Branch("Edep", &m_Edep, "Edep/D");
    m_tree2->Branch("Hadronedep", &m_Hadronedep, "Hadronedep/D");

    m_TCMap = new TrgEclMapping();
  }

  void EclBackgroundStudyModule::beginRun()
  {
    // Print run number
    B2INFO("BeamBkgNeutron: Processing. ");
  }

  void EclBackgroundStudyModule::event()
  {
    // loop over ECLHits
    for (const ECLHit& hit : m_ECLHits) {
      m_CellId = hit.getCellId();
      m_TcId = m_TCMap->getTCIdFromXtalId(m_CellId);
      m_Edep = hit.getEnergyDep();
      m_TimeAve = hit.getTimeAve();
      // fill the tree
      m_tree1->Fill();
    }

    // loop over ECLSimHits
    for (const ECLSimHit& hit : m_ECLSimHits) {
      m_CellId = hit.getCellId();
      m_TcId = m_TCMap->getTCIdFromXtalId(m_CellId);
      m_Pdg = hit.getPDGCode();
      m_FlightTime = hit.getFlightTime();
      m_Edep = hit.getEnergyDep();
      m_Hadronedep = hit.getHadronEnergyDep();
      // fill the tree
      m_tree2->Fill();
    }

    // increase the entry counter
    m_iEntry++;
  }

  void EclBackgroundStudyModule::endRun()
  {
  }

  void EclBackgroundStudyModule::terminate()
  {
    // CPU time end

    // Announce
    B2INFO("EclBackgroundStudy finished.");
    B2INFO("nEntries = " << m_iEntry);

    // write
    m_ff->cd();
    m_tree1->Write();
    m_tree2->Write();
    // close the tree
    m_ff->Close();
  }

  void EclBackgroundStudyModule::printModuleParams() const
  {
    B2INFO("BeamBkgNeutron: output file name = " << m_filename);
  }
} // end Belle2 namespace
