/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/dbobjects/TOPCalChannelMask.h>

#include <framework/logging/Logger.h>

using namespace std;

namespace Belle2 {

  void TOPCalChannelMask::setStatus(int moduleID, unsigned channel, EStatus status)
  {
    int module = moduleID - 1;
    if (!check(module, channel)) {
      B2WARNING("Channel status not set");
      return;
    }
    m_status[module][channel] = status;
  }

  void TOPCalChannelMask::setActive(int moduleID, unsigned channel)
  {
    int module = moduleID - 1;
    if (!check(module, channel)) {
      B2WARNING("Channel status 'active' not set");
      return;
    }
    m_status[module][channel] = c_Active;
  }

  void TOPCalChannelMask::setDead(int moduleID, unsigned channel)
  {
    int module = moduleID - 1;
    if (!check(module, channel)) {
      B2WARNING("Channel status 'dead' not set");
      return;
    }
    m_status[module][channel] = c_Dead;
  }

  void TOPCalChannelMask::setNoisy(int moduleID, unsigned channel)
  {
    int module = moduleID - 1;
    if (!check(module, channel)) {
      B2WARNING("Channel status 'noisy' not set");
      return;
    }
    m_status[module][channel] = c_Noisy;
  }

  TOPCalChannelMask::EStatus TOPCalChannelMask::getStatus(int moduleID,
                                                          unsigned channel) const
  {
    int module = moduleID - 1;
    if (!check(module, channel)) {
      B2WARNING("Returning dead channel value");
      return c_Dead;
    }
    return m_status[module][channel];
  }

  bool TOPCalChannelMask::isActive(int moduleID, unsigned channel) const
  {
    int module = moduleID - 1;
    if (!check(module, channel)) {
      B2WARNING("Returning false");
      return false;
    }
    return (m_status[module][channel] == c_Active);
  }

  bool TOPCalChannelMask::check(const int module, const unsigned channel) const
  {
    if (module >= c_numModules) {
      B2ERROR("Invalid module number (" << ClassName() << ")");
      return false;
    }
    if (module < 0) {
      B2ERROR("Invalid module number (" << ClassName() << ")");
      return false;
    }
    if (channel >= c_numChannels) {
      B2ERROR("Invalid channel number (" << ClassName() << ")");
      return false;
    }
    return true;
  }

  int TOPCalChannelMask::getNumOf(EStatus check) const
  {
    int n = 0;
    for (const auto& statuses : m_status) {
      for (const auto status : statuses) {
        if (status == check) n++;
      }
    }
    return n;
  }

  int TOPCalChannelMask::getNumOf(EStatus check, int moduleID) const
  {
    unsigned module = moduleID - 1;
    if (module < c_numModules) {
      int n = 0;
      const auto& statuses = m_status[module];
      for (const auto status : statuses) {
        if (status == check) n++;
      }
      return n;
    }
    B2ERROR("Invalid module number (" << ClassName() << ")");
    return 0;
  }

} // end Belle2 namespace
