/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <background/modules/BeamBkgGenerator/BeamBkgGeneratorModule.h>

// framework - DataStore
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/GearDir.h>

// data objects
#include <framework/dataobjects/EventMetaData.h>
#include <mdst/dataobjects/MCParticle.h>
#include <generators/SAD/dataobjects/SADMetaHit.h>

// random generator
#include <TRandom.h>

using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(BeamBkgGenerator)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  BeamBkgGeneratorModule::BeamBkgGeneratorModule() : Module()

  {
    // set module description
    setDescription("Beam background generator based on SAD files. "
                   "The generator picks up particles from the SAD file randomly "
                   "according to their rates. "
                   "Number of events is determined from 'realTime' and overall rate, and "
                   "the generator terminates the execution when this number is reached.");

    // Add parameters
    addParam("fileName", m_fileName, "name of the SAD file converted to root");
    addParam("treeName", m_treeName, "name of the TTree in the SAD file", string("sad"));
    addParam("ringName", m_ringName, "name of the superKEKB ring (LER or HER)");
    addParam("realTime", m_realTime,
             "equivalent superKEKB running time to generate sample [ns].");

    //m_readerSAD = new ReaderSAD();
  }

  BeamBkgGeneratorModule::~BeamBkgGeneratorModule()
  {
  }

  void BeamBkgGeneratorModule::initialize()
  {
    // register MCParticles

    StoreArray<MCParticle> mcParticles;
    mcParticles.registerInDataStore();

    StoreArray<SADMetaHit> sadHits;
    sadHits.registerInDataStore();


    // check steering parameters

    if (m_ringName != "LER" and m_ringName != "HER") {
      B2ERROR("ringName can only be LER or HER");
    }
    if (m_realTime <= 0) {
      B2ERROR("realTime must be positive");
    }

    // open SAD file and get TTree

    m_file = TFile::Open(m_fileName.c_str());
    if (!m_file) {
      B2ERROR(m_fileName << ": can't open file");
      return;
    }
    m_tree = (TTree*) m_file->Get(m_treeName.c_str());
    if (!m_tree) {
      B2ERROR(m_treeName << ": no such TTree in the SAD file");
      return;
    }

    m_tree->SetBranchAddress("s", &m_sad.s);
    m_tree->SetBranchAddress("x", &m_sad.x);
    m_tree->SetBranchAddress("px", &m_sad.px);
    m_tree->SetBranchAddress("y", &m_sad.y);
    m_tree->SetBranchAddress("py", &m_sad.py);
    m_tree->SetBranchAddress("E", &m_sad.E);
    m_tree->SetBranchAddress("rate", &m_sad.rate);

    // for SADMetaHit
    m_tree->SetBranchAddress("ss", &m_sad.ss);
    m_tree->SetBranchAddress("sraw", &m_sad.sraw);
    m_tree->SetBranchAddress("ssraw", &m_sad.ssraw);
    m_tree->SetBranchAddress("nturn", &m_sad.nturn);
    m_tree->SetBranchAddress("xraw", &m_sad.xraw);
    m_tree->SetBranchAddress("yraw", &m_sad.yraw);
    m_tree->SetBranchAddress("r", &m_sad.r);
    m_tree->SetBranchAddress("rr", &m_sad.rr);
    m_tree->SetBranchAddress("dp_over_p0", &m_sad.dp_over_p0);
    m_tree->SetBranchAddress("watt", &m_sad.watt);

    int numEntries = m_tree->GetEntries();
    if (numEntries <= 0) {
      B2ERROR("SAD tree is empty");
      return;
    }

    // construct vector of cumulative rates and determine number of events to generate

    m_rates.push_back(0);
    for (int i = 0; i < numEntries; i++) {
      m_tree->GetEntry(i);
      m_rates.push_back(m_sad.rate + m_rates.back());
      m_counters.push_back(0);
    }

    m_numEvents = gRandom->Poisson(m_rates.back() * m_realTime / Unit::s);

    // set rotation from SAD to Belle II

    if (m_ringName == "LER") {
      GearDir ring("/Detector/SuperKEKB/LER/");
      m_rotation.RotateY(ring.getAngle("angle"));
      m_ring = 2;
    } else {
      GearDir ring("/Detector/SuperKEKB/HER/");
      m_rotation.RotateY(ring.getAngle("angle"));
      m_ring = 1;
    }

    m_sectionOrdering.insert(m_sectionOrdering.end(), {1, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2});

    B2RESULT("BG rate: " << m_rates.back() / 1e6 << " MHz, events to generate: "
             << m_numEvents);

  }

  void BeamBkgGeneratorModule::beginRun()
  {
  }

  void BeamBkgGeneratorModule::event()
  {

    // check event counter

    if (m_eventCounter >= m_numEvents) {
      StoreObjPtr<EventMetaData> eventMetaData;
      eventMetaData->setEndOfData(); // stop event processing
      return;
    }
    m_eventCounter++;

    // get SAD entry

    int i = generateEntry();
    m_tree->GetEntry(i);
    m_counters[i]++;

    // make SADMetaHit
    int ring_section = -1;
    double ssraw = m_sad.ss;
    if (m_sad.ss < 0) ssraw += 3016.;
    int section = (int)(ssraw * 12 / 3016.);
    if ((unsigned)section < m_sectionOrdering.size()) ring_section = m_sectionOrdering[section];

    StoreArray<SADMetaHit> SADMetaHits;
    SADMetaHits.appendNew(SADMetaHit(m_sad.ssraw, m_sad.sraw, m_sad.ss, m_sad.s,
                                     0., m_sad.nturn,
                                     m_sad.x, m_sad.y, m_sad.px, m_sad.py, m_sad.xraw, m_sad.yraw,
                                     m_sad.r, m_sad.rr, m_sad.dp_over_p0, m_sad.E, m_sad.rate,
                                     m_sad.watt, m_ring, ring_section));


    // transform to Belle II (flip sign of y and s, rotate)

    TVector3 position(m_sad.x * Unit::m, -m_sad.y * Unit::m, -m_sad.s * Unit::m);
    position.Transform(m_rotation);

    double pz = sqrt(m_sad.E * m_sad.E - m_sad.px * m_sad.px - m_sad.py * m_sad.py);
    if (m_ringName == "LER") pz = -pz;
    TVector3 momentum(m_sad.px, -m_sad.py, pz);
    momentum.Transform(m_rotation);

    // PDG code and mass

    int pdgCode = Const::electron.getPDGCode();
    double mass = Const::electron.getMass();
    if (m_ringName == "LER") pdgCode = -pdgCode;

    // append SAD particle to MCParticles

    StoreArray<MCParticle> MCParticles;
    MCParticle* part = MCParticles.appendNew();
    part->setPDG(pdgCode);
    part->setMass(mass);
    part->setProductionVertex(position);
    part->setProductionTime(0);
    part->setMomentum(momentum);
    part->setEnergy(sqrt(momentum.Mag2() + mass * mass));
    part->setValidVertex(true);
    part->setStatus(MCParticle::c_PrimaryParticle);
    part->addStatus(MCParticle::c_StableInGenerator);

    // FarBeamLine region transformation
    /*
    if (abs(m_sad.s) > 4.) { // [m]
      double particlePosSADfar[] = {m_sad.x,-m_sad.y,0};
      double particlePosGeant4[] = {0.0, 0.0, 0.0};
      double particleMomSADfar[] = {m_sad.px,-m_sad.py,pz};
      double particleMomGeant4[] = {0.0, 0.0, 0.0};

      if (m_ringName == "LER") {
        m_transMatrix = new TGeoHMatrix(m_readerSAD->SADtoGeant(m_readerSAD->c_LER,m_sad.s));
      } else {
        m_transMatrix = new TGeoHMatrix(m_readerSAD->SADtoGeant(m_readerSAD->c_HER,m_sad.s));
      }

      m_transMatrix->LocalToMaster(particlePosSADfar, particlePosGeant4);
      m_transMatrix->LocalToMasterVect(particleMomSADfar, particleMomGeant4);

      part->setMomentum(TVector3(particleMomGeant4));
      part->setProductionVertex(TVector3(particlePosGeant4));
    }
    */
  }


  void BeamBkgGeneratorModule::endRun()
  {
  }

  void BeamBkgGeneratorModule::terminate()
  {
    // close SAD file
    m_file->Close();

    B2RESULT("BG rate: " << m_rates.back() / 1e6 << " MHz, equivalent time: "
             << m_realTime / Unit::us << " us, "
             << m_eventCounter << " events generated");
    if (m_eventCounter != m_numEvents)
      B2ERROR("Number of generated events does not match the equivalent running time: "
              << m_numEvents << " events needed, but "
              << m_eventCounter << " generated");

    int imax = 0;
    double average = 0;
    for (unsigned i = 0; i < m_counters.size(); i++) {
      if (m_counters[i] > m_counters[imax]) imax = i;
      average += m_counters[i];
    }
    average /= m_counters.size();
    B2RESULT("SAD particle usage: on average " << average << " times, max "
             << m_counters[imax] << " times (entry " << imax << ")");

  }


  int BeamBkgGeneratorModule::generateEntry() const
  {
    double rate = gRandom->Uniform(m_rates.back());
    int i1 = 0;
    int i2 = m_rates.size() - 1;
    while (i2 - i1 > 1) {
      int i = (i1 + i2) / 2;
      if (rate > m_rates[i]) {
        i1 = i;
      } else {
        i2 = i;
      }
    }
    return i1;
  }

  TGeoHMatrix BeamBkgGeneratorModule::SADtoGeant(TString accRing, double s)
  {
    // 0<sraw<3016.3145 m
    // -1500<s<1500 m

    //static double max_s_her = 3016.3145 * Unit::m;
    //static double max_s_ler = 3016.3026 * Unit::m;

    //get parameters from .xml file
    static GearDir content = Gearbox::getInstance().getDetectorComponent("FarBeamLine");

    map<string, straightElement> straights;
    map<string, bendingElement> bendings;
    for (const GearDir& element : content.getNodes("Straight")) {

      string name = element.getString("@name");
      string type = element.getString("@type");

      if (type != "pipe") continue;

      straightElement straight;

      straight.x0 = element.getLength("X0");
      straight.z0 = element.getLength("Z0");
      straight.l = element.getLength("L");
      straight.phi = element.getLength("PHI");

      straights[name] = straight;
    }

    string str_checklist[] = {"LHR1", "LHR2", "LLR1", "LLR2", "LLR3", "LLR4", "LLR5", "LHL1", "LHL2", "LLL1", "LLL2", "LLL3", "LLL4"};
    for (const string& str : str_checklist) {
      if (straights.count(str) == 0)
        B2FATAL("You need FarBeamLine.xml to run SADInput module. Please include FarBeamLine.xml in Belle2.xml. You also need to change 'length' in Belle2.xml to be 40m.");
    }

    for (const GearDir& element : content.getNodes("Bending")) {

      string name = element.getString("@name");
      string type = element.getString("@type");

      if (type != "pipe") continue;

      bendingElement bending;

      bending.rt = element.getLength("RT");
      bending.x0 = element.getLength("X0");
      bending.z0 = element.getLength("Z0");
      bending.sphi = element.getLength("SPHI");
      bending.dphi = element.getLength("DPHI");

      bendings[name] = bending;
    }

    string bend_checklist[] = {"BLC2RE", "BC1RP", "BLCWRP", "BLC1RP", "BLC2RP", "BLC1LE", "BC1LP", "BLC1LP", "BLC2LP"};
    for (const string& bnd : bend_checklist) {
      if (bendings.count(bnd) == 0)
        B2FATAL("You need FarBeamLine.xml to run SADInput module. Please include FarBeamLine.xml in Belle2.xml. You also need to change 'length' in Belle2.xml to be 40m.");
    }

    static double her_breakpoints[6];
    static double ler_breakpoints[16];

    // positive s
    her_breakpoints[0] = straights["LHL1"].l;
    her_breakpoints[1] = her_breakpoints[0] + bendings["BLC1LE"].rt * bendings["BLC1LE"].dphi;
    her_breakpoints[2] = her_breakpoints[1] + straights["LHL2"].l;

    // negative s
    her_breakpoints[3] = -straights["LHR1"].l;
    her_breakpoints[4] = her_breakpoints[3] - bendings["BLC2RE"].rt * bendings["BLC2RE"].dphi;
    her_breakpoints[5] = her_breakpoints[4] - straights["LHR2"].l;

    // positive s
    ler_breakpoints[0] = straights["LLL1"].l;
    ler_breakpoints[1] = ler_breakpoints[0] + bendings["BC1LP"].rt * bendings["BC1LP"].dphi;
    ler_breakpoints[2] = ler_breakpoints[1] + straights["LLL2"].l;
    ler_breakpoints[3] = ler_breakpoints[2] + bendings["BLC1LP"].rt * bendings["BLC1LP"].dphi;
    ler_breakpoints[4] = ler_breakpoints[3] + straights["LLL3"].l;
    ler_breakpoints[5] = ler_breakpoints[4] + bendings["BLC2LP"].rt * bendings["BLC2LP"].dphi;
    ler_breakpoints[6] = ler_breakpoints[5] + straights["LLL4"].l;

    // negative s
    ler_breakpoints[7] = -straights["LLR1"].l;
    ler_breakpoints[8] = ler_breakpoints[7] - bendings["BC1RP"].rt * bendings["BC1RP"].dphi;
    ler_breakpoints[9] = ler_breakpoints[8] - straights["LLR2"].l;
    ler_breakpoints[10] = ler_breakpoints[9] - bendings["BLCWRP"].rt * bendings["BLCWRP"].dphi;
    ler_breakpoints[11] = ler_breakpoints[10] - straights["LLR3"].l;
    ler_breakpoints[12] = ler_breakpoints[11] - bendings["BLC1RP"].rt * bendings["BLC1RP"].dphi;
    ler_breakpoints[13] = ler_breakpoints[12] - straights["LLR4"].l;
    ler_breakpoints[14] = ler_breakpoints[13] - bendings["BLC2RP"].rt * bendings["BLC2RP"].dphi;
    ler_breakpoints[15] = ler_breakpoints[14] - straights["LLR5"].l;

    double dx = 0;
    double dz = 0;
    double phi = 0;
    if (accRing == "LER") {
      // LER
      // positive s
      if (400.0 * Unit::cm < s) {
        if (s < ler_breakpoints[0]) {
          phi = straights["LLL1"].phi;
          dx = straights["LLL1"].x0 + s * sin(phi);
          dz = straights["LLL1"].z0 + s * cos(phi);
        } else if (s < ler_breakpoints[1]) {
          double sloc = s - ler_breakpoints[0];
          phi = bendings["BC1LP"].sphi + sloc / bendings["BC1LP"].rt;
          // Torus is created in x-y plain.
          // It is then rotated to x-z plain,
          // and its direction changes to reversed,
          // thus phi_real=-phi_xml
          phi = -phi;
          dx = bendings["BC1LP"].x0 + bendings["BC1LP"].rt * cos(-phi);
          dz = bendings["BC1LP"].z0 + bendings["BC1LP"].rt * sin(-phi);
        } else if (s < ler_breakpoints[2]) {
          double sloc = s - ler_breakpoints[1];
          phi = straights["LLL2"].phi;
          dx = straights["LLL2"].x0 + sloc * sin(phi);
          dz = straights["LLL2"].z0 + sloc * cos(phi);
        } else if (s < ler_breakpoints[3]) {
          double sloc = s - ler_breakpoints[2];
          phi = bendings["BLC1LP"].sphi + sloc / bendings["BLC1LP"].rt;
          phi = -phi;
          dx = bendings["BLC1LP"].x0 + bendings["BLC1LP"].rt * cos(-phi);
          dz = bendings["BLC1LP"].z0 + bendings["BLC1LP"].rt * sin(-phi);
        } else if (s < ler_breakpoints[4]) {
          double sloc = s - ler_breakpoints[3];
          phi = straights["LLL3"].phi;
          dx = straights["LLL3"].x0 + sloc * sin(phi);
          dz = straights["LLL3"].z0 + sloc * cos(phi);
        } else if (s < ler_breakpoints[5]) {
          double sloc = s - ler_breakpoints[4];
          // Torus dphi may be only positive,
          // while direction of increasing |s| is sometimes negative,
          // and we need to use -s and not change phi.
          // Since we add pi to phi later,
          // we subtract it now for this element.
          phi = bendings["BLC2LP"].sphi + bendings["BLC2LP"].dphi - sloc / bendings["BLC2LP"].rt;
          phi = -phi;
          dx = bendings["BLC2LP"].x0 + bendings["BLC2LP"].rt * cos(-phi);
          dz = bendings["BLC2LP"].z0 + bendings["BLC2LP"].rt * sin(-phi);
          phi -= M_PI;
        } else if (s < ler_breakpoints[6]) {
          double sloc = s - ler_breakpoints[5];
          phi = straights["LLL4"].phi;
          dx = straights["LLL4"].x0 + sloc * sin(phi);
          dz = straights["LLL4"].z0 + sloc * cos(phi);
        }
        // For this direction rotation angle of elements changes to negative,
        // while SAD coordinates keep orientation.
        // We need to compensate.
        phi += M_PI;
      }
      // negative s
      else if (s < -400.0 * Unit::cm) {
        if (s > ler_breakpoints[7]) {
          double sloc = -s;
          phi = straights["LLR1"].phi;
          dx = straights["LLR1"].x0 + sloc * sin(phi);
          dz = straights["LLR1"].z0 + sloc * cos(phi);
        } else if (s > ler_breakpoints[8]) {
          double sloc = ler_breakpoints[7] - s;
          phi = bendings["BC1RP"].sphi + bendings["BC1RP"].dphi - sloc / bendings["BC1RP"].rt;
          phi = -phi;
          dx = bendings["BC1RP"].x0 + bendings["BC1RP"].rt * cos(-phi);
          dz = bendings["BC1RP"].z0 + bendings["BC1RP"].rt * sin(-phi);
          phi += M_PI;
        } else if (s > ler_breakpoints[9]) {
          double sloc = ler_breakpoints[8] - s;
          phi = straights["LLR2"].phi;
          dx = straights["LLR2"].x0 + sloc * sin(phi);
          dz = straights["LLR2"].z0 + sloc * cos(phi);
        } else if (s > ler_breakpoints[10]) {
          double sloc = ler_breakpoints[9] - s;
          phi = bendings["BLCWRP"].sphi + bendings["BLCWRP"].dphi - sloc / bendings["BLCWRP"].rt;
          phi = -phi;
          dx = bendings["BLCWRP"].x0 + bendings["BLCWRP"].rt * cos(-phi);
          dz = bendings["BLCWRP"].z0 + bendings["BLCWRP"].rt * sin(-phi);
          phi += M_PI;
        } else if (s > ler_breakpoints[11]) {
          double sloc = ler_breakpoints[10] - s;
          phi = straights["LLR3"].phi;
          dx = straights["LLR3"].x0 + sloc * sin(phi);
          dz = straights["LLR3"].z0 + sloc * cos(phi);
        } else if (s > ler_breakpoints[12]) {
          double sloc = ler_breakpoints[11] - s;
          phi = bendings["BLC1RP"].sphi + bendings["BLC1RP"].dphi - sloc / bendings["BLC1RP"].rt;
          phi = -phi;
          dx = bendings["BLC1RP"].x0 + bendings["BLC1RP"].rt * cos(-phi);
          dz = bendings["BLC1RP"].z0 + bendings["BLC1RP"].rt * sin(-phi);
          phi += M_PI;
        } else if (s > ler_breakpoints[13]) {
          double sloc = ler_breakpoints[12] - s;
          phi = straights["LLR4"].phi;
          dx = straights["LLR4"].x0 + sloc * sin(phi);
          dz = straights["LLR4"].z0 + sloc * cos(phi);
        } else if (s > ler_breakpoints[14]) {
          double sloc = ler_breakpoints[13] - s;
          phi = bendings["BLC2RP"].sphi + sloc / bendings["BLC2RP"].rt;
          phi = -phi;
          dx = bendings["BLC2RP"].x0 + bendings["BLC2RP"].rt * cos(-phi);
          dz = bendings["BLC2RP"].z0 + bendings["BLC2RP"].rt * sin(-phi);
        } else if (s > ler_breakpoints[15]) {
          double sloc = ler_breakpoints[14] - s;
          phi = straights["LLR5"].phi;
          dx = straights["LLR5"].x0 + sloc * sin(phi);
          dz = straights["LLR5"].z0 + sloc * cos(phi);
        }
      }
    }
    if (accRing == "HER") {
      // HER
      // positive s
      if (400.0 * Unit::cm < s) {
        if (s < her_breakpoints[0]) {
          phi = straights["LHL1"].phi;
          dx = straights["LHL1"].x0 + s * sin(phi);
          dz = straights["LHL1"].z0 + s * cos(phi);
        } else if (s < her_breakpoints[1]) {
          double sloc = s - her_breakpoints[0];
          phi = bendings["BLC1LE"].sphi + sloc / bendings["BLC1LE"].rt;
          phi = -phi;
          dx = bendings["BLC1LE"].x0 + bendings["BLC1LE"].rt * cos(-phi);
          dz = bendings["BLC1LE"].z0 + bendings["BLC1LE"].rt * sin(-phi);
        } else if (s < her_breakpoints[2]) {
          double sloc = s - her_breakpoints[1];
          phi = straights["LHL2"].phi;
          dx = straights["LHL2"].x0 + sloc * sin(phi);
          dz = straights["LHL2"].z0 + sloc * cos(phi);
        }
        phi += M_PI;
      }
      // negative s
      else if (s < -400.0 * Unit::cm) {
        if (s > her_breakpoints[3]) {
          double sloc = -s;
          phi = straights["LHR1"].phi;
          dx = straights["LHR1"].x0 + sloc * sin(phi);
          dz = straights["LHR1"].z0 + sloc * cos(phi);
        } else if (s > her_breakpoints[4]) {
          double sloc = her_breakpoints[3] - s;
          phi = bendings["BLC2RE"].sphi + sloc / bendings["BLC2RE"].rt;
          phi = -phi;
          dx = bendings["BLC2RE"].x0 + bendings["BLC2RE"].rt * cos(-phi);
          dz = bendings["BLC2RE"].z0 + bendings["BLC2RE"].rt * sin(-phi);
        } else if (s > her_breakpoints[5]) {
          double sloc = her_breakpoints[4] - s;
          phi = straights["LHR2"].phi;
          dx = straights["LHR2"].x0 + sloc * sin(phi);
          dz = straights["LHR2"].z0 + sloc * cos(phi);
        }
      }
    }

    TGeoHMatrix matrix("SADTrafo");
    matrix.RotateY(phi / Unit::deg);
    matrix.SetDx(dx);
    matrix.SetDz(dz);
    return matrix;
  }


} // end Belle2 namespace

