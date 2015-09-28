/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Stefan Wunsch                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/pxdClusterRescue/PXDClusterRescueNNAnalysisModule.h>
#include <framework/datastore/StoreArray.h>
#include <pxd/dataobjects/PXDCluster.h>

using namespace Belle2;

REG_MODULE(PXDClusterRescueNNAnalysis)

PXDClusterRescueNNAnalysisModule::PXDClusterRescueNNAnalysisModule() : Module()
{
  // set description
  setDescription("Analyse classification qualitiy of PXDClusterRescueNN module.");

  // add module parameters
  addParam("filenameLog", m_filenameLog, "Filename of output log file.", std::string("PXDClusterRescueNNAnalysis.log"));
  addParam("dataType", m_dataType, "Data type of input data ('signal' or 'background').", std::string("signal/background"));
  addParam("maxPt", m_maxPt, "Maximum pt of particles.", float(0.065));
  addParam("minRelationWeight", m_minRelationWeight, "Minimum relation weight of MCParticle to PXDCluster.", float(0.3));
  addParam("particleInclude", m_particleInclude, "List of included particles by PDG number.", std::vector<int>({211, -211}));
  addParam("namePXDClusterAll", m_namePXDClusterAll, "Name of input PXD clusters in DataStore with all PXDClusters.",
           std::string(""));
  addParam("namePXDClusterFiltered", m_namePXDClusterFiltered,
           "Name of input PXD clusters in DataStore which are filtered with PXDClusterRescueNN module.", std::string("PXDClusterFiltered"));
}

PXDClusterRescueNNAnalysisModule::~PXDClusterRescueNNAnalysisModule()
{
}

void PXDClusterRescueNNAnalysisModule::initialize()
{
  // set PXDCluster data on required
  StoreArray<PXDCluster>::required();

  // set counter to zero
  m_countSignal = 0;
  m_countSignalClass = 0;
  m_countSignalValid = 0;
  m_countBackground = 0;
  m_countBackgroundClass = 0;
  m_countBackgroundValid = 0;

  // open log file
  m_fileLog.open(m_filenameLog);

  // write log file: header, timestamp, input parameter, number of training variables
  m_fileLog << "// Log file for PXDClusterRescueNNAnalysisModule module" << std::endl;

  time_t time_now = time(0);
  tm* gmt_time = gmtime(&time_now);
  char* time_string = asctime(gmt_time);
  m_fileLog << "Timestamp (GMT): " << time_string;

  m_fileLog << "Input data type: " << m_dataType << std::endl;
  m_fileLog << "Filename log: " << m_filenameLog << std::endl;
  m_fileLog << "Max pt: " << m_maxPt << std::endl;
  m_fileLog << "Included PDG numbers: ";
  for (unsigned int k = 0; k < m_particleInclude.size(); k++) {
    m_fileLog << m_particleInclude[k];
    if (k == m_particleInclude.size() - 1) m_fileLog << std::endl;
    else m_fileLog << " ";
  }
}

void PXDClusterRescueNNAnalysisModule::beginRun()
{
}

void PXDClusterRescueNNAnalysisModule::event()
{
  // load PXDClusters from DataStore
  StoreArray<PXDCluster> pxdClusters(m_namePXDClusterAll.c_str());
  int pxdClustersEntries = pxdClusters.getEntries();

  // go through clusters
  bool isSignalNetwork;
  bool isSignalParticle;

  for (int k = 0; k < pxdClustersEntries; k++) {
    // get classification by particle (exactly same result as PXDClusterGetNNData module for same input)
    if (m_dataType == "background") {
      isSignalParticle = !(m_PXDClusterGetNNData.PXDClusterIsType(*(pxdClusters[k]), m_maxPt, m_minRelationWeight, m_particleInclude,
                                                                  m_dataType));
    } else {
      isSignalParticle = m_PXDClusterGetNNData.PXDClusterIsType(*(pxdClusters[k]), m_maxPt, m_minRelationWeight, m_particleInclude,
                                                                m_dataType);
    }

    // get classification by neural network
    // if a relation from original PXDCluster set to filtered PXDCluster subset is given, network classified cluster as signal
    RelationVector<PXDCluster> relatedClusters = pxdClusters[k]->getRelationsTo<PXDCluster>(m_namePXDClusterFiltered.c_str());
    if (relatedClusters.size() != 0) isSignalNetwork = 1;
    else isSignalNetwork = 0;

    // increment counter
    if ((isSignalNetwork) && (isSignalParticle)) { m_countSignal++; m_countSignalClass++; m_countSignalValid++; }
    else if ((!isSignalNetwork) && (!isSignalParticle)) { m_countBackground++; m_countBackgroundClass++; m_countBackgroundValid++; }
    else if ((!isSignalNetwork) && (isSignalParticle)) { m_countSignal++; m_countBackgroundClass++; }
    else if ((isSignalNetwork) && (!isSignalParticle)) { m_countBackground++; m_countSignalClass++; }

    m_countClusters++;
  }
}

void PXDClusterRescueNNAnalysisModule::endRun()
{
}

void PXDClusterRescueNNAnalysisModule::terminate()
{
  // write log file with classification counters
  m_fileLog << std::endl;
  m_fileLog << "Count signals: " << m_countSignal << std::endl;
  m_fileLog << "Count classified signals: " << m_countSignalClass << std::endl;
  m_fileLog << "Count valid classified signals: " << m_countSignalValid << std::endl;
  m_fileLog << "Percentage classified signals of all signals: " << 100.0 * float(m_countSignalClass) / m_countSignal << std::endl;
  m_fileLog << "Percentage valid classified signals of classified signals: " << 100.0 * float(m_countSignalValid) / m_countSignalClass
            << std::endl;
  m_fileLog << "Percentage valid classified signals of all signals: " << 100.0 * float(m_countSignalValid) / m_countSignal <<
            std::endl;
  m_fileLog << std::endl;
  m_fileLog << "Count background: " << m_countBackground << std::endl;
  m_fileLog << "Count classified background: " << m_countBackgroundClass << std::endl;
  m_fileLog << "Count valid classified background: " << m_countBackgroundValid << std::endl;
  m_fileLog << "Percentage classified background of all background: " << 100.0 * float(m_countBackgroundClass) / m_countBackground <<
            std::endl;
  m_fileLog << "Percentage valid classified background of classified background: " << 100.0 * float(
              m_countBackgroundValid) / m_countBackgroundClass << std::endl;
  m_fileLog << "Percentage valid classified background of all background: " << 100.0 * float(m_countBackgroundValid) /
            m_countBackground << std::endl;
  m_fileLog << std::endl;
  m_fileLog << "Count all clusters: " << m_countClusters << std::endl;
  m_fileLog << "Percentage signal: " << 100.0 * float(m_countSignal) / m_countClusters << std::endl;
  m_fileLog << "Percentage background: " << 100.0 * float(m_countBackground) / m_countClusters << std::endl;

  // close log file
  m_fileLog.close();
}
