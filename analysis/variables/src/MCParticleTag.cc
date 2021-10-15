/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/VariableManager/Manager.h>
#include <analysis/variables/MCParticleTag.h>

// analysis utilities
#include <analysis/utility/GenBplusTag.h>
#include <analysis/utility/GenB0Tag.h>
#include <analysis/utility/GenBsTag.h>
#include <analysis/utility/GenDTag.h>
#include <analysis/utility/GenTauTag.h>

// mdst dataobjects
#include <mdst/dataobjects/MCParticle.h>

// framework
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

#include <vector>
#include <iostream>
#include <cmath>


namespace Belle2 {
  namespace Variable {

    double BplusMode(const Particle*)
    {
      GenBplusTag gBtag;
      StoreArray<MCParticle> MC_Particle_list;
      if (!MC_Particle_list) {
        B2WARNING("GenMCTagTool::eval - missing MCParticles array");
        return std::numeric_limits<double>::quiet_NaN();
      }
      int modeB = -99;

      for (const auto& iMCParticle : MC_Particle_list) {
        if (iMCParticle.getPDG() != 521) continue;

        std::vector<int> DauPDG;
        for (auto const& mcDaughter : iMCParticle.getDaughters()) {
          DauPDG.push_back(mcDaughter->getPDG());
        }
        modeB  = gBtag.Mode_B_plus(DauPDG);
        break;
      }

      return modeB;
    }

    double BminusMode(const Particle*)
    {
      GenBplusTag gBtag;
      StoreArray<MCParticle> MC_Particle_list;
      if (!MC_Particle_list) {
        B2WARNING("GenMCTagTool::eval - missing MCParticles array");
        return std::numeric_limits<double>::quiet_NaN();
      }
      int modeB = -99;

      for (const auto& iMCParticle : MC_Particle_list) {
        if (iMCParticle.getPDG() != -521) continue;

        std::vector<int> DauPDG;
        for (auto const& mcDaughter : iMCParticle.getDaughters()) {
          DauPDG.push_back(mcDaughter->getPDG());
        }
        modeB  = gBtag.Mode_B_minus(DauPDG);
        break;
      }

      return modeB;
    }

    double B0Mode(const Particle*)
    {
      GenB0Tag gBtag;
      StoreArray<MCParticle> MC_Particle_list;
      if (!MC_Particle_list) {
        B2WARNING("GenMCTagTool::eval - missing MCParticles array");
        return std::numeric_limits<double>::quiet_NaN();
      }
      int modeB1 = -99, modeB2 = -99, modeB = -99, nb1 = 0, nb2 = 0;

      for (const auto& iMCParticle : MC_Particle_list) {
        if (iMCParticle.getPDG() == 511) {
          nb1++;
          if (nb1 == 1) {
            std::vector<int> DauPDG1;
            for (auto const& mcDaughter : iMCParticle.getDaughters()) {
              DauPDG1.push_back(mcDaughter->getPDG());
            }
            modeB1 = gBtag.Mode_B0(DauPDG1);
            break;
          }
        }
        if (iMCParticle.getPDG() == -511) {
          nb2++;
          if (nb2 == 2) {
            std::vector<int> DauPDG2;
            for (auto const& mcDaughter : iMCParticle.getDaughters()) {
              DauPDG2.push_back(mcDaughter->getPDG());
            }
            modeB2 = gBtag.Mode_anti_B0(DauPDG2);
            break;
          }
        }
      }

      if (modeB1 != -99) { modeB = modeB1;}
      else if (modeB2 != -99) { modeB = modeB2;}

      return modeB;
    }

    double Bbar0Mode(const Particle*)
    {
      GenB0Tag gBtag;
      StoreArray<MCParticle> MC_Particle_list;
      if (!MC_Particle_list) {
        B2WARNING("GenMCTagTool::eval - missing MCParticles array");
        return std::numeric_limits<double>::quiet_NaN();
      }
      int modeB1 = -99, modeB2 = -99, modeB = -99, nb1 = 0, nb2 = 0;

      for (const auto& iMCParticle : MC_Particle_list) {
        if (iMCParticle.getPDG() == -511) {
          nb1++;
          if (nb1 == 1) {
            std::vector<int> DauPDG1;
            for (auto const& mcDaughter : iMCParticle.getDaughters()) {
              DauPDG1.push_back(mcDaughter->getPDG());
            }
            modeB1 = gBtag.Mode_anti_B0(DauPDG1);
            break;
          }
        }
        if (iMCParticle.getPDG() == 511) {
          nb2++;
          if (nb2 == 2) {
            std::vector<int> DauPDG2;
            for (auto const& mcDaughter : iMCParticle.getDaughters()) {
              DauPDG2.push_back(mcDaughter->getPDG());
            }
            modeB2 = gBtag.Mode_B0(DauPDG2);
            break;
          }
        }
      }

      if (modeB1 != -99) { modeB = modeB1;}
      else if (modeB2 != -99) { modeB = modeB2;}

      return modeB;
    }

    double Bs0Mode(const Particle*)
    {
      GenBsTag gBtag;
      StoreArray<MCParticle> MC_Particle_list;
      if (!MC_Particle_list) {
        B2WARNING("GenMCTagTool::eval - missing MCParticles array");
        return std::numeric_limits<double>::quiet_NaN();
      }
      int modeB1 = -99, modeB2 = -99, modeB = -99, nb1 = 0, nb2 = 0;

      for (const auto& iMCParticle : MC_Particle_list) {
        if (iMCParticle.getPDG() == 531) {
          nb1++;
          if (nb1 == 1) {
            std::vector<int> DauPDG1;
            for (auto const& mcDaughter : iMCParticle.getDaughters()) {
              DauPDG1.push_back(mcDaughter->getPDG());
            }
            modeB1 = gBtag.Mode_Bs0(DauPDG1);
            break;
          }
        }
        if (iMCParticle.getPDG() == -531) {
          nb2++;
          if (nb2 == 2) {
            std::vector<int> DauPDG2;
            for (auto const& mcDaughter : iMCParticle.getDaughters()) {
              DauPDG2.push_back(mcDaughter->getPDG());
            }
            modeB2 = gBtag.Mode_anti_Bs0(DauPDG2);
            break;
          }
        }
      }

      if (modeB1 != -99) { modeB = modeB1;}
      else if (modeB2 != -99) { modeB = modeB2;}

      return modeB;
    }

    double Bsbar0Mode(const Particle*)
    {
      GenBsTag gBtag;
      StoreArray<MCParticle> MC_Particle_list;
      if (!MC_Particle_list) {
        B2WARNING("GenMCTagTool::eval - missing MCParticles array");
        return std::numeric_limits<double>::quiet_NaN();
      }
      int modeB1 = -99, modeB2 = -99, modeB = -99, nb1 = 0, nb2 = 0;

      for (const auto& iMCParticle : MC_Particle_list) {
        if (iMCParticle.getPDG() == -531) {
          nb1++;
          if (nb1 == 1) {
            std::vector<int> DauPDG1;
            for (auto const& mcDaughter : iMCParticle.getDaughters()) {
              DauPDG1.push_back(mcDaughter->getPDG());
            }
            modeB1 = gBtag.Mode_anti_Bs0(DauPDG1);
            break;
          }
        }
        if (iMCParticle.getPDG() == 531) {
          nb2++;
          if (nb2 == 2) {
            std::vector<int> DauPDG2;
            for (auto const& mcDaughter : iMCParticle.getDaughters()) {
              DauPDG2.push_back(mcDaughter->getPDG());
            }
            modeB2 = gBtag.Mode_Bs0(DauPDG2);
            break;
          }
        }
      }

      if (modeB1 != -99) { modeB = modeB1;}
      else if (modeB2 != -99) { modeB = modeB2;}

      return modeB;
    }

    double DstplusMode(const Particle*)
    {
      GenDTag gDtag;
      StoreArray<MCParticle> MC_Particle_list;
      if (!MC_Particle_list) {
        B2WARNING("GenMCTagTool::eval - missing MCParticles array");
        return std::numeric_limits<double>::quiet_NaN();
      }
      int modeD = -99;

      for (const auto& iMCParticle : MC_Particle_list) {
        if (iMCParticle.getPDG() != 413) continue;

        std::vector<int> DauPDG;
        for (auto const& mcDaughter : iMCParticle.getDaughters()) {
          DauPDG.push_back(mcDaughter->getPDG());
        }
        modeD = gDtag.Mode_Dst_plus(DauPDG);
        break;
      }

      return modeD;
    }

    double DstminusMode(const Particle*)
    {
      GenDTag gDtag;
      StoreArray<MCParticle> MC_Particle_list;
      if (!MC_Particle_list) {
        B2WARNING("GenMCTagTool::eval - missing MCParticles array");
        return std::numeric_limits<double>::quiet_NaN();
      }
      int modeD = -99;

      for (const auto& iMCParticle : MC_Particle_list) {
        if (iMCParticle.getPDG() != -413) continue;

        std::vector<int> DauPDG;
        for (auto const& mcDaughter : iMCParticle.getDaughters()) {
          DauPDG.push_back(mcDaughter->getPDG());
        }
        modeD = gDtag.Mode_Dst_minus(DauPDG);
        break;
      }

      return modeD;
    }

    double DsplusMode(const Particle*)
    {
      GenDTag gDtag;
      StoreArray<MCParticle> MC_Particle_list;
      if (!MC_Particle_list) {
        B2WARNING("GenMCTagTool::eval - missing MCParticles array");
        return std::numeric_limits<double>::quiet_NaN();
      }
      int modeD = -99;

      for (const auto& iMCParticle : MC_Particle_list) {
        if (iMCParticle.getPDG() != 431) continue;

        std::vector<int> DauPDG;
        for (auto const& mcDaughter : iMCParticle.getDaughters()) {
          DauPDG.push_back(mcDaughter->getPDG());
        }
        modeD = gDtag.Mode_Ds_plus(DauPDG);
        break;
      }

      return modeD;
    }

    double DsminusMode(const Particle*)
    {
      GenDTag gDtag;
      StoreArray<MCParticle> MC_Particle_list;
      if (!MC_Particle_list) {
        B2WARNING("GenMCTagTool::eval - missing MCParticles array");
        return std::numeric_limits<double>::quiet_NaN();
      }
      int modeD = -99;

      for (const auto& iMCParticle : MC_Particle_list) {
        if (iMCParticle.getPDG() != -431) continue;

        std::vector<int> DauPDG;
        for (auto const& mcDaughter : iMCParticle.getDaughters()) {
          DauPDG.push_back(mcDaughter->getPDG());
        }
        modeD = gDtag.Mode_Ds_minus(DauPDG);
        break;
      }

      return modeD;
    }

    double DplusMode(const Particle*)
    {
      GenDTag gDtag;
      StoreArray<MCParticle> MC_Particle_list;
      if (!MC_Particle_list) {
        B2WARNING("GenMCTagTool::eval - missing MCParticles array");
        return std::numeric_limits<double>::quiet_NaN();
      }
      int modeD = -99;

      for (const auto& iMCParticle : MC_Particle_list) {
        if (iMCParticle.getPDG() != 411) continue;

        std::vector<int> DauPDG;
        for (auto const& mcDaughter : iMCParticle.getDaughters()) {
          DauPDG.push_back(mcDaughter->getPDG());
        }
        modeD = gDtag.Mode_D_plus(DauPDG);
        break;
      }

      return modeD;
    }

    double DminusMode(const Particle*)
    {
      GenDTag gDtag;
      StoreArray<MCParticle> MC_Particle_list;
      if (!MC_Particle_list) {
        B2WARNING("GenMCTagTool::eval - missing MCParticles array");
        return std::numeric_limits<double>::quiet_NaN();
      }
      int modeD = -99;

      for (const auto& iMCParticle : MC_Particle_list) {
        if (iMCParticle.getPDG() != -411) continue;

        std::vector<int> DauPDG;
        for (auto const& mcDaughter : iMCParticle.getDaughters()) {
          DauPDG.push_back(mcDaughter->getPDG());
        }
        modeD = gDtag.Mode_D_minus(DauPDG);
        break;
      }

      return modeD;
    }

    double D0Mode(const Particle*)
    {
      GenDTag gDtag;
      StoreArray<MCParticle> MC_Particle_list;
      if (!MC_Particle_list) {
        B2WARNING("GenMCTagTool::eval - missing MCParticles array");
        return std::numeric_limits<double>::quiet_NaN();
      }
      int modeD = -99;

      for (const auto& iMCParticle : MC_Particle_list) {
        if (iMCParticle.getPDG() != 421) continue;

        std::vector<int> DauPDG;
        for (auto const& mcDaughter : iMCParticle.getDaughters()) {
          DauPDG.push_back(mcDaughter->getPDG());
        }
        modeD = gDtag.Mode_D0(DauPDG);
        break;
      }

      return modeD;
    }

    double Dbar0Mode(const Particle*)
    {
      GenDTag gDtag;
      StoreArray<MCParticle> MC_Particle_list;
      if (!MC_Particle_list) {
        B2WARNING("GenMCTagTool::eval - missing MCParticles array");
        return std::numeric_limits<double>::quiet_NaN();
      }
      int modeD = -99;

      for (const auto& iMCParticle : MC_Particle_list) {
        if (iMCParticle.getPDG() != -421) continue;

        std::vector<int> DauPDG;
        for (auto const& mcDaughter : iMCParticle.getDaughters()) {
          DauPDG.push_back(mcDaughter->getPDG());
        }
        modeD = gDtag.Mode_anti_D0(DauPDG);
        break;
      }

      return modeD;
    }

    double TauplusMode(const Particle*)
    {
      GenTauTag gTKtag;
      StoreArray<MCParticle> MC_Particle_list;
      if (!MC_Particle_list) {
        B2WARNING("GenMCTagTool::eval - missing MCParticles array");
        return std::numeric_limits<double>::quiet_NaN();
      }
      int modeTau = -1;

      for (const auto& iMCParticle : MC_Particle_list) {
        if (iMCParticle.getPDG() != -15) continue;

        std::vector<int> DauPDG;
        for (auto const& mcDaughter : iMCParticle.getDaughters()) {
          if (mcDaughter->getPDG() == 24) {
            for (auto const& WDaughter : mcDaughter->getDaughters()) {
              DauPDG.push_back(WDaughter->getPDG());
            }
          } else DauPDG.push_back(mcDaughter->getPDG());
        }
        modeTau = gTKtag.Mode_tau_plus(DauPDG);
        break;
      }

      return modeTau;
    }

    double TauminusMode(const Particle*)
    {
      GenTauTag gTKtag;
      StoreArray<MCParticle> MC_Particle_list;
      if (!MC_Particle_list) {
        B2WARNING("GenMCTagTool::eval - missing MCParticles array");
        return std::numeric_limits<double>::quiet_NaN();
      }
      int modeTau = -1;

      for (const auto& iMCParticle : MC_Particle_list) {
        if (iMCParticle.getPDG() != 15) continue;

        std::vector<int> DauPDG;
        for (auto const& mcDaughter : iMCParticle.getDaughters()) {
          if (mcDaughter->getPDG() == -24) {
            for (auto const& WDaughter : mcDaughter->getDaughters()) {
              DauPDG.push_back(WDaughter->getPDG());
            }
          } else DauPDG.push_back(mcDaughter->getPDG());
        }
        modeTau = gTKtag.Mode_tau_minus(DauPDG);
        break;
      }

      return modeTau;
    }

    VARIABLE_GROUP("MCParticle tag variables");
    REGISTER_VARIABLE("BplusMode", BplusMode, "[Eventbased] It will return the decays mode of B+ particles");
    REGISTER_VARIABLE("BminusMode", BminusMode, "[Eventbased] It will return the decays mode of B- particles");
    REGISTER_VARIABLE("B0Mode", B0Mode, "[Eventbased] It will return the decays mode of B0 particles");
    REGISTER_VARIABLE("Bbar0Mode", Bbar0Mode, "[Eventbased] It will return the decays mode of anti-B0 particles");
    REGISTER_VARIABLE("Bs0Mode", Bs0Mode, "[Eventbased] It will return the decays mode of B_s0 particles");
    REGISTER_VARIABLE("Bsbar0Mode", Bsbar0Mode, "[Eventbased] It will return the decays mode of anti-B_s0 particles");
    REGISTER_VARIABLE("DstplusMode", DstplusMode, "[Eventbased] It will return the decays mode of D*+ particles");
    REGISTER_VARIABLE("DstminusMode", DstminusMode, "[Eventbased] It will return the decays mode of D*- particles");
    REGISTER_VARIABLE("DsplusMode", DsplusMode, "[Eventbased] It will return the decays mode of D_s+ particles");
    REGISTER_VARIABLE("DsminusMode", DsminusMode, "[Eventbased] It will return the decays mode of D_s- particles");
    REGISTER_VARIABLE("DplusMode", DplusMode, "[Eventbased] It will return the decays mode of D+ particles");
    REGISTER_VARIABLE("DminusMode", DminusMode, "[Eventbased] It will return the decays mode of D- particles");
    REGISTER_VARIABLE("D0Mode", D0Mode, "[Eventbased] It will return the decays mode of D0 particles");
    REGISTER_VARIABLE("Dbar0Mode", Dbar0Mode, "[Eventbased] It will return the decays mode of anti-D0 particles");
    REGISTER_VARIABLE("TauplusMode", TauplusMode, "[Eventbased] It will return the decays mode of tau+ particles");
    REGISTER_VARIABLE("TauminusMode", TauminusMode, "[Eventbased] It will return the decays mode of tau- particles");
  }
}
