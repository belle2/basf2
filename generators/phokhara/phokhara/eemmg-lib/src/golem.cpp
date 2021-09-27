/*
 * cache.h - golem-mode wrapper function
 *
 * this file is part of PJFry library
 * Copyright 2011 Valery Yundin
 */

#include "common.h"
#include "cache.h"
#include "kinem.h"

#include "pjfry.h"
#include "golem.h"

int Golem::N=0;
int Golem::caylen=0;

MinorBase* Golem::minortbl[128]={0};
Minor5::Ptr Golem::minors5[6];
Minor4::Ptr Golem::minors4[15];
Minor3::Ptr Golem::minors3[20];
Minor2::Ptr Golem::minors2[15];

double Golem::Cay[6*(6+1)/2];
unsigned int Golem::bitfield;
unsigned int Golem::bitmask;

void Golem::initgolem95(int n)
{
  assert(4<=n && n<=6);
// cleanup old minor cache
  switch (N)
  {
    case 6:
#ifdef USE_GOLEM_MODE_6
      for (int i=0; i<6; i++) {
        minors5[i]=Minor5::Ptr();
      }
      for (int i=0; i<15; i++) {
        minors4[i]=Minor4::Ptr();
      }
      for (int i=0; i<20; i++) {
        minors3[i]=Minor3::Ptr();
      }
      for (int i=0; i<15; i++) {
        minors2[i]=Minor2::Ptr();
      }
#else
      assert(N<=5);
#endif /* USE_GOLEM_MODE_6 */
      break;
    case 5:
      minors5[0]=Minor5::Ptr();
      for (int i=0; i<5; i++) {
        minors4[i]=Minor4::Ptr();
      }
      for (int i=0; i<10; i++) {
        minors3[i]=Minor3::Ptr();
        minors2[i]=Minor2::Ptr();
      }
      break;
    case 4:
      minors4[0]=Minor4::Ptr();
      for (int i=0; i<4; i++) {
        minors3[i]=Minor3::Ptr();
      }
      for (int i=0; i<6; i++) {
        minors2[i]=Minor2::Ptr();
      }
      break;
  }
// set N to the new value
  N=n;
  caylen=N*(N+1)/2;
  for (int i=0; i<caylen; i++) {
    Cay[i]=0;
  }
  bitfield=0;
  bitmask=(1<<caylen)-1;
}

double Golem::getmat(int i, int j)
{
  int idx=MinorBase::ns(i,j);
  assert(idx<caylen);
  unsigned int mark=1<<idx;
  assert((bitfield & mark) != 0);
  return Cay[idx];
}

#ifdef USE_GOLEM_MODE_6
void Golem::prepare6()
{
  for (int i5=1; i5<=6; i5++) {
#define nss(i,j) MinorBase::nss(i<i5 ? i : i+1, j<i5 ? j : j+1 )
    const double m1 = -0.5*Cay[nss(1,1)];
    const double m2 = -0.5*Cay[nss(2,2)];
    const double p2 = Cay[nss(1,2)]+m1+m2;
    const double m3 = -0.5*Cay[nss(3,3)];
    const double s23= Cay[nss(1,3)]+m1+m3;
    const double p3 = Cay[nss(2,3)]+m2+m3;
    const double m4 = -0.5*Cay[nss(4,4)];
    const double s15= Cay[nss(1,4)]+m1+m4;
    const double s34= Cay[nss(2,4)]+m2+m4;
    const double p4 = Cay[nss(3,4)]+m3+m4;
    const double m5 = -0.5*Cay[nss(5,5)];
    const double p1 = Cay[nss(1,5)]+m1+m5;
    const double s12= Cay[nss(2,5)]+m2+m5;
    const double s45= Cay[nss(3,5)]+m3+m5;
    const double p5 = Cay[nss(4,5)]+m4+m5;
#undef nss
    Kinem5 k5(p1, p2, p3, p4, p5, s12, s23, s34, s45, s15, m1, m2, m3, m4, m5);
    Minor5::Ptr mptr5=Minor5::create(k5);
    MCache::cm5.insert(MCache::Entry5(k5,mptr5));
    minors5[6-i5]=mptr5;
    mptr5->psix=i5;
    minortbl[(1<<i5)]=minors5[6-i5].operator->();

    MCache::Array4::iterator it4=MCache::cm4.begin();
    Minor4::Ptr* mptr4=minors4;
    for (int s=6; s>=1; s--) {
      if (s==i5) continue;
      (*mptr4)=it4->val;
      minortbl[(1<<i5)|(1<<s)]=mptr4->operator->();
      ++it4;
      ++mptr4;
    }

    MCache::Array3::iterator it3=MCache::cm3.begin();
    Minor3::Ptr* mptr3=minors3;
    for (int s=6; s>=1; s--) {
      if (s==i5) continue;
      for (int t=6; t>=s+1; t--) {
        if (t==i5) continue;
        (*mptr3)=it3->val;
        minortbl[(1<<i5)|(1<<s)|(1<<t)]=mptr3->operator->();
        ++it3;
        ++mptr3;
      }
    }

    MCache::Array2::iterator it2=MCache::cm2.begin();
    Minor2::Ptr* mptr2=minors2;
    for (int s=6; s>=1; s--) {
      if (s==i5) continue;
      for (int t=6; t>=s+1; t--) {
        if (t==i5) continue;
        for (int u=6; u>=t+1; u--) {
          if (u==i5) continue;
          (*mptr2)=it2->val;
          minortbl[(1<<i5)|(1<<s)|(1<<t)|(1<<u)]=mptr2->operator->();
          ++it2;
          ++mptr2;
        }
      }
    }
  }
}
#endif /* USE_GOLEM_MODE_6 */

void Golem::prepare5()
{
  const double m1 = -0.5*Cay[0];
  const double m2 = -0.5*Cay[2];
  const double p2 = Cay[1]+m1+m2;
  const double m3 = -0.5*Cay[5];
  const double s23= Cay[3]+m1+m3;
  const double p3 = Cay[4]+m2+m3;
  const double m4 = -0.5*Cay[9];
  const double s15= Cay[6]+m1+m4;
  const double s34= Cay[7]+m2+m4;
  const double p4 = Cay[8]+m3+m4;
  const double m5 = -0.5*Cay[14];
  const double p1 = Cay[10]+m1+m5;
  const double s12= Cay[11]+m2+m5;
  const double s45= Cay[12]+m3+m5;
  const double p5 = Cay[13]+m4+m5;
  Kinem5 k5(p1, p2, p3, p4, p5, s12, s23, s34, s45, s15, m1, m2, m3, m4, m5);

  // since Minor5 defines both 5- and 4-point kinematics
  // if it is on top, we can be sure that lower minors are in order
  Minor5::Ptr mptr5=MCache::cm5.begin()->val;
  if ( ! (MCache::cm5.begin()->key == k5) ) {
    mptr5=Minor5::create(k5);
    MCache::cm5.insert(MCache::Entry5(k5,mptr5));
  }
  minors5[0]=mptr5;
  minortbl[0]=minors5[0].operator->();

  MCache::Array4::iterator it4=MCache::cm4.begin();
  Minor4::Ptr* mptr4=minors4;
  for (int s=5; s>=1; s--) {
    (*mptr4)=it4->val;
    minortbl[(1<<s)]=mptr4->operator->();
    ++it4;
    ++mptr4;
  }

  MCache::Array3::iterator it3=MCache::cm3.begin();
  Minor3::Ptr* mptr3=minors3;
  for (int s=5; s>=1; s--) {
    for (int t=5; t>=s+1; t--) {
      (*mptr3)=it3->val;
      minortbl[(1<<s)|(1<<t)]=mptr3->operator->();
      ++it3;
      ++mptr3;
    }
  }

  MCache::Array2::iterator it2=MCache::cm2.begin();
  Minor2::Ptr* mptr2=minors2;
  for (int s=5; s>=1; s--) {
    for (int t=5; t>=s+1; t--) {
      for (int u=5; u>=t+1; u--) {
        (*mptr2)=it2->val;
        minortbl[(1<<s)|(1<<t)|(1<<u)]=mptr2->operator->();
        ++it2;
        ++mptr2;
      }
    }
  }
}

void Golem::prepare4()
{
  const double m1 = -0.5*Cay[0];
  const double m2 = -0.5*Cay[2];
  const double p2 = Cay[1]+m1+m2;
  const double m3 = -0.5*Cay[5];
  const double s23= Cay[3]+m1+m3;
  const double p3 = Cay[4]+m2+m3;
  const double m4 = -0.5*Cay[9];
  const double p1 = Cay[6]+m1+m4;
  const double s12= Cay[7]+m2+m4;
  const double p4 = Cay[8]+m3+m4;
  Kinem4 k4(p1, p2, p3, p4, s12, s23, m1, m2, m3, m4);

  // we can check that top m5 is 4-point and m4->key==k4
  // but it is simpler to just create it anew
  Minor5::create(k4);
  Minor4::Ptr mptr4=MCache::cm4.begin()->val;
  MCache::cm4.insert(MCache::Entry4(k4,mptr4));
  minors4[0]=mptr4;
  minortbl[0]=minors4[0].operator->();

  MCache::Array3::iterator it3=MCache::cm3.begin();
  Minor3::Ptr* mptr3=minors3;
  for (int s=4; s>=1; s--) {
    (*mptr3)=it3->val;
    minortbl[(1<<s)]=mptr3->operator->();
    ++it3;
    ++mptr3;
  }

  MCache::Array2::iterator it2=MCache::cm2.begin();
  Minor2::Ptr* mptr2=minors2;
  for (int s=4; s>=1; s--) {
    for (int t=4; t>=s+1; t--) {
      (*mptr2)=it2->val;
      minortbl[(1<<s)|(1<<t)]=mptr2->operator->();
      ++it2;
      ++mptr2;
    }
  }
}

void Golem::preparesmatrix()
{
  switch (N)
  {
#ifdef USE_GOLEM_MODE_6
    case 6:
      prepare6();
      break;
#endif /* USE_GOLEM_MODE_6 */
    case 5:
      prepare5();
      break;
    case 4:
      prepare4();
      break;
    default:
      assert(4<=N && N<=6);
  }
}

void Golem::setmat(int i, int j, double val)
{
  int idx=MinorBase::ns(i,j);
  unsigned int mark=1<<idx;
  Cay[idx]=val;
  bitfield|=mark;
//   if (bitfield == bitmask) {
//     preparesmatrix();
//   }
}


  //  5 point base

#ifdef USE_GOLEM_ZERO_CHECK
#   define ZCHK /* */
#else
#   define ZCHK //
#endif

#define golem5(type,indices) \
  assert(N>=5); \
  ncomplex ivalue=minortbl[s]->type indices;

ncomplex Golem::ga50(int s, int ep)
{
  golem5(A,(ep))
  return ivalue;
}

ncomplex Golem::ga51(int i, int s, int ep)
{
  ZCHK if ( ((1<<i))&s ) return 0;
  golem5(A,(ep, i))
  return -ivalue;
}

ncomplex Golem::ga52(int i, int j, int s, int ep)
{
  ZCHK if ( ((1<<j)|(1<<i))&s ) return 0;
  golem5(A,(ep, i, j))
  return ivalue;
}

ncomplex Golem::gb52(int s, int ep)
{
  golem5(B,(ep))
  return ivalue;
}

ncomplex Golem::ga53(int i, int j, int k, int s, int ep)
{
  ZCHK if ( ((1<<k)|(1<<j)|(1<<i))&s ) return 0;
  golem5(A,(ep, i, j, k))
  return -ivalue;
}

ncomplex Golem::gb53(int i, int s, int ep)
{
  ZCHK if ( ((1<<i))&s ) return 0;
  golem5(B,(ep, i))
  return -ivalue;
}

ncomplex Golem::ga54(int i, int j, int k, int l, int s, int ep)
{
  ZCHK if ( ((1<<l)|(1<<k)|(1<<j)|(1<<i))&s ) return 0;
  golem5(A,(ep, i, j, k, l))
  return ivalue;
}

ncomplex Golem::gb54(int i, int j, int s, int ep)
{
  ZCHK if ( ((1<<j)|(1<<i))&s ) return 0;
  golem5(B,(ep, i, j))
  return ivalue;
}

ncomplex Golem::gc54(int s, int ep)
{
  golem5(C,(ep))
  return ivalue;
}

ncomplex Golem::ga55(int i, int j, int k, int l, int m, int s, int ep)
{
  ZCHK if ( ((1<<m)|(1<<l)|(1<<k)|(1<<j)|(1<<i))&s ) return 0;
  golem5(A,(ep, i, j, k, l, m))
  return -ivalue;
}

ncomplex Golem::gb55(int i, int j, int k, int s, int ep)
{
  ZCHK if ( ((1<<k)|(1<<j)|(1<<i))&s ) return 0;
  golem5(B,(ep, i, j, k))
  return -ivalue;
}

ncomplex Golem::gc55(int i, int s, int ep)
{
  ZCHK if ( ((1<<i))&s ) return 0;
  golem5(C,(ep, i))
  return -ivalue;
}

#undef golem5

// Skip coefficient cache
#define golem4(type,indices) \
  ncomplex ivalue=minortbl[s]->type indices;

ncomplex Golem::ga40(int s, int ep)
{
  golem4(A,(ep))
  return ivalue;
}

ncomplex Golem::ga41(int i, int s, int ep)
{
  ZCHK if ( ((1<<i))&s ) return 0;
  golem4(A,(ep, i))
  return -ivalue;
}

ncomplex Golem::ga42(int i, int j, int s, int ep)
{
  ZCHK if ( ((1<<j)|(1<<i))&s ) return 0;
  golem4(A,(ep, i, j))
  return ivalue;
}

ncomplex Golem::gb42(int s, int ep)
{
  golem4(B,(ep))
  return ivalue;
}

ncomplex Golem::ga43(int i, int j, int k, int s, int ep)
{
  ZCHK if ( ((1<<k)|(1<<j)|(1<<i))&s ) return 0;
  golem4(A,(ep, i, j, k))
  return -ivalue;
}

ncomplex Golem::gb43(int i, int s, int ep)
{
  ZCHK if ( ((1<<i))&s ) return 0;
  golem4(B,(ep,i))
  return -ivalue;
}

ncomplex Golem::ga44(int i, int j, int k, int l, int s, int ep)
{
  ZCHK if ( ((1<<l)|(1<<k)|(1<<j)|(1<<i))&s ) return 0;
  golem4(A,(ep, i, j, k, l))
  return ivalue;
}

ncomplex Golem::gb44(int i, int j, int s, int ep)
{
  ZCHK if ( ((1<<j)|(1<<i))&s ) return 0;
  golem4(B,(ep, i, j))
  return ivalue;
}

ncomplex Golem::gc44(int s, int ep)
{
  golem4(C,(ep))
  return ivalue;
}

#undef golem4

// Skip coefficient cache
#define golem3(type,indices) \
  ncomplex ivalue=minortbl[s]->type indices;

ncomplex Golem::ga30(int s, int ep)
{
  golem3(A,(ep))
  return ivalue;
}

ncomplex Golem::ga31(int i, int s, int ep)
{
  ZCHK if ( ((1<<i))&s ) return 0;
  golem3(A,(ep, i))
  return -ivalue;
}

ncomplex Golem::ga32(int i, int j, int s, int ep)
{
  ZCHK if ( ((1<<j)|(1<<i))&s ) return 0;
  golem3(A,(ep, i, j))
  return ivalue;
}

ncomplex Golem::gb32(int s, int ep)
{
  golem3(B,(ep))
  return ivalue;
}

ncomplex Golem::ga33(int i, int j, int k, int s, int ep)
{
  ZCHK if ( ((1<<k)|(1<<j)|(1<<i))&s ) return 0;
  golem3(A,(ep, i, j, k))
  return -ivalue;
}

ncomplex Golem::gb33(int i, int s, int ep)
{
  ZCHK if ( ((1<<i))&s ) return 0;
  golem3(B,(ep, i))
  return -ivalue;
}

#undef golem3

// Skip coefficient cache
#define golem2(type,indices) \
  ncomplex ivalue=minortbl[s]->type indices;

ncomplex Golem::ga20(int s, int ep)
{
  golem2(A,(ep))
  return ivalue;
}

ncomplex Golem::ga21(int i, int s, int ep)
{
  ZCHK if ( ((1<<i))&s ) return 0;
  golem2(A,(ep, i))
  return -ivalue;
}

ncomplex Golem::ga22(int i, int j, int s, int ep)
{
  ZCHK if ( ((1<<j)|(1<<i))&s ) return 0;
  golem2(A,(ep, i, j))
  return ivalue;
}

ncomplex Golem::gb22(int s, int ep)
{
  golem2(B,(ep))
  return ivalue;
}

#undef golem2

#undef ZCHK

// ====================================================
// ====================================================
// ====================================================
// ====================================================
// ====================================================
// ====================================================
double pggetmusq_()
{
  return ICache::getMu2();
}

void pgsetmusq_(double *mu2)
{
  ICache::setMu2(*mu2);
  Golem::preparesmatrix();
}

double pggetmat_(int *i, int *j)
{
  return Golem::getmat(*i,*j);
}

void pgpreparesmatrix_()
{
  Golem::preparesmatrix();
}

void pgsetmat_(int *i, int *j, double *val)
{
  Golem::setmat(*i,*j,*val);
}

void pginitgolem95_(int *n)
{
  Golem::initgolem95(*n);
}

//  5 point base
pj_complex pga50_(int *s, int *ep)
{
  return Golem::ga50(*s,*ep);
}

pj_complex pga51_(int *i, int *s, int *ep)
{
  return Golem::ga51(*i,*s,*ep);
}

pj_complex pga52_(int *i, int *j, int *s, int *ep)
{
  return Golem::ga52(*i,*j,*s,*ep);
}

pj_complex pgb52_(int *s, int *ep)
{
  return Golem::gb52(*s,*ep);
}

pj_complex pga53_(int *i, int *j, int *k, int *s, int *ep)
{
  return Golem::ga53(*i,*j,*k,*s,*ep);
}

pj_complex pgb53_(int *i, int *s, int *ep)
{
  return Golem::gb53(*i,*s,*ep);
}

pj_complex pga54_(int *i, int *j, int *k, int *l, int *s, int *ep)
{
  return Golem::ga54(*i,*j,*k,*l,*s,*ep);
}

pj_complex pgb54_(int *i, int *j, int *s, int *ep)
{
  return Golem::gb54(*i,*j,*s,*ep);
}

pj_complex pgc54_(int *s, int *ep)
{
  return Golem::gc54(*s,*ep);
}

pj_complex pga55_(int *i, int *j, int *k, int *l, int *m, int *s, int *ep)
{
  return Golem::ga55(*i,*j,*k,*l,*m,*s,*ep);
}

pj_complex pgb55_(int *i, int *j, int *k, int *s, int *ep)
{
  return Golem::gb55(*i,*j,*k,*s,*ep);
}

pj_complex pgc55_(int *i, int *s, int *ep)
{
  return Golem::gc55(*i,*s,*ep);
}

pj_complex pga40_(int *s, int *ep)
{
  return Golem::ga40(*s,*ep);
}

pj_complex pga41_(int *i, int *s, int *ep)
{
  return Golem::ga41(*i,*s,*ep);
}

pj_complex pga42_(int *i, int *j, int *s, int *ep)
{
  return Golem::ga42(*i,*j,*s,*ep);
}

pj_complex pgb42_(int *s, int *ep)
{
  return Golem::gb42(*s,*ep);
}

pj_complex pga43_(int *i, int *j, int *k, int *s, int *ep)
{
  return Golem::ga43(*i,*j,*k,*s,*ep);
}

pj_complex pgb43_(int *i, int *s, int *ep)
{
  return Golem::gb43(*i,*s,*ep);
}

pj_complex pga44_(int *i, int *j, int *k, int *l, int *s, int *ep)
{
  return Golem::ga44(*i,*j,*k,*l,*s,*ep);
}

pj_complex pgb44_(int *i, int *j, int *s, int *ep)
{
  return Golem::gb44(*i,*j,*s,*ep);
}

pj_complex pgc44_(int *s, int *ep)
{
  return Golem::gc44(*s,*ep);
}

pj_complex pga30_(int *s, int *ep)
{
  return Golem::ga30(*s,*ep);
}

pj_complex pga31_(int *i, int *s, int *ep)
{
  return Golem::ga31(*i,*s,*ep);
}

pj_complex pga32_(int *i, int *j, int *s, int *ep)
{
  return Golem::ga32(*i,*j,*s,*ep);
}

pj_complex pgb32_(int *s, int *ep)
{
  return Golem::gb32(*s,*ep);
}

pj_complex pga33_(int *i, int *j, int *k, int *s, int *ep)
{
  return Golem::ga33(*i,*j,*k,*s,*ep);
}

pj_complex pgb33_(int *i, int *s, int *ep)
{
  return Golem::gb33(*i,*s,*ep);
}

pj_complex pga20_(int *s, int *ep)
{
  return Golem::ga20(*s,*ep);
}

pj_complex pga21_(int *i, int *s, int *ep)
{
  return Golem::ga21(*i,*s,*ep);
}

pj_complex pga22_(int *i, int *j, int *s, int *ep)
{
  return Golem::ga22(*i,*j,*s,*ep);
}

pj_complex pgb22_(int *s, int *ep)
{
  return Golem::gb22(*s,*ep);
}
