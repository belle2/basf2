/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <generators/modules/OverrideGenerationFlagsModule.h>
#include <framework/database/Database.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(OverrideGenerationFlags)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

OverrideGenerationFlagsModule::OverrideGenerationFlagsModule() : Module()
{
  setDescription("Ovverride generation flags for BeamParameters "
                 "from database.");
  addParam("smearEnergy", m_SmearEnergy,
           "Smear energy when generating initial events.", true);
  addParam("smearDirection", m_SmearDirection,
           "Smear beam direction when generating initial events.", true);
  addParam("smearVertex", m_SmearVertex,
           "Smear vertex position when generating initial events.", true);
  addParam("generateCMS", m_GenerateCMS,
           "Generate events in CMS, not lab system.", false);
}

OverrideGenerationFlagsModule::~OverrideGenerationFlagsModule()
{
}

void OverrideGenerationFlagsModule::initialize()
{
}

void OverrideGenerationFlagsModule::terminate()
{
}

void OverrideGenerationFlagsModule::beginRun()
{
  BeamParameters* beamParameters = new BeamParameters(*m_BeamParameters);
  int flags = 0;
  if (m_GenerateCMS)
    flags |= BeamParameters::c_generateCMS;
  if (m_SmearEnergy)
    flags |= BeamParameters::c_smearBeamEnergy;
  if (m_SmearDirection)
    flags |= BeamParameters::c_smearBeamDirection;
  if (m_SmearVertex)
    flags |= BeamParameters::c_smearVertex;
  beamParameters->setGenerationFlags(flags);
  /* Override for one run. */
  DBStore::Instance().addConstantOverride("BeamParameters", beamParameters,
                                          true);
}

void OverrideGenerationFlagsModule::endRun()
{
}

void OverrideGenerationFlagsModule::event()
{
}
