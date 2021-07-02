/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//
// $Id: B2BIIFixMdst_ecl.cc 11174 2010-09-29 06:29:44Z hitoshi $
//
// $Log$
//
// Revision 2.0 2015/03/11 tkeck
// Conversion to Belle II
//
// Revision 1.30  2005/07/28 09:56:02  hitoshi
// for exp43 run700-1149 (by Miyabayashi)
//
// Revision 1.29  2005/07/02 08:04:37  hitoshi
// for exp43 run300-699 (by Miyabayashi)
//
// Revision 1.28  2005/06/06 04:28:01  katayama
// New for exp. 43 run - 300 from Miyabayasi and Senyo san
//
// Revision 1.27  2005/04/17 09:38:42  hitoshi
// for Exp41 r1200-end (by Miyabayashi)
//
// Revision 1.26  2005/04/15 08:29:56  hitoshi
// for exp41 r800-1199 (by Miyabayashi)
//
// Revision 1.25  2005/03/29 16:24:45  hitoshi
// for Exp41 runno<=799 (by Miyabayashi)
//
// Revision 1.24  2005/03/10 15:09:52  hitoshi
// for Exp39 (by Miyabayashi san)
//
// Revision 1.23  2004/07/14 02:13:57  katayama
// for exp. 37-1913
//
// Revision 1.22  2004/07/10 00:24:04  katayama
// exp. 37 run < 1400
//
// Revision 1.21  2004/06/25 05:22:40  hitoshi
// *** empty log message ***
//
// Revision 1.20  2004/06/19 17:09:13  hitoshi
// for exp35 (by Miyabayashi san)
//
// Revision 1.19  2004/06/07 05:46:28  hitoshi
// for exp33. also mass window of make_pi0 is widened (by Miyabaya).
//
// Revision 1.18  2003/07/11 09:13:44  hitoshi
// added a comment (by Miyabayashi; no change in function itself).
//
// Revision 1.17  2003/06/30 22:16:29  hitoshi
// update for exp27 (by Miyabayashi, Sanjay, Senyo).
//
// Revision 1.16  2003/05/31 09:22:54  hitoshi
// updae for exp25 (ny Miyabayashi and Senyo).
//
// Revision 1.15  2003/03/19 05:02:53  hitoshi
// minor change (by Miyabayashi).
//
// Revision 1.14  2003/02/19 10:37:29  hitoshi
// incorporated an asymmetric pi0 resol function (by Miyabayashi).
//
// Revision 1.13  2002/12/28 23:10:08  katayama
// ecl and benergy for exp. 21 and 32 from Sanjay and Miyavayashi san
//
// Revision 1.12  2002/07/03 05:18:18  hitoshi
// ECL corrections for exp19 run900 - 1643 (by Miyabayashi san).
//
// Revision 1.11  2002/06/22 07:37:58  katayama
// From miyabayashi san. exp. 9 and exp. 19 ~899
//
// Revision 1.10  2002/06/19 12:10:36  katayama
// New for exp. 7 from Senyo and Miyabayashi sans
//
// Revision 1.9  2002/06/10 17:33:48  hitoshi
// added new corections for e11 (by Miyabayashi).
//
// Revision 1.8  2002/06/09 15:28:29  hitoshi
// added corrections for run<460 in e19.
//
// Revision 1.7  2002/06/01 12:09:32  hitoshi
// added corrections for runs1-299 in exp19 (by Miyabayashi).
//
// Revision 1.6  2002/05/24 08:03:30  hitoshi
// added corrections for exp13 reprocessed data (by Miyabayashi san).
//
// Revision 1.5  2002/05/09 08:57:55  hitoshi
// added corrections for exp17 (by Miyabayashi san).
//
// Revision 1.4  2002/04/23 04:57:46  katayama
// New correction from Miyabayashi san
//
// Revision 1.3  2002/04/19 07:24:39  katayama
// Added pi0/vertex, calibdedx
//
// Revision 1.2  2002/04/05 01:19:32  katayama
// templates for ecl_primary_vertex
//
// Revision 1.1  2002/03/13 02:55:20  katayama
// First version
//
//
#include <iostream>
#include <cmath>
#include <cfloat>

#include <b2bii/modules/B2BIIMdstInput/B2BIIFixMdstModule.h>
#include "belle_legacy/panther/panther.h"

#include "belle_legacy/tables/mdst.h"
#include "belle_legacy/tables/belletdf.h"

#include "CLHEP/Vector/ThreeVector.h"
#include "CLHEP/Vector/LorentzVector.h"
#include "CLHEP/Matrix/Vector.h"
#include "CLHEP/Matrix/Matrix.h"

namespace Belle2 {


//=====================================================================
//**** correct_ecl.cc ***** created 2001/June/11th *****
// Correct photon energy according to pi0 mass peak
// analysis.
// Version1.2.1 2002/Feb./20th Kenkichi Miya.
// Exp.15 proper correction curve and version control sequence
// were installed. option=0 and version=2 are recommended.
// Version1.2 2001/Nov./29th Kenkichi Miya.
// About exp.15, the correction curve of exp.13 will be applied
// for a while. 2001/Nov./29th.
// version control option added 2001/Nov./29th.
// The correction curve for exp.13 was implemented 2001/Nov./20th.
// Mdst_gamma and Mdst_ecl's energy and errors are corrected.
// input option was added 2001/July/23rd.
//******************************************************

//==================================
// The function giving correction factor.
// Correcponding Data/MC so that energy in data should be divided by this.
//==================================
//Original definition.
//static double ecl_adhoc_corr( int Exp, int Run, double Energy, double)
//Modified 20081222
  /** The function giving correction factor.
   *  Corresponding Data/MC so that energy in data should be divided by this.
   */
  static double ecl_adhoc_corr(int Exp, int Run, int iflag05th,
                               double Energy, double)
  {
    double return_value;

    // Default.
    return_value = 1.0;

    double x = std::log10(Energy * 1000.0);
    switch (Exp) {
      case 7:
        //if( x < 2.5 )
        //{
        //   return_value = 0.85445 + 0.89162e-1*x
        //      -0.96202e-2*x*x -0.10944e-2*x*x*x;
        //}
        // New curve for b20020416 processed mdst.
        if (x < 2.6) {
          return_value = 0.85878 + 0.81947e-1 * x - 0.99708e-2 * x * x
                         - 0.28161e-3 * x * x * x;
        }
        break;
      case 9:
        //if( x < 3.0 )
        //{
        //   return_value = 0.83416 + 0.93534e-1*x
        //      -0.71632e-2*x*x -0.19072e-2*x*x*x;
        //}
        // New curve for b20020416 processed mdst.
        if (x < 3.2) {
          return_value = 0.86582 + 0.63194e-1 * x - 0.59391e-2 * x * x
                         + 0.17727e-2 * x * x * x - 0.62325e-3 * x * x * x * x;
        }
        break;
      case 11:
        // Proper curve for b20010507 processed mdst.
        //if( x < 3.0 )
        //{
        //   return_value = 0.84927 + 0.86308e-1*x
        //      -0.93898e-2*x*x -0.10557e-2*x*x*x;
        //}
        //
        // Proper curve for b20020416 processed mdst.
        if (x < 3.7) {
          if (x < 3.0) {
            return_value = 0.88989 + 0.41082e-1 * x
                           - 0.21919e-2 * x * x + 0.27116e-2 * x * x * x - 0.89113e-3 * x * x * x * x;
          } else { // Linear extrapolation to unity.
            return_value = 0.994441 + 4.90176e-3 * (x - 3.0);
          }
        }
        break;
      case 13:
        // Proper curve for b20010507 processed mdst.
        //if( x < 3.0 )
        //{
        //return_value = 0.78138 + 0.17899*x
        // -0.48821e-1*x*x +0.43692e-2*x*x*x;
        //}
        //
        // Proper curve for b20020416 processed mdst.
        if (x < 3.2) {
          return_value = 0.87434 + 0.62145e-1 * x
                         - 0.29691e-2 * x * x - 0.15843e-2 * x * x * x + 0.86858e-4 * x * x * x * x;
        }
        break;
      case 15:
        // Proper curve for b20020125 processed mdst.
        //if( x < 3.0 )
        //{
        //   return_value = 0.86858 + 0.75587e-1*x
        //      -0.11150e-1*x*x -0.21454e-4*x*x*x;
        //}
        //else if( x < 3.67 ) // Lenear extrapolation to be unity.
        //{
        //   return_value = 0.994412 + 8.11e-3*( x - 3.0 );
        //}
        // Proper curve for b20020416 processed mdst.
        if (x < 3.2) {
          return_value = 0.83073 + 0.10137 * x
                         - 0.59946e-2 * x * x - 0.56516e-2 * x * x * x + 0.87225e-3 * x * x * x * x;
        }

        break;
      case 17:
        // Proper curve for b20020416 processed mdst.
        if (x < 3.0) {
          return_value = 0.89260 + 0.54731e-1 * x
                         - 0.25736e-2 * x * x - 0.16493e-2 * x * x * x + 0.1032e-3 * x * x * x * x;
        } else if (x < 3.5) {
          return_value = 0.997459 + 0.0059039 * (x - 3.0);
        }
        break;
      case 19:
        // Correction curve obtained by Run001-299 HadronB
        // processed by b20020416.
        if (Run < 300) {
          if (x < 3.0) {
            return_value = 0.86432 + 0.87554e-1 * x
                           - 0.84182e-2 * x * x - 0.39880e-2 * x * x * x + 0.69435e-3 * x * x * x * x;
          }
        }
        if (299 < Run && Run < 900) { // For Run300-899 or later by b20020416.
          if (x < 2.9) {
            return_value = 0.92082 + 0.45896e-1 * x
                           - 0.68067e-2 * x * x + 0.94055e-3 * x * x * x - 0.27717e-3 * x * x * x * x;
          }
        }
        if (899 < Run) { // Till the end.
          if (x < 3.1) {
            return_value = 0.85154 + 0.97812e-1 * x
                           - 0.85774e-2 * x * x - 0.59092e-2 * x * x * x + 0.1121e-2 * x * x * x * x;
          }
        }
        break;
      case 21:
        // Proper curve for b20021205 processed mdst.
        if (x < 3.0) {
          return_value = 0.84940 + 0.86266e-1 * x
                         - 0.82204e-2 * x * x - 0.12912e-2 * x * x * x;
        }
        break;
      case 23:
        // Proper curve for b20021205 processed mdst.
        if (x < 3.1) {
          return_value = 0.85049 + 0.85418e-1 * x
                         - 0.45747e-2 * x * x - 0.40081e-2 * x * x * x + 0.52113e-3 * x * x * x * x;
        }
        break;
      case 25:
        if (x < 3.0) {
          return_value = 0.87001 + 0.73693e-1 * x
                         - 0.80094e-2 * x * x - 0.78527e-3 * x * x * x + 0.25888e-4 * x * x * x * x;
        }
        break;
      case 27: // It turned out no signif. change through all the runs.
        if (x < 2.85) {
          return_value = 0.90051 + 0.56094e-1 * x
                         - 0.14842e-1 * x * x + 0.55555e-2 * x * x * x - 0.10378e-2 * x * x * x * x;
        }
        break;
      case 31:
        if (iflag05th == 1) { // For 0.5MeV threshold.
          // For b20040429.
          if (x < 2.95) {
            return_value = 0.80295 + 0.13395 * x
                           - 0.10773e-1 * x * x - 0.85861e-2 * x * x * x + 0.15331e-2 * x * x * x * x;
          }
        } else { // For theta-dep. threshold.
          if (x < 3.25) {
            return_value = 0.57169 + 0.32548 * x
                           - 0.41157e-1 * x * x - 0.21971e-1 * x * x * x + 0.50114e-2 * x * x * x * x;
          }
        }
        break;
      case 33:
        if (iflag05th == 1) { // For 0.5MeV threshold.
          // For b20040429.
          if (x < 2.64) {
            return_value = 0.81153 + 0.10847 * x
                           - 0.24652e-2 * x * x - 0.54738e-2 * x * x * x + 0.41243e-3 * x * x * x * x;
          }
        } else { // For theta-dep. threshold, b20090127.
          if (x < 3.15) {
            return_value = 0.59869 + 0.29276 * x
                           - 0.15849e-1 * x * x - 0.31322e-1 * x * x * x + 0.62491e-2 * x * x * x * x;
          }
        }
        break;
      case 35:
        if (iflag05th == 1) { // For 0.5MeV threshold.
          // For b20040429.
          if (x < 2.54) {
            return_value = 0.83528 + 0.10402 * x
                           - 0.62047e-2 * x * x - 0.62411e-2 * x * x * x + 0.10312e-2 * x * x * x * x;
          }
        } else { // For theta-dep. threshold, b20090127.
          if (x < 3.2) {
            return_value =  0.58155 + 0.30642 * x
                            - 0.16981e-1 * x * x - 0.32609e-1 * x * x * x + 0.64874e-2 * x * x * x * x;
          }
        }
        break;
      case 37:
        if (iflag05th == 1) { // For 0.5MeV threshold.
          if (x < 2.5) {
            return_value = 0.83706 + 0.10726 * x
                           - 0.62423e-2 * x * x - 0.42425e-2 * x * x * x;
          }
        } else { // For theta-dep. threshold.
          if (x < 3.15) {
            return_value =  0.58801 + 0.30569 * x
                            - 0.18832e-1 * x * x - 0.32116e-1 * x * x * x + 0.64899e-2 * x * x * x * x;
          }
        }
        break;
      case 39:
        if (iflag05th == 1) { // For 0.5MeV threshold.
          if (x < 3.0) {
            if (x < 2.8) {
              return_value = 0.80827 + 0.095353 * x
                             - 0.14818e-2 * x * x - 0.23854e-2 * x * x * x - 0.22454e-3 * x * x * x * x;
            } else {
              return_value = 0.0112468 * (x - 2.8) + 0.997847;
            }
          }
        } else { // For theta-dep. threshold.
          if (x < 3.25) {
            return_value = 0.61133 + 0.27115 * x
                           - 0.12724e-1 * x * x - 0.28167e-1 * x * x * x + 0.54699e-2 * x * x * x * x;
          }
        }
        break;
      case 41:
        if (iflag05th == 1) { // For 0.5MeV threshold.
          if (Run < 800) {
            if (x < 2.5) {
              return_value = 0.90188 + 0.76563e-1 * x
                             - 0.16328e-1 * x * x + 0.56816e-3 * x * x * x;
            }
          }
          if (799 < Run) { // Run1200-1261 no signif. change from Run800-1199.
            if (x < 2.95) {
              return_value = 0.88077 + 0.71720e-1 * x
                             - 0.92197e-2 * x * x - 0.15932e-2 * x * x * x + 0.38133e-3 * x * x * x * x;
            }
          }
        } else { // For theta-dep. threshold.
          if (x < 3.2) {
            return_value =  0.57808 + 0.31083 * x
                            - 0.20247e-1 * x * x - 0.31658e-1 * x * x * x + 0.64087e-2 * x * x * x * x;
          }
        }
        break;
      case 43: // modified 20090829.
        if (iflag05th == 1) { // For 0.5MeV threshold.
          if (Run < 300) {
            if (x < 3.3) {
              return_value = 0.85592 + 0.93352e-1 * x
                             - 0.93144e-2 * x * x - 0.43681e-2 * x * x * x + 0.7971e-3 * x * x * x * x;
            }
          }
          if (299 < Run && Run < 700) { // added 20050630.
            if (x < 3.0) {
              return_value = 0.89169 + 0.73301e-1 * x
                             - 0.13856e-1 * x * x + 0.47303e-3 * x * x * x;
            }
          }
          if (699 < Run) { // added 20050727
            if (x < 3.1) {
              return_value = 0.90799 + 0.69815e-1 * x
                             - 0.17490e-1 * x * x + 0.14651e-2 * x * x * x;
            }
          }
        } else { // For theta-dep. threshold b20090127.
          if (x < 3.25) {
            return_value = 0.58176 + 0.30245 * x - 0.16390e-1 * x * x
                           - 0.32258e-1 * x * x * x + 0.64121e-2 * x * x * x * x;
          }
          //if( 999 < Run ){// For 5S runs, 2/fb, obsolete 20091209.
          //if( x < 3.3 ){
          //return_value = 0.58463 + 0.29780*x -0.14441e-1*x*x
          //-0.32323e-1*x*x*x + 0.63607e-2*x*x*x*x;
          //}
          //}
        }
        break;
      case 45:
        if (iflag05th == 1) { // For 0.5MeV threshold.
          if (x < 3.1) { // added 20060413
            return_value = 0.84823 + 0.10394 * x
                           - 0.92233e-2 * x * x - 0.72700e-2 * x * x * x + 0.14552e-2 * x * x * x * x;
          }
        } else { // For theta-dep. threshold b20090127.
          if (x < 3.25) {
            return_value = 0.62797 + 0.23897 * x
                           + 0.10261e-1 * x * x - 0.35700e-1 * x * x * x + 0.63846e-2 * x * x * x * x;
          }
        }
        break;
      case 47:
        if (iflag05th == 1) { // For 0.5MeV threshold.
          if (x < 3.1) { // added 20060420
            return_value = 0.86321 + 0.82987e-1 * x
                           - 0.12139e-1 * x * x - 0.12920e-3 * x * x * x;
          }
        } else { // For theta-dep. threshold b20090127.
          if (x < 3.25) {
            return_value = 0.58061 + 0.30399 * x
                           - 0.17520e-1 * x * x - 0.31559e-1 * x * x * x + 0.62681e-2 * x * x * x * x;
          }
        }
        break;
      case 49:
        if (iflag05th == 1) { // For 0.5MeV threshold.
          if (x < 3.13) { // added 20060511
            if (x < 2.80) {
              return_value = 0.85616 + 0.78041e-1 * x
                             - 0.38987e-2 * x * x - 0.31224e-2 * x * x * x + 0.33374e-3 * x * x * x * x;
            } else {
              return_value = ((1.0 - 0.99608) / (3.13 - 2.80)) * (x - 2.80) + 0.99608;
            }
          }
        } else { // For theta-dep. threshold b20090127.
          if (x < 3.3) {
            return_value = 0.57687 + 0.29948 * x
                           - 0.10594e-1 * x * x - 0.34561e-1 * x * x * x + 0.67064e-2 * x * x * x * x;
          }
        }
        break;
      case 51:
        if (iflag05th == 1) { // For 0.5MeV threshold, added 20070323.
          if (x < 3.07) {
            return_value = 0.89063 + 0.72152e-1 * x
                           - 0.14143e-1 * x * x + 0.73116e-3 * x * x * x;
          }
        } else { // For theta-dep. threshold b20090127.
          if (x < 3.3) {
            return_value = 0.59310 + 0.28618 * x
                           - 0.13858e-1 * x * x - 0.30230e-1 * x * x * x + 0.59173e-2 * x * x * x * x;
          }
        }
        break;
      case 53: // modified 20090807.
        if (iflag05th == 1) { // For 0.5MeV threshold.
          if (x < 2.63) {
            //std::cout<<"Exp.53 0.5MeV threshold."<<std::endl;
            return_value = 0.83425 + 0.10468 * x
                           - 0.53641e-2 * x * x - 0.60276e-2 * x * x * x + 0.77763e-3 * x * x * x * x;
          }
        } else { // For theta-dep. threshold b20090127.
          if (x < 3.1) {
            //std::cout<<"Exp.53 theta-dep. threshold."<<std::endl;
            return_value = 0.66100 + 0.25192 * x
                           - 0.16419e-1 * x * x - 0.25720e-1 * x * x * x + 0.52189e-2 * x * x * x * x;
          }
        }
        break;
      case 55:
        if (iflag05th == 1) { // For 0.5MeV threshold.
          if (x < 3.3) {
            return_value = 0.86202 + 0.79575e-1 * x
                           - 0.66721e-2 * x * x - 0.28609e-2 * x * x * x + 0.42784e-3 * x * x * x * x;
          }
        } else { // For theta-dep. threshold b20090127.
          if (x < 3.25) {
            return_value = 0.58789 + 0.29310 * x
                           - 0.15784e-1 * x * x - 0.30619e-1 * x * x * x + 0.60648e-2 * x * x * x * x;
          }
        }
        break;
      case 61:
        if (Run < 1210) { // On 4S by Th.-dep. threshold.
          if (x < 3.5) {
            return_value = 0.68839 + 0.18218 * x
                           - 0.23140e-2 * x * x - 0.17439e-1 * x * x * x + 0.29960e-2 * x * x * x * x;
          }
        }
        if (Run > 1209) { // 5S scan.
          if (iflag05th == 1) { // processed by 0.5MeV threshold.
            if (x < 3.3) {
              if (x < 2.8) {
                return_value = 0.94294 - 0.77497e-2 * x
                               - 0.43464e-3 * x * x + 0.99837e-2 * x * x * x - 0.23726e-2 * x * x * x * x;
              } else {
                return_value = 0.0162997 * (x - 2.80) + 0.991162;
              }
            }
          } else { // 5S scan. by Th.-dep. threshold.
            if (x < 3.5) {
              return_value = 0.64816 + 0.22492 * x
                             - 0.91745e-2 * x * x - 0.21736e-1 * x * x * x + 0.41333e-2 * x * x * x * x;
            }
          }
        }
        break;
      case 63:
        if (x < 3.4) {
          return_value = 0.69302 + 0.18393 * x
                         - 0.10983e-1 * x * x - 0.14148e-1 * x * x * x + 0.27298e-2 * x * x * x * x;
        }
        break;
      case 65:
        if (Run > 1000) { // 1S run turned out to be same as 5S scan.
          if (iflag05th == 1) { // For 0.5MeV threshold.
            if (x < 3.3) {
              if (x < 2.8) {
                return_value = 0.94294 - 0.77497e-2 * x
                               - 0.43464e-3 * x * x + 0.99837e-2 * x * x * x - 0.23726e-2 * x * x * x * x;
              } else {
                return_value = 0.0162997 * (x - 2.80) + 0.991162;
              }
            }
          } else { // For theta-dep. threshold, b20090127.
            if (x < 3.6) {
              return_value = 0.70987 + 0.16230 * x
                             - 0.64867e-2 * x * x - 0.12021e-1 * x * x * x + 0.20874e-2 * x * x * x * x;
            }
          }
        } else { // 4S runs processed with new lib.
          if (x < 3.4) {
            return_value = 0.66833 + 0.20602 * x
                           - 0.14322e-1 * x * x - 0.15712e-1 * x * x * x + 0.31114e-2 * x * x * x * x;
          }
        }
        break;
      case 67:
        if (x < 3.4) {
          return_value = 0.64196 + 0.23069 * x
                         - 0.20303e-1 * x * x - 0.15680e-1 * x * x * x + 0.31611e-2 * x * x * x * x;
        }
        break;
      case 69:
        if (x < 3.35) {
          //return_value = 0.64196 + 0.23752*x
          //-0.85197e-2*x*x -0.24366e-1*x*x*x +0.46397e-2*x*x*x*x;
          return_value = 0.99 * (0.64196 + 0.23752 * x
                                 - 0.85197e-2 * x * x - 0.24366e-1 * x * x * x
                                 + 0.46397e-2 * x * x * x * x);
        }
        break;
      case 71:
        if (x < 3.35) {
          return_value = 0.66541 + 0.12579 * x
                         + 0.89999e-1 * x * x - 0.58305e-1 * x * x * x + 0.86969e-2 * x * x * x * x;
        }
        break;
      case 73:
        if (x < 3.45) {
          return_value = 0.62368 + 0.24142 * x
                         - 0.11677e-1 * x * x - 0.22477e-1 * x * x * x + 0.42765e-2 * x * x * x * x;
        }
        break;
    }
    // This return value has to be used as division.
    // i.e. E_corrected = E_original/return_vale.
    return return_value;
  }

//=================================================================
// Fix up wrong calib. in Exp.45 by Isamu Nakamura, added 20060413.
//=================================================================
  /** The function giving correction factor in Exp.45. */
  static double ecl_adhoc_corr_45(int exp, int /*run*/, int cid)
  {

    int bcid[] = {
      243, 244, 316, 317, 318, 412, 413, 414, 508, 509, 510, 604, 605, 606
    };

    double bcfact[] = {
      1.06215 , 1.06589 , 0.953827, 1.06607 , 0.943696, 0.910855, 1.01201 ,
      1.01704 , 0.954612, 1.02761 , 1.00716 , 1.0319  , 1.03135 , 1.05782
    };

    if (exp != 45) return 1.;

    for (int i = 0 ; i < 14 ; i++)
      if (bcid[i] == cid)
        return bcfact[i];

    return 1.;
  }

//=====================
// Correct energy scale (MC) to make pi0 peak nominal. 20080530
//=====================
  /** Correct energy scale (MC) to make pi0 peak nominal. */
  static double ecl_mcx3_corr(int /*exp*/, int /*run*/, double energy, double /*theta*/)
  {
    //KM std::cout<<"ecl_mcx3_corr called."<<std::endl;
    double return_value;
    double x = std::log10(energy * 1000.0);
    return_value = 1.0;
    if (x < 3.2) {
      return_value = 0.72708 + 0.22735 * x
                     - 0.20603e-1 * x * x - 0.24644e-1 * x * x * x + 0.53480e-2 * x * x * x * x;
    }
    //std::cout<<"energy="<<energy<<" ecl_mcx3="<<return_value<<std::endl;
    return return_value;
  }

//===== mpi0pdg.cc ===== cerated 2001/07/17 =====
// Make MC mass peak to PDG's one.
// Version0.1 2001/07/17 trial version.
//===============================================
  /** Make MC mass peak to PDG value. */
  static double mpi0pdg(double Energy) // Energy in GeV.
  {
    // return value.
    double return_value;
    // pi0 mass in PDG.
    const double m_pdg = 134.9766;

    // default = do nothing.
    return_value = 1.0;

    if (0.1 < Energy && Energy < 1.0) {
      double x = log10(Energy * 1000.0);
      // Curve given by S.Uchida at 2001/07/17.
      return_value
        = m_pdg / (135.21 + 5.3812 * x - 4.2160 * x * x + 0.74650 * x * x * x);
    }
    // This return value has to be used as product.
    // i.e. E_corrected = E_original*return_value.
    return return_value;
  }

//**** pi0resol ***** implemented 2003/Feb./17th *****
// Standalone func. of pi0 mass resolution function
// coded by Hayashii-san.
//*******************************
//*** Original comments ********
// pi0resol: pi0 resolution function
//
//******************************
// input  p     : pi0 lab momentum in GeV
//        theta : pi0 lab polar angle in the unit of Deg.
//        side  :
//             ="lower"   ; return the sigma of the lower(left) side.
//             ="higher"  ; return the sigma of the higher(right) side.
//        mcdata:
//             =0  accessing Exp. data
//             =1  accessing MC
//        exp   :  experiment number
//
// output      : return pi0 mass resolution in the unit of  GeV.
//**********************************
// Original definition.
//double pi0resol(double p, double theta, char* side, bool mcdata, int exp )
  double B2BIIFixMdstModule::pi0resol(double p, double theta, const char* side, bool mcdata,
                                      int /*exp*/, int /*option*/)
  {
// option is added to become compatible with future gsim modification.
    int iside = 0;

    if (!strcmp(side, "lower"))  iside = 1;
    if (!strcmp(side, "higher")) iside = 2;
    if (iside == 0) {
      B2ERROR("Error pi0resol. Wrong input parameter=" << side);
      return -1;
    }
//  dout(Debugout::INFO,"B2BIIFixMdst_ecl") <<" iside="<<iside<<std::endl;
//----------------------------------------------------
//        20 <= theta  <= 45
//----------------------------------------------------
//   0.15 < p  < 0.8
//    -- data   L: lower side  H: higher side
    const double data_thf_pl_L[] = { 9.8233, -13.3489, 18.3271, -7.6668};
    const double derr_thf_pl_L[] = { 0.3274, 0.755, 0.8611, 0.521};
    const double data_thf_pl_H[] = { 6.1436, -7.9668, 12.4766, -5.3201};
    const double derr_thf_pl_H[] = { 0.2903, 0.6754, 0.7724, 0.466};
//   0.8 < p  < 5.3
//    -- data   L: lower side  H: higher side
    const double data_thf_ph_L[] = { 7.1936, -0.6378, 0.5912, -0.075};
    const double derr_thf_ph_L[] = { 0.1975, 0.2036, 0.0793, 0.0121};
    const double data_thf_ph_H[] = { 4.4923, 0.5532, 0.2658, -0.0343};
    const double derr_thf_ph_H[] = { 0.1883, 0.2019, 0.0803, 0.0122};
//----------------------------------------------------
//        20 <= theta  <= 45
//----------------------------------------------------
//   0.15 < p  < 0.8
//    -- MC   L: lower side  H: higher side
    const double mc_thf_pl_L[] = { 4.8093, 3.4567, -3.7898, 1.7553};
    const double merr_thf_pl_L[] = { 0.2145, 0.527, 0.6153, 0.3712};
    const double mc_thf_pl_H[] = { 4.6176, -2.9049, 4.2994, -1.4776};
    const double merr_thf_pl_H[] = { 0.1969, 0.4826, 0.555, 0.3346};
//   0.8 < p  < 5.3
//    -- MC   L: lower side  H: higher side
    const double mc_thf_ph_L[] = { 6.3166, -0.5993, 0.5845, -0.0695};
    const double merr_thf_ph_L[] = { 0.1444, 0.1468, 0.0571, 0.0087};
    const double mc_thf_ph_H[] = { 2.9719, 1.7999, -0.2418, 0.028};
    const double merr_thf_ph_H[] = { 0.1318, 0.1362, 0.0538, 0.0082};

//----------------------------------------------------
//        45 <= theta  <= 100
//----------------------------------------------------
//   0.15 < p  < 0.8
//    -- data   L: lower side  H: higher side
    const double data_thm_pl_L[] = { 7.7573, -4.8855, 6.5561, -2.4788};
    const double derr_thm_pl_L[] = { 0.1621, 0.4894, 0.6882, 0.4985};
    const double data_thm_pl_H[] = { 6.9075, -10.5036, 18.5196, -9.224};
    const double derr_thm_pl_H[] = { 0.1458, 0.4383, 0.639, 0.4726};
//   0.8 < p  < 5.3
//    -- data   L: lower side  H: higher side
    const double data_thm_ph_L[] = { 5.2347, 2.1827, -0.6563, 0.1545};
    const double derr_thm_ph_L[] = { 0.0986, 0.1281, 0.0627, 0.0117};
    const double data_thm_ph_H[] = { 3.2114, 3.3806, -0.8635, 0.1371};
    const double derr_thm_ph_H[] = { 0.0927, 0.1205, 0.058, 0.0106};
//
//----------------------------------------------------
//        45 <= theta  <= 100
//----------------------------------------------------
//   0.15 < p  < 0.8
//    -- MC   L: lower side  H: higher side
    const double mc_thm_pl_L[] = { 6.1774, -2.1831, 3.6615, -1.1813};
    const double merr_thm_pl_L[] = {   0.11, 0.327, 0.476, 0.3655};
    const double mc_thm_pl_H[] = { 4.0239, -0.7485, 3.6203, -1.4823};
    const double merr_thm_pl_H[] = { 0.0991, 0.3034, 0.4223, 0.3069};
//   0.8 < p  < 5.3
//    -- MC   L: lower side  H: higher side
    const double mc_thm_ph_L[] = { 4.5966, 2.261, -0.4938, 0.0984};
    const double merr_thm_ph_L[] = { 0.0711, 0.0917, 0.0448, 0.0081};
    const double mc_thm_ph_H[] = { 3.4609, 2.0069, -0.0498, -0.0018};
    const double merr_thm_ph_H[] = { 0.0966, 0.1314, 0.0574, 0.0086};
//
//----------------------------------------------------
//        100 <= theta  <= 150
//----------------------------------------------------
//   0.15 < p  < 0.8
//    -- data   L: lower side  H: higher side
    const double data_thb_pl_L[] = {11.5829, -21.6715, 30.2368, -13.0389};
    const double derr_thb_pl_L[] = { 0.2742, 0.7256, 0.9139, 0.6006};
    const double data_thb_pl_H[] = { 8.0227, -14.7387, 23.1042, -10.4233};
    const double derr_thb_pl_H[] = { 0.2466, 0.6512, 0.8239, 0.5419};
//   0.8 < p  < 5.3
//    -- data   L: lower side  H: higher side
    const double data_thb_ph_L[] = { 7.5872, -1.8994, 1.6526, -0.2755};
    const double derr_thb_ph_L[] = { 0.1999, 0.2638, 0.1422, 0.0335};
    const double data_thb_ph_H[] = { 6.3542, -2.5164, 2.5763, -0.4803};
    const double derr_thb_ph_H[] = { 0.1885, 0.2527, 0.136, 0.0318};
//----------------------------------------------------
//        100 <= theta  <= 150
//----------------------------------------------------
//   0.15 < p  < 0.8
//    -- MC   L: lower side  H: higher side
    const double mc_thb_pl_L[] = { 5.2707, 2.5607, -3.1377, 1.8434};
    const double merr_thb_pl_L[] = { 0.1801, 0.5048, 0.6741, 0.4343};

    const double mc_thb_pl_H[] = { 2.5867, 7.6982, -10.0677, 5.312};
    const double merr_thb_pl_H[] = { 0.1658, 0.4651, 0.6063, 0.3925};
//   0.8 < p  < 5.3
//    -- MC   L: lower side  H: higher side
    const double mc_thb_ph_L[] = { 6.5206, -1.5103, 1.9054, -0.3609};
    const double merr_thb_ph_L[] = { 0.1521, 0.2048, 0.1108, 0.0255};
    const double mc_thb_ph_H[] = { 3.4397, 2.2372, -0.1214, -0.0004};
    const double merr_thb_ph_H[] = { 0.1324, 0.1822, 0.1065, 0.0252};

//
    double resol;
    double para[4] = { };
    double error[4] = { };

    double pbuf = p;
//--
//       theta< 45
//---
    if (theta <= 45.) {
//--
//                       p-low
//---
      if (pbuf <= 0.8) {
        if (!mcdata) {
          //--  data
          for (int i = 0; i <= 3; i++) {
            if (iside == 1) {
              para[i] = data_thf_pl_L[i];
              error[i] = derr_thf_pl_L[i];
            } else {
              para[i] = data_thf_pl_H[i];
              error[i] = derr_thf_pl_H[i];
            }
          } // for
        } else {
          //--  mc
          for (int i = 0; i <= 3; i++) {
            if (iside == 1) {
              para[i] = mc_thf_pl_L[i];
              error[i] = merr_thf_pl_L[i];
            } else {
              para[i] = mc_thf_pl_H[i];
              error[i] = merr_thf_pl_H[i];
            }
          }  //for
        }
      } else {
//--
//                       p-high
//---
//  use p= 5.2 value  if p >= 5.2 for forward region
        if (pbuf >= 5.2) {pbuf = 5.2;}
        if (!mcdata) {
          //--  data
          for (int i = 0; i <= 3; i++) {
            if (iside == 1) {
              para[i] = data_thf_ph_L[i];
              error[i] = derr_thf_ph_L[i];
            } else {
              para[i] = data_thf_ph_H[i];
              error[i] = derr_thf_ph_H[i];
            }
          }// for
        } else {
          //--  mc
          for (int i = 0; i <= 3; i++) {
            if (iside == 1) {
              para[i] = mc_thf_ph_L[i];
              error[i] = merr_thf_ph_L[i];
            } else {
              para[i] = mc_thf_ph_H[i];
              error[i] = merr_thf_ph_H[i];
            }
          }// for
        }
      } // p-range
//--
//     45<  theta< 100
//---
    } else if (theta > 45. && theta <= 100.) {
//--
//                       p-low
//---
      if (pbuf <= 0.8) {
        if (!mcdata) {
          //--  data
          for (int i = 0; i <= 3; i++) {
            if (iside == 1) {
              para[i] = data_thm_pl_L[i];
              error[i] = derr_thm_pl_L[i];
            } else {
              para[i] = data_thm_pl_H[i];
              error[i] = derr_thm_pl_H[i];
            }
          } // for
        } else {
          //--  mc
          for (int i = 0; i <= 3; i++) {
            if (iside == 1) {
              para[i] = mc_thm_pl_L[i];
              error[i] = merr_thm_pl_L[i];
            } else {
              para[i] = mc_thm_pl_H[i];
              error[i] = merr_thm_pl_H[i];
            }
          } //for
        }
      } else {
//--
//                       p-high
//---
//  use p= 5.2 value  if p >= 5.2 for middle region
        if (pbuf >= 5.2) {pbuf = 5.2;}
        if (!mcdata) {
          //--  data
          for (int i = 0; i <= 3; i++) {
            if (iside == 1) {
              para[i] = data_thm_ph_L[i];
              error[i] = derr_thm_ph_L[i];
            } else {
              para[i] = data_thm_ph_H[i];
              error[i] = derr_thm_ph_H[i];
            }
          }  //for
        } else {
          //--  mc
          for (int i = 0; i <= 3; i++) {
            if (iside == 1) {
              para[i] = mc_thm_ph_L[i];
              error[i] = merr_thm_ph_L[i];
            } else {
              para[i] = mc_thm_ph_H[i];
              error[i] = merr_thm_ph_H[i];
            }
          }  //for
        }
      } // p-range
//--
//       theta> 100
//---
    } else if (theta > 100.) {
//--
//                       p-low
//---
      if (pbuf <= 0.8) {
        if (!mcdata) {
          //--  data
          for (int i = 0; i <= 3; i++) {
            if (iside == 1) {
              para[i] = data_thb_pl_L[i];
              error[i] = derr_thb_pl_L[i];
            } else {
              para[i] = data_thb_pl_H[i];
              error[i] = derr_thb_pl_H[i];
            }
          }  //for
        } else {
          //--  mc
          for (int i = 0; i <= 3; i++) {
            if (iside == 1) {
              para[i] = mc_thb_pl_L[i];
              error[i] = merr_thb_pl_L[i];
            } else {
              para[i] = mc_thb_pl_H[i];
              error[i] = merr_thb_pl_H[i];
            }
          }  //for
        }
      } else {
//--
//                       p-high
//---
//  use p= 3.0 value  if p >= 3.0 for backward region
        if (pbuf >= 3.0) {pbuf = 3.0;}
        if (!mcdata) {
          //--  data
          for (int i = 0; i <= 3; i++) {
            if (iside == 1) {
              para[i] = data_thb_ph_L[i];
              error[i] = derr_thb_ph_L[i];
            } else {
              para[i] = data_thb_ph_H[i];
              error[i] = derr_thb_ph_H[i];
            }
          }  //for
        } else {
          //--  mc
          for (int i = 0; i <= 3; i++) {
            if (iside == 1) {
              para[i] = mc_thb_ph_L[i];
              error[i] = merr_thb_ph_L[i];
            } else {
              para[i] = mc_thb_ph_H[i];
              error[i] = merr_thb_ph_H[i];
            }
          }  //for
        }
      } // p-range


    }  //theta range
//--
//  evaluate resolution in the unit of GeV.
//--
    resol = para[0] + para[1] * pbuf + para[2] * pbuf * pbuf +
            para[3] * pbuf * pbuf * pbuf;
    resol = resol / 1000.;

//--
// Evaluate the error od sigma using diagonal errors.
//--
    double eresol = error[0] * error[0] + (pbuf * error[1]) * (pbuf * error[1])
                    + (pbuf * pbuf * error[2]) * (pbuf * pbuf * error[2])
                    + (pbuf * pbuf * pbuf * error[3]) * (pbuf * pbuf * pbuf * error[3]);
    eresol = sqrt(eresol) / 1000.;

    //     dout(Debugout::INFO,"B2BIIFixMdst_ecl") <<" theta="<<theta<<" p="<<pbuf
    //     <<" para="<< para[0]<<", "<< para[1]<<", "
    //     << para[2]<<", "<<para[3]
    //     <<" resol="<<resol<<" er="<<eresol<<std::endl;
    return resol;
  }
// Following lines were commented out, because asymmetric line shape is
// not taken into account. 2003/Feb./17th KM.
//==================================================
// pi0 mass resolution function.
// Original was coded by H.Hayashii
// Implemented into B2BIIFixMdst_ecl in 2002/11/30 by KM
//==================================================
//*******************************
// pi0resol: pi0 resolution function
//
//******************************
// input  p     : pi0 lab momentum in GeV
//        theta : pi0 lab polar angle in the unit of Deg.
//        mcdata: =1/0  MC/Data
//        exp   :  experiment number
// The unit of returned value is in GeV/c^2. (Note by KM).
//static double pi0resol(double p, double theta, bool mcdata,int exp )
//{
//-----------------------------------------------------------
//   20<=theta<=45
//-----------------------------------------------------------
//      0.15<=  p <=0.8
// -- data
//   const double data_thf_pl[]={9.4962, -21.906, 31.229, -13.424};
//   const double derr_thf_pl[]={0.4014,   0.978,  1.174, 0.786};
// -- mc
//   const double mc_thf_pl[]  ={4.3911,  -2.121,   4.278,  -1.696};
//   const double merr_thf_pl[]={0.258,    0.584,   0.617,   0.294};
//---------------------------
//      0.8<=  p <=5.3
// -- data
//   const double data_thf_ph[]={4.6545,   0.5212, 0.1370, -0.0214};
//   const double derr_thf_ph[]={0.189,    0.1866, 0.069,   0.099 };
// -- mc
//   const double mc_thf_ph[]  ={4.034,    0.8925,  -0.040,  -0.0034};
//   const double merr_thf_ph[]={0.205,    0.227,    0.088,   0.013 };
//
//-----------------------------------------------------------
//   45<=theta<=100
//-----------------------------------------------------------
//      0.15<=  p <=0.8
// -- data
//   const double data_thm_pl[]={6.6403,  -9.281, 16.179,  -8.2410 };
//   const double derr_thm_pl[]={0.1353,   0.406,  0.579,   0.434  };
// -- mc
//   const double mc_thm_pl[]  ={3.7939,   1.877,  -0.564,   0.098};
//   const double merr_thm_pl[]={0.145,    0.484,   0.705,   0.510};
//---------------------------
//      0.8<=  p <=5.3
// -- data
//   const double data_thm_ph[]={3.8323,   2.0226, -0.4476,  0.0833};
//   const double derr_thm_ph[]={0.1492,   0.2093,  0.0932,  0.0137};
// -- mc
//   const double mc_thm_ph[]  ={3.3595,   2.2174,  -0.341,   0.0469};
//   const double merr_thm_ph[]={0.1311,   0.168,    0.077,   0.0131};
//
//-----------------------------------------------------------
//   100<=theta<=150
//-----------------------------------------------------------
//      0.15<=  p <=0.8
// -- data
//   const double data_thb_pl[]={8.1320, -14.985, 22.567, -10.150  };
//   const double derr_thb_pl[]={0.2071,   0.537,  0.659,   0.458  };
// -- mc
//   const double mc_thb_pl[]  ={2.8217,   7.257,  -7.936,   3.201};
//   const double merr_thb_pl[]={0.2085,   0.529,   0.642,   0.387};
//---------------------------
//      0.8<=  p <=5.3
// -- data
//   const double data_thb_ph[]={4.969 ,   0.1843,  0.5477, -0.1011};
//   const double derr_thb_ph[]={0.164 ,   0.2430,  0.1289,  0.0288};
// -- mc
//   const double mc_thb_ph[]  ={3.3595,   2.998 ,  -1.081,   0.1781};
//   const double merr_thb_ph[]={0.2625,   0.341,    0.180,   0.0411};
//
//
//   double resol;
//   double para[4];
//   double error[4];
//
//   double pbuf = p;
//--
//       theta< 45
//---
//   if( theta <=45.) {
//--
//                       p-low
//---
//      if( pbuf <= 0.8){
//   if(!mcdata){
//      //--  data
//      for( int i=0; i <= 3; i++){
//         para[i] = data_thf_pl[i];
//         error[i]= derr_thf_pl[i];}
//   } else{
//      //--  mc
//      for( int i=0; i <= 3; i++){
//         para[i] = mc_thf_pl[i];
//         error[i]= merr_thf_pl[i];}
//   }
//      } else {
//--
//                       p-high
//---
//  use p= 5.2 value  if p >= 5.2 for forward region
//   if(pbuf >= 5.2) {pbuf = 5.2;}
//   if(!mcdata){
//      //--  data
//      for( int i=0; i <= 3; i++){
//         para[i] = data_thf_ph[i];
//         error[i]= derr_thf_ph[i];}
//   } else{
//      //--  mc
//      for( int i=0; i <= 3; i++){
//         para[i] = mc_thf_ph[i];
//         error[i]= merr_thf_ph[i];}
//   }
//      } // p-range
//--
//     45<  theta< 100
//---
//   } else if( theta>45.&& theta<=100.){
//--
//                       p-low
//---
//      if( pbuf <= 0.8){
//   if(!mcdata){
//      //--  data
//      for( int i=0; i <= 3; i++){
//         para[i] = data_thm_pl[i];
//         error[i]= derr_thm_pl[i];}
//   } else{
//      //--  mc
//      for( int i=0; i <= 3; i++){
//         para[i] = mc_thm_pl[i];
//         error[i]= merr_thm_pl[i];}
//   }
//      } else {
//--
//                       p-high
//---
//  use p= 5.2 value  if p >= 5.2 for middle region
//   if(pbuf >= 5.2) {pbuf = 5.2;}
//   if(!mcdata){
//      //--  data
//      for( int i=0; i <= 3; i++){
//         para[i] = data_thm_ph[i];
//         error[i]= derr_thm_ph[i];}
//   } else{
//      //--  mc
//      for( int i=0; i <= 3; i++){
//         para[i] = mc_thm_ph[i];
//         error[i]= merr_thm_ph[i];}
//   }
//      } // p-range
//--
//       theta> 100
//---
//   } else if( theta>100.){
//--
//                       p-low
//---
//      if( pbuf <= 0.8){
//   if(!mcdata){
//      //--  data
//      for( int i=0; i <= 3; i++){
//         para[i] = data_thb_pl[i];
//         error[i]= derr_thb_pl[i];}
//   } else{
//      //--  mc
//      for( int i=0; i <= 3; i++){
//         para[i] = mc_thb_pl[i];
//         error[i]= merr_thb_pl[i];}
//   }
//      } else {
//--
//                       p-high
//---
//  use p= 4.0 value  if p >= 4.0 for backward region
//   if(pbuf >= 4.0) {pbuf = 4.0;}
//   if(!mcdata){
//      //--  data
//      for( int i=0; i <= 3; i++){
//         para[i] = data_thb_ph[i];
//         error[i]= derr_thb_ph[i];}
//   } else{
//      //--  mc
//      for( int i=0; i <= 3; i++){
//         para[i] = mc_thb_ph[i];
//         error[i]= merr_thb_ph[i];}
//   }
//      } // p-range
//
//
//   }  //theta range
//--
//  evaluate resolution in the unit of GeV.
//--
//   resol = para[0] + para[1]*pbuf + para[2]*pbuf*pbuf +
//      para[3]*pbuf*pbuf*pbuf;
//   resol = resol/1000.;
//
//--
// This error evaluation is not correct, since the off-diagonal part
// is not took into account.
//--
//   double eresol = error[0]*error[0] + (pbuf* error[1])*(pbuf*error[1])
//      + (pbuf*pbuf* error[2])* (pbuf*pbuf* error[2])
//      + (pbuf*pbuf*pbuf* error[3])*(pbuf*pbuf*pbuf* error[3]);
//   eresol = sqrt(eresol)/1000.;
//
//   //     dout(Debugout::INFO,"B2BIIFixMdst_ecl") <<" theta="<<theta<<" p="<<pbuf
//   //     <<" para="<< para[0]<<", "<< para[1]<<", "
//     //     << para[2]<<", "<<para[3]
//   //     <<" resol="<<resol<<" er="<<eresol<<std::endl;
//   return resol;
//}

//======================================================
// Correct photon's momenta and error matrix.
//======================================================
  void B2BIIFixMdstModule::correct_ecl(int option, int version)
  {

    int ecl_threshold(0);
    if (m_reprocess_version == 0) {
      ecl_threshold = 1;
    } else if (m_reprocess_version == 1) {
      ecl_threshold = 0;
    }

    //dout(Debugout::INFO,"B2BIIFixMdst_ecl") << "This is version1.2.1" << std::endl;
    // dout(Debugout::INFO,"B2BIIFixMdst_ecl") << "entering correct_ecl()" << std::endl;
    // Check whether "version" is supported.
    if (version < 1 || version > 2) {
      B2WARNING("correct_ecl :: Warning! ");
      B2WARNING("version=" << version << " is not supported! ");
      B2WARNING("Exit doing nothing!");
    }

    //---------------------
    // Control sequence based on belle_event table.
    //---------------------
    Belle::Belle_event_Manager& evtmgr = Belle::Belle_event_Manager::get_manager();
    if (0 == evtmgr.count()) {
      // Do nothing if not exist, because we can not know whether
      // this event is Exp. data or MC.
      return;
    }
    //Check data or mc. Data:expmc=1, MC:expmc=2.
    int expmc = evtmgr[0].ExpMC();
    // Exp, Run, number to be given to correction func.
    int Eno = evtmgr[0].ExpNo();
    int Rno = evtmgr[0].RunNo();

    // If option is 0 and this event is MC, do nothing.
    // Modify : Exp.61 processed by b20080502, MC is also corrected.
    //org if( option==0 && expmc==2 )
    if (option == 0 && expmc == 2) {
      //if( Eno<55) //test
      //std::cout<<"ecl_threshold="<<ecl_threshold<<std::endl;
      //      if( Eno<60 || ( Eno==61 && Rno>1209 ) /* 5S scan runs. */
      //   || ( Eno==61 && m_correct_ecl_5s==1 ) /* Exp.61 old lib case.*/
      //   || ( Eno==65 && Rno>1000 ) /* 1S runs.*/
      //   || ( Eno==65 && m_correct_ecl_5s==1 ) /* Exp.65 old lib case.*/)
      if (ecl_threshold == 1) {
        return;
      }
    }

    // Correction curve version control.
    int i_previous(0);
    // Mdst_event_add table is there?
    Belle::Mdst_event_add_Manager& mevtmgr = Belle::Mdst_event_add_Manager::get_manager();
    // When Mdst_event_add exists,
    if (mevtmgr.count() > 0) {
      // Up to Exp.13, same treatment as V1.0 2001/Nov./30th
      //if( Eno <= 13 )
      // Modify; Exp.13 moved to same treatment as Exp.15 or later.
      // 20020524 Kenkichi Miyabayashi.
      if (Eno <= 11) {
        // If already the energy is corrected, exit.
        if (mevtmgr[0].flag(3) == 1) {
          return;
        }
        // Otherwise, set the flag at proper entity, flag(3).
        // Note that frag(1) and frag(2) have already been used
        // in scale_momenta().
        mevtmgr[0].flag(3, 1);
      }
      // Exp.13 and later...last update 2002/May./24 K.Miya.
      else { // Exp.15 and later...last update 2002/Feb./20
        // If well-established version was already applied, exit.
        if (mevtmgr[0].flag(3) >= version) {
          return;
        }
        // Otherwise, check the previous version.
        i_previous = mevtmgr[0].flag(3);
        if (i_previous == 0 || i_previous == 1) {
          mevtmgr[0].flag(3, version);
        } else { // Previous version is unsupported one.
          // Make Warning and exit.
          B2WARNING("correct_ecl :: Warning! ");
          B2WARNING("Previously, uncorrect version was used. ");
          B2WARNING(" Exit doing nothing");
          return;
        }
      }
    } else { // Create Mdst_event_add and set flag.
      Belle::Mdst_event_add& meadd = mevtmgr.add();
      // Up to Exp.13, same treatment as before.
      if (Eno <= 13) {
        meadd.flag(3, 1);
      } else { // For Exp.15, set version.
        meadd.flag(3, version);
      }
    }

    //--------------------------
    // If no ad_hoc correction has been applied so far or
    // correction curve is new version,
    // overwrite proper panther tables.
    //--------------------------
    // Get Mdst_ecl and Mdst_gamma.
    Belle::Mdst_ecl_Manager&   eclmgr   = Belle::Mdst_ecl_Manager::get_manager();
    Belle::Mdst_gamma_Manager& gammamgr = Belle::Mdst_gamma_Manager::get_manager();

    // Get Mdst_ecl_aux. Added 20060413
    Belle::Mdst_ecl_aux_Manager& eclauxmgr = Belle::Mdst_ecl_aux_Manager::get_manager();
    std::vector<Belle::Mdst_ecl_aux>::iterator itaux = eclauxmgr.begin();

    // Correct energy and error matrix in Mdst_ecl.
    double factor;
    double factor13;

    for (std::vector<Belle::Mdst_ecl>::iterator itecl = eclmgr.begin();
         itecl != eclmgr.end(); ++itecl) {
      Belle::Mdst_ecl& shower = *itecl;
      // Shower energy and polar angle
      double shower_energy = shower.energy();
      double shower_theta  = shower.theta();

      // Fix wrong calib. for Exp. 45.
      //      if( Eno==45 )
      if (expmc == 1 && m_reprocess_version == 0 && Eno == 45) {
        int cellID = (*itaux).cId();
        double factor45 = ecl_adhoc_corr_45(Eno, Rno, cellID);
        //if( factor45!=1.0 )
        //{
        //int idecl=shower.get_ID();
        // dout(Debugout::DDEBUG,"B2BIIFixBelle::Mdst_ecl")<<"Exp45 fix idecl="<<idecl<<" cellID="<<cellID
        //<<" factor45="<<factor45<<std::endl;
        //}

        shower.energy(shower.energy() / factor45);
        double factor452 = factor45 * factor45;
        shower.error(0, shower.error(0) / factor452); // Energy diagonal.
        shower.error(1, shower.error(1) / factor45); // Energy-phi.
        shower.error(3, shower.error(3) / factor45); // Energy-theta.

        // Here, also take care of Belle::Mdst_gamma.
        for (std::vector<Belle::Mdst_gamma>::iterator itgam45 = gammamgr.begin();
             itgam45 != gammamgr.end(); ++itgam45) {
          Belle::Mdst_gamma& gamma45 = *itgam45;
          if (gamma45.ecl().get_ID() == shower.get_ID()) {
            gamma45.px(gamma45.px() / factor45);
            gamma45.py(gamma45.py() / factor45);
            gamma45.pz(gamma45.pz() / factor45);
            //int idgam=gamma45.get_ID();
            //if( factor45!=1.0 )
            //{
            // dout(Debugout::DDEBUG,"B2BIIFixBelle::Mdst_ecl")<< "Exp45 fix idgamma="<<idgam<<std::endl;
            //}
          }
        }
      }

      // control sequence by option and expmc.
      // option=0
      switch (option) {
        case 0: // Option set as default.
          if (expmc == 2) { // This event record is MC data.
            // processed b20080331 or older, already skip.
            //KM std::cout<<"mdst_ecl"<<std::endl;
            factor
              = ecl_mcx3_corr(Eno, Rno, shower_energy, cos(shower_theta));
          } else { // Exp. data.
            //Original definition.
            //factor
            // = ecl_adhoc_corr( Eno, Rno,
            //  shower_energy, cos(shower_theta));
            //Modified 20081222
            factor
              = ecl_adhoc_corr(Eno, Rno, ecl_threshold,
                               shower_energy, cos(shower_theta));
            // Special treatment of Exp.15 processed by b20020125.
            if (Eno == 15 && i_previous == 1) {
              // dout(Debugout::INFO,"B2BIIFixBelle::Mdst_ecl") << "Exp.15 version=1 applied case!" << std::endl;
              //dout(Debugout::INFO,"B2BIIFixBelle::Mdst_ecl") << "ecl factor=" << factor << " " ;
              // Original scaling is done by Exp.13 curve.
              //Original definition.
              //factor13
              //= ecl_adhoc_corr( 13, 1,
              //shower_energy, cos(shower_theta));
              //Modified 20081222
              factor13
                = ecl_adhoc_corr(13, 1, ecl_threshold,
                                 shower_energy, cos(shower_theta));
              factor = factor / factor13;
              //dout(Debugout::INFO,"B2BIIFixBelle::Mdst_ecl") << "factor(rel)=" << factor << " ";
            }
          }
          break;
        case 1:
          if (expmc == 2) { // This event record is MC data.
            factor = 1.0 / mpi0pdg(shower.energy());

          } else { // This event record is real data.
            //Original definition.
            //factor
            // = ecl_adhoc_corr( Eno, Rno,
            //     shower.energy(), cos(shower.theta()));
            //Modified 20081222
            factor
              = ecl_adhoc_corr(Eno, Rno, ecl_threshold,
                               shower.energy(), cos(shower.theta()));
            // Special treatment of Exp.15 processed by b20020125.
            if (Eno == 15 && i_previous == 1) {
              // Original scaling is done by Exp.13 curve.
              //Original definition.
              //factor13
              // = ecl_adhoc_corr( 13, 1,
              //         shower_energy, cos(shower_theta));
              //Modified 20081222
              factor13
                = ecl_adhoc_corr(13, 1, ecl_threshold,
                                 shower_energy, cos(shower_theta));
              factor = factor / factor13;
              //dout(Debugout::INFO,"B2BIIFixBelle::Mdst_ecl") << "factor(rel)=" << factor << " ";
            }

            factor = factor / mpi0pdg(shower.energy());
          }
          break;
        default:
          factor = 1.0;
      }

      // energy correction.
      shower.energy(shower.energy() / factor);

      // error matrix correction.
      double factor2 = factor * factor;
      shower.error(0, shower.error(0) / factor2); // Energy diagonal.
      shower.error(1, shower.error(1) / factor); // Energy-phi.
      shower.error(3, shower.error(3) / factor); // Energy-theta.

      // Incriment Belle::Mdst_ecl_aux pointer.
      ++itaux;
    }

    // Correct energy in Belle::Mdst_gamma.
    for (std::vector<Belle::Mdst_gamma>::iterator itgam = gammamgr.begin();
         itgam != gammamgr.end(); ++itgam) {
      Belle::Mdst_gamma& gamma = *itgam;
      // Create the gamma's 3vector
      CLHEP::Hep3Vector gamma_3v(gamma.px(), gamma.py(), gamma.pz());
      double gamma_energy = gamma_3v.mag();
      double gamma_cos    = gamma_3v.cosTheta();

      // control sequence by option and expmc.
      switch (option) {
        case 0: // Option as default.
          if (expmc == 2) { // This event record is MC data.
            // processed b20080331 or older, already skip.
            //KM std::cout<<"mdst_gamma"<<std::endl;
            factor
              = ecl_mcx3_corr(Eno, Rno, gamma_energy, gamma_cos);
          } else { // Exp. data.
            //Original definition
            //factor
            // = ecl_adhoc_corr( Eno, Rno,
            //     gamma_energy, gamma_cos);
            //Modified 20081222
            factor
              = ecl_adhoc_corr(Eno, Rno, ecl_threshold,
                               gamma_energy, gamma_cos);
            // Special treatment of Exp.15 processed by b20020125.
            if (Eno == 15 && i_previous == 1) {
              // dout(Debugout::INFO,"B2BIIFixBelle::Mdst_ecl") << "Exp.15 version=1 applied case!" << std::endl;
              //dout(Debugout::INFO,"B2BIIFixBelle::Mdst_ecl") << "factor=" << factor << " " ;
              // Original scaling is done by Exp.13 curve.
              //Original definition.
              //factor13
              // = ecl_adhoc_corr( 13, 1,
              //         gamma_energy, gamma_cos);
              //Modified 20081222
              factor13
                = ecl_adhoc_corr(13, 1, ecl_threshold,
                                 gamma_energy, gamma_cos);
              factor = factor / factor13;
              //dout(Debugout::INFO,"B2BIIFixBelle::Mdst_ecl") << "gamma factor(rel)=" << factor << " " << std::endl;
            }
          }
          break;
        case 1:
          if (expmc == 2) {
            factor = 1.0 / mpi0pdg(gamma_3v.mag());

          } else {
            //Original definition.
            //factor
            // = ecl_adhoc_corr( Eno, Rno,
            //     gamma_3v.mag(), gamma_3v.cosTheta());
            //Modified 20081222
            factor
              = ecl_adhoc_corr(Eno, Rno, ecl_threshold,
                               gamma_3v.mag(), gamma_3v.cosTheta());
            // Special treatment of Exp.15 processed by b20020125.
            if (Eno == 15 && i_previous == 1) {
              // dout(Debugout::INFO,"B2BIIFixBelle::Mdst_ecl") << "Exp.15 version=1 applied case!" << std::endl;
              //dout(Debugout::INFO,"B2BIIFixBelle::Mdst_ecl") << "factor=" << factor << " " ;
              // Original scaling is done by Exp.13 curve.
              //Original definition.
              //factor13
              // = ecl_adhoc_corr( 13, 1,
              //         gamma_energy, gamma_cos);
              //Modified 20081222
              factor13
                = ecl_adhoc_corr(13, 1, ecl_threshold,
                                 gamma_energy, gamma_cos);
              factor = factor / factor13;
              //dout(Debugout::INFO,"B2BIIFixBelle::Mdst_ecl") << "gamma factor(rel)=" << factor << " " << std::endl;
            }
            factor = factor / mpi0pdg(gamma_3v.mag());
          }
          break;
        default:
          factor = 1.0;
      }

      // factor should be used as a division.
      gamma.px(gamma.px() / factor);
      gamma.py(gamma.py() / factor);
      gamma.pz(gamma.pz() / factor);
    }
  }


//=====================================================================
//***** make_pi0.cc ********** created 2001/Jul./21st *****
// Create Belle::Mdst_pi0 from Belle::Mdst_gamma and Belle::Mdst_ecl to let
// people get mass-constraint fitted momentum of pi0
// after ad_hoc correction.
// input : option = 0; same as the existent Rececl_pi0
//                     low_limit and up_limit are ignored.
//         option = 1; users can modify mass window as they
//                    like. The boundary of window is defined
//                    by low_limit ans up_limit (in GeV).
//         option = 2; users can modify mass window in the manner of
//                    -Xsigma < Mgg - Mpi0 < +Xsigma. The value of
//                    sigma(mass resolution) is calculated by pi0resol
//                    function which is originally coded by Hayashii-san.
//*********************************************************
  void B2BIIFixMdstModule::make_pi0(int option, double low_limit, double up_limit)
  {
    //---------------------
    // Check Exp. number and Data/MC.
    //---------------------
    Belle::Belle_event_Manager& evtmgr = Belle::Belle_event_Manager::get_manager();
    if (0 == evtmgr.count()) {
      // Do nothing if not exist, because we can not know whether
      // this event is Exp. data or MC.
      return;
    }
    //Check data or mc. Data:expmc=1, MC:expmc=2.
    int expmc = evtmgr[0].ExpMC();
    // Check Exp. number.
    int Eno = evtmgr[0].ExpNo();
    // Set mcdata frag to be compatible with pi0resol func.
    bool mcdata = 0;
    if (expmc == 2) {
      mcdata = 1;
    }

    // pi0 mass of PDG.
    const double mpi0_pdg = 0.1349739;

    // Default mass window.
    // const double low_default = 0.1178;
    // const double up_default  = 0.1502;
    const double low_default = 0.080;  // modified 20040606
    const double up_default  = 0.180;  // modified 20040606

    // Maximum iteration of fit.
    const int iter_max = 5;

    // Check whether proper option and mass window are given.
    switch (option) {
      case 0:    // option=0 case, set default mass window.
        low_limit = low_default;
        up_limit = up_default;
        break;
      case 1:   // option=1 case, check given mass window.
        if (mpi0_pdg < low_limit || up_limit < mpi0_pdg) {
          // If mass window is not correct, do nothing.
          B2ERROR("Invalid mass window between ");
          B2ERROR(" and " << up_limit);
          return;
        }
        break;
      case 2: // pi0 cand. are selected by -Xsigma < Mgg-Mpi0 < +Xsigma.
        // dout(Debugout::DDEBUG,"B2BIIFixBelle::Mdst_ecl") << "option=2 was selected." << std::endl;
        //dbg dout(Debugout::INFO,"B2BIIFixBelle::Mdst_ecl") << "mass window is " << low_limit << " & ";
        //dbg dout(Debugout::INFO,"B2BIIFixBelle::Mdst_ecl") << up_limit << std::endl;
        if (0.0 < low_limit || up_limit < 0.0) {
          B2ERROR("option=2 was selected. ");
          B2ERROR("Invalid mass window! " << low_limit);
          B2ERROR(" sould be negative, or " << up_limit);
          B2ERROR(" should be positive.");
          return;
        }
        break;
      default: // Otherwise, invalid option, do nothing.
        B2ERROR("Invalid option=" << option);
        return;
    }

    // At first, clear already existing Belle::Mdst_pi0.
    Belle::Mdst_pi0_Manager::get_manager().remove();

    // Get Belle::Mdst_gamma for photon's momentum
    // and get Belle::Mdst_ecl for error matrix.
    Belle::Mdst_gamma_Manager& gammamgr = Belle::Mdst_gamma_Manager::get_manager();

    // Re-allocate Belle::Mdst_pi0 table.
    Belle::Mdst_pi0_Manager& pi0mgr = Belle::Mdst_pi0_Manager::get_manager();

    // If Only one photon in the event, no need to do anything.
    if (gammamgr.count() < 2) {
      return;
    }

    // Make combination of two Belle::Mdst_gamma.
    for (std::vector<Belle::Mdst_gamma>::iterator itgamma = gammamgr.begin();
         itgamma != gammamgr.end(); ++itgamma) {
      Belle::Mdst_gamma& gamma1 = *itgamma;
      CLHEP::Hep3Vector gamma1_3v(gamma1.px(), gamma1.py(), gamma1.pz());
      CLHEP::HepLorentzVector gamma1_lv(gamma1_3v, gamma1_3v.mag());

      Belle::Mdst_ecl& ecl1 = gamma1.ecl();

      for (std::vector<Belle::Mdst_gamma>::iterator jtgamma = itgamma + 1;
           jtgamma != gammamgr.end(); ++jtgamma) {
        Belle::Mdst_gamma& gamma2 = *jtgamma;
        CLHEP::Hep3Vector gamma2_3v(gamma2.px(), gamma2.py(), gamma2.pz());
        CLHEP::HepLorentzVector gamma2_lv(gamma2_3v, gamma2_3v.mag());

        Belle::Mdst_ecl& ecl2 = gamma2.ecl();

        // Invariant mass before const. fit.
        CLHEP::HepLorentzVector gamgam_lv = gamma1_lv + gamma2_lv;
        const double mass_before = gamgam_lv.mag();

        // In the case of option=0 or 1, criteria is controlled
        // by the inv. mass.
        double mass_ctrl{0};
        if (option == 0 || option == 1) {
          mass_ctrl = mass_before;
        }
        if (option == 2) {
          // Asymmetric lineshape is taken into account.
          if (mass_before * mpi0pdg(mass_before) < mpi0_pdg) {
            mass_ctrl = (mass_before * mpi0pdg(mass_before) - mpi0_pdg) /
                        pi0resol(gamgam_lv.vect().mag(),
                                 gamgam_lv.theta() * 180. / M_PI, "lower",
                                 mcdata, Eno, option);
          } else {
            mass_ctrl = (mass_before * mpi0pdg(mass_before) - mpi0_pdg) /
                        pi0resol(gamgam_lv.vect().mag(),
                                 gamgam_lv.theta() * 180. / M_PI, "higher",
                                 mcdata, Eno, option);
          }
        }

        // If invariant mass is inside the window,
        // if( low_limit < mass_before && mass_before < up_limit ) // old.
        if (low_limit < mass_ctrl && mass_ctrl < up_limit) {
          // dout(Debugout::DDEBUG,"B2BIIFixBelle::Mdst_ecl")<<"mass="<<mass_before;
          // dout(Debugout::DDEBUG,"B2BIIFixBelle::Mdst_ecl")<<" p="<<gamgam_lv.vect().mag()<<" theta=";
          // dout(Debugout::DDEBUG,"B2BIIFixBelle::Mdst_ecl")<<gamgam_lv.theta()<<" mcdata="<<mcdata;
          // dout(Debugout::DDEBUG,"B2BIIFixBelle::Mdst_ecl")<<" higher="<<
          //   pi0resol(gamgam_lv.vect().mag(),
          //    gamgam_lv.theta()*180./M_PI,
          //  "higher",mcdata, Eno, option )<<" or ";
          // dout(Debugout::DDEBUG,"B2BIIFixBelle::Mdst_ecl")<<" lower="<<
          //   pi0resol(gamgam_lv.vect().mag(),
          //    gamgam_lv.theta()*180./M_PI,
          //  "lower",mcdata, Eno, option )<<std::endl;
          // Error matrix(covariant matrix)
          CLHEP::HepMatrix V(6, 6, 0);

          V[0][0] = ecl1.error(0);
          V[0][1] = V[1][0] = ecl1.error(1);
          V[1][1] = ecl1.error(2);
          V[0][2] = V[2][0] = ecl1.error(3);
          V[1][2] = V[2][1] = ecl1.error(4);
          V[2][2] = ecl1.error(5);
          V[3][3] = ecl2.error(0);
          V[3][4] = V[4][3] = ecl2.error(1);
          V[4][4] = ecl2.error(2);
          V[3][5] = V[5][3] = ecl2.error(3);
          V[4][5] = V[5][4] = ecl2.error(4);
          V[5][5] = ecl2.error(5);

          // Measurements; i.e. initial parameters.
          CLHEP::HepMatrix y0(6, 1);
          y0[0][0] = ecl1.energy();
          y0[1][0] = ecl1.phi();
          y0[2][0] = ecl1.theta();
          y0[3][0] = ecl2.energy();
          y0[4][0] = ecl2.phi();
          y0[5][0] = ecl2.theta();

          // Copy them to proper matrix which is given to fit.
          CLHEP::HepMatrix y(y0);
          // Delivative.
          CLHEP::HepMatrix Dy(6, 1, 0);

          int iter = 0;
          double f_old = DBL_MAX;
          double chi2_old = DBL_MAX;
          double /*mass_gg,*/ chi2 = DBL_MAX;
          bool exit_flag = false;

          // Set parameters to decide whether converged.
          const double Df_limit = 0.1;
          const double Dchi2_limit = 0.1;
          // Do mass constraint fit; iterate until convergence.
          while (1) {
            const double& E1 = y[0][0];
            const double& E2 = y[3][0];
            const double sin_theta1 = std::sin(y[2][0]);
            const double cos_theta1 = std::cos(y[2][0]);
            const double sin_theta2 = std::sin(y[5][0]);
            const double cos_theta2 = std::cos(y[5][0]);
            const double Dphi = y[1][0] - y[4][0];
            const double cos_Dphi = std::cos(Dphi);
            const double sin_Dphi = std::sin(Dphi);
            const double open_angle = sin_theta1 * sin_theta2 * cos_Dphi
                                      + cos_theta1 * cos_theta2;
            const double mass2_gg = 2 * E1 * E2 * (1 - open_angle);
            //mass_gg = (mass2_gg > 0) ? std::sqrt(mass2_gg) : -std::sqrt(-mass2_gg);

            // No more iteration. Break to return.
            if (exit_flag || ++iter > iter_max)
              break;

            // constraint
            CLHEP::HepMatrix f(1, 1);
            f[0][0] = mass2_gg - (mpi0_pdg * mpi0_pdg);

            // dG/dq_i, G = (M_gg - M_pi0) = 0 is the constraint.
            CLHEP::HepMatrix B(1, 6);
            B[0][0] = mass2_gg / E1;
            B[0][1] = 2 * E1 * E2 * sin_theta1 * sin_theta2 * sin_Dphi;
            B[0][2] = 2 * E1 * E2 * (-cos_theta1 * sin_theta2 * cos_Dphi
                                     + sin_theta1 * cos_theta2);
            B[0][3] = mass2_gg / E2;
            B[0][4] = -B[0][1];
            B[0][5] = 2 * E1 * E2 * (-sin_theta1 * cos_theta2 * cos_Dphi
                                     + cos_theta1 * sin_theta2);

            const double sigma2_mass2_gg = (B * V * B.T())[0][0];

            // Proceed one step to get newer value y.
            Dy = V * B.T() * (B * Dy - f) / sigma2_mass2_gg;
            y = y0 + Dy;
            int ierr;
            chi2 = (Dy.T() * V.inverse(ierr) * Dy)[0][0];
            double Dchi2 = fabs(chi2 - chi2_old);
            chi2_old = chi2;
            double Df = fabs(f[0][0] - f_old);
            f_old = f[0][0];

            // When chi-sq. change is small enough and mass is
            if (Dchi2 < Dchi2_limit && Df < Df_limit)
              exit_flag = true;;

          }
          // Sort fitted result into proper variables.
          double pi0_E = y[0][0] + y[3][0];
          double pi0_px = y[0][0] * std::cos(y[1][0]) * std::sin(y[2][0])
                          + y[3][0] * std::cos(y[4][0]) * std::sin(y[5][0]);
          double pi0_py = y[0][0] * std::sin(y[1][0]) * std::sin(y[2][0])
                          + y[3][0] * std::sin(y[4][0]) * std::sin(y[5][0]);
          double pi0_pz = y[0][0] * std::cos(y[2][0]) + y[3][0] * std::cos(y[5][0]);
          double pi0_mass = mass_before;

          //  m_chi2 = chi2;  // protect...
          double pi0_chi2 = (chi2 > FLT_MAX) ? FLT_MAX : chi2;

          // Fill Belle::Mdst_pi0 based on the fit result.
          Belle::Mdst_pi0& pi0 = pi0mgr.add();
          pi0.gamma(0, gamma1);
          pi0.gamma(1, gamma2);
          pi0.px(pi0_px);
          pi0.py(pi0_py);
          pi0.pz(pi0_pz);
          pi0.energy(pi0_E);
          pi0.mass(pi0_mass);
          pi0.chisq(pi0_chi2);
        }
      }
    }
  }

  void B2BIIFixMdstModule::make_pi0_primary_vertex(int option, double low_limit, double up_limit,
                                                   const HepPoint3D& /*epvtx*/,
                                                   const CLHEP::HepSymMatrix& epvtx_err)
  {
#if 0
    // pi0 mass of PDG.
    const double mpi0_pdg = 0.1349739;

    // Default mass window.
    const double low_default = 0.1178;
    const double up_default  = 0.1502;

    // Maximum iteration of fit.
    const int iter_max = 5;

    // Check whether proper option and mass window are given.
    switch (option) {
      case 0:    // option=0 case, set default mass window.
        low_limit = low_default;
        up_limit = up_default;
        break;
      case 1:   // option=1 case, check given mass window.
        if (mpi0_pdg < low_limit || up_limit < mpi0_pdg) {
          // If mass window is not correct, do nothing.
          dout(Debugout::ERR, "B2BIIFixBelle::Mdst_ecl") << "Invalid mass window between " << low_limit;
          dout(Debugout::ERR, "B2BIIFixBelle::Mdst_ecl") << " and " << up_limit << std::endl;
          return;
        }
        break;
      default: // Otherwise, invalid option, do nothing.
        dout(Debugout::ERR, "B2BIIFixBelle::Mdst_ecl") << "Invalid option=" << option << std::endl;
        return;
    }

    // At first, clear already existing Belle::Mdst_pi0.
    Belle::Mdst_pi0_Manager::get_manager().remove();

    // Get Belle::Mdst_gamma for photon's momentum
    // and get Belle::Mdst_ecl for error matrix.
    Belle::Mdst_gamma_Manager& gammamgr = Belle::Mdst_gamma_Manager::get_manager();

    // Re-allocate Belle::Mdst_pi0 table.
    Belle::Mdst_pi0_Manager& pi0mgr = Belle::Mdst_pi0_Manager::get_manager();

    // If Only one photon in the event, no need to do anything.
    if (gammamgr.count() < 2) {
      return;
    }

    // Make combination of two Belle::Mdst_gamma.
    for (std::vector<Belle::Mdst_gamma>::iterator itgamma = gammamgr.begin();
         itgamma != gammamgr.end(); ++itgamma) {
      Belle::Mdst_gamma& gamma1 = *itgamma;
      CLHEP::Hep3Vector gamma1_3v(gamma1.px(), gamma1.py(), gamma1.pz());
      CLHEP::HepLorentzVector gamma1_lv(gamma1_3v, gamma1_3v.mag());

      Belle::Mdst_ecl& ecl1 = gamma1.ecl();

      const double r_i       = ecl1.r();
      const double dzr_i     = std::sqrt(epvtx_err[2][2]) / r_i;

      const double theta_i0  = ecl1.theta();
      const double sin_th_i0 = std::sin(theta_i0);

      for (std::vector<Belle::Mdst_gamma>::iterator jtgamma = itgamma + 1;
           jtgamma != gammamgr.end(); ++jtgamma) {
        Belle::Mdst_gamma& gamma2 = *jtgamma;
        CLHEP::Hep3Vector gamma2_3v(gamma2.px(), gamma2.py(), gamma2.pz());
        CLHEP::HepLorentzVector gamma2_lv(gamma2_3v, gamma2_3v.mag());

        Belle::Mdst_ecl& ecl2 = gamma2.ecl();

        // Invariant mass before const. fit.
        CLHEP::HepLorentzVector gamgam_lv = gamma1_lv + gamma2_lv;
        const double mass_before = gamgam_lv.mag();

        // If invariant mass is inside the window,
        if (low_limit < mass_before && mass_before < up_limit) {
          // Error matrix(covariant matrix)
          CLHEP::HepMatrix V(6, 6, 0);

          V[0][0] = ecl1.error(0);
          V[0][1] = V[1][0] = ecl1.error(1);
          V[1][1] = ecl1.error(2);
          V[0][2] = V[2][0] = ecl1.error(3);
          V[1][2] = V[2][1] = ecl1.error(4);
          V[2][2] = ecl1.error(5);
          V[3][3] = ecl2.error(0);
          V[3][4] = V[4][3] = ecl2.error(1);
          V[4][4] = ecl2.error(2);
          V[3][5] = V[5][3] = ecl2.error(3);
          V[4][5] = V[5][4] = ecl2.error(4);
          V[5][5] = ecl2.error(5);

          // Correlation term
          const double r_j      = ecl2.r();
          const double dzr_j    = sqrt(epvtx_err[2][2]) / r_j;

          const double theta_j0  = ecl2.theta();
          const double sin_th_j0 = std::sin(theta_j0);

          V[2][5] = V[5][2] = dzr_i * sin_th_i0 * dzr_j * sin_th_j0;
          // Measurements; i.e. initial parameters.
          CLHEP::HepMatrix y0(6, 1);
          y0[0][0] = ecl1.energy();
          y0[1][0] = ecl1.phi();
          y0[2][0] = ecl1.theta();
          y0[3][0] = ecl2.energy();
          y0[4][0] = ecl2.phi();
          y0[5][0] = ecl2.theta();

          // Copy them to proper matrix which is given to fit.
          CLHEP::HepMatrix y(y0);
          // Delivative.
          CLHEP::HepMatrix Dy(6, 1, 0);

          int iter = 0;
          double Df, f_old = DBL_MAX;
          double Dchi2, chi2_old = DBL_MAX;
          double mass_gg, chi2 = DBL_MAX;
          bool exit_flag = false;

          // Set parameters to decide whether converged.
          const double Df_limit = 0.1;
          const double Dchi2_limit = 0.1;
          // Do mass constraint fit; iterate until convergence.
          while (1) {
            const double& E1 = y[0][0];
            const double& E2 = y[3][0];
            const double sin_theta1 = std::sin(y[2][0]);
            const double cos_theta1 = std::cos(y[2][0]);
            const double sin_theta2 = std::sin(y[5][0]);
            const double cos_theta2 = std::cos(y[5][0]);
            const double Dphi = y[1][0] - y[4][0];
            const double cos_Dphi = std::cos(Dphi);
            const double sin_Dphi = std::sin(Dphi);
            const double open_angle = sin_theta1 * sin_theta2 * cos_Dphi
                                      + cos_theta1 * cos_theta2;
            const double mass2_gg = 2 * E1 * E2 * (1 - open_angle);
            mass_gg = (mass2_gg > 0) ? std::sqrt(mass2_gg) : -std::sqrt(-mass2_gg);

            // No more iteration. Break to return.
            if (exit_flag || ++iter > iter_max)
              break;

            // constraint
            CLHEP::HepMatrix f(1, 1);
            f[0][0] = mass2_gg - (mpi0_pdg * mpi0_pdg);

            // dG/dq_i, G = (M_gg - M_pi0) = 0 is the constraint.
            CLHEP::HepMatrix B(1, 6);
            B[0][0] = mass2_gg / E1;
            B[0][1] = 2 * E1 * E2 * sin_theta1 * sin_theta2 * sin_Dphi;
            B[0][2] = 2 * E1 * E2 * (-cos_theta1 * sin_theta2 * cos_Dphi
                                     + sin_theta1 * cos_theta2);
            B[0][3] = mass2_gg / E2;
            B[0][4] = -B[0][1];
            B[0][5] = 2 * E1 * E2 * (-sin_theta1 * cos_theta2 * cos_Dphi
                                     + cos_theta1 * sin_theta2);

            const double sigma2_mass2_gg = (B * V * B.T())[0][0];

            // Proceed one step to get newer value y.
            Dy = V * B.T() * (B * Dy - f) / sigma2_mass2_gg;
            y = y0 + Dy;
            int ierr;
            chi2 = (Dy.T() * V.inverse(ierr) * Dy)[0][0];
            Dchi2 = fabs(chi2 - chi2_old);
            chi2_old = chi2;
            Df = fabs(f[0][0] - f_old);
            f_old = f[0][0];

            // When chi-sq. change is small enough and mass is
            if (Dchi2 < Dchi2_limit && Df < Df_limit)
              exit_flag = true;;

          }
          // Sort fitted result into proper variables.
          double pi0_E = y[0][0] + y[3][0];
          double pi0_px = y[0][0] * std::cos(y[1][0]) * std::sin(y[2][0])
                          + y[3][0] * std::cos(y[4][0]) * std::sin(y[5][0]);
          double pi0_py = y[0][0] * std::sin(y[1][0]) * std::sin(y[2][0])
                          + y[3][0] * std::sin(y[4][0]) * std::sin(y[5][0]);
          double pi0_pz = y[0][0] * std::cos(y[2][0]) + y[3][0] * std::cos(y[5][0]);
          double pi0_mass = mass_before;

          //  m_chi2 = chi2;  // protect...
          double pi0_chi2 = (chi2 > FLT_MAX) ? FLT_MAX : chi2;

          // Fill Belle::Mdst_pi0 based on the fit result.
          Belle::Mdst_pi0& pi0 = pi0mgr.add();
          pi0.gamma(0, gamma1);
          pi0.gamma(1, gamma2);
          pi0.px(pi0_px);
          pi0.py(pi0_py);
          pi0.pz(pi0_pz);
          pi0.energy(pi0_E);
          pi0.mass(pi0_mass);
          pi0.chisq(pi0_chi2);
        }
      }
    }
#else
    // pi0 mass of PDG. ;
    const double mpi0_pdg = 0.1349739;
    const double m2pi0_pdg = mpi0_pdg * mpi0_pdg;
    // Default mass window.;
    const double low_default = 0.1178;
    const double up_default  = 0.1502;
    // Maximum iteration of fit.;
    const int iter_max = 5;

    // Check whether proper option and mass window are given.;
    switch (option) {
      case 0:    // option=0 case, set default mass window.;
        low_limit = low_default;
        up_limit = up_default;
        break;
      case 1:   // option=1 case, check given mass window.;
        if (mpi0_pdg < low_limit || up_limit < mpi0_pdg) {
          // If mass window is not correct, do nothing.;
          B2ERROR("Invalid mass window between " << low_limit);
          B2ERROR(" and " << up_limit);
          return;
        }
        break;
      default: // Otherwise, invalid option, do nothing.;
        B2ERROR("Invalid option=" << option);
        return;
    }
    Belle::Mdst_pi0_Manager&  pi0_mgr     = Belle::Mdst_pi0_Manager::get_manager();
    Belle::Mdst_gamma_Manager& gamma_mgr  = Belle::Mdst_gamma_Manager::get_manager();

    // At first, clear already existing Belle::Mdst_pi0.;
    Belle::Mdst_pi0_Manager::get_manager().remove();
    // If Only one photon in the event, no need to do anything.;
    if (gamma_mgr.count() < 2) {
      return;
    }

    for (std::vector<Belle::Mdst_gamma>::iterator i = gamma_mgr.begin();
         i != gamma_mgr.end(); ++i) {
      const Belle::Mdst_gamma& gamma_i = *i;
      if (!gamma_i.ecl()) {
        continue;
      }
      const Belle::Mdst_ecl& ecl_i = gamma_i.ecl();
      const double r_i      = ecl_i.r();
      const double e_i0     = ecl_i.energy();
      const double phi_i0   = ecl_i.phi();
      const double theta_i0 = ecl_i.theta();

      const double sin_th_i0 = std::sin(theta_i0);
      const double cos_th_i0 = std::cos(theta_i0);

      CLHEP::HepSymMatrix err_i(3, 0);
      err_i[0][0] = ecl_i.error(0);
      err_i[1][0] = ecl_i.error(1); err_i[1][1] = ecl_i.error(2);
      err_i[2][0] = ecl_i.error(3); err_i[2][1] = ecl_i.error(4); err_i[2][2] = ecl_i.error(5);

      const double dzr_i = std::sqrt(epvtx_err[2][2]) / r_i;

      for (std::vector<Belle::Mdst_gamma>::iterator j = i + 1; j != gamma_mgr.end(); ++j) {
        const Belle::Mdst_gamma& gamma_j = *j;
        if (!gamma_j.ecl()) {
          continue;
        }
        const Belle::Mdst_ecl& ecl_j = gamma_j.ecl();
        const double r_j      = ecl_j.r();
        const double e_j0     = ecl_j.energy();
        const double phi_j0   = ecl_j.phi();
        const double theta_j0 = ecl_j.theta();

        const double sin_th_j0 = std::sin(theta_j0);
        const double cos_th_j0 = std::cos(theta_j0);

        CLHEP::HepSymMatrix err_j(3, 0);
        err_j[0][0] = ecl_j.error(0);
        err_j[1][0] = ecl_j.error(1); err_j[1][1] = ecl_j.error(2);
        err_j[2][0] = ecl_j.error(3); err_j[2][1] = ecl_j.error(4); err_j[2][2] = ecl_j.error(5);

        const double dzr_j = sqrt(epvtx_err[2][2]) / r_j;

        // -------------------------------------------------------------------- ;

        double dth_i_x_dth_j = dzr_i * sin_th_i0 * dzr_j * sin_th_j0;

        double det = err_i[2][2] * err_j[2][2] - dth_i_x_dth_j * dth_i_x_dth_j;


        double e_i = e_i0, phi_i = phi_i0, theta_i = theta_i0, sin_th_i = sin_th_i0, cos_th_i = cos_th_i0;
        double e_j = e_j0, phi_j = phi_j0, theta_j = theta_j0, sin_th_j = sin_th_j0, cos_th_j = cos_th_j0;


        double dphi     = phi_i - phi_j;
        double sin_dphi = std::sin(dphi);
        double cos_dphi = std::cos(dphi);

        // Cos of opening angle ;
        double cos_th_ij = sin_th_i * sin_th_j * cos_dphi + cos_th_i * cos_th_j;
        double m2_ij     = 2.0 * e_i * e_j * (1.0 - cos_th_ij);
        double mass      = std::sqrt(m2_ij);

        if (mass < low_limit || mass > up_limit)
          continue;

        double de_i = 0.0, dphi_i = 0.0, dtheta_i = 0.0;
        double de_j = 0.0, dphi_j = 0.0, dtheta_j = 0.0;

        int it = 0;

        double f      = 1.0e+6;
        double chisq  = 1.0e+6;
        double df = 0.0, dchisq = 0.0;

        const double del_chisq = 0.1;
        const double del_f   = 0.1;
        //const int icovar  = 1;

        double mass0     = 0.0;
        double dmass0    = 0.0;
        //double dmass     = 0.0;
        //double sdev      = 0.0;

        do {

          const double dcos_th_ij_dphi_i = -sin_th_i * sin_th_j * sin_dphi;
          const double dcos_th_ij_dphi_j =  sin_th_i * sin_th_j * sin_dphi;
          const double dcos_th_ij_dth_i  =  cos_th_i * sin_th_j * cos_dphi - sin_th_i * cos_th_j;
          const double dcos_th_ij_dth_j  =  cos_th_j * sin_th_i * cos_dphi - sin_th_j * cos_th_i;

          const double dm2_de_i  = m2_ij / e_i;
          const double dm2_de_j  = m2_ij / e_j;

          const double dm2_dcos_th_ij = -2.0 * e_i * e_j;

          const double dm2_dphi_i = dm2_dcos_th_ij * dcos_th_ij_dphi_i;
          const double dm2_dphi_j = dm2_dcos_th_ij * dcos_th_ij_dphi_j;
          const double dm2_dth_i  = dm2_dcos_th_ij * dcos_th_ij_dth_i;
          const double dm2_dth_j  = dm2_dcos_th_ij * dcos_th_ij_dth_j;

          double dm2_2
            = dm2_de_i * dm2_de_i     * err_i[0][0]
              + dm2_de_j * dm2_de_j     * err_j[0][0]
              + dm2_dphi_i * dm2_dphi_i * err_i[1][0]
              + dm2_dphi_j * dm2_dphi_j * err_j[1][0]
              + dm2_dth_i * dm2_dth_i   * err_i[2][0]
              + dm2_dth_j * dm2_dth_j   * err_j[2][0]
              + 2.0 * dm2_dth_i * dm2_dth_j * dth_i_x_dth_j;

          if (dm2_2 < 0.0) {
            dm2_2 = 0.0;
          }
          if (it++ == 0) {
            mass0  = mass;
            dmass0 = std::sqrt(dm2_2);
            /*if (mass0 > 0.0) {
              dmass = 0.5 * dmass0 / mass;
            }*/
          }
          //const double residual = mass - mpi0_pdg;
          //const double pull     = residual / dmass;
          if (it >= iter_max ||
              mass < low_default || mass > up_default) {
            it = -it;
            //sdev = (dmass > 0.0) ? pull : -100;
            break;
          }

          const double del_m2
            = (m2pi0_pdg - m2_ij
               + dm2_de_i   * de_i
               + dm2_de_j   * de_j
               + dm2_dphi_i * dphi_i
               + dm2_dphi_j * dphi_j
               + dm2_dth_i  * dtheta_i
               + dm2_dth_j  * dtheta_j) / dm2_2;

          de_i     = del_m2 * dm2_de_i   * err_i[0][0];
          de_j     = del_m2 * dm2_de_j   * err_j[0][0];
          dphi_i   = del_m2 * dm2_dphi_i * err_i[1][1];
          dphi_j   = del_m2 * dm2_dphi_j * err_j[1][1];
          dtheta_i = del_m2 * (dm2_dth_i  * err_i[2][2] + dm2_dth_j  * dth_i_x_dth_j);
          dtheta_j = del_m2 * (dm2_dth_j  * err_j[2][2] + dm2_dth_i  * dth_i_x_dth_j);


          dchisq = chisq;
          chisq
            = de_i * de_i     / err_i[0][0]
              + de_j * de_j     / err_j[0][0]
              + dphi_i * dphi_i / err_i[1][1]
              + dphi_j * dphi_j / err_j[1][1]
              + (dtheta_i * dtheta_i * err_i[2][2]
                 + dtheta_j * dtheta_j * err_j[2][2]
                 + 2.0 * dtheta_i * dtheta_j * dth_i_x_dth_j) / det;
          dchisq -= chisq;


          e_i     = e_i0     + de_i;
          e_j     = e_j0     + de_j;
          phi_i   = phi_i0   + dphi_i;
          phi_j   = phi_j0   + dphi_j;
          theta_i = theta_i0 + dtheta_i;
          theta_j = theta_j0 + dtheta_j;

          sin_th_i = std::sin(theta_i);
          cos_th_i = std::cos(theta_i);
          sin_th_j = std::sin(theta_j);
          cos_th_j = std::cos(theta_j);

          dth_i_x_dth_j = dzr_i * sin_th_i * dzr_j * sin_th_j;
          det = err_i[2][2] * err_j[2][2] - dth_i_x_dth_j * dth_i_x_dth_j;

          dphi     = phi_i - phi_j;
          sin_dphi = std::sin(dphi);
          cos_dphi = std::cos(dphi);

          cos_th_ij = sin_th_i * sin_th_j * cos_dphi + cos_th_i * cos_th_j;
          m2_ij     = 2.0 * e_i * e_j * (1.0 - cos_th_ij);
          mass      = std::sqrt(m2_ij);

          df = f;
          f = fabs(m2pi0_pdg - m2_ij) / dmass0;
          df -= f;
          ++it;
        } while (std::fabs(df) > del_f || std::fabs(dchisq) > del_chisq);
        const double cos_phi_i = std::cos(phi_i);
        const double cos_phi_j = std::cos(phi_j);
        const double sin_phi_i = std::sin(phi_i);
        const double sin_phi_j = std::sin(phi_j);

        const CLHEP::HepLorentzVector p4_i(e_i * sin_th_i * cos_phi_i,
                                           e_i * sin_th_i * sin_phi_i, e_i * cos_th_i, e_i);
        const CLHEP::HepLorentzVector p4_j(e_j * sin_th_j * cos_phi_j,
                                           e_j * sin_th_j * sin_phi_j, e_j * cos_th_j, e_j);

        const CLHEP::HepLorentzVector p4_pi0(p4_i + p4_j);

        // Fill Belle::Mdst_pi0 based on the fit result.;
        Belle::Mdst_pi0& pi0 = pi0_mgr.add();
        pi0.gamma(0, gamma_i);
        pi0.gamma(1, gamma_j);
        pi0.px(p4_pi0.x());
        pi0.py(p4_pi0.y());
        pi0.pz(p4_pi0.z());
        pi0.energy(p4_pi0.e());
        pi0.mass(mass0);
        pi0.chisq(chisq);
      }
    }
#endif
    return;
  }

  void B2BIIFixMdstModule::correct_ecl_primary_vertex(const HepPoint3D& epvtx,
                                                      const CLHEP::HepSymMatrix& epvtx_err)
  {

    Belle::Mdst_gamma_Manager& Gamma  = Belle::Mdst_gamma_Manager::get_manager();

    for (std::vector<Belle::Mdst_gamma>::iterator
         it = Gamma.begin(); it != Gamma.end(); ++it) {
      double r(it->ecl().r());
      double theta(it->ecl().theta());
      double phi(it->ecl().phi());
      //    double stheta(std::sqrt(it->ecl().error(2)));
      //    double sphi(std::sqrt(it->ecl().error(5)));
      double st(std::sin(theta));
      double ct(std::cos(theta));
      double sp(std::sin(phi));
      double cp(std::cos(phi));
      HepPoint3D gamma_pos(r * st * cp, r * st * sp, r * ct);
      CLHEP::Hep3Vector gamma_vec(gamma_pos - epvtx);
      double hsq(gamma_vec.perp2());
      double rsq(gamma_vec.mag2());
      double stheta_sq_new = it->ecl().error(5) + epvtx_err(3, 3) * (hsq / (rsq * rsq));
      CLHEP::Hep3Vector gamma_dir(gamma_vec.unit());
      double e(it->ecl().energy());
      it->px(e * gamma_dir.x());
      it->py(e * gamma_dir.y());
      it->pz(e * gamma_dir.z());
      it->ecl().theta(gamma_dir.theta());
      it->ecl().r(gamma_vec.mag());
      it->ecl().error(5, stheta_sq_new);
    }
  }


} // namespace Belle
