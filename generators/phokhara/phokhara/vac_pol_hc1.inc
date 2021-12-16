*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
* THIS VERSION IS CHANGED FOR TESTS:
* INCLUDES LEPTONIC PART + HADRONIC PART WITH NARROW RESONANCES SUBTRACTED
* NARROW RESONANCES == PHI, J/PSI, Ypsilon
*
*;;;;;;;;;;;;;;;;;;;;;;;;;;;; -*- Mode: Fortran -*- ;;;;;;;;;;;;;;;;;;;;;;;;;;;
*; alphaQED.f --- 
*; Author          : Fred Jegerlehner
*; Created On      : Sat Nov  1 22:49:46 2003
*; Last Modified By: Fred Jegerlehner
*; Last Modified On: Mon Nov  3 02:23:59 2003
*; RCS: $Id$
*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
*; Copyright (c) 2003 Fred Jegerlehner
*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
*; 
c      PROGRAM alphaQEDn
C Calculating the running QED coupling alpha(E)
C Leptons: 1--loop, 2--loop exact, 3--loop in high energy approximation
C Quarks form routine dhadr5n including effective s--channel shifts in low energy range
C r1....,r2....,r3.... : oneloop, twoloop, threeloop result
C  ..real: realpart, ..imag: imaginary part;
C ......l: light fermions only (high energy approximation for 3--loops) 
C dallepQED1n=r1real,dallepQED2n=r2real,dallepQED3l=r3reall
C dallepQED3l also returns oneloop and twoloop high energy (light fermion) approximations
c!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
c
c H.Czyz: - the constants() are called in subroutine 'input' of PHOKHARA
c where also the vacpol at 's' is calculated for FSR contributions;
c different scales (Q^2, Q_a^2, Q_b^2) contributions are calculated
c in proper places of PHOKHARA. The function dggvap is now complex*16
c and contains also the imaginary part of the vacpol and is used
c as additional factor 1.d0/(1.d0-dggvap(s, 0.d0)) in the amplitudes.
c Few commons were added to 'common.inc', previously written separately
c
c!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
c
c      call constants()
C LEPTONflag=all,had,lep,ele,muo,tau -> iLEP=-3,-2,-1,1,2,3
C Default: 
C      LEPTONflag='all'
C      iLEP  = -3  ! for sum of leptons + quarks              
C Decomment the folowwing two lines for changing the default
c      LEPTONflag='tau'
c      iLEP=LFLAG(LEPTONflag)
************************************************************************
c
c **********************************************************************
c **********************************************************************

      subroutine constants()
      implicit none
      include 'common.inc'
      data pi,pi2,ln2/3.141592653589793d0,9.869604401089358d0,
     &                0.6931471805599453d0/    
      data zeta2,zeta3/1.644934066848226d0,1.20205690315959d0/
      data zeta5/1.036927755143370d0/
      DATA ALP/0.0072973544D0/
      data ml /0.51099906d-3,105.658389d-3,1.7771d0/
      data small,large,large_3/1.d-4,1.d6,8.d0/
      save
      adp   = alp/pi
      adp2  = adp*adp
      adp3  = adp*adp2
C switch off/on warnings iwarnings=0/1 default is 1 
c H.C. - warnings set to null !!!!!!!!!!!!!!!!!!!!!!!!!!!
      iwarnings=0
      iwarnings=1
C LEPTONflag=all,had,lep,ele,muo,tau -> iLEP=-3,-2,-1,1,2,3
      LEPTONflag='all'
      iLEP  = -3  ! for sum of leptons + quarks
C Overwrite default switches for replacing exact by asymptotic expansions 
C (required to enforce numerical stability)
C Do not change without checking whethe new switches give acceptable results
C z=|s|/m^2 (threshold= 4 m^2), 
C low energy expansion up to O(z^3): c_1*z+c_2*z^2
C high energy expansion up to O(z^-3): c_1/z+c_2/z^2 up to logs
C
*      small=1d-6   !  default 
*      large=1d6    !  default 
C
C corresponding switch for 3--loop high energy approximation 
C where exact result is not available at lower energies 
C i.e. below large_3*|s| 3--loop contribution is taken to be zero
C
*     large_3=8.d0  ! default  
C

      st2=0.23d0        ! Reference value for weak mixing parameter
      als=0.118d0       ! alpha strong
      mtop=175.d0       ! top quark mass

      return
      end

c ***************************************************************************
c ***************************************************************************

       FUNCTION dggvap(sp,erfl)
C      -----------------------
C Fermionic contributions to the running of alpha
C Function collecting 1-loop, 2-loop, 3-loop leptonic contributions (e, mu, tau) 
C + hadronic contribution from light hadrons (udscb)
C + top quark contribution (t); here at 1-loop (good up to 200 GeV LEP I/II) 
C erfl is the errorflag: 0.d0->central value +/- 1.d0 -> upper/lower bound
       implicit none
       complex*16 dggvap
       real*8 sp,erfl
       real*8 c0
       real*8 mtop2
       real*8 dallepQED1,dallepQED2,dallepQED3l,
     &        dalQED1ferm,res_re,res_im
       real*8 r1real,r1imag,r2real,r2imag,imag
       real*8 r1reall,r1imagl,r2reall,r2imagl,r3reall,r3imagl
       real*8 fac,ee,e,der,errder,deg,errdeg,der_help
       real*8 hfun
       include 'common.inc'      
       e=dsqrt(dabs(sp))
       res_im=0.d0
       if (sp.lt.0.d0) e=-e
       if ((LEPTONflag.eq.'all').or.(LEPTONflag.eq.'had')) then
c !!!!!!!!!!!
         if((e.gt.0.9d0).and.(e.lt.1.1d0))then
          call dhadr5n(0.9d0,st2,der,errder,deg,errdeg)
          call dhadr5n(1.1d0,st2,der_help,errder,deg,errdeg)
          der = der+(e-0.9d0)/(1.1d0-0.9d0)*(der_help-der)
         elseif((e.gt.2.4d0).and.(e.lt.3.5d0))then
          call dhadr5n(2.4d0,st2,der,errder,deg,errdeg)
          call dhadr5n(3.5d0,st2,der_help,errder,deg,errdeg)
          der = der+(e-2.4d0)/(3.5d0-2.4d0)*(der_help-der)
         elseif((e.gt.3.5d0).and.(e.lt.4.d0))then
          call dhadr5n(3.5d0,st2,der,errder,deg,errdeg)
          call dhadr5n(4.0d0,st2,der_help,errder,deg,errdeg)
          der = der+(e-3.5d0)/(4.d0-3.5d0)*(der_help-der)
         else
          call dhadr5n(e,st2,der,errder,deg,errdeg)
         endif
c !!!!!!!!!!!
          dahadr=der
       else 
          dahadr=0.d0
       endif
       if ((LEPTONflag.eq.'had').or.(LEPTONflag.eq.'top')) then
          dalept=0.d0
       else 
          r1real =dallepQED1(sp,r1imag)
          r2real =dallepQED2(sp,r2imag)
c !!!!!!!!!!!!!!!
c the third loop switched of as it is not reliable in low energy region
c          r3reall=dallepQED3l(sp,r1reall,
c     &                          r1imagl,r2reall,r2imagl,r3imagl)
          r3reall = 0.d0
          r3imagl = 0.d0
c!!!!!!!!!!!!!!!
          res_re=r1real+r2real+r3reall
          res_im=r1imag+r2imag+r3imagl
          dalept=res_re
       endif
       if ((LEPTONflag.eq.'all').or.(LEPTONflag.eq.'top')) then
          mtop2=mtop*mtop
          daltop=dalQED1ferm(sp,mtop2,imag)*4.d0/3.d0
       else 
          daltop=0.d0
       endif
       dggvap=dcmplx(dalept+dahadr+erfl*errder,res_im)
       RETURN
       END
c ***********************************************************************
c ***********************************************************************

      function dallepQED1(sp,r1imag)
      implicit none
      integer i
      real *8 dallepQED1,sp,m2,r1real,r1imag,x0,y0,dalQED1ferm,imag
      include 'common.inc'
      x0  = 0.d0
      y0  = 0.d0
      if ((LEPTONflag.eq.'had').or.(LEPTONflag.eq.'top')) then
         dallepQED1=0.d0
         r1imag=0.d0
         return
      endif
      if ((LEPTONflag.eq.'all').or.(LEPTONflag.eq.'lep')) then
         do i=1,3 
            m2=ml(i)**2
            x0=x0+dalQED1ferm(sp,m2,imag)
            y0=y0+imag
         enddo
      else 
         iLEP=LFLAG(LEPTONflag)
         m2=ml(iLEP)**2
         x0=x0+dalQED1ferm(sp,m2,imag)
         y0=y0+imag
      endif
      r1real=x0
      r1imag=y0
      dallepQED1=r1real
      return
      end
C      
      function dalQED1ferm(sp,m2,imag)
      implicit none
      integer ini
      real *8 dalQED1ferm,dalQED1,sp,m2,imag,x
      real *8 s0,m2ds,m2ds2,lnm2ds,z,r,y,y2,y3,epy,emy,
     &        epysq,emysq,emy2,emy3,emy4,lny,abr1,ddilog,dli3,
     &        lnm2dsimag,lnyimag,tau,sint,sint2,sint3,cost
      real *8 null,one,two,four,eight,half,fourth,fourthird,
     &        twentyninth,sixteenthird,c1,c2
      include 'common.inc'
      save
      data ini/0/
      if (ini.eq.0) goto 2
 1    continue
C switches for z=sp/m2 input via common.inc from constants.f
c      small=eps
c      large=one/eps
      s0=four*m2                       ! threshold
      z=sp/m2
      x=s0/sp
      imag=null
C LOW ENERGY ASYMPTOTE
      if (abs(z).lt.small) then 
c         write (*,*) ' LOW ENERGY ASYMPTOTE'
         dalQED1 = c1*z+c2*z*z
         goto 9
      endif
C TIME-LIKE BRANCH      
      if (z.gt.null) then
C HIGH ENERGY ASYMPTOTE (TIME-LIKE)
      if (z.gt.large) then
c         write (*,*) ' HIGH ENERGY ASYMPTOTE (TIME-LIKE)'
         m2ds=m2/sp
         lnm2ds=log(m2ds)
         lnm2dsimag=null
         if (sp.gt.s0) lnm2dsimag=pi
         dalQED1 =  -fourthird*lnm2ds-twentyninth
     &              +eight*m2ds*(one+m2ds*(lnm2ds-half))
         imag    =  lnm2dsimag*(-fourthird+eight*m2ds*m2ds)
      else if (sp.gt.s0) then 
         r=sqrt(one-x)
         y=(r-one)/(r+one)
C for analytic continuation y = y + i epsilon
         y2=y*y
         epy=one+y
         emy=one-y
         epysq=one+y2
         emy2=emy*emy
         emy3=emy2*emy
         lny=log(abs(y))
         abr1=epysq-four*y
         lnyimag=null
         if (sp.gt.s0) lnyimag=pi 
         dalQED1 = -twentyninth+sixteenthird*y/emy2
     &             -fourthird*epy*abr1/emy3*lny
         imag    = lnyimag*(-fourthird*epy*abr1/emy3)
      else 
         sint2=sp/s0
         sint =dsqrt(sint2)
         sint3=sint2*sint
         tau=dasin(sint)
         cost=cos(tau)
         dalQED1 = -twentyninth + fourthird*
     &             (tau*cost*(one + two*sint2) - sint)/sint3
      endif
C SPACE-LIKE BRANCH      
      else
      if (-z.gt.large) then
C HIGH ENERGY ASYMPTOTE (SPACE-LIKE)
c         write (*,*) ' HIGH ENERGY ASYMPTOTE (SPACE-LIKE)'
         m2ds=-m2/sp
         lnm2ds=log(m2ds)
         dalQED1 =  -fourthird*lnm2ds-twentyninth
     &              +eight*m2ds*(one+m2ds*(lnm2ds-half))
         imag    = null
      else 
         r=sqrt(one-x)
         y=(r-one)/(r+one)
C for analytic continuation y = y + i epsilon
         y2=y*y
         epy=one+y
         emy=one-y
         epysq=one+y2
         emy2=emy*emy
         emy3=emy2*emy
         lny=log(y)
         abr1=epysq-four*y
         lnyimag=null
         if (sp.gt.s0) lnyimag=pi 
         dalQED1 = -twentyninth+sixteenthird*y/emy2
     &             -fourthird*epy*abr1/emy3*lny
         imag    = null
      endif
      endif
 9    dalQED1ferm=dalQED1*adp*fourth
      imag=imag*adp*fourth
      RETURN
 2    null=0.d0
      one=1.d0
      two=2.d0
      four=4.d0
      eight=8.d0
      half=0.5d0
      fourth=0.25d0
      fourthird=4.d0/3.d0
      twentyninth=20.d0/9.d0
      sixteenthird=16.d0/3.d0
      c1=-4.d0/15.d0
      c2=-1.d0/35.d0
      ini=1
c      write (*,*) ' numerical constants initialized'
      goto 1
      END      


      integer function LFLAG(Label) 
      implicit none
      character*3 Label
      integer i
      i=-3
      if (Label.eq.'had') i=-2 
      if (Label.eq.'lep') i=-1 
      if (Label.eq.'ele') i= 1 
      if (Label.eq.'muo') i= 2 
      if (Label.eq.'tau') i= 3 
      LFLAG=i
      return
      end

      function dallepQED2(sp,r2imag)
C Källen&Saby result 2--loop QEC = 2--loop QCD divided by NC=3 and CF=4/3 
      implicit none
      integer i,nl
      real *8 dallepqed2,sp,x1,y1,x1l,r2real,r2imag,
     &        dalQED2ferm,m2,imag,M1
      real *8 null,one,two,half,four,qua
      include 'common.inc'
      data null,one,two,half,four,qua/0.d0,1.d0,2.d0,.5d0,4.d0,.25d0/
      x1  = null
      y1  = null
      if ((LEPTONflag.eq.'had').or.(LEPTONflag.eq.'top')) then
         dallepQED2=0.d0
         r2imag=0.d0
         return
      endif
      nl=1
      iLEP=LFLAG(LEPTONflag)
      if ((LEPTONflag.eq.'all').or.(LEPTONflag.eq.'lep')) nl=3
      do i=1,nl
         M1=ml(i)
         if (nl.eq.1) M1=ml(iLEP)
C
         m2=M1**2
         x1=x1+dalQED2ferm(sp,m2,imag)
         y1=y1+imag
      enddo
      r2real=x1
      r2imag=y1
      dallepqed2 = r2real
      return
      end
C      
      function dalQED2ferm(sp,m2,imag)
C Exact 2-loop result, originally by Kallen and Sabry 1955, 
C here based on a compact form worked out by M.Yu. Kalmykov 2003 
      implicit none
      integer ini
      real *8 dalQED2ferm,dalQED2,sp,m2,imag,x
      real *8 s0,m2ds,m2ds2,lnm2ds,z,omx,r,y,y2,y3,y4,epy,emy,
     &        epysq,emysq,emy2,emy3,emy4,lny,abr1,ddilog,dli3,
     &        lnm2dsimag,lnyimag,sint,sint2,sint3,sint4,sint4i,
     &        tau,cost,phi,chi,fac1,term2,Clausen2,Clausen3
      real *8 null,one,two,three,four,five,six,seven,eight,fourty,
     &        twentytwo,thirtytwo,fourtyeight,third,onesixteenth,
     &        tenthird,hundertfourthird,sixteenzeta3,eightthird,
     &        sixteenthird,thirtytwothird,sixtyfourzeta3,c1,c2,
     &        fourth,sixteen,fourteenthird,twentysixthird
      include 'common.inc'
      save
      data ini/0/
      if (ini.eq.0) goto 2
 1    continue
C switches for z=sp/m2 input via common.inc from constants.f
c      small=eps
c      large=one/eps
      s0=four*m2                       ! threshold
      z=sp/m2
      x=s0/sp
      omx=one-x
      imag=null
      lnm2dsimag=null
C LOW ENERGY ASYMPTOTE
      if (abs(z).lt.small) then 
c         write (*,*) ' LOW ENERGY ASYMPTOTE'
         dalQED2 = c1*z+c2*z*z
c         write (*,*) sp,dalQED2*adp2*onesixteenth
         goto 9
      endif
C TIME-LIKE BRANCH      
      if (z.gt.null) then
C HIGH ENERGY ASYMPTOTE (TIME-LIKE)
      if (z.gt.large) then
c         write (*,*) ' HIGH ENERGY ASYMPTOTE (TIME-LIKE)'
         m2ds=m2/sp
         m2ds2=m2ds*m2ds
         lnm2ds=log(m2ds)
         lnm2dsimag=pi
         dalQED2 =  -four*lnm2ds-tenthird+sixteenzeta3
     &              +fourtyeight*m2ds*lnm2ds
     &              -m2ds2*(eightthird+sixtyfourzeta3+fourty*lnm2ds
     &              -fourtyeight*(lnm2ds**2-lnm2dsimag**2))
         imag    = lnm2dsimag*(-four+fourtyeight*m2ds
     &              -m2ds2*(fourty-fourtyeight*two*lnm2ds))
      else if (sp.gt.s0) then 
         r=sqrt(omx)
         y=(r-one)/(r+one)
C for analytic continuation y = y + i epsilon
         y2=y*y
         y3=y*y2
         y4=y*y3
         epy=one+y
         emy=one-y
         epysq=one+y2
         emysq=one-y2
         emy2=emy*emy
         emy3=emy2*emy
         emy4=emy2*emy2
         lny=log(abs(y))
         abr1=epysq-four*y
         lnyimag=pi 
         dalQED2 = (-tenthird + hundertfourthird*y/emy2
     &        + sixteenzeta3 *(one - four*y2/emy4)
     &    +(  + eightthird*y*(two+seven*y-twentytwo*y2+six*y3)*
     &        (lny**2-lnyimag**2)
     &        - four*emysq*(epysq-eight*y)*lny
     &      +(- sixteenthird*(log(emy)
     &          + two*log(epy))*(lny*(epysq*lny - two*emysq)
     &          -lnyimag*(epysq*lnyimag))
     &        + thirtytwothird*(ddilog(y) + two*ddilog(-y))
     &          *(emysq - two*epysq*lny)
     &        + thirtytwo*epysq*(dli3(y) + two*dli3(-y)))*abr1)/emy4)
         imag    = lnyimag*(
     &    +(  + eightthird*y*(two+seven*y-twentytwo*y2+six*y3)*two*lny
     &        - four*emysq*(epysq-eight*y)
     &      +(- sixteenthird*(log(emy) + two*log(epy))
     &        *(two*epysq*lny - two*emysq)
     &        - two*thirtytwothird*(ddilog(y) + two*ddilog(-y))*epysq
     &                                                  )*abr1)/emy4)
      else 
         sint2=sp/s0
         sint =dsqrt(sint2)
         sint3=sint2*sint
         sint4=sint3*sint
         tau=dasin(sint)
         cost=cos(tau)
         phi=two*tau
         chi=pi-phi
         sint4i=one/sint4
         fac1=one-fourth*sint4i
         term2=cost*(one+two*sint2)/sint3
         dalQED2 = sixteen*
     &            (two*Clausen3(phi)+four*Clausen3(chi)+zeta3)*fac1
     &           + sixteenthird*(Clausen2(phi)-two*Clausen2(chi))*
     &            (eight*tau*fac1-term2)
     &           + thirtytwothird*(log(two*sint)+two*log(two*cost))*
     &            (two*tau*fac1-term2)*tau -tenthird
     &           + four*tau*cost*(three+two*sint2)/sint3
     &           - twentysixthird/sint2 + (fourteenthird/sint4
     &           + sixteenthird/sint2 - thirtytwo)*tau**2
         imag    = null
      endif
C SPACE-LIKE BRANCH      
      else
      if (-z.gt.large) then
C HIGH ENERGY ASYMPTOTE (SPACE-LIKE)
c         write (*,*) ' HIGH ENERGY ASYMPTOTE (SPACE-LIKE)'
         m2ds=-m2/sp
         m2ds2=m2ds*m2ds
         lnm2ds=log(m2ds)
         dalQED2 =  -four*lnm2ds-tenthird+sixteenzeta3
     &              +fourtyeight*m2ds*lnm2ds
     &              -m2ds2*(eightthird+sixtyfourzeta3+fourty*lnm2ds
     &              -fourtyeight*lnm2ds**2)
         imag    = null
      else 
         r=sqrt(omx)
         y=(r-one)/(r+one)
C for analytic continuation y = y + i epsilon
         y2=y*y
         y3=y*y2
         epy=one+y
         emy=one-y
         epysq=one+y2
         emysq=one-y2
         emy2=emy*emy
         emy3=emy2*emy
         emy4=emy2*emy2
         lny=log(y)
         abr1=epysq-four*y
         dalQED2 = (-tenthird + hundertfourthird*y/emy2
     &        + sixteenzeta3 *(one - four*y2/emy4)
     &    +(  + eightthird*y*(two+seven*y-twentytwo*y2+six*y3)*lny**2
     &        - four*emysq*(epysq-eight*y)*lny
     &      +(- sixteenthird*(log(emy)
     &          + two*log(epy))*lny*(epysq*lny - two*emysq)
     &        + thirtytwothird*(ddilog(y) + two*ddilog(-y))
     &          *(emysq - two*epysq*lny)
     &        + thirtytwo*epysq*(dli3(y) + two*dli3(-y)))*abr1)/emy4)
         imag    = null
      endif
      endif
 9    dalQED2ferm=dalQED2*adp2*onesixteenth
      imag=imag*adp2*onesixteenth
      RETURN
 2    null=0.d0
      one=1.d0
      two=2.d0
      three=3.d0
      four=4.d0
      five=5.d0
      six=6.d0
      seven=7.d0
      eight=8.d0
      sixteen=16.d0
      fourty=40.d0
      twentytwo=22.d0
      thirtytwo=32.d0
      fourtyeight=48.d0
      third=1.d0/3.d0
      fourth=0.25d0
      onesixteenth=1.d0/16.d0
      tenthird=10.d0*third
      hundertfourthird=104.d0*third
      sixteenzeta3=16.d0*zeta3
      eightthird=8.d0*third
      fourteenthird=14.d0*third
      sixteenthird=16.d0*third
      twentysixthird=26.d0*third
      thirtytwothird=32.d0*third
      sixtyfourzeta3=64.d0*zeta3
      c1=-328.d0/81.d0
      c2=-449.d0/675.d0
      ini=1
c      write (*,*) ' numerical constants initialized'
      goto 1
      END      
C
      function dallepQED3l(sp,r1real,r1imag,r2real,r2imag,r3imag)
C Warning: only works for large sp >> m_tau**2
c      QED VP contribution 1-,2-,3-loop
c r1 1-loop contribution from light lepton
c r2 2-loop contribution from light leptons (Källen/Sabry 55)
c r3 3-loop contribution from light leptons (Steinhauser 98, PLB429(1998)158)
c      M1,M2 on-shell masses;
c      M1 valence lepton (current-current fermion loop), M2 sea lepton (extra fermion loop)
c      Piq2  = adp/4.d0*(Pi_0+adp*Pi_1
c     &     + adp**2*(PiA_2+Pil+PiF_2+Pih_2))
      implicit none
      integer i,nl,testprint
      real *8 dallepQED3l,sp,q2,M1,M12,M22,sabs,sign,fac1,fac2,fac3
      real *8 alpha,Lnqmone,Lnqmtwo,Lnqmone2,M12dq2
      real *8 iLnqmone,iLnqmtwo
      real *8 r1real,r1imag,r2real,r2imag,r3real,r3imag
      real *8 rPi_0,rPi_1,rPi_2,rPiA_2,rPil_2,rPiF_2,rPih_2,rPiq2
      real *8 iPi_0,iPi_1,iPi_2,iPiA_2,iPil_2,iPiF_2,iPih_2,iPiq2
      real *8 e,x0,y0,x1,y1,z,
     &        rxa,ixa,rxf,ixf,rxle,rxhe,ipih_2m,ipih_2t,
     &        ixle,ixhe,rxlm,rxhm,ixlm,ixhm,rxlt,ipil_2e,ipil_2m,
     &        ixlt,rxht,ixht
      include 'common.inc'
C testprint=1: print individual terms to fort.2, fort.3
      testprint=0
      fac1=-adp /4.d0
      fac2=-adp2/4.d0
      fac3=-adp3/4.d0
      sabs=dabs(sp)
      e=sqrt(sabs)
      dallepQED3l=0.d0
      r1real=0.d0
      r1imag=0.d0
      r2real=0.d0
      r2imag=0.d0
      r3real=0.d0
      r3imag=0.d0
      x0  = 0.d0
      y0  = 0.d0
      x1  = 0.d0
      y1  = 0.d0
      rxa = 0.d0
      ixa = 0.d0
      rxf = 0.d0
      ixf = 0.d0
      if ((LEPTONflag.eq.'had').or.(LEPTONflag.eq.'top')) then
         return
      endif
      nl=1
      iLEP=LFLAG(LEPTONflag)
      if ((LEPTONflag.eq.'all').or.(LEPTONflag.eq.'lep')) nl=3
      do i=1,nl
         M1=ml(i)
         if (nl.eq.1) M1=ml(iLEP)
         M12=M1**2
         z=sp/M12
         if (abs(z).lt.large_3) then
           if (iwarnings.ne.0) then
             write (*,*) ' 3--loop high energy approximation'
             write (*,*) ' out of range: result=0.0 at energy= ',e
           endif
           return
         endif
         x0=x0+rpi_0(sp,M12,ipi_0)
         y0=y0+ipi_0
         x1=x1+rpi_1(sp,M12,ipi_1)
         y1=y1+ipi_1
         rxa=rxa+rpia_2(sp,M12,ipia_2)
         ixa=ixa+ipia_2
         rxf=rxf+rpif_2(sp,M12,ipif_2)
         ixf=ixf+ipif_2
         if (testprint.eq.1) then
         write (2,*) ' m:',M1,' A:',fac3*rpia_2(sp,M12,ipia_2),
     &                       ' F:',fac3*rpif_2(sp,M12,ipif_2)
         endif
      enddo
C light-heavy contribution:
C electron
      rxle= 0.d0 
      rxhe= 0.d0 
      ixle= 0.d0 
      ixhe= 0.d0 
      if ((iLEP.eq.-1).or.(iLEP.eq.1)) then
        rxhe=rpih_2(sp,ml(2)**2,ipih_2m)+rpih_2(sp,ml(3)**2,ipih_2t)
        ixhe=ipih_2m+ipih_2t
         if (testprint.eq.1) then
         write (2,*) ' ele:',
     &       ' h-m:',fac3*rpih_2(sp,ml(2)**2,ipih_2m),
     &       ' h-t:',fac3*rpih_2(sp,ml(3)**2,ipih_2t)
         endif
      endif
C muon
      rxlm= 0.d0
      rxhm= 0.d0
      ixlm= 0.d0
      ixhm= 0.d0
      if ((iLEP.eq.-1).or.(iLEP.eq.2)) then
         rxlm=rpil_2(sp,ml(2)**2,ml(1)**2,ipil_2) 
         rxhm=rpih_2(sp,ml(3)**2,ipih_2)
         ixlm=ipil_2
         ixhm=ipih_2
         if (testprint.eq.1) then
         write (2,*) ' muo:',
     &       ' l-e:',fac3*rpil_2(sp,ml(2)**2,ml(1)**2,ipil_2),
     &       ' h-t:',fac3*rpih_2(sp,ml(3)**2,ipih_2)
         endif
      endif
C tau
      rxlt= 0.d0
      ixlt= 0.d0
      rxht= 0.d0
      ixht= 0.d0
      if ((iLEP.eq.-1).or.(iLEP.eq.3)) then
        rxlt=rpil_2(sp,ml(3)**2,ml(1)**2,ipil_2e)
     &    +rpil_2(sp,ml(3)**2,ml(2)**2,ipil_2m) 
        ixlt=ipil_2e
     &    +ipil_2m
         if (testprint.eq.1) then
         write (2,*) ' tau:',
     &       ' l-e:',fac3*rpil_2(sp,ml(3)**2,ml(1)**2,ipil_2e),
     &       ' l-m:',fac3*rpil_2(sp,ml(3)**2,ml(2)**2,ipil_2m)
         endif
      endif
C summing up      
      rPi_2=rxa+rxf+rxle+rxlm+rxlt+rxhe+rxhm+rxht
      iPi_2=ixa+ixf+ixle+ixlm+ixlt+ixhe+ixhm+ixht
      if (testprint.eq.1) then
         write (3,*) LEPTONflag
         write (3,*) fac3*rxa,fac3*rxf
         write (3,*) fac3*rxle,fac3*rxlm,fac3*rxlt
         write (3,*) fac3*rxhe,fac3*rxhm,fac3*rxht 
      endif
C
      r1real=fac1*x0
      r1imag=fac1*y0
      r2real=fac2*x1
      r2imag=fac2*y1
      r3real=fac3*rPi_2
      r3imag=fac3*iPi_2
      dallepQED3l=r3real
      return
      end
C
      function rpi_0(sp,M12,ipi_0)
      implicit none
      real *8 rpi_0,ipi_0,sp,sabs,M12,r12,Lnqmone,M12ds
      real *8 iLnqmone
      real *8 pi,pi2,ln2,zeta2,zeta3,zeta5
      common /consts/pi,pi2,ln2,zeta2,zeta3,zeta5
      sabs=abs(sp)
      M12ds=M12/sp
      Lnqmone=log(sabs/M12)
      iLnqmone=0.d0
      if (sp.gt.4.d0*M12) iLnqmone=-pi
      rPi_0  = 20.d0/9.d0 - 4.d0/3.d0*Lnqmone+8.d0*M12ds
      iPi_0  =            - 4.d0/3.d0*iLnqmone
      return
      end

      function rpi_1(sp,M12,ipi_1)
      implicit none
      real *8 rpi_1,ipi_1,sp,sabs,M12,r12,Lnqmone,M12ds
      real *8 iLnqmone
      real *8 pi,pi2,ln2,zeta2,zeta3,zeta5
      common /consts/pi,pi2,ln2,zeta2,zeta3,zeta5
      sabs=abs(sp)
      M12ds=M12/sp
      Lnqmone=log(sabs/M12)
      iLnqmone=0.d0
      if (sp.gt.4.d0*M12) iLnqmone=-pi
      rPi_1  = 5.d0/6.d0 - 4.d0*zeta3
     &       - Lnqmone - 12.d0*M12ds*Lnqmone
      iPi_1  =
     &       - iLnqmone - 12.d0*M12ds*iLnqmone
      return
      end

      function rpia_2(sp,M12,ipia_2)
      implicit none
      real *8 rpia_2,ipia_2,sp,sabs,M12,r12,Lnqmone
      real *8 iLnqmone
      real *8 pi,pi2,ln2,zeta2,zeta3,zeta5
      common /consts/pi,pi2,ln2,zeta2,zeta3,zeta5
      sabs=abs(sp)
      r12=sabs/M12
      Lnqmone=log(r12)
      iLnqmone=0.d0
      if (sp.gt.4.d0*M12) iLnqmone=-pi
      rPiA_2 = - 121.d0/48.d0 + (- 5.d0 + 8.d0*ln2)*zeta2
     &        - 99.d0/16.d0*zeta3 + 10.d0*zeta5 + 1.d0/8.d0*Lnqmone
      iPiA_2= + 1.d0/8.d0*iLnqmone
      return
      end

      function rpil_2(sp,M12,M22,ipil_2)
      implicit none
      real *8 rpil_2,ipil_2,sp,sabs,M12,r12,Lnqmone,M22,r22,Lnqmtwo
      real *8 iLnqmone,iLnqmtwo
      real *8 pi,pi2,ln2,zeta2,zeta3,zeta5
      common /consts/pi,pi2,ln2,zeta2,zeta3,zeta5
      sabs=abs(sp)
      r12=sabs/M12
      r22=sabs/M22
      Lnqmone=log(r12)
      Lnqmtwo=log(r22)
      iLnqmone=0.d0
      iLnqmtwo=0.d0
      if (sp.gt.4.d0*M12) iLnqmone=-pi
      if (sp.gt.4.d0*M22) iLnqmtwo=-pi
      rPil_2 = - 116.d0/27.d0 + 4.d0/3.d0*zeta2 + 38.d0/9.d0*zeta3
     &        + 14.d0/9.d0*Lnqmone + (5.d0/18.d0
     &        - 4.d0/3.d0*zeta3)*Lnqmtwo
     &        + 1.d0/6.d0*(Lnqmone**2-iLnqmone**2)
     &        - 1.d0/3.d0*(Lnqmone*Lnqmtwo-iLnqmone*iLnqmtwo)
      iPil_2=
     &        + 14.d0/9.d0*iLnqmone + (5.d0/18.d0
     &        - 4.d0/3.d0*zeta3)*iLnqmtwo
     &        + 1.d0/6.d0*2.d0*Lnqmone*iLnqmone
     &        - 1.d0/3.d0*(Lnqmone*iLnqmtwo+iLnqmone*Lnqmtwo)
      return
      end

      function rpif_2(sp,M12,ipif_2)
      implicit none
      real *8 rpif_2,ipif_2,sp,sabs,M12,r12,Lnqmone
      real *8 iLnqmone
      real *8 pi,pi2,ln2,zeta2,zeta3,zeta5
      common /consts/pi,pi2,ln2,zeta2,zeta3,zeta5
      sabs=abs(sp)
      r12=sabs/M12
      Lnqmone=log(r12)
      iLnqmone=0.d0
      if (sp.gt.4.d0*M12) iLnqmone=-pi
      rPiF_2 = - 307.d0/216.d0 - 8.d0/3.d0*zeta2 + 545.d0/144.d0*zeta3
     &        + (11.d0/6.d0 - 4.d0/3.d0*zeta3)*Lnqmone
     &        - 1.d0/6.d0*(Lnqmone**2-iLnqmone**2)
      iPiF_2=
     &        + (11.d0/6.d0 - 4.d0/3.d0*zeta3)*iLnqmone
     &        - 1.d0/6.d0*2.d0*Lnqmone*iLnqmone
      return
      end

      function rpih_2(sp,M22,ipih_2)
      implicit none
      real *8 rpih_2,ipih_2,sp,sabs,M22,r22,Lnqmtwo
      real *8 iLnqmtwo
      real *8 pi,pi2,ln2,zeta2,zeta3,zeta5
      common /consts/pi,pi2,ln2,zeta2,zeta3,zeta5
      sabs=abs(sp)
      r22=sabs/M22
      Lnqmtwo=log(r22)
      iLnqmtwo=0.d0
      if (sp.gt.4.d0*M22) iLnqmtwo=-pi
      rPih_2 = - 37.d0/6.d0 + 38.d0/9.d0*zeta3
     &        + (11.d0/6.d0 - 4.d0/3.d0*zeta3)*Lnqmtwo
     &        - 1.d0/6.d0*(Lnqmtwo**2-iLnqmtwo**2)
      iPih_2 =
     &        + (11.d0/6.d0 - 4.d0/3.d0*zeta3)*Lnqmtwo
     &        - 1.d0/6.d0*2.d0*(Lnqmtwo*iLnqmtwo)
      return
      end
C
       function ddilog(x)                            
C  ******************************************************************
C  *                                                                *
C  *                 program for calculating                        *
C  *      the real part of the dilogarithm for real arguments       *
C  *                                                                *
C  *      functions: ddilog(x)                                      *
C  *                 rli2(x)                                        *
C  *                 clausen2(phi)                                  *
C  *                 cl2(phi)                                       *
C  *                                                                *
C  *           F. Jegerlehner, Paul Scherrer Institute              *
C  *                                                                *
C  *                     Version: 25-OCT-1990                       *
C  *                                                                *
C  ******************************************************************
       implicit none
       real *8 pi6,null,one,two,half,qua,x,r
       real *8 ddilog,rli2,sparg,omx,clo
       common /polylog1/pi6,null,one,two,half,qua
       save   /polylog1/
       data pi6/1.644934066848226d0/                     
       data null,one,two,half,qua/0.d0,1.d0,2.d0,.5d0,.25d0/
       ddilog=0.d0
       if (x.eq.one) then
         ddilog=pi6
       return
       endif
       r=dabs(x)
       if (r.le.half) then
          ddilog=rli2(x)
          return
       else if (x.le.-two) then
          sparg=one/x 
          ddilog=-rli2(sparg)-half*dlog(-x)**2-pi6
          return
       else if (x.ge.two) then
          sparg=one/x 
          ddilog=-rli2(sparg)-half*dlog(x)**2+two*pi6
          return
       else if (x.lt.-one) then 
          sparg=one/(one-x)
          ddilog=rli2(sparg)-dlog(-x)*dlog(one-x)
     &           +half*dlog(one-x)**2-pi6
          return
       else if (x.lt.-half) then
          omx =one-x 
          sparg=one-one/omx                            
          ddilog=-rli2(sparg)-half*dlog(omx)**2
          return
       else if (x.gt.one) then
          clo=dlog(x) 
          sparg=one-one/x 
          ddilog=rli2(sparg)+half*clo**2
     &          -clo*dlog(x-one)+pi6
          return
       else if (x.gt.half) then 
          sparg=one-x                            
          ddilog=-rli2(sparg)-dlog(x)*dlog(sparg)+pi6
          return
       endif
       end

      function rli2(x)
c Spence function for real arguments of modulus smaller than one
      real *8 one,half,qua,x,rli2,b,z,z2
      integer ini
      dimension b(10)
      common /polylog2/one,half,qua,b,ini
      save   /polylog2/
      data ini/0/,one,half,qua/1.d0,.5d0,.25d0/
      if (ini.eq.0) goto 2
 1    z=-dlog(one-x)
      z2=z*z
      rli2=z*(z2*(z2*(z2*(z2*(z2*(z2*(z2*(z2*(z2*(z2*b(10)+b(9))
     &  +b(8))+b(7))+b(6))+b(5))+b(4))+b(3))+b(2))+b(1))
     &  +one-z*qua)
      return
 2       b(1) =  2.7777777777777778d-02
         b(2) = -2.7777777777777778d-04
         b(3) =  4.7241118669690098d-06
         b(4) = -9.1857730746619635d-08
         b(5) =  1.8978869988970999d-09
         b(6) = -4.0647616451442255d-11
         b(7) =  8.9216910204564526d-13
         b(8) = -1.9939295860721076d-14
         b(9) =  4.5189800296199182d-16
         b(10)= -1.0356517612181247d-17
      ini=1
      goto 1
      end

      function clausen2(phi)
c Clausens integral for arbitrary real arguments ( defined as the
c imaginary part of the complex Spence function on the unit circle 
c z=exp(i*phi) )  (2pi-periodic,odd)
      implicit none
      real *8 null,one,pi,zpi,phi,phiabs,cl2,clausen2
      common /polylog3/null,one,pi,zpi
      save   /polylog3/
      data pi /3.141592653589793d0/,zpi /6.283185307179586d0/    
      data null,one/0.d0,1.d0/
      phi=dmod(phi,zpi)
      if (phi.gt.pi) phi=phi-zpi
      phiabs=dabs(phi)
      if (phi.eq.null) then 
         clausen2=null
      else
         clausen2=phi/phiabs*cl2(phiabs)
      endif
      return
      end
         
      function cl2(phi)        
c Clausens integral for real arguments 0<phi<pi
      implicit none
      real *8 one,b,phi,z,z2,cl2,pi,pi2
      integer ini
      dimension b(15)
      common /polylog4/one,pi,pi2,b,ini
      save   /polylog4/
      data pi /3.141592653589793d0/,pi2 /1.570796326794897d0/    
      data ini/0/,one/1.d0/
      if (ini.eq.0) goto 2
 1    z=phi
      z2=z*z
      if (phi.le.pi2) then 
      if (phi.eq.0d0) then
      cl2=0d0
      return
      endif
      cl2= z*(z2*(z2*(z2*(z2*(z2*(z2*(z2*(z2*(z2*(z2* b(10)+ b(9))
     &  + b(8))+ b(7))+ b(6))+ b(5))+ b(4))+ b(3))+ b(2))+ b(1))
     &  +one-dlog(dabs(z)))
      else                                       
      cl2= z*(z2*(z2*(z2*(z2*(z2*(z2*(z2*(z2*(z2*(z2*(z2*(z2*(z2*
     & (z2*(z2* b(15)+ b(14))+ b(13))+ b(12))+ b(11))+ b(10))+ b(9))
     &  + b(8))+ b(7))+ b(6))+ b(5))+ b(4))+ b(3))+ b(2))+ b(1))
     &  +one-dlog(dabs(z)))
      endif
      return
 2       b(1) =  1.3888888888888889E-02
         b(2) =  6.9444444444444444E-05
         b(3) =  7.8735197782816830E-07
         b(4) =  1.1482216343327454E-08
         b(5) =  1.8978869988970999E-10
         b(6) =  3.3873013709535213E-12
         b(7) =  6.3726364431831804E-14
         b(8) =  1.2462059912950673E-15
         b(9) =  2.5105444608999546E-17
         b(10)=  5.1782588060906234E-19
         b(11)=  1.0887357368300848E-20
         b(12)=  2.3257441143020872E-22
         b(13)=  5.0351952131473897E-24
         b(14)=  1.1026499294381215E-25
         b(15)=  2.4386585509007344E-27
      ini=1
      goto 1
      end
C
       function dli3(x)                            
C  ******************************************************************
C  *                                                                *
C  *                 program for calculations of                    *
C  *              trilogatithms and related functions               *
C  *                                                                *
C  *      functions: dli3(x)                                        *
C  *                 rli3(x)                                        *
C  *                 rs12(x)                                        *
C  *                 clausen3(phi)                                  *
C  *                 cl3(phi)                                       *
C  *                                                                *
C  *           F. Jegerlehner, Paul Scherrer Institute              *
C  *                                                                *
C  *                     Version: 25-OCT-1990                       *
C  *                                                                *
C  ******************************************************************
c double precision calculation of the real part of the trilogarithm
       implicit none
       real *8 zeta3,zeta2,null,one,two,half,qua,i3,i6,x,r
       real *8 dli3,rli3,rs12,ddilog,sparg,omx
       real *8 clo,cloy,clom
       common /polylog5/zeta3,zeta2,null,one,two,half,qua,i3,i6
       save   /polylog5/
       data zeta2,zeta3/1.644934066848226d0,1.20205690315959d0/
       data null,one,two,half,qua,i3,i6/0.d0,1.d0,2.d0,.5d0,.25d0
     &                   ,.3333333333333333d0,.1666666666666667d0/
       dli3=0.d0
       if (x.eq.one) then
         dli3=zeta3
       return
       endif
       r=dabs(x)
       if (r.le.half) then
          dli3=rli3(x)
          return
       else if (x.le.-two) then
          sparg=one/x 
          clo=dlog(-x)
          dli3=rli3(sparg)-i6*clo**3-zeta2*clo
          return
       else if (x.ge.two) then
          sparg=one/x 
          clo=dlog(x)
c imaginary part ignored
          dli3=rli3(sparg)-i6*clo**3+two*zeta2*clo
          return
       else if (x.lt.-one) then 
          sparg=one/(one-x)
          clo =dlog(-x)
          cloy=dlog(sparg)
          clom=dlog(one-sparg)
          dli3= rs12(sparg)-rli3(sparg)+clom*ddilog(sparg)
     &           +i6*cloy**3+half*cloy*clom*clo-zeta2*clo
          return
       else if (x.lt.-half) then
          omx =one-x 
          sparg=one-one/omx                            
          clo=dlog(omx)
          dli3= rs12(sparg)-rli3(sparg)-clo*ddilog(sparg)-i6*clo**3
          return
       else if (x.gt.one) then
          clo=dlog(x) 
          sparg=one-one/x 
c imaginary part ignored
          dli3=-rs12(sparg)+clo*ddilog(sparg)+i3*clo**3
     &           -half*clo**2*dlog(x-one)+zeta2*clo+zeta3
          return
       else if (x.gt.half) then 
          clo=dlog(x) 
          sparg=one-x                            
          dli3=-rs12(sparg)-clo*ddilog(sparg)
     &           -half*clo**2*dlog(sparg)+zeta2*clo+zeta3
          return
       endif
       end

      function rli3(x)
c function Li(3) for real arguments of modulus smaller than one
      implicit none
      real *8 one,zeta3,x,rli3,b,z
      integer ini
      dimension b(21)
      common /polylog6/one,zeta3,b,ini
      save   /polylog6/
      data ini/0/,one/1.d0/
      data zeta3/1.20205690315959d0/
      if (ini.eq.0) goto 2
 1    if (x.eq.one) then
         rli3=zeta3
      return
      endif
      z=-dlog(one-x)          
      rli3=z*(z*(z*(z*(z*(z*(z*(z*(z*(z*(z*(z*(z*(z*(z*(z*(z*(z*(z*(z*
     & (z*b(20)+b(19))+b(18))+b(17))+b(16))+b(15))+b(14))+b(13))
     & +b(12))+b(11))+b(10))+b( 9))+b( 8))+b( 7))+b( 6))+b( 5))+b( 4))
     & +b(3))+b(2))+b(1))+one)
      return
C Coefficients for Li3:
C       b(0) =   1.0d0
 2      b(1) = - 3.75d-01
        b(2) =   7.8703703703703703d-02
        b(3) = - 8.6805555555555555d-03
        b(4) =   1.2962962962962963d-04
        b(5) =   8.1018518518518519d-05
        b(6) = - 3.4193571608537595d-06
        b(7) = - 1.3286564625850340d-06
        b(8) =   8.6608717561098513d-08
        b(9) =   2.5260875955320400d-08
        b(10)= - 2.1446944683640648d-09
        b(11)= - 5.1401106220129789d-10
        b(12)=   5.2495821146008294d-11
        b(13)=   1.0887754406636318d-11
        b(14)= - 1.2779396094493695d-12
        b(15)= - 2.3698241773087452d-13
        b(16)=   3.1043578879654623d-14
        b(17)=   5.2617586299125061d-15
        b(18)= - 7.5384795499492654d-16
        b(19)= - 1.1862322577752285d-16
        b(20)=   1.8316979965491383d-17
        b(21)=   2.7068171031837350d-18
      ini=1
      goto 1
      end

      function rs12(x)
c function S(1,2) for real arguments of modulus smaller than one
      implicit none
      real *8 one,two,half,qua,i6,zeta3
      real *8 x,rs12,dlog,dfloat,b,z,z2,ir
      integer ini,i
      common /polylog7/one,two,half,qua,i6,zeta3,b,ini
      save   /polylog7/
      dimension b(10)
      data ini/0/,one,two,half,qua/1.d0,2.d0,.5d0,.25d0/
      data i6,zeta3/.1666666666666667d0,1.20205690315959d0/
      if (ini.eq.0) goto 2
 1    if (x.eq.one) then
      rs12=zeta3
      return
      endif
      z=-dlog(one-x)
      z2=z*z
      rs12=z2*(z2*(z2*(z2*(z2*(z2*(z2*(z2*(z2*(z2*(z2*b(10)+b(9))
     &  +b(8))+b(7))+b(6))+b(5))+b(4))+b(3))+b(2))+b(1))
     &  +half-z*i6)*half
      return
 2       b(1) =  2.7777777777777778d-02
         b(2) = -2.7777777777777778d-04
         b(3) =  4.7241118669690098d-06
         b(4) = -9.1857730746619635d-08
         b(5) =  1.8978869988970999d-09
         b(6) = -4.0647616451442255d-11
         b(7) =  8.9216910204564526d-13
         b(8) = -1.9939295860721076d-14
         b(9) =  4.5189800296199182d-16
         b(10)= -1.0356517612181247d-17
      do i=1,10
         ir=dfloat(i)
         b(i)=b(i)*(two*ir+one)/(two*ir+two)
      enddo
      ini=1
      goto 1
      end

      function clausen3(phi)
c Clausens integral for arbitrary real arguments ( defined as the
c real part of the complex trilogarithm (Li3) on the unit circle 
c z=exp(i*phi) )  (2pi-periodic,even)
      implicit none
      real *8 null,one,pi,zpi,zeta3
      real *8 phi,phiabs,cl3,clausen3
      common /polylog8/null,one,pi,zpi,zeta3
      save   /polylog8/
      data pi /3.141592653589793d0/,zpi /6.283185307179586d0/    
      data zeta3/1.20205690315959d0/
      data null,one/0.d0,1.d0/
      phi=dmod(phi,zpi)
      if (phi.gt.pi) phi=phi-zpi
      phiabs=dabs(phi)
      if (phi.eq.null) then 
         clausen3=zeta3
      else
         clausen3=cl3(phiabs)
      endif
      return
      end
         
      function cl3(phi)        
c Clausens integral of 3rd order for real arguments 0<phi<pi
      implicit none
      real *8 one,half,threequa,b,phi,z,z2,cl3,pi,pi2,zeta3
      integer ini
      dimension b(15)
      common /polylog9/one,half,threequa,pi,pi2,zeta3,b,ini
      save   /polylog9/
      data zeta3/1.20205690315959d0/
      data pi /3.141592653589793d0/,pi2 /1.570796326794897d0/    
      data ini/0/,one,half,threequa/1.0d0,.50d0,.75d0/
      if (ini.eq.0) goto 2
 1    z=phi
      z2=z*z
      if (phi.le.pi2) then 
      if (phi.eq.0d0) then
      cl3=zeta3
      return
      endif
      cl3=-z2*(z2*(z2*(z2*(z2*(z2*(z2*(z2*(z2*(z2*(z2* b(10)+ b(9))
     &  + b(8))+ b(7))+ b(6))+ b(5))+ b(4))+ b(3))+ b(2))+ b(1))
     &  +threequa-half*dlog(dabs(z)))+zeta3
      else                                       
      cl3=-z2*(z2*(z2*(z2*(z2*(z2*(z2*(z2*(z2*(z2*(z2*(z2*(z2*(z2*
     & (z2*(z2* b(15)+ b(14))+ b(13))+ b(12))+ b(11))+ b(10))+ b(9))
     &  + b(8))+ b(7))+ b(6))+ b(5))+ b(4))+ b(3))+ b(2))+ b(1))
     &  +threequa-half*dlog(dabs(z)))+zeta3
      endif
      return
 2       b(1) =  3.4722222222222222E-03
         b(2) =  1.1574074074074074E-05
         b(3) =  9.8418997228521037E-08
         b(4) =  1.1482216343327454E-09
         b(5) =  1.5815724990809166E-11
         b(6) =  2.4195009792525152E-13
         b(7) =  3.9828977769894878E-15
         b(8) =  6.9233666183059291E-17
         b(9) =  1.2552722304499773E-18
         b(10)=  2.3537540027684652E-20
         b(11)=  4.5363989034586869E-22
         b(12)=  8.9451696703926431E-24
         b(13)=  1.7982840046954963E-25
         b(14)=  3.6754997647937384E-27
         b(15)=  7.6208079715647952E-29
      ini=1
      goto 1
      end


c;;;;;;;;;;;;;;;;;;;;;;;;;;;;; -*- Mode: Fortran -*- ;;;;;;;;;;;;;;;;;;;;;;;;;
c;; hadr5new.f --- 
c;; Author          : Fred Jegerlehner
c;; Created On      : Sat Jul 12 17:45:19 2003
c;; Last Modified By: Fred Jegerlehner
c;; Last Modified On: Sun Nov  2 00:17:36 2003
c;; RCS: $Id$
c;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
c;; Copyright (c) 2003 Fred Jegerlehner
c;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
c;; 
       subroutine hadr5n(e,st2,der,errder,deg,errdeg)
c single precision HADR5 ; double precision DHADR5
c ******************************************************************
c *                                                                *
c *      subroutine for the evaluation of the light hadron         *
c *           contributions to Delta_r  and  Delta_g               *
c *                    using fits to the                           *
c *          QED vacuum polarization from e^+ e^- data             *
c *                                                                *
c *    F. Jegerlehner, DESY, Platanenalle 6, D-15738 Zeuthen       *
c *                                                                *
c *    E-mail: fred.jegerlehner@desy.de                            *
c *    Phone :   +49-33762-77-259                                  *
c *                                                                *
c *    Reference: F. Jegerlehner, Z. Phys. C32 (1986) 195          *
c *               H. Burkhardt et al., Z. Phys. C42 (1989) 497     *
c *               S. Eidelman, F. Jegerlehner, Z. Phys. C (1995)   *
c *                                                                *
c ******************************************************************
c       VERSION: 24/02/1995
c
C  Notation: E energy ( momentum transfer ): E>0 timelike , E<0 spacelike
C            st2 is sin^2(Theta); st2=0.2322 is the reference value
C  the routine returns the hadronic contribution of 5 flavors (u,d,s,c,b)
C                 to   DER=Delta_r with hadronic error ERRDER
C                and   DEG=Delta_g with hadronic error ERRDEG
C  The effective value of the fine structure constant alphaQED at energy
C  E is alphaQED(E)=alphaQED(0)/(1-Delta_r) ,similarly for the SU(2)
C  coupling alphaSU2(E)=alphaSU2(0)/(1-Delta_g), where Delta_r(g) is the
C  sum of leptonic, hadronic contributions (top to be added).
C
C
      IMPLICIT NONE
      INTEGER NA,NB,NC,IJ,I,ini
      PARAMETER(NA=882,NB=530,NC=250)
      real e,st2,der,errder,deg,errdeg
      real dal(2),dg2(2)
      REAL ETA(NA),DAT(NA,2),ESA(NB),DAS(NB,2),EMA(NC),DAM(NC,2)
      COMMON /DATT/ETA,DAT
      COMMON /DATS/ESA,DAS
      COMMON /DATM/EMA,DAM
      data ini/0/
C initialize data

      call dalhad_spacelike

      call dalhad_timelike

      call dalhad_timelike1

      if (e.eq.0.0) then
        der=0.0
        errder=0.0
        deg=0.0
        errdeg=0.0
        return
      else if ((e.lt.ETA(NA)).and.(e.ge.ETA(1))) then
        ij=NA
        do while (ETA(ij).ge.e)
          ij=ij-1
        enddo
        DO I=1,2
        dal(i)=DAT(IJ,I)
     &    +(DAT(IJ+1,I)-DAT(IJ,I))/(ETA(IJ+1)-ETA(IJ))*(E-ETA(IJ))
        ENDDO
        der=dal(1)
        errder=dal(2)
        deg=0.0
        errdeg=0.0
        return
      else if ((e.le.EMA(NC)).and.(e.gt.EMA(1))) then
        if ((e.gt.3.0).and.(e.lt.12.0).and.(ini.eq.0)) then
           write (*,*) ' ******************************************'
           write (*,*) ' *  Warning: results may not be reliable  *'
           write (*,*) ' * close to J/Psi and Upsilon resonances! *'
           write (*,*) ' ***************FJ@DESY********************'
           ini=1
        endif
        ij=NC
        do while (EMA(ij).ge.e)
          ij=ij-1
        enddo
        DO I=1,2
        dal(i)=DAM(IJ,I)
     &    +(DAM(IJ+1,I)-DAM(IJ,I))/(EMA(IJ+1)-EMA(IJ))*(E-EMA(IJ))
        ENDDO
        der=dal(1)
        errder=dal(2)
        deg=0.0
        errdeg=0.0
        return
      else if ((e.le.ESA(NB)).and.(e.gt.0.0)) then
        ij=NB
        do while (ESA(ij).ge.e)
          ij=ij-1
        enddo
        DO I=1,2
        dal(i)=DAS(IJ,I)
     &    +(DAS(IJ+1,I)-DAS(IJ,I))/(ESA(IJ+1)-ESA(IJ))*(E-ESA(IJ))
        ENDDO
        der=dal(1)
        errder=dal(2)
        deg=0.0
        errdeg=0.0
        return
      else
         write(*,*) ' out of range! '
      endif
      return
      end

       subroutine dhadr5n(de,dst2,dder,derrder,ddeg,derrdeg)
c ******************************************************************
c *    F. Jegerlehner, DESY Zeuthen, D-15738 Zeuthen           *
c ******************************************************************
c Converts hadr5 to doubleprecision variables dhadr5
c
       implicit none
       real *8 de,dst2,dder,ddeg,derrder,derrdeg
       real    se,sst2,sder,sdeg,serrder,serrdeg
       se  =sngl(de)
       sst2=sngl(dst2)
       call hadr5n(se,sst2,sder,serrder,sdeg,serrdeg)
       dder   =dble(sder)
       ddeg   =dble(sdeg)
       derrder=dble(serrder)
       derrdeg=dble(serrdeg)
       return
       end

      subroutine dalhad_spacelike
* hadronic contribution of the 5 light quark flavors evaluated using 
* e^+e^-data. CMD-2 2001 data have a normalization problem. I added a energy 
* independent +1.5 % correction and incresed the syst error by 0.5%. 
* Results thus are preliminary! Arrays:
* ETA(I)  : - energy E in GeV; the minus sign indicates the spacelike region
* DAT(I,1): Delta alpha^5(-E**2) 
* DAT(I,2): error (systematics dominated, should be treated as a systematic error)
C
      IMPLICIT NONE
      INTEGER NA,I,J
      PARAMETER(NA=882)
      REAL ETA(NA),DAT(NA,2)
      COMMON /DATT/ETA,DAT
C 882
      DATA  (ETA(I), I=1,100) /
     &  -1.000E+03,-9.900E+02,-9.800E+02,-9.700E+02,-9.600E+02,
     &  -9.500E+02,-9.400E+02,-9.300E+02,-9.200E+02,-9.100E+02,
     &  -9.000E+02,-8.900E+02,-8.800E+02,-8.700E+02,-8.600E+02,
     &  -8.500E+02,-8.400E+02,-8.300E+02,-8.200E+02,-8.100E+02,
     &  -8.000E+02,-7.900E+02,-7.800E+02,-7.700E+02,-7.600E+02,
     &  -7.500E+02,-7.400E+02,-7.300E+02,-7.200E+02,-7.100E+02,
     &  -7.000E+02,-6.900E+02,-6.800E+02,-6.700E+02,-6.600E+02,
     &  -6.500E+02,-6.400E+02,-6.300E+02,-6.200E+02,-6.100E+02,
     &  -6.000E+02,-5.900E+02,-5.800E+02,-5.700E+02,-5.600E+02,
     &  -5.500E+02,-5.400E+02,-5.300E+02,-5.200E+02,-5.100E+02,
     &  -5.000E+02,-4.900E+02,-4.800E+02,-4.700E+02,-4.600E+02,
     &  -4.500E+02,-4.400E+02,-4.300E+02,-4.200E+02,-4.100E+02,
     &  -4.000E+02,-3.900E+02,-3.800E+02,-3.700E+02,-3.600E+02,
     &  -3.500E+02,-3.400E+02,-3.300E+02,-3.200E+02,-3.100E+02,
     &  -3.000E+02,-2.900E+02,-2.800E+02,-2.700E+02,-2.600E+02,
     &  -2.500E+02,-2.400E+02,-2.300E+02,-2.200E+02,-2.100E+02,
     &  -2.000E+02,-1.900E+02,-1.800E+02,-1.700E+02,-1.600E+02,
     &  -1.500E+02,-1.400E+02,-1.300E+02,-1.200E+02,-1.100E+02,
     &  -1.000E+02,-9.900E+01,-9.800E+01,-9.700E+01,-9.600E+01,
     &  -9.500E+01,-9.400E+01,-9.300E+01,-9.200E+01,-9.100E+01/

      DATA  (ETA(I), I=101,200) /
     &  -9.000E+01,-8.900E+01,-8.800E+01,-8.700E+01,-8.600E+01,
     &  -8.500E+01,-8.400E+01,-8.300E+01,-8.200E+01,-8.100E+01,
     &  -8.000E+01,-7.900E+01,-7.800E+01,-7.700E+01,-7.600E+01,
     &  -7.500E+01,-7.400E+01,-7.300E+01,-7.200E+01,-7.100E+01,
     &  -7.000E+01,-6.900E+01,-6.800E+01,-6.700E+01,-6.600E+01,
     &  -6.500E+01,-6.400E+01,-6.300E+01,-6.200E+01,-6.100E+01,
     &  -6.000E+01,-5.900E+01,-5.800E+01,-5.700E+01,-5.600E+01,
     &  -5.500E+01,-5.400E+01,-5.300E+01,-5.200E+01,-5.100E+01,
     &  -5.000E+01,-4.900E+01,-4.800E+01,-4.700E+01,-4.600E+01,
     &  -4.500E+01,-4.400E+01,-4.300E+01,-4.200E+01,-4.100E+01,
     &  -4.000E+01,-3.900E+01,-3.800E+01,-3.700E+01,-3.600E+01,
     &  -3.500E+01,-3.400E+01,-3.300E+01,-3.200E+01,-3.100E+01,
     &  -3.000E+01,-2.900E+01,-2.800E+01,-2.700E+01,-2.600E+01,
     &  -2.500E+01,-2.400E+01,-2.300E+01,-2.200E+01,-2.100E+01,
     &  -2.000E+01,-1.900E+01,-1.800E+01,-1.700E+01,-1.600E+01,
     &  -1.500E+01,-1.400E+01,-1.300E+01,-1.200E+01,-1.100E+01,
     &  -1.000E+01,-9.910E+00,-9.820E+00,-9.730E+00,-9.640E+00,
     &  -9.550E+00,-9.460E+00,-9.370E+00,-9.280E+00,-9.190E+00,
     &  -9.100E+00,-9.010E+00,-8.920E+00,-8.830E+00,-8.740E+00,
     &  -8.650E+00,-8.560E+00,-8.470E+00,-8.380E+00,-8.290E+00/

      DATA  (ETA(I), I=201,300) /
     &  -8.200E+00,-8.110E+00,-8.020E+00,-7.930E+00,-7.840E+00,
     &  -7.750E+00,-7.660E+00,-7.570E+00,-7.480E+00,-7.390E+00,
     &  -7.300E+00,-7.210E+00,-7.120E+00,-7.030E+00,-6.940E+00,
     &  -6.850E+00,-6.760E+00,-6.670E+00,-6.580E+00,-6.490E+00,
     &  -6.400E+00,-6.310E+00,-6.220E+00,-6.130E+00,-6.040E+00,
     &  -5.950E+00,-5.860E+00,-5.770E+00,-5.680E+00,-5.590E+00,
     &  -5.500E+00,-5.410E+00,-5.320E+00,-5.230E+00,-5.140E+00,
     &  -5.050E+00,-4.960E+00,-4.870E+00,-4.780E+00,-4.690E+00,
     &  -4.600E+00,-4.510E+00,-4.420E+00,-4.330E+00,-4.240E+00,
     &  -4.150E+00,-4.060E+00,-3.970E+00,-3.880E+00,-3.790E+00,
     &  -3.700E+00,-3.610E+00,-3.520E+00,-3.430E+00,-3.340E+00,
     &  -3.250E+00,-3.160E+00,-3.070E+00,-2.980E+00,-2.890E+00,
     &  -2.800E+00,-2.710E+00,-2.620E+00,-2.530E+00,-2.440E+00,
     &  -2.350E+00,-2.260E+00,-2.170E+00,-2.080E+00,-1.990E+00,
     &  -1.900E+00,-1.810E+00,-1.720E+00,-1.630E+00,-1.540E+00,
     &  -1.450E+00,-1.360E+00,-1.270E+00,-1.180E+00,-1.090E+00,
     &  -1.000E+00,-9.910E-01,-9.820E-01,-9.730E-01,-9.640E-01,
     &  -9.550E-01,-9.460E-01,-9.370E-01,-9.280E-01,-9.190E-01,
     &  -9.100E-01,-9.010E-01,-8.920E-01,-8.830E-01,-8.740E-01,
     &  -8.650E-01,-8.560E-01,-8.470E-01,-8.380E-01,-8.290E-01/

      DATA  (ETA(I), I=301,400) /
     &  -8.200E-01,-8.110E-01,-8.020E-01,-7.930E-01,-7.840E-01,
     &  -7.750E-01,-7.660E-01,-7.570E-01,-7.480E-01,-7.390E-01,
     &  -7.300E-01,-7.210E-01,-7.120E-01,-7.030E-01,-6.940E-01,
     &  -6.850E-01,-6.760E-01,-6.670E-01,-6.580E-01,-6.490E-01,
     &  -6.400E-01,-6.310E-01,-6.220E-01,-6.130E-01,-6.040E-01,
     &  -5.950E-01,-5.860E-01,-5.770E-01,-5.680E-01,-5.590E-01,
     &  -5.500E-01,-5.410E-01,-5.320E-01,-5.230E-01,-5.140E-01,
     &  -5.050E-01,-4.960E-01,-4.870E-01,-4.780E-01,-4.690E-01,
     &  -4.600E-01,-4.510E-01,-4.420E-01,-4.330E-01,-4.240E-01,
     &  -4.150E-01,-4.060E-01,-3.970E-01,-3.880E-01,-3.790E-01,
     &  -3.700E-01,-3.610E-01,-3.520E-01,-3.430E-01,-3.340E-01,
     &  -3.250E-01,-3.160E-01,-3.070E-01,-2.980E-01,-2.890E-01,
     &  -2.800E-01,-2.710E-01,-2.620E-01,-2.530E-01,-2.440E-01,
     &  -2.350E-01,-2.260E-01,-2.170E-01,-2.080E-01,-1.990E-01,
     &  -1.900E-01,-1.810E-01,-1.720E-01,-1.630E-01,-1.540E-01,
     &  -1.450E-01,-1.360E-01,-1.270E-01,-1.180E-01,-1.090E-01,
     &  -1.000E-01,-9.910E-02,-9.820E-02,-9.730E-02,-9.640E-02,
     &  -9.550E-02,-9.460E-02,-9.370E-02,-9.280E-02,-9.190E-02,
     &  -9.100E-02,-9.010E-02,-8.920E-02,-8.830E-02,-8.740E-02,
     &  -8.650E-02,-8.560E-02,-8.470E-02,-8.380E-02,-8.290E-02/

      DATA  (ETA(I), I=401,500) /
     &  -8.200E-02,-8.110E-02,-8.020E-02,-7.930E-02,-7.840E-02,
     &  -7.750E-02,-7.660E-02,-7.570E-02,-7.480E-02,-7.390E-02,
     &  -7.300E-02,-7.210E-02,-7.120E-02,-7.030E-02,-6.940E-02,
     &  -6.850E-02,-6.760E-02,-6.670E-02,-6.580E-02,-6.490E-02,
     &  -6.400E-02,-6.310E-02,-6.220E-02,-6.130E-02,-6.040E-02,
     &  -5.950E-02,-5.860E-02,-5.770E-02,-5.680E-02,-5.590E-02,
     &  -5.500E-02,-5.410E-02,-5.320E-02,-5.230E-02,-5.140E-02,
     &  -5.050E-02,-4.960E-02,-4.870E-02,-4.780E-02,-4.690E-02,
     &  -4.600E-02,-4.510E-02,-4.420E-02,-4.330E-02,-4.240E-02,
     &  -4.150E-02,-4.060E-02,-3.970E-02,-3.880E-02,-3.790E-02,
     &  -3.700E-02,-3.610E-02,-3.520E-02,-3.430E-02,-3.340E-02,
     &  -3.250E-02,-3.160E-02,-3.070E-02,-2.980E-02,-2.890E-02,
     &  -2.800E-02,-2.710E-02,-2.620E-02,-2.530E-02,-2.440E-02,
     &  -2.350E-02,-2.260E-02,-2.170E-02,-2.080E-02,-1.990E-02,
     &  -1.900E-02,-1.810E-02,-1.720E-02,-1.630E-02,-1.540E-02,
     &  -1.450E-02,-1.360E-02,-1.270E-02,-1.180E-02,-1.090E-02,
     &  -1.000E-02,-9.910E-03,-9.820E-03,-9.730E-03,-9.640E-03,
     &  -9.550E-03,-9.460E-03,-9.370E-03,-9.280E-03,-9.190E-03,
     &  -9.100E-03,-9.010E-03,-8.920E-03,-8.830E-03,-8.740E-03,
     &  -8.650E-03,-8.560E-03,-8.470E-03,-8.380E-03,-8.290E-03/

      DATA  (ETA(I), I=501,600) /
     &  -8.200E-03,-8.110E-03,-8.020E-03,-7.930E-03,-7.840E-03,
     &  -7.750E-03,-7.660E-03,-7.570E-03,-7.480E-03,-7.390E-03,
     &  -7.300E-03,-7.210E-03,-7.120E-03,-7.030E-03,-6.940E-03,
     &  -6.850E-03,-6.760E-03,-6.670E-03,-6.580E-03,-6.490E-03,
     &  -6.400E-03,-6.310E-03,-6.220E-03,-6.130E-03,-6.040E-03,
     &  -5.950E-03,-5.860E-03,-5.770E-03,-5.680E-03,-5.590E-03,
     &  -5.500E-03,-5.410E-03,-5.320E-03,-5.230E-03,-5.140E-03,
     &  -5.050E-03,-4.960E-03,-4.870E-03,-4.780E-03,-4.690E-03,
     &  -4.600E-03,-4.510E-03,-4.420E-03,-4.330E-03,-4.240E-03,
     &  -4.150E-03,-4.060E-03,-3.970E-03,-3.880E-03,-3.790E-03,
     &  -3.700E-03,-3.610E-03,-3.520E-03,-3.430E-03,-3.340E-03,
     &  -3.250E-03,-3.160E-03,-3.070E-03,-2.980E-03,-2.890E-03,
     &  -2.800E-03,-2.710E-03,-2.620E-03,-2.530E-03,-2.440E-03,
     &  -2.350E-03,-2.260E-03,-2.170E-03,-2.080E-03,-1.990E-03,
     &  -1.900E-03,-1.810E-03,-1.720E-03,-1.630E-03,-1.540E-03,
     &  -1.450E-03,-1.360E-03,-1.270E-03,-1.180E-03,-1.090E-03,
     &  -1.000E-03,-9.910E-04,-9.820E-04,-9.730E-04,-9.640E-04,
     &  -9.550E-04,-9.460E-04,-9.370E-04,-9.280E-04,-9.190E-04,
     &  -9.100E-04,-9.010E-04,-8.920E-04,-8.830E-04,-8.740E-04,
     &  -8.650E-04,-8.560E-04,-8.470E-04,-8.380E-04,-8.290E-04/

      DATA  (ETA(I), I=601,700) /
     &  -8.200E-04,-8.110E-04,-8.020E-04,-7.930E-04,-7.840E-04,
     &  -7.750E-04,-7.660E-04,-7.570E-04,-7.480E-04,-7.390E-04,
     &  -7.300E-04,-7.210E-04,-7.120E-04,-7.030E-04,-6.940E-04,
     &  -6.850E-04,-6.760E-04,-6.670E-04,-6.580E-04,-6.490E-04,
     &  -6.400E-04,-6.310E-04,-6.220E-04,-6.130E-04,-6.040E-04,
     &  -5.950E-04,-5.860E-04,-5.770E-04,-5.680E-04,-5.590E-04,
     &  -5.500E-04,-5.410E-04,-5.320E-04,-5.230E-04,-5.140E-04,
     &  -5.050E-04,-4.960E-04,-4.870E-04,-4.780E-04,-4.690E-04,
     &  -4.600E-04,-4.510E-04,-4.420E-04,-4.330E-04,-4.240E-04,
     &  -4.150E-04,-4.060E-04,-3.970E-04,-3.880E-04,-3.790E-04,
     &  -3.700E-04,-3.610E-04,-3.520E-04,-3.430E-04,-3.340E-04,
     &  -3.250E-04,-3.160E-04,-3.070E-04,-2.980E-04,-2.890E-04,
     &  -2.800E-04,-2.710E-04,-2.620E-04,-2.530E-04,-2.440E-04,
     &  -2.350E-04,-2.260E-04,-2.170E-04,-2.080E-04,-1.990E-04,
     &  -1.900E-04,-1.810E-04,-1.720E-04,-1.630E-04,-1.540E-04,
     &  -1.450E-04,-1.360E-04,-1.270E-04,-1.180E-04,-1.090E-04,
     &  -1.000E-04,-9.910E-05,-9.820E-05,-9.730E-05,-9.640E-05,
     &  -9.550E-05,-9.460E-05,-9.370E-05,-9.280E-05,-9.190E-05,
     &  -9.100E-05,-9.010E-05,-8.920E-05,-8.830E-05,-8.740E-05,
     &  -8.650E-05,-8.560E-05,-8.470E-05,-8.380E-05,-8.290E-05/

      DATA  (ETA(I), I=701,800) /
     &  -8.200E-05,-8.110E-05,-8.020E-05,-7.930E-05,-7.840E-05,
     &  -7.750E-05,-7.660E-05,-7.570E-05,-7.480E-05,-7.390E-05,
     &  -7.300E-05,-7.210E-05,-7.120E-05,-7.030E-05,-6.940E-05,
     &  -6.850E-05,-6.760E-05,-6.670E-05,-6.580E-05,-6.490E-05,
     &  -6.400E-05,-6.310E-05,-6.220E-05,-6.130E-05,-6.040E-05,
     &  -5.950E-05,-5.860E-05,-5.770E-05,-5.680E-05,-5.590E-05,
     &  -5.500E-05,-5.410E-05,-5.320E-05,-5.230E-05,-5.140E-05,
     &  -5.050E-05,-4.960E-05,-4.870E-05,-4.780E-05,-4.690E-05,
     &  -4.600E-05,-4.510E-05,-4.420E-05,-4.330E-05,-4.240E-05,
     &  -4.150E-05,-4.060E-05,-3.970E-05,-3.880E-05,-3.790E-05,
     &  -3.700E-05,-3.610E-05,-3.520E-05,-3.430E-05,-3.340E-05,
     &  -3.250E-05,-3.160E-05,-3.070E-05,-2.980E-05,-2.890E-05,
     &  -2.800E-05,-2.710E-05,-2.620E-05,-2.530E-05,-2.440E-05,
     &  -2.350E-05,-2.260E-05,-2.170E-05,-2.080E-05,-1.990E-05,
     &  -1.900E-05,-1.810E-05,-1.720E-05,-1.630E-05,-1.540E-05,
     &  -1.450E-05,-1.360E-05,-1.270E-05,-1.180E-05,-1.090E-05,
     &  -1.000E-05,-9.910E-06,-9.820E-06,-9.730E-06,-9.640E-06,
     &  -9.550E-06,-9.460E-06,-9.370E-06,-9.280E-06,-9.190E-06,
     &  -9.100E-06,-9.010E-06,-8.920E-06,-8.830E-06,-8.740E-06,
     &  -8.650E-06,-8.560E-06,-8.470E-06,-8.380E-06,-8.290E-06/

      DATA  (ETA(I), I=801,882) /
     &  -8.200E-06,-8.110E-06,-8.020E-06,-7.930E-06,-7.840E-06,
     &  -7.750E-06,-7.660E-06,-7.570E-06,-7.480E-06,-7.390E-06,
     &  -7.300E-06,-7.210E-06,-7.120E-06,-7.030E-06,-6.940E-06,
     &  -6.850E-06,-6.760E-06,-6.670E-06,-6.580E-06,-6.490E-06,
     &  -6.400E-06,-6.310E-06,-6.220E-06,-6.130E-06,-6.040E-06,
     &  -5.950E-06,-5.860E-06,-5.770E-06,-5.680E-06,-5.590E-06,
     &  -5.500E-06,-5.410E-06,-5.320E-06,-5.230E-06,-5.140E-06,
     &  -5.050E-06,-4.960E-06,-4.870E-06,-4.780E-06,-4.690E-06,
     &  -4.600E-06,-4.510E-06,-4.420E-06,-4.330E-06,-4.240E-06,
     &  -4.150E-06,-4.060E-06,-3.970E-06,-3.880E-06,-3.790E-06,
     &  -3.700E-06,-3.610E-06,-3.520E-06,-3.430E-06,-3.340E-06,
     &  -3.250E-06,-3.160E-06,-3.070E-06,-2.980E-06,-2.890E-06,
     &  -2.800E-06,-2.710E-06,-2.620E-06,-2.530E-06,-2.440E-06,
     &  -2.350E-06,-2.260E-06,-2.170E-06,-2.080E-06,-1.990E-06,
     &  -1.900E-06,-1.810E-06,-1.720E-06,-1.630E-06,-1.540E-06,
     &  -1.450E-06,-1.360E-06,-1.270E-06,-1.180E-06,-1.090E-06,
     &  -1.000E-06, 0.000E+00/

        DATA ((DAT(I,J), I=1,100), J=1,1) /
     &   4.380E-02, 4.372E-02, 4.365E-02, 4.357E-02, 4.349E-02,
     &   4.341E-02, 4.333E-02, 4.325E-02, 4.317E-02, 4.308E-02,
     &   4.300E-02, 4.291E-02, 4.283E-02, 4.274E-02, 4.265E-02,
     &   4.256E-02, 4.248E-02, 4.239E-02, 4.229E-02, 4.220E-02,
     &   4.211E-02, 4.201E-02, 4.192E-02, 4.182E-02, 4.172E-02,
     &   4.162E-02, 4.152E-02, 4.142E-02, 4.132E-02, 4.121E-02,
     &   4.111E-02, 4.100E-02, 4.089E-02, 4.078E-02, 4.067E-02,
     &   4.056E-02, 4.045E-02, 4.033E-02, 4.021E-02, 4.009E-02,
     &   3.997E-02, 3.985E-02, 3.972E-02, 3.960E-02, 3.947E-02,
     &   3.934E-02, 3.921E-02, 3.907E-02, 3.893E-02, 3.880E-02,
     &   3.865E-02, 3.851E-02, 3.836E-02, 3.821E-02, 3.806E-02,
     &   3.791E-02, 3.775E-02, 3.759E-02, 3.742E-02, 3.726E-02,
     &   3.709E-02, 3.691E-02, 3.673E-02, 3.655E-02, 3.636E-02,
     &   3.617E-02, 3.597E-02, 3.577E-02, 3.557E-02, 3.535E-02,
     &   3.514E-02, 3.491E-02, 3.468E-02, 3.444E-02, 3.419E-02,
     &   3.394E-02, 3.368E-02, 3.340E-02, 3.312E-02, 3.282E-02,
     &   3.251E-02, 3.219E-02, 3.185E-02, 3.150E-02, 3.112E-02,
     &   3.072E-02, 3.030E-02, 2.985E-02, 2.936E-02, 2.883E-02,
     &   2.826E-02, 2.820E-02, 2.813E-02, 2.807E-02, 2.801E-02,
     &   2.795E-02, 2.788E-02, 2.782E-02, 2.775E-02, 2.769E-02/

        DATA ((DAT(I,J), I=101,200), J=1,1) /
     &   2.762E-02, 2.756E-02, 2.749E-02, 2.742E-02, 2.735E-02,
     &   2.728E-02, 2.721E-02, 2.714E-02, 2.706E-02, 2.699E-02,
     &   2.692E-02, 2.684E-02, 2.677E-02, 2.669E-02, 2.661E-02,
     &   2.653E-02, 2.645E-02, 2.637E-02, 2.629E-02, 2.620E-02,
     &   2.612E-02, 2.603E-02, 2.595E-02, 2.586E-02, 2.577E-02,
     &   2.568E-02, 2.558E-02, 2.549E-02, 2.540E-02, 2.530E-02,
     &   2.520E-02, 2.510E-02, 2.500E-02, 2.489E-02, 2.479E-02,
     &   2.468E-02, 2.457E-02, 2.446E-02, 2.435E-02, 2.423E-02,
     &   2.412E-02, 2.400E-02, 2.387E-02, 2.375E-02, 2.362E-02,
     &   2.349E-02, 2.336E-02, 2.322E-02, 2.308E-02, 2.294E-02,
     &   2.279E-02, 2.264E-02, 2.249E-02, 2.233E-02, 2.217E-02,
     &   2.200E-02, 2.183E-02, 2.165E-02, 2.147E-02, 2.128E-02,
     &   2.109E-02, 2.089E-02, 2.068E-02, 2.047E-02, 2.024E-02,
     &   2.001E-02, 1.977E-02, 1.952E-02, 1.926E-02, 1.899E-02,
     &   1.870E-02, 1.840E-02, 1.808E-02, 1.775E-02, 1.739E-02,
     &   1.702E-02, 1.662E-02, 1.619E-02, 1.572E-02, 1.522E-02,
     &   1.468E-02, 1.462E-02, 1.457E-02, 1.452E-02, 1.446E-02,
     &   1.441E-02, 1.436E-02, 1.430E-02, 1.425E-02, 1.419E-02,
     &   1.414E-02, 1.408E-02, 1.402E-02, 1.397E-02, 1.391E-02,
     &   1.385E-02, 1.379E-02, 1.373E-02, 1.367E-02, 1.361E-02/

        DATA ((DAT(I,J), I=201,300), J=1,1) /
     &   1.355E-02, 1.349E-02, 1.342E-02, 1.336E-02, 1.330E-02,
     &   1.323E-02, 1.317E-02, 1.310E-02, 1.303E-02, 1.297E-02,
     &   1.290E-02, 1.283E-02, 1.276E-02, 1.269E-02, 1.262E-02,
     &   1.254E-02, 1.247E-02, 1.240E-02, 1.232E-02, 1.225E-02,
     &   1.217E-02, 1.209E-02, 1.201E-02, 1.193E-02, 1.185E-02,
     &   1.177E-02, 1.169E-02, 1.160E-02, 1.152E-02, 1.143E-02,
     &   1.134E-02, 1.125E-02, 1.116E-02, 1.107E-02, 1.098E-02,
     &   1.089E-02, 1.079E-02, 1.069E-02, 1.059E-02, 1.049E-02,
     &   1.039E-02, 1.029E-02, 1.018E-02, 1.007E-02, 9.964E-03,
     &   9.853E-03, 9.740E-03, 9.625E-03, 9.508E-03, 9.389E-03,
     &   9.267E-03, 9.143E-03, 9.017E-03, 8.888E-03, 8.756E-03,
     &   8.621E-03, 8.484E-03, 8.344E-03, 8.201E-03, 8.054E-03,
     &   7.904E-03, 7.750E-03, 7.592E-03, 7.430E-03, 7.265E-03,
     &   7.094E-03, 6.919E-03, 6.738E-03, 6.552E-03, 6.361E-03,
     &   6.162E-03, 5.957E-03, 5.744E-03, 5.523E-03, 5.292E-03,
     &   5.052E-03, 4.800E-03, 4.535E-03, 4.257E-03, 3.963E-03,
     &   3.653E-03, 3.620E-03, 3.588E-03, 3.556E-03, 3.523E-03,
     &   3.490E-03, 3.457E-03, 3.424E-03, 3.391E-03, 3.357E-03,
     &   3.323E-03, 3.289E-03, 3.255E-03, 3.221E-03, 3.186E-03,
     &   3.151E-03, 3.116E-03, 3.081E-03, 3.046E-03, 3.010E-03/

        DATA ((DAT(I,J), I=301,400), J=1,1) /
     &   2.974E-03, 2.938E-03, 2.902E-03, 2.866E-03, 2.829E-03,
     &   2.792E-03, 2.755E-03, 2.718E-03, 2.681E-03, 2.643E-03,
     &   2.605E-03, 2.567E-03, 2.529E-03, 2.491E-03, 2.452E-03,
     &   2.413E-03, 2.375E-03, 2.335E-03, 2.296E-03, 2.257E-03,
     &   2.217E-03, 2.177E-03, 2.137E-03, 2.097E-03, 2.057E-03,
     &   2.017E-03, 1.977E-03, 1.936E-03, 1.895E-03, 1.855E-03,
     &   1.814E-03, 1.773E-03, 1.732E-03, 1.691E-03, 1.650E-03,
     &   1.609E-03, 1.567E-03, 1.526E-03, 1.485E-03, 1.444E-03,
     &   1.403E-03, 1.362E-03, 1.321E-03, 1.280E-03, 1.239E-03,
     &   1.199E-03, 1.158E-03, 1.118E-03, 1.078E-03, 1.038E-03,
     &   9.988E-04, 9.596E-04, 9.207E-04, 8.821E-04, 8.438E-04,
     &   8.060E-04, 7.686E-04, 7.316E-04, 6.952E-04, 6.592E-04,
     &   6.238E-04, 5.891E-04, 5.549E-04, 5.214E-04, 4.886E-04,
     &   4.565E-04, 4.253E-04, 3.948E-04, 3.652E-04, 3.364E-04,
     &   3.086E-04, 2.818E-04, 2.560E-04, 2.312E-04, 2.075E-04,
     &   1.849E-04, 1.634E-04, 1.432E-04, 1.241E-04, 1.064E-04,
     &   8.985E-05, 8.827E-05, 8.670E-05, 8.515E-05, 8.361E-05,
     &   8.209E-05, 8.057E-05, 7.907E-05, 7.759E-05, 7.612E-05,
     &   7.466E-05, 7.321E-05, 7.178E-05, 7.036E-05, 6.896E-05,
     &   6.757E-05, 6.619E-05, 6.482E-05, 6.347E-05, 6.213E-05/

        DATA ((DAT(I,J), I=401,500), J=1,1) /
     &   6.081E-05, 5.950E-05, 5.821E-05, 5.692E-05, 5.566E-05,
     &   5.440E-05, 5.316E-05, 5.193E-05, 5.072E-05, 4.952E-05,
     &   4.833E-05, 4.716E-05, 4.600E-05, 4.486E-05, 4.373E-05,
     &   4.261E-05, 4.151E-05, 4.042E-05, 3.935E-05, 3.829E-05,
     &   3.724E-05, 3.621E-05, 3.519E-05, 3.419E-05, 3.320E-05,
     &   3.223E-05, 3.127E-05, 3.032E-05, 2.939E-05, 2.847E-05,
     &   2.757E-05, 2.668E-05, 2.580E-05, 2.494E-05, 2.409E-05,
     &   2.326E-05, 2.244E-05, 2.164E-05, 2.085E-05, 2.008E-05,
     &   1.932E-05, 1.857E-05, 1.784E-05, 1.713E-05, 1.642E-05,
     &   1.574E-05, 1.506E-05, 1.441E-05, 1.376E-05, 1.313E-05,
     &   1.252E-05, 1.192E-05, 1.133E-05, 1.076E-05, 1.021E-05,
     &   9.664E-06, 9.138E-06, 8.626E-06, 8.128E-06, 7.645E-06,
     &   7.177E-06, 6.724E-06, 6.286E-06, 5.862E-06, 5.453E-06,
     &   5.058E-06, 4.679E-06, 4.314E-06, 3.964E-06, 3.628E-06,
     &   3.308E-06, 3.002E-06, 2.711E-06, 2.435E-06, 2.174E-06,
     &   1.927E-06, 1.695E-06, 1.479E-06, 1.277E-06, 1.089E-06,
     &   9.168E-07, 9.004E-07, 8.841E-07, 8.680E-07, 8.520E-07,
     &   8.362E-07, 8.205E-07, 8.049E-07, 7.896E-07, 7.743E-07,
     &   7.592E-07, 7.443E-07, 7.295E-07, 7.149E-07, 7.004E-07,
     &   6.860E-07, 6.718E-07, 6.578E-07, 6.439E-07, 6.301E-07/

        DATA ((DAT(I,J), I=501,600), J=1,1) /
     &   6.165E-07, 6.031E-07, 5.897E-07, 5.766E-07, 5.636E-07,
     &   5.507E-07, 5.380E-07, 5.254E-07, 5.130E-07, 5.007E-07,
     &   4.886E-07, 4.766E-07, 4.648E-07, 4.531E-07, 4.416E-07,
     &   4.302E-07, 4.190E-07, 4.079E-07, 3.970E-07, 3.862E-07,
     &   3.756E-07, 3.651E-07, 3.547E-07, 3.445E-07, 3.345E-07,
     &   3.246E-07, 3.149E-07, 3.053E-07, 2.958E-07, 2.865E-07,
     &   2.774E-07, 2.684E-07, 2.595E-07, 2.508E-07, 2.422E-07,
     &   2.339E-07, 2.256E-07, 2.175E-07, 2.095E-07, 2.017E-07,
     &   1.940E-07, 1.865E-07, 1.791E-07, 1.719E-07, 1.648E-07,
     &   1.579E-07, 1.512E-07, 1.445E-07, 1.380E-07, 1.317E-07,
     &   1.255E-07, 1.195E-07, 1.136E-07, 1.079E-07, 1.023E-07,
     &   9.686E-08, 9.157E-08, 8.642E-08, 8.143E-08, 7.659E-08,
     &   7.189E-08, 6.734E-08, 6.295E-08, 5.870E-08, 5.459E-08,
     &   5.064E-08, 4.684E-08, 4.318E-08, 3.967E-08, 3.631E-08,
     &   3.310E-08, 3.004E-08, 2.713E-08, 2.436E-08, 2.175E-08,
     &   1.928E-08, 1.696E-08, 1.479E-08, 1.277E-08, 1.089E-08,
     &   9.170E-09, 9.006E-09, 8.843E-09, 8.681E-09, 8.522E-09,
     &   8.363E-09, 8.206E-09, 8.051E-09, 7.897E-09, 7.745E-09,
     &   7.594E-09, 7.444E-09, 7.296E-09, 7.150E-09, 7.005E-09,
     &   6.861E-09, 6.719E-09, 6.579E-09, 6.440E-09, 6.302E-09/

        DATA ((DAT(I,J), I=601,700), J=1,1) /
     &   6.166E-09, 6.031E-09, 5.898E-09, 5.767E-09, 5.636E-09,
     &   5.508E-09, 5.381E-09, 5.255E-09, 5.131E-09, 5.008E-09,
     &   4.887E-09, 4.767E-09, 4.649E-09, 4.532E-09, 4.417E-09,
     &   4.303E-09, 4.190E-09, 4.080E-09, 3.970E-09, 3.862E-09,
     &   3.756E-09, 3.651E-09, 3.548E-09, 3.446E-09, 3.345E-09,
     &   3.246E-09, 3.149E-09, 3.053E-09, 2.958E-09, 2.865E-09,
     &   2.774E-09, 2.684E-09, 2.595E-09, 2.508E-09, 2.423E-09,
     &   2.339E-09, 2.256E-09, 2.175E-09, 2.095E-09, 2.017E-09,
     &   1.940E-09, 1.865E-09, 1.791E-09, 1.719E-09, 1.649E-09,
     &   1.579E-09, 1.512E-09, 1.445E-09, 1.381E-09, 1.317E-09,
     &   1.255E-09, 1.195E-09, 1.136E-09, 1.079E-09, 1.023E-09,
     &   9.686E-10, 9.157E-10, 8.643E-10, 8.143E-10, 7.659E-10,
     &   7.189E-10, 6.735E-10, 6.295E-10, 5.870E-10, 5.459E-10,
     &   5.064E-10, 4.684E-10, 4.318E-10, 3.967E-10, 3.631E-10,
     &   3.310E-10, 3.004E-10, 2.713E-10, 2.436E-10, 2.175E-10,
     &   1.928E-10, 1.696E-10, 1.479E-10, 1.277E-10, 1.090E-10,
     &   9.170E-11, 9.006E-11, 8.843E-11, 8.681E-11, 8.522E-11,
     &   8.363E-11, 8.206E-11, 8.051E-11, 7.897E-11, 7.745E-11,
     &   7.594E-11, 7.444E-11, 7.296E-11, 7.150E-11, 7.005E-11,
     &   6.861E-11, 6.719E-11, 6.579E-11, 6.440E-11, 6.302E-11/

        DATA ((DAT(I,J), I=701,800), J=1,1) /
     &   6.166E-11, 6.031E-11, 5.898E-11, 5.767E-11, 5.636E-11,
     &   5.508E-11, 5.380E-11, 5.255E-11, 5.131E-11, 5.008E-11,
     &   4.887E-11, 4.767E-11, 4.649E-11, 4.532E-11, 4.417E-11,
     &   4.303E-11, 4.191E-11, 4.080E-11, 3.970E-11, 3.862E-11,
     &   3.756E-11, 3.651E-11, 3.548E-11, 3.446E-11, 3.345E-11,
     &   3.246E-11, 3.149E-11, 3.053E-11, 2.959E-11, 2.865E-11,
     &   2.774E-11, 2.684E-11, 2.595E-11, 2.508E-11, 2.423E-11,
     &   2.339E-11, 2.256E-11, 2.175E-11, 2.095E-11, 2.017E-11,
     &   1.940E-11, 1.865E-11, 1.791E-11, 1.719E-11, 1.649E-11,
     &   1.579E-11, 1.511E-11, 1.445E-11, 1.380E-11, 1.317E-11,
     &   1.255E-11, 1.195E-11, 1.136E-11, 1.079E-11, 1.023E-11,
     &   9.686E-12, 9.157E-12, 8.643E-12, 8.143E-12, 7.659E-12,
     &   7.189E-12, 6.735E-12, 6.295E-12, 5.870E-12, 5.459E-12,
     &   5.064E-12, 4.684E-12, 4.318E-12, 3.967E-12, 3.631E-12,
     &   3.310E-12, 3.004E-12, 2.713E-12, 2.436E-12, 2.175E-12,
     &   1.928E-12, 1.696E-12, 1.479E-12, 1.277E-12, 1.090E-12,
     &   9.170E-13, 9.006E-13, 8.843E-13, 8.682E-13, 8.522E-13,
     &   8.363E-13, 8.206E-13, 8.051E-13, 7.897E-13, 7.745E-13,
     &   7.594E-13, 7.444E-13, 7.296E-13, 7.150E-13, 7.005E-13,
     &   6.861E-13, 6.719E-13, 6.579E-13, 6.440E-13, 6.302E-13/

        DATA ((DAT(I,J), I=801,882), J=1,1) /
     &   6.166E-13, 6.031E-13, 5.898E-13, 5.767E-13, 5.636E-13,
     &   5.508E-13, 5.381E-13, 5.255E-13, 5.131E-13, 5.008E-13,
     &   4.887E-13, 4.767E-13, 4.649E-13, 4.532E-13, 4.417E-13,
     &   4.303E-13, 4.190E-13, 4.080E-13, 3.970E-13, 3.862E-13,
     &   3.756E-13, 3.651E-13, 3.548E-13, 3.446E-13, 3.345E-13,
     &   3.246E-13, 3.149E-13, 3.053E-13, 2.958E-13, 2.865E-13,
     &   2.774E-13, 2.684E-13, 2.595E-13, 2.508E-13, 2.423E-13,
     &   2.339E-13, 2.256E-13, 2.175E-13, 2.095E-13, 2.017E-13,
     &   1.940E-13, 1.865E-13, 1.792E-13, 1.719E-13, 1.648E-13,
     &   1.579E-13, 1.511E-13, 1.445E-13, 1.380E-13, 1.317E-13,
     &   1.255E-13, 1.195E-13, 1.136E-13, 1.079E-13, 1.023E-13,
     &   9.686E-14, 9.157E-14, 8.643E-14, 8.143E-14, 7.659E-14,
     &   7.189E-14, 6.735E-14, 6.295E-14, 5.870E-14, 5.459E-14,
     &   5.064E-14, 4.684E-14, 4.318E-14, 3.967E-14, 3.631E-14,
     &   3.310E-14, 3.004E-14, 2.713E-14, 2.436E-14, 2.175E-14,
     &   1.928E-14, 1.696E-14, 1.479E-14, 1.277E-14, 1.090E-14,
     &   9.170E-15, 0.000E+00/

        DATA ((DAT(I,J), I=1,100), J=2,2) /
     &   3.330E-04, 3.320E-04, 3.330E-04, 3.320E-04, 3.330E-04,
     &   3.330E-04, 3.330E-04, 3.330E-04, 3.330E-04, 3.330E-04,
     &   3.330E-04, 3.330E-04, 3.330E-04, 3.330E-04, 3.330E-04,
     &   3.330E-04, 3.330E-04, 3.330E-04, 3.330E-04, 3.330E-04,
     &   3.330E-04, 3.330E-04, 3.320E-04, 3.330E-04, 3.320E-04,
     &   3.330E-04, 3.330E-04, 3.330E-04, 3.320E-04, 3.330E-04,
     &   3.330E-04, 3.330E-04, 3.330E-04, 3.330E-04, 3.330E-04,
     &   3.330E-04, 3.330E-04, 3.330E-04, 3.330E-04, 3.330E-04,
     &   3.330E-04, 3.320E-04, 3.330E-04, 3.320E-04, 3.330E-04,
     &   3.320E-04, 3.330E-04, 3.330E-04, 3.320E-04, 3.330E-04,
     &   3.320E-04, 3.320E-04, 3.330E-04, 3.320E-04, 3.320E-04,
     &   3.320E-04, 3.320E-04, 3.320E-04, 3.330E-04, 3.320E-04,
     &   3.330E-04, 3.330E-04, 3.320E-04, 3.330E-04, 3.320E-04,
     &   3.330E-04, 3.320E-04, 3.330E-04, 3.320E-04, 3.320E-04,
     &   3.330E-04, 3.330E-04, 3.320E-04, 3.330E-04, 3.320E-04,
     &   3.330E-04, 3.330E-04, 3.320E-04, 3.320E-04, 3.320E-04,
     &   3.320E-04, 3.320E-04, 3.330E-04, 3.320E-04, 3.320E-04,
     &   3.320E-04, 3.320E-04, 3.320E-04, 3.320E-04, 3.310E-04,
     &   3.310E-04, 3.320E-04, 3.310E-04, 3.310E-04, 3.310E-04,
     &   3.310E-04, 3.310E-04, 3.300E-04, 3.310E-04, 3.310E-04/

        DATA ((DAT(I,J), I=101,200), J=2,2) /
     &   3.310E-04, 3.300E-04, 3.310E-04, 3.310E-04, 3.310E-04,
     &   3.310E-04, 3.310E-04, 3.310E-04, 3.300E-04, 3.300E-04,
     &   3.310E-04, 3.300E-04, 3.300E-04, 3.300E-04, 3.300E-04,
     &   3.300E-04, 3.300E-04, 3.300E-04, 3.300E-04, 3.290E-04,
     &   3.300E-04, 3.300E-04, 3.300E-04, 3.290E-04, 3.300E-04,
     &   3.300E-04, 3.290E-04, 3.290E-04, 3.290E-04, 3.280E-04,
     &   3.290E-04, 3.290E-04, 3.280E-04, 3.290E-04, 3.280E-04,
     &   3.280E-04, 3.280E-04, 3.280E-04, 3.280E-04, 3.270E-04,
     &   3.270E-04, 3.270E-04, 3.260E-04, 3.260E-04, 3.260E-04,
     &   3.250E-04, 3.250E-04, 3.250E-04, 3.250E-04, 3.240E-04,
     &   3.240E-04, 3.240E-04, 3.230E-04, 3.230E-04, 3.220E-04,
     &   3.210E-04, 3.200E-04, 3.200E-04, 3.200E-04, 3.180E-04,
     &   3.170E-04, 3.170E-04, 3.160E-04, 3.140E-04, 3.130E-04,
     &   3.120E-04, 3.110E-04, 3.080E-04, 3.060E-04, 3.040E-04,
     &   3.010E-04, 2.980E-04, 2.960E-04, 2.920E-04, 2.880E-04,
     &   2.830E-04, 2.780E-04, 2.710E-04, 2.630E-04, 2.540E-04,
     &   2.440E-04, 2.480E-04, 2.470E-04, 2.450E-04, 2.440E-04,
     &   2.430E-04, 2.420E-04, 2.410E-04, 2.390E-04, 2.390E-04,
     &   2.370E-04, 2.360E-04, 2.350E-04, 2.330E-04, 2.310E-04,
     &   2.310E-04, 2.290E-04, 2.280E-04, 2.260E-04, 2.250E-04/

        DATA ((DAT(I,J), I=201,300), J=2,2) /
     &   2.240E-04, 2.220E-04, 2.210E-04, 2.180E-04, 2.170E-04,
     &   2.160E-04, 2.140E-04, 2.120E-04, 2.110E-04, 2.090E-04,
     &   2.080E-04, 2.060E-04, 2.050E-04, 2.030E-04, 2.000E-04,
     &   1.990E-04, 1.970E-04, 1.960E-04, 1.940E-04, 1.920E-04,
     &   1.900E-04, 1.880E-04, 1.860E-04, 1.840E-04, 1.820E-04,
     &   1.800E-04, 1.780E-04, 1.760E-04, 1.740E-04, 1.720E-04,
     &   1.700E-04, 1.680E-04, 1.660E-04, 1.630E-04, 1.610E-04,
     &   1.590E-04, 1.570E-04, 1.540E-04, 1.520E-04, 1.500E-04,
     &   1.470E-04, 1.450E-04, 1.430E-04, 1.409E-04, 1.380E-04,
     &   1.355E-04, 1.332E-04, 1.308E-04, 1.283E-04, 1.259E-04,
     &   1.233E-04, 1.209E-04, 1.185E-04, 1.160E-04, 1.135E-04,
     &   1.110E-04, 1.085E-04, 1.060E-04, 1.035E-04, 1.009E-04,
     &   9.850E-05, 9.600E-05, 9.340E-05, 9.100E-05, 8.850E-05,
     &   8.600E-05, 8.350E-05, 8.100E-05, 7.840E-05, 7.590E-05,
     &   7.340E-05, 7.090E-05, 6.820E-05, 6.560E-05, 6.290E-05,
     &   6.010E-05, 5.730E-05, 5.430E-05, 5.120E-05, 4.800E-05,
     &   4.470E-05, 4.430E-05, 4.400E-05, 4.360E-05, 4.320E-05,
     &   4.280E-05, 4.250E-05, 4.210E-05, 4.180E-05, 4.140E-05,
     &   4.100E-05, 4.070E-05, 4.030E-05, 3.990E-05, 3.950E-05,
     &   3.910E-05, 3.870E-05, 3.830E-05, 3.800E-05, 3.760E-05/

        DATA ((DAT(I,J), I=301,400), J=2,2) /
     &   3.720E-05, 3.680E-05, 3.640E-05, 3.600E-05, 3.560E-05,
     &   3.520E-05, 3.470E-05, 3.430E-05, 3.390E-05, 3.350E-05,
     &   3.310E-05, 3.270E-05, 3.220E-05, 3.180E-05, 3.140E-05,
     &   3.100E-05, 3.050E-05, 3.010E-05, 2.960E-05, 2.920E-05,
     &   2.870E-05, 2.820E-05, 2.780E-05, 2.730E-05, 2.690E-05,
     &   2.640E-05, 2.600E-05, 2.540E-05, 2.490E-05, 2.450E-05,
     &   2.400E-05, 2.350E-05, 2.300E-05, 2.260E-05, 2.210E-05,
     &   2.160E-05, 2.110E-05, 2.060E-05, 2.000E-05, 1.950E-05,
     &   1.910E-05, 1.860E-05, 1.800E-05, 1.750E-05, 1.700E-05,
     &   1.650E-05, 1.600E-05, 1.550E-05, 1.500E-05, 1.450E-05,
     &   1.397E-05, 1.346E-05, 1.296E-05, 1.244E-05, 1.195E-05,
     &   1.145E-05, 1.095E-05, 1.045E-05, 9.960E-06, 9.470E-06,
     &   8.990E-06, 8.520E-06, 8.050E-06, 7.580E-06, 7.130E-06,
     &   6.680E-06, 6.230E-06, 5.800E-06, 5.380E-06, 4.980E-06,
     &   4.570E-06, 4.190E-06, 3.820E-06, 3.450E-06, 3.110E-06,
     &   2.770E-06, 2.460E-06, 2.160E-06, 1.870E-06, 1.610E-06,
     &   1.359E-06, 1.336E-06, 1.312E-06, 1.289E-06, 1.266E-06,
     &   1.243E-06, 1.221E-06, 1.198E-06, 1.175E-06, 1.153E-06,
     &   1.131E-06, 1.110E-06, 1.088E-06, 1.067E-06, 1.045E-06,
     &   1.025E-06, 1.004E-06, 9.830E-07, 9.620E-07, 9.420E-07/

        DATA ((DAT(I,J), I=401,500), J=2,2) /
     &   9.230E-07, 9.030E-07, 8.840E-07, 8.640E-07, 8.450E-07,
     &   8.260E-07, 8.080E-07, 7.890E-07, 7.710E-07, 7.520E-07,
     &   7.340E-07, 7.170E-07, 7.000E-07, 6.820E-07, 6.650E-07,
     &   6.480E-07, 6.320E-07, 6.150E-07, 5.990E-07, 5.820E-07,
     &   5.660E-07, 5.510E-07, 5.350E-07, 5.200E-07, 5.060E-07,
     &   4.910E-07, 4.760E-07, 4.620E-07, 4.470E-07, 4.330E-07,
     &   4.200E-07, 4.060E-07, 3.930E-07, 3.800E-07, 3.670E-07,
     &   3.540E-07, 3.420E-07, 3.290E-07, 3.180E-07, 3.060E-07,
     &   2.950E-07, 2.830E-07, 2.720E-07, 2.610E-07, 2.500E-07,
     &   2.400E-07, 2.300E-07, 2.190E-07, 2.100E-07, 2.010E-07,
     &   1.910E-07, 1.820E-07, 1.730E-07, 1.640E-07, 1.560E-07,
     &   1.474E-07, 1.393E-07, 1.316E-07, 1.240E-07, 1.166E-07,
     &   1.095E-07, 1.026E-07, 9.590E-08, 8.950E-08, 8.320E-08,
     &   7.720E-08, 7.140E-08, 6.580E-08, 6.050E-08, 5.540E-08,
     &   5.050E-08, 4.580E-08, 4.140E-08, 3.710E-08, 3.320E-08,
     &   2.940E-08, 2.590E-08, 2.250E-08, 1.950E-08, 1.660E-08,
     &   1.400E-08, 1.374E-08, 1.350E-08, 1.325E-08, 1.301E-08,
     &   1.277E-08, 1.252E-08, 1.229E-08, 1.205E-08, 1.182E-08,
     &   1.160E-08, 1.136E-08, 1.114E-08, 1.092E-08, 1.069E-08,
     &   1.047E-08, 1.026E-08, 1.004E-08, 9.830E-09, 9.620E-09/

        DATA ((DAT(I,J), I=501,600), J=2,2) /
     &   9.410E-09, 9.210E-09, 9.010E-09, 8.800E-09, 8.610E-09,
     &   8.400E-09, 8.220E-09, 8.020E-09, 7.830E-09, 7.650E-09,
     &   7.460E-09, 7.280E-09, 7.100E-09, 6.920E-09, 6.750E-09,
     &   6.570E-09, 6.400E-09, 6.220E-09, 6.060E-09, 5.900E-09,
     &   5.730E-09, 5.570E-09, 5.410E-09, 5.260E-09, 5.110E-09,
     &   4.960E-09, 4.810E-09, 4.660E-09, 4.520E-09, 4.380E-09,
     &   4.230E-09, 4.100E-09, 3.960E-09, 3.830E-09, 3.690E-09,
     &   3.580E-09, 3.440E-09, 3.320E-09, 3.200E-09, 3.070E-09,
     &   2.960E-09, 2.850E-09, 2.730E-09, 2.620E-09, 2.520E-09,
     &   2.410E-09, 2.310E-09, 2.200E-09, 2.100E-09, 2.010E-09,
     &   1.910E-09, 1.820E-09, 1.740E-09, 1.650E-09, 1.560E-09,
     &   1.479E-09, 1.398E-09, 1.320E-09, 1.244E-09, 1.169E-09,
     &   1.098E-09, 1.028E-09, 9.610E-10, 8.960E-10, 8.340E-10,
     &   7.740E-10, 7.150E-10, 6.590E-10, 6.060E-10, 5.550E-10,
     &   5.050E-10, 4.590E-10, 4.140E-10, 3.720E-10, 3.320E-10,
     &   2.950E-10, 2.590E-10, 2.260E-10, 1.950E-10, 1.670E-10,
     &   1.400E-10, 1.375E-10, 1.350E-10, 1.326E-10, 1.301E-10,
     &   1.276E-10, 1.253E-10, 1.229E-10, 1.205E-10, 1.182E-10,
     &   1.160E-10, 1.137E-10, 1.114E-10, 1.091E-10, 1.069E-10,
     &   1.048E-10, 1.026E-10, 1.004E-10, 9.840E-11, 9.620E-11/

        DATA ((DAT(I,J), I=601,700), J=2,2) /
     &   9.420E-11, 9.210E-11, 9.010E-11, 8.800E-11, 8.610E-11,
     &   8.410E-11, 8.210E-11, 8.020E-11, 7.830E-11, 7.640E-11,
     &   7.460E-11, 7.280E-11, 7.100E-11, 6.920E-11, 6.740E-11,
     &   6.570E-11, 6.400E-11, 6.230E-11, 6.060E-11, 5.900E-11,
     &   5.730E-11, 5.570E-11, 5.410E-11, 5.260E-11, 5.110E-11,
     &   4.960E-11, 4.800E-11, 4.670E-11, 4.520E-11, 4.370E-11,
     &   4.230E-11, 4.100E-11, 3.960E-11, 3.830E-11, 3.700E-11,
     &   3.570E-11, 3.450E-11, 3.320E-11, 3.200E-11, 3.080E-11,
     &   2.970E-11, 2.850E-11, 2.740E-11, 2.630E-11, 2.510E-11,
     &   2.410E-11, 2.300E-11, 2.210E-11, 2.110E-11, 2.010E-11,
     &   1.920E-11, 1.820E-11, 1.730E-11, 1.640E-11, 1.570E-11,
     &   1.479E-11, 1.398E-11, 1.319E-11, 1.243E-11, 1.170E-11,
     &   1.098E-11, 1.028E-11, 9.610E-12, 8.960E-12, 8.330E-12,
     &   7.730E-12, 7.150E-12, 6.600E-12, 6.060E-12, 5.540E-12,
     &   5.060E-12, 4.590E-12, 4.140E-12, 3.720E-12, 3.330E-12,
     &   2.940E-12, 2.590E-12, 2.260E-12, 1.950E-12, 1.660E-12,
     &   1.400E-12, 1.375E-12, 1.350E-12, 1.326E-12, 1.301E-12,
     &   1.277E-12, 1.253E-12, 1.230E-12, 1.205E-12, 1.182E-12,
     &   1.160E-12, 1.137E-12, 1.114E-12, 1.091E-12, 1.069E-12,
     &   1.047E-12, 1.026E-12, 1.004E-12, 9.840E-13, 9.620E-13/

        DATA ((DAT(I,J), I=701,800), J=2,2) /
     &   9.410E-13, 9.210E-13, 9.010E-13, 8.800E-13, 8.610E-13,
     &   8.410E-13, 8.210E-13, 8.030E-13, 7.830E-13, 7.640E-13,
     &   7.460E-13, 7.270E-13, 7.100E-13, 6.920E-13, 6.740E-13,
     &   6.570E-13, 6.400E-13, 6.230E-13, 6.060E-13, 5.900E-13,
     &   5.730E-13, 5.570E-13, 5.410E-13, 5.260E-13, 5.110E-13,
     &   4.960E-13, 4.800E-13, 4.670E-13, 4.520E-13, 4.370E-13,
     &   4.230E-13, 4.100E-13, 3.960E-13, 3.830E-13, 3.700E-13,
     &   3.570E-13, 3.450E-13, 3.320E-13, 3.200E-13, 3.080E-13,
     &   2.970E-13, 2.850E-13, 2.740E-13, 2.630E-13, 2.510E-13,
     &   2.410E-13, 2.300E-13, 2.210E-13, 2.110E-13, 2.010E-13,
     &   1.920E-13, 1.820E-13, 1.730E-13, 1.640E-13, 1.570E-13,
     &   1.479E-13, 1.398E-13, 1.319E-13, 1.243E-13, 1.170E-13,
     &   1.098E-13, 1.028E-13, 9.610E-14, 8.960E-14, 8.330E-14,
     &   7.730E-14, 7.150E-14, 6.600E-14, 6.060E-14, 5.540E-14,
     &   5.060E-14, 4.590E-14, 4.140E-14, 3.720E-14, 3.330E-14,
     &   2.940E-14, 2.590E-14, 2.260E-14, 1.950E-14, 1.660E-14,
     &   1.400E-14, 1.375E-14, 1.350E-14, 1.326E-14, 1.301E-14,
     &   1.277E-14, 1.253E-14, 1.230E-14, 1.205E-14, 1.182E-14,
     &   1.160E-14, 1.137E-14, 1.114E-14, 1.091E-14, 1.069E-14,
     &   1.047E-14, 1.026E-14, 1.004E-14, 9.840E-15, 9.620E-15/

        DATA ((DAT(I,J), I=801,882), J=2,2) /
     &   9.410E-15, 9.210E-15, 9.010E-15, 8.800E-15, 8.610E-15,
     &   8.410E-15, 8.210E-15, 8.030E-15, 7.830E-15, 7.640E-15,
     &   7.460E-15, 7.270E-15, 7.100E-15, 6.920E-15, 6.740E-15,
     &   6.570E-15, 6.400E-15, 6.230E-15, 6.060E-15, 5.900E-15,
     &   5.730E-15, 5.570E-15, 5.410E-15, 5.260E-15, 5.110E-15,
     &   4.960E-15, 4.800E-15, 4.670E-15, 4.520E-15, 4.370E-15,
     &   4.230E-15, 4.100E-15, 3.960E-15, 3.830E-15, 3.700E-15,
     &   3.570E-15, 3.450E-15, 3.320E-15, 3.200E-15, 3.080E-15,
     &   2.970E-15, 2.850E-15, 2.740E-15, 2.630E-15, 2.510E-15,
     &   2.410E-15, 2.300E-15, 2.210E-15, 2.110E-15, 2.010E-15,
     &   1.920E-15, 1.820E-15, 1.730E-15, 1.640E-15, 1.570E-15,
     &   1.479E-15, 1.398E-15, 1.319E-15, 1.243E-15, 1.170E-15,
     &   1.098E-15, 1.028E-15, 9.610E-16, 8.960E-16, 8.330E-16,
     &   7.730E-16, 7.150E-16, 6.600E-16, 6.060E-16, 5.540E-16,
     &   5.060E-16, 4.590E-16, 4.140E-16, 3.720E-16, 3.330E-16,
     &   2.940E-16, 2.590E-16, 2.260E-16, 1.950E-16, 1.660E-16,
     &   1.400E-16, 0.000E+00/
C      
      save
      RETURN
      END

      subroutine dalhad_timelike
* hadronic contribution of the 5 light quark flavors evaluated using 
* e^+e^-data. CMD-2 2001 data have a normalization problem. I added a energy 
* independent +1.5 % correction and incresed the syst error by 0.5%. 
* Results thus are preliminary! Arrays:
* ESA(I)  : energy E in GeV; 
* DAS(I,1): Delta alpha^5(E**2) 
* DAS(I,2): error (systematics dominated, should be treated as a systematic error)
C
      IMPLICIT NONE
      INTEGER NB,I,J
      PARAMETER(NB=530)
      REAL ESA(NB),DAS(NB,2)
      COMMON /DATS/ESA,DAS
C 530
      DATA  (ESA(I), I=1,100) /
     &   1.000E-06, 3.001E-03, 6.001E-03, 9.001E-03, 1.200E-02,
     &   1.500E-02, 1.800E-02, 2.100E-02, 2.400E-02, 2.700E-02,
     &   3.000E-02, 3.300E-02, 3.600E-02, 3.900E-02, 4.200E-02,
     &   4.500E-02, 4.800E-02, 5.100E-02, 5.400E-02, 5.700E-02,
     &   6.000E-02, 6.300E-02, 6.600E-02, 6.900E-02, 7.200E-02,
     &   7.500E-02, 7.800E-02, 8.100E-02, 8.400E-02, 8.700E-02,
     &   9.000E-02, 9.300E-02, 9.600E-02, 9.900E-02, 1.020E-01,
     &   1.050E-01, 1.080E-01, 1.110E-01, 1.140E-01, 1.170E-01,
     &   1.200E-01, 1.230E-01, 1.260E-01, 1.290E-01, 1.320E-01,
     &   1.350E-01, 1.380E-01, 1.410E-01, 1.440E-01, 1.470E-01,
     &   1.500E-01, 1.530E-01, 1.560E-01, 1.590E-01, 1.620E-01,
     &   1.650E-01, 1.680E-01, 1.710E-01, 1.740E-01, 1.770E-01,
     &   1.800E-01, 1.830E-01, 1.860E-01, 1.890E-01, 1.920E-01,
     &   1.950E-01, 1.980E-01, 2.010E-01, 2.040E-01, 2.070E-01,
     &   2.100E-01, 2.130E-01, 2.160E-01, 2.190E-01, 2.220E-01,
     &   2.250E-01, 2.280E-01, 2.310E-01, 2.340E-01, 2.370E-01,
     &   2.400E-01, 2.430E-01, 2.460E-01, 2.490E-01, 2.520E-01,
     &   2.550E-01, 2.580E-01, 2.610E-01, 2.640E-01, 2.670E-01,
     &   2.700E-01, 2.710E-01, 2.730E-01, 2.737E-01, 2.760E-01,
     &   2.763E-01, 2.770E-01, 3.200E-01, 3.387E-01, 3.573E-01/

      DATA  (ESA(I), I=101,200) /
     &   3.760E-01, 3.947E-01, 4.133E-01, 4.320E-01, 4.507E-01,
     &   4.693E-01, 4.880E-01, 5.067E-01, 5.253E-01, 5.440E-01,
     &   5.627E-01, 5.813E-01, 6.000E-01, 6.100E-01, 6.150E-01,
     &   6.200E-01, 6.250E-01, 6.300E-01, 6.350E-01, 6.400E-01,
     &   6.450E-01, 6.500E-01, 6.550E-01, 6.600E-01, 6.650E-01,
     &   6.700E-01, 6.750E-01, 6.800E-01, 6.850E-01, 6.900E-01,
     &   6.950E-01, 7.000E-01, 7.050E-01, 7.100E-01, 7.150E-01,
     &   7.200E-01, 7.250E-01, 7.300E-01, 7.350E-01, 7.400E-01,
     &   7.450E-01, 7.500E-01, 7.550E-01, 7.600E-01, 7.650E-01,
     &   7.700E-01, 7.704E-01, 7.708E-01, 7.712E-01, 7.716E-01,
     &   7.720E-01, 7.724E-01, 7.728E-01, 7.732E-01, 7.736E-01,
     &   7.740E-01, 7.744E-01, 7.748E-01, 7.752E-01, 7.756E-01,
     &   7.760E-01, 7.764E-01, 7.768E-01, 7.772E-01, 7.776E-01,
     &   7.780E-01, 7.784E-01, 7.788E-01, 7.792E-01, 7.796E-01,
     &   7.800E-01, 7.804E-01, 7.808E-01, 7.812E-01, 7.816E-01,
     &   7.820E-01, 7.824E-01, 7.828E-01, 7.832E-01, 7.836E-01,
     &   7.840E-01, 7.844E-01, 7.848E-01, 7.852E-01, 7.856E-01,
     &   7.860E-01, 7.864E-01, 7.868E-01, 7.872E-01, 7.876E-01,
     &   7.880E-01, 7.884E-01, 7.888E-01, 7.892E-01, 7.896E-01,
     &   7.900E-01, 7.905E-01, 7.910E-01, 7.915E-01, 7.920E-01/

      DATA  (ESA(I), I=201,300) /
     &   7.925E-01, 7.930E-01, 7.935E-01, 7.940E-01, 7.945E-01,
     &   7.950E-01, 7.955E-01, 7.960E-01, 7.965E-01, 7.970E-01,
     &   7.975E-01, 7.980E-01, 7.985E-01, 7.990E-01, 7.995E-01,
     &   8.000E-01, 8.005E-01, 8.010E-01, 8.015E-01, 8.020E-01,
     &   8.025E-01, 8.030E-01, 8.035E-01, 8.040E-01, 8.045E-01,
     &   8.050E-01, 8.055E-01, 8.060E-01, 8.065E-01, 8.070E-01,
     &   8.075E-01, 8.080E-01, 8.085E-01, 8.090E-01, 8.095E-01,
     &   8.100E-01, 8.105E-01, 8.110E-01, 8.115E-01, 8.120E-01,
     &   8.125E-01, 8.130E-01, 8.135E-01, 8.140E-01, 8.145E-01,
     &   8.150E-01, 8.155E-01, 8.160E-01, 8.165E-01, 8.170E-01,
     &   8.175E-01, 8.180E-01, 8.185E-01, 8.190E-01, 8.195E-01,
     &   8.200E-01, 8.220E-01, 8.240E-01, 8.260E-01, 8.280E-01,
     &   8.300E-01, 8.320E-01, 8.340E-01, 8.360E-01, 8.380E-01,
     &   8.400E-01, 8.420E-01, 8.440E-01, 8.460E-01, 8.480E-01,
     &   8.500E-01, 8.520E-01, 8.540E-01, 8.560E-01, 8.580E-01,
     &   8.600E-01, 8.620E-01, 8.640E-01, 8.660E-01, 8.680E-01,
     &   8.700E-01, 8.720E-01, 8.740E-01, 8.760E-01, 8.780E-01,
     &   8.800E-01, 8.820E-01, 8.840E-01, 8.860E-01, 8.880E-01,
     &   8.900E-01, 8.920E-01, 8.940E-01, 8.960E-01, 8.980E-01,
     &   9.000E-01, 9.020E-01, 9.040E-01, 9.060E-01, 9.080E-01/

      DATA  (ESA(I), I=301,400) /
     &   9.100E-01, 9.120E-01, 9.140E-01, 9.160E-01, 9.180E-01,
     &   9.200E-01, 9.240E-01, 9.280E-01, 9.320E-01, 9.360E-01,
     &   9.400E-01, 9.440E-01, 9.480E-01, 9.520E-01, 9.560E-01,
     &   9.600E-01, 9.640E-01, 9.680E-01, 9.720E-01, 9.760E-01,
     &   9.800E-01, 9.840E-01, 9.880E-01, 9.920E-01, 9.960E-01,
     &   1.004E+00, 1.008E+00, 1.012E+00, 1.016E+00, 1.020E+00,
     &   1.024E+00, 1.028E+00, 1.032E+00, 1.036E+00, 1.044E+00,
     &   1.048E+00, 1.052E+00, 1.056E+00, 1.060E+00, 1.064E+00,
     &   1.068E+00, 1.072E+00, 1.076E+00, 1.080E+00, 1.084E+00,
     &   1.088E+00, 1.092E+00, 1.096E+00, 1.100E+00, 2.000E+01,
     &   2.100E+01, 2.200E+01, 2.300E+01, 2.400E+01, 2.500E+01,
     &   2.600E+01, 2.700E+01, 2.800E+01, 2.900E+01, 3.000E+01,
     &   3.100E+01, 3.200E+01, 3.300E+01, 3.400E+01, 3.500E+01,
     &   3.600E+01, 3.700E+01, 3.800E+01, 3.900E+01, 4.000E+01,
     &   4.100E+01, 4.200E+01, 4.300E+01, 4.400E+01, 4.500E+01,
     &   4.600E+01, 4.700E+01, 4.800E+01, 4.900E+01, 5.000E+01,
     &   5.100E+01, 5.200E+01, 5.300E+01, 5.400E+01, 5.500E+01,
     &   5.600E+01, 5.700E+01, 5.800E+01, 5.900E+01, 6.000E+01,
     &   6.100E+01, 6.200E+01, 6.300E+01, 6.400E+01, 6.500E+01,
     &   6.600E+01, 6.700E+01, 6.800E+01, 6.900E+01, 7.000E+01/

      DATA  (ESA(I), I=401,500) /
     &   7.100E+01, 7.200E+01, 7.300E+01, 7.400E+01, 7.500E+01,
     &   7.600E+01, 7.700E+01, 7.800E+01, 7.900E+01, 8.000E+01,
     &   8.100E+01, 8.200E+01, 8.300E+01, 8.400E+01, 8.500E+01,
     &   8.600E+01, 8.700E+01, 8.800E+01, 8.900E+01, 9.000E+01,
     &   9.100E+01, 9.200E+01, 9.300E+01, 9.400E+01, 9.500E+01,
     &   9.600E+01, 9.700E+01, 9.800E+01, 9.900E+01, 1.000E+02,
     &   1.090E+02, 1.180E+02, 1.270E+02, 1.360E+02, 1.450E+02,
     &   1.540E+02, 1.630E+02, 1.720E+02, 1.810E+02, 1.900E+02,
     &   1.990E+02, 2.080E+02, 2.170E+02, 2.260E+02, 2.350E+02,
     &   2.440E+02, 2.530E+02, 2.620E+02, 2.710E+02, 2.800E+02,
     &   2.890E+02, 2.980E+02, 3.070E+02, 3.160E+02, 3.250E+02,
     &   3.340E+02, 3.430E+02, 3.520E+02, 3.610E+02, 3.700E+02,
     &   3.790E+02, 3.880E+02, 3.970E+02, 4.060E+02, 4.150E+02,
     &   4.240E+02, 4.330E+02, 4.420E+02, 4.510E+02, 4.600E+02,
     &   4.690E+02, 4.780E+02, 4.870E+02, 4.960E+02, 5.050E+02,
     &   5.140E+02, 5.230E+02, 5.320E+02, 5.410E+02, 5.500E+02,
     &   5.590E+02, 5.680E+02, 5.770E+02, 5.860E+02, 5.950E+02,
     &   6.040E+02, 6.130E+02, 6.220E+02, 6.310E+02, 6.400E+02,
     &   6.490E+02, 6.580E+02, 6.670E+02, 6.760E+02, 6.850E+02,
     &   6.940E+02, 7.030E+02, 7.120E+02, 7.210E+02, 7.300E+02/

      DATA  (ESA(I), I=501,530) /
     &   7.390E+02, 7.480E+02, 7.570E+02, 7.660E+02, 7.750E+02,
     &   7.840E+02, 7.930E+02, 8.020E+02, 8.110E+02, 8.200E+02,
     &   8.290E+02, 8.380E+02, 8.470E+02, 8.560E+02, 8.650E+02,
     &   8.740E+02, 8.830E+02, 8.920E+02, 9.010E+02, 9.100E+02,
     &   9.190E+02, 9.280E+02, 9.370E+02, 9.460E+02, 9.550E+02,
     &   9.640E+02, 9.730E+02, 9.820E+02, 9.910E+02, 1.000E+03/

        DATA ((DAS(I,J), I=1,100), J=1,1) /
     &  -9.736E-15,-8.259E-08,-3.303E-07,-7.431E-07,-1.321E-06,
     &  -2.064E-06,-2.973E-06,-4.048E-06,-5.289E-06,-6.696E-06,
     &  -8.269E-06,-1.001E-05,-1.192E-05,-1.399E-05,-1.624E-05,
     &  -1.865E-05,-2.123E-05,-2.398E-05,-2.690E-05,-3.000E-05,
     &  -3.326E-05,-3.670E-05,-4.031E-05,-4.410E-05,-4.806E-05,
     &  -5.220E-05,-5.652E-05,-6.101E-05,-6.568E-05,-7.054E-05,
     &  -7.557E-05,-8.079E-05,-8.619E-05,-9.178E-05,-9.755E-05,
     &  -1.035E-04,-1.097E-04,-1.160E-04,-1.226E-04,-1.293E-04,
     &  -1.362E-04,-1.434E-04,-1.507E-04,-1.582E-04,-1.660E-04,
     &  -1.739E-04,-1.821E-04,-1.904E-04,-1.990E-04,-2.078E-04,
     &  -2.168E-04,-2.261E-04,-2.356E-04,-2.453E-04,-2.552E-04,
     &  -2.653E-04,-2.758E-04,-2.864E-04,-2.973E-04,-3.084E-04,
     &  -3.198E-04,-3.315E-04,-3.434E-04,-3.556E-04,-3.680E-04,
     &  -3.808E-04,-3.938E-04,-4.071E-04,-4.207E-04,-4.346E-04,
     &  -4.488E-04,-4.634E-04,-4.782E-04,-4.934E-04,-5.089E-04,
     &  -5.248E-04,-5.410E-04,-5.577E-04,-5.747E-04,-5.921E-04,
     &  -6.099E-04,-6.281E-04,-6.468E-04,-6.660E-04,-6.856E-04,
     &  -7.058E-04,-7.266E-04,-7.479E-04,-7.699E-04,-7.926E-04,
     &  -8.161E-04,-8.089E-04,-8.406E-04,-8.306E-04,-8.662E-04,
     &  -8.533E-04,-8.751E-04,-1.262E-03,-1.438E-03,-1.612E-03/

        DATA ((DAS(I,J), I=101,200), J=1,1) /
     &  -1.812E-03,-1.991E-03,-2.223E-03,-2.460E-03,-2.697E-03,
     &  -3.007E-03,-3.356E-03,-3.682E-03,-4.029E-03,-4.532E-03,
     &  -5.050E-03,-5.545E-03,-6.280E-03,-6.663E-03,-6.863E-03,
     &  -7.069E-03,-7.281E-03,-7.497E-03,-7.719E-03,-7.944E-03,
     &  -8.173E-03,-8.404E-03,-8.636E-03,-8.866E-03,-9.093E-03,
     &  -9.313E-03,-9.522E-03,-9.715E-03,-9.887E-03,-1.003E-02,
     &  -1.014E-02,-1.020E-02,-1.020E-02,-1.013E-02,-9.987E-03,
     &  -9.745E-03,-9.399E-03,-8.941E-03,-8.372E-03,-7.702E-03,
     &  -6.954E-03,-6.172E-03,-5.429E-03,-4.842E-03,-4.600E-03,
     &  -4.939E-03,-5.012E-03,-5.092E-03,-5.180E-03,-5.274E-03,
     &  -5.376E-03,-5.485E-03,-5.597E-03,-5.720E-03,-5.846E-03,
     &  -5.974E-03,-6.104E-03,-6.233E-03,-6.356E-03,-6.470E-03,
     &  -6.567E-03,-6.640E-03,-6.677E-03,-6.665E-03,-6.584E-03,
     &  -6.415E-03,-6.127E-03,-5.688E-03,-5.059E-03,-4.197E-03,
     &  -3.059E-03,-1.607E-03, 1.835E-04, 2.313E-03, 4.745E-03,
     &   7.403E-03, 1.017E-02, 1.291E-02, 1.549E-02, 1.777E-02,
     &   1.968E-02, 2.120E-02, 2.232E-02, 2.308E-02, 2.354E-02,
     &   2.375E-02, 2.377E-02, 2.365E-02, 2.342E-02, 2.313E-02,
     &   2.279E-02, 2.242E-02, 2.203E-02, 2.165E-02, 2.127E-02,
     &   2.088E-02, 2.035E-02, 1.983E-02, 1.936E-02, 1.888E-02/

        DATA ((DAS(I,J), I=201,300), J=1,1) /
     &   1.841E-02, 1.798E-02, 1.756E-02, 1.718E-02, 1.679E-02,
     &   1.642E-02, 1.609E-02, 1.575E-02, 1.546E-02, 1.515E-02,
     &   1.485E-02, 1.458E-02, 1.431E-02, 1.408E-02, 1.383E-02,
     &   1.359E-02, 1.338E-02, 1.316E-02, 1.297E-02, 1.277E-02,
     &   1.258E-02, 1.239E-02, 1.223E-02, 1.206E-02, 1.188E-02,
     &   1.173E-02, 1.157E-02, 1.144E-02, 1.128E-02, 1.115E-02,
     &   1.101E-02, 1.089E-02, 1.075E-02, 1.062E-02, 1.051E-02,
     &   1.038E-02, 1.029E-02, 1.017E-02, 1.006E-02, 9.953E-03,
     &   9.843E-03, 9.760E-03, 9.655E-03, 9.565E-03, 9.465E-03,
     &   9.391E-03, 9.295E-03, 9.214E-03, 9.123E-03, 9.033E-03,
     &   8.968E-03, 8.883E-03, 8.810E-03, 8.728E-03, 8.669E-03,
     &   8.590E-03, 8.320E-03, 8.070E-03, 7.818E-03, 7.602E-03,
     &   7.400E-03, 7.210E-03, 7.022E-03, 6.852E-03, 6.692E-03,
     &   6.540E-03, 6.379E-03, 6.242E-03, 6.110E-03, 5.983E-03,
     &   5.862E-03, 5.738E-03, 5.625E-03, 5.510E-03, 5.581E-03,
     &   5.610E-03, 5.586E-03, 5.661E-03, 5.684E-03, 5.705E-03,
     &   5.683E-03, 5.688E-03, 5.646E-03, 5.651E-03, 5.621E-03,
     &   5.579E-03, 5.588E-03, 5.622E-03, 5.555E-03, 5.545E-03,
     &   5.473E-03, 5.490E-03, 5.456E-03, 5.465E-03, 5.382E-03,
     &   5.342E-03, 5.337E-03, 5.254E-03, 5.251E-03, 5.205E-03/

        DATA ((DAS(I,J), I=301,400), J=1,1) /
     &   5.185E-03, 5.102E-03, 5.086E-03, 5.028E-03, 4.931E-03,
     &   4.902E-03, 4.894E-03, 4.725E-03, 4.550E-03, 4.365E-03,
     &   4.186E-03, 3.968E-03, 3.764E-03, 3.498E-03, 3.227E-03,
     &   2.904E-03, 2.587E-03, 2.163E-03, 1.705E-03, 1.144E-03,
     &   5.114E-04,-2.812E-04,-1.269E-03,-2.578E-03,-4.424E-03,
     &  -1.046E-02,-1.577E-02,-2.603E-02,-4.838E-02, 3.453E-02,
     &   5.283E-02, 3.471E-02, 2.630E-02, 2.175E-02, 1.621E-02,
     &   1.446E-02, 1.324E-02, 1.230E-02, 1.154E-02, 1.092E-02,
     &   1.040E-02, 9.966E-03, 9.588E-03, 9.242E-03, 8.955E-03,
     &   8.691E-03, 8.450E-03, 8.227E-03, 8.027E-03, 1.862E-02,
     &   1.892E-02, 1.920E-02, 1.947E-02, 1.973E-02, 1.998E-02,
     &   2.021E-02, 2.044E-02, 2.066E-02, 2.087E-02, 2.107E-02,
     &   2.127E-02, 2.146E-02, 2.164E-02, 2.182E-02, 2.200E-02,
     &   2.216E-02, 2.232E-02, 2.248E-02, 2.264E-02, 2.279E-02,
     &   2.294E-02, 2.308E-02, 2.322E-02, 2.335E-02, 2.349E-02,
     &   2.362E-02, 2.374E-02, 2.387E-02, 2.399E-02, 2.411E-02,
     &   2.423E-02, 2.434E-02, 2.445E-02, 2.456E-02, 2.467E-02,
     &   2.478E-02, 2.488E-02, 2.498E-02, 2.508E-02, 2.518E-02,
     &   2.528E-02, 2.537E-02, 2.547E-02, 2.556E-02, 2.565E-02,
     &   2.574E-02, 2.583E-02, 2.592E-02, 2.600E-02, 2.609E-02/

        DATA ((DAS(I,J), I=401,500), J=1,1) /
     &   2.617E-02, 2.625E-02, 2.633E-02, 2.641E-02, 2.649E-02,
     &   2.657E-02, 2.664E-02, 2.672E-02, 2.679E-02, 2.686E-02,
     &   2.693E-02, 2.701E-02, 2.708E-02, 2.715E-02, 2.722E-02,
     &   2.728E-02, 2.735E-02, 2.742E-02, 2.748E-02, 2.755E-02,
     &   2.761E-02, 2.767E-02, 2.774E-02, 2.780E-02, 2.786E-02,
     &   2.792E-02, 2.798E-02, 2.804E-02, 2.809E-02, 2.815E-02,
     &   2.865E-02, 2.910E-02, 2.951E-02, 2.990E-02, 3.025E-02,
     &   3.059E-02, 3.090E-02, 3.118E-02, 3.146E-02, 3.171E-02,
     &   3.195E-02, 3.217E-02, 3.238E-02, 3.258E-02, 3.276E-02,
     &   3.293E-02, 3.309E-02, 3.323E-02, 3.336E-02, 3.347E-02,
     &   3.357E-02, 3.366E-02, 3.372E-02, 3.376E-02, 3.373E-02,
     &   3.365E-02, 3.338E-02, 3.303E-02, 3.323E-02, 3.345E-02,
     &   3.386E-02, 3.415E-02, 3.444E-02, 3.470E-02, 3.499E-02,
     &   3.523E-02, 3.548E-02, 3.571E-02, 3.596E-02, 3.618E-02,
     &   3.638E-02, 3.660E-02, 3.679E-02, 3.699E-02, 3.718E-02,
     &   3.736E-02, 3.754E-02, 3.771E-02, 3.788E-02, 3.804E-02,
     &   3.821E-02, 3.836E-02, 3.852E-02, 3.867E-02, 3.882E-02,
     &   3.896E-02, 3.909E-02, 3.924E-02, 3.937E-02, 3.951E-02,
     &   3.964E-02, 3.976E-02, 3.989E-02, 4.001E-02, 4.013E-02,
     &   4.025E-02, 4.037E-02, 4.048E-02, 4.060E-02, 4.071E-02/

        DATA ((DAS(I,J), I=501,530), J=1,1) /
     &   4.082E-02, 4.092E-02, 4.103E-02, 4.114E-02, 4.124E-02,
     &   4.134E-02, 4.145E-02, 4.154E-02, 4.164E-02, 4.174E-02,
     &   4.183E-02, 4.192E-02, 4.202E-02, 4.211E-02, 4.219E-02,
     &   4.229E-02, 4.238E-02, 4.247E-02, 4.255E-02, 4.264E-02,
     &   4.272E-02, 4.280E-02, 4.289E-02, 4.297E-02, 4.304E-02,
     &   4.313E-02, 4.321E-02, 4.328E-02, 4.336E-02, 4.344E-02/

        DATA ((DAS(I,J), I=1,100), J=2,2) /
     &   1.427E-16, 1.261E-09, 5.040E-09, 1.135E-08, 2.020E-08,
     &   3.150E-08, 4.540E-08, 6.180E-08, 8.080E-08, 1.023E-07,
     &   1.264E-07, 1.530E-07, 1.820E-07, 2.140E-07, 2.490E-07,
     &   2.850E-07, 3.250E-07, 3.670E-07, 4.120E-07, 4.590E-07,
     &   5.100E-07, 5.630E-07, 6.180E-07, 6.770E-07, 7.370E-07,
     &   8.010E-07, 8.680E-07, 9.380E-07, 1.010E-06, 1.084E-06,
     &   1.163E-06, 1.244E-06, 1.327E-06, 1.414E-06, 1.505E-06,
     &   1.600E-06, 1.700E-06, 1.790E-06, 1.890E-06, 2.000E-06,
     &   2.100E-06, 2.220E-06, 2.330E-06, 2.460E-06, 2.580E-06,
     &   2.700E-06, 2.830E-06, 2.960E-06, 3.100E-06, 3.240E-06,
     &   3.390E-06, 3.540E-06, 3.690E-06, 3.840E-06, 4.000E-06,
     &   4.170E-06, 4.330E-06, 4.510E-06, 4.690E-06, 4.870E-06,
     &   5.050E-06, 5.250E-06, 5.440E-06, 5.640E-06, 5.850E-06,
     &   6.050E-06, 6.270E-06, 6.490E-06, 6.720E-06, 6.950E-06,
     &   7.200E-06, 7.440E-06, 7.690E-06, 7.950E-06, 8.210E-06,
     &   8.490E-06, 8.770E-06, 9.050E-06, 9.350E-06, 9.650E-06,
     &   9.970E-06, 1.029E-05, 1.061E-05, 1.096E-05, 1.131E-05,
     &   1.166E-05, 1.204E-05, 1.242E-05, 1.282E-05, 1.323E-05,
     &   1.366E-05, 1.670E-05, 1.410E-05, 1.715E-05, 1.456E-05,
     &   1.761E-05, 1.471E-05, 2.950E-05, 3.200E-05, 3.550E-05/

        DATA ((DAS(I,J), I=101,200), J=2,2) /
     &   3.970E-05, 4.330E-05, 4.820E-05, 5.320E-05, 5.800E-05,
     &   6.470E-05, 7.230E-05, 7.900E-05, 8.600E-05, 9.730E-05,
     &   1.083E-04, 1.183E-04, 1.348E-04, 1.430E-04, 1.472E-04,
     &   1.516E-04, 1.560E-04, 1.605E-04, 1.650E-04, 1.697E-04,
     &   1.743E-04, 1.788E-04, 1.833E-04, 1.875E-04, 1.915E-04,
     &   1.951E-04, 1.982E-04, 2.008E-04, 2.021E-04, 2.030E-04,
     &   2.030E-04, 2.000E-04, 1.960E-04, 1.890E-04, 1.797E-04,
     &   1.685E-04, 1.541E-04, 1.381E-04, 1.224E-04, 1.114E-04,
     &   1.121E-04, 1.303E-04, 1.656E-04, 2.154E-04, 2.797E-04,
     &   2.610E-04, 2.677E-04, 2.748E-04, 2.821E-04, 2.898E-04,
     &   2.977E-04, 3.060E-04, 3.145E-04, 3.235E-04, 3.328E-04,
     &   3.424E-04, 3.521E-04, 3.622E-04, 3.724E-04, 3.827E-04,
     &   3.929E-04, 4.027E-04, 4.120E-04, 4.204E-04, 4.274E-04,
     &   4.325E-04, 4.348E-04, 4.334E-04, 4.273E-04, 4.152E-04,
     &   3.958E-04, 3.682E-04, 3.319E-04, 2.878E-04, 2.391E-04,
     &   1.937E-04, 1.660E-04, 1.730E-04, 2.100E-04, 2.590E-04,
     &   3.090E-04, 3.520E-04, 3.880E-04, 4.150E-04, 4.330E-04,
     &   4.450E-04, 4.510E-04, 4.530E-04, 4.500E-04, 4.450E-04,
     &   4.380E-04, 4.300E-04, 4.220E-04, 4.130E-04, 4.030E-04,
     &   2.106E-03, 2.063E-03, 2.021E-03, 1.980E-03, 1.940E-03/

        DATA ((DAS(I,J), I=201,300), J=2,2) /
     &   1.902E-03, 1.863E-03, 1.825E-03, 1.788E-03, 1.753E-03,
     &   1.719E-03, 1.685E-03, 1.651E-03, 1.618E-03, 1.586E-03,
     &   1.554E-03, 1.524E-03, 1.493E-03, 1.464E-03, 1.434E-03,
     &   1.406E-03, 1.377E-03, 1.350E-03, 1.322E-03, 1.296E-03,
     &   1.268E-03, 1.243E-03, 1.217E-03, 1.192E-03, 1.167E-03,
     &   1.142E-03, 1.118E-03, 1.094E-03, 1.071E-03, 1.048E-03,
     &   1.025E-03, 1.003E-03, 9.807E-04, 9.593E-04, 9.375E-04,
     &   9.167E-04, 8.949E-04, 8.743E-04, 8.540E-04, 8.337E-04,
     &   8.138E-04, 7.939E-04, 7.745E-04, 7.552E-04, 7.363E-04,
     &   7.173E-04, 6.988E-04, 6.804E-04, 6.623E-04, 6.444E-04,
     &   6.266E-04, 6.091E-04, 5.917E-04, 5.746E-04, 5.575E-04,
     &   5.408E-04, 4.756E-04, 4.133E-04, 3.543E-04, 2.983E-04,
     &   2.459E-04, 1.983E-04, 1.578E-04, 1.350E-04, 1.640E-04,
     &   2.013E-04, 2.425E-04, 2.851E-04, 3.281E-04, 3.708E-04,
     &   4.131E-04, 4.551E-04, 4.964E-04, 5.281E-04, 5.300E-04,
     &   5.320E-04, 5.332E-04, 5.356E-04, 5.370E-04, 5.382E-04,
     &   5.391E-04, 5.402E-04, 5.412E-04, 5.425E-04, 5.439E-04,
     &   5.445E-04, 5.455E-04, 5.462E-04, 5.465E-04, 5.472E-04,
     &   5.474E-04, 5.482E-04, 5.486E-04, 5.497E-04, 5.500E-04,
     &   5.508E-04, 5.516E-04, 5.516E-04, 5.524E-04, 5.532E-04/

        DATA ((DAS(I,J), I=301,400), J=2,2) /
     &   5.548E-04, 5.545E-04, 5.550E-04, 5.555E-04, 5.557E-04,
     &   5.569E-04, 5.575E-04, 5.586E-04, 5.600E-04, 5.612E-04,
     &   5.633E-04, 5.654E-04, 5.674E-04, 5.682E-04, 5.692E-04,
     &   5.690E-04, 5.694E-04, 5.701E-04, 5.712E-04, 5.735E-04,
     &   5.774E-04, 5.824E-04, 5.908E-04, 6.027E-04, 6.230E-04,
     &   7.080E-04, 8.030E-04, 1.020E-03, 1.559E-03, 9.470E-04,
     &   1.380E-03, 9.520E-04, 7.760E-04, 6.930E-04, 6.110E-04,
     &   5.890E-04, 5.750E-04, 5.640E-04, 5.570E-04, 5.500E-04,
     &   5.456E-04, 5.426E-04, 5.401E-04, 5.376E-04, 5.355E-04,
     &   5.342E-04, 5.326E-04, 5.307E-04, 5.299E-04, 3.760E-04,
     &   3.700E-04, 3.670E-04, 3.640E-04, 3.600E-04, 3.580E-04,
     &   3.550E-04, 3.540E-04, 3.520E-04, 3.510E-04, 3.500E-04,
     &   3.480E-04, 3.470E-04, 3.470E-04, 3.460E-04, 3.450E-04,
     &   3.440E-04, 3.430E-04, 3.430E-04, 3.420E-04, 3.420E-04,
     &   3.420E-04, 3.410E-04, 3.400E-04, 3.400E-04, 3.400E-04,
     &   3.390E-04, 3.390E-04, 3.390E-04, 3.390E-04, 3.390E-04,
     &   3.390E-04, 3.380E-04, 3.380E-04, 3.370E-04, 3.370E-04,
     &   3.370E-04, 3.370E-04, 3.370E-04, 3.370E-04, 3.370E-04,
     &   3.370E-04, 3.360E-04, 3.360E-04, 3.360E-04, 3.360E-04,
     &   3.360E-04, 3.360E-04, 3.360E-04, 3.350E-04, 3.350E-04/

        DATA ((DAS(I,J), I=401,500), J=2,2) /
     &   3.350E-04, 3.360E-04, 3.350E-04, 3.350E-04, 3.360E-04,
     &   3.350E-04, 3.350E-04, 3.350E-04, 3.350E-04, 3.350E-04,
     &   3.350E-04, 3.350E-04, 3.350E-04, 3.340E-04, 3.350E-04,
     &   3.350E-04, 3.350E-04, 3.350E-04, 3.340E-04, 3.350E-04,
     &   3.340E-04, 3.350E-04, 3.340E-04, 3.350E-04, 3.350E-04,
     &   3.340E-04, 3.350E-04, 3.350E-04, 3.340E-04, 3.340E-04,
     &   3.340E-04, 3.340E-04, 3.330E-04, 3.330E-04, 3.340E-04,
     &   3.340E-04, 3.330E-04, 3.330E-04, 3.330E-04, 3.330E-04,
     &   3.330E-04, 3.330E-04, 3.330E-04, 3.330E-04, 3.330E-04,
     &   3.330E-04, 3.330E-04, 3.330E-04, 3.330E-04, 3.330E-04,
     &   3.330E-04, 3.330E-04, 3.330E-04, 3.330E-04, 3.330E-04,
     &   3.330E-04, 3.330E-04, 3.330E-04, 3.330E-04, 3.330E-04,
     &   3.320E-04, 3.330E-04, 3.330E-04, 3.330E-04, 3.330E-04,
     &   3.330E-04, 3.330E-04, 3.330E-04, 3.330E-04, 3.330E-04,
     &   3.330E-04, 3.320E-04, 3.330E-04, 3.330E-04, 3.330E-04,
     &   3.330E-04, 3.320E-04, 3.330E-04, 3.330E-04, 3.330E-04,
     &   3.330E-04, 3.330E-04, 3.330E-04, 3.330E-04, 3.330E-04,
     &   3.330E-04, 3.320E-04, 3.330E-04, 3.330E-04, 3.330E-04,
     &   3.330E-04, 3.330E-04, 3.330E-04, 3.320E-04, 3.330E-04,
     &   3.330E-04, 3.330E-04, 3.330E-04, 3.330E-04, 3.330E-04/

        DATA ((DAS(I,J), I=501,530), J=2,2) /
     &   3.330E-04, 3.330E-04, 3.330E-04, 3.320E-04, 3.330E-04,
     &   3.330E-04, 3.330E-04, 3.330E-04, 3.330E-04, 3.330E-04,
     &   3.330E-04, 3.330E-04, 3.330E-04, 3.330E-04, 3.330E-04,
     &   3.330E-04, 3.330E-04, 3.330E-04, 3.330E-04, 3.330E-04,
     &   3.330E-04, 3.330E-04, 3.330E-04, 3.330E-04, 3.330E-04,
     &   3.330E-04, 3.320E-04, 3.330E-04, 3.330E-04, 3.330E-04/
C      
      save
      RETURN
      END

      subroutine dalhad_timelike1
* hadronic contribution of the 5 light quark flavors evaluated using 
* e^+e^-data. CMD-2 2001 data have a normalization problem. I added a energy 
* independent +1.5 % correction and incresed the syst error by 0.5%. 
* Results thus are preliminary! Arrays:
* ESA(I)  : energy E in GeV; 
* DAS(I,1): Delta alpha^5(E**2) 
* DAS(I,2): error (systematics dominated, should be treated as a systematic error)
C
      IMPLICIT NONE
      INTEGER NC,I,J
      PARAMETER(NC=250)
      REAL EMA(NC),DAM(NC,2)
      COMMON /DATM/EMA,DAM
C 250
      DATA  (EMA(I), I=1,100) /
     &   1.600E+00, 1.700E+00, 1.800E+00, 1.900E+00, 2.000E+00,
     &   2.100E+00, 2.200E+00, 2.300E+00, 2.400E+00, 2.500E+00,
     &   2.600E+00, 2.620E+00, 2.640E+00, 2.660E+00, 2.680E+00,
     &   2.700E+00, 2.720E+00, 2.740E+00, 2.760E+00, 2.780E+00,
     &   2.800E+00, 2.820E+00, 2.840E+00, 2.860E+00, 2.880E+00,
     &   2.900E+00, 2.920E+00, 2.940E+00, 2.960E+00, 2.980E+00,
     &   3.000E+00, 3.020E+00, 3.040E+00, 3.060E+00, 3.080E+00,
     &   3.100E+00, 3.108E+00, 3.115E+00, 3.122E+00, 3.130E+00,
     &   3.138E+00, 3.145E+00, 3.152E+00, 3.160E+00, 3.168E+00,
     &   3.175E+00, 3.182E+00, 3.190E+00, 3.197E+00, 3.205E+00,
     &   3.213E+00, 3.220E+00, 3.227E+00, 3.235E+00, 3.243E+00,
     &   3.250E+00, 3.257E+00, 3.265E+00, 3.273E+00, 3.280E+00,
     &   3.287E+00, 3.295E+00, 3.303E+00, 3.310E+00, 3.318E+00,
     &   3.325E+00, 3.332E+00, 3.340E+00, 3.348E+00, 3.355E+00,
     &   3.362E+00, 3.370E+00, 3.378E+00, 3.385E+00, 3.392E+00,
     &   3.400E+00, 3.410E+00, 3.420E+00, 3.430E+00, 3.440E+00,
     &   3.450E+00, 3.460E+00, 3.470E+00, 3.480E+00, 3.490E+00,
     &   3.500E+00, 3.510E+00, 3.520E+00, 3.530E+00, 3.540E+00,
     &   3.550E+00, 3.560E+00, 3.570E+00, 3.580E+00, 3.590E+00,
     &   3.620E+00, 3.626E+00, 3.631E+00, 3.637E+00, 3.642E+00/

      DATA  (EMA(I), I=101,200) /
     &   3.648E+00, 3.654E+00, 3.659E+00, 3.665E+00, 3.670E+00,
     &   3.676E+00, 3.682E+00, 3.687E+00, 3.690E+00, 3.693E+00,
     &   3.695E+00, 3.698E+00, 3.699E+00, 3.700E+00, 3.704E+00,
     &   3.704E+00, 3.708E+00, 3.710E+00, 3.710E+00, 3.712E+00,
     &   3.715E+00, 3.716E+00, 3.720E+00, 3.720E+00, 3.721E+00,
     &   3.725E+00, 3.726E+00, 3.729E+00, 3.730E+00, 3.732E+00,
     &   3.733E+00, 3.737E+00, 3.740E+00, 3.742E+00, 3.743E+00,
     &   3.746E+00, 3.749E+00, 3.750E+00, 3.750E+00, 3.754E+00,
     &   3.754E+00, 3.759E+00, 3.760E+00, 3.763E+00, 3.766E+00,
     &   3.767E+00, 3.770E+00, 3.771E+00, 3.772E+00, 3.776E+00,
     &   3.777E+00, 3.780E+00, 3.782E+00, 3.788E+00, 3.790E+00,
     &   3.794E+00, 3.799E+00, 3.800E+00, 3.805E+00, 3.810E+00,
     &   3.810E+00, 3.816E+00, 3.820E+00, 3.822E+00, 3.827E+00,
     &   3.830E+00, 3.833E+00, 3.838E+00, 3.840E+00, 3.844E+00,
     &   3.850E+00, 3.850E+00, 3.855E+00, 3.860E+00, 3.861E+00,
     &   3.866E+00, 3.870E+00, 3.872E+00, 3.878E+00, 3.880E+00,
     &   3.883E+00, 3.889E+00, 3.890E+00, 3.894E+00, 3.900E+00,
     &   4.000E+00, 4.005E+00, 4.010E+00, 4.015E+00, 4.020E+00,
     &   4.025E+00, 4.030E+00, 4.035E+00, 4.040E+00, 4.045E+00,
     &   4.050E+00, 4.055E+00, 4.060E+00, 4.065E+00, 4.070E+00/

      DATA  (EMA(I), I=201,250) /
     &   4.075E+00, 4.080E+00, 4.085E+00, 4.090E+00, 4.095E+00,
     &   4.100E+00, 4.105E+00, 4.110E+00, 4.115E+00, 4.120E+00,
     &   4.125E+00, 4.130E+00, 4.135E+00, 4.140E+00, 4.145E+00,
     &   4.150E+00, 4.155E+00, 4.160E+00, 4.165E+00, 4.170E+00,
     &   4.175E+00, 4.180E+00, 4.185E+00, 4.190E+00, 4.195E+00,
     &   4.200E+00, 5.500E+00, 6.500E+00, 7.500E+00, 8.500E+00,
     &   1.050E+01, 1.100E+01, 1.150E+01, 1.350E+01, 1.362E+01,
     &   1.375E+01, 1.388E+01, 1.400E+01, 1.450E+01, 1.500E+01,
     &   1.550E+01, 1.600E+01, 1.650E+01, 1.700E+01, 1.750E+01,
     &   1.800E+01, 1.850E+01, 1.900E+01, 1.950E+01, 2.000E+01/

        DATA ((DAM(I,J), I=1,100), J=1,1) /
     &   4.012E-03, 4.261E-03, 4.427E-03, 4.415E-03, 4.318E-03,
     &   4.324E-03, 4.407E-03, 4.464E-03, 4.464E-03, 4.368E-03,
     &   4.156E-03, 4.090E-03, 4.013E-03, 3.930E-03, 3.818E-03,
     &   3.700E-03, 3.561E-03, 3.397E-03, 3.224E-03, 3.014E-03,
     &   2.788E-03, 2.519E-03, 2.202E-03, 1.840E-03, 1.398E-03,
     &   8.780E-04, 2.395E-04,-5.553E-04,-1.572E-03,-2.925E-03,
     &  -4.823E-03,-7.339E-03,-1.176E-02,-2.123E-02,-5.422E-02,
     &   3.444E-01, 1.010E-01, 6.094E-02, 4.450E-02, 3.562E-02,
     &   3.007E-02, 2.630E-02, 2.359E-02, 2.154E-02, 1.994E-02,
     &   1.867E-02, 1.763E-02, 1.678E-02, 1.605E-02, 1.535E-02,
     &   1.470E-02, 1.412E-02, 1.360E-02, 1.314E-02, 1.273E-02,
     &   1.235E-02, 1.200E-02, 1.169E-02, 1.140E-02, 1.113E-02,
     &   1.087E-02, 1.064E-02, 1.042E-02, 1.021E-02, 1.001E-02,
     &   9.825E-03, 9.647E-03, 9.473E-03, 9.307E-03, 9.147E-03,
     &   8.992E-03, 8.841E-03, 8.696E-03, 8.550E-03, 8.415E-03,
     &   8.269E-03, 8.090E-03, 7.911E-03, 7.730E-03, 7.538E-03,
     &   7.349E-03, 7.192E-03, 7.020E-03, 6.863E-03, 6.704E-03,
     &   6.543E-03, 6.503E-03, 6.464E-03, 6.426E-03, 6.398E-03,
     &   6.389E-03, 6.459E-03, 6.549E-03, 6.789E-03, 7.459E-03,
     &  -2.514E-03,-2.685E-03,-3.083E-03,-3.731E-03,-4.668E-03/

        DATA ((DAM(I,J), I=101,200), J=1,1) /
     &  -5.995E-03,-7.868E-03,-1.062E-02,-1.493E-02,-2.246E-02,
     &  -3.861E-02,-9.647E-02, 3.645E-01, 1.180E-01, 7.257E-02,
     &   5.660E-02, 4.302E-02, 4.072E-02, 3.891E-02, 3.246E-02,
     &   3.174E-02, 2.737E-02, 2.572E-02, 2.540E-02, 2.392E-02,
     &   2.196E-02, 2.140E-02, 1.968E-02, 1.947E-02, 1.935E-02,
     &   1.793E-02, 1.740E-02, 1.665E-02, 1.638E-02, 1.588E-02,
     &   1.557E-02, 1.462E-02, 1.410E-02, 1.376E-02, 1.349E-02,
     &   1.297E-02, 1.249E-02, 1.229E-02, 1.224E-02, 1.160E-02,
     &   1.159E-02, 1.110E-02, 1.101E-02, 1.101E-02, 1.133E-02,
     &   1.169E-02, 1.252E-02, 1.291E-02, 1.299E-02, 1.395E-02,
     &   1.407E-02, 1.417E-02, 1.406E-02, 1.359E-02, 1.341E-02,
     &   1.308E-02, 1.261E-02, 1.255E-02, 1.220E-02, 1.187E-02,
     &   1.185E-02, 1.154E-02, 1.135E-02, 1.128E-02, 1.104E-02,
     &   1.093E-02, 1.083E-02, 1.063E-02, 1.057E-02, 1.045E-02,
     &   1.028E-02, 1.028E-02, 1.013E-02, 1.001E-02, 9.988E-03,
     &   9.862E-03, 9.775E-03, 9.735E-03, 9.618E-03, 9.569E-03,
     &   9.503E-03, 9.402E-03, 9.379E-03, 9.295E-03, 9.199E-03,
     &   8.654E-03, 8.639E-03, 8.621E-03, 8.609E-03, 8.601E-03,
     &   8.603E-03, 8.609E-03, 8.623E-03, 8.630E-03, 8.648E-03,
     &   8.663E-03, 8.680E-03, 8.694E-03, 8.704E-03, 8.719E-03/

        DATA ((DAM(I,J), I=201,250), J=1,1) /
     &   8.737E-03, 8.747E-03, 8.758E-03, 8.767E-03, 8.772E-03,
     &   8.775E-03, 8.782E-03, 8.797E-03, 8.816E-03, 8.851E-03,
     &   8.889E-03, 8.928E-03, 8.956E-03, 8.980E-03, 8.991E-03,
     &   8.989E-03, 8.979E-03, 8.973E-03, 8.964E-03, 8.954E-03,
     &   8.944E-03, 8.944E-03, 8.942E-03, 8.943E-03, 8.941E-03,
     &   8.936E-03, 1.150E-02, 1.180E-02, 1.232E-02, 1.315E-02,
     &   1.413E-02, 1.550E-02, 1.572E-02, 1.622E-02, 1.628E-02,
     &   1.633E-02, 1.638E-02, 1.644E-02, 1.665E-02, 1.686E-02,
     &   1.706E-02, 1.725E-02, 1.744E-02, 1.763E-02, 1.780E-02,
     &   1.798E-02, 1.815E-02, 1.831E-02, 1.847E-02, 1.862E-02/

        DATA ((DAM(I,J), I=1,100), J=2,2) /
     &   4.220E-04, 4.386E-04, 4.473E-04, 4.434E-04, 4.609E-04,
     &   4.794E-04, 4.711E-04, 4.734E-04, 4.777E-04, 4.855E-04,
     &   4.966E-04, 4.965E-04, 4.980E-04, 5.005E-04, 5.039E-04,
     &   5.071E-04, 5.091E-04, 5.129E-04, 5.183E-04, 5.254E-04,
     &   5.341E-04, 5.406E-04, 5.500E-04, 5.627E-04, 5.792E-04,
     &   6.007E-04, 6.263E-04, 6.617E-04, 7.104E-04, 7.803E-04,
     &   8.869E-04, 1.358E-03, 2.081E-03, 3.264E-03, 6.100E-03,
     &   2.402E-02, 9.518E-03, 6.259E-03, 4.725E-03, 3.764E-03,
     &   3.077E-03, 2.549E-03, 2.123E-03, 1.773E-03, 1.483E-03,
     &   1.244E-03, 1.055E-03, 9.160E-04, 8.440E-04, 8.120E-04,
     &   7.730E-04, 7.390E-04, 7.120E-04, 6.870E-04, 6.660E-04,
     &   6.480E-04, 6.330E-04, 6.180E-04, 6.070E-04, 5.980E-04,
     &   5.910E-04, 5.850E-04, 5.793E-04, 5.741E-04, 5.705E-04,
     &   5.677E-04, 5.663E-04, 5.660E-04, 5.669E-04, 5.678E-04,
     &   5.693E-04, 5.715E-04, 5.746E-04, 5.783E-04, 5.831E-04,
     &   5.886E-04, 5.918E-04, 5.970E-04, 6.039E-04, 6.127E-04,
     &   6.234E-04, 6.345E-04, 6.473E-04, 6.621E-04, 6.793E-04,
     &   6.986E-04, 8.741E-04, 1.109E-03, 1.400E-03, 1.752E-03,
     &   2.180E-03, 2.711E-03, 3.401E-03, 4.384E-03, 6.076E-03,
     &   5.225E-03, 4.546E-03, 4.036E-03, 3.658E-03, 3.403E-03/

        DATA ((DAM(I,J), I=101,200), J=2,2) /
     &   3.281E-03, 3.323E-03, 3.594E-03, 4.217E-03, 5.497E-03,
     &   8.425E-03, 1.913E-02, 6.645E-02, 2.070E-02, 1.226E-02,
     &   9.289E-03, 6.767E-03, 6.340E-03, 6.007E-03, 4.826E-03,
     &   4.694E-03, 3.904E-03, 3.608E-03, 3.549E-03, 3.287E-03,
     &   2.941E-03, 2.844E-03, 2.549E-03, 2.514E-03, 2.494E-03,
     &   2.259E-03, 2.174E-03, 2.056E-03, 2.014E-03, 1.938E-03,
     &   1.893E-03, 1.760E-03, 1.694E-03, 1.651E-03, 1.619E-03,
     &   1.562E-03, 1.512E-03, 1.493E-03, 1.489E-03, 1.431E-03,
     &   1.429E-03, 1.374E-03, 1.356E-03, 1.303E-03, 1.246E-03,
     &   1.209E-03, 1.163E-03, 1.153E-03, 1.153E-03, 1.159E-03,
     &   1.160E-03, 1.153E-03, 1.136E-03, 1.086E-03, 1.067E-03,
     &   1.034E-03, 9.880E-04, 9.830E-04, 9.510E-04, 9.200E-04,
     &   9.170E-04, 8.900E-04, 8.720E-04, 8.660E-04, 8.450E-04,
     &   8.350E-04, 8.260E-04, 8.149E-04, 8.126E-04, 8.076E-04,
     &   8.022E-04, 8.018E-04, 7.979E-04, 7.968E-04, 7.964E-04,
     &   7.968E-04, 7.976E-04, 7.984E-04, 8.019E-04, 8.039E-04,
     &   8.069E-04, 8.138E-04, 8.154E-04, 8.219E-04, 8.320E-04,
     &   1.203E-03, 1.243E-03, 1.275E-03, 1.290E-03, 1.275E-03,
     &   1.212E-03, 1.094E-03, 9.506E-04, 8.793E-04, 9.563E-04,
     &   1.113E-03, 1.252E-03, 1.338E-03, 1.378E-03, 1.388E-03/

        DATA ((DAM(I,J), I=201,250), J=2,2) /
     &   1.383E-03, 1.372E-03, 1.361E-03, 1.352E-03, 1.346E-03,
     &   1.344E-03, 1.345E-03, 1.346E-03, 1.346E-03, 1.341E-03,
     &   1.326E-03, 1.297E-03, 1.247E-03, 1.172E-03, 1.074E-03,
     &   9.652E-04, 8.729E-04, 8.347E-04, 8.685E-04, 9.535E-04,
     &   1.053E-03, 1.140E-03, 1.204E-03, 1.244E-03, 1.264E-03,
     &   1.267E-03, 1.066E-03, 1.222E-03, 1.546E-03, 1.694E-03,
     &   1.555E-03, 1.751E-03, 1.066E-03, 4.650E-04, 4.610E-04,
     &   4.570E-04, 4.530E-04, 4.480E-04, 4.360E-04, 4.250E-04,
     &   4.170E-04, 4.090E-04, 4.030E-04, 3.970E-04, 3.930E-04,
     &   3.890E-04, 3.850E-04, 3.810E-04, 3.780E-04, 3.760E-04/
C      
      save
      RETURN
      END
