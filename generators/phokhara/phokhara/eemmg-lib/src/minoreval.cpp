/*
 * minoreval.cpp - integral coefficient functions
 *
 * this file is part of PJFry library
 * Copyright 2011 Valery Yundin
 */

#include "minor.h"
#include "cache.h"

/* ========================================================
 * ========================================================
 *
 *                PENTAGON coefficients - 5 point
 *
 * ========================================================
 * ========================================================
 */

/* --------------------------------------------------------
    5-point coefficients rank-0
 * --------------------------------------------------------
 */
ncomplex Minor5::evalE(int ep)
{
  ncomplex ivalue=0;
  ncomplex sum1=0;
  const double d00=M1(0, 0);
  for (int s=1; s<=5; s++) {
    sum1+=M1(0, s)*I4s(ep, s);
  }
  ivalue=sum1/d00;
  return ivalue;
}

/* --------------------------------------------------------
    5-point coefficients rank-1
 * --------------------------------------------------------
 */
ncomplex Minor5::evalE(int ep, int i)
{
  ncomplex ivalue=0;
  ncomplex sum1=0;
  const double d00=M1(0, 0);
  for (int s=1; s<=5; s++) {
    sum1+=M2(0, i, 0, s)*I4s(ep, s);
  }
  ivalue=-sum1/d00;
  return ivalue;
}

/* --------------------------------------------------------
    5-point coefficients rank-2
 * --------------------------------------------------------
 */
ncomplex Minor5::evalE(int ep, int i, int j)
{
  ncomplex ivalue=0;
  const double d00=M1(0, 0);
  if (i==0 && j==0) {
    ncomplex sum1=0;
    for (int s=1; s<=5; s++) {
      sum1+=M1(0, s)*I4Ds(ep, s);
    }
    ivalue=-sum1/(2*d00);
  }
  else {
    assert(i!=0 && j!=0); // E01, E02, etc do not exist
    ncomplex sum1=0;
    for (int s=1; s<=5; s++) {
      sum1+=M2(0, i, s, j)*I4Ds(ep, s);
      sum1+=M2(0, s, 0, j)*I4Dsi(ep, s, i);
    }
//     if (i!=j) { // is symmetrization needed?
//       ncomplex sumX=0;
//       for (int s=1; s<=5; s++) {
//         sumX+=M2(0, j, s, i)*I4Ds(ep, s);
//         sumX+=M2(0, s, 0, i)*I4Dsi(ep, s, j);
//       }
//       sum1=0.5*(sum1+sumX);
//     }
    ivalue=sum1/d00;
  }
  return ivalue;
}

/* --------------------------------------------------------
    5-point coefficients rank-3
 * --------------------------------------------------------
 */
ncomplex Minor5::evalE(int ep, int i, int j, int k)
{
  ncomplex ivalue=0;
  const double d00=M1(0, 0);
  if (i==0 && j==0) {
    assert(i==0 && j==0 && k!=0); // E000 does not exist, E100 is a wrong syntax

    /* // Fleischer's formula 6.13
    ncomplex sum1=0;
    for (int s=1; s<=5; s++) {
      sum1+=0.5*M2(0, s, 0, k)*I4Ds(ep, s);
      sum1+=-M1(s, k)*I4D2s(ep, s); // (4-1)/3=1 // NB d-1=3 and d=4
    }
    ivalue=sum1/d00;
    */
    // This variant looks simpler (does not depend on I4D2s)
    ncomplex sum1=0;
    for (int s=1; s<=5; s++) {
      sum1+=0.5*M2(0, s, 0, k)*I4Ds(ep, s);
      sum1+=M1(s, 0)*I4D2si(ep, s, k);
    }
    ivalue=sum1/(3*d00);
  }
  else {
    assert(i!=0 && j!=0 && k!=0); // E110, E012, etc do not exist
    ncomplex sum1=0;
    ncomplex sumX=0;
    ncomplex sumY=0;
    if (i == j) {
      for (int s=1; s<=5; s++) {
        sum1+=2*M2(0, j, s, k)*I4D2si(ep, s, i);
        sum1+=M2(0, s, 0, k)*I4D2sij(ep, s, i, j);
      }
      if (i != k) {
        for (int s=1; s<=5; s++) {
          sumX+=M2(0, j, s, i)*I4D2si(ep, s, k);
          sumX+=M2(0, k, s, i)*I4D2si(ep, s, j);
          sumX+=M2(0, s, 0, i)*I4D2sij(ep, s, k, j);
        }
        sum1=(sum1+2.*sumX)/3.;
      }
    } else { // i!=j
      for (int s=1; s<=5; s++) {
        sum1+=M2(0, j, s, k)*I4D2si(ep, s, i);
        sum1+=M2(0, i, s, k)*I4D2si(ep, s, j);
        sum1+=M2(0, s, 0, k)*I4D2sij(ep, s, i, j);
      }
      if (i != k) {
        for (int s=1; s<=5; s++) {
          sumX+=M2(0, j, s, i)*I4D2si(ep, s, k);
          sumX+=M2(0, k, s, i)*I4D2si(ep, s, j);
          sumX+=M2(0, s, 0, i)*I4D2sij(ep, s, k, j);
        }
      }
      else { sumX=sum1; }
      if (j != k) {
        for (int s=1; s<=5; s++) {
          sumY+=M2(0, k, s, j)*I4D2si(ep, s, i);
          sumY+=M2(0, i, s, j)*I4D2si(ep, s, k);
          sumY+=M2(0, s, 0, j)*I4D2sij(ep, s, i, k);
        }
      }
      else { sumY=sum1; }
      sum1=(sum1+sumX+sumY)/3.;
    }
    ivalue=-sum1/d00;
  }
  return ivalue;
}

/* --------------------------------------------------------
    5-point coefficients rank-4
 * --------------------------------------------------------
 */
ncomplex Minor5::evalE(int ep, int i, int j, int k, int l)
{
  ncomplex ivalue=0;
  const double d00=M1(0, 0);

  if (i==0 && j==0) {
    if (k==0 && l==0) {
      ncomplex sum1=0;
      for (int s=1; s<=5; s++) {
#ifndef USE_GOLEM_MODE
// Cancel pole and finite part with E00ij - LoopTools-like convention
        sum1+=M1(s, 0)*(I4D2s(ep, s)+(ep==0 ? (-1./6.+1./4.) : (ep==1? -1./6.: 0.)));
#else
// Golem95 convention
        sum1+=M1(s, 0)*(I4D2s(ep, s)+(ep==0 ? -1./9. : 0.));
#endif
      }
      ivalue=0.25*sum1/d00;
    }
    else {
      assert(i==0 && j==0 && k!=0 && l!=0); // E0001 does not exist, E1200 is a wrong syntax
      ncomplex sum1=0;

      for (int s=1; s<=5; s++) {
#ifndef USE_GOLEM_MODE
// Cancel pole and finite part with E0000 - LoopTools-like convention
        sum1+=0.5*(M2(0, k, s, l)+M2(0, l, s, k))*(I4D2s(ep, s)+(ep==0 ? (-1./6.+1./4.) : (ep==1? -1./6.: 0.)));
#else
// Golem95 convention
        sum1+=0.5*(M2(0, k, s, l)+M2(0, l, s, k))*(I4D2s(ep, s)+(ep==0 ? -1./9. : 0.));
#endif
        sum1+=0.5*(M2(0, s, 0, l)*I4D2si(ep, s, k)+M2(0, s, 0, k)*I4D2si(ep, s, l));
        sum1+=0.5*M1(s, 0)*(I4D3sij(ep, s, k, l)+I4D3sij(ep, s, l, k));
      }
      ivalue=-0.25*sum1/d00;
    }
  }
  else {
    assert(i!=0 && j!=0 && k!=0 && l!=0); // E110, E012, etc do not exist
    ncomplex sum1234=0;
    for (int s=1; s<=5; s++) {
      ncomplex sum1=M2(0,k,s,l)*I4D3sij(ep,s,i,j)
                   +M2(0,i,s,l)*I4D3sij(ep,s,k,j)
                   +M2(0,j,s,l)*I4D3sij(ep,s,i,k)
                   +M2(0,s,0,l)*I4D3sijk(ep,s,i,j,k);
      ncomplex sum2=sum1;
      if (l!=k) {
        sum2=M2(0,l,s,k)*I4D3sij(ep,s,i,j)
            +M2(0,i,s,k)*I4D3sij(ep,s,l,j)
            +M2(0,j,s,k)*I4D3sij(ep,s,i,l)
            +M2(0,s,0,k)*I4D3sijk(ep,s,i,j,l);
      }
      ncomplex sum3=sum1;
      if (j==k) {
        sum3=sum2;
      }
      else if (l!=j) {
        sum3=M2(0,k,s,j)*I4D3sij(ep,s,i,l)
            +M2(0,i,s,j)*I4D3sij(ep,s,k,l)
            +M2(0,l,s,j)*I4D3sij(ep,s,i,k)
            +M2(0,s,0,j)*I4D3sijk(ep,s,i,l,k);
      }
      ncomplex sum4=sum1;
      if (i==j) {
        sum4=sum3;
      }
      else if (l!=i) {
        sum4=M2(0,k,s,i)*I4D3sij(ep,s,l,j)
            +M2(0,l,s,i)*I4D3sij(ep,s,k,j)
            +M2(0,j,s,i)*I4D3sij(ep,s,l,k)
            +M2(0,s,0,i)*I4D3sijk(ep,s,l,j,k);
      }
      sum1234+=sum1+sum2+sum3+sum4;
    }
    ivalue=sum1234/(4*d00);
  }
  return ivalue;
}

/* --------------------------------------------------------
    5-point coefficients rank-5
 * --------------------------------------------------------
 */
ncomplex Minor5::evalE(int ep, int i, int j, int k, int l, int m)
{
  ncomplex ivalue=0;
  const double d00=M1(0, 0);

  if (i==0 && j==0) {
    if (k==0 && l==0) {
      assert(m!=0); // E00000 does not exist
      ncomplex sum1=0;
      for (int s=1; s<=5; s++) {
        sum1+=+M2(0, s, 0, m)*I4D2s(ep, s)
              +M1(s, 0)*(4.*I4D3si(ep, s, m)-2.*(ep<2 ? I4D3si(ep+1, s, m) : 0.));
      }
      ivalue=-sum1/(20.*d00);
    }
    else {
      assert(i==0 && j==0 && k!=0 && l!=0 && m!=0); // E00012 does not exist, E00100 is a wrong syntax
      ncomplex sum1=0;
      ncomplex sumX=0;
      ncomplex sumY=0;
      if (k == l) {
        for (int s=1; s<=5; s++) {
          sum1+=+2*M2(0, k, s, m)*I4D3si(ep, s, l)
                  +M2(0, s, 0, m)*I4D3sij(ep, s, k, l);
        }
        if (ep==0) sum1+=1./24.*(M1(k, m)-M2(0, k, l, m));
        if (k != m) {
          for (int s=1; s<=5; s++) {
            sumX+=+M2(0, m, s, k)*I4D3si(ep, s, l)
                  +M2(0, l, s, k)*I4D3si(ep, s, m)
                  +M2(0, s, 0, k)*I4D3sij(ep, s, m, l);
          }
          if (ep==0) sumX+=1./48.*(M1(m, k)+M1(l, k)-M2(0, l, m, k));
          sum1=(sum1+2.*sumX)/3.;
        }
      } else { // k!=l
        for (int s=1; s<=5; s++) {
          sum1+=+M2(0, k, s, m)*I4D3si(ep, s, l)
                +M2(0, l, s, m)*I4D3si(ep, s, k)
                +M2(0, s, 0, m)*I4D3sij(ep, s, k, l);
        }
        if (ep==0) sum1+=1./48.*(M1(k, m)+M1(l, m)-M2(0, k, l, m)-M2(0, l, k, m));
        if (k != m) {
          for (int s=1; s<=5; s++) {
            sumX+=+M2(0, m, s, k)*I4D3si(ep, s, l)
                  +M2(0, l, s, k)*I4D3si(ep, s, m)
                  +M2(0, s, 0, k)*I4D3sij(ep, s, m, l);
          }
          if (ep==0) sumX+=1./48.*(M1(m, k)+M1(l, k)-M2(0, m, l, k)-M2(0, l, m, k));
        }
        else { sumX=sum1; }
        if (l != m) {
          for (int s=1; s<=5; s++) {
            sumY+=+M2(0, k, s, l)*I4D3si(ep, s, m)
                  +M2(0, m, s, l)*I4D3si(ep, s, k)
                  +M2(0, s, 0, l)*I4D3sij(ep, s, k, m);
          }
          if (ep==0) sumY+=1./48.*(M1(k, l)+M1(m, l)-M2(0, k, m, l)-M2(0, m, k, l));
        }
        else { sumY=sum1; }
        sum1=(sum1+sumX+sumY)/3.;
      }
      sumX=0;
      for (int s=1; s<=5; s++) {
        sumX+=M1(s,0)*I4D4sijk(ep, s, k, l, m);
      }
      sum1=3.*sum1+2.*sumX;
      ivalue=sum1/(10.*d00);
    }
  }
  else {
    assert(i!=0 && j!=0 && k!=0 && l!=0 && m!=0);
    ncomplex sum12345=0;
    for (int s=1; s<=5; s++) {
      ncomplex sum1=+M2(0, l, s, m)*I4D4sijk(ep, s, i, j, k)
                    +M2(0, i, s, m)*I4D4sijk(ep, s, l, j, k)
                    +M2(0, j, s, m)*I4D4sijk(ep, s, i, l, k)
                    +M2(0, k, s, m)*I4D4sijk(ep, s, i, j, l)
                    +M2(0, s, 0, m)*I4D4sijkl(ep, s, i, j, k, l);
      ncomplex sum2=sum1;
      if (m!=l) {
        sum2=+M2(0, m, s, l)*I4D4sijk(ep, s, i, j, k)
             +M2(0, i, s, l)*I4D4sijk(ep, s, m, j, k)
             +M2(0, j, s, l)*I4D4sijk(ep, s, i, m, k)
             +M2(0, k, s, l)*I4D4sijk(ep, s, i, j, m)
             +M2(0, s, 0, l)*I4D4sijkl(ep, s, i, j, k, m);
      }
      ncomplex sum3=sum1;
      if (k==l) {
        sum3=sum2;
      }
      else if (m!=k) {
        sum3=+M2(0, l, s, k)*I4D4sijk(ep, s, i, j, m)
             +M2(0, i, s, k)*I4D4sijk(ep, s, l, j, m)
             +M2(0, j, s, k)*I4D4sijk(ep, s, i, l, m)
             +M2(0, m, s, k)*I4D4sijk(ep, s, i, j, l)
             +M2(0, s, 0, k)*I4D4sijkl(ep, s, i, j, m, l);
      }
      ncomplex sum4=sum1;
      if (j==k) {
        sum4=sum3;
      }
      else if (m!=j) {
        sum4=+M2(0, l, s, j)*I4D4sijk(ep, s, i, m, k)
             +M2(0, i, s, j)*I4D4sijk(ep, s, l, m, k)
             +M2(0, m, s, j)*I4D4sijk(ep, s, i, l, k)
             +M2(0, k, s, j)*I4D4sijk(ep, s, i, m, l)
             +M2(0, s, 0, j)*I4D4sijkl(ep, s, i, m, k, l);
      }
      ncomplex sum5=sum1;
      if (i==j) {
        sum5=sum4;
      }
      else if (m!=i) {
        sum5=+M2(0, l, s, i)*I4D4sijk(ep, s, m, j, k)
             +M2(0, m, s, i)*I4D4sijk(ep, s, l, j, k)
             +M2(0, j, s, i)*I4D4sijk(ep, s, m, l, k)
             +M2(0, k, s, i)*I4D4sijk(ep, s, m, j, l)
             +M2(0, s, 0, i)*I4D4sijkl(ep, s, m, j, k, l);
      }
      sum12345+=sum1+sum2+sum3+sum4+sum5;
    }
    ivalue=-sum12345/(5*d00);
  }
  return ivalue;
}

/* ========================================================
 * ========================================================
 *
 *                BOX coefficients - 4 point
 *
 * ========================================================
 * ========================================================
 */
#ifdef USE_GOLEM_MODE_6
    int psix;
    #define ix(i) i<pm5->psix ? i : i-1
#else
    #define ix(i) i
#endif
/* --------------------------------------------------------
    4-point scalar for GolemMode
 * --------------------------------------------------------
 */
#ifdef USE_GOLEM_MODE
ncomplex Minor4::A(int ep)
{
  ncomplex ivalue=pm5->I4s(ep, ps);
  return ivalue;
}
#endif /* USE_GOLEM_MODE */

/* --------------------------------------------------------
    4-point coefficients rank-1
 * --------------------------------------------------------
 */
// Global chord indexing, golem-like
#ifdef USE_GOLEM_MODE
ncomplex Minor4::A(int ep, int i)
{
  ncomplex ivalue=-pm5->I4Dsi(ep, ps, ix(i+offs));
  return ivalue;
}
#endif /* USE_GOLEM_MODE */

// Local chord indexing
ncomplex Minor4::evalD(int ep, int i)
{
  ncomplex ivalue=0;
  if (i>=ps || ps==5) i=i+1;
  ivalue=-pm5->I4Dsi(ep, ps, i);
  return ivalue;
}

/* --------------------------------------------------------
    4-point coefficients rank-2
 * --------------------------------------------------------
 */
// Global chord indexing, golem-like
#ifdef USE_GOLEM_MODE
ncomplex Minor4::A(int ep, int i, int j)
{
  ncomplex ivalue=pm5->I4D2sij(ep, ps, ix(i+offs), ix(j+offs));
  return ivalue;
}

ncomplex Minor4::B(int ep)
{
  ncomplex ivalue=-0.5*pm5->I4Ds(ep, ps);
  return ivalue;
}
#endif /* USE_GOLEM_MODE */

// Local chord indexing
ncomplex Minor4::evalD(int ep, int i, int j)
{
  ncomplex ivalue=0;

  if (i==0 && j==0) {
    ivalue=-0.5*pm5->I4Ds(ep, ps);
  }
  else {
    assert(i!=0 && j!=0); // D01, D02, etc do not exist
    if (i>=ps || ps==5) i=i+1;
    if (j>=ps || ps==5) j=j+1;

    ivalue=pm5->I4D2sij(ep, ps, i, j);
  }
  return ivalue;
}

/* --------------------------------------------------------
    4-point coefficients rank-3
 * --------------------------------------------------------
 */
// Global chord indexing, golem-like
#ifdef USE_GOLEM_MODE
ncomplex Minor4::A(int ep, int i, int j, int k)
{
  ncomplex ivalue=-pm5->I4D3sijk(ep, ps, ix(i+offs), ix(j+offs), ix(k+offs));
  return ivalue;
}

ncomplex Minor4::B(int ep, int k)
{
  ncomplex ivalue=0.5*pm5->I4D2si(ep, ps, ix(k+offs));
  return ivalue;
}
#endif /* USE_GOLEM_MODE */

// Local chord indexing
ncomplex Minor4::evalD(int ep, int i, int j, int k)
{
  ncomplex ivalue=0;

  if (i==0 && j==0) {
    assert(k!=0);                 // D000 does not exist, D100 is a wrong syntax
    if (k>=ps || ps==5) k=k+1;

    ivalue=0.5*pm5->I4D2si(ep, ps, k);
  }
  else {
    assert(i!=0 && j!=0 && k!=0); // D110, D012, etc do not exist
    if (i>=ps || ps==5) i=i+1;
    if (j>=ps || ps==5) j=j+1;
    if (k>=ps || ps==5) k=k+1;
    ivalue=-pm5->I4D3sijk(ep, ps, i, j, k);
  }
  return ivalue;
}

/* --------------------------------------------------------
    4-point coefficients rank-4
 * --------------------------------------------------------
 */
// Global chord indexing, golem-like
#ifdef USE_GOLEM_MODE
ncomplex Minor4::A(int ep, int i, int j, int k, int l)
{
  ncomplex ivalue=pm5->I4D4sijkl(ep, ps, ix(i+offs), ix(j+offs), ix(k+offs), ix(l+offs));
  return ivalue;
}

ncomplex Minor4::B(int ep, int k, int l)
{
  ncomplex ivalue=-0.5*pm5->I4D3sij(ep, ps, ix(k+offs), ix(l+offs));
  return ivalue;
}

ncomplex Minor4::C(int ep)
{
  ncomplex ivalue=0.25*pm5->I4D2s(ep, ps);
  return ivalue;
}
#endif /* USE_GOLEM_MODE */

// Local chord indexing
ncomplex Minor4::evalD(int ep, int i, int j, int k, int l)
{
  ncomplex ivalue=0;
  if (i==0 && j==0) {
    if (k==0 && l==0) {
      ivalue=0.25*pm5->I4D2s(ep, ps);
    }
    else {
      assert(i==0 && j==0 && k!=0 && l!=0); // D0001 does not exist, D1200 is a wrong syntax
      if (k>=ps || ps==5) k=k+1;
      if (l>=ps || ps==5) l=l+1;

      ivalue=-0.5*pm5->I4D3sij(ep, ps, k, l);
    }
  }
  else {
    assert(i!=0 && j!=0 && k!=0 && l!=0); // D110, D012, etc do not exist
    if (i>=ps || ps==5) i=i+1;
    if (j>=ps || ps==5) j=j+1;
    if (k>=ps || ps==5) k=k+1;
    if (l>=ps || ps==5) l=l+1;
    ivalue=pm5->I4D4sijkl(ep, ps, i, j, k, l);
  }
  return ivalue;
}

/* ========================================================
 * ========================================================
 *
 *                TRIANGLE coefficients - 3 point
 *
 * ========================================================
 * ========================================================
 */
/* --------------------------------------------------------
    3-point scalar for GolemMode
 * --------------------------------------------------------
 */
#ifdef USE_GOLEM_MODE
ncomplex Minor3::A(int ep)
{
  ncomplex ivalue=pm5->I3st(ep, ps, pt);
  return ivalue;
}
#endif /* USE_GOLEM_MODE */

/* --------------------------------------------------------
    3-point coefficients rank-1
 * --------------------------------------------------------
 */
// Global chord indexing, golem-like
#ifdef USE_GOLEM_MODE
ncomplex Minor3::A(int ep, int i)
{
  ncomplex ivalue=-pm5->I3Dsti(ep, ps, pt, ix(i+offs));
  return ivalue;
}
#endif /* USE_GOLEM_MODE */

// Local chord indexing
ncomplex Minor3::evalC(int ep, int i)
{
  ncomplex ivalue=0;
  if (i>=ps) i=i+1;
  if (i>=pt || pt==5) {
    i=i+1;
    if (i==ps) i=i+1;
  }
  ivalue=-pm5->I3Dsti(ep, ps, pt, i);
  return ivalue;
}

/* --------------------------------------------------------
    3-point coefficients rank-2
 * --------------------------------------------------------
 */
// Global chord indexing, golem-like
#ifdef USE_GOLEM_MODE
ncomplex Minor3::A(int ep, int i, int j)
{
  ncomplex ivalue=pm5->I3D2stij(ep, ps, pt, ix(i+offs), ix(j+offs));
  return ivalue;
}

ncomplex Minor3::B(int ep)
{
  ncomplex ivalue=-0.5*pm5->I3Dst(ep, ps, pt);
  return ivalue;
}
#endif /* USE_GOLEM_MODE */

// Local chord indexing
ncomplex Minor3::evalC(int ep, int i, int j)
{
  ncomplex ivalue=0;

  if (i==0 && j==0) {
    ivalue=-0.5*pm5->I3Dst(ep, ps, pt);
  }
  else {
    assert(i!=0 && j!=0); // C01, C02, etc do not exist
    int tmp;
    tswap(i,j,tmp);

    if (i>=ps) i=i+1;
    if (i>=pt || pt==5) {
      i=i+1;
      if (i==ps) i=i+1;
    }
    if (j>=ps) j=j+1;
    if (j>=pt || pt==5) {
      j=j+1;
      if (j==ps) j=j+1;
    }

    ivalue=pm5->I3D2stij(ep, ps, pt, i, j);
  }
  return ivalue;
}

/* --------------------------------------------------------
    3-point coefficients rank-3
 * --------------------------------------------------------
 */
// Global chord indexing, golem-like
#ifdef USE_GOLEM_MODE
ncomplex Minor3::A(int ep, int i, int j, int k)
{
  ncomplex ivalue=-pm5->I3D3stijk(ep, ps, pt, ix(i+offs), ix(j+offs), ix(k+offs));
  return ivalue;
}

ncomplex Minor3::B(int ep, int k)
{
  ncomplex ivalue=0.5*pm5->I3D2sti(ep, ps, pt, ix(k+offs));
  return ivalue;
}
#endif /* USE_GOLEM_MODE */

// Local chord indexing
ncomplex Minor3::evalC(int ep, int i, int j, int k)
{
  ncomplex ivalue=0;

  if (i==0 && j==0) {
    assert(i==0 && j==0 && k!=0); // C000 does not exist, C100 is a wrong syntax
    if (k>=ps) k=k+1;
    if (k>=pt || pt==5) {
      k=k+1;
      if (k==ps) k=k+1;
    }

    ivalue=0.5*pm5->I3D2sti(ep, ps, pt, k);
  }
  else {
    assert(i!=0 && j!=0 && k!=0); // D110, D012, etc do not exist
    if (i>=ps) i=i+1;
    if (i>=pt || pt==5) {
        i=i+1;
        if (i==ps) i=i+1;
    }
    if (j>=ps) j=j+1;
    if (j>=pt || pt==5) {
        j=j+1;
        if (j==ps) j=j+1;
    }
    if (k>=ps) k=k+1;
    if (k>=pt || pt==5) {
        k=k+1;
        if (k==ps) k=k+1;
    }
    ivalue=-pm5->I3D3stijk(ep, ps, pt, i, j, k);
  }
  return ivalue;
}

/* ========================================================
 * ========================================================
 *
 *             BUBBLE coefficients - 2 point
 *
 * ========================================================
 * ========================================================
 */
/* --------------------------------------------------------
    2-point scalar for GolemMode
 * --------------------------------------------------------
 */
#ifdef USE_GOLEM_MODE
ncomplex Minor2::A(int ep)
{
  ncomplex ivalue=pm5->I2stu(ep, ps, pt, pu);
  return ivalue;
}
#endif /* USE_GOLEM_MODE */

/* --------------------------------------------------------
    2-point coefficients rank-1
 * --------------------------------------------------------
 */
#ifdef USE_GOLEM_MODE
ncomplex Minor2::A(int ep, int i)
{
  ncomplex ivalue=-pm5->I2Dstui(ep, ps, pt, pu, ix(i+offs));
  return ivalue;
}
#endif /* USE_GOLEM_MODE */

// Local chord indexing
ncomplex Minor2::evalB(int ep, int i)
{
  ncomplex ivalue=0;

  if (i>=ps) i=i+1;
  if (i>=pt) {
    i=i+1;
    if (i==ps) i=i+1;
  }
  if (i>=pu || pu==5) {
    i=i+1;
    if (i==ps) i=i+1;
    if (i==pt) i=i+1;
  }
  ivalue=-pm5->I2Dstui(ep, ps, pt, pu, i);
  return ivalue;
}

/* --------------------------------------------------------
    2-point coefficients rank-2
 * --------------------------------------------------------
 */
#ifdef USE_GOLEM_MODE
ncomplex Minor2::A(int ep, int i, int j)
{
  ncomplex ivalue=pm5->I2D2stuij(ep, ps, pt, pu, ix(i+offs), ix(j+offs));
  return ivalue;
}

ncomplex Minor2::B(int ep)
{
  ncomplex ivalue=-0.5*pm5->I2Dstu(ep, ps, pt, pu);
  return ivalue;
}
#endif /* USE_GOLEM_MODE */

// Local chord indexing
ncomplex Minor2::evalB(int ep, int i, int j)
{
  ncomplex ivalue=0;

  if (i==0 && j==0) {
    ivalue=-0.5*pm5->I2Dstu(ep, ps, pt, pu);
  }
  else {
    assert(i!=0 && j!=0); // B01, B02, etc do not exist
    if (i>=ps) i=i+1;
    if (i>=pt) {
      i=i+1;
      if (i==ps) i=i+1;
    }
    if (i>=pu || pu==5) {
      i=i+1;
      if (i==ps) i=i+1;
      if (i==pt) i=i+1;
    }
    ivalue=pm5->I2D2stuij(ep, ps, pt, pu, i, i);
  }
  return ivalue;
}

