/*
 * minor.cpp - constructors, signed minors and integrals
 *
 * this file is part of PJFry library
 * Copyright 2011 Valery Yundin
 */

#include "minor.h"
#include "cache.h"

const unsigned char MinorBase::ti2[8]={0, 1, 3, 6, 10, 15, 21, 28};
const unsigned char MinorBase::ti3[8]={0, 1, 4, 10, 20, 35, 56, 84};
const unsigned char MinorBase::ti4[8]={0, 1, 5, 15, 35, 70, 126, 210};
const unsigned char MinorBase::ti5[8]={0, 1, 6, 21, 56, 126, 252, 0};//462};

const double MinorBase::teps=1e-14;
const double MinorBase::heps=1e-15;

const double MinorBase::ceps=5e-2;

const double MinorBase::deps1=5e-2;
const double MinorBase::deps2=5e-2;
const double MinorBase::deps3=5e-2;

const double MinorBase::seps1=1e-8;
const double MinorBase::seps2=1e-5;

const double MinorBase::epsir1=5e-6;
const double MinorBase::epsir2=5e-6; // m.b. 5e-5 is better

double MinorBase::deps=1e-14;
double MinorBase::meps=1e-10;
double MinorBase::m3eps=1;

void MinorBase::Rescale(double factor)
{
  meps*=factor;
  m3eps*=factor*factor;
}

const unsigned char MinorBase::idxtbl[64]={
  0,     //     0    0b0
  0,     //0    1    0b1
  1,     //1    2    0b10
  0,     //01   3    0b11
  2,     //2    4    0b100
  1,     //02   5    0b101
  5,     //12   6    0b110
  0,     //012  7    0b111
  3,     //3    8    0b1000
  2,     //03   9    0b1001
  6,     //13   10   0b1010
  1,     //013  11   0b1011
  9,     //23   12   0b1100
  4,     //023  13   0b1101
  10,    //123  14   0b1110
  104,   //     15   0b1111
  4,     //4    16   0b10000
  3,     //04   17   0b10001
  7,     //14   18   0b10010
  2,     //014  19   0b10011
  10,    //24   20   0b10100
  5,     //024  21   0b10101
  11,    //124  22   0b10110
  104,   //     23   0b10111
  12,    //34   24   0b11000
  7,     //034  25   0b11001
  13,    //134  26   0b11010
  104,   //     27   0b11011
  16,    //234  28   0b11100
  104,   //     29   0b11101
  104,   //     30   0b11110
  105,   //     31   0b11111
  5,     //5    32   0b100000
  4,     //05   33   0b100001
  8,     //15   34   0b100010
  3,     //015  35   0b100011
  11,    //25   36   0b100100
  6,     //025  37   0b100101
  12,    //125  38   0b100110
  104,   //     39   0b100111
  13,    //35   40   0b101000
  8,     //035  41   0b101001
  14,    //135  42   0b101010
  104,   //     43   0b101011
  17,    //235  44   0b101100
  104,   //     45   0b101101
  104,   //     46   0b101110
  105,   //     47   0b101111
  14,    //45   48   0b110000
  9,     //045  49   0b110001
  15,    //145  50   0b110010
  104,   //     51   0b110011
  18,    //245  52   0b110100
  104,   //     53   0b110101
  104,   //     54   0b110110
  105,   //     55   0b110111
  19,    //345  56   0b111000
  104,   //     57   0b111001
  104,   //     58   0b111010
  105,   //     59   0b111011
  104,   //     60   0b111100
  105,   //     61   0b111101
  105,   //     62   0b111110
  255,   //     63   0b111111
};

// Find first three indices which are not occupied by set[] and put them in free[]
void MinorBase::freeidxM3(int set[], int free[])
{
  free[0]=0;
  free[1]=1;
  free[2]=2;

  int ic=0;
  int nc=0;
  while (ic < 3 && nc < 3) {
    if (free[nc]==set[ic]) {
      for (int cc=nc; cc<3; cc++) {
        free[cc]++;
      }
      ic++;
    } else {
      nc++;
    }
  }
}

/* ------------------------------------------------------------
* ------------------------------------------------------------
*                       Minor2 section
* ------------------------------------------------------------
* ------------------------------------------------------------
*/

// Constructor from higher rank minor
Minor2::Minor2(const Kinem2& k, Minor5::Ptr mptr5, int s, int t, int u, int is)
              : kinem(k), pm5(mptr5), ps(s), pt(t), pu(u), offs(is)
{
//   printf("Minor2 from Minor5\n");
}

/* ========================================================
 * ========================================================
 *
 *                Minor3 section
 *
 * ========================================================
 * ========================================================
 */

// Constructor from higher rank minor
Minor3::Minor3(const Kinem3& k, Minor5::Ptr mptr5, int s, int t, int is)
              : kinem(k), pm5(mptr5), ps(s), pt(t), offs(is)
{
//   printf("Minor3 from Minor5\n");
}

/* ========================================================
 * ========================================================
 *
 *                Minor4 section
 *
 * ========================================================
 * ========================================================
 */

// Direct construction is proxied through dummy Minor5
// see MCache::getMinor4 in cache.cpp

// Constructor from higher rank minor
Minor4::Minor4(const Kinem4 &k, Minor5::Ptr mptr5, int s, int is)
              : kinem(k), pm5(mptr5), ps(s), offs(is)
{
//   printf("Minor4 from Minor5\n");
}

/* ========================================================
 * ========================================================
 *
 *                Minor5 section
 *
 * ========================================================
 * ========================================================
 */

/* --------------------------------------------------------
 *    Minors with 4 scratched lines
 * --------------------------------------------------------
 */
double Minor5::M4ii(int u, int v, int i)
{
  return (Cay[nss(u,u)]*Cay[nss(v,v)]-Cay[nss(u,v)]*Cay[nss(u,v)]);
}

double Minor5::M4ui(int u, int v, int i)
{
  return (Cay[nss(u,v)]*Cay[ns (i,v)]-Cay[ns (i,u)]*Cay[nss(v,v)]);
}

double Minor5::M4vi(int u, int v, int i)
{
  return (Cay[nss(u,v)]*Cay[ns (i,u)]-Cay[ns (i,v)]*Cay[nss(u,u)]);
}

double Minor5::M4uu(int u, int v, int i)
{
  return (Cay[nss(i,i)]*Cay[nss(v,v)]-Cay[ns (i,v)]*Cay[ns (i,v)]);
}

double Minor5::M4vu(int u, int v, int i)
{
  return (Cay[ns (i,v)]*Cay[ns (i,u)]-Cay[ns (u,v)]*Cay[nss(i,i)]);
}

double Minor5::M4vv(int u, int v, int i)
{
  return (Cay[nss(i,i)]*Cay[nss(u,u)]-Cay[ns (i,u)]*Cay[ns (i,u)]);
}


/* --------------------------------------------------------
 *    Preprocessor definitions
 * --------------------------------------------------------
 */
#define k5s1 s12,p3,p4,p5,s45,s34,m2,m3,m4,m5
#define k5s2 p1,s23,p4,p5,s45,s15,m1,m3,m4,m5
#define k5s3 p1,p2,s34,p5,s12,s15,m1,m2,m4,m5
#define k5s4 p1,p2,p3,s45,s12,s23,m1,m2,m3,m5
#define k5s5 p2,p3,p4,s15,s23,s34,m2,m3,m4,m1

#define k5st12 s45,p4, p5, m3,m4,m5
#define k5st13 s12,s34,p5, m2,m4,m5
#define k5st14 s12,p3, s45,m2,m3,m5
#define k5st15 p3, p4, s34,m3,m4,m2
#define k5st23 p1, s15,p5, m1,m4,m5
#define k5st24 p1, s23,s45,m1,m3,m5
#define k5st25 s23,p4, s15,m3,m4,m1
#define k5st34 p1, p2, s12,m1,m2,m5
#define k5st35 p2, s34,s15,m2,m4,m1
#define k5st45 p2, p3, s23,m2,m3,m1

#define k5stu123  p5, m4, m5
#define k5stu124 s45, m3, m5
#define k5stu125  p4, m4, m3
#define k5stu134 s12, m2, m5
#define k5stu135 s34, m4, m2
#define k5stu145  p3, m3, m2
#define k5stu234  p1, m1, m5
#define k5stu235 s15, m4, m1
#define k5stu245 s23, m3, m1
#define k5stu345  p2, m2, m1

#define m5create4(s) \
{ \
  Kinem4 k4=Kinem4(k5s##s); \
  Minor4::Ptr minor=Minor4::create(k4,self,s, offs); \
  MCache::insertMinor4(k4,minor); \
}

#define m5create3(s,t) \
{ \
  Kinem3 k3=Kinem3(k5st##s##t); \
  Minor3::Ptr minor=Minor3::create(k3,self,s,t, offs); \
  MCache::INSERTMINOR3(k3,minor); \
}

#define m5create2(s,t,u) \
{ \
  Kinem2 k2=Kinem2(k5stu##s##t##u); \
  Minor2::Ptr minor=Minor2::create(k2,self,s,t,u, offs); \
  MCache::INSERTMINOR2(k2,minor); \
}

/* --------------------------------------------------------
 *    Real 5-point kinematics
 * --------------------------------------------------------
 */
Minor5::Minor5(const Kinem5& k) : kinem(k), smax(5), pmaxS4(), pmaxS3()
{
#ifdef USE_GOLEM_MODE_6
  psix=6;
#endif
  const double p1=kinem.p1();
  const double p2=kinem.p2();
  const double p3=kinem.p3();
  const double p4=kinem.p4();
  const double p5=kinem.p5();
  const double s12=kinem.s12();
  const double s23=kinem.s23();
  const double s34=kinem.s34();
  const double s45=kinem.s45();
  const double s15=kinem.s15();
  const double m1=kinem.m1();
  const double m2=kinem.m2();
  const double m3=kinem.m3();
  const double m4=kinem.m4();
  const double m5=kinem.m5();

  Cay[ 0]=2*m1;
  Cay[ 1]=m1+m2-p2;  Cay[ 2]=2*m2;
  Cay[ 3]=m1+m3-s23; Cay[ 4]=m2+m3-p3;  Cay[ 5]=2*m3;
  Cay[ 6]=m1+m4-s15; Cay[ 7]=m2+m4-s34; Cay[ 8]=m3+m4-p4;  Cay[ 9]=2*m4;
  Cay[10]=m1+m5-p1;  Cay[11]=m2+m5-s12; Cay[12]=m3+m5-s45; Cay[13]=m4+m5-p5; Cay[14]=2*m5;

  // create subkinematics minors
  Ptr self=Ptr(this);
  const int offs=0;

  m5create4(1);
  m5create4(2);
  m5create4(3);
  m5create4(4);
  m5create4(5);

  m5create3(1,2);
  m5create3(1,3);
  m5create3(1,4);
  m5create3(1,5);
  m5create3(2,3);
  m5create3(2,4);
  m5create3(2,5);
  m5create3(3,4);
  m5create3(3,5);
  m5create3(4,5);

  m5create2(1,2,3);
  m5create2(1,2,4);
  m5create2(1,2,5);
  m5create2(1,3,4);
  m5create2(1,3,5);
  m5create2(1,4,5);
  m5create2(2,3,4);
  m5create2(2,3,5);
  m5create2(2,4,5);
  m5create2(3,4,5);

  maxCay(); // triggers chain of evalM1, evalM2 and evalM3
}

/* --------------------------------------------------------
 *    Dummy 5-from-4 kinematics
 * --------------------------------------------------------
 */
Minor5::Minor5(const Kinem4& k) : smax(1), pmaxS4(), pmaxS3()
{
#ifdef USE_GOLEM_MODE_6
  psix=6;
#endif
//  12 pinched dummy 5-point kinematics
  const double p3=k.p2();
  const double p4=k.p3();
  const double p5=k.p4();
  const double s12=k.p1();
  const double s34=k.s23();
  const double s45=k.s12();
  const double m2=k.m1();
  const double m3=k.m2();
  const double m4=k.m3();
  const double m5=k.m4();
  kinem=Kinem5(0.,0.,p3,p4,p5,s12,0.,s34,s45,0.,0.,m2,m3,m4,m5);

  Cay[ 0]=0;
  Cay[ 1]=m2;  Cay[ 2]=2*m2;
  Cay[ 3]=m3;  Cay[ 4]=m2+m3-p3;  Cay[ 5]=2*m3;
  Cay[ 6]=m4;  Cay[ 7]=m2+m4-s34; Cay[ 8]=m3+m4-p4;  Cay[ 9]=2*m4;
  Cay[10]=m5;  Cay[11]=m2+m5-s12; Cay[12]=m3+m5-s45; Cay[13]=m4+m5-p5; Cay[14]=2*m5;

  // create subkinematics minors
  Ptr self=Ptr(this);
  const int offs=1;

  m5create4(1);

  m5create3(1,2);
  m5create3(1,3);
  m5create3(1,4);
  m5create3(1,5);

  m5create2(1,2,3);
  m5create2(1,2,4);
  m5create2(1,2,5);
  m5create2(1,3,4);
  m5create2(1,3,5);
  m5create2(1,4,5);

  maxCay(); // triggers chain of evalM1, evalM2 and evalM3
}

#undef m5create4
#undef m5create3
#undef m5create2

/* --------------------------------------------------------
 *
 *                5-point signed minors
 *
 * --------------------------------------------------------
 */

void Minor5::maxCay()
{
  for (int i=1; i<=DCay-1; i++) {
    for (int ip=i+1; ip<=DCay-1; ip++) {
      const double m1=kinem.mass(i);
      const double m2=kinem.mass(ip);
      const double maxM = m1>m2 ? m1 : m2;
      pmaxM2[im2(i,ip)-5] = maxM>meps ? maxM : meps; // NOTE meps depends on mu2 scale
    }
  }

  for (int i=1; i<=DCay-1; i++) {
    for (int j=i; j<=DCay-1; j++) {
      const double cay=fabs(Cay[nss(i,j)]);
      for (int s=1; s<=smax; s++) {
        if (s==i || s==j) continue;
        if (pmaxS4[s-1] < cay) pmaxS4[s-1]=cay;
        for (int t=s+1; t<=DCay-1; t++) {
          if (t==i || t==j) continue;
          const int idx = im2(s,t)-5;
          if (pmaxS3[idx] < cay ) pmaxS3[idx]=cay;
        }
      }
    }
  }
  if (not fEval[E_M1] ) {
    evalM1();
  }
  // Normalize with |G|/|S|
  for (int s=1; s<=smax; s++) {
    pmaxS4[s-1]=fabs(pmaxS4[s-1]*M1(s,s)/M2(0,s,0,s));
    for (int t=s+1; t<=DCay-1; t++) {
      const int idx=im2(s,t)-5;

      int i=0;
      double dsits0t=0;
      for (int ii=1; ii<=DCay-1; ii++) {
        if (i==s || i==t) continue;
        const double val=fabs(M3(0,s,t,ii,s,t));
        if (val > dsits0t) {
          dsits0t=val;
          i=ii;
        }
      }
      imax3[idx]=i;

      const double maxcay3=pmaxS3[idx];
      const double dstst=M2(s,t,s,t);
      const double ds0ts0t=M3(0,s,t,0,s,t);

      pmaxS3[idx]=fabs((maxcay3*dstst)/ds0ts0t);
      pmaxT3[idx]=fabs(ds0ts0t/(maxcay3*M3(0,s,t,i,s,t)));
      pmaxU3[idx]=fabs(dstst/M3(0,s,t,i,s,t));
    }
  }
}

/* --------------------------------------------------------
    Return one-index minor with proper sign
 * --------------------------------------------------------
 */
double Minor5::M1(int i, int l)
{
  return pM1[is(i,l)];
}

/* --------------------------------------------------------
    Return two-index minor with proper sign
 * --------------------------------------------------------
 */
double Minor5::M2(int i, int j, int l, int m)
{
  int sign=signM2ud(i,j,l,m);
  if (sign==0) return 0;

  int uidx=im2(i,j);
  int lidx=im2(l,m);

  return pM2[is(uidx,lidx)]*sign;
}

/* --------------------------------------------------------
  Return three-index minor with proper sign
* --------------------------------------------------------
*/
double Minor5::M3(int i, int j, int k, int l, int m, int n)
{
  int sign=signM3ud(i,j,k,l,m,n);
  if (sign==0) return 0;

  int uidx=im3(i,j,k);
  int lidx=im3(l,m,n);

  return pM3[is(uidx,lidx)]*sign;
}

/* --------------------------------------------------------
  Evaluate all 15 one-index minors (need 2-idx minors)
* --------------------------------------------------------
*/
void Minor5::evalM1()
{
  if (not fEval[E_M2] ) {
    evalM2();
  }
#ifndef NDEBUG
  for (int i=0; i<=DCay-1; i++) {
    for (int l=0; l<=i; l++) {
      pM1[iss(l,i)]=std::numeric_limits<double>::quiet_NaN();
    }
  }
#endif
//   for (int i=0; i<=0; i++)
  {
    const int i=0;
//     for (int l=0; l<=i; l++) {
    {
      const int l=0;
//       int j = i==0 ? 1 : 0;
      const int j=1;

      double m1ele=0;
      for (int m=1; m<=DCay-1; m++) {
        m1ele+=M2(i,j,l,m)*Cay[nss(j,m)];
      }
      pM1[is(i,l)]=m1ele;
    }
  }
  const int j=0;
  for (int i=1; i<=smax; i++) {
    for (int l=0; l<=i; l++) {
      double m1ele=0;
      for (int m=1; m<=DCay-1; m++) {
        m1ele+=M2(i,j,l,m);
      }
      pM1[iss(l,i)]=m1ele;
    }
  }
  fEval[E_M1]=true;
}

/* --------------------------------------------------------
  Evaluate Gram3 with the least precision loss
* --------------------------------------------------------
*/
double Minor5::gram3(double p1, double p2, double p3)
{
  double g3;
  if (fabs(p1) > fabs(p2)) {
    if (fabs(p1) > fabs(p3)) {
      const double diff=(p1 - p2 - p3);
      const double subs=(-4.)*p2*p3;
      g3=diff*diff+subs;
    }
    else {
      const double diff=(p3 - p2 - p1);
      const double subs=(-4.)*p2*p1;
      g3=diff*diff+subs;
    }
  }
  else {
    if (fabs(p2) > fabs(p3)) {
      const double diff=(p2 - p1 - p3);
      const double subs=(-4.)*p1*p3;
      g3=diff*diff+subs;
    }
    else {
      const double diff=(p3 - p2 - p1);
      const double subs=(-4.)*p2*p1;
      g3=diff*diff+subs;
    }
  }
  return g3;
}

/* --------------------------------------------------------
    Evaluate all 120 two-index minors (need 3-idx minors)
 * --------------------------------------------------------
 */
void Minor5::evalM2()
{
  if (not fEval[E_M3] ) {
    evalM3();
  }
#ifndef NDEBUG
  for (int i=0; i<=DCay-1; i++) {
  for (int j=i+1; j<=DCay-1; j++) {
    const int uidx=im2(i,j);
    for (int l=0; l<=DCay-1; l++) {
    for (int m=l+1; m<=DCay-1; m++) {
      int lidx=im2(l,m);
      if (lidx > uidx) continue;
      pM2[is(uidx,lidx)]=std::numeric_limits<double>::quiet_NaN();
    }
    }
  }
  }
#endif
//   for (int i=0; i<=0; i++)
  {
  const int i=0;
  for (int j=i+1; j<=DCay-1; j++) {
    const int uidx=im2(i,j);
    const int k = (j==1 ? 2 : 1);
    for (int l=0; l<=smax; l++) {
    for (int m=l+1; m<=DCay-1; m++) {
      int lidx=im2(l,m);
      if (lidx > uidx) continue;

      double m2ele=M3(i,j,k,l,m,0);
      for (int n=1; n<DCay; n++) {
        m2ele+=M3(i,j,k,l,m,n)*Cay[ns(k,n)];
      }
      pM2[is(uidx,lidx)]=m2ele;
    }
    }
  }
  }
  const int k=0;
  for (int i=1; i<=smax; i++) {
  for (int j=i+1; j<=DCay-1; j++) {
    const int uidx=im2(i,j);
    for (int l=0; l<=smax; l++) {
    for (int m=l+1; m<=DCay-1; m++) {
      int lidx=im2(l,m);
      if (lidx > uidx) continue;

      double m2ele=0;
      for (int n=1; n<DCay; n++) {
        m2ele+=M3(i,j,k,l,m,n);
      }
      pM2[is(uidx,lidx)]=m2ele;
    }
    }
  }
  }
  fEval[E_M2]=true;
}

/* --------------------------------------------------------
    Evaluate all 210 three-index minors
 * --------------------------------------------------------
 */
void Minor5::evalM3()
{
#ifndef NDEBUG
  for (int i=0; i<=DCay-1; i++) {
  for (int j=i+1; j<=DCay-1; j++) {
  for (int k=j+1; k<=DCay-1; k++) {
    const int uidx=im3(i,j,k);
    for (int l=0; l<=DCay-1; l++) {
    for (int m=l+1; m<=DCay-1; m++) {
    for (int n=m+1; n<=DCay-1; n++) {
      int lidx=im3(l,m,n);
      if (lidx > uidx) continue;
      pM3[is(uidx,lidx)]=std::numeric_limits<double>::quiet_NaN();
    }
    }
    }
  }
  }
  }
#endif
//   for (int i=0; i<=0; i++) {
  {
  const int i=0;
  for (int j=i+1; j<=DCay-2; j++) {
  for (int k=j+1; k<=DCay-1; k++) {
    const int uidx=im3(i,j,k);
//     for (int l=0; l<=0; l++) {
    {
    const int l=0;
    for (int m=l+1; m<=DCay-2; m++) {
    for (int n=m+1; n<=DCay-1; n++) {
      int lidx=im3(l,m,n);
      if (lidx > uidx) continue;

      int iu[3]={i,j,k};
      int nu[3];
      freeidxM3(iu, nu);

      int id[3]={l,m,n};
      int nd[3];
      freeidxM3(id, nd);

      int powsign=-2*((i+j+k+l+m+n)&0x1)+1;

      // nu[0]!=0 and nd[0]!=0
      pM3[is(uidx,lidx)]=powsign*(
        + (+Kay(nu[0],nd[1])*Kay(nu[1],nd[2])
           -Kay(nu[0],nd[2])*Kay(nu[1],nd[1]))*Kay(nu[2],nd[0])
        + (+Kay(nu[0],nd[2])*Kay(nu[1],nd[0])
           -Kay(nu[0],nd[0])*Kay(nu[1],nd[2]))*Kay(nu[2],nd[1])
        + (+Kay(nu[0],nd[0])*Kay(nu[1],nd[1])
           -Kay(nu[0],nd[1])*Kay(nu[1],nd[0]))*Kay(nu[2],nd[2])
        );
    }
    }
    }
    for (int l=1; l<=smax; l++) {
    for (int m=l+1; m<=DCay-2; m++) {
    for (int n=m+1; n<=DCay-1; n++) {
      int lidx=im3(l,m,n);
      if (lidx > uidx) continue;

      int iu[3]={i,j,k};
      int nu[3];
      freeidxM3(iu, nu);

      int id[3]={l,m,n};
      int nd[3];
      freeidxM3(id, nd);

      int powsign=-2*((i+j+k+l+m+n)&0x1)+1;

      // nu[0]!=0 and nd[0]==0
      pM3[is(uidx,lidx)]=powsign*(
        + (+Kay(nu[0],nd[1])*Kay(nu[1],nd[2])
           -Kay(nu[0],nd[2])*Kay(nu[1],nd[1]))
        + (+Kay(nu[0],nd[2])
           -Kay(nu[1],nd[2]))*Kay(nu[2],nd[1])
        + (+Kay(nu[1],nd[1])
           -Kay(nu[0],nd[1]))*Kay(nu[2],nd[2])
        );
    }
    }
    }
  }
  }
  }

  for (int i=1; i<=smax; i++) {
  for (int j=i+1; j<=DCay-2; j++) {
  for (int k=j+1; k<=DCay-1; k++) {
    const int uidx=im3(i,j,k);
//     for (int l=0; l<=0; l++) {
    {
    const int l=0;
    for (int m=l+1; m<=DCay-2; m++) {
    for (int n=m+1; n<=DCay-1; n++) {
      int lidx=im3(l,m,n);
      if (lidx > uidx) continue;

      int iu[3]={i,j,k};
      int nu[3];
      freeidxM3(iu, nu);

      int id[3]={l,m,n};
      int nd[3];
      freeidxM3(id, nd);

      int powsign=-2*((i+j+k+l+m+n)&0x1)+1;

      // nu[0]==0 and nd[0]!=0
      pM3[is(uidx,lidx)]=powsign*(
        + (+Kay(nu[1],nd[2])
           -Kay(nu[1],nd[1]))*Kay(nu[2],nd[0])
        + (+Kay(nu[1],nd[0])
           -Kay(nu[1],nd[2]))*Kay(nu[2],nd[1])
        + (+Kay(nu[1],nd[1])
           -Kay(nu[1],nd[0]))*Kay(nu[2],nd[2])
        );
    }
    }
    }
    for (int l=1; l<=smax; l++) {
    for (int m=l+1; m<=DCay-2; m++) {
    for (int n=m+1; n<=DCay-1; n++) {
      int lidx=im3(l,m,n);
      if (lidx > uidx) continue;

      int iu[3]={i,j,k};
      int nu[3];
      freeidxM3(iu, nu);

      int id[3]={l,m,n};
      int nd[3];
      freeidxM3(id, nd);

      int powsign=-2*((i+j+k+l+m+n)&0x1)+1;

      // nu[0]==0 and nd[0]==0
      pM3[is(uidx,lidx)]=powsign*(
        + Kay(nu[1],nd[2])
        - Kay(nu[1],nd[1])
        + Kay(nu[2],nd[1])
        - Kay(nu[2],nd[2])
        );
    }
    }
    }
  }
  }
  }
  fEval[E_M3]=true;
}

/* --------------------------------------------------------
 *
 *                Plain scalar integrals
 *
 * --------------------------------------------------------
 */

/* --------------------------------------------------------
    I4s box
 * --------------------------------------------------------
 */
ncomplex Minor5::I4s(int ep, int s)
{
  if (not fEval[E_I4s+ep]) {
    I4sEval(ep);
  }
  return pI4s[ep][s-1];
}
void Minor5::I4sEval(int ep) // IR-div
{
  // Kinematics is in LT notation (for calling qcdloop)
  double p1=kinem.p1();
  double p2=kinem.p2();
  double p3=kinem.p3();
  double p4=kinem.p4();
  double p5=kinem.p5();
  double s12=kinem.s12();
  double s23=kinem.s23();
  double s34=kinem.s34();
  double s45=kinem.s45();
  double s15=kinem.s15();
  double m1=kinem.m1();
  double m2=kinem.m2();
  double m3=kinem.m3();
  double m4=kinem.m4();
  double m5=kinem.m5();

  pI4s[ep][1-1]=ICache::getI4(ep, Kinem4(s12,p3,p4,p5,s45,s34,m5,m2,m3,m4));
  if (smax==5) {
    pI4s[ep][2-1]=ICache::getI4(ep, Kinem4(p1,s23,p4,p5,s45,s15,m5,m1,m3,m4));
    pI4s[ep][3-1]=ICache::getI4(ep, Kinem4(p1,p2,s34,p5,s12,s15,m5,m1,m2,m4));
    pI4s[ep][4-1]=ICache::getI4(ep, Kinem4(p1,p2,p3,s45,s12,s23,m5,m1,m2,m3));
    pI4s[ep][5-1]=ICache::getI4(ep, Kinem4(p2,p3,p4,s15,s23,s34,m1,m2,m3,m4));
  }
  fEval[E_I4s+ep]=true;
}

/* --------------------------------------------------------
 *   I3st triangle
 * --------------------------------------------------------
 */
ncomplex Minor5::I3st(int ep, int s, int t) // IR-div
{
  assert(s!=t);
  if (not fEval[E_I3st+ep]) {
    I3stEval(ep);
  }
  int idx = im2(s,t)-5;
  return pI3st[ep][idx];
}

void Minor5::I3stEval(int ep)
{
  // Kinematics is in LT notation (for calling qcdloop)
  double p1=kinem.p1();
  double p2=kinem.p2();
  double p3=kinem.p3();
  double p4=kinem.p4();
  double p5=kinem.p5();
  double s12=kinem.s12();
  double s23=kinem.s23();
  double s34=kinem.s34();
  double s45=kinem.s45();
  double s15=kinem.s15();
  double m1=kinem.m1();
  double m2=kinem.m2();
  double m3=kinem.m3();
  double m4=kinem.m4();
  double m5=kinem.m5();

  // it is symmetric with zeroes on main diagonal
  pI3st[ep][im2(1,2)-5]=ICache::getI3(ep, Kinem3(s45,p4, p5, m5,m3,m4));
  pI3st[ep][im2(1,3)-5]=ICache::getI3(ep, Kinem3(s12,s34,p5, m5,m2,m4));
  pI3st[ep][im2(1,4)-5]=ICache::getI3(ep, Kinem3(s12,p3, s45,m5,m2,m3));
  pI3st[ep][im2(1,5)-5]=ICache::getI3(ep, Kinem3(p3, p4, s34,m2,m3,m4));
  if (smax==5) {
    pI3st[ep][im2(2,3)-5]=ICache::getI3(ep, Kinem3(p1, s15,p5, m5,m1,m4));
    pI3st[ep][im2(2,4)-5]=ICache::getI3(ep, Kinem3(p1, s23,s45,m5,m1,m3));
    pI3st[ep][im2(2,5)-5]=ICache::getI3(ep, Kinem3(s23,p4, s15,m1,m3,m4));
    pI3st[ep][im2(3,4)-5]=ICache::getI3(ep, Kinem3(p1, p2, s12,m5,m1,m2));
    pI3st[ep][im2(3,5)-5]=ICache::getI3(ep, Kinem3(p2, s34,s15,m1,m2,m4));
    pI3st[ep][im2(4,5)-5]=ICache::getI3(ep, Kinem3(p2, p3, s23,m1,m2,m3));
  }
  fEval[E_I3st+ep]=true;
}

/* --------------------------------------------------------
 *   I2stu bubble
 * --------------------------------------------------------
 */
ncomplex Minor5::I2stu(int ep, int s, int t, int u)
{
  assert(t!=u && u!=s && s!=t);
  if (ep>=2) return 0;

  if (not fEval[E_I2stu+ep]) {
    I2stuEval(ep);
  }
  int idx=im3(s,t,u)-10;
  return pI2stu[ep][idx];
}

void Minor5::I2stuEval(int ep)
{
  // Kinematics is in LT notation (for calling qcdloop)
  double p1=kinem.p1();
  double p2=kinem.p2();
  double p3=kinem.p3();
  double p4=kinem.p4();
  double p5=kinem.p5();
  double s12=kinem.s12();
  double s23=kinem.s23();
  double s34=kinem.s34();
  double s45=kinem.s45();
  double s15=kinem.s15();
  double m1=kinem.m1();
  double m2=kinem.m2();
  double m3=kinem.m3();
  double m4=kinem.m4();
  double m5=kinem.m5();

  // it is symmetric with zeroes on main diagonal
  pI2stu[ep][im3(1,2,3)-10]=ICache::getI2(ep, Kinem2( p5, m5, m4));
  pI2stu[ep][im3(1,2,4)-10]=ICache::getI2(ep, Kinem2(s45, m5, m3));
  pI2stu[ep][im3(1,2,5)-10]=ICache::getI2(ep, Kinem2( p4, m3, m4));
  pI2stu[ep][im3(1,3,4)-10]=ICache::getI2(ep, Kinem2(s12, m5, m2));
  pI2stu[ep][im3(1,3,5)-10]=ICache::getI2(ep, Kinem2(s34, m2, m4));
  pI2stu[ep][im3(1,4,5)-10]=ICache::getI2(ep, Kinem2( p3, m2, m3));
  if (smax==5) {
    pI2stu[ep][im3(2,3,4)-10]=ICache::getI2(ep, Kinem2( p1, m5, m1));
    pI2stu[ep][im3(2,3,5)-10]=ICache::getI2(ep, Kinem2(s15, m1, m4));
    pI2stu[ep][im3(2,4,5)-10]=ICache::getI2(ep, Kinem2(s23, m1, m3));
    pI2stu[ep][im3(3,4,5)-10]=ICache::getI2(ep, Kinem2( p2, m1, m2));
  }
  fEval[E_I2stu+ep]=true;
}

/* --------------------------------------------------------
 *
 *                Rank-1 functions
 *
 * --------------------------------------------------------
 */

/* --------------------------------------------------------
 *   I4Ds box in D+2 dim
 * --------------------------------------------------------
 */
ncomplex Minor5::I4Ds(int ep, int s)
{
  if (ep!=0) return 0; // I4Ds is finite
  if (not fEval[E_I4Ds+ep]) {
    I4DsEval(ep);
  }
  return pI4Ds[ep][s-1];
}

void Minor5::I4DsEval(const int ep)
{
  assert(ep==0);
  for (int s=1; s<=smax; s++) {
    const double dss=M1(s, s);
    const double d0s0s=M2(0, s, 0, s);
    ncomplex ivalue=0;

    ncomplex sum1=0;
    for (int t=1; t<=5; t++) {
      if (t==s) continue;
      sum1-=M2(s,t,s,0)*I3st(ep, s, t);
    }
    sum1+=d0s0s*I4s(ep, s);
    ivalue=sum1/dss;

    const double x=dss/d0s0s;
    if (pmaxS4[s-1] <= deps1) {
    ncomplex sump;
    do {
      assert(ep==0);

      double dsts0[6];
      sum1=0;
      for (int t=1; t<=5; t++) {
        if (t==s) continue;
        dsts0[t]=M2(s,t,s,0);
        sum1+=dsts0[t]*I3Dst(0, s, t);
      }

      double xn=1;
      ncomplex dv,s21;

      ncomplex sum[3];
      sum[0]=sump=sum1;

#define stepI4D(n,a,b) \
      xn*=x; \
      dv=0; \
      for (int t=1; t<=5; t++) { \
        if (t==s) continue; \
        dv+=dsts0[t]*(a*I3D##n##st(0, s, t) - b*I3D##n##st(1, s, t)); \
      } \
      dv*=xn; \
      sum1+=dv;

      stepI4D(2,3.,2.)
      if (   fabs(sum1.real()*teps)>=fabs(dv.real())
          && fabs(sum1.imag()*teps)>=fabs(dv.imag()))
        break;
      sum[1]=sump=sum1;
      s21=sum[1]-sum[0];

      stepI4D(3,15.,16.)
      sump=sum1;
      stepWynn(0)
      stepI4D(4,105.,142.)
      stepWynn(1)
      stepI4D(5,945.,1488.)
      stepWynn(2)
      stepI4D(6,10395.,18258.)
      stepWynn(3)
      stepI4D(7,135135.,258144.)
      stepWynn(4)
//       stepI4D(8,2027025.,4142430.)
//       stepWynn(5)
//       stepI4D(9,34459425.,74475360.)
//       stepWynn(6)
#undef stepI4D
    } while (0);
    ivalue=sump/d0s0s;
    }
    pI4Ds[ep][s-1]=ivalue;
  }
  fEval[E_I4Ds+ep]=true;
}

/* --------------------------------------------------------
 *   I4Dsi box in D+2 dim with a dot
 * --------------------------------------------------------
 */
ncomplex Minor5::I4Dsi(int ep, int s, int i) // IR-div
{
  if (s==i) return 0;
  if (not fEval[E_I4Dsi+ep]) {
    I4DsiEval(ep);
  }
  return pI4Dsi[ep][i-1][s-1];
}

void Minor5::I4DsiEval(int ep)
{
  for (int s=1; s<=smax; s++) {
  for (int i=1; i<=CIDX; i++) {
    if (s==i) continue;
    ncomplex ivalue=0;

    if (pmaxS4[s-1] <= deps1 || (fabs(M1(s,s))<1e-11 && fabs(M2(0,s,0,s))>0) ) {
      ncomplex sum1=0;
      for (int t=1; t<=5; t++) {
        if (t==s) continue;
        sum1+=M3(0, s, t, 0, s, i)*I3st(ep, s, t);
      }
      sum1-=M2(0, s, i, s)*I4Ds(ep, s);
      ivalue=sum1/M2(0, s, 0, s);
    } else {
      ncomplex sum1=0;
      for (int t=1; t<=5; t++) {
        if (t==s) continue;
        sum1+=M2(s, t, s, i)*I3st(ep, s, t);
      }
      sum1-=M2(0, s, i, s)*I4s(ep, s);
      ivalue=sum1/M1(s, s);
    }
    pI4Dsi[ep][i-1][s-1]=ivalue;
  }
  }
  fEval[E_I4Dsi+ep]=true;
}

/* --------------------------------------------------------
 *
 *                Rank-2 functions
 *
 * --------------------------------------------------------
 */

/* --------------------------------------------------------
 *   I3Dst triangle in D+2 dim
 * --------------------------------------------------------
 */
ncomplex Minor5::I3Dst(int ep, int s, int t)
{
  assert(s!=t);
  if      (ep==1) return -0.5;
  else if (ep==2) return 0;
  if (not fEval[E_I3Dst+ep]) {
    I3DstEval(ep);
  }
  int idx = im2(s,t)-5;
  return pI3Dst[ep][idx];
}

void Minor5::I3DstEval(int ep)
{
  assert(ep==0);
  for (int s=1; s<=smax; s++) {
  for (int t=s+1; t<=5; t++) {
    int idx = im2(s,t)-5;
    const double dstst=M2(s,t,s,t);
    const double d0st0st=M3(0,s,t,0,s,t);
    ncomplex ivalue=0;

    if ( pmaxT3[idx]!=0 && ( pmaxT3[idx] <= epsir1 && pmaxU3[idx] <= epsir1 ) ) {
      // IR
      int i=imax3[idx];
      int iu[3]={i-1,s-1,t-1};
      int tmp;
      tswap(iu[0],iu[2],tmp);
      tswap(iu[1],iu[2],tmp);
      tswap(iu[0],iu[1],tmp);
      int nu[3];
      freeidxM3(iu, nu);
      int u=nu[0]+1;
      int v=nu[1]+1;
      const double Dii=M4ii(u,v,i);
      const double Dui=M4ui(u,v,i);
      const double Dvi=M4vi(u,v,i);
      ncomplex sum1=+Dii*(I2stu(0, s, t, i)+I2stu(1, s, t, i))  // (i, i)
                    +Dui*(I2stu(0, s, t, u)+I2stu(1, s, t, u))  // (u, i)
                    +Dvi*(I2stu(0, s, t, v)+I2stu(1, s, t, v)); // (v, i)
      ivalue=0.5*sum1/M3(0,s,t,i,s,t);
    } else if (pmaxS3[idx] <= ceps) {
      // EXP
      const double x=dstst/d0st0st;
      ncomplex sump;
      do {
        double dstust0[6];
        ncomplex sum1=0;
        for (int u=1; u<=5; u++) {
          if (u==t || u==s) continue;
          dstust0[u]=M3(s,t,u,s,t,0);
          sum1+=dstust0[u]*I2Dstu(0, s, t, u);
        }

        double xn=1;
        ncomplex dv,s21;

        ncomplex sum[3];
        sum[0]=sump=sum1;

#define stepI3D(n,a,b) \
        xn*=x; \
        dv=0; \
        for (int u=1; u<=5; u++) { \
          if (u==t || u==s) continue; \
          dv+=dstust0[u]*(a*I2D##n##stu(0, s, t, u) - b*I2D##n##stu(1, s, t, u)); \
        } \
        dv*=xn; \
        sum1+=dv;

        stepI3D(2,4.,2.)
        if (   fabs(sum1.real()*teps)>=fabs(dv.real())
            && fabs(sum1.imag()*teps)>=fabs(dv.imag()))
          break;
        sum[1]=sump=sum1;
        s21=sum[1]-sum[0];

        stepI3D(3,24.,20.)
        sump=sum1;
        stepWynn(0)
        stepI3D(4,192.,208.)
        stepWynn(1)
        stepI3D(5,1920.,2464.)
        stepWynn(2)
        stepI3D(6,23040.,33408.)
        stepWynn(3)
//         stepI3D(7,322560.,513792.)
//         stepWynn(4)
#undef stepI3D
      } while (0);
      ivalue=sump/d0st0st;
    } else {
      // NORMAL
      ncomplex sum1=0;
      for (int u=1; u<=5; u++) {
        if (u==t || u==s) continue;
        sum1-=M3(u,s,t,0,s,t)*I2stu(ep, s, t, u);
      }
      sum1+=d0st0st*I3st(ep, s, t);
      ivalue=sum1/(2*dstst)-0.5; // 2*(-1/2)/2 == -0.5
    }
    pI3Dst[ep][idx]=ivalue;
  }
  }
  fEval[E_I3Dst+ep]=true;
}


/* --------------------------------------------------------
 *   I4D2s box in D+4 dim
 * --------------------------------------------------------
 */
ncomplex Minor5::I4D2s(int ep, int s)
{
  if      (ep==1) return 1./6.;
  else if (ep==2) return 0;
  if (not fEval[E_I4D2s+ep]) {
    I4D2sEval(ep);
  }
  return pI4D2s[ep][s-1];
}

void Minor5::I4D2sEval(int ep) {
  for (int s=1; s<=smax; s++) {
    const double dss=M1(s, s);
    const double d0s0s=M2(0, s, 0, s);
    ncomplex ivalue=0;

    ncomplex sum1=0;
    for (int t=1; t<=5; t++) {
      if (t==s) continue;
      sum1-=M2(s,t,s,0)*I3Dst(ep, s, t);
    }
    sum1+=d0s0s*I4Ds(ep, s);
    ivalue=sum1/(3*dss)+1./9.; // +2*(1/6)/3

    const double x=dss/d0s0s;
    if (pmaxS4[s-1] <= deps2) {
    ncomplex sump;
    do {
      assert(ep==0);

      double dsts0[6];
      sum1=0;
      for (int t=1; t<=5; t++) {
        if (t==s) continue;
        dsts0[t]=M2(s,t,s,0);
        sum1+=dsts0[t]*I3D2st(0, s, t);
      }

      double xn=1;
      ncomplex dv,s21;

      ncomplex sum[3];
      sum[0]=sump=sum1;

#define stepI4D(n,a,b) \
      xn*=x; \
      dv=0; \
      for (int t=1; t<=5; t++) { \
        if (t==s) continue; \
        dv+=dsts0[t]*(a*I3D##n##st(0, s, t) - b*I3D##n##st(1, s, t)); \
      } \
      dv*=xn; \
      sum1+=dv;

      stepI4D(3,5.,2.)
      if (   fabs(sum1.real()*teps)>=fabs(dv.real())
          && fabs(sum1.imag()*teps)>=fabs(dv.imag()))
        break;
      sum[1]=sump=sum1;
      s21=sum[1]-sum[0];

      stepI4D(4,35.,24.)
      sump=sum1;
      stepWynn(0)
      stepI4D(5,315.,286.)
      stepWynn(1)
      stepI4D(6,3465.,3776.)
      stepWynn(2)
      stepI4D(7,45045.,56018.)
      stepWynn(3)
//       stepI4D(8,675675.,930360.)
//       stepWynn(4)
//       stepI4D(9,11486475.,17167470.)
//       stepWynn(5)
#undef stepI4D
    } while (0);
    ivalue=sump/d0s0s;
    }
    pI4D2s[ep][s-1]=ivalue;
  }
  fEval[E_I4D2s+ep]=true;
}

/* --------------------------------------------------------
 *
 *                Rank-3 functions
 *
 * --------------------------------------------------------
 */

/* --------------------------------------------------------
 *   I4D2si box in D+4 dim with a dot
 * --------------------------------------------------------
 */
ncomplex Minor5::I4D2si(int ep, int s, int i)
{
  if (s==i) return 0;
  if (ep!=0) return 0; // I4D2si is finite
  if (not fEval[E_I4D2si+ep]) {
    I4D2siEval(ep);
  }
  return pI4D2si[ep][i-1][s-1];
}

void Minor5::I4D2siEval(int ep)
{
  for (int s=1; s<=smax; s++) {
  for (int i=1; i<=CIDX; i++) {
    if (s==i) continue;
    ncomplex ivalue=0;

    if (pmaxS4[s-1] <= deps2 || (fabs(M1(s,s))<1e-11 && fabs(M2(0,s,0,s))>0) ) {
      ncomplex sum1=0;
      for (int t=1; t<=5; t++) {
        if (t==s) continue;
        sum1+=M3(0, s, t, 0, s, i)*I3Dst(ep, s, t);
      }
      sum1+=M2(0, s, i, s)*(-3.*I4D2s(ep, s)+1./3.); // 1/3 == 2*1/6
      ivalue=sum1/M2(0, s, 0, s);
    } else {
      ncomplex sum1=0;
      for (int t=1; t<=5; t++) {
        if (t==s) continue;
        sum1+=M2(s, t, s, i)*I3Dst(ep, s, t);
      }
      sum1-=M2(0, s, i, s)*I4Ds(ep, s);
      ivalue=sum1/M1(s, s);
    }

    /* // Test for formula 6.11
    const double ds0=M1(s, 0);
    ncomplex sum1=M2(0,s,0,i)*I4Ds(ep, s)-3*M1(s,i)*I4D2s(ep,s);
    sum1+=(ep == 2) ? 0 : 2*M1(s,i)*I4D2s(ep+1,s);
    for (int t=1; t<=5; t++) {
      sum1+=-M2(s,t,i,0)*I3Dst(ep, s, t);
    }
    ncomplex ivalue=sum1/ds0;
    */

    pI4D2si[ep][i-1][s-1]=ivalue;
    }
  }
  fEval[E_I4D2si+ep]=true;
}

/* --------------------------------------------------------
 *   I3Dsti triangle in D+2 dim with a dot
 * --------------------------------------------------------
 */
ncomplex Minor5::I3Dsti(int ep, int s, int t, int i) // IR-div
{
  assert(s!=t && s!=i && t!=i);
  if (not fEval[E_I3Dsti+ep]) {
    I3DstiEval(ep);
  }
  int idx = im2(s,t)-5;
  return pI3Dsti[ep][i-1][idx];
}

void Minor5::I3DstiEval(int ep)
{
  for (int i=1; i<=CIDX; i++) {
  for (int s=1; s<=smax; s++) { if (i==s) continue;
  for (int t=s+1; t<=5; t++) {  if (i==t) continue;
    int idx = im2(s,t)-5;

    const double ds0ts0t=M3(s,0,t,s,0,t);
    if (ep!=0 && fabs(ds0ts0t) > m3eps) { // if ds0ts0t!=0 I3Dsti is finite
      pI3Dsti[ep][i-1][idx]=0;
      continue;
    }

    ncomplex ivalue=0;

    if ( ep!=0 ||
       ( (pmaxT3[idx]==0 || (pmaxT3[idx] > epsir2 || pmaxU3[idx] > epsir2))
      && pmaxS3[idx] > ceps ) ) {
      // Variant with Gram3
      ncomplex sum1=0;
      for (int u=1; u<=5; u++) {
        if (u==t || u==s) continue;
        sum1+=M3(u,s,t,i,s,t)*I2stu(ep,s,t,u);
      }
      sum1-=M3(0,s,t,i,s,t)*I3st(ep, s, t);
      ivalue=sum1/M2(s,t,s,t);
    } else {
      ncomplex sum1=0;
      int iu[3]={i-1,s-1,t-1};
      int tmp;
      tswap(iu[0],iu[2],tmp);
      tswap(iu[1],iu[2],tmp);
      tswap(iu[0],iu[1],tmp);
      int nu[3];
      freeidxM3(iu, nu);
      int u=nu[0]+1;
      int v=nu[1]+1;

      if ( pmaxT3[idx] <= epsir2 && pmaxU3[idx] <= epsir2 ) {
        // small G3 & C3
        assert(ep==0);
        int j=imax3[idx];
        sum1=0;
        ncomplex const I3term=I3st(ep,s,t)+2.*I3st(ep+1,s,t);
        ncomplex const I2Uterm=I2stui(ep,s,t,u,i,v)+2.*I2stui(ep+1,s,t,u,i,v);
        ncomplex const I2Vterm=I2stui(ep,s,t,v,i,u)+2.*I2stui(ep+1,s,t,v,i,u);
        if (j==i) { // j->i
          const double Dii=M4ii(u,v,i);
          const double Dui=M4ui(u,v,i);
          const double Dvi=M4vi(u,v,i);
          sum1+=+Dii*(I3term)            // (i, i)
                +Dui*(I2Uterm)           // (u, i)
                +Dvi*(I2Vterm);          // (v, i)
        } else if (j==u) { // j->u
          const double Dui=M4ui(u,v,i);
          const double Duu=M4uu(u,v,i);
          const double Dvu=M4vu(u,v,i);
          sum1+=+Dui*(I3term)            // (u, i)
                +Duu*(I2Uterm)           // (u, u)
                +Dvu*(I2Vterm);          // (v, u)
        } else { assert(j==v); // j->v
          const double Dvi=M4vi(u,v,i);
          const double Dvu=M4vu(u,v,i);
          const double Dvv=M4vv(u,v,i);
          sum1+=+Dvi*(I3term)            // (v, i)
                +Dvu*(I2Uterm)           // (v, u)
                +Dvv*(I2Vterm);          // (v, v)
        }
        ivalue=sum1/(M3(s,0,t,s,j,t));
      } else {
        // small G3
        assert(ep==0);
        const double Dii=M4ii(u,v,i);
        const double Dui=M4ui(u,v,i);
        const double Dvi=M4vi(u,v,i);
        sum1+=Dii*I2stu(ep,s,t,i)        // (i, i)
             +Dui*I2stu(ep,s,t,u)        // (u, i)
             +Dvi*I2stu(ep,s,t,v);       // (v, i)
        sum1+=M3(s,0,t,s,i,t)*(-2.*I3Dst(ep, s, t)-1.); //+2.*I3Dst(ep+1, s, t));
        ivalue=sum1/ds0ts0t;
      }
    }
    pI3Dsti[ep][i-1][idx]=ivalue;
  }
  }
  }
  fEval[E_I3Dsti+ep]=true;
}

/* --------------------------------------------------------
 *   I4D2sij box in D+4 dim with two dots
 * --------------------------------------------------------
 */
ncomplex Minor5::I4D2sij(int ep, int s, int i, int j) // IR-div
{
  if (s==i || s==j) return 0;
  if (not fEval[E_I4D2sij+ep]) {
    I4D2sijEval(ep);
  }
  return pI4D2sij[ep][is(i-1,j-1)][s-1];
}

void Minor5::I4D2sijEval(int ep)
{
  for (int s=1; s<=smax; s++) {
  // symmetric in 'i,j'
  for (int i=1; i<=CIDX; i++) { if (s==i) continue;
  for (int j=i; j<=CIDX; j++) { if (s==j) continue;
    ncomplex ivalue=0;

    if (pmaxS4[s-1] <= deps2 || (fabs(M1(s,s))<1e-11 && fabs(M2(0,s,0,s))>0) ) {
      ncomplex sum1=0;
      for (int t=1; t<=5; t++) {
        if (t==s || t==i) continue;
        sum1+=M3(s,0,t,s,0,j)*I3Dsti(ep, s, t, i);
      }
      sum1+=M3(s,0,i,s,0,j)*I4Ds(ep, s);
      sum1+=M2(s,0,s,j)*(-2.*I4D2si(ep, s, i));
      ivalue=sum1/M2(0, s, 0, s);
    } else {
      ncomplex sum1=0;
      for (int t=1; t<=5; t++) {
        if (t==s || t==i) continue;
        sum1+=M2(s,t,s,j)*I3Dsti(ep, s, t, i);
      }
      sum1+=M2(s,i,s,j)*I4Ds(ep, s);
      sum1-=M2(s,0,s,j)*I4Dsi(ep, s, i);
      ivalue=sum1/M1(s, s);
    }
    pI4D2sij[ep][iss(i-1,j-1)][s-1]=ivalue;
  }
  }
  }
  fEval[E_I4D2sij+ep]=true;
}

/* --------------------------------------------------------
 *
 *                Rank-4 functions
 *
 * --------------------------------------------------------
 */

/* --------------------------------------------------------
 *   I2Dstu bubble in D+2 dim
 * --------------------------------------------------------
 */
ncomplex Minor5::I2Dstu(int ep, int s, int t, int u)
{
  assert(t!=u && u!=s && s!=t);
  if (ep==2) return 0;
  if (not fEval[E_I2Dstu+ep]) {
    I2DstuEval(0,ep,1,2,3,4,5,kinem.p5());
    I2DstuEval(1,ep,1,2,4,3,5,kinem.s45());
    I2DstuEval(2,ep,1,2,5,3,4,kinem.p4());

    I2DstuEval(3,ep,1,3,4,2,5,kinem.s12());
    I2DstuEval(4,ep,1,3,5,2,4,kinem.s34());

    I2DstuEval(5,ep,1,4,5,2,3,kinem.p3());

    if (smax==5) {
      I2DstuEval(6,ep,2,3,4,1,5,kinem.p1());
      I2DstuEval(7,ep,2,3,5,1,4,kinem.s15());

      I2DstuEval(8,ep,2,4,5,1,3,kinem.s23());


      I2DstuEval(9,ep,3,4,5,1,2,kinem.p2());
    }

    fEval[E_I2Dstu+ep]=true;
  }
  int idx=im3(s,t,u)-10;
  return pI2Dstu[ep][idx];
}

void Minor5::I2DstuEval(int idx, int ep, int s, int t, int u, int m, int n, double qsq)
{
  ncomplex sum1=0;
  if (ep==0) {
    const double dstustu=-2*qsq; /*M3(s,t,u,s,t,u);*/

    const double msq1=kinem.mass(m);
    const double msq2=kinem.mass(n);
    const double s_cutoff=seps1*pmaxM2[im2(m,n)-5];

    if (fabs(dstustu) <= s_cutoff) {
      const double mm12=msq1-msq2;
      if (fabs(mm12) < meps) {
        sum1=-ICache::getI1(ep, Kinem1(msq1));
      }
      else {
        sum1= - 0.25*( msq1 + msq2 )
              + 0.5*( - msq1*ICache::getI1(ep, Kinem1(msq1))
                      + msq2*ICache::getI1(ep, Kinem1(msq2))
                      )/(mm12);
      }
    }
    else {
      ncomplex sumX=3.*I2stu(ep,s,t,u)+2.*I2stu(ep+1,s,t,u);
      ncomplex sumY=3.*ICache::getI1(ep, Kinem1(msq2))+2*msq2;
      ncomplex sumZ=3.*ICache::getI1(ep, Kinem1(msq1))+2*msq1;

      const double ds0tu=(Cay[nss(m,m)]*Cay[nss(n,n)]-Cay[nss(m,n)]*Cay[nss(m,n)]);
      sum1+=sumX*ds0tu;

      const double dsvtuY=-(Cay[nss(n,n)]-Cay[nss(m,n)]); /* minus sign of minor v=m */
      sum1-=sumY*dsvtuY;

      const double dsvtuZ=+(Cay[nss(m,n)]-Cay[nss(m,m)]); /* plus sign of minor v=n */
      sum1-=sumZ*dsvtuZ;

      sum1/=9*dstustu;
    }
  }
  else { assert(ep==1);
    sum1=-(Cay[nss(m,m)]+Cay[nss(m,n)]+Cay[nss(n,n)])/6.;
  }
  pI2Dstu[ep][idx]=sum1;
}

/* --------------------------------------------------------
 *   I3D2st triangle in D+4 dim
 * --------------------------------------------------------
 */
ncomplex Minor5::I3D2st(int ep, int s, int t)
{
  assert(s!=t);
  if (ep==2) return 0;
  if (not fEval[E_I3D2st+ep]) {
    I3D2stEval(ep);
  }
  int idx = im2(s,t)-5;
  return pI3D2st[ep][idx];
}

void Minor5::I3D2stEval(int ep)
{
  for (int s=1; s<=smax; s++) {
  for (int t=s+1; t<=5; t++) {
    int idx = im2(s,t)-5;
    ncomplex ivalue=0;

    if (ep==0) {
      const double dstst=M2(s,t,s,t);
      const double d0st0st=M3(0,s,t,0,s,t);

      if ( pmaxT3[idx]!=0 && ( pmaxT3[idx] <= epsir1 && pmaxU3[idx] <= epsir1 ) ) {
        // IR
        int i=imax3[idx];
        int iu[3]={i-1,s-1,t-1};
        int tmp;
        tswap(iu[0],iu[2],tmp);
        tswap(iu[1],iu[2],tmp);
        tswap(iu[0],iu[1],tmp);
        int nu[3];
        freeidxM3(iu, nu);
        int u=nu[0]+1;
        int v=nu[1]+1;
        const double Dii=M4ii(u,v,i);
        const double Dui=M4ui(u,v,i);
        const double Dvi=M4vi(u,v,i);
        ncomplex sum1=+Dii*(I2Dstu(0, s, t, i)+0.5*I2Dstu(1, s, t, i))  // (i, i)
                      +Dui*(I2Dstu(0, s, t, u)+0.5*I2Dstu(1, s, t, u))  // (u, i)
                      +Dvi*(I2Dstu(0, s, t, v)+0.5*I2Dstu(1, s, t, v)); // (v, i)
        ivalue=0.25*sum1/M3(0,s,t,i,s,t);
      } else if (pmaxS3[idx] <= ceps) {
        // EXP
        const double x=dstst/d0st0st;
        ncomplex sump;
        do {
          double dstust0[6];
          ncomplex sum1=0;
          for (int u=1; u<=5; u++) {
            if (u==t || u==s) continue;
            dstust0[u]=M3(s,t,u,s,t,0);
            sum1+=dstust0[u]*I2D2stu(0, s, t, u);
          }

          double xn=1;
          ncomplex dv,s21;

          ncomplex sum[3];
          sum[0]=sump=sum1;

#define stepI3D(n,a,b) \
          xn*=x; \
          dv=0; \
          for (int u=1; u<=5; u++) { \
            if (u==t || u==s) continue; \
            dv+=dstust0[u]*(a*I2D##n##stu(0, s, t, u) - b*I2D##n##stu(1, s, t, u)); \
          } \
          dv*=xn; \
          sum1+=dv;

          stepI3D(3,6.,2.)
          if (   fabs(sum1.real()*teps)>=fabs(dv.real())
              && fabs(sum1.imag()*teps)>=fabs(dv.imag()))
            break;
          sum[1]=sump=sum1;
          s21=sum[1]-sum[0];

          stepI3D(4,48.,28.)
          sump=sum1;
          stepWynn(0)
          stepI3D(5,480.,376.)
          stepWynn(1)
          stepI3D(6,5760.,5472.)
          stepWynn(2)
//           stepI3D(7,80640.,88128.)
//           stepWynn(3)
//           stepI3D(8,1290240.,1571328.)
//           stepWynn(4)
#undef stepI3D
        } while (0);
        ivalue=sump/d0st0st;
      } else {
        // NORMAL
        ncomplex sum1=0;
        for (int u=1; u<=5; u++) {
          if (u==t || u==s) continue;
          sum1-=M3(u,s,t,0,s,t)*I2Dstu(ep, s, t, u);
        }
        sum1+=d0st0st*I3Dst(ep, s, t);
        ivalue=sum1/(4*dstst)+I3D2st(ep+1, s, t)*0.5; // 2*x/4 == 0.5*x
      }
    } else {
      assert(ep==1);
      int iu[3]={0,s,t};
      int nu[3];
      freeidxM3(iu, nu);
      ivalue=(Cay[nss(nu[0],nu[0])]+Cay[nss(nu[1],nu[1])]+Cay[nss(nu[2],nu[2])]
             +Cay[nss(nu[0],nu[1])]+Cay[nss(nu[0],nu[2])]+Cay[nss(nu[1],nu[2])])/24.;
    }
    pI3D2st[ep][idx]=ivalue;
  }
  }
  fEval[E_I3D2st+ep]=true;
}

/* --------------------------------------------------------
 *   I4D3s box in D+6 dim
 * --------------------------------------------------------
 */
ncomplex Minor5::I4D3s(int ep, int s)
{
  if (ep==2) return 0;
  if (not fEval[E_I4D3s+ep]) {
    I4D3sEval(ep);
  }
  return pI4D3s[ep][s-1];
}
void Minor5::I4D3sEval(int ep)
{
  for (int s=1; s<=smax; s++) {
    const double dss=M1(s, s);
    const double d0s0s=M2(0, s, 0, s);
    ncomplex ivalue=0;

    if (ep==0) {
      ncomplex sum1=0;
      for (int t=1; t<=5; t++) {
        if (t==s) continue;
        sum1-=M2(s,t,s,0)*I3D2st(ep, s, t);
      }
      sum1+=d0s0s*I4D2s(ep, s);
      ivalue=sum1/(5*dss)+2.*I4D3s(ep+1, s)/5.;

      const double x=dss/d0s0s;
      if (pmaxS4[s-1] <= deps3) {
      ncomplex sump;
      do {
        assert(ep==0);

        double dsts0[6];
        sum1=0;
        for (int t=1; t<=5; t++) {
          if (t==s) continue;
          dsts0[t]=M2(s,t,s,0);
          sum1+=dsts0[t]*I3D3st(0, s, t);
        }

        double xn=1;
        ncomplex dv,s21;

        ncomplex sum[3];
        sum[0]=sump=sum1;

#define stepI4D(n,a,b) \
        xn*=x; \
        dv=0; \
        for (int t=1; t<=5; t++) { \
          if (t==s) continue; \
          dv+=dsts0[t]*(a*I3D##n##st(0, s, t) - b*I3D##n##st(1, s, t)); \
        } \
        dv*=xn; \
        sum1+=dv;

        stepI4D(4,7.,2.)
        if (   fabs(sum1.real()*teps)>=fabs(dv.real())
            && fabs(sum1.imag()*teps)>=fabs(dv.imag()))
          break;
        sum[1]=sump=sum1;
        s21=sum[1]-sum[0];

        stepI4D(5,63.,32.)
        sump=sum1;
        stepWynn(0)
        stepI4D(6,693.,478.)
        stepWynn(1)
        stepI4D(7,9009.,7600.)
        stepWynn(2)
//       stepI4D(8,135135.,132018.)
//         stepWynn(3)
//       stepI4D(9,2297295.,2514576.)
//         stepWynn(4)
//       stepI4D(10,43648605.,52371534.)
//         stepWynn(5)
#undef stepI4D
      } while (0);
      ivalue=sump/d0s0s;
      }
    }
    else {
      assert(ep==1);
      double sum1=0;
      for (int i=1; i<=5; i++) { if (i==s) continue;
      for (int j=i; j<=5; j++) { if (j==s) continue;
        sum1+=Cay[nss(i,j)];
      }
      }
      ivalue=-sum1/120.;
    }
    pI4D3s[ep][s-1]=ivalue;
  }
  fEval[E_I4D3s+ep]=true;
}

/* --------------------------------------------------------
 *   I3D2sti triangle in D+4 dim with a dot
 * --------------------------------------------------------
 */
ncomplex Minor5::I3D2sti(int ep, int s, int t, int i)
{
  assert(s!=t && s!=i && t!=i);
  if      (ep==1) return 1./6.;
  else if (ep==2) return 0.;
  if (not fEval[E_I3D2sti+ep]) {
    I3D2stiEval(ep);
  }
  int idx = im2(s,t)-5;
  return pI3D2sti[ep][i-1][idx];
}

void Minor5::I3D2stiEval(int ep)
{
  for (int i=1; i<=CIDX; i++) {
  for (int s=1; s<=smax; s++) { if (i==s) continue;
  for (int t=s+1; t<=5; t++) {  if (i==t) continue;
    int idx = im2(s,t)-5;
    ncomplex ivalue=0;

    if ( (pmaxT3[idx]==0 || (pmaxT3[idx] > epsir2 || pmaxU3[idx] > epsir2))
      && pmaxS3[idx] > ceps ) {
      // Variant with Gram3
      ncomplex sum1=0;
      for (int u=1; u<=5; u++) {
        if (u==t || u==s) continue;
        sum1+=M3(u,s,t,i,s,t)*I2Dstu(ep,s,t,u);
      }
      sum1-=M3(0,s,t,i,s,t)*I3Dst(ep, s, t);
      ivalue=sum1/M2(s,t,s,t);
    } else {
      assert(ep==0);
      int iu[3]={i-1,s-1,t-1};
      int tmp;
      tswap(iu[0],iu[2],tmp);
      tswap(iu[1],iu[2],tmp);
      tswap(iu[0],iu[1],tmp);
      int nu[3];
      freeidxM3(iu, nu);
      int u=nu[0]+1;
      int v=nu[1]+1;

      if ( pmaxT3[idx] <= epsir2 && pmaxU3[idx] <= epsir2 ) {
        // small G3 & C3
        int j=imax3[idx];
        ncomplex sum1=0;
        ncomplex const I3term=I3Dst(ep,s,t)-1./3.;  //+2./3.*I3Dst(ep+1,s,t))
        ncomplex const I2Uterm=I2Dstui(ep, s, t, u, i)+2./3.*I2Dstui(ep+1, s, t, u, i);
        ncomplex const I2Vterm=I2Dstui(ep, s, t, v, i)+2./3.*I2Dstui(ep+1, s, t, v, i);

        if (j==i) { // j->i
          const double Dii=M4ii(u,v,i);
          const double Dui=M4ui(u,v,i);
          const double Dvi=M4vi(u,v,i);
          sum1+=+Dii*(I3term)             // (i, i)
                +Dui*(I2Uterm)            // (u, i)
                +Dvi*(I2Vterm);           // (v, i)
        } else if (j==u) { // j->u
          const double Dui=M4ui(u,v,i);
          const double Duu=M4uu(u,v,i);
          const double Dvu=M4vu(u,v,i);
          sum1+=+Dui*(I3term)             // (u, i)
                +Duu*(I2Uterm)            // (u, u)
                +Dvu*(I2Vterm);           // (v, u)
        } else { assert(j==v); // j->v
          const double Dvi=M4vi(u,v,i);
          const double Dvv=M4vv(u,v,i);
          const double Dvu=M4vu(u,v,i);
          sum1+=+Dvi*(I3term)             // (v, i)
                +Dvu*(I2Uterm)            // (v, u)
                +Dvv*(I2Vterm);           // (v, v)
        }
        ivalue=sum1/(3*M3(s,0,t,s,j,t));
      } else {
        // small G3
        const double Dii=M4ii(u,v,i);
        const double Dui=M4ui(u,v,i);
        const double Dvi=M4vi(u,v,i);
        ncomplex sum1=0;
        sum1+=Dii*I2Dstu(ep, s, t, i)     // (i, i)
             +Dui*I2Dstu(ep, s, t, u)     // (u, i)
             +Dvi*I2Dstu(ep, s, t, v);    // (v, i)
        sum1+=M3(s,0,t,s,i,t)*(-4.*I3D2st(ep, s, t)+2.*I3D2st(ep+1, s, t));
        ivalue=sum1/M3(s,0,t,s,0,t);
      }
    }
    pI3D2sti[ep][i-1][idx]=ivalue;
  }
  }
  }
  fEval[E_I3D2sti+ep]=true;
}

/* --------------------------------------------------------
 *   I4D3si box in D+6 dim with a dot
 * --------------------------------------------------------
 */
ncomplex Minor5::I4D3si(int ep, int s, int i)
{
  if (s==i) return 0;
  if      (ep==1) return -1./24.;
  else if (ep==2) return 0;
  if (not fEval[E_I4D3si+ep]) {
    I4D3siEval(ep);
  }
  return pI4D3si[ep][i-1][s-1];
}

void Minor5::I4D3siEval(int ep)
{
  for (int s=1; s<=smax; s++) {
  for (int i=1; i<=CIDX; i++) {
    if (s==i) continue;
    ncomplex ivalue=0;

    if (pmaxS4[s-1] <= deps3) {
      ncomplex sum1=0;
      for (int t=1; t<=5; t++) {
        if (t==s) continue;
        sum1+=M3(0, s, t, 0, s, i)*I3D2st(ep, s, t);
      }
      sum1+=M2(0, s, i, s)*(-5.*I4D3s(ep, s)+2.*I4D3s(ep+1, s));
      ivalue=sum1/M2(0, s, 0, s);
    } else {
      ncomplex sum1=0;
      for (int t=1; t<=5; t++) {
        if (t==s) continue;
        sum1+=M2(s, t, s, i)*I3D2st(ep, s, t);
      }
      sum1-=M2(0, s, i, s)*I4D2s(ep, s);
      ivalue=sum1/M1(s, s);
    }
    pI4D3si[ep][i-1][s-1]=ivalue;
  }
  }
  fEval[E_I4D3si+ep]=true;
}

/* --------------------------------------------------------
 *   I4D3sij box in D+6 dim with two dots
 * --------------------------------------------------------
 */
ncomplex Minor5::I4D3sij(int ep, int s, int i, int j)
{
  if (s==i || s==j) return 0;
  else if (ep!=0) return 0;  // I4D3sij is finite
  if (not fEval[E_I4D3sij+ep]) {
    I4D3sijEval(ep);
  }
  return pI4D3sij[ep][is(i-1,j-1)][s-1];
}

void Minor5::I4D3sijEval(int ep)
{
  for (int s=1; s<=smax; s++) {
  // symmetric in 'i,j'
  for (int i=1; i<=CIDX; i++) { if (s==i) continue;
  for (int j=i; j<=CIDX; j++) { if (s==j) continue;
    ncomplex ivalue=0;

    if (pmaxS4[s-1] <= deps3) {
      ncomplex sum1=0;
      for (int t=1; t<=5; t++) {
        if (t==s || t==i) continue;
        sum1+=M3(s,0,t,s,0,j)*I3D2sti(ep, s, t, i);
      }
      sum1+=M3(s,0,i,s,0,j)*I4D2s(ep, s);
      sum1+=M2(s,0,s,j)*(-4.*I4D3si(ep, s, i)+2.*I4D3si(ep+1, s, i));
      ivalue=sum1/M2(0,s,0,s);
    } else {
      ncomplex sum1=0;
      for (int t=1; t<=5; t++) {
        if (t==s || t==i) continue;
        sum1+=M2(s,t,s,j)*I3D2sti(ep, s, t, i);
      }
      sum1+=M2(s,i,s,j)*I4D2s(ep, s);
      sum1-=M2(s,0,s,j)*I4D2si(ep, s, i);
      ivalue=sum1/M1(s,s);
    }
    pI4D3sij[ep][iss(i-1,j-1)][s-1]=ivalue;
  }
  }
  }
  fEval[E_I4D3sij+ep]=true;
}


/* --------------------------------------------------------
 *   I2Dstui bubble in D+2 dim with a dot
 * --------------------------------------------------------
 */
ncomplex Minor5::I2Dstui(int ep, int s, int t, int u, int i)
{
  assert(s!=t && t!=u && u!=s && s!=i && t!=i && u!=i);
//   if (ep==1) return -0.5; // not quite true
  if (ep==2) return 0;
  if (not fEval[E_I2Dstui+ep]) {
    I2DstuiEval(ep,1,4,5,2,3,kinem.p3());
    I2DstuiEval(ep,1,3,5,2,4,kinem.s34());
    I2DstuiEval(ep,1,3,4,2,5,kinem.s12());
    I2DstuiEval(ep,1,4,5,3,2,kinem.p3());
    I2DstuiEval(ep,1,2,5,3,4,kinem.p4());
    I2DstuiEval(ep,1,2,4,3,5,kinem.s45());
    I2DstuiEval(ep,1,3,5,4,2,kinem.s34());
    I2DstuiEval(ep,1,2,5,4,3,kinem.p4());
    I2DstuiEval(ep,1,2,3,4,5,kinem.p5());
#ifdef USE_ZERO_CHORD
    I2DstuiEval(ep,1,3,4,5,2,kinem.s12());
    I2DstuiEval(ep,1,2,4,5,3,kinem.s45());
    I2DstuiEval(ep,1,2,3,5,4,kinem.p5());
#endif

    if (smax==5) {
      I2DstuiEval(ep,3,4,5,1,2,kinem.p2());
      I2DstuiEval(ep,2,4,5,1,3,kinem.s23());
      I2DstuiEval(ep,2,3,5,1,4,kinem.s15());
      I2DstuiEval(ep,2,3,4,1,5,kinem.p1());
      I2DstuiEval(ep,3,4,5,2,1,kinem.p2());
      I2DstuiEval(ep,2,4,5,3,1,kinem.s23());
      I2DstuiEval(ep,2,3,5,4,1,kinem.s15());
#ifdef USE_ZERO_CHORD
      I2DstuiEval(ep,2,3,4,5,1,kinem.p1());
#endif
    }

    fEval[E_I2Dstui+ep]=true;
  }
  int ip=15-s-t-u-i;
  return pI2Dstui[ep][i-1][ip-1];
}

void Minor5::I2DstuiEval(int ep, int s, int t, int u, int i, int ip, double qsq)
{
  ncomplex sum1=0;
  if (ep==0) {
    const double dstustu=-2*qsq; /*M3(s,t,u,s,t,u);*/
    const double msq1=kinem.mass(i);
    const double msq2=kinem.mass(ip);
    const double s_cutoff=seps1*pmaxM2[im2(i,ip)-5];

    if (fabs(dstustu) <= s_cutoff) {
      const double mm12=msq1-msq2;
      if (fabs(mm12) < meps) {
        if (msq1 > meps) {
          sum1=(msq1 - ICache::getI1(ep, Kinem1(msq1)))/(2*msq1);
        } else {
          sum1=0;
        }
      }
      else {
        sum1=(msq1 + msq2)/(4*msq1 - 4*msq2)
              - ((msq1 - 2*msq2)*ICache::getI1(ep, Kinem1(msq1))
                  + msq2*ICache::getI1(ep, Kinem1(msq2))
                )/(2*mm12*mm12);
      }
    }
    else {
      sum1+=+(Cay[nss(ip,ip)]-Cay[ns(i,ip)])*I2stu(ep,s,t,u);
      sum1+=+ICache::getI1(ep, Kinem1(msq1));
      sum1+=-ICache::getI1(ep, Kinem1(msq2));
      sum1/=dstustu;
    }
  }
  else { assert(ep==1);
    if ( fabs(qsq) < meps
        && fabs(kinem.mass(i)) < meps
        && fabs(kinem.mass(ip)) < meps ) {
      sum1=0;
    }
    else {
      sum1=-0.5;
    }
  }
  pI2Dstui[ep][i-1][ip-1]=sum1;
}

/* --------------------------------------------------------
 *   I3D2stij triangle in D+4 dim with two dots
 * --------------------------------------------------------
 */
ncomplex Minor5::I3D2stij(int ep, int s, int t, int i, int j) // IR-div
{
  assert(s!=t && s!=i && s!=j && t!=i && t!=j);
  if (not fEval[E_I3D2stij+ep]) {
    I3D2stijEval(ep);
  }
  int idx = im2(s,t)-5;
  return pI3D2stij[ep][is(i-1,j-1)][idx];
}

void Minor5::I3D2stijEval(int ep)
{
  for (int s=1; s<=smax; s++) {
  for (int t=s+1; t<=5; t++) {
    int idx = im2(s,t)-5;

    const double ds0ts0t=M3(s,0,t,s,0,t);
    if (ep!=0 && fabs(ds0ts0t) > m3eps) { // if ds0ts0t!=0 I3D2stij is finite
      for (int ij=iss(1-1,1-1); ij<=iss(CIDX-1,CIDX-1); ij++) {
        pI3D2stij[ep][ij][idx]=0;
      }
      continue;
    }

    const double dstst=M2(s,t,s,t);
    // symmetric in 'i,j'
    for (int i=1; i<=CIDX; i++) { if (i==s || i==t) continue;
    for (int j=i; j<=CIDX; j++) { if (j==s || j==t) continue;
      ncomplex ivalue=0;

      if (pmaxS3[idx] > ceps || ep!=0) {
        // Variant with Gram3
        ncomplex sum1=0;
        for (int u=1; u<=5; u++) {
          if (u==t || u==s || u==i) continue;
          sum1+=M3(s,u,t,s,j,t)*I2Dstui(ep, s, t, u, i);
        }
        sum1+=-M3(s,0,t,s,j,t)*I3Dsti(ep, s, t, i)+M3(s,i,t,s,j,t)*I3Dst(ep, s, t);
        ivalue=sum1/dstst;
      } else {
        ncomplex sum1=0;
        int iu[3]={j-1,s-1,t-1};
        int tmp;
        tswap(iu[0],iu[2],tmp);
        tswap(iu[1],iu[2],tmp);
        tswap(iu[0],iu[1],tmp);
        int nu[3];
        freeidxM3(iu, nu);
        int u=nu[0]+1;
        int v=nu[1]+1;
        const double Djj=M4ii(u,v,j);
        const double Duj=M4ui(u,v,j);
        const double Dvj=M4vi(u,v,j);
        if ( fabs(ds0ts0t) > 0. ) {
          if (j==i) {
            sum1+=+Djj*I3Dst(ep,s,t)
                  +Duj*I2Dstui(ep, s, t, u, i)
                  +Dvj*I2Dstui(ep, s, t, v, i);
          } else {
            sum1+=Djj*I2Dstui(ep, s, t, j, i);
            if (i==u) {
              sum1+=+Duj*I3Dst(ep,s,t)
                    +Dvj*I2Dstui(ep, s, t, v, i);
            } else {
              sum1+=+Dvj*I3Dst(ep,s,t)
                    +Duj*I2Dstui(ep, s, t, u, i);
            }
          }
          if (ep<2)
            sum1+=M3(s,0,t,s,j,t)*(-3.*I3D2sti(ep, s, t, i)+2.*I3D2sti(ep+1, s, t, i));
          else
            sum1+=M3(s,0,t,s,j,t)*(-3.*I3D2sti(ep, s, t, i));
          ivalue=sum1/ds0ts0t;
        } else {
          ivalue=std::numeric_limits<double>::quiet_NaN();
          // TODO add: need I2Dstuij and I2stui
        }
      }
      pI3D2stij[ep][iss(i-1,j-1)][idx]=ivalue;
    }
    }
  }
  }
  fEval[E_I3D2stij+ep]=true;
}

/* --------------------------------------------------------
 *   I4D3sijk box in D+6 dim with three dots
 * --------------------------------------------------------
 */
ncomplex Minor5::I4D3sijk(int ep, int s, int i, int j, int k) // IR-div
{
  if (s==i || s==j || s==k) return 0;
  if (not fEval[E_I4D3sijk+ep]) {
    I4D3sijkEval(ep);
  }
  return pI4D3sijk[ep][is(i-1,j-1,k-1)][s-1];
}

void Minor5::I4D3sijkEval(int ep)
{
  for (int s=1; s<=smax; s++) {
  // symmetric in 'i,j,k'
  for (int i=1; i<=CIDX; i++) { if (i==s) continue;
  for (int j=i; j<=CIDX; j++) { if (j==s) continue;
  for (int k=j; k<=CIDX; k++) { if (k==s) continue;
    ncomplex ivalue=0;

    if (pmaxS4[s-1] <= deps3) {
      ncomplex sum1=0;
      for (int t=1; t<=5; t++) {
          if (s==t || t==i || t==j) continue;
          sum1+=M3(s,0,t,s,0,k)*I3D2stij(ep,s,t,i,j);
      }
      sum1+=+M3(s,0,i,s,0,k)*I4D2si(ep, s, j)
            +M3(s,0,j,s,0,k)*I4D2si(ep, s, i);
      if (ep<2) {
          sum1+=M2(s,0,s,k)*(-3.*I4D3sij(ep, s, i, j)+2.*I4D3sij(ep+1, s, i, j));
      }
      else { // ep==2
          sum1+=M2(s,0,s,k)*(-3.*I4D3sij(ep, s, i, j));
      }
      ivalue=sum1/M2(s,0,s,0);
    } else {
      ncomplex sum1=0;
      for (int t=1; t<=5; t++) {
        if (t==s || t==i || t==j) continue;
        sum1+=M2(s,t,s,k)*I3D2stij(ep,s,t,i,j);
      }
      sum1-=M2(s,0,s,k)*I4D2sij(ep,s,i,j);
      sum1+=M2(s,i,s,k)*I4D2si(ep,s,j)+M2(s,j,s,k)*I4D2si(ep,s,i);
      ivalue=sum1/M1(s,s);
    }
    pI4D3sijk[ep][iss(i-1,j-1,k-1)][s-1]=ivalue;
  }
  }
  }
  }
  fEval[E_I4D3sijk+ep]=true;
}

/* --------------------------------------------------------
 *
 *                Rank-5 functions
 *
 * --------------------------------------------------------
 */

/* --------------------------------------------------------
 *   I2D2stu bubble in D+4 dim
 * --------------------------------------------------------
 */
ncomplex Minor5::I2D2stu(int ep, int s, int t, int u)
{
  assert(t!=u && u!=s && s!=t);
  if (ep==2) return 0;
  if (not fEval[E_I2D2stu+ep]) {
    I2D2stuEval(0,ep,1,2,3,4,5,kinem.p5());
    I2D2stuEval(1,ep,1,2,4,3,5,kinem.s45());
    I2D2stuEval(2,ep,1,2,5,3,4,kinem.p4());

    I2D2stuEval(3,ep,1,3,4,2,5,kinem.s12());
    I2D2stuEval(4,ep,1,3,5,2,4,kinem.s34());

    I2D2stuEval(5,ep,1,4,5,2,3,kinem.p3());

    if (smax==5) {
      I2D2stuEval(6,ep,2,3,4,1,5,kinem.p1());
      I2D2stuEval(7,ep,2,3,5,1,4,kinem.s15());

      I2D2stuEval(8,ep,2,4,5,1,3,kinem.s23());


      I2D2stuEval(9,ep,3,4,5,1,2,kinem.p2());
    }

    fEval[E_I2D2stu+ep]=true;
  }
  int idx=im3(s,t,u)-10;
  return pI2D2stu[ep][idx];
}

void Minor5::I2D2stuEval(int idx, int ep, int s, int t, int u, int m, int n, double qsq)
{
  ncomplex sum1=0;
  if (ep==0) {
    const double dstustu=-2*qsq; /*M3(s,t,u,s,t,u);*/
    const double msq1=kinem.mass(m);
    const double msq2=kinem.mass(n);
    const double s_cutoff=seps1*pmaxM2[im2(m,n)-5];

    if (fabs(dstustu) <= s_cutoff) {
      const double mm12=msq1-msq2;
      if (fabs(mm12) < meps) {
        sum1=0.25*msq1*(msq1 + 2.*ICache::getI1(ep, Kinem1(msq1)));
      }
      else {
        sum1= 5*(msq1*msq1 + msq1*msq2 + msq2*msq2)/36
              + ( + msq1*msq1*ICache::getI1(ep, Kinem1(msq1))
                  - msq2*msq2*ICache::getI1(ep, Kinem1(msq2))
                  )/(6*mm12);
      }
    }
    else {
      ncomplex sumX=5.*I2Dstu(ep,s,t,u)+2.*I2Dstu(ep+1,s,t,u);
      ncomplex sumY=-0.25*msq2*(10.*ICache::getI1(ep, Kinem1(msq2))+9*msq2);
      ncomplex sumZ=-0.25*msq1*(10.*ICache::getI1(ep, Kinem1(msq1))+9*msq1);

      const double ds0tu=(Cay[nss(m,m)]*Cay[nss(n,n)]-Cay[nss(m,n)]*Cay[nss(m,n)]);
      sum1+=sumX*ds0tu;

      const double dsvtuY=-(Cay[nss(n,n)]-Cay[nss(m,n)]); /* minus sign of minor v=m */
      sum1-=sumY*dsvtuY;

      const double dsvtuZ=+(Cay[nss(m,n)]-Cay[nss(m,m)]); /* plus sign of minor v=n */
      sum1-=sumZ*dsvtuZ;

      sum1/=25*dstustu;
    }
  }
  else { assert(ep==1);
    const double y11=Cay[nss(m,m)];
    const double y12=Cay[nss(m,n)];
    const double y22=Cay[nss(n,n)];
    sum1= ( 3*( y11*(y11+y12)+(y12+y22)*y22)+2*y12*y12+y11*y22 )/120;
  }
  pI2D2stu[ep][idx]=sum1;
}

/* --------------------------------------------------------
 *   I3D3st triangle in D+6 dim
 * --------------------------------------------------------
 */
ncomplex Minor5::I3D3st(int ep, int s, int t)
{
  assert(s!=t);
  if (ep==2) return 0;
  if (not fEval[E_I3D3st+ep]) {
    I3D3stEval(ep);
  }
  int idx = im2(s,t)-5;
  return pI3D3st[ep][idx];
}

void Minor5::I3D3stEval(int ep)
{
  for (int s=1; s<=smax; s++) {
  for (int t=s+1; t<=5; t++) {
    int idx = im2(s,t)-5;
    ncomplex ivalue=0;

    if (ep==0) {
      const double dstst=M2(s,t,s,t);
      const double d0st0st=M3(0,s,t,0,s,t);

      if ( pmaxT3[idx]!=0 && ( pmaxT3[idx] <= epsir1 && pmaxU3[idx] <= epsir1 ) ) {
        // IR
        int i=imax3[idx];
        int iu[3]={i-1,s-1,t-1};
        int tmp;
        tswap(iu[0],iu[2],tmp);
        tswap(iu[1],iu[2],tmp);
        tswap(iu[0],iu[1],tmp);
        int nu[3];
        freeidxM3(iu, nu);
        int u=nu[0]+1;
        int v=nu[1]+1;
        const double Dii=M4ii(u,v,i);
        const double Dui=M4ui(u,v,i);
        const double Dvi=M4vi(u,v,i);
        ncomplex sum1=+Dii*(I2D2stu(0, s, t, i)+I2D2stu(1, s, t, i)/3.)  // (i, i)
                      +Dui*(I2D2stu(0, s, t, u)+I2D2stu(1, s, t, u)/3.)  // (u, i)
                      +Dvi*(I2D2stu(0, s, t, v)+I2D2stu(1, s, t, v)/3.); // (v, i)
        ivalue=sum1/(6*M3(0,s,t,i,s,t));
      } else if (pmaxS3[idx] <= ceps) {
        // EXP
        const double x=dstst/d0st0st;
        ncomplex sump;
        do {
          assert(ep==0);

          double dstust0[6];
          ncomplex sum1=0;
          for (int u=1; u<=5; u++) {
            if (u==t || u==s) continue;
            dstust0[u]=M3(s,t,u,s,t,0);
            sum1+=dstust0[u]*I2D3stu(0, s, t, u);
          }

          double xn=1;
          ncomplex dv,s21;

          ncomplex sum[3];
          sum[0]=sump=sum1;

#define stepI3D(n,a,b) \
          xn*=x; \
          dv=0; \
          for (int u=1; u<=5; u++) { \
            if (u==t || u==s) continue; \
            dv+=dstust0[u]*(a*I2D##n##stu(0, s, t, u) - b*I2D##n##stu(1, s, t, u)); \
          } \
          dv*=xn; \
          sum1+=dv;

          stepI3D(4,8.,2.)
          if (   fabs(sum1.real()*teps)>=fabs(dv.real())
              && fabs(sum1.imag()*teps)>=fabs(dv.imag()))
            break;
          sum[1]=sump=sum1;
          s21=sum[1]-sum[0];

          stepI3D(5,80.,36.)
          sump=sum1;
          stepWynn(0)
          stepI3D(6,960.,592.)
          stepWynn(1)
//           stepI3D(7,13440.,10208.)
//           stepWynn(2)
//           stepI3D(8,215040.,190208.)
//           stepWynn(3)
//           stepI3D(9,3870720.,3853824.)
//           stepWynn(4)
#undef stepI3D
        } while (0);
        ivalue=sump/d0st0st;
      } else {
        // NORMAL
        ncomplex sum1=0;
        for (int u=1; u<=5; u++) {
          if (u==t || u==s) continue;
          sum1-=M3(u,s,t,0,s,t)*I2D2stu(ep, s, t, u);
        }
        sum1+=d0st0st*I3D2st(ep, s, t);
        ivalue=sum1/(6*dstst)+I3D3st(ep+1, s, t)/3.;
      }
    } else {
      assert(ep==1);
      int iu[3]={0,s,t};
      int nu[3];
      freeidxM3(iu, nu);
      const double y11=Cay[nss(nu[0],nu[0])];
      const double y12=Cay[nss(nu[0],nu[1])];
      const double y13=Cay[nss(nu[0],nu[2])];
      const double y22=Cay[nss(nu[1],nu[1])];
      const double y23=Cay[nss(nu[1],nu[2])];
      const double y33=Cay[nss(nu[2],nu[2])];
      ivalue=-(+3*(y11*(y11+y12+y13)+y22*(y12+y22+y23)+y33*(y13+y23+y33))
               +2*(y12*(y12+y23)+y13*(y12+y13)+y23*(y13+y23))
               +  (y11*(y22+y23)+y22*(y13+y33)+y33*(y11+y12))
               )/720.;
    }
    pI3D3st[ep][idx]=ivalue;
  }
  }
  fEval[E_I3D3st+ep]=true;
}

/* --------------------------------------------------------
 *   I4D4s box in D+8 dim
 * --------------------------------------------------------
 */
ncomplex Minor5::I4D4s(int ep, int s)
{
  if (ep==2) return 0;
  if (not fEval[E_I4D4s+ep]) {
    I4D4sEval(ep);
  }
  return pI4D4s[ep][s-1];
}

void Minor5::I4D4sEval(int ep)
{
  for (int s=1; s<=smax; s++) {
    const double dss=M1(s, s);
    const double d0s0s=M2(0, s, 0, s);
    ncomplex ivalue=0;

    if (ep==0) {
      ncomplex sum1=0;
      for (int t=1; t<=5; t++) {
        if (t==s) continue;
        sum1-=M2(s,t,s,0)*I3D3st(ep, s, t);
      }
      sum1+=d0s0s*I4D3s(ep, s);
      ivalue=sum1/(7*dss)+2.*I4D4s(ep+1, s)/7.;

      const double x=dss/d0s0s;
      if (pmaxS4[s-1] <= deps3) {
      ncomplex sump;
      do {
        assert(ep==0);

        double dsts0[6];
        sum1=0;
        for (int t=1; t<=5; t++) {
          if (t==s) continue;
          dsts0[t]=M2(s,t,s,0);
          sum1+=dsts0[t]*I3D4st(0, s, t);
        }

        double xn=1;
        ncomplex dv,s21;

        ncomplex sum[3];
        sum[0]=sump=sum1;

#define stepI4D(n,a,b) \
        xn*=x; \
        dv=0; \
        for (int t=1; t<=5; t++) { \
          if (t==s) continue; \
          dv+=dsts0[t]*(a*I3D##n##st(0, s, t) - b*I3D##n##st(1, s, t)); \
        } \
        dv*=xn; \
        sum1+=dv;

        stepI4D(5,9.,2.)
        if (   fabs(sum1.real()*teps)>=fabs(dv.real())
            && fabs(sum1.imag()*teps)>=fabs(dv.imag()))
          break;
        sum[1]=sump=sum1;
        s21=sum[1]-sum[0];

        stepI4D(6,99.,40.)
        sump=sum1;
        stepWynn(0)
        stepI4D(7,1287.,718.)
        stepWynn(1)
//         stepI4D(8,19305.,13344.)
//         stepWynn(2)
//         stepI4D(9,328185.,265458.)
//         stepWynn(3)
//         stepI4D(10,6235515.,5700072.)
//         stepWynn(4)
//         stepI4D(11,130945815.,132172542.)
//         stepWynn(5)
#undef stepI4D

      } while (0);
      ivalue=sump/d0s0s;
      }
    }
    else {
      assert(ep==1);
      double sum1=0;
      for (int i=1; i<=5; i++) { if (i==s) continue;
      for (int j=i; j<=5; j++) { if (j==s) continue;
      for (int k=j; k<=5; k++) { if (k==s) continue;
      for (int l=k; l<=5; l++) { if (l==s) continue;
        sum1+=+Cay[nss(i,j)]*Cay[nss(k,l)]
              +Cay[nss(i,k)]*Cay[nss(j,l)]
              +Cay[nss(i,l)]*Cay[nss(j,k)];
      }
      }
      }
      }
      ivalue=sum1/5040.;
    }
    pI4D4s[ep][s-1]=ivalue;
  }
  fEval[E_I4D4s+ep]=true;
}


/* --------------------------------------------------------
 *   I4D4si box in D+8 dim with a dot
 * --------------------------------------------------------
 */
ncomplex Minor5::I4D4si(int ep, int s, int i)
{
  if (s==i) return 0;
  if (ep==2) return 0;
  if (not fEval[E_I4D4si+ep]) {
    I4D4siEval(ep);
  }
  return pI4D4si[ep][i-1][s-1];
}

void Minor5::I4D4siEval(int ep)
{
  for (int s=1; s<=smax; s++) {
  for (int i=1; i<=CIDX; i++) { if (s==i) continue;
    ncomplex ivalue=0;

    if (ep == 0) {
      if (pmaxS4[s-1] <= deps3) {
        ncomplex sum1=0;
        for (int t=1; t<=5; t++) {
          if (t==s) continue;
          sum1+=M3(0, s, t, 0, s, i)*I3D3st(ep, s, t);
        }
        sum1+=M2(0, s, i, s)*(-7.*I4D4s(ep, s)+2.*I4D4s(ep+1, s));
        ivalue=sum1/M2(0, s, 0, s);
      } else {
        ncomplex sum1=0;
        for (int t=1; t<=5; t++) {
          if (t==s) continue;
          sum1+=M2(s,t,s,i)*I3D3st(ep, s, t);
        }
        sum1-=M2(s,0,s,i)*I4D3s(ep, s);
        sum1/=M1(s,s);
        ivalue=sum1;
      }
    } else {
      assert(ep==1);
      double sum1=0;
      sum1+=Cay[nss(i,i)];
      for (int j=1; j<=5; j++) {
        if (j==s) continue;
        sum1+=Cay[ns(i,j)];
        for (int k=j; k<=5; k++) {
          if (k==s) continue;
          sum1+=Cay[nss(j,k)];
        }
      }
      ivalue=sum1/720.;
    }
    pI4D4si[ep][i-1][s-1]=ivalue;
  }
  }
  fEval[E_I4D4si+ep]=true;
}

/* --------------------------------------------------------
 *   I3D3sti triangle in D+6 dim with a dot
 * --------------------------------------------------------
 */
ncomplex Minor5::I3D3sti(int ep, int s, int t, int i)
{
  assert(s!=t && s!=i && t!=i);
  if (ep==2) return 0.;
  if (not fEval[E_I3D3sti+ep]) {
    I3D3stiEval(ep);
  }
  int idx = im2(s,t)-5;
  return pI3D3sti[ep][i-1][idx];
}

void Minor5::I3D3stiEval(int ep)
{
  for (int i=1; i<=CIDX; i++) {
  for (int s=1; s<=smax; s++) { if (i==s) continue;
  for (int t=s+1; t<=5; t++) {  if (i==t) continue;
    int idx = im2(s,t)-5;
    ncomplex ivalue=0;

    if (ep==0) {
      if ( (pmaxT3[idx]==0 || (pmaxT3[idx] > epsir2 || pmaxU3[idx] > epsir2))
        && pmaxS3[idx] > ceps ) {
        // Variant with Gram3
        ncomplex sum1=0;
        for (int u=1; u<=5; u++) {
          if (u==t || u==s) continue;
          sum1+=M3(u,s,t,i,s,t)*I2D2stu(ep,s,t,u);
        }
        sum1-=M3(0,s,t,i,s,t)*I3D2st(ep,s,t);
        ivalue=sum1/M2(s,t,s,t);
      } else {
        assert(ep==0);
        int iu[3]={i-1,s-1,t-1};
        int tmp;
        tswap(iu[0],iu[2],tmp);
        tswap(iu[1],iu[2],tmp);
        tswap(iu[0],iu[1],tmp);
        int nu[3];
        freeidxM3(iu, nu);
        int u=nu[0]+1;
        int v=nu[1]+1;

        if ( pmaxT3[idx] <= epsir2 && pmaxU3[idx] <= epsir2 ) {
          // small G3 & C3
          int j=imax3[idx];
          ncomplex sum1=0;
          ncomplex const I3term=I3D2st(ep,s,t)+2./5.*I3D2st(ep+1,s,t);
          ncomplex const I2Uterm=I2D2stui(ep, s, t, u, i)+2./5.*I2D2stui(ep+1, s, t, u, i);
          ncomplex const I2Vterm=I2D2stui(ep, s, t, v, i)+2./5.*I2D2stui(ep+1, s, t, v, i);

          if (j==i) { // j->i
            const double Dii=M4ii(u,v,i);
            const double Dui=M4ui(u,v,i);
            const double Dvi=M4vi(u,v,i);
            sum1+=+Dii*(I3term)                      // (i, i)
                  +Dui*(I2Uterm)                     // (u, i)
                  +Dvi*(I2Vterm);                    // (v, i)
          } else if (j==u) { // j->u
            const double Dui=M4ui(u,v,i);
            const double Duu=M4uu(u,v,i);
            const double Dvu=M4vu(u,v,i);
            sum1+=+Dui*(I3term)                      // (u, i)
                  +Duu*(I2Uterm)                     // (u, u)
                  +Dvu*(I2Vterm);                    // (v, u)
          } else { assert(j==v); // j->v
            const double Dvi=M4vi(u,v,i);
            const double Dvv=M4vv(u,v,i);
            const double Dvu=M4vu(u,v,i);
            sum1+=+Dvi*(I3term)                      // (v, i)
                  +Dvu*(I2Uterm)                     // (v, u)
                  +Dvv*(I2Vterm);                    // (v, v)
          }
          ivalue=sum1/(5*M3(s,0,t,s,j,t));
        } else {
          // small G3
          const double Dii=M4ii(u,v,i);
          const double Dui=M4ui(u,v,i);
          const double Dvi=M4vi(u,v,i);
          ncomplex sum1=0;
          sum1+=Dii*I2D2stu(ep, s, t, i)             // (i, i)
               +Dui*I2D2stu(ep, s, t, u)             // (u, i)
               +Dvi*I2D2stu(ep, s, t, v);            // (v, i)
          sum1+=M3(s,0,t,s,i,t)*(-6.*I3D3st(ep, s, t)+2.*I3D3st(ep+1, s, t));
          ivalue=sum1/M3(s,0,t,s,0,t);
        }
      }
    } else {
      assert(ep==1);
      int iu[3]={0,s,t};
      int nu[3];
      freeidxM3(iu, nu);
      int j= nu[1]==i ? nu[0] : nu[1];
      int k= nu[2]==i ? nu[0] : nu[2];
      ivalue=-( 3*Cay[nss(i,i)]+2*(Cay[ns(i,j)]+Cay[ns(i,k)])
                 +Cay[nss(j,j)]+Cay[ns(j,k)]+Cay[nss(k,k)]
              )/120.;
    }
    pI3D3sti[ep][i-1][idx]=ivalue;
  }
  }
  }
  fEval[E_I3D3sti+ep]=true;
}

/* --------------------------------------------------------
 *   I4D4sij box in D+8 dim with two dots
 * --------------------------------------------------------
 */
ncomplex Minor5::I4D4sij(int ep, int s, int i, int j)
{
  if (s==i || s==j) return 0;
  if      (ep==1) return ( i==j ? 1./60. : 1./120. );
  else if (ep==2) return 0;
  if (not fEval[E_I4D4sij+ep]) {
    I4D4sijEval(ep);
  }
  return pI4D4sij[ep][is(i-1,j-1)][s-1];
}

void Minor5::I4D4sijEval(int ep)
{
  for (int s=1; s<=smax; s++) {
  // symmetric in 'i,j'
  for (int i=1; i<=CIDX; i++) { if (s==i) continue;
  for (int j=i; j<=CIDX; j++) { if (s==j) continue;
    ncomplex ivalue=0;

    if (pmaxS4[s-1] <= deps3) {
      ncomplex sum1=0;
      for (int t=1; t<=5; t++) {
        if (t==s || t==i) continue;
        sum1+=M3(s,0,t,s,0,j)*I3D3sti(ep, s, t, i);
      }
      sum1+=M3(s,0,i,s,0,j)*I4D3s(ep, s);
      sum1+=M2(s,0,s,j)*(-6.*I4D4si(ep, s, i)+2.*I4D4si(ep+1, s, i));
      ivalue=sum1/M2(0,s,0,s);
    } else {
      ncomplex sum1=0;
      for (int t=1; t<=5; t++) {
        if (t==s || t==i) continue;
        sum1+=M2(s,t,s,j)*I3D3sti(ep, s, t, i);
      }
      sum1+=M2(s,i,s,j)*I4D3s(ep, s);
      sum1-=M2(s,0,s,j)*I4D3si(ep, s, i);
      sum1/=M1(s,s);
      ivalue=sum1;
    }
    pI4D4sij[ep][iss(i-1,j-1)][s-1]=ivalue;
  }
  }
  }
  fEval[E_I4D4sij+ep]=true;
}

/* --------------------------------------------------------
 *   I2D2stui bubble in D+4 dim with a dot
 * --------------------------------------------------------
 */
ncomplex Minor5::I2D2stui(int ep, int s, int t, int u, int i)
{
  assert(s!=t && t!=u && u!=s && s!=i && t!=i && u!=i);
  if (ep==2) return 0;
  if (not fEval[E_I2D2stui+ep]) {
    I2D2stuiEval(ep,1,4,5,2,3,kinem.p3());
    I2D2stuiEval(ep,1,3,5,2,4,kinem.s34());
    I2D2stuiEval(ep,1,3,4,2,5,kinem.s12());
    I2D2stuiEval(ep,1,4,5,3,2,kinem.p3());
    I2D2stuiEval(ep,1,2,5,3,4,kinem.p4());
    I2D2stuiEval(ep,1,2,4,3,5,kinem.s45());
    I2D2stuiEval(ep,1,3,5,4,2,kinem.s34());
    I2D2stuiEval(ep,1,2,5,4,3,kinem.p4());
    I2D2stuiEval(ep,1,2,3,4,5,kinem.p5());
#ifdef USE_ZERO_CHORD
    I2D2stuiEval(ep,1,3,4,5,2,kinem.s12());
    I2D2stuiEval(ep,1,2,4,5,3,kinem.s45());
    I2D2stuiEval(ep,1,2,3,5,4,kinem.p5());
#endif

    if (smax==5) {
      I2D2stuiEval(ep,3,4,5,1,2,kinem.p2());
      I2D2stuiEval(ep,2,4,5,1,3,kinem.s23());
      I2D2stuiEval(ep,2,3,5,1,4,kinem.s15());
      I2D2stuiEval(ep,2,3,4,1,5,kinem.p1());
      I2D2stuiEval(ep,3,4,5,2,1,kinem.p2());
      I2D2stuiEval(ep,2,4,5,3,1,kinem.s23());
      I2D2stuiEval(ep,2,3,5,4,1,kinem.s15());
#ifdef USE_ZERO_CHORD
      I2D2stuiEval(ep,2,3,4,5,1,kinem.p1());
#endif
    }

    fEval[E_I2D2stui+ep]=true;
  }
  int ip=15-s-t-u-i; // ip
  return pI2D2stui[ep][i-1][ip-1];
}

void Minor5::I2D2stuiEval(int ep, int s, int t, int u, int i, int ip, double qsq)
{
  ncomplex sum1=0;
  if (ep==0) {
    const double dstustu=-2*qsq; /*M3(s,t,u,s,t,u);*/
    const double msq1=kinem.mass(i);
    const double msq2=kinem.mass(ip);
    const double s_cutoff=seps1*pmaxM2[im2(i,ip)-5];

    if (fabs(dstustu) <= s_cutoff) {
      const double mm12=msq1-msq2;
      if (fabs(mm12) < meps) {
        sum1=0.5*ICache::getI1(ep, Kinem1(msq1));
      }
      else { assert(ep==0);
        sum1=(4*msq1*msq1-5*(msq1+msq2)*msq2)/(36*mm12)
            + (  msq1*(2*msq1 - 3*msq2)*ICache::getI1(ep, Kinem1(msq1))
                + msq2*msq2*ICache::getI1(ep, Kinem1(msq2))
                )/(6*mm12*mm12);
      }
    }
    else {
      sum1+=+(Cay[nss(ip,ip)]-Cay[ns(i,ip)])*I2Dstu(ep,s,t,u);
      sum1+=-0.5*msq1*(ICache::getI1(ep, Kinem1(msq1))+0.5*msq1);
      sum1+=+0.5*msq2*(ICache::getI1(ep, Kinem1(msq2))+0.5*msq2);
      sum1/=dstustu;
    }
  }
  else { assert(ep==1);
    if ( fabs(qsq) < meps
        && fabs(kinem.mass(i)) < meps
        && fabs(kinem.mass(ip)) < meps ) {
      sum1=0;
    }
    else {
      sum1=(3*Cay[nss(i,i)] + 2*Cay[ns(i,ip)] + Cay[nss(ip,ip)])/24.;
    }
  }
  pI2D2stui[ep][i-1][ip-1]=sum1;
}


/* --------------------------------------------------------
 *   I3D3stij triangle in D+6 dim with two dots
 * --------------------------------------------------------
 */
ncomplex Minor5::I3D3stij(int ep, int s, int t, int i, int j)
{
  assert(s!=t && s!=i && s!=j && t!=i && t!=j);
  if      (ep==1) return ( i==j ? -1./12. : -1./24. ); // -1/12 == -2/24
  else if (ep==2) return 0;
  if (not fEval[E_I3D3stij+ep]) {
    I3D3stijEval(ep);
  }
  int idx = im2(s,t)-5;
  return pI3D3stij[ep][is(i-1,j-1)][idx];
}

void Minor5::I3D3stijEval(int ep)
{
  for (int s=1; s<=smax; s++) {
  for (int t=s+1; t<=5; t++) {
    int idx = im2(s,t)-5;
    const double dstst=M2(s,t,s,t);
    // symmetric in 'i,j'
    for (int i=1; i<=CIDX; i++) { if (i==s || i==t) continue;
    for (int j=i; j<=CIDX; j++) { if (j==s || j==t) continue;
      ncomplex ivalue=0;

      if ( (pmaxT3[idx]==0 || (pmaxT3[idx] > epsir2 || pmaxU3[idx] > epsir2))
        && pmaxS3[idx] > ceps ) {
        // Variant with Gram3
        ncomplex sum1=0;
        for (int u=1; u<=5; u++) {
          if (u==t || u==s || u==i) continue;
          sum1+=M3(s,u,t,s,j,t)*I2D2stui(ep, s, t, u, i);
        }
        sum1+=-M3(s,0,t,s,j,t)*I3D2sti(ep, s, t, i)+M3(s,i,t,s,j,t)*I3D2st(ep, s, t);
        ivalue=sum1/dstst;
      } else {
        assert(ep==0);
        int iu[3]={j-1,s-1,t-1};
        int tmp;
        tswap(iu[0],iu[2],tmp);
        tswap(iu[1],iu[2],tmp);
        tswap(iu[0],iu[1],tmp);
        int nu[3];
        freeidxM3(iu, nu);
        int u=nu[0]+1;
        int v=nu[1]+1;

        const double Djj=M4ii(u,v,j);
        const double Duj=M4ui(u,v,j);
        const double Dvj=M4vi(u,v,j);
        if ( pmaxT3[idx] <= epsir2 && pmaxU3[idx] <= epsir2 ) {
          // small G3 & C3
          int k=imax3[idx];
          ncomplex sum1=0;
          if (i==j) {
            if (k==j) {
              sum1+=2*Djj*(I3D2sti(ep,s,t,j)+0.5*I3D2sti(ep+1,s,t,j))
                     +Duj*(I2D2stuij(ep,s,t,u,j,j)+0.5*I2D2stuij(ep+1,s,t,u,j,j))
                     +Dvj*(I2D2stuij(ep,s,t,v,j,j)+0.5*I2D2stuij(ep+1,s,t,v,j,j));
            } else if (k==u) {
              const double Duu=M4uu(u,v,j);
              const double Duv=M4vu(u,v,j);
              sum1+=2*Duj*(I3D2sti(ep,s,t,j)+0.5*I3D2sti(ep+1,s,t,j))
                     +Duu*(I2D2stuij(ep,s,t,u,j,j)+0.5*I2D2stuij(ep+1,s,t,u,j,j))
                     +Duv*(I2D2stuij(ep,s,t,v,j,j)+0.5*I2D2stuij(ep+1,s,t,v,j,j));
            } else { // k==v
              const double Dvv=M4vv(u,v,j);
              const double Dvu=M4vu(u,v,j);
              sum1+=2*Dvj*(I3D2sti(ep,s,t,j)+0.5*I3D2sti(ep+1,s,t,j))
                     +Dvu*(I2D2stuij(ep,s,t,u,j,j)+0.5*I2D2stuij(ep+1,s,t,u,j,j))
                     +Dvv*(I2D2stuij(ep,s,t,v,j,j)+0.5*I2D2stuij(ep+1,s,t,v,j,j));
            }
          } else if (k==j) {
            sum1+=Djj*(I3D2sti(ep,s,t,i)+0.5*I3D2sti(ep+1,s,t,i));
            if (i==u) {
              sum1+=Duj*(I3D2sti(ep,s,t,j)+0.5*I3D2sti(ep+1,s,t,j))
                   +Dvj*(I2D2stuij(ep,s,t,v,i,j)+0.5*I2D2stuij(ep+1,s,t,v,i,j));
            } else { // i==v
              sum1+=Dvj*(I3D2sti(ep,s,t,j)+0.5*I3D2sti(ep+1,s,t,j))
                   +Duj*(I2D2stuij(ep,s,t,u,i,j)+0.5*I2D2stuij(ep+1,s,t,u,i,j));
            }
          } else if (k==i) {
            if (k==u) {
              const double Duu=M4uu(u,v,j);
              const double Duv=M4vu(u,v,j);
              sum1+=Duj*(I3D2sti(ep,s,t,i)+0.5*I3D2sti(ep+1,s,t,i))
                   +Duu*(I3D2sti(ep,s,t,j)+0.5*I3D2sti(ep+1,s,t,j))
                   +Duv*(I2D2stuij(ep,s,t,v,i,j)+0.5*I2D2stuij(ep+1,s,t,v,i,j));
            } else { // k==v
              const double Dvv=M4vv(u,v,j);
              const double Dvu=M4vu(u,v,j);
              sum1+=Dvj*(I3D2sti(ep,s,t,i)+0.5*I3D2sti(ep+1,s,t,i))
                   +Dvv*(I3D2sti(ep,s,t,j)+0.5*I3D2sti(ep+1,s,t,j))
                   +Dvu*(I2D2stuij(ep,s,t,u,i,j)+0.5*I2D2stuij(ep+1,s,t,u,i,j));
            }
          } else {
            if (k==u) { // i==v
              const double Duu=M4uu(u,v,j);
              const double Duv=M4vu(u,v,j);
              sum1+=Duj*(I3D2sti(ep,s,t,i)+0.5*I3D2sti(ep+1,s,t,i))
                   +Duv*(I3D2sti(ep,s,t,j)+0.5*I3D2sti(ep+1,s,t,j))
                   +Duu*(I2D2stuij(ep,s,t,u,i,j)+0.5*I2D2stuij(ep+1,s,t,u,i,j));
            } else { // k==v, i==u
              const double Dvv=M4vv(u,v,j);
              const double Dvu=M4vu(u,v,j);
              sum1+=Dvj*(I3D2sti(ep,s,t,i)+0.5*I3D2sti(ep+1,s,t,i))
                   +Dvu*(I3D2sti(ep,s,t,j)+0.5*I3D2sti(ep+1,s,t,j))
                   +Dvv*(I2D2stuij(ep,s,t,v,i,j)+0.5*I2D2stuij(ep+1,s,t,v,i,j));
            }
          }
          ivalue=sum1/(4*M3(s,0,t,s,k,t));
        } else {
          // small G3
          ncomplex sum1=0;
          if (j==i) {
            sum1+=+Djj*I3D2st(ep,s,t)
                  +Duj*I2D2stui(ep, s, t, u, i)
                  +Dvj*I2D2stui(ep, s, t, v, i);
          } else {
            sum1+=Djj*I2D2stui(ep, s, t, j, i);
            if (i==u) {
              sum1+=+Duj*I3D2st(ep,s,t)
                    +Dvj*I2D2stui(ep, s, t, v, i);
            } else {
              sum1+=+Dvj*I3D2st(ep,s,t)
                    +Duj*I2D2stui(ep, s, t, u, i);
            }
          }
          sum1+=M3(s,0,t,s,j,t)*(-5.*I3D3sti(ep, s, t, i)+2.*I3D3sti(ep+1, s, t, i));
          ivalue=sum1/M3(s,0,t,s,0,t);
        }
      }
      pI3D3stij[ep][iss(i-1,j-1)][idx]=ivalue;
    }
    }
  }
  }
  fEval[E_I3D3stij+ep]=true;
}

/* --------------------------------------------------------
 *   I4D4sijk box in D+8 dim with three dots
 * --------------------------------------------------------
 */
ncomplex Minor5::I4D4sijk(int ep, int s, int i, int j, int k)
{
  if (s==i || s==j || s==k) return 0;
  if (ep==2) return 0;       // I4D4sijk finite
  if (not fEval[E_I4D4sijk+ep]) {
    I4D4sijkEval(ep);
  }
  return pI4D4sijk[ep][is(i-1,j-1,k-1)][s-1];
}

void Minor5::I4D4sijkEval(int ep)
{
  for (int s=1; s<=smax; s++) {
  // symmetric in 'i,j,k'
  for (int i=1; i<=CIDX; i++) { if (i==s) continue;
  for (int j=i; j<=CIDX; j++) { if (j==s) continue;
  for (int k=j; k<=CIDX; k++) { if (k==s) continue;
    ncomplex ivalue=0;

    if (pmaxS4[s-1] <= deps3) {
      ncomplex sum1=0;
      for (int t=1; t<=5; t++) {
        if (s==t || t==i || t==j) continue;
        sum1+=M3(s,0,t,s,0,k)*I3D3stij(ep,s,t,i,j);
      }
      sum1+=+M3(s,0,i,s,0,k)*I4D3si(ep, s, j)
            +M3(s,0,j,s,0,k)*I4D3si(ep, s, i);

      sum1+=M2(s,0,s,k)*(-5.*I4D4sij(ep, s, i, j)+2.*I4D4sij(ep+1, s, i, j));

      ivalue=sum1/M2(s,0,s,0);
    } else {
      ncomplex sum1=0;
      for (int t=1; t<=5; t++) {
        if (t==s || t==i || t==j) continue;
        sum1+=M2(s,t,s,k)*I3D3stij(ep,s,t,i,j);
      }
      sum1-=M2(s,0,s,k)*I4D3sij(ep,s,i,j);
      sum1+=M2(s,i,s,k)*I4D3si(ep,s,j)+M2(s,j,s,k)*I4D3si(ep,s,i);
      ivalue=sum1/M1(s,s);
    }
    pI4D4sijk[ep][iss(i-1,j-1,k-1)][s-1]=ivalue;
  }
  }
  }
  }
  fEval[E_I4D4sijk+ep]=true;
}

/* --------------------------------------------------------
 *   I2D2stuij bubble in D+4 dim with two dots
 * --------------------------------------------------------
 */
ncomplex Minor5::I2D2stuij(int ep, int s, int t, int u, int i, int j)
{
  assert(s!=t && t!=u && u!=s && s!=i && t!=i && u!=i && s!=j && t!=j && u!=j);
  if (ep==2) return 0;
  if (not fEval[E_I2D2stuij+ep]) {
    I2D2stuijEval(ep,1,2,3,4,5,kinem.p5());
    I2D2stuijEval(ep,1,2,4,3,5,kinem.s45());
    I2D2stuijEval(ep,1,2,5,3,4,kinem.p4());
    I2D2stuijEval(ep,1,2,5,4,3,kinem.p4());

    I2D2stuijEval(ep,1,3,4,2,5,kinem.s12());
    I2D2stuijEval(ep,1,3,5,2,4,kinem.s34());
    I2D2stuijEval(ep,1,3,5,4,2,kinem.s34());

    I2D2stuijEval(ep,1,4,5,2,3,kinem.p3());
    I2D2stuijEval(ep,1,4,5,3,2,kinem.p3());

#ifdef USE_ZERO_CHORD
    I2D2stuijEval(ep,1,2,3,5,4,kinem.p5());
    I2D2stuijEval(ep,1,2,4,5,3,kinem.s45());
    I2D2stuijEval(ep,1,3,4,5,2,kinem.s12());
#endif
    if (smax==5) {
      I2D2stuijEval(ep,2,3,4,1,5,kinem.p1());
      I2D2stuijEval(ep,2,3,5,1,4,kinem.s15());
      I2D2stuijEval(ep,2,3,5,4,1,kinem.s15());
      I2D2stuijEval(ep,2,4,5,1,3,kinem.s23());
      I2D2stuijEval(ep,2,4,5,3,1,kinem.s23());
      I2D2stuijEval(ep,3,4,5,1,2,kinem.p2());
      I2D2stuijEval(ep,3,4,5,2,1,kinem.p2());
#ifdef USE_ZERO_CHORD
      I2D2stuijEval(ep,2,3,4,5,1,kinem.p1());
#endif
    }

    fEval[E_I2D2stuij+ep]=true;
  }
  int ip=15-s-t-u-i; // ip
  return pI2D2stuij[ep][i-1][ip-1][i==j ? 0 : 1];
}

void Minor5::I2D2stuijEval(int ep, int s, int t, int u, int i, int ip, double qsq)
{
  ncomplex sum0=0;
  ncomplex sum1=0;
  if (ep==0) {
    const double dstustu=-2*qsq; /*M3(s,t,u,s,t,u);*/
    const double msq1=kinem.mass(i);
    const double msq2=kinem.mass(ip);
    const double s_cutoff=seps2*pmaxM2[im2(i,ip)-5];

    if (fabs(dstustu) <= s_cutoff) {
      const double mm12=msq1-msq2;
      if (fabs(mm12) < meps) {
        if (msq1 > meps) {
          sum1=( ICache::getI1(ep, Kinem1(msq1)) - msq1 )/(6*msq1);
        } else {
          sum1=0;
        }
        sum0=2.*sum1;
      }
      else {
        sum0=2.*( (-4*msq1*msq1 + 5*msq1*msq2 + 5*msq2*msq2)/6.
                + ( (msq1*msq1 - 3*msq1*msq2 + 3*msq2*msq2)*ICache::getI1(ep, Kinem1(msq1))
                  - msq2*msq2*ICache::getI1(ep, Kinem1(msq2))
                  )/mm12
              )/(6.*mm12*mm12);
        sum1=(-(msq1*msq1 + 10*msq1*msq2 + msq2*msq2)/6. +
                ( msq1*(msq1 - 3*msq2)*ICache::getI1(ep, Kinem1(msq1))
                + (3*msq1 - msq2)*msq2*ICache::getI1(ep, Kinem1(msq2))
                )/mm12
              )/(6.*mm12*mm12);
      }
    }
    else {
      sum0+=+(Cay[nss(ip,ip)]-Cay[ns(i,ip)])*I2Dstui(ep,s,t,u,i);
      sum0+=-ICache::getI1(ep, Kinem1(msq1));
      sum0+=-I2Dstu(ep,s,t,u);
      sum0/=dstustu;

      sum1+=(Cay[nss(i ,i )]-Cay[ns(i,ip)])*I2Dstui(ep,s,t,u,i);
      sum1+=ICache::getI1(ep, Kinem1(msq1));
      /* Symmetrization is not needed */
//       sum1+=(Cay[nss(ip,ip)]-Cay[ns(ip,i)])*I2Dstui(ep,s,t,u,ip);
//       sum1+=ICache::getI1(ep, Kinem1(msq2));
//       sum1/=2.0;
      sum1+=I2Dstu(ep,s,t,u);
      sum1/=dstustu;
    }
  }
  else { assert(ep==1);
    if ( fabs(qsq) < meps
        && fabs(kinem.mass(i)) < meps
        && fabs(kinem.mass(ip)) < meps ) {
      sum0=0;
      sum1=0;
    }
    else {
      sum0=2./6.;
      sum1=1./6.;
    }
  }
  pI2D2stuij[ep][i-1][ip-1][0]=sum0;
  pI2D2stuij[ep][i-1][ip-1][1]=sum1;
}

/* --------------------------------------------------------
 *   I3D3stijk triangle in D+6 dim with three dots
 * --------------------------------------------------------
 */
ncomplex Minor5::I3D3stijk(int ep, int s, int t, int i, int j, int k) // IR-div
{
  assert(s!=t && s!=i && s!=j && s!=k && t!=i && t!=j && t!=k);
  if (not fEval[E_I3D3stijk+ep]) {
    I3D3stijkEval(ep);
  }
  int idx = im2(s,t)-5;
  return pI3D3stijk[ep][is(i-1,j-1,k-1)][idx];
}

void Minor5::I3D3stijkEval(int ep)
{
  for (int s=1; s<=smax; s++) {
  for (int t=s+1; t<=5; t++) {
    int idx = im2(s,t)-5;

    const double ds0ts0t=M3(s,0,t,s,0,t);
    if (ep!=0 && fabs(ds0ts0t) > m3eps) { // if ds0ts0t!=0 I3D3stijk is finite
      for (int ijk=iss(1-1,1-1,1-1); ijk<=iss(CIDX-1,CIDX-1,CIDX-1); ijk++) {
        pI3D3stijk[ep][ijk][idx]=0;
      }
      continue;
    }

    const double dstst=M2(s,t,s,t);
    for (int i=1; i<=CIDX; i++) { if (i==s || i==t) continue;
    for (int j=i; j<=CIDX; j++) { if (j==s || j==t) continue;
    for (int k=j; k<=CIDX; k++) { if (k==s || k==t) continue;
      ncomplex ivalue=0;

      if (pmaxS3[idx] > ceps || ep!=0) {
        // Variant with Gram3
        ncomplex sum1=0;
        for (int u=1; u<=5; u++) {
          if (u==s || u==t || u==i || u==j) continue;
          sum1+=M3(s,u,t,s,k,t)*I2D2stuij(ep, s, t, u, i, j);
        }
        sum1-=M3(s,0,t,s,k,t)*I3D2stij(ep, s, t, i, j);
        sum1+=M3(s,i,t,s,k,t)*I3D2sti(ep, s, t, j)+M3(s,j,t,s,k,t)*I3D2sti(ep, s, t, i);
        ivalue=sum1/dstst;
      } else {
        ncomplex sum1=0;
        int iu[3]={k-1,s-1,t-1};
        int tmp;
        tswap(iu[0],iu[2],tmp);
        tswap(iu[1],iu[2],tmp);
        tswap(iu[0],iu[1],tmp);
        int nu[3];
        freeidxM3(iu, nu);
        int u=nu[0]+1;
        int v=nu[1]+1;
        const double Dkk=M4ii(u,v,k);
        const double Duk=M4ui(u,v,k);
        const double Dvk=M4vi(u,v,k);
        if ( fabs(ds0ts0t) > 0. ) {
          if (j==i) {
            if (j==k) {
              sum1+=+2*Dkk*I3D2sti(ep,s,t,j)
                      +Duk*I2D2stuij(ep, s, t, u, j, j)
                      +Dvk*I2D2stuij(ep, s, t, v, j, j);
            } else {
              sum1+=Dkk*I2D2stuij(ep, s, t, k, j, j);
              if (j==u) {
                sum1+=+2*Duk*I3D2sti(ep,s,t,j)
                        +Dvk*I2D2stuij(ep, s, t, v, j, j);
              } else {
                sum1+=+2*Dvk*I3D2sti(ep,s,t,j)
                        +Duk*I2D2stuij(ep, s, t, u, j, j);
              }
            }
          } else {
            if (j==k) {
              sum1+=+Dkk*I3D2sti(ep,s,t,i);
              if (i==u) {
                sum1+=+Duk*I3D2sti(ep,s,t,j)
                      +Dvk*I2D2stuij(ep, s, t, v, i, j);
              } else {
                sum1+=+Dvk*I3D2sti(ep,s,t,j)
                      +Duk*I2D2stuij(ep, s, t, u, i, j);
              }
            } else {
              sum1+=+Duk*I3D2sti(ep,s,t,v)
                    +Dvk*I3D2sti(ep,s,t,u)
                    +Dkk*I2D2stuij(ep, s, t, k, i, j);
            }
          }
          if (ep<2)
            sum1+=M3(s,0,t,s,k,t)*(-4.*I3D3stij(ep, s, t, i, j)+2.*I3D3stij(ep+1, s, t, i, j));
          else
            sum1+=M3(s,0,t,s,k,t)*(-4.*I3D3stij(ep, s, t, i, j));
          ivalue=sum1/ds0ts0t;
        } else {
          ivalue=std::numeric_limits<double>::quiet_NaN();
        // TODO add and check, needs I2D2stuijk
        }
      }
    pI3D3stijk[ep][iss(i-1,j-1,k-1)][idx]=ivalue;
    }
    }
    }
  }
  }
  fEval[E_I3D3stijk+ep]=true;
}

/* --------------------------------------------------------
 *   I4D4sijkl box in D+8 dim with four dots
 * --------------------------------------------------------
 */
ncomplex Minor5::I4D4sijkl(int ep, int s, int i, int j, int k, int l) // IR-div
{
  if (s==i || s==j || s==k || s==l) return 0;
  if (not fEval[E_I4D4sijkl+ep]) {
    I4D4sijklEval(ep);
  }
  return pI4D4sijkl[ep][is(i-1,j-1,k-1,l-1)][s-1];
}

void Minor5::I4D4sijklEval(int ep)
{
  for (int s=1; s<=smax; s++) {
  // symmetric in 'i,j,k,l'
  for (int i=1; i<=CIDX; i++) { if (s==i) continue;
  for (int j=i; j<=CIDX; j++) { if (s==j) continue;
  for (int k=j; k<=CIDX; k++) { if (s==k) continue;
  for (int l=k; l<=CIDX; l++) { if (s==l) continue;
    ncomplex ivalue=0;

    if (pmaxS4[s-1] <= deps3) {
      ncomplex sum1=0;
      for (int t=1; t<=5; t++) {
        if (s==t || t==i || t==j || t==k) continue;
        sum1+=M3(s,0,t,s,0,l)*I3D3stijk(ep,s,t,i,j,k);
      }
      sum1+=+M3(s,0,i,s,0,l)*I4D3sij(ep, s, j, k)
            +M3(s,0,j,s,0,l)*I4D3sij(ep, s, i, k)
            +M3(s,0,k,s,0,l)*I4D3sij(ep, s, i, j);
      if (ep<2) {
        sum1+=M2(s,0,s,l)*(-4.*I4D4sijk(ep, s, i, j, k)+2.*I4D4sijk(ep+1, s, i, j, k));
      }
      else { // ep==2
        sum1+=M2(s,0,s,l)*(-4.*I4D4sijk(ep, s, i, j, k));
      }
      ivalue=sum1/M2(s,0,s,0);
    } else {
      ncomplex sum1=0;
      for (int t=1; t<=5; t++) {
        if (t==s || t==i || t==j || t==k) continue;
        sum1+=M2(s,t,s,l)*I3D3stijk(ep,s,t,i,j,k);
      }
      sum1-=M2(s,0,s,l)*I4D3sijk(ep,s,i,j,k);
      sum1+=M2(s,i,s,l)*I4D3sij(ep,s,j,k)
          +M2(s,j,s,l)*I4D3sij(ep,s,i,k)
          +M2(s,k,s,l)*I4D3sij(ep,s,i,j);
      ivalue=sum1/M1(s,s);
    }
    pI4D4sijkl[ep][iss(i-1,j-1,k-1,l-1)][s-1]=ivalue;
  }
  }
  }
  }
  }
  fEval[E_I4D4sijkl+ep]=true;
}
