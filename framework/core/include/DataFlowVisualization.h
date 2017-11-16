/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/datastore/DependencyMap.h>

#include <iosfwd>

namespace Belle2 {
  class Module;
  class Path;

  /** class to visualize data flow between modules. */
  class DataFlowVisualization {
  public:
    /** Constructor. */
    explicit DataFlowVisualization(const DependencyMap* dependencyMap);

    /** Create graphs with datastore inputs/outputs of each module in path
     *
     * @param filename         file saved to (in DOT format).
     * @param path             Path to visualize.
     * */
    void visualizePath(const std::string& filename, const Path& path);

    /** Create independent I/O graph for a single module (without requiring a steering file).
     *
     * Output will be saved to ModuleName.dot.
     */
    static void executeModuleAndCreateIOPlot(const std::string& module);
  private:
    /** Create I/O graph for a single module (written to file). */
    void generateModulePlot(std::ofstream& file, const Module& mod, bool steeringFileFlow = false);

    /** Create a subgraph for the given Path (including conditional paths). */
    static void plotPath(std::ofstream& file, const Path& path, const std::string& pathName = "");

    /** If the given array name isn't found in any of info's fields, it is added to m_unknownArrays (and true returned).*/
    bool checkArrayUnknown(const std::string& name, const DependencyMap::ModuleInfo& info);

    /** Stores information on inputs/outputs of each module, as obtained by require()/createEntry(); */
    const DependencyMap* m_map;

    std::set<std::string> m_allInputs; /**< set of all inputs (including optionals), for steering file visualisation. */
    std::set<std::string> m_allOutputs; /**< set of all outputs, for steering file visualisation. */
    std::set<std::string> m_unknownArrays; /**< set of array only being used in relations, for steering file visualisation. */

    std::string m_fillcolor[DependencyMap::c_NEntryTypes]; /**< fill colors. */
    std::string m_arrowcolor[DependencyMap::c_NEntryTypes]; /**< arrow colors. */
  };
}
