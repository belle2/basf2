#pragma once
/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


// include VariableManager
#include <analysis/VariableManager/Manager.h>

// include the Belle II Particle class
#include <analysis/dataobjects/Particle.h>

namespace Belle2 {
  namespace Variable {
    /*
    * Given a reconstructed B meson candidate e.g. by the FEI the FSP are determined and a relation between reconstructed and generated FSP is drawn.
    * Afterwards for each FSP the index of the B meson is determined on generator level. The index of the B meson with the most occurrences is returned.
    * If no B meson found, returns -1.
    */
    int mostcommonBTagIndex(const Particle* part);
    /*
    Given a MCParticle this function returns the index of the B meson. If now B meson found, returns -1.
    */
    int finddescendant(const MCParticle* mcpart);
    /*
    Given an index of a B meson candidate, it determines the number of FSP on generator level. These FSP must be stable in the generator and cannot be a radiative particle.
    */
    std::vector<int> truthFSPTag(int BTag_index);
    /*
    By giving this function an index of a B meson candidate, it will return the percentage of missing particles from this given index.
    If continuum event this will return -1.
    */
    Manager::FunctionPtr percentageMissingParticlesBTag(const std::vector<std::string>&  arguments);
    /*
    By giving this function an index of a B meson candidate, it will return the percentage of wrong particles not originating from the given index.
    If continuum event this will return -1.
    */
    Manager::FunctionPtr percentageWrongParticlesBTag(const std::vector<std::string>&  arguments);
  } // Variable namespace
} // Belle2 namespace
