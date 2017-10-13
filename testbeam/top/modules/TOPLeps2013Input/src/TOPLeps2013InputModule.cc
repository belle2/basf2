/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <testbeam/top/modules/TOPLeps2013Input/TOPLeps2013InputModule.h>
#include <top/geometry/TOPGeometryPar.h>



// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// DataStore classes
#include <framework/dataobjects/EventMetaData.h>
#include <top/dataobjects/TOPDigit.h>
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/ExtHit.h>

#include <TMatrixDSym.h>

using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(TOPLeps2013Input)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPLeps2013InputModule::TOPLeps2013InputModule() : Module()
  {
    // set module description (e.g. insert text)
    setDescription("Input of LEPS 2013 test beam data from a specific root ntuple (top tree)");

    // Add parameters
    addParam("inputFileName", m_inputFileName, "Input file name",
             string(""));
    addParam("ntofMin", m_ntofMin, "minimal TOF hits to select event", 1);
    addParam("ntofMax", m_ntofMax, "maximal TOF hits to select event", 1);
    addParam("rfTimeMin", m_rfTimeMin, "minimal RF time to select event [ns]", 41.0);
    addParam("x0", m_x0, "track starting position in x [cm]", 0.0);
    addParam("y0", m_y0, "track starting position in y [cm]", 0.0);
    addParam("z0", m_z0, "track starting position in z [cm]", 0.0);
    addParam("p", m_p, "track momentum magnitude [GeV/c]", 2.1);
    addParam("theta", m_theta, "track polar angle [deg]", 90.0);
    addParam("phi", m_phi, "track azimuthal angle [deg]", 0.0);
    addParam("t0", m_t0, "common offset to be added to photon TDC times [ns]", 0.0);

    // initialize other private data members
    m_file = NULL;
    m_treeTop = NULL;
    m_treeLeps = NULL;
    m_top.clear();
    m_ntof = 0;

    m_numPMTchannels = 0;
    m_numChannels = 0;
    m_tdcWidth = 0;
    m_tdcOverflow = 0;
    m_timeMax = 0;
    m_numEntries = 0;
    m_entryCounter = 0;

  }

  TOPLeps2013InputModule::~TOPLeps2013InputModule()
  {
  }

  void TOPLeps2013InputModule::initialize()
  {
    if (m_inputFileName.empty()) B2FATAL("file name not given");

    m_file = new TFile(m_inputFileName.c_str());
    if (!m_file->IsOpen()) B2FATAL("can't open file " << m_inputFileName);

    m_treeTop = (TTree*)m_file->Get("top");
    if (!m_treeTop) B2FATAL("No top tree found in file " << m_inputFileName);

    m_treeTop->SetBranchAddress("runNum", &(m_top.runNum));
    m_treeTop->SetBranchAddress("eventNum", &(m_top.eventNum));
    m_treeTop->SetBranchAddress("eventflag", &(m_top.eventflag));
    m_treeTop->SetBranchAddress("eventtag_top", &(m_top.eventtag_top));

    m_treeTop->SetBranchAddress("nhit", &(m_top.nhit));
    m_treeTop->SetBranchAddress("pmt", &(m_top.pmtid_mcp));
    m_treeTop->SetBranchAddress("ch", &(m_top.ch_mcp));
    m_treeTop->SetBranchAddress("tdc0", &(m_top.tdc0_mcp));
    m_treeTop->SetBranchAddress("adc0", &(m_top.adc0_mcp));
    m_treeTop->SetBranchAddress("pmtflag", &(m_top.pmtflag_mcp));

    m_treeTop->SetBranchAddress("rf", &(m_top.rf));
    m_treeTop->SetBranchAddress("rf_time", &(m_top.rf_time));
    m_treeTop->SetBranchAddress("bunchNum", &(m_top.bunchNum));

    m_treeTop->SetBranchAddress("trigS_tdc", &(m_top.trigS_tdc));
    m_treeTop->SetBranchAddress("trigS_adc", &(m_top.trigS_adc));
    m_treeTop->SetBranchAddress("trigM_tdc", &(m_top.trigM_tdc));
    m_treeTop->SetBranchAddress("trigM_adc", &(m_top.trigM_adc));
    m_treeTop->SetBranchAddress("timing_tdc", &(m_top.timing_tdc));
    m_treeTop->SetBranchAddress("timing_adc", &(m_top.timing_adc));
    m_treeTop->SetBranchAddress("veto_adc", &(m_top.veto_adc));
    m_treeTop->SetBranchAddress("ratemon", &(m_top.ratemon));

    m_treeTop->SetBranchAddress("tdc0_ch", &(m_top.tdc0_ch));
    m_treeTop->SetBranchAddress("adc0_ch", &(m_top.adc0_ch));
    m_treeTop->SetBranchAddress("pmtflag_ch", &(m_top.pmtflag_ch));
    m_treeTop->SetBranchAddress("tdc", &(m_top.tdc_mcp));
    m_treeTop->SetBranchAddress("tdc_ch", &(m_top.tdc_ch));

    m_treeTop->SetBranchAddress("trk_x", &(m_top.trk_x));
    m_treeTop->SetBranchAddress("trk_z_x", &(m_top.trk_z_x));
    m_treeTop->SetBranchAddress("trk_qual_x", &(m_top.trk_qual_x));

    m_treeTop->SetBranchAddress("trk_y", &(m_top.trk_y));
    m_treeTop->SetBranchAddress("trk_z_y", &(m_top.trk_z_y));
    m_treeTop->SetBranchAddress("trk_qual_y", &(m_top.trk_qual_y));

    m_treeTop->SetBranchAddress("trk1_x", &(m_top.trk1_x));
    m_treeTop->SetBranchAddress("trk1_y", &(m_top.trk1_y));
    m_treeTop->SetBranchAddress("trk1_z", &(m_top.trk1_z));

    m_treeTop->SetBranchAddress("trk2_x", &(m_top.trk2_x));
    m_treeTop->SetBranchAddress("trk2_y", &(m_top.trk2_y));
    m_treeTop->SetBranchAddress("trk2_z", &(m_top.trk2_z));

    m_treeTop->SetBranchAddress("trk_top_x", &(m_top.trk_top_x));
    m_treeTop->SetBranchAddress("trk_top_y", &(m_top.trk_top_y));
    m_treeTop->SetBranchAddress("trk_top_z", &(m_top.trk_top_z));

    m_treeLeps = (TTree*)m_file->Get("leps");
    if (m_treeLeps) {
      m_treeLeps->SetBranchAddress("ntof", &(m_ntof));
    } else {
      m_ntof = 1;
      B2WARNING("TOPLeps2013Input: leps tree not found, ntof set to 1");
    }

    m_numEntries = m_treeTop->GetEntries();
    m_entryCounter = 0;

    // data store objects registration

    StoreObjPtr<EventMetaData> evtMetaData;
    evtMetaData.registerInDataStore();

    StoreArray<TOPDigit> digits;
    digits.registerInDataStore();

    StoreArray<Track> tracks;
    tracks.registerInDataStore();

    StoreArray<TrackFitResult> trackFitResults;
    trackFitResults.registerInDataStore();

    StoreArray<ExtHit> extHits;
    extHits.registerInDataStore();

    tracks.registerRelationTo(extHits);

  }


  void TOPLeps2013InputModule::beginRun()
  {
  }


  void TOPLeps2013InputModule::event()
  {
    // initialize
    const auto* geo = TOP::TOPGeometryPar::Instance()->getGeometry();
    const auto& pmtArray = geo->getModule(1).getPMTArray();
    m_numPMTchannels = pmtArray.getPMT().getNumPixels();
    m_numChannels = pmtArray.getNumPixels();
    if (m_numChannels > 512) B2FATAL("Number of channels > 512");
    m_tdcWidth = geo->getNominalTDC().getBinWidth() * Unit::ns / Unit::ps;
    m_tdcOverflow = geo->getNominalTDC().getOverflowValue();
    m_timeMax = (float) m_tdcOverflow * m_tdcWidth;

    m_top.clear();

    // create data store objects
    StoreObjPtr<EventMetaData> evtMetaData;
    evtMetaData.create();
    StoreArray<TOPDigit> digits;
    StoreArray<Track> tracks;
    StoreArray<TrackFitResult> trackFitResults;
    StoreArray<ExtHit> extHits;

    // read from ntuple and select event
    bool select = false;
    do {
      if (m_entryCounter == m_numEntries) {
        evtMetaData->setEndOfData(); // stop event processing
        return;
      }
      m_treeTop->GetEntry(m_entryCounter);
      if (m_treeLeps) m_treeLeps->GetEntry(m_entryCounter);
      m_entryCounter++;
      select = m_ntof >= m_ntofMin && m_ntof <= m_ntofMax;
      if (m_treeLeps) select = select && m_top.rf_time * Unit::ps / Unit::ns > m_rfTimeMin;
    } while (!select);

    // set event metadata
    evtMetaData->setEvent(m_top.eventNum);
    evtMetaData->setRun(m_top.runNum);

    // write good data to data store
    for (int i = 0; i < (int)m_top.nhit; i++) {
      if (m_top.pmtflag_mcp[i] != 1) continue;
      double t = m_top.tdc_mcp[i] * Unit::ps / Unit::ns + m_t0;
      if (t < 0) continue;
      if (t > m_timeMax) continue;
      int ich = (m_top.pmtid_mcp[i] - 1) * m_numPMTchannels + m_top.ch_mcp[i];
      int TDC = t / m_tdcWidth;
      if (TDC < m_tdcOverflow)
        digits.appendNew(1, getNewNumbering(ich), t * Unit::ps);
    }

    // write track info to data store
    TVector3 momentum; momentum.SetMagThetaPhi(m_p, m_theta * Unit::deg, m_phi * Unit::deg);
    TVector3 position(m_x0, m_y0, m_z0);
    TMatrixDSym dummyMatrix(6);
    TrackFitResult* trackFitResult = trackFitResults.appendNew(
                                       position,
                                       momentum,
                                       dummyMatrix,
                                       1,                 // charge
                                       Const::electron,   // ParticleType.
                                       0.5,               // p-value.
                                       0.0,               // b-field.
                                       0,                 //HitPatternCDC
                                       0                  //HitPatternVXD
                                     );


    Track* track = tracks.appendNew();
    short index = trackFitResult->getArrayIndex();
    track->setTrackFitResultIndex(Const::electron, index);
    track->setTrackFitResultIndex(Const::muon, index);
    track->setTrackFitResultIndex(Const::pion, index);
    track->setTrackFitResultIndex(Const::kaon, index);
    track->setTrackFitResultIndex(Const::proton, index);

    // extrapolate track to bar (to the plane at x = inner radius)
    TVector3 direction = momentum.Unit();
    int barID = 1;
    double x = geo->getInnerRadius();
    double path = (x - position.X()) / direction.X();
    double y = position.Y() + path * direction.Y();
    double z = position.Z() + path * direction.Z();
    TVector3 hit(x, y, z);
    int pdgCode = Const::pion.getPDGCode();
    double mass = Const::pion.getMass();
    double beta = m_p / sqrt(m_p * m_p + mass * mass);
    double tof = path / (beta * Const::speedOfLight);
    TMatrixDSym covariance(6);
    ExtHit* extHit = extHits.appendNew(pdgCode,
                                       Const::EDetector::TOP,
                                       barID,
                                       EXT_ENTER,
                                       tof,
                                       hit,
                                       momentum,
                                       covariance);
    track->addRelationTo(extHit);

    B2INFO("run " << evtMetaData->getRun()
           << " event " << evtMetaData->getEvent()
           << ": ntof=" << m_ntof
           << " nhit=" << digits.getEntries()
           << " rf_time=" << m_top.rf_time << " ps");
  }


  void TOPLeps2013InputModule::endRun()
  {
  }

  void TOPLeps2013InputModule::terminate()
  {
  }

  int TOPLeps2013InputModule::getNewNumbering(int pixelID) const
  {
    if (pixelID == 0) return 0;

    const auto* geo = TOP::TOPGeometryPar::Instance()->getGeometry();
    const auto& pmtArray = geo->getModule(1).getPMTArray();
    int Npmtx = pmtArray.getNumColumns();
    int Npadx = pmtArray.getPMT().getNumColumns();
    int Npady = pmtArray.getPMT().getNumRows();

    pixelID--;
    int ix = pixelID % Npadx;
    pixelID /= Npadx;
    int iy = pixelID % Npady;
    pixelID /= Npady;
    int ipmtx = pixelID % Npmtx;
    int ipmty = pixelID / Npmtx;

    ix = Npadx - 1 - ix;
    ipmtx = Npmtx - 1 - ipmtx;

    int i = ix + ipmtx * Npadx;
    int j = iy + ipmty * Npady;
    int nx = Npmtx * Npadx;
    return i + j * nx + 1;

  }


} // end Belle2 namespace

