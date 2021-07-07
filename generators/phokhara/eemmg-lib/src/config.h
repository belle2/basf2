
/* Define to a macro mangling the given C identifier (in lower and upper
   case), which must not contain underscores, for linking with Fortran. */
#define F77_FUNC(name,NAME) name ## _

/* As F77_FUNC, but for C identifiers containing underscores. */
#define F77_FUNC_(name,NAME) name ## _

/* Define to 1 if you have the <qcdloop1.h> header file. */
/* #undef HAVE_QCDLOOP1_H */

/* "enable cache for 5 and 6-point coefficients" */
#define USE_CACHE_HIGH "1"

/* "enable cache for 2- */
/* #undef USE_CACHE_LOW */

/* "Use g77 and ifort calling conventions for complex return values" */
#define USE_F2C "1"

/* "build golem95-like interface" */
/* #undef USE_GOLEM_MODE */

/* use libavh_olo for scalar integrals */
/* #undef USE_ONELOOP */

/* use libqcdloop for scalar integrals */
#define USE_QCDLOOP "1"

