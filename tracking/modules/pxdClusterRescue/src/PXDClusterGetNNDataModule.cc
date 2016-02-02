/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Stefan Wunsch                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/pxdClusterRescue/PXDClusterGetNNDataModule.h>
#include <framework/datastore/StoreArray.h>
#include <pxd/dataobjects/PXDDigit.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <mdst/dataobjects/MCParticle.h>

#include <ctime>

using namespace Belle2;

REG_MODULE(PXDClusterGetNNData)

PXDClusterGetNNDataModule::PXDClusterGetNNDataModule() : Module()
{
  // set description
  setDescription("Get data from PXDClusters for neural network training.");

  // add module parameters
  addParam("filenameData", m_filenameData, "Filename of output training variables data file.",
           std::string("PXDClusterGetNNData_signal_background.data"));
  addParam("filenameLog", m_filenameLog, "Filename of output log file.", std::string("PXDClusterGetNNData_signal_background.log"));
  addParam("dataType", m_dataType, "Data type of input data ('signal' or 'background').", std::string("signal/background"));
  addParam("maxPt", m_maxPt, "Maximum pt of particles.", float(0.065));
  addParam("minRelationWeight", m_minRelationWeight, "Minimum relation weight of MCParticle to PXDCluster.", float(0.3));
  addParam("particleInclude", m_particleInclude, "List of included particles by PDG number.", std::vector<int>({211, -211}));
}

PXDClusterGetNNDataModule::~PXDClusterGetNNDataModule()
{
}

void PXDClusterGetNNDataModule::initialize()
{
  // set PXDCluster data on required
  StoreArray<PXDCluster>::required();

  // open data and log file
  m_fileData.open(m_filenameData);
  m_fileLog.open(m_filenameLog);

  // error handling: check for open files
  if (!m_fileData.is_open()) B2ERROR("Can not open output data file.");
  if (!m_fileLog.is_open()) B2ERROR("Can not open output log file.");

  // write log file: header, timestamp, input parameter, number of training variables
  m_fileLog << "// Log file for PXDClusterGetNNDataModule module" << std::endl;

  time_t time_now = time(0);
  tm* gmt_time = gmtime(&time_now);
  char* time_string = asctime(gmt_time);
  m_fileLog << "Timestamp (GMT): " << time_string;

  m_fileLog << "Input data type: " << m_dataType << std::endl;
  m_fileLog << "Filename data: " << m_filenameData << std::endl;
  m_fileLog << "Filename log: " << m_filenameLog << std::endl;
  m_fileLog << "Max pt: " << m_maxPt << std::endl;
  m_fileLog << "Included PDG numbers: ";
  for (unsigned int k = 0; k < m_particleInclude.size(); k++) {
    m_fileLog << m_particleInclude[k];
    if (k == m_particleInclude.size() - 1) m_fileLog << std::endl;
    else m_fileLog << " ";
  }

  m_fileLog << "Number of training variables: " << m_PXDClusterRescueNN.getNumTrainingVariables() << std::endl;
}

void PXDClusterGetNNDataModule::beginRun()
{
}

void PXDClusterGetNNDataModule::event()
{
  // load PXDClusters from DataStore
  StoreArray<PXDCluster> pxdClusters;

  // go through clusters
  int NumTrainingVariables = m_PXDClusterRescueNN.getNumTrainingVariables();
  std::vector<float> pxdClusterVariables(NumTrainingVariables);
  bool isValid;

  for (auto && pxdCluster : pxdClusters) {
    // check PXDCluster on being valid for used in neural network training
    isValid = PXDClusterIsType(pxdCluster, m_maxPt, m_minRelationWeight, m_particleInclude, m_dataType);

    // if PXCluster is valid save training variables to file
    if (isValid) {
      // load trainings variables from PXDCluster
      m_PXDClusterRescueNN.getPXDClusterTrainingVariables(&pxdCluster, pxdClusterVariables.data());

      // write training variables to file
      for (int p = 0; p < NumTrainingVariables; p++) {
        m_fileData << pxdClusterVariables[p];
        if (p == NumTrainingVariables - 1) m_fileData << std::endl;
        else m_fileData << " ";
      }
    }
  }
}

bool PXDClusterGetNNDataModule::PXDClusterIsType(PXDCluster const& pxdCluster, float const maxPt,
                                                 float const minRelationWeight,
                                                 std::vector<int> const& particleInclude,
                                                 std::string const& dataType)
{
  // if given data is background, do not make any selections and return PXDCluster type as background
  if (dataType == "background") return true;

  // if given data is signal data, check if at least one particle is on the pdg number include list

  // get related MC particle and go through particles
  RelationVector<MCParticle> particles = pxdCluster.getRelationsTo<MCParticle>();

  for (unsigned int k = 0; k < particles.size(); k++) {
    // check if particle pdg number is matching an included pdg number
    for (unsigned int p = 0; p < particleInclude.size(); p++) {
      if (particles[k]->getPDG() == particleInclude[p]) {
        // check particle relation weight to PXDCluster on greater than minRelationWeight
        if (particles.weight(k) > minRelationWeight) {
          // check for pt less than maxPt
          if ((particles[k]->getMomentum()).Pt() < maxPt) {
            // at least one particle with matching properties has been found
            return true;
          }
        }
      }
    }
  }

  // return PXDCluster not as signal if not a single valid particle is found
  return false;
}

void PXDClusterGetNNDataModule::endRun()
{
}

void PXDClusterGetNNDataModule::terminate()
{
}
