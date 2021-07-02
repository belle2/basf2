/*
 * minorex.cpp - extra integrals for asymptotic expansion
 *
 * this file is part of PJFry library
 * Copyright 2011 Valery Yundin
 */

#include "minor.h"
#include "cache.h"

/* ========================================================
 *               IR triangles
 * ========================================================
 */

/* --------------------------------------------------------
 *   I2mDstu bubble in D-2 dim
 * --------------------------------------------------------
 */
ncomplex Minor5::I2mDstu(int ep, int s, int t, int u, int m, int n)
{
  ncomplex sum1=0;
  const double dstustu=M3(s,t,u,s,t,u);
  const double msq1=kinem.mass(m);
  const double msq2=kinem.mass(n);
  if (ep==0) {
    const double s_cutoff=seps1*pmaxM2[im2(m,n)-5];

    if (fabs(dstustu) <= s_cutoff) {
      const double mm12=msq1-msq2;
      if (fabs(mm12) < meps) {
        if (msq1 > meps) {
          sum1=1/msq1;
        } else {
          sum1=0;
        }
      }
      else {
        sum1= ( - (msq1>meps ? ICache::getI1(ep, Kinem1(msq1))/msq1 : 1.)
                + (msq2>meps ? ICache::getI1(ep, Kinem1(msq2))/msq2 : 1.)
              )/(mm12);
      }
    }
    else {
      ncomplex sumX=I2stu(ep,s,t,u)-2.*I2stu(ep+1,s,t,u);
      ncomplex sumY=msq2>meps ? 1.-ICache::getI1(ep, Kinem1(msq2))/msq2 : 0;
      ncomplex sumZ=msq1>meps ? 1.-ICache::getI1(ep, Kinem1(msq1))/msq1 : 0;

      const double ds0tu=(Cay[nss(m,m)]*Cay[nss(n,n)]-Cay[ns(m,n)]*Cay[ns(m,n)]);
      sum1+=sumX*dstustu;

      const double dsvtuY=-(Cay[nss(n,n)]-Cay[ns(m,n)]); /* minus sign of minor v=m */
      sum1+=sumY*dsvtuY;

      const double dsvtuZ=+(Cay[ns(m,n)]-Cay[nss(m,m)]); /* plus sign of minor v=n */
      sum1+=sumZ*dsvtuZ;

      sum1/=ds0tu;
    }
  } else if (ep==1) {
    if (fabs(msq1) < meps) {
      if (fabs(msq2) < meps) {
        if (fabs(dstustu) > meps) {
          sum1=2./(-0.5*dstustu);                 // I(s,0,0)
        } else {
          sum1=0;                                 // I(0,0,0)
        }
      } else {
        sum1=1./((-0.5*dstustu)-msq2);            // I(s,0,m2)
      }
    } else if (fabs(msq2) < meps) {
        sum1=1./((-0.5*dstustu)-msq1);            // I(s,m1,0)
    }
  }
  return sum1;
}

/* --------------------------------------------------------
 *   I2stui bubble in D dim with a dot
 * --------------------------------------------------------
 */
ncomplex Minor5::I2stui(int ep, int s, int t, int u, int i, int ip)
{
  ncomplex sum1=0;
  const double dstustu=M3(s,t,u,s,t,u);
  const double msq1=kinem.mass(i);
  const double msq2=kinem.mass(ip);
  if (ep==0) {
    const double s_cutoff=seps1*pmaxM2[im2(i,ip)-5];

    if (fabs(dstustu) <= s_cutoff) {
      const double mm12=msq1-msq2;
      if (fabs(mm12) < meps) {
        if (msq1 > meps) {
          sum1=-1/(2*msq1);
        } else {
          sum1=0;
        }
      }
      else {
        if (msq1 > meps) {
          sum1=-1/(msq1 - msq2)
                - ( + msq2*ICache::getI1(ep, Kinem1(msq1))
                    - msq1*ICache::getI1(ep, Kinem1(msq2))
                  )/(msq1*mm12*mm12);
        } else {
          sum1=ICache::getI1(ep, Kinem1(msq2))/(msq2*msq2);
        }
      }
    }
    else {
      sum1+=+(Cay[nss(ip,ip)]-Cay[ns(i,ip)])*I2mDstu(ep,s,t,u,i,ip);
      sum1+=msq1>meps ? 1.-ICache::getI1(ep, Kinem1(msq1))/msq1 : 0;
      sum1-=msq2>meps ? 1.-ICache::getI1(ep, Kinem1(msq2))/msq2 : 0;
      sum1/=dstustu;
    }
  } else if (ep==1) {
    if (fabs(msq1) < meps) {
      if (fabs(dstustu) > meps) {
        if (fabs(msq2) < meps) {
          sum1=-1./(-0.5*dstustu);            // I(s,0,0)
        } else {
          sum1=-1./((-0.5*dstustu)-msq2);     // I(s,0,m2)
        }
      } else {
        sum1=1./msq2;                         // I(0,0,m2)
      }
    }
  }
  return sum1;
}

/* ========================================================
 *               SMALL Gram4 expansion
 * ========================================================
 */

/* --------------------------------------------------------
 *   I2D3stu bubble in D+6 dim
 * --------------------------------------------------------
 */
ncomplex Minor5::I2D3stu(int ep, int s, int t, int u)
{
  assert(t!=u && u!=s && s!=t); //if (t==u || u==s || s==t) return 0;
  if (ep==2) return 0;
  if (not fEval[E_I2D3stu+ep]) {
    I2D3stuEval(0,ep,1,2,3,4,5,kinem.p5());
    I2D3stuEval(1,ep,1,2,4,3,5,kinem.s45());
    I2D3stuEval(2,ep,1,2,5,3,4,kinem.p4());

    I2D3stuEval(3,ep,1,3,4,2,5,kinem.s12());
    I2D3stuEval(4,ep,1,3,5,2,4,kinem.s34());

    I2D3stuEval(5,ep,1,4,5,2,3,kinem.p3());

    if (smax==5) {
      I2D3stuEval(6,ep,2,3,4,1,5,kinem.p1());
      I2D3stuEval(7,ep,2,3,5,1,4,kinem.s15());

      I2D3stuEval(8,ep,2,4,5,1,3,kinem.s23());


      I2D3stuEval(9,ep,3,4,5,1,2,kinem.p2());
    }

    fEval[E_I2D3stu+ep]=true;
  }
  int idx=im3(s,t,u)-10;
  return pI2D3stu[ep][idx];
}

void Minor5::I2D3stuEval(int idx, int ep, int s, int t, int u, int m, int n, double qsq)
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
        sum1=-(5*msq1 + 6.*ICache::getI1(ep, Kinem1(msq1)))*msq1*msq1/36.;
      }
      else {
        sum1= -13*((msq1+msq2)*(msq1*msq1+msq2*msq2))/288
              + ( - msq1*msq1*msq1*ICache::getI1(ep, Kinem1(msq1))
                  + msq2*msq2*msq2*ICache::getI1(ep, Kinem1(msq2))
                  )/(24*mm12);
      }
    }
    else {
      ncomplex sumX=7.*I2D2stu(ep,s,t,u)+2.*I2D2stu(ep+1,s,t,u);
      ncomplex sumY=(42.*ICache::getI1(ep, Kinem1(msq2))+47*msq2)*msq2*msq2/36.;
      ncomplex sumZ=(42.*ICache::getI1(ep, Kinem1(msq1))+47*msq1)*msq1*msq1/36.;

      const double ds0tu=(Cay[nss(m,m)]*Cay[nss(n,n)]-Cay[nss(m,n)]*Cay[nss(m,n)]);
      sum1+=sumX*ds0tu;

      const double dsvtuY=-(Cay[nss(n,n)]-Cay[nss(m,n)]); /* minus sign of minor v=m */
      sum1-=sumY*dsvtuY;

      const double dsvtuZ=+(Cay[nss(m,n)]-Cay[nss(m,m)]); /* plus sign of minor v=n */
      sum1-=sumZ*dsvtuZ;

      sum1/=49*dstustu;
    }
  }
  else { assert(ep==1);
    const double y11=Cay[nss(m,m)];
    const double y12=Cay[nss(m,n)];
    const double y22=Cay[nss(n,n)];
    sum1=-(+ y11*y11*(y22 + 5*(y11 + y12))
            + y22*y22*(y11 + 5*(y22 + y12))
            + 2*y12*y12*(y12 + 2*(y11 + y22))
            + 3*y11*y22*y12
            )/1680;
  }
  pI2D3stu[ep][idx]=sum1;
}

/* --------------------------------------------------------
 *   I3D4st triangle in D+8 dim
 * --------------------------------------------------------
 */
ncomplex Minor5::I3D4st(int ep, int s, int t)
{
  assert(s!=t); //if (s==t) return 0;
  if (ep==2) return 0;
  if (not fEval[E_I3D4st+ep]) {
    I3D4stEval(ep);
  }
  int idx = im2(s,t)-5;
  return pI3D4st[ep][idx];
}

void Minor5::I3D4stEval(int ep)
{
  for (int s=1; s<=smax; s++) {
    for (int t=s+1; t<=5; t++) {
      int idx = im2(s,t)-5;

      const double dstst=M2(s,t,s,t);
      ncomplex ivalue=0;

      if (pmaxS3[idx] <= deps) {
        printf("I3D4 - M2({%d,%d}) <= eps\n",s,t);
        ivalue=std::numeric_limits<double>::quiet_NaN();
      }
      else {
        double cf=1./8.;
        for (int ei=ep; ei<=1; ei++) {
          ncomplex sum1=M3(0,s,t,0,s,t)*I3D3st(ei, s, t);
          for (int u=1; u<=5; u++) {
            if (t==u || s==u) continue;
            sum1-=M3(u,s,t,0,s,t)*I2D3stu(ei, s, t, u);
          }
          ivalue+=cf*sum1;
          cf*=1./4.;
        }
        ivalue/=dstst;
      }
      pI3D4st[ep][idx]=ivalue;
    }
  }
  fEval[E_I3D4st+ep]=true;
}


/* --------------------------------------------------------
 *   I2D4stu bubble in D+8 dim
 * --------------------------------------------------------
 */
ncomplex Minor5::I2D4stu(int ep, int s, int t, int u)
{
  assert(t!=u && u!=s && s!=t); //if (t==u || u==s || s==t) return 0;
  if (ep==2) return 0;
  if (not fEval[E_I2D4stu+ep]) {
    I2D4stuEval(0,ep,1,2,3,4,5,kinem.p5());
    I2D4stuEval(1,ep,1,2,4,3,5,kinem.s45());
    I2D4stuEval(2,ep,1,2,5,3,4,kinem.p4());

    I2D4stuEval(3,ep,1,3,4,2,5,kinem.s12());
    I2D4stuEval(4,ep,1,3,5,2,4,kinem.s34());

    I2D4stuEval(5,ep,1,4,5,2,3,kinem.p3());

    if (smax==5) {
      I2D4stuEval(6,ep,2,3,4,1,5,kinem.p1());
      I2D4stuEval(7,ep,2,3,5,1,4,kinem.s15());

      I2D4stuEval(8,ep,2,4,5,1,3,kinem.s23());


      I2D4stuEval(9,ep,3,4,5,1,2,kinem.p2());
    }

    fEval[E_I2D4stu+ep]=true;
  }
  int idx=im3(s,t,u)-10;
  return pI2D4stu[ep][idx];
}

void Minor5::I2D4stuEval(int idx, int ep, int s, int t, int u, int m, int n, double qsq)
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
        sum1=(13*msq1 + 12.*ICache::getI1(ep, Kinem1(msq1)))*msq1*msq1*msq1/288.;
      }
      else {
        const double msq1p4=(msq1*msq1)*(msq1*msq1);
        const double msq2p4=(msq2*msq2)*(msq2*msq2);
        sum1= (77*(msq1*msq1p4-msq2*msq2p4)/7200
              + ( + msq1p4*ICache::getI1(ep, Kinem1(msq1))
                  - msq2p4*ICache::getI1(ep, Kinem1(msq2))
                  )/120.)/mm12;
      }
    }
    else {
      ncomplex sumX=9.*I2D3stu(ep,s,t,u)+2.*I2D3stu(ep+1,s,t,u);
      ncomplex sumY=-(36.*ICache::getI1(ep, Kinem1(msq2))+47*msq2)*msq2*msq2*msq2/96.;
      ncomplex sumZ=-(36.*ICache::getI1(ep, Kinem1(msq1))+47*msq1)*msq1*msq1*msq1/96.;

      const double ds0tu=(Cay[nss(m,m)]*Cay[nss(n,n)]-Cay[nss(m,n)]*Cay[nss(m,n)]);
      sum1+=sumX*ds0tu;

      const double dsvtuY=-(Cay[nss(n,n)]-Cay[nss(m,n)]); /* minus sign of minor v=m */
      sum1-=sumY*dsvtuY;

      const double dsvtuZ=+(Cay[nss(m,n)]-Cay[nss(m,m)]); /* plus sign of minor v=n */
      sum1-=sumZ*dsvtuZ;

      sum1/=81*dstustu;
    }
    /* printf("I2D4stu(%e,%e,%e)^%d = %e,%e\n",-0.5*dstustu,msq1,msq2,ep,sum1.real(),sum1.imag());
    */
  }
  else { assert(ep==1);
    const double y11=Cay[nss(m,m)];
    const double y12=Cay[nss(m,n)];
    const double y22=Cay[nss(n,n)];
    sum1=(
          +y11*y11*(y11*(35*(y11+y12)+5*y22)+15*y12*(2*y12+y22))
          +y22*y22*(y22*(35*(y22+y12)+5*y11)+15*y12*(2*y12+y11))
          +y12*y12*(y12*(8*y12+20*y11+20*y22)+24*y11*y22)
          +3*y11*y11*y22*y22
            )/120960;
  }
  pI2D4stu[ep][idx]=sum1;
}

/* --------------------------------------------------------
 *   I3D5st triangle in D+10 dim
 * --------------------------------------------------------
 */
ncomplex Minor5::I3D5st(int ep, int s, int t)
{
  assert(s!=t); //if (s==t) return 0;
  if (ep==2) return 0;
  if (not fEval[E_I3D5st+ep]) {
    I3D5stEval(ep);
  }
  int idx = im2(s,t)-5;
  return pI3D5st[ep][idx];
}

void Minor5::I3D5stEval(int ep)
{
  for (int s=1; s<=smax; s++) {
    for (int t=s+1; t<=5; t++) {
      int idx = im2(s,t)-5;

      const double dstst=M2(s,t,s,t);
      ncomplex ivalue=0;

      if (pmaxS3[idx] <= deps) {
        printf("I3D5 - M2({%d,%d}) <= eps\n",s,t);
        ivalue=std::numeric_limits<double>::quiet_NaN();
      }
      else {
        double cf=1./10.;
        for (int ei=ep; ei<=1; ei++) {
          ncomplex sum1=M3(0,s,t,0,s,t)*I3D4st(ei, s, t);
          for (int u=1; u<=5; u++) {
            if (t==u || s==u) continue;
            sum1-=M3(u,s,t,0,s,t)*I2D4stu(ei, s, t, u);
          }
          ivalue+=cf*sum1;
          cf*=1./5.;
        }
        ivalue/=dstst;
      }
      pI3D5st[ep][idx]=ivalue;
    }
  }
  fEval[E_I3D5st+ep]=true;
}

/* --------------------------------------------------------
 *   I2D5stu bubble in D+10 dim
 * --------------------------------------------------------
 */
ncomplex Minor5::I2D5stu(int ep, int s, int t, int u)
{
  assert(t!=u && u!=s && s!=t); //if (t==u || u==s || s==t) return 0;
  if (ep==2) return 0;
  if (not fEval[E_I2D5stu+ep]) {
    I2D5stuEval(0,ep,1,2,3,4,5,kinem.p5());
    I2D5stuEval(1,ep,1,2,4,3,5,kinem.s45());
    I2D5stuEval(2,ep,1,2,5,3,4,kinem.p4());

    I2D5stuEval(3,ep,1,3,4,2,5,kinem.s12());
    I2D5stuEval(4,ep,1,3,5,2,4,kinem.s34());

    I2D5stuEval(5,ep,1,4,5,2,3,kinem.p3());

    if (smax==5) {
      I2D5stuEval(6,ep,2,3,4,1,5,kinem.p1());
      I2D5stuEval(7,ep,2,3,5,1,4,kinem.s15());

      I2D5stuEval(8,ep,2,4,5,1,3,kinem.s23());


      I2D5stuEval(9,ep,3,4,5,1,2,kinem.p2());
      }

    fEval[E_I2D5stu+ep]=true;
  }
  int idx=im3(s,t,u)-10;
  return pI2D5stu[ep][idx];
}

void Minor5::I2D5stuEval(int idx, int ep, int s, int t, int u, int m, int n, double qsq)
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
        sum1=-(77*msq1 + 60.*ICache::getI1(ep, Kinem1(msq1)))*(msq1*msq1)*(msq1*msq1)/7200.;
      }
      else {
        const double msq1p5=(msq1*msq1)*(msq1*msq1)*msq1;
        const double msq2p5=(msq2*msq2)*(msq2*msq2)*msq2;
        sum1=-(29*(msq1*msq1p5-msq2*msq2p5)/14400
              + ( + msq1p5*ICache::getI1(ep, Kinem1(msq1))
                  - msq2p5*ICache::getI1(ep, Kinem1(msq2))
                  )/720.)/mm12;
      }
    }
    else {
      ncomplex sumX=11.*I2D4stu(ep,s,t,u)+2.*I2D4stu(ep+1,s,t,u);
      ncomplex sumY=(660.*ICache::getI1(ep, Kinem1(msq2))+967*msq2)*(msq2*msq2)*(msq2*msq2)/7200.;
      ncomplex sumZ=(660.*ICache::getI1(ep, Kinem1(msq1))+967*msq1)*(msq1*msq1)*(msq1*msq1)/7200.;

      const double ds0tu=(Cay[nss(m,m)]*Cay[nss(n,n)]-Cay[nss(m,n)]*Cay[nss(m,n)]);
      sum1+=sumX*ds0tu;

      const double dsvtuY=-(Cay[nss(n,n)]-Cay[nss(m,n)]); /* minus sign of minor v=m */
      sum1-=sumY*dsvtuY;

      const double dsvtuZ=+(Cay[nss(m,n)]-Cay[nss(m,m)]); /* plus sign of minor v=n */
      sum1-=sumZ*dsvtuZ;

      sum1/=121*dstustu;
    }
  }
  else { assert(ep==1);
    const double y11=Cay[nss(m,m)];
    const double y12=Cay[nss(m,n)];
    const double y22=Cay[nss(n,n)];
    sum1=-(
            y11*y11*y11*(y11*(63*(y11+y12)+7*y22)+7*y12*(8*y12+3*y22)+3*y22*y22)
          + y22*y22*y22*(y22*(63*(y22+y12)+7*y11)+7*y12*(8*y12+3*y11)+3*y11*y11)
          + y12*y12*y12*(8*y12*(y12+3*y11+3*y22)+6*(7*y11*y11+4*y11*y22+7*y22*y22))
          + y11*y22*y12*(4*y12*(9*(y11+y22)+4*y12)+15*y11*y22)
            )/2661120;
  }
  pI2D5stu[ep][idx]=sum1;
}

/* --------------------------------------------------------
 *   I3D6st triangle in D+12 dim
 * --------------------------------------------------------
 */
ncomplex Minor5::I3D6st(int ep, int s, int t)
{
  assert(s!=t); //if (s==t) return 0;
  if (ep==2) return 0;
  if (not fEval[E_I3D6st+ep]) {
    I3D6stEval(ep);
  }
  int idx = im2(s,t)-5;
  return pI3D6st[ep][idx];
}

void Minor5::I3D6stEval(int ep)
{
  for (int s=1; s<=smax; s++) {
    for (int t=s+1; t<=5; t++) {
      int idx = im2(s,t)-5;

      const double dstst=M2(s,t,s,t);
      ncomplex ivalue=0;

      if (pmaxS3[idx] <= deps) {
        printf("I3D6 - M2({%d,%d}) <= eps\n",s,t);
        ivalue=std::numeric_limits<double>::quiet_NaN();
      }
      else {
        double cf=1./12.;
        for (int ei=ep; ei<=1; ei++) {
          ncomplex sum1=M3(0,s,t,0,s,t)*I3D5st(ei, s, t);
          for (int u=1; u<=5; u++) {
            if (t==u || s==u) continue;
            sum1-=M3(u,s,t,0,s,t)*I2D5stu(ei, s, t, u);
          }
          ivalue+=cf*sum1;
          cf*=1./6.;
        }
        ivalue/=dstst;
      }
      pI3D6st[ep][idx]=ivalue;
    }
  }
  fEval[E_I3D6st+ep]=true;
}

/* --------------------------------------------------------
 *   I2D6stu bubble in D+12 dim
 * --------------------------------------------------------
 */
ncomplex Minor5::I2D6stu(int ep, int s, int t, int u)
{
  assert(t!=u && u!=s && s!=t); //if (t==u || u==s || s==t) return 0;
  if (ep==2) return 0;
  if (not fEval[E_I2D6stu+ep]) {
    I2D6stuEval(0,ep,1,2,3,4,5,kinem.p5());
    I2D6stuEval(1,ep,1,2,4,3,5,kinem.s45());
    I2D6stuEval(2,ep,1,2,5,3,4,kinem.p4());

    I2D6stuEval(3,ep,1,3,4,2,5,kinem.s12());
    I2D6stuEval(4,ep,1,3,5,2,4,kinem.s34());

    I2D6stuEval(5,ep,1,4,5,2,3,kinem.p3());

    if (smax==5) {
      I2D6stuEval(6,ep,2,3,4,1,5,kinem.p1());
      I2D6stuEval(7,ep,2,3,5,1,4,kinem.s15());

      I2D6stuEval(8,ep,2,4,5,1,3,kinem.s23());


      I2D6stuEval(9,ep,3,4,5,1,2,kinem.p2());
    }

    fEval[E_I2D6stu+ep]=true;
  }
  int idx=im3(s,t,u)-10;
  return pI2D6stu[ep][idx];
}

void Minor5::I2D6stuEval(int idx, int ep, int s, int t, int u, int m, int n, double qsq)
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
        sum1=(29*msq1 + 20.*ICache::getI1(ep, Kinem1(msq1)))*(msq1*msq1)*(msq1*msq1)*msq1/14400.;
      }
      else {
        const double msq1p6=(msq1*msq1)*(msq1*msq1)*(msq1*msq1);
        const double msq2p6=(msq2*msq2)*(msq2*msq2)*(msq2*msq2);
        sum1=(223*(msq1*msq1p6-msq2*msq2p6)/705600
              + ( + msq1p6*ICache::getI1(ep, Kinem1(msq1))
                  - msq2p6*ICache::getI1(ep, Kinem1(msq2))
                  )/5040.)/mm12;
      }
    }
    else {
      ncomplex sumX=13.*I2D5stu(ep,s,t,u)+2.*I2D5stu(ep+1,s,t,u);
      ncomplex sumY=-(260.*ICache::getI1(ep, Kinem1(msq2))+417*msq2)*(msq2*msq2)*(msq2*msq2)*msq2/14400.;
      ncomplex sumZ=-(260.*ICache::getI1(ep, Kinem1(msq1))+417*msq1)*(msq1*msq1)*(msq1*msq1)*msq1/14400.;

      const double ds0tu=(Cay[nss(m,m)]*Cay[nss(n,n)]-Cay[nss(m,n)]*Cay[nss(m,n)]);
      sum1+=sumX*ds0tu;

      const double dsvtuY=-(Cay[nss(n,n)]-Cay[nss(m,n)]); /* minus sign of minor v=m */
      sum1-=sumY*dsvtuY;

      const double dsvtuZ=+(Cay[nss(m,n)]-Cay[nss(m,m)]); /* plus sign of minor v=n */
      sum1-=sumZ*dsvtuZ;

      sum1/=169*dstustu;
    }
  }
  else { assert(ep==1);
    const double y11=Cay[nss(m,m)];
    const double y12=Cay[nss(m,n)];
    const double y22=Cay[nss(n,n)];
    sum1=(
      y11*y11*y11*(y11*(21*y11*(11*(y11+y12)+y22)+210*y12*y12+7*y22*y22+63*y22*y12)
      +y12*y12*(168*y12+112*y22))+y22*y22*y22*(y22*(21*y22*(11*(y22
      +y12)+y11)+210*y12*y12+7*y11*y11+63*y11*y12)+y12*y12*(168*y12+112*y11))
      +y12*y12*(y12*y12*(16*y12*y12+112*(y11*y11+y22*y22)+56*y12*(y11+y22)
      +120*y11*y22)+y11*y22*(90*y11*y22+140*y12*(y22+y11)))+y11*y11*y22*y22*(35*(y11+y22)*y12+5*y11*y22)
      )/138378240;
  }
  pI2D6stu[ep][idx]=sum1;
}

/* --------------------------------------------------------
 *   I3D7st triangle in D+14 dim
 * --------------------------------------------------------
 */
ncomplex Minor5::I3D7st(int ep, int s, int t)
{
  assert(s!=t); //if (s==t) return 0;
  if (ep==2) return 0;
  if (not fEval[E_I3D7st+ep]) {
    I3D7stEval(ep);
  }
  int idx = im2(s,t)-5;
  return pI3D7st[ep][idx];
}

void Minor5::I3D7stEval(int ep)
{
  for (int s=1; s<=smax; s++) {
    for (int t=s+1; t<=5; t++) {
      int idx = im2(s,t)-5;

      const double dstst=M2(s,t,s,t);
      ncomplex ivalue=0;

      if (pmaxS3[idx] <= deps) {
        printf("I3D7 - M2({%d,%d}) <= eps\n",s,t);
        ivalue=std::numeric_limits<double>::quiet_NaN();
      }
      else {
        double cf=1./14.;
        for (int ei=ep; ei<=1; ei++) {
          ncomplex sum1=M3(0,s,t,0,s,t)*I3D6st(ei, s, t);
          for (int u=1; u<=5; u++) {
            if (t==u || s==u) continue;
            sum1-=M3(u,s,t,0,s,t)*I2D6stu(ei, s, t, u);
          }
          ivalue+=cf*sum1;
          cf*=1./7.;
        }
        ivalue/=dstst;
      }
      pI3D7st[ep][idx]=ivalue;
    }
  }
  fEval[E_I3D7st+ep]=true;
}

