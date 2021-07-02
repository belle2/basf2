/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//
// $Id: B2BIIFixMdst_tof.cc 10002 2007-02-26 06:56:17Z katayama $
//
// $Log$
//
// Revision 2.0 2015/03/11 tkeck
// Conversion to Belle II
//
// Revision 1.6  2002/06/22 01:14:24  katayama
// For exp. 9. (same as exp. 11 and 13)
//
// Revision 1.5  2002/06/20 23:41:50  katayama
// New tof for exp. 7
//
// Revision 1.4  2002/06/10 12:28:53  katayama
// Better exp. 15 from Mike san. Also work for exp. 11
//
// Revision 1.3  2002/05/28 23:46:08  katayama
// New correction for reprocessed exp. 13 from Peters/Jones san
//
// Revision 1.2  2002/05/04 23:54:35  hitoshi
// Updated correctios for exp15. Also added corrections for exp17.
//
// Revision 1.1  2002/04/26 01:25:07  katayama
// New correction from Perers san
//
//
//

#include <b2bii/modules/B2BIIMdstInput/B2BIIFixMdstModule.h>
#include "belle_legacy/panther/panther.h"

#include <cmath>
#include <cfloat>

#include "TMath.h"
#include "TVector3.h"

#include "CLHEP/Vector/ThreeVector.h"

#include "belle_legacy/tables/mdst.h"
#include "belle_legacy/tables/belletdf.h"

namespace Belle2 {


//===============================================================
  void B2BIIFixMdstModule::shift_tof(const int mode)
  {
//===============================================================
// Shifts tof times to account for residuals.
// Based on scale_momenta code
//======================================================
//Check existence of belle_event

    Belle::Belle_event_Manager& evtmgr = Belle::Belle_event_Manager::get_manager();
    if (evtmgr.count() == 0) return;  //do nothing if no Belle::Belle_Event

    Belle::Mdst_event_add_Manager& addmgr = Belle::Mdst_event_add_Manager::get_manager();
    if (addmgr.count() == 0) return;  //do nothing if no Belle::Mdst_Event_Add
    if (addmgr[0].flag(4) != 0) return; //do nothing if already shifted

//check mode
    if (mode < 0 || mode > 2) {
      B2ERROR("shift_tof: invalid mode specified;");
      B2ERROR("mode must be 0, 1, 2");
      return;
    }

    int expmc = evtmgr[0].ExpMC();
    int expno = evtmgr[0].ExpNo();
    int runno = evtmgr[0].RunNo();

    if (expmc == 2) return;  //for now, apply no shift to MC

    addmgr[0].flag(4, 1);    //set flag

    // Loop over all charged tracks

    Belle::Mdst_charged_Manager& ChgMgr = Belle::Mdst_charged_Manager::get_manager();
    for (std::vector<Belle::Mdst_charged>::iterator it = ChgMgr.begin();
         it != ChgMgr.end(); ++it) {
      Belle::Mdst_charged& Chg = *it;
      if (Chg) {
        // Get mdst_tof table for the track
        Belle::Mdst_tof& Tof = Chg.tof();
        if (Tof) {
          // Get momentum and charge of track
          TVector3 Mom(Chg.px(), Chg.py(), Chg.pz());
          double pmom = Mom.Mag();
          double sgn = Chg.charge();
          // Loop over mass hypotheses
          for (int im = 0; im < 5; im++) {
            double shift = 0.;
            shift_tof_set(expno, runno, mode, im, pmom, sgn, shift);
            if (fabs(shift) < .005) continue;
            double ot = Tof.tof_exp(im);              //old t (expected)
            double odt = ot - Tof.tof();              //old dt
            double opid = Tof.pid(im);                //old pid
//         double ocl=Tof.cl(im);                    //old cl
            int ndf = (int)(Tof.ndf(im) + .001);      //ndf
            if (opid > 0. && opid < 1.) {
              double ct = ot + shift;                //corr t
              double cdt = ct - Tof.tof();           //corr dt
              double err = fabs(odt) / sqrt(-2.*log(opid)); //est error
              double cch = pow(cdt / err, 2);        //corr chisq
              double cpid = exp(-.5 * cch);          //corr pid
              double ccl = TMath::Prob(cch, ndf);    //corr cl
              Tof.tof_exp(im, ct);
              Tof.pid(im, cpid);
              Tof.cl(im, ccl);
            }
          }
        }
      }
    }
  }
  void B2BIIFixMdstModule::shift_tof_set(const int expno, const int runno,
                                         const int mode, const int im,
                                         const double pmom, const double sgn,
                                         double& shift)
  {
    static const double m[5] = {.000511, .10566, .13957, .49368, .93827};
    shift = 0.;
    double beta = pmom / sqrt(pmom * pmom + m[im] * m[im]);
    switch (expno) {
      case 7:
        if (mode == 1) {
          switch (im) {
            case 2:
              if (runno < 536) shift = 0.;
              else if (runno < 1440) shift = .5054 - .5216 * std::min(beta, .955);
              else shift = .8321 - .8648 * std::min(beta, .96);
              break;
            case 3:
              if (runno < 536) shift = -.0414 * exp(-pow(beta - .538, 2) / .0569);
              else if (runno < 1440) shift = -12.3 * exp(-pow(beta + .288, 2) / .1197);
              else shift = 0.;
              break;
            case 4:
              if (sgn > 0.) {
                shift = -.876 * exp(-pow(beta + .1818, 2) / .1947);
              } else {
                if (runno < 1440)
                  shift = .01 - .1028 * exp(-pow(beta - .4454, 2) / .00272);
                else shift = .01 - .064 * exp(-pow(beta - .4273, 2) / .00317);
              }
              break;
            default:
              break;
          } // end switch(im)
        } // end if(mode)
        break;
      case  9:
      case 11:
      case 13:
        if (mode == 1) {
          switch (im) {
            case 2:
              shift = 1.089 - 1.131 * std::min(beta, .955);
              break;
            case 4:
              if (sgn > 0.) shift = -.876 * exp(-pow(beta + .1818, 2) / .1947);
              else shift = .01 - .1028 * exp(-pow(beta - .4454, 2) / .00272);
              break;
            default:
              break;
          } // end switch(im)
        } // end if(mode)
        break;
      case 15:
        if (mode == 1) {
          switch (im) {
            case 2:
//      if(sgn>0.) shift=-4.73+9.853*beta-5.139*beta*beta;
//  The following stmt replaced the one above on 6/7/02 to remove the
// -16 ps value at beta=1.  MWP
              if (sgn > 0.) shift = -.0183 * exp(-pow(beta - .911, 2) / .00161);
              break;
            case 3:
              shift = -6.6 * exp(-beta / .1);
              break;
            case 4:
              if (sgn > 0.) shift = -.736 * exp(-pow(beta + .04158, 2) / .119);
              else shift = .02 - .1475 * exp(-pow(beta - .4267, 2) / .00249);
              break;
            default:
              break;
          }  // end switch(im)
        }  // end if(mode)
        break;
      case 17:
      case 19:                 //same corrections for Exps 17 and 19
        if (mode == 1) {
          switch (im) {
            case 4:
              if (sgn > 0.) shift = -.3259 * exp(-pow(beta - .1042, 2) / .0817);
              else shift = .02 - .1475 * exp(-pow(beta - .4267, 2) / .00249);
              break;
            default:
              break;
          } // end switch(im)
        } // end if(mode)
      default:
        break;
    }  // end switch(expno)
  }
} // namespace Belle
