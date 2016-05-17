#include <framework/utilities/Utils.h>

#include <TSystem.h>

#include <gtest/gtest.h>

using namespace std;
using namespace Belle2;

namespace {
  TEST(Utilities, MemSize)
  {
    //compare memory measurements done by root and ourselves
    ProcInfo_t meminfo;
    gSystem->GetProcInfo(&meminfo);
    auto v = Utils::getVirtualMemoryKB();
    ASSERT_EQ(v, meminfo.fMemVirtual);

    gSystem->GetProcInfo(&meminfo);
    auto r = Utils::getRssMemoryKB();
    ASSERT_EQ(r, meminfo.fMemResident);
  }
}
