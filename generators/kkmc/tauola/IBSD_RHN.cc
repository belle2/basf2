#include <math.h>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <cmath>
#include <complex>

extern "C" void pigamma_right_(const double& Mtau, const double& Mpi, const double& m_rho, const double& Gamma_rho, const double& m_a1, const double& Gamma_a1,
			       const double& CV_RL, const double& CV_RR, const double& CS_LL, const double& CS_LR, const double& MnuR,
			       const double *ptau, const double *pnu, const double *ppi, const double *k, double &omega, double *hj);

void pigamma_right_(const double& Mtau, const double& Mpi, const double& m_rho, const double& Gamma_rho, const double& m_a1, const double& Gamma_a1,
		    const double& CV_RL, const double& CV_RR, const double& CS_LL, const double& CS_LR, const double& MnuR,
		    const double* ptau, const double* pnu, const double* ppi, const double* k, double& omega, double* hj)
{
  // Physical constants
  // const double Mtau  = 1.777;
  // const double Mpi   = 0.139568;
  // const double m_rho = 0.7749; // PRD 78, 2008, 072006
  // const double Gamma_rho = 0.1486;
  // const double m_a1 = 1.251;
  // const double Gamma_a1 = 0.599;
  const double M_Borel = 3.35; // arXiv:2010.00549 [hep-ph]
  const double M_u = 2.16e-3;    // Mass of up quark in GeV
  const double M_d = 4.67e-3;    // Mass of down quark in GeV
  const double f_pi  = 0.092;

  // Wilson coefficients (real as requested)
  // const double CV_RL = 0.0;
  // const double CV_RR = 0.0;
  // const double CS_LL = 0.0;
  // const double CS_LR = 0.0;
  const std::complex<double> I(0.0, 1.0);

  // RH neutrino mass (set to zero by default; update if needed)
  //  const double MnuR = 0.0;
  const double MnuR_sq = MnuR * MnuR;

  // Lorentz products with (+,-,-,-)
  const double PpiPtau = ptau[3] * ppi[3] - ptau[2] * ppi[2] - ptau[1] * ppi[1] - ptau[0] * ppi[0];
  const double PnuPpi  = pnu[3] * ppi[3]  - pnu[2] * ppi[2]  - pnu[1] * ppi[1]  - pnu[0] * ppi[0];
  const double PnuPtau = ptau[3] * pnu[3] - ptau[2] * pnu[2] - ptau[1] * pnu[1] - ptau[0] * pnu[0];
  const double PkPtau  = k[3] * ptau[3]   - k[2] * ptau[2]   - k[1] * ptau[1]   - k[0] * ptau[0];
  const double PkPpi   = k[3] * ppi[3]    - k[2] * ppi[2]    - k[1] * ppi[1]    - k[0] * ppi[0];
  const double PkPnu   = k[3] * pnu[3]    - k[2] * pnu[2]    - k[1] * pnu[1]    - k[0] * pnu[0];

  const double t = (k[3] + ppi[3]) * (k[3] + ppi[3]) - (k[2] + ppi[2]) * (k[2] + ppi[2]) - (k[1] + ppi[1]) * (k[1] + ppi[1]) - (k[0] + ppi[0]) * (k[0] + ppi[0]);

  // Eq. (50), (51)
  const std::complex<double> FV_pi = (f_pi / (m_rho * m_rho - t - I * m_rho * Gamma_rho)) * std::exp((m_rho * m_rho) / (M_Borel * M_Borel));
  const std::complex<double> FA_pi = (f_pi / (m_a1 * m_a1 - t - I * m_a1 * Gamma_a1)) * std::exp((m_a1 * m_a1) / (M_Borel * M_Borel));

  const double Mtau_sq = Mtau * Mtau;
  const double Mpi_sq = Mpi * Mpi;
  const double fpi_sq = f_pi * f_pi;
  const double PkPtau_sq = PkPtau * PkPtau;
  const double PkPpi_sq = PkPpi * PkPpi;
  const double Mtau_3 = Mtau_sq * Mtau;

  // Real-coefficient combinations used in Eqs. (33)-(44)
  const double mass_coeff = Mpi_sq / (Mtau * (M_u + M_d));
  const double CIB_R = (CV_RL - CV_RR) + mass_coeff * (CS_LL - CS_LR);
  const double kappa_plus = Mtau_sq * (CIB_R * CIB_R) + MnuR_sq * ((CV_RR - CV_RL) * (CV_RR - CV_RL));
  const double kappa_minus = Mtau_sq * (CIB_R * CIB_R) - MnuR_sq * ((CV_RR - CV_RL) * (CV_RR - CV_RL));
  const double CV_sum = CV_RL + CV_RR;
  const double CV_diff = CV_RL - CV_RR;

  double p1[3], p2[3], p3[3];
  double h1[3], h2[3], h3[3], h4[3], h5[3], h6[3];

  // Eq. (33)
  const double omega_1 = fpi_sq * (
      ((2.0 * MnuR_sq * Mtau_sq * CV_diff * CIB_R) - kappa_plus * PnuPtau) * (Mpi_sq / PkPpi_sq)
      + (kappa_plus * (PkPtau + Mtau_sq) * (PkPnu - PnuPtau) + 2.0 * MnuR_sq * Mtau_sq * Mtau_sq * CV_diff * CIB_R) / PkPtau_sq
      + (kappa_plus * (2.0 * PnuPtau * PpiPtau + PkPtau * PnuPpi - PkPnu * PpiPtau) - 4.0 * MnuR_sq * Mtau_sq * CV_diff * CIB_R * PpiPtau) / (PkPpi * PkPtau)
  );

  // Eq. (34)
  for (int j = 0; j < 3; ++j) {
    p1[j] = pnu[j] * (Mpi_sq / PkPpi_sq + Mtau_sq / PkPtau_sq - 2.0 * PpiPtau / (PkPpi * PkPtau));
    p2[j] = k[j] * ((PnuPtau - PkPnu) / PkPtau_sq - PnuPpi / (PkPpi * PkPtau));
    p3[j] = ppi[j] * (PkPnu / (PkPpi * PkPtau));
    h1[j] = (fpi_sq * Mtau * kappa_minus / omega_1) * (p1[j] + p2[j] + p3[j]);
  }

  const double common_sd = (PkPnu * PkPpi * PpiPtau) - (Mpi_sq * PkPnu * PkPtau) + (PkPpi * PkPtau * PnuPpi);

  // Eq. (35)-(36)
  const double omega_2 = 2.0 * (CV_sum * CV_sum) * std::norm(FV_pi) * common_sd;
  for (int j = 0; j < 3; ++j) {
    p1[j] = k[j] * (Mpi_sq * PkPnu - PkPpi * PnuPpi);
    p2[j] = ppi[j] * (PkPnu * PkPpi);
    h2[j] = (-2.0 * (CV_sum * CV_sum) * std::norm(FV_pi) * Mtau / omega_2) * (p1[j] - p2[j]);
  }

  // Eq. (37)-(38)
  const double omega_3 = 2.0 * (CV_diff * CV_diff) * std::norm(FA_pi) * common_sd;
  for (int j = 0; j < 3; ++j) {
    p1[j] = k[j] * (Mpi_sq * PkPnu - PkPpi * PnuPpi);
    p2[j] = ppi[j] * (PkPnu * PkPpi);
    h3[j] = (-2.0 * (CV_diff * CV_diff) * std::norm(FA_pi) * Mtau / omega_3) * (p1[j] - p2[j]);
  }

  // Eq. (39)-(40)
  const double re_va = CV_sum * CV_diff * std::real(FV_pi * std::conj(FA_pi));
  const double omega_4 = -4.0 * re_va * ((PkPnu * PkPpi * PpiPtau) - (PkPpi * PkPtau * PnuPpi));
  for (int j = 0; j < 3; ++j) {
    p1[j] = k[j] * (PkPpi * PnuPpi);
    p2[j] = ppi[j] * (PkPnu * PkPpi);
    h4[j] = (4.0 * re_va * Mtau / omega_4) * (p1[j] - p2[j]);
  }

  // Eq. (41)-(42)
  const double re_iv = CV_sum * std::real(std::conj(FV_pi));
  const double omega_5 = (-2.0 * re_iv * f_pi / PkPtau) * ((Mtau_sq * CIB_R * PkPnu) - (MnuR_sq * CV_diff * PkPtau)) * PkPpi;
  for (int j = 0; j < 3; ++j) {
    p1[j] = k[j] * (CIB_R * ((PpiPtau * PkPnu) - (PpiPtau * PnuPtau) + (Mtau_sq * PnuPpi)) + MnuR_sq * CV_diff * PkPpi);
    p2[j] = ppi[j] * (CIB_R * ((PkPnu * PkPtau) + Mtau_sq * PkPnu - PkPtau * PnuPtau));
    h5[j] = (2.0 * re_iv * f_pi * Mtau / (PkPtau * omega_5)) * (p1[j] - p2[j]);
  }

  // Eq. (43)-(44)
  const double re_ia = CV_diff * std::real(std::conj(FA_pi));
  const double omega_6 = (2.0 * f_pi * re_ia / (PkPpi * PkPtau)) * (
      PkPpi_sq * (MnuR_sq * CV_diff * (PkPtau + Mtau_sq) + Mtau_sq * CIB_R * (PkPnu - PnuPtau))
      + Mpi_sq * PkPtau * (MnuR_sq * CV_diff * PkPtau - Mtau_sq * CIB_R * PkPnu)
      + PkPpi * PkPtau * (Mtau_sq * CIB_R * PnuPpi - 2.0 * MnuR_sq * CV_diff * PpiPtau)
      + Mtau_sq * CIB_R * PkPpi * PkPnu * PpiPtau
  );
  for (int j = 0; j < 3; ++j) {
    p1[j] = k[j] * (
        Mpi_sq * PkPtau * (MnuR_sq * CV_diff - CIB_R * PnuPtau)
        - PkPpi * PpiPtau * (CIB_R * (PkPnu - PnuPtau) + MnuR_sq * CV_diff)
        + MnuR_sq * CV_diff * PkPpi_sq
    );
    p2[j] = pnu[j] * CIB_R * (Mpi_sq * PkPtau_sq - 2.0 * PkPtau * PkPpi * PpiPtau + Mtau_sq * PkPpi_sq);
    p3[j] = ppi[j] * PkPtau * PkPpi * (CIB_R * (PkPnu + PnuPtau) - CV_diff * MnuR_sq);
    h6[j] = (2.0 * f_pi * re_ia * Mtau / (PkPpi * PkPtau * omega_6)) * (p1[j] + p2[j] + p3[j]);
  }

  omega = omega_1 + omega_2 + omega_3 + omega_4 + omega_5 + omega_6;
  for (int j = 0; j < 3; ++j) {
    hj[j] = (omega_1 * h1[j] + omega_2 * h2[j] + omega_3 * h3[j] + omega_4 * h4[j] + omega_5 * h5[j] + omega_6 * h6[j]) / omega;
  }
}
