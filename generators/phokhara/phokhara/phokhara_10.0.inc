      implicit none 
c ------------------ couplings, masses, momenta -------------------
      double precision pi,gev2nbarn,gev2pbarn,alpha,me,mmu,
     & mpi,mpi0,Sp,ebeam,rmass(10),momenta(16,0:3),mp,mnt,mKp,mKn,
     & mlamb,meta,GFermi,sinthW2
c ------------------------ Kaons commons  -------------------------
      double precision m_phi0_Kp,g_phi0_Kp,m_phi1_Kp,g_phi1_Kp,
     & m_phi2_Kp,g_phi2_Kp,m_phi3_Kp,g_phi3_Kp,m_om0_Kp,g_om0_Kp,
     & m_om1_Kp,g_om1_Kp,m_om2_Kp,g_om2_Kp,m_om3_Kp,g_om3_Kp,m_om4_Kp,
     & g_om4_Kp,m_rho0_Kp,g_rho0_Kp,m_rho1_Kp,g_rho1_Kp,m_rho2_Kp,
     & g_rho2_Kp,m_rho3_Kp,g_rho3_Kp,m_rho4_Kp,g_rho4_Kp
      real*8 c_phi0_Kp,c_phi1_Kp,c_phi2_Kp,c_phi3_Kp,c_phin_Kp,
     & c_rho0_Kp,c_rho1_Kp,c_rho2_Kp,c_rho3_Kp,c_rho4_Kp,c_rhon_Kp,
     & c_om0_Kp,c_om1_Kp,c_om2_Kp,c_om3_Kp,c_om4_Kp,c_omn_Kp,
     & eta_phi_Kp,gam_phi_Kp,gam_rho_Kp,
     & gam_om_Kp,beta_phi_Kp,beta_rho_Kp,beta_om_Kp
      real*8 coeff_rho_Kp(1000),coeff_om_Kp(1000),coeff_phi_Kp(1000),
     & mass_n_rho_Kp(1000),mass_n_om_Kp(1000),mass_n_phi_Kp(1000),
     & gam_n_rho_Kp(1000),gam_n_om_Kp(1000),gam_n_phi_Kp(1000)
      real*8 tab_qq_Kp(80000)
      complex*16 tab_sum_rho_Kp(80000),tab_sum_om_Kp(80000),
     & tab_sum_phi_Kp(80000),KaonFormFactor_Sp
c ----------------------- Pi+ Pi- commons -------------------------
      double precision m_rho0_pion,g_rho0_pion,m_rho1_pion,g_rho1_pion,
     & m_rho2_pion,g_rho2_pion,m_rho3_pion,g_rho3_pion,m_om0_pion,
     & g_om0_pion    
      real*8 c_0_pion,c_1_pion,c_2_pion,c_3_pion,c_n_pion,c_om_pion,
     & gam_rho_pion,beta_rho_pion
      complex*16 PionFormFactor_Sp
      real*8 m_n_pionGS(0:2000),gam_n_pionGS(0:2000),c_n_pionGS(0:2000),
     1       dd_ffpi(0:2000),poch_ffpi(0:2000),par_pionGS(25),
     2       HH_p_ffpi(0:2000)
      complex*16 eephi,ee12,ee23,ee34,ee45
      real*8 grid_qq(1000001)
      complex*16 grid_ffpi(1000001)
c ------------------------- 4pi commons ---------------------------
      real*8 a1m_4pi,a1g_4pi,rhom_4pi,rhog_4pi,rho1m_4pi,rho1g_4pi,
     & rho2m_4pi,rho2g_4pi,omm_4pi,omg_4pi,f0m_4pi,f0g_4pi,pim_4pi,
     & coupl1_4pi,coupl2_4pi,coupl3_4pi,coupl_om_4pi,rho1m_f_4pi,
     & rho1g_f_4pi,rho2m_f_4pi,rho2g_f_4pi,rho3m_f_4pi,rho3g_f_4pi,
     & b1_a1_4pi,b2_a1_4pi,b3_a1_4pi,b1_f0_4pi,b2_f0_4pi,b3_f0_4pi,
     & b1_om_4pi,b2_om_4pi,b3_om_4pi,omm2_4pi,ommg_4pi,f0m2_4pi,
     & f0mg_4pi,a1m2_4pi,con_4pi
c ------------------ 3pi (Pi+ Pi- Pi0) commons --------------------
      real*8 omm0_3pi,omg0_3pi,omm1_3pi,omg1_3pi,omm2_3pi,omg2_3pi,
     & phim0_3pi,phig0_3pi,rhom0m_3pi,rhog0m_3pi,rhom0p_3pi,rhog0p_3pi,
     & rhom0z_3pi,rhog0z_3pi,rhom1m_3pi,rhog1m_3pi,rhom1p_3pi,
     & rhog1p_3pi,rhom1z_3pi,rhog1z_3pi,rhom2m_3pi,rhog2m_3pi,
     & rhom2p_3pi,rhog2p_3pi,rhom2z_3pi,rhog2z_3pi,
     & aa_3pi,bb_3pi,dd_3pi,ee_3pi,gg_3pi,jj_3pi,kk_3pi
c ------------------------ Lambdas -------------------------------
       double precision alpha_lamb,ratio_lamb      
c -------------------------- eta-pi-pi ---------------------------------
      double precision fpi,par_eta(9)
c ------------------ 0ph commons -------------------
      real*8 Ophvirtsoft
      complex*16 Leptonic_epl_emin(0:3,0:3)
c -------------------- narrow resonances -------------------------
      real*8 mjp,gamjp,gamjpee,bjpmm,mp2s,gamp2s,gamp2see,Br_phi_ee
      complex*16 KaonFormFactor_mjp,KaonFormFactor_mp2s,cjp_Kp,
     &           cjp_K0,cp2s_Kp,cp2s_K0
c -------------------------- cuts --------------------------------
      double precision Emin,gmin,phot1cut,phot2cut,pi1cut,pi2cut,
     & piphcut,accecut,q2min,w,q2_max_c,ver_s,q2_min_c
c ------------------------ histograms -----------------------------
      character*20 title(0:20)
      double precision xlow(0:20),xup(0:20)
      integer bins(0:20),iprint
c ------------------------- Maxima --------------------------------
      double precision Mmax(0:2),gross(0:2),klein(0:2),tr(0:2)
     &,count(0:2)
c ------------------------ generation -----------------------------
      real*8 onemb2c2
c -------------------------- flags --------------------------------
      integer nlo,ph0,pion,fsr,fsrnlo,ivac,FF_Pion,f0_model,FF_kaon,
     &        narr_res,FF_pp,chi_sw,chi_pion,be_r,FF_Pgg,nlo2
c ------------------------ vacuum ---------------------------------
      complex*16 cvac_s,cvac_s_pi,cvac_s_pi1
c -----------------------------------------------------------------
c ------------------ couplings, masses, momenta -------------------
      common /ctes/pi,gev2nbarn,gev2pbarn,alpha,me,mmu,mpi,mpi0,Sp,mp,
     & ebeam,ver_s,rmass,momenta,cvac_s,mnt,mKp,mKn,cvac_s_pi,
     & cvac_s_pi1,mlamb,meta,GFermi,sinthW2
c ---------------------- Kaons commons ----------------------------
      common/mass_gamm_Kp/m_phi0_Kp,g_phi0_Kp,m_phi1_Kp,g_phi1_Kp,
     & m_phi2_Kp,g_phi2_Kp,m_phi3_Kp,g_phi3_Kp,m_om0_Kp,g_om0_Kp,
     & m_om1_Kp,g_om1_Kp,m_om2_Kp,g_om2_Kp,m_om3_Kp,g_om3_Kp,m_om4_Kp,
     & g_om4_Kp,m_rho0_Kp,g_rho0_Kp,m_rho1_Kp,g_rho1_Kp,m_rho2_Kp,
     & g_rho2_Kp,m_rho3_Kp,g_rho3_Kp,m_rho4_Kp,g_rho4_Kp
      common/param_Kp/c_phi0_Kp,c_phi1_Kp,c_phi2_Kp,c_phi3_Kp,
     & c_phin_Kp,c_rho0_Kp,c_rho1_Kp,c_rho2_Kp,c_rho3_Kp,c_rho4_Kp,
     & c_rhon_Kp,c_om0_Kp,c_om1_Kp,c_om2_Kp,c_om3_Kp,c_om4_Kp,c_omn_Kp,
     & eta_phi_Kp,gam_phi_Kp,gam_rho_Kp,gam_om_Kp,beta_phi_Kp,
     & beta_rho_Kp,beta_om_Kp
      common/param_sum_Kp/coeff_rho_Kp,coeff_om_Kp,coeff_phi_Kp,
     & mass_n_rho_Kp,mass_n_om_Kp,mass_n_phi_Kp,gam_n_rho_Kp,
     & gam_n_om_Kp,gam_n_phi_Kp,tab_qq_Kp,
     & tab_sum_rho_Kp,tab_sum_om_Kp,tab_sum_phi_Kp
c --------------------- Pi+ Pi- commons ---------------------------
      common/mass_gamm_pion/m_rho0_pion,g_rho0_pion,m_rho1_pion,
     & g_rho1_pion,m_rho2_pion,g_rho2_pion,m_rho3_pion,g_rho3_pion,
     & m_om0_pion,g_om0_pion
      common/param_pion/c_0_pion,c_1_pion,c_2_pion,c_3_pion,c_n_pion,
     & c_om_pion
      common/sum_par_pion/gam_rho_pion,beta_rho_pion
c ---
      common/FF_Sp/PionFormFactor_Sp,KaonFormFactor_Sp
      common/pion_ff_2008/par_pionGS,m_n_pionGS,gam_n_pionGS,c_n_pionGS,
     1 dd_ffpi,poch_ffpi,eephi,ee12,ee23,ee34,ee45,HH_p_ffpi
      common/cgrid/grid_qq,grid_ffpi
c ------------------------ 4pi commons -----------------------------
      common /pi4_par/ 
     & a1m_4pi,a1g_4pi,rhom_4pi,rhog_4pi,rho1m_4pi,rho1g_4pi,
     & rho2m_4pi,rho2g_4pi,omm_4pi,omg_4pi,f0m_4pi,f0g_4pi,pim_4pi,
     & coupl1_4pi,coupl2_4pi,coupl3_4pi,coupl_om_4pi,rho1m_f_4pi,
     & rho1g_f_4pi,rho2m_f_4pi,rho2g_f_4pi,rho3m_f_4pi,rho3g_f_4pi,
     & b1_a1_4pi,b2_a1_4pi,b3_a1_4pi,b1_f0_4pi,b2_f0_4pi,b3_f0_4pi,
     & b1_om_4pi,b2_om_4pi,b3_om_4pi,omm2_4pi,ommg_4pi,f0m2_4pi,
     & f0mg_4pi,a1m2_4pi,con_4pi
c -------------------- 3pi (Pi+ Pi- Pi0) commons --------------------
      common/mass_3pi/omm0_3pi,omg0_3pi,omm1_3pi,omg1_3pi,omm2_3pi,
     & omg2_3pi,phim0_3pi,phig0_3pi 
      common/rho_3pi/rhom0m_3pi,rhog0m_3pi,rhom0p_3pi,rhog0p_3pi,
     & rhom0z_3pi,rhog0z_3pi,rhom1m_3pi,rhog1m_3pi,rhom1p_3pi,
     & rhog1p_3pi,rhom1z_3pi,rhog1z_3pi,rhom2m_3pi,rhog2m_3pi,
     & rhom2p_3pi,rhog2p_3pi,rhom2z_3pi,rhog2z_3pi
      common/formf_3pi/aa_3pi,bb_3pi,dd_3pi,ee_3pi,gg_3pi,jj_3pi,kk_3pi
c -------------------------- Lambdas ---------------------------------
      common/lambda_par/ alpha_lamb,ratio_lamb
c -------------------------- eta-pi-pi ---------------------------------
      common/eta_par/fpi,par_eta
c ------------------ 0ph commons -------------------
      common/eetensor/Leptonic_epl_emin,Ophvirtsoft
c -------------------- narrow resonances -------------------------
      common/narrow_res/mjp,gamjp,gamjpee,mp2s,gamp2s,gamp2see
     &  ,Br_phi_ee,cjp_Kp,cjp_K0,cp2s_Kp,cp2s_K0,
     &   KaonFormFactor_mjp,KaonFormFactor_mp2s
c ---------------------------- cuts ----------------------------------
      common/cuts/ Emin,gmin,phot1cut,phot2cut,pi1cut,pi2cut,
     & piphcut,accecut,q2min,w,q2_max_c,q2_min_c
c ------------------------- histograms -------------------------------
      common /histo/   xlow,xup,bins,title
c --------------------------- Maxima ---------------------------------
      common /maxima/  Mmax,gross,klein,tr,count,iprint
c --------------------------- flags ----------------------------------
      common/flags/nlo,ph0,pion,fsr,fsrnlo,ivac,FF_Pion,f0_model,
     & FF_kaon,narr_res,FF_pp,chi_sw,chi_pion,be_r,FF_Pgg,nlo2
c ------------------------ generation -----------------------------
      common /generation/onemb2c2
c-----------------------Nucleons Common FF_pp=1-------------------------
      real*8 beta1_pp,beta2_pp,beta3_pp,beta4_pp,beta5_pp,beta6_pp,
     & beta7_pp,beta8_pp,beta9_pp,beta10_pp,beta11_pp,beta12_pp,
     & beta13_pp,beta14_pp,beta15_pp,beta16_pp,beta17_pp,
     & beta18_pp,beta19_pp,beta20_pp,
     & mrho_pp0,gammarho_pp0,momega_pp0,gammaomega_pp0,
     & mrho_pp1,gammarho_pp1,momega_pp1,gammaomega_pp1,
     & mrho_pp2,gammarho_pp2,momega_pp2,gammaomega_pp2,
     & mrho_pp3,gammarho_pp3,momega_pp3,gammaomega_pp3,
     & mrho_pp4,gammarho_pp4,momega_pp4,gammaomega_pp4
      complex*16 par1,par2,par3,par4,par5,par6,par7,par8,
     2    par9,par10,par11,par12,par13,par14,par15,par16
      common/protparam2/beta1_pp,beta2_pp,beta3_pp,beta4_pp,beta5_pp,
     & beta6_pp,beta7_pp,beta8_pp,beta9_pp,beta10_pp,beta11_pp,
     & beta12_pp,beta13_pp,beta14_pp,beta15_pp,beta16_pp,beta17_pp,
     & beta18_pp,beta19_pp,beta20_pp,mrho_pp0,gammarho_pp0,
     & momega_pp0,
     & gammaomega_pp0,mrho_pp1,gammarho_pp1,momega_pp1,
     &gammaomega_pp1,mrho_pp2,gammarho_pp2,momega_pp2,
     &gammaomega_pp2,mrho_pp3,gammarho_pp3,momega_pp3,
     &gammaomega_pp3,
     & mrho_pp4,gammarho_pp4,momega_pp4,gammaomega_pp4,
     & par1,par2,par3,par4,par5,par6,par7,par8,
     2    par9,par10,par11,par12,par13,par14,par15,par16
c----------------Chi production common-----------------------------------------
      real*8 mchic1,gamchic1,c_chi,c_chipsi,gamchiee,
     & mchic2,gamchic2,c_chi2,c_chi2psi,gamchiee2,ac,m2c,aj,
     & apsi
      complex*16 gg1,gg2
      common/chi_prod/ mchic1,gamchic1,c_chi,c_chipsi,gg1,
     & gamchiee,
     & mchic2,gamchic2,c_chi2,c_chi2psi,gamchiee2,gg2,ac,m2c,
     & aj,apsi
c----------beam resolution---------------------------------------------------------
      real*8 beamres
      common/rozm/ beamres
c-----------------------------------------------------------------------
      real*8 hv1,A1pi0,A2pi0,Fsmv2Hv2,A1eta,A1etaP,fsmv1,H_om,F_om,
     1 F_phi,fchir,A_pi_phiom,A_eta_phiom
      real*8 GammaTotalrho_c,GammaTotalrho_c2,GammaTotalrho_c3,
     1 GammaTotalomega_c,GammaTotalomega_c2,GammaTotalomega_c3,
     2 GammaTotalphi_c,GammaTotalphi_c2,GammaTotalphi_c3
      real*8 Mrho_c,Momega_c,Mphi_c,
     1 Mrho_pr,Momega_pr,Mphi_pr,
     2 Mrho_dpr,Momega_dpr,Mphi_dpr
      real*8 metaf,metaP,mpi0f
      real*8 Cq_c,Cs_c,Cq_P,Cs_P
      real*8 A3pi0,A3eta,A3etaP
      common/pscalars/hv1,A1pi0,A2pi0,Fsmv2Hv2,A1eta,A1etaP,fsmv1,H_om,
     1 F_om,F_phi,fchir,A_pi_phiom, A_eta_phiom,
     2 GammaTotalrho_c,GammaTotalrho_c2,GammaTotalrho_c3,
     2 GammaTotalomega_c,GammaTotalomega_c2,GammaTotalomega_c3,
     3 GammaTotalphi_c,GammaTotalphi_c2,GammaTotalphi_c3,
     4 Mrho_c,Momega_c,Mphi_c,
     5 Mrho_pr,Momega_pr,Mphi_pr,
     6 Mrho_dpr,Momega_dpr,Mphi_dpr,metaf,metaP,mpi0f,
     7 Cq_c,Cs_c,Cq_P,Cs_P,A3pi0,A3eta,A3etaP
c-------------------common quad momenta--------------------------------
       real*16 piq,Sp_q,w_q,ebeam_q,Qjp_q,Qp2s_q,
     1 mpi_q,mjp_q,gamjp_q,el_m2q,me_q,alpha_q
       common/quad_param/ piq,Sp_q,w_q,ebeam_q,Qjp_q,Qp2s_q,
     1 mpi_q,mjp_q,gamjp_q,el_m2q,me_q,alpha_q
