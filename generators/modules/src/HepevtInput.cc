/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <generators/modules/HepevtInput.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreDefs.h>
#include <framework/datastore/StoreArray.h>

#include <boost/format.hpp>
#include <boost/foreach.hpp>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(HepevtInput, "HepevtInput")

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

HepevtInput::HepevtInput() : Module()
{
  //Set module properties
  setDescription("Hepevt file input");
  setPropertyFlags(c_TriggersEndOfData | c_ReadsDataSingleProcess | c_RequiresSingleProcess);

  //Parameter definition
  addParam("inputFileName", m_inputFileName, string(""), "Hepevt filename", true);
  addParam("skipEvents", m_skipEventNumber, 0, "Skip this number of events before starting.");
}


void HepevtInput::initialize()
{
  try {
    m_hepevt.open(m_inputFileName);
    m_hepevt.skipEvents(m_skipEventNumber);
  } catch (runtime_error &e) {
    FATAL_S(e.what());
  }
}


void HepevtInput::event()
{
  try {
    MCParticleGraph mpg;
    if (!m_hepevt.getEvent(mpg)) {
      setProcessRecordType(prt_EndOfData);
      return;
    }
    mpg.generate(DEFAULT_MCPARTICLES, MCParticleGraph::set_decay_info | MCParticleGraph::check_cyclic);
  } catch (runtime_error &e) {
    ERROR_S(e.what());
    setProcessRecordType(prt_EndOfData);
  }
}
