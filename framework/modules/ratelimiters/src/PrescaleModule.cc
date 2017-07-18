/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: David Dossett                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/modules/ratelimiters/PrescaleModule.h>
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

  addParam("prescale", m_prescale, "Fraction of events that will return True from this module. Range(0.0 -> 1.0)", float(1.0));
}

void PrescaleModule::event()
{
  setReturnValue(getPrescaleChoice());
}

/// I'm a little worried about floating point precision when comparing to 0.0 and 1.0 as special values.
/// But since they are exact floats and a user will usually set them (or leave them as default) as exactly
/// equal to 0.0 or 1.0 rather than calculating them in almost every case, I think we can assume that the equalities hold.
bool PrescaleModule::getPrescaleChoice()
{
  if (m_prescale == 1.) {
    return true;
  } else if (m_prescale == 0.) {
    return false;
  } else {
    const double randomNumber = gRandom->Uniform();
    return randomNumber < m_prescale;
  }
}
