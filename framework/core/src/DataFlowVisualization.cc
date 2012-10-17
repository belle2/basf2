#include <framework/core/DataFlowVisualization.h>

#include <fstream>
using namespace Belle2;

DataFlowVisualization::DataFlowVisualization(const std::map<std::string, DataStore::ModuleInfo>& moduleInfo, const ModulePtrList& modules):
  m_moduleInfo(moduleInfo),
  m_modules(modules)
{
}

void DataFlowVisualization::generateModulePlots(const std::string& filename, bool steeringFileFlow)
{
  const std::string inputfillcolor = "cadetblue3";
  const std::string inputarrowcolor = "cornflowerblue";
  const std::string outputfillcolor = "orange";
  const std::string outputarrowcolor = "firebrick";
  const std::string unknownfillcolor = "gray82";



  std::ofstream file(filename.c_str());
  if (steeringFileFlow)
    file << "digraph allModules {\n";

  std::set<std::string> allInputs;
  std::set<std::string> allOutputs;
  std::set<std::string> unknownArrays;

  //for steering file data flow graph, we may get multiple definitions of each node
  //graphviz merges these into the last one, so we'll go through module list in reverse (all boxes should be coloured as outputs)
  for (ModulePtrList::const_reverse_iterator it = m_modules.rbegin(); it != m_modules.rend(); ++it) {
    const std::string& name = (*it)->getName();
    std::map<std::string, DataStore::ModuleInfo>::const_iterator foundInfoIter = m_moduleInfo.find(name);
    if (foundInfoIter == m_moduleInfo.end())
      continue;
    const DataStore::ModuleInfo& moduleInfo = foundInfoIter->second;

    if (moduleInfo.inputs.empty() && moduleInfo.outputs.empty())
      return;

    if (!steeringFileFlow)
      file << "digraph " << name << " {\n";
    file << "  " << name << ";\n";

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
          allInputs.insert(*setit);
          file << "  " << *setit << " -> " << name  << " [color=" << arrowcolor << "];\n";
        } else {
          allOutputs.insert(*setit);
          file << "  " << name << " -> " << *setit << " [color=" << arrowcolor << "];\n";
        }
      }

      for (std::set<std::string>::const_iterator setit = relations.begin(); setit != relations.end(); ++setit) {
        const std::string& relname = *setit;
        size_t pos = relname.rfind("To");
        if (pos != relname.find("To")) {
          B2WARNING("generateDotFile(): couldn't split relation name!")
          //Searching for parts in input/output lists might be helpful...
          continue;
        }

        const std::string from = relname.substr(0, pos);
        const std::string to = relname.substr(pos + 2);

        //any connected arrays that are neither input nor output?
        if (moduleInfo.outputs.count(from) == 0 && moduleInfo.inputs.count(from) == 0) {
          unknownArrays.insert(from);
          if (!steeringFileFlow)
            file << "  " << from << " [shape=box,style=filled,fillcolor=" << unknownfillcolor << "];\n";
        }
        if (moduleInfo.outputs.count(to) == 0 && moduleInfo.inputs.count(to) == 0) {
          unknownArrays.insert(to);
          if (!steeringFileFlow)
            file << "  " << to << " [shape=box,style=filled,fillcolor=" << unknownfillcolor << "];\n";
        }

        file << "  " << from << " -> " << to << " [color=" << arrowcolor << ",style=dashed];\n";
      }
    }
    if (!steeringFileFlow)
      file << "}\n\n";
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
    for (std::set<std::string>::const_iterator it = allOutputs.begin(); it != allOutputs.end(); ++it) {
      file << "  " << *it << " [shape=box,style=filled,fillcolor=" << outputfillcolor << "];\n";
    }
    for (std::set<std::string>::const_iterator it = allInputs.begin(); it != allInputs.end(); ++it) {
      if (allOutputs.count(*it) == 0)
        file << "  " << *it << " [shape=box,style=filled,fillcolor=" << inputfillcolor << "];\n";
    }
    for (std::set<std::string>::const_iterator it = unknownArrays.begin(); it != unknownArrays.end(); ++it) {
      if (allOutputs.count(*it) == 0 && allInputs.count(*it) == 0)
        file << "  " << *it << " [shape=box,style=filled,fillcolor=" << unknownfillcolor << "];\n";
    }


    file << "}\n\n";
  }
}
