/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/core/Module.h>
#include <framework/core/Path.h>

namespace Belle2 {
/// Helper utils for path arithmetic needed in the pEventProcessor
  class PathUtils {
  public:
    /** Is a parallel section made up of these modules worth creating processes for?
     *
     * False if every module in it is flagged Module::c_NoParallelWork, i.e. the section would
     * fork three processes to run nothing but setup code. Such a section is merged back into
     * the input path and the search for a parallel section continues after it.
     *
     * That is what lets a module which is not parallel processing certified sit between the
     * setup modules and the real work without forcing the whole job into a single process:
     * otherwise the setup modules open a section that the uncertified module closes again on
     * the very next module, and everything after it ends up in the output path.
     */
    static bool isWorthParallelising(const Path& path);

    /**
       * Split the given part into the input, main and output path (in this order) by looking onto the
       * parallel certificate of the modules.
       */
    static std::tuple<PathPtr, PathPtr, PathPtr> splitPath(const PathPtr& path);

    /** Adds internal zmq modules to the paths. */
    static ModulePtrList preparePaths(PathPtr& inputPath, PathPtr& mainPath, PathPtr& outputPath);

    /** Find the histogram manager in the paths and return it. */
    static ModulePtr getHistogramManager(PathPtr& inputPath);

    /** Return only modules which have the TerminateGlobally Module flag set. */
    static ModulePtrList getTerminateGloballyModules(const ModulePtrList& modules);

    /** Prepend given 'prependModules' to 'modules', if they're not already present. */
    static void prependModulesIfNotPresent(ModulePtrList* modules, const ModulePtrList& prependModules);

    /// Append the given module to the path
    static void appendModule(PathPtr& path, const ModulePtr& module);

    /// Prepend the given modules to the path, keeping their order
    static void prependModules(PathPtr& path, const ModulePtrList& modules);
  };
} // namespace Belle2