#ifndef DATAFLOWVISUALIZATION_H
#define DATAFLOWVISUALIZATION_H

#include <framework/core/Module.h>
#include <framework/datastore/DataStore.h>

#include <iosfwd>

namespace Belle2 {
  /** class to visualize data flow between modules. */
  class DataFlowVisualization {
  public:
    /** Constructor. */
    DataFlowVisualization(const std::map<std::string, DataStore::ModuleInfo>& moduleInfo, const ModulePtrList& modules);

    /** Create graphs with datastore inputs/outputs of each module
     *
     * @param filename         file saved to (in DOT format).
     * @param steeringFileFlow create single graph showing data flow in current steering file.
     * */
    void generateModulePlots(const std::string& filename, bool steeringFileFlow = false);

    /** Create I/O graph for a single module (written to file). */
    void generateModulePlot(std::ofstream& file, const std::string& module, bool steeringFileFlow = false);

    /** Create independent I/O graph for a single module (without requiring a steering file).
     *
     * Output will be saved to ModuleName.dot.
     */
    static void executeModuleAndCreateIOPlot(const std::string& module);
  private:
    /** Stores information on inputs/outputs of each module, as obtained by require()/createEntry(); */
    const std::map<std::string, DataStore::ModuleInfo>& m_moduleInfo;
    /** List of modules, in order of initialisation. */
    const ModulePtrList& m_modules;

    std::set<std::string> m_allInputs; /**< set of all inputs, for steering file visualisation. */
    std::set<std::string> m_allOutputs; /**< set of all outputs, for steering file visualisation. */
    std::set<std::string> m_unknownArrays; /**< set of array only being used in relations, for steering file visualisation. */
  };
}
#endif
