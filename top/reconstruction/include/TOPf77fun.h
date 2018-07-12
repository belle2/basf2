//*****************************************************************************
//-----------------------------------------------------------------------------
// Fast simulation and reconstruction package for TOP counter (F77 core)
// M. Staric, March-2009, Sept-2011, Avg-2013
//-----------------------------------------------------------------------------
//
// TOPf77fun.h
// Definitions of FORTRAN 77 functions
//-----------------------------------------------------------------------------
//*****************************************************************************

#ifndef _TOPf77fun_h
#define _TOPf77fun_h

extern "C" {
  // fortran functions defined in top_geo.F
  void set_topvol_(float*, float*, float*, float*);
  void set_bfield_(float*);
  void set_simu_par_(float*, float*);
  void set_qbar_redg_(float*);
  int set_qbar_(float*, float*, float*, float*, float*, float*, float*, int*, int*);
  void set_bbwin_(int*, float*);
  void set_extvol_(int*, int*, int*, float*, float*, float*, float*, float*, float*, float*, float*);
  void arrange_pmt_(int*, int*, float*, float*, float*, float*);
  void set_rmi_(int*, float*);
  void set_xyc_(int*, float*, float*);
  int top_geo_finalize_(int*);
  float phase_index_(float*);
  float group_index_(float*);
  float abspt_leng_(float*);
  float mirror_refl_(float*);
  int inside_qbar_(float*, float*, float*, int*);
  int inside_extvol_(float*, float*, float*, int*);
  void to_global_(float*, float*, int*);
  void to_local_(float*, float*, int*);
  void track2top_(float*, float*, float*, float*, int*);
  void get_qbar_(int*, float*, float*, float*, int*, int*, int*, int*);
  void get_mirror_(int*, float*, float*, float*, float*, float*);
  void get_expvol_(int*, float*, float*, float*, float*, float*, float*, int*, int*);
  float get_bbwin_(int*);

  // fortran functions defined in top_pmt.F
  void set_pmt_(float*, float*, float*, float*, int*, int*, float*);
  void set_tts_(int*, float*, float*, float*);
  void read_qeffi_(const char*, float*, int);
  void set_qeffi_(float*, float*, int*, float*);
  float qe_pmt_(float*);
  void qe_range_(float*, float*);
  float qe_max_();
  float qe_nzero_();

  // fortran functions defined in top_tdc.F
  void set_tdc_(int*, float*, float*);
  void set_cfd_(float*, float*);

  // fortran functions defined in top_par.F
  void set_top_par_(float*, float*);
  void get_top_par_(float*, float*);

  // fortran functions defined in top_rtra.F
  void rtra_clear_();
  void rtra_set_hypo_(int*, float*);
  void rtra_set_hypid_(int*, int*);
  void rtra_get_hypid_(int*, int*);
  void rtra_put_(float*, float*, float*, float*, float*, float*, float*,
                 int*, int*, int*, int*);
  int rtra_getnum_();
  float rtra_get_plkh_(int*, int*);
  float rtra_get_sfot_(int*, int*);
  float rtra_get_bfot_(int*);
  int rtra_get_nfot_(int*);
  int rtra_get_flag_(int*);
  void rtra_get_(int*, float*, float*, int*, int*, int*, int*, int*);
  void rtra_gethit_(int*, int*, float*, float*, float*, float*, float*, int*);

  // fortran functions defined in top_data.F
  void data_clear_();
  void data_restore_();
  void data_put_(int*, int*, float*, float*, int*);
  int data_getnum_();
  void data_get_(int*, int*, int*, float*, float*);

  // fortran functions defined in top_digi_func.F
  int ich_digi_(float*, float*, int*, int*);
  int ich_digiz_(float*, float*, float*, int*);
  void ichxyz_(int*, int*, float*, float*, float*);
  int ichan_(int*, int*, int*, int*, int*, int*);
  void ichiii_(int*, int*, int*, int*, int*, int*, int*);
  int tdc_digi_(float*);
  float tdc_to_t_(int*);

  // fortran functions defined in top_reco.F
  void top_reco_();
  float cher_angle_(float*, float*, float*);
  void getnum_pulls_(int*);
  void get_pulls_(int*, float*, float*, float*, float*, float*, int*);
  float get_pdf_(int*, float*, float*, float*);

}

#endif







