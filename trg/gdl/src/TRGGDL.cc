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
#include "trg/trg/Utilities.h"
#include "trg/gdl/TRGGDL.h"
// framework - DataStore
#include <framework/datastore/StoreObjPtr.h>

#include <mdst/dataobjects/TRGSummary.h>
#include <trg/grl/dataobjects/TRGGRLInfo.h>

#include <framework/logging/Logger.h>

#include <TH1I.h>

#include <stdexcept>

#define N_TIMING_REGISTERS 4

using namespace std;

namespace Belle2 {

  void ftd_0_01(bool* b, bool* i);
  /**do the final trigger decision*/
  //void dotrigger(std::vector<int>& res, std::vector<int> ob);

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
                    const std::string& Phase,
                    bool algFromDB,
                    const std::string& algFilePath,
                    int debugLevel)
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
                        Phase,
                        algFromDB,
                        algFilePath,
                        debugLevel);
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
                 const std::string& Phase,
                 bool algFromDB,
                 const std::string& algFilePath,
                 int debugLevel)
    : _debugLevel(debugLevel),
      _configFilename(configFile),
      _simulationMode(simulationMode),
      _fastSimulationMode(fastSimulationMode),
      _firmwareSimulationMode(firmwareSimulationMode),
      _Phase(Phase),
      _algFilePath(algFilePath),
      _clock(Belle2_GDL::GDLSystemClock),
      _offset(15.3),
      _isb(0),
      _osb(0),
      _algFromDB(algFromDB)
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

    for (int i = 0; i < m_InputBitsDB->getninbit(); i++) {
      _inpBitNames.push_back(std::string(m_InputBitsDB->getinbitname(i)));
    }

    for (int i = 0; i < m_FTDLBitsDB->getnoutbit(); i++) {
      _oupBitNames.push_back(std::string(m_FTDLBitsDB->getoutbitname(i)));
    }

  }

  void
  TRGGDL::initialize(void)
  {
    if (_debugLevel > 19) {
      for (int i = 0; i < m_InputBitsDB->getninbit(); i++) {
        B2DEBUG(20, "TRGGDL::initialize, inputBits: " << i << ", " << m_InputBitsDB->getinbitname(i));
      }
      for (int i = 0; i < m_FTDLBitsDB->getnoutbit(); i++) {
        B2DEBUG(20, "TRGGDL::initialize, outputBits: " << i << ", " << m_FTDLBitsDB->getoutbitname(i));
      }
      for (int i = 0; i < db_algs->getnalgs(); i++) {
        B2DEBUG(20, "TRGGDL::initialize, algs: " << i << ", " << db_algs->getalg(i));
      }
    }
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
    /*
    TRGDebug::enterStage("TRGGDL update");

    if (TRGDebug::level())
      cout << TRGDebug::tab() << name() << " do nothing..." << endl;

    TRGDebug::leaveStage("TRGGDL update");
    */
  }

  TRGGDL::~TRGGDL()
  {
    clear();
  }

  void
  TRGGDL::simulate(void)
  {
    const bool fast = (_simulationMode == 1);
    const bool firm = (_simulationMode == 2);
    const bool data = (_simulationMode == 3);
    if (fast)
      fastSimulation();
    if (firm)
      firmwareSimulation();
    if (data)
      dataSimulation();
  }

  void
  TRGGDL::fastSimulation(void)
  {
    B2DEBUG(20, "TRGGDL::fastSimulation starts.");
    TRGDebug::enterStage("TRGGDL fastSim");

    if (TRGDebug::level())
      cout << TRGDebug::tab() << name() << " do nothing..." << endl;

    TRGDebug::leaveStage("TRGGDL fastSim");

    if (!m_InputBitsDB) B2INFO("no database of gdl input bits");
    int N_InputBits = m_InputBitsDB->getninbit();
    if (!m_FTDLBitsDB)  B2INFO("no database of gdl ftdl bits");
    int N_OutputBits = m_FTDLBitsDB->getnoutbit();
    if (!m_PrescalesDB)  B2INFO("no database of gdl prescale");
    int N_AlgsBits = db_algs->getnalgs();
    if (!db_algs)  B2INFO("no global database of gdl ftd logics");
    if (N_OutputBits > N_AlgsBits) {
      B2DEBUG(20, "#Algs and #Ftdl is different");
      N_OutputBits = N_AlgsBits;
    }

    StoreObjPtr<TRGSummary> GDLResult;
    if (GDLResult) {
      B2WARNING("TRGGDL::fastSimulation(): TRGSummary exist already, check it!!!!");
      return;
    } else {
      GDLResult.create();
      StoreObjPtr<TRGGRLInfo> grlinfo("TRGGRLObjects");
      if (!grlinfo) {
        B2WARNING("TRGGRLInfo doesn't exist!!!!");
        return;
      } else {
        if (_debugLevel > 89) printf("TRGGDL:TRGGRLInfo found.\n");
      }

      _inpBits.clear();
      int input_summary = 0;
      for (int i = 0; i < N_InputBits; i++) {
        if (_debugLevel > 89) printf("TRGGDL:ABC:i(%d)\n", i);
        if ((i % 32) == 0) {
          if (i > 0) {
            GDLResult->setInputBits(i / 32 - 1, input_summary);
          }
          input_summary = 0;
        }
        if (grlinfo->getInputBits(i)) input_summary |= (1 << (i % 32));
        _inpBits.push_back(grlinfo->getInputBits(i));
        if (i == N_InputBits - 1) {
          GDLResult->setInputBits(i / 32, input_summary);
        }
      }

      int L1Summary = 0;
      int L1Summary_psnm = 0;
      std::string str;
      std::vector<std::string> algs;
      std::ifstream isload(_algFilePath.c_str(), std::ios::in);
      int index = 0;
      while (std::getline(isload, str)) {
        algs.push_back(str);
        index++;
      }
      isload.close();

      for (int i = 0; i < N_OutputBits; i++) {
        if (_debugLevel > 89) printf("TRGGDL:ABB:i(%d)\n", i);
        if ((i % 32) == 0) {
          if (i > 0) {
            GDLResult->setFtdlBits(i / 32 - 1, L1Summary);
            GDLResult->setPsnmBits(i / 32 - 1, L1Summary_psnm);
          }
          L1Summary = 0;
          L1Summary_psnm = 0;
        }
        std::string alg = _algFromDB ? db_algs->getalg(i) : algs[i];
        if (_debugLevel > 89) printf("TRGGDL:i(%d), alg(%s)\n", i, db_algs->getalg(i).c_str());
        if (isFiredFTDL(_inpBits, alg)) {
          L1Summary |= (1 << (i % 32));
          if (doprescale(m_PrescalesDB->getprescales(i))) {
            L1Summary_psnm |= (1 << (i % 32));
          }
        }
        GDLResult->setPreScale((i / 32), (i % 32), m_PrescalesDB->getprescales(i));
        if (i == N_OutputBits - 1) {
          GDLResult->setFtdlBits(i / 32, L1Summary);
          GDLResult->setPsnmBits(i / 32, L1Summary_psnm);
        }
      }
    }
  }

  void
  TRGGDL::dataSimulation(void)
  {
//  B2DEBUG(20,"TRGGDL::dataSimulation starts.");
    StoreObjPtr<EventMetaData> bevt;
    /*
    unsigned _exp = bevt->getExperiment();
    unsigned _run = bevt->getRun();
    */
    if (_debugLevel > 9) printf("TRGGDL:dataSimulation Start\n");
    unsigned _evt = bevt->getEvent();
//  TRGDebug::enterStage("TRGGDL dataSim");

//  if (TRGDebug::level())
//    cout << TRGDebug::tab() << name() << " do nothing..." << endl;

    if (!m_InputBitsDB) B2INFO("no database of gdl input bits");
    int N_InputBits = m_InputBitsDB->getninbit();
    if (!m_FTDLBitsDB)  B2INFO("no database of gdl ftdl bits");
    int N_OutputBits = m_FTDLBitsDB->getnoutbit();
    if (!m_PrescalesDB) B2INFO("no database of gdl prescale");
    int N_AlgsBits = db_algs->getnalgs();
    if (!db_algs) B2INFO("no database of gdl ftdl bit logic");
    if (N_OutputBits > N_AlgsBits) {
      B2DEBUG(20, "#Algs and #FTDL is different");
      N_OutputBits = N_AlgsBits;
    }

    if (_debugLevel > 19)
      printf("TRGGDL:N_InputBits(%d), N_OutputBits(%d)\n", N_InputBits, N_OutputBits);

    StoreObjPtr<TRGSummary> GDLResult;
    if (! GDLResult) {
      B2WARNING("TRGGDL::dataSimulation(): TRGSummary not found. Check it!!!!");
      return;
    } else {

      if (_debugLevel > 89) printf("TRGGDL:TRGSummary Found.\n");
      _inpBits.clear();
      _ftdBits.clear();
      _psnBits.clear();
      for (int i = 0; i < N_InputBits; i++) {
        bool inputBit;
        try { inputBit = GDLResult->testInput(i); }
        catch (const std::exception&) { inputBit = false; }
        _inpBits.push_back(inputBit);
      }

      if (_algFromDB) {
        for (int i = 0; i < N_OutputBits; i++) {
          bool ftdl_fired = isFiredFTDL(_inpBits, db_algs->getalg(i));
          bool psnm_fired = false;
          _ftdBits.push_back(ftdl_fired);
          if (ftdl_fired) {
            if (doprescale(m_PrescalesDB->getprescales(i))) {
              psnm_fired = true;
            }
          }
          psnm_fired ? _psnBits.push_back(true) : _psnBits.push_back(false);
          GDLResult->setPreScale((i / 32), (i % 32), m_PrescalesDB->getprescales(i));
          if (! strcmp(m_FTDLBitsDB->getoutbitname(i), "hie")) {
            if (_debugLevel == 971 && ftdl_fired) {
              int i_ehigh    = m_InputBitsDB->getinbitnum("ehigh");
              int i_bha_veto = m_InputBitsDB->getinbitnum("bha_veto");
              printf("TRGGDL:hie:i=%d,evt=%u,ps=%d,ehigh=%d,bha_veto=%d,ftdl_fired=%d,psnm_fired=%d,i_ehigh=%d,i_bha_veto=%d,obitname=%s\n",
                     i, _evt, m_PrescalesDB->getprescales(i),
                     _inpBits[i_ehigh] ? 1 : 0,
                     _inpBits[i_bha_veto] ? 1 : 0,
                     ftdl_fired,
                     psnm_fired ? 1 : 0,
                     i_ehigh,
                     i_bha_veto,
                     m_FTDLBitsDB->getoutbitname(i));
            }
          }
        }

      } else {
        std::string str;
        std::vector<std::string> algs;
        std::ifstream isload(_algFilePath.c_str(), std::ios::in);
        int index = 0;
        while (std::getline(isload, str)) {
          algs.push_back(str);
          index++;
        }
        isload.close();

        for (int i = 0; i < N_OutputBits; i++) {
          bool ftdl_fired = isFiredFTDL(_inpBits, algs[i]);
          bool psnm_fired = false;
          _ftdBits.push_back(ftdl_fired);
          if (ftdl_fired) {
            if (doprescale(m_PrescalesDB->getprescales(i))) {
              psnm_fired = true;
            }
          }
          psnm_fired ? _psnBits.push_back(true) : _psnBits.push_back(false);
          GDLResult->setPreScale((i / 32), (i % 32), m_PrescalesDB->getprescales(i));
        }

      }


    }
  }

  bool
  TRGGDL::isFiredFTDL(std::vector<bool> input, std::string alg)
  {
    if (alg.length() == 0) return true;
    const char* cst = alg.c_str();
    bool reading_word = false;
    bool result_the_term = true; // init value must be true
    bool not_flag = false;
    unsigned begin_word = 0;
    unsigned word_length = 0;
    // notation steeing side must follow
    //  no blank between '!' and word
    for (unsigned i = 0; i < alg.length(); i++) {
      if (('0' <= cst[i] && cst[i] <= '9') ||
          ('_' == cst[i]) || ('!' == cst[i])) {
        if (reading_word) { // must not be '!'
          word_length++;
          if (i == alg.length() - 1) {
            bool fired = input[atoi(alg.substr(begin_word, word_length).c_str())];
            B2DEBUG(20,
                    alg.substr(begin_word, word_length).c_str()
                    << "(" << fired << ")");
            if (((!not_flag && fired) || (not_flag && !fired)) && result_the_term) {
              return true;
            }
          }
        } else {
          // start of new word
          reading_word = true;
          if ('!' == cst[i]) {
            begin_word = i + 1;
            not_flag = true;
            word_length = 0;
          } else {
            begin_word = i;
            not_flag = false;
            word_length = 1;
            if (i == alg.length() - 1) {
              // one char bit ('f',...) comes end of conditions, 'xxx+f'
              bool fired = input[atoi(alg.substr(begin_word, word_length).c_str())];
              B2DEBUG(20,
                      alg.substr(begin_word, word_length).c_str()
                      << "(" << fired << ")");
              // cppcheck-suppress knownConditionTrueFalse
              if (((!not_flag && fired) || (not_flag && !fired)) && result_the_term) {
                return true;
              }
            }
          }
        }
      } else if ('+' == cst[i] || i == alg.length() - 1) {
        // End of the term.
        if (reading_word) { // 'xxx+'
          if (result_the_term) {
            bool fired = input[atoi(alg.substr(begin_word, word_length).c_str())];
            B2DEBUG(20,
                    alg.substr(begin_word, word_length).c_str()
                    << "(" << fired << ")");
            if ((!not_flag && fired) || (not_flag && !fired)) {
              return true;
            } else {
              // this term is denied by the latest bit
            }
          } else {
            // already false.
          }
          reading_word = false;
        } else {
          // prior char is blank, 'xxx  +'
          if (result_the_term) {
            return true;
          } else {
            // already false
          }
        }
        result_the_term = true; //  go to next term
      } else {
        // can be blank (white space) or any delimiter.
        if (reading_word) {
          // end of a word, 'xxxx '
          if (result_the_term) {
            // worth to try
            bool fired = input[atoi(alg.substr(begin_word, word_length).c_str())];
            B2DEBUG(20,
                    alg.substr(begin_word, word_length).c_str()
                    << "(" << fired << ")");
            if ((!not_flag && fired) || (not_flag && !fired)) {
              // go to next word
            } else {
              result_the_term = false;
            }
          } else {
            // already false
          }
          reading_word = false;
        } else {
          // 2nd blank 'xx  ' or leading blanck '^ '
        }
      }
    }
    return false;
  }

  bool TRGGDL::doprescale(int f)
  {
    if (f == 0) return false;
    if (f == 1) return true;
    double ran = gRandom->Uniform(f);
    return (ceil(ran) == f);
  }


  void
  TRGGDL::firmwareSimulation(void)
  {
//  TRGDebug::enterStage("TRGGDL firmSim");

//  if (TRGDebug::level())
//    cout << TRGDebug::tab() << "Making dummy input signals" << endl;

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

      ++lines;
    }

  }

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

  void
  TRGGDL::accumulateInp(TH1I* h)
  {
    for (std::size_t i = 0; i < _inpBits.size(); i++) {
      if (_inpBits[i]) h->Fill(i);
    }
  }

  void
  TRGGDL::accumulateFtd(TH1I* h)
  {
    for (std::size_t i = 0; i < _ftdBits.size(); i++) {
      if (_ftdBits[i]) h->Fill(i);
    }
  }

  void
  TRGGDL::accumulatePsn(TH1I* h)
  {
    for (std::size_t i = 0; i < _psnBits.size(); i++) {
      if (_psnBits[i]) h->Fill(i);
    }
  }

} // namespace Belle2
