//
// $Id: B2BIIFixMdst_trk.cc 11331 2011-07-25 11:58:12Z hitoshi $
//
// $Log$
//
// Revision 2.0 2015/03/11 tkeck
// Conversion to Belle II
//
// Revision 1.47  2006/05/29
// scale_error for exp. 45,47,49
//
// Revision 1.46  2005/07/28 09:55:00  hitoshi
// for exp43 run700-1149 (by Senyo)
//
// Revision 1.45  2005/07/02 08:07:48  hitoshi
// for exp43 run300-699 (by Senyo)
//
// Revision 1.44  2005/06/06 04:28:01  katayama
// New for exp. 43 run - 300 from Miyabayasi and Senyo san
//
// Revision 1.43  2005/05/14 03:47:34  hitoshi
// scale error for exp39 and 41 (by Kusaka)
//
// Revision 1.42  2005/04/17 09:43:56  hitoshi
// for Exp41 r1200-end (by Senyo)
//
// Revision 1.41  2005/04/15 05:49:28  hitoshi
// for Exp41 r800-1199 (by Senyo)
//
// Revision 1.40  2005/03/31 10:45:30  hitoshi
// for Exp41 runno<=799 (by Senyo)
//
// Revision 1.39  2005/03/08 06:10:02  hitoshi
// scale_momenta for Exp39 (by Senyo)
//
// Revision 1.38  2004/07/26 03:29:46  katayama
// scale_error for exp. 37
//
// Revision 1.37  2004/07/14 07:34:17  katayama
// added cassert
//
// Revision 1.36  2004/07/12 08:27:04  hitoshi
// scale error for svd2 (by kusaka)
//
// Revision 1.35  2004/07/12 05:25:06  hitoshi
// scale momenta for exp37 r1400-end (by senyo)
//
// Revision 1.34  2004/07/09 05:39:37  hitoshi
// protection for mis-use for exp>=30
//
// Revision 1.33  2004/07/09 04:36:07  hitoshi
// scale momenta for exp37 run1-1399 (by senyo)
//
// Revision 1.32  2004/06/23 19:06:55  hitoshi
// scale momenta for exp31 (by Senyo)
//
// Revision 1.31  2004/06/16 06:48:36  hitoshi
// scale momenta for exp35 (by Senyo san)
//
// Revision 1.30  2004/06/06 03:20:47  hitoshi
// scale momenta for exp33 (by senyo)
//
// Revision 1.29  2004/04/25 16:33:33  hitoshi
// modified to be able to handle output from low p finder (by kakuno). modified
// Benergy to fetch data from DB (by Shibata).
//
// Revision 1.28  2004/03/09 04:46:00  hitoshi
// scale momenta for exp31 run1100-end by Senyo
//
// Revision 1.27  2004/03/01 03:12:42  hitoshi
// scale_momenta for exp31 upto run1099 (by Senyo).
//
// Revision 1.26  2003/07/31 07:47:58  katayama
// New version from Higuhi san
//
// Revision 1.25  2003/07/11 08:41:23  hitoshi
// fixed a bug.
//
// Revision 1.24  2003/07/11 06:41:22  katayama
// for exp. 27 continuum
//
// Revision 1.23  2003/07/10 13:45:38  hitoshi
// update for exp27 run700- (by Sanjay and Senyo).
//
// Revision 1.22  2003/06/30 22:16:29  hitoshi
// update for exp27 (by Miyabayashi, Sanjay, Senyo).
//
// Revision 1.21  2003/06/24 07:55:17  katayama
// Remived unused parameters
//
// Revision 1.20  2003/06/24 07:38:25  katayama
// added cstdio
//
// Revision 1.19  2003/06/18 12:22:02  katayama
// cannot compile with gcc3
//
// Revision 1.18  2003/06/12 13:25:43  hitoshi
// scale_error implemented last year (by Higuchi; valid up to exp19) is incorporated
//
// Revision 1.17  2003/05/31 09:22:54  hitoshi
// updae for exp25 (ny Miyabayashi and Senyo).
//
// Revision 1.16  2002/12/26 13:35:01  katayama
// scale_momenta for exp. 21 and 23 from Senyo san
//
// Revision 1.15  2002/10/12 15:33:07  hitoshi
// A bug fix: 1.46 -> 14.6kG (pointed out by N.root). Negligible effect on V0 mass.
//
// Revision 1.14  2002/07/04 21:06:44  katayama
// Final version
//
// Revision 1.13  2002/07/03 04:50:54  hitoshi
// scale_momenta for exp19 run900 - 1499 (by Senyo san).
//
// Revision 1.12  2002/06/22 09:40:59  katayama
// From Senyo san, for exp. 9 and exp.19 899
//
// Revision 1.11  2002/06/19 12:10:36  katayama
// New for exp. 7 from Senyo and Miyabayashi sans
//
// Revision 1.10  2002/06/13 00:40:08  hitoshi
// added scale momenta factors (from Senyo) for run1000 - end.
//
// Revision 1.9  2002/06/07 11:54:42  hitoshi
// added scale_momenta for run 1 - 999 in e11.
//
// Revision 1.8  2002/06/04 06:36:34  hitoshi
// added scale momenta for e19 runs300-459 (by Senyo).
//
// Revision 1.7  2002/05/29 07:15:14  hitoshi
// added scale momnta factors for exp19 run1-280.
//
// Revision 1.6  2002/05/24 13:27:45  hitoshi
// added scale_momenta for exp13 reprocessed data (by Senyo san).
//
// Revision 1.5  2002/05/08 12:12:54  hitoshi
// added scale_momenta factors for exp17 (by Senyo san).
//
// Revision 1.4  2002/04/23 15:11:55  hitoshi
// new scale factors for e15 mdst reprocessed with 20020405.
//
// Revision 1.3  2002/04/19 07:24:39  katayama
// Added pi0/vertex, calibdedx
//
// Revision 1.2  2002/04/05 01:19:33  katayama
// templates for ecl_primary_vertex
//
// Revision 1.1  2002/03/13 02:55:20  katayama
// First version
//
//

#include <b2bii/modules/B2BIIMdstInput/B2BIIFixMdstModule.h>

#include <cstdio>
#include <cmath>
#include <cfloat>
#include <cassert>

#include "CLHEP/Vector/ThreeVector.h"
#include "CLHEP/Matrix/Vector.h"
#include <TRandom.h>

#include "belle_legacy/helix/Helix.h"

#include "belle_legacy/tables/mdst.h"
#include "belle_legacy/tables/belletdf.h"
#include "belle_legacy/tables/evtvtx.h"
#include "belle_legacy/ip/IpProfile.h"

// kfitter
#include <analysis/KFit/VertexFitKFit.h>

namespace Belle2 {


//==========================================================
  double B2BIIFixMdstModule::vee_mass_nofit(const Belle::Mdst_vee2& vee2, float scale)
  {
//==========================================================
// Calculates V^0 mass with non-constraint fit results.
//==========================================================

    HepPoint3D vtx(vee2.vx(), vee2.vy(), vee2.vz());

    const double     pion_mass = 0.13956995;
    const double   proton_mass = 0.93827231;
    const double electron_mass = 0.000511;

    double massp(0.0), massm(0.0);
    if (1 == vee2.kind()) {
      massp = pion_mass;
      massm = pion_mass;
    } else if (2 == vee2.kind()) {
      massp = proton_mass;
      massm = pion_mass;
    } else if (3 == vee2.kind()) {
      massp = pion_mass;
      massm = proton_mass;
    } else if (4 == vee2.kind()) {
      massp = electron_mass;
      massm = electron_mass;
    }

//Calculate mass
//in case where decay vtx is outside of beam pipe
    //  if(BsCouTab(MDST_VEE_DAUGHTERS)) {
    if (vee2.daut()) {

      Belle::Mdst_vee_daughters& vdau = vee2.daut();

      CLHEP::HepVector a(5);
      a[0] = vdau.helix_p(0); a[1] = vdau.helix_p(1);
      a[2] = vdau.helix_p(2) / scale;
      a[3] = vdau.helix_p(3); a[4] = vdau.helix_p(4);
      Belle::Helix  helixp(vtx, a);

      a[0] = vdau.helix_m(0); a[1] = vdau.helix_m(1);
      a[2] = vdau.helix_m(2) / scale;
      a[3] = vdau.helix_m(3); a[4] = vdau.helix_m(4);
      Belle::Helix  helixm(vtx, a);
      // dout(Debugout::DDEBUG,"B2BIIFixMdst_trk") << "outside bp" << std::endl;
      CLHEP::HepLorentzVector v0 = helixp.momentum(0., massp) + helixm.momentum(0., massm);
      return v0.mag();

//in case where decay vtx is inside of beam pipe
    } else {

      Belle::Mdst_charged& chargedp = vee2.chgd(0);
      Belle::Mdst_trk&      trkp     = chargedp.trk();
      Belle::Mdst_trk_fit& trkfp    = trkp.mhyp(2);
      HepPoint3D pvtp(trkfp.pivot_x(), trkfp.pivot_y(), trkfp.pivot_z());
      CLHEP::HepVector a(5);
      a[0] = trkfp.helix(0); a[1] = trkfp.helix(1);
      a[2] = trkfp.helix(2);
      a[3] = trkfp.helix(3); a[4] = trkfp.helix(4);
      Belle::Helix  helixp(pvtp, a);
      helixp.bFieldZ(14.6);
      helixp.pivot(vtx);
      a = helixp.a();
      a[2] = a[2] / scale;
      helixp.a(a);

      Belle::Mdst_charged& chargedm = vee2.chgd(1);
      Belle::Mdst_trk&      trkm     = chargedm.trk();
      Belle::Mdst_trk_fit& trkfm    = trkm.mhyp(2);
      HepPoint3D pvtm(trkfm.pivot_x(), trkfm.pivot_y(), trkfm.pivot_z());
      a[0] = trkfm.helix(0); a[1] = trkfm.helix(1);
      a[2] = trkfm.helix(2);
      a[3] = trkfm.helix(3); a[4] = trkfm.helix(4);
      Belle::Helix  helixm(pvtm, a);
      helixm.bFieldZ(14.6);
      helixm.pivot(vtx);
      a = helixm.a();
      a[2] = a[2] / scale;
      helixm.a(a);
      // dout(Debugout::DDEBUG,"B2BIIFixMdst_trk") << "inside bp" << std::endl;
      CLHEP::HepLorentzVector v0 = helixp.momentum(0., massp) + helixm.momentum(0., massm);
      return v0.mag();

    }

  }

//===============================================================
  void B2BIIFixMdstModule::scale_momenta(float scale_data, float scale_mc, int mode)
  {
//===============================================================
// Scales charged track momenta, track params. and error
// matrices.
//======================================================

    // dout(Debugout::DDEBUG,"B2BIIFixMdst_trk") << "sacel momenta is called" << std::endl;

//Check existence of belle_event
    Belle::Belle_event_Manager& evtmgr = Belle::Belle_event_Manager::get_manager();
    // dout(Debugout::DDEBUG,"B2BIIFixMdst_trk") << evtmgr.count() << std::endl;
    if (0 == evtmgr.count()) return; //do nothing if not exist
    // dout(Debugout::DDEBUG,"B2BIIFixMdst_trk") << "after check" << std::endl;

    Belle::Mdst_event_add_Manager& mevtmgr = Belle::Mdst_event_add_Manager::get_manager();
    if (mevtmgr.count() > 0 && 1 == mevtmgr[0].flag_scale()) return; //do nothing if already scaled

//check mode
    if (mode < 0 || mode > 2) {
      B2ERROR("scale_momenta: invalid mode specified;");
      B2ERROR("mode must be 0, 1, 2");
      return;
    }

//set scale factor
    int expmc = evtmgr[0].ExpMC();
    int expno = evtmgr[0].ExpNo();
    int runno = evtmgr[0].RunNo();

    double scale = 1.;
    if (1 == expmc) {
      if (mode == 0) {
        scale = scale_data;
      } else if (mode >= 1) {
        //      scale_momenta_set( mode, expno, runno, scale);
        if (m_reprocess_version == 0) {
          scale_momenta_set_v1(mode, expno, runno, scale);
        } else if (m_reprocess_version == 1) {
          scale_momenta_set_v2(mode, expno, runno, scale);
        }
      }
    } else if (2 == expmc) {
      scale = scale_mc;
    }

    if (1. == scale) return; //do nothing if scale=1
    // dout(Debugout::DDEBUG,"B2BIIFixMdst_trk") << scale << std::endl;

//Turn on flag, save scale factor
    if (mevtmgr.count() > 0) {
      mevtmgr[0].flag_scale(1);
      mevtmgr[0].scale(scale);
    } else {
      Belle::Mdst_event_add& meadd = mevtmgr.add();
      meadd.flag_scale(1);
      meadd.scale(scale);
    }
    // dout(Debugout::DDEBUG,"B2BIIFixMdst_trk") << mevtmgr[0].flag_scale() << std::endl;
    // dout(Debugout::DDEBUG,"B2BIIFixMdst_trk") << mevtmgr[0].scale() << std::endl;


//Scale momenta in mdst_charged
    Belle::Mdst_charged_Manager& chgmgr = Belle::Mdst_charged_Manager::get_manager();
    for (std::vector<Belle::Mdst_charged>::iterator it = chgmgr.begin();
         it != chgmgr.end(); ++it) {
      Belle::Mdst_charged& charged = *it;
      charged.px(scale * charged.px());
      charged.py(scale * charged.py());
      charged.pz(scale * charged.pz());
    }

//Scale momenta in mdst_vee2
    Belle::Mdst_vee2_Manager& vee2mgr = Belle::Mdst_vee2_Manager::get_manager();
    for (std::vector<Belle::Mdst_vee2>::iterator it = vee2mgr.begin();
         it != vee2mgr.end(); ++it) {
      Belle::Mdst_vee2& vee2 = *it;
      double v0mass_scale = vee_mass_nofit(vee2, scale);
      double v0mass_noscl = vee_mass_nofit(vee2);
      double esq_scale =
        (v0mass_scale - v0mass_noscl) * (v0mass_scale + v0mass_noscl)
        +  vee2.energy() * vee2.energy() +
        (scale - 1.) * (scale + 1.) * (vee2.px() * vee2.px() +
                                       vee2.py() * vee2.py() +
                                       vee2.pz() * vee2.pz());
      vee2.energy(std::sqrt(esq_scale));
      vee2.px(scale * vee2.px());
      vee2.py(scale * vee2.py());
      vee2.pz(scale * vee2.pz());
    }

//Scale error matrices in mdst_trk_fit
    double scalei = 1. / scale;
    double scalei2 = scalei / scale;

    Belle::Mdst_trk_fit_Manager& trkfmgr = Belle::Mdst_trk_fit_Manager::get_manager();
    for (std::vector<Belle::Mdst_trk_fit>::iterator it = trkfmgr.begin();
         it != trkfmgr.end(); ++it) {
      Belle::Mdst_trk_fit& trkf = *it;
      trkf.helix(2, scalei * trkf.helix(2));
      trkf.error(3, scalei * trkf.error(3));
      trkf.error(4, scalei * trkf.error(4));
      trkf.error(5, scalei2 * trkf.error(5));
      trkf.error(8, scalei * trkf.error(8));
      trkf.error(12, scalei * trkf.error(12));
    }

//Scale error matrices in mdst_daughters
    Belle::Mdst_vee_daughters_Manager& vdaumgr = Belle::Mdst_vee_daughters_Manager::get_manager();
    for (std::vector<Belle::Mdst_vee_daughters>::iterator it = vdaumgr.begin();
         it != vdaumgr.end(); ++it) {
      Belle::Mdst_vee_daughters& vdau = *it;
      vdau.helix_p(2, scalei * vdau.helix_p(2));
      vdau.error_p(3, scalei * vdau.error_p(3));
      vdau.error_p(4, scalei * vdau.error_p(4));
      vdau.error_p(5, scalei2 * vdau.error_p(5));
      vdau.error_p(8, scalei * vdau.error_p(8));
      vdau.error_p(12, scalei * vdau.error_p(12));

      vdau.helix_m(2, scalei * vdau.helix_m(2));
      vdau.error_m(3, scalei * vdau.error_m(3));
      vdau.error_m(4, scalei * vdau.error_m(4));
      vdau.error_m(5, scalei2 * vdau.error_m(5));
      vdau.error_m(8, scalei * vdau.error_m(8));
      vdau.error_m(12, scalei * vdau.error_m(12));
    }
  }

//=====================================================================
  void B2BIIFixMdstModule::scale_momenta_set(const int mode, const int expno,
                                             const int runno, double& scale)
  {
//=====================================================================

    scale = 1.;

    if (mode == 1) {
      //factors for 2001 summer confs. analyses
      if (expno == 7) {scale = 1.001932; return;}
      if (expno == 9) {scale = 1.001413; return;}
      if (expno == 11) {scale = 1.001366; return;}
      //    if(expno==13) {scale = 1.000909; return;} //for '01 summer conf. papers; using up to run400
      if (expno == 13) {scale = 1.001460; return;} //updated factor using up to run1599
      if (expno >= 15) {B2ERROR("scale_momenta mode=1: not ready for exp >=15");}
    }

    if (mode == 2) {
      if (expno == 7) {
        if (runno >=   0 && runno <=  99) {scale = 1.00262126; return;}
        if (runno >= 100 && runno <= 199) {scale = 1.00253537; return;}
        if (runno >= 200 && runno <= 299) {scale = 1.00246555; return;}
        if (runno >= 300 && runno <= 399) {scale = 1.0026452 ; return;}
        if (runno >= 400 && runno <= 499) {scale = 1.00290386; return;}
        if (runno >= 500 && runno <= 599) {scale = 1.00287753; return;}
        if (runno >= 600 && runno <= 699) {scale = 1.00266845; return;}
        if (runno >= 700 && runno <= 799) {scale = 1.00184472; return;}
        if (runno >= 800 && runno <= 899) {scale = 1.00200021; return;}
        if (runno >= 900 && runno <= 999) {scale = 1.00178485; return;}
        if (runno >= 1000 && runno <= 1099) {scale = 1.00196071; return;}
        if (runno >= 1100 && runno <= 1199) {scale = 1.00224453; return;}
        if (runno >= 1200 && runno <= 1299) {scale = 1.00209138; return;}
        if (runno >= 1300 && runno <= 1399) {scale = 1.00145455; return;}
        if (runno >= 1400 && runno <= 1499) {scale = 1.00119243; return;}
        if (runno >= 1500 && runno <= 1599) {scale = 1.00115572; return;}
        if (runno >= 1600 && runno <= 1699) {scale = 1.00133916; return;}
        if (runno >= 1700 && runno <= 1799) {scale = 1.00186441; return;}
        if (runno >= 1800 && runno <= 1899) {scale = 1.00206786; return;}
        if (runno >= 1900 && runno <= 1999) {scale = 1.00184794; return;}
        if (runno >= 2000 && runno <= 2099) {scale = 1.00187219; return;}
        if (runno >= 2100 && runno <= 2199) {scale = 1.00214844; return;}
        if (runno >= 2200 && runno <= 2299) {scale = 1.00154502; return;}
        if (runno >= 2300 && runno <= 2399) {scale = 1.00179518; return;}
        if (runno >= 2400 && runno <= 2499) {scale = 1.00198983; return;}
        if (runno >= 2500 && runno <= 2599) {scale = 1.00219242; return;}
        if (runno >= 2600 && runno <= 2699) {scale = 1.00227363; return;}
        if (runno >= 2700 && runno <= 2799) {scale = 1.00202328; return;}
        if (runno >= 2800 && runno <= 2899) {scale = 1.00193147; return;}
      } else if (expno == 9) {
        if (runno >=   0 && runno <=  99) {scale = 1.00126754; return;}
        if (runno >= 100 && runno <= 199) {scale = 1.00119683; return;}
        if (runno >= 200 && runno <= 299) {scale = 1.00123771; return;}
        if (runno >= 300 && runno <= 399) {scale = 1.00142625; return;}
        if (runno >= 400 && runno <= 499) {scale = 1.00134481; return;}
        if (runno >= 500 && runno <= 599) {scale = 1.00131973; return;}
        if (runno >= 600 && runno <= 699) {scale = 1.00145963; return;}
        if (runno >= 700 && runno <= 799) {scale = 1.00161304; return;}
        if (runno >= 800 && runno <= 899) {scale = 1.00141795; return;}
        if (runno >= 900 && runno <= 999) {scale = 1.00137157; return;}
        if (runno >= 1000 && runno <= 1099) {scale = 1.00126209; return;}
        if (runno >= 1100 && runno <= 1199) {scale = 1.00147941; return;}
        if (runno >= 1200 && runno <= 1299) {scale = 1.00145555; return;}
      } else if (expno == 11) {
        if (runno >=   0 && runno <=  99) {scale = 1.00149576; return;}
        if (runno >= 100 && runno <= 199) {scale = 1.00155153; return;}
        if (runno >= 200 && runno <= 299) {scale = 1.0013597 ; return;}
        if (runno >= 300 && runno <= 399) {scale = 1.00147504; return;}
        if (runno >= 400 && runno <= 499) {scale = 1.00138242; return;}
        if (runno >= 500 && runno <= 599) {scale = 1.00144554; return;}
        if (runno >= 600 && runno <= 699) {scale = 1.00136961; return;}
        if (runno >= 700 && runno <= 799) {scale = 1.00141443; return;}
        if (runno >= 800 && runno <= 899) {scale = 1.00136617; return;}
        if (runno >= 900 && runno <= 999) {scale = 1.00126262; return;}
        if (runno >= 1000 && runno <= 1099) {scale = 1.00133337; return;}
        if (runno >= 1100 && runno <= 1199) {scale = 1.00137906; return;}
        if (runno >= 1200 && runno <= 1299) {scale = 1.00127944; return;}
        //assume same factor for runno>=1300
        if (runno >= 1300 && runno <= 1399) {scale = 1.00127944; return;}
      } else if (expno == 13) {
        if (runno >=   0 && runno <=  99) {scale = 1.00110691; return;}
        if (runno >= 100 && runno <= 199) {scale = 1.00106123; return;}
        if (runno >= 200 && runno <= 299) {scale = 1.00109934; return;}
        if (runno >= 300 && runno <= 399) {scale = 1.00105759; return;}
        if (runno >= 400 && runno <= 499) {scale = 1.000986887; return;}
        if (runno >= 500 && runno <= 599) {scale = 1.000928764; return;}
        if (runno >= 600 && runno <= 699) {scale = 1.00103925; return;}
        if (runno >= 700 && runno <= 799) {scale = 1.0010591 ; return;}
        if (runno >= 800 && runno <= 899) {scale = 1.00127043; return;}
        if (runno >= 900 && runno <= 999) {scale = 1.00154033; return;}
        if (runno >= 1000 && runno <= 1099) {scale = 1.00180656; return;}
        if (runno >= 1100 && runno <= 1199) {scale = 1.00202059; return;}
        if (runno >= 1200 && runno <= 1299) {scale = 1.00184308; return;}
        if (runno >= 1300 && runno <= 1399) {scale = 1.00180153; return;}
        if (runno >= 1400 && runno <= 1499) {scale = 1.00189577; return;}
        if (runno >= 1500 && runno <= 1599) {scale = 1.00176026; return;}
        //assume same factor for runno>=1600
        if (runno >= 1600 && runno <= 1699) {scale = 1.00176026; return;}

      } else if (expno == 15) {
        if (runno >=   0 && runno <=  99) {scale = 1.00178719; return;}
        if (runno >= 100 && runno <= 199) {scale = 1.00205712; return;}
        if (runno >= 200 && runno <= 299) {scale = 1.00197622; return;}
        if (runno >= 300 && runno <= 399) {scale = 1.00191558; return;}
        if (runno >= 400 && runno <= 499) {scale = 1.00207795; return;}
        if (runno >= 500 && runno <= 599) {scale = 1.00191871; return;}
        if (runno >= 600 && runno <= 699) {scale = 1.00187917; return;}
        if (runno >= 700 && runno <= 799) {scale = 1.0019573 ; return;}
        if (runno >= 800 && runno <= 899) {scale = 1.00196562; return;}
        if (runno >= 900 && runno <= 999) {scale = 1.00205215; return;}
        if (runno >= 1000 && runno <= 1099) {scale = 1.00200559; return;}
        if (runno >= 1100 && runno <= 1199) {scale = 1.00198568; return;}
        if (runno >= 1200 && runno <= 1299) {scale = 1.00212539; return;}
        if (runno >= 1300 && runno <= 1399) {scale = 1.00199158; return;}
        if (runno >= 1400 && runno <= 1499) {scale = 1.00208257; return;}

      } else if (expno >= 17) { B2ERROR("scale_momenta mode=2: not ready for exp >=17");}

    }
    return;
  }

//=====================================================================
  void B2BIIFixMdstModule::scale_momenta_set_v1(const int mode, const int expno,
                                                const int runno, double& scale)
  {
//=====================================================================

    //for e15 mdst processed with b20020405

    scale = 1.;

    if (mode == 1) {
      B2ERROR("scale_momenta mode=1 not ready for exp " << expno);
      return;
    }

    if (mode == 2) {
      if (expno == 7) {
        if (runno >=   0 && runno <=  99) {scale = 1.00284313; return;}
        if (runno >= 100 && runno <= 199) {scale = 1.00272563; return;}
        if (runno >= 200 && runno <= 299) {scale = 1.00276953; return;}
        if (runno >= 300 && runno <= 399) {scale = 1.00286722; return;}
        if (runno >= 400 && runno <= 499) {scale = 1.00318492; return;}
        if (runno >= 500 && runno <= 599) {scale = 1.00304208; return;}
        if (runno >= 600 && runno <= 699) {scale = 1.00292413; return;}
        if (runno >= 700 && runno <= 799) {scale = 1.00201286; return;}
        if (runno >= 800 && runno <= 899) {scale = 1.00225237; return;}
        if (runno >= 900 && runno <= 999) {scale = 1.00210375; return;}
        if (runno >= 1000 && runno <= 1099) {scale = 1.00213222; return;}
        if (runno >= 1100 && runno <= 1199) {scale = 1.00253683; return;}
        if (runno >= 1200 && runno <= 1299) {scale = 1.00234023; return;}
        if (runno >= 1300 && runno <= 1399) {scale = 1.00168421; return;}
        if (runno >= 1400 && runno <= 1499) {scale = 1.00136169; return;}
        if (runno >= 1500 && runno <= 1599) {scale = 1.00138431; return;}
        if (runno >= 1600 && runno <= 1699) {scale = 1.00154521; return;}
        if (runno >= 1700 && runno <= 1799) {scale = 1.00204405; return;}
        if (runno >= 1800 && runno <= 1899) {scale = 1.00223498; return;}
        if (runno >= 1900 && runno <= 1999) {scale = 1.00198975; return;}
        if (runno >= 2000 && runno <= 2099) {scale = 1.00201868; return;}
        if (runno >= 2100 && runno <= 2199) {scale = 1.00233875; return;}
        if (runno >= 2200 && runno <= 2299) {scale = 1.00175269; return;}
        if (runno >= 2300 && runno <= 2399) {scale = 1.00192858; return;}
        if (runno >= 2400 && runno <= 2499) {scale = 1.00217693; return;}
        if (runno >= 2500 && runno <= 2599) {scale = 1.00209337; return;}
        if (runno >= 2600 && runno <= 2699) {scale = 1.00244277; return;}
        if (runno >= 2700 && runno <= 2799) {scale = 1.00221485; return;}
        if (runno >= 2800 && runno <= 2899) {scale = 1.00214678; return;}
      } else if (expno == 9) {
        if (runno >=   0 && runno <=  99) {scale = 1.00127702; return;}
        if (runno >= 100 && runno <= 199) {scale = 1.00119957; return;}
        if (runno >= 200 && runno <= 299) {scale = 1.00132677; return;}
        if (runno >= 300 && runno <= 399) {scale = 1.0014785 ; return;}
        if (runno >= 400 && runno <= 499) {scale = 1.00144872; return;}
        if (runno >= 500 && runno <= 599) {scale = 1.00145376; return;}
        if (runno >= 600 && runno <= 699) {scale = 1.00151659; return;}
        if (runno >= 700 && runno <= 799) {scale = 1.00167384; return;}
        if (runno >= 800 && runno <= 899) {scale = 1.00153754; return;}
        if (runno >= 900 && runno <= 999) {scale = 1.0014984 ; return;}
        if (runno >= 1000 && runno <= 1099) {scale = 1.0013764 ; return;}
        if (runno >= 1100 && runno <= 1199) {scale = 1.00145545; return;}
        if (runno >= 1200 && runno <= 1299) {scale = 1.0017164 ; return;}
      } else if (expno == 11) {
        if (runno >=   0 && runno <=  99) {scale = 1.00159624; return;}
        if (runno >= 100 && runno <= 199) {scale = 1.00153365; return;}
        if (runno >= 200 && runno <= 299) {scale = 1.00143809; return;}
        if (runno >= 300 && runno <= 399) {scale = 1.00148495; return;}
        if (runno >= 400 && runno <= 499) {scale = 1.00143193; return;}
        if (runno >= 500 && runno <= 599) {scale = 1.00141905; return;}
        if (runno >= 600 && runno <= 699) {scale = 1.0013992 ; return;}
        if (runno >= 700 && runno <= 799) {scale = 1.00148836; return;}
        if (runno >= 800 && runno <= 899) {scale = 1.00139019; return;}
        if (runno >= 900 && runno <= 999) {scale = 1.00151389; return;}
        if (runno >= 1000 && runno <= 1099) {scale = 1.00142988; return;}
        if (runno >= 1100 && runno <= 1199) {scale = 1.00143519; return;}
        if (runno >= 1200 && runno <= 1299) {scale = 1.00132641; return;}
      } else if (expno == 13) {
        if (runno >=   0 && runno <=  99) {scale = 1.00126317; return;}
        if (runno >= 100 && runno <= 199) {scale = 1.00118837; return;}
        if (runno >= 200 && runno <= 299) {scale = 1.00122276; return;}
        if (runno >= 300 && runno <= 399) {scale = 1.00113233; return;}
        if (runno >= 400 && runno <= 499) {scale = 1.00111677; return;}
        if (runno >= 500 && runno <= 599) {scale = 1.00100963; return;}
        if (runno >= 600 && runno <= 699) {scale = 1.0011474 ; return;}
        if (runno >= 700 && runno <= 799) {scale = 1.00116245; return;}
        if (runno >= 800 && runno <= 899) {scale = 1.00139826; return;}
        if (runno >= 900 && runno <= 999) {scale = 1.0017014 ; return;}
        if (runno >= 1000 && runno <= 1099) {scale = 1.00190706; return;}
        if (runno >= 1100 && runno <= 1199) {scale = 1.00214688; return;}
        if (runno >= 1200 && runno <= 1299) {scale = 1.00207336; return;}
        if (runno >= 1300 && runno <= 1399) {scale = 1.00192885; return;}
        if (runno >= 1400 && runno <= 1499) {scale = 1.00196289; return;}
        if (runno >= 1500 && runno <= 1599) {scale = 1.00185017; return;}
        if (runno >= 1600 && runno <= 1699) {scale = 1.00191256; return;}
      } else if (expno == 15) {
        if (runno >=   0 && runno <=  99) {scale = 1.001953;   return;}
        if (runno >= 100 && runno <= 199) {scale = 1.00210508; return;}
        if (runno >= 200 && runno <= 299) {scale = 1.002139;   return;}
        if (runno >= 300 && runno <= 399) {scale = 1.00207803; return;}
        if (runno >= 400 && runno <= 499) {scale = 1.0022331;  return;}
        if (runno >= 500 && runno <= 599) {scale = 1.00206421; return;}
        if (runno >= 600 && runno <= 699) {scale = 1.00205153; return;}
        if (runno >= 700 && runno <= 799) {scale = 1.00214664; return;}
        if (runno >= 800 && runno <= 899) {scale = 1.0021456;  return;}
        if (runno >= 900 && runno <= 999) {scale = 1.00221055; return;}
        if (runno >= 1000 && runno <= 1099) {scale = 1.00216472; return;}
        if (runno >= 1100 && runno <= 1199) {scale = 1.00214208; return;}
        if (runno >= 1200 && runno <= 1299) {scale = 1.00228874; return;}
        if (runno >= 1300 && runno <= 1399) {scale = 1.00215047; return;}
        if (runno >= 1400 && runno <= 1499) {scale = 1.00219808; return;}
      } else if (expno == 17) {
        if (runno >=   0 && runno <=  99) {scale = 1.00216742; return;}
        if (runno >= 100 && runno <= 199) {scale = 1.00229433; return;}
        if (runno >= 200 && runno <= 299) {scale = 1.0022749 ; return;}
        if (runno >= 300 && runno <= 399) {scale = 1.00231283; return;}
        if (runno >= 400 && runno <= 499) {scale = 1.00223654; return;}
        if (runno >= 500 && runno <= 599) {scale = 1.00236524; return;}
        if (runno >= 600 && runno <= 699) {scale = 1.00229312; return;}
        if (runno >= 700 && runno <= 799) {scale = 1.002262  ; return;}
        if (runno >= 800 && runno <= 899) {scale = 1.00220104; return;}
        if (runno >= 900 && runno <= 999) {scale = 1.00215475; return;}
      } else if (expno == 19) {
        if (runno >=   0 && runno <=  99) {scale = 1.00227682; return;}
        if (runno >= 100 && runno <= 199) {scale = 1.00235795; return;}
        if (runno >= 200 && runno <= 299) {scale = 1.00251509; return;}
        if (runno >= 300 && runno <= 399) {scale = 1.0022106 ; return;}
        if (runno >= 400 && runno <= 499) {scale = 1.00230963; return;}
        if (runno >= 500 && runno <= 599) {scale = 1.002229  ; return;}
        if (runno >= 600 && runno <= 699) {scale = 1.00215402; return;}
        if (runno >= 700 && runno <= 799) {scale = 1.00217892; return;}
        if (runno >= 800 && runno <= 899) {scale = 1.0022305 ; return;}
        if (runno >= 900 && runno <= 999) {scale = 1.0021458 ; return;}
        if (runno >= 1000 && runno <= 1099) {scale = 1.00215792; return;}
        if (runno >= 1100 && runno <= 1199) {scale = 1.0022667 ; return;}
        if (runno >= 1200 && runno <= 1299) {scale = 1.00219581; return;}
        if (runno >= 1300 && runno <= 1399) {scale = 1.00218378; return;}
        if (runno >= 1400 && runno <= 1499) {scale = 1.00232172; return;}
        if (runno >= 1500 && runno <= 1599) {scale = 1.00217172; return;}
        if (runno >= 1600 && runno <= 1700) {scale = 1.00208639; return;}
        if (runno >= 1700 && runno <= 1799) {scale = 1.00188253; return;}
        if (runno >= 1800) {
          B2ERROR("scale_momenta not ready for this exp,run "
                  << expno << "," << runno);
        }
      } else if (expno == 21) {
        if (runno >=   0 && runno <=  99) {scale = 1.00228182; return;}
        if (runno >= 100 && runno <= 199) {scale = 1.00233446; return;}
        if (runno >= 200 && runno <= 299) {scale = 1.00238846; return;}
        if (runno >= 300 && runno <= 399) {scale = 1.00238061 ; return;}
        if (runno >= 400) {
          B2ERROR("scale_momenta not ready for this exp,run "
                  << expno << "," << runno);
        }
      } else if (expno == 23) {
        if (runno >=   0 && runno <=  99) {scale = 1.00216089; return;}
        if (runno >= 100 && runno <= 199) {scale = 1.0019783; return;}
        if (runno >= 200 && runno <= 299) {scale = 1.00172162; return;}
        if (runno >= 300 && runno <= 399) {scale = 1.0017938 ; return;}
        if (runno >= 400 && runno <= 499) {scale = 1.00177832 ; return;}
        if (runno >= 500 && runno <= 599) {scale = 1.0017609 ; return;}
        if (runno >= 600 && runno <= 699) {scale = 1.0017756 ; return;}
        if (runno >= 700) {
          B2ERROR("scale_momenta not ready for this exp,run "
                  << expno << "," << runno);
        }
      } else if (expno == 25) {
        if (runno >=   0 && runno <=  99) {scale = 1.00190068; return;}
        if (runno >= 100 && runno <= 199) {scale = 1.00199038; return;}
        if (runno >= 200 && runno <= 299) {scale = 1.00196171; return;}
        if (runno >= 300 && runno <= 399) {scale = 1.00200167; return;}
        if (runno >= 400 && runno <= 499) {scale = 1.0019667 ; return;}
        if (runno >= 500 && runno <= 599) {scale = 1.00212109; return;}
        if (runno >= 600 && runno <= 699) {scale = 1.00202115; return;}
        if (runno >= 700 && runno <= 799) {scale = 1.00196456; return;}
        if (runno >= 800 && runno <= 899) {scale = 1.00200392; return;}
        if (runno >= 900 && runno <= 999) {scale = 1.00199495; return;}
        if (runno >= 1000 && runno <= 1099) {scale = 1.00202212; return;}
        if (runno >= 1100 && runno <= 1199) {scale = 1.00205312; return;}
        if (runno >= 1200 && runno <= 1299) {scale = 1.00200891; return;}
        if (runno >= 1300 && runno <= 1399) {scale = 1.00196866; return;}
        if (runno >= 1400 && runno <= 1499) {scale = 1.0020257 ; return;}
        if (runno >= 1500 && runno <= 1599) {scale = 1.00196995; return;}
        if (runno >= 1600 && runno <= 1699) {scale = 1.00189232; return;}
        if (runno >= 1700 && runno <= 1799) {scale = 1.00179028; return;}
        if (runno >= 1800 && runno <= 1899) {scale = 1.00179459; return;}
        if (runno >= 1900 && runno <= 1999) {scale = 1.001762  ; return;}
        if (runno >= 2000 && runno <= 2099) {scale = 1.00179514; return;}
        if (runno >= 2100 && runno <= 2199) {scale = 1.00175211; return;}
        if (runno >= 2200) {
          B2ERROR("scale_momenta not ready for this exp,run "
                  << expno << "," << runno);
        }
      } else if (expno == 27) {
        if (runno >=   0 && runno <=  99) {scale = 1.00192647; return;}
        if (runno >= 100 && runno <= 199) {scale = 1.0019717 ; return;}
        if (runno >= 200 && runno <= 299) {scale = 1.00196428; return;}
        if (runno >= 300 && runno <= 399) {scale = 1.00190998; return;}
        if (runno >= 400 && runno <= 499) {scale = 1.00204645; return;}
        if (runno >= 500 && runno <= 599) {scale = 1.0020687 ; return;}
        if (runno >= 600 && runno <= 699) {scale = 1.00198209; return;}

        if (runno >= 700 && runno <= 800) {scale = 1.00187505; return;}
        if (runno >= 801 && runno <= 900) {scale = 1.00186805; return;}
        if (runno >= 901 && runno <= 1000) {scale = 1.00185576; return;}
        if (runno >= 1001 && runno <= 1079) {scale = 1.00177176; return;}
        if (runno >= 1080 && runno <= 1100) {scale = 1.00177184; return;}
        if (runno >= 1101 && runno <= 1200) {scale = 1.00174057; return;}
        if (runno >= 1201 && runno <= 1251) {scale = 1.00189649; return;}
        if (runno >= 1252 && runno <= 1300) {scale = 1.00189999; return;}
        if (runno >= 1301 && runno <= 1400) {scale = 1.0018818 ; return;}
        if (runno >= 1401 && runno <= 1500) {scale = 1.00200148; return;}
        if (runno >= 1501 && runno <= 1600) {scale = 1.00220137; return;}
        if (runno >= 1601 && runno <= 1632) {scale = 1.00217034; return;}
        if (runno >= 1633) {
          B2ERROR("scale_momenta not ready for this exp,run "
                  << expno << "," << runno);
        }
      } else if (expno == 31) {
        if (runno >=   0 && runno <=  99) {scale = 1.000373374; return;}
        if (runno >= 100 && runno <= 199) {scale = 1.000684843; return;}
        if (runno >= 200 && runno <= 299) {scale = 1.000741216; return;}
        if (runno >= 300 && runno <= 399) {scale = 1.00092523 ; return;}
        if (runno >= 400 && runno <= 499) {scale = 1.00104576 ; return;}
        if (runno >= 500 && runno <= 599) {scale = 1.00103982 ; return;}
        if (runno >= 600 && runno <= 699) {scale = 1.0010601  ; return;}
        if (runno >= 700 && runno <= 799) {scale = 1.000982515; return;}
        if (runno >= 800 && runno <= 899) {scale = 1.00101577 ; return;}
        if (runno >= 900 && runno <= 999) {scale = 1.000972193; return;}
        if (runno >= 1000 && runno <= 1099) {scale = 1.00102536 ; return;}
        if (runno >= 1100 && runno <= 1199) {scale = 1.00127571 ; return;}
        if (runno >= 1200 && runno <= 1299) {scale = 1.00117938 ; return;}
        if (runno >= 1300 && runno <= 1399) {scale = 1.00121084 ; return;}
        if (runno >= 1400 && runno <= 1499) {scale = 1.00125911 ; return;}
        if (runno >= 1500 && runno <= 1599) {scale = 1.0012907  ; return;}
        if (runno >= 1600 && runno <= 1699) {scale = 1.00125362 ; return;}
        if (runno >= 1700 && runno <= 1799) {scale = 1.00131686 ; return;}
        if (runno >= 1800) {
          B2ERROR("scale_momenta not ready for this exp,run "
                  << expno << "," << runno);
        }
      } else if (expno == 33) {
        if (runno >=   0 && runno <=  99) {scale = 1.00136407; return;}
        if (runno >= 100 && runno <= 199) {scale = 1.0013701 ; return;}
        if (runno >= 200 && runno <= 299) {scale = 1.00156244; return;}
        if (runno >= 300 && runno <= 399) {scale = 1.00138501; return;}
        if (runno >= 400 && runno <= 499) {scale = 1.00148468; return;}
        if (runno >= 500 && runno <= 599) {scale = 1.0013797 ; return;}
        if (runno >= 600 && runno <= 699) {scale = 1.00152298; return;}
        if (runno >= 700 && runno <= 799) {scale = 1.001524  ; return;}
        if (runno >= 800 && runno <= 899) {scale = 1.0014635 ; return;}
        if (runno >= 900) {
          B2ERROR("scale_momenta not ready for this exp,run "
                  << expno << "," << runno);
        }
      } else if (expno == 35) {
        if (runno >=   0 && runno <=  99) {scale = 1.00135452; return;}
        if (runno >= 100 && runno <= 199) {scale = 1.00135798; return;}
        if (runno >= 200 && runno <= 299) {scale = 1.00139979; return;}
        if (runno >= 300 && runno <= 399) {scale = 1.00136061; return;}
        if (runno >= 400 && runno <= 499) {scale = 1.00135776; return;}
        if (runno >= 500 && runno <= 599) {scale = 1.00118666; return;}
        if (runno >= 600 && runno <= 699) {scale = 1.0011346 ; return;}
        if (runno >= 700) {
          B2ERROR("scale_momenta not ready for this exp,run "
                  << expno << "," << runno);
        }
      } else if (expno == 37) {
        if (runno >=   0 && runno <=  99) {scale = 1.00076522 ; return;}
        if (runno >= 100 && runno <= 199) {scale = 1.000875154; return;}
        if (runno >= 200 && runno <= 299) {scale = 1.000946763; return;}
        if (runno >= 300 && runno <= 399) {scale = 1.000868444; return;}
        if (runno >= 400 && runno <= 499) {scale = 1.000781409; return;}
        if (runno >= 500 && runno <= 599) {scale = 1.000781171; return;}
        if (runno >= 600 && runno <= 699) {scale = 1.000857962; return;}
        if (runno >= 700 && runno <= 799) {scale = 1.000756082; return;}
        if (runno >= 800 && runno <= 899) {scale = 1.000727355; return;}
        if (runno >= 900 && runno <= 999) {scale = 1.000740236; return;}
        if (runno >= 1000 && runno <= 1099) {scale = 1.000499606; return;}
        if (runno >= 1100 && runno <= 1199) {scale = 1.000481664; return;}
        if (runno >= 1200 && runno <= 1299) {scale = 1.000706924; return;}
        if (runno >= 1300 && runno <= 1399) {scale = 1.000673738; return;}
        if (runno >= 1400 && runno <= 1499) {scale = 1.000662648; return;}
        if (runno >= 1500 && runno <= 1599) {scale = 1.000671198; return;}
        if (runno >= 1600 && runno <= 1699) {scale = 1.000604364; return;}
        if (runno >= 1700 && runno <= 1799) {scale = 1.000717372; return;}
        if (runno >= 1800 && runno <= 1899) {scale = 1.000512399; return;}
        if (runno >= 1900 && runno <= 1999) {scale = 1.000436958; return;}
        if (runno >= 2000) {
          B2ERROR("scale_momenta not ready for this exp,run "
                  << expno << "," << runno);
        }
      } else if (expno == 39) {
        if (runno >=   0 && runno <=  99) {scale = 1.000504342; return;}
        if (runno >= 100 && runno <= 199) {scale = 1.000704544; return;}
        if (runno >= 200 && runno <= 299) {scale = 1.00094335 ; return;}
        if (runno >= 300 && runno <= 399) {scale = 1.000928819; return;}
        if (runno >= 400 && runno <= 499) {scale = 1.000884638; return;}
        if (runno >= 500 && runno <= 599) {scale = 1.00083459 ; return;}
        if (runno >= 600 && runno <= 699) {scale = 1.000767604; return;}
        if (runno >= 700 && runno <= 799) {scale = 1.000882219; return;}
        if (runno >= 800 && runno <= 899) {scale = 1.000781437; return;}
        if (runno >= 900 && runno <= 999) {scale = 1.000853168; return;}
        if (runno >= 1000 && runno <= 1099) {scale = 1.000926527; return;}
        if (runno >= 1100 && runno <= 1199) {scale = 1.000942882; return;}
        if (runno >= 1200 && runno <= 1299) {scale = 1.000932802; return;}
        if (runno >= 1300 && runno <= 1399) {scale = 1.000898892; return;}
        if (runno >= 1400) {
          B2ERROR("scale_momenta not ready for this exp,run "
                  << expno << "," << runno);
        }
      } else if (expno == 41) {
        if (runno >=   0 && runno <=  99) {scale = 1.00178427; return;}
        if (runno >= 100 && runno <= 199) {scale = 1.00188559; return;}
        if (runno >= 200 && runno <= 299) {scale = 1.0019292 ; return;}
        if (runno >= 300 && runno <= 399) {scale = 1.00196352; return;}
        if (runno >= 400 && runno <= 499) {scale = 1.0019078 ; return;}
        if (runno >= 500 && runno <= 599) {scale = 1.00185598; return;}
        if (runno >= 600 && runno <= 699) {scale = 1.00191314; return;}
        if (runno >= 700 && runno <= 799) {scale = 1.00179647; return;}
        if (runno >= 800 && runno <= 899) {scale = 1.00189776; return;}
        if (runno >= 900 && runno <= 999) {scale = 1.00184798; return;}
        if (runno >= 1000 && runno <= 1099) {scale = 1.00177963; return;}
        if (runno >= 1100 && runno <= 1199) {scale = 1.00176408; return;}
        if (runno >= 1200 && runno <= 1299) {scale = 1.00171371; return;}
        if (runno >= 1300) {
          B2ERROR("scale_momenta not ready for this exp,run "
                  << expno << "," << runno);
        }
      } else if (expno == 43) {
        if (runno >=   0 && runno <=  99) {scale = 1.00142307; return;}
        if (runno >= 100 && runno <= 199) {scale = 1.000979455; return;}
        if (runno >= 200 && runno <= 299) {scale = 1.000974458; return;}
        if (runno >= 300 && runno <= 399) {scale = 1.00103301; return;}
        if (runno >= 400 && runno <= 499) {scale = 1.001111994; return;}
        if (runno >= 500 && runno <= 599) {scale = 1.00100635; return;}
        if (runno >= 600 && runno <= 699) {scale = 1.00105078; return;}
        if (runno >= 700 && runno <= 799) {scale = 1.00103593; return;}
        if (runno >= 800 && runno <= 899) {scale = 1.00105158; return;}
        if (runno >= 900 && runno <= 999) {scale = 1.000955608; return;}
        if (runno >= 1000 && runno <= 1099) {scale = 1.00099199; return;}
        if (runno >= 1100 && runno <= 1199) {scale = 1.0011439; return;}
        if (runno >= 1200) {
          B2ERROR("scale_momenta not ready for this exp,run "
                  << expno << "," << runno);
        }
      } else if (expno == 45) {
        if (runno >=   0 && runno <=  99) {scale = 1.00126261; return;}
        if (runno >= 100 && runno <= 199) {scale = 1.00138601; return;}
        if (runno >= 200 && runno <= 299) {scale = 1.00135372; return;}
        if (runno >= 300 && runno <= 399) {scale = 1.00141286; return;}
        if (runno >= 400 && runno <= 499) {scale = 1.00147822; return;}
        if (runno >= 500) {
          B2ERROR("scale_momenta not ready for this exp,run "
                  << expno << "," << runno);
        }
      } else if (expno == 47) {
        if (runno >=   0 && runno <=  99) {scale = 1.00156977; return;}
        if (runno >= 100 && runno <= 199) {scale = 1.00155614; return;}
        if (runno >= 200 && runno <= 299) {scale = 1.0016555; return;}
        if (runno >= 300 && runno <= 399) {scale = 1.00167046; return;}
        if (runno >= 400 && runno <= 499) {scale = 1.00168705; return;}
        if (runno >= 500 && runno <= 599) {scale = 1.00169555; return;}
        if (runno >= 600 && runno <= 699) {scale = 1.00175653; return;}
        if (runno >= 700 && runno <= 799) {scale = 1.00174358; return;}
        if (runno >= 800 && runno <= 899) {scale = 1.00174004; return;}
        if (runno >= 900) {
          B2ERROR("scale_momenta not ready for this exp,run "
                  << expno << "," << runno);
        }
      } else if (expno == 49) {
        if (runno >=   0 && runno <=  99) {scale = 1.00158837; return;}
        if (runno >= 100 && runno <= 199) {scale = 1.00163884; return;}
        if (runno >= 200 && runno <= 299) {scale = 1.00160595; return;}
        if (runno >= 300 && runno <= 399) {scale = 1.00149916; return;}
        if (runno >= 400 && runno <= 499) {scale = 1.0014956 ; return;}
        if (runno >= 500 && runno <= 599) {scale = 1.00156212; return;}
        if (runno >= 600 && runno <= 699) {scale = 1.00121868; return;}
        if (runno >= 700 && runno <= 799) {scale = 1.00134613; return;}
        if (runno >= 800 && runno <= 899) {scale = 1.00138985; return;}
        if (runno >= 900 && runno <= 999) {scale = 1.00129356; return;}
        if (runno >= 1000 && runno <= 1099) {scale = 1.00119732; return;}
        if (runno >= 1100 && runno <= 1199) {scale = 1.00121481; return;}
        if (runno >= 1200 && runno <= 1299) {scale = 1.00121108; return;}
        if (runno >= 1300) {
          B2ERROR("scale_momenta not ready for this exp,run "
                  << expno << "," << runno);
        }
      } else if (expno == 51) {
        if (runno >=   0 && runno <=  99) {scale = 1.00160252; return;}
        if (runno >= 100 && runno <= 199) {scale = 1.00156099; return;}
        if (runno >= 200 && runno <= 299) {scale = 1.00154760; return;}
        if (runno >= 300 && runno <= 399) {scale = 1.00146316; return;}
        if (runno >= 400 && runno <= 499) {scale = 1.00145525; return;}
        if (runno >= 500 && runno <= 599) {scale = 1.00134429; return;}
        if (runno >= 600 && runno <= 699) {scale = 1.00135581; return;}
        if (runno >= 700 && runno <= 799) {scale = 1.00134382; return;}
        if (runno >= 800 && runno <= 899) {scale = 1.00126462; return;}
        if (runno >= 900 && runno <= 999) {scale = 1.00130752; return;}
        if (runno >= 1000 && runno <= 1099) {scale = 1.00130452; return;}
        if (runno >= 1100 && runno <= 1199) {scale = 1.00131440; return;}
        if (runno >= 1200 && runno <= 1299) {scale = 1.00130864; return;}
        if (runno >= 1300 && runno <= 1399) {scale = 1.00105290; return;}
        if (runno >= 1400 && runno <= 1499) {scale = 1.00126645; return;}
        if (runno >= 1500 && runno <= 1599) {scale = 1.00126383; return;}
        if (runno >= 1600 && runno <= 1699) {scale = 1.00141111; return;}
        if (runno >= 1700 && runno <= 1799) {scale = 1.00126220; return;}
        if (runno >= 1800 && runno <= 1899) {scale = 1.00105098; return;}
        if (runno >= 1900) {
          B2ERROR("scale_momenta not ready for this exp,run "
                  << expno << "," << runno);
        }
      } else if (expno == 53) {
        if (runno >=   0 && runno <=  99) {scale = 1.0011516; return;}
        if (runno >= 100 && runno <= 199) {scale = 1.00115527; return;}
        if (runno >= 200 && runno <= 299) {scale = 1.00114844; return;}
        if (runno >= 300) {
          B2ERROR("scale_momenta not ready for this exp,run "
                  << expno << "," << runno);
        }
      } else if (expno == 55) {
        if (runno >=   0 && runno <=  99) {scale = 1.00114284; return;}
        if (runno >= 100 && runno <= 199) {scale = 1.00111458; return;}
        if (runno >= 200 && runno <= 299) {scale = 1.00109686; return;}
        if (runno >= 300 && runno <= 399) {scale = 1.00119475; return;}
        if (runno >= 400 && runno <= 499) {scale = 1.00117818; return;}
        if (runno >= 500 && runno <= 599) {scale = 1.00115789; return;}
        if (runno >= 600 && runno <= 699) {scale = 1.00122261; return;}
        if (runno >= 700 && runno <= 799) {scale = 1.00118454; return;}
        if (runno >= 800 && runno <= 899) {scale = 1.00118042; return;}
        if (runno >= 900 && runno <= 999) {scale = 1.00124759; return;}
        if (runno >= 1000 && runno <= 1099) {scale = 1.00128055; return;}
        if (runno >= 1100 && runno <= 1199) {scale = 1.00119131; return;}
        if (runno >= 1200 && runno <= 1299) {scale = 1.00122238; return;}
        if (runno >= 1300 && runno <= 1399) {scale = 1.00129538; return;}
        if (runno >= 1400 && runno <= 1499) {scale = 1.00130387; return;}
        if (runno >= 1500 && runno <= 1599) {scale = 1.00130858; return;}
        if (runno >= 1600 && runno <= 1699) {scale = 1.00111854; return;}
        if (runno >= 1700 && runno <= 1799) {scale = 1.00136261; return;}
        if (runno >= 1800) {
          B2ERROR("scale_momenta not ready for this exp,run "
                  << expno << "," << runno);
        }
      } else if (expno == 61) {
        if (runno >=   0 && runno <=  99) {scale = 1.0009992; return;}
        if (runno >= 100 && runno <= 199) {scale = 1.00113704; return;}
        if (runno >= 200 && runno <= 299) {scale = 1.00129904; return;}
        if (runno >= 300 && runno <= 399) {scale = 1.00141879; return;}
        if (runno >= 400 && runno <= 499) {scale = 1.00146707; return;}
        if (runno >= 500 && runno <= 599) {scale = 1.00150101; return;}
        if (runno >= 600 && runno <= 699) {scale = 1.00147322; return;}
        if (runno >= 700 && runno <= 799) {scale = 1.00153929; return;}
        if (runno >= 800 && runno <= 899) {scale = 1.00159997; return;}
        if (runno >= 900 && runno <= 999) {scale = 1.00164032; return;}
        if (runno >= 1000 && runno <= 1099) {scale = 1.00165878; return;}
        if (runno >= 1100 && runno <= 1199) {scale = 1.00163475; return;}
        if (runno >= 1200 && runno <= 1207) {scale = 1.00166193; return;}
        if (runno >= 1208 && runno <= 1299) {scale = 1.00235824; return;}
        if (runno >= 1300 && runno <= 1399) {scale = 1.00242282; return;}
        if (runno >= 1400) {
          B2ERROR("scale_momenta not ready for this exp,run "
                  << expno << "," << runno);
        }
      } else if (expno == 65) {
        if (runno <= 999) {
          B2ERROR("scale_momenta not ready for this exp,run "
                  << expno << "," << runno);
        }
        if (runno >= 1000 && runno <= 1336) {scale = 1.00145234; return;}
        if (runno >= 1336) {
          B2ERROR("scale_momenta not ready for this exp,run "
                  << expno << "," << runno);
        }
      } else
        B2ERROR("scale_momenta mode=2 not ready for exp " << expno);
    }
    return;
  }

//=====================================================================
  void B2BIIFixMdstModule::scale_momenta_set_v2(const int mode, const int expno,
                                                const int runno, double& scale)
  {
//=====================================================================

    //for e15 mdst processed with b20020405

    scale = 1.;

    if (mode == 1) {
      B2ERROR("scale_momenta mode=1 not ready for exp " << expno);
      return;
    }

    if (mode == 2) {
      if (expno == 7) {

      } else if (expno == 9) {

      } else if (expno == 11) {

      } else if (expno == 13) {

      } else if (expno == 15) {

      } else if (expno == 17) {

      } else if (expno == 19) {
        B2ERROR("scale_momenta not ready for this exp,run "
                << expno << "," << runno);

      } else if (expno == 21) {
        B2ERROR("scale_momenta not ready for this exp,run "
                << expno << "," << runno);

      } else if (expno == 23) {
        B2ERROR("scale_momenta not ready for this exp,run "
                << expno << "," << runno);

      } else if (expno == 25) {
        B2ERROR("scale_momenta not ready for this exp,run "
                << expno << "," << runno);

      } else if (expno == 27) {
        B2ERROR("scale_momenta not ready for this exp,run "
                << expno << "," << runno);

      } else if (expno == 31) {
        if (runno >=   0 && runno <= 137) {
          B2ERROR("scale_momenta not ready for this exp,run "
                  << expno << "," << runno);
        }
        if (runno >= 138 && runno <= 199) {scale = 1.000931841; return;}
        if (runno >= 200 && runno <= 299) {scale = 1.000916397; return;}
        if (runno >= 300 && runno <= 399) {scale = 1.00108023 ; return;}
        if (runno >= 400 && runno <= 499) {scale = 1.00118662 ; return;}
        if (runno >= 500 && runno <= 599) {scale = 1.00117739 ; return;}
        if (runno >= 600 && runno <= 699) {scale = 1.00119542 ; return;}
        if (runno >= 700 && runno <= 799) {scale = 1.00110396 ; return;}
        if (runno >= 800 && runno <= 899) {scale = 1.00109603 ; return;}
        if (runno >= 900 && runno <= 999) {scale = 1.00112795 ; return;}
        if (runno >= 1000 && runno <= 1099) {scale = 1.00118365 ; return;}
        if (runno >= 1100 && runno <= 1199) {scale = 1.00142214 ; return;}
        if (runno >= 1200 && runno <= 1299) {scale = 1.00133150 ; return;}
        if (runno >= 1300 && runno <= 1399) {scale = 1.00132831 ; return;}
        if (runno >= 1400 && runno <= 1499) {scale = 1.00136554 ; return;}
        if (runno >= 1500 && runno <= 1599) {scale = 1.00141187 ; return;}
        if (runno >= 1600 && runno <= 1699) {scale = 1.00136628 ; return;}
        if (runno >= 1700 && runno <= 1799) {scale = 1.00139273 ; return;}
        if (runno >= 1800) {
          B2ERROR("scale_momenta not ready for this exp,run "
                  << expno << "," << runno);
        }
      } else if (expno == 33) {
        if (runno >=   0 && runno <=  99) {scale = 1.00149319; return;}
        if (runno >= 100 && runno <= 199) {scale = 1.00150915; return;}
        if (runno >= 200 && runno <= 299) {scale = 1.00173040; return;}
        if (runno >= 300 && runno <= 399) {scale = 1.00150449; return;}
        if (runno >= 400 && runno <= 499) {scale = 1.00161519; return;}
        if (runno >= 500 && runno <= 599) {scale = 1.00151670; return;}
        if (runno >= 600 && runno <= 699) {scale = 1.00164347; return;}
        if (runno >= 700 && runno <= 799) {scale = 1.00164165; return;}
        if (runno >= 800 && runno <= 899) {scale = 1.00161369; return;}
        if (runno >= 900) {
          B2ERROR("scale_momenta not ready for this exp,run "
                  << expno << "," << runno);
        }
      } else if (expno == 35) {
        if (runno >=   0 && runno <=  99) {scale = 1.00147034; return;}
        if (runno >= 100 && runno <= 199) {scale = 1.00148523; return;}
        if (runno >= 200 && runno <= 299) {scale = 1.00153372; return;}
        if (runno >= 300 && runno <= 399) {scale = 1.00148256; return;}
        if (runno >= 400 && runno <= 499) {scale = 1.00144902; return;}
        if (runno >= 500 && runno <= 599) {scale = 1.00131501; return;}
        if (runno >= 600 && runno <= 699) {scale = 1.00126371; return;}
        if (runno >= 700) {
          B2ERROR("scale_momenta not ready for this exp,run "
                  << expno << "," << runno);
        }
      } else if (expno == 37) {
        if (runno >=   0 && runno <=  99) {scale = 1.000916277; return;}
        if (runno >= 100 && runno <= 199) {scale = 1.001035310; return;}
        if (runno >= 200 && runno <= 299) {scale = 1.001123403; return;}
        if (runno >= 300 && runno <= 399) {scale = 1.001017718; return;}
        if (runno >= 400 && runno <= 499) {scale = 1.000932890; return;}
        if (runno >= 500 && runno <= 599) {scale = 1.000928479; return;}
        if (runno >= 600 && runno <= 699) {scale = 1.000997938; return;}
        if (runno >= 700 && runno <= 799) {scale = 1.000899663; return;}
        if (runno >= 800 && runno <= 899) {scale = 1.000860910; return;}
        if (runno >= 900 && runno <= 999) {scale = 1.000882920; return;}
        if (runno >= 1000 && runno <= 1099) {scale = 1.000616966; return;}
        if (runno >= 1100 && runno <= 1199) {scale = 1.000613018; return;}
        if (runno >= 1200 && runno <= 1299) {scale = 1.000832338; return;}
        if (runno >= 1300 && runno <= 1399) {scale = 1.000803640; return;}
        if (runno >= 1400 && runno <= 1499) {scale = 1.000770454; return;}
        if (runno >= 1500 && runno <= 1599) {scale = 1.000786608; return;}
        if (runno >= 1600 && runno <= 1699) {scale = 1.000718089; return;}
        if (runno >= 1700 && runno <= 1799) {scale = 1.000826042; return;}
        if (runno >= 1800 && runno <= 1899) {scale = 1.000638150; return;}
        if (runno >= 1900 && runno <= 1999) {scale = 1.000529173; return;}
        if (runno >= 2000) {
          B2ERROR("scale_momenta not ready for this exp,run "
                  << expno << "," << runno);
        }
      } else if (expno == 39) {
        if (runno >=   0 && runno <=  99) {scale = 1.000610857; return;}
        if (runno >= 100 && runno <= 199) {scale = 1.000838583; return;}
        if (runno >= 200 && runno <= 299) {scale = 1.00105918 ; return;}
        if (runno >= 300 && runno <= 399) {scale = 1.00105841 ; return;}
        if (runno >= 400 && runno <= 499) {scale = 1.001025523; return;}
        if (runno >= 500 && runno <= 599) {scale = 1.000967373; return;}
        if (runno >= 600 && runno <= 699) {scale = 1.000898585; return;}
        if (runno >= 700 && runno <= 799) {scale = 1.001003199; return;}
        if (runno >= 800 && runno <= 899) {scale = 1.000897072; return;}
        if (runno >= 900 && runno <= 999) {scale = 1.000972551; return;}
        if (runno >= 1000 && runno <= 1099) {scale = 1.001044677; return;}
        if (runno >= 1100 && runno <= 1199) {scale = 1.00106451 ; return;}
        if (runno >= 1200 && runno <= 1299) {scale = 1.00108570 ; return;}
        if (runno >= 1300 && runno <= 1399) {scale = 1.00102381 ; return;}
        if (runno >= 1400) {
          B2ERROR("scale_momenta not ready for this exp,run "
                  << expno << "," << runno);
        }
      } else if (expno == 41) {
        if (runno >=   0 && runno <=  99) {scale = 1.00189378; return;}
        if (runno >= 100 && runno <= 199) {scale = 1.00197304; return;}
        if (runno >= 200 && runno <= 299) {scale = 1.00204049; return;}
        if (runno >= 300 && runno <= 399) {scale = 1.00205065; return;}
        if (runno >= 400 && runno <= 499) {scale = 1.00199205; return;}
        if (runno >= 500 && runno <= 599) {scale = 1.00195618; return;}
        if (runno >= 600 && runno <= 699) {scale = 1.00200889; return;}
        if (runno >= 700 && runno <= 799) {scale = 1.00190365; return;}
        if (runno >= 800 && runno <= 899) {scale = 1.00204192; return;}
        if (runno >= 900 && runno <= 999) {scale = 1.00196542; return;}
        if (runno >= 1000 && runno <= 1099) {scale = 1.00189706; return;}
        if (runno >= 1100 && runno <= 1199) {scale = 1.00187422; return;}
        if (runno >= 1200 && runno <= 1299) {scale = 1.00183714; return;}
        if (runno >= 1300) {
          B2ERROR("scale_momenta not ready for this exp,run "
                  << expno << "," << runno);
        }
      } else if (expno == 43) {
        if (runno >=   0 && runno <=  99) {scale = 1.00151737; return;}
        if (runno >= 100 && runno <= 199) {scale = 1.00110489; return;}
        if (runno >= 200 && runno <= 299) {scale = 1.00108144; return;}
        if (runno >= 300 && runno <= 399) {scale = 1.00114918; return;}
        if (runno >= 400 && runno <= 499) {scale = 1.00122723; return;}
        if (runno >= 500 && runno <= 599) {scale = 1.00111069; return;}
        if (runno >= 600 && runno <= 699) {scale = 1.00115667; return;}
        if (runno >= 700 && runno <= 799) {scale = 1.00113759; return;}
        if (runno >= 800 && runno <= 899) {scale = 1.00115609; return;}
        if (runno >= 900 && runno <= 999) {scale = 1.00105426; return;}
        //tentative for quality check of 5S mdst in 2009 Ang.-Sep.
        //      if (runno>=1000 && runno <=1034) {scale = 1.00099199; return;}
        //      if (runno>=1035 && runno <=1099) {scale = 1.00111222; return;}
        if (runno >= 1000 && runno <= 1099) {scale = 1.00111210; return;}
        if (runno >= 1100 && runno <= 1199) {scale = 1.00123104; return;}
        if (runno >= 1200) {
          B2ERROR("scale_momenta not ready for this exp,run "
                  << expno << "," << runno);
        }
      } else if (expno == 45) {
        if (runno >=   0 && runno <=  99) {scale = 1.00136477; return;}
        if (runno >= 100 && runno <= 199) {scale = 1.00151600; return;}
        if (runno >= 200 && runno <= 299) {scale = 1.00146757; return;}
        if (runno >= 300 && runno <= 399) {scale = 1.00153299; return;}
        if (runno >= 400 && runno <= 499) {scale = 1.00159018; return;}
        if (runno >= 500) {
          B2ERROR("scale_momenta not ready for this exp,run "
                  << expno << "," << runno);
        }
      } else if (expno == 47) {
        if (runno >=   0 && runno <=  99) {scale = 1.00166672; return;}
        if (runno >= 100 && runno <= 199) {scale = 1.00165120; return;}
        if (runno >= 200 && runno <= 299) {scale = 1.00175597; return;}
        if (runno >= 300 && runno <= 399) {scale = 1.00177319; return;}
        if (runno >= 400 && runno <= 499) {scale = 1.00179552; return;}
        if (runno >= 500 && runno <= 599) {scale = 1.00179413; return;}
        if (runno >= 600 && runno <= 699) {scale = 1.00186237; return;}
        if (runno >= 700 && runno <= 799) {scale = 1.00183016; return;}
        if (runno >= 800 && runno <= 899) {scale = 1.00184324; return;}
        if (runno >= 900) {
          B2ERROR("scale_momenta not ready for this exp,run "
                  << expno << "," << runno);
        }
      } else if (expno == 49) {
        if (runno >=   0 && runno <=  99) {scale = 1.00171645; return;}
        if (runno >= 100 && runno <= 199) {scale = 1.00177728; return;}
        if (runno >= 200 && runno <= 299) {scale = 1.00173301; return;}
        if (runno >= 300 && runno <= 399) {scale = 1.00162075; return;}
        if (runno >= 400 && runno <= 499) {scale = 1.00163153; return;}
        if (runno >= 500 && runno <= 599) {scale = 1.00168559; return;}
        if (runno >= 600 && runno <= 699) {scale = 1.00139227; return;}
        if (runno >= 700 && runno <= 799) {scale = 1.00148583; return;}
        if (runno >= 800 && runno <= 899) {scale = 1.00150403; return;}
        if (runno >= 900 && runno <= 999) {scale = 1.00142759; return;}
        if (runno >= 1000 && runno <= 1099) {scale = 1.00134573; return;}
        if (runno >= 1100 && runno <= 1199) {scale = 1.00138313; return;}
        if (runno >= 1200 && runno <= 1299) {scale = 1.00151369; return;}
        if (runno >= 1300) {
          B2ERROR("scale_momenta not ready for this exp,run "
                  << expno << "," << runno);
        }
      } else if (expno == 51) {
        if (runno >=   0 && runno <=  99) {scale = 1.00165035; return;}
        if (runno >= 100 && runno <= 199) {scale = 1.00161504; return;}
        if (runno >= 200 && runno <= 299) {scale = 1.00160162; return;}
        if (runno >= 300 && runno <= 399) {scale = 1.00152725; return;}
        if (runno >= 400 && runno <= 499) {scale = 1.00149943; return;}
        if (runno >= 500 && runno <= 599) {scale = 1.00141294; return;}
        if (runno >= 600 && runno <= 699) {scale = 1.00140154; return;}
        if (runno >= 700 && runno <= 799) {scale = 1.00140759; return;}
        if (runno >= 800 && runno <= 899) {scale = 1.00133671; return;}
        if (runno >= 900 && runno <= 999) {scale = 1.00136792; return;}
        if (runno >= 1000 && runno <= 1099) {scale = 1.00135251; return;}
        if (runno >= 1100 && runno <= 1199) {scale = 1.00138229; return;}
        if (runno >= 1200 && runno <= 1299) {scale = 1.00134938; return;}
        if (runno >= 1300 && runno <= 1399) {scale = 1.00106240; return;}
        if (runno >= 1400 && runno <= 1499) {scale = 1.00132666; return;}
        if (runno >= 1500 && runno <= 1599) {scale = 1.00132654; return;}
        if (runno >= 1600 && runno <= 1699) {scale = 1.00146619; return;}
        if (runno >= 1700 && runno <= 1799) {scale = 1.00131902; return;}
        if (runno >= 1800 && runno <= 1899) {scale = 1.00114243; return;}
        if (runno >= 1900) {
          B2ERROR("scale_momenta not ready for this exp,run "
                  << expno << "," << runno);
        }
      } else if (expno == 53) {
        if (runno >=   0 && runno <=  99) {scale = 1.00125475; return;}
        if (runno >= 100 && runno <= 199) {scale = 1.00124954; return;}
        if (runno >= 200 && runno <= 299) {scale = 1.00122914; return;}
        if (runno >= 300) {
          B2ERROR("scale_momenta not ready for this exp,run "
                  << expno << "," << runno);
        }
      } else if (expno == 55) {
        if (runno >=   0 && runno <=  99) {scale = 1.00119352; return;}
        if (runno >= 100 && runno <= 199) {scale = 1.00117130; return;}
        if (runno >= 200 && runno <= 299) {scale = 1.00115825; return;}
        if (runno >= 300 && runno <= 399) {scale = 1.00125005; return;}
        if (runno >= 400 && runno <= 499) {scale = 1.00124720; return;}
        if (runno >= 500 && runno <= 599) {scale = 1.00122234; return;}
        if (runno >= 600 && runno <= 699) {scale = 1.00128709; return;}
        if (runno >= 700 && runno <= 799) {scale = 1.00123081; return;}
        if (runno >= 800 && runno <= 899) {scale = 1.00124198; return;}
        if (runno >= 900 && runno <= 999) {scale = 1.00131118; return;}
        if (runno >= 1000 && runno <= 1099) {scale = 1.00132496; return;}
        if (runno >= 1100 && runno <= 1199) {scale = 1.00126186; return;}
        if (runno >= 1200 && runno <= 1299) {scale = 1.00127849; return;}
        if (runno >= 1300 && runno <= 1399) {scale = 1.00135312; return;}
        if (runno >= 1400 && runno <= 1499) {scale = 1.00136637; return;}
        if (runno >= 1500 && runno <= 1599) {scale = 1.00136270; return;}
        if (runno >= 1600 && runno <= 1699) {scale = 1.00118422; return;}
        if (runno >= 1700 && runno <= 1799) {scale = 1.00142667; return;}
        if (runno >= 1800) {
          B2ERROR("scale_momenta not ready for this exp,run "
                  << expno << "," << runno);
        }
      } else if (expno == 61) {
        if (runno >=   0 && runno <=  99) {scale = 1.00103013; return;}
        if (runno >= 100 && runno <= 199) {scale = 1.00116185; return;}
        if (runno >= 200 && runno <= 299) {scale = 1.00133560; return;}
        if (runno >= 300 && runno <= 399) {scale = 1.00145027; return;}
        if (runno >= 400 && runno <= 499) {scale = 1.00147949; return;}
        if (runno >= 500 && runno <= 599) {scale = 1.00151022; return;}
        if (runno >= 600 && runno <= 699) {scale = 1.00150439; return;}
        if (runno >= 700 && runno <= 799) {scale = 1.00155006; return;}
        if (runno >= 800 && runno <= 899) {scale = 1.00162396; return;}
        if (runno >= 900 && runno <= 999) {scale = 1.00168542; return;}
        if (runno >= 1000 && runno <= 1099) {scale = 1.00168249; return;}
        if (runno >= 1100 && runno <= 1207) {scale = 1.00166891; return;}
        if (runno >= 1208 && runno <= 1299) {scale = 1.00249956; return;}
        if (runno >= 1300 && runno <= 1399) {scale = 1.00255134; return;}
        if (runno >= 1400) {
          B2ERROR("scale_momenta not ready for this exp,run "
                  << expno << "," << runno);
        }
      } else if (expno == 63) {
        if (runno >=   0 && runno <=  99) {scale = 1.00129667; return;}
        if (runno >= 100 && runno <= 199) {scale = 1.00123725; return;}
        if (runno >= 200 && runno <= 299) {scale = 1.00126795; return;}
        if (runno >= 300 && runno <= 399) {scale = 1.00122458; return;}
        if (runno >= 400 && runno <= 499) {scale = 1.00116489; return;}
        if (runno >= 500 && runno <= 599) {scale = 1.00116968; return;}
        if (runno >= 600 && runno <= 699) {scale = 1.000918379; return;}
        if (runno >= 700 && runno <= 799) {scale = 1.0010429; return;}
        if (runno >= 800) {
          B2ERROR("scale_momenta not ready for this exp,run "
                  << expno << "," << runno);
        }
      } else if (expno == 65) {
        if (runno >=   0 && runno <=  99) {scale = 1.00116975; return;}
        if (runno >= 100 && runno <= 199) {scale = 1.00111926; return;}
        if (runno >= 200 && runno <= 299) {scale = 1.00110162; return;}
        if (runno >= 300 && runno <= 399) {scale = 1.00109524; return;}
        if (runno >= 400 && runno <= 499) {scale = 1.00106913; return;}
        if (runno >= 500 && runno <= 599) {scale = 1.00110941; return;}
        if (runno >= 600 && runno <= 699) {scale = 1.000897865; return;}
        if (runno >= 700 && runno <= 999) {scale = 1.00104385; return;}
        if (runno >= 1000 && runno <= 1299) {scale = 1.000876489; return;}
        if (runno >= 1000) {
          B2ERROR("scale_momenta not ready for this exp,run "
                  << expno << "," << runno);
        }
      } else if (expno == 67) {
        if (runno >=   0 && runno <= 199) {scale = 1.000826364; return;}
        if (runno >= 200 && runno <= 299) {scale = 1.000836576; return;}
        if (runno >= 300 && runno <= 399) {scale = 1.000904815; return;}
        if (runno >= 400 && runno <= 499) {scale = 1.000966045; return;}
        if (runno >= 500 && runno <= 599) {scale = 1.000988147; return;}
        if (runno >= 600 && runno <= 699) {scale = 1.000988147; return;}
        if (runno >= 700 && runno <= 742) {scale = 1.000837414; return;}
        if (runno >= 1000 && runno <= 1099) {scale = 1.000984865; return;}
        if (runno >= 1100 && runno <= 1123) {scale = 1.00105248 ; return;}
        if (runno >= 1124) {
          B2ERROR("scale_momenta not ready for this exp,run "
                  << expno << "," << runno);
        }
      } else if (expno == 69) {
        if (runno >=   0 && runno <=  99) {scale = 1.000791450; return;}
        if (runno >= 100 && runno <= 199) {scale = 1.000891748; return;}
        if (runno >= 200 && runno <= 299) {scale = 1.000866165; return;}
        if (runno >= 300 && runno <= 399) {scale = 1.000838834; return;}
        if (runno >= 400 && runno <= 499) {scale = 1.000811878; return;}
        if (runno >= 500 && runno <= 599) {scale = 1.000779810; return;}
        if (runno >= 600 && runno <= 699) {scale = 1.000799086; return;}
        if (runno >= 700 && runno <= 799) {scale = 1.000833797; return;}
        if (runno >= 800 && runno <= 899) {scale = 1.000875203; return;}
        if (runno >= 900 && runno <= 999) {scale = 1.000891998; return;}
        if (runno >= 1000 && runno <= 1099) {scale = 1.000921074; return;}
        if (runno >= 1100 && runno <= 1199) {scale = 1.000900829; return;}
        if (runno >= 1200 && runno <= 1299) {scale = 1.000958405; return;}
        if (runno >= 1300 && runno <= 1399) {scale = 1.000836841; return;}
        if (runno >= 1400) {
          B2ERROR("scale_momenta not ready for this exp,run "
                  << expno << "," << runno);
        }
      } else if (expno == 71) {
        if (runno >=   0 && runno <=  99) {scale = 1.000962999; return;}
        if (runno >= 100 && runno <= 199) {scale = 1.001478932; return;}
        if (runno >= 200 && runno <= 300) {scale = 1.001486524; return;}
        if (runno >= 301 && runno <= 384) {scale = 1.001430843; return;}
        if (runno >= 385 && runno <= 499) {scale = 1.001505696; return;}
        if (runno >= 500 && runno <= 599) {scale = 1.001523980; return;}
        if (runno >= 600 && runno <= 699) {scale = 1.001480830; return;}
        if (runno >= 1000 && runno <= 1013) {scale = 1.001480830; return;}
        if (runno >= 2000 && runno <= 2099) {scale = 1.001617882; return;}
        if (runno >= 2100 && runno <= 2199) {scale = 1.001644395; return;}
        if (runno >= 2200 && runno <= 2299) {scale = 1.001722184; return;}
        if (runno >= 700 && runno <= 999) {
          B2ERROR("scale_momenta not ready for this exp,run "
                  << expno << "," << runno);
        }
        if (runno >= 1014 && runno <= 1999) {
          B2ERROR("scale_momenta not ready for this exp,run "
                  << expno << "," << runno);
        }
        if (runno >= 2299) {
          B2ERROR("scale_momenta not ready for this exp,run "
                  << expno << "," << runno);
        }
      } else if (expno == 73) {
        if (runno >=   0 && runno <=  99) {scale = 1.000721587; return;}
        if (runno >= 100 && runno <= 199) {scale = 1.000707089; return;}
        if (runno >= 200 && runno <= 299) {scale = 1.000722517; return;}
        if (runno >= 300 && runno <= 399) {scale = 1.000722517; return;} //nodata
        if (runno >= 400 && runno <= 499) {scale = 1.000750776; return;}
        if (runno >= 500 && runno <= 599) {scale = 1.000729771; return;}
        if (runno >= 600 && runno <= 699) {scale = 1.000751190; return;}
        if (runno >= 700 && runno <= 799) {scale = 1.000702455; return;}
        if (runno >= 800 && runno <= 899) {scale = 1.000771074; return;}
        if (runno >= 900 && runno <= 999) {scale = 1.000868463; return;}
        if (runno >= 1000) {
          B2ERROR("scale_momenta not ready for this exp,run "
                  << expno << "," << runno);
        }

      } else
        B2ERROR("scale_momenta mode=2 not ready for exp " << expno);
    }
    return;
  }

//=====================================================================

// Scale values are obtained from cosmic data.
//
// [EXP]
// EXP15: 20011205_2146
// EXP  : 20010507_0455
// EXP7 : 20000517_1555
// EXP7 : 20001015_1102
// EXP9 : 20001228_1026
// [MC]
// 20011214_0817
// 20010523_0725
// 20001228_1026
// 20000911_2205
// 20000517_1555
// 20000430_1200

// SE_REMOVE_BELLE_HEADER=1 is used for debug. --> normally 0!!!
//#define SE_REMOVE_BELLE_HEADER 0



////////////////////////////////////////////////////////////////////////
// Begin of scale_error() implementation
////////////////////////////////////////////////////////////////////////

  static int SE_Message_Level; /* 0: none, 1: warning, 2: info */
  static int SE_Reprocess_Version = 0;


  typedef void (*cal_scale_error_func_t)(double scale[5], const double pt, const double tanl);

  struct cal_scale_error_func_set_t {
    cal_scale_error_func_t m_hadMC;
    cal_scale_error_func_t m_cosMC;
    cal_scale_error_func_t m_cosDATA;
  };


  static bool operator==(const cal_scale_error_func_set_t& lhs, const cal_scale_error_func_set_t& rhs)
  {
    return (lhs.m_hadMC == rhs.m_hadMC) && (lhs.m_cosMC == rhs.m_cosMC) && (lhs.m_cosDATA == rhs.m_cosDATA);
  }

  /*static bool operator!=(const cal_scale_error_func_set_t& lhs, const cal_scale_error_func_set_t& rhs)
  {
    return !(lhs == rhs);
  }*/

// Dummy function
  static void null_scale(double[5], double, double)
  {
    return;
  }

  /*****                *****/
  /***** misc functions *****/
  /*****                *****/
  static void
  get_event_id(int* no_exp, int* no_run, int* no_evt, int* no_frm, int* expmc)
  {
    *no_exp = -1, *no_run = -1, *no_evt = -1, *no_frm = -1;

    belle_event* belle_event;
    belle_event = (struct belle_event*)BsGetEnt(BELLE_EVENT, 1, BBS_No_Index);
    if (belle_event) {
      *no_exp = belle_event->m_ExpNo;
      *no_run = belle_event->m_RunNo;
      *no_evt = belle_event->m_EvtNo & 0x0fffffff;
      *no_frm = (unsigned int)belle_event->m_EvtNo >> 28;
      *expmc  = belle_event->m_ExpMC;
    }
  }


  static bool
  is_already_scaled(void)
  {
    Belle::Mdst_event_add_Manager& addmgr = Belle::Mdst_event_add_Manager::get_manager();
    if (addmgr.count() >= 1) {

      if (addmgr[0].flag_error() == 1) {
        if (SE_Message_Level >= 1) B2ERROR("scale_error: already applied");
        return true;
      } else {
        if (SE_Message_Level >= 2) B2ERROR("scale_error: to be applied");
        addmgr[0].flag_error(1);
        return false;
      }

    } else {

      static int first = 1;
      B2ERROR("scale_error: no Mdst_event_add");
      if (first) {
        B2ERROR("scale_error: analysis continues");
        B2ERROR("scale_error: scale_error will not be applied");
        first = 0;
      }
      return true;
    }
  }


  /*****                     *****/
  /***** misc math functions *****/
  /*****                     *****/
  inline double
  vfunc(const double x, const double x1, const double yc, const double a1, const double a2)
  {
    return x < x1 ? (x - x1) * a1 + yc :
           (x - x1) * a2 + yc ;
  }

  inline double
  cupfunc(const double x, const double x1, const double x2, const double yc, const double a1, const double a2)
  {
    return x < x1 ? (x - x1) * a1 + yc :
           x > x2 ? (x - x2) * a2 + yc : yc;
  }

  inline double
  rootfunc(const double x, const double x1, const double x2, const double yc, const double a1, const double a2)
  {
    return x < x1 ? (x - x1) * a1 + yc :
           x > x2 ? (x2 - x1) * a2 + yc : (x - x1) * a2 + yc;
  }

  inline double
  lambdafunc(const double x, const double x1, const double x2, const double yc, const double a1, const double a2, const double a3)
  {
    return x < x1 ? (x - x1) * a1 + yc :
           x < x2 ? (x - x1) * a2 + yc : (x - x2) * a3 + (x2 - x1) * a2 + yc;
  }



  /*****                                    *****/
  /***** scaling parameters for EXP07-EXP23 *****/
  /*****                                    *****/

//
// Scale error for Exp.7-23 Cosmic MC
// old day by somebody
//
  static void
  cal_scale_error_EXP0723_cosmic_mc(double scale[5], const double pt, const double /*tanl*/)
  {
    // pt
    scale[0] = +1.0115E+00 - 3.6966E-02 * pt;
    scale[1] = +9.8369E-01 + 3.2783E-02 * pt;
    scale[2] = +6.8401E-01 + 1.0190E-01 * pt;
    scale[3] = +1.0968E+00 - 3.3011E-02 * pt;
    scale[4] = +1.0992E+00 - 2.7929E-02 * pt;
  }

//
// Scale error for Exp.25-27 Cosmic MC
// old day by somebody
//
  static void
  cal_scale_error_EXP2527_cosmic_mc(double scale[5], const double pt, const double /*tanl*/)
  {
    // pt
    scale[0] = +1.0257E+00 - 0.30671E-01 * pt;
    scale[1] = +1.0503E+00 + 0.97257E-02 * pt;
    scale[2] = +0.70751E+00 + 0.93039E-01 * pt;
    scale[3] = +1.0720E+00 - 0.15976E-01 * pt;
    scale[4] = +1.0530E+00 + 0.63696E-02 * pt;
  }

//
// Scale error for Exp.31 Cosmic MC
// July 09th, 2004, by KUSAKA Akito
//
  static void
  cal_scale_error_EXP31_cosmic_mc(double scale[5], double pt, double /*tanl*/)
  {
    // pt
    scale[0] = 1.0874   - 0.26640E-01 * pt;
    scale[1] = 1.0320  + 0.18869E-01 * pt;
    scale[2] = 0.75302 + 0.89109E-01 * pt;
    scale[3] = 1.1435   - 0.73830E-01 * pt;
    scale[4] = 1.1227   - 0.19112E-01 * pt;
  }

//
// Scale error for Exp.33 Cosmic MC for caseA
// June 21th, 2004, by KUSAKA Akito
// July 07th, 2004, modification on scale[3], by KUSAKA Akito
//
//
// Scale error for Exp.35 Cosmic MC for caseB
// February 3rd, 2010, by Takeo Higuchi
//
  static void
  cal_scale_error_EXP33_cosmic_mc(double scale[5], double pt, double /*tanl*/)
  {
    if (SE_Reprocess_Version == 0) {
      // pt
      scale[0] = 1.1197  - 0.15104E-01 * pt;
      scale[1] = 1.0502  + 0.22138E-01 * pt;
      scale[2] = 0.75267 + 0.85377E-01 * pt;
      // Change to linear fit for robustness (Jul. 07th, 2004. KUSAKA Akito)
      // scale[3] = 1.3726  +(-0.31170 + 0.74074E-01*pt)*pt;
      scale[3] = 1.1608   - 0.43478E-01 * pt;
      scale[4] = 1.0936  + 0.18447E-01 * pt;
    } else {
      // pt
      scale[0] = 0.98971 - 0.12162E-01 * pt;
      scale[1] = 1.0132  + 0.22283E-01 * pt;
      scale[2] = 0.74947 + 0.81233E-01 * pt;
      scale[3] = 1.0601  - 0.54626E-01 * pt;
      scale[4] = 1.0454  - 0.33036E-02 * pt;
    }
  }

//
// Scale error for Exp.35 Cosmic MC for caseA
// June 21th, 2004, by KUSAKA Akito
//
//
// Scale error for Exp.35 Cosmic MC for caseB
// February 3rd, 2010, by Takeo Higuchi
//
  static void
  cal_scale_error_EXP35_cosmic_mc(double scale[5], double pt, double tanl)
  {
    if (SE_Reprocess_Version == 0) {
      // pt
      scale[0] = 1.0835  + 0.79781E-02 * pt;
      scale[1] = 1.0685  + 0.13339E-01 * pt;
      scale[2] = 0.72615 + 0.96936E-01 * pt;
      scale[3] = 1.1298  - 0.35734E-01 * pt;
      scale[4] = 1.0994  + 0.13150E-01 * pt;
    } else {
      // pt
      cal_scale_error_EXP33_cosmic_mc(scale, pt, tanl);
    }
  }

//
// Scale error for Exp.37 Cosmic MC
// July 21st, 2004, by KUSAKA Akito
//
  static void
  cal_scale_error_EXP37_cosmic_mc(double scale[5], double pt, double /*tanl*/)
  {
    // pt
    scale[0] = 1.0694   - 0.19048E-01 * pt;
    scale[1] = 1.0732  + 0.95531E-02 * pt;
    scale[2] = 0.74888 + 0.89957E-01 * pt;
    scale[3] = 1.1107   - 0.57216E-01 * pt;
    scale[4] = 1.1098   - 0.13305E-01 * pt;
  }

//
// Scale error for Exp.39,41 Cosmic MC
// (Made from Exp.39 Cosmic MC. Confirmed for exp39,41)
// April, 20th, 2005, by KUSAKA Akito
//
  static void
  cal_scale_error_EXP3941_cosmic_mc(double scale[5], double pt, double /*tanl*/)
  {
    // pt
    scale[0] = 1.055465  - 0.2863498E-01 * pt;
    scale[1] = 1.005986  + 0.2709512E-01 * pt;
    scale[2] = 0.7459061 + 0.8352030E-01 * pt;
    scale[3] = 1.056039  - 0.6258768E-01 * pt;
    scale[4] = 1.043329  - 0.2975207E-03 * pt;
  }

//
// Scale error for Exp.43 Cosmic MC
// Sep., 12th, 2005, by KUSAKA Akito
//
  static void
  cal_scale_error_EXP43_cosmic_mc(double scale[5], double pt, double /*tanl*/)
  {
    // pt
    scale[0] = 1.013759    - 0.8743831E-02 * pt;
    scale[1] = 1.071626    - 0.1333353E-01 * pt;
    scale[2] = 0.7507483   + 0.8399138E-01 * pt;
    scale[3] = 1.054345    - 0.5644758E-01 * pt;
    scale[4] = 1.020721    + 0.1323117E-01 * pt;
  }

//
// Scale error for Exp.45 Cosmic MC
// May., 27th, 2006, by fmiyuki
//
  static void
  cal_scale_error_EXP45_cosmic_mc(double scale[5], double pt, double /*tanl*/)
  {
    // pt
    scale[0] =  1.011008    - 0.2272887E-01 * pt;
    scale[1] =  1.030603    + 0.8892579E-02 * pt;
    scale[2] =  0.7181793   + 0.9717058E-01 * pt;
    scale[3] =  1.065804    - 0.6852337E-01 * pt;
    scale[4] =  1.085136    - 0.2324515E-01 * pt;
  }

//
// Scale error for Exp.47 Cosmic MC
// May., 27th, 2006, by fmiyuki
//
  static void
  cal_scale_error_EXP47_cosmic_mc(double scale[5], double pt, double /*tanl*/)
  {
    // pt
    scale[0] = 0.9806778    - 0.2010826E-02 * pt;
    scale[1] = 0.9996797    + 0.2633917E-01 * pt;
    scale[2] = 0.7450445    + 0.7637244E-01 * pt;
    scale[3] =  1.084419    - 0.6828102E-01 * pt;
    scale[4] =  1.013550    + 0.1201861E-01 * pt;
  }

//
// Scale error for Exp.49 Cosmic MC
// May., 27th, 2006, by fmiyuki
//
  static void
  cal_scale_error_EXP49_cosmic_mc(double scale[5], double pt, double /*tanl*/)
  {
    // pt
    scale[0] = 1.000635    - 0.1659129E-01 * pt;
    scale[1] = 1.046513    - 0.2994663E-02 * pt;
    scale[2] = 0.7241409   + 0.9558808E-01 * pt;
    scale[3] = 1.062597    - 0.6663921E-01 * pt;
    scale[4] = 1.076486    - 0.2023062E-01 * pt;
  }

//
// Scale error for Exp.51 Cosmic MC
// May 24th, 2007 by higuchit
//
  static void
  cal_scale_error_EXP51_cosmic_mc(double scale[5], double pt, double /*tanl*/)
  {
    // pt
    scale[0] = 0.98242     - 0.71780E-02 * pt;
    scale[1] = 1.0465      + 0.44401E-03 * pt;
    scale[2] = 0.71433     + 0.10176    * pt;
    scale[3] = 1.0875      - 0.80972E-01 * pt;
    scale[4] = 1.0777      - 0.20428E-01 * pt;
  }

//
// Scale error for Exp.53 Cosmic MC
// May 24th, 2007 by higuchit
//
  static void
  cal_scale_error_EXP53_cosmic_mc(double scale[5], double pt, double /*tanl*/)
  {
    // pt
    scale[0] = 0.99832     - 0.17290E-01 * pt;
    scale[1] = 1.0434      + 0.47995E-02 * pt;
    scale[2] = 0.72111     + 0.10093    * pt;
    scale[3] = 1.1022      - 0.87951E-01 * pt;
    scale[4] = 1.0643      - 0.11962E-01 * pt;
  }

//
// Scale error for Exp.55 Cosmic MC
// June 21st, 2007 by higuchit
//
  static void
  cal_scale_error_EXP55_cosmic_mc(double scale[5], double pt, double /*tanl*/)
  {
    // pt
    scale[0] = 0.98871     - 0.88399E-02 * pt;
    scale[1] = 1.0159      + 0.16468E-01 * pt;
    scale[2] = 0.72813     + 0.94737E-01 * pt;
    scale[3] = 1.1060      - 0.88323E-01 * pt;
    scale[4] = 1.0914      - 0.30607E-01 * pt;
  }

//
// Scale error for Exp.61-65 Cosmic MC
// March 4th, 2009 by higuchit
//
  static void
  cal_scale_error_EXP6165_cosmic_mc(double scale[5], double pt, double tanl)
  {
    // pt
    scale[0] = 0.97322     - 0.29003E-02 * pt;
    scale[1] = 0.94704     + 0.44719E-01 * pt;
    scale[2] = 0.73547     + 0.98431E-01 * pt;
    scale[3] = 1.0752      - 0.77818E-01 * pt;
    scale[4] = 1.0759      - 0.27057E-01 * pt;

    // tanl
    scale[0] *= 0.99600     - 0.97573E-02 * tanl;
    scale[1] *= 1.0080      - 0.37122E-01 * tanl;
    scale[2] *= 0.99150     - 0.13390E-01 * tanl;
    scale[3] *= 0.99758     - 0.43508E-01 * tanl;
    scale[4] *= 0.99913     + 0.34211E-01 * tanl;
  }

//
// Scale error for Exp.67 Cosmic MC
// October 8th, 2009 by higuchit
//
  static void
  cal_scale_error_EXP67_cosmic_mc(double scale[5], double pt, double tanl)
  {
    // pt
    scale[0] = 0.99845     - 0.18739E-01 * pt;
    scale[1] = 1.0024      + 0.17833E-01 * pt;
    scale[2] = 0.72369     + 0.96994E-01 * pt;
    scale[3] = 1.0605      - 0.64072E-01 * pt;
    scale[4] = 1.0623      - 0.14320E-01 * pt;

    // tanl
    scale[0] *= 0.99597     - 0.13069E-01 * tanl;
    scale[1] *= 1.0072      - 0.18622E-01 * tanl;
    scale[2] *= 0.99515     - 0.16591E-01 * tanl;
    scale[3] *= 0.99466     - 0.14489E-02 * tanl;
    scale[4] *= 1.0036      - 0.15095E-01 * tanl;
  }


//
// Scale error for Exp.69 Cosmic MC
// January 29th, 2010 by higuchit
// Scale error for Exp.71 Cosmic MC
// May 12th, 2010 by higuchit
//
  static void
  cal_scale_error_EXP6971_cosmic_mc(double scale[5], double pt, double tanl)
  {
    // pt
    scale[0] = 0.96783     - 0.37174E-02 * pt;
    scale[1] = 0.97888     + 0.34689E-01 * pt;
    scale[2] = 0.80870     + 0.68302E-01 * pt;
    scale[3] = 1.0615      - 0.60750E-01 * pt;
    scale[4] = 1.0372      - 0.52332E-03 * pt;

    // tanl
    scale[0] *= 0.99897     + 0.62207E-02 * tanl;
    scale[1] *= 1.0037      + 0.10714E-01 * tanl;
    scale[2] *= 0.99129     - 0.92521E-02 * tanl;
    scale[3] *= 0.99483     - 0.43402E-02 * tanl;
    scale[4] *= 1.0062      + 0.15306E-01 * tanl;
  }


//
// Scale error for Exp.7-23 Cosmic data
// old day by somebody
//
  static void
  cal_scale_error_EXP0723_cosmic_data(double scale[5], const double pt, const double /*tanl*/)
  {
    // pt
    scale[0] = +1.1280E+00 - 7.6839E-03 * pt;
    scale[1] = +1.1368E+00 + 2.8106E-02 * pt;
    scale[2] = +7.6448E-01 + 1.1248E-01 * pt;
    scale[3] = +1.1396E+00 - 3.6738E-02 * pt;
    scale[4] = +1.1766E+00 - 2.7477E-02 * pt;
  }

//
// Scale error for Exp.25-27 Cosmic data
// old day by somebody
//
  static void
  cal_scale_error_EXP2527_cosmic_data(double scale[5], const double pt, const double /*tanl*/)
  {
    // pt
    scale[0] = +1.1002E+00 + 3.9852E-02 * pt;
    scale[1] = rootfunc(pt, 1.0015, 1.6591, 1.0420, -0.31809, 0.17149);
    scale[2] = +8.4706E-01 + 0.8914E-01 * pt;
    scale[3] = +1.1067E+00 + 1.0304E-02 * pt;
    scale[4] = +1.0495E+00 + 3.9923E-02 * pt;
  }

//
// Scale error for Exp.31 Cosmic data
// July 9th, 2004, by KUSAKA Akito
//
  static void
  cal_scale_error_EXP31_cosmic_data(double scale[5], double pt, double /*tanl*/)
  {
    // pt
    scale[0] = 1.1908  + 0.20678E-02 * pt;
    scale[1] = 1.0304  + 0.54826E-01 * pt;
    scale[2] = 1.0806  + 0.33829E-01 * pt;
    scale[3] = 1.1325   - 0.34811E-01 * pt;
    scale[4] = 1.1549   - 0.20974E-01 * pt;
  }

//
// Scale error for Exp.33 Cosmic data for caseA
// June 21th, 2004, by KUSAKA Akito
//
//
// Scale error for Exp.35 Cosmic data for caseB
// February 3rd, 2010, by Takeo Higuchi
//
  static void
  cal_scale_error_EXP33_cosmic_data(double scale[5], double pt, double /*tanl*/)
  {
    if (SE_Reprocess_Version == 0) {
      // pt
      scale[0] = 1.2175  + 0.14290E-01 * pt;
      scale[1] = 1.0362  + 0.80614E-01 * pt;
      scale[2] = 1.0313  + 0.12930E-01 * pt;
      scale[3] = 1.0411  + 0.17319E-01 * pt;
      scale[4] = 1.0445  + 0.27526E-01 * pt;
    } else {
      // pt
      scale[0] = 1.2712  - 0.42098E-01 * pt;
      scale[1] = 1.0634  - 0.13652E-01 * pt;
      scale[2] = 0.95435 + 0.64895E-01 * pt;
      scale[3] = 1.0758  + 0.10778E-01 * pt;
      scale[4] = 1.0892  - 0.75700E-02 * pt;
    }
  }

//
// Scale error for Exp.35 Cosmic data for caseA
// June 21th, 2004, by KUSAKA Akito
// July 07th, 2004, modification on scale[3], by KUSAKA Akito
//
//
// Scale error for Exp.35 Cosmic data for caseB
// February 3rd, 2010, by Takeo Higuchi
//
  static void
  cal_scale_error_EXP35_cosmic_data(double scale[5], double pt, double tanl)
  {
    if (SE_Reprocess_Version == 0) {
      // pt
      scale[0] = 1.1084  + 0.45825E-01 * pt;
      scale[1] = 1.1014  + 0.14211E-01 * pt;
      scale[2] = 0.99716 + 0.52509E-01 * pt;
      scale[3] = 1.1361  - 0.25355E-01 * pt;
      scale[4] = 1.1079  - 0.92563E-02 * pt;
    } else {
      cal_scale_error_EXP33_cosmic_data(scale, pt, tanl);
    }
  }

//
// Scale error for Exp.37 Cosmic data
// July 21st, 2004, by KUSAKA Akito
//
  static void
  cal_scale_error_EXP37_cosmic_data(double scale[5], double pt, double /*tanl*/)
  {
    // pt
    scale[0] = 1.2129  + 0.36787E-01 * pt;
    scale[1] = 1.0750  + 0.32722E-01 * pt;
    scale[2] = 0.98340 + 0.39096E-01 * pt;
    scale[3] = 1.1768   - 0.43894E-01 * pt;
    scale[4] = 1.1275   - 0.12562E-02 * pt;
  }

//
// Scale error for Exp.39,41 Cosmic data
// April 20th, 2004, by KUSAKA Akito
//
  static void
  cal_scale_error_EXP3941_cosmic_data(double scale[5], double pt, double /*tanl*/)
  {
    // pt
    scale[0] = 1.182285  + 0.4142677E-01 * pt;
    scale[1] = 1.090022  + 0.2995044E-01 * pt;
    scale[2] = 0.9581690 + 0.5764173E-01 * pt;
    scale[3] = 1.119173   - 0.2177483E-01 * pt;
    scale[4] = 1.126323   - 0.3786523E-02 * pt;
  }

//
// Scale error for Exp.43 Cosmic data
// Sep., 12th, 2005 by KUSAKA Akito
//
  static void
  cal_scale_error_EXP43_cosmic_data(double scale[5], double pt, double /*tanl*/)
  {
    // pt
    scale[0] = 1.262359  + 0.1411302E-01 * pt;
    scale[1] = 1.051793  + 0.6225422E-01 * pt;
    scale[2] = 0.9668697 + 0.5492099E-01 * pt;
    scale[3] = 1.110838   - 0.2386986E-01 * pt;
    scale[4] = 1.106516  + 0.1239970E-01 * pt;
  }

//
// Scale error for Exp.45+47 Cosmic data
// May., 29th, 2006 by fmiyuki
//
  static void
  cal_scale_error_EXP4547_cosmic_data(double scale[5], double pt, double /*tanl*/)
  {
    // pt 8bin
    scale[0] =  1.164526    + 0.7671143E-01 * pt;
    scale[1] =  1.094987    + 0.2949413E-01 * pt;
    scale[2] =  1.084826    + 0.2513991E-01 * pt;
    scale[3] =  1.099221    - 0.2389658E-02 * pt;
    scale[4] =  1.146892    + 0.2682884E-02 * pt;
  }

//
// Scale error for Exp.49 Cosmic data
// May., 27th, 2006 by fmiyuki
//
  static void
  cal_scale_error_EXP49_cosmic_data(double scale[5], double pt, double /*tanl*/)
  {
    // pt 8bin
    scale[0] =  1.243211    + 0.5776083E-01 * pt;
    scale[1] =  1.209483    - 0.3366023E-01 * pt;
    scale[2] =  1.059087    + 0.4300838E-01 * pt;
    scale[3] =  1.123665    + 0.6342933E-02 * pt;
    scale[4] =  1.208850    - 0.3171053E-01 * pt;
  }

//
// Scale error for Exp.51 Cosmic data
// May 24th, 2007 by higuchit
//
  static void
  cal_scale_error_EXP51_cosmic_data(double scale[5], double pt, double tanl)
  {
    // pt 5bin
    scale[0] =  1.2097      + 0.36177E-01 * pt;
    scale[1] =  1.2218      - 0.84772E-02 * pt;
    scale[2] =  0.97937     + 0.26397E-01 * pt;
    scale[3] =  1.0944      - 0.12745E-02 * pt;
    scale[4] =  1.2140      - 0.56809E-01 * pt;

    // tanl
    scale[0] *= 1.0492      + 0.72972E-01 * tanl;
    scale[1] *= 1.0298      + 0.40625E-02 * tanl;
    scale[2] *= 0.93367     + 0.11969E-01 * tanl;
    scale[3] *= 1.0170      + 0.41039E-01 * tanl;
    scale[4] *= 1.0677      - 0.41684E-01 * tanl;
  }

//
// Scale error for Exp.53 Cosmic data
// May 24th, 2007 by higuchit
//
  static void
  cal_scale_error_EXP53_cosmic_data(double scale[5], double pt, double /*tanl*/)
  {
    // pt 5bin
    scale[0] =  1.2587      + 0.32782E-01 * pt;
    scale[1] =  1.2413      - 0.36535E-01 * pt;
    scale[2] =  0.97465     + 0.22937E-01 * pt;
    scale[3] =  1.1197      - 0.59050E-02 * pt;
    scale[4] =  1.1877      - 0.32893E-01 * pt;
  }

//
// Scale error for Exp.55 Cosmic data
// June 21st, 2007 by higuchit
//
  static void
  cal_scale_error_EXP55_cosmic_data(double scale[5], double pt, double tanl)
  {
    // pt 5bin
    scale[0] =  1.2654      + 0.15660E-01 * pt;
    scale[1] =  1.0278      + 0.81680E-01 * pt;
    scale[2] =  0.94291     + 0.54575E-01 * pt;
    scale[3] =  1.1151      - 0.52387E-02 * pt;
    scale[4] =  1.1020      + 0.10518E-01 * pt;

    // tanl 5bin
    scale[0] *= 1.0334      + 0.10236E-01 * tanl;
    scale[1] *= 1.0346      + 0.11024E-01 * tanl;
    // scale[2] *= 0.91773     -0.52276E-02*tanl;
    scale[3] *= 1.0140      + 0.19030E-01 * tanl;
    scale[4] *= 1.0345      - 0.90291E-01 * tanl;
  }

//
// Scale error for Exp.61-65 Cosmic data
// March 4th, 2009 by higuchit
//
  static void
  cal_scale_error_EXP6165_cosmic_data(double scale[5], double pt, double tanl)
  {
    // pt 5bin
    scale[0] =  1.1184      + 0.55671E-01 * pt;
    scale[1] =  1.1142      + 0.74526E-02 * pt;
    scale[2] =  0.93626     + 0.66670E-01 * pt;
    scale[3] =  1.1003      - 0.11587E-01 * pt;
    scale[4] =  1.1589      - 0.40223E-01 * pt;

    // tanl 5bin
    scale[0] *= 1.0442      - 0.46775E-01 * tanl;
    scale[1] *= 1.0337      + 0.44071E-01 * tanl;
    // scale[2] *= 0.91898     -0.28268E-02*tanl;
    scale[3] *= 1.0037      + 0.29736E-01 * tanl;
    scale[4] *= 1.0378      - 0.73239E-01 * tanl;
  }

//
// Scale error for Exp.67 Cosmic data
// October 8th, 2009 by higuchit
//
  static void
  cal_scale_error_EXP67_cosmic_data(double scale[5], double pt, double tanl)
  {
    // pt 5bin
    scale[0] =  1.1598      + 0.28880E-01 * pt;
    scale[1] =  1.0844      + 0.23384E-01 * pt;
    scale[2] =  0.94566     + 0.53157E-01 * pt;
    scale[3] =  1.1578      - 0.31156E-01 * pt;
    scale[4] =  1.0725      + 0.13714E-01 * pt;

    // tanl 5bin
    scale[0] *= 1.0223      - 0.56165E-02 * tanl;
    scale[1] *= 1.0203      - 0.81857E-02 * tanl;
    // scale[2] *= 0.90812     +0.15679E-01*tanl;
    scale[3] *= 0.99703     + 0.14976E-01 * tanl;
    scale[4] *= 1.0212      + 0.41300E-01 * tanl;
  }


//
// Scale error for Exp.69 Cosmic data
// January 29th, 2010 by higuchit
// Scale error for Exp.71 Cosmic data
// May 12th, 2010 by higuchit
//
  static void
  cal_scale_error_EXP6971_cosmic_data(double scale[5], double pt, double tanl)
  {
    // pt 5bin
    scale[0] =  1.2101      + 0.29496E-01 * pt;
    scale[1] =  1.0723      + 0.13773E-01 * pt;
    scale[2] =  0.90988     + 0.78667E-01 * pt;
    scale[3] =  1.1444      - 0.20780E-01 * pt;
    scale[4] =  1.0962      + 0.83309E-02 * pt;

    // tanl 5bin
    scale[0] *= 1.0207      - 0.17810E-01 * tanl;
    scale[1] *= 1.0272      + 0.61498E-01 * tanl;
    // scale[2] *= 0.91584     +0.16138E-01*tanl;
    scale[3] *= 0.99766     + 0.32434E-01 * tanl;
    scale[4] *= 1.0200      - 0.14935E-01 * tanl;
  }


//
// Scale error for Exp.7-23 Hadron MC
// old day by somebody
//
  static void
  cal_scale_error_EXP0723_hadronic_mc(double scale[5], const double pt, const double tanl)
  {
    // pt
    scale[0] = vfunc(pt, 0.34602, 1.0605, -0.36011, 0.38189E-01);
    scale[1] = 1.0612;
    scale[2] = cupfunc(pt, 0.44599, 1.2989, 0.85646, -0.73968, 0.17425);
    scale[3] = 1.1460 - 0.57101E-01 * pt;
    scale[4] = 1.0859;

    // tanl
    scale[0] *= +1.0122E+00 + 1.3568E-04 * tanl + 1.9856E-02 * tanl * tanl;
    scale[1] *= +1.0002E+00 - 4.5128E-03 * tanl + 1.6211E-02 * tanl * tanl;
    scale[2] *= +9.7051E-01 + 2.6876E-02 * tanl + 8.2365E-02 * tanl * tanl;
    scale[3] *= +9.7198E-01 + 3.8373E-02 * tanl + 4.9111E-02 * tanl * tanl;
    scale[4] *= +9.8880E-01 + 2.2090E-02 * tanl + 2.2701E-02 * tanl * tanl;
  }

//
// Scale error for Exp.25-27 Hadron MC
// old day by somebody
//
  static void
  cal_scale_error_EXP2527_hadronic_mc(double scale[5], const double pt, const double tanl)
  {
    // pt
    scale[0] = 1.092E+00 - 0.86138E-01 * pt;
    scale[1] = 1.0448E+00 - 0.26158E-01 * pt;
    scale[2] = 1.1942E+00 - 1.0025E+00 * pt + 0.85334 * pt * pt - 0.20305 * pt * pt * pt;
    scale[3] = 1.1260E+00 - 0.46048E-01 * pt;
    scale[4] = 1.0378E+00 + 0.5109E-01 * pt;

    // tanl
    scale[0] *= +1.0136E+00 + 0.15293E-01 * tanl;
    scale[1] *= +0.9936E+00 + 0.71983E-02 * tanl + 0.14004E-01 * tanl * tanl;
    scale[2] *= +0.97532E+00 + 0.22718E-01 * tanl + 0.73556E-01 * tanl * tanl;
    scale[3] *= vfunc(tanl, -0.26650, 0.92765, -0.13393, 0.11704);
    scale[4] *= +0.98203E+00 + 0.21612E-01 * tanl + 0.32751E-01 * tanl * tanl;
  }

//
// Scale error for Exp.31 Hadron MC
// July 9th, 2004, by KUSAKA Akito
//
  static void
  cal_scale_error_EXP31_hadronic_mc(double scale[5], double pt, double tanl)
  {
    // pt
    scale[0] = 1.2619  - 0.11170 * pt;
    scale[1] = 1.2263  - 0.24619E-01 * pt;
    scale[2] = vfunc(pt, 1.1432, 0.87555, -0.24211, 0.10876);
    scale[3] = 1.3254  - 0.21162 * pt;
    scale[4] = 1.1609 + 0.73808E-02 * pt;

    // tanl
    scale[3] *= vfunc(tanl, -0.12026, 0.95892, -0.71461E-01, 0.69535E-01);
  }

//
// Scale error for Exp.33, 35 Hadron MC
// June 19th, 2004, by KUSAKA Akito
// July 09th, 2004, modification on scale[2](pt) for low pt.
// July 09th, 2004, modification on scale[3](tanl) for high tanl.
//
  static void
  cal_scale_error_EXP33_hadronic_mc(double scale[5], double pt, double tanl)
  {
    if (SE_Reprocess_Version == 0) {
      // pt
      scale[0] = 1.3083  - 0.10894 * pt;
      scale[1] = 1.2471  - 0.30959E-01 * pt;
      // scale[2] = 0.94594 -0.21216E-01*pt;
      scale[2] = vfunc(pt, 0.79736, 0.90954, -0.45673, 0.37072E-02);
      scale[3] = 1.3307  - 0.17559 * pt;
      scale[4] = 1.2408  - 0.29914E-01 * pt;

      // tanl
      // scale[3] *= 0.95593 + (0.39351E-01 + 0.10450*tanl)*tanl;
      scale[3] *= vfunc(tanl, -0.63864E-01, 0.95936, -0.11035, 0.10057);
    } else {
      // pt
      scale[0] = vfunc(pt, 1.4506, 1.0599, -0.10534, -0.14118);
      scale[1] = lambdafunc(pt, 0.23691, 0.47860, 1.1996, -0.39938E-01, 0.23518, -0.62661E-01);
      scale[2] = lambdafunc(pt, 0.45865, 0.92603, 0.95622, -0.51715, -0.12868, 0.45187E-01);
      scale[3] = 1.2555 - 0.19038 * pt;
      scale[4] = vfunc(pt, 0.68946, 1.1772, 0.15927, -0.79017E-01);
    }
  }

//
// Scale error for Exp.35 Hadron MC
// February 3rd, 2010, by Takeo Higuchi for caseB
//
  static void
  cal_scale_error_EXP35_hadronic_mc(double scale[5], double pt, double tanl)
  {
    cal_scale_error_EXP33_hadronic_mc(scale, pt, tanl);
  }

//
// Scale error for Exp.37 Hadron MC
// July 21st, 2004, by KUSAKA Akito
//
  static void
  cal_scale_error_EXP37_hadronic_mc(double scale[5], double pt, double tanl)
  {
    // pt
    scale[0] = 1.2548  - 0.87786E-01 * pt;
    scale[1] = 1.2562  - 0.89296E-03 * pt;
    scale[2] = vfunc(pt, 0.67905, 0.91705, -0.57837, -0.37276E-02);
    scale[3] = 1.3180  - 0.19459 * pt;
    scale[4] = 1.1652 + 0.33812E-01 * pt;

    // tanl
    scale[3] *= vfunc(tanl, -0.47522E-01, 0.96537, -0.41720E-01, 0.68031E-01);
  }

//
// Scale error for Exp.39,41 Hadron MC
// (Made from Exp.39 Hadron MC. Confirmed for exp39,41)
// April 20th, by KUSAKA Akito
//
  static void
  cal_scale_error_EXP3941_hadronic_mc(double scale[5], double pt, double /*tanl*/)
  {
    // pt
    scale[0] = 1.236096  - 0.1015809 * pt;
    scale[1] = 1.248921  - 0.1130577E-01 * pt;
    scale[2] = vfunc(pt, 0.5129282, 0.9300365, -0.9450958, -0.1157168E-01);
    scale[3] = vfunc(pt, 1.650362,  0.9371865, -0.2142342, 0.7384746E-01);
    scale[4] = vfunc(pt, 0.7497343, 1.204451,   0.1333454, -0.7826934E-01);
  }

//
// Scale error for Exp.43 Hadron MC
// Sep. 12th, 2005 by KUSAKA Akito
//
  static void
  cal_scale_error_EXP43_hadronic_mc(double scale[5], double pt, double /*tanl*/)
  {
    // pt
    scale[0] = 1.240166    - 0.1051634 * pt;
    scale[1] = 1.257180    - 0.3122512E-01 * pt;
    scale[2] = vfunc(pt, 0.5272015, 0.9386514, -0.9648152, -0.1623573E-01);
    scale[3] = 1.280515    - 0.1991213 * pt;
    scale[4] = vfunc(pt, 0.6579201, 1.192409, 0.1197880, -0.5404800E-01);
  }

//
// Scale error for Exp.45 Hadron MC
// May. 27th, 2006 by fmiyuki
//
  static void
  cal_scale_error_EXP45_hadronic_mc(double scale[5], double pt, double tanl)
  {
    // pt
    scale[0] =  1.213823    - .1029683 * pt;
    scale[1] =  1.239279    - .3706657E-01 * pt;
    scale[2] = vfunc(pt, 0.6145123 , 0.8834459 , -.4620622 , 0.2099150E-01);
    scale[3] =  1.253126    - .1884352 * pt;
    scale[4] = vfunc(pt, 0.4928604 , 1.169158 , 0.2063893 , -.5428730E-01);

    // tanl
    scale[2] *= vfunc(tanl, -.1240821 , 0.9274375 , -.8750933E-01 , 0.8611448E-01);
  }

//
// Scale error for Exp.47 Hadron MC
// May. 27th, 2006 by fmiyuki
//
  static void
  cal_scale_error_EXP47_hadronic_mc(double scale[5], double pt, double /*tanl*/)
  {
    // pt
    scale[0] = 1.218633    - .1078999 * pt;
    scale[1] = 1.237288    - .2890434E-01 * pt;
    scale[2] = vfunc(pt, 0.4334312 , 0.9027213 , -.7119852 , 0.1031877E-01);
    scale[3] =  1.252394    - .1835607 * pt;
    scale[4] = vfunc(pt, 0.6194937 , 1.168190 , 0.1285120 , -.5815693E-01);
  }

//
// Scale error for Exp.49 Hadron MC
// May. 27th, 2006 by fmiyuki
//
  static void
  cal_scale_error_EXP49_hadronic_mc(double scale[5], double pt, double /*tanl*/)
  {
    // pt
    scale[0] =  1.217751    - .1075724 * pt;
    scale[1] =  1.233774    - .3122749E-01 * pt;
    scale[2] = vfunc(pt, 0.5276512 , 0.8852152 , -.7025786 , 0.3136450E-01);
    scale[3] =  1.258038    - .1949899 * pt;
    scale[4] = vfunc(pt, 0.5924365 , 1.162905 , 0.9632715E-01, -.6221822E-01);

  }

//
// Scale error for Exp.51 Hadron MC
// May 24th, 2007 by higuchit
//
  static void
  cal_scale_error_EXP51_hadronic_mc(double scale[5], double pt, double tanl)
  {
    // pt
    scale[0] = vfunc(pt, 0.35776, 1.1919, 0.84229E-01, -0.88550E-01);
    scale[1] = vfunc(pt, 0.37833, 1.2394, 0.34089,     -0.54440E-01);
    scale[2] = lambdafunc(pt, 0.87688, 1.4065, 0.86733, -0.23657, 0.21714E-01, 0.20876);
    scale[3] = lambdafunc(pt, 0.39825, 1.0950, 1.2104, 0.11718E-01, -0.21145, -0.89681E-01);
    scale[4] = vfunc(pt, 0.48051, 1.1672, 0.19241,     -0.32273E-01);

    // tanl
    scale[2] *= vfunc(tanl, -0.40697, 0.92948, -0.29453, 0.59416E-01);
  }

//
// Scale error for Exp.53 Hadron MC
// May 24th, 2007 by higuchit
//
  static void
  cal_scale_error_EXP53_hadronic_mc(double scale[5], double pt, double /*tanl*/)
  {
    // pt
    scale[0] = lambdafunc(pt, 0.50681, 1.2150, 1.2266, 0.99662E-01, -0.23508, -0.52268E-01);
    scale[1] = lambdafunc(pt, 0.50787, 1.2308, 1.3108, 0.42334, -0.25502, -0.13522E-01);
    scale[2] = vfunc(pt, 1.2149, 0.88700, -0.15323, 0.97993E-01);
    scale[3] = lambdafunc(pt, 0.45595, 1.1561, 1.2383, 0.17605, -0.34753, -0.97049E-01);
    scale[4] = vfunc(pt, 0.55269, 1.2261, 0.39706, -0.12333);
  }

//
// Scale error for Exp.55 Hadron MC
// June 21st, 2007 by higuchit
//
  static void
  cal_scale_error_EXP55_hadronic_mc(double scale[5], double pt, double /*tanl*/)
  {
    // pt
    scale[0] = vfunc(pt, 0.34925, 1.1780, 0.89069E-01, -0.10090);
    scale[1] = vfunc(pt, 0.35168, 1.2380, 0.41479, -0.63250E-01);
    scale[2] = vfunc(pt, 1.2497, 0.88495, -0.14338, 0.94307E-01);
    scale[3] = lambdafunc(pt, 0.29565, 1.4502, 1.2139, 0.14353, -0.21211, -0.82968E-01);
    scale[4] = vfunc(pt, 1.2496, 1.1216, 0.97174E-02, -0.41166E-01);
  }

//
// Scale error for Exp.61-65 Hadron MC
// March 4th, 2009 by higuchit
//
  static void
  cal_scale_error_EXP6165_hadronic_mc(double scale[5], double pt, double /*tanl*/)
  {
    // pt
    scale[0] = vfunc(pt, 1.4501, 1.0348, -0.12125, -0.27519E-01);
    scale[1] = lambdafunc(pt, 0.48988, 1.5501, 1.2544, 0.21014, -0.10419, 0.74755E-01);
    scale[2] = lambdafunc(pt, 0.44993, 1.3616, 0.93316, -0.58441, -0.30814E-01, 0.90806E-01);
    scale[3] = 1.2385      - 0.17733 * pt;
    scale[4] = vfunc(pt, 0.75590, 1.1726, 0.12749, -0.75183E-01);
  }

//
// Scale error for Exp.67 Hadron MC
// October 8th, 2009 by higuchit
//
  static void
  cal_scale_error_EXP67_hadronic_mc(double scale[5], double pt, double /*tanl*/)
  {
    // pt
    scale[0] = vfunc(pt, 1.0501, 1.0852, -0.14604, -0.66317E-01);
    scale[1] = lambdafunc(pt, 0.37538, 2.5672, 1.2362, 0.14203, -0.58242E-01, 0.28431E-02);
    scale[2] = lambdafunc(pt, 0.52700, 1.3506, 0.90327, -0.55627, 0.13131E-02, 0.11059);
    scale[3] = 1.2577      - 0.19572 * pt;
    scale[4] = vfunc(pt, 0.69484, 1.1636, 0.96169E-01, -0.80421E-01);
  }


//
// Scale error for Exp.69 Hadron MC
// January 29th, 2010 by higuchit
// Scale error for Exp.71 Hadron MC
// May 12th, 2010 by higuchit
//
  static void
  cal_scale_error_EXP6971_hadronic_mc(double scale[5], double pt, double /*tanl*/)
  {
    // pt
    scale[0] = vfunc(pt, 1.7258, 1.0126, -0.12693, 0.31924E-01);
    scale[1] = lambdafunc(pt, 0.37552, 1.2217, 1.2532, 0.37276, -0.77830E-01, -0.31378E-01);
    scale[2] = lambdafunc(pt, 0.40176, 1.1748, 0.95524, -0.72331, -0.53213E-01, 0.90074E-01);
    scale[3] = 1.2644      - 0.19783 * pt;
    scale[4] = vfunc(pt, 0.56934, 1.1649, 0.13098, -0.52232E-01);
  }


  /*****                  *****/
  /***** scale error core *****/
  /*****                  *****/
  static const struct cal_scale_error_func_set_t
    EXP0723_scale = {
    cal_scale_error_EXP0723_hadronic_mc,
    cal_scale_error_EXP0723_cosmic_mc,
    cal_scale_error_EXP0723_cosmic_data,
  },
  EXP2527_scale = {
    cal_scale_error_EXP2527_hadronic_mc,
    cal_scale_error_EXP2527_cosmic_mc,
    cal_scale_error_EXP2527_cosmic_data
  },
  EXP31_scale = {
    cal_scale_error_EXP31_hadronic_mc,
    cal_scale_error_EXP31_cosmic_mc,
    cal_scale_error_EXP31_cosmic_data
  },
  EXP33_scale = {
    cal_scale_error_EXP33_hadronic_mc,
    cal_scale_error_EXP33_cosmic_mc,
    cal_scale_error_EXP33_cosmic_data
  },
  EXP35_scale = {
    cal_scale_error_EXP35_hadronic_mc,
    cal_scale_error_EXP35_cosmic_mc,
    cal_scale_error_EXP35_cosmic_data
  },
  EXP37_scale = {
    cal_scale_error_EXP37_hadronic_mc,
    cal_scale_error_EXP37_cosmic_mc,
    cal_scale_error_EXP37_cosmic_data
  },
  EXP3941_scale = {
    cal_scale_error_EXP3941_hadronic_mc,
    cal_scale_error_EXP3941_cosmic_mc,
    cal_scale_error_EXP3941_cosmic_data
  },
  EXP43_scale = {
    cal_scale_error_EXP43_hadronic_mc,
    cal_scale_error_EXP43_cosmic_mc,
    cal_scale_error_EXP43_cosmic_data
  },
  EXP45_scale = {
    cal_scale_error_EXP45_hadronic_mc,
    cal_scale_error_EXP45_cosmic_mc,
    cal_scale_error_EXP4547_cosmic_data
  },
  EXP47_scale = {
    cal_scale_error_EXP47_hadronic_mc,
    cal_scale_error_EXP47_cosmic_mc,
    cal_scale_error_EXP4547_cosmic_data
  },
  EXP49_scale = {
    cal_scale_error_EXP49_hadronic_mc,
    cal_scale_error_EXP49_cosmic_mc,
    cal_scale_error_EXP49_cosmic_data
  },
  EXP51_scale = {
    cal_scale_error_EXP51_hadronic_mc,
    cal_scale_error_EXP51_cosmic_mc,
    cal_scale_error_EXP51_cosmic_data
  },
  EXP53_scale = {
    cal_scale_error_EXP53_hadronic_mc,
    cal_scale_error_EXP53_cosmic_mc,
    cal_scale_error_EXP53_cosmic_data
  },
  EXP55_scale = {
    cal_scale_error_EXP55_hadronic_mc,
    cal_scale_error_EXP55_cosmic_mc,
    cal_scale_error_EXP55_cosmic_data
  },
  EXP6165_scale = {
    cal_scale_error_EXP6165_hadronic_mc,
    cal_scale_error_EXP6165_cosmic_mc,
    cal_scale_error_EXP6165_cosmic_data
  },
  EXP67_scale = {
    cal_scale_error_EXP67_hadronic_mc,
    cal_scale_error_EXP67_cosmic_mc,
    cal_scale_error_EXP67_cosmic_data
  },
  EXP6971_scale = {
    cal_scale_error_EXP6971_hadronic_mc,
    cal_scale_error_EXP6971_cosmic_mc,
    cal_scale_error_EXP6971_cosmic_data
  },
  DUMMY_scale = {
    null_scale,
    null_scale,
    null_scale
  };



  static cal_scale_error_func_set_t
  get_scale_error_func_for_exprun(const int no_exp, const int /*no_run*/)
  {
    return
      (7 <= no_exp && no_exp <= 23) ? EXP0723_scale :
      (25 <= no_exp && no_exp <= 27) ? EXP2527_scale :
      (31 == no_exp)               ? EXP31_scale   :
      (33 == no_exp)               ? EXP33_scale   :
      (35 == no_exp)               ? EXP35_scale   :
      (37 == no_exp)               ? EXP37_scale   :
      (39 == no_exp || 41 == no_exp) ? EXP3941_scale :
      (43 == no_exp)               ? EXP43_scale   :
      (45 == no_exp)               ? EXP45_scale   :
      (47 == no_exp)               ? EXP47_scale   :
      (49 == no_exp)               ? EXP49_scale   :
      (51 == no_exp)               ? EXP51_scale   :
      (53 == no_exp)               ? EXP53_scale   :
      (55 == no_exp)               ? EXP55_scale   :
      (61 <= no_exp && no_exp <= 65) ? EXP6165_scale :
      (67 == no_exp)               ? EXP67_scale   :
      // ( 69==no_exp || 71==no_exp ) ? EXP6971_scale : DUMMY_scale;
      (69 <= no_exp && 73 >= no_exp) ? EXP6971_scale : DUMMY_scale;
  }

  static void
  cal_scale_error
  (double scale[5],
   const double pt, const double tanl,
   const int expmc, const int no_exp, const int no_run)
  {
    double scale_error_hadronic_mc[5] = {1.0, 1.0, 1.0, 1.0, 1.0};
    double scale_error_cosmic_mc[5]   = {1.0, 1.0, 1.0, 1.0, 1.0};
    double scale_error_cosmic_data[5] = {1.0, 1.0, 1.0, 1.0, 1.0};


    struct cal_scale_error_func_set_t cal_func = get_scale_error_func_for_exprun(no_exp, no_run);
    if (cal_func == DUMMY_scale) {
      B2ERROR("!!!! scale_error: FATAL: FATAL ERROR occured. Abort !!!!");
      // Scale error is not prepared for this EXP number!
      assert(0);
    }


    static bool first = true;
    static int belle_scale_error_debug_mode = 0;
    if (first) {
      first = false;
      char* belle_scale_error_debug = getenv("BELLE_SCALE_ERROR_DEBUG");
      // one of NULL, "none", "cosmic_mc", "cosmic_data", "hadronic_mc"

      if (!belle_scale_error_debug)
        belle_scale_error_debug_mode = 0;
      else if (!strcasecmp(belle_scale_error_debug, "none"))
        belle_scale_error_debug_mode = 1;
      else if (!strcasecmp(belle_scale_error_debug, "cosmic_mc"))
        belle_scale_error_debug_mode = 2;
      else if (!strcasecmp(belle_scale_error_debug, "cosmic_data"))
        belle_scale_error_debug_mode = 3;
      else if (!strcasecmp(belle_scale_error_debug, "hadron_mc") || !strcasecmp(belle_scale_error_debug, "hadronic_mc"))
        belle_scale_error_debug_mode = 4;
      else {
        B2ERROR("BELLE_SCALE_ERROR_DEBUG=" << belle_scale_error_debug << ": bad directive");
        assert(0);
      }

      if (belle_scale_error_debug)
        B2ERROR("BELLE_SCALE_ERROR_DEBUG=" << belle_scale_error_debug << ": applied");
    }


    switch (belle_scale_error_debug_mode) {
      case 0:
        (cal_func.m_hadMC)(scale_error_hadronic_mc, pt, tanl);
        if (expmc == 1) { // Experiment
          if (SE_Message_Level >= 2) B2ERROR("scale_error: info: scale on real data");
          (cal_func.m_cosMC)(scale_error_cosmic_mc, pt, tanl);
          (cal_func.m_cosDATA)(scale_error_cosmic_data, pt, tanl);
        } else {
          if (SE_Message_Level >= 2) B2ERROR("scale_error: info: scale on MC data");
        }
        break;

      case 1:
        //1;
        break;

      case 2:
        (cal_func.m_cosMC)(scale_error_hadronic_mc, pt, tanl);
        break;

      case 3:
        (cal_func.m_cosDATA)(scale_error_hadronic_mc, pt, tanl);
        break;

      case 4:
        (cal_func.m_hadMC)(scale_error_hadronic_mc, pt, tanl);
        break;
    }


    /* compute scaling factor */
    for (int i = 0; i < 5; i++)
      scale[i] = scale_error_hadronic_mc[i] * scale_error_cosmic_data[i] / scale_error_cosmic_mc[i];


    /* message out */
    if (SE_Message_Level >= 2)
      B2ERROR("scale_error: info: hadronic MC: "
              << scale_error_hadronic_mc[0] << " " << scale_error_hadronic_mc[1] << " "
              << scale_error_hadronic_mc[2] << " " << scale_error_hadronic_mc[3] << " "
              << scale_error_hadronic_mc[4]);
    if (SE_Message_Level >= 2)
      B2ERROR("scale_error: info: cosmic MC: "
              << scale_error_cosmic_mc[0] << " " << scale_error_cosmic_mc[1] << " "
              << scale_error_cosmic_mc[2] << " " << scale_error_cosmic_mc[3] << " "
              << scale_error_cosmic_mc[4]);
    if (SE_Message_Level >= 2)
      B2ERROR("scale_error: info: cosmic data: "
              << scale_error_cosmic_data[0] << " " << scale_error_cosmic_data[1] << " "
              << scale_error_cosmic_data[2] << " " << scale_error_cosmic_data[3] << " "
              << scale_error_cosmic_data[4]);

    if (SE_Message_Level >= 2)
      B2ERROR("scale_error: info: final scale: "
              << scale[0] << " " << scale[1] << " " << scale[2] << " "
              << scale[3] << " " << scale[4]);
    /* check parameter limit */
    const double scale_lo = 0.5, scale_hi = 4.0;
    bool too_lo = false, too_hi = false;
    for (int i = 0; i < 5; i++) {
      if (scale[i] < scale_lo) { scale[i] = scale_lo; too_lo = true;}
      if (scale[i] > scale_hi) { scale[i] = scale_hi; too_hi = true;}
    }

    if (SE_Message_Level >= 1 && (too_lo || too_hi)) {
      B2ERROR("scale_error: warning: scale factor beyond the limit: "
              << scale_error_hadronic_mc[0] * scale_error_cosmic_data[0] / scale_error_cosmic_mc[0] << " "
              << scale_error_hadronic_mc[1] * scale_error_cosmic_data[1] / scale_error_cosmic_mc[1] << " "
              << scale_error_hadronic_mc[2] * scale_error_cosmic_data[2] / scale_error_cosmic_mc[2] << " "
              << scale_error_hadronic_mc[3] * scale_error_cosmic_data[3] / scale_error_cosmic_mc[3] << " "
              << scale_error_hadronic_mc[4] * scale_error_cosmic_data[4] / scale_error_cosmic_mc[4]);
    }
  }


  /**
   * The implementation of B2BIIFixMdstModule::scale_error()
   * @return 0 if scale error is properly applied.
   *         1 if scale error is already applied.
   *         2 if scale error for this exp is not available.
   * @sa B2BIIFixMdstModule::scale_error()
   */
  static int scale_error_impl(const int message_level, const int reprocess_version)
  {
    SE_Message_Level = message_level;
    SE_Reprocess_Version = reprocess_version;

    int no_exp{0}, no_run{0}, no_evt{0}, no_frm{0}, expmc{0};
    get_event_id(&no_exp, &no_run, &no_evt, &no_frm, &expmc);

    if (DUMMY_scale == get_scale_error_func_for_exprun(no_exp, no_run)) return 2;
    if (is_already_scaled()) return 1;

    static int first_exp73 = true;
    if (no_exp == 73 && first_exp73) {
      first_exp73 = false;
      B2ERROR(
        "scale_error: warning: scale parameters for exp#71 are tentatively used for exp#73.");
    }

    double scale[5] = { // ... temporary
      1.0, // dr
      1.0, // phi0
      1.0, // kappa
      1.0, // dz
      1.0, // tanl
    };


    // scale error matrices in mdst_trk_fit
    Belle::Mdst_trk_fit_Manager& fitmgr = Belle::Mdst_trk_fit_Manager::get_manager();
    for (std::vector<Belle::Mdst_trk_fit>::iterator it = fitmgr.begin(); it != fitmgr.end(); it++) {
      Belle::Mdst_trk_fit& fit = *it;
      if (fit.helix(2) == 0.) continue;

      const double pt   = 1. / fabs(fit.helix(2));
      const double tanl = fit.helix(4);

      cal_scale_error(scale, pt, tanl, expmc, no_exp, no_run);

      fit.error(0, scale[0]*scale[0]*fit.error(0));
      fit.error(1, scale[1]*scale[0]*fit.error(1));
      fit.error(2, scale[1]*scale[1]*fit.error(2));
      fit.error(3, scale[2]*scale[0]*fit.error(3));
      fit.error(4, scale[2]*scale[1]*fit.error(4));
      fit.error(5, scale[2]*scale[2]*fit.error(5));
      fit.error(6, scale[3]*scale[0]*fit.error(6));
      fit.error(7, scale[3]*scale[1]*fit.error(7));
      fit.error(8, scale[3]*scale[2]*fit.error(8));
      fit.error(9, scale[3]*scale[3]*fit.error(9));
      fit.error(10, scale[4]*scale[0]*fit.error(10));
      fit.error(11, scale[4]*scale[1]*fit.error(11));
      fit.error(12, scale[4]*scale[2]*fit.error(12));
      fit.error(13, scale[4]*scale[3]*fit.error(13));
      fit.error(14, scale[4]*scale[4]*fit.error(14));
    }


    // scale error matrices in mdst_daughters
    Belle::Mdst_vee_daughters_Manager& daumgr = Belle::Mdst_vee_daughters_Manager::get_manager();
    for (std::vector<Belle::Mdst_vee_daughters>::iterator it = daumgr.begin(); it != daumgr.end(); it++) {
      Belle::Mdst_vee_daughters& dau = *it;
      if (dau.helix_p(2) == 0. || dau.helix_m(2) == 0.) continue;

      // positive track
      const double pt_p   = 1. / fabs(dau.helix_p(2));
      const double tanl_p = dau.helix_p(4);

      cal_scale_error(scale, pt_p, tanl_p, expmc, no_exp, no_run);

      dau.error_p(0, scale[0]*scale[0]*dau.error_p(0));
      dau.error_p(1, scale[1]*scale[0]*dau.error_p(1));
      dau.error_p(2, scale[1]*scale[1]*dau.error_p(2));
      dau.error_p(3, scale[2]*scale[0]*dau.error_p(3));
      dau.error_p(4, scale[2]*scale[1]*dau.error_p(4));
      dau.error_p(5, scale[2]*scale[2]*dau.error_p(5));
      dau.error_p(6, scale[3]*scale[0]*dau.error_p(6));
      dau.error_p(7, scale[3]*scale[1]*dau.error_p(7));
      dau.error_p(8, scale[3]*scale[2]*dau.error_p(8));
      dau.error_p(9, scale[3]*scale[3]*dau.error_p(9));
      dau.error_p(10, scale[4]*scale[0]*dau.error_p(10));
      dau.error_p(11, scale[4]*scale[1]*dau.error_p(11));
      dau.error_p(12, scale[4]*scale[2]*dau.error_p(12));
      dau.error_p(13, scale[4]*scale[3]*dau.error_p(13));
      dau.error_p(14, scale[4]*scale[4]*dau.error_p(14));

      // negative track
      const double pt_m   = 1. / fabs(dau.helix_m(2));
      const double tanl_m = dau.helix_m(4);

      cal_scale_error(scale, pt_m, tanl_m, expmc, no_exp, no_run);

      dau.error_m(0, scale[0]*scale[0]*dau.error_m(0));
      dau.error_m(1, scale[1]*scale[0]*dau.error_m(1));
      dau.error_m(2, scale[1]*scale[1]*dau.error_m(2));
      dau.error_m(3, scale[2]*scale[0]*dau.error_m(3));
      dau.error_m(4, scale[2]*scale[1]*dau.error_m(4));
      dau.error_m(5, scale[2]*scale[2]*dau.error_m(5));
      dau.error_m(6, scale[3]*scale[0]*dau.error_m(6));
      dau.error_m(7, scale[3]*scale[1]*dau.error_m(7));
      dau.error_m(8, scale[3]*scale[2]*dau.error_m(8));
      dau.error_m(9, scale[3]*scale[3]*dau.error_m(9));
      dau.error_m(10, scale[4]*scale[0]*dau.error_m(10));
      dau.error_m(11, scale[4]*scale[1]*dau.error_m(11));
      dau.error_m(12, scale[4]*scale[2]*dau.error_m(12));
      dau.error_m(13, scale[4]*scale[3]*dau.error_m(13));
      dau.error_m(14, scale[4]*scale[4]*dau.error_m(14));
    }

    return 0;
  }


  void B2BIIFixMdstModule::scale_error(const int message_level)
  {
    static int result_before = -1;

    const int result = scale_error_impl(message_level, this->m_reprocess_version);

    if (result != result_before) {
      switch (result) {
        case 0: {
          B2INFO(
            "scale_error: info: scale error is properly applied.");
          break;
        }

        case 1: {
          B2ERROR(
            "scale_error: warning: scale error is not applied. Reason: it has been already applied.");
          break;
        }

        case 2: {
          int no_exp, no_run, no_evt, no_frm, expmc;
          get_event_id(&no_exp, &no_run, &no_evt, &no_frm, &expmc);
          B2ERROR(
            "scale_error: error: scale error is not applied. Reason: it is not available for exp " << no_exp << ". Exit! (I'll crash job.)");
          exit(-1);
          break;
        }

        default: assert(0); // Should not be here!
      }

      result_before = result;
    }
  }


////////////////////////////////////////////////////////////////////////
// End of scale_error() implementation
////////////////////////////////////////////////////////////////////////


  int B2BIIFixMdstModule::set_primary_vertex(HepPoint3D& epvtx,
                                             CLHEP::HepSymMatrix& epvtx_err)
  {

    Belle::Evtvtx_primary_vertex_Manager& evtvtx_mgr
      = Belle::Evtvtx_primary_vertex_Manager::get_manager();

    Belle::Evtvtx_trk_Manager& evttrk_mgr = Belle::Evtvtx_trk_Manager::get_manager();

    Belle::Evtvtx_primary_vertex_Manager::iterator itvtx = evtvtx_mgr.begin();
    if (itvtx == evtvtx_mgr.end()) return 4;

    if (!(itvtx->quality() == 2 || itvtx->quality() == 3)) return 3;

    // TKECK fit primary vertex with our kfitter!

    Belle2::analysis::VertexFitKFit kv;
    HepPoint3D pvtx(itvtx->PV_x(), itvtx->PV_y(), itvtx->PV_z());

    kv.setInitialVertex(pvtx);

    unsigned int nchrgd(0);
    for (Belle::Evtvtx_trk_Manager::iterator i = evttrk_mgr.begin(); i != evttrk_mgr.end(); ++i) {

      const Belle::Mdst_charged& p = i->charged();
      if (p.trk().mhyp(2).nhits(3) < 2) continue;
      if (p.trk().mhyp(2).nhits(4) < 2) continue;

      int hyp = 2;
      const HepPoint3D pivot(p.trk().mhyp(hyp).pivot_x(),
                             p.trk().mhyp(hyp).pivot_y(),
                             p.trk().mhyp(hyp).pivot_z());
      CLHEP::HepVector  a(5);
      a[0] = p.trk().mhyp(hyp).helix(0);
      a[1] = p.trk().mhyp(hyp).helix(1);
      a[2] = p.trk().mhyp(hyp).helix(2);
      a[3] = p.trk().mhyp(hyp).helix(3);
      a[4] = p.trk().mhyp(hyp).helix(4);
      CLHEP::HepSymMatrix Ea(5, 0);
      Ea[0][0] = p.trk().mhyp(hyp).error(0);
      Ea[1][0] = p.trk().mhyp(hyp).error(1);
      Ea[1][1] = p.trk().mhyp(hyp).error(2);
      Ea[2][0] = p.trk().mhyp(hyp).error(3);
      Ea[2][1] = p.trk().mhyp(hyp).error(4);
      Ea[2][2] = p.trk().mhyp(hyp).error(5);
      Ea[3][0] = p.trk().mhyp(hyp).error(6);
      Ea[3][1] = p.trk().mhyp(hyp).error(7);
      Ea[3][2] = p.trk().mhyp(hyp).error(8);
      Ea[3][3] = p.trk().mhyp(hyp).error(9);
      Ea[4][0] = p.trk().mhyp(hyp).error(10);
      Ea[4][1] = p.trk().mhyp(hyp).error(11);
      Ea[4][2] = p.trk().mhyp(hyp).error(12);
      Ea[4][3] = p.trk().mhyp(hyp).error(13);
      Ea[4][4] = p.trk().mhyp(hyp).error(14);
      Belle::Helix helix(pivot, a, Ea);

      CLHEP::HepLorentzVector mom;
      CLHEP::HepSymMatrix err(7, 0);
      HepPoint3D pos(0, 0, 0);
      if (pivot.x() != 0. ||
          pivot.y() != 0. ||
          pivot.z() != 0.) {
        helix.pivot(HepPoint3D(0., 0., 0.));
        mom = helix.momentum(0., 0.13957f, pos, err);
      } else {
        mom = helix.momentum(0., 0.13957f, pos, err);
      }

      kv.addTrack(mom, pos, err, p.charge());
      ++nchrgd;
    }

    if (Belle::IpProfile::usable()) {
      // event depend. IP position and error matrix
      HepPoint3D ip_position = Belle::IpProfile::e_position();
      CLHEP::HepSymMatrix ip_err = Belle::IpProfile::e_position_err();
      kv.setIpProfile(ip_position, ip_err);
    }
    // Fit
    unsigned int err(1);
    if (nchrgd > 2 || (nchrgd > 0 && Belle::IpProfile::usable())) err = kv.doFit();
    if (err == 0) {
      epvtx = kv.getVertex();
      epvtx_err = kv.getVertexError();
    }
    return err;

    /*
    kvertexfitter kv;

    HepPoint3D pvtx(itvtx->PV_x(), itvtx->PV_y(), itvtx->PV_z());

    kv.initialVertex(pvtx);

    Ptype ptype_pi_plus("PI+");
    Ptype ptype_pi_minus("PI-");

    unsigned int nchrgd(0);
    for(Belle::Evtvtx_trk_Manager::iterator i = evttrk_mgr.begin();
    i != evttrk_mgr.end(); ++i){
      //      if(!good_charged(i->charged())) continue;

      if(i->charged().trk().mhyp(2).nhits(3) < 2) continue;
      if(i->charged().trk().mhyp(2).nhits(4) < 2) continue;

      Particle tmp(i->charged(),
       (i->charged().charge()>0.0 ? ptype_pi_plus : ptype_pi_minus));
      addTrack2fit(kv, tmp);
      ++nchrgd;
    }

    if(Belle::IpProfile::usable()){

      // event depend. IP position and error matrix
      HepPoint3D ip_position = Belle::IpProfile::e_position();
      HepSymMatrix ip_err = Belle::IpProfile::e_position_err();
      //    HepSymMatrix m_ip_err_b = Belle::IpProfile::e_position_err_b_life_smeared();

      // Add IP profile information
      addBeam2fit(kv, ip_position, ip_err);
    }
    // Fit
    unsigned int err(1);
    if(nchrgd>2||(nchrgd>0&&Belle::IpProfile::usable())) err = kv.fit();
    if(err==0) {
      epvtx = kv.get_vertex();
      epvtx_err = kv.get_err_vertex();
    }
    return err;
    */
  }


//========================================================
// Add Mdst_trk_extra and Mdst_vee_extra
// to Mdst_trk and Mdst_vee2, respectively.
//=======================================================

  extern "C"
  void recsim_mdst_propgt_(float*, float[], float[], float[],
                           float[], float[], int*);

  int
  B2BIIFixMdstModule::add_extra_trk_vee2()
  {
    Belle::Mdst_event_add_Manager& mevtmgr = Belle::Mdst_event_add_Manager::get_manager();
    if (mevtmgr.count() == 0) { // no Belle::Mdst_event_add table
      return -2;
    } else if (mevtmgr.count() == 1) { // no second entity in Belle::Mdst_event_add table
      mevtmgr.add();
    }
    unsigned flag(mevtmgr[1].flag(0));
    if (flag != 0) return -1;  // do nothing if already added
    Belle::Mdst_trk_extra_Manager& teMgr(Belle::Mdst_trk_extra_Manager::get_manager());
    Belle::Mdst_trk_Manager& tMgr(Belle::Mdst_trk_Manager::get_manager());
    Belle::Mdst_charged_Manager& chMgr(Belle::Mdst_charged_Manager::get_manager());
    const int i_ch(chMgr.count() + 1);
    const int i_trk(tMgr.count() + 1);
    for (std::vector<Belle::Mdst_trk_extra>::const_iterator i = teMgr.begin();
         i != teMgr.end(); i++) {
      Belle::Mdst_trk& trk(tMgr.add());
      trk.quality((*i).quality());
      for (int j = 0; j < 5; j++) trk.mhyp(j, (*i).mhyp(j));

      // Commented out by A. Zupanc: see below
      // Belle::Mdst_trk_fit& tf(trk.mhyp(2));
      if (!(trk.quality() & 15u)) {
        B2FATAL("recsim_mdst_propgt_ is missing");

        /*
        Commented by A.Zupanc: the recsim_mdst_propgt_ is missing therefore this part of the code
        can not be executed

              Belle::Mdst_charged& ch(chMgr.add());
              ch.charge((tf.helix(2) >= 0) ? 1. : -1.);

              float pivot[3], helix[5], error[15], helix0[5], error0[15];
              for (int k = 0; k < 3; k++) pivot[k] = tf.pivot(k);
              for (int k = 0; k < 5; k++) helix[k] = tf.helix(k);
              for (int k = 0; k < 15; k++) error[k] = tf.error(k);

              float amass = tf.mass();

              // propagate helix to origin
              int iret;

              //recsim_mdst_propgt_(&amass, pivot, helix,  error,
              //                    helix0, error0, &iret);
              if (iret == 0) {
                ch.px(-sin(helix0[1]) / fabs(helix0[2]));
                ch.py(cos(helix0[1]) / fabs(helix0[2]));
                ch.pz(helix0[4] / fabs(helix0[2]));
              }
              ch.mass(amass);
              ch.trk(trk);
        */
      } else {
        B2ERROR("Warning from B2BIIFixMdst: strange track in Belle::Mdst_trk_extra: quality="
                << trk.quality());
      }
    }


    Belle::Mdst_sim_trk_extra_Manager& steMgr
    (Belle::Mdst_sim_trk_extra_Manager::get_manager());
    Belle::Mdst_sim_trk_Manager& stMgr(Belle::Mdst_sim_trk_Manager::get_manager());
    int ist(0);
    for (std::vector<Belle::Mdst_sim_trk_extra>::const_iterator i = steMgr.begin();
         i != steMgr.end(); i++) {
      Belle::Mdst_sim_trk& strk(stMgr.add());
      int argn = tMgr.count() - teMgr.count() + ist;
      if (argn < tMgr.count()) {
        strk.trk(tMgr[argn]);
      }
      ist++;
      strk.hepevt((*i).hepevt());
      strk.relation(0, (*i).relation(0));
      strk.relation(1, (*i).relation(1));
      strk.relation(2, (*i).relation(2));
    }


    Belle::Mdst_svd_hit_extra_Manager& sheMgr
    (Belle::Mdst_svd_hit_extra_Manager::get_manager());
    Belle::Mdst_svd_hit_Manager& shMgr(Belle::Mdst_svd_hit_Manager::get_manager());
    for (std::vector<Belle::Mdst_svd_hit_extra>::iterator it = sheMgr.begin();
         it != sheMgr.end(); it++) {
      Belle::Mdst_svd_hit_extra& she = *it;
      Belle::Mdst_svd_hit& sh(shMgr.add());
      sh.lsa(she.lsa());
      sh.width(she.width());
      sh.electrons(she.electrons());
      sh.dssd(she.dssd());
      sh.type(she.type());
      int argn = tMgr.count() - teMgr.count() + she.trk().get_ID() - 1;
      if (argn < tMgr.count()) {
        sh.trk(tMgr[argn]);
      }
    }


    Belle::Mdst_vee2_Manager& vMgr(Belle::Mdst_vee2_Manager::get_manager());
    Belle::Mdst_vee2_extra_Manager& veMgr(Belle::Mdst_vee2_extra_Manager::get_manager());
    const int i_vee2(vMgr.count() + 1);
    for (std::vector<Belle::Mdst_vee2_extra>::const_iterator i = veMgr.begin();
         i != veMgr.end(); i++) {
      Belle::Mdst_vee2& vee(vMgr.add());
      vee.kind((*i).kind());

      if ((*i).extra_ID(0)) {
        Belle::Mdst_charged& tmp(chMgr[(*i).extra_ID(0) + i_ch - 2]);
        vee.chgd(0, tmp);
      } else {
        vee.chgd(0, (*i).chgd());
      }

      if ((*i).extra_ID(1)) {
        Belle::Mdst_charged& tmp(chMgr[(*i).extra_ID(1) + i_ch - 2]);
        vee.chgd(1, tmp);
      } else {
        vee.chgd(1, (*i).chgd());
      }

      vee.px((*i).px());
      vee.py((*i).py());
      vee.pz((*i).pz());
      vee.energy((*i).energy());
      vee.vx((*i).vx());
      vee.vy((*i).vy());
      vee.vz((*i).vz());
      vee.z_dist((*i).z_dist());
      vee.chisq((*i).chisq());
      vee.type((*i).type());
      vee.daut((*i).daut());
    }
    teMgr.remove();
    steMgr.remove();
    sheMgr.remove();
    veMgr.remove();
    if (i_ch & 0xffffff00) B2ERROR("Warning from B2BIIFixMdst: overflow! i_ch = " << i_ch);
    if (i_trk & 0xffffff00) B2ERROR("Warning from B2BIIFixMdst: overflow! i_trk = " << i_trk);
    if (i_vee2 & 0xffffff00) B2ERROR("Warning from B2BIIFixMdst: overflow! i_vee2 = " << i_vee2);
    flag = i_ch & 0xff;
    flag |= ((i_trk & 0xff) << 8);
    flag |= ((i_vee2 & 0xff) << 16);
    mevtmgr[1].flag(0, flag);
    return (int)flag;
  }
#if 0
  int
  B2BIIFixMdstModule::remove_extra_trk_vee2()
  {
    Belle::Mdst_event_add_Manager& mevtmgr = Belle::Mdst_event_add_Manager::get_manager();
    if (mevtmgr.count() <= 1) { // no Belle::Mdst_event_add table
      return -2;
    }
    unsigned flag(mevtmgr[1].flag(0));
    if (0 == flag) return -1;  // do nothing if no extra tracks in Belle::Mdst_charged etc.
    const int i_ch(flag & 0xff);
    const int i_trk((flag >> 8) & 0xff);
    const int i_vee2((flag >> 16) & 0xff);
    Belle::Mdst_charged_Manager& chMgr(Belle::Mdst_charged_Manager::get_manager());
    Belle::Mdst_trk_Manager& tMgr(Belle::Mdst_trk_Manager::get_manager());
    Belle::Mdst_vee2_Manager& veeMgr(Belle::Mdst_vee2_Manager::get_manager());
    if (i_ch) {
      while (chMgr.count() >= i_ch) chMgr.remove(chMgr[chMgr.count() - 1]);
    }
    if (i_trk) {
      while (tMgr.count() >= i_trk) tMgr.remove(tMgr[tMgr.count() - 1]);
    }
    if (i_vee2) {
      while (veeMgr.count() >= i_vee2) veeMgr.remove(veeMgr[veeMgr.count() - 1]);
    }
    flag = 0;
    mevtmgr[1].flag(0, flag);
    return 0;
  }
#else
  int
  B2BIIFixMdstModule::remove_extra_trk_vee2()
  {
    Belle::Mdst_event_add_Manager& mevtmgr = Belle::Mdst_event_add_Manager::get_manager();
    if (mevtmgr.count() <= 1) { // no Belle::Mdst_event_add table
      return -2;
    }
    unsigned flag(mevtmgr[1].flag(0));
    if (0 == flag) return -1;  // do nothing if no extra tracks in Belle::Mdst_charged etc.
    const int i_ch(flag & 0xff);
    const int i_trk((flag >> 8) & 0xff);
    const int i_vee2((flag >> 16) & 0xff);
    //Belle::Mdst_charged_Manager////& chMgr(Belle::Mdst_charged_Manager::get_manager());
    Belle::Mdst_trk_Manager& tMgr(Belle::Mdst_trk_Manager::get_manager());
    Belle::Mdst_vee2_Manager& veeMgr(Belle::Mdst_vee2_Manager::get_manager());
    Belle::Mdst_trk_extra_Manager& teMgr(Belle::Mdst_trk_extra_Manager::get_manager());
    Belle::Mdst_vee2_extra_Manager& veeeMgr(Belle::Mdst_vee2_extra_Manager::get_manager());
    if (i_trk) {
      std::vector<int> extra_ID;
      if (teMgr.count()) teMgr.remove();
      for (std::vector<Belle::Mdst_trk>::iterator i = tMgr.begin();
           i != tMgr.end(); i++) {
        if ((*i).get_ID() < i_trk) {
          extra_ID.push_back(0);
          continue;
        }
        if (!((*i).quality() & 16u)) {
          B2ERROR("Warning from B2BIIFixMdst: inconsistency between Belle::Mdst_trk and Belle::Mdst_evet_add"
                 );
        }
        Belle::Mdst_trk_extra& trk(teMgr.add());
        for (int j = 0; j < 5; j++) trk.mhyp(j, (*i).mhyp(j));
        trk.quality((*i).quality());
        extra_ID.push_back(trk.get_ID());
      }
      if (i_vee2) {
        if (veeeMgr.count()) veeeMgr.remove();
        for (std::vector<Belle::Mdst_vee2>::iterator i = veeMgr.begin();
             i != veeMgr.end(); i++) {
          if ((*i).get_ID() < i_vee2) continue;
          if (!((*i).chgd(0).trk().quality() & 16u) && !((*i).chgd(1).trk().quality() & 16u)) {
            B2ERROR("Warning from B2BIIFixMdst: inconsistency between Belle::Mdst_vee2 and Belle::Mdst_evet_add"
                   );
          }
          Belle::Mdst_vee2_extra& vee(veeeMgr.add());
          vee.kind((*i).kind());
          const int extra_id0(extra_ID[(*i).chgd(0).trk_ID() - 1]);
          const int extra_id1(extra_ID[(*i).chgd(1).trk_ID() - 1]);
          if (extra_id0) {
            vee.extra(0, teMgr[extra_id0 - 1]);
          } else {
            vee.chgd((*i).chgd(0));
          }
          if (extra_id1) {
            vee.extra(1, teMgr[extra_id1 - 1]);
          } else {
            if (vee.chgd_ID()) {
              B2ERROR("Warning from B2BIIFixMdst: both tracks of Belle::Mdst_vee2_extra are good track"
                     );
            }
            vee.chgd((*i).chgd(1));
          }

          vee.px((*i).px());
          vee.py((*i).py());
          vee.pz((*i).pz());
          vee.energy((*i).energy());
          vee.vx((*i).vx());
          vee.vy((*i).vy());
          vee.vz((*i).vz());
          vee.z_dist((*i).z_dist());
          vee.chisq((*i).chisq());
          vee.type((*i).type());
          vee.daut((*i).daut());
        }
        int id = BsCouTab(MDST_VEE2);
        while (id >= i_vee2) BsDelEnt(MDST_VEE2, id--);
      }
      int id = BsCouTab(MDST_TRK);
      while (id >= i_trk) BsDelEnt(MDST_TRK, id--);
    }
    if (i_ch) {
      int id = BsCouTab(MDST_CHARGED);
      while (id >= i_ch) BsDelEnt(MDST_CHARGED, id--);
    }
    flag = 0;
    mevtmgr[1].flag(0, flag);
    return 0;
  }

//=======================================================================
//Perform extra-smearing for MC tracks. The relevant code is extracted
//from the module smear_trk originally coded by Marko Staric.
//=======================================================================

  static void scale_err_ms(Belle::Mdst_trk_fit& fit, double scale[]);
  static void smear_trk_ms(Belle::Mdst_trk_fit& fit, double scale[]);
  static void smear_charged();

//==========================
  void B2BIIFixMdstModule::smear_trk()
  {
//==========================
    int expNo = 0, runNo = 0, expmc = 1;
    Belle::Belle_event_Manager& evtMgr = Belle::Belle_event_Manager::get_manager();
    if (evtMgr.count()) {
      expNo  = evtMgr[0].ExpNo();
      runNo  = evtMgr[0].RunNo();
      expmc  = evtMgr[0].ExpMC();
    }
    if (expmc == 1) return; // nothing done for real data

    Belle::Mdst_event_add_Manager& addmgr = Belle::Mdst_event_add_Manager::get_manager();
    if (addmgr.count() == 0) {
      B2ERROR("No Belle::Mdst_event_add table -> kill the job");
      exit(-1);
    } else if (addmgr.count() >= 2) {
      if (addmgr[1].flag(1) != 0) return;  //do nothing if already smeared
    }

    int flag_err = addmgr[0].flag_error();

    static bool start = true;
    if (start) {
      start = false;
      B2ERROR("smear_trk: MC events -> track smearing is ON\n");
    }

    double scale_mc[5] = {1, 1, 1, 1, 1};
    double scale_rd[5] = {1, 1, 1, 1, 1};
    double scale[5];

    Belle::Mdst_trk_fit_Manager& fitmgr = Belle::Mdst_trk_fit_Manager::get_manager();
    for (std::vector<Belle::Mdst_trk_fit>::iterator it = fitmgr.begin(); it != fitmgr.end(); it++) {
      Belle::Mdst_trk_fit& fit = *it;
      if (fit.helix(2) == 0.) continue;

      double pt = 1. / fabs(fit.helix(2));
      double tanl = fit.helix(4);

      cal_scale_error(scale_mc, pt, tanl, 0, expNo, runNo);
      cal_scale_error(scale_rd, pt, tanl, 1, expNo, runNo);
      for (int i = 0; i < 5; i++) scale[i] = scale_rd[i] / scale_mc[i];

      if (flag_err == 0) scale_err_ms(fit, scale_mc);
      smear_trk_ms(fit, scale);
      scale_err_ms(fit, scale);
    }

    if (m_smear_trk == 2) smear_charged();

    //set flag indicating already-err-scaled
    addmgr[0].flag_error(1);

    //set flag indicating already-smeared
    if (addmgr.count() == 1) {
      Belle::Mdst_event_add& meadd = addmgr.add();
      meadd.flag(1, 1);
    } else if (addmgr.count() >= 2) {
      addmgr[1].flag(1, 1);
    }

  }

//====================================================
  void scale_err_ms(Belle::Mdst_trk_fit& fit, double scale[])
  {
//====================================================
    fit.error(0, scale[0]*scale[0]*fit.error(0));
    fit.error(1, scale[1]*scale[0]*fit.error(1));
    fit.error(2, scale[1]*scale[1]*fit.error(2));
    fit.error(3, scale[2]*scale[0]*fit.error(3));
    fit.error(4, scale[2]*scale[1]*fit.error(4));
    fit.error(5, scale[2]*scale[2]*fit.error(5));
    fit.error(6, scale[3]*scale[0]*fit.error(6));
    fit.error(7, scale[3]*scale[1]*fit.error(7));
    fit.error(8, scale[3]*scale[2]*fit.error(8));
    fit.error(9, scale[3]*scale[3]*fit.error(9));
    fit.error(10, scale[4]*scale[0]*fit.error(10));
    fit.error(11, scale[4]*scale[1]*fit.error(11));
    fit.error(12, scale[4]*scale[2]*fit.error(12));
    fit.error(13, scale[4]*scale[3]*fit.error(13));
    fit.error(14, scale[4]*scale[4]*fit.error(14));

  }

//====================================================
  void smear_trk_ms(Belle::Mdst_trk_fit& fit, double scale[])
  {
//====================================================
    const int n = 5;
    double a[n][n];
    int k = 0;
    for (int i = 0; i < n; i++) {
      for (int j = 0; j <= i; j++) {
        a[i][j] = fit.error(k) * (scale[i] * scale[j] - 1.0);
        if (i != j) a[j][i] = a[i][j];
        k++;
      }
    }

    double u[n][n];
    double s;
    //int pozdef = 1;
    for (int j = 0; j < n; j++) {
      s = a[j][j];
      for (int k2 = 0; k2 < j; k2++) s -= u[j][k2] * u[j][k2];
      if (s > 0) {
        u[j][j] = sqrt(s);
        for (int i = j + 1; i < n; i++) {
          s = a[i][j];
          for (int k3 = 0; k3 < j; k3++) s -= u[i][k3] * u[j][k3];
          u[i][j] = s / u[j][j];
        }
      } else {
        for (int i = j; i < n; i++) u[i][j] = 0;
        //pozdef = 0;
      }
    }

    double g[n];
    for (int i = 0; i < n; i++) g[i] = gRandom->Gaus();
    double x[n];
    for (int i = 0; i < n; i++) {
      x[i] = 0;
      if (u[i][i] > 0) {
        for (int j = 0; j <= i; j++) x[i] += u[i][j] * g[j];
      }
    }
    for (int i = 0; i < n; i++) fit.helix(i, fit.helix(i) + x[i]);

  }

//====================
  void smear_charged()
  {
//====================
    Belle::Mdst_charged_Manager& chgmgr = Belle::Mdst_charged_Manager::get_manager();
    for (std::vector<Belle::Mdst_charged>::iterator it = chgmgr.begin(); it != chgmgr.end(); it++) {
      Belle::Mdst_charged& c = *it;
      Belle::Mdst_trk_fit& t = c.trk().mhyp(2);

      double kp = std::abs(t.helix(2));
      kp = std::max(kp, 1.e-10);
      kp = std::min(kp,  1.e10);
      c.px(-sin(t.helix(1)) / kp);
      c.py(cos(t.helix(1)) / kp);
      c.pz(t.helix(4) / kp);
    }

  }

#endif
} // namespace Belle
