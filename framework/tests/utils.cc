/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <framework/utilities/Utils.h>

#include <TSystem.h>

#include <gtest/gtest.h>

using namespace std;
using namespace Belle2;

namespace {
  TEST(Utilities, MemSize)
  {
    //compare memory measurements done by root and ourselves
    //(should differ by less than 1MB even though function calls are a bit different)
    ProcInfo_t meminfo;
    gSystem->GetProcInfo(&meminfo);
    long v = Utils::getVirtualMemoryKB();
    ASSERT_TRUE(abs(v - meminfo.fMemVirtual) < 1000);

    gSystem->GetProcInfo(&meminfo);
    long r = Utils::getRssMemoryKB();
    ASSERT_TRUE(abs(r - meminfo.fMemResident) < 1000);
  }
}
