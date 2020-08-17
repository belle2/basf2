/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sourav Patra, Vishal Bhardwaj                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <analysis/VariableManager/Manager.h>
#include <analysis/variables/MCParticleTag.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/TauPairDecay.h>
#include <analysis/utility/GenBplusTag.h>
#include <analysis/utility/GenB0Tag.h>
#include <analysis/utility/GenBsTag.h>
#include <analysis/utility/GenDTag.h>
#include <analysis/utility/GenTauTag.h>
#include <mdst/dataobjects/MCParticle.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <framework/core/Environment.h>
#include <framework/database/DBObjPtr.h>
#include <framework/core/Module.h>

#include <queue>
#include <vector>
#include <string>
#include <iostream>
#include <cmath>


namespace Belle2 {
  namespace Variable {

    int BplusMode(const Particle* part)
    {
      GenBplusTag gBtag;
      if (!part) {
        B2WARNING("GenMCTagTool::eval - no Particle found!");
        return -999;
      }
      StoreArray<MCParticle> MC_Particle_list;
      if (!MC_Particle_list) {
        B2WARNING("GenMCTagTool::eval - missing MCParticles array");
        return -888;
      }
      int modeB = -99;
      int Entries = MC_Particle_list.getEntries();

      for (int i = 0; i < Entries; ++i) {
        const MCParticle* iMCParticle = MC_Particle_list[i];
        if (iMCParticle->getPDG() == 521) {
          std::vector<MCParticle*> MC_Daughters = iMCParticle->getDaughters();
          int  DEntries = MC_Daughters.size();
          std::vector<int> DauPDG;
          for (int jd = 0; jd < DEntries; ++jd) {
            DauPDG.push_back(MC_Daughters[jd]->getPDG());
          }
          modeB  = gBtag.Mode_B_plus(DauPDG);
        }
      }

      return modeB;
    }


    int BminusMode(const Particle* part)
    {
      GenBplusTag gBtag;
      if (!part) {
        B2WARNING("GenMCTagTool::eval - no Particle found!");
        return -999;
      }
      StoreArray<MCParticle> MC_Particle_list;
      if (!MC_Particle_list) {
        B2WARNING("GenMCTagTool::eval - missing MCParticles array");
        return -888;
      }
      int modeB = -99;
      int Entries = MC_Particle_list.getEntries();

      for (int i = 0; i < Entries; ++i) {
        const MCParticle* iMCParticle = MC_Particle_list[i];
        if (iMCParticle->getPDG() == -521) {
          std::vector<MCParticle*> MC_Daughters = iMCParticle->getDaughters();
          int  DEntries = MC_Daughters.size();
          std::vector<int> DauPDG;
          for (int jd = 0; jd < DEntries; ++jd) {
            DauPDG.push_back(MC_Daughters[jd]->getPDG());
          }
          modeB  = gBtag.Mode_B_minus(DauPDG);
        }
      }

      return modeB;
    }


    int B0Mode(const Particle* part)
    {
      GenB0Tag gBtag;
      if (!part) {
        B2WARNING("GenMCTagTool::eval - no Particle found!");
        return -999;
      }
      StoreArray<MCParticle> MC_Particle_list;
      if (!MC_Particle_list) {
        B2WARNING("GenMCTagTool::eval - missing MCParticles array");
        return -888;
      }
      int modeB1 = -99, modeB2 = -99, modeB = -99, nb1 = 0, nb2 = 0;
      int Entries = MC_Particle_list.getEntries();

      for (int i = 0; i < Entries; i++) {
        const MCParticle* iMCParticle = MC_Particle_list[i];
        if (iMCParticle->getPDG() == 511) {
          nb1++;
          if (nb1 == 1) {
            std::vector<MCParticle*> MC_Daughters1 = iMCParticle->getDaughters();
            int  DEntries1 = MC_Daughters1.size();
            std::vector<int> DauPDG1;
            for (int jd = 0; jd < DEntries1; ++jd) {
              DauPDG1.push_back(MC_Daughters1[jd]->getPDG());
            }
            modeB1 = gBtag.Mode_B0(DauPDG1);
          }
        }
        if (iMCParticle->getPDG() == -511) {
          nb2++;
          if (nb2 == 2) {
            std::vector<MCParticle*> MC_Daughters2 = iMCParticle->getDaughters();
            int  DEntries2 = MC_Daughters2.size();
            std::vector<int> DauPDG2;
            for (int jd = 0; jd < DEntries2; ++jd) {
              DauPDG2.push_back(MC_Daughters2[jd]->getPDG());
            }
            modeB2 = gBtag.Mode_anti_B0(DauPDG2);
          }
        }
      }

      if (modeB1 != -99) { modeB = modeB1;}
      else if (nb2 > 1 && modeB2 != -99) { modeB = modeB2;}

      return modeB;
    }


    int Bbar0Mode(const Particle* part)
    {
      GenB0Tag gBtag;
      if (!part) {
        B2WARNING("GenMCTagTool::eval - no Particle found!");
        return -999;
      }
      StoreArray<MCParticle> MC_Particle_list;
      if (!MC_Particle_list) {
        B2WARNING("GenMCTagTool::eval - missing MCParticles array");
        return -888;
      }
      int modeB1 = -99, modeB2 = -99, modeB = -99, nb1 = 0, nb2 = 0;
      int Entries = MC_Particle_list.getEntries();

      for (int i = 0; i < Entries; i++) {
        const MCParticle* iMCParticle = MC_Particle_list[i];
        if (iMCParticle->getPDG() == -511) {
          nb1++;
          if (nb1 == 1) {
            std::vector<MCParticle*> MC_Daughters1 = iMCParticle->getDaughters();
            int  DEntries1 = MC_Daughters1.size();
            std::vector<int> DauPDG1;
            for (int jd = 0; jd < DEntries1; ++jd) {
              DauPDG1.push_back(MC_Daughters1[jd]->getPDG());
            }
            modeB1 = gBtag.Mode_anti_B0(DauPDG1);
          }
        }
        if (iMCParticle->getPDG() == 511) {
          nb2++;
          if (nb2 == 2) {
            std::vector<MCParticle*> MC_Daughters2 = iMCParticle->getDaughters();
            int  DEntries2 = MC_Daughters2.size();
            std::vector<int> DauPDG2;
            for (int jd = 0; jd < DEntries2; ++jd) {
              DauPDG2.push_back(MC_Daughters2[jd]->getPDG());
            }
            modeB2 = gBtag.Mode_B0(DauPDG2);
          }
        }
      }

      if (modeB1 != -99) { modeB = modeB1;}
      else if (nb2 > 1 && modeB2 != -99) { modeB = modeB2;}

      return modeB;
    }

    int Bs0Mode(const Particle* part)
    {
      GenBsTag gBtag;
      if (!part) {
        B2WARNING("GenMCTagTool::eval - no Particle found!");
        return -999;
      }
      StoreArray<MCParticle> MC_Particle_list;
      if (!MC_Particle_list) {
        B2WARNING("GenMCTagTool::eval - missing MCParticles array");
        return -888;
      }
      int modeB1 = -99, modeB2 = -99, modeB = -99, nb1 = 0, nb2 = 0;
      int Entries = MC_Particle_list.getEntries();

      for (int i = 0; i < Entries; i++) {
        const MCParticle* iMCParticle = MC_Particle_list[i];
        if (iMCParticle->getPDG() == 531) {
          nb1++;
          if (nb1 == 1) {
            std::vector<MCParticle*> MC_Daughters1 = iMCParticle->getDaughters();
            int  DEntries1 = MC_Daughters1.size();
            std::vector<int> DauPDG1;
            for (int jd = 0; jd < DEntries1; ++jd) {
              DauPDG1.push_back(MC_Daughters1[jd]->getPDG());
            }
            modeB1 = gBtag.Mode_Bs0(DauPDG1);
          }
        }
        if (iMCParticle->getPDG() == -531) {
          nb2++;
          if (nb2 == 2) {
            std::vector<MCParticle*> MC_Daughters2 = iMCParticle->getDaughters();
            int  DEntries2 = MC_Daughters2.size();
            std::vector<int> DauPDG2;
            for (int jd = 0; jd < DEntries2; ++jd) {
              DauPDG2.push_back(MC_Daughters2[jd]->getPDG());
            }
            modeB2 = gBtag.Mode_anti_Bs0(DauPDG2);
          }
        }
      }

      if (modeB1 != -99) { modeB = modeB1;}
      else if (nb2 > 1 && modeB2 != -99) { modeB = modeB2;}

      return modeB;
    }


    int Bsbar0Mode(const Particle* part)
    {
      GenBsTag gBtag;
      if (!part) {
        B2WARNING("GenMCTagTool::eval - no Particle found!");
        return -999;
      }
      StoreArray<MCParticle> MC_Particle_list;
      if (!MC_Particle_list) {
        B2WARNING("GenMCTagTool::eval - missing MCParticles array");
        return -888;
      }
      int modeB1 = -99, modeB2 = -99, modeB = -99, nb1 = 0, nb2 = 0;
      int Entries = MC_Particle_list.getEntries();

      for (int i = 0; i < Entries; i++) {
        const MCParticle* iMCParticle = MC_Particle_list[i];
        if (iMCParticle->getPDG() == -531) {
          nb1++;
          if (nb1 == 1) {
            std::vector<MCParticle*> MC_Daughters1 = iMCParticle->getDaughters();
            int  DEntries1 = MC_Daughters1.size();
            std::vector<int> DauPDG1;
            for (int jd = 0; jd < DEntries1; ++jd) {
              DauPDG1.push_back(MC_Daughters1[jd]->getPDG());
            }
            modeB1 = gBtag.Mode_anti_Bs0(DauPDG1);
          }
        }
        if (iMCParticle->getPDG() == 531) {
          nb2++;
          if (nb2 == 2) {
            std::vector<MCParticle*> MC_Daughters2 = iMCParticle->getDaughters();
            int  DEntries2 = MC_Daughters2.size();
            std::vector<int> DauPDG2;
            for (int jd = 0; jd < DEntries2; ++jd) {
              DauPDG2.push_back(MC_Daughters2[jd]->getPDG());
            }
            modeB2 = gBtag.Mode_Bs0(DauPDG2);
          }
        }
      }

      if (modeB1 != -99) { modeB = modeB1;}
      else if (nb2 > 1 && modeB2 != -99) { modeB = modeB2;}

      return modeB;
    }


    int DstplusMode(const Particle* part)
    {
      GenDTag gDtag;
      if (!part) {
        B2WARNING("GenMCTagTool::eval - no Particle found!");
        return -999;
      }
      StoreArray<MCParticle> MC_Particle_list;
      if (!MC_Particle_list) {
        B2WARNING("GenMCTagTool::eval - missing MCParticles array");
        return -888;
      }
      int modeD = -99;
      int Entries = MC_Particle_list.getEntries();

      for (int i = 0; i < Entries; ++i) {
        const MCParticle* iMCParticle = MC_Particle_list[i];
        if (iMCParticle->getPDG() == 413) {
          std::vector<MCParticle*> MC_Daughters = iMCParticle->getDaughters();
          int  DEntries = MC_Daughters.size();
          std::vector<int> DauPDG;
          for (int jd = 0; jd < DEntries; ++jd) {
            DauPDG.push_back(MC_Daughters[jd]->getPDG());
          }
          modeD  = gDtag.Mode_Dst_plus(DauPDG);
        }
      }

      return modeD;
    }


    int DstminusMode(const Particle* part)
    {
      GenDTag gDtag;
      if (!part) {
        B2WARNING("GenMCTagTool::eval - no Particle found!");
        return -999;
      }
      StoreArray<MCParticle> MC_Particle_list;
      if (!MC_Particle_list) {
        B2WARNING("GenMCTagTool::eval - missing MCParticles array");
        return -888;
      }
      int modeD = -99;
      int Entries = MC_Particle_list.getEntries();

      for (int i = 0; i < Entries; ++i) {
        const MCParticle* iMCParticle = MC_Particle_list[i];
        if (iMCParticle->getPDG() == -413) {
          std::vector<MCParticle*> MC_Daughters = iMCParticle->getDaughters();
          int  DEntries = MC_Daughters.size();
          std::vector<int> DauPDG;
          for (int jd = 0; jd < DEntries; ++jd) {
            DauPDG.push_back(MC_Daughters[jd]->getPDG());
          }
          modeD  = gDtag.Mode_Dst_minus(DauPDG);
        }
      }

      return modeD;
    }


    int DsplusMode(const Particle* part)
    {
      GenDTag gDtag;
      if (!part) {
        B2WARNING("GenMCTagTool::eval - no Particle found!");
        return -999;
      }
      StoreArray<MCParticle> MC_Particle_list;
      if (!MC_Particle_list) {
        B2WARNING("GenMCTagTool::eval - missing MCParticles array");
        return -888;
      }
      int modeD = -99;
      int Entries = MC_Particle_list.getEntries();

      for (int i = 0; i < Entries; ++i) {
        const MCParticle* iMCParticle = MC_Particle_list[i];
        if (iMCParticle->getPDG() == 431) {
          std::vector<MCParticle*> MC_Daughters = iMCParticle->getDaughters();
          int  DEntries = MC_Daughters.size();
          std::vector<int> DauPDG;
          for (int jd = 0; jd < DEntries; ++jd) {
            DauPDG.push_back(MC_Daughters[jd]->getPDG());
          }
          modeD  = gDtag.Mode_Ds_plus(DauPDG);
        }
      }

      return modeD;
    }


    int DsminusMode(const Particle* part)
    {
      GenDTag gDtag;
      if (!part) {
        B2WARNING("GenMCTagTool::eval - no Particle found!");
        return -999;
      }
      StoreArray<MCParticle> MC_Particle_list;
      if (!MC_Particle_list) {
        B2WARNING("GenMCTagTool::eval - missing MCParticles array");
        return -888;
      }
      int modeD = -99;
      int Entries = MC_Particle_list.getEntries();

      for (int i = 0; i < Entries; ++i) {
        const MCParticle* iMCParticle = MC_Particle_list[i];
        if (iMCParticle->getPDG() == -431) {
          std::vector<MCParticle*> MC_Daughters = iMCParticle->getDaughters();
          int  DEntries = MC_Daughters.size();
          std::vector<int> DauPDG;
          for (int jd = 0; jd < DEntries; ++jd) {
            DauPDG.push_back(MC_Daughters[jd]->getPDG());
          }
          modeD  = gDtag.Mode_Ds_minus(DauPDG);
        }
      }

      return modeD;
    }


    int DplusMode(const Particle* part)
    {
      GenDTag gDtag;
      if (!part) {
        B2WARNING("GenMCTagTool::eval - no Particle found!");
        return -999;
      }
      StoreArray<MCParticle> MC_Particle_list;
      if (!MC_Particle_list) {
        B2WARNING("GenMCTagTool::eval - missing MCParticles array");
        return -888;
      }
      int modeD = -99;
      int Entries = MC_Particle_list.getEntries();

      for (int i = 0; i < Entries; ++i) {
        const MCParticle* iMCParticle = MC_Particle_list[i];
        if (iMCParticle->getPDG() == 411) {
          std::vector<MCParticle*> MC_Daughters = iMCParticle->getDaughters();
          int  DEntries = MC_Daughters.size();
          std::vector<int> DauPDG;
          for (int jd = 0; jd < DEntries; ++jd) {
            DauPDG.push_back(MC_Daughters[jd]->getPDG());
          }
          modeD  = gDtag.Mode_D_plus(DauPDG);
        }
      }

      return modeD;
    }


    int DminusMode(const Particle* part)
    {
      GenDTag gDtag;
      if (!part) {
        B2WARNING("GenMCTagTool::eval - no Particle found!");
        return -999;
      }
      StoreArray<MCParticle> MC_Particle_list;
      if (!MC_Particle_list) {
        B2WARNING("GenMCTagTool::eval - missing MCParticles array");
        return -888;
      }
      int modeD = -99;
      int Entries = MC_Particle_list.getEntries();

      for (int i = 0; i < Entries; ++i) {
        const MCParticle* iMCParticle = MC_Particle_list[i];
        if (iMCParticle->getPDG() == -411) {
          std::vector<MCParticle*> MC_Daughters = iMCParticle->getDaughters();
          int  DEntries = MC_Daughters.size();
          std::vector<int> DauPDG;
          for (int jd = 0; jd < DEntries; ++jd) {
            DauPDG.push_back(MC_Daughters[jd]->getPDG());
          }
          modeD  = gDtag.Mode_D_minus(DauPDG);
        }
      }

      return modeD;
    }


    int D0Mode(const Particle* part)
    {
      GenDTag gDtag;
      if (!part) {
        B2WARNING("GenMCTagTool::eval - no Particle found!");
        return -999;
      }
      StoreArray<MCParticle> MC_Particle_list;
      if (!MC_Particle_list) {
        B2WARNING("GenMCTagTool::eval - missing MCParticles array");
        return -888;
      }
      int modeD = -99;
      int Entries = MC_Particle_list.getEntries();

      for (int i = 0; i < Entries; ++i) {
        const MCParticle* iMCParticle = MC_Particle_list[i];
        if (iMCParticle->getPDG() == 421) {
          std::vector<MCParticle*> MC_Daughters = iMCParticle->getDaughters();
          int  DEntries = MC_Daughters.size();
          std::vector<int> DauPDG;
          for (int jd = 0; jd < DEntries; ++jd) {
            DauPDG.push_back(MC_Daughters[jd]->getPDG());
          }
          modeD  = gDtag.Mode_D0(DauPDG);
        }
      }

      return modeD;
    }


    int Dbar0Mode(const Particle* part)
    {
      GenDTag gDtag;
      if (!part) {
        B2WARNING("GenMCTagTool::eval - no Particle found!");
        return -999;
      }
      StoreArray<MCParticle> MC_Particle_list;
      if (!MC_Particle_list) {
        B2WARNING("GenMCTagTool::eval - missing MCParticles array");
        return -888;
      }
      int modeD = -99;
      int Entries = MC_Particle_list.getEntries();

      for (int i = 0; i < Entries; ++i) {
        const MCParticle* iMCParticle = MC_Particle_list[i];
        if (iMCParticle->getPDG() == -421) {
          std::vector<MCParticle*> MC_Daughters = iMCParticle->getDaughters();
          int  DEntries = MC_Daughters.size();
          std::vector<int> DauPDG;
          for (int jd = 0; jd < DEntries; ++jd) {
            DauPDG.push_back(MC_Daughters[jd]->getPDG());
          }
          modeD  = gDtag.Mode_anti_D0(DauPDG);
        }
      }

      return modeD;
    }


    int TauplusMode(const Particle* part)
    {
      GenTauTag gTKtag;
      if (!part) {
        B2WARNING("GenMCTagTool::eval - no Particle found!");
        return -999;
      }
      StoreArray<MCParticle> MC_Particle_list;
      if (!MC_Particle_list) {
        B2WARNING("GenMCTagTool::eval - missing MCParticles array");
        return -888;
      }
      int modeTau = -99;
      int Entries = MC_Particle_list.getEntries();

      for (int i = 0; i < Entries; ++i) {
        const MCParticle* iMCParticle = MC_Particle_list[i];
        if (iMCParticle->getPDG() == -15) {
          std::vector<MCParticle*> MC_Daughters = iMCParticle->getDaughters();
          int  DEntries = MC_Daughters.size();
          std::vector<int> DauPDG;
          for (int jd = 0; jd < DEntries; ++jd) {
            DauPDG.push_back(MC_Daughters[jd]->getPDG());
          }
          modeTau  = gTKtag.Mode_tau_plus(DauPDG);
        }
      }

      return modeTau;
    }


    int TauminusMode(const Particle* part)
    {
      GenTauTag gTKtag;
      if (!part) {
        B2WARNING("GenMCTagTool::eval - no Particle found!");
        return -999;
      }
      StoreArray<MCParticle> MC_Particle_list;
      if (!MC_Particle_list) {
        B2WARNING("GenMCTagTool::eval - missing MCParticles array");
        return -888;
      }
      int modeTau = -99;
      int Entries = MC_Particle_list.getEntries();

      for (int i = 0; i < Entries; ++i) {
        const MCParticle* iMCParticle = MC_Particle_list[i];
        if (iMCParticle->getPDG() == 15) {
          std::vector<MCParticle*> MC_Daughters = iMCParticle->getDaughters();
          int  DEntries = MC_Daughters.size();
          std::vector<int> DauPDG;
          for (int jd = 0; jd < DEntries; ++jd) {
            DauPDG.push_back(MC_Daughters[jd]->getPDG());
          }
          modeTau  = gTKtag.Mode_tau_minus(DauPDG);
        }
      }

      return modeTau;
    }


    VARIABLE_GROUP("MCParticle tag variables");
    REGISTER_VARIABLE("BplusMode", BplusMode, "It will return the decays mode of B+ particles");
    REGISTER_VARIABLE("BminusMode", BminusMode, "It will return the decays mode of B- particles");
    REGISTER_VARIABLE("B0Mode", B0Mode, "It will return the decays mode of B0 particles");
    REGISTER_VARIABLE("Bbar0Mode", Bbar0Mode, "It will return the decays mode of anti-B0 particles");
    REGISTER_VARIABLE("Bs0Mode", Bs0Mode, "It will return the decays mode of B_s0 particles");
    REGISTER_VARIABLE("Bsbar0Mode", Bsbar0Mode, "It will return the decays mode of anti-B_s0 particles");
    REGISTER_VARIABLE("DstplusMode", DstplusMode, "It will return the decays mode of D*+ particles");
    REGISTER_VARIABLE("DstminusMode", DstminusMode, "It will return the decays mode of D*- particles");
    REGISTER_VARIABLE("DsplusMode", DsplusMode, "It will return the decays mode of D_s+ particles");
    REGISTER_VARIABLE("DsminusMode", DsminusMode, "It will return the decays mode of D_s- particles");
    REGISTER_VARIABLE("DplusMode", DplusMode, "It will return the decays mode of D+ particles");
    REGISTER_VARIABLE("DminusMode", DminusMode, "It will return the decays mode of D- particles");
    REGISTER_VARIABLE("D0Mode", D0Mode, "It will return the decays mode of D0 particles");
    REGISTER_VARIABLE("Dbar0Mode", Dbar0Mode, "It will return the decays mode of anti-D0 particles");
    REGISTER_VARIABLE("TauplusMode", TauplusMode, "It will return the decays mode of tau+ particles");
    REGISTER_VARIABLE("TauminusMode", TauminusMode, "It will return the decays mode of tau- particles");
  }
  class MCParticleTagModule: public Module {};
  REG_MODULE(MCParticleTag);
}
