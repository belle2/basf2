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
/// Helper utils for path arithmetics needed in the pEventProcessor
  class PathUtils {
  public:
    /**
       * Split the given part into the input, main and output path (in this order) by looking onto the
       * parallel certificate of the modules.
       */
    static std::tuple<PathPtr, PathPtr, PathPtr> splitPath(const PathPtr& path);

    /** Adds internal zmq modules to the paths. */
    static ModulePtrList preparePaths(PathPtr& inputPath, PathPtr& mainPath, PathPtr& outputPath);

    /** Find the histogram manager in the paths and return it. */
    static ModulePtr getHistogramManager(PathPtr& inputPath, PathPtr& mainPath, PathPtr& outputPath);

    /** Return only modules which have the TerminateGlobally Module flag set. */
    static ModulePtrList getTerminateGloballyModules(const ModulePtrList& modules);

    /** Prepend given 'prependModules' to 'modules', if they're not already present. */
    static void prependModulesIfNotPresent(ModulePtrList* modules, const ModulePtrList& prependModules);

    /// Prepend given modules to the path
    static void appendModule(PathPtr& path, const ModulePtr& module);

    /// Append given modules to the path
    static void prependModule(PathPtr& path, const ModulePtr& module);
  };
} // namespace Belle2