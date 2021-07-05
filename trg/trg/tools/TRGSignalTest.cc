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
// Filename : TRGSignalTest.cc
// Section  : TRG
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A program to test TRGSignal etc.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#define TRG_SHORT_NAMES

#include <string>
#include <iostream>
#include "trg/trg/Signal.h"

using namespace std;
using namespace Belle2;

#define DEBUG_LEVEL   0
#define NAME          "TRGSignalTest"
#define VERSION       "version 0.00"

int
main()
{

  cout << NAME << " ... " << VERSION << endl;
  // const string tab = "    ";

  //...Clock...
  TRGClock c("127MHz", 0, 127);

  //...TRGSignal::active test...
  cout << "TRGSignal::active test ..." << endl;
  TRGSignal t0("t0", c);
  t0.set(0, 10);
  t0.dump();

  cout << "t0.active()=" << t0.active() << endl;
  cout << "t0.active(-10, -1)=" << t0.active(-10, -1) << endl;
  cout << "t0.active(-1, 1)=" << t0.active(-1, 1) << endl;
  cout << "t0.active(-10, 20)=" << t0.active(-10, 20) << endl;
  cout << "t0.active(3, 4)=" << t0.active(3, 4) << endl;
  cout << "t0.active(3, 12)=" << t0.active(3, 12) << endl;
  cout << "t0.active(12, 100)=" << t0.active(12, 100) << endl;

  //...TRGSignal::unset test...
  cout << "TRGSignal::unset test ..." << endl;
  TRGSignal tmp = t0;
  tmp.dump();
  cout << "t0.unset(-10, -1)" << endl;
  tmp.unset(-10, -1);
  tmp.dump(); tmp = t0;
  cout << "t0.unset(-1, 1)" << endl;
  tmp.unset(-1, 1);
  tmp.dump(); tmp = t0;
  cout << "t0.unset(-10, 20)" << endl;
  tmp.unset(-10, 20);
  tmp.dump(); tmp = t0;
  cout << "t0.unset(3, 4)" << endl;
  tmp.unset(3, 4);
  tmp.dump(); tmp = t0;
  cout << "t0.unset(3, 12)" << endl;
  tmp.unset(3, 12);
  tmp.dump(); tmp = t0;
  cout << "t0.unset(12, 100)" << endl;
  tmp.unset(12, 100);
  tmp.dump();
  // tmp = t0; // not used, so commented out

  //...Termination...
  cout << NAME << " ... terminated" << endl;
  return 0;
}
