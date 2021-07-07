/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "generators/evtgen/EvtBSemiTauonicDecayRateCalculator.h"
#include "generators/evtgen/models/EvtBSemiTauonicHelicityAmplitudeCalculator.h"

#include <cstdlib>
#include <iostream>
#include <fstream>

#include <unistd.h>

using namespace Belle2;

void usage()
{
  std::cout << "Usage: BSTD_calc_R_2HDM_Type2 [-h] [-m m_H+] m_B m_D* m_D m_tau m_lep rho1^2 rhoA1^2 R1(1) R2(1) aS1 aR3 m_b m_c" <<
            std::endl;
  std::cout << "Calculate R(D), R(D*), polarizations of tau and D* " << std::endl;
  std::cout << "as a function of tan(beta)/m_H+ for 2HDM Type2 using the BSTD model." << std::endl;
  std::cout << "CS2 term is neglected unless m_H+ value for CS2 term is specified by -m option." << std::endl;
  std::cout << std::endl;
  std::cout << "The results are written to " << std::endl;
  std::cout << "dstar.r_pol.dat, d.r_pol.dat " << std::endl;
  std::cout << "in the ASCII text format." << std::endl;
  std::cout << std::endl;
  std::cout << "Options: " << std::endl;
  std::cout << " -h\tShow this usage." << std::endl;
  std::cout << " -m m_H+\tSet charged higgs mass with m_H+ for CS2 term." << std::endl;
}

/** Program to make R(D), R(D*), polarizations of tau and D* vs tan(beta)/m_H^+ data for 2HDM Type2 using BSTD model based on [M. Tanaka and R. Watanabe PRD87,034028(2013)]. */
int main(int argc, char* argv[])
{

  // charged higgs mass
  double m_H(-1.);
  while (1) {
    // get options
    int c = getopt(argc, argv, "rh:");
    if (c == -1)break; //end of parameter.
    switch (c) {
      case 'h':
        usage();
        exit(0);
        break;
      default:
      case 'm':
        m_H = atof(optarg);
        break;
      case '?':
        std::cerr << "Unknown option" << std::endl;
        usage();
        exit(1);
    }
  }

  const int narg = argc - optind;
  if (narg != 13) {
    usage();
    exit(1);
  }

  const double m_B(atof(argv[optind++]));
  const double m_Dst(atof(argv[optind++]));
  const double m_D(atof(argv[optind++]));
  const double m_tau(atof(argv[optind++]));
  const double m_lep(atof(argv[optind++]));

  const double rho12(atof(argv[optind++]));
  const double rhoA12(atof(argv[optind++]));
  const double ffR11(atof(argv[optind++]));
  const double ffR21(atof(argv[optind++]));
  const double aS1(atof(argv[optind++]));
  const double aR3(atof(argv[optind++]));
  const double m_b(atof(argv[optind++]));
  const double m_c(atof(argv[optind++]));

  // quark masses
  double min, max;
  int npoints;

  std::cout << "Input tan(beta)/m_H+ range: min max npoints>" << std::endl;
  std::cin >> min >> max >> npoints;

  EvtBSemiTauonicHelicityAmplitudeCalculator BSTD(rho12, rhoA12, ffR11, ffR21, aS1, aR3, m_b, m_c,
                                                  0, 0, 0, 0, 0,
                                                  m_B, m_D, m_Dst);
  EvtBSemiTauonicDecayRateCalculator CalcDecayRate;

  std::cout << "tau mass: " << m_tau << std::endl;
  std::cout << "light lepton mass: " << m_lep << std::endl;

  std::cout << "b quark mass: " << m_b << std::endl;
  std::cout << "c quark mass: " << m_c << std::endl;
  if (m_H > 0)std::cout << "CS2 term will be calculated with H+ mass: " << m_H << std::endl;
  else std::cout << "CS2 term will be neglected." << std::endl;
  std::cout << "tan(beta)/m_H+ min: " << min << std::endl;
  std::cout << "tan(beta)/m_H+ max: " << max << std::endl;
  std::cout << "#points : " << npoints << std::endl;

  std::cout << "SM R(D) R(D*) Ptau(D) Ptau(D*) P(D*)" << std::endl;
  std::cout << CalcDecayRate.RGammaD(BSTD, m_tau, m_lep) << " ";
  std::cout << CalcDecayRate.RGammaDstar(BSTD, m_tau, m_lep) << " ";
  std::cout << CalcDecayRate.PtauD(BSTD, m_tau) << " ";
  std::cout << CalcDecayRate.PtauDstar(BSTD, m_tau) << " ";
  std::cout << CalcDecayRate.PDstar(BSTD, m_tau);
  std::cout << std::endl;

  std::ofstream ofileds("dstar.r_pol.dat");
  ofileds << "# tanbdmh rds ptau pds" << std::endl;

  std::ofstream ofiled("d.r_pol.dat");
  ofiled << "# tanbdmh rd ptau" << std::endl;

  double CS1, CS2;
  const double dtbmh = (max - min) / (double)npoints;
  if (max == min) {
    double tanbdmh = min;
    CS1 = -m_b * m_tau * tanbdmh * tanbdmh;
    if (m_H > 0) CS2 = -m_c * m_tau / m_H / m_H;
    else CS2 = 0;
    BSTD.setCS1(CS1);
    BSTD.setCS2(CS2);

    double rd     =  CalcDecayRate.RGammaD(BSTD, m_tau, m_lep);
    double rds    = CalcDecayRate.RGammaDstar(BSTD, m_tau, m_lep);
    double ptaud  =  CalcDecayRate.PtauD(BSTD, m_tau);
    double ptauds = CalcDecayRate.PtauDstar(BSTD, m_tau);
    double pds    =    CalcDecayRate.PDstar(BSTD, m_tau);

    ofileds << tanbdmh << " " << rds << " " << ptauds << " " << pds << std::endl;
    ofiled << tanbdmh << " " << rd << " " << ptaud << " " << std::endl;

    std::cout << "tan(beta)/m_H+: " << tanbdmh
              << " CS1: " << CS1 << " CS2: " << CS2
              << " R(D): " << rd << " R(D*): " << rds << std::endl;
  } else {
    for (double tanbdmh = min + dtbmh / 2.;
         tanbdmh < max; tanbdmh += dtbmh) {
      CS1 = -m_b * m_tau * tanbdmh * tanbdmh;
      if (m_H > 0) CS2 = -m_c * m_tau / m_H / m_H;
      else CS2 = 0;
      BSTD.setCS1(CS1);
      BSTD.setCS2(CS2);

      double rd     =  CalcDecayRate.RGammaD(BSTD, m_tau, m_lep);
      double rds    = CalcDecayRate.RGammaDstar(BSTD, m_tau, m_lep);
      double ptaud  =  CalcDecayRate.PtauD(BSTD, m_tau);
      double ptauds = CalcDecayRate.PtauDstar(BSTD, m_tau);
      double pds    =    CalcDecayRate.PDstar(BSTD, m_tau);

      ofileds << tanbdmh << " " << rds << " " << ptauds << " " << pds << std::endl;
      ofiled << tanbdmh << " " << rd << " " << ptaud << " " << std::endl;

      std::cout << "tan(beta)/m_H+: " << tanbdmh
                << " CS1: " << CS1 << " CS2: " << CS2
                << " R(D): " << rd << " R(D*): " << rds << std::endl;
    }
  }
}
