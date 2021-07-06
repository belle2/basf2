/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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

