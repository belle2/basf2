/*
 * pjfry.h - interface header
 *
 * this file is part of PJFry library
 * Copyright 2011 Valery Yundin
 */

#ifndef QUL_PJFRY_H
#define QUL_PJFRY_H

/* if language is C++ use standard complex template type */
#ifdef __cplusplus
#   include <complex>
#   define QUL_DEFINE_COMPLEX(R, C) typedef std::complex<R> C
#else
/* if <complex.h> is included, use the C99 complex type.
 * else define a type bit-compatible with C99 complex */
#   if defined(_Complex_I) && defined(complex) && defined(I)
#       define QUL_DEFINE_COMPLEX(R, C) typedef R _Complex C
#   else
#       define QUL_DEFINE_COMPLEX(R, C) typedef struct { R re,im; } C
#   endif
#endif

QUL_DEFINE_COMPLEX(double, pj_complex);

#ifdef __cplusplus
namespace PJFry {
  // Functions without explicit epsilon default to eps=0
  pj_complex E0v0(double p1,  double p2,  double p3,  double p4,  double p5,
                  double s12, double s23, double s34, double s45, double s15,
                  double m1,  double m2,  double m3,  double m4,  double m5);
  pj_complex E0v1(int i,
                  double p1,  double p2,  double p3,  double p4,  double p5,
                  double s12, double s23, double s34, double s45, double s15,
                  double m1,  double m2,  double m3,  double m4,  double m5);
  pj_complex E0v2(int i, int j,
                  double p1,  double p2,  double p3,  double p4,  double p5,
                  double s12, double s23, double s34, double s45, double s15,
                  double m1,  double m2,  double m3,  double m4,  double m5);
  pj_complex E0v3(int i, int j, int k,
                  double p1,  double p2,  double p3,  double p4,  double p5,
                  double s12, double s23, double s34, double s45, double s15,
                  double m1,  double m2,  double m3,  double m4,  double m5);
  pj_complex E0v4(int i, int j, int k, int l,
                  double p1,  double p2,  double p3,  double p4,  double p5,
                  double s12, double s23, double s34, double s45, double s15,
                  double m1,  double m2,  double m3,  double m4,  double m5);
  pj_complex E0v5(int i, int j, int k, int l, int m,
                  double p1,  double p2,  double p3,  double p4,  double p5,
                  double s12, double s23, double s34, double s45, double s15,
                  double m1,  double m2,  double m3,  double m4,  double m5);

  pj_complex E0v0(double p1,  double p2,  double p3,  double p4,  double p5,
                  double s12, double s23, double s34, double s45, double s15,
                  double m1,  double m2,  double m3,  double m4,  double m5, int ep);
  pj_complex E0v1(int i,
                  double p1,  double p2,  double p3,  double p4,  double p5,
                  double s12, double s23, double s34, double s45, double s15,
                  double m1,  double m2,  double m3,  double m4,  double m5, int ep);
  pj_complex E0v2(int i, int j,
                  double p1,  double p2,  double p3,  double p4,  double p5,
                  double s12, double s23, double s34, double s45, double s15,
                  double m1,  double m2,  double m3,  double m4,  double m5, int ep);
  pj_complex E0v3(int i, int j, int k,
                  double p1,  double p2,  double p3,  double p4,  double p5,
                  double s12, double s23, double s34, double s45, double s15,
                  double m1,  double m2,  double m3,  double m4,  double m5, int ep);
  pj_complex E0v4(int i, int j, int k, int l,
                  double p1,  double p2,  double p3,  double p4,  double p5,
                  double s12, double s23, double s34, double s45, double s15,
                  double m1,  double m2,  double m3,  double m4,  double m5, int ep);
  pj_complex E0v5(int i, int j, int k, int l, int m,
                  double p1,  double p2,  double p3,  double p4,  double p5,
                  double s12, double s23, double s34, double s45, double s15,
                  double m1,  double m2,  double m3,  double m4,  double m5, int ep);


  pj_complex D0v0(double p1,  double p2,  double p3,  double p4,
                  double s12, double s23,
                  double m1,  double m2,  double m3,  double m4);
  pj_complex D0v1(int i,
                  double p1,  double p2,  double p3,  double p4,
                  double s12, double s23,
                  double m1,  double m2,  double m3,  double m4);
  pj_complex D0v2(int i, int j,
                  double p1,  double p2,  double p3,  double p4,
                  double s12, double s23,
                  double m1,  double m2,  double m3,  double m4);
  pj_complex D0v3(int i, int j, int k,
                  double p1,  double p2,  double p3,  double p4,
                  double s12, double s23,
                  double m1,  double m2,  double m3,  double m4);
  pj_complex D0v4(int i, int j, int k, int l,
                  double p1,  double p2,  double p3,  double p4,
                  double s12, double s23,
                  double m1,  double m2,  double m3,  double m4);

  pj_complex D0v0(double p1,  double p2,  double p3,  double p4,
                  double s12, double s23,
                  double m1,  double m2,  double m3,  double m4, int ep);
  pj_complex D0v1(int i,
                  double p1,  double p2,  double p3,  double p4,
                  double s12, double s23,
                  double m1,  double m2,  double m3,  double m4, int ep);
  pj_complex D0v2(int i, int j,
                  double p1,  double p2,  double p3,  double p4,
                  double s12, double s23,
                  double m1,  double m2,  double m3,  double m4, int ep);
  pj_complex D0v3(int i, int j, int k,
                  double p1,  double p2,  double p3,  double p4,
                  double s12, double s23,
                  double m1,  double m2,  double m3,  double m4, int ep);
  pj_complex D0v4(int i, int j, int k, int l,
                  double p1,  double p2,  double p3,  double p4,
                  double s12, double s23,
                  double m1,  double m2,  double m3,  double m4, int ep);


  pj_complex C0v0(double p1, double p2, double p3,
                  double m1, double m2, double m3);
  pj_complex C0v1(int i,
                  double p1, double p2, double p3,
                  double m1, double m2, double m3);
  pj_complex C0v2(int i, int j,
                  double p1, double p2, double p3,
                  double m1, double m2, double m3);
  pj_complex C0v3(int i, int j, int k,
                  double p1, double p2, double p3,
                  double m1, double m2, double m3);

  pj_complex C0v0(double p1, double p2, double p3,
                  double m1, double m2, double m3, int ep);
  pj_complex C0v1(int i,
                  double p1, double p2, double p3,
                  double m1, double m2, double m3, int ep);
  pj_complex C0v2(int i, int j,
                  double p1, double p2, double p3,
                  double m1, double m2, double m3, int ep);
  pj_complex C0v3(int i, int j, int k,
                  double p1, double p2, double p3,
                  double m1, double m2, double m3, int ep);


  pj_complex B0v0(double p1, double m1, double m2);
  pj_complex B0v1(int i,
                  double p1, double m1, double m2);
  pj_complex B0v2(int i, int j,
                  double p1, double m1, double m2);
  pj_complex B0v3(int i, int j, int k,
                  double p1, double m1, double m2);

  pj_complex B0v0(double p1, double m1, double m2, int ep);
  pj_complex B0v1(int i,
                  double p1, double m1, double m2, int ep);
  pj_complex B0v2(int i, int j,
                  double p1, double m1, double m2, int ep);
  pj_complex B0v3(int i, int j, int k,
                  double p1, double m1, double m2, int ep);


  pj_complex A0v0(double m1);
  pj_complex A0v0(double m1, int ep);

  double GetMu2();
  double SetMu2(double newmu2);
  void ClearCache();
}
#endif /* __cplusplus */


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

double pgetmusq_();
void psetmusq_(double* mu2);
void pclearcache_();

#ifdef USE_F2C
// f2c,g77,ifort calling convention section (result is the first parameter)
void pa0_(pj_complex* rslt, double* m1, int* ep);

void pb0_(pj_complex* rslt, double* p1, double* m1, double* m2, int* ep);
void pb0i_(pj_complex* rslt, int* i,
           double* p1, double* m1, double* m2, int* ep);
void pb0ij_(pj_complex* rslt, int* i, int* j,
            double* p1, double* m1, double* m2, int* ep);

void pc0_(pj_complex* rslt, double* p1, double* p2, double* p3, double* m1, double* m2, double* m3, int* ep);
void pc0i_(pj_complex* rslt, int* i,
           double* p1, double* p2, double* p3, double* m1, double* m2, double* m3, int* ep);
void pc0ij_(pj_complex* rslt, int* i, int* j,
            double* p1, double* p2, double* p3, double* m1, double* m2, double* m3, int* ep);
void pc0ijk_(pj_complex* rslt, int* i, int* j, int* k,
             double* p1, double* p2, double* p3, double* m1, double* m2, double* m3, int* ep);

void pd0_(pj_complex* rslt, double* p1, double* p2, double* p3, double* p4, double* s12, double* s23, double* m1, double* m2,
          double* m3, double* m4, int* ep);
void pd0i_(pj_complex* rslt, int* i,
           double* p1, double* p2, double* p3, double* p4, double* s12, double* s23, double* m1, double* m2, double* m3, double* m4, int* ep);
void pd0ij_(pj_complex* rslt, int* i, int* j,
            double* p1, double* p2, double* p3, double* p4, double* s12, double* s23, double* m1, double* m2, double* m3, double* m4, int* ep);
void pd0ijk_(pj_complex* rslt, int* i, int* j, int* k,
             double* p1, double* p2, double* p3, double* p4, double* s12, double* s23, double* m1, double* m2, double* m3, double* m4, int* ep);
void pd0ijkl_(pj_complex* rslt, int* i, int* j, int* k, int* l,
              double* p1, double* p2, double* p3, double* p4, double* s12, double* s23, double* m1, double* m2, double* m3, double* m4, int* ep);

void pe0_(pj_complex* rslt, double* p1, double* p2, double* p3, double* p4, double* p5, double* s12, double* s23, double* s34,
          double* s45, double* s15, double* m1, double* m2, double* m3, double* m4, double* m5, int* ep);
void pe0i_(pj_complex* rslt, int* i,
           double* p1, double* p2, double* p3, double* p4, double* p5, double* s12, double* s23, double* s34, double* s45, double* s15,
           double* m1, double* m2, double* m3, double* m4, double* m5, int* ep);
void pe0ij_(pj_complex* rslt, int* i, int* j,
            double* p1, double* p2, double* p3, double* p4, double* p5, double* s12, double* s23, double* s34, double* s45, double* s15,
            double* m1, double* m2, double* m3, double* m4, double* m5, int* ep);
void pe0ijk_(pj_complex* rslt, int* i, int* j, int* k,
             double* p1, double* p2, double* p3, double* p4, double* p5, double* s12, double* s23, double* s34, double* s45, double* s15,
             double* m1, double* m2, double* m3, double* m4, double* m5, int* ep);
void pe0ijkl_(pj_complex* rslt, int* i, int* j, int* k, int* l,
              double* p1, double* p2, double* p3, double* p4, double* p5, double* s12, double* s23, double* s34, double* s45, double* s15,
              double* m1, double* m2, double* m3, double* m4, double* m5, int* ep);
void pe0ijklm_(pj_complex* rslt, int* i, int* j, int* k, int* l, int* m,
               double* p1, double* p2, double* p3, double* p4, double* p5, double* s12, double* s23, double* s34, double* s45, double* s15,
               double* m1, double* m2, double* m3, double* m4, double* m5, int* ep);
#else
// GNU calling convention section (result is a complex return value)
pj_complex pa0_(double* m1, int* ep);

pj_complex pb0_(double* p1, double* m1, double* m2, int* ep);
pj_complex pb0i_(int* i,
                 double* p1, double* m1, double* m2, int* ep);
pj_complex pb0ij_(int* i, int* j,
                  double* p1, double* m1, double* m2, int* ep);

pj_complex pc0_(double* p1, double* p2, double* p3, double* m1, double* m2, double* m3, int* ep);
pj_complex pc0i_(int* i,
                 double* p1, double* p2, double* p3, double* m1, double* m2, double* m3, int* ep);
pj_complex pc0ij_(int* i, int* j,
                  double* p1, double* p2, double* p3, double* m1, double* m2, double* m3, int* ep);
pj_complex pc0ijk_(int* i, int* j, int* k,
                   double* p1, double* p2, double* p3, double* m1, double* m2, double* m3, int* ep);

pj_complex pd0_(double* p1, double* p2, double* p3, double* p4, double* s12, double* s23, double* m1, double* m2, double* m3,
                double* m4, int* ep);
pj_complex pd0i_(int* i,
                 double* p1, double* p2, double* p3, double* p4, double* s12, double* s23, double* m1, double* m2, double* m3, double* m4, int* ep);
pj_complex pd0ij_(int* i, int* j,
                  double* p1, double* p2, double* p3, double* p4, double* s12, double* s23, double* m1, double* m2, double* m3, double* m4, int* ep);
pj_complex pd0ijk_(int* i, int* j, int* k,
                   double* p1, double* p2, double* p3, double* p4, double* s12, double* s23, double* m1, double* m2, double* m3, double* m4, int* ep);
pj_complex pd0ijkl_(int* i, int* j, int* k, int* l,
                    double* p1, double* p2, double* p3, double* p4, double* s12, double* s23, double* m1, double* m2, double* m3, double* m4, int* ep);

pj_complex pe0_(double* p1, double* p2, double* p3, double* p4, double* p5, double* s12, double* s23, double* s34, double* s45,
                double* s15, double* m1, double* m2, double* m3, double* m4, double* m5, int* ep);
pj_complex pe0i_(int* i,
                 double* p1, double* p2, double* p3, double* p4, double* p5, double* s12, double* s23, double* s34, double* s45, double* s15,
                 double* m1, double* m2, double* m3, double* m4, double* m5, int* ep);
pj_complex pe0ij_(int* i, int* j,
                  double* p1, double* p2, double* p3, double* p4, double* p5, double* s12, double* s23, double* s34, double* s45, double* s15,
                  double* m1, double* m2, double* m3, double* m4, double* m5, int* ep);
pj_complex pe0ijk_(int* i, int* j, int* k,
                   double* p1, double* p2, double* p3, double* p4, double* p5, double* s12, double* s23, double* s34, double* s45, double* s15,
                   double* m1, double* m2, double* m3, double* m4, double* m5, int* ep);
pj_complex pe0ijkl_(int* i, int* j, int* k, int* l,
                    double* p1, double* p2, double* p3, double* p4, double* p5, double* s12, double* s23, double* s34, double* s45, double* s15,
                    double* m1, double* m2, double* m3, double* m4, double* m5, int* ep);
pj_complex pe0ijklm_(int* i, int* j, int* k, int* l, int* m,
                     double* p1, double* p2, double* p3, double* p4, double* p5, double* s12, double* s23, double* s34, double* s45, double* s15,
                     double* m1, double* m2, double* m3, double* m4, double* m5, int* ep);
#endif

#ifdef USE_GOLEM_MODE
double pggetmusq_();
void pgsetmusq_(double* mu2);

void pginitgolem95_(int* n);
double pggetmat_(int* i, int* j);
void pgpreparesmatrix_();
void pgsetmat_(int* i, int* j, double* val);

//  5 point base
pj_complex pga50_(int* s, int* ep);
pj_complex pga51_(int* i, int* s, int* ep);
pj_complex pga52_(int* i, int* j, int* s, int* ep);
pj_complex pgb52_(int* s, int* ep);
pj_complex pga53_(int* i, int* j, int* k, int* s, int* ep);
pj_complex pgb53_(int* i, int* s, int* ep);
pj_complex pga54_(int* i, int* j, int* k, int* l, int* s, int* ep);
pj_complex pgb54_(int* i, int* j, int* s, int* ep);
pj_complex pgc54_(int* s, int* ep);
pj_complex pga55_(int* i, int* j, int* k, int* l, int* m, int* s, int* ep);
pj_complex pgb55_(int* i, int* j, int* k, int* s, int* ep);
pj_complex pgc55_(int* i, int* s, int* ep);

pj_complex pga40_(int* s, int* ep);
pj_complex pga41_(int* i, int* s, int* ep);
pj_complex pga42_(int* i, int* j, int* s, int* ep);
pj_complex pgb42_(int* s, int* ep);
pj_complex pga43_(int* i, int* j, int* k, int* s, int* ep);
pj_complex pgb43_(int* i, int* s, int* ep);
pj_complex pga44_(int* i, int* j, int* k, int* l, int* s, int* ep);
pj_complex pgb44_(int* i, int* j, int* s, int* ep);
pj_complex pgc44_(int* s, int* ep);

pj_complex pga30_(int* s, int* ep);
pj_complex pga31_(int* i, int* s, int* ep);
pj_complex pga32_(int* i, int* j, int* s, int* ep);
pj_complex pgb32_(int* s, int* ep);
pj_complex pga33_(int* i, int* j, int* k, int* s, int* ep);
pj_complex pgb33_(int* i, int* s, int* ep);

pj_complex pga20_(int* s, int* ep);
pj_complex pga21_(int* i, int* s, int* ep);
pj_complex pga22_(int* i, int* j, int* s, int* ep);
pj_complex pgb22_(int* s, int* ep);

#endif /* USE_GOLEM_MODE */

#ifdef __cplusplus
}
#endif

#endif /* QUL_PJFRY_H */
