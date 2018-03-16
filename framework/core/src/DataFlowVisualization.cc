/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

//first to avoid _XOPEN_SOURCE warnings
#include <framework/core/Module.h>
#include <framework/core/Path.h>
#include <framework/core/ModuleManager.h>

#include <framework/core/DataFlowVisualization.h>
#include <framework/datastore/DataStore.h>

#include <fstream>

using namespace Belle2;


namespace {
  const std::string unknownfillcolor = "gray82";
}

DataFlowVisualization::DataFlowVisualization(const DependencyMap* dependencyMap):
  m_map(dependencyMap)
{
  m_fillcolor[DependencyMap::c_Input] = "cornflowerblue";
  m_fillcolor[DependencyMap::c_OptionalInput] = "lightblue";
  m_fillcolor[DependencyMap::c_Output] = "orange";

  m_arrowcolor[DependencyMap::c_Input] = "cornflowerblue";
  m_arrowcolor[DependencyMap::c_OptionalInput] = "lightblue";
  m_arrowcolor[DependencyMap::c_Output] = "firebrick";
}

void DataFlowVisualization::visualizePath(const std::string& filename, const Path& path)
{
  std::ofstream file(filename.c_str());
  file << "digraph allModules {\n";
  file << "  rankdir=LR;\n"; //left -> right
  file << "  compound=true;\n"; //allow edges to subgraphs

  //for steering file data flow graph, we may get multiple definitions of each node
  //graphviz merges these into the last one, so we'll go through module list in reverse (all boxes should be coloured as outputs)
  const bool steeringFileFlow = true;
  for (ModulePtr mod : path.buildModulePathList())
    generateModulePlot(file, *mod, steeringFileFlow);

  plotPath(file, path);

  //add nodes
  for (const std::string& name : m_allOutputs) {
    file << "  \"" << name << "\" [shape=box,style=filled,fillcolor=" << m_fillcolor[DependencyMap::c_Output] << "];\n";
  }
  for (const std::string& name : m_allInputs) {
    if (m_allOutputs.count(name) == 0)
      file << "  \"" << name << "\" [shape=box,style=filled,fillcolor=" << m_fillcolor[DependencyMap::c_Input] << "];\n";
  }
  for (const std::string& name : m_unknownArrays) {
    if (m_allOutputs.count(name) == 0 && m_allInputs.count(name) == 0)
      file << "  \"" << name << "\" [shape=box,style=filled,fillcolor=" << unknownfillcolor << "];\n";
  }

  file << "}\n\n";

  B2INFO("Data flow diagram created. You can use 'dot dataflow.dot -Tps -o dataflow.ps' to create a PostScript file from it.");
}

void DataFlowVisualization::plotPath(std::ofstream& file, const Path& path, const std::string& pathName)
{
  //graph name must begin with cluster for fancy graphics!
  const std::string graphname = pathName.empty() ? "clusterMain" : ("cluster" + pathName);
  file << "  subgraph \"" << graphname << "\" {\n";
  if (pathName.empty()) {
    file << "    rank=min;\n";
  } else {
    file << "    rank=same;\n";
  }
  file << "    style=solid;\n";
  file << "    color=grey;\n";
  file << "    \"" << graphname  << "_inv\" [shape=point,style=invis];\n";
  std::string lastModule("");
  //connect modules in right order...
  for (ModulePtr mod : path.getModules()) {
    const std::string& module = DependencyMap::getModuleID(*mod);
    file << "    \"" << module << "\";\n";
    if (!lastModule.empty()) {
      file << "    \"" << lastModule << "\" -> \"" << module << "\" [color=black];\n";
    }
    if (mod->hasCondition()) {
      for (const auto& condition : mod->getAllConditions()) {
        const std::string& conditionName = condition.getString();
        plotPath(file, *condition.getPath(), conditionName);
        file << "    \"" << module << "\" -> \"cluster" << conditionName << "_inv\" " <<
             "[color=grey,lhead=\"cluster" << conditionName << "\",label=\"" << conditionName << "\",fontcolor=grey];\n";
      }
    }

    lastModule = module;
  }
  file << "  }\n";
}

void DataFlowVisualization::generateModulePlot(std::ofstream& file, const Module& mod, bool steeringFileFlow)
{
  const std::string& name = DependencyMap::getModuleID(mod);
  const std::string& label = mod.getName();
  if (!steeringFileFlow)
    file << "digraph \"" << name << "\" {\n";
  file << " " << name << " [label=\"" << label << "\"];\n";

  const auto foundInfoIter = m_map->getModuleInfoMap().find(name);
  if (foundInfoIter != m_map->getModuleInfoMap().end()) {
    const DependencyMap::ModuleInfo& moduleInfo = foundInfoIter->second;
    for (int i = 0; i < DependencyMap::c_NEntryTypes; i++) {
      const std::set<std::string>& entries = moduleInfo.entries[i];
      const std::set<std::string>& relations = moduleInfo.relations[i];
      const std::string fillcolor = m_fillcolor[i];
      const std::string arrowcolor = m_arrowcolor[i];

      for (std::string dsentry : entries) {
        if (!steeringFileFlow)
          file << "  \"" << dsentry << "\" [shape=box,style=filled,fillcolor=" << fillcolor << "];\n";
        if (i == DependencyMap::c_Output) {
          m_allOutputs.insert(dsentry);
          file << "  \"" << name << "\" -> \"" << dsentry << "\" [color=" << arrowcolor << "];\n";
        } else {
          m_allInputs.insert(dsentry);
          file << "  \"" << dsentry << "\" -> \"" << name  << "\" [color=" << arrowcolor << "];\n";
        }
      }

      for (const std::string& relname : relations) {
        size_t pos = relname.rfind("To");
        if (pos == std::string::npos or pos != relname.find("To")) {
          B2WARNING("generateModulePlot(): couldn't split relation name!");
          //Searching for parts in input/output lists might be helpful...
          continue;
        }

        const std::string from = relname.substr(0, pos);
        const std::string to = relname.substr(pos + 2);

        //any connected arrays that are neither input nor output?
        if (checkArrayUnknown(from, moduleInfo)) {
          if (!steeringFileFlow)
            file << "  \"" << from << "\" [shape=box,style=filled,fillcolor=" << unknownfillcolor << "];\n";
        }
        if (checkArrayUnknown(to, moduleInfo)) {
          if (!steeringFileFlow)
            file << "  \"" << to << "\" [shape=box,style=filled,fillcolor=" << unknownfillcolor << "];\n";
        }

        file << "  \"" << from << "\" -> \"" << to << "\" [color=" << arrowcolor << ",style=dashed];\n";
      }
    }
  }
  if (!steeringFileFlow)
    file << "}\n\n";
}


void DataFlowVisualization::executeModuleAndCreateIOPlot(const std::string& module)
{
  // construct given module and gearbox
  std::shared_ptr<Module> modulePtr = ModuleManager::Instance().registerModule(module);
  std::shared_ptr<Module> gearboxPtr = ModuleManager::Instance().registerModule("Gearbox");

  // call initialize() method
  //may throw some ERRORs, but that's OK.
  // TODO:(ignore missing inputs)
  gearboxPtr->initialize();
  DataStore::Instance().getDependencyMap().setModule(*modulePtr);
  modulePtr->initialize();

  // create plot
  const std::string filename = module + ".dot";
  DataFlowVisualization v(&DataStore::Instance().getDependencyMap());
  std::ofstream file(filename.c_str());
  v.generateModulePlot(file, *modulePtr, false);

  //clean up to avoid problems with ~TROOT
  modulePtr->terminate();
  gearboxPtr->terminate();
}

bool DataFlowVisualization::checkArrayUnknown(const std::string& name, const DependencyMap::ModuleInfo& info)
{
  for (int i = 0; i < DependencyMap::c_NEntryTypes; i++) {
    if (info.entries[i].count(name) != 0)
      return false; //found
  }

  //not found
  m_unknownArrays.insert(name);
  return true;
}
