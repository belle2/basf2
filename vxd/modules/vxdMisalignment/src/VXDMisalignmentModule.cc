/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                                          *
 *                                                                        *
 **************************************************************************/

#include <vxd/modules/vxdMisalignment/VXDMisalignmentModule.h>
#include <vxd/geometry/MisalignmentCache.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::VXD;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(VXDMisalignment)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

VXDMisalignmentModule::VXDMisalignmentModule() :
  Module(), m_xmlFilename("")
{
  //Set module properties
  setDescription("Initialize VXD misalignment");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("misalignmentFilename", m_xmlFilename,
           "Name of an xml misalignment data file. If empty, no misalignment will take place.",
           string(""));
}

void VXDMisalignmentModule::beginRun()
{
  // Initialize the MisalignmentCache
  VXD::MisalignmentCache& cache = VXD::MisalignmentCache::getInstance();
  cache.clear();
  cache.readMisalignmentsFromXml(m_xmlFilename);
}

