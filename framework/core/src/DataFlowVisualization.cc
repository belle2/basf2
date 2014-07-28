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


#include <fstream>


using namespace Belle2;
typedef DataStore::ModuleInfo MInfo;

const std::string unknownfillcolor = "gray82";

DataFlowVisualization::DataFlowVisualization(const std::map<std::string, DataStore::ModuleInfo>& moduleInfo):
  m_moduleInfo(moduleInfo)
{
  m_fillcolor[MInfo::c_Input] = "cornflowerblue";
  m_fillcolor[MInfo::c_OptionalInput] = "lightblue";
  m_fillcolor[MInfo::c_Output] = "orange";

  m_arrowcolor[MInfo::c_Input] = "cornflowerblue";
  m_arrowcolor[MInfo::c_OptionalInput] = "lightblue";
  m_arrowcolor[MInfo::c_Output] = "firebrick";
}

void DataFlowVisualization::generateModulePlots(const std::string& filename, const Path& path, bool steeringFileFlow)
{
  std::ofstream file(filename.c_str());
  if (steeringFileFlow) {
    file << "digraph allModules {\n";
    file << "  rankdir=LR;\n"; //left -> right
    file << "  compound=true;\n"; //allow edges to subgraphs
  }

  const ModulePtrList& moduleList = path.getModules();
  //for steering file data flow graph, we may get multiple definitions of each node
  //graphviz merges these into the last one, so we'll go through module list in reverse (all boxes should be coloured as outputs)
  for (ModulePtrList::const_reverse_iterator it = moduleList.rbegin(); it != moduleList.rend(); ++it) {
    const std::string& name = (*it)->getName();
    generateModulePlot(file, name, steeringFileFlow);
  }

  if (steeringFileFlow) {
    plotPath(file, path);

    //add nodes
    for (std::set<std::string>::const_iterator it = m_allOutputs.begin(); it != m_allOutputs.end(); ++it) {
      file << "  \"" << *it << "\" [shape=box,style=filled,fillcolor=" << m_fillcolor[MInfo::c_Output] << "];\n";
    }
    for (std::set<std::string>::const_iterator it = m_allInputs.begin(); it != m_allInputs.end(); ++it) {
      if (m_allOutputs.count(*it) == 0)
        file << "  \"" << *it << "\" [shape=box,style=filled,fillcolor=" << m_fillcolor[MInfo::c_Input] << "];\n";
    }
    for (std::set<std::string>::const_iterator it = m_unknownArrays.begin(); it != m_unknownArrays.end(); ++it) {
      if (m_allOutputs.count(*it) == 0 && m_allInputs.count(*it) == 0)
        file << "  \"" << *it << "\" [shape=box,style=filled,fillcolor=" << unknownfillcolor << "];\n";
    }


    file << "}\n\n";
  }
}

void DataFlowVisualization::plotPath(std::ofstream& file, const Path& path, const std::string& pathName)
{
  const ModulePtrList& moduleList = path.getModules();
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
  for (ModulePtrList::const_iterator it = moduleList.begin(); it != moduleList.end(); ++it) {
    const std::string& module = (*it)->getName();
    file << "    \"" << module << "\";\n";
    if (!lastModule.empty()) {
      file << "    \"" << lastModule << "\" -> \"" << module << "\" [color=black];\n";
    }
    if ((*it)->hasCondition()) {
      const Path* conditionPath = (*it)->getConditionPath().get();
      plotPath(file, *conditionPath, module);
      file << "    \"" << module << "\" -> \"cluster" << module << "_inv\" [color=grey,lhead=\"cluster" << module << "\"];\n";
    }

    lastModule = module;
  }
  file << "  }\n";
}

void DataFlowVisualization::generateModulePlot(std::ofstream& file, const std::string& name, bool steeringFileFlow)
{
  if (!steeringFileFlow)
    file << "digraph \"" << name << "\" {\n";
  file << "  \"" << name << "\";\n";

  std::map<std::string, DataStore::ModuleInfo>::const_iterator foundInfoIter = m_moduleInfo.find(name);
  if (foundInfoIter != m_moduleInfo.end()) {
    const MInfo& moduleInfo = foundInfoIter->second;
    for (int i = 0; i < MInfo::c_NEntryTypes; i++) {
      const std::set<std::string>& entries = moduleInfo.entries[i];
      const std::set<std::string>& relations = moduleInfo.relations[i];
      const std::string fillcolor = m_fillcolor[i];
      const std::string arrowcolor = m_arrowcolor[i];

      for (std::set<std::string>::const_iterator setit = entries.begin(); setit != entries.end(); ++setit) {
        if (!steeringFileFlow)
          file << "  \"" << *setit << "\" [shape=box,style=filled,fillcolor=" << fillcolor << "];\n";
        if (i == MInfo::c_Output) {
          m_allOutputs.insert(*setit);
          file << "  \"" << name << "\" -> \"" << *setit << "\" [color=" << arrowcolor << "];\n";
        } else {
          m_allInputs.insert(*setit);
          file << "  \"" << *setit << "\" -> \"" << name  << "\" [color=" << arrowcolor << "];\n";
        }
      }

      for (std::set<std::string>::const_iterator setit = relations.begin(); setit != relations.end(); ++setit) {
        const std::string& relname = *setit;
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
  boost::shared_ptr<Module> modulePtr = ModuleManager::Instance().registerModule(module);
  boost::shared_ptr<Module> gearboxPtr = ModuleManager::Instance().registerModule("Gearbox");

  // call initialize() method
  //may throw some ERRORs, but that's OK.
  // TODO:(ignore missing inputs)
  gearboxPtr->initialize();
  DataStore::Instance().setModule(modulePtr->getName());
  modulePtr->initialize();

  // create plot
  const std::string filename = module + ".dot";
  DataFlowVisualization v(DataStore::Instance().getModuleInfoMap());
  std::ofstream file(filename.c_str());
  v.generateModulePlot(file, module, false);

  //clean up to avoid problems with ~TROOT
  modulePtr->terminate();
  gearboxPtr->terminate();
}

bool DataFlowVisualization::checkArrayUnknown(const std::string& name, const DataStore::ModuleInfo& info)
{
  for (int i = 0; i < MInfo::c_NEntryTypes; i++) {
    if (info.entries[i].count(name) != 0)
      return false; //found
  }

  //not found
  m_unknownArrays.insert(name);
  return true;
}
