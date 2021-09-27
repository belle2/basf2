/*
 * pjfry.cpp - interface functions
 *
 * this file is part of PJFry library
 * Copyright 2011 Valery Yundin
 */

#include "common.h"
#include "cache.h"
#include "kinem.h"

#include "pjfry.h"

/* ------------------------------------------------------------
 * ------------------------------------------------------------
 *                PENTAGON Integral section
 * ------------------------------------------------------------
 * ------------------------------------------------------------
*/

// Masses shifted to the left to comply with LoopTools notation

pj_complex PJFry::E0v0(const double p1,  const double p2,  const double p3,  const double p4,  const double p5,
                      const double s12, const double s23, const double s34, const double s45, const double s15,
                      const double m5,  const double m1,  const double m2,  const double m3,  const double m4)
{
  Kinem5 kinem(p1, p2, p3, p4, p5, s12, s23, s34, s45, s15, m1, m2, m3, m4, m5);
  return ICache::getE(0, kinem);
}

pj_complex PJFry::E0v0(const double p1,  const double p2,  const double p3,  const double p4,  const double p5,
                      const double s12, const double s23, const double s34, const double s45, const double s15,
                      const double m5,  const double m1,  const double m2,  const double m3,  const double m4, const int ep)
{
  Kinem5 kinem(p1, p2, p3, p4, p5, s12, s23, s34, s45, s15, m1, m2, m3, m4, m5);
  return ICache::getE(abs(ep), kinem);
}


pj_complex PJFry::E0v1(const int i,
                      const double p1,  const double p2,  const double p3,  const double p4,  const double p5,
                      const double s12, const double s23, const double s34, const double s45, const double s15,
                      const double m5,  const double m1,  const double m2,  const double m3,  const double m4)
{
  Kinem5 kinem(p1, p2, p3, p4, p5, s12, s23, s34, s45, s15, m1, m2, m3, m4, m5);
  return -ICache::getE(0, i, kinem);
}

pj_complex PJFry::E0v1(const int i,
                      const double p1,  const double p2,  const double p3,  const double p4,  const double p5,
                      const double s12, const double s23, const double s34, const double s45, const double s15,
                      const double m5,  const double m1,  const double m2,  const double m3,  const double m4, const int ep)
{
  Kinem5 kinem(p1, p2, p3, p4, p5, s12, s23, s34, s45, s15, m1, m2, m3, m4, m5);
  return -ICache::getE(abs(ep), i, kinem);
}


pj_complex PJFry::E0v2(const int i, const int j,
                      const double p1,  const double p2,  const double p3,  const double p4,  const double p5,
                      const double s12, const double s23, const double s34, const double s45, const double s15,
                      const double m5,  const double m1,  const double m2,  const double m3,  const double m4)
{
  Kinem5 kinem(p1, p2, p3, p4, p5, s12, s23, s34, s45, s15, m1, m2, m3, m4, m5);
  return ICache::getE(0, i, j, kinem);
}

pj_complex PJFry::E0v2(const int i, const int j,
                      const double p1,  const double p2,  const double p3,  const double p4,  const double p5,
                      const double s12, const double s23, const double s34, const double s45, const double s15,
                      const double m5,  const double m1,  const double m2,  const double m3,  const double m4, const int ep)
{
  Kinem5 kinem(p1, p2, p3, p4, p5, s12, s23, s34, s45, s15, m1, m2, m3, m4, m5);
  return ICache::getE(abs(ep), i, j, kinem);
}


pj_complex PJFry::E0v3(const int i, const int j, const int k,
                      const double p1,  const double p2,  const double p3,  const double p4,  const double p5,
                      const double s12, const double s23, const double s34, const double s45, const double s15,
                      const double m5,  const double m1,  const double m2,  const double m3,  const double m4)
{
  Kinem5 kinem(p1, p2, p3, p4, p5, s12, s23, s34, s45, s15, m1, m2, m3, m4, m5);
  return -ICache::getE(0, i, j, k, kinem);
}

pj_complex PJFry::E0v3(const int i, const int j, const int k,
                      const double p1,  const double p2,  const double p3,  const double p4,  const double p5,
                      const double s12, const double s23, const double s34, const double s45, const double s15,
                      const double m5,  const double m1,  const double m2,  const double m3,  const double m4, const int ep)
{
  Kinem5 kinem(p1, p2, p3, p4, p5, s12, s23, s34, s45, s15, m1, m2, m3, m4, m5);
  return -ICache::getE(abs(ep), i, j, k, kinem);
}


pj_complex PJFry::E0v4(const int i, const int j, const int k, const int l,
                      const double p1,  const double p2,  const double p3,  const double p4,  const double p5,
                      const double s12, const double s23, const double s34, const double s45, const double s15,
                      const double m5,  const double m1,  const double m2,  const double m3,  const double m4)
{
  Kinem5 kinem(p1, p2, p3, p4, p5, s12, s23, s34, s45, s15, m1, m2, m3, m4, m5);
  return ICache::getE(0, i, j, k, l, kinem);
}

pj_complex PJFry::E0v4(const int i, const int j, const int k, const int l,
                      const double p1,  const double p2,  const double p3,  const double p4,  const double p5,
                      const double s12, const double s23, const double s34, const double s45, const double s15,
                      const double m5,  const double m1,  const double m2,  const double m3,  const double m4, const int ep)
{
  Kinem5 kinem(p1, p2, p3, p4, p5, s12, s23, s34, s45, s15, m1, m2, m3, m4, m5);
  return ICache::getE(abs(ep), i, j, k, l, kinem);
}


pj_complex PJFry::E0v5(const int i, const int j, const int k, const int l, const int m,
                      const double p1,  const double p2,  const double p3,  const double p4,  const double p5,
                      const double s12, const double s23, const double s34, const double s45, const double s15,
                      const double m5,  const double m1,  const double m2,  const double m3,  const double m4)
{
  Kinem5 kinem(p1, p2, p3, p4, p5, s12, s23, s34, s45, s15, m1, m2, m3, m4, m5);
  return -ICache::getE(0, i, j, k, l, m, kinem);
}

pj_complex PJFry::E0v5(const int i, const int j, const int k, const int l, const int m,
                      const double p1,  const double p2,  const double p3,  const double p4,  const double p5,
                      const double s12, const double s23, const double s34, const double s45, const double s15,
                      const double m5,  const double m1,  const double m2,  const double m3,  const double m4, const int ep)
{
  Kinem5 kinem(p1, p2, p3, p4, p5, s12, s23, s34, s45, s15, m1, m2, m3, m4, m5);
  return -ICache::getE(abs(ep), i, j, k, l, m, kinem);
}

/* ------------------------------------------------------------
 * ------------------------------------------------------------
 *                BOX Integral section
 * ------------------------------------------------------------
 * ------------------------------------------------------------
*/

// No mass shift for scalar 4-poconst int D0

pj_complex PJFry::D0v0(const double p1,  const double p2,  const double p3,  const double p4,
                      const double s12, const double s23,
                      const double m1,  const double m2,  const double m3,  const double m4)
{
  Kinem4 kinem(p1, p2, p3, p4, s12, s23, m1, m2, m3, m4);
  return ICache::getD(0, kinem);
}

pj_complex PJFry::D0v0(const double p1,  const double p2,  const double p3,  const double p4,
                      const double s12, const double s23,
                      const double m1,  const double m2,  const double m3,  const double m4, const int ep)
{
  Kinem4 kinem(p1, p2, p3, p4, s12, s23, m1, m2, m3, m4);
  return ICache::getD(abs(ep), kinem);
}

// Masses shifted to the left to comply with LoopTools notation

pj_complex PJFry::D0v1(const int i,
                      const double p1,  const double p2,  const double p3,  const double p4,
                      const double s12, const double s23,
                      const double m4,  const double m1,  const double m2,  const double m3)
{
  Kinem4 kinem(p1, p2, p3, p4, s12, s23, m1, m2, m3, m4);
  return -ICache::getD(0, i, kinem);
}

pj_complex PJFry::D0v1(const int i,
                      const double p1,  const double p2,  const double p3,  const double p4,
                      const double s12, const double s23,
                      const double m4,  const double m1,  const double m2,  const double m3, const int ep)
{
  Kinem4 kinem(p1, p2, p3, p4, s12, s23, m1, m2, m3, m4);
  return -ICache::getD(abs(ep), i, kinem);
}


pj_complex PJFry::D0v2(const int i, const int j,
                      const double p1,  const double p2,  const double p3,  const double p4,
                      const double s12, const double s23,
                      const double m4,  const double m1,  const double m2,  const double m3)
{
  Kinem4 kinem(p1, p2, p3, p4, s12, s23, m1, m2, m3, m4);
  return ICache::getD(0, i, j, kinem);
}

pj_complex PJFry::D0v2(const int i, const int j,
                      const double p1,  const double p2,  const double p3,  const double p4,
                      const double s12, const double s23,
                      const double m4,  const double m1,  const double m2,  const double m3, const int ep)
{
  Kinem4 kinem(p1, p2, p3, p4, s12, s23, m1, m2, m3, m4);
  return ICache::getD(abs(ep), i, j, kinem);
}


pj_complex PJFry::D0v3(const int i, const int j, const int k,
                      const double p1,  const double p2,  const double p3,  const double p4,
                      const double s12, const double s23,
                      const double m4,  const double m1,  const double m2,  const double m3)
{
  Kinem4 kinem(p1, p2, p3, p4, s12, s23, m1, m2, m3, m4);
  return -ICache::getD(0, i, j, k, kinem);
}

pj_complex PJFry::D0v3(const int i, const int j, const int k,
                      const double p1,  const double p2,  const double p3,  const double p4,
                      const double s12, const double s23,
                      const double m4,  const double m1,  const double m2,  const double m3, const int ep)
{
  Kinem4 kinem(p1, p2, p3, p4, s12, s23, m1, m2, m3, m4);
  return -ICache::getD(abs(ep), i, j, k, kinem);
}


pj_complex PJFry::D0v4(const int i, const int j, const int k, const int l,
                      const double p1,  const double p2,  const double p3,  const double p4,
                      const double s12, const double s23,
                      const double m4,  const double m1,  const double m2,  const double m3)
{
  Kinem4 kinem(p1, p2, p3, p4, s12, s23, m1, m2, m3, m4);
  return ICache::getD(0, i, j, k, l, kinem);
}

pj_complex PJFry::D0v4(const int i, const int j, const int k, const int l,
                      const double p1,  const double p2,  const double p3,  const double p4,
                      const double s12, const double s23,
                      const double m4,  const double m1,  const double m2,  const double m3, const int ep)
{
  Kinem4 kinem(p1, p2, p3, p4, s12, s23, m1, m2, m3, m4);
  return ICache::getD(abs(ep), i, j, k, l, kinem);
}


/* ------------------------------------------------------------
 * ------------------------------------------------------------
 *                TRIANGLE Integral section
 * ------------------------------------------------------------
 * ------------------------------------------------------------
*/

// No mass shift for scalar 3-poconst int C0

pj_complex PJFry::C0v0(const double p1,  const double p2,  const double p3,
                      const double m1,  const double m2,  const double m3)
{
  Kinem3 kinem(p1, p2, p3, m1, m2, m3);
  return ICache::getC(0, kinem);
}

pj_complex PJFry::C0v0(const double p1,  const double p2,  const double p3,
                      const double m1,  const double m2,  const double m3, const int ep)
{
  Kinem3 kinem(p1, p2, p3, m1, m2, m3);
  return ICache::getC(abs(ep), kinem);
}

// Masses shifted to the left to comply with LoopTools notation

pj_complex PJFry::C0v1(const int i,
                      const double p1,  const double p2,  const double p3,
                      const double m3,  const double m1,  const double m2)
{
  Kinem3 kinem(p1, p2, p3, m1, m2, m3);
  return -ICache::getC(0, i, kinem);
}

pj_complex PJFry::C0v1(const int i,
                      const double p1,  const double p2,  const double p3,
                      const double m3,  const double m1,  const double m2, const int ep)
{
  Kinem3 kinem(p1, p2, p3, m1, m2, m3);
  return -ICache::getC(abs(ep), i, kinem);
}


pj_complex PJFry::C0v2(const int i, const int j,
                      const double p1,  const double p2,  const double p3,
                      const double m3,  const double m1,  const double m2)
{
  Kinem3 kinem(p1, p2, p3, m1, m2, m3);
  return ICache::getC(0, i, j, kinem);
}

pj_complex PJFry::C0v2(const int i, const int j,
                      const double p1,  const double p2,  const double p3,
                      const double m3,  const double m1,  const double m2, const int ep)
{
  Kinem3 kinem(p1, p2, p3, m1, m2, m3);
  return ICache::getC(abs(ep), i, j, kinem);
}


pj_complex PJFry::C0v3(const int i, const int j, const int k,
                      const double p1,  const double p2,  const double p3,
                      const double m3,  const double m1,  const double m2)
{
  Kinem3 kinem(p1, p2, p3, m1, m2, m3);
  return -ICache::getC(0, i, j, k, kinem);
}

pj_complex PJFry::C0v3(const int i, const int j, const int k,
                      const double p1,  const double p2,  const double p3,
                      const double m3,  const double m1,  const double m2, const int ep)
{
  Kinem3 kinem(p1, p2, p3, m1, m2, m3);
  return -ICache::getC(abs(ep), i, j, k, kinem);
}

/* ------------------------------------------------------------
 * ------------------------------------------------------------
 *                BUBBLE Integral section
 * ------------------------------------------------------------
 * ------------------------------------------------------------
*/

// Masses shifted to the left to comply with LoopTools notation

pj_complex PJFry::B0v0(const double p1, const double m2, const double m1)
{
  Kinem2 kinem(p1, m1, m2);
  return ICache::getB(0, kinem);
}

pj_complex PJFry::B0v0(const double p1, const double m2, const double m1, const int ep)
{
    Kinem2 kinem(p1, m1, m2);
    return ICache::getB(abs(ep), kinem);
}


pj_complex PJFry::B0v1(const int i, const double p1, const double m2, const double m1)
{
  Kinem2 kinem(p1, m1, m2);
  return -ICache::getB(0, i, kinem);
}

pj_complex PJFry::B0v1(const int i, const double p1, const double m2, const double m1, const int ep)
{
    Kinem2 kinem(p1, m1, m2);
    return -ICache::getB(abs(ep), i, kinem);
}


pj_complex PJFry::B0v2(const int i, const int j, const double p1, const double m2, const double m1)
{
  Kinem2 kinem(p1, m1, m2);
  return ICache::getB(0, i, j, kinem);
}

pj_complex PJFry::B0v2(const int i, const int j, const double p1, const double m2, const double m1, const int ep)
{
    Kinem2 kinem(p1, m1, m2);
    return ICache::getB(abs(ep), i, j, kinem);
}

/* ------------------------------------------------------------
* ------------------------------------------------------------
*                TADPOLE Integral section
* ------------------------------------------------------------
* ------------------------------------------------------------
*/


pj_complex PJFry::A0v0(const double m1)
{
    return ICache::getA(0, Kinem1(m1));
}

pj_complex PJFry::A0v0(const double m1, const int ep)
{
    return ICache::getA(abs(ep), Kinem1(m1));
}

/* ------------------------------------------------------------
 * ------------------------------------------------------------
 *                MISC section
 * ------------------------------------------------------------
 * ------------------------------------------------------------
 */


double PJFry::GetMu2()
{
  return ICache::getMu2();
}


double PJFry::SetMu2(const double mu2)
{
  return ICache::setMu2(mu2);
}

void PJFry::ClearCache()
{
  // clear only coefficient cache and minor cache
  // keep scalar integrals cache
  ICache::ClearCC();
  MCache::Clear();
}

// =============================================================
// =============================================================
// =============================================================

double pgetmusq_()
{
  return PJFry::GetMu2();
}

void psetmusq_(double *mu2)
{
  PJFry::SetMu2(*mu2);
}

void pclearcache_()
{
  PJFry::ClearCache();
}

#ifdef USE_F2C
// f2c,g77,ifort calling convention section (result is the first parameter)
void pe0_(pj_complex *rslt,
          double *p1,  double *p2,  double *p3,  double *p4,  double *p5,
          double *s12, double *s23, double *s34, double *s45, double *s15,
          double *m1,  double *m2,  double *m3,  double *m4,  double *m5, int *ep)
{
  *rslt=PJFry::E0v0(*p1,*p2,*p3,*p4,*p5,*s12,*s23,*s34,*s45,*s15,*m1,*m2,*m3,*m4,*m5,*ep);
}

void pe0i_(pj_complex *rslt,
           int *i,
           double *p1,  double *p2,  double *p3,  double *p4,  double *p5,
           double *s12, double *s23, double *s34, double *s45, double *s15,
           double *m1,  double *m2,  double *m3,  double *m4,  double *m5, int *ep)
{
  *rslt=PJFry::E0v1(*i,*p1,*p2,*p3,*p4,*p5,*s12,*s23,*s34,*s45,*s15,*m1,*m2,*m3,*m4,*m5,*ep);
}

void pe0ij_(pj_complex *rslt,
            int *i, int *j,
            double *p1,  double *p2,  double *p3,  double *p4,  double *p5,
            double *s12, double *s23, double *s34, double *s45, double *s15,
            double *m1,  double *m2,  double *m3,  double *m4,  double *m5, int *ep)
{
  *rslt=PJFry::E0v2(*i,*j,*p1,*p2,*p3,*p4,*p5,*s12,*s23,*s34,*s45,*s15,*m1,*m2,*m3,*m4,*m5,*ep);
}

void pe0ijk_(pj_complex *rslt,
             int *i, int *j, int *k,
             double *p1,  double *p2,  double *p3,  double *p4,  double *p5,
             double *s12, double *s23, double *s34, double *s45, double *s15,
             double *m1,  double *m2,  double *m3,  double *m4,  double *m5, int *ep)
{
  *rslt=PJFry::E0v3(*i,*j,*k,*p1,*p2,*p3,*p4,*p5,*s12,*s23,*s34,*s45,*s15,*m1,*m2,*m3,*m4,*m5,*ep);
}

void pe0ijkl_(pj_complex *rslt,
              int *i, int *j, int *k, int *l,
              double *p1,  double *p2,  double *p3,  double *p4,  double *p5,
              double *s12, double *s23, double *s34, double *s45, double *s15,
              double *m1,  double *m2,  double *m3,  double *m4,  double *m5, int *ep)
{
  *rslt=PJFry::E0v4(*i,*j,*k,*l,*p1,*p2,*p3,*p4,*p5,*s12,*s23,*s34,*s45,*s15,*m1,*m2,*m3,*m4,*m5,*ep);
}

void pe0ijklm_(pj_complex *rslt,
               int *i, int *j, int *k, int *l, int *m,
               double *p1,  double *p2,  double *p3,  double *p4,  double *p5,
               double *s12, double *s23, double *s34, double *s45, double *s15,
               double *m1,  double *m2,  double *m3,  double *m4,  double *m5, int *ep)
{
  *rslt=PJFry::E0v5(*i,*j,*k,*l,*m,*p1,*p2,*p3,*p4,*p5,*s12,*s23,*s34,*s45,*s15,*m1,*m2,*m3,*m4,*m5,*ep);
}

void pd0_(pj_complex *rslt,
          double *p1,  double *p2,  double *p3,  double *p4,
          double *s12, double *s23,
          double *m1,  double *m2,  double *m3,  double *m4, int *ep)
{
  *rslt=PJFry::D0v0(*p1,*p2,*p3,*p4,*s12,*s23,*m1,*m2,*m3,*m4,*ep);
}

void pd0i_(pj_complex *rslt,
           int *i,
           double *p1,  double *p2,  double *p3,  double *p4,
           double *s12, double *s23,
           double *m1,  double *m2,  double *m3,  double *m4, int *ep)
{
  *rslt=PJFry::D0v1(*i,*p1,*p2,*p3,*p4,*s12,*s23,*m1,*m2,*m3,*m4,*ep);
}

void pd0ij_(pj_complex *rslt,
            int *i, int *j,
            double *p1,  double *p2,  double *p3,  double *p4,
            double *s12, double *s23,
            double *m1,  double *m2,  double *m3,  double *m4, int *ep)
{
  *rslt=PJFry::D0v2(*i,*j,*p1,*p2,*p3,*p4,*s12,*s23,*m1,*m2,*m3,*m4,*ep);
}

void pd0ijk_(pj_complex *rslt,
             int *i, int *j, int *k,
             double *p1,  double *p2,  double *p3,  double *p4,
             double *s12, double *s23,
             double *m1,  double *m2,  double *m3,  double *m4, int *ep)
{
  *rslt=PJFry::D0v3(*i,*j,*k,*p1,*p2,*p3,*p4,*s12,*s23,*m1,*m2,*m3,*m4,*ep);
}

void pd0ijkl_(pj_complex *rslt,
              int *i, int *j, int *k, int *l,
              double *p1,  double *p2,  double *p3,  double *p4,
              double *s12, double *s23,
              double *m1,  double *m2,  double *m3,  double *m4, int *ep)
{
  *rslt=PJFry::D0v4(*i,*j,*k,*l,*p1,*p2,*p3,*p4,*s12,*s23,*m1,*m2,*m3,*m4,*ep);
}

void pc0_(pj_complex *rslt,
          double *p1, double *p2, double *p3,
          double *m1, double *m2, double *m3, int *ep)
{
  *rslt=PJFry::C0v0(*p1,*p2,*p3,*m1,*m2,*m3,*ep);
}

void pc0i_(pj_complex *rslt,
           int *i,
           double *p1, double *p2, double *p3,
           double *m1, double *m2, double *m3, int *ep)
{
  *rslt=PJFry::C0v1(*i,*p1,*p2,*p3,*m1,*m2,*m3,*ep);
}

void pc0ij_(pj_complex *rslt,
            int *i, int *j,
            double *p1, double *p2, double *p3,
            double *m1, double *m2, double *m3, int *ep)
{
  *rslt=PJFry::C0v2(*i,*j,*p1,*p2,*p3,*m1,*m2,*m3,*ep);
}

void pc0ijk_(pj_complex *rslt,
             int *i, int *j, int *k,
             double *p1, double *p2, double *p3,
             double *m1, double *m2, double *m3, int *ep)
{
  *rslt=PJFry::C0v3(*i,*j,*k,*p1,*p2,*p3,*m1,*m2,*m3,*ep);
}

void pb0_(pj_complex *rslt,
          double *p1, double *m1, double *m2, int *ep)
{
  *rslt=PJFry::B0v0(*p1,*m1,*m2,*ep);
}

void pb0i_(pj_complex *rslt,
           int *i,
           double *p1, double *m1, double *m2, int *ep)
{
  *rslt=PJFry::B0v1(*i,*p1,*m1,*m2,*ep);
}

void pb0ij_(pj_complex *rslt,
            int *i, int *j,
            double *p1, double *m1, double *m2, int *ep)
{
  *rslt=PJFry::B0v2(*i,*j,*p1,*m1,*m2,*ep);
}

void pa0_(pj_complex *rslt, double *m1, int *ep)
{
  *rslt=PJFry::A0v0(*m1,*ep);
}
#else
// GNU calling convention section (result is a complex return value)
pj_complex pe0_( double *p1,  double *p2,  double *p3,  double *p4,  double *p5,
               double *s12, double *s23, double *s34, double *s45, double *s15,
               double *m1,  double *m2,  double *m3,  double *m4,  double *m5, int *ep)
{
  return PJFry::E0v0(*p1,*p2,*p3,*p4,*p5,*s12,*s23,*s34,*s45,*s15,*m1,*m2,*m3,*m4,*m5,*ep);
}

pj_complex pe0i_(int *i,
               double *p1,  double *p2,  double *p3,  double *p4,  double *p5,
               double *s12, double *s23, double *s34, double *s45, double *s15,
               double *m1,  double *m2,  double *m3,  double *m4,  double *m5, int *ep)
{
  return PJFry::E0v1(*i,*p1,*p2,*p3,*p4,*p5,*s12,*s23,*s34,*s45,*s15,*m1,*m2,*m3,*m4,*m5,*ep);
}

pj_complex pe0ij_(int *i, int *j,
               double *p1,  double *p2,  double *p3,  double *p4,  double *p5,
               double *s12, double *s23, double *s34, double *s45, double *s15,
               double *m1,  double *m2,  double *m3,  double *m4,  double *m5, int *ep)
{
  return PJFry::E0v2(*i,*j,*p1,*p2,*p3,*p4,*p5,*s12,*s23,*s34,*s45,*s15,*m1,*m2,*m3,*m4,*m5,*ep);
}

pj_complex pe0ijk_(int *i, int *j, int *k,
               double *p1,  double *p2,  double *p3,  double *p4,  double *p5,
               double *s12, double *s23, double *s34, double *s45, double *s15,
               double *m1,  double *m2,  double *m3,  double *m4,  double *m5, int *ep)
{
  return PJFry::E0v3(*i,*j,*k,*p1,*p2,*p3,*p4,*p5,*s12,*s23,*s34,*s45,*s15,*m1,*m2,*m3,*m4,*m5,*ep);
}

pj_complex pe0ijkl_(int *i, int *j, int *k, int *l,
               double *p1,  double *p2,  double *p3,  double *p4,  double *p5,
               double *s12, double *s23, double *s34, double *s45, double *s15,
               double *m1,  double *m2,  double *m3,  double *m4,  double *m5, int *ep)
{
  return PJFry::E0v4(*i,*j,*k,*l,*p1,*p2,*p3,*p4,*p5,*s12,*s23,*s34,*s45,*s15,*m1,*m2,*m3,*m4,*m5,*ep);
}

pj_complex pe0ijklm_(int *i, int *j, int *k, int *l, int *m,
               double *p1,  double *p2,  double *p3,  double *p4,  double *p5,
               double *s12, double *s23, double *s34, double *s45, double *s15,
               double *m1,  double *m2,  double *m3,  double *m4,  double *m5, int *ep)
{
  return PJFry::E0v5(*i,*j,*k,*l,*m,*p1,*p2,*p3,*p4,*p5,*s12,*s23,*s34,*s45,*s15,*m1,*m2,*m3,*m4,*m5,*ep);
}

pj_complex pd0_(double *p1,  double *p2,  double *p3,  double *p4,
              double *s12, double *s23,
              double *m1,  double *m2,  double *m3,  double *m4, int *ep)
{
  return PJFry::D0v0(*p1,*p2,*p3,*p4,*s12,*s23,*m1,*m2,*m3,*m4,*ep);
}

pj_complex pd0i_(int *i,
               double *p1,  double *p2,  double *p3,  double *p4,
               double *s12, double *s23,
               double *m1,  double *m2,  double *m3,  double *m4, int *ep)
{
  return PJFry::D0v1(*i,*p1,*p2,*p3,*p4,*s12,*s23,*m1,*m2,*m3,*m4,*ep);
}

pj_complex pd0ij_(int *i, int *j,
               double *p1,  double *p2,  double *p3,  double *p4,
               double *s12, double *s23,
               double *m1,  double *m2,  double *m3,  double *m4, int *ep)
{
  return PJFry::D0v2(*i,*j,*p1,*p2,*p3,*p4,*s12,*s23,*m1,*m2,*m3,*m4,*ep);
}

pj_complex pd0ijk_(int *i, int *j, int *k,
               double *p1,  double *p2,  double *p3,  double *p4,
               double *s12, double *s23,
               double *m1,  double *m2,  double *m3,  double *m4, int *ep)
{
  return PJFry::D0v3(*i,*j,*k,*p1,*p2,*p3,*p4,*s12,*s23,*m1,*m2,*m3,*m4,*ep);
}

pj_complex pd0ijkl_(int *i, int *j, int *k, int *l,
               double *p1,  double *p2,  double *p3,  double *p4,
               double *s12, double *s23,
               double *m1,  double *m2,  double *m3,  double *m4, int *ep)
{
  return PJFry::D0v4(*i,*j,*k,*l,*p1,*p2,*p3,*p4,*s12,*s23,*m1,*m2,*m3,*m4,*ep);
}

pj_complex pc0_(double *p1, double *p2, double *p3,
              double *m1, double *m2, double *m3, int *ep)
{
  return PJFry::C0v0(*p1,*p2,*p3,*m1,*m2,*m3,*ep);
}

pj_complex pc0i_(int *i,
               double *p1, double *p2, double *p3,
               double *m1, double *m2, double *m3, int *ep)
{
  return PJFry::C0v1(*i,*p1,*p2,*p3,*m1,*m2,*m3,*ep);
}

pj_complex pc0ij_(int *i, int *j,
               double *p1, double *p2, double *p3,
               double *m1, double *m2, double *m3, int *ep)
{
  return PJFry::C0v2(*i,*j,*p1,*p2,*p3,*m1,*m2,*m3,*ep);
}

pj_complex pc0ijk_(int *i, int *j, int *k,
               double *p1, double *p2, double *p3,
               double *m1, double *m2, double *m3, int *ep)
{
  return PJFry::C0v3(*i,*j,*k,*p1,*p2,*p3,*m1,*m2,*m3,*ep);
}

pj_complex pb0_( double *p1, double *m1, double *m2, int *ep)
{
  return PJFry::B0v0(*p1,*m1,*m2,*ep);
}

pj_complex pb0i_(int *i,
               double *p1, double *m1, double *m2, int *ep)
{
  return PJFry::B0v1(*i,*p1,*m1,*m2,*ep);
}

pj_complex pb0ij_(int *i, int *j,
               double *p1, double *m1, double *m2, int *ep)
{
  return PJFry::B0v2(*i,*j,*p1,*m1,*m2,*ep);
}

pj_complex pa0_( double *m1, int *ep)
{
  return PJFry::A0v0(*m1,*ep);
}
#endif
