/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/particledb/EvtGenParticlePDG.h>
#include <sstream>
#include <iomanip>

namespace Belle2 {
  std::string EvtGenParticlePDG::__repr__() const
  {
    std::stringstream buf;
    buf << "<Particle " << std::quoted(GetName());
    buf << " mass=" << Mass() << " width=" << Width();
    buf << std::setprecision(2) << " charge=" << (Charge() / 3.);
    buf << " pdg=" << PdgCode() << ">";
    return buf.str();
  }
}
