/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2018 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Sam Cunliffe                                             *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/modules/ExtraInfoPrinter/ExtraInfoPrinterModule.h>

using namespace Belle2;

REG_MODULE(ExtraInfoPrinter)

ExtraInfoPrinterModule::ExtraInfoPrinterModule() : Module()
{
  setDescription("Prints the names of all ExtraInfo set for each particle in"
                 " the event. Useful for debugging and development.");

  addParam("particleList", m_listName, "Name of the particle list (an empty "
           "string prints the EventExtraInfo)");
  addParam("printOnce", m_printOnce, "Print for the first event, or for all "
           "events (default true)", true);
}

void ExtraInfoPrinterModule::initialize()
{
  if (m_listName == "") {
    m_eee.isRequired();
  } else {
    m_list.isRequired(m_listName);
    m_peem.isRequired();
  }
}

void ExtraInfoPrinterModule::event()
{
  if (m_printOnce && m_hasPrinted)
    return;

  // print the EventExtraInfo names
  if (m_listName == "") {
    if (m_eee.isValid()) {
      std::ostringstream stream;
      stream << "EventExtraInfo for this event: ";
      for (auto const& name : m_eee->getNames()) stream << name << " ";
      B2INFO(stream.str());
      m_hasPrinted = true;
    }
    return;
  }

  // print the ParticleExtraInfo names for the input ParticleList
  unsigned int n = m_list->getListSize();
  if (n < 1)
    B2WARNING("The list: " << m_listName << " doesn't contain any particles");

  for (unsigned i = 0; i < n; i++) {
    const Particle* p = m_list->getParticle(i);
    std::ostringstream stream;
    stream << "ExtraInfo for this particle: ";
    for (auto const& name : p->getExtraInfoNames()) stream << name << " ";
    B2INFO(stream.str());
    m_hasPrinted = true;
  }
  return;
}
