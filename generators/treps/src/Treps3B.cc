// -*- C++ -*-
//
// Package:     <package>
// Module:      Treps3B
//
// Description: <two-line class summary>
//  Two-photon Monte Carlo Generator TREPS Ver.3 (translated and modified
//          from TREPS2 of FORTRAN version
//              For BASF Application
// Implimentation:
//     <Notes on implimentation>
//
// Author:      Sadaharu Uehara
// Created:      Jul.16 1997
// $Id$
//
// Revision history
//
//
// Modified: To use RandFlat instead of bare HepRandom
//         9-MAY-2000 S.Uehara
//        RETURNED TO THE OLD RANDOM-NUMBER GENERATOR 14-MAY-2000
// $Log$

// system include files
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string.h>
#include <math.h>
#include <TVector3.h>
#include <TLorentzVector.h>
#include <TRandom.h>
#include <stdlib.h>

// user include files
#include <generators/treps/Treps3B.h>

#include <framework/logging/Logger.h>

namespace Belle2 {


  //
  // constructor
  //

  TrepsB::TrepsB(void) :
    sutool(), w(0.), ntot(0), nsave(0), ebeam(0.), q2max(0.), cost_cut(0.), pt_cut(0.), cost_flag(0), pt_flag(0),
    qzmin(0.), qptmin(0.), parti(nullptr), parts(nullptr), ephi(0.), etheta(0.), s(0.), imode(0), dmin(0.), dmax(0.),
    ffmax(0.), rs(0.), vmax(0.), pppp(nullptr), wf(0.), inmode(0), wtcount(0), totalCrossSectionForMC(0.), ndecay(0),
    pmodel(0), fmodel(0), ievent(0), treh1(nullptr), treh2(nullptr), treh3(nullptr), treh4(nullptr), treh5(nullptr),
    treh6(nullptr), npart(0), partgen(nullptr), zdis(nullptr), zpdis(nullptr), zsdis(nullptr)
  {
    for (int i = 0; i < 5000; ++i) {
      wtcond[i] = 0.;
      wthead[i] = 0;
    }
  }

  //
  // member functions
  //
  void TrepsB::initg(void)
  {
    B2DEBUG(20, " UtrepsB initg  Pmodel " << TrepsB::pmodel);
    //parametrization of pi+pi- partial waves
    if (TrepsB::pmodel == 251) {
      // pi+pi-
      static double data001[21] = { 20., 15., 9., 8., 8., 7., 5.3, 5.2,
                                    5.2, 5.2, 5.2, 4., 3., 2., 2.,
                                    2., 2., 2., 1., 0., 0.
                                  };
      static double data201[21] = { 1., 5., 5, 5., 4., 4., 4., 4.,
                                    3., 3., 3., 2., 1., 1., 1.,
                                    1., 0., 0., 0., 1., 1.
                                  };
      static double data221[21] = { 1., 5., 5., 6., 7., 7.18, 6.68, 6.27,
                                    5.91, 5.61, 5., 5., 4., 3., 2.,
                                    2., 2., 1., 0., 0., 0.
                                  };
      b00 = Interps(0.3, 2.3, 20, data001);
      b20 = Interps(0.3, 2.3, 20, data201);
      b22 = Interps(0.3, 2.3, 20, data221);
    }
    if (TrepsB::pmodel == 252) {
      // K+K-
      static double data002[15] = { 2., 2., 2., 2., 2., 2., 3., 4.,
                                    4.5, 5., 5., 5., 4.5, 4.5, 4.3
                                  };
      static double data202[15] = { 1., 2., 3., 4., 5., 5., 5., 5.,
                                    5., 5., 5., 4., 3.0, 2.2, 1.
                                  };
      static double data222[15] = { 0., 0., 0., 0., 0., 0., -1., -3.,
                                    -3., -3., -3., -4., -3.5, -3., -2.5
                                  };
      b00 = Interps(1.0, 2.4, 14, data002);
      b20 = Interps(1.0, 2.4, 14, data202);
      b22 = Interps(1.0, 2.4, 14, data222);
    }
    if (TrepsB::pmodel == 253) {
      // ppbar
      static double data003[9] = { 2.3, 2.3, 7.26, 7.94, 8.53, 7.38, 3.25, 1.98, 2.30};
      static double data203[9] = { 9.63, 9.63, 10.73, 8.02, 6.18, 3.37, 0.63, 0.10, 0.66};
      static double data223[9] = { 1.48, 1.48, -4.62, -6.12, -6.78, -5.35, -1.82, -1.02, -1.63};
      b00 = Interps(2.05, 2.85, 8, data003);
      b20 = Interps(2.05, 2.85, 8, data203);
      b22 = Interps(2.05, 2.85, 8, data223);
    }
    return;
  }

  void TrepsB::initp(void)
  // read parameter-input file and load the parameters
  {

    B2INFO("Treps: Parameters are read from the file, " << parameterFile);
    B2INFO("Treps: W list is read from the file, " << wlistFile);
    B2INFO("Treps: Differential cross section list is read from the file, " << diffcrosssectionFile);

    partgen = new Part_gen [30];
    parti = new Part_cont [10];
    parts = new Part_cont [20];
    pppp = new TLorentzVector [30];

    std::ifstream infile(parameterFile);
    if (!infile) {
      B2FATAL("Can't open input file: " << parameterFile);
    } else {
      // read particle properties
      infile >> ndecay >> pmodel >> fmodel ;   //Physics model pmodel, Form factor model  added

      if (ndecay < 2) {
        B2FATAL("decay particles must be >=2 : " << ndecay) ;
        exit(1);
      }

      int ndecs = 0  ;

      int icodex, ndecx ;
      double pmassx, pchargx, pwidthx ;

      for (int i = 0 ; i < ndecay ; i++) {
        sutool.nextline(infile);

        infile >> icodex >> pmassx >> pchargx >> ndecx >> pwidthx ;

        parti[i] =  Part_cont(icodex, pmassx, pchargx, ndecx, pwidthx);

        if (ndecx >= 2) ndecs += ndecx ;
      }

      for (int j = 0 ; j < ndecs ; j++) {
        sutool.nextline(infile);

        infile >> icodex >> pmassx >> pchargx ;
        Part_cont _pp  = Part_cont(icodex, pmassx, pchargx);
        parts[j] = _pp ;
      }

      // Boost of the total system
      double tsws4 = pfeb.Mag() + pfpb.Mag() ;
      TVector3 tsws = TVector3(-(pfeb + pfpb));
      double emsys = tsws4 * tsws4 - tsws.Mag2() ;

      peb = TLorentzVector(pfeb, pfeb.Mag());

      ppb = TLorentzVector(pfpb, pfpb.Mag());

      TLorentzVector ppp = peb;
      ppp.Boost(tsws.x() / tsws4, tsws.y() / tsws4, tsws.z() / tsws4);
      ephi = ppp.Phi();
      etheta = ppp.Theta();

      tsws = -tsws ;
      tswsb = (1. / tsws4) * tsws;

      // output input parameters
      //*********************************
      B2INFO("") ;
      B2INFO("****** Treps3 Input Parameters in Initialization ******");
      B2INFO("  Beam energy in e+e- cm system (GeV): " << ebeam);
      B2INFO("  e- lab. momentum: " << pfeb.x() << " " << pfeb.y() << " " << pfeb.z());
      B2INFO("  e+ lab. momentum: " << pfpb.x() << " " << pfpb.y() << " " << pfpb.z());
      B2INFO("  Q2 max (GeV2): " << q2max);
      B2INFO("  Save-condition (|cos(theta_cm)|): " << cost_cut << ", "
             << cost_flag);
      B2INFO("                      (pt (GeV/c)): " << pt_cut << ", "
             << pt_flag);
      B2INFO("  Number of decay particles: " << ndecay);
      B2INFO("  Physics model: " << pmodel);
      B2INFO("  Form-factor model: " << fmodel);


      B2INFO("  P/S  code      mass  charge decs      width");

      for (int i = 0 ; i < ndecay ; i++) {
        B2INFO("   P " << std::setw(6) << parti[i].icode
               << std::setw(11) << std::setprecision(4) << parti[i].pmass
               << std::setw(6) << std::setprecision(2) << parti[i].pcharg
               << std::setw(4) << parti[i].ndec
               << std::setw(11) << std::setprecision(4) << parti[i].pwidth
              );
      }
      for (int i = 0 ; i < ndecs ; i++) {
        B2INFO("   S " << std::setw(6) << parts[i].icode
               << std::setw(11) << std::setprecision(4) << parts[i].pmass
               << std::setw(6) << std::setprecision(2) << parts[i].pcharg);
      }
      B2INFO("*******************************************************");
      //*********************************

      if (abs(emsys - 4 * ebeam * ebeam) > 0.001) {
        B2FATAL(" Wrong beam fractional momenta ");
        exit(1) ;
      }

      // other impotant variables
      s = 4.*ebeam * ebeam ;
      ntot = 0 ;
      nsave = 0;
    }

  }

  void TrepsB::create_hist(void)
  {
    treh1 = new TH1F("PZSUMCM", "PZSUMCM", 100, -10., 10.);
    treh2 = new TH1F("PTSUMCM", "PTSUMCM", 100, 0., 1.);
    treh3 = new TH1F("EALLCM", "EALLCM", 100, 0., 20.);
    treh4 = new TH1F("PZSUMLAB", "PZSUMLAB", 100, -10., 10.);
    treh5 = new TH1F("PTSUMLAB", "PTSUMLAB", 100, 0., 1.);
    treh6 = new TH1F("WFINAL", "WFINAL", 100, 0., 5.);

    zdis = new TH1F("kin3_z", "kin3_z", 40, -1., 1.);
    zpdis = new TH1F("kin3_zp", "kin3_zp", 40, -1., 1.);
    zsdis = new TH1F("kin3_zs", "kin3_zs", 40, -1., 1.);

  }

  void TrepsB::wtable()
  {
    totalCrossSectionForMC = 0.;

    // Load Differential Cross Section table
    std::ifstream infile(diffcrosssectionFile);
    if (!infile) {
      B2FATAL("Can't open W-list input file") ;
    } else {
      double i_w; // W [GeV]
      double diffCrossSection; // Number of events for given W

      double previousW = 0.;
      double previousDCS = 0.; // Diff(erential) Cross Section
      while (infile >> i_w >> diffCrossSection) {
        if (i_w > 9000. || i_w < 0.) continue;
        diffCrossSectionOfW[i_w] = diffCrossSection;

        // Calculate total cross section up to the bin.
        // This will be used for importance sampling. NOT CORRECT cross section
        if (diffCrossSection > previousDCS and previousDCS != 0) {
          // If current diffCrossSection is higher than previous and not first time, use diffCrossSection
          totalCrossSectionForMC += (i_w - previousW) * diffCrossSection * 1.01; // For safety, 1 % higher value is set
        } else {
          // If previous diffCrossSection is higher than current or first time, use previousDCS
          totalCrossSectionForMC += (i_w - previousW) * previousDCS * 1.01;// For safety, 1 % higher value is set
        }
        // Store current cross section with w
        WOfCrossSectionForMC[totalCrossSectionForMC] = i_w;

        previousW = i_w;
        previousDCS = diffCrossSection;
      }

      B2DEBUG(20, " wtable loaded");
    }
  }

  double TrepsB::wtable(int mode)
  {
    double prew = 0.;
    if (mode == 0) {

      // initialization and load table

      for (int i = 0 ; i < 5000 ; i++) {
        wthead[i] = 999999999;
        wtcond[i] = 0.0;
      }

      B2DEBUG(20, " wtable mode=0  initialized");

      // Load Wlist_table

      std::ifstream infile(wlistFile);
      if (!infile) {
        B2FATAL("Can't open W-list input file") ;
      } else {
        double w1;
        int n1;
        int hpoint, nwpoint;
        int inmodein;

        hpoint = 1;
        nwpoint = 0;

        infile >> inmodein;
        inmode = inmodein;

        while (!infile.eof() && inmode == 0) {
          sutool.nextline(infile);
          infile >> w1 >> n1 ;
          if (w1 > 9000. || w1 < 0.) continue;

          if (nwpoint == 0) wthead[0] = 1;
          wtcond[nwpoint] =  w1;
          nwpoint++ ;
          hpoint += n1;
          B2DEBUG(20,  w1 << " GeV  " << n1 << " events   " << hpoint - 1 << "events in total");
          wthead[nwpoint] = hpoint;
        }
        while (!infile.eof() && (inmode == 1 || inmode == 2)) {
          sutool.nextline(infile);
          infile >> w1 >> n1 ;
          if (w1 > 9000. || w1 < 0.) continue;
          wf = w1;
          w = (double)wf;
          updateW();
          if (inmode == 1)
            B2INFO(w << " " << twlumi() << "   //W(GeV) and Two-photon lumi. func.(wide)(1/GeV)");
          else
            B2INFO(w << " " << twlumi_n() << "   //W(GeV) and Two-photon lumi. func.(narrow)(nb/keV/(2J+1))");

        }

        B2DEBUG(20, wthead[0] << " " << wtcond[0]);
        B2DEBUG(20, " wtable mode=0  loaded");
      }
      return 0.0;
    } else if (mode == 1) {
      // Get W
      int i = 0;

      do {
        if (wtcount >= wthead[i])  prew = wtcond[i];
        i++;
      } while (wthead[i] < 999999999);


      return prew;
    } else {
      B2FATAL("Undefined mode for wtable. Can be 1 or 0. Called with  " << mode) ;
    }
    return 0.0;
  }



  void TrepsB::setW(double ww)
  {
    // set W parameter
    w = ww;
    updateW();
  }


  void TrepsB::updateW(void)
  {
    if (q2max < 0.0) q2max = s - w * w - 2.0 * me * w ;
    rs = w * w / s ;
    double zmax = 1.0 - me / ebeam ;
    double ymin = rs / zmax ;
    dmin = ymin - zmax ;
    dmax = -dmin ;
    // added by S.U on Sep.29,1997
    imode = 0;

    // Unused variable, gives a compilation warning
    //double xxx = tpgetd(0, rs, dmin, dmax, q2max);
    tpgetz(0);

    B2DEBUG(20, "In Treps, W has been set to be " << std::setprecision(5) <<
            w << " GeV");
  }

  double TrepsB::twlumi()
  {
    // calculate and return two-photon luminosity function
    imode = 1;
    double xxx = simpsn1(dmin, dmax, 10000);
    return xxx * w * 0.5 / ebeam / ebeam ;
  }

  double TrepsB::twlumi_n()
  {
    // calculate and return two-photon luminosity function
    //        for very narrow resonance
    return twlumi() * twopi * twopi / w / w * 0.38938 ;
  }

  double TrepsB::tpgetd(int mode, double _rs, double _dmin, double _dmax,
                        double _q2max)
  {

    if (mode == 0) {
      // calc mode
      ffmax = 0. ;
      int n = 10000 ;
      for (int i = 1; i <= n ; i++) {
        double d = _dmin + (_dmax - _dmin) / (double)n * (double)i;
        double r = (2.0 + d * d / _rs + sqrt(pow((2.0 + d * d / _rs), 2) - 4.0)) * 0.5 ;
        if (d < 0.0)
          r = (2.0 + d * d / _rs - sqrt(pow((2.0 + d * d / _rs), 2) - 4.0)) * 0.5 ;
        double ff = tpxint(r, _rs, _q2max) / (0.5 * sqrt(_rs / r) * (1.0 + 1.0 / r));
        if (ff > ffmax) ffmax = ff ;
      }
      return 0.0;
    } else {
      // Generate mode
      double d, ff;
      do {
        d = _dmin + (_dmax - _dmin) * gRandom->Uniform();
        double r;
        if (d > 0.0) {
          r = (2. + d * d / _rs + sqrt((2. + d * d / _rs) * (2. + d * d / _rs) - 4.)) * 0.5 ;
        } else {
          r = (2. + d * d / _rs - sqrt((2. + d * d / _rs) * (2. + d * d / _rs) - 4.)) * 0.5 ;
        }
        ff = tpxint(r, _rs, _q2max) / (0.5 * sqrt(_rs / r) * (1. + 1. / r));
      } while (gRandom->Uniform()*ffmax > ff);
      return d;
    }
  }

  double TrepsB::tpgetz(int mode)
  {
    double z;

    if (mode == 0) {
      vmax = 0.0 ;
      // max val. search
      for (int i = -1000 ; i <= 1000 ; i++) {
        z = (double)i * 0.001 ;
        double v = tpangd(z, w);
        if (v > vmax) vmax = v;
      }
    } else {
      do { z = (gRandom->Uniform() - 0.5) * 2. ; }
      while (vmax * gRandom->Uniform() >= tpangd(z, w));
    }

    return z;
  }

  int TrepsB::event_gen(int iev)
  {
    // generates one event
    B2DEBUG(20, "W = " << w);
    imode = 0 ;
    int comp = 0 ;
    int npoint;

    do {
      double d = tpgetd(1, rs, dmin, dmax, q2max);
      double r ;
      if (d > 0.0) {
        r = (2.0 + d * d / rs + sqrt(pow((2.0 + d * d / rs), 2) - 4.0)) * 0.5;
      } else {
        r = (2.0 + d * d / rs - sqrt(pow((2.0 + d * d / rs), 2) - 4.0)) * 0.5;
      }

      double z = sqrt(rs / r);
      double y = sqrt(rs * r);

      double q2p, q2m, q2min, sf ;

      do {
        do {
          q2p = tpgetq(z, q2max) ;
          q2m = tpgetq(z, q2max) ;
          q2min = me * me * pow(w, 4) / s / (s - w * w);
          sf = ((s * s + (s - w * w) * (s - w * w)) / 2. / s / s - (s - w * w) * q2min / s / q2p)
               * ((s * s + (s - w * w) * (s - w * w)) / 2. / s / s - (s - w * w) * q2min / s / q2m);
        } while (gRandom->Uniform() > sf);
      } while (gRandom->Uniform() > tpform(q2p, w)*tpform(q2m, w));

      /* cppcheck-suppress variableScope */
      double q2;
      int ivirt = 0;

      TVector3 tv1, tv2, tv3, tv4;

      if (q2m < q2zero || q2p < q2zero) {
        // single tag approx.
        if (q2m > q2p) {
          q2 = q2m + q2p;
          ivirt = -1 ;
        } else {
          q2 = q2p + q2m ;
          ivirt = 1 ;
          z = y ;
        }

        if (q2m < q2zero && q2p < q2zero) {
          // real-real approx.
          q2 = 0.0 ;
          ivirt = 0 ;
        }

        double rk2 = z * ebeam ;
        double rk1 = 0.25 / rk2 * (w * w + q2 - rk2 * q2 / ebeam);
        double rk13 = q2 / 2.0 / ebeam + rk1 ;
        double qqqq = rk1 * rk1 - rk13 * rk13 + q2;
        if (ivirt == 0) qqqq = 0.0 ;

        if (qqqq < 0.0) continue ;

        double rk11 = -sqrt(qqqq);
        tv1 = TVector3(-rk11, 0., sqrt(ebeam * ebeam - me * me) - rk13);
        tv2 = TVector3(0., 0., -(sqrt(ebeam * ebeam - me * me) - rk2));
        tv3 = TVector3(-rk11, 0.,
                       -(sqrt(ebeam * ebeam - me * me) - rk13));
        tv4 = TVector3(0., 0., sqrt(ebeam * ebeam - me * me) - rk2);

      } else {
        // double-tag case
        ivirt = -1 ;
        if (q2p > q2m) {
          ivirt = 1;
          double qtmp = q2p ;
          q2p = q2m;
          q2m = qtmp ;
          z = y ;
        }
        double rk2 = z * ebeam ;
        double aaaa = 4.*rk2 + q2p / ebeam ;
        double bbbb = w * w + q2m + q2p - q2m * q2p / 2. / ebeam / ebeam - q2m * rk2 / ebeam ;
        double cccc = 4.*q2m * q2p * (1. - rk2 / ebeam - q2p / 4. / ebeam / ebeam);
        double rphi = gRandom->Uniform() * twopi;
        double rsrs = 1. + aaaa / cccc / pow(cos(rphi), 2) *
                      (4.*aaaa * ebeam * ebeam - aaaa * q2m - 4.*bbbb * ebeam);
        if (rsrs < 0.0) continue ;
        double rk1 = (2.*aaaa * bbbb + cccc * pow(cos(rphi), 2) / ebeam *
                      (sqrt(rsrs) - 1.)) / 2. / aaaa / aaaa;
        if ((bbbb - aaaa * rk1)*cos(rphi) > 0.0)
          rk1 = (2.*aaaa * bbbb + cccc * pow(cos(rphi), 2) / ebeam *
                 (-sqrt(rsrs) - 1.)) / 2. / aaaa / aaaa;
        if ((bbbb - aaaa * rk1)*cos(rphi) > 0.0) continue ;

        tv1 = TVector3(sqrt(q2m * (1. - rk1 / ebeam - q2m / 4. / ebeam / ebeam)),
                       0.,
                       sqrt(ebeam * ebeam - me * me) - (q2m / 2. / ebeam + rk1));
        tv2 = TVector3(sqrt(q2p * (1. - rk2 / ebeam - q2p / 4. / ebeam / ebeam)) * cos(rphi),
                       sqrt(q2p * (1. - rk2 / ebeam - q2p / 4. / ebeam / ebeam)) * sin(rphi),
                       - (sqrt(ebeam * ebeam - me * me) - (q2p / 2. / ebeam + rk2)));
        tv3 = TVector3(sqrt(q2m * (1. - rk1 / ebeam - q2m / 4. / ebeam / ebeam)),
                       0.,
                       -(sqrt(ebeam * ebeam - me * me) - (q2m / 2. / ebeam + rk1)));
        tv4 = TVector3(sqrt(q2p * (1. - rk2 / ebeam - q2p / 4. / ebeam / ebeam)) * cos(rphi),
                       sqrt(q2p * (1. - rk2 / ebeam - q2p / 4. / ebeam / ebeam)) * sin(rphi),
                       sqrt(ebeam * ebeam - me * me) - (q2p / 2. / ebeam + rk2));

      }

      if (gRandom->Uniform() > 0.5) {
        pe = TLorentzVector(tv1, sqrt(tv1.Mag2() + me * me));
        pp = TLorentzVector(tv2, sqrt(tv2.Mag2() + me * me));
      } else {
        pp = TLorentzVector(tv3, sqrt(tv3.Mag2() + me * me));
        pe = TLorentzVector(tv4, sqrt(tv4.Mag2() + me * me));
      }
      // two-photon system generated. Rotates it for general directions
      double dphi = gRandom->Uniform() * twopi ;
      pe.RotateZ(dphi);
      pp.RotateZ(dphi);

      TVector3 ws3 =  -(pe + pp).Vect();
      TLorentzVector ws = TLorentzVector(ws3, sqrt(ws3.Mag2() + w * w));

      TVector3 wsb = (1. / ws.T()) * ws3 ;

      // decide of 4-momenta of particles in the two-photon system

      // particle masses
      tpgetm(parti, ndecay);

      // angular distribution
      if (ndecay == 2) {
        // two-body
        double zz = tpgetz(1);
        double phi = twopi * gRandom->Uniform();
        int jcode;
        double m1 =  parti[0].pmassp;
        double m2 =  parti[1].pmassp;
        double pm = sutool.p2bdy(w, m1, m2, jcode);
        if (jcode == 0) continue ;
        pppp[0] = TLorentzVector(pm * sqrt(1. - zz * zz) * cos(phi),
                                 pm * sqrt(1. - zz * zz) * sin(phi),
                                 pm * zz, sqrt(pm * pm + m1 * m1));

        pppp[1] = TLorentzVector(-(pppp[0].Vect()),
                                 sqrt(pm * pm + m2 * m2));

        pppp[0].Boost(wsb);
        pppp[1].Boost(wsb);

      } else {
        // more than two body
        double* massa = new double [ndecay];
        for (int i = 0; i < ndecay ; i++) massa[i] = parti[i].pmassp ;

        int jcode = sutool.pdecy(w, massa , wsb, pppp, ndecay);
        if (jcode == 0) continue;

        delete [] massa ;
      }
      // more decay
      npoint = 0;
      int nlist = 0;
      int jcode = 0;

      for (int i = 1 ; i <= ndecay ; i++) {
        if (parti[i - 1].ndec <= 1) {
          npoint++ ;
          partgen[npoint - 1].part_prop = parti[i - 1];
          partgen[npoint - 1].p = pppp[i - 1];
          jcode = 1;
        } else {
          nlist++ ;
          npoint++ ;

          double* massa = new double [ parti[i - 1].ndec ];
          for (int j = 0; j < parti[i - 1].ndec ; j++)
            massa[j] = parts[nlist - 1 + j].pmass ;
          TVector3 pppb = (1. / pppp[i - 1].T()) * pppp[i - 1].Vect();
          TLorentzVector* ppptmp = new TLorentzVector [ parti[i - 1].ndec ];

          jcode = sutool.pdecy(parti[i - 1].pmassp, massa, pppb, ppptmp,
                               parti[i - 1].ndec);
          for (int j = 0; j < parti[i - 1].ndec ; j++)
            partgen[npoint - 1 + j].p = ppptmp[j] ;

          delete [] ppptmp ;
          delete [] massa ;

          if (jcode == 0) break ;

          for (int k = nlist ; k <= nlist + parti[i - 1].ndec - 1 ; k++) {
            partgen[npoint - 1].part_prop = parts[k - 1];
            npoint++ ;
          }
          nlist += parti[i - 1].ndec - 1 ;
          npoint--;

        }
      }
      if (jcode == 0) continue ;

      int iret = tpuser(partgen, npoint);
      if (iret <= 0) continue ;
      comp = 1;
    } while (comp != 1);
    //************* Generation end ********************
    // variable for the kinematical check
    TLorentzVector pfinal = TLorentzVector(0., 0., 0., 0.);
    for (int i = 0 ; i < npoint ; i++)pfinal += partgen[i].p ;
    TLorentzVector pcm = pe + pp + pfinal;
    double ptsumcm = sqrt(pfinal.X() * pfinal.X() + pfinal.Y() * pfinal.Y());
    double pzsumcm = pfinal.Z();
    double eall = pcm.T();
    double wcal = sqrt(pfinal.T() * pfinal.T() -
                       ptsumcm * ptsumcm - pzsumcm * pzsumcm);
    treh1->Fill((float)pzsumcm, 1.0);
    treh2->Fill((float)ptsumcm, 1.0);
    treh3->Fill((float)eall, 1.0);
    treh6->Fill((float)wcal, 1.0);

    // cos(theta)-pt cut
    int rcode = 1 ;
    for (int i = 0 ; i < npoint ; i++) {
      double zz = partgen[i].p.CosTheta();
      if (abs(zz) > cost_cut && abs(partgen[i].part_prop.pcharg) > qzmin) rcode = 0;
      double ptp = sqrt(pow(partgen[i].p.X(), 2) + pow(partgen[i].p.Y(), 2));
      if (ptp < pt_cut && abs(partgen[i].part_prop.pcharg) > qptmin) rcode = 0;
    }
    //************* final boost ***********
    sutool.rotate(pe, etheta, ephi);
    pe.Boost(tswsb);
    sutool.rotate(pp, etheta, ephi);
    pp.Boost(tswsb);
    for (int i = 0; i < npoint ; i++) {
      sutool.rotate(partgen[i].p, etheta, ephi);
      partgen[i].p.Boost(tswsb);
    }

    TLorentzVector plab(0., 0., 0., 0.);
    for (int i = 0; i < npoint ; i++) plab += partgen[i].p ;

    treh4->Fill((float)(plab.Z()), 1.0);
    treh5->Fill((float)(sqrt(plab.X()*plab.X() + plab.Y()*plab.Y())), 1.0);

    // std::cout << iev << std::endl;
    ievent = iev;
    npart = npoint;

    if (rcode == 0) return 0;

    //trkpsh(iev, pe, pp, partgen, npoint);
    nsave++ ;
    return 1;
  }

  double TrepsB::tpgetq(double z, double _q2max)
  {
    // get one Q2 value
    double q2min = me * me * z * z / (1. - z);
    double rk = 1. / log(_q2max / q2min);
    double ccc = rk * log(q2min);
    double rr = gRandom->Uniform();
    double xx = (rr + ccc) / rk ;

    return exp(xx);
  }

  void TrepsB::tpgetm(Part_cont* part, int _ndecay)
  {
    // get a mass value from Breit-Wigner distribution
    //  const double tmax = 1.3258 ; // 2Gamma
    const double tmax = 1.4289 ; // 3.5Gamma

    for (int i = 0; i < _ndecay ; i++) {
      if (part[i].pwidth < 0.001) {
        part[i].pmassp = part[i].pmass ;
      } else {
        part[i].pmassp = part[i].pmass + 0.5 * part[i].pwidth *
                         tan(2.*tmax * (gRandom->Uniform() - 0.5));
        if (part[i].pmassp < 0.0) part[i].pmassp = 0.0 ;
      }
    }
  }


  //
  // const member functions
  //

  void TrepsB::terminate() const
  {

    B2DEBUG(20, nsave << " events saved.");
    //  trfile->write();
    //std::cout << "Histograms are written in "<< filnam_hist << std::endl;
  }

  double TrepsB::tpdfnc(double d) const
  {
    // New (Working) version 2016 April S.Uehara
    const double alpppi = 0.002322816 ;
    double y0 = (d + sqrt(d * d + 4.*rs)) * 0.5;
    double z0 = y0 - d;
    double xminy = log(me * me * y0 * y0 / (1.0 - y0));
    double xminz = log(me * me * z0 * z0 / (1.0 - z0));
    double xmax = log(q2max) ;
    double integy = simpsny(d, xminy, xmax, 1000);
    double integz = simpsnz(d, xminz, xmax, 1000);
    double integrd = (integy + (-1. + y0) / y0) * (integz + (-1. + z0) / z0) / sqrt(d * d + 4.*rs)
                     * alpppi * alpppi;

    return integrd;
  }

  double TrepsB::simpsny(double d, double xl, double xh, int n) const
  {
    int nn = n / 2 * 2 ;
    double h = (xh - xl) / (double)nn ;
    double _s = tpdfy(d, xl) + tpdfy(d, xh) ;
    for (int i = 1 ; i <= nn - 1 ; i += 2) {
      double x = xl + (double)i * h ;
      _s += 4.0 * tpdfy(d, x) ;
    }
    for (int i = 2 ; i <= nn - 2 ; i += 2) {
      double x = xl + (double)i * h ;
      _s += 2.0 * tpdfy(d, x) ;
    }
    return h * _s / 3.0 ;
  }

  double TrepsB::tpdfy(double d, double x) const
  {
    double q2 = exp(x);
    double y = ((d - q2 / s) + sqrt(d * d - 2.*q2 / s * d + 4.*q2 / s + 4.*rs)) * 0.5;
    double integrq = (1. + (1. - y) * (1. - y)) / y * (1. - (-d + 2.) * q2 / (d * d + 4.*rs) / s) * 0.5;
    if (y - d >= 1.) integrq = 0.; // z=y-d
    return integrq * tplogf(x);
  }

  double TrepsB::simpsnz(double d, double xl, double xh, int n) const
  {
    int nn = n / 2 * 2 ;
    double h = (xh - xl) / (double)nn ;
    double _s = tpdfz(d, xl) + tpdfz(d, xh) ;
    for (int i = 1 ; i <= nn - 1 ; i += 2) {
      double x = xl + (double)i * h ;
      _s += 4.0 * tpdfz(d, x) ;
    }
    for (int i = 2 ; i <= nn - 2 ; i += 2) {
      double x = xl + (double)i * h ;
      _s += 2.0 * tpdfz(d, x) ;
    }
    return h * _s / 3.0 ;
  }

  double TrepsB::tpdfz(double d, double x) const
  {
    double q2 = exp(x);
    double z = ((-d - q2 / s) + sqrt(d * d - 2.*q2 / s * d + 4.*q2 / s + 4.*rs)) * 0.5;
    double integrq = (1. + (1. - z) * (1. - z)) / z * (1. - (-d + 2.) * q2 / (d * d + 4.*rs) / s) * 0.5;
    if (z + d >= 1.) integrq = 0.; //y=z+d
    return integrq * tplogf(x);
  }

  double TrepsB::tpxint(double r, double _rs, double _q2max) const
  {
    // Unused variable, gives a compilation warning
    //const double alpppi = 0.002322816 ;

    double y = sqrt(r * _rs);
    double z = sqrt(_rs / r);
    return tpf(y, _q2max) * tpf(z, _q2max) * 0.5 / r ;
  }

  double TrepsB::tpf(double z, double _q2max) const
  {
    const double alpppi = 0.002322816 ;
    double tpfx ;
    if (imode == 0) {
      tpfx = 1. / z * ((1. + (1. - z) * (1. - z)) * log(_q2max * (1. - z) / me / me / z / z) * 0.5
                       - 1.0 + z) * alpppi ;
    } else {
      double xmin = log(me * me * z * z / (1.0 - z));
      double xmax = log(_q2max) ;
      tpfx = 1. / z * ((1. + (1. - z) * (1. - z)) * simpsn2(xmin, xmax, 1000) * 0.5
                       - 1.0 + z) * alpppi ;
    }
    return tpfx ;
  }

  double TrepsB::simpsn1(double xl, double xh, int n) const
  {
    int nn = n / 2 * 2 ;
    double h = (xh - xl) / (double)nn ;
    double _s = tpdfnc(xl) + tpdfnc(xh) ;
    for (int i = 1 ; i <= nn - 1 ; i += 2) {
      double x = xl + (double)i * h ;
      _s += 4.0 * tpdfnc(x) ;
    }
    for (int i = 2 ; i <= nn - 2 ; i += 2) {
      double x = xl + (double)i * h ;
      _s += 2.0 * tpdfnc(x) ;
    }
    return h * _s / 3.0 ;
  }

  double TrepsB::simpsn2(double xl, double xh, int n) const
  {
    int nn = n / 2 * 2 ;
    double h = (xh - xl) / (double)nn ;
    double _s = tplogf(xl) + tplogf(xh) ;
    for (int i = 1 ; i <= nn - 1 ; i += 2) {
      double x = xl + (double)i * h ;
      _s += 4.0 * tplogf(x) ;
    }
    for (int i = 2 ; i <= nn - 2 ; i += 2) {
      double x = xl + (double)i * h ;
      _s += 2.0 * tplogf(x) ;
    }
    return h * _s / 3.0 ;
  }

  double TrepsB::tplogf(double x) const
  {
    double q2 = exp(x);
    return tpform(q2, w) ;
  }


  double TrepsB::tpform(double _q2, double _w) const
  {
    //form factor effect
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

  double TrepsB::tpangd(double _z, double _w)
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
      double gtot0 = 0.03;

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
        double mr0 = 0.98;
        double speak = 18.3;
        double mm = 0.1396;
        double gf = pow(qfunc(ww * ww, mm) / qfunc(mr * mr, mm), 5) *
                    d2func(qfunc(ww * ww, mm) * sr) / d2func(qfunc(mr * mr, mm) * sr);
        std::complex<double> imagu(0.0, 1.0);
        std::complex<double> rbr = sqrt(40.*3.14159 * mr / ww * ggg * gtot * br) * gf /
                                   ((-ww * ww + mr * mr) - imagu * mr * gtot * gf) *
                                   sqrt(389000.);
        rbr = (cos(phas) + imagu * sin(phas)) * rbr;

        dcs2 = pow(v00 + v20 * sqrt(5.) * 0.5 * (3.*zz * zz - 1.), 2) +
               (v22 * v22 + 2.*v22 * rbr.real() + pow(std::abs(rbr), 2)) *
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
      double gtotfa = 0.13;

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
        double mrfa = 1.29;
        double speak = 40.;
        double mm = 0.4937;
        double gf = pow(qfunc(ww * ww, mm) / qfunc(mr * mr, mm), 5) *
                    d2func(qfunc(ww * ww, mm) * sr) / d2func(qfunc(mr * mr, mm) * sr);
        std::complex<double> imagu(0.0, 1.0);
        std::complex<double> rbr = sqrt(40.*3.14159 * mr / ww * ggg * gtot * br) * gf /
                                   ((-ww * ww + mr * mr) - imagu * mr * gtot * gf) *
                                   sqrt(389000.);

        dcs2 = s00 + s20 * pow(sqrt(5.) * 0.5 * (3.*zz * zz - 1.), 2) +
               (s22 + pow(std::abs(rbr), 2)
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

  int TrepsB::tpuser(Part_gen* part, int _npart)
  {
    // user decision routine for extra generation conditions.
    // Return positive integer for the generation, otherwise, this event will
    // be canceled.
    // CAUTION!: The 4-momenta of particles are represented in the e+e- c.m. system
    //

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
      if (TrepsB::pmodel == 308) {
        // 2- -> VP -> 3P
        wei = z * zp - 0.5 * u * up * cos(phip);
        weimax = 1.5;
      }
      if (TrepsB::pmodel == 309) {
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

      B2DEBUG(20,  " $$ 3B $$ " << wei << " " << iret);
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

  //void TrepsB::trkpsh(int iev, TLorentzVector _pe, TLorentzVector _pp, Part_gen* part, int n) const {}

  double TrepsB::d2func(double y) const
  {
    return 1. / (9. + 3.*y * y + y * y * y * y);
  }

  double TrepsB::qfunc(double y, double mm) const
  {
    return sqrt(0.25 * y - mm * mm);
  }

  void TrepsB::print_event() const
  {

    const int& iev = ievent;
    const int& n = npart;
    const Part_gen* part = partgen;

    TLorentzVector pf(0., 0., 0., 0.);
    for (int i = 0 ; i < n; i++) pf += part[i].p ;
    TLorentzVector psum = pf + pe + pp ;

    double q2e = -((pe - peb).Mag2());
    double q2p = -((pp - ppb).Mag2());
    double www = pf.Mag();

    B2DEBUG(20, "");
    B2DEBUG(20, "**************** Event# = " << iev << " ******************");
    for (int i = 0; i < n; i++) {
      B2DEBUG(20, std::setw(2) << i + 1 << std::setw(11) << std::setprecision(4) << part[i].p.X() <<
              std::setw(11) << std::setprecision(4) << part[i].p.Y() <<
              std::setw(11) << std::setprecision(4) << part[i].p.Z() <<
              std::setw(11) << std::setprecision(4) << part[i].p.T() <<
              std::setw(7) << part[i].part_prop.icode <<
              std::setw(11) << std::setprecision(4) << part[i].part_prop.pmass <<
              std::setw(6) << std::setprecision(2) << part[i].part_prop.pcharg);
    }
    B2DEBUG(20, std::setw(2) << "e-" << std::setw(11) << std::setprecision(4) << pe.X() <<
            std::setw(11) << std::setprecision(4) << pe.Y() <<
            std::setw(11) << std::setprecision(4) << pe.Z() <<
            std::setw(11) << std::setprecision(4) << pe.T() <<
            "       " <<
            std::setw(11) << std::setprecision(4) << me <<
            std::setw(6) << std::setprecision(2) << -1.0);

    B2DEBUG(20, std::setw(2) << "e+" << std::setw(11) << std::setprecision(4) << pp.X() <<
            std::setw(11) << std::setprecision(4) << pp.Y() <<
            std::setw(11) << std::setprecision(4) << pp.Z() <<
            std::setw(11) << std::setprecision(4) << pp.T() <<
            "       " <<
            std::setw(11) << std::setprecision(4) << me <<
            std::setw(6) << std::setprecision(2) << 1.0);

    B2DEBUG(20, "-----------------------------------------------");
    B2DEBUG(20, std::setw(2) << "S:" << std::setw(11) << std::setprecision(4) << psum.X() <<
            std::setw(11) << std::setprecision(4) << psum.Y() <<
            std::setw(11) << std::setprecision(4) << psum.Z() <<
            std::setw(11) << std::setprecision(4) << psum.T());
    B2DEBUG(20, " Q2:(" << q2e << ", " << q2p << ")" <<
            "   W: " << www <<
            "   ptlab: " << sqrt(pf.X()*pf.X() + pf.Y()*pf.Y()) <<
            "   pzlab: " << pf.Z());
  }

  void TrepsB::tpkin3(Part_gen* part,
                      int index1, int index2, int index3, double& z, double& m12, double& zp,
                      double& phip, double& zs, double& phis, double& phi0)
  {

    TLorentzVector p1, p2, p3;   //a->(1,2), b=3
    p1 = part[index1].p;
    p2 = part[index2].p;
    p3 = part[index3].p;

    TVector3 tpcm = (-1. / (p1.T() + p2.T() + p3.T())) * (p1 + p2 + p3).Vect();

    p1.Boost(tpcm); p2.Boost(tpcm); p3.Boost(tpcm);

    // Now, we are at two-photon c.m.s.

    TLorentzVector pa = p1 + p2;

    m12 = pa.M();
    z = pa.CosTheta();
    phi0 = pa.Phi();

    sutool.rotate(p1, 0., phi0);
    sutool.rotate(p2, 0., phi0);
    sutool.rotate(p3, 0., phi0);

    // Now, we are in x-z plane of 12

    pa = p1 + p2;
    TVector3 pacm = (-1. / pa.T()) * pa.Vect();
    TVector3 az = (1. / (pa.Vect()).Mag()) * pa.Vect();
    p1.Boost(pacm); p2.Boost(pacm);

    // Now, we are at 12 c.m.s.

    zp = p1.CosTheta(); phip = p1.Phi();

    TVector3 ay = TVector3(0., 1., 0.);
    TVector3 ax = ay.Cross(az);

    zs = az.Dot(p1.Vect()) / (p1.Vect()).Mag();
    TVector3 azk = az.Cross(p1.Vect());
    double cosphis = ay.Dot(azk) * (1. / azk.Mag());
    double sinphis = ax.Dot(azk) * (-1. / azk.Mag());
    phis = atan2(sinphis, cosphis);

  }

  void TrepsB::tpkin4(Part_gen* part,
                      int index1, int index2, int index3, int index4,
                      double& z, double& m12, double& zp,
                      double& phip, double& zs, double& phis,
                      double& m34, double& zpp,
                      double& phipp, double& zss, double& phiss,
                      double& phi0)
  {

    TLorentzVector p1, p2, p3, p4;   //a->(1,2), b->(3,4)
    p1 = part[index1].p;
    p2 = part[index2].p;
    p3 = part[index3].p;
    p4 = part[index4].p;

    TVector3 tpcm = (-1. / (p1.T() + p2.T() + p3.T() + p4.T())) * (p1 + p2 + p3 + p4).Vect();

    p1.Boost(tpcm); p2.Boost(tpcm); p3.Boost(tpcm); p4.Boost(tpcm);

    // Now, we are at two-photon c.m.s.

    TLorentzVector pa = p1 + p2;

    m12 = pa.M();
    z = pa.CosTheta();
    phi0 = pa.Phi();

    sutool.rotate(p1, 0., phi0);
    sutool.rotate(p2, 0., phi0);
    sutool.rotate(p3, 0., phi0);
    sutool.rotate(p4, 0., phi0);

    // Now, we are in x-z plane of 12

    pa = p1 + p2;
    TVector3 pacm = (-1. / pa.T()) * pa.Vect();
    TVector3 az = (1. / (pa.Vect()).Mag()) * pa.Vect();
    p1.Boost(pacm); p2.Boost(pacm);

    // Now, we are at 12 c.m.s.

    zp = p1.CosTheta(); phip = p1.Phi();

    TVector3 ay = TVector3(0., 1., 0.);
    TVector3 ax = ay.Cross(az);

    zs = az.Dot(p1.Vect()) / (p1.Vect()).Mag();
    TVector3 azk = az.Cross(p1.Vect());
    double cosphis = ay.Dot(azk) * (1. / azk.Mag());
    double sinphis = ax.Dot(azk) * (-1. / azk.Mag());
    phis = atan2(sinphis, cosphis);

    // next go to 34

    TLorentzVector pb = p3 + p4;

    m34 = pb.M();

    TVector3 pbcm = (-1. / pb.T()) * pb.Vect();
    p3.Boost(pbcm); p4.Boost(pbcm);

    // Now, we are at 34 c.m.s.

    zpp = p3.CosTheta(); phipp = p3.Phi();

    // 2004.02.29   Here, I change the definision of zss and phiss
    //            Use the same system as 3

    zss = az.Dot(p3.Vect()) / (p3.Vect()).Mag();
    TVector3 bzk = az.Cross(p3.Vect());
    double cosphiss = ay.Dot(bzk) * (1. / bzk.Mag());
    double sinphiss = ax.Dot(bzk) * (-1. / bzk.Mag());
    phiss = atan2(sinphiss, cosphiss);

  }
  void TrepsB::tpkin5(Part_gen* part,
                      int index1, int index2, int index3, int index4, int index5,
                      double& z, double& m12, double& zp,
                      double& phip, double& zs, double& phis,
                      TVector3& ps3, TVector3& ps4, TVector3& ps5,
                      double& phi0)
  {

    TLorentzVector p1, p2, p3, p4, p5;   //a->(1,2), b->(3,4, 5)
    p1 = part[index1].p;
    p2 = part[index2].p;
    p3 = part[index3].p;
    p4 = part[index4].p;
    p5 = part[index5].p;

    TVector3 tpcm = (-1. / (p1.T() + p2.T() + p3.T() + p4.T() + p5.T())) * (p1 + p2 + p3 + p4 + p5).Vect();

    p1.Boost(tpcm); p2.Boost(tpcm); p3.Boost(tpcm); p4.Boost(tpcm); p5.Boost(tpcm);

    // Now, we are at two-photon c.m.s.

    TLorentzVector pa = p1 + p2;

    m12 = pa.M();
    z = pa.CosTheta();
    phi0 = pa.Phi();

    sutool.rotate(p1, 0., phi0);
    sutool.rotate(p2, 0., phi0);
    sutool.rotate(p3, 0., phi0);
    sutool.rotate(p4, 0., phi0);
    sutool.rotate(p5, 0., phi0);

    pa = p1 + p2;
    TVector3 pacm = (-1. / pa.T()) * pa.Vect();
    TVector3 az = (1. / pa.Vect().Mag()) * pa.Vect();
    p1.Boost(pacm); p2.Boost(pacm);

    // Now, we are at 12 c.m.s.

    zp = p1.CosTheta(); phip = p1.Phi();

    TVector3 ay = TVector3(0., 1., 0.);
    TVector3 ax = ay.Cross(az);


    zs = az.Dot(p1.Vect()) / p1.Vect().Mag();
    TVector3 azk = az.Cross(p1.Vect());
    double cosphis = ay.Dot(azk) * (1. / azk.Mag());
    double sinphis = ax.Dot(azk) * (-1. / azk.Mag());
    phis = atan2(sinphis, cosphis);

    // next go to 345

    TLorentzVector pb = p3 + p4 + p5;


    TVector3 pbcm = (-1. / pb.T()) * pb.Vect();
    p3.Boost(pbcm); p4.Boost(pbcm); p5.Boost(pbcm);
    ps3 = p3.Vect(); ps4 = p4.Vect(); ps5 = p5.Vect();

    // Now, we are at 345 c.m.s.
    // double mm = 0.1396*0.1396;
  }

} // namespace Belle2
