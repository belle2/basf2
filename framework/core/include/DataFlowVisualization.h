#ifndef DATAFLOWVISUALIZATION_H
#define DATAFLOWVISUALIZATION_H

#include <framework/datastore/DataStore.h>
#include <framework/core/Module.h>

namespace Belle2 {
  /** class to visualize data flow between modules. */
  class DataFlowVisualization {
  public:
    DataFlowVisualization(const std::map<std::string, DataStore::ModuleInfo>& moduleInfo, const ModulePtrList& modules);

    /** Create graphs with datastore inputs/outputs of each module
     *
     * @param filename         file saved to (in DOT format).
     * @param steeringFileFlow create single graph showing data flow in current steering file.
     * */
    void generateModulePlots(const std::string& filename, bool steeringFileFlow = false);
  private:
    const std::map<std::string, DataStore::ModuleInfo>& m_moduleInfo;
    const ModulePtrList& m_modules;
  };
}
#endif
