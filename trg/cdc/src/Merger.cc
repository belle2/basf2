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
// Filename : Merger.cc
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a CDC Merger board
//-----------------------------------------------------------------------------
// $Log$
// 2013,1005 first working verion 0.00
//      1014 added the unpacker part
//-----------------------------------------------------------------------------

#define TRGCDC_SHORT_NAMES

#include <algorithm>
#include <iostream>
#include <iomanip>
#include "trg/trg/Debug.h"
#include "trg/trg/State.h"
#include "trg/trg/Channel.h"
#include "trg/cdc/TRGCDC.h"
#include "trg/cdc/Merger.h"
#include "trg/cdc/FrontEnd.h"
#include "trg/cdc/Wire.h"

using namespace std;

namespace Belle2 {

  TRGCDCMerger::TRGCDCMerger(const std::string& name,
                             unitType type,
                             const TRGClock& systemClock,
                             const TRGClock& dataClock,
                             const TRGClock& userClockInput,
                             const TRGClock& userClockOutput)
    : TRGBoard(name, systemClock, dataClock, userClockOutput, userClockOutput),
      _type(type),
      _misb(0),
      _mosb(0)
  {
    userClockInput.name(); //jb
  }

  TRGCDCMerger::~TRGCDCMerger()
  {
  }

  string
  TRGCDCMerger::version(void)
  {
    return ("TRGCDCMerger version 0.00");
  }

  TRGCDCMerger::unitType
  TRGCDCMerger::type(void) const
  {
    return _type;
  }


  int
  TRGCDCMerger::implementation(const TRGCDCMerger::unitType& type,
                               ofstream& outfile)
  {

    // string cname = "UNKNOWN";
    //    if (_type == innerType)
    outfile << "-- inner type" << endl;
    string cname = "TRGCDCMergerInner";
    //     }
    //     else {
    //         outfile << "-- outer type" << endl;
    //         cname = "TRGCDCMergerOuter";
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
  TRGCDCMerger::implementationPort(const TRGCDCMerger::unitType&,
                                   ofstream& outfile)
  {
    outfile << "    port (" << endl;

    //...125MHz clock (TRG system clock)...
    outfile << "       -- 127.216MHz clock (TRG system clock)" << endl;
    outfile << "          clk : in std_logic;" << endl;

    //...Coarse timing...
    outfile << "       -- Coarse timing (counter with 127.216MHz clock)" << endl;
    outfile << "          tmc : in std_logic_vector(0 to 7);" << endl;

    //...Hit pattern...
    outfile << "       -- Hit pattern(80 bits for 80 wires)" << endl;
    outfile << "          ptn : in std_logic_vector(0 to 80);" << endl;

    //...Fine timing for each wire...
    /*
    outfile << "       -- Fine timing within 127.216MHz clock" << endl;
    for (unsigned i = 0; i < 80; i++) {
      outfile << "          w" << TRGUtil::itostring(i)
        << " : in std_logic_vector(0 to 3);" << endl;
    }
    */

    //...Hit pattern output...
    outfile << "       -- Hit pattern output" << endl;
    outfile << "          hit : out std_logic_vector(0 to 80);" << endl;
    outfile << "       -- 4 bit fine timing for 16 priority wires" << endl;
    outfile << "          pri : out std_logic_vector(0 to 63);" << endl;
    outfile << "       -- 2 bit fine timing for the fastest hit in 16 TS"   << endl;
    outfile << "          fst : out std_logic_vector(0 to 31)" << endl;
    outfile << "    );" << endl;

    return 0;
  }

  void
  TRGCDCMerger::push_back(const TRGCDCFrontEnd* a)
  {
    std::vector<const TRGCDCFrontEnd*>::push_back(a);
  }

  void
  TRGCDCMerger::dump(const string& message, const string& pre) const
  {
    TRGBoard::dump(message, pre);
  }

  void
  TRGCDCMerger::simulate(void)
  {

    TRGDebug::enterStage("Merger simulattion");

    //...Clear input signal bundle...
    if (_misb) {
      for (unsigned i = 0; i < _misb->size(); i++)
        delete(* _misb)[i];
      delete _misb;
    }
    if (_mosb) {
      for (unsigned i = 0; i < _mosb->size(); i++)
        delete(* _mosb)[i];
      delete _mosb;
    }

    //...Clock...
    const TRGClock& dClock = clockData();

    // ... Make input signal bundle .... ???
    TRGSignalVector input(name() + "inputFrontEnds", dClock);
    const string ni = name() + "InputSignalBundle";
    _misb = new TRGSignalBundle(ni, dClock);


    // Input from FrontEnds
    //  const unsigned nFrontEnds = size();

    //  TRGSignalBundle*  testin = ((*this)[0])->_osb;
    //  TRGSignalBundle  testin2 = new TRGSignalBundle( ((*this)[0])->output() );
    //TRGSignalBundle * testin = new TRGSignalBundle * ((*this)[0]->output());
    //TRGSignalVector * input1 = new TRGSignalVector ( *(((*this)[0]->_osb)[0])[0] ),
    //                * input2 = new TRGSignalVector ( *(((*this)[1]->_osb)[0])[0] );


    // _osb in FrontEnd.h is private, use getOSB() to get input bundle.
    TRGSignalVector* input1 = new TRGSignalVector(*((*(*this)[0]->getOSB())[0]));
    TRGSignalVector* input2 = new TRGSignalVector(*((*(*this)[1]->getOSB())[0]));


    for (unsigned s = 0; s < input1->size(); s++)  {
      TRGSignal ws = (*input1)[s];
      input += ws;
    }
    for (unsigned s = 0; s < input2->size(); s++)  {
      TRGSignal ws = (*input2)[s];
      input += ws;
    }

    //...Create FrontEnd output data
    TRGSignalVector* fedata = new TRGSignalVector(input);
    fedata->clock(dClock);
    fedata->name(name() + "@dataClock");
    _misb->push_back(fedata);

//  cout<<"Merger input start"<<endl;
//  _misb->dump("detail", TRGDebug::tab());
//  cout<<"Merger input end"<<endl;

    //...Data clock...
    // Data clock position data is omitted. Is this problem?

    //...Make output signal bundle...
    const string no = name() + "OutSigBundle";
    if (type() == innerType) {
      _mosb = new TRGSignalBundle(no,
                                  dClock,
                                  * _misb,
                                  256,
                                  TCMerger::packerInner);
    } else if (type() == outerType) {
      _mosb = new TRGSignalBundle(no,
                                  dClock,
                                  * _misb,
                                  256,
                                  TCMerger::packerOuter);
    }
    mosb = _mosb;

    if (TRGCDC::getTRGCDC()->firmwareSimulationMode() & 0x2)
      mosb->dumpCOE("",
                    TRGCDC::getTRGCDC()->firmwareSimulationStartDataClock(),
                    TRGCDC::getTRGCDC()->firmwareSimulationStopDataClock());

    //...Output to a channel...
    output(0)->signal(_mosb);

    // Terminate
    delete input1;
    delete input2;

    if (TRGDebug::level() > 1) {
      _misb->dump("", TRGDebug::tab());
      _mosb->dump("", TRGDebug::tab());
    }

    TRGDebug::leaveStage("Merger simulattion");
  }


  TRGState
  TCMerger::packerInner(const TRGState& input)
  {

    //...inputInsdie should be 32 hit pattern and 2x16x5 timing, total 192 bits...
    //   plus edge information
    //
    // Wire numbers and TS ID
    //
    // outside
    //
    //       +--+--+--+--+-    -+--+--+--+--+--+--+
    //       |  79 |  78 | .... |  66 |  65 |  64 |
    //    +--+--+--+--+--+-    -+--+--+--+--+--+--+
    //    |  63 |  62 | ..... | 50 |  49 |  48 |
    //    +--+--+--+--+--+-    -+--+--+--+--+--+--+
    //       |  47 |  46 | .... |  34 |  33 |  32 |
    //       +--+--+--+--+-    -+--+--+--+--+--+--+
    //
    //    +--+--+--+--+-    -+--+--+--+--+--+--+
    //    |  31 |  30 | .... |  18 |  17 |  16 |
    //    +--+--+--+--+--+-    -+--+--+--+--+--+--+
    //       |  15 |  14 | ..... | 2  |  1  |  0  |
    //    +--+--+--+--+--+-    -+--+--+--+--+--+--+
    //    |  x  |  x  | .... |  x  |  x  |  x  |
    //    +--+--+--+--+-    -+--+--+--+--+--+--+
    //
    //          15    14   ....    2     1     0      <- partial TS ID, aligned with the 2nd row
    //
    // inside
    //
    //...Prepare a state for output...
    // hit map:            80x1
    // priority hit time:  16x4
    // fastest hit time:   16x4
    // 2nd proi. hit flag: 16x1
    // edge time info:      5x4
    // cc:                  1x9
    // N/A and reserved:   fill to total 256 bits

    TRGState s(80 + 16 * 4 + 16 * 4 + 16 + 5 * 4 + 1 * 9 + 3); // fill to 256 bits

    //...Set up bool array...
    bool* binput = new bool[input.size()];
    input.copy2bool(binput);
    bool* binside = & binput[0];
    bool* boutside = & binput[256];     // half of the input.size(), should be 256


    // ... hitmap ...
    const bool* const hitptn0 = & binside[0];    // 16x2 = 32 bits
    const bool* const hitptn1 = & boutside[0];   // 16x3 = 48 bits
    s.set(0, 32, hitptn0);
    s.set(32, 48, hitptn1);
    unsigned p = 80;

    bool PTS_in_hit[16];
    bool PTS_out_hit[16];
    for (unsigned i = 0; i < 16; i++) {
      if (i == 0) {
        PTS_in_hit[i] = hitptn0[0] || hitptn0[16];
        PTS_out_hit[i] = hitptn1[0] || hitptn1[1] || hitptn1[16] || hitptn1[17] || hitptn1[32] || hitptn1[33] || hitptn1[34];
      } else if (i == 1) {
        PTS_in_hit[i] = hitptn0[1] || hitptn0[16] || hitptn0[17];
        PTS_out_hit[i] = hitptn1[0] || hitptn1[1] || hitptn1[2] || hitptn1[16] || hitptn1[17] || hitptn1[18] || hitptn1[32] || hitptn1[33]
                         || hitptn1[34] || hitptn1[35];
      } else if (i == 14) {
        PTS_in_hit[i] = hitptn0[14] || hitptn0[29] || hitptn0[30];
        PTS_out_hit[i] = hitptn1[13] || hitptn1[14] || hitptn1[15] || hitptn1[28] || hitptn1[29] || hitptn1[30] || hitptn1[31]
                         || hitptn1[44] || hitptn1[45] || hitptn1[46] || hitptn1[47];
      } else if (i == 15) {
        PTS_in_hit[i] = hitptn0[15] || hitptn0[30] || hitptn0[31];
        PTS_out_hit[i] = hitptn1[14] || hitptn1[15] || hitptn1[29] || hitptn1[30] || hitptn1[31] || hitptn1[45] || hitptn1[46]
                         || hitptn1[47];
      } else {
        PTS_in_hit[i] = hitptn0[i] || hitptn0[i + 15] || hitptn0[i + 16];
        PTS_out_hit[i] = hitptn1[i - 1] || hitptn1[i] || hitptn1[i + 1] || hitptn1[i + 14] || hitptn1[i + 15] || hitptn1[i + 16]
                         || hitptn1[i + 17] || hitptn1[i + 30] || hitptn1[i + 31] || hitptn1[i + 32] || hitptn1[i + 33] || hitptn1[i + 34];
      }
    }


    // ... priority time ... and 2nd priority location .... already prepared in FrontEnd packer
    const bool* const pt[16] = {
      &binside[33],  &binside[38],  &binside[43],  &binside[48],
      &binside[53],  &binside[58],  &binside[63],  &binside[68],
      &binside[73],  &binside[78],  &binside[83],  &binside[88],
      &binside[93],  &binside[98],  &binside[103], &binside[108]
    };

    for (unsigned i = 0; i < 16; i++) {
      s.set(p, 4, pt[i]);
      p += 4;
    }

    // ... fastest time ...
    const bool* const ftinside[16] = {
      &binside[128], &binside[133], &binside[138], &binside[143],
      &binside[148], &binside[153], &binside[158], &binside[163],
      &binside[168], &binside[173], &binside[178], &binside[183],
      &binside[188], &binside[193], &binside[198], &binside[203]
    };
    const bool* const ftoutside[16] = {
      &boutside[48], &boutside[53], &boutside[58], &boutside[63],
      &boutside[68], &boutside[73], &boutside[78], &boutside[83],
      &boutside[88], &boutside[93], &boutside[98], &boutside[103],
      &boutside[108], &boutside[113], &boutside[118], &boutside[123]
    };


    // need to consider if that area is hit
    for (unsigned i = 0; i < 16; i++) {
      TRGState st[2];
      st[0] = TRGState(5, ftinside[i]);
      st[1] = TRGState(5, ftoutside[i]);
      if (!PTS_in_hit[i]) st[0].set(5, true);
      if (!PTS_out_hit[i])   st[1].set(5, true);

      if (PTS_in_hit[i] || PTS_out_hit[i]) {
        if (st[0] < st[1]) {
          s.set(p, 4, &binside[129 + i * 5]);
        } else {
          s.set(p, 4, &boutside[49 + i * 5]);
        }
      } else {
      }
      p += 4;
    }

    // ... 2nd priority location ...
    s.set(p, 16, &binside[112]);
    p += 16;

    // ... edge information ...

    if (hitptn0[31])  {
      s.set(p, 4, &binside[209]);     // 4-bit hit time of cell[31]
    } else {
    }
    p += 4;


    if (hitptn1[32])  {
      s.set(p, 4, &boutside[129]);     // 4-bit hit time of cell[64]
    } else {
    }
    p += 4;


    if (hitptn1[0] || hitptn1[16] || hitptn1[32] || hitptn1[33])  {
      s.set(p, 4, &boutside[134]);     // edge info. purely from outside FE. cell 32, 48, 64, 65
    } else {
    }
    p += 4;



    TRGState et[2];                          // edge info. from both FE, cell 31 and 47, 62, 63, 78, 79
    et[0] = TRGState(5, &binside[208]);
    et[1] = TRGState(5, &boutside[138]);

    if ((!hitptn1[15]) && (!hitptn1[30]) && (!hitptn1[31]) && (!hitptn1[46]) && (!hitptn1[47])) et[1].set(5, true);
    if ((!hitptn0[31])) et[0].set(5, true);

    if (hitptn0[31] ||  hitptn1[15] || hitptn1[30] || hitptn1[31] || hitptn1[46] || hitptn1[47])  {
      if (et[0] < et[1]) {
        s.set(p, 4, &binside[209]);
      } else {
        s.set(p, 4, &boutside[139]);
      }
    } else {
    }

    p += 4;



    if (hitptn1[31] || hitptn1[47]) {
      s.set(p, 4, &boutside[144]);   // edge info. purely from outside FE. cell 63, 79
    } else {
    }
//p+=4;

    // ...clock counter ...
    // no process for cc at this moment

    // ...N/A and reserved bit ...
    // no process for these at this moment

//...Debug...
    if (TRGDebug::level() > 1) {
      unpackerInner(input, s);
    }

//...Termination...
    delete[] binput;

    return s;
  }




  TRGState
  TCMerger::packerOuter(const TRGState& input)
  {

    // packerOuter: for SuperLayer 1 - 8
    //
    //   inputInsdie should be 48 hit pattern and 1x16x5 + 1x16x5 timing,
    //   plus edge information
    //   total 192 bits...     (whole data pack 256 bits)
    //
    //   inputOutside should be 48 hit pattern and 1x16x5 + 1x16x5 timing,
    //   plus edge information
    //   total 192 bits...     (whole data pack 256 bits)
    //
    // Wire numbers and TS ID
    //
    // Outside
    //
    //    +--+--+--+--+-    -+--+--+--+--+--+--+
    //    |  x  |  x  | .... |  x  |  x  |  x  |
    //    +--+--+--+--+--+-    -+--+--+--+--+--+--+
    //       |  79 |  78 | ..... | 66 |  65 |  64 |
    //    +--+--+--+--+--+-    -+--+--+--+--+--+--+
    //    |  63 |  62 | .... |  50 |  49 |  48 |
    //    +--+--+--+--+--+-    -+--+--+--+--+--+
    //
    //       +--+--+--+--+-    -+--+--+--+--+--+--+
    //       |  47 |  46 | .... |  34 |  33 |  32 |
    //    +--+--+--+--+--+-    -+--+--+--+--+--+--+
    //    |  31 |  30 | ..... | 18 |  17 |  16 |
    //    +--+--+--+--+--+-    -+--+--+--+--+--+--+
    //       |  15 |  14 | .... |  2  |  1  |  0  |
    //       +--+--+--+--+-    -+--+--+--+--+--+--+
    //
    //          15    14   ....    2     1     0      <- partial TS ID
    //
    //...Prepare a state for output...
    // hit map:            80x1
    // priority hit time:  16x4
    // fastest hit time:   16x4
    // 2nd proi. hit flag: 16x1
    // edge time info:      3x4
    // cc:                  1x9
    // N/A and reserved:   fill to total 256 bits
    TRGState s(80 + 16 * 4 + 16 * 4 + 16 + 3 * 4 + 9 + 11);


    bool* binput = new bool[input.size()];
    input.copy2bool(binput);
    bool* binside = & binput[0];
    bool* boutside = & binput[256];     // half of the input.size(), should be 256


    // ... hitmap ...
    const bool* const hitptn0 = & binside[0];   // 16x3 = 48 bits
    const bool* const hitptn1 = & boutside[0];  // 16x2 = 32 bits
    s.set(0, 48, hitptn0);
    s.set(48, 32, hitptn1);

    bool PTS_in_hit[16];
    bool PTS_out_hit[16];
    for (unsigned i = 0; i < 16; i++) {
      if (i == 0) {
        PTS_in_hit[i] = hitptn0[0] || hitptn0[1] || hitptn0[16] || hitptn0[32] ;
        PTS_out_hit[i] = hitptn1[0] || hitptn1[16] || hitptn1[17];
      } else if (i == 15) {
        PTS_in_hit[i] = hitptn0[14] || hitptn0[15] || hitptn0[30] || hitptn0[31] || hitptn0[47] ;
        PTS_out_hit[i] = hitptn1[14] || hitptn1[15] || hitptn1[30] || hitptn1[31];
      } else {
        PTS_in_hit[i] = hitptn0[i - 1] || hitptn0[i] || hitptn0[i + 1] || hitptn0[i + 15] || hitptn0[i + 16] || hitptn0[i + 32];
        PTS_out_hit[i] = hitptn1[i - 1] || hitptn1[i] || hitptn1[i + 15] || hitptn1[i + 16] || hitptn1[i + 17];
      }
    }

    // ... priority time ...
    /*
    const bool * const ptinside[16] = {
      &binside[49],  &binside[54],  &binside[59],  &binside[64],
      &binside[69],  &binside[74],  &binside[79],  &binside[84],
      &binside[89],  &binside[94],  &binside[99],  &binside[104],
      &binside[109], &binside[114], &binside[119], &binside[124]
    };
    const bool * const ptoutside[16] = {
      &boutside[49],  &boutside[54],  &boutside[59],  &boutside[64],
      &boutside[69],  &boutside[74],  &boutside[79],  &boutside[84],
      &boutside[89],  &boutside[94],  &boutside[99],  &boutside[104],
      &boutside[109], &boutside[114], &boutside[119], &boutside[124]
    };
    */

    // i = 0...15
    // inside binside[31+i]
    // outside boutside[i]
//  const bool scflag[2] = {false, true};
    const bool dummy[6] = {false, false, false, false, false, true};
    const TRGState stDummy(6, dummy);
    unsigned p = 80;

    for (unsigned i = 0; i < 16; i++) {
      //unsigned sc=0;
      if (hitptn0[32 + i])  {
        s.set(p, 4, &binside[49 + 5 * i]);;
      } else {
        // decide which 2nd priority time to use if 1st priority cell not hit
        TRGState st[2];
        if (i == 0) {
          if (hitptn1[0]) {
            s.set(p, 4, &boutside[49]);
//          sc = 1;
            s.set(208 + i, 1, true);
          } else {
//          sc = 0;
//    s.set(209+i, 1, &scflag[sc]);
          }
        } else {
          st[0] = TRGState(5, &boutside[43 + i * 5]);
          st[1] = TRGState(5, &boutside[48 + i * 5]);
          if (!hitptn1[i - 1]) st[0].set(5, true);
          if (!hitptn1[i])   st[1].set(5, true);

          if (hitptn1[i] || hitptn1[i - 1]) {
            if (st[1] < st[0]) {
              s.set(p, 4, &boutside[49 + i * 5]);
              s.set(208 + i, 1, true); // only set scflag to true when left is it and also faster
            } else {
//      sc = 1;
              s.set(p, 4, &boutside[44 + i * 5]);
            }
          } else {
          }


        }

      }
//    s.set(209+i, 1, &scflag[sc]);  //the scflag is only consider when (1st not hit) && (2nd is hit)
      p += 4;
    }


    // ... fastest time ...




    /*
    const bool * const ftinside[16] = {
      &binside[129], &binside[134], &binside[139], &binside[144],
      &binside[149], &binside[154], &binside[159], &binside[164],
      &binside[169], &binside[174], &binside[179], &binside[184],
      &binside[189], &binside[194], &binside[199], &binside[204]
    };
    const bool * const ftoutside[16] = {
      &boutside[129], &boutside[134], &boutside[139], &boutside[144],
      &boutside[149], &boutside[154], &boutside[159], &boutside[164],
      &boutside[169], &boutside[174], &boutside[179], &boutside[184],
      &boutside[189], &boutside[194], &boutside[199], &boutside[204]
    };
    */
    // need to consider if those areas are hit
    p = 144;
    for (unsigned i = 0; i < 16; i++) {
      TRGState st[2];
      st[0] = TRGState(5, &binside[128 + i * 5]);
      st[1] = TRGState(5, &boutside[128 + i * 5]);

      if (!PTS_in_hit[i]) st[0].set(5, true);
      if (!PTS_out_hit[i])   st[1].set(5, true);

      if (PTS_in_hit[i] || PTS_out_hit[i]) {
        if (st[0] < st[1]) {
          s.set(p, 4, &binside[129 + i * 5]);
        } else {
          s.set(p, 4, &boutside[129 + i * 5]);
        }
      } else {
      }
      /*
            if ( st[0] < st[1] ) {
          s.set(p, 4, &binside[129 + i*5]);
            }
            else {
          s.set(p, 4, &boutside[129 + i*5]);
            }
      */

      p += 4;
    }

    // ... edge information ...
    /*
     const bool * const etinside[16] = {&binside[209], &binside[214] };
     const bool * const etoutside[16] = {&boutside[209], &boutside[214] };
     */
    p = 224;

    if (hitptn1[15]) {
      s.set(p, 4, &boutside[124]);    // 4-bit hit time of cell[63]
    }

    for (unsigned i = 0; i < 2; i++) {
      TRGState et[2];
      // need to consider if the two areas hit
      et[0] = TRGState(5, &binside[208 + i * 5]);
      et[1] = TRGState(5, &boutside[208 + i * 5]);

      if (i == 0) {
        if (!hitptn0[0]) et[0].set(5, true);
        if (!hitptn1[16])   et[1].set(5, true);
        p += 4;
        if (hitptn0[0] || hitptn1[16]) {
          if (et[0] < et[1]) {
            s.set(p, 4, &binside[209 + i * 5]);
          } else {
            s.set(p, 4, &boutside[209 + i * 5]);
          }
        } else {
        }

      } else if (i == 1) {
        if (!hitptn0[15] && !hitptn0[31]) et[0].set(5, true);
        if (!hitptn1[15] && !hitptn1[31])   et[1].set(5, true);
        p += 4;
        if (hitptn0[15] || hitptn0[31] || hitptn1[15] || hitptn1[31]) {
          if (et[0] < et[1]) {
            s.set(p, 4, &binside[209 + i * 5]);
          } else {
            s.set(p, 4, &boutside[209 + i * 5]);
          }
        } else {
        }


      }
      //      p += 4;
    }  // warning, here 'p' is at the beginning position of the last sensible data


    // ...clock counter ...
    // no process for cc at this moment

    // ...N/A and reserved bit ...
    // no process for these at this moment


//...Debug...
    if (TRGDebug::level() > 1) {
      unpackerOuter(input, s);
    }

//...Termination...
    delete[] binput;
    return s;

  }




  void
  TCMerger:: unpackerInner(const TRGState& input,
                           const TRGState& output)
  {

    TRGState inputInside = input.subset(0, 256);
    TRGState inputOutside = input.subset(256, 256);

    unsigned ipos = 0, o = 0;

    cout << "======================= Merger unpackerInner ================================= " << endl;
    cout << "input bit information: " << endl;
    for (unsigned bi = 0; bi < input.size(); bi++) {
      if (input[bi]) cout << "* " ;
      else cout << ". ";
      if ((bi % 16) == 15) cout << endl;
      if (bi == 255) cout << "--------------------------------" << endl;
    }

    cout << "Input bit size = " << input.size() << " and Output bit size =" << output.size() << endl << endl;

    cout << "Hit map: " << endl;
    cout << "inputOutside : wire hit pattern" << endl;
    cout << "        ";
    for (unsigned i = 0; i < 48; i++) {
      const unsigned j = 47 - i;
      if (i && ((i % 8) == 0))
        cout << "_";
      if (inputOutside[j])
        cout << "*";
      else
        cout << ".";
    }
    cout << endl;
    cout << "inputInside : wire hit pattern" << endl;
    cout << "        ";
    for (unsigned i = 0; i < 32; i++) {
      const unsigned j = 31 - i;
      if (i && ((i % 8) == 0))
        cout << "_";
      if (inputInside[j])
        cout << "*";
      else
        cout << ".";
    }
    cout << endl;
    cout << "-------------------------------------------" << endl;
    cout << "Output : wire hit pattern" << endl;
    cout << "        ";
    for (unsigned i = 0; i < 80; i++) {
      const unsigned j = 79 - i;
      if ((i % 16) == 0) {
        cout << endl << "        ";
        if (int(i / 16) % 2) {
        } else cout << " ";
      }
      if (output[j])
        cout << "O ";
      else
        cout << ". ";
    }
    cout << endl;
    cout << " -=+=-=+=-=-=+=-=+=-=-=+=-=+=-=-=+=-=+=-=-=+=-=+=-" << endl << endl;

    cout << "Priority cell timing:" << endl;
    cout << "inputOutside : no priority cell timing information" << endl;
    cout << "inputInside : priority cell timing" << endl;
    ipos = 32;
    for (unsigned i = 0; i < 16; i++) {
      TRGState s = inputInside.subset(ipos + i * 5, 5);
      if ((i % 4) == 0)
        cout << "        ";
      cout << setw(2) << i << ": " << s << "  ";
      if ((i % 4) == 3)
        cout << endl;
    }
    cout << "inputInsidepriority cell location flag" << endl;
    ipos = 112;
    for (unsigned i = 0; i < 16; i++) {
      if ((i % 4) == 0)
        cout << "        ";
      if (inputInside[ipos + i])
        cout << setw(2) << i << ": (1)Left    ";
      else
        cout << setw(2) << i << ": (0)Right   ";
      if ((i % 4) == 3)
        cout << endl;
    }

    cout << "ouput : priority cell timing" << endl;
    o = 80;
    for (unsigned i = 0; i < 16; i++) {
      TRGState s = output.subset(o + i * 4, 4);
      if ((i % 4) == 0)
        cout << "        ";
      cout << setw(2) << i << ": " << s << "   ";
      if ((i % 4) == 3)
        cout << endl;
    }
    cout << "output : priority cell location flag" << endl;
    o = 208;
    for (unsigned i = 0; i < 16; i++) {
      if ((i % 4) == 0)
        cout << "        ";
      if (output[o + i])
        cout << setw(2) << i << ": (1)Left    ";
      else
        cout << setw(2) << i << ": (0)Right   ";
      if ((i % 4) == 3)
        cout << endl;
    }
    cout << " -=+=-=+=-=-=+=-=+=-=-=+=-=+=-=-=+=-=+=-=-=+=-=+=-=" << endl << endl;

    cout << "Fastest timing of each TrackSegment: " << endl;
    cout << "inputOutside : fastest timing" << endl;
    ipos = 128;
    for (unsigned i = 0; i < 16; i++) {
      TRGState s = inputOutside.subset(ipos + i * 5, 5);
      if ((i % 4) == 0)
        cout << "        ";
      cout << setw(2) << i << ": " << s << "  ";
      if ((i % 4) == 3)
        cout << endl;
    }
    ipos = 48;
    cout << "inputInside : fastest timing" << endl;
    for (unsigned i = 0; i < 16; i++) {
      TRGState s = inputInside.subset(ipos + i * 5, 5);
      if ((i % 4) == 0)
        cout << "        ";
      cout << setw(2) << i << ": " << s << "  ";
      if ((i % 4) == 3)
        cout << endl;
    }
    cout << "output : fastest timing" << endl;
    o = 144;
    for (unsigned i = 0; i < 16; i++) {
      TRGState s = output.subset(o + i * 4, 4);
      if ((i % 4) == 0)
        cout << "        ";
      cout << setw(2) << i << ": " << s << "   ";
      if ((i % 4) == 3)
        cout << endl;
    }
    cout << " -=+=-=+=-=-=+=-=+=-=-=+=-=+=-=-=+=-=+=-=-=+=-=+=-=" << endl << endl;

    cout << "Edge time information: " << endl;
    cout << "inputOutside : timing of missing wires" << endl;
    ipos = 128;
    for (unsigned i = 0; i < 4; i++) {
      TRGState s = inputOutside.subset(ipos + i * 5, 5);
      if ((i % 4) == 0)
        cout << "        ";
      cout << setw(2) << i << ": " << s << "  ";
      if ((i % 4) == 3)
        cout << endl;
    } cout << endl;
    ipos = 208;
    cout << "inputInside : timing of missing wires" << endl;
    for (unsigned i = 0; i < 1; i++) {
      TRGState s = inputInside.subset(ipos + i * 5, 5);
      if ((i % 4) == 0)
        cout << "        ";
      cout << setw(2) << i << ": " << s << "  ";
      if ((i % 4) == 3)
        cout << endl;
    }  cout << endl;
    cout << "output : timing of missing wires" << endl;
    o = 224;
    for (unsigned i = 0; i < 5; i++) {
      TRGState s = output.subset(o + i * 4, 4);
      if ((i % 4) == 0)
        cout << "        ";
      cout << setw(2) << i << ": " << s << "   ";
      if ((i % 4) == 3)
        cout << endl;
    }
    cout  << endl;

    cout << "==================    End of Merger unpackerInner    ========================= " << endl;


  }



  void
  TCMerger:: unpackerOuter(const TRGState& input,
                           const TRGState& output)
  {

    TRGState inputInside = input.subset(0, 256);
    TRGState inputOutside = input.subset(256, 256);

    unsigned o = 0;

    cout << "======================= Merger unpackerOuter================================== " << endl;
    cout << "input bit information: " << endl;
    for (unsigned bi = 0; bi < input.size(); bi++) {
      if (input[bi]) cout << "* " ;
      else cout << ". ";
      if ((bi % 16) == 15) cout << endl;
      if (bi == 255) cout << "--------------------------------" << endl;
    }

    cout << "Input bit size = " << input.size() << " and Output bit size =" << output.size() << endl << endl;

    cout << "Hit map: " << endl;
    cout << "inputOutside : wire hit pattern" << endl;
    cout << "        ";
    for (unsigned i = 0; i < 48; i++) {
      const unsigned j = 47 - i;
      if (i && ((i % 8) == 0))
        cout << "_";
      if (inputOutside[j])
        cout << "*";
      else
        cout << ".";
    }
    cout << endl;
    cout << "inputInside : wire hit pattern" << endl;
    cout << "        ";
    for (unsigned i = 0; i < 48; i++) {
      const unsigned j = 48 - i - 1;
      if (i && ((i % 8) == 0))
        cout << "_";
      if (inputInside[j])
        cout << "*";
      else
        cout << ".";
    }
    cout << endl;
    cout << "-------------------------------------------" << endl;
    cout << "Output : wire hit pattern" << endl;
    cout << "        ";
    for (unsigned i = 0; i < 80; i++) {
      const unsigned j = 79 - i;
      /*
      if (i && ((i % 8) == 0))
        cout << "_";
      if (output[j])
        cout << "* ";
      else
        cout << ". ";
      if ( (i % 32) == 31 ) cout << endl << "        ";
      */
      if ((i % 16) == 0) {
        cout << endl << "        ";
        if (int(i / 16) % 2) {
        } else cout << " ";
      }
      if (output[j])
        cout << "O ";
      else
        cout << ". ";
      //    if ( (i % 32) == 31 ) cout << endl << "        ";
    }
    cout << endl;
    cout << " -=+=-=+=-=-=+=-=+=-=-=+=-=+=-=-=+=-=+=-=-=+=-=+=-" << endl << endl;

    cout << "Priority cell timing:" << endl;
    cout << "inputOutside : priority cell timing" << endl;
    o = 48;
    for (unsigned i = 0; i < 16; i++) {
      TRGState s = inputOutside.subset(o + i * 5, 5);
      if ((i % 4) == 0)
        cout << "        ";
      cout << setw(2) << i << ": " << s << "  ";
      if ((i % 4) == 3)
        cout << endl;
    }
    cout << "inputInside : priority cell timing" << endl;
    for (unsigned i = 0; i < 16; i++) {
      TRGState s = inputInside.subset(o + i * 5, 5);
      if ((i % 4) == 0)
        cout << "        ";
      cout << setw(2) << i << ": " << s << "  ";
      if ((i % 4) == 3)
        cout << endl;
    }
    cout << "ouput : priority cell timing" << endl;
    o = 80;
    for (unsigned i = 0; i < 16; i++) {
      TRGState s = output.subset(o + i * 4, 4);
      if ((i % 4) == 0)
        cout << "        ";
      cout << setw(2) << i << ": " << s << "   ";
      if ((i % 4) == 3)
        cout << endl;
    }
    cout << "output : priority cell location flag" << endl;
    o = 208;
    for (unsigned i = 0; i < 16; i++) {
      if ((i % 4) == 0)
        cout << "        ";
      if (output[o + i])
        cout << setw(2) << i << ": (1)Left    ";
      else
        cout << setw(2) << i << ": (0)Right   ";
      if ((i % 4) == 3)
        cout << endl;
    }
    cout << " -=+=-=+=-=-=+=-=+=-=-=+=-=+=-=-=+=-=+=-=-=+=-=+=-=" << endl << endl;

    cout << "Fastest timing of each TrackSegment: " << endl;
    cout << "inputOutside : fastest timing" << endl;
    o = 128;
    for (unsigned i = 0; i < 16; i++) {
      TRGState s = inputOutside.subset(o + i * 5, 5);
      if ((i % 4) == 0)
        cout << "        ";
      cout << setw(2) << i << ": " << s << "  ";
      if ((i % 4) == 3)
        cout << endl;
    }
    cout << "inputInside : fastest timing" << endl;
    for (unsigned i = 0; i < 16; i++) {
      TRGState s = inputInside.subset(o + i * 5, 5);
      if ((i % 4) == 0)
        cout << "        ";
      cout << setw(2) << i << ": " << s << "  ";
      if ((i % 4) == 3)
        cout << endl;
    }
    cout << "output : fastest timing" << endl;
    o = 144;
    for (unsigned i = 0; i < 16; i++) {
      TRGState s = output.subset(o + i * 4, 4);
      if ((i % 4) == 0)
        cout << "        ";
      cout << setw(2) << i << ": " << s << "   ";
      if ((i % 4) == 3)
        cout << endl;
    }
    cout << " -=+=-=+=-=-=+=-=+=-=-=+=-=+=-=-=+=-=+=-=-=+=-=+=-=" << endl << endl;

    cout << "Edge time information: " << endl;
    cout << "inputOutside : timing of missing wires" << endl;
    o = 208;
    for (unsigned i = 0; i < 2; i++) {
      TRGState s = inputOutside.subset(o + i * 5, 5);
      if ((i % 4) == 0)
        cout << "        ";
      cout << setw(2) << i << ": " << s << "  ";
      if ((i % 4) == 3)
        cout << endl;
    } cout << endl;
    cout << "inputInside : timing of missing wires" << endl;
    for (unsigned i = 0; i < 2; i++) {
      TRGState s = inputInside.subset(o + i * 5, 5);
      if ((i % 4) == 0)
        cout << "        ";
      cout << setw(2) << i << ": " << s << "  ";
      if ((i % 4) == 3)
        cout << endl;
    }  cout << endl;
    cout << "output : timing of missing wires" << endl;
    o = 224;
    for (unsigned i = 0; i < 3; i++) {
      TRGState s = output.subset(o + i * 4, 4);
      if ((i % 4) == 0)
        cout << "        ";
      cout << setw(2) << i << ": " << s << "   ";
      if ((i % 4) == 3)
        cout << endl;
    }
    cout  << endl;

    cout << "=================     End of Merger unpackerOuter    ========================= " << endl;


  }

  void //Dump all the details of _mosb into a .log file, do it in the end of simulate()
  TRGCDCMerger::dump_log(void) const
  {
    if (type() == innerType) {
      dump_log_inner();
    } else {
      dump_log_outer();
    }
  }

  void
  TRGCDCMerger::dump_log_inner(void) const
  {

    const TRGClock& dClock = TRGCDC::getTRGCDC()->dataClock();
    ofstream output((name() + ".log").c_str());

    output << "Inner Superlayer Merger output dump" << endl << endl;

    const std::vector<int> changetime = _mosb->stateChanges();
    std::vector<vector<int>> boolvector(changetime.size());
    for (unsigned ch_t = 0; ch_t < changetime.size(); ch_t++)       {

      for (unsigned b = 0; b < (* _mosb)[0]->size(); b++) {
        boolvector[ch_t].push_back(((* _mosb)[0]->state(changetime[ch_t])[b])  ? 1 : 0);
      }

      output << "# of clk: " << changetime[ch_t] << " (" << dClock.absoluteTime(changetime[ch_t]) << " ns), signal vector: " << endl;

      output << "Hitmap: " << endl << " ";
      for (int b = 0; b < 80; b++) {
        output << boolvector[ch_t][79 - b] << " ";
        if (b == 15) output << endl;
        else if (b == 31) output << endl << " ";
        else if (b == 47) output << endl ;
        else if (b == 63) output << endl << " ";
      }
      output << endl;

      for (int b = 0; b < 16; b++) {
        output << "PT#" << b << ": " << boolvector[ch_t][ 80 + 4 * b + 3 ] << boolvector[ch_t][ 80 + 4 * b + 2 ] << boolvector[ch_t][ 80 + 4
               * b + 1 ]
               << boolvector[ch_t][ 80 + 4 * b ] << endl;
      }
      output << endl;

      for (int b = 0; b < 16; b++) {
        output << "FT#" << b << ": " << boolvector[ch_t][ 144 + 4 * b + 3 ] << boolvector[ch_t][ 144 + 4 * b + 2 ] << boolvector[ch_t][ 144
               + 4 * b + 1 ]
               << boolvector[ch_t][ 144 + 4 * b ] << endl;
      }

      output << "Secondary: " << endl;
      for (int b = 0; b < 16; b++) {
        output << boolvector[ch_t][ 223 - b ] ;
      }

      output << endl;
      output << "ET#0(31): " << endl << boolvector[ch_t][227] << boolvector[ch_t][226] << boolvector[ch_t][225] << boolvector[ch_t][224]
             << endl;
      output << "ET#1(64(out32)): " << endl << boolvector[ch_t][231] << boolvector[ch_t][230] << boolvector[ch_t][229] <<
             boolvector[ch_t][228]
             << endl;
      output << "ET#2(32, 48, 64, 65(outside 0, 16, 32, 33)): " << endl << boolvector[ch_t][235] << boolvector[ch_t][234] <<
             boolvector[ch_t][233]
             << boolvector[ch_t][232] << endl;
      output << "ET#3(31, 47, 62, 63, 78, 79(inside 31, outside 15, 30, 31, 46, 47)): " << endl << boolvector[ch_t][239] <<
             boolvector[ch_t][238]
             << boolvector[ch_t][237] << boolvector[ch_t][236] << endl;
      output << "ET#4(63, 79(outside 31, 47)): " << endl << boolvector[ch_t][243] << boolvector[ch_t][242] << boolvector[ch_t][241]
             << boolvector[ch_t][240] << endl;



      output << endl;
    }

    output.close();

  }

  void
  TRGCDCMerger::dump_log_outer(void) const
  {

    const TRGClock& dClock = TRGCDC::getTRGCDC()->dataClock();
    ofstream output((name() + ".log").c_str());

    output << "Outer Superlayer Merger output dump" << endl << endl;

    const std::vector<int> changetime = _mosb->stateChanges();
    std::vector<vector<int>> boolvector(changetime.size());
    for (unsigned ch_t = 0; ch_t < changetime.size(); ch_t++)       {

      for (unsigned b = 0; b < (* _mosb)[0]->size(); b++) {
        boolvector[ch_t].push_back(((* _mosb)[0]->state(changetime[ch_t])[b])  ? 1 : 0);
      }

      output << "# of clk: " << changetime[ch_t] << " (" << dClock.absoluteTime(changetime[ch_t]) << " ns), signal vector: " << endl;

      output << "Hitmap: " << endl << " ";
      for (int b = 0; b < 80; b++) {
        output << boolvector[ch_t][79 - b] << " ";
        if (b == 15) output << endl;
        else if (b == 31) output << endl << " ";
        else if (b == 47) output << endl ;
        else if (b == 63) output << endl << " ";
      }
      output << endl;

      for (int b = 0; b < 16; b++) {
        output << "PT#" << b << ": " << boolvector[ch_t][ 80 + 4 * b + 3 ] << boolvector[ch_t][ 80 + 4 * b + 2 ] << boolvector[ch_t][ 80 + 4
               * b + 1 ]
               << boolvector[ch_t][ 80 + 4 * b ] << endl;
      }
      output << endl;

      for (int b = 0; b < 16; b++) {
        output << "FT#" << b << ": " << boolvector[ch_t][ 144 + 4 * b + 3 ] << boolvector[ch_t][ 144 + 4 * b + 2 ] << boolvector[ch_t][ 144
               + 4 * b + 1 ]
               << boolvector[ch_t][ 144 + 4 * b ] << endl;
      }

      output << "Secondary: ";
      for (int b = 0; b < 16; b++) {
        output << boolvector[ch_t][ 223 - b ] ;
      }

      output << endl;
      output << "ET#0(63(outside 15)): " << endl << boolvector[ch_t][227] << boolvector[ch_t][226] << boolvector[ch_t][225] <<
             boolvector[ch_t][224]
             << endl;
      output << "ET#1(0,64(inside 0, out 16)): " << endl << boolvector[ch_t][231] << boolvector[ch_t][230] << boolvector[ch_t][229]
             << boolvector[ch_t][228] << endl;
      output << "ET#2(15,31,63,79(inside 15, 31, outside 15, 31)): " << endl << boolvector[ch_t][235] << boolvector[ch_t][234]
             << boolvector[ch_t][233] << boolvector[ch_t][232] << endl;

      output << endl;
    }


    output.close();

  }


} // namespace Belle2
