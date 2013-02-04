#include <framework/core/DataFlowVisualization.h>

#include <framework/core/ModuleManager.h>

#include <fstream>


using namespace Belle2;

const std::string inputfillcolor = "cadetblue3";
const std::string inputarrowcolor = "cornflowerblue";
const std::string outputfillcolor = "orange";
const std::string outputarrowcolor = "firebrick";
const std::string unknownfillcolor = "gray82";

DataFlowVisualization::DataFlowVisualization(const std::map<std::string, DataStore::ModuleInfo>& moduleInfo, const ModulePtrList& modules):
  m_moduleInfo(moduleInfo),
  m_modules(modules)
{
}

void DataFlowVisualization::generateModulePlots(const std::string& filename, bool steeringFileFlow)
{
  std::ofstream file(filename.c_str());
  if (steeringFileFlow)
    file << "digraph allModules {\n";

  //for steering file data flow graph, we may get multiple definitions of each node
  //graphviz merges these into the last one, so we'll go through module list in reverse (all boxes should be coloured as outputs)
  for (ModulePtrList::const_reverse_iterator it = m_modules.rbegin(); it != m_modules.rend(); ++it) {
    const std::string& name = (*it)->getName();
    generateModulePlot(file, name, steeringFileFlow);
  }

  if (steeringFileFlow) {
    //connect modules in right order...
    std::string lastModule = "";
    for (ModulePtrList::const_iterator it = m_modules.begin(); it != m_modules.end(); ++it) {
      const std::string& module = (*it)->getName();
      if (!lastModule.empty()) {
        file << "  " << lastModule << " -> " << module << " [color=black];\n";
      }

      lastModule = module;
    }


    //add nodes
    for (std::set<std::string>::const_iterator it = m_allOutputs.begin(); it != m_allOutputs.end(); ++it) {
      file << "  " << *it << " [shape=box,style=filled,fillcolor=" << outputfillcolor << "];\n";
    }
    for (std::set<std::string>::const_iterator it = m_allInputs.begin(); it != m_allInputs.end(); ++it) {
      if (m_allOutputs.count(*it) == 0)
        file << "  " << *it << " [shape=box,style=filled,fillcolor=" << inputfillcolor << "];\n";
    }
    for (std::set<std::string>::const_iterator it = m_unknownArrays.begin(); it != m_unknownArrays.end(); ++it) {
      if (m_allOutputs.count(*it) == 0 && m_allInputs.count(*it) == 0)
        file << "  " << *it << " [shape=box,style=filled,fillcolor=" << unknownfillcolor << "];\n";
    }


    file << "}\n\n";
  }
}

void DataFlowVisualization::generateModulePlot(std::ofstream& file, const std::string& name, bool steeringFileFlow)
{
  if (!steeringFileFlow)
    file << "digraph " << name << " {\n";
  file << "  " << name << ";\n";

  std::map<std::string, DataStore::ModuleInfo>::const_iterator foundInfoIter = m_moduleInfo.find(name);
  if (foundInfoIter != m_moduleInfo.end()) {
    const DataStore::ModuleInfo& moduleInfo = foundInfoIter->second;
    //i=0: input, i=1: ouput
    for (int i = 0; i < 2; i++) {
      std::set<std::string> objects = moduleInfo.inputs;
      std::set<std::string> relations = moduleInfo.inputRelations;
      //colors for input
      std::string fillcolor = inputfillcolor;
      std::string arrowcolor = inputarrowcolor;
      if (i == 1) {
        objects = moduleInfo.outputs;
        relations = moduleInfo.outputRelations;

        //colors for output
        fillcolor = outputfillcolor;
        arrowcolor = outputarrowcolor;
      }

      for (std::set<std::string>::const_iterator setit = objects.begin(); setit != objects.end(); ++setit) {
        if (!steeringFileFlow)
          file << "  " << *setit << " [shape=box,style=filled,fillcolor=" << fillcolor << "];\n";
        if (i == 0) {
          m_allInputs.insert(*setit);
          file << "  " << *setit << " -> " << name  << " [color=" << arrowcolor << "];\n";
        } else {
          m_allOutputs.insert(*setit);
          file << "  " << name << " -> " << *setit << " [color=" << arrowcolor << "];\n";
        }
      }

      for (std::set<std::string>::const_iterator setit = relations.begin(); setit != relations.end(); ++setit) {
        const std::string& relname = *setit;
        size_t pos = relname.rfind("To");
        if (pos != relname.find("To")) {
          B2WARNING("generateModulePlot(): couldn't split relation name!");
          //Searching for parts in input/output lists might be helpful...
          continue;
        }

        const std::string from = relname.substr(0, pos);
        const std::string to = relname.substr(pos + 2);

        //any connected arrays that are neither input nor output?
        if (moduleInfo.outputs.count(from) == 0 && moduleInfo.inputs.count(from) == 0) {
          m_unknownArrays.insert(from);
          if (!steeringFileFlow)
            file << "  " << from << " [shape=box,style=filled,fillcolor=" << unknownfillcolor << "];\n";
        }
        if (moduleInfo.outputs.count(to) == 0 && moduleInfo.inputs.count(to) == 0) {
          m_unknownArrays.insert(to);
          if (!steeringFileFlow)
            file << "  " << to << " [shape=box,style=filled,fillcolor=" << unknownfillcolor << "];\n";
        }

        file << "  " << from << " -> " << to << " [color=" << arrowcolor << ",style=dashed];\n";
      }
    }
  }
  if (!steeringFileFlow)
    file << "}\n\n";
}


void DataFlowVisualization::executeModuleAndCreateIOPlot(const std::string& module)
{
  // construct given module and gearbox
  boost::shared_ptr<Module> modulePtr = ModuleManager::Instance().registerModule(module);
  boost::shared_ptr<Module> gearboxPtr = ModuleManager::Instance().registerModule("Gearbox");

  // call initialize() method
  //may throw some ERRORs, but that's OK.
  // TODO:(ignore missing inputs)
  DataStore::Instance().setInitializeActive(true);
  gearboxPtr->initialize();
  DataStore::Instance().setModule(modulePtr->getName());
  modulePtr->initialize();
  DataStore::Instance().setInitializeActive(false);

  // create plot
  const std::string filename = module + ".dot";
  ModulePtrList moduleList;
  moduleList.push_back(modulePtr);
  DataFlowVisualization v(DataStore::Instance().getModuleInfoMap(), moduleList);
  std::ofstream file(filename.c_str());
  v.generateModulePlot(file, module, false);

  //clean up to avoid problems with ~TROOT
  modulePtr->terminate();
  gearboxPtr->terminate();
}
