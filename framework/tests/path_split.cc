/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/pcore/PathUtils.h>

#include <framework/core/Module.h>
#include <framework/core/Path.h>

#include <gtest/gtest.h>

#include <string>
#include <vector>

using namespace Belle2;

namespace {

  /** A module that only carries a type name and property flags.
   *
   * splitPath() looks at nothing else, so the test needs no module registry and
   * no shared libraries beyond libframework.
   */
  class FlaggedModule : public Module {
  public:
    /** @param type  the type name, used only to make the expectations readable
     *  @param flags ORed Module::EModulePropFlags */
    FlaggedModule(const std::string& type, unsigned int flags)
    {
      setType(type);
      setPropertyFlags(flags);
    }
  };

  /** Convenience maker. */
  ModulePtr mod(const std::string& type, unsigned int flags = 0)
  {
    return ModulePtr(new FlaggedModule(type, flags));
  }

  const unsigned int c_par = Module::c_ParallelProcessingCertified;

  /** Parallel certified, but not worth forking for on its own. */
  const unsigned int c_setup = Module::c_ParallelProcessingCertified | Module::c_NoParallelWork;

  /** Type names of the modules on a path, or {"<null>"} if the path is null. */
  std::vector<std::string> typesOf(const PathPtr& path)
  {
    if (not path) return {"<null>"};
    std::vector<std::string> out;
    for (const ModulePtr& m : path->getModules()) out.push_back(m->getType());
    return out;
  }

  /** Split the path and check all three results at once. */
  void expectSplit(const PathPtr& path,
                   const std::vector<std::string>& expectedIn,
                   const std::vector<std::string>& expectedMain,
                   const std::vector<std::string>& expectedOut)
  {
    const auto [in, main, out] = PathUtils::splitPath(path);
    EXPECT_EQ(expectedIn, typesOf(in)) << "input path";
    EXPECT_EQ(expectedMain, typesOf(main)) << "main path";
    EXPECT_EQ(expectedOut, typesOf(out)) << "output path";
  }

  /** The regression test for the beam background overlay bug.
   *
   * add_simulation() puts a parallel certified StatisticsSummary ("Sum_PreSimulation")
   * in front of BGOverlayInput, which is deliberately not parallel certified so that
   * it runs before the fork. StatisticsSummary is therefore flagged c_NoParallelWork:
   * that lone statistics module must not be allowed to open a parallel section, or
   * BGOverlayInput closes it again immediately and the whole simulation and
   * reconstruction ends up in the output path, running single process.
   */
  TEST(PathSplit, StatisticsSummaryDoesNotOpenAParallelSection)
  {
    PathPtr path(new Path);
    path->addModule(mod("EventInfoSetter"));
    path->addModule(mod("StatisticsSummary", c_setup));
    path->addModule(mod("BGOverlayInput"));
    path->addModule(mod("Gearbox", c_setup));
    path->addModule(mod("Geometry", c_setup));
    path->addModule(mod("RealWork", c_par));
    path->addModule(mod("RootOutput"));

    expectSplit(path,
    {"EventInfoSetter", "StatisticsSummary", "BGOverlayInput"},
    {"Gearbox", "Geometry", "RealWork"},
    {"RootOutput"});
  }

  /** The pre-existing rescue, which the fix above reuses: a parallel section made
   *  only of c_NoParallelWork modules is merged back. */
  TEST(PathSplit, GearboxOnlySectionIsMergedBack)
  {
    PathPtr path(new Path);
    path->addModule(mod("EventInfoSetter"));
    path->addModule(mod("Gearbox", c_setup));
    path->addModule(mod("Geometry", c_setup));
    path->addModule(mod("SomeInputModule"));
    path->addModule(mod("RealWork", c_par));
    path->addModule(mod("RootOutput"));

    expectSplit(path,
    {"EventInfoSetter", "Gearbox", "Geometry", "SomeInputModule"},
    {"RealWork"},
    {"RootOutput"});
  }

  /** A section with real work in it must survive, even next to useless modules. */
  TEST(PathSplit, UsefulSectionIsKept)
  {
    PathPtr path(new Path);
    path->addModule(mod("RootInput"));
    path->addModule(mod("Gearbox", c_setup));
    path->addModule(mod("RealWork", c_par));
    path->addModule(mod("MoreWork", c_par));
    path->addModule(mod("RootOutput"));

    expectSplit(path, {"RootInput"}, {"Gearbox", "RealWork", "MoreWork"}, {"RootOutput"});
  }

  /** Nothing parallel certified at all: the whole path stays in the input path and
   *  the job falls back to single process.
   *
   * Since no module here is flagged c_TerminateInAllProcesses, splitPath() nulls the
   * two empty paths rather than returning them empty. Callers test "null or empty"
   * anyway (see ZMQEventProcessor.cc), but it is the null that they actually get, and
   * a change to empty would be a silent change of contract.
   */
  TEST(PathSplit, NothingParallelGivesNoMainPath)
  {
    PathPtr path(new Path);
    path->addModule(mod("RootInput"));
    path->addModule(mod("SerialWork"));
    path->addModule(mod("RootOutput"));

    const auto [in, main, out] = PathUtils::splitPath(path);

    EXPECT_EQ(std::vector<std::string>({"RootInput", "SerialWork", "RootOutput"}), typesOf(in));
    EXPECT_FALSE(main) << "an empty main path must come back as null, not as an empty path";
    EXPECT_FALSE(out) << "an empty output path must come back as null, not as an empty path";
  }

}  // namespace
