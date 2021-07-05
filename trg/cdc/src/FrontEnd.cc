/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : FrontEnd.cc
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a CDC front-end board
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#define TRG_SHORT_NAMES
#define TRGCDC_SHORT_NAMES

#include "trg/trg/Utilities.h"
#include "trg/trg/Debug.h"
#include "trg/trg/State.h"
#include "trg/cdc/TRGCDC.h"
#include "trg/cdc/FrontEnd.h"
#include "trg/cdc/Wire.h"

using namespace std;

namespace Belle2 {

  TRGCDCFrontEnd::TRGCDCFrontEnd(const std::string& name,
                                 boardType type,
                                 const TRGClock& systemClock,
                                 const TRGClock& dataClock,
                                 const TRGClock& userClockOutput)
    : TRGBoard(name,
               systemClock,
               dataClock,
               userClockOutput,
               userClockOutput),
      _type(type),
      _isb(0),
      _osb(0)
  {

  }

  TRGCDCFrontEnd::~TRGCDCFrontEnd()
  {
  }

  string
  TRGCDCFrontEnd::version(void)
  {
    return ("TRGCDCFrontEnd version 0.01");
  }

  TRGCDCFrontEnd::boardType
  TRGCDCFrontEnd::type(void) const
  {
    return _type;
  }

  int
  TRGCDCFrontEnd::implementation(const TRGCDCFrontEnd::boardType& type,
                                 ofstream& outfile)
  {

    //string cname = "UNKNOWN";
    //    if (_type == innerInside) {
    outfile << "-- inner type" << endl;
    string cname = "TRGCDCFrontEndInnerInside";
    //     }
    //     else {
    //         outfile << "-- outer type" << endl;
    //         cname = "TRGCDCFrontEndOuter";
    //     }

    outfile << "--" << endl;
    outfile << endl;
    outfile << "    component " << cname << endl;
    // int err = implementationPort(type, outfile);
    outfile << "    end component;" << endl;

    outfile << "--------------------------------------------------------------"
            << endl;

    outfile << "entity " << cname << "is" << endl;
    int err = implementationPort(type, outfile);
    outfile << "end " << cname << ";" << endl;
    outfile << endl;
    outfile << "architecture Behavioral of " << cname << " is" << endl;
    outfile << "    -- local" << endl;
    outfile << endl;
    outfile << "begin" << endl;
    outfile << "end Behavioral;" << endl;

    return err;
  }

  int
  TRGCDCFrontEnd::implementationPort(const TRGCDCFrontEnd::boardType&,
                                     ofstream& outfile)
  {
    outfile << "    port (" << endl;

    //...125MHz clock (TRG system clock)...
    outfile << "       -- 125MHz clock (TRG system clock)" << endl;
    outfile << "          clk : in std_logic;" << endl;

    //...Coarse timing...
    outfile << "       -- Coarse timing (counter with 125MHz clock)" << endl;
    outfile << "          tmc : in std_logic_vector(0 to 12);" << endl;

    //...Hit pattern...
    outfile << "       -- Hit pattern(48 bits for 48 wires)" << endl;
    outfile << "          ptn : in std_logic_vector(0 to 47);" << endl;

    //...Fine timing for each wire...
    outfile << "       -- Fine timing within 125MHz clock" << endl;
    for (unsigned i = 0; i < 48; i++) {
      outfile << "          w" << TRGUtil::itostring(i)
              << " : in std_logic_vector(0 to 3);" << endl;
    }

    //...Hit pattern output...
    outfile << "       -- Hit pattern output" << endl;
    outfile << "          hit : out std_logic_vector(0 to 47);" << endl;
    outfile << "       -- 4 bit fine timing for 16 priority wires" << endl;
    outfile << "          pri : out std_logic_vector(0 to 63);" << endl;
    outfile << "       -- 2 bit fine timing for the fastest hit in 16 TS"
            << endl;
    outfile << "          fst : out std_logic_vector(0 to 31)" << endl;
    outfile << "    );" << endl;

    return 0;
  }

  void
  TRGCDCFrontEnd::push_back(const TRGCDCWire* a)
  {
    std::vector<const TRGCDCWire*>::push_back(a);
  }

  void
  TRGCDCFrontEnd::dump(const string& message, const string& pre) const
  {
    TRGBoard::dump(message, pre);
  }

  void
  TRGCDCFrontEnd::simulate(void)
  {


    //...Clear input signal bundle...
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

    //...Clock...
    const TRGClock& dClock = clockData();

    //...Input from wires...
    const unsigned nWires = size();
    TRGSignalVector input(name() + "inputSignals", dClock);
    for (unsigned i = 0; i < nWires; i++) {

      //...This may be too expensive. There must be a class to hold pointers.
      input += (* this)[i]->signal();
    }

    //...Make input signal bundle...
    const string ni = name() + "InputSignalBundle";
    _isb = new TRGSignalBundle(ni, dClock);

    //...Create hit pattern bits...
    TRGSignalVector* pattern = new TRGSignalVector(input);
    pattern->clock(dClock);
    pattern->name(name() + "@dataClock");
    _isb->push_back(pattern);

    // if (TRGDebug::level() > 1) {
    //     if (input.active()) {
    //         input.dump("", TRGDebug::tab());
    //         pattern->dump("", TRGDebug::tab());
    //     }
    // }

    // output wire hit info into a .log file
    // ofstream wireinfo((name()+"_wireinfo.log").c_str());


    //...Create Wire timing bits...
    for (unsigned i = 0; i < nWires; i++) {

      const TRGSignal& s = input[i];
      const string n = s.name() + ":5bits";
      TRGSignalVector* bit5 = new TRGSignalVector(n, dClock);

      //...No hit case : only dummy data...
      if (!(* pattern)[i].active()) {
        TRGSignal sig0(n + "-0", dClock);
        TRGSignal sig1(n + "-1", dClock);
        TRGSignal sig2(n + "-2", dClock);
        TRGSignal sig3(n + "-3", dClock);
        TRGSignal sig4(n + "-4", dClock);
        bit5->push_back(sig0);
        bit5->push_back(sig1);
        bit5->push_back(sig2);
        bit5->push_back(sig3);
        bit5->push_back(sig4);
      }

      //...Hit case...
      else {

        // wire hit info -> .log
        /*
          if (type() == innerInside) { wireinfo << endl << "     #" << (i > 15 ? i - 16 : 99999 ) << " wire is hit" << endl; }
          else {  wireinfo << endl << " #" <<  i  << " wire is hit" << endl; }
          output << endl;
        */

        // s is wire signal which is hit.
        const std::vector<int> timing = s.stateChanges();
        const unsigned nStates = timing.size();
        // Loop over all the changing timing.
        for (unsigned j = 0; j < nStates; j++) {
          const int pos = timing[j];

          //...Look at leading edges only...
          if (s.state(pos)) {
            const double at = s.clock().absoluteTime(pos);
            const int pos1 = dClock.position(at);
            const double phase = dClock.phase(at);
            const unsigned bits = unsigned(phase / (360. / 32.));

            //                cout << "at,phase,bits,pos1=" << at << "," << phase << ","
            //               << bits <<","<<pos1<< endl;

            // Change priority timing from unsigned into SignalVector.
            if (bit5->active()) {
              for (unsigned k = 0; k < 5; k++) {
                if ((bits >> k) & 1)
                  (*bit5)[k].set(pos1, pos1 + 1);
              }
            } else {
              for (unsigned k = 0; k < 5; k++) {
                const string n1 = n + "-" + TRGUtilities::itostring(k);
                TRGSignal sig(n1, dClock);
                if ((bits >> k) & 1)
                  sig.set(pos1, pos1 + 1);
                (* bit5) += sig;
              }
            }


            //wire info -> .log
            /*
            //timing = 00000 case:
            if ( !(bit5->stateChanges()).size() ) {
            output << "# of clk: " << pos1 << " (" << pos1*32 << " ns), signal vector: 0 0 0 0 0" << endl;
            }

            const std::vector<int> bit5_changetime = bit5->stateChanges();
            std::vector<vector<int>> bit5_boolvector(bit5_changetime.size());
            for (unsigned ch_t = 0; ch_t < bit5_changetime.size(); ch_t++)       {

            for (unsigned b = 0; b < bit5->size(); b++){
            bit5_boolvector[ch_t].push_back(  (bit5->state( bit5_changetime[ch_t])[b])  ? 1 : 0 );
            }

            wireinfo << "# of clk: " << bit5_changetime[ch_t] << " (" << dClock.absoluteTime(bit5_changetime[ch_t]) << " ns), signal vector: " ;

            for (unsigned b = 0; b < bit5->size(); b++){
            wireinfo << bit5_boolvector[ch_t][ bit5->size() - b - 1] << " " ;
            }

            wireinfo << endl;
            }
            */


#ifdef TRG_DEBUG
//                    bit5->dump("detail", TRGDebug::tab());
#endif
          }
        }
      }

      _isb->push_back(bit5);
    }

    //...Data clock...
    // Data clock position data is omitted. Is this problem?

    //...Make output signal bundle...
    const string no = name() + "OutputSignalBundle";
    if (type() == innerInside)
      _osb = new TRGSignalBundle(no,
                                 dClock,
                                 * _isb,
                                 256,
                                 TCFrontEnd::packerInnerInside);
    else if (type() == innerOutside)
      _osb = new TRGSignalBundle(no,
                                 dClock,
                                 * _isb,
                                 256,
                                 TCFrontEnd::packerInnerOutside);
    else if (type() == outerInside)
      _osb = new TRGSignalBundle(no,
                                 dClock,
                                 * _isb,
                                 256,
                                 TCFrontEnd::packerOuterInside);
    else if (type() == outerOutside)
      _osb = new TRGSignalBundle(no,
                                 dClock,
                                 * _isb,
                                 256,
                                 TCFrontEnd::packerOuterOutside);

    if (TRGDebug::level() > 1) {
      cout << TRGDebug::tab() << name() << " input bundle" << endl;
      _isb->dump("", TRGDebug::tab() + "    ");
      cout << TRGDebug::tab() << name() << " output bundle" << endl;
      _osb->dump("", TRGDebug::tab() + "    ");
    }
  }

  TRGState
  TCFrontEnd::packerInnerInside(const TRGState& input)
  {

    //...Input should be 48 hit pattern and 48x5 timing, total 288 bits...
    //
    // Wire numbers and TS ID
    //
    // outside
    //
    //    +--+--+--+--+-    -+--+--+--+--+--+--+
    //    |  31 |  30 | .... |  18 |  17 |  16 |
    //    +--+--+--+--+--+-    -+--+--+--+--+--+--+
    //       |  15 |  14 | .....|  2  |  1  |  0  |
    //    +--+--+--+--+--+-    -+--+--+--+--+--+--+
    //    |  x  |  x  | .... |  x  |  x  |  x  |
    //    +--+--+--+--+-    -+--+--+--+--+--+--+
    //
    //          15    14   ....    2     1     0      <- partial TS ID
    //
    // inside
    //

    //...Prepare a state for output...
    TRGState s(32 + 16 * 5 + 16 + 16 * 5 + 1 * 5 + 43);  // 1*5 for missing wire + 43 bits to fill to 256 bits

    //...Set up bool array...
    bool* b = new bool[input.size()];
    input.copy2bool(b);

    //...Naming...
    const bool* const hitptn = & b[16];   // 16x2 = 32 bits
    const bool* const timing[32] = {
      & b[128], & b[133], & b[138], & b[143],
      & b[148], & b[153], & b[158], & b[163],
      & b[168], & b[173], & b[178], & b[183],
      & b[188], & b[193], & b[198], & b[203],
      & b[208], & b[213], & b[218], & b[223],
      & b[228], & b[233], & b[238], & b[243],
      & b[248], & b[253], & b[258], & b[263],
      & b[268], & b[273], & b[278], & b[283]
    };

    //...Store hit pattern...
    s.set(0, 32, hitptn);
    unsigned p = 32;

    //bool true_5[5] = {true, true, true, true, true};

    //...Priority timing...
    TRGState secondPriority(16);
    for (unsigned i = 0; i < 16; i++) {

      //...Priority cell...
      if (hitptn[i]) {
        s.set(p, 5, timing[i]);
      }

      //...Second priority cells...
      else {

        //...Right side edge...
        if (i == 0) {
          if (hitptn[16]) {
            s.set(p, 5, timing[16]);
            secondPriority.set(i, true);
          } else {
            // s.set(p, 5, true_5);
            secondPriority.set(i, false);
          }
        }

        //...Others...
        else {

          //...Both secondarys have hit...
          if (hitptn[i + 15] && hitptn[i + 16]) {
            const unsigned t0 = TRGState::toUnsigned(5, timing[15]);
            const unsigned t1 = TRGState::toUnsigned(5, timing[16]);
            if (t0 < t1) {
              s.set(p, 5, timing[i + 15]);
              secondPriority.set(i, false);
            } else {
              s.set(p, 5, timing[i + 16]);
              secondPriority.set(i, true);
            }
          }

          //...Secondary at right side...
          else if (hitptn[i + 15]) {
            s.set(p, 5, timing[i + 15]);
            secondPriority.set(i, false);
          }

          //...Secondary at left side...
          else if (hitptn[i + 16]) {
            s.set(p, 5, timing[i + 16]);
            secondPriority.set(i, true);
          }

          // No secondary case. No action
          else {
            //s.set(p, 5, true_5);
            //secondPriority.set(i, true);
          }
        }
      }

      p += 5;
    }

    s.set(p, secondPriority);
    p += 16;

    //...Fastest timing...
    for (unsigned i = 0; i < 16; i++) {

      //...Right side edge...
      if (i == 0) {
        const bool h[2] = {hitptn[0], hitptn[16]};
        const unsigned hh = TRGState::toUnsigned(2, h);
        const unsigned t0 = TRGState::toUnsigned(5, timing[0]);
        const unsigned t1 = TRGState::toUnsigned(5, timing[16]);

        //...No hit case : no action
        if (hh == 0) {
          //s.set(p, 5, true_5);
        }

        //...One hit case...
        else if (hh == 1) {
          s.set(p, 5, timing[0]);
        } else if (hh == 2) {
          s.set(p, 5, timing[16]);
        }

        //...Two hit case...
        else {
          if (t0 <= t1)
            s.set(p, 5, timing[0]);
          else
            s.set(p, 5, timing[16]);
        }
      }

      //...Others...
      else {
        const unsigned i0 = i;
        const unsigned i1 = i + 15;
        const unsigned i2 = i + 16;
        const bool h[3] = {hitptn[i0], hitptn[i1], hitptn[i2]};
        const unsigned hh = TRGState::toUnsigned(3, h);
        const unsigned t0 = TRGState::toUnsigned(5, timing[i0]);
        const unsigned t1 = TRGState::toUnsigned(5, timing[i1]);
        const unsigned t2 = TRGState::toUnsigned(5, timing[i2]);

        //...No hit case : no action
        if (hh == 0) {
          //s.set(p, 5, true_5);
        }

        //...One hit case...
        else if (hh == 1) {
          // cout << "p=" << p << endl;
          // cout << "one hit pri" << endl;
          // s.dump("detail", "one hit pri ");
          // s.subset(p, 5).dump("detail", "one hit pri ");
          s.set(p, 5, timing[i0]);
          // s.dump("detail", "one hit pri ");
          // s.subset(p, 5).dump("detail", "one hit pri ");
        } else if (hh == 2) {
          //    cout << "one hit sec0" << endl;
          s.set(p, 5, timing[i1]);
        } else if (hh == 4) {
          //    cout << "one hit sec1" << endl;
          s.set(p, 5, timing[i2]);
        }

        //...Two hit case...
        else if (hh == 3) {
          if (t0 <= t1)
            s.set(p, 5, timing[i0]);
          else
            s.set(p, 5, timing[i1]);
        } else if (hh == 5) {
          if (t0 <= t2)
            s.set(p, 5, timing[i0]);
          else
            s.set(p, 5, timing[i2]);
        } else if (hh == 6) {
          if (t1 <= t2)
            s.set(p, 5, timing[i1]);
          else
            s.set(p, 5, timing[i2]);
        }

        //...Three hit case...
        else {
          if ((t0 <= t1) && (t0 <= t2))
            s.set(p, 5, timing[i0]);
          else if (t1 <= t2)
            s.set(p, 5, timing[i1]);
          else
            s.set(p, 5, timing[i2]);
        }
      }

      p += 5;
    }

    //...Timing of missing wires on edge TS...
    if (hitptn[31]) {
      s.set(p, 5, timing[31]);
    } else {
      //s.set(p, 5, true_5);
    }
    // p+=5;

    //...Debug...
#ifdef TRG_DEBUG
    // input.dump("detail", TRGDebug::tab() + "FrontEnd_II in ");
    // s.dump("detail", TRGDebug::tab() + "FrontEnd_II out ");
//    unpackerInnerInside(input, s);
#endif

    //...Termination...
    delete[] b;
    return s;
  }

  TRGState
  TCFrontEnd::packerInnerOutside(const TRGState& input)
  {

    //...Input should be 48 hit pattern and 48x5 timing, total 288 bits...
    //
    // Wire numbers and TS ID
    //
    // outside
    //
    //    +--+--+--+--+-    -+--+--+--+--+--+--+
    //    |  47 |  46 | .... |  34 |  33 |  32 |
    // +--+--+--+--+--+-    -+--+--+--+--+--+--+
    // |  31 |  30 | ..... | 18 |  17 |  16 |
    // +--+--+--+--+--+-    -+--+--+--+--+--+--+
    //    |  15 |  14 | .... |  2  |  1  |  0  |
    //    +--+--+--+--+-    -+--+--+--+--+--+--+
    //
    //       15    14   ....    2     1     0      <- partial TS ID
    //
    // inside
    //

    //...Prepare a state for output...
    TRGState s(48 + 16 * 5 + 4 * 5 + 108);     // 4*5 for missing area timings + 108 to fill to 256 bits

    //...Set up bool array...
    bool* b = new bool[input.size()];
    input.copy2bool(b);

    //...Naming...
    const bool* const hitptn = & b[0];    // 16x3 = 48 bits
    const bool* const timing[48] = {
      & b[48], & b[53], & b[58], & b[63],
      & b[68], & b[73], & b[78], & b[83],
      & b[88], & b[93], & b[98], & b[103],
      & b[108], & b[113], & b[118], & b[123],
      & b[128], & b[133], & b[138], & b[143],
      & b[148], & b[153], & b[158], & b[163],
      & b[168], & b[173], & b[178], & b[183],
      & b[188], & b[193], & b[198], & b[203],
      & b[208], & b[213], & b[218], & b[223],
      & b[228], & b[233], & b[238], & b[243],
      & b[248], & b[253], & b[258], & b[263],
      & b[268], & b[273], & b[278], & b[283]
    };

    //...Store hit pattern...
    s.set(0, 48, hitptn);
    unsigned p = 48;

    //...Fastest timing...
    const bool dummy[6] = {false, false, false, false, false, true},
                          dummymax[5] = {true, true, true, true, true};
    const TRGState wtDummy(6, dummy), wtDummymax(5, dummymax);
    for (unsigned i = 0; i < 20; i++) {   // 0 ... 15 for 16 standard ones, 16...19 for four edge areas.
      TRGState wt[12];

      if (i == 0) { // TS ID 0 has missing wires
        wt[0] = wtDummy;
        wt[1] = TRGState(5, timing[0]);
        wt[2] = TRGState(5, timing[1]);
        wt[3] = wtDummy;
        wt[4] = wtDummy;
        wt[5] = TRGState(5, timing[16]);
        wt[6] = TRGState(5, timing[17]);
        wt[7] = wtDummy;
        wt[8] = wtDummy;
        wt[9] = TRGState(5, timing[32]);
        wt[10] = TRGState(5, timing[33]);
        wt[11] = TRGState(5, timing[34]);

        //...Append 6th bit to indicate hit or not (no hit = 1)...
        if (! hitptn[0]) wt[1].set(5, true);
        if (! hitptn[1]) wt[2].set(5, true);
        if (! hitptn[16]) wt[5].set(5, true);
        if (! hitptn[17]) wt[6].set(5, true);
        if (! hitptn[32]) wt[9].set(5, true);
        if (! hitptn[33]) wt[10].set(5, true);
        if (! hitptn[34]) wt[11].set(5, true);
      } else if (i == 1) { // TS ID 1 has missing wires
        wt[0] = TRGState(5, timing[0]);
        wt[1] = TRGState(5, timing[1]);
        wt[2] = TRGState(5, timing[2]);
        wt[3] = wtDummy;
        wt[4] = TRGState(5, timing[16]);
        wt[5] = TRGState(5, timing[17]);
        wt[6] = TRGState(5, timing[18]);
        wt[7] = wtDummy;
        wt[8] = TRGState(5, timing[32]);
        wt[9] = TRGState(5, timing[33]);
        wt[10] = TRGState(5, timing[34]);
        wt[11] = TRGState(5, timing[35]);

        //...Append 6th bit to indicate hit or not (no hit = 1)...
        if (! hitptn[0]) wt[0].set(5, true);
        if (! hitptn[1]) wt[1].set(5, true);
        if (! hitptn[2]) wt[2].set(5, true);
        if (! hitptn[16]) wt[4].set(5, true);
        if (! hitptn[17]) wt[5].set(5, true);
        if (! hitptn[18]) wt[6].set(5, true);
        if (! hitptn[32]) wt[8].set(5, true);
        if (! hitptn[33]) wt[9].set(5, true);
        if (! hitptn[34]) wt[10].set(5, true);
        if (! hitptn[35]) wt[11].set(5, true);
      } else if (i == 14) { // TS ID 14 has missing wires
        wt[0] = TRGState(5, timing[13]);
        wt[1] = TRGState(5, timing[14]);
        wt[2] = TRGState(5, timing[15]);
        wt[3] = TRGState(5, timing[28]);
        wt[4] = TRGState(5, timing[29]);
        wt[5] = TRGState(5, timing[30]);
        wt[6] = TRGState(5, timing[31]);
        wt[7] = TRGState(5, timing[44]);
        wt[8] = TRGState(5, timing[45]);
        wt[9] = TRGState(5, timing[46]);
        wt[10] = TRGState(5, timing[47]);
        wt[11] = wtDummy;

        //...Append 6th bit to indicate hit or not (no hit = 1)...
        if (! hitptn[13]) wt[0].set(5, true);
        if (! hitptn[14]) wt[1].set(5, true);
        if (! hitptn[15]) wt[2].set(5, true);
        if (! hitptn[28]) wt[3].set(5, true);
        if (! hitptn[29]) wt[4].set(5, true);
        if (! hitptn[30]) wt[5].set(5, true);
        if (! hitptn[31]) wt[6].set(5, true);
        if (! hitptn[44]) wt[7].set(5, true);
        if (! hitptn[45]) wt[8].set(5, true);
        if (! hitptn[46]) wt[9].set(5, true);
        if (! hitptn[47]) wt[10].set(5, true);
      } else if (i == 15) { // TS ID 15 has missing wires
        wt[0] = TRGState(5, timing[14]);
        wt[1] = TRGState(5, timing[15]);
        wt[2] = wtDummy;
        wt[3] = TRGState(5, timing[29]);
        wt[4] = TRGState(5, timing[30]);
        wt[5] = TRGState(5, timing[31]);
        wt[6] = wtDummy;
        wt[7] = TRGState(5, timing[45]);
        wt[8] = TRGState(5, timing[46]);
        wt[9] = TRGState(5, timing[47]);
        wt[10] = wtDummy;
        wt[11] = wtDummy;

        //...Append 6th bit to indicate hit or not (no hit = 1)...
        if (! hitptn[14]) wt[0].set(5, true);
        if (! hitptn[15]) wt[1].set(5, true);
        if (! hitptn[29]) wt[3].set(5, true);
        if (! hitptn[30]) wt[4].set(5, true);
        if (! hitptn[31]) wt[5].set(5, true);
        if (! hitptn[45]) wt[7].set(5, true);
        if (! hitptn[46]) wt[8].set(5, true);
        if (! hitptn[47]) wt[9].set(5, true);
      } else if (i == 16) { // edge area 1, only cell 32 at position 11
        wt[0] = wtDummy;
        wt[1] = wtDummy;
        wt[2] = wtDummy;
        wt[3] = wtDummy;
        wt[4] = wtDummy;
        wt[5] = wtDummy;
        wt[6] = wtDummy;
        wt[7] = wtDummy;
        wt[8] = wtDummy;
        wt[9] = wtDummy;
        wt[10] = wtDummy;
        wt[11] = TRGState(5, timing[32]);

        //...Append 6th bit to indicate hit or not (no hit = 1)...
        if (! hitptn[32]) wt[11].set(5, true);
      } else if (i == 17) { // edge area 1, cell 0, 16, 32, 33 at position 2, 6, 10, 11
        wt[0] = wtDummy;
        wt[1] = wtDummy;
        wt[2] = TRGState(5, timing[0]);
        wt[3] = wtDummy;
        wt[4] = wtDummy;
        wt[5] = wtDummy;
        wt[6] = TRGState(5, timing[16]);
        wt[7] = wtDummy;
        wt[8] = wtDummy;
        wt[9] = wtDummy;
        wt[10] = TRGState(5, timing[32]);
        wt[11] = TRGState(5, timing[33]);

        //...Append 6th bit to indicate hit or not (no hit = 1)...
        if (! hitptn[0])  wt[2].set(5, true);
        if (! hitptn[16]) wt[6].set(5, true);
        if (! hitptn[32]) wt[10].set(5, true);
        if (! hitptn[33]) wt[11].set(5, true);
      } else if (i == 18) { // edge area 3, cell 15, 30, 31, 46, 47 at position 0, 3, 4, 7, 8
        wt[0] = TRGState(5, timing[15]);
        wt[1] = wtDummy;
        wt[2] = wtDummy;
        wt[3] = TRGState(5, timing[30]);
        wt[4] = TRGState(5, timing[31]);
        wt[5] = wtDummy;
        wt[6] = wtDummy;
        wt[7] = TRGState(5, timing[46]);
        wt[8] = TRGState(5, timing[47]);
        wt[9] = wtDummy;
        wt[10] = wtDummy;
        wt[11] = wtDummy;

        //...Append 6th bit to indicate hit or not (no hit = 1)...
        if (! hitptn[15])  wt[0].set(5, true);
        if (! hitptn[30])  wt[3].set(5, true);
        if (! hitptn[31])  wt[4].set(5, true);
        if (! hitptn[46])  wt[7].set(5, true);
        if (! hitptn[47])  wt[8].set(5, true);
      } else if (i == 19) { // edge area 4, only cell 31 and 47 at position 3 and 7
        wt[0] = wtDummy;
        wt[1] = wtDummy;
        wt[2] = wtDummy;
        wt[3] = TRGState(5, timing[31]);
        wt[4] = wtDummy;
        wt[5] = wtDummy;
        wt[6] = wtDummy;
        wt[7] = TRGState(5, timing[47]);
        wt[8] = wtDummy;
        wt[9] = wtDummy;
        wt[10] = wtDummy;
        wt[11] = wtDummy;

        //...Append 6th bit to indicate hit or not (no hit = 1)...
        if (! hitptn[31]) wt[3].set(5, true);
        if (! hitptn[47]) wt[7].set(5, true);
      } else {
        wt[0] = TRGState(5, timing[i - 1]);
        wt[1] = TRGState(5, timing[i]);
        wt[2] = TRGState(5, timing[i + 1]);
        wt[3] = TRGState(5, timing[i + 14]);
        wt[4] = TRGState(5, timing[i + 15]);
        wt[5] = TRGState(5, timing[i + 16]);
        wt[6] = TRGState(5, timing[i + 17]);
        wt[7] = TRGState(5, timing[i + 30]);
        wt[8] = TRGState(5, timing[i + 31]);
        wt[9] = TRGState(5, timing[i + 32]);
        wt[10] = TRGState(5, timing[i + 33]);
        wt[11] = TRGState(5, timing[i + 34]);

        //...Append 6th bit to indicate hit or not (no hit = 1)...
        if (! hitptn[i - 1]) wt[0].set(5, true);
        if (! hitptn[i]) wt[1].set(5, true);
        if (! hitptn[i + 1]) wt[2].set(5, true);
        if (! hitptn[i + 14]) wt[3].set(5, true);
        if (! hitptn[i + 15]) wt[4].set(5, true);
        if (! hitptn[i + 16]) wt[5].set(5, true);
        if (! hitptn[i + 17]) wt[6].set(5, true);
        if (! hitptn[i + 30]) wt[7].set(5, true);
        if (! hitptn[i + 31]) wt[8].set(5, true);
        if (! hitptn[i + 32]) wt[9].set(5, true);
        if (! hitptn[i + 33]) wt[10].set(5, true);
        if (! hitptn[i + 34]) wt[11].set(5, true);
      }

      //...Look for the fastest hit...
      unsigned fastest0 = 0;
      unsigned fastest1 = 0;
      unsigned fastest2 = 0;
      unsigned fastest3 = 0;
      unsigned fastest4 = 0;
      unsigned fastest5 = 0;
      if (wt[0] < wt[1])
        fastest0 = 0;
      else
        fastest0 = 1;
      if (wt[2] < wt[3])
        fastest1 = 2;
      else
        fastest1 = 3;
      if (wt[4] < wt[5])
        fastest2 = 4;
      else
        fastest2 = 5;
      if (wt[6] < wt[7])
        fastest3 = 6;
      else
        fastest3 = 7;
      if (wt[8] < wt[9])
        fastest4 = 8;
      else
        fastest4 = 9;
      if (wt[10] < wt[11])
        fastest5 = 10;
      else
        fastest5 = 11;

      unsigned fastest10 = 0;
      if (wt[fastest0] < wt[fastest1])
        fastest10 = fastest0;
      else
        fastest10 = fastest1;

      unsigned fastest11 = 0;
      if (wt[fastest2] < wt[fastest3])
        fastest11 = fastest2;
      else
        fastest11 = fastest3;

      unsigned fastest12 = 0;
      if (wt[fastest4] < wt[fastest5])
        fastest12 = fastest4;
      else
        fastest12 = fastest5;

      unsigned fastest101 = 0;
      if (wt[fastest10] < wt[fastest11])
        fastest101 = fastest10;
      else
        fastest101 = fastest11;

      unsigned fastest102 = 0;
      if (wt[fastest101] < wt[fastest12])
        fastest102 = fastest101;
      else
        fastest102 = fastest12;

      TRGState fastest(5);
      if (! wt[fastest102].active(5))
        fastest = wt[fastest102].subset(0, 5);

      s.set(p, fastest);
      p += 5;
    }

    ////...Timing of missing wires on edge TS...
    //s.set(p, 5, timing[0]);
    //p += 5;
    //s.set(p, 5, timing[16]);
    //p += 5;
    //s.set(p, 5, timing[32]);
    //p += 5;
    //s.set(p, 5, timing[33]);
    //p += 5;
    //s.set(p, 5, timing[15]);
    //p += 5;
    //s.set(p, 5, timing[30]);
    //p += 5;
    //s.set(p, 5, timing[31]);
    //p += 5;
    //s.set(p, 5, timing[46]);
    //p += 5;
    //s.set(p, 5, timing[47]);

#ifdef TRG_DEBUG
//        unpackerInnerOutside(input, s);
#endif

    delete[] b;
    return s;
  }

  TRGState
  TCFrontEnd::packerOuterInside(const TRGState& input)
  {

    //...Input should be 48 hit pattern and 48x5 timing, total 288 bits...
    //
    // Wire numbers and TS ID
    //
    // outside
    //
    //    +--+--+--+--+-    -+--+--+--+--+--+--+
    //    |  47 |  46 | .... |  34 |  33 |  32 |  <- priority wire layer
    // +--+--+--+--+--+-    -+--+--+--+--+--+--+
    // |  31 |  30 | ..... | 18 |  17 |  16 |
    // +--+--+--+--+--+-    -+--+--+--+--+--+--+
    //    |  15 |  14 | .... |  2  |  1  |  0  |
    //    +--+--+--+--+-    -+--+--+--+--+--+--+
    //
    //       15    14   ....    2     1     0      <- partial TS ID
    //
    // inside
    //
    //

    //...Prepare a state for output...
    //TRGState s(48 + 16 * 5 + 16 * 5 + 3 * 5);  // 3*5 for missing wires
    TRGState s(48 + 16 * 5 + 16 * 5 + 2 * 5 + 38);  // 2*5 for edge fastest time, 38 dummy bits to fill to 256 bits

    //...Set up bool array...
    bool* b = new bool[input.size()];
    input.copy2bool(b);

    //...Naming...
    const bool* const hitptn = & b[0];    // 16x3 = 48 bits
    const bool* const timing[48] = {
      & b[48], & b[53], & b[58], & b[63],
      & b[68], & b[73], & b[78], & b[83],
      & b[88], & b[93], & b[98], & b[103],
      & b[108], & b[113], & b[118], & b[123],
      & b[128], & b[133], & b[138], & b[143],
      & b[148], & b[153], & b[158], & b[163],
      & b[168], & b[173], & b[178], & b[183],
      & b[188], & b[193], & b[198], & b[203],
      & b[208], & b[213], & b[218], & b[223],
      & b[228], & b[233], & b[238], & b[243],
      & b[248], & b[253], & b[258], & b[263],
      & b[268], & b[273], & b[278], & b[283]
    };

    //...Store hit pattern...
    s.set(0, 48, hitptn);
    unsigned p = 48;

    //...Priority cell timing...
    for (unsigned i = 0; i < 16; i++) {
      s.set(p, 5, timing[32 + i]);
      p += 5;
    }

    //...Fastest timing...
    const bool dummy[6] = {false, false, false, false, false, true};
    const TRGState wtDummy(6, dummy);
    for (unsigned i = 0; i < 18; i++) {       // 0 ... 15 for 16 standard ones, 16,17 for two edge areas.
      TRGState wt[6];

      if (i == 0) { // TS ID 0 has missing wires
        wt[0] = wtDummy;
        wt[1] = TRGState(5, timing[0]);
        wt[2] = TRGState(5, timing[1]);
        wt[3] = wtDummy;
        wt[4] = TRGState(5, timing[16]);
        wt[5] = TRGState(5, timing[32]);

        //...Append 6th bit to indicate hit or not (no hit = 1)...
        if (! hitptn[0]) wt[1].set(5, true);
        if (! hitptn[1]) wt[2].set(5, true);
        if (! hitptn[16]) wt[4].set(5, true);
        if (! hitptn[32]) wt[5].set(5, true);
      } else if (i == 15) { // TS ID 15 has missing wires
        wt[0] = TRGState(5, timing[14]);
        wt[1] = TRGState(5, timing[15]);
        wt[2] = wtDummy;
        wt[3] = TRGState(5, timing[30]);
        wt[4] = TRGState(5, timing[31]);
        wt[5] = TRGState(5, timing[47]);

        //...Append 6th bit to indicate hit or not (no hit = 1)...
        if (! hitptn[14]) wt[0].set(5, true);
        if (! hitptn[15]) wt[1].set(5, true);
        if (! hitptn[30]) wt[3].set(5, true);
        if (! hitptn[31]) wt[4].set(5, true);
        if (! hitptn[47]) wt[5].set(5, true);
      } else if (i == 16)  { // edge area 1 only for cell 0
        wt[0] = wtDummy;
        wt[1] = wtDummy;
        //wt[2] = TRGState(5, timing[16]);
        wt[2] = TRGState(5, timing[0]);
        wt[3] = wtDummy;
        wt[4] = wtDummy;
        wt[5] = wtDummy;
        //...Append 6th bit to indicate hit or not (no hit = 1)...
        if (! hitptn[0]) wt[2].set(5, true);
      } else if (i == 17)  { // edge area 2 for cell 15 and 31 only
        wt[0] = TRGState(5, timing[15]);
        wt[1] = wtDummy;
        wt[2] = wtDummy;
        wt[3] = TRGState(5, timing[31]);
        wt[4] = wtDummy;
        wt[5] = wtDummy;

        //...Append 6th bit to indicate hit or not (no hit = 1)...
        if (! hitptn[15]) wt[0].set(5, true);
        if (! hitptn[31]) wt[3].set(5, true);
      } else {
        wt[0] = TRGState(5, timing[i - 1]);
        wt[1] = TRGState(5, timing[i]);
        wt[2] = TRGState(5, timing[i + 1]);
        wt[3] = TRGState(5, timing[i + 15]);
        wt[4] = TRGState(5, timing[i + 16]);
        wt[5] = TRGState(5, timing[i + 32]);

        //...Append 6th bit to indicate hit or not (no hit = 1)...
        if (! hitptn[i - 1])  wt[0].set(5, true);
        if (! hitptn[i])      wt[1].set(5, true);
        if (! hitptn[i + 1])  wt[2].set(5, true);
        if (! hitptn[i + 15]) wt[3].set(5, true);
        if (! hitptn[i + 16]) wt[4].set(5, true);
        if (! hitptn[i + 32]) wt[5].set(5, true);
      }

      //...Look for the fastest hit...
      unsigned fastest0 = 0;
      unsigned fastest1 = 0;
      unsigned fastest2 = 0;
      if (wt[0] < wt[1])
        fastest0 = 0;
      else
        fastest0 = 1;
      if (wt[2] < wt[3])
        fastest1 = 2;
      else
        fastest1 = 3;
      if (wt[4] < wt[5])
        fastest2 = 4;
      else
        fastest2 = 5;

      unsigned fastest3 = 0;
      if (wt[fastest0] < wt[fastest1])
        fastest3 = fastest0;
      else
        fastest3 = fastest1;

      unsigned fastest4 = 0;
      if (wt[fastest2] < wt[fastest3])
        fastest4 = fastest2;
      else
        fastest4 = fastest3;

      TRGState fastest(5);
      if (! wt[fastest4].active(5))
        fastest = wt[fastest4].subset(0, 5);

      s.set(p, fastest);
      p += 5;
    }

    //s.set(p, 5, timing[0]);
    //p += 5;
    //s.set(p, 5, timing[15]);
    //p += 5;
    //s.set(p, 5, timing[31]);

#ifdef TRG_DEBUG
//    unpackerOuterInside(input, s);
#endif

    delete[] b;
    return s;
  }

  TRGState
  TCFrontEnd::packerOuterOutside(const TRGState& input)
  {

    //...Input should be 48 hit pattern and 48x5 timing, total 288 bits...
    //
    // Wire numbers and TS ID
    //
    // outside
    //
    //    +--+--+--+--+-    -+--+--+--+--+--+--+
    //    |  47 |  46 | .... |  34 |  33 |  32 |
    //    +--+--+--+--+--+-    -+--+--+--+--+--+--+
    //       |  31 |  30 | ..... | 18 |  17 |  16 |
    //    +--+--+--+--+--+-    -+--+--+--+--+--+--+
    //    |  15 |  14 | .... |  2  |  1  |  0  |  <- second priority wire layer
    //    +--+--+--+--+-    -+--+--+--+--+--+--+
    //
    //       15    14   ....    2     1     0      <- partial TS ID
    //
    // inside
    //
    //

    //...Prepare a state for output...
    //TRGState s(48 + 16 * 5 + 16 * 5 + 3 * 5);  // 3*5 for missing wires
    TRGState s(48 + 16 * 5 + 16 * 5 + 2 * 5 + 38);  // 2*5 for edge fastest time, 38 dummy bits to fill to 256 bits


    //...Set up bool array...
    bool* b = new bool[input.size()];
    input.copy2bool(b);

    //...Naming...
    const bool* const hitptn = & b[0];    // 16x3 = 48 bits
    const bool* const timing[48] = {
      & b[48], & b[53], & b[58], & b[63],
      & b[68], & b[73], & b[78], & b[83],
      & b[88], & b[93], & b[98], & b[103],
      & b[108], & b[113], & b[118], & b[123],
      & b[128], & b[133], & b[138], & b[143],
      & b[148], & b[153], & b[158], & b[163],
      & b[168], & b[173], & b[178], & b[183],
      & b[188], & b[193], & b[198], & b[203],
      & b[208], & b[213], & b[218], & b[223],
      & b[228], & b[233], & b[238], & b[243],
      & b[248], & b[253], & b[258], & b[263],
      & b[268], & b[273], & b[278], & b[283]
    };

    //...Store hit pattern...
    s.set(0, 48, hitptn);
    unsigned p = 48;


    //...Second priority cell timing...
    for (unsigned i = 0; i < 16; i++) {
      s.set(p, 5, timing[i]);
      p += 5;
    }

    //...Fastest timing...
    const bool dummy[6] = {false, false, false, false, false, true};
    const TRGState wtDummy(6, dummy);
    for (unsigned i = 0; i < 18; i++) {      // 0 ... 15 for 16 standard ones, 16,17 for two edge areas.
      TRGState wt[5];

      if (i == 0) { // TS ID 0 has missing wires
        wt[0] = wtDummy;
        wt[1] = TRGState(5, timing[0]);
        wt[2] = wtDummy;
        wt[3] = TRGState(5, timing[16]);
        wt[4] = TRGState(5, timing[17]);

        //...Append 6th bit to indicate hit or not (no hit = 1)...
        if (! hitptn[0]) wt[1].set(5, true);
        if (! hitptn[16]) wt[3].set(5, true);
        if (! hitptn[17]) wt[4].set(5, true);
      } else if (i == 15) { // TS ID 15 has missing wires
        wt[0] = TRGState(5, timing[14]);
        wt[1] = TRGState(5, timing[15]);
        wt[2] = TRGState(5, timing[30]);
        wt[3] = TRGState(5, timing[31]);
        wt[4] = wtDummy;

        //...Append 6th bit to indicate hit or not (no hit = 1)...
        if (! hitptn[14]) wt[0].set(5, true);
        if (! hitptn[15]) wt[1].set(5, true);
        if (! hitptn[30]) wt[2].set(5, true);
        if (! hitptn[31]) wt[3].set(5, true);
      } else if (i == 16)  { // edge area 1 only for cell 16
        wt[0] = wtDummy;
        wt[1] = wtDummy;
        wt[2] = wtDummy;
        wt[3] = wtDummy;
        wt[4] = TRGState(5, timing[16]);
        //...Append 6th bit to indicate hit or not (no hit = 1)...
        if (! hitptn[16]) wt[4].set(5, true);
      } else if (i == 17)  { // edge area 2 for cell 15 and 31 only
        wt[0] = TRGState(5, timing[15]);
        wt[1] = wtDummy;
        wt[2] = TRGState(5, timing[31]);
        wt[3] = wtDummy;
        wt[4] = wtDummy;

        //...Append 6th bit to indicate hit or not (no hit = 1)...
        if (! hitptn[15]) wt[0].set(5, true);
        if (! hitptn[31]) wt[2].set(5, true);
      } else {
        wt[0] = TRGState(5, timing[i - 1]);
        wt[1] = TRGState(5, timing[i]);
        wt[2] = TRGState(5, timing[i + 15]);
        wt[3] = TRGState(5, timing[i + 16]);
        wt[4] = TRGState(5, timing[i + 17]);

        //...Append 6th bit to indicate hit or not (no hit = 1)...
        if (! hitptn[i - 1])  wt[0].set(5, true);
        if (! hitptn[i])      wt[1].set(5, true);
        if (! hitptn[i + 15]) wt[2].set(5, true);
        if (! hitptn[i + 16]) wt[3].set(5, true);
        if (! hitptn[i + 17]) wt[4].set(5, true);
      }

      //...Look for the fastest hit...
      unsigned fastest0 = 0;
      unsigned fastest1 = 0;
      if (wt[0] < wt[1])
        fastest0 = 0;
      else
        fastest0 = 1;
      if (wt[2] < wt[3])
        fastest1 = 2;
      else
        fastest1 = 3;

      unsigned fastest2 = 0;
      if (wt[fastest0] < wt[fastest1])
        fastest2 = fastest0;
      else
        fastest2 = fastest1;

      unsigned fastest3 = 0;
      if (wt[fastest2] < wt[4])
        fastest3 = fastest2;
      else
        fastest3 = 4;

      TRGState fastest(5);
      if (! wt[fastest3].active(5))
        fastest = wt[fastest3].subset(0, 5);

      s.set(p, fastest);
      p += 5;
    }

    //s.set(p, 5, timing[16]);
    //p += 5;
    //s.set(p, 5, timing[15]);
    //p += 5;
    //s.set(p, 5, timing[31]);

#ifdef TRG_DEBUG
//    unpackerOuterOutside(input, s);
#endif

    delete[] b;
    return s;
  }

  void
  TCFrontEnd::unpackerInnerInside(const TRGState& input,
                                  const TRGState& output)
  {

    cout << "Input bit size=" << input.size() << endl;

    cout << "Input : wire hit pattern" << endl;
    cout << "        ";
    for (unsigned i = 0; i < 48; i++) {
      const unsigned j = 48 - i - 1;
      if (i && ((i % 8) == 0))
        cout << "_";
      if (input[j])
        cout << "1";
      else
        cout << "0";
    }
    cout << endl;
    cout << "Input : wire hit timing" << endl;
    unsigned o = 48;
    for (unsigned i = 0; i < 48; i++) {
      TRGState s = input.subset(o + i * 5, 5);
      if ((i % 4) == 0)
        cout << "        ";
      cout << i << ": " << s << "  ";
      if ((i % 4) == 3)
        cout << endl;
    }

    cout << "Output bit size=" << output.size() << endl;

    cout << "Output : wire hit pattern" << endl;
    cout << "        ";
    for (unsigned i = 0; i < 32; i++) {
      const unsigned j = 32 - i - 1;
      if (i && ((i % 8) == 0))
        cout << "_";
      if (output[j])
        cout << "1";
      else
        cout << "0";
    }
    cout << endl;

    cout << "Output : priority cell timing" << endl;
    o = 32;
    for (unsigned i = 0; i < 16; i++) {
      TRGState s = output.subset(o + i * 5, 5);
      if ((i % 4) == 0)
        cout << "        ";
      cout << i << ": " << s << "  ";
      if ((i % 4) == 3)
        cout << endl;
    }

    cout << "Output : second priority cell position" << endl;
    cout << "        ";
    o = 112;
    for (unsigned i = 0; i < 16; i++) {
      TRGState s = output.subset(o + i, 1);
      if (i && ((i % 8) == 0))
        cout << "_";
      if (s.active())
        cout << "1";
      else
        cout << "0";
    }
    cout << endl;

    cout << "Output : fastest timing" << endl;
    o = 128;
    for (unsigned i = 0; i < 16; i++) {
      TRGState s = output.subset(o + i * 5, 5);
      if ((i % 4) == 0)
        cout << "        ";
      cout << i << ": " << s << "  ";
      if ((i % 4) == 3)
        cout << endl;
    }

    cout << "Output : timing of missing wires" << endl;
    o = 208;
    for (unsigned i = 0; i < 1; i++) {
      TRGState s = output.subset(o + i * 5, 5);
      if ((i % 4) == 0)
        cout << "        ";
      cout << i << ": " << s << "  ";
      if ((i % 4) == 3)
        cout << endl;
    }

    cout << endl;
  }

  void
  TCFrontEnd::unpackerInnerOutside(const TRGState& input,
                                   const TRGState& output)
  {

    cout << "Input bit size=" << input.size() << endl;

    cout << "Input : wire hit pattern" << endl;
    cout << "        ";
    for (unsigned i = 0; i < 48; i++) {
      const unsigned j = 48 - i - 1;
      if (i && ((i % 8) == 0))
        cout << "_";
      if (input[j])
        cout << "1";
      else
        cout << "0";
    }
    cout << endl;
    cout << "Input : wire hit timing" << endl;
    unsigned o = 48;
    for (unsigned i = 0; i < 48; i++) {
      TRGState s = input.subset(o + i * 5, 5);
      if ((i % 4) == 0)
        cout << "        ";
      cout << i << ": " << s << "  ";
      if ((i % 4) == 3)
        cout << endl;
    }

    cout << "Output bit size=" << output.size() << endl;

    cout << "Output : wire hit pattern" << endl;
    cout << "        ";
    for (unsigned i = 0; i < 48; i++) {
      const unsigned j = 48 - i - 1;
      if (i && ((i % 8) == 0))
        cout << "_";
      if (output[j])
        cout << "1";
      else
        cout << "0";
    }
    cout << endl;

    cout << "Output : fastest timing" << endl;
    o = 48;
    for (unsigned i = 0; i < 16; i++) {
      TRGState s = output.subset(o + i * 5, 5);
      if ((i % 4) == 0)
        cout << "        ";
      cout << i << ": " << s << "  ";
      if ((i % 4) == 3)
        cout << endl;
    }

    cout << "Output : timing of missing wires" << endl;
    o = 128;
    for (unsigned i = 0; i < 4; i++) {
      TRGState s = output.subset(o + i * 5, 5);
      if ((i % 4) == 0)
        cout << "        ";
      cout << i << ": " << s << "  ";
      if ((i % 4) == 3)
        cout << endl;
    }

    cout << endl;
  }

  void
  TCFrontEnd::unpackerOuterInside(const TRGState& input,
                                  const TRGState& output)
  {

    cout << "Input bit size=" << input.size() << endl;

    cout << "Input : wire hit pattern" << endl;
    cout << "        ";
    for (unsigned i = 0; i < 48; i++) {
      const unsigned j = 48 - i - 1;
      if (i && ((i % 8) == 0))
        cout << "_";
      if (input[j])
        cout << "1";
      else
        cout << "0";
    }
    cout << endl;
    cout << "Input : wire hit timing" << endl;
    unsigned o = 48;
    for (unsigned i = 0; i < 48; i++) {
      TRGState s = input.subset(o + i * 5, 5);
      if ((i % 4) == 0)
        cout << "        ";
      cout << i << ": " << s << "  ";
      if ((i % 4) == 3)
        cout << endl;
    }

    cout << "Output bit size=" << output.size() << endl;

    cout << "Output : wire hit pattern" << endl;
    cout << "        ";
    for (unsigned i = 0; i < 48; i++) {
      const unsigned j = 48 - i - 1;
      if (i && ((i % 8) == 0))
        cout << "_";
      if (output[j])
        cout << "1";
      else
        cout << "0";
    }
    cout << endl;

    cout << "Output : priority cell timing" << endl;
    o = 48;
    for (unsigned i = 0; i < 16; i++) {
      TRGState s = output.subset(o + i * 5, 5);
      if ((i % 4) == 0)
        cout << "        ";
      cout << i << ": " << s << "  ";
      if ((i % 4) == 3)
        cout << endl;
    }

    cout << "Output : fastest timing" << endl;
    o = 128;
    for (unsigned i = 0; i < 16; i++) {
      TRGState s = output.subset(o + i * 5, 5);
      if ((i % 4) == 0)
        cout << "        ";
      cout << i << ": " << s << "  ";
      if ((i % 4) == 3)
        cout << endl;
    }

    cout << "Output : timing of missing wires" << endl;
    o = 208;
    for (unsigned i = 0; i < 2; i++) {
      TRGState s = output.subset(o + i * 5, 5);
      if ((i % 4) == 0)
        cout << "        ";
      cout << i << ": " << s << "  ";
      if ((i % 4) == 3)
        cout << endl;
    }

    cout << endl;
  }

  void
  TCFrontEnd::unpackerOuterOutside(const TRGState& input,
                                   const TRGState& output)
  {

    cout << "Input bit size=" << input.size() << endl;

    cout << "Input : wire hit pattern" << endl;
    cout << "        ";
    for (unsigned i = 0; i < 48; i++) {
      const unsigned j = 48 - i - 1;
      if (i && ((i % 8) == 0))
        cout << "_";
      if (input[j])
        cout << "1";
      else
        cout << "0";
    }
    cout << endl;
    cout << "Input : wire hit timing" << endl;
    unsigned o = 48;
    for (unsigned i = 0; i < 48; i++) {
      TRGState s = input.subset(o + i * 5, 5);
      if ((i % 4) == 0)
        cout << "        ";
      cout << i << ": " << s << "  ";
      if ((i % 4) == 3)
        cout << endl;
    }

    cout << "Output bit size=" << output.size() << endl;

    cout << "Output : wire hit pattern" << endl;
    cout << "        ";
    for (unsigned i = 0; i < 48; i++) {
      const unsigned j = 48 - i - 1;
      if (i && ((i % 8) == 0))
        cout << "_";
      if (output[j])
        cout << "1";
      else
        cout << "0";
    }
    cout << endl;

    cout << "Output : Second priority cell timing" << endl;
    o = 48;
    for (unsigned i = 0; i < 16; i++) {
      TRGState s = output.subset(o + i * 5, 5);
      if ((i % 4) == 0)
        cout << "        ";
      cout << i << ": " << s << "  ";
      if ((i % 4) == 3)
        cout << endl;
    }

    cout << "Output : fastest timing" << endl;
    o = 128;
    for (unsigned i = 0; i < 16; i++) {
      TRGState s = output.subset(o + i * 5, 5);
      if ((i % 4) == 0)
        cout << "        ";
      cout << i << ": " << s << "  ";
      if ((i % 4) == 3)
        cout << endl;
    }

    cout << "Output : timing of missing wires" << endl;
    o = 208;
    for (unsigned i = 0; i < 2; i++) {
      TRGState s = output.subset(o + i * 5, 5);
      if ((i % 4) == 0)
        cout << "        ";
      cout << i << ": " << s << "  ";
      if ((i % 4) == 3)
        cout << endl;
    }
    cout << endl;
  }


  void //Dump all the details of _mosb into a .log file, do it in the end of simulate()
  TRGCDCFrontEnd::dump_log(void) const
  {

    if (type() == innerInside) dump_log_innerInside();
    else if (type() == innerOutside) dump_log_innerOutside();
    else if (type() == outerInside) dump_log_outerInside();
    else if (type() == outerOutside) dump_log_outerOutside();

  }

  void
  TRGCDCFrontEnd::dump_log_innerInside(void) const
  {

    const TRGClock& dClock = TRGCDC::getTRGCDC()->dataClock();
    ofstream output((name() + ".log").c_str());

    output << "InnerInside FrontEnd output dump" << endl << endl;

    const std::vector<int> changetime = _osb->stateChanges();
    std::vector<vector<int>> boolvector(changetime.size());
    for (unsigned ch_t = 0; ch_t < changetime.size(); ch_t++)       {

      for (unsigned b = 0; b < (* _osb)[0]->size(); b++) {
        boolvector[ch_t].push_back(((* _osb)[0]->state(changetime[ch_t])[b])  ? 1 : 0);
      }

      output << "# of clk: " << changetime[ch_t] << " (" << dClock.absoluteTime(changetime[ch_t]) << " ns), signal vector: " << endl;

      output << "Hitmap: " << endl ;
      for (unsigned b = 0; b < 32; b++) {
        output << boolvector[ch_t][31 - b] << " " ;
        if (b == 15) output << endl << " ";
      }
      output << endl;

      for (unsigned b = 0; b < 16; b++) {
        output << "PT#" << b << ": " << boolvector[ch_t][ 32 + 5 * b + 4 ] << boolvector[ch_t][ 32 + 5 * b + 3 ] << boolvector[ch_t][ 32 + 5
               * b + 2 ]
               << boolvector[ch_t][ 32 + 5 * b + 1 ] << boolvector[ch_t][ 32 + 5 * b] << endl;
      }
      output << endl;

      output << "Secondary: ";
      for (int b = 0; b < 16; b++) {
        output <<  boolvector[ch_t][ 127 - b ] ;
      }
      output << endl;

      for (unsigned b = 0; b < 16; b++) {
        output << "FT#" << b << ": " << boolvector[ch_t][ 128 + 5 * b + 4 ] << boolvector[ch_t][ 128 + 5 * b + 3 ] << boolvector[ch_t][ 128
               + 5 * b + 2 ]
               << boolvector[ch_t][ 128 + 5 * b + 1 ] << boolvector[ch_t][ 128 + 5 * b] << endl;
      }

      output << endl;
      output << "ET#0(cell 31): " << endl << boolvector[ch_t][212] << boolvector[ch_t][211] << boolvector[ch_t][210]
             << boolvector[ch_t][209] << boolvector[ch_t][208] << endl;

      output << endl;
    }

    output.close();

  }


  void
  TRGCDCFrontEnd::dump_log_innerOutside(void) const
  {

    const TRGClock& dClock = TRGCDC::getTRGCDC()->dataClock();
    ofstream output((name() + ".log").c_str());

    output << "InnerOutside FrontEnd output dump" << endl << endl;

    const std::vector<int> changetime = _osb->stateChanges();
    std::vector<vector<int>> boolvector(changetime.size());
    for (unsigned ch_t = 0; ch_t < changetime.size(); ch_t++)       {

      for (unsigned b = 0; b < (* _osb)[0]->size(); b++) {
        boolvector[ch_t].push_back(((* _osb)[0]->state(changetime[ch_t])[b])  ? 1 : 0);
      }

      output << "# of clk: " << changetime[ch_t] << " (" << dClock.absoluteTime(changetime[ch_t]) << " ns), signal vector: " << endl;

      output << "Hitmap: " << endl << " ";
      for (unsigned b = 0; b < 48; b++) {
        output << boolvector[ch_t][47 - b] << " ";
        if (b == 15) output << endl ;
        else if (b == 31) output << endl << " ";
      }
      output << endl;

      for (unsigned b = 0; b < 16; b++) {
        output << "FT#" << b << ": " << boolvector[ch_t][ 48 + 5 * b + 4 ] << boolvector[ch_t][ 48 + 5 * b + 3 ] << boolvector[ch_t][ 48 + 5
               * b + 2 ]
               << boolvector[ch_t][ 48 + 5 * b + 1 ] << boolvector[ch_t][ 48 + 5 * b] << endl;
      }

      output << endl;
      output << "ET#0(cell 32): " << endl << boolvector[ch_t][132] << boolvector[ch_t][131] << boolvector[ch_t][130]
             << boolvector[ch_t][129] << boolvector[ch_t][128] << endl;
      output << "ET#1(cell 0, 16, 32, 33): " << endl << boolvector[ch_t][137] << boolvector[ch_t][136] << boolvector[ch_t][135]
             << boolvector[ch_t][134] << boolvector[ch_t][133] << endl;
      output << "ET#0(cell 15, 30, 31, 46, 47): " << endl << boolvector[ch_t][142] << boolvector[ch_t][141] << boolvector[ch_t][140]
             << boolvector[ch_t][139] << boolvector[ch_t][138] << endl;
      output << "ET#3(cell 31, 47): " << endl << boolvector[ch_t][147] << boolvector[ch_t][146] << boolvector[ch_t][145]
             << boolvector[ch_t][144] << boolvector[ch_t][143] << endl;

      output << endl;
    }

    output.close();
  }


  void
  TRGCDCFrontEnd::dump_log_outerInside(void) const
  {

    const TRGClock& dClock = TRGCDC::getTRGCDC()->dataClock();
    ofstream output((name() + ".log").c_str());

    output << "OuterInside FrontEnd output dump" << endl << endl;

    const std::vector<int> changetime = _osb->stateChanges();
    std::vector<vector<int>> boolvector(changetime.size());
    for (unsigned ch_t = 0; ch_t < changetime.size(); ch_t++)       {

      for (unsigned b = 0; b < (* _osb)[0]->size(); b++) {
        boolvector[ch_t].push_back(((* _osb)[0]->state(changetime[ch_t])[b])  ? 1 : 0);
      }

      output << "# of clk: " << changetime[ch_t] << " (" << dClock.absoluteTime(changetime[ch_t]) << " ns), signal vector: " << endl;

      output << "Hitmap: " << endl << " ";
      for (unsigned b = 0; b < 48; b++) {
        output << boolvector[ch_t][47 - b] << " " ;
        if (b == 15) output << endl;
        else if (b == 31) output << endl << " ";
      }
      output << endl;

      for (unsigned b = 0; b < 16; b++) {
        output << "PT#" << b << ": " << boolvector[ch_t][ 48 + 5 * b + 4 ] << boolvector[ch_t][ 48 + 5 * b + 3 ] << boolvector[ch_t][ 48 + 5
               * b + 2 ]
               << boolvector[ch_t][ 48 + 5 * b + 1 ] << boolvector[ch_t][ 48 + 5 * b] << endl;
      }
      output << endl;

      for (unsigned b = 0; b < 16; b++) {
        output << "FT#" << b << ": " << boolvector[ch_t][ 128 + 5 * b + 4 ] << boolvector[ch_t][ 128 + 5 * b + 3 ] << boolvector[ch_t][ 128
               + 5 * b + 2 ]
               << boolvector[ch_t][ 128 + 5 * b + 1 ] << boolvector[ch_t][ 128 + 5 * b] << endl;
      }

      output << endl;
      output << "ET#0(cell 0): " << endl << boolvector[ch_t][212] << boolvector[ch_t][211] << boolvector[ch_t][210]
             << boolvector[ch_t][209] << boolvector[ch_t][208] << endl;
      output << "ET#1(cell 15, 31): " << endl << boolvector[ch_t][217] << boolvector[ch_t][216] << boolvector[ch_t][215]
             << boolvector[ch_t][214] << boolvector[ch_t][213] << endl;

      output << endl;
    }

    output.close();

  }

  void
  TRGCDCFrontEnd::dump_log_outerOutside(void) const
  {

    const TRGClock& dClock = TRGCDC::getTRGCDC()->dataClock();
    ofstream output((name() + ".log").c_str());

    output << "OuterOutside FrontEnd output dump" << endl << endl;
    const std::vector<int> changetime = _osb->stateChanges();
    std::vector<vector<int>> boolvector(changetime.size());
    for (unsigned ch_t = 0; ch_t < changetime.size(); ch_t++)       {

      for (unsigned b = 0; b < (* _osb)[0]->size(); b++) {
        boolvector[ch_t].push_back(((* _osb)[0]->state(changetime[ch_t])[b])  ? 1 : 0);
      }

      output << "# of clk: " << changetime[ch_t] << " (" << dClock.absoluteTime(changetime[ch_t]) << " ns), signal vector: " << endl;

      output << "Hitmap: " << endl;
      for (unsigned b = 0; b < 48; b++) {
        output << boolvector[ch_t][47 - b] << " ";
        if (b == 15) output << endl << " ";
        else if (b == 31) output << endl ;
      }
      output << endl;

      for (unsigned b = 0; b < 16; b++) {
        output << "PT#" << b << ": " << boolvector[ch_t][ 48 + 5 * b + 4 ] << boolvector[ch_t][ 48 + 5 * b + 3 ] << boolvector[ch_t][ 48 + 5
               * b + 2 ]
               << boolvector[ch_t][ 48 + 5 * b + 1 ] << boolvector[ch_t][ 48 + 5 * b] << endl;
      }
      output << endl;

      for (unsigned b = 0; b < 16; b++) {
        output << "FT#" << b << ": " << boolvector[ch_t][ 128 + 5 * b + 4 ] << boolvector[ch_t][ 128 + 5 * b + 3 ] << boolvector[ch_t][ 128
               + 5 * b + 2 ]
               << boolvector[ch_t][ 128 + 5 * b + 1 ] << boolvector[ch_t][ 128 + 5 * b] << endl;
      }

      output << endl;
      output << "ET#0(cell 16): " << endl << boolvector[ch_t][212] << boolvector[ch_t][211] << boolvector[ch_t][210]
             << boolvector[ch_t][209] << boolvector[ch_t][208] << endl;
      output << "ET#1(cell 15, 31): " << endl << boolvector[ch_t][217] << boolvector[ch_t][216] << boolvector[ch_t][215]
             << boolvector[ch_t][214] << boolvector[ch_t][213] << endl;


      output << endl;
    }


    output.close();
  }



} // namespace Belle2
