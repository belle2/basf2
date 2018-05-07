/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
