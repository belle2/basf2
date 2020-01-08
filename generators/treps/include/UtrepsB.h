/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kiyoshi Hayasaka                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/logging/Logger.h>
#include <generators/treps/Particle_array.h>
#include <generators/treps/Sutool.h>
#include <generators/treps/Treps3B.h>

namespace Belle2 {
  /**
   * Input from TREPS generator for ee->eeff
   */

  class UtrepsB : public TrepsB {

  public:

    UtrepsB(void);
    ~UtrepsB() {};

    /** initialization of Pparametrization of pi+pi- partial waves */
    void initg() ;
    Interps b00, b20, b22; // Parameter sets

    /* virtual double tpform(double, double) const override ;
    /* virtual double tpangd(double, double) override ;
    /* virtual int tpuser(TLorentzVector, TLorentzVector, Part_gen*, int); // user decision routine for extra generation conditions. */

    // FormFactor effect */
    virtual double tpform(double _q2, double _w) const
    {
      double dis = 1. / pow((1.0 + _q2 / (_w * _w)), 2);
      if (TrepsB::fmodel == 1) {
        // rho form factor
        dis = 1. / pow((1.0 + _q2 / (0.77 * 0.77)), 2);
      }
      if (TrepsB::fmodel == 2) {
        // J/psi form factor
        dis = 1. / pow((1.0 + _q2 / (3.097 * 3.097)), 2);
      }

      return dis;
    }

    // angular distribution for final 2-body case */
    virtual double tpangd(double _z, double _w)
    {
      // no normalization is necessary

      double ww = _w;
      double zz = _z;

      double dcs;

      dcs = 1.;

      if (TrepsB::pmodel == 201) {
        // 2+(0) --> PP
        dcs = 3.*zz * zz - 1.;
        return dcs;
      }

      if (TrepsB::pmodel == 202) {
        // 2+(2) --> PP
        dcs = pow(1. - zz * zz, 2);
        return dcs;
      }

      if (TrepsB::pmodel == 251) {
        // pi+pi-
        double mr = 1.2755;
        double gtot = 0.1859;
        double br = 0.561;
        double sr = 3.62;
        double ggg = 2.62e-6;
        double phas = 22.8 / 180.*3.14159;
        double mr0 = 0.98;
        double gtot0 = 0.03;
        double speak = 18.3;
        double mm = 0.1396;

        double v00 = 0., v20 = 0., v22 = 0. ;

        if (ww < 0.295) {
          v00 = v20 = v22 = 0.0 ;
        } else if (ww < 2.3) {
          v00 = b00.get_val(ww);
          v20 = b20.get_val(ww);
          v22 = b22.get_val(ww);
        }

        double dcs2 = 0.;

        if (ww < 2.3) {
          double gf = pow(qfunc(ww * ww, mm) / qfunc(mr * mr, mm), 5) *
                      d2func(qfunc(ww * ww, mm) * sr) / d2func(qfunc(mr * mr, mm) * sr);
          dbcomplex  imagu = dbcomplex(0.0, 1.0);
          dbcomplex  rbr = sqrt(40.*3.14159 * mr / ww * ggg * gtot * br) * gf /
                           ((-ww * ww + mr * mr) - imagu * mr * gtot * gf) *
                           sqrt(389000.);
          rbr = (cos(phas) + imagu * sin(phas)) * rbr;

          dcs2 = pow(v00 + v20 * sqrt(5.) * 0.5 * (3.*zz * zz - 1.), 2) +
                 (v22 * v22 + 2.*v22 * rbr.real() + pow(rbr.abs(), 2)) *
                 15. / 8.*pow(1. - zz * zz, 2)
                 + speak * 0.25 * gtot0 * gtot0 / (pow(ww - mr0, 2) + 0.25 * gtot0 * gtot0);
        } else {
          if (abs(zz) < 0.8)  dcs2 = 263. / pow(ww, 6) / pow(1. - zz * zz, 2);
          else dcs2 = 263. / pow(ww, 6) / pow(1. - 0.8 * 0.8, 2);
        }
        return dcs2 * 0.5;
      }

      if (TrepsB::pmodel == 252) {
        //K+K-
        double mr = 1.522;
        double gtot = 0.0814;
        double br = 0.45;
        double sr = 3.62;
        double ggg = 0.081e-6;
        double mrfa = 1.29;
        double gtotfa = 0.13;
        double speak = 40.;
        double mm = 0.4937;

        double s00 = 0., s20 = 0., s22 = 0. ;

        if (ww < 1.1) {
          s00 = s20 = s22 = 0.0 ;
        } else if (ww < 2.4) {
          s00 = b00.get_val(ww);
          s20 = b20.get_val(ww);
          s22 = b22.get_val(ww);
        }

        double dcs2 = 0.;

        if (ww < 2.4) {
          double gf = pow(qfunc(ww * ww, mm) / qfunc(mr * mr, mm), 5) *
                      d2func(qfunc(ww * ww, mm) * sr) / d2func(qfunc(mr * mr, mm) * sr);
          dbcomplex  imagu = dbcomplex(0.0, 1.0);
          dbcomplex  rbr = sqrt(40.*3.14159 * mr / ww * ggg * gtot * br) * gf /
                           ((-ww * ww + mr * mr) - imagu * mr * gtot * gf) *
                           sqrt(389000.);

          dcs2 = s00 + s20 * pow(sqrt(5.) * 0.5 * (3.*zz * zz - 1.), 2) +
                 (s22 + pow(rbr.abs(), 2)
                  + speak * 0.25 * gtotfa * gtotfa / (pow(ww - mrfa, 2) + 0.25 * gtotfa * gtotfa)) *
                 15. / 8.*pow(1. - zz * zz, 2) ;
        } else {
          if (abs(zz) < 0.8)  dcs2 = 241. / pow(ww, 6) / pow(1. - zz * zz, 2);
          else dcs2 = 241. / pow(ww, 6) / pow(1. - 0.8 * 0.8, 2);
        }
        return dcs2 * 0.5;
      }

      if (TrepsB::pmodel == 253) {
        //ppbar
        double s00 = 0., s20 = 0., s22 = 0. ;

        if (ww < 2.85) {
          s00 = b00.get_val(ww);
          s20 = b20.get_val(ww);
          s22 = b22.get_val(ww);
        } else {
          double ww0 = 2.85;
          s00 = b00.get_val(ww0) * pow(ww0 / ww, 12);
          s20 = b20.get_val(ww0) * pow(ww0 / ww, 12);
          s22 = b22.get_val(ww0) * pow(ww0 / ww, 12);
        }
        double dcs2 = 0.;

        // eta_c resonance
        double mr = 2.984;
        double gtot = 0.032;
        double gggbr ; gggbr = 0.00152 * 0.000005;

        double sigr = 8.*3.14159 * mr / ww * gggbr * gtot / (pow(mr * mr - ww * ww, 2) + mr * mr * gtot * gtot)
                      * 389000.;

        if (ww > 2.0)
          dcs2 = s00 + sigr + s20 * (5. / 4.) * pow(3.*zz * zz - 1, 2) + s22 * (15. / 8.) * pow(1. - zz * zz, 2);

        return dcs2 * 0.5;
      }


      return dcs;

    }

    // user decision routine for extra generation conditions. */
    virtual int tpuser(TLorentzVector _pe, TLorentzVector _pp,
                       Part_gen* part, int _npart)
    {
      // user decision routine for extra generation conditions.
      // Return positive integer for the generation, otherwise, this event will
      // be canceled.
      // CAUTION!: The 4-momenta of particles are in e+e- cms

      int iret = 1;

      // 3-body physics models

      if (_npart == 3 && TrepsB::pmodel >= 301  && TrepsB::pmodel <= 399) {

        int index1 = 0, index2 = 1, index3 = 2;
        double z, m12, zp, phip, zs, phis, phi0;

        tpkin3(part, index1, index2, index3, z, m12, zp,
               phip, zs, phis, phi0);

        double u = sqrt(1. - z * z);
        double up = sqrt(1. - zp * zp);
        double us = sqrt(1. - zs * zs);

        double wei = 0., weimax = 0.;



        if (TrepsB::pmodel == 301) {
          // 0- -> VP -> 3P
          wei = zs * zs;
          weimax = 1.;
        }
        if (TrepsB::pmodel == 302) {
          // 2+(0) -> VP -> 3P
          wei = z * z * u * u * up * up * pow(sin(phip), 2);
          weimax = 1.;
        }
        if (TrepsB::pmodel == 303) {
          // 2+(2) -> VP -> 3P
          wei = u * u * (u * u * zp * zp + z * z * up * up - 2.*u * z * up * zp * cos(phip));
          weimax = 2.5;
        }
        if (TrepsB::pmodel == 304) {
          // 0- -> TP -> 3P
          wei = pow(3.*zs * zs - 1., 2);
          weimax = 4.;
        }
        if (TrepsB::pmodel == 305) {
          // 2+(0) -> TP -> 3P
          wei = u * u * up * up * zp * zp * pow(sin(phip), 2);
          weimax = 1.;
        }
        if (TrepsB::pmodel == 306) {
          // 2+(2) -> TP -> 3P
          wei = up * up * (z * z * up * up + u * u * zp * zp - 2.*z * u * zp * up * cos(phip));
          weimax = 2.5;
        }
        if (TrepsB::pmodel == 307) {
          // 2- -> SP -> 3P
          wei = pow(3.*zp * zp - 1., 2);
          weimax = 2.;
        }
        if (TrepsB::pmodel == 307) {
          // 2- -> VP -> 3P
          wei = z * zp - 0.5 * u * up * cos(phip);
          weimax = 1.5;
        }
        if (TrepsB::pmodel == 308) {
          // 2- -> TP -> 3P
          wei = pow(3.*zs * zs - 1., 2);
          weimax = 4.;
        }

        if (TrepsB::pmodel == 331) {
          // 2+(0) -> VP -> PgP
          wei = u * u * (1. - us * us * pow(cos(phis), 2));
          weimax = 1.;
        }
        if (TrepsB::pmodel == 332) {
          // 2+(2) -> VP -> PgP
          wei = u * u * (1. + z * z * zs * zs - u * u * us * us * pow(cos(phis), 2));
          weimax = 2.;
        }
        if (TrepsB::pmodel == 333) {
          // 0+/- -> Vg -> PPg
          wei = us * us;
          weimax = 1.;
        }
        if (TrepsB::pmodel == 334) {
          // 0+ -> Vg (E1) --> llg
          wei = 1. + zs * zs;
          weimax = 2.;
        }
        if (TrepsB::pmodel == 335) {
          // 2+(0) -> Vg (E1) --> llg
          wei = (9.*z * z * z * z - 12.*z * z + 5.) / 24.*(1. + zs * zs)
                + 0.75 * z * z * (1. - z * z) * (1. - zs * zs) + 0.5 * z * u * zs * us * (-3.*z * z + 2.) * cos(phis)
                + 0.125 * (1. - z * z) * (3.*z * z - 1.) * (1. - zs * zs) * cos(2.*phis);
          weimax = 13. / 12. + 0.75 + 1.0 + 0.25;
        }
        if (TrepsB::pmodel == 336) {
          // 2+(2) -> Vg (E1) --> llg
          wei = 0.125 * pow(1. + z * z, 2) * (1. + zs * zs)
                + 0.25 * (1. - z * z * z * z) * (1. - zs * zs) - 0.5 * z * u * zs * us * (1. + z * z) * cos(phis)
                + 0.125 * (1. - z * z * z * z) * (1. - zs * zs) * cos(2.*phis);
          weimax = 13. / 8.;
        }
        if (weimax * gRandom->Uniform()  > wei) iret = 0;

        B2DEBUG(10,  " $$ 3B $$ " << wei << " " << iret);
      }


      if (_npart == 4 && TrepsB::pmodel >= 401  && TrepsB::pmodel <= 499) {

        int index1 = 0, index2 = 1, index3 = 2, index4 = 3;
        double z, m12, zp, phip, zs, phis, m34, zpp, phipp, zss, phiss, phi0;

        tpkin4(part,
               index1, index2, index3, index4, z, m12, zp,
               phip, zs, phis, m34, zpp, phipp, zss, phiss, phi0) ;

      }

      return iret ;
    }


  private:
    double d2func(double) const;
    double qfunc(double, double) const;

  };

}
