#ifndef DATAFLOWVISUALIZATION_H
#define DATAFLOWVISUALIZATION_H

#include <framework/datastore/DataStore.h>
#include <framework/core/Module.h>

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
  private:
    /** Stores information on inputs/outputs of each module, as obtained by require()/createEntry(); */
    const std::map<std::string, DataStore::ModuleInfo>& m_moduleInfo;
    /** List of modules, in order of initialisation. */
    const ModulePtrList& m_modules;
  };
}
#endif
