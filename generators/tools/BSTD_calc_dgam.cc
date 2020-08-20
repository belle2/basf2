/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Koji Hara                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
  std::cout << "Usage: BSTD_calc_dgam [-hr] m_B m_D* m_D m_tau m_lep rho1^2 rhoA1^2 R1(1) R2(1) aS1 aR3 m_b m_c" << std::endl;
  std::cout << "Calculate differential decay rate for each helicity state, " << std::endl;
  std::cout << "the ratios R(D) and R(D*), " << std::endl;
  std::cout << "and polarization P_tau and P_D* for B->D(*)taunu decays " << std::endl;
  std::cout << "using the BSTD model." << std::endl;
  std::cout << std::endl;
  std::cout << "The results are written to " << std::endl;
  std::cout << "dstar.{w,costau,2D}.dat, d.{w,costau,2D}.dat " << std::endl;
  std::cout << "and r_pol.dat in the ASCII text format." << std::endl;
  std::cout << std::endl;
  std::cout << "Options: " << std::endl;
  std::cout << " -h\tShow this usage." << std::endl;
  std::cout << " -r\tCalculate only R and polarizations, without writing output files." << std::endl;
}

/** Program to make dGamma/dw, dGamma/dcostau data and calculate R(D), R(D*) and polarizations of tau and D* for B->D(*)taunu decay using BSTD model based on [M. Tanaka and R. Watanabe PRD87,034028(2013)]. */
int main(int argc, char* argv[])
{

  bool calcROnly = false;

  while (1) {
    // get options
    int c = getopt(argc, argv, "rh");
    if (c == -1)break; //end of parameter.
    switch (c) {
      case 'h':
        usage();
        exit(0);
        break;
      case 'r':
        calcROnly = true;
        break;
      default:
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

  //  double m_tau(1.777),mB(5.2795),mDstar(2.01027),mD(1.86962); // B0
  //  double m_tau(1.777),mB(5.2791),mDstar(2.00697),mD(1.86484); // B+
  double AbsCV1, ArgCV1, AbsCV2, ArgCV2, AbsCS1, ArgCS1, AbsCS2, ArgCS2, AbsCT, ArgCT;

  std::cout << "Input Abs(CV1) Arg(CV1) Abs(CV2) Arg(CV2) Abs(CS1) Arg(CS1) Abs(CS2) Arg(CS2) Abs(CT) Arg(CT) >" << std::endl;
  std::cin >> AbsCV1 >> ArgCV1 >> AbsCV2 >> ArgCV2 >> AbsCS1 >> ArgCS1 >> AbsCS2 >> ArgCS2 >> AbsCT >> ArgCT;
  EvtComplex Coeffs[5];
  Coeffs[0] = EvtComplex(AbsCV1 * cos(ArgCV1), AbsCV1 * sin(ArgCV1));
  Coeffs[1] = EvtComplex(AbsCV2 * cos(ArgCV2), AbsCV2 * sin(ArgCV2));
  Coeffs[2] = EvtComplex(AbsCS1 * cos(ArgCS1), AbsCS1 * sin(ArgCS1));
  Coeffs[3] = EvtComplex(AbsCS2 * cos(ArgCS2), AbsCS2 * sin(ArgCS2));
  Coeffs[4] = EvtComplex(AbsCT * cos(ArgCT), AbsCT * sin(ArgCT));

  EvtBSemiTauonicHelicityAmplitudeCalculator BSTD(rho12, rhoA12, ffR11, ffR21, aS1, aR3, m_b, m_c,
                                                  Coeffs[0], Coeffs[1], Coeffs[2], Coeffs[3], Coeffs[4],
                                                  m_B, m_D, m_Dst);
  EvtBSemiTauonicDecayRateCalculator CalcDecayRate;

  std::cout << "tau mass: " << m_tau << std::endl;
  std::cout << "light lepton mass: " << m_lep << std::endl;

  if (!calcROnly) {
    // calculate and write differential decay rate to files
    const double totgamd = CalcDecayRate.GammaD(BSTD, m_tau);

    const double totgamds = CalcDecayRate.GammaDstar(BSTD, m_tau);

    int i_tauhel[2] = { -1, 1}; //  left, right
    int i_dhel[4] = { -1, 0, 1, 2};
    double dgam[2][4];
    double wmin = 1.0;  //BSTD.wmin();
    double wmax = 1.35; //BSTD.wmax(m_tau,1);
    double dw = (wmax - wmin) / 70.;

    // dGam/dw
    std::cout << "Calculating dGam/dw ..." << std::endl;
    std::ofstream ofile1("dstar.w.dat");
    ofile1 << "# w dgamlm dgamrm dgaml0 dgamr0 dgamlp dgamrp total" << std::endl;
    for (double w = wmin + dw / 2.;
         w < wmax; w += dw) {
      double tot(0);
      for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
          dgam[i][j] = CalcDecayRate.dGammadw(BSTD, m_tau, i_tauhel[i], i_dhel[j], w) / totgamds * dw;
          tot += dgam[i][j];
        }
      }
      ofile1 << w
             << " " << dgam[0][0] << " " << dgam[1][0]
             << " " << dgam[0][1] << " " << dgam[1][1]
             << " " << dgam[0][2] << " " << dgam[1][2]
             << " " << tot << std::endl;
    }

    std::ofstream ofile2("d.w.dat");
    ofile2 << "# w dgaml dgamr total" << std::endl;
    wmin = 1.0;  //BSTD.wmin();
    wmax = 1.42; //BSTD.wmax(m_tau,2);
    dw = (wmax - wmin) / 84;
    for (double w = wmin + dw / 2.;
         w < wmax; w += dw) {
      double tot(0);
      for (int i = 0; i < 2; i++) {
        int j = 3;
        dgam[i][j] = CalcDecayRate.dGammadw(BSTD, m_tau, i_tauhel[i], i_dhel[j], w) / totgamd * dw;
        tot += dgam[i][j];
      }
      ofile2 << w
             << " " << dgam[0][3] << " " << dgam[1][3]
             << " " << tot << std::endl;
    }

    // dGam/dcostau
    std::cout << "Calculating dGam/dcostau ..." << std::endl;
    double dcostau = (1 - (-1)) / 100.;
    std::ofstream ofilecostau1("dstar.costau.dat");
    ofilecostau1 << "# costau dgamlm dgamrm dgaml0 dgamr0 dgamlp dgamrp total" << std::endl;
    for (double costau = -1 + dcostau / 2.;
         costau < 1; costau += dcostau) {
      double tot(0);
      for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
          dgam[i][j] = CalcDecayRate.dGammadcostau(BSTD, m_tau, i_tauhel[i], i_dhel[j], costau) / totgamds * dcostau;
          tot += dgam[i][j];
        }
      }
      ofilecostau1 << costau
                   << " " << dgam[0][0] << " " << dgam[1][0]
                   << " " << dgam[0][1] << " " << dgam[1][1]
                   << " " << dgam[0][2] << " " << dgam[1][2]
                   << " " << tot << std::endl;
    }

    std::ofstream ofilecostau2("d.costau.dat");
    ofilecostau2 << "# costau dgaml dgamr total" << std::endl;
    for (double costau = -1 + dcostau / 2.;
         costau < 1; costau += dcostau) {
      double tot(0);
      for (int i = 0; i < 2; i++) {
        int j = 3;
        dgam[i][j] = CalcDecayRate.dGammadcostau(BSTD, m_tau, i_tauhel[i], i_dhel[j], costau) / totgamd * dcostau;
        tot += dgam[i][j];
      }
      ofilecostau2 << costau
                   << " " << dgam[0][3] << " " << dgam[1][3]
                   << " " << tot << std::endl;
    }

    // 2D
    std::cout << "Calculating dGam/dw/dcostau ..." << std::endl;
    wmin = 1.0;
    wmax = 1.35;
    dw = (wmax - wmin) / 20.;
    dcostau = (1 - (-1)) / 20.;
    std::ofstream ofile2d1("dstar.2d.dat");
    ofile2d1 << "# w costau dgamlm dgamrm dgaml0 dgamr0 dgamlp dgamrp total" << std::endl;
    for (double w = wmin + dw / 2.; w < wmax; w += dw) {
      for (double costau = -1 + dcostau / 2.; costau < 1; costau += dcostau) {
        double tot(0);
        for (int i = 0; i < 2; i++) {
          for (int j = 0; j < 3; j++) {
            dgam[i][j] = CalcDecayRate.dGammadwdcostau(BSTD, m_tau, i_tauhel[i], i_dhel[j], w, costau) / totgamds * dcostau * dw;
            tot += dgam[i][j];
          }
        }
        ofile2d1 << w << " " << costau
                 << " " << dgam[0][0] << " " << dgam[1][0]
                 << " " << dgam[0][1] << " " << dgam[1][1]
                 << " " << dgam[0][2] << " " << dgam[1][2]
                 << " " << tot << std::endl;
      }
    }

    wmin = 1.0;
    wmax = 1.42;
    dw = (wmax - wmin) / 20.;
    dcostau = (1 - (-1)) / 20.;
    std::ofstream ofile2d2("d.2d.dat");
    ofile2d2 << "# w costau dgaml dgamr total" << std::endl;
    for (double w = wmin + dw / 2.; w < wmax; w += dw) {
      for (double costau = -1 + dcostau / 2.; costau < 1; costau += dcostau) {
        double tot(0);
        for (int i = 0; i < 2; i++) {
          int j = 3;
          dgam[i][j] = CalcDecayRate.dGammadwdcostau(BSTD, m_tau, i_tauhel[i], i_dhel[j], w, costau) / totgamd * dcostau * dw;
          tot += dgam[i][j];
        }
        ofile2d2 << w << " " << costau
                 << " " << dgam[0][3] << " " << dgam[1][3]
                 << " " << tot << std::endl;
      }
    }
  }

  // R and polarization data
  /* cppcheck-suppress duplicateCondition */
  if (!calcROnly) {
    std::ofstream ofile3("r_pol.dat");
    ofile3 << "# rgamd rgamds ptaud ptauds pds" << std::endl;
    ofile3 << CalcDecayRate.RGammaD(BSTD, m_tau, m_lep) << " ";
    ofile3 << CalcDecayRate.RGammaDstar(BSTD, m_tau, m_lep) << " ";
    ofile3 << CalcDecayRate.PtauD(BSTD, m_tau) << " ";
    ofile3 << CalcDecayRate.PtauDstar(BSTD, m_tau) << " ";
    ofile3 << CalcDecayRate.PDstar(BSTD, m_tau);
    ofile3 << std::endl;
  }

  std::cout << "# rgamd rgamds ptaud ptauds pds" << std::endl;
  std::cout << CalcDecayRate.RGammaD(BSTD, m_tau, m_lep) << " ";
  std::cout << CalcDecayRate.RGammaDstar(BSTD, m_tau, m_lep) << " ";
  std::cout << CalcDecayRate.PtauD(BSTD, m_tau) << " ";
  std::cout << CalcDecayRate.PtauDstar(BSTD, m_tau) << " ";
  std::cout << CalcDecayRate.PDstar(BSTD, m_tau);
  std::cout << std::endl;

  return 0;
}

