/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <vector>

namespace Belle2 {
  class MCParticle;
  class Particle;

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
    By giving this function a B meson candidate, it will return the percentage of missing particles from the mostcommonBTagIndex.
    If continuum event this will return -1.
    */
    double percentageMissingParticlesBTag(const Particle* part);
    /*
    By providing this function with a B meson candidate, it will return the percentage of wrong particles not originating from the mostcommonBTagIndex.
    If continuum event this will return -1.
    */
    double percentageWrongParticlesBTag(const Particle* part);
  } // Variable namespace
} // Belle2 namespace
