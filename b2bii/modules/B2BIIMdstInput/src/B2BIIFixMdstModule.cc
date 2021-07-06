/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//
// $Id: B2BIIFixMdst_impl.cc 11331 2011-07-25 11:58:12Z hitoshi $
//
// $Log$
//
// Revision 2.0 2015/03/11 tkeck
// Conversion to Belle II
//
// Revision 1.40  2005/08/25 10:08:40  hitoshi
// not try to update s_benergy_value in the begin_run in order to suppress the unwanted warning message
//
// Revision 1.39  2005/03/10 14:14:57  hitoshi
// check table sizes
//
// Revision 1.38  2005/01/27 13:29:09  katayama
// fixed a bug
//
// Revision 1.37  2004/07/27 08:55:48  hitoshi
// not to output a misleading warning message about benergy (by Matsumoto)
//
// Revision 1.36  2004/04/25 16:33:33  hitoshi
// modified to be able to handle output from low p finder (by kakuno). modified
// Benergy to fetch data from DB (by Shibata).
//
// Revision 1.35  2004/03/09 10:23:04  hitoshi
// beam energies for exp31 (by Sanjay/Hulya)
//
// Revision 1.34  2003/07/10 13:45:38  hitoshi
// update for exp27 run700- (by Sanjay and Senyo).
//
// Revision 1.33  2003/07/03 00:06:22  katayama
// aviod seg fault
//
// Revision 1.32  2003/07/01 07:15:18  hitoshi
// fixed typo in warning message.
//
// Revision 1.31  2003/06/30 22:16:29  hitoshi
// update for exp27 (by Miyabayashi, Sanjay, Senyo).
//
// Revision 1.30  2003/06/24 07:55:16  katayama
// Remived unused parameters
//
// Revision 1.29  2003/06/01 09:21:01  hitoshi
// update for exp25 (by Sanjay).
//
// Revision 1.28  2002/12/28 23:10:08  katayama
// ecl and benergy for exp. 21 and 32 from Sanjay and Miyavayashi san
//
// Revision 1.27  2002/09/06 03:47:03  hitoshi
// Endcap_MX_layer=11 is now the default (by muon group).
//
// Revision 1.25  2002/07/10 09:35:04  katayama
// Call check_beginrun and then call begin_run
//
// Revision 1.24  2002/07/03 04:14:51  hitoshi
// beam energies for run900 - 1643 in e19 (by Sanjay san)
//
// Revision 1.23  2002/06/27 01:38:11  katayama
// Call Calib_dEdx_begin_run
//
// Revision 1.22  2002/06/25 10:53:03  katayama
// good_event bug fixed
//
// Revision 1.21  2002/06/22 06:22:44  katayama
// New from Sanjay san (exp. 19 run 460-899)
//
//
// Revision 1.21  2002/06/22 12:48:50  sanjay
// added benergy for run460-900 in e19
//
// Revision 1.20  2002/06/10 23:31:42  hitoshi
// returns nominal benergy for MC data at begin_run.
//
// Revision 1.19  2002/06/10 22:43:58  katayama
// Benergy works at begin run time
//
// Revision 1.18  2002/06/07 10:31:47  hitoshi
// added benergy for runs300 - 459 in e19.
//
// Revision 1.17  2002/06/06 05:05:54  hitoshi
// returns status for events with no Belle_event table.
//
// Revision 1.16  2002/06/04 07:59:08  hitoshi
// added protection for bad events: i.e. events w/o Belle_event or mdst_event_addtable.
//
// Revision 1.15  2002/06/03 21:39:49  hitoshi
// added benergy for run64-299 in e19 (by Sanjay).
//
// Revision 1.14  2002/05/08 11:55:51  hitoshi
// added benergy for exp17 (by Sanjay san).
//
// Revision 1.13  2002/04/26 01:25:07  katayama
// New correction from Perers san
//
// Revision 1.12  2002/04/19 07:24:39  katayama
// Added pi0/vertex, calibdedx
//
// Revision 1.11  2002/04/05 01:19:33  katayama
// templates for ecl_primary_vertex
//
// Revision 1.10  2002/03/31 06:54:41  katayama
// fix_pi0
//
// Revision 1.9  2002/03/30 23:13:41  katayama
// typo
//
// Revision 1.8  2002/03/30 10:31:12  katayama
// fixed
//
// Revision 1.7  2002/03/30 01:06:05  katayama
// HadronB & L4passed events only
//
// Revision 1.6  2002/03/19 04:25:47  katayama
// Add parameters for scale_momenta, set nominal b energy
//
// Revision 1.5  2002/03/16 07:28:39  hitoshi
// modified to remove exp9 svd bad events (following Sumisawa).
//
// Revision 1.4  2002/03/14 01:06:33  katayama
// added s_benergy_value
//
// Revision 1.3  2002/03/13 14:27:12  hitoshi
// changed the default not to call scale_error.
//
// Revision 1.2  2002/03/13 02:55:20  katayama
// First version
//
//

#include <b2bii/modules/B2BIIMdstInput/B2BIIFixMdstModule.h>

#include "belle_legacy/panther/panther.h"

#include <iostream>
#include <string>

#include "belle_legacy/ip/IpProfile.h"

#include "belle_legacy/benergy/BeamEnergy.h"

#include "belle_legacy/tables/mdst.h"
#include "belle_legacy/tables/belletdf.h"
#include "belle_legacy/tables/evtcls.h"
#include "belle_legacy/kid/dEdxCalib.h"
#include "belle_legacy/tables/level4.h"

namespace Belle2 {

  REG_MODULE(B2BIIFixMdst)
  /** Nomial beam energy */
#define NOMINAL_ENERGY 5.290024915

  double B2BIIFixMdstModule::s_benergy_value = NOMINAL_ENERGY;

  B2BIIFixMdstModule::B2BIIFixMdstModule() : Module()
  {

    setDescription("Used to fix the old Belle I mdst files before processing. This module was taken from the old Belle I code and converted into a basf2 module."
                   "There are a lot of magic numbers in this module, where do they come from? Noone knows..."
                   "Return value is -1 if something went wrong, do not use these events!");

    addParam("Scale_momenta", m_scale_momenta, "Apply scale_momenta", 1);
    addParam("scale_momenta_scale_data", m_scale_momenta_scale_data, "scale_momenta scale_data", 0.0f);
    addParam("scale_momenta_scale_mc", m_scale_momenta_scale_mc, "scale_momenta scale_mc", 1.0f);
    addParam("scale_momenta_mode", m_scale_momenta_mode, "scale_momenta_mode", 2);
    addParam("Scale_error", m_scale_error, "Apply scale_error", 1);

    addParam("Smear_trk", m_smear_trk, "Do extra-smearing for MC tracks", 2);
    addParam("Extra_trk_vee2", m_add_or_remove_extra_trk_vee2,
             "1: add to Mdst_charged etc, -1: remove (move to Mdst_*_extra), 0:do nothing", 0);

    addParam("Correct_ecl", m_correct_ecl, "Apply correct_ecl", 1);
    addParam("Correct_ecl_pv", m_correct_ecl_primary_vertex, "Apply correct_ecl_pv", 0);
    addParam("Correct_ecl_option", m_correct_ecl_option, "Set correct_ecl option", 0);
    addParam("Correct_ecl_version", m_correct_ecl_version, "Set correct_ecl version", 2);
    addParam("Correct_ecl_5s", m_correct_ecl_5s, "Correct_ecl flag for 5S runs", 0); // KM 20080608

    addParam("Make_pi0", m_make_pi0, "Apply make_pi0 (after correct_ecl)", 1);
    addParam("Make_pi0_pv", m_make_pi0_primary_vertex, "Apply make_pi0_pv (after correct_ecl)", 0);
    addParam("Make_pi0_option", m_make_pi0_option, "Set make_pi0 option", 0);
    addParam("Make_pi0_lower_limit", m_make_pi0_lower_limit, "Set make_pi0 lower mass limit", 0.0);
    addParam("Make_pi0_upper_limit", m_make_pi0_upper_limit, "Set make_pi0 upper mass limit", 0.0);

    addParam("Benergy", m_benergy, "Apply Benergy", 1);
    addParam("BenergyDB", m_benergy_db, "Apply Benergy from DB or file ", 0);
    addParam("BenergyMCDB", m_benergy_mcdb, "Apply Benergy from DB or default value in MC", 0);

    addParam("Good_event", m_good_event, "Select good event only", 1);
    addParam("HadronA", m_hadron_a_only, "Select Hadron A event only", 1);
    addParam("HadronB", m_hadron_b_only, "Select Hadron B event only", 1);
    addParam("Level4passed", m_l4passed_only, "Select level4-passed event only", 1);

    addParam("Scale_error_dataset", m_scale_error_dataset_name, "Set scale_error_dataset name", std::string(""));
    addParam("Calib_dedx", m_calib_dedx, "Do dedx calibration", 1);
    addParam("Shift_tof_mode", m_shift_tof_mode, "Do tof_correction", 1);
    addParam("Endcap_MX_layer", m_eklm_max_layer, "Max layer number of KLM endcap", 11);
    addParam("UseECL", m_use_ecl, "Use (1) or don't use (0) ECL in muid", 0);
    addParam("ExpNo", m_expno, "Use data's exp # (0) or this value (>0) in muid", 0);

    addParam("Table_size", m_table_size, "Check table sizes", 1);
    addParam("Limit_mdst_ecl_trk", m_limit_mdst_ecl_trk, "Limit of mdst_ecl_trk table size", 600);
    addParam("Limit_mdst_klm_cluster_hit", m_limit_mdst_klm_cluster_hit, "Limit of mdst_klm_cluster_hit table size", 450);

    m_reprocess_version = 0;
    addParam("Reprocess_version", m_reprocess_version_specified, "Specify reprocess version", -1);
    addParam("5Srun", m_5Srun, "5S,2S,1S run or not", 0);
    addParam("Check_version_consistency", m_check_version_consistency, "Check consistency betw. env. variable and mdst version", 1);
  }

  void B2BIIFixMdstModule::initialize()
  {
    Muid_init();
  }

  void B2BIIFixMdstModule::terminate()
  {
    Muid_term();
  }

  void B2BIIFixMdstModule::event()
  {

    if ((BsCouTab(BELLE_EVENT) == 0) || (BsCouTab(MDST_EVENT_ADD) == 0)) {
      B2INFO("Warning from B2BIIFixMdst: No Belle_event or Mdst_event_add table; no correction for event");
      setReturnValue(-1);
      return;
    }

    if (m_add_or_remove_extra_trk_vee2 == 1) {
      add_extra_trk_vee2();
    } else if (m_add_or_remove_extra_trk_vee2 == -1) {
      remove_extra_trk_vee2();
    }

    if (m_scale_momenta == 1) scale_momenta(m_scale_momenta_scale_data,
                                              m_scale_momenta_scale_mc,
                                              m_scale_momenta_mode);

    if (m_scale_error == 1) scale_error();

    if (m_smear_trk != 0) smear_trk();

    if (m_correct_ecl == 1) correct_ecl(m_correct_ecl_option, m_correct_ecl_version);

    HepPoint3D pv;
    CLHEP::HepSymMatrix dpv(3, 0);
    int err(1);
    if (m_correct_ecl_primary_vertex == 1) {
      err = set_primary_vertex(pv, dpv);
      if (err == 0)correct_ecl_primary_vertex(pv, dpv);
    }

    if (m_make_pi0 == 1 || m_make_pi0_primary_vertex == 1) {
      if (m_make_pi0_primary_vertex == 1 && err == 0) {
        make_pi0_primary_vertex(m_make_pi0_option, m_make_pi0_lower_limit, m_make_pi0_upper_limit, pv, dpv);
      } else {
        make_pi0(m_make_pi0_option, m_make_pi0_lower_limit, m_make_pi0_upper_limit);
      }
      fix_pi0_brecon();
    }

    if (m_benergy == 1) s_benergy_value = Benergy();

    if (m_good_event == 1) {
      if (good_event()) {
        setReturnValue(1);
      } else {
        setReturnValue(-1);
        B2DEBUG(99, "B2BIIFixMdst: Not a good event");
        return;
      }
    } else {
      setReturnValue(0);
    }

    if (m_l4passed_only == 1) {
      if (BsCouTab(L4_SUMMARY)) {
        struct l4_summary* l =
          (struct l4_summary*) BsGetEnt(L4_SUMMARY, 1, BBS_No_Index);
        if (l->m_type == 0) {
          setReturnValue(-1);
          B2DEBUG(99, "B2BIIFixMdst: L4 cut");
          return;
        }
      }
    }

    if (m_hadron_a_only == 1) {
      Belle::Evtcls_flag_Manager&   EvtFlagMgr  = Belle::Evtcls_flag_Manager::get_manager();
      Belle::Evtcls_flag_Manager::iterator  it1 = EvtFlagMgr.begin();
      if (it1 != EvtFlagMgr.end() && *it1) {
        if ((*it1).flag(0) < 10) {
          setReturnValue(-1);
          B2DEBUG(99, "B2BIIFixMdst: HadA cut");
          return;
        }
      }
    }

    if (m_hadron_b_only == 1) {
      Belle::Evtcls_hadronic_flag_Manager& HadMgr
        = Belle::Evtcls_hadronic_flag_Manager::get_manager();
      Belle::Evtcls_hadronic_flag_Manager::iterator ith = HadMgr.begin();
      if (ith != HadMgr.end() && *ith) {
        if ((*ith).hadronic_flag(2) <= 0) {
          setReturnValue(-1);
          B2DEBUG(99, "B2BIIFixMdst: HadB cut");
          return;
        }
      }
    }

    if (m_table_size == 1) {
      Belle::Belle_event_Manager& bevt = Belle::Belle_event_Manager::get_manager();
      //    if( bevt[0].ExpNo() >=39 ) {
      if ((m_reprocess_version == 0 && bevt.count() > 0 && bevt[0].ExpNo() >= 39) ||
          (m_reprocess_version >= 1)) {
        if (Belle::Mdst_ecl_trk_Manager::get_manager().count() > m_limit_mdst_ecl_trk) {
          setReturnValue(-1);
          B2INFO("B2BIIFixMdst: " <<  Belle::Mdst_ecl_trk_Manager::get_manager().count() << " " << m_limit_mdst_ecl_trk);
          return;
        }
        if (Belle::Mdst_klm_cluster_hit_Manager::get_manager().count()
            > m_limit_mdst_klm_cluster_hit) {
          setReturnValue(-1);
          B2INFO("B2BIIFixMdst: " <<  Belle::Mdst_klm_cluster_hit_Manager::get_manager().count() << " " << m_limit_mdst_klm_cluster_hit);
          return;
        }
      }
    }

    if (m_calib_dedx != 0) {
      Belle::Belle_event_Manager& bevt = Belle::Belle_event_Manager::get_manager();
      if (bevt[0].ExpMC() == 1 || m_reprocess_version == 0 || m_calib_dedx == -1) Belle::Calib_dEdx();
    }

    if (m_shift_tof_mode > 0) shift_tof(m_shift_tof_mode);

    if (m_mapped_expno > 0) Muid_event();

    return;
  }

  void B2BIIFixMdstModule::beginRun()
  {

    if (m_5Srun == 0 && m_correct_ecl_5s == 1) m_5Srun = 1;

    m_reprocess_version = get_reprocess_version();
    B2INFO("reprocess version= " << m_reprocess_version);

    Belle::IpProfile::begin_run();

    if (m_benergy_db == 0) Belle::BeamEnergy::begin_run();

    Belle::Belle_runhead_Manager& evtmgr = Belle::Belle_runhead_Manager::get_manager();
    Belle::Belle_runhead_Manager::const_iterator belleevt = evtmgr.begin();
    if (belleevt != evtmgr.end() && (*belleevt)) {
      int expmc = belleevt->ExpMC();
      int exp = belleevt->ExpNo();
      int run = belleevt->RunNo();
      //if(m_benergy==1 && expmc==1) s_benergy_value = Benergy(exp, run);

      if (((31 <= exp && exp <= 55) || (69 <= exp && exp <= 73)) && m_check_version_consistency == 1) {
        char* env = std::getenv("USE_GRAND_REPROCESS_DATA");
        if ((env && m_reprocess_version == 0) ||
            (!env && m_reprocess_version == 1)) {
          B2ERROR("Env. variable USE_GRAND_REPROCESS_DATA and input mdst reprocess version=" << m_reprocess_version <<
                  " (=0:old; =1:new) are inconsistent -> kill the job. Set USE_GRAND_REPROCESS_DATA or input mdst file properly and retry.");
          exit(-1);
        }
      }

      if (m_calib_dedx != 0) {
        if (expmc == 1 || m_reprocess_version == 0 || m_calib_dedx == -1)
          Belle::Calib_dEdx_begin_run(m_reprocess_version, m_5Srun);
      }

      if (m_mapped_expno > 0) Muid_begin_run(expmc, exp, run);

    } else {
      B2ERROR("[B2BIIFixMdst] Error: Cannot read \"Belle::Belle_RunHead\".");
    }
  }

  int
  B2BIIFixMdstModule::get_reprocess_version()
  {

    int version(0);
    Belle::Belle_version_Manager& bvmgr = Belle::Belle_version_Manager::get_manager();

    if (m_reprocess_version_specified >= 0) {
      //*** version set forcedly ***
      version = m_reprocess_version_specified;
      B2INFO("reprocess version forcedly set to " << version);

    } else {
      //*** version set based on info. in run-header ***
      Belle::Belle_processing_Manager& bpmgr = Belle::Belle_processing_Manager::get_manager();
      Belle::Belle_runhead_Manager& evtmgr = Belle::Belle_runhead_Manager::get_manager();
      Belle::Belle_runhead_Manager::const_iterator belleevt = evtmgr.begin();

      int expmc(0), exp(0), run(0);
      if (belleevt != evtmgr.end() && (*belleevt)) {
        expmc = belleevt->ExpMC();
        exp = belleevt->ExpNo();
        run = belleevt->RunNo();
      }

      int B_l_d = 0;
      //int Date  = 0;
      if (bpmgr.count() > 0) {
        int i = bpmgr.count() - 1;
        B_l_d = bpmgr[i].Belle_level_date();
        //Date  = bpmgr[i].Date();
      }

      if (bvmgr.count() > 0) {
        //Belle::Belle_version is in run-header: MC or (DATA with level > 20081107)
        //      int j = bvmgr.count() - 1;
        int j = 0;
        int chk = std::abs(bvmgr[j].SVD())
                  + std::abs(bvmgr[j].CDC())
                  + std::abs(bvmgr[j].ECL());
        if (chk == 0) {
          version = 0;
        } else {
          version = 1;
        }
      } else {
        //no Belle::Belle_version in run-header: DATA with level <= 20081107
        if (expmc == 2 || (expmc == 1
                           && B_l_d > 20081107))
          B2ERROR("No Belle::Belle_version table in run-header for MC or (Data processed with Belle_level > 20081107). Strange !");

        if (B_l_d == 20081107) {
          version = 1;
          //exception: U(1S) run w/old tracking & old ECL
          //    if( exp==65 && run >= 1000 && Date <= 20081130 ) version = 0;
          if (exp == 65 && run >= 1000) version = 0;
        } else {
          version = 0;
        }
      }
    }

    if (bvmgr.count() == 0) {
      Belle::Belle_version& bv = bvmgr.add();
      bv.SVD(version);
      bv.CDC(version);
      bv.ECL(version);
    } else if (bvmgr.count() > 0) {
      if (bvmgr[0].SVD() != version) {
        B2WARNING("Belle::Belle_version_SVD(=" << bvmgr[0].SVD() << ") is inconsistent with version(=" << version <<
                  "); replace it anyway");
        bvmgr[0].SVD(version);
      }
      if (bvmgr[0].CDC() != version) {
        B2WARNING("Belle::Belle_version_CDC(=" << bvmgr[0].CDC() << ") is inconsistent with version(=" << version <<
                  "); replace it anyway");
        bvmgr[0].CDC(version);
      }
      if (bvmgr[0].ECL() != version) {
        B2WARNING("Belle::Belle_version_ECL(=" << bvmgr[0].ECL() << ") is inconsistent with version(=" << version <<
                  "); replace it anyway");
        bvmgr[0].ECL(version);
      }
    }

    return version;
  }

  bool B2BIIFixMdstModule::good_event()
  {

    bool ret = true;

    Belle::Mdst_quality_Manager& qmgr = Belle::Mdst_quality_Manager::get_manager();
    if (0 == qmgr.count()) return ret; //do nothing if not exist

    for (std::vector<Belle::Mdst_quality>::iterator it = qmgr.begin();
         it != qmgr.end(); ++it) {
      Belle::Mdst_quality& q = *it;
      std::string c(q.name()());
      //    if((strcmp(c,"SVD ")==0) && (q.quality() & 0x02)) ret = false;
      //also to remove exp9 bad svd events
      if ((strcmp(c.c_str(), "SVD ") == 0) && (q.quality() & 0x8002)) ret = false;
    }
    return ret;
  }

//=====================================================================
//
//  Author:  T. Matsumoto
//  based on dcpvrare_ml:0127, interface is modified by M. Nakao
//
//  Modified by T.Shibata 2004.04.15
//            Benergy value get from Database
//

//#include <panther/panther.h>
//#include BELLETDF_H

  double B2BIIFixMdstModule::Benergy(int expnum, int runnum)
  {

    double energy(NOMINAL_ENERGY);
    double factor(0.001);

    //  if( m_benergy_db == 0 && BsCouTab(BELLE_EVENT) > 0 ){
    if (m_benergy_db == 0) {
      Belle::Belle_event_Manager& bevt_mgr = Belle::Belle_event_Manager::get_manager();
      Belle::Belle_event_Manager::const_iterator bevt = bevt_mgr.begin();
      if (bevt != bevt_mgr.end() && (*bevt)) {
        if (bevt->ExpMC() == 1) energy = Belle::BeamEnergy::E_beam2();
        if (bevt->ExpMC() == 2) {
          if (m_benergy_mcdb == 0) return energy;    //energy = 5.290000;
          if (m_benergy_mcdb != 0) energy = Belle::BeamEnergy::E_beam2();
        }
        return energy;
      }
    }

    if ((expnum == 0 || runnum == 0) && BsCouTab(BELLE_EVENT) > 0) {
      Belle::Belle_event& bevt = Belle::Belle_event_Manager::get_manager()[0];
      if (bevt.ExpMC() == 1) {
        expnum = bevt.ExpNo();
        runnum = bevt.RunNo();
      }
    }

    if (expnum == 5) {
      if (runnum >= 74 && runnum <= 472)      energy += -0.540 * factor;
    } else if (expnum == 7) {
      if (runnum >= 6 && runnum <= 475)      energy +=  0.305 * factor;
      else if (runnum >= 538 && runnum <= 895)  energy += -1.016 * factor;
      else if (runnum >= 904 && runnum <= 1079) energy += -1.065 * factor;
      else if (runnum >= 1101 && runnum <= 1248) energy += -0.608 * factor;
      else if (runnum >= 1257 && runnum <= 1399) energy += -1.132 * factor;
      else if (runnum >= 1400 && runnum <= 1599) energy += -1.446 * factor;
      else if (runnum >= 1600 && runnum <= 1799) energy += -1.836 * factor;
      else if (runnum >= 1800 && runnum <= 1999) energy += -1.829 * factor;
      else if (runnum >= 2000 && runnum <= 2156) energy += -1.732 * factor;
      else if (runnum >= 2294 && runnum <= 2599) energy += -2.318 * factor;
      else if (runnum >= 2600 && runnum <= 2865) energy += -1.363 * factor;
    } else if (expnum == 9) {
      if (runnum >= 17   && runnum <= 399)   energy += -2.767 * factor;
      else if (runnum >= 400  && runnum <= 499)   energy += -2.826 * factor;
      else if (runnum >= 500  && runnum <= 593)   energy += -2.311 * factor;
      else if (runnum >= 601  && runnum <= 699)   energy += -2.138 * factor;
      else if (runnum >= 700  && runnum <= 899)   energy += -2.267 * factor;
      else if (runnum >= 900  && runnum <= 999)   energy += -2.412 * factor;
      else if (runnum >= 1000 && runnum <= 1099)   energy += -2.229 * factor;
      else if (runnum >= 1100 && runnum <= 1220)   energy += -1.842 * factor;
      //new(2001/06/23)
    } else if (expnum == 11) {
      if (runnum >= 1   && runnum <= 161) energy += -2.173 * factor;
      if (runnum >= 261 && runnum <= 399) energy += -1.392 * factor;
      if (runnum >= 400 && runnum <= 499) energy += -1.321 * factor;
      if (runnum >= 500 && runnum <= 599) energy += -1.394 * factor;
      if (runnum >= 600 && runnum <= 699) energy += -1.827 * factor;
      if (runnum >= 700 && runnum <= 799) energy += -1.912 * factor;
      if (runnum >= 800 && runnum <= 899) energy += -1.603 * factor;
      if (runnum >= 900 && runnum <= 998) energy += -1.759 * factor;
      if (runnum >= 1001 && runnum <= 1099) energy += -1.915 * factor;
      if (runnum >= 1100 && runnum <= 1199) energy += -1.513 * factor;
      if (runnum >= 1200 && runnum <= 1287) energy += -0.639 * factor;
    } else if (expnum == 13) {
      if (runnum >= 1 && runnum <= 99) energy +=  0.279 * factor;
      if (runnum >= 100 && runnum <= 198) energy +=  0.140 * factor;
      if (runnum >= 200 && runnum <= 297) energy += -0.095 * factor;
      if (runnum >= 301 && runnum <= 440) energy +=  0.232 * factor;
      if (runnum >= 551  && runnum <= 699)  energy += -0.385 * factor;
      if (runnum >= 700  && runnum <= 799)  energy += -0.654 * factor;
      if (runnum >= 800  && runnum <= 899)  energy += -0.972 * factor;
      if (runnum >= 900  && runnum <= 999)  energy += -0.770 * factor;
      if (runnum >= 1000 && runnum <= 1099) energy += -0.353 * factor;
      if (runnum >= 1100 && runnum <= 1199) energy += -0.490 * factor;
      if (runnum >= 1200 && runnum <= 1299) energy += -0.687 * factor;
      if (runnum >= 1300 && runnum <= 1399) energy += -0.545 * factor;
      //if ( runnum >=1400 && runnum <=1523 ) energy += -0.783*factor;
      if (runnum >= 1400) energy += -0.783 * factor;
      // there isn't sufficient amount of data between 1589..1623 (0.1 fb-1),
      // so we decided to take the value of last run range, which should be
      // better than 0 correction.   (c.f. dcpvrare_ml:0146 and 0147)

    } else if (expnum == 15) {
      //    // new February 15, 2002
      //    if(runnum < 764 ) energy = 5.288103;
      //    if(runnum > 763 && runnum < 951 ) energy = 5.286505;
      //    if(runnum > 950 ) energy = 5.287404;
      // new February 20, 2002
      if (runnum <  764)                  energy = 5.288103;
      if (runnum >  763 && runnum <  951) energy = 5.287005;
      if (runnum >  950 && runnum < 1251) energy = 5.287804;
      if (runnum > 1250)                  energy = 5.288603;

    } else if (expnum == 17) {
      if (runnum <= 199)                energy = 5.288702;
      if (runnum >= 200 && runnum <= 399) energy = 5.289202;
      if (runnum >= 400 && runnum <= 485) energy = 5.289401;
      if (runnum >= 545 && runnum <= 799) energy = 5.289501;
      if (runnum >= 800 && runnum <= 937) energy = 5.289401;

    } else if (expnum == 19) {
      if (runnum >= 64  && runnum <= 199) energy = 5.2887023;
      if (runnum >= 200 && runnum <= 299) energy = 5.2886025;
      if (runnum >= 300 && runnum <= 399) energy = 5.28830299;
      if (runnum >= 400 && runnum <= 459) energy = 5.28820313;
      if (runnum >= 461 && runnum <= 599) energy = 5.28830299;
      if (runnum >= 600 && runnum <= 666) energy = 5.28780366;
      if (runnum >= 763 && runnum <= 999) energy = 5.28820313;
      if (runnum >= 1000 && runnum <= 1099) energy = 5.28780366;
      if (runnum >= 1100 && runnum <= 1199) energy = 5.28770378;
      if (runnum >= 1200 && runnum <= 1399) energy = 5.28750402;
      if (runnum >= 1400 && runnum <= 1499) energy = 5.28740413;
      if (runnum >= 1500 && runnum <= 1599) energy = 5.28720434;
      if (runnum >= 1600 && runnum <= 1643) energy = 5.28760390;


      // Dec 28th, 2002

    } else if (expnum == 21) {
      if (runnum >= 0 && runnum <= 180) energy = 5.28650496;
      if (runnum >= 181 && runnum <= 324) energy = 5.2880034;
    } else if (expnum == 23) {
      if (runnum >= 0 && runnum <= 238) energy = 5.28860254;
      if (runnum >= 357 && runnum <= 607) energy = 5.28880222;

      //May 31st, 2003

    } else if (expnum == 25) {
      if (runnum >= 0 && runnum <= 99) energy = 5.28950099;
      if (runnum >= 100 && runnum <= 199) energy = 5.28870238;
      if (runnum >= 200 && runnum <= 299) energy = 5.28910171;
      if (runnum >= 300 && runnum <= 399) energy = 5.28890205;
      if (runnum >= 400 && runnum <= 799) energy = 5.28850269;
      if (runnum >= 800 && runnum <= 899) energy = 5.28790353;
      if (runnum >= 900 && runnum <= 999) energy = 5.28810326;
      if (runnum >= 1000 && runnum <= 1099) energy = 5.28830298;
      if (runnum >= 1100 && runnum <= 1199) energy = 5.28820313;
      if (runnum >= 1200 && runnum <= 1299) energy = 5.28780366;
      if (runnum >= 1300 && runnum <= 1462) energy = 5.28820313;
      if (runnum >= 1600 && runnum <= 1699) energy = 5.28830299;
      if (runnum >= 1700 && runnum <= 1799) energy = 5.28780366;
      if (runnum >= 1800 && runnum <= 1899) energy = 5.28810327;
      if (runnum >= 1900 && runnum <= 2122) energy = 5.28830299;

      //June 30th , 2003

    } else if (expnum == 27) {
      if (runnum >= 0 && runnum <= 180) energy = 5.28860254;
      if (runnum >= 181 && runnum <= 288) energy = 5.28870238;
      if (runnum >= 367 && runnum <= 499) energy = 5.28880222;
      if (runnum >= 500 && runnum <= 590) energy = 5.28890206;
      if (runnum >= 591 && runnum <= 699) energy = 5.28910172;
      if (runnum >= 700 && runnum <= 1079) energy = 5.28900189;
      if (runnum >= 1252 && runnum <= 1299) energy = 5.28850269;
      if (runnum >= 1300 && runnum <= 1632) energy = 5.28870238;

      //March 9th , 2004

    } else if (expnum == 31) {
      if (runnum >=   0 && runnum <= 166) energy = 5.287404131;
      if (runnum >= 231 && runnum <= 470) energy = 5.287903534;
      if (runnum >= 471 && runnum <= 698) energy = 5.28810327;
      if (runnum >= 699 && runnum <= 804) energy = 5.288602541;
      if (runnum >= 805 && runnum <= 936) energy = 5.288702384;
      if (runnum >= 1071 && runnum <= 1440) energy = 5.288902;
      if (runnum >= 1441 && runnum <= 1715) energy = 5.289301;

    } else if (expnum >= 32) {
      B2ERROR("Warning: Benergy is not ready for exp >=32.");
    }
    return energy;
  }

} // namespace Belle
