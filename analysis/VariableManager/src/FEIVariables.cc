#include <analysis/VariableManager/FEIVariables.h>

namespace Belle2 {
  namespace Variable {

    double mostcommonBTagIndex(const Particle* part)
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
      double tag_index = -1;
      double tag_N = 0;
      for (const auto& [key, value] : tag_candidates) {
        if (tag_candidates[key] > tag_N) {
          tag_index = key;
          tag_N = tag_candidates[key];
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
          std::vector<double> B_PDG = {511, 521};
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
    std::vector<double> truthFSPTag(double BTag_index)
    {
      StoreArray<MCParticle> MC_Particle_list;
      std::vector<double> fsp_truth_index;
      for (const MCParticle iMCParticle : MC_Particle_list) {
        if ((BTag_index == finddescendant(&iMCParticle)) && (iMCParticle.hasStatus(MCParticle::c_StableInGenerator) == true)
            && (iMCParticle.hasStatus(MCParticle::c_IsISRPhoton) == false) && iMCParticle.hasStatus(MCParticle::c_IsFSRPhoton) == false
            && (iMCParticle.hasStatus(MCParticle::c_IsPHOTOSPhoton) == false)) {
          fsp_truth_index.push_back(iMCParticle.getArrayIndex());
        }
      }
      return fsp_truth_index;
    }
    Manager::FunctionPtr percentageMissingParticlesBTag(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[0]);
        auto func = [var](const Particle * part)-> double{
          double index = var->function(part);
          if (index == -1)
          {
            return -1;
          } else{
            std::vector<double> fsp_FEI_index;
            const std::vector<const Particle*>& fsp_tag = part->getFinalStateDaughters();
            for (const Particle* fsp : fsp_tag)
            {
              const MCParticle* mc_fsp = fsp->getMCParticle();
              if (mc_fsp) {
                double tag_index = finddescendant(mc_fsp);
                if (tag_index == index) {
                  fsp_FEI_index.push_back(mc_fsp->getArrayIndex());
                }
              }
            }
            std::vector<double> diff;
            std::vector<double> fsp_truth_index = truthFSPTag(index);
            std::sort(fsp_truth_index.begin(), fsp_truth_index.end());
            std::sort(fsp_FEI_index.begin(), fsp_FEI_index.end());
            std::set_difference(fsp_truth_index.begin(), fsp_truth_index.end(), fsp_FEI_index.begin(), fsp_FEI_index.end(), std::inserter(diff, diff.begin()));
            return double(diff.size()) / double(fsp_truth_index.size());
          }
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for function percentageMissingParticlesBTag");
      }
    }
    Manager::FunctionPtr percentageWrongParticlesBTag(const std::vector<std::string>&  arguments)
    {
      if (arguments.size() == 1) {
        const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[0]);
        auto func = [var](const Particle * part)-> double{
          double index = var->function(part);
          if (index == -1)
          {
            return -1;
          } else{
            const std::vector<const Particle*>& fsp_tag = part->getFinalStateDaughters();
            double wrong_FEI = 0.0;
            for (const Particle* fsp : fsp_tag)
            {
              const MCParticle* mc_fsp = fsp->getMCParticle();
              if (mc_fsp) {
                double tag_index = finddescendant(mc_fsp);
                if (tag_index != index) {
                  wrong_FEI += 1.0;
                }
              }
            }
            return wrong_FEI / truthFSPTag(index).size();
          }
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for function percentageWrongParticlesBTag");
      }

    }
    VARIABLE_GROUP("FEIVariables");
    REGISTER_VARIABLE("mostcommonBTagIndex", mostcommonBTagIndex,
                      "By giving e.g. a FEI B meson candidate the B meson index on generator level is determined, where most reconstructed particles can be assigned to. If no B meson found on generator level -1 is returned.");
    REGISTER_VARIABLE("percentageMissingParticlesBTag(BMesonIndex)", percentageMissingParticlesBTag,
                      "Get the percentage of missing particles by providing an index of a B meson. So the number of particles not reconstructed by e.g. the FEI are determined and divided by the number of generated particles using the given index of the B meson. If no B meson found on generator level -1 is returned.");
    REGISTER_VARIABLE("percentageWrongParticlesBTag(BMesonIndex)", percentageWrongParticlesBTag,
                      "Get the percentage of wrong particles by providing an index of a B meson. In this context wrong means that the reconstructed particles originated from the other B meson. The absolute number is divided by the total number of generated FSP from the given B meson index. If no B meson found on generator level -1 is returned.");
  }
}