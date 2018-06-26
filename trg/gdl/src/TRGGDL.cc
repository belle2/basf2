//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TRGGDL.cc
// Section  : TRG GDL
// Owner    : Yoshihito Iwasaki, Chunhua Li
// Email    : yoshihito.iwasaki@kek.jp, chunha.li@unimelb.edu.au
//-----------------------------------------------------------------------------
// Description : A class to represent GDL.
//-----------------------------------------------------------------------------
// 0.00 : 2013/12/13 : First version
//-----------------------------------------------------------------------------

#define TRG_SHORT_NAMES
#define TRGGDL_SHORT_NAMES

#include <TRandom.h>
#include <fstream>
#include <stdio.h>
#include "trg/trg/Debug.h"
#include "trg/trg/Time.h"
#include "trg/trg/State.h"
#include "trg/trg/Signal.h"
#include "trg/trg/Channel.h"
#include "trg/trg/Utilities.h"
#include "trg/gdl/TRGGDL.h"
// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <mdst/dataobjects/TRGSummary.h>
#include <trg/grl/dataobjects/TRGGRLInfo.h>

#include <framework/logging/Logger.h>

#define N_TIMING_REGISTERS 4

using namespace std;

namespace Belle2 {

  void ftd_0_01(bool* b, bool* i);
  void dotrigger(std::vector<int>& res, std::vector<int> ob);

  TRGGDL*
  TRGGDL::_gdl = 0;

  void
  (* TRGGDL::_ftd)(bool* b, bool* i) = 0;

  string
  TRGGDL::name(void) const
  {
    return "TRGGDL";
  }

  string
  TRGGDL::version(void) const
  {
    return string("TRGGDL 0.01");
  }

  TRGGDL*
  TRGGDL::getTRGGDL(const string& configFile,
                    unsigned simulationMode,
                    unsigned fastSimulationMode,
                    unsigned firmwareSimulationMode,
                    const std::string& Phase)
  {
    if (_gdl) {
      //delete _gdl;
      _gdl = 0;
    }

    if (configFile != "good-bye") {
      _gdl = new TRGGDL(configFile,
                        simulationMode,
                        fastSimulationMode,
                        firmwareSimulationMode,
                        Phase);
    } else {
      cout << "TRGGDL::getTRGGDL ... good-bye" << endl;
      //        delete _gdl;
      _gdl = 0;
    }

    return _gdl;
  }

  TRGGDL*
  TRGGDL::getTRGGDL(void)
  {
    if (! _gdl)
      cout << "TRGGDL::getTRGGDL !!! TRGGDL is not created yet" << endl;
    return _gdl;
  }

  TRGGDL::TRGGDL(const string& configFile,
                 unsigned simulationMode,
                 unsigned fastSimulationMode,
                 unsigned firmwareSimulationMode,
                 const std::string& Phase)
    : _debugLevel(0),
      _configFilename(configFile),
      _simulationMode(simulationMode),
      _fastSimulationMode(fastSimulationMode),
      _firmwareSimulationMode(firmwareSimulationMode),
      _Phase(Phase),
      _clock(Belle2_GDL::GDLSystemClock),
      _offset(15.3),
      _isb(0),
      _osb(0)
  {

    if (TRGDebug::level()) {
      cout << "TRGGDL ... TRGGDL initializing with " << _configFilename
           << endl
           << "           mode=0x" << hex << _simulationMode << dec << endl;
    }

    initialize();

    if (TRGDebug::level()) {
      cout << "TRGGDL ... TRGGDL created with " << _configFilename << endl;
      _clock.dump();
    }
  }

  void
  TRGGDL::initialize(void)
  {
    //if it is firmware simulation, do the cofigurnation
    //fastsimulation doesn't use the configuration currently
    if (_simulationMode == 2) configure();
  }

  void
  TRGGDL::terminate(void)
  {
  }

  void
  TRGGDL::dump(const string& msg) const
  {
    if (msg.find("name")    != string::npos ||
        msg.find("version") != string::npos ||
        msg.find("detail")  != string::npos ||
        msg == "") {
    }
    if (msg.find("detail") != string::npos ||
        msg.find("state") != string::npos) {
      cout << "Debug Level=" << _debugLevel;
      cout << endl;
    }
  }

  void
  TRGGDL::clear(void)
  {
  }

  void
  TRGGDL::fastClear(void)
  {
  }

  void
  TRGGDL::update(bool)
  {
    TRGDebug::enterStage("TRGGDL update");

    if (TRGDebug::level())
      cout << TRGDebug::tab() << name() << " do nothing..." << endl;

    TRGDebug::leaveStage("TRGGDL update");
  }

  TRGGDL::~TRGGDL()
  {
    clear();
  }

  void
  TRGGDL::simulate(void)
  {
    const bool fast = (_simulationMode & 1);
    const bool firm = (_simulationMode & 2);
    if (fast)
      fastSimulation();
    if (firm)
      firmwareSimulation();
  }

  void
  TRGGDL::fastSimulation(void)
  {
    TRGDebug::enterStage("TRGGDL fastSim");

    if (TRGDebug::level())
      cout << TRGDebug::tab() << name() << " do nothing..." << endl;

    TRGDebug::leaveStage("TRGGDL fastSim");

    StoreObjPtr<TRGSummary> GDLResult;
    if (GDLResult) {
      B2WARNING("TRGSummary exist already, check it!!!!");
      return;
    } else {
      //TRGGDLResults* GDLResult = L1TrgResults.appendNew();
      GDLResult.create();
      StoreObjPtr<TRGGRLInfo> grlinfo("TRGGRLObjects");
      if (!grlinfo) {
        B2WARNING("TRGGRLInfo doesn't exist!!!!");
        return;
      }

      //get the objects defined in GRL, the name have to be unified to the name in TriggerMenufile
      std::vector<int> obj;
      int nTrk3D = (grlinfo->getN3Dfittertrk()); //0
      int nTrkZ10 = (grlinfo->getN3DfittertrkZ10()); //1
      int nTrkZ25 = (grlinfo->getN3DfittertrkZ25()); //2
      int nClust = (grlinfo->getNcluster()); //3
      int n300MeV = (grlinfo->getNhig300cluster());
      int n1GeV415 = (grlinfo->getNhigh1GeVcluster415()); //4
      int n1GeV2316 = (grlinfo->getNhigh1GeVcluster2316()); //5
      //int n1GeV117 = (grlinfo->getNhigh1GeVcluster117()); //6 // Since it is not used, commanded to reduce warning
      int n2GeV = (grlinfo->getNhigh2GeVcluster()); //7
      int n2GeV414 = (grlinfo->getNhigh2GeVcluster414()); //8
      int n2GeV231516 = (grlinfo->getNhigh2GeVcluster231516()); //9
      int n2GeV117 = (grlinfo->getNhigh2GeVcluster117()); //10
      int nTrkBhabha = (grlinfo->getBhabhaVeto()); //11
      int nECLBhabha = (grlinfo->geteclBhabhaVeto()); //12
      int nPhiPairHigh = (grlinfo->getPhiPairHigh()); //13
      int nPhiPairLow = (grlinfo->getPhiPairLow()); //14
      int n3DPair = (grlinfo->get3DPair()); //15
      int nSameHem1Trk = (grlinfo->getNSameHem1Trk()); //16
      int nOppHem1Trk = (grlinfo->getNOppHem1Trk()); //17
      int eed = (grlinfo->geteed()); //18
      int fed = (grlinfo->getfed()); //19


      const int ntrg = 20;
      bool passBeforePrescale[ntrg];
      int sf[ntrg];
      bool Phase2 = (_Phase == "Phase2");

      int itrig = 0;
      passBeforePrescale[itrig] = nTrk3D >= 3;
      sf[itrig] = 1;

      itrig = 1;
      if (Phase2) {
        passBeforePrescale[itrig] = nTrk3D == 2 && nTrkZ25 >= 1 && nTrkBhabha == 0;
      } else {
        passBeforePrescale[itrig] = nTrk3D == 2 && nTrkZ10 >= 1 && nTrkBhabha == 0;
      }
      sf[itrig] = 1;

      itrig = 2;
      passBeforePrescale[itrig] = nTrk3D == 2 && nTrkBhabha == 0;
      sf[itrig] = 20;

      itrig = 3;
      passBeforePrescale[itrig] = nTrk3D == 2 && nTrkBhabha > 0;
      if (Phase2) sf[itrig] = 1;
      else sf[itrig] = 2;

      itrig = 4;
      if (Phase2) {
        passBeforePrescale[itrig] = nTrk3D == 1 && nTrkZ25 == 1 && nSameHem1Trk >= 1 && n2GeV == 0;
      } else {
        passBeforePrescale[itrig] = nTrk3D == 1 && nTrkZ10 == 1 && nSameHem1Trk >= 1 && n2GeV == 0;
      }
      sf[itrig] = 1;

      itrig = 5;
      if (Phase2) {
        passBeforePrescale[itrig] = nTrk3D == 1 && nTrkZ25 == 1 && nOppHem1Trk >= 1 && n2GeV == 0;
      } else {
        passBeforePrescale[itrig] = nTrk3D == 1 && nTrkZ10 == 1 && nOppHem1Trk >= 1 && n2GeV == 0;
      }
      sf[itrig] = 1;

      itrig = 6;
      if (Phase2) {
        passBeforePrescale[itrig] = nClust >= 3 && n300MeV >= 1 && nECLBhabha == 0;
      } else {
        passBeforePrescale[itrig] = nClust >= 3 && n300MeV >= 2 && nECLBhabha == 0;
      }
      sf[itrig] = 1;

      itrig = 7;
      passBeforePrescale[itrig] = n2GeV414 >= 1 && nTrkBhabha == 0;
      sf[itrig] = 1;

      itrig = 8;
      passBeforePrescale[itrig] = n2GeV414 >= 1 && nTrkBhabha >= 1;
      if (Phase2)sf[itrig] = 1;
      else sf[itrig] = 2;

      itrig = 9;
      passBeforePrescale[itrig] = n2GeV231516 >= 1 && nTrkBhabha == 0 && nECLBhabha == 0;
      sf[itrig] = 1;

      itrig = 10;
      passBeforePrescale[itrig] = n2GeV231516 >= 1 && (nTrkBhabha >= 1 || nECLBhabha >= 1);
      sf[itrig] = 1;

      itrig = 11;
      passBeforePrescale[itrig] = n2GeV117 >= 1 && nTrkBhabha == 0 && nECLBhabha == 0;
      if (Phase2)sf[itrig] = 10;
      else sf[itrig] = 20;

      itrig = 12;
      passBeforePrescale[itrig] = n2GeV117 >= 1 && (nTrkBhabha >= 1 || nECLBhabha >= 1);
      if (Phase2)sf[itrig] = 10;
      else sf[itrig] = 20;

      itrig = 13;
      passBeforePrescale[itrig] = n1GeV415 == 1 && n300MeV == 1;
      sf[itrig] = 1;

      itrig = 14;
      passBeforePrescale[itrig] = n1GeV2316 == 1 && n300MeV == 1;
      if (Phase2)sf[itrig] = 1;
      else sf[itrig] = 5;

      itrig = 15;
      passBeforePrescale[itrig] = nPhiPairHigh >= 1 && n2GeV == 0;
      sf[itrig] = 1;

      itrig = 16;
      if (Phase2) {
        passBeforePrescale[itrig] = nPhiPairLow >= 1 && n2GeV == 0;
        sf[itrig] = 1;
      } else {
        passBeforePrescale[itrig] = nPhiPairLow >= 1 && n2GeV == 0 && nTrkZ25 == nTrk3D;
        sf[itrig] = 3;
      }

      itrig = 17;
      passBeforePrescale[itrig] = n3DPair >= 1 && n2GeV == 0;
      if (Phase2)sf[itrig] = 1;
      else sf[itrig] = 5;

      itrig = 18;
      passBeforePrescale[itrig] = eed;
      sf[itrig] = 1;

      itrig = 19;
      passBeforePrescale[itrig] = fed;
      sf[itrig] = 1;


      int L1Summary = 0;
      int L1Summary_psnm = 0;
      //std::vector<int> trgres;
      //dotrigger(trgres, obj);
      for (unsigned int i = 0; i < ntrg; i++) {
        int bitval = 0;
        int bitval_psnm = 0;
        if (passBeforePrescale[i]) bitval_psnm = doprescale(sf[i]);
        bitval = passBeforePrescale[i];
        L1Summary = L1Summary | (bitval << i);
        L1Summary_psnm = L1Summary_psnm | (bitval_psnm << i);
        GDLResult->setPreScale(0, i, sf[i]);
      }
      //GDLResult->setTRGSummary(0, L1Summary);
      GDLResult->setFtdlBits(0, L1Summary);
      GDLResult->setPsnmBits(0, L1Summary_psnm);
    }
  }

  int TRGGDL::doprescale(int f)
  {
    int Val = 0;
    double ran = gRandom->Uniform(f);
    if (ceil(ran) == f) Val = 1;
    return Val;
  }


  void
  TRGGDL::firmwareSimulation(void)
  {
    TRGDebug::enterStage("TRGGDL firmSim");

    if (TRGDebug::level())
      cout << TRGDebug::tab() << "Making dummy input signals" << endl;

    //...Clear signal bundles...
    if (_isb) {
      for (unsigned i = 0; i < _isb->size(); i++)
        delete(* _isb)[i];
      delete _isb;
    }
    if (_osb) {
      for (unsigned i = 0; i < _osb->size(); i++)
        delete(* _osb)[i];
      delete _osb;
    }
    if (_tsb) {
      for (unsigned i = 0; i < _tsb->size(); i++)
        delete(* _tsb)[i];
      delete _tsb;
    }
    if (_tosb) {
      for (unsigned i = 0; i < _tosb->size(); i++)
        delete(* _tosb)[i];
      delete _tosb;
    }

    //...Input bits...
    const unsigned nInput = _input.size();
    TRGSignalVector& input = * new TRGSignalVector(name() + "InputSignals",
                                                   _clock);
    TRGSignalVector& timing = * new TRGSignalVector(name() + "TimingSignals",
                                                    _clock);
    for (unsigned i = 0; i < nInput; i++) {

      TRGSignal s;

      //...Create dummy timing signal for even number input bits...
      if ((i % 2) == 0) {
        int tdcCount = 100;
        TRGTime rise = TRGTime(tdcCount, true, _clock, _input[i]);
        TRGTime fall = rise;
        fall.shift(1).reverse();
        s = TRGSignal(rise & fall);
        s.name(_input[i]);
      }

      //...Create dummy timing signal for odd number input bits...
      else {
        int tdcCount = 130;
        TRGTime rise = TRGTime(tdcCount, true, _clock, _input[i]);
        TRGTime fall = rise;
        fall.shift(1).reverse();
        s = TRGSignal(rise & fall);
        s.name(_input[i]);
      }

      //...Add to a signal vector...
      input += s;

      //...Timing signal...
      bool t =
        (i == 14) || (i == 15) || (i == 16) || (i == 17) ||
        (i == 41) || (i == 42) || (i == 43) || (i == 44) ||
        (i == 49) || (i == 50) || (i == 51) || (i == 52) || (i == 53);
      if (t)
        timing += s;
    }

    //...Make input signal bundle...
    const string ni = name() + "InputSignalBundle";
    _isb = new TRGSignalBundle(ni, _clock);
    _isb->push_back(& input);

    //...Make output signal bundle...
    const string no = name() + "OutputSignalBundle";
    _osb = new TRGSignalBundle(no,
                               _clock,
                               * _isb,
                               14,
                               TRGGDL::decision);

    //...Timing signal bundle...
    TRGSignalVector& ftd = * new TRGSignalVector(name() + "FTD", _clock);
//  ftd += (* ((* _osb)[0])[13]);
//  ftd += * (* _osb)[0][13];
    TRGSignal ddddd = (* (* _osb)[0])[13];
    ftd += ddddd;

    const string nt = name() + "TimingSignalBundle";
    _tsb = new TRGSignalBundle(nt, _clock);
    _tsb->push_back(& timing);
    _tsb->push_back(& ftd);

    //...Timing out signal bundle...
    const string nto = name() + "TimingOutSignalBundle";
    _tosb = new TRGSignalBundle(nto,
                                _clock,
                                * _tsb,
                                3,
                                7,
                                TRGGDL::timingDecision);

    if (TRGDebug::level()) {
      if (input.active()) {
        _isb->dump("detail", TRGDebug::tab());
        _osb->dump("detail", TRGDebug::tab());
        _tsb->dump("detail", TRGDebug::tab());
        _tosb->dump("detail", TRGDebug::tab());
      }
    }

    TRGDebug::leaveStage("TRGGDL firmSim");
  }

  void
  TRGGDL::configure(void)
  {

    //...Open input data definition...
    const string fni = _configFilename + ".inp";
    ifstream infile(fni.c_str(), ios::in);
    if (infile.fail()) {
      cout << "TRGGDL !!! can not open file : " << fni << endl;
      return;
    }
    getInput(infile);
    infile.close();

    //...Open output data definition...
    const string fno = _configFilename + ".oup";
    ifstream outfile(fno.c_str(), ios::in);
    if (outfile.fail()) {
      cout << "TRGGDL !!! can not open file : " << fno << endl;
      return;
    }
    getOutput(outfile);
    outfile.close();

    //...Open algorithm data definition...
    const string fna = _configFilename + ".alg";
    ifstream algfile(fna.c_str(), ios::in);
    if (algfile.fail()) {
      cout << "TRGGDL !!! can not open file : " << fna << endl;
      return;
    }
    getAlgorithm(algfile);
    algfile.close();

    //...FTD function...
    string ftd = _configFilename;
    string::size_type s = ftd.find_last_of("/");
    if (s != string::npos)
      ftd = ftd.substr(s + 1);
    if (ftd == "ftd_0.01") {
      _ftd = ftd_0_01;
    }
    /*
      //get trigger menu file
      const string fmenu = _configFilename+".menu"
      ifstream menufile(fmenu.c_str(), ios::in);
      if(menufile.fail()){
      cout<<"trigger menufile "<<fmenu<<"  can not open"<<endl;
      return;
      }
      getTriggerMenu(menufile);
      menufile.close();
     */

    //...Summary...
    if (TRGDebug::level()) {
      cout << "TRGGDL Config file = " << _configFilename << endl;
      cout << "    ftd=" << ftd << endl;
      cout << "TRGGDL Input Bits" << endl;
      for (unsigned i = 0; i < _input.size(); i++)
        cout << TRGDebug::tab(4) << i << " : " << _input[i] << endl;
      cout << "TRGGDL Output Bits" << endl;
      for (unsigned i = 0; i < _output.size(); i++)
        cout << TRGDebug::tab(4) << i << " : " << _output[i]
             << " : " << _algorithm[i] << endl;
    }
  }

  void
  TRGGDL::getInput(ifstream& ifs)
  {
    if (TRGDebug::level()) {
      cout << "TRGGDL::getInput ... reading input data" << endl;
    }

    char b[800];
    unsigned lines = 0;
    while (! ifs.eof()) {
      ifs.getline(b, 800);

      //...The first word should be input bit number...
      // const string w0 = TRGUtilities::carstring(b);

      //...Bit name...
      string cdr = TRGUtilities::cdrstring(b);
      const string w1 = TRGUtilities::carstring(cdr);

      if (w1.size())
        _input.push_back(w1);


      // if (TRGDebug::level()) {
      //     cout << w0 << "," << w1 << endl;
      // }

      ++lines;
    }

  }

  void
  TRGGDL::getOutput(ifstream& ifs)
  {

    if (TRGDebug::level()) {
      cout << "TRGGDL::getOutput ... reading output data" << endl;
    }

    char b[800];
    unsigned lines = 0;
    while (! ifs.eof()) {
      ifs.getline(b, 800);

      //...The first word should be input bit number...
      // const string w0 = TRGUtilities::carstring(b);

      //...Bit name...
      string cdr = TRGUtilities::cdrstring(b);
      const string w1 = TRGUtilities::carstring(cdr);

      if (w1.size())
        _output.push_back(w1);

      // if (TRGDebug::level()) {
      //     cout << w0 << "," << w1 << endl;
      // }

      ++lines;
    }

  }

  void
  TRGGDL::getAlgorithm(ifstream& ifs)
  {

    if (TRGDebug::level()) {
      cout << "TRGGDL::getAlgorithm ... reading algorithm data" << endl;
    }

    char b[800];
    unsigned lines = 0;
    while (! ifs.eof()) {
      ifs.getline(b, 800);

      //...The first word should be input bit number...
      // const string w0 = TRGUtilities::carstring(b);

      //...':'...
      string cdr = TRGUtilities::cdrstring(b);
      // const string w1 = TRGUtilities::carstring(cdr);

      //...Algorithm...
      cdr = TRGUtilities::cdrstring(cdr);
      const string w2 = cdr;

      if (w2.size())
        _algorithm.push_back(w2);

      // if (TRGDebug::level()) {
      //     cout << w0 << "," << w1 << "," << w2 << endl;
      // }

      ++lines;
    }

  }

  /*void
  TRGGDL::getTriggerMenu(ifstream & menuf){

  std::string sline;
  std::string scut;
  int prescaf;
  int index;
  if(menf.is_open()){
  while(!menuf.eof())
  {
  std::getline(menuf, sline);
  //sscanf(sline, " %i %s %i", &index, scut, &prescaf)
  _trgmenu.push_back(scut);
  //_prescale.push_back(prescaf);
  }
  }
  }
  */

  TRGState
  TRGGDL::decision(const TRGState& input)
  {

    //...Prepare states for output...
    TRGState s(14);

    //...Set up bool array...
    bool* in = new bool[input.size()];
    bool* ou = new bool[14];
    input.copy2bool(in);

    // //...Simulate output bit 4...
    // s.set(4, in[15] & (! in[18]) & in[33] & (! in[51]));

    // //...Simulate output bit 10...
    // s.set(10, in[46]);

    //...FTD logic...
    _ftd(ou, in);
    bool active = false;
    for (unsigned i = 0; i < 13; i++) {
      if (ou[i])
        active = true;
      s.set(i, ou[i]);
    }
    if (active)
      s.set(13, true);

    if (TRGDebug::level()) {
      input.dump("detail", TRGDebug::tab(4) + "GDL in ");
      s.dump("detail", TRGDebug::tab(4) + "GDL out ");
    }

    //...Termination...
    delete[] in;
    delete[] ou;
    return s;
  }

  TRGState
  TRGGDL::timingDecision(const TRGState& input,
                         TRGState& reg,
                         bool& active)
  {

    //...Prepare registers...
    //   reg[0] := timing logic active, NOT USED
    //   reg[3:1] := state (0 to 7)
    //   reg[6:4] := counter after timing logic active

    //   reg[1] := waiting for timing of TOP, ECL, and CDC
    //   reg[2] := TOP timing active
    //   reg[3] := ECL timing active
    //   reg[4] := CDC timing active
    //   reg[7:5] := remaining count to issue timing signal



    //...Input preparations...
    //   Assuming ftl_0.01
    //   Last bit is additional FTD signal
    bool in[14];

    //...CDC...
    in[0] = input[0];
    in[1] = input[1];
    in[2] = input[2];
    in[3] = input[3];
    /*
    bool cdc = in[0];
    bool * cdct = & in[1];
    */

    //...ECL...
    in[4] = input[4];
    in[5] = input[5];
    in[6] = input[6];
    in[7] = input[7];
    /*
    bool ecl = in[4];
    bool * eclt = & in[5];
    */

    //...TOP...
    in[8] = input[8];
    in[9] = input[9];
    in[10] = input[10];
    in[11] = input[11];
    in[12] = input[12];
    /*
    bool top = in[8];
    bool * topt = & in[9];
    */

    //...FTD...
    in[13] = input[13];
    bool ftd = in[13];

    //...Get state info...
    unsigned state = unsigned(reg.subset(1, 3));
    unsigned count = unsigned(reg.subset(4, 3));

    //...State machine...
    bool timing = false;
    unsigned source = 0;
    if (state == 0) {
      if (ftd) {
        active = 1;
        state = 1;
        count = 1;
      } else {
        active = 0;
      }
    } else if (state == 1) {
      if (count == 7) {
        active = 0;
        state = 0;
        timing = true;
      } else {
        active = 1;
        ++count;
      }
    }

    //...Set registers...
    reg.set(1, 3, state);
    reg.set(4, 3, count);

    //...Output...
    TRGState out(3);
    out.set(0, 1, timing);
    out.set(1, 2, source);

    if (TRGDebug::level()) {
      input.dump("detail", TRGDebug::tab(4) + "Timing in,");
      reg.dump("detail", TRGDebug::tab(4) + "Registers,");
      cout << TRGDebug::tab(4) << "ftd,active,state,count=" << ftd << ","
           << active << "," << state
           << "," << count << endl;

      out.dump("detail", TRGDebug::tab(4) + "Timing out,");
    }

    //...Termination...
    return out;
  }

} // namespace Belle2
