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

  /** Convenience maker.
   *
   * The flags are the ones the real modules of that name declare. No flags at all is
   * not a shortcut: EventInfoSetter and BGOverlayInput genuinely call no
   * setPropertyFlags(), which is what makes them close a parallel section.
   */
  ModulePtr mod(const std::string& type, unsigned int flags = 0)
  {
    return ModulePtr(new FlaggedModule(type, flags));
  }

  const unsigned int c_par = Module::c_ParallelProcessingCertified;

  /** RootInput's flags: an input module is never put in the parallel section. */
  const unsigned int c_input = Module::c_Input;

  /** RootOutput's flags. */
  const unsigned int c_output = Module::c_Output;

  /** Parallel certified, but not worth forking for on its own (Gearbox, Geometry,
   *  StatisticsSummary). */
  const unsigned int c_setup = Module::c_ParallelProcessingCertified | Module::c_NoParallelWork;

  /** HistoManager's flags: not parallel certified, and copied into all three paths. */
  const unsigned int c_histo = Module::c_HistogramManager | Module::c_NoParallelWork;

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
    path->addModule(mod("RootOutput", c_output));

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
    path->addModule(mod("SomeUncertifiedModule"));
    path->addModule(mod("RealWork", c_par));
    path->addModule(mod("RootOutput", c_output));

    expectSplit(path,
    {"EventInfoSetter", "Gearbox", "Geometry", "SomeUncertifiedModule"},
    {"RealWork"},
    {"RootOutput"});
  }

  /** A section with real work in it must survive, even next to useless modules. */
  TEST(PathSplit, UsefulSectionIsKept)
  {
    PathPtr path(new Path);
    path->addModule(mod("RootInput", c_input));
    path->addModule(mod("Gearbox", c_setup));
    path->addModule(mod("RealWork", c_par));
    path->addModule(mod("MoreWork", c_par));
    path->addModule(mod("RootOutput", c_output));

    expectSplit(path, {"RootInput"}, {"Gearbox", "RealWork", "MoreWork"}, {"RootOutput"});
  }

  /** An input or output module is kept out of the parallel section even when it is
   *  parallel processing certified.
   *
   * No module declares that combination today, so nothing else pins it: without the
   * exemption the input module would open the section itself and the input path would
   * come back null, i.e. the reading would happen once per worker.
   */
  TEST(PathSplit, CertifiedInputAndOutputModulesStayOutOfTheMainPath)
  {
    PathPtr path(new Path);
    path->addModule(mod("CertifiedInput", c_input | c_par));
    path->addModule(mod("Gearbox", c_setup));
    path->addModule(mod("RealWork", c_par));
    path->addModule(mod("CertifiedOutput", c_output | c_par));

    expectSplit(path, {"CertifiedInput"}, {"Gearbox", "RealWork"}, {"CertifiedOutput"});
  }

  /** The histogram manager runs in every process, so it is put on all three paths. */
  TEST(PathSplit, HistogramManagerGoesOnAllThreePaths)
  {
    PathPtr path(new Path);
    path->addModule(mod("HistoManager", c_histo));
    path->addModule(mod("RootInput", c_input));
    path->addModule(mod("RealWork", c_par));
    path->addModule(mod("RootOutput", c_output));

    expectSplit(path,
    {"HistoManager", "RootInput"},
    {"HistoManager", "RealWork"},
    {"HistoManager", "RootOutput"});
  }

  /** Merging a pointless parallel section back must not disturb the histogram manager.
   *
   * The previous test crossed with the BGOverlayInput one: a DQM job with a simulation
   * behind it. The section StatisticsSummary opens used to contain the histogram
   * manager, so merging it back duplicated the manager into the input path and dropped
   * it from the main path -- leaving the workers filling no histograms at all.
   */
  TEST(PathSplit, MergeKeepsHistogramManagerInMainPath)
  {
    PathPtr path(new Path);
    path->addModule(mod("HistoManager", c_histo));
    path->addModule(mod("EventInfoSetter"));
    path->addModule(mod("StatisticsSummary", c_setup));
    path->addModule(mod("BGOverlayInput"));
    path->addModule(mod("Gearbox", c_setup));
    path->addModule(mod("RealWork", c_par));
    path->addModule(mod("RootOutput", c_output));

    expectSplit(path,
    {"HistoManager", "EventInfoSetter", "StatisticsSummary", "BGOverlayInput"},
    {"HistoManager", "Gearbox", "RealWork"},
    {"HistoManager", "RootOutput"});
  }

  /** Whether a section is worth parallelising must not depend on the histogram manager.
   *
   * The manager is on the main path because it has to run in every process, not because
   * it is part of the section. Were it there during the splitting, [manager, Gearbox]
   * would look like useful work whenever the manager lacks c_NoParallelWork -- the same
   * collapse, from the other side. HistoManager does carry the flag today, so this pins
   * the splitter rather than that one module.
   */
  TEST(PathSplit, HistogramManagerDoesNotKeepAPointlessSectionAlive)
  {
    PathPtr path(new Path);
    path->addModule(mod("CustomHistogramManager", Module::c_HistogramManager));
    path->addModule(mod("EventInfoSetter"));
    path->addModule(mod("Gearbox", c_setup));
    path->addModule(mod("SomeUncertifiedModule"));
    path->addModule(mod("RealWork", c_par));
    path->addModule(mod("RootOutput", c_output));

    expectSplit(path,
    {"CustomHistogramManager", "EventInfoSetter", "Gearbox", "SomeUncertifiedModule"},
    {"CustomHistogramManager", "RealWork"},
    {"CustomHistogramManager", "RootOutput"});
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
    path->addModule(mod("RootInput", c_input));
    path->addModule(mod("SerialWork"));
    path->addModule(mod("RootOutput", c_output));

    const auto [in, main, out] = PathUtils::splitPath(path);

    EXPECT_EQ(std::vector<std::string>({"RootInput", "SerialWork", "RootOutput"}), typesOf(in));
    EXPECT_FALSE(main) << "an empty main path must come back as null, not as an empty path";
    EXPECT_FALSE(out) << "an empty output path must come back as null, not as an empty path";
  }

}  // namespace
