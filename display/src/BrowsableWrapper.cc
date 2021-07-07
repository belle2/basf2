/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <display/BrowsableWrapper.h>

#include <framework/logging/Logger.h>

#include <TVirtualPad.h>

using namespace Belle2;

std::map<TVirtualPad*, std::string> BrowsableWrapper::s_pads;

void BrowsableWrapper::Browse(TBrowser* b)
{
  if (!m_wrapped) {
    B2ERROR("Trying to browse invalid object!");
    return;
  }
  std::string name(m_wrapped->GetName());
  s_pads[gPad] = name;
  gPad->Connect("Closed()", "Belle2::BrowsableWrapper", this, "padClosed()");

  m_wrapped->Browse(b);
}

void BrowsableWrapper::padClosed()
{
  TVirtualPad* pad = dynamic_cast<TVirtualPad*>(static_cast<TQObject*>(gTQSender));
  if (!pad) {
    B2ERROR("Sender is not a pad?");
    return;
  }

  B2WARNING("TODO remove - deleting pad");
  s_pads.erase(pad);
}
