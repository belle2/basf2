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
// Filename : MergerViewer
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A program to view Merger COE data files
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#define TRG_SHORT_NAMES

#include <iostream>
#include <fstream>
#include <string>
#include <bitset>
#include "trg/trg/Utilities.h"

using namespace std;
using namespace Belle2;

#define DEBUG_LEVEL   0
#define NAME          "MergerViewer"
#define VERSION       "version 0.00"

int
main(int argc, char* argv[])
{

  cout << NAME << " ... " << VERSION << endl;
  const string tab = "    ";

  //...Check arguments...
  if (argc != 2) {
    cout << NAME << " !!! arguments not good" << endl;
    cout << tab << " 1 : Merger COE data file" << endl;
    return -1;
  }

  //...Open COE data file...
  string inname = argv[1];
  ifstream infile(inname.c_str(), ios::in);
  if (infile.fail()) {
    cout << NAME << " !!! can not open file" << endl
         << "    " << inname << endl;
    return -2;
  }

  char b[800];
  unsigned line = 0;
  while (! infile.eof()) {
    infile.getline(b, 800);
    string l(b);

    //...Emptty...
    if (l.size() == 0)
      continue;

    //...Comment line...
    if (l[0] == ';')
      continue;

    //...Memory init config line...
    if (l.find("memory") != string::npos)
      continue;

    //...Clock counter...
    bitset<5> cc(l.substr(0, 5));
    cout << "--- " << cc.to_ulong();

    //...Reserved...
    if (l.substr(5, 11) != "00000000000")
      cout << " something wrong with reserved bits ";

    //...Clock counter in real data...
    bitset<9> rcc(l.substr(11 + 5, 9));
    cout << " " << rcc.to_ulong();

    //...Hit map...
    bitset<80> hm(l.substr(5 + 16 * 11, 16 * 5));
    if (hm.none()) {
      cout << endl << "-" << endl;
    } else {
      cout << " : " << hm.count() << " hit(s)" << endl;
      for (unsigned j = 0; j < 5; j++) {
        unsigned left = 256 + 5 - (4 - j) * 16;
        if (j % 2) cout << " ";
        for (unsigned i = 0; i < 16; i++) {
          cout << l[left - (16 - i)] << " ";
        }
        cout << endl;
      }
    }

    ++line;
  }

  //...Termination...
  cout << NAME << " ... terminated" << endl;
}
