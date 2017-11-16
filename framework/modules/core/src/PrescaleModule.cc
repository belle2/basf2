/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: David Dossett                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
