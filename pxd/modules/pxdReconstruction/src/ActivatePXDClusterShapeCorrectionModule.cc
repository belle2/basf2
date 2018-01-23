/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kodys                                              *
 *                                                                        *
 * Prepared for activation of cluster shape correction                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "pxd/modules/pxdReconstruction/ActivatePXDClusterShapeCorrectionModule.h"

using namespace Belle2;
using namespace Belle2::PXD;
using namespace std;
using boost::format;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ActivatePXDClusterShapeCorrection)


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ActivatePXDClusterShapeCorrectionModule::ActivatePXDClusterShapeCorrectionModule() : Module()
{
  //Set module properties
  setDescription("Activation of PXD Cluster Shape Correction");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("ClShCorrectionFileName", m_ClShCorrectionFileName, "Name of file with corrections for Cluster Shapes",
           std::string("pxd/data/PXD-ClasterShapeCorrections.root"));
  addParam("ClShCorrectionLogFileName", m_ClShCorrectionLogFileName, "Name of log file with corrections for Cluster Shapes",
           std::string(""));

}


ActivatePXDClusterShapeCorrectionModule::~ActivatePXDClusterShapeCorrectionModule()
{
}

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void ActivatePXDClusterShapeCorrectionModule::initialize()
{

  HitCorrector::getInstance().setLogFile(m_ClShCorrectionLogFileName);
  HitCorrector::getInstance().initialize();
  // HitCorrector::getInstance().initialize(m_ClShCorrectionFileName);

}

void ActivatePXDClusterShapeCorrectionModule::beginRun()
{
}


void ActivatePXDClusterShapeCorrectionModule::event()
{
}


void ActivatePXDClusterShapeCorrectionModule::endRun()
{
}


void ActivatePXDClusterShapeCorrectionModule::terminate()
{
}
