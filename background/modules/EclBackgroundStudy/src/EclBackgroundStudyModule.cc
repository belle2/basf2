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
    m_tree1->Branch("CellId", &m_CellId);
    m_tree1->Branch("TcId", &m_TcId);
    m_tree1->Branch("Edep", &m_Edep);
    m_tree1->Branch("TimeAve", &m_TimeAve);

    m_tree2->Branch("CellId", &m_CellId);
    m_tree2->Branch("TcId", &m_TcId);
    m_tree2->Branch("Pdg", &m_Pdg);
    m_tree2->Branch("FlightTime", &m_FlightTime);
    m_tree2->Branch("Edep", &m_Edep);
    m_tree2->Branch("Hadronedep", &m_Hadronedep);

    m_TCMap = new TrgEclMapping();
  }

  void EclBackgroundStudyModule::beginRun()
  {
    // Print run number
    B2INFO("EclBackgroundStudy: Processing. ");
  }

  void EclBackgroundStudyModule::event()
  {
    m_CellId.clear();
    m_TcId.clear();
    m_Pdg.clear();
    m_Edep.clear();
    m_TimeAve.clear();
    m_FlightTime.clear();
    m_Hadronedep.clear();

    // loop over ECLHits
    for (const ECLHit& hit : m_ECLHits) {
      m_CellId.push_back(hit.getCellId());
      m_TcId.push_back(m_TCMap->getTCIdFromXtalId(hit.getCellId()));
      m_Edep.push_back(hit.getEnergyDep());
      m_TimeAve.push_back(hit.getTimeAve());
    }
    // fill the tree
    m_tree1->Fill();

    m_CellId.clear();
    m_TcId.clear();
    m_Pdg.clear();
    m_Edep.clear();
    m_TimeAve.clear();
    m_FlightTime.clear();
    m_Hadronedep.clear();

    // loop over ECLSimHits
    for (const ECLSimHit& hit : m_ECLSimHits) {
      m_CellId.push_back(hit.getCellId());
      m_TcId.push_back(m_TCMap->getTCIdFromXtalId(hit.getCellId()));
      m_Pdg.push_back(hit.getPDGCode());
      m_FlightTime.push_back(hit.getFlightTime());
      m_Edep.push_back(hit.getEnergyDep());
      m_Hadronedep.push_back(hit.getHadronEnergyDep());
    }
    // fill the tree
    m_tree2->Fill();

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
    B2INFO("EclBackgroundStudy: output file name = " << m_filename);
  }
} // end Belle2 namespace
