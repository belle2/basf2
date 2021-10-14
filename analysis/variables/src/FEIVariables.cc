/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/variables/FEIVariables.h>
#include <mdst/dataobjects/MCParticle.h>
#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>
#include <map>
#include <cmath>
#include <algorithm>
#include <iterator>
#include <limits>

namespace Belle2 {
  namespace Variable {

    int mostcommonBTagIndex(const Particle* part)
    {
      std::map <int, int>tag_candidates;
      const std::vector<const Particle*>& fsp_tag = part->getFinalStateDaughters();
      for (const Particle* fsp : fsp_tag) {
        const MCParticle* mc_fsp = fsp->getMCParticle();
        if (mc_fsp) {
          int tag_index = finddescendant(mc_fsp);
          if (tag_candidates.find(tag_index) == tag_candidates.end()) {
            tag_candidates[tag_index] = 1;
          } else {
            tag_candidates[tag_index]++;
          }
        }
      }
      int tag_index = -1;
      int tag_N = 0;
      for (const auto& [key, value] : tag_candidates) {
        if (value > tag_N) {
          tag_index = key;
          tag_N = value;
        }
      }
      return tag_index;
    }
    int finddescendant(const MCParticle* mcpart)
    {
      const MCParticle* i_mcpart = mcpart;
      while (i_mcpart) {
        auto* i_mcpart_mother = i_mcpart->getMother();
        if (i_mcpart_mother) {
          std::vector<int> B_PDG = {511, 521};
          auto result = std::find(std::begin(B_PDG), std::end(B_PDG), abs(i_mcpart_mother->getPDG()));
          if (result != std::end(B_PDG)) {
            return i_mcpart_mother->getArrayIndex();
          }
          i_mcpart = i_mcpart_mother;
        } else {
          return -1;
        }
      }
      return -2;
    }
    std::vector<int> truthFSPTag(int BTag_index)
    {
      StoreArray<MCParticle> MC_Particle_list;
      std::vector<int> fsp_truth_index;
      for (const MCParticle iMCParticle : MC_Particle_list) {
        if ((BTag_index == finddescendant(&iMCParticle)) && (iMCParticle.hasStatus(MCParticle::c_StableInGenerator) == true)
            && (iMCParticle.hasStatus(MCParticle::c_IsISRPhoton) == false) && iMCParticle.hasStatus(MCParticle::c_IsFSRPhoton) == false
            && (iMCParticle.hasStatus(MCParticle::c_IsPHOTOSPhoton) == false)) {
          fsp_truth_index.push_back(iMCParticle.getArrayIndex());
        }
      }
      return fsp_truth_index;
    }
    double percentageMissingParticlesBTag(const Particle* part)
    {
      int index = mostcommonBTagIndex(part);
      if (index == -1) {
        return -1;
      } else {
        std::vector<int> fsp_FEI_index;
        const std::vector<const Particle*>& fsp_tag = part->getFinalStateDaughters();
        for (const Particle* fsp : fsp_tag) {
          const MCParticle* mc_fsp = fsp->getMCParticle();
          if (mc_fsp) {
            int tag_index = finddescendant(mc_fsp);
            if (tag_index == index) {
              fsp_FEI_index.push_back(mc_fsp->getArrayIndex());
            }
          }
        }
        std::vector<int> diff;
        std::vector<int> fsp_truth_index = truthFSPTag(index);
        std::sort(fsp_truth_index.begin(), fsp_truth_index.end());
        std::sort(fsp_FEI_index.begin(), fsp_FEI_index.end());
        std::set_difference(fsp_truth_index.begin(), fsp_truth_index.end(), fsp_FEI_index.begin(), fsp_FEI_index.end(), std::inserter(diff,
                            diff.begin()));
        return double(diff.size()) / double(fsp_truth_index.size());
      }
    }
    double percentageWrongParticlesBTag(const Particle* part)
    {
      int index = mostcommonBTagIndex(part);
      if (index == -1) {
        return -1;
      } else {
        const std::vector<const Particle*>& fsp_tag = part->getFinalStateDaughters();
        int wrong_FEI = 0;
        for (const Particle* fsp : fsp_tag) {
          const MCParticle* mc_fsp = fsp->getMCParticle();
          if (mc_fsp) {
            int tag_index = finddescendant(mc_fsp);
            if (tag_index != index) {
              wrong_FEI ++;
            }
          }
        }
        return double(wrong_FEI) / double(truthFSPTag(index).size());
      }
    }
    VARIABLE_GROUP("FEIVariables");
    REGISTER_VARIABLE("mostcommonBTagIndex", mostcommonBTagIndex,
                      "By giving e.g. a FEI B meson candidate the B meson index on generator level is determined, where most reconstructed particles can be assigned to. If no B meson found on generator level -1 is returned.");
    REGISTER_VARIABLE("percentageMissingParticlesBTag", percentageMissingParticlesBTag,
                      "Get the percentage of missing particles by using the mostcommonBTagIndex. So the number of particles not reconstructed by e.g. the FEI are determined and divided by the number of generated particles using the given index of the B meson. If no B meson found on generator level -1 is returned.");
    REGISTER_VARIABLE("percentageWrongParticlesBTag", percentageWrongParticlesBTag,
                      "Get the percentage of wrong particles by using the mostcommonBTagIndex. In this context wrong means that the reconstructed particles originated from the other B meson. The absolute number is divided by the total number of generated FSP from the given B meson index. If no B meson found on generator level -1 is returned.");
  }
}