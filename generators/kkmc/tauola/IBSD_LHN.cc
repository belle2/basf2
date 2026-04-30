#include <math.h>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <cmath>
#include <complex>

extern "C" void pigamma_left_(const double& Mtau, const double& Mpi,const double& m_rho, const double& Gamma_rho,const double& m_a1,const double& Gamma_a1,
			      const double& CV_LL, const double& CV_LR, const double& CS_RL, const double& CS_RR,
			      const double *ptau, const double *pnu, const double *ppi, const double *k, double &omega, double *hj);


void pigamma_left_(const double& Mtau, const double& Mpi,const double& m_rho, const double& Gamma_rho,const double& m_a1,const double& Gamma_a1,
		   const double& CV_LL, const double& CV_LR, const double& CS_RL, const double& CS_RR,
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
  //  const double CV_LL = 0.0;
  //  const double CV_LR = 0.0;
  //  const double CS_RL = 0.0;
  //  const double CS_RR = 0.0;
  const std::complex<double> I(0.0, 1.0);
    
  // Compute scalar products using Minkowski metric
  const double PpiPtau = ptau[3] * ppi[3] - ptau[2] * ppi[2] - ptau[1] * ppi[1] - ptau[0] * ppi[0]; 
  const double PnuPpi  = pnu[3] * ppi[3]  - pnu[2] * ppi[2]  - pnu[1] * ppi[1]  - pnu[0] * ppi[0];  
  const double PnuPtau = ptau[3] * pnu[3] - ptau[2] * pnu[2] - ptau[1] * pnu[1] - ptau[0] * pnu[0];  
  const double PkPtau  = k[3] * ptau[3]   - k[2] * ptau[2]   - k[1] * ptau[1]   - k[0] * ptau[0];  
  const double PkPpi   = k[3] * ppi[3]    - k[2] * ppi[2]    - k[1] * ppi[1]    - k[0] * ppi[0];  
  const double PkPnu   = k[3] * pnu[3]    - k[2] * pnu[2]    - k[1] * pnu[1]    - k[0] * pnu[0];

  const double t = (k[3] + ppi[3]) * (k[3] + ppi[3]) - (k[2] + ppi[2]) * (k[2] + ppi[2]) - (k[1] + ppi[1]) * (k[1] + ppi[1]) - (k[0] + ppi[0]) * (k[0] + ppi[0]);
  
  // F_V^π(t)
  const std::complex<double> FV_pi = (f_pi / (m_rho * m_rho - t - I * m_rho * Gamma_rho)) * std::exp((m_rho * m_rho) / (M_Borel * M_Borel));
  // F_A^π(t)
  const std::complex<double> FA_pi = (f_pi / (m_a1 * m_a1 - t - I * m_a1 * Gamma_a1)) * std::exp((m_a1 * m_a1) / (M_Borel * M_Borel));

  const double Mtau_sq = Mtau * Mtau;
  const double Mpi_sq = Mpi * Mpi;
  const double fpi_sq = (f_pi) * (f_pi);
  const double PpiPtau_sq = PpiPtau * PpiPtau;
  const double PkPtau_sq = PkPtau * PkPtau;
  const double PkPpi_sq = PkPpi * PkPpi;
  const double PkPnu_sq = PkPnu * PkPnu;
  const double PnuPpi_sq = PnuPpi * PnuPpi; 
  const double PnuPtau_sq = PnuPtau * PnuPtau;
  const double Mtau_3 = Mtau_sq * Mtau;

  // Compute decay amplitude squared |A|^2
  // -----------------------------------------------------------------------------------------------------
  // 
  //      |A_L|^2 = |A_L^IB|^2 + |A_L^V|^2 + |A_L^A|^2 
  //                + 2Re(A_L^V A_L^A*) + 2Re(A_L^IB A_L^V*) + 2Re(A_L^IB A_L^A*)
  //
  // -----------------------------------------------------------------------------------------------------
  
  // common combination of wilson coefficients and form factors
  

  const double mass_coeff = Mpi_sq / (Mtau * (M_u + M_d));
  const double CL_sum = 1.0 + CV_LL + CV_LR;
  const double CL_diff = 1.0 + CV_LL - CV_LR;
  const double CL_IB = CL_diff + mass_coeff * (CS_RL - CS_RR);
  
  // Declare variables
  double p1[3], p2[3], p3[3];
  double h1[3], h2[3], h3[3], h4[3], h5[3], h6[3];
  
  // Compute omega, h_j for |A_L^IB|^2
  
  
  // \omega  = |CL_IB|^2 f^2_\pi m_\tau^2 \left\{-m_\pi^2\frac{p_\nu \cdot p_\tau}{(k\cdot p_\pi)^2}
  //				 + \frac{(k\cdot p_\tau + m_\tau^2)(k\cdot p_\nu - p_\nu\cdot p_\tau) }{(k\cdot p_\tau)^2}   + \frac{2 \,(p_\nu\cdot p_\tau) (p_\pi\cdot p_\tau) + (k\cdot p_\tau) (p_\nu \cdot p_\pi) - (k\cdot p_\nu) (p_\pi \cdot p_\tau)}{(k\cdot p_\pi) (k\cdot p_\tau)}\right\},
  //
  double omega_1 = (CL_IB * CL_IB) * fpi_sq * Mtau_sq * (-Mpi_sq * (PnuPtau / PkPpi_sq) + ((PkPtau + Mtau_sq) * (PkPnu - PnuPtau) / PkPtau_sq) + 
							    ((2.0 * PnuPtau * PpiPtau + PkPtau * PnuPpi - PkPnu * PpiPtau) / (PkPpi * PkPtau)));
  
  // Compute h for each spatial component j = 1,2,3
  for (int j = 0; j < 3; j++) {
    // p_\nu^j \left(\frac{m_\pi^2}{(k\cdot p_\pi)^2} + \frac{m_\tau^2}{(k\cdot p_\tau)^2} - 
    // \frac{2 \,p_\pi\cdot p_\tau}{(k\cdot p_\pi) (k\cdot p_\tau)}\right)
    p1[j] = pnu[j] * (Mpi_sq / PkPpi_sq + Mtau_sq / PkPtau_sq - 2.0 * PpiPtau / (PkPpi * PkPtau));
    // k^j \left(\frac{p_\nu\cdot p_\tau - k\cdot p_\nu}{(k\cdot p_\tau)^2} 
    // - \frac{p_\nu \cdot p_\pi}{(k\cdot p_\pi )(k\cdot p_\tau)}\right)
    p2[j] = k[j] * ((PnuPtau - PkPnu) / PkPtau_sq - PnuPpi / (PkPpi * PkPtau));
    // p_\pi^j \frac{k\cdot p_\nu}{(k\cdot p_\pi)(k\cdot p_\tau)}
    p3[j] = ppi[j] * PkPnu / (PkPpi * PkPtau);
    
    h1[j] = (-(CL_IB * CL_IB) * fpi_sq * Mtau_3 / omega_1) * (p1[j] + p2[j] + p3[j]);
  }
  
  // compute omega, h_j for |A_L^V|^2
  
  // \omega  = 2 |(1+C^V_{LL} +  C^V_{LR})F_V^{(\pi)}|^2 \left[(k\cdot p_\nu)(k \cdot p_\pi)(p_\pi\cdot p_\tau) 
  //          - m_\pi^2 (k\cdot p_\nu)(k \cdot p_\tau) + (k\cdot p_\pi)(k\cdot p_\tau)(p_\nu\cdot p_\pi) \right],
  
  const double common_sd = (PkPnu * PkPpi * PpiPtau) - (Mpi_sq * PkPnu * PkPtau) + (PkPpi * PkPtau * PnuPpi);
  double omega_2 = 2.0 * (CL_sum * CL_sum) * std::norm(FV_pi) * common_sd;
  
  // Compute h for each spatial component j = 1,2,3
  for (int j = 0; j < 3; j++) {
    // k^j \left\{m_\pi^2 (k\cdot p_\nu) - (k\cdot p_\pi)(p_\nu\cdot p_\pi)\right\}
    p1[j] = k[j] * (Mpi_sq * PkPnu - PkPpi * PnuPpi); // j-1 ----> J
    // p_\pi^j (k\cdot p_\nu)(k\cdot p_\pi)
    p2[j] = ppi[j] * PkPnu * PkPpi;
    // 2 |(1+C^V_{LL} +  C^V_{LR})F_V^{(\pi)}|^2 \,\frac{m_\tau}{\omega}[p1[j] - p2[j]]
    
    h2[j] = (2.0 * (CL_sum * CL_sum) * std::norm(FV_pi) * Mtau / omega_2) * (p1[j] - p2[j]);
  }
  
  // compute omega, h_j for |A_L^A|^2
  
  // \omega  = 2 |(1+C^V_{LL} -  C^V_{LR})F_A^{(\pi)}|^2 \left[(k\cdot p_\nu)(k \cdot p_\pi)(p_\pi\cdot p_\tau) - 
  //              m_\pi^2 (k\cdot p_\nu)(k \cdot p_\tau) + (k\cdot p_\pi)(k\cdot p_\tau)(p_\nu\cdot p_\pi) \right],
  
  double omega_3 = 2.0 * (CL_diff * CL_diff) * std::norm(FA_pi) * common_sd;
  
  // Compute h for each spatial component j = 1,2,3
  
  for (int j = 0; j < 3; j++) {
    // k^j \left\{m_\pi^2 (k\cdot p_\nu) - (k\cdot p_\pi)(p_\nu\cdot p_\pi)\right\}
    p1[j] = k[j] * (Mpi_sq * PkPnu - PkPpi * PnuPpi);
    // p_\pi^j (k\cdot p_\nu)(k\cdot p_\pi)
    p2[j] = ppi[j] * PkPnu * PkPpi;
    // h^j   = 2 |(1+C^V_{LL} -  C^V_{LR})F_A^{(\pi)}|^2 \,\frac{m_\tau}{\omega} [p1[j]- p2[j]]
    
    h3[j] = (2.0 * (CL_diff * CL_diff) * std::norm(FA_pi) * Mtau / omega_3) * (p1[j] - p2[j]);
  }
  
  // compute interference terms 2Re(A_L^V A_L^A*)
  
  // \omega  = 4 \operatorname{Re}[F_V^{(\pi)}F_A^{(\pi)\ast} (1+C^V_{LL} +  C^V_{LR})(1+C^V_{LL} -  C^V_{LR})^\ast] 
  //              \left[(k\cdot p_\nu)(k \cdot p_\pi)(p_\pi\cdot p_\tau) - (k\cdot p_\pi)(k\cdot p_\tau)(p_\nu\cdot p_\pi) \right],
  
  const double re_va = CL_sum * CL_diff * std::real(FV_pi * std::conj(FA_pi));
  const double common_va = (PkPnu * PkPpi * PpiPtau) - (PkPpi * PkPtau * PnuPpi);
  double omega_4 = 4.0 * re_va * common_va;
  
  // Compute each spatial component j = 1,2,3
  for (int j = 0; j < 3; j++) {
    // k^j (k\cdot p_\pi)(p_\nu\cdot p_\pi)
    p1[j] = k[j] * PkPpi * PnuPpi;
    // p_\pi^j (k\cdot p_\nu)(k\cdot p_\pi)
    p2[j] = ppi[j] * PkPnu * PkPpi;
    
    // 4  \operatorname{Re}[F_V^{(\pi)}F_A^{(\pi)\ast} (1+C^V_{LL} +  C^V_{LR})(1+C^V_{LL} -  C^V_{LR})^\ast]\frac{m_\tau}{\omega}
    // [p1[j] - p2[j]]
    
    h4[j] = (4.0 * re_va * Mtau / omega_4) * (p1[j] - p2[j]);
  }
  
  // compute interference terms 2Re(A_L^IB A_L^V*)
  
  // \omega  = 2\operatorname{Re} [F_V^{(\pi)}  (1+C^V_{LL} -  C^V_{LR})(1+C^V_{LL} +  C^V_{LR})^\ast]  f_\pi m_\tau^2  
  //           \frac{(k\cdot p_\nu)(k\cdot p_\pi)}{k\cdot p_\tau},
  
  const double re_iv = CL_diff * CL_IB * std::real(FV_pi);
  double omega_5 = 2.0 * re_iv * f_pi * Mtau_sq * (PkPnu * PkPpi / PkPtau);
  
  // Compute each spatial component j = 1,2,3
  
  for (int j = 0; j < 3; j++) {
    // k^j\left\{(p_\pi\cdot p_\tau)(k\cdot p_\nu) - (p_\pi\cdot p_\tau)(p_\nu\cdot p_\tau) + m_\tau^2\,  p_\nu\cdot p_\pi\right\}
    p1[j] = k[j] * ((PpiPtau * PkPnu) - (PpiPtau * PnuPtau) + (Mtau_sq * PnuPpi));
    // p_\pi^j \left\{(k\cdot p_\nu)(k\cdot p_\tau) + m_\tau^2\, k\cdot p_\nu - (k\cdot p_\tau) (p_\nu\cdot p_\tau)\right\}
    p2[j] = ppi[j] * ((PkPnu * PkPtau) + Mtau_sq * PkPnu - PkPtau * PnuPtau);
    
    // h^j   = \frac{2\operatorname{Re} [F_V^{(\pi)}  (1+C^V_{LL} -  C^V_{LR})(1+C^V_{LL} +  C^V_{LR})^\ast]f_\pi}{k\cdot p_\tau}\frac{m_\tau}{\omega}[ p1[j] - p2[j]]
    
    h5[j] = ((2.0 * re_iv * f_pi * Mtau) / (PkPtau * omega_5)) * (p1[j] - p2[j]);
  }
  
  // compute interference terms 2Re(A_L^IB A_L^A*)
  
  // \omega  =\frac{ 2 f_\pi m_\tau^2  |1+C^V_{LL} -  C^V_{LR}|^2 \operatorname{Re}(F_A^{(\pi)}) }{(k\cdot p_\pi)(k\cdot p_\tau)}
  //          \left[ (k\cdot p_\nu)\left\{(k\cdot p_\pi)^2 + (k\cdot p_\pi)(p_\pi \cdot p_\tau) - m_\pi^2\, k\cdot p_\tau\right\}     
  //          + (k\cdot p_\pi)(k\cdot p_\tau)(p_\nu\cdot p_\pi) - (k\cdot p_\pi)^2 (p_\nu \cdot p_\tau)\right],
  
  const double re_ia = CL_diff * CL_IB * std::real(FA_pi);
  double omega_6 = ((2.0 * f_pi * Mtau_sq * re_ia) / (PkPpi * PkPtau)) *
    ((PkPnu * ( PkPpi_sq + (PkPpi * PpiPtau) - (Mpi_sq * PkPtau))) + (PkPpi * PkPtau * PnuPpi) - ( PkPpi_sq * PnuPtau));
  
  // Compute each spatial component j = 1,2,3
  
  for (int j = 0; j < 3; j++) {
    // k^j\left\{-m_\pi^2\, (k\cdot p_\tau) (p_\nu\cdot p_\tau) + (k\cdot p_\pi)(p_\pi\cdot p_\tau)(p_\nu \cdot p_\tau - k\cdot p_\nu)\right\}
    p1[j] = k[j] * (-Mpi_sq * PkPtau * PnuPtau + PkPpi * PpiPtau * (PnuPtau - PkPnu));
    // p_\nu^j \left\{m_\pi^2 (k\cdot p_\tau)^2 - 2(k\cdot p_\tau)(k\cdot p_\pi)(p_\pi\cdot p_\tau) + m_\tau^2\, (k\cdot p_\pi)(k\cdot p_\pi) \right\}
    p2[j] = pnu[j] * (Mpi_sq * PkPtau_sq - 2.0 * PkPtau * PkPpi * PpiPtau + Mtau_sq * PkPpi_sq);
    // p_\pi^j \left\{(k\cdot p_\tau)(k\cdot p_\pi)(k\cdot p_\nu + p_\nu \cdot p_\tau)\right\}
    p3[j] = ppi[j] * (PkPtau * PkPpi * (PkPnu + PnuPtau));
    
    // h^j   = -\frac{2 f_\pi \operatorname{Re}(F_A^{(\pi)} CL_diff CL_IB^*)}{(k\cdot p_\pi)(k\cdot p_\tau)}\frac{m_\tau}{\omega} [p1[j] + p2[j] + p3[j]]
    
    h6[j] = ((-2.0 * f_pi * re_ia * Mtau) / (PkPpi * PkPtau * omega_6)) * (p1[j] + p2[j] + p3[j]);
  }
  
  // Sum total amplitude
  omega = omega_1 + omega_2 + omega_3 + omega_4 + omega_5 + omega_6;
  
  for (int j = 0; j < 3 ; j++ ){
    hj[j] = (omega_1*h1[j] + omega_2*h2[j] + omega_3*h3[j] + omega_4*h4[j] + omega_5*h5[j] + omega_6*h6[j] ) / omega;
  }

  
  return;
}
