/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/modules/core/PrescaleModule.h>
#include <TRandom.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(Prescale)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PrescaleModule::PrescaleModule() : Module()
{
  //Set module properties
  setDescription("Returns True or False randomly for each event. "
                 "Fraction of the time that True is returned is set by the 'prescale' parameter. "
                 "You can use conditional basf2 paths to allow only the events that return True (or False) to continue.");

  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("prescale", m_prescale, "Fraction of events that will return True from this module. Range(0.0 -> 1.0)", double(1.0));
}

void PrescaleModule::event()
{
  setReturnValue(gRandom->Uniform() < m_prescale);
}
