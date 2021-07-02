/*
 * qcdloop1.h - header file for QCDLoop1 library
 *
 */

#ifndef QCDLOOP1_H
#define QCDLOOP1_H

/* NB endline spaces are significant */
#ifdef QL_BUILDSYMS
#   define QL_EXPORT(type) QL_EXPORT_MARK
#else
#   define QL_EXPORT(type) type
#endif

/* if language is C++ use standard complex template type */
#ifdef __cplusplus
#   include <complex>
#   define QL_DEFINE_COMPLEX(R, C) typedef std::complex<R> C
#else
/* if <complex.h> is included, use the C99 complex type.
 * else define a type bit-compatible with C99 complex */
#   if defined(_Complex_I) && defined(complex) && defined(I)
#       define QL_DEFINE_COMPLEX(R, C) typedef R _Complex C
#   else
#       define QL_DEFINE_COMPLEX(R, C) typedef struct { R re,im; } C
#   endif
#endif

QL_DEFINE_COMPLEX(double, ql_cmplx_dbl);

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct ffprec_struct {
  double xloss;
  double precx;
  double precc;
  double xalogm;
  double xclogm;
  double xalog2;
  double xclog2;
  double reqprc;
} ffprec_type;

typedef struct ffflag_struct {
  int lwrite;
  int ltest;
  int l4also;
  int ldc3c4;
  int lmem;
  int lwarn;
  int ldot;
  int nevent;
  int ner;
  int id;
  int idsub;
  int nwidth;
  int nschem;
  int onshel;
  int idot;
} ffflag_type;

QL_EXPORT(void) qlinit_();
QL_EXPORT(void) ffexi_();

#ifdef USE_F2C
QL_EXPORT(void) qli1_(ql_cmplx_dbl* rslt, double* m1, double* mu2, int* ep);
QL_EXPORT(void) qli2_(ql_cmplx_dbl* rslt, double* p1, double* m1, double* m2, double* mu2, int* ep);
QL_EXPORT(void) qli3_(ql_cmplx_dbl* rslt, double* p1, double* p2, double* p3, double* m1, double* m2, double* m3, double* mu2,
                      int* ep);
QL_EXPORT(void) qli4_(ql_cmplx_dbl* rslt, double* p1, double* p2, double* p3, double* p4, double* s12, double* s23, double* m1,
                      double* m2, double* m3, double* m4, double* mu2, int* ep);
#else
QL_EXPORT(ql_cmplx_dbl) qli1_(double* m1, double* mu2, int* ep);
QL_EXPORT(ql_cmplx_dbl) qli2_(double* p1, double* m1, double* m2, double* mu2, int* ep);
QL_EXPORT(ql_cmplx_dbl) qli3_(double* p1, double* p2, double* p3, double* m1, double* m2, double* m3, double* mu2, int* ep);
QL_EXPORT(ql_cmplx_dbl) qli4_(double* p1, double* p2, double* p3, double* p4, double* s12, double* s23, double* m1, double* m2,
                              double* m3, double* m4, double* mu2, int* ep);
#endif

extern QL_EXPORT(struct ffprec_struct) ffprec_;
extern QL_EXPORT(struct ffflag_struct) ffflag_;

#ifdef __cplusplus
}
#endif

#endif /* QCDLOOP1_H */
