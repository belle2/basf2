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
// Filename : TRGCDCTRGPackerTB.cc
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A program to test TRGPacker on CDCFE
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#define TRG_SHORT_NAMES

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <string.h>
#include "trg/trg/Utilities.h"

using namespace std;
using namespace Belle2;

#define DEBUG_LEVEL   1
#define NAME          "TRGCDCTRGPackerTB"
#define VERSION       "version 0.00"
#define ENV_PATH      "BELLE2_LOCAL_DIR"
#define N_FRONT       500
#define NOT_CONNECTED 99999

int
main(int argc, char* argv[])
{

  cout << NAME << " ... " << VERSION << endl;
  const string tab = "    ";

  //...Check arguments...
  if (argc < 2) {
    cout << NAME << " !!! two arguments necessary" << endl
         << tab << " 1 : input data file" << endl;
    return -1;
  }

  //...Date...
  // string ts0 = TRGUtil::dateStringF();
  // string ts1 = TRGUtil::dateString();

  //...Get a path to data...
//  const string path = getenv(ENV_PATH);
//  const string inname = path + "/data/trg/" + argv[1];
//  const string outname = path + "/data/trg/" + argv[1] + ".out";
  const string inname = argv[1];
  const string outname = string(argv[1]) + ".out";
  cout << tab << "input data    : " << inname << endl;
  cout << tab << "output data   : " << outname << endl;

  //...Open configuration data...
  ifstream infile(inname.c_str(), ios::in);
  if (infile.fail()) {
    cout << NAME << " !!! can not open file" << endl
         << "    " << inname << endl;
    return -2;
  }

  //...Read data
  char b[800];
  unsigned lines = 0;
  multimap<unsigned, unsigned> signals;
  while (! infile.eof()) {
    infile.getline(b, 800);
    string l(b);

    if (l.size() == 0)
      continue;

    bool skip = false;
    unsigned id = 999;
    unsigned pos = 999;
    for (unsigned i = 0; i < 2; i++) {
      string car = TRGUtil::carstring(l);
      l = TRGUtil::cdrstring(l);

      if (car[0] == '#') {
        skip = true;
        break;
      }

      if (i == 0) {
        id = atoi(car.c_str());
      } else if (i == 1) {
        pos = atoi(car.c_str());
      }
    }

    if (skip)
      continue;

    //...Store signal...
    signals.insert(pair<unsigned, unsigned>(id, pos));

    if (DEBUG_LEVEL)
      cout << lines
           << " : " << id
           << " " << pos << endl;

    ++lines;
  }
  infile.close();

  //...Make signal bit map...
  unsigned bm[48];
  memset(bm, 0, sizeof(unsigned) * 48);
  for (map<unsigned, unsigned>::iterator i = signals.begin();
       i != signals.end();
       ++i) {

    bm[i->first] |= (1 << i->second);

    if (DEBUG_LEVEL)
      cout << i->first << " -> " << i->second << endl;
  }

  //...Open output data...
  ofstream outfile(outname.c_str(), ios::out);
  if (outfile.fail()) {
    cout << NAME << " !!! can not open file" << endl
         << "    " << outname << endl;
    return -2;
  }

  //...Make an output...
  outfile << "Input signal bit map" << endl;
  for (unsigned i = 0; i < 48; i++) {
    outfile.width(2);
    outfile << i << " : ";
    outfile.width(6);
    outfile << bm[i] << " : ";
    for (unsigned j = 0; j < 32; j++) {
      if (j && ((j % 4) == 0))
        outfile << " ";
      unsigned x = ((bm[i] >> (31 - j)) & 1);
      if (x)
        outfile << 1;
      else
        outfile << ".";
    }
    outfile << endl;
  }

  //...Simulate TRGPacker (hit pattern in 16ns x2)...
  outfile << "Hit pattern in 16ns" << endl;
  outfile << "                   "
          << "7654321_987654321_987654321_987654321_9876543210" << endl;
  unsigned long long hitptn[2];
  memset(hitptn, 0, sizeof(unsigned long long) * 2);
  for (unsigned i = 0; i < 2; i++) {
    for (unsigned j = 0; j < 48; j++) {
      bool hit = (bm[j] >> (i * 16)) & 0xffff;
      if (hit)
        hitptn[i] |= ((unsigned long long) 1 << j);
//      cout << "j,bm[j]=" << j << "," << bm[j] << "," << hit << ","
//           << hitptn[i] << endl;

    }
    outfile << i << " : ";
    outfile.width(12);
    outfile << hex << hitptn[i] << " : ";
    for (unsigned k = 0; k < 48; k++) {
      unsigned x = ((hitptn[i] >> (47 - k)) & 1);
      if (x)
        outfile << 1;
      else
        outfile << ".";
    }
    outfile << endl;
  }

  //...Simulate TRGPacker (fine timing in 16ns x2)...
  outfile << "Fine timing of central cells in 16ns" << endl;
  unsigned fineTiming[2][48];
  memset(fineTiming, 0, sizeof(unsigned) * 2 * 48);
  for (unsigned i = 0; i < 2; i++) {
    outfile << i << " : ";
    for (unsigned j = 0; j < 48; j++) {
      unsigned timing = (bm[j] >> (i * 16)) & 0xffff;
      fineTiming[i][j] = timing;
      outfile.width(4);
      outfile << hex << fineTiming[i][j];
      if ((j % 8) == 7)
        outfile << endl << "    ";
      else
        outfile << " ";

    }
    outfile << endl;
  }




  //...Termination...
  cout << NAME << " ... terminated" << endl;
}
