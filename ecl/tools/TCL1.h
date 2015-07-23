// @(#)root/star:$Name:  $:$Id: TCL.h,v 1.2 2001/05/29 19:08:08 brun Exp $
// Author: Valery Fine(fine@bnl.gov)   25/09/99

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/
#ifndef ROOT_TCL
#define ROOT_TCL
//
// The set of methods to work with the plain matrix / vector
// "derived" from  http://wwwinfo.cern.ch/asdoc/shortwrupsdir/f110/top.html
//
// $Id: TCL.h,v 1.2 2001/05/29 19:08:08 brun Exp $
// $Log: TCL.h,v $
// Revision 1.2  2001/05/29 19:08:08  brun
// New version of some STAR classes from Valery.
//
// Revision 1.2  2001/05/27 02:38:10  fine
// New method trsedu to solev Ax=B from Victor
//
// Revision 1.1.1.1  2000/11/27 22:57:13  fisyak
//
//
// Revision 1.1.1.1  2000/05/16 17:00:49  rdm
// Initial import of ROOT into CVS
//

#include "Rtypes.h"
#include <string.h>

// http://wwwinfo.cern.ch/asdoc/shortwrupsdir/f110/top.html

///////////////////////////////////////////////////////////////////////////////////////
//                                                                                   //
// The routines of MXPACK compute the product of two matrices or the product of      //
// their transposed matrices and may add or subtract to the resultant matrix         //
// a third one, add or subtract one matrix from another, or transfer a matrix,       //
// its negative, or a multiple of it, transpose a given matrix, build up a unit      //
// matrix, multiply a matrix by a diagonal (from left or from right) and may         //
// add the result to another matrix, add to square matrix the multiple of a diagonal //
// matrix, compute the products <IMG WIDTH=79 HEIGHT=12 ALIGN=BOTTOM ALT="tex2html_wrap_inline191" SRC="gif/mxpack_ABAt.gif"> (<IMG WIDTH=16 HEIGHT=12 ALIGN=BOTTOM ALT="tex2html_wrap_inline193" SRC="gif/mxpack_At.gif"> denotes the transpose of <IMG WIDTH=1
// It is assumed that matrices are begin_html <B>row-wise without gaps</B> end_html without gaps.                     //
//                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////

class TArrayD;

class TCL  {
public:

  static int*    ucopy(const int*    a, int*    b, int n);
  static float*  ucopy(const float*  a, float*  b, int n);
  static double* ucopy(const float*  a, double* b, int n);
  static float*  ucopy(const double* a, float*  b, int n);
  static double* ucopy(const double* a, double* b, int n);
  static void**  ucopy(const void** a, void**  b, int n);

  static float*  vzero(float* a,  int n2);
  static double* vzero(double* a, int n2);
  static void**  vzero(void** a,  int n2);

  static float*  vadd(const float* b,  const float*  c,  float* a, int n);
  static double* vadd(const double* b, const double* c, double* a, int n);

  static float*  vadd(const float* b,  const double* c, float* a, int n);
  static double* vadd(const double* b, const float*  c, double* a, int n);

  static float   vdot(const float*  b, const float*  a, int n);
  static double  vdot(const double* b, const double* a, int n);

  static float*  vsub(const float* a, const float*  b, float* x, int n);
  static double* vsub(const double* a, const double* b, double* x, int n);

  static float*  vcopyn(const float* a,  float* x, int n);
  static double* vcopyn(const double* a, double* x, int n);

  static float*  vscale(const float*  a, float  scale, float*  b, int n);
  static double* vscale(const double* a, double scale, double* b, int n);

  static float*  vlinco(const float*  a, float  fa, const float*  b, float  fb, float*  x, int n);
  static double* vlinco(const double* a, double fa, const double* b, double fb, double* x, int n);

  static float*  vmatl(const float*  g, const float*  c, float*  x, int n = 3, int m = 3);
  static double* vmatl(const double* g, const double* c, double* x, int n = 3, int m = 3);

  static float*  vmatr(const float*  c, const float*  g, float*  x, int n = 3, int m = 3);
  static double* vmatr(const double* c, const double* g, double* x, int n = 3, int m = 3);

  static float* mxmad_0_(int n, const float* a, const float* b, float* c, int i, int j, int k);

  static float* mxmad(const float* a, const float* b, float* c, int i, int j, int k);
  static float* mxmad1(const float* a, const float* b, float* c, int i, int j, int k);
  static float* mxmad2(const float* a, const float* b, float* c, int i, int j, int k);
  static float* mxmad3(const float* a, const float* b, float* c, int i, int j, int k);
  static float* mxmpy(const float* a, const float* b, float* c, int i, int j, int k);
  static float* mxmpy1(const float* a, const float* b, float* c, int i, int j, int k);
  static float* mxmpy2(const float* a, const float* b, float* c, int i, int j, int k);
  static float* mxmpy3(const float* a, const float* b, float* c, int i, int j, int k);
  static float* mxmub(const float* a, const float* b, float* c, int i, int j, int k);
  static float* mxmub1(const float* a, const float* b, float* c, int i, int j, int k);
  static float* mxmub2(const float* a, const float* b, float* c, int i, int j, int k);
  static float* mxmub3(const float* a, const float* b, float* c, int i, int j, int k);

  static float* mxmlrt_0_(int n__, const float* a, const float* b, float* c, int ni, int nj);
  static float* mxmlrt(const float* a, const float* b, float* c, int ni, int nj);
  static float* mxmltr(const float* a, const float* b, float* c, int ni, int nj);
  static float* mxtrp(const float* a, float* b, int i, int j);

  static double* mxmad_0_(int n, const double* a, const double* b, double* c, int i, int j, int k);

  static double* mxmad(const double* a, const double* b, double* c, int i, int j, int k);
  static double* mxmad1(const double* a, const double* b, double* c, int i, int j, int k);
  static double* mxmad2(const double* a, const double* b, double* c, int i, int j, int k);
  static double* mxmad3(const double* a, const double* b, double* c, int i, int j, int k);
  static double* mxmpy(const double* a, const double* b, double* c, int i, int j, int k);
  static double* mxmpy1(const double* a, const double* b, double* c, int i, int j, int k);
  static double* mxmpy2(const double* a, const double* b, double* c, int i, int j, int k);
  static double* mxmpy3(const double* a, const double* b, double* c, int i, int j, int k);
  static double* mxmub(const double* a, const double* b, double* c, int i, int j, int k);
  static double* mxmub1(const double* a, const double* b, double* c, int i, int j, int k);
  static double* mxmub2(const double* a, const double* b, double* c, int i, int j, int k);
  static double* mxmub3(const double* a, const double* b, double* c, int i, int j, int k);

  static double* mxmlrt_0_(int n__, const double* a, const double* b, double* c, int ni, int nj);
  static double* mxmlrt(const double* a, const double* b, double* c, int ni, int nj);
  static double* mxmltr(const double* a, const double* b, double* c, int ni, int nj);
  static double* mxtrp(const double* a, double* b, int i, int j);

// * TR pack

  static float* traat(float* a, float* s, int m, int n);
  static float* tral(float* a, float* u, float* b, int m, int n);
  static float* tralt(float* a, float* u, float* b, int m, int n);
  static float* tras(float* a, float* s, float* b, int m, int n);
  static float* trasat(float* a, float* s, float* r__, int m, int n);
  static float* trasat(double* a, float* s, float* r__, int m, int n);
  static float* trata(float* a, float* r__, int m, int n);
  static float* trats(float* a, float* s, float* b, int m, int n);
  static float* tratsa(float* a, float* s, float* r__, int m, int n);
  //    static float *trchlu(float *a, float *b, int n);
  //    static float *trchul(float *a, float *b, int n);
  //    static float *trinv(float *t, float *s, int n);
  static float* trla(float* u, float* a, float* b, int m, int n);
  static float* trlta(float* u, float* a, float* b, int m, int n);
  //    static float *trpck(float *s, float *u, int n);
  static float* trqsq(float* q, float* s, float* r__, int m);
  static float* trsa(float* s, float* a, float* b, int m, int n);
  //    static float *trsinv(float *g, float *gi, int n);
  //    static float *trsmlu(float *u, float *s, int n);
  //    static float *trsmul(float *g, float *gi, int n);
  //    static float *trupck(float *u, float *s, int m);
  static float* trsat(float* s, float* a, float* b, int m, int n);

// Victor Perevoztchikov's addition:
  static float* trsequ(float* smx, int m = 3, float* b = 0, int n = 1);

// ---   double version

  static double* traat(double* a, double* s, int m, int n);
  static double* tral(double* a, double* u, double* b, int m, int n);
  static double* tralt(double* a, double* u, double* b, int m, int n);
  static double* tras(double* a, double* s, double* b, int m, int n);
  static double* trasat(double* a, double* s, double* r__, int m, int n);
  static double* trata(double* a, double* r__, int m, int n);
  static double* trats(double* a, double* s, double* b, int m, int n);
  static double* tratsa(double* a, double* s, double* r__, int m, int n);
  static double* trchlu(double* a, double* b, int n);
  static double* trchul(double* a, double* b, int n);
  static double* trinv(double* t, double* s, int n);
  static double* trla(double* u, double* a, double* b, int m, int n);
  static double* trlta(double* u, double* a, double* b, int m, int n);
  static double* trpck(double* s, double* u, int n);
  static double* trqsq(double* q, double* s, double* r__, int m);
  static double* trsa(double* s, double* a, double* b, int m, int n);
  static double* trsinv(double* g, double* gi, int n);
  static double* trsmlu(double* u, double* s, int n);
  static double* trsmul(double* g, double* gi, int n);
  static double* trupck(double* u, double* s, int m);
  static double* trsat(double* s, double* a, double* b, int m, int n);

//  Victor Perevoztchikov's addition:
  static double* trsequ(double* smx, int m = 3, double* b = 0, int n = 1);

  ClassDef(TCL, 0) //C++ replacement for CERNLIB matrix / triangle matrix packages: F110 and F112

};
//// Alexander Bobrov addition:


double* TCL::trpck(double* s, double* u, int n)
{
  // trpck.F -- translated by f2c (version 19970219).
  // CERN PROGLIB# F112    TRPCK           .VERSION KERNFOR  2.08  741218 */
  // ORIG. 18/12/74 WH */
//

  //see original documentation of CERNLIB package F112


  int i__, ia, ind, ipiv;

  // Parameter adjuTments
  //   --u;    --s;

  // Function Body
  ia = -1;
  ind = -1;
  ipiv = -1;

  for (i__ = 0; i__ < n; ++i__) {
    //   for (i__ = 1; i__ < =n; ++i__) {
    ipiv += i__ + 1;
    do {
      ++ia;
      ++ind;
      //  printf("s[%d]= %f i__=%d ia=%d ind=%d ipiv=%d \n",ia,s[ia],i__,ia,ind,ipiv);
      u[ind] = s[ia];

    } while (ind < ipiv);
    ia = ia + n - i__ - 1;
  }

  return 0;
} /* trpck_ */


/* Subroutine */

double* TCL::trsinv(double* g, double* gi, int n)
{
  // trsinv.F -- translated by f2c (version 19970219).
  // CERN PROGLIB# F112    TRSINV          .VERSION KERNFOR  2.08  741218
  // ORIG. 18/12/74 WH
//

  //see original documentation of CERNLIB package F112



  // Function Body

  int n2;
  int i;
  n2 = (n * (n + 1)) / 2;

  /*
  for(i=0;i<n2;i++){
         printf("input gi[%d]=%f g[%d]=%f \n",i,gi[i],i,g[i]);
  }

  */
  trchlu(g, gi, n);
  //   n2=n*n;
  /*
  for(i=0;i<n2;i++){
                  printf("trchlu gi[%d]=%f g[%d]=%f \n",i,gi[i],i,g[i]);
  }
  */
  trinv(gi, gi, n);

  //   n2=n*n;
  /*
  for(i=0;i<n2;i++){
    //  printf("trinv gi[%d]=%f g[%d]=%f \n",i,gi[i],i,g[i]);

  }

  */
  trsmul(gi, gi, n);

  // n2=n*n;
  /*
  for(i=0;i<n2;i++){
        printf("trsmul gi[%d]=%f g[%d]=%f \n",i,gi[i],i,g[i]);
  }
  */
  return 0;
} // trsinv_


/*
 float *TCL::trpck( float *s, float *u, int n)
{
   // trpck.F -- translated by f2c (version 19970219).
   // CERN PROGLIB# F112    TRPCK           .VERSION KERNFOR  2.08  741218
   // ORIG. 18/12/74 WH
 //

  //see original documentation of CERNLIB package F112


   int i__, ia, ind, ipiv;

   // Parameter adjuTments
   //   --u;    --s;

   // Function Body
   ia = 0;
   ind = 0;
   ipiv = 0;

   for (i__ = 1; i__ <= n; ++i__) {
      ipiv += i__;
      do {
         ++ia;
         ++ind;
         u[ind] = s[ia];
      } while (ind < ipiv);
      ia = ia + n - i__;
   }

   ++u;
   return u;
} // trpck_

*/


/* Subroutine */double* TCL::trupck(double* u, double* s, int m)
{
  // trupck.F -- translated by f2c (version 19970219).
  // CERN PROGLIB# F112    TRUPCK          .VERSION KERNFOR  2.08  741218
  // ORIG. 18/12/74 WH
//

  //see original documentation of CERNLIB package F112




  int i__, im, is, iu, iv, ih, m2;

  // Parameter adjuTments
  //   --s;    --u;

  // Function Body
  m2 = m * m;
  is = m2;
  iu = (m2 + m) / 2;
  i__ = m - 1;

  do {
    im = i__ * m;
    do {
      //  printf("u[%d]= %f is=%d  \n",iu,u[iu-1],is);
      s[is - 1] = u[iu - 1];
      --is;
      --iu;
    } while (is > im);
    is = is - m + i__;
    --i__;
  } while (i__ >= 0);

  is = 1;
  do {
    iv = is;
    ih = is;
    while (1) {
      iv += m;
      ++ih;
      if (iv > m2)    break;
      // printf("s[%d]= %f ih=%d  \n",ih,s[ih-1],iv);
      s[ih - 1] = s[iv - 1];
    }
    is = is + m + 1;
  } while (is < m2);

  return 0;
} // trupck_

/* Subroutine */double* TCL::trsmlu(double* u, double* s, int n)
{
  // trsmlu.F -- translated by f2c (version 19970219).
  // CERN PROGLIB# F112    TRSMLU          .VERSION KERNFOR  4.15  861204 */
  // ORIG. 18/12/74 WH */
//

  //see original documentation of CERNLIB package F112



  // Local variables
  int lhor, lver, i__, k, l, ind;
  double sum;

  // Parameter adjuTments
  //   --s;    --u;

  // Function Body
  ind = (n * n + n) / 2;

  for (i__ = 1; i__ <= n; ++i__) {
    lver = ind;

    for (k = i__; k <= n; ++k, --ind) {
      lhor = ind;    sum = 0.;
      for (l = k; l <= n; ++l, --lver, --lhor)
        sum += u[lver] * u[lhor];
      s[ind] = sum;
    }
  }

  return 0;
} //  trsmlu_


double* TCL::trinv(double* t, double* s, int n)
{
  // trinv.F -- translated by f2c (version 19970219).
  // CERN PROGLIB# F112    TRINV           .VERSION KERNFOR  4.15  861204
  // ORIG. 18/12/74 WH
  //
//

  //see original documentation of CERNLIB package F112


  int lhor, ipiv, lver,  j;
  double r__;
  int mx, ndTep, ind;
  double sum;

  // Parameter adjuTments
  //   --s;    --t;

  // Function Body
  mx = (n * n + n) / 2;
  ipiv = mx;

  int i = n;
  do {
    r__ = 0.;
    //      printf("t[%d]=%f \n",ipiv-1,t[ipiv-1]);
    if (t[ipiv - 1] > 0.)  r__ = (double)1. / t[ipiv - 1];
    s[ipiv - 1] = r__;
    ndTep = n;
    ind = mx - n + i;
    //      printf("ipiv=%d r__=%f ind=%d\n",ipiv-1,r__,ind);
    while (ind != ipiv) {
      sum = 0.;
      if (r__ != 0.) {
        lhor = ipiv;
        lver = ind;
        j = i;

        do {
          lhor += j;
          ++lver;
          sum += t[lhor - 1] * s[lver - 1];
          ++j;
        } while (lhor < ind);
      }
      //    printf("ind=%d sum=%f \n",ind-1,-sum * r__);
      s[ind - 1] = -sum * r__;
      --ndTep;
      ind -= ndTep;
    }

    ipiv -= i;
    --i;
  } while (i > 0);

  return 0;
} // trinv_


double* TCL::trchul(double* a, double* b, int n)
{
//

  //see original documentation of CERNLIB package F112


  // Local variables
  int ipiv, kpiv, i__;
  double r__;
  int nTep;
  double dc;
  int id, kd;
  double sum;


  // CERN PROGLIB# F112    TRCHUL          .VERSION KERNFOR  4.16  870601
  // ORIG. 18/12/74 WH


  // Parameter adjuTments
  //   --b;    --a;

  // Function Body
  kpiv = (n * n + n) / 2;

  i__ = n;
  do {
    ipiv = kpiv;
    r__ = a[ipiv];

    do {
      sum = 0.;
      if (i__ == n)           goto L40;
      if (r__ == (double)0.)  goto L42;
      id = ipiv;
      kd = kpiv;
      nTep = i__;

      do {
        kd += nTep;
        id += nTep;
        ++nTep;
        sum += b[id] * b[kd];
      } while (nTep < n);

L40:
      sum = a[kpiv] - sum;
L42:
      if (kpiv < ipiv) b[kpiv] = sum * r__;
      else {
        //            dc = TMath::Sqrt(sum);
        dc = sqrt(sum);
        b[kpiv] = dc;
        if (r__ > (double)0.)         r__ = (double)1. / dc;
      }
      --kpiv;
    } while (kpiv > ipiv - i__);

    --i__;
  } while (i__ > 0);

  return 0;
} // trchul_

/* Subroutine */double* TCL::trsmul(double* g, double* gi, int n)
{
  // trsmul.F -- translated by f2c (version 19970219).
  // CERN PROGLIB# F112    TRSMUL          .VERSION KERNFOR  4.15  861204
  // ORIG. 18/12/74 WH
//

  //see original documentation of CERNLIB package F112



  // Local variables
  int lhor, lver, lpiv, i__, j, k, ind;
  double sum;

  // Parameter adjuTments
  //   --gi;    --g;

  // Function Body
  ind = 1;
  lpiv = 0;
  for (i__ = 1; i__ <= n; ++i__) {
    lpiv += i__;
    for (j = 1; j <= i__; ++j, ++ind) {
      lver = lpiv;
      lhor = ind;
      sum = 0.;
      for (k = i__; k <= n; lhor += k, lver += k, ++k)
        sum += g[lver - 1] * g[lhor - 1];
      gi[ind - 1] = sum;
    }
  }

  return 0;
} // trsmul_



double* TCL::trchlu(double* a, double* b, int n)
{
  // trchlu.F -- translated by f2c (version 19970219).
//

  //see original documentation of CERNLIB package F112


  // Local variables
  int ipiv, kpiv, i__, j;
  double r__, dc;
  int id, kd;
  double sum;


  // CERN PROGLIB# F112    TRCHLU          .VERSION KERNFOR  4.16  870601
  // ORIG. 18/12/74 W.HART


  // Parameter adjuTments
//   --b;    --a;

  // Function Body
  ipiv = 0;

  i__ = 0;
  /*
  for (j = 0; j <= n*(n+1)/2; ++j) {
         printf(" b[kd]=%f kd=%d  \n ",b[j],j);
     }
  */
  do {
    ++i__;
    ipiv += i__;
    kpiv = ipiv;
    r__ = a[ipiv - 1];
    //     printf("sd r__=%f \n",r__);

    for (j = i__; j <= n; ++j) {
      sum = 0.;
      if (i__ == 1)       goto L40;
      if (r__ == 0.)      goto L42;
      id = ipiv - i__ + 1;
      kd = kpiv - i__ + 1;

      do {
        //    printf("do sum1=%0.10f b[kd]=%f b[id]=%f kd-1=%d id-1=%d id=%d ipiv=%d \n ",sum,b[kd-1],b[id-1],kd-1,id-1,id,ipiv);
        sum += b[kd - 1] * b[id - 1];
        //       printf(" sum1=%0.10f  \n ",sum);
        ++kd;   ++id;
      } while (id < ipiv);

      //  printf("sum1=%f a=%f kpiv=%d  ",sum,a[kpiv],kpiv);

L40:
      sum = a[kpiv - 1] - sum;
      //  printf("sum2=%0.8f a=%0.8f kpiv=%d ",sum,a[kpiv-1],kpiv);
L42:
      if (j != i__) {
        b[kpiv - 1] = sum * r__;
        //    printf("A b[%d]= %0.10f \n",kpiv-1,b[kpiv-1]);
      } else {
        //            dc = TMath::Sqrt(sum);
        //    printf("X sum=%0.8f  \n",sum);
        //      if(sum<0){sum=0.;}
        dc = sqrt(sum);
        b[kpiv - 1] = dc;
        //     printf("B b[%d]= %0.5f \n",kpiv-1,b[kpiv-1]);
        if (r__ > 0.) {
          r__ = (double)1. / dc;
          //     printf("if r__=%f \n",r__);
        }
      }
      kpiv += j;
    }   //for

  } while (i__ < n);

  return 0;
} // trchlu_




//___________________________________________________________________________
inline float* TCL::mxmad(const float* a, const float* b, float* c, int i, int j, int k)
{
// Begin_Html <P ALIGN=CENTER> <IMG SRC="gif/mxpack_mxmad.gif"> </P> End_Html //
  return mxmad_0_(0, a, b, c, i, j, k);
}

//___________________________________________________________________________
inline float* TCL::mxmad1(const float* a, const float* q, float* c, int i, int j, int k)
{
// Begin_Html <P ALIGN=CENTER> <IMG SRC="gif/mxpack_mxmad1.gif"> </P> End_Html //
  return mxmad_0_(1, a, q, c, i, j, k);
}

//___________________________________________________________________________
/*
inline float *TCL::trpck(float *s, float *u, int n)
{
 // Begin_Html <P ALIGN=CENTER> <IMG SRC="gif/mxpack_mxmad1.gif"> </P> End_Html //
 return trpck_0_(s, u, n);  }

*/

//___________________________________________________________________________
inline float* TCL::mxmad2(const float* p, const float* b, float* c, int i, int j, int k)
{
// Begin_Html <P ALIGN=CENTER> <IMG SRC="gif/mxpack_mxmad2.gif"> </P> End_Html //
  return mxmad_0_(2, p, b, c, i, j, k);
}

//___________________________________________________________________________
inline float* TCL::mxmad3(const float* p, const float* q, float* c, int i, int j, int k)
{
// Begin_Html <P ALIGN=CENTER> <IMG SRC="gif/mxpack_mxmad3.gif"> </P> End_Html //
  return mxmad_0_(3, p, q, c, i, j, k);
}

//___________________________________________________________________________
inline float* TCL::mxmpy(const float* a, const float* b, float* c, int i, int j, int k)
{
// Begin_Html <P ALIGN=CENTER> <IMG SRC="gif/mxpack_mxmpy.gif"> </P> End_Html //
  return mxmad_0_(4, a, b, c, i, j, k);
}

//___________________________________________________________________________
inline float* TCL::mxmpy1(const float* a, const float* q, float* c, int i, int j, int k)
{
// Begin_Html <P ALIGN=CENTER> <IMG SRC="gif/mxpack_mxmpy1.gif"> </P> End_Html //
  return mxmad_0_(5, a, q, c, i, j, k);
}

//___________________________________________________________________________
inline float* TCL::mxmpy2(const float* p, const float* b, float* c, int i, int j, int k)
{
// Begin_Html <P ALIGN=CENTER> <IMG SRC="gif/mxpack_mxmpy2.gif"> </P> End_Html //
  return mxmad_0_(6, p, b, c, i, j, k);
}

//___________________________________________________________________________
inline float* TCL::mxmpy3(const float* p, const float* q, float* c, int i, int j, int k)
{
// Begin_Html <P ALIGN=CENTER> <IMG SRC="gif/mxpack_mxmpy3.gif"> </P> End_Html //
  return mxmad_0_(7, p, q, c, i, j, k);
}

//___________________________________________________________________________
inline float* TCL::mxmub(const float* a, const float* b, float* c, int i, int j, int k)
{
// Begin_Html <P ALIGN=CENTER> <IMG SRC="gif/mxpack_mxmub.gif"> </P> End_Html //
  return mxmad_0_(8, a, b, c, i, j, k);
}

//___________________________________________________________________________
inline float* TCL::mxmub1(const float* a, const float* q, float* c, int i, int j, int k)
{
// Begin_Html <P ALIGN=CENTER> <IMG SRC="gif/mxpack_mxmub1.gif"> </P> End_Html //
  return mxmad_0_(9, a, q, c, i, j, k);
}

//___________________________________________________________________________
inline float* TCL::mxmub2(const float* p, const float* b, float* c, int i, int j, int k)
{
// Begin_Html <P ALIGN=CENTER> <IMG SRC="gif/mxpack_mxmub2.gif"> </P> End_Html //
  return mxmad_0_(10, p, b, c, i, j, k);
}

//___________________________________________________________________________
inline float* TCL::mxmub3(const float* p, const float* q, float* c, int i, int j, int k)
{
// Begin_Html <P ALIGN=CENTER> <IMG SRC="gif/mxpack_mxmub3.gif"> </P> End_Html //
  return mxmad_0_(11, p, q, c, i, j, k);
}

//___________________________________________________________________________
inline float* TCL::mxmlrt(const float* a, const float* b, float* x, int ni, int nj)
{
// Begin_Html <P ALIGN=CENTER> <IMG SRC="gif/mxpack_mxmlrt.gif"> </P> End_Html //
  return mxmlrt_0_(0, a, b, x, ni, nj);
}

//___________________________________________________________________________
inline float* TCL::mxmltr(const float* a, const float* b, float* x, int ni, int nj)
{
// Begin_Html <P ALIGN=CENTER> <IMG SRC="gif/mxpack_mxmltr.gif"> </P> End_Html //
  return mxmlrt_0_(1, a, b, x, ni, nj);
}


//--   double version --

//___________________________________________________________________________
inline double* TCL::mxmad(const double* a, const double* b, double* c, int i, int j, int k)
{
// Begin_Html <P ALIGN=CENTER> <IMG SRC="gif/mxpack_mxmad.gif"> </P> End_Html //
  return mxmad_0_(0, a, b, c, i, j, k);
}

//___________________________________________________________________________
inline double* TCL:: mxmad1(const double* a, const double* b, double* c, int i, int j, int k)
{
// Begin_Html <P ALIGN=CENTER> <IMG SRC="gif/mxpack_mxmad.gif"> </P> End_Html //
  return mxmad_0_(1, a, b, c, i, j, k);
}

//___________________________________________________________________________
inline double* TCL::mxmad2(const double* a, const double* b, double* c, int i, int j, int k)
{
// Begin_Html <P ALIGN=CENTER> <IMG SRC="gif/mxpack_mxmad.gif"> </P> End_Html //
  return mxmad_0_(2, a, b, c, i, j, k);
}

//___________________________________________________________________________
inline double* TCL::mxmad3(const double* a, const double* b, double* c, int i, int j, int k)
{
// Begin_Html <P ALIGN=CENTER> <IMG SRC="gif/mxpack_mxmad.gif"> </P> End_Html //
  return mxmad_0_(3, a, b, c, i, j, k);
}

//___________________________________________________________________________
inline double* TCL::mxmpy(const double* a, const double* b, double* c, int i, int j, int k)
{
// Begin_Html <P ALIGN=CENTER> <IMG SRC="gif/mxpack_mxmad.gif"> </P> End_Html //
  return mxmad_0_(4, a, b, c, i, j, k);
}

//___________________________________________________________________________
inline double* TCL::mxmpy1(const double* a, const double* b, double* c, int i, int j, int k)
{
// Begin_Html <P ALIGN=CENTER> <IMG SRC="gif/mxpack_mxmad.gif"> </P> End_Html //
  return mxmad_0_(5, a, b, c, i, j, k);
}

//___________________________________________________________________________
inline double* TCL::mxmpy2(const double* a, const double* b, double* c, int i, int j, int k)
{
// Begin_Html <P ALIGN=CENTER> <IMG SRC="gif/mxpack_mxmad.gif"> </P> End_Html //
  return mxmad_0_(6, a, b, c, i, j, k);
}

//___________________________________________________________________________
inline double* TCL::mxmpy3(const double* a, const double* b, double* c, int i, int j, int k)
{
// Begin_Html <P ALIGN=CENTER> <IMG SRC="gif/mxpack_mxmad.gif"> </P> End_Html //
  return mxmad_0_(7, a, b, c, i, j, k);
}

//___________________________________________________________________________
inline double* TCL::mxmub(const double* a, const double* b, double* c, int i, int j, int k)
{
// Begin_Html <P ALIGN=CENTER> <IMG SRC="gif/mxpack_mxmad.gif"> </P> End_Html //
  return mxmad_0_(8, a, b, c, i, j, k);
}

//___________________________________________________________________________
inline double* TCL::mxmub1(const double* a, const double* b, double* c, int i, int j, int k)
{
// Begin_Html <P ALIGN=CENTER> <IMG SRC="gif/mxpack_mxmad.gif"> </P> End_Html //
  return mxmad_0_(9, a, b, c, i, j, k);
}

//___________________________________________________________________________
inline double* TCL::mxmub2(const double* a, const double* b, double* c, int i, int j, int k)
{
// Begin_Html <P ALIGN=CENTER> <IMG SRC="gif/mxpack_mxmad.gif"> </P> End_Html //
  return mxmad_0_(10, a, b, c, i, j, k);
}

//___________________________________________________________________________
inline double* TCL::mxmub3(const double* a, const double* b, double* c, int i, int j, int k)
{
// Begin_Html <P ALIGN=CENTER> <IMG SRC="gif/mxpack_mxmad.gif"> </P> End_Html //
  return mxmad_0_(11, a, b, c, i, j, k);
}

//___________________________________________________________________________
inline double* TCL::mxmlrt(const double* a, const double* b, double* c, int ni, int nj)
{
// Begin_Html <P ALIGN=CENTER> <IMG SRC="gif/mxpack_mxmad.gif"> </P> End_Html //
  return  mxmlrt_0_(0, a, b, c, ni, nj);
}

//___________________________________________________________________________
inline double* TCL::mxmltr(const double* a, const double* b, double* c, int ni, int nj)
{
// Begin_Html <P ALIGN=CENTER> <IMG SRC="gif/mxpack_mxmad.gif"> </P> End_Html //
  return mxmlrt_0_(1, a, b, c, ni, nj);
}

// ----

//________________________________________________________
inline int*  TCL::ucopy(const int*  b, int*  a, int n)
{ if (n <= 0) return 0; memcpy(a, b, n * sizeof(int)); return a;}

//________________________________________________________
inline float* TCL::ucopy(const float* b, float* a, int n)
{ if (n <= 0) return 0; memcpy(a, b, n * sizeof(float)); return a;}

//________________________________________________________
inline float* TCL::ucopy(const double* b, float* a, int n)
{
  if (n <= 0) return 0;
  for (int i = 0; i < n; i++, a++, b++) *a = float(*b);
  return a;
}

//________________________________________________________
inline double* TCL::ucopy(const float* b, double* a, int n)
{
  if (n <= 0) return 0;
  for (int i = 0; i < n; i++, a++, b++) *a = double(*b);
  return a;
}

//________________________________________________________
inline double* TCL::ucopy(const double* b, double* a, int n)
{ if (n <= 0) return 0; memcpy(a, b, n * sizeof(double)); return a;}

//________________________________________________________
inline void** TCL::ucopy(const void** b, void**  a, int n)
{ if (n <= 0) return 0; memcpy(a, b, n * sizeof(void*)); return a;}


//________________________________________________________
inline float* TCL::vadd(const float* b, const float* c,  float* a, int n)
{
  if (n <= 0)  return 0;
  for (int i = 0; i < n; i++) a[i] = b[i] + c[i];
  return a;
}

//________________________________________________________
inline double* TCL::vadd(const double* b, const double* c,  double* a, int n)
{
  if (n <= 0)  return 0;
  for (int i = 0; i < n; i++) a[i] = b[i] + c[i];
  return a;
}

//________________________________________________________
inline float*  TCL::vadd(const float* b, const double* c,  float* a, int n)
{
  if (n <= 0)  return 0;
  for (int i = 0; i < n; i++) a[i] = b[i] + c[i];
  return a;
}

//________________________________________________________
inline double* TCL::vadd(const double* b, const float* c,  double* a, int n)
{
  if (n <= 0)  return 0;
  for (int i = 0; i < n; i++) a[i] = b[i] + c[i];
  return a;
}

//________________________________________________________
inline float  TCL::vdot(const float*  b, const float* a, int n)
{
  float x = 0;
  if (n > 0)
    for (int i = 0; i < n; i++, a++, b++) x += (*a) * (*b);
  return x;
}
//________________________________________________________
inline double TCL::vdot(const double* b, const double* a, int n)
{
  double  x = 0;
  if (n > 0)
    for (int i = 0; i < n; i++, a++, b++) x += (*a) * (*b);
  return x;
}
//________________________________________________________
inline float* TCL::vsub(const float* a, const float* b, float* x, int n)
{
  if (n <= 0) return 0;
  for (int i = 0; i < n; i++) x[i] = a[i] - b[i];
  return x;
}

//________________________________________________________
inline double* TCL::vsub(const double* a, const double* b, double* x, int n)
{
  if (n <= 0) return 0;
  for (int i = 0; i < n; i++) x[i] = a[i] - b[i];
  return x;
}

//________________________________________________________
inline float* TCL::vcopyn(const float* a, float* x, int n)
{
  if (n <= 0) return 0;
  for (int i = 0; i < n; i++) x[i] = -a[i];
  return x;
}
//________________________________________________________
inline double* TCL::vcopyn(const double* a, double* x, int n)
{
  if (n <= 0) return 0;
  for (int i = 0; i < n; i++) x[i] = -a[i];
  return x;
}

//________________________________________________________
inline float* TCL::vzero(float* a, int n1)
{
  if (n1 <= 0) return 0;
  return (float*)memset(a, 0, n1 * sizeof(float));
}

//________________________________________________________
inline double* TCL::vzero(double* a, int n1)
{
  if (n1 <= 0) return 0;
  return (double*)memset(a, 0, n1 * sizeof(double));
}

//________________________________________________________
inline void** TCL::vzero(void** a, int n1)
{
  if (n1 <= 0) return 0;
  return (void**)memset(a, 0, n1 * sizeof(void*));
}

//________________________________________________________
inline float* TCL::vscale(const float* a, float scale, float* b, int n)
{
  for (int i = 0; i < n; i++) b[i] = scale * a[i];
  return b;
}

//________________________________________________________
inline double* TCL::vscale(const double* a, double scale, double* b, int n)
{
  for (int i = 0; i < n; i++) b[i] = scale * a[i];
  return b;
}

//________________________________________________________
inline float* TCL::vlinco(const float* a, float fa, const float* b, float fb, float* x, int n)
{
  for (int i = 0; i < n; i++) {x[i] = a[i] * fa + b[i] * fb;};
  return x;
}

//________________________________________________________
inline double* TCL::vlinco(const double* a, double fa, const double* b, double fb, double* x, int n)
{
  for (int i = 0; i < n; i++) x[i] = a[i] * fa + b[i] * fb;
  return x;
}

//_____________________________________________________________________________
inline float* TCL::vmatl(const float* G, const float* c, float* x, int n, int m)
{
  //  x = G*c
  for (int i = 0; i < n; i++) {
    double sum = 0;
    for (int j = 0; j < m; j++) sum += G[j + m * i] * c[j];
    x[i] = sum;
  }
  return x;
}

//_____________________________________________________________________________
inline double* TCL::vmatl(const double* G, const double* c, double* x, int n, int m)
{
  //  x = G*c
  for (int i = 0; i < n; i++) {
    double sum = 0;
    for (int j = 0; j < m; j++) sum += G[j + m * i] * c[j];
    x[i] = sum;
  }
  return x;
}


//_____________________________________________________________________________
inline float* TCL::vmatr(const float* c, const float* G, float* x, int n, int m)
{
  //  x = c*G
  for (int j = 0; j < m; j++) {
    double sum = 0;
    for (int i = 0; i < n; i++) sum += G[j + n * i] * c[i];
    x[j] = sum;
  }
  return x;
}

//_____________________________________________________________________________
inline double* TCL::vmatr(const double* c, const double* G, double* x, int n, int m)
{
  //  x = c*G
  for (int j = 0; j < m; j++) {
    double sum = 0;
    for (int i = 0; i < n; i++) sum += G[j + n * i] * c[i];
    x[j] = sum;
  }
  return x;
}

#endif
