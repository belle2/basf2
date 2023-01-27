/*
 * cache.h - cache classes
 *
 * this file is part of PJFry library
 * Copyright 2011 Valery Yundin
 */

#include "cache.h"
#include "integral.h"

/* ------------------------------------------------------------
 * ------------------------------------------------------------
 *                      ICache section
 * ------------------------------------------------------------
 * ------------------------------------------------------------
*/

double ICache::mu2=1;

double ICache::getMu2()
{
  return mu2;
}

double ICache::setMu2(const double newmu2)
{
  if (newmu2 > 0 && newmu2 != mu2) {
    MinorBase::Rescale(newmu2/mu2);
    mu2=newmu2;
#ifdef USE_ONELOOP
    double mu=sqrt(mu2);
    F77_FUNC_(avh_olo_mu_set,AVH_OLO_MU_SET)(&mu);
#endif
    ICache::Clear();
    MCache::Clear();
  }
  return mu2;
}

void ICache::Clear()
{
  ClearCC();
  ClearIC();
}

/* clear coefficient cache */
void ICache::ClearCC()
{
  ic5[0].reset();
  ic5[1].reset();
  ic4[0].reset();
  ic4[1].reset();
  ic3[0].reset();
  ic3[1].reset();
  ic2[0].reset();
  ic2[1].reset();
}

/* clear scalar integral cache */
void ICache::ClearIC()
{
  ics4.reset();
  ics3.reset();
  ics2.reset();
  ics1.reset();
}

/* clear minor cache */
void MCache::Clear()
{
  cm5.reset();
  cm4.reset();
  cm3.reset();
  cm2.reset();
}

// const double ICache::sNan=std::numeric_limits<double>::signaling_NaN();
// const int64_t ICache::sNAN=0x7ffc0000BA13BA13LL;
// const double ICache::sNAN.d64=reinterpret_cast<const double&>(sNAN); // Bad because breaks strict aliasing
// const double ICache::sNAN.d64=((const ICache::ID64&)sNAN).dbl;

const ICache::ID64 ICache::sNAN={ 0x7ffc0000BA13BA13LL };

ICache::Array5 ICache::ic5[3];
ICache::Array4 ICache::ic4[3];
ICache::Array3 ICache::ic3[3];
ICache::Array2 ICache::ic2[3];

ICache::ArrayS4 ICache::ics4;
ICache::ArrayS3 ICache::ics3;
ICache::ArrayS2 ICache::ics2;
ICache::ArrayS1 ICache::ics1;

/* ===========================================================
 *
 *           PENTAGON:  5 point coefficients
 *
 * ===========================================================
 */

/* --------------------------------------------------------
    Rank-5 PENTAGON
 * --------------------------------------------------------
 */
ncomplex ICache::getE(int ep, int i, int j, int k, int l, int m, const Kinem5 &kin)
{
#ifdef USE_CACHE_HIGH
  assert( (i==0 && j==0 && ( (k==0 && l==0 && m!=0) || (k!=0 && l!=0 && m!=0) ))
         || (i!=0 && j!=0 && k!=0 && l!=0 && m!=0) );
  int coefn=0;
  if (i==0 && j==0) {
    if (k==0 && l==0)
      coefn=(m-1)+ee00001;
    else
      coefn=Minor5::is(k-1,l-1,m-1)+ee00111;
  }
  else {
    coefn=Minor5::iss(i-1,j-1,k-1,l-1,m-1)+ee11111;
  }
  Save5 *s5=getS5(ep, kin, coefn);

  ncomplex ivalue=(*s5)[coefn];
  if (ivalue.real() == sNAN) {
    ivalue=MCache::getMinor5(kin)->evalE(ep, i, j, k, l, m);
    (*s5)[coefn]=ivalue;
  }
#else
  ncomplex ivalue=MCache::getMinor5(kin)->evalE(ep, i, j, k, l, m);
#endif /* USE_CACHE_HIGH */
  return ivalue;
}

/* --------------------------------------------------------
    Rank-4 PENTAGON
 * --------------------------------------------------------
 */
ncomplex ICache::getE(int ep, int i, int j, int k, int l, const Kinem5 &kin)
{
#ifdef USE_CACHE_HIGH
  assert( (i==0 && j==0 && ( (k==0 && l==0) || (k!=0 && l!=0) )) || (i!=0 && j!=0 && k!=0 && l!=0) );
  int coefn=0;
  if (i==0 && j==0) {
    if (k==0 && l==0)
      coefn=ee0000;
    else
      coefn=Minor5::is(k-1,l-1)+ee0011;
  }
  else {
    coefn=Minor5::is(i-1,j-1,k-1,l-1)+ee1111;
  }
  Save5 *s5=getS5(ep, kin, coefn);

  ncomplex ivalue=(*s5)[coefn];
  if (ivalue.real() == sNAN) {
    ivalue=MCache::getMinor5(kin)->evalE(ep, i, j, k, l);
    (*s5)[coefn]=ivalue;
  }
#else
  ncomplex ivalue=MCache::getMinor5(kin)->evalE(ep, i, j, k, l);
#endif /* USE_CACHE_HIGH */
  return ivalue;
}

/* --------------------------------------------------------
    Rank-3 PENTAGON
 * --------------------------------------------------------
 */
ncomplex ICache::getE(int ep, int i, int j, int k, const Kinem5 &kin)
{
#ifdef USE_CACHE_HIGH
  assert( (i==0 && j==0 && k!=0) || (i!=0 && j!=0 && k!=0) );
  int coefn=(i==0 && j==0) ? (k-1)+ee001 : Minor5::is(i-1,j-1,k-1)+ee111;
  Save5 *s5=getS5(ep, kin, coefn);

  ncomplex ivalue=(*s5)[coefn];
  if (ivalue.real() == sNAN) {
    ivalue=MCache::getMinor5(kin)->evalE(ep, i, j, k);
    (*s5)[coefn]=ivalue;
  }
#else
  ncomplex ivalue=MCache::getMinor5(kin)->evalE(ep, i, j, k);
#endif /* USE_CACHE_HIGH */
  return ivalue;
}

/* --------------------------------------------------------
    Rank-2 PENTAGON
 * --------------------------------------------------------
 */
ncomplex ICache::getE(int ep, int i, int j, const Kinem5 &kin)
{
#ifdef USE_CACHE_HIGH
  assert( (i==0 && j==0) || (i!=0 && j!=0) );
  int coefn=(i==0 && j==0) ? ee00 : Minor5::is(i-1,j-1)+ee11;
  Save5 *s5=getS5(ep, kin, coefn);

  ncomplex ivalue=(*s5)[coefn];
  if (ivalue.real() == sNAN) {
    ivalue=MCache::getMinor5(kin)->evalE(ep, i, j);
    (*s5)[coefn]=ivalue;
  }
#else
  ncomplex ivalue=MCache::getMinor5(kin)->evalE(ep, i, j);
#endif /* USE_CACHE_HIGH */
  return ivalue;
}

/* --------------------------------------------------------
    Rank-1 PENTAGON
 * --------------------------------------------------------
 */
ncomplex ICache::getE(int ep, int i, const Kinem5 &kin)
{
#ifdef USE_CACHE_HIGH
  int coefn=(i-1)+ee1;
  Save5 *s5=getS5(ep, kin, coefn);

  ncomplex ivalue=(*s5)[coefn];
  if (ivalue.real() == sNAN) {
    ivalue=MCache::getMinor5(kin)->evalE(ep, i);
    (*s5)[coefn]=ivalue;
  }
#else
  ncomplex ivalue=MCache::getMinor5(kin)->evalE(ep, i);
#endif /* USE_CACHE_HIGH */
  return ivalue;
}

/* --------------------------------------------------------
    Rank-0 PENTAGON
 * --------------------------------------------------------
 */
ncomplex ICache::getE(int ep, const Kinem5 &kin)
{
#ifdef USE_CACHE_HIGH
  int coefn=ee0;
  Save5 *s5=getS5(ep, kin, coefn);

  ncomplex ivalue=(*s5)[coefn];
  if (ivalue.real() == sNAN) {
    ivalue=MCache::getMinor5(kin)->evalE(ep);
    (*s5)[coefn]=ivalue;
  }
#else
  ncomplex ivalue=MCache::getMinor5(kin)->evalE(ep);
#endif /* USE_CACHE_HIGH */
  return ivalue;
}

/* ===========================================================
 *
 *                BOX:    4 point coefficients
 *
 * ===========================================================
 */

/* --------------------------------------------------------
    Rank-4 BOX
 * --------------------------------------------------------
 */
ncomplex ICache::getD(int ep, int i, int j, int k, int l, const Kinem4 &kin)
{
#ifdef USE_CACHE_LOW
  assert( (i==0 && j==0 && ( (k==0 && l==0) || (k!=0 && l!=0) )) || (i!=0 && j!=0 && k!=0 && l!=0) );
  int coefn=0;
  if (i==0 && j==0) {
    if (k==0 && l==0)
      coefn=dd0000;
    else
      coefn=Minor4::is(k-1,l-1)+dd0011;
  }
  else {
    coefn=Minor5::is(i-1,j-1,k-1,l-1)+dd1111;
  }
  Save4 *s4=getS4(ep, kin, coefn);

  ncomplex ivalue=(*s4)[coefn];
  if (ivalue.real() == sNAN) {
    ivalue=MCache::getMinor4(kin)->evalD(ep, i, j, k, l);
    (*s4)[coefn]=ivalue;
  }
#else
  ncomplex ivalue=MCache::getMinor4(kin)->evalD(ep, i, j, k, l);
#endif /* USE_CACHE_LOW */
  return ivalue;
}

/* --------------------------------------------------------
    Rank-3 BOX
 * --------------------------------------------------------
 */
ncomplex ICache::getD(int ep, int i, int j, int k, const Kinem4 &kin)
{
#ifdef USE_CACHE_LOW
  assert( (i==0 && j==0 && k!=0) || (i!=0 && j!=0 && k!=0) );
  int coefn=(i==0 && j==0) ? (k-1)+dd001 : Minor4::is(i-1,j-1,k-1)+dd111;
  Save4 *s4=getS4(ep, kin, coefn);

  ncomplex ivalue=(*s4)[coefn];
  if (ivalue.real() == sNAN) {
    ivalue=MCache::getMinor4(kin)->evalD(ep, i, j, k);
    (*s4)[coefn]=ivalue;
  }
#else
  ncomplex ivalue=MCache::getMinor4(kin)->evalD(ep, i, j, k);
#endif /* USE_CACHE_LOW */
  return ivalue;
}

/* --------------------------------------------------------
    Rank-2 BOX
 * --------------------------------------------------------
 */
ncomplex ICache::getD(int ep, int i, int j, const Kinem4 &kin)
{
#ifdef USE_CACHE_LOW
  assert( (i==0 && j==0) || (i!=0 && j!=0) );
  int coefn=(i==0 && j==0) ? dd00 : Minor4::is(i-1,j-1)+dd11;
  Save4 *s4=getS4(ep, kin, coefn);

  ncomplex ivalue=(*s4)[coefn];
  if (ivalue.real() == sNAN) {
    ivalue=MCache::getMinor4(kin)->evalD(ep, i, j);
    (*s4)[coefn]=ivalue;
  }
#else
  ncomplex ivalue=MCache::getMinor4(kin)->evalD(ep, i, j);
#endif /* USE_CACHE_LOW */
  return ivalue;
}

/* --------------------------------------------------------
    Rank-1 BOX
 * --------------------------------------------------------
 */
ncomplex ICache::getD(int ep, int i, const Kinem4 &kin)
{
#ifdef USE_CACHE_LOW
  int coefn=(i-1)+dd1;
  Save4 *s4=getS4(ep, kin, coefn);

  ncomplex ivalue=(*s4)[coefn];
  if (ivalue.real() == sNAN) {
    ivalue=MCache::getMinor4(kin)->evalD(ep, i);
    (*s4)[coefn]=ivalue;
  }
#else
  ncomplex ivalue=MCache::getMinor4(kin)->evalD(ep, i);
#endif /* USE_CACHE_LOW */
  return ivalue;
}

/* ===========================================================
 *
 *            TRIANGLE:    3 point coefficients
 *
 * ===========================================================
 */


/* --------------------------------------------------------
    Rank-3 TRIANGLE
 * --------------------------------------------------------
 */
ncomplex ICache::getC(int ep, int i, int j, int k, const Kinem3 &kin)
{
#ifdef USE_CACHE_LOW
  assert( (i==0 && j==0 && k!=0) || (i!=0 && j!=0 && k!=0) );
  int coefn=(i==0 && j==0) ? (k-1)+cc001 : Minor3::is(i-1,j-1,k-1)+cc111;
  Save3 *s3=getS3(ep, kin, coefn);

  ncomplex ivalue=(*s3)[coefn];
  Minor3::Ptr pm3;
  if (ivalue.real() == sNAN && (pm3=MCache::getMinor3(kin))!=0) {
    ivalue=pm3->evalC(ep, i, j, k);
    (*s3)[coefn]=ivalue;
  }
#else
  Minor3::Ptr pm3=MCache::getMinor3(kin);
  ncomplex ivalue= ( pm3!=0 ? pm3->evalC(ep, i, j, k) : sNAN.d64 );
#endif /* USE_CACHE_LOW */
#ifndef NDEBUG
  if (pm3==0) printf("C%d%d%d(%.18e,%.18e,%.18e,%e,%e,%e)=%f\n",i,j,k,kin.p1(),kin.p2(),kin.p3(),kin.m1(),kin.m2(),kin.m3(),ivalue.real());
#endif
  return ivalue;
}

/* --------------------------------------------------------
    Rank-2 TRIANGLE
 * --------------------------------------------------------
 */
ncomplex ICache::getC(int ep, int i, int j, const Kinem3 &kin)
{
#ifdef USE_CACHE_LOW
  assert( (i==0 && j==0) || (i!=0 && j!=0) );
  int coefn=(i==0 && j==0) ? cc00 : Minor3::is(i-1,j-1)+cc11;
  Save3 *s3=getS3(ep, kin, coefn);

  ncomplex ivalue=(*s3)[coefn];
  Minor3::Ptr pm3;
  if (ivalue.real() == sNAN && (pm3=MCache::getMinor3(kin))!=0) {
    ivalue=pm3->evalC(ep, i, j);
    (*s3)[coefn]=ivalue;
  }
#else
  Minor3::Ptr pm3=MCache::getMinor3(kin);
  ncomplex ivalue= ( pm3!=0 ? pm3->evalC(ep, i, j) : sNAN.d64 );
#endif /* USE_CACHE_LOW */
#ifndef NDEBUG
  if (pm3==0) printf("C%d%d(%.18e,%.18e,%.18e,%e,%e,%e)=%f\n",i,j,kin.p1(),kin.p2(),kin.p3(),kin.m1(),kin.m2(),kin.m3(),ivalue.real());
#endif
  return ivalue;
}

/* --------------------------------------------------------
    Rank-1 TRIANGLE
 * --------------------------------------------------------
 */
ncomplex ICache::getC(int ep, int i, const Kinem3 &kin)
{
#ifdef USE_CACHE_LOW
  int coefn=(i-1)+cc1;
  Save3 *s3=getS3(ep, kin, coefn);

  ncomplex ivalue=(*s3)[coefn];
  Minor3::Ptr pm3;
  if (ivalue.real() == sNAN && (pm3=MCache::getMinor3(kin))!=0) {
    ivalue=pm3->evalC(ep, i);
    (*s3)[coefn]=ivalue;
  }
#else
  Minor3::Ptr pm3=MCache::getMinor3(kin);
  ncomplex ivalue= ( pm3!=0 ? pm3->evalC(ep, i) : sNAN.d64 );
#endif /* USE_CACHE_LOW */
#ifndef NDEBUG
  if (pm3==0) printf("C%d(%.18e,%.18e,%.18e,%e,%e,%e)=%f\n",i,kin.p1(),kin.p2(),kin.p3(),kin.m1(),kin.m2(),kin.m3(),ivalue.real());
#endif
  return ivalue;
}

/* ===========================================================
 *
 *            BUBBLE:    2 point coefficients
 *
 * ===========================================================
 */

/* --------------------------------------------------------
    Rank-2 BUBBLE
 * --------------------------------------------------------
 */
ncomplex ICache::getB(int ep, int i, int j, const Kinem2 &kin)
{
#ifdef USE_CACHE_LOW
  assert( (i==0 && j==0) || (i!=0 && j!=0) );
  int coefn=(i==0 && j==0) ? bb00 : Minor2::is(i-1,j-1)+bb11;
  Save2 *s2=getS2(ep, kin, coefn);

  ncomplex ivalue=(*s2)[coefn];
  Minor2::Ptr pm2;
  if (ivalue.real() == sNAN && (pm2=MCache::getMinor2(kin))!=0) {
    ivalue=pm2->evalB(ep, i, j);
    (*s2)[coefn]=ivalue;
  }
#else
  Minor2::Ptr pm2=MCache::getMinor2(kin);
  ncomplex ivalue= ( pm2!=0 ? pm2->evalB(ep, i, j) : sNAN.d64 );
#endif /* USE_CACHE_LOW */
#ifndef NDEBUG
  if (pm2==0) printf("B%d%d(%.18e,%.18e,%.18e)=%f\n",i,j,kin.p1(),kin.m1(),kin.m2(),ivalue.real());
#endif
  return ivalue;
}

/* --------------------------------------------------------
    Rank-1 BUBBLE
 * --------------------------------------------------------
 */
ncomplex ICache::getB(int ep, int i, const Kinem2 &kin)
{
#ifdef USE_CACHE_LOW
  int coefn=(i-1)+bb1;
  Save2 *s2=getS2(ep, kin, coefn);

  ncomplex ivalue=(*s2)[coefn];
  Minor2::Ptr pm2;
  if (ivalue.real() == sNAN && (pm2=MCache::getMinor2(kin))!=0) {
    ivalue=pm2->evalB(ep, i);
    (*s2)[coefn]=ivalue;
  }
#else
  Minor2::Ptr pm2=MCache::getMinor2(kin);
  ncomplex ivalue= ( pm2!=0 ? pm2->evalB(ep, i) : sNAN.d64 );
#endif /* USE_CACHE_LOW */
#ifndef NDEBUG
  if (pm2==0) printf("B%d(%.18e,%.18e,%.18e)=%f\n",i,kin.p1(),kin.m1(),kin.m2(),ivalue.real());
#endif
  return ivalue;
}

/* ===========================================================
 *
 *               Get saved value
 *
 * ===========================================================
 */

// ICache::Save5* ICache::getS5(int ep, const Kinem5 &kin, int coefn)
// {
//   Save5 *s5=0;
//   for (Array5::iterator it5=ic5[ep].begin(); it5 != ic5[ep].end(); ++it5) {
//     if (it5->key == kin) {
//       s5=it5->val.get();
//       break;
//     }
//   }
//   if (s5 == 0) {
//     Save5::Ptr sptr(new Save5(ncomplex(sNAN.d64, 0)));
//     s5=sptr.get();
//     ic5[ep].insert(Entry5(kin, sptr));
//   }
//   return s5;
// }

#define getSave(n) \
ICache::Save##n * ICache::getS##n(int ep, const Kinem##n &kin, int coefn) \
{ \
  Save##n *s##n=0; \
  for (Array##n::iterator it##n=ic##n[ep].begin(); it##n != ic##n[ep].end(); ++it##n) { \
    if (it##n->key == kin) { \
      s##n=it##n->val.get(); \
      break; \
    } \
  } \
  if (s##n == 0) { \
    Save##n::Ptr sptr(new Save##n(ncomplex(sNAN.d64, 0))); \
    s##n=sptr.get(); \
    ic##n[ep].insert(Entry##n(kin, sptr)); \
  } \
  return s##n; \
} \

getSave(5)
getSave(4)
getSave(3)
getSave(2)

#undef getSave

// ncomplex ICache::getI2(int ep, const Kinem2 &k)
// {
//   ncomplex ivalue(sNAN.d64, 0);
//   for (ArrayS2::iterator it2=ics2[ep].begin(); it2 != ics2[ep].end(); ++it2) {
//     if (it2->key == k) {
//       ivalue=it2->val;
//       break;
//     }
//   }
//   if (ivalue.real() == sNAN) {
//     ivalue=qlI2(k.p1(),
//                 k.m1(), k.m2(),
//                 -ep);
//     ics2[ep].insert(EntryS2(k,ivalue));
//   }
//   return ivalue;
// }

#define getIN(n) \
ncomplex ICache::getI##n(int ep, const Kinem##n &k) \
{ \
  Ival ivalue; \
  bool found=false; \
  for (ArrayS##n::iterator it##n=ics##n.begin(); it##n != ics##n.end(); ++it##n) { \
    if (it##n->key == k) { \
      ivalue=it##n->val; \
      found=true; \
      break; \
    } \
  } \
  if ( ! found ) { \
    ivalue=qlI##n(k); \
    ics##n.insert(EntryS##n(k,ivalue)); \
  } \
  return ivalue.val[ep]; \
}

getIN(1)
getIN(2)
getIN(3)
getIN(4)

#undef getIN

/* ------------------------------------------------------------
 * ------------------------------------------------------------
 *                       MCache section
 * ------------------------------------------------------------
 * ------------------------------------------------------------
*/

MCache::Array5 MCache::cm5;
MCache::Array4 MCache::cm4;
MCache::Array3 MCache::cm3;
MCache::Array2 MCache::cm2;

// Minor5::Ptr MCache::getMinor5(const Kinem5 &k)
// {
//   Minor5::Ptr minor;
//   for (Array5::iterator it5=cm5.begin(); it5!=cm5.end(); ++it5) {
//     if (it5->key == k) {
//       minor=it5->val;
//       break;
//     }
//   }
//   if (minor==0) {
//     minor=Minor5::create(k);
//     cm5.insert(Entry5(k,minor));
//   }
//   return minor;
// }

#define getMinorN(n) \
Minor##n::Ptr MCache::getMinor##n(const Kinem##n &k) \
{ \
  Minor##n::Ptr minor; \
  for (Array##n::iterator it##n=cm##n.begin(); it##n!=cm##n.end(); ++it##n) { \
    if (it##n->key == k) { \
      minor=it##n->val; \
      break; \
    } \
  } \
  /* if (minor==0) { \
    minor=Minor##n::create(k); \
    cm##n.insert(Entry##n(k,minor)); \
  } \
  assert(minor!=0); */ \
  return minor; \
}

getMinorN(3)
getMinorN(2)

#undef getMinorN

Minor5::Ptr MCache::getMinor5(const Kinem5 &k)
{
  Minor5::Ptr minor;
  for (Array5::iterator it5=cm5.begin(); it5!=cm5.end(); ++it5) {
    if (it5->key == k) {
      minor=it5->val;
      break;
    }
  }
  if (minor==0) {
    minor=Minor5::create(k);
    cm5.insert(Entry5(k,minor));
  }
  assert(minor!=0);
  return minor;
}

Minor4::Ptr MCache::getMinor4(const Kinem4 &k)
{
  Minor4::Ptr minor;
  for (Array4::iterator it4=cm4.begin(); it4!=cm4.end(); ++it4) {
    if (it4->key == k) {
      minor=it4->val;
      break;
    }
  }
  if (minor==0) {
    Minor5::create(k);
    minor=cm4.begin()->val;
    cm4.insert(Entry4(k,minor));
  }
  assert(minor!=0);
  return minor;
}


#ifdef USE_SMART_INSERT

void MCache::smartinsertMinor3(const Kinem3 &k, Minor3::Ptr &m)
{
  for (Array3::iterator it3=cm3.begin(); it3!=cm3.end(); ++it3) {
    if (it3->key == k) {
      cm3.remove(it3);
      break;
    }
  }
  insertMinor3(k,m);
}

void MCache::smartinsertMinor2(const Kinem2 &k, Minor2::Ptr &m)
{
  for (Array2::iterator it2=cm2.begin(); it2!=cm2.end(); ++it2) {
    if (it2->key == k) {
      cm2.remove(it2);
      break;
    }
  }
  insertMinor2(k,m);
}

#endif

// -------------------------------------------------------
