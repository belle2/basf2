/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
    static void preparePaths(PathPtr& inputPath, PathPtr& mainPath, PathPtr& outputPath,
                             const std::string& socketAddress);

    /** Find the histogram manager in the paths and return it. */
    static ModulePtr getHistogramManager(PathPtr& inputPath, PathPtr& mainPath, PathPtr& outputPath);

    /** Return only modules which have the given Module flag set. */
    static ModulePtrList getModulesWithFlag(const ModulePtrList& modules, Module::EModulePropFlags flag);

    /** Return only modules which do not have the given Module flag set. */
    static ModulePtrList getModulesWithoutFlag(const ModulePtrList& modules, Module::EModulePropFlags flag);

    /** Prepend given 'prependModules' to 'modules', if they're not already present. */
    static void prependModulesIfNotPresent(ModulePtrList* modules, const ModulePtrList& prependModules);

    /// Prepend given modules to the path
    static void appendModule(PathPtr& path, ModulePtr module);

    /// Append given modules to the path
    static void prependModule(PathPtr& path, ModulePtr module);
  };
}