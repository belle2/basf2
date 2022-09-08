/*
 * integral.h - scalar integrals
 *
 * this file is part of PJFry library
 * Copyright 2011 Valery Yundin
 */

#ifndef QUL_INTEGRAL_H
#define QUL_INTEGRAL_H

#include "common.h"
#include "kinem.h"
#include "cache.h"

// #define USE_ONELOOP 1
// #define USE_QCDLOOP 1

class Initialize {
public:
  Initialize();
  ~Initialize();
};

ICache::Ival qlI1(const Kinem1& k);
ICache::Ival qlI2(const Kinem2& k);
ICache::Ival qlI3(const Kinem3& k);
ICache::Ival qlI4(const Kinem4& k);

#ifdef USE_QCDLOOP
#define qlprec F77_FUNC(ffprec,FFPREC)
#define qlflag F77_FUNC(ffflag,FFFLAG)
# ifdef HAVE_QCDLOOP1_H
#   include <qcdloop1.h>
# else
extern "C" {
  void F77_FUNC(qlinit, QLINIT)();
  void F77_FUNC(ffexi, FFEXI)();

#ifdef USE_F2C
  void F77_FUNC(qli1, QLI1)(std::complex<double>* rslt, double* m1, double* mu2, int* ep);
  void F77_FUNC(qli2, QLI2)(std::complex<double>* rslt, double* p1, double* m1, double* m2, double* mu2, int* ep);
  void F77_FUNC(qli3, QLI3)(std::complex<double>* rslt, double* p1, double* p2, double* p3, double* m1, double* m2, double* m3,
                            double* mu2, int* ep);
  void F77_FUNC(qli4, QLI4)(std::complex<double>* rslt, double* p1, double* p2, double* p3, double* p4, double* s12, double* s23,
                            double* m1, double* m2, double* m3, double* m4, double* mu2, int* ep);
#else
  std::complex<double> F77_FUNC(qli1, QLI1)(double* m1, double* mu2, int* ep);
  std::complex<double> F77_FUNC(qli2, QLI2)(double* p1, double* m1, double* m2, double* mu2, int* ep);
  std::complex<double> F77_FUNC(qli3, QLI3)(double* p1, double* p2, double* p3, double* m1, double* m2, double* m3, double* mu2,
                                            int* ep);
  std::complex<double> F77_FUNC(qli4, QLI4)(double* p1, double* p2, double* p3, double* p4, double* s12, double* s23, double* m1,
                                            double* m2, double* m3, double* m4, double* mu2, int* ep);
#endif

  extern struct {
    double xloss, precx, precc, xalogm, xclogm, xalog2, xclog2, reqprc;
  } qlprec;
  extern struct {
    int lwrite, ltest, l4also, ldc3c4, lmem, lwarn, ldot,
        nevent, ner, id, idsub, nwidth, nschem, onshel, idot;
  } qlflag;
}
#endif /* HAVE_QCDLOOP1_H */
#endif /* USE_QCDLOOP */

#ifdef USE_ONELOOP
extern "C" {
  void F77_FUNC_(avh_olo_mu_set, AVH_OLO_MU_SET)(double* mu);
  void F77_FUNC_(avh_olo_onshell, AVH_OLO_ONSHELL)(double* thrs);
  void F77_FUNC_(avh_olo_a0m, AVH_OLO_A0M)(std::complex<double>* rslt, double* m1);
  void F77_FUNC_(avh_olo_b0m, AVH_OLO_B0M)(std::complex<double>* rslt, double* p1, double* m1, double* m2);
  void F77_FUNC_(avh_olo_c0m, AVH_OLO_C0M)(std::complex<double>* rslt, double* p1, double* p2, double* p3, double* m1, double* m2,
                                           double* m3);
  void F77_FUNC_(avh_olo_d0m, AVH_OLO_D0M)(std::complex<double>* rslt, double* p1, double* p2, double* p3, double* p4, double* p12,
                                           double* p23, double* m1, double* m2, double* m3, double* m4);
}
#endif /* USE_ONELOOP */

#endif /* QUL_INTEGRAL_H */
