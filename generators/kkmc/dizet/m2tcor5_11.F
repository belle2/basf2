      SUBROUTINE GDEGNL
     & (GMU,MZ,MT,AMH,MW,PI3QF,SMAN,DRDREM,DRHOD,DKDREM,DROREM)      
c
c       This file contains the 2 loop O(g^4 mt^2/mw^2) corrections 
c       to various ew parameters.
c       As a general rule the routines with names "something" contain
c       the various corrections in the MSbar framework. The routines
c       with names "somethingOS" contain the additional corrections 
c       coming from the expansion of the  OS sine in the one-loop result.
c       To get an OS result one should put together the routines
c       something + somethingOS with coupling constants in both routines
c       expressed in terms of OS quantities.
c       In OS framework in units
c       Nc*(alfa/(4.d0*pi*s2))**2.d0*(mt^2/(4.d0 mw^2))**2.d0
c       one has :
c
c       Delta r ^(2l) = 16.d0*drs2lew + 4.d0*zt*c2*drs2lewOS
c                      - 16.d0*deleoe2lew
c                      - c2/s2*(tobf2lew + 4.d0*zt*c2*tobf2lewOS)
c
c       sineff = ( 1+ delta ) s2
c       delta^(2l)    = k2lew -4.d0*c2*c2*zt*k2lewOS
c                       + c2/s2*(tobf2lew + 4.d0*zt*c2*tobf2lewOS)
c        mz = Z mass
c        mu = 't Hooft mass
c        mt = top mass
c        mh = Higgs mass
c        zt = (mz/mt)**2
c        ht = (mh/mt)**2
c        s2 = sin^2 theta_w
c        c2 = cos^2 theta_w
c        sman = s (Mandelstein variable)
c        ddilog = double precision dilogarithmic function
c        Cl2 = Clausen integral of second order
c
      IMPLICIT REAL*8(A-Z)
      COMMON/CDZCON/PI,PI2,F1,D3,ALFAI,AL4PI,AL2PI,AL1PI
c
c     common/datcom/ needed for Degrassi package
        double precision gmud,repiggh0,mz0,alfa0,alfastr,delQCD,
     1                pide,mel,mmu,mtau,mb,gamz,Nc,zeta3
c
        common/datcom/gmud,repiggh0,mz0,alfa0,alfastr,delQCD,
     1                pide,mel,mmu,mtau,mb,gamz,Nc,zeta3
c
      pide=pi
      zeta3=D3
c
c     color factor
      Nc = 3.d0
      alfa0=1d0/ALFAI
*
* protection by shifting of Higgs mass if it is = 2*mt or 2*mz
*
      MH=AMH
      if(ABS(AMH-2d0*MT).LT.5d-5) MH=2d0*MT-1d-4
      if(ABS(AMH-2d0*MZ).LT.5d-5) MH=2d0*MZ-1d-4
*
      c2=MW**2/mz**2
      s2=1d0-c2
      mu=mz
c      sman=mz**2
      zt = (mz/mt)**2
      ht = (mh/mt)**2
      xt = gmu/sqrt(2d0)*mt**2/8d0/pi**2
c
      call ew2ltwodel(zt,ht,deleoe2lew,s2,mz,mu)
c
      call ew2ldeltarw(zt,ht,drs2lew,s2,mz,mu)
      call ew2ldeltarwOS(zt,drs2lewOS,s2,mz,mu)
c
      call ew2ltobf(zt,ht,tobf2lew,s2,mz,mu)
      call ew2ltobfOS(zt,tobf2lewOS,s2,mz,mu)
c
      call kappacur2l(zt,ht,k2lew,c2,mz,mu)
      call kappacur2lOS(zt,k2lewOS,c2,mz,mu,sman,pi3qf)
c
      call ew2leta(zt,ht,eta2lew,c2,mz,mu)
      call ew2letaOS(zt,ht,eta2lewOS,s2,mz,mu,sman,pi3qf)
c
      DRHOD =Nc*xt**2*(tobf2lew+4.d0*zt*c2*tobf2lewOS)
      DKHOD =Nc*xt**2*(tobf2lew+4.d0*zt*c2*tobf2lewOS)
      DRDREM=
     &+Nc*(alfa0/(4.d0*pi*s2))**2*(mt**2/(4.d0*mw**2))**2
     &*(16.d0*drs2lew+4.d0*zt*c2*drs2lewOS-16.d0*deleoe2lew)
      DKDREM=Nc*xt**2*(k2lew-4.d0*c2*c2*zt*k2lewOS)
c
c Dear Dima, here are the routines that contain the O(g^4 mt^2/mw^2) 
c corrections to the factor eta relevant for the widths, but the b. 
c The notation is identical to that of the previuos routines. Notice that
c the you have already the routines containing the O(g^4 mt^2/mw^2)
c corrections to the overall prefactor. In particular the corrections to
c f^prime bar are contained in the subroutines ew2ldeltarw and
c ew2ldeltarwOS. All best Giuseppe
c 
      eta2l = 4.d0*eta2lew +eta2lewOS - 8.d0*c2*log(c2)
c   
*      DETAF =Nc*(alfa0/(4.d0*pi*s2))**2*(mt**2/(4.d0*mw**2))*eta2l
      DROREM=Nc*xt**2
     &      *(4.d0*zt*c2*eta2l-(16.d0*drs2lew+4.d0*zt*c2*drs2lewOS))
c       
      return
      end
c
c*********************** FUNCTIONS ****************************
c
c       function J1 in Bertolini-Sirlin

        double precision function j1(x,y)
        double precision x,y,d,ln,j
        d=4.d0*x*y-1.d0
        j=dlog(x)+1.d0/3.d0
        j=j+(3.d0+d)*(ln(d)-1.d0)
        j1=j/6.d0
        return
        end
c
c       function lambda present in J1 and J2
        double precision function ln(d)
        double precision d,d1
        if (d.lt.0.d0) go to 10
        ln=dsqrt(d)*datan(1.d0/dsqrt(d))
        go to 20
10      continue
        d1=dabs(d)
        d1=dsqrt(d1)
        ln=0.5d0*d1*dlog(dabs((1.d0+d1)/(1.d0-d1)))
20      continue
        return
        end
c
c-----------------------------------------------------------------
c	The below functions U and V are related to the functions
c	f(y) and g(y) in the Degrassi-Sirlin paper as follows:
c	V(x)=f(y)
c	U(x)+0.25 = -2*g(y)
c	where y=1/x
c----------------------------------------------------------------
c
c       function V present in the vertices (real part)
        double precision function revf(x)
        double precision  a1,a2,x,ddilog
c
        double precision gmu,repiggh0,mz0,alfa0,alfastr,delQCD,
     1                pi,mel,mmu,mtau,mb,gamz,Nc,zeta3
c
        common/datcom/gmu,repiggh0,mz0,alfa0,alfastr,delQCD,
     1                pi,mel,mmu,mtau,mb,gamz,Nc,zeta3
c
        a1=2.d0*(2.d0+x)-0.5d0+(3.d0+2.d0*x)*dlog(x)
        a2=dlog(1.d0+1.d0/x)*dlog(1.d0+1.d0/x)
     1  -(pi*pi)/3.d0
        a2=a2+2.d0*ddilog(x/(x+1.d0))
        a2=(1.d0+x)*(1.d0+x)*a2
        revf=a1+a2
        return
        end
c
c       Function U present in the vertices 
        double precision function ug(x)
        double precision a1,x,dat
*
* here the code fails for x < 1/4 or above 'a' threshold
* correction is proposed
* old:  
c       a1=dsqrt(4.d0*x-1.d0)
c       a2=(1.d0+2.d0*x)*a1*datan(1.d0/a1)
c       a2=-1.5d0+x-a2
c       a2=4.d0*x*(2.d0+x)*datan(1.d0/a1)*datan(1.d0/a1)+a2
c       ug=a2
* new:
        a1=4.d0*x-1.d0
        if(a1.le.0.d0) then
         dat=.5d0*dlog(dabs((1+dsqrt(dabs(a1)))
     1                     /(1-dsqrt(dabs(a1)))))
        elseif(a1.gt.0.d0) then
         dat=datan(1.d0/dsqrt(a1))
        endif
        ug=4.d0*x*(2.d0+x)*dat*dat-1.5d0+x
     1    -(1.d0+2.d0*x)*dsqrt(dabs(a1))*dat
        return
        end
c
c       function phi
        real*8 function phi(x)
        real*8 x,Cl2,ddilog
c
        double precision gmu,repiggh0,mz0,alfa0,alfastr,delQCD,
     1                pi,mel,mmu,mtau,mb,gamz,Nc,zeta3
c
        common/datcom/gmu,repiggh0,mz0,alfa0,alfastr,delQCD,
     1                pi,mel,mmu,mtau,mb,gamz,Nc,zeta3
c
        if(x.lt.1.d0) then
          phi =  4.d0*dsqrt(x/(1.d0-x))*Cl2(2.d0*dasin(dsqrt(x)))
        else
          phi= dsqrt(x/(x-1.d0))*
     1        (-4.d0*ddilog((1.d0-dsqrt(1.d0-1.d0/x))/2.d0) +
     2        2.d0*dlog((1.d0-dsqrt(1.d0-1.d0/x))/2.d0)**2.d0 -
     3        dlog(4.d0*x)**2.d0+pi*pi/3.d0)
        endif
        return
        end
c
c       function g(x) --> gdeg(x)
        real*8 function gdeg(x)
        real*8 x
c
        double precision gmu,repiggh0,mz0,alfa0,alfastr,delQCD,
     1                pi,mel,mmu,mtau,mb,gamz,Nc,zeta3
c
        common/datcom/gmu,repiggh0,mz0,alfa0,alfastr,delQCD,
     1                pi,mel,mmu,mtau,mb,gamz,Nc,zeta3
        if(x.gt.4.d0) then
        gdeg=2.d0*dsqrt(x/4.d0 -1.d0)*dlog((1.d0-dsqrt(1.d0-4.d0/x))/
     1        (1.d0 + dsqrt(1.d0 -4.d0/x)))
        else
        gdeg=dsqrt(4.d0-x)*(pi - 2.d0*dasin(dsqrt(x/4.d0))) 
        endif
        return
        end
c
c       function B0(q,m1,m2,mu2)); it is the - I_3 of DS once the pole
c       1/eps has been extracted. q is the momentum squared, m1 and m2
c       the mass squared and mu2 the squared of the `t Hooft mass
        real*8 function B0(q,m1,m2,mu2)
        real*8 q,m1,m2,Omega,mu2
        B0 = -( dlog(q/mu2)-2.d0 + 
     1         1.d0/2.d0*( 1.d0 + (m1/q-m2/q))*dlog(m1/q) +
     2         1.d0/2.d0*( 1.d0 - (m1/q-m2/q))*dlog(m2/q) +
     3         2.d0*Omega(m1/q,m2/q))
        return
        end

c       function Omega(a,b) contained in B0
        real*8 function Omega(a,b)
        real*8 a,b,cbig
        Cbig = (a+b)/2.d0 - (a-b)**2.d0/4.d0 -1.d0/4.d0
        if(Cbig.gt.0.d0) then
            Omega = dsqrt(Cbig)*
     1          (datan((1.d0 + a - b)/(2.d0*dsqrt(Cbig))) +
     2           datan((1.d0 - a + b)/(2.d0*dsqrt(Cbig))) )
        else
            Cbig = - Cbig
            Omega = 1.d0/2.d0*dsqrt(Cbig)*
     1              dlog((a/2.d0 +b/2.d0 -1.d0/2.d0 -dsqrt(Cbig))/
     2              (a/2.d0 + b/2.d0 -1.d0/2.d0 + dsqrt(Cbig)))
                  
        endif
        return
        end
c
      DOUBLE PRECISION FUNCTION CL2(THETA)
C     ************************************
C CALCULATES CLAUSEN'S INTEGRAL OF SECOND ORDER FOR REAL ARGUMENTS
C THETA.
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
      PARAMETER (N=30,K=20,DM=1.D-15,PM=1.D-35,VM=1.D35)
      DIMENSION B(N)
      B(1)=.1D1/.6D1
      B(2)=.1D1/.30D2
      B(3)=.1D1/.42D2
      B(4)=.1D1/.30D2
      B(5)=.5D1/.66D2
      B(6)=.691D3/.2730D4
      B(7)=.7D1/.6D1
      B(8)=.3617D4/.510D3
      B(9)=.43867D5/.798D3
      B(10)=.174611D6/.330D3
      B(11)=.854513D6/.138D3
      B(12)=.236364091D9/.2730D4
      B(13)=.8553103D7/.6D1
      B(14)=.23749461029D11/.870D3
      B(15)=.8615841276005D13/.14322D5
      B(16)=.7709321041217D13/.510D3
      B(17)=.2577687858367D13/.6D1
      B(18)=.26315271553053477373D20/.1919190D7
      B(19)=.2929993913841559D16/.6D1
      B(20)=.261082718496449122051D21/.13530D5
      B(21)=.1520097643918070802691D22/.1806D4
      B(22)=.27833269579301024235023D23/.690D3
      B(23)=.596451111593912163277961D24/.282D3
      B(24)=.5609403368997817686249127547D28/.46410D5
      B(25)=.495057205241079648212477525D27/.66D2
      B(26)=.801165718135489957347924991853D30/.1590D4
      B(27)=.29149963634884862421418123812691D32/.798D3
      B(28)=.2479392929313226753685415739663229D34/.870D3
      B(29)=.84483613348880041862046775994036021D35/.354D3
      B(30)=.1215233140483755572040304994079820246041491D35/.56786730D0
      PI=.4D1*DATAN(.1D1)
      X=DMOD(THETA,.2D1*PI)
      IF (X.GT.PI) X=X-.2D1*PI
      IF (X.LE.-PI) X=X+.2D1*PI
      IF (X.EQ.0.D0) THEN
      CL2=0.D0
      ELSE
      H=.1D1-DLOG(DABS(X))
      XS=X**2
      P=XS
      V=.2D1
      DO 1 I=1,K
      D=B(I)/(DFLOAT(2*I*(2*I+1))*V)*P
      H=H+D
      IF (D.LT.DM.OR.P.LT.PM.OR.V.GT.VM) GOTO 2
      P=P*XS
1     V=V*DFLOAT((2*I+1)*(2*I+2))
2     CL2=X*H
      END IF
      RETURN
C***********************************************************************
      END
c
c***************   ROUTINES ********************************************
c
c       This subroutine contains the 2 loop expressions of
c       [Aww (mw^2) - Aww(0)]/mw^2 and the vertex contribution
c       in units
c       Nc*(alfa0/(4.d0*pi*s2))**2.d0/(zt*c2)**2.d0
c       OUTPUT = drs2lew
c     
        subroutine ew2ldeltarw(zt,ht,drs2lew,s2,mz,mu)
        real*8 zt,ht,drs2lew,s2,c2,mz,ln,vertex,aww,mz2,B0,gdeg,phi,
     1         ddilog,mu,mu2
c
        double precision gmu,repiggh0,mz0,alfa0,alfastr,delQCD,
     1                pi,mel,mmu,mtau,mb,gamz,Nc,zeta3
c
        common/datcom/gmu,repiggh0,mz0,alfa0,alfastr,delQCD,
     1                pi,mel,mmu,mtau,mb,gamz,Nc,zeta3
        c2 = 1.d0 - s2  
        mz2 = mz*mz
        mu2 = mu*mu
        if(dsqrt(ht).lt.0.3d0) then
c       LIGHT HIGGS CASE
c
c       [Aww (mw^2) - Aww(0)]/mw^2
c
        aww = 
     1  35.d0*ht/288.d0-5.d0*ht*ht/(144.d0*c2*zt)- 41.d0*zt/96.d0   -
     2  zt/(48.d0*c2) +325.d0*c2*zt/144.d0 - c2*dsqrt(ht)*pi*zt/3.d0- 
     3  c2*pi*pi*zt/36.d0+B0(c2*mz2, ht*mz2/zt,c2*mz2,mu2)*
     4  (-5.d0*ht/36.d0+5.d0*ht*ht/(144.d0*c2*zt)+5.d0*c2*zt/12.d0) +
     5  B0(c2*mz2, c2*mz2, mz2,mu2)*
     6  (5.d0*zt/12.d0 + zt/(48.d0*c2) - c2*zt/2.d0)  +
     9  (5.d0*(-1.d0)*ht/144.d0-19.d0*zt/12.d0-(425.d0*c2*zt/144.d0)+
     $  25.d0*zt/(16.d0*s2)-5.d0*c2*c2*zt*zt/(16.d0*(ht-c2*zt)))*
     1  dlog(c2) + zt*dlog(c2)**2.d0/2.d0 + c2*zt*dlog(c2)**2.d0/2.d0 + 
     2  zt*dlog(c2)**2.d0/(2.d0*(-s2)) -5.d0*ht*dlog(ht)/48.d0 + 
     3  5.d0*ht*ht*dlog(ht)/(144.d0*c2*zt) +5.d0*c2*zt*dlog(ht)/16.d0 +
     4  5.d0*c2*c2*zt**2.d0*dlog(ht)/(16.d0*(ht - c2*zt)) +
     5  (-5.d0*ht/36.d0 + 5.d0*ht*ht/(144.d0*c2*zt) + 5.d0*zt/12.d0 +
     6  zt/(48.d0*c2) -7.d0*c2*zt/3.d0 + 2.d0*zt*dlog(c2) + 
     7  2.d0*c2*zt*dlog(c2) - 2.d0*zt*dlog(c2)/s2)*(dlog(1.d0/zt) +
     $                     dlog(mz2/mu2) ) + 
     8  (-5.d0*ht/144.d0 +5.d0*zt/12.d0 + zt/(48.d0*c2) - 
     9  209.d0*c2*zt/144.d0 -5.d0*c2*c2*zt*zt/(16.d0*(ht - c2*zt)) +
     $  zt*dlog(c2) + c2*zt*dlog(c2) +zt*dlog(c2)/(-s2))*dlog(zt)
c
        else
c
c       HEAVY HIGGS CASE
c
        aww =
     1  c2*(4.d0 - ht)*zt*ln(-1.d0 + 4.d0/ht)/(12.d0*ht) +
     2  c2*(19.d0+21.d0*ht-12.d0*ht*ht-31.d0*ht**3.d0+9.d0*ht**4.d0)*zt*
     3  ddilog(1.d0 - ht)/(72.d0*ht*ht) +
     4  ((1.d0 + 69.d0*c2 - 145.d0*c2*c2)*zt*dlog(c2))/(48.d0*(-s2)) +
     5  (zt*(-228.d0*c2*c2*ht + 228.d0*c2**3.d0*ht + 18.d0*ht*ht + 
     6  351.d0*c2*ht*ht- 2226.d0*c2*c2*ht*ht +1857.d0*c2**3.d0*ht*ht-
     7  36.d0*c2*c2*ht**3.d0+36.d0*c2**3.d0*ht**3.d0+38.d0*c2*c2*pi*pi -
     8  38.d0*c2**3.d0*pi*pi+42.d0*c2*c2*ht*pi*pi-42.d0*c2**3.d0*ht*pi*
     9  pi + B0(c2*mz2, c2*mz2, mz2,mu2)*(-18.d0*ht*ht-342.d0*c2*ht*ht +
     $  792.d0*c2*c2*ht*ht - 432.d0*c2**3.d0*ht*ht) +
     3  gdeg(ht)
     3  *(144.d0*c2*c2*s2*ht**(2.5d0)-36.d0*c2*c2*s2*ht**(3.5d0))+     
     4  432.d0*c2**3.d0*ht*ht*dlog(c2)**2.d0))/(864.d0*c2*(-s2)*ht*ht)+
     5  (c2*(-31.d0+2.d0*ht-9.d0*ht*ht+3.d0*ht**3.d0)*zt*dlog(ht))/
     6  (72.d0*ht) +((1.d0 + 20.d0*c2 - 132.d0*c2*c2)*zt/(48.d0*c2) + 
     7  (2.d0*c2*c2*zt*dlog(c2))/(-s2))*(dlog(1.d0/zt) +dlog(mz2/mu2))+ 
     8  (zt*(-1.d0-19.d0*c2+93.d0*c2*c2-73.d0*c2**3.d0+48.d0*c2**3.d0*
     9  dlog(c2))*dlog(zt))/(48.d0*c2*(-s2)) + (c2*(-24.d0 + 74.d0*ht + 
     $  35.d0*ht*ht-49.d0*ht**3.d0+9.d0*ht**4.d0)*zt*phi(ht/4.d0))/
     1  (144.d0*ht*ht)
        endif
c
c       vertex contribution
c
        vertex =
     1  zt*( (-2.d0*c2 + 5.d0*c2*c2)*dlog(c2)/(2.d0*(-s2)) +
     2  (3.d0*c2-3.d0*c2*c2-c2*c2*dlog(c2)**2.d0)/(2.d0*(-s2))+
     3  (-c2 + c2*c2 - c2*c2*dlog(c2))/(-s2)*(dlog(1.d0/zt) +
     4      2.d0*dlog(mz2/mu2)) )

        drs2lew = aww + vertex
        return
        end
c
c       This subroutine contains the two loop contribution to
c       the electric charge in units
c       Nc*(alfa0/(4.d0*pi*s2))**2.d0/(zt*c2)**2.d0
c       OUTPUT = deleoe2lew       
c
        subroutine ew2ltwodel(zt,ht,deleoe2lew,s2,mz,mu)
        real*8 zt,ht,deleoe2lew,s2,c2,mz,phi,gdeg,mu,mz2,mu2
c     
        double precision gmu,repiggh0,mz0,alfa0,alfastr,delQCD,
     1                pi,mel,mmu,mtau,mb,gamz,Nc,zeta3
c
        common/datcom/gmu,repiggh0,mz0,alfa0,alfastr,delQCD,
     1                pi,mel,mmu,mtau,mb,gamz,Nc,zeta3
        c2 = 1.d0 - s2  
        mz2 = mz*mz
        mu2= mu*mu
        if(dsqrt(ht).lt.0.3d0) then
c       LIGHT HIGGS CASE
         deleoe2lew =
     1   -s2*c2*zt*( (2.d0-dlog(ht))/9.d0+(100.d0+256.d0*(-4.d0 + 2.d0*
     2   dsqrt(ht)*pi)+96.d0*dlog(ht))/864.d0 +
     3    13.d0/18.d0*(dlog(1.d0/zt)+ dlog(mz2/mu2)) )
c
c
        else
c
c       HEAVY HIGGS CASE
c
        deleoe2lew =
     $  -s2*c2*zt*((100.d0-25.d0*ht+96.d0*dlog(ht)+48.d0*ht*dlog(ht) +
     1  256.d0*(-4.d0+ht/2.d0-(-4.d0 + ht)*dsqrt(ht)*gdeg(ht)/4.d0 +
     2  (6.d0 - ht)*ht*dlog(ht)/4.d0)-64.d0*ht*(-4.d0 + ht/2.d0 - 
     3  (-4.d0+ht)*dsqrt(ht)*gdeg(ht)/4.d0+(6.d0-ht)*ht*dlog(ht)/4.d0))/
     4  (216.d0*(4.d0-ht))+13.d0/18.d0*(dlog(1.d0/zt)+dlog(mz2/mu2))+
     5  4.d0*(-1.d0 + ht)*phi(ht/4.d0)/(9.d0*(-4.d0 + ht)*ht))
c 
        endif
c
        return
        end

c       This subroutine computes  the 2 loop expressions of
c       [Aww (mw^2)/mw^2 - Azz(mz^2)/mw^2] 
c       in units 
c       Nc*(alfa0/(4.d0*pi*s2))**2/(4.d0*zt*c2)**2.d0
c       OUTPUT = tobf2lew
c
        subroutine ew2ltobf(zt,ht,tobf2lew,s2,mz,mu)
        real*8 zt,ht,tobf2lew,s2,c2,mz,mz2,gdeg,ddilog,b0,ln,mu,mu2,phi,
     1         sht,mt
c
        double precision gmu,repiggh0,mz0,alfa0,alfastr,delQCD,
     1                pi,mel,mmu,mtau,mb,gamz,Nc,zeta3
c
        common/datcom/gmu,repiggh0,mz0,alfa0,alfastr,delQCD,
     1                pi,mel,mmu,mtau,mb,gamz,Nc,zeta3
c
        c2 = 1.d0 - s2  
        mz2 = mz*mz
        mu2 = mu*mu
        mt = mz/Dsqrt(zt)
        sht = Dsqrt(ht)
c
        lew=0
        if(lew.eq.1) then
c       BARBIERI result
c
        tobf2lew =
     $  25.d0 - 4.d0*ht + pi*pi*(0.5d0 -1.d0/ht) +
     3  (-4.d0 + ht)*dsqrt(ht)*gdeg(ht)/2.d0 +
     $  (-6.d0 - 6.d0*ht + ht*ht/2.d0)*dlog(ht) +
     4  (54.d0/ht-135.d0+108.d0*ht-27.d0*ht*ht)*ddilog(1.d0-ht)/(9.d0) 
     5  + 1.5d0*(-10.d0 + 6.d0*ht -ht*ht)*phi(ht/4.d0)
        return
        endif
c
c       Our result
        if(ht.lt.0.25d0) then
c       LIGHT HIGGS CASE
c
        tobf2lew =
     1  19.d0 - (53.d0*ht)/3.d0 - 4.d0*dsqrt(ht)*pi - 2.d0*pi*pi    + 
     2  2.d0*ht*pi*pi + 8.d0*ht*ht/(9.d0*zt) -5.d0*ht*ht/(9.d0*c2*zt)+
     3  3.d0/2.d0*ht**(3.d0/2.d0)*pi + B0(mz2, ht*mz2/zt, mz2,mu2)*
     4  (32.d0*ht/9.d0 - 8.d0*ht*ht/(9.d0*zt)) + 
     5  B0(c2*mz2, ht*mz2/zt, c2*mz2,mu2)*(-20.d0*ht/9.d0+
     6  5.d0*ht*ht/(9.d0*c2*zt))-5.d0*ht*dlog(c2)/9.d0 -
     7  2.d0*ht*dlog(ht) - 8.d0*ht*ht*dlog(ht)/(9.d0*zt) + 
     8  5.d0*ht*ht*dlog(ht)/(9.d0*c2*zt)+ 
     9  ht*(5.d0*ht-8.d0*c2*ht+12.d0*c2*zt)*(dlog(1.d0/zt) +
     $      Dlog(mz2/mu2))/(9.d0*c2*zt)+ 
     $  ht*dlog(zt)/3.d0 +  zt*((-9.d0 + 845.d0*c2 + 427.d0*c2*c2 - 
     1  366.d0*c2**3.d0+136.d0*c2*dsqrt(ht)*pi- 
     2  464.d0*c2*c2*dsqrt(ht)*pi+ 256.d0*c2**3.d0*dsqrt(ht)*pi - 
     3  119.d0*c2*pi*pi + 44.d0*c2*c2*pi*pi - 
     4  396.d0*c2*B0(mz2, ht*mz2/zt, mz2, mu2) +
     5  B0(mz2, c2*mz2, c2*mz2, mu2)*(-18.d0*c2 -324.d0*c2*c2 +
     6  288.d0*c2**3.d0)+180.d0*c2*c2*B0(c2*mz2,ht*mz2/zt,c2*mz2,mu2) +
     7  B0(c2*mz2, c2*mz2, mz2, mu2)*(9.d0+180.d0*c2 -
     8  216.d0*c2*c2))/(27.d0*c2) + (-3.d0-32.d0*c2-48.d0*c2*c2)*
     9  dlog(c2)/9.d0 + (3.d0-10.d0*c2-328.d0*c2*c2 + 56.d0*c2**3.d0)*
     $  (dlog(1.d0/zt)+dlog(mz2/mu2))/(9.d0*c2)+(3.d0-11.d0*c2 -
     $  16.d0*c2*c2 - 48.d0*c2**3.d0)*dlog(zt)/(9.d0*c2)) +
     2  4.d0*zt*B0(mz2, ht*mz2/zt, mz2,mu2)
c
        else
c
        if(ht.lt.4.d0) then
c
c
c       INTERPOLATION
c
        tobf2lew =
     1  -15.642064d0 + 0.036381841d0*mt + dsqrt(sht)*(2.30111d0- 
     1   0.013429d0*mt) + sht*(0.0180877d0*mt -9.95272d0) + 
     2   ht*(5.68703d0 - 0.0156807d0*mt) +
     3  ht*sht*(0.00536879d0*mt - 1.64687d0) +ht*ht*(0.185188d0 -
     4  0.000646014*mt) +
     5  8.d0/9.d0*zt*(4.d0 - 26.d0*c2-5.d0*c2*c2)*dlog(mz2/mu2)

        else

c       HEAVY HIGGS CASE
c
        tobf2lew =
     $  (10800.d0*c2*ht**3.d0-4428.d0*c2*ht**4.d0+432.d0*c2*ht**5.d0 -
     1  432.d0*c2*ht**2.d0*pi**2.d0 + 324.d0*c2*ht**3.d0*pi*pi - 
     2  54.d0*c2*ht**4.d0*pi*pi)/(108.d0*c2*(4.d0 - ht)*ht**3.d0) +
     3  ((-4.d0 + ht)*dsqrt(ht)*gdeg(ht))/2.d0 +
     4  ((54.d0*ht*ht-135.d0*ht**3.d0+108.d0*ht**4.d0-27.d0*ht**5.d0)*
     5  ddilog(1.d0 - ht))/(9.d0*ht**3.d0) + ((-5184.d0*ht**3.d0 - 
     6  2592.d0*ht**4.d0+2700*ht**5.d0-540*ht**6.d0 + 27.d0*ht**7.d0)*
     7  dlog(ht))/(54.d0*(-4.d0 + ht)**2.d0*ht**3.d0) + 
     8  (-12960*ht**2.d0 + 14256.d0*ht**3.d0 - 5994.d0*ht**4.d0 +
     9  1134.d0*ht**5.d0-81.d0*ht**6.d0)*phi(ht/4.d0)/
     $  (54.d0*(-4.d0 + ht)**2.d0*ht*ht) +
     1  zt*((3552.d0*c2*c2*ht*ht-144.d0*ht**3.d0+10772.d0*c2*ht**3.d0+
     2  6112.d0*c2*c2*ht**3.d0-7392.d0*c2**3.d0*ht**3.d0+36.d0*ht**4.d0-
     3  2134.d0*c2*ht**4.d0 - 2742.d0*c2*c2*ht**4.d0-136.d0*c2*ht**5.d0+
     4  2872.d0*c2**3.d0*ht**4.d0+248.d0*c2*c2*ht**5.d0 - 
     5  256.d0*c2**3.d0*ht**5.d0 - 592.d0*c2*c2*ht*pi*pi +
     6  244.d0*c2*c2*ht**2.d0*pi*pi - 1904.d0*c2*ht**3.d0*pi*pi +
     7  872.d0*c2*c2*ht**3.d0*pi*pi + 476.d0*c2*ht**4.d0*pi*pi - 
     8  224.d0*c2*c2*ht**4.d0*pi*pi)/(108.d0*c2*(4.d0 - ht)*ht**3.d0) -
     9  2.d0*(1.d0+18.d0*c2-16.d0*c2*c2)*B0(mz2,c2*mz2,c2*mz2,mu2)/3.d0+
     $  (1.d0+20.d0*c2-24.d0*c2*c2)*B0(c2*mz2,c2*mz2,mz2,mu2)/(3.d0*c2)+
     1  (-34.d0+116.d0*c2-64.d0*c2*c2)*(-4.d0+ht)*dsqrt(ht)*gdeg(ht)/54+
     2  40*(-s2)*(4.d0 - ht)*Ln(-1.d0 + 4.d0/ht)/(3.d0*ht) +
     3  (74.d0*c2*ht-12.d0*c2*ht*ht-24.d0*c2*ht**3.d0-44.d0*c2*ht**4.d0+
     4  18.d0*c2*ht**5.d0)*ddilog(1.d0 - ht)/(9.d0*ht**3.d0) +
     5  (-3.d0-42.d0*c2-48.d0*c2*c2)*dlog(c2)/(9.d0) +
     6  (23040.d0*ht*ht-30144.d0*c2*ht*ht +
     7  ht**3.d0*(-10884.d0 + 17856.d0*c2 +  1536.d0*c2*c2) + 
     8  ht**4.d0*(5094.d0 - 14124.d0*c2 + 6528.d0*c2*c2)  +
     9  ht**5.d0*(-2302.d0 + 7094.d0*c2 - 4288.d0*c2*c2) + 
     $  ht**6.d0*(476.d0 - 1516.d0*c2 + 896.d0*c2*c2)+
     1  ht**7.d0*(-34.d0 + 116.d0*c2 - 64.d0*c2*c2))*dlog(ht)/
     2  (54.d0*(-4.d0 + ht)**2.d0*ht**3.d0) -
     3  (3.d0+122.d0*c2-388.d0*c2*c2+56.d0*c2**3.d0)*
     $   (dlog(zt)- dlog(mz2/mu2))/(9.d0*c2) +
     4  (3.d0+5.d0*c2-26.d0*c2*c2-48.d0*c2**3.d0)*dlog(zt)/(9.d0*c2) +
     5  (23040.d0-23040.d0*c2-25860.d0*ht+25344.d0*c2*ht + 
     6  1536.d0*c2*c2*ht+ht*ht*(10236.d0-7872.d0*c2 -1920*c2*c2) +
     7  ht**3.d0*(-1890.d0 -  2856.d0*c2 + 384.d0*c2*c2) + 
     8  ht**4.d0*(144.d0+2724.d0*c2)-672.d0*c2*ht**5.d0 + 
     9  54.d0*c2*ht**6.d0)*phi(ht/4.d0)/(54.d0*(-4.d0+ht)**2.d0*ht*ht))+
     $  4.d0*zt*(1.d0 - dlog(ht)+ dlog(zt)-dlog(mz2/mu2))
c
        endif
        endif
c
        return
        end
c
c       This subroutine computes  the additional term to be added
c       to the Msbar 2 loop expression of
c       [Aww (mw^2)/mw^2 - Azz(mz^2)/mw^2] 
c       in units 
c       Nc*(alfa0/(4.d0*pi*s2))**2/(4.d0*zt*c2)
c       OUTPUT = tobf2lewOS
c   
        subroutine ew2ltobfOS(zt,tobf2lewOS,s2,mz,mu)
        real*8 zt,tobf2lewOS,s2,c2,mz,mu
        c2 = 1.d0 - s2  
        tobf2lewOS  = -(
     1   2.d0*(18.d0 - 181.d0*c2 + 166.d0*c2*c2 - 
     2   216.d0*c2**3.d0)/(27.d0*c2) + 
     3   DSqrt(-1.d0 + 4.d0*c2)*(2.d0*(1.d0 +26.d0*c2+24.d0*c2*c2)*
     4   DATan(1/Sqrt(-1.d0 + 4.d0*c2))/3.d0 - 
     5   (1.d0 + 11.d0*c2 + 12.d0*c2*c2)*
     6   DATan(Sqrt(-1.d0 + 4.d0*c2))/(3.d0*c2*c2)) + 
     7   DLog(c2)/(6.d0*c2*c2)*
     8   (1.d0 + 9.d0*c2 - 4.d0*c2*c2 + 60.d0*c2**3.d0) + 
     9   (-20.d0 + 32.d0*c2)*dLog(zt)/9.d0 +
     $   Dlog(mz*mz/mu/mu)*(4.d0/3.d0 + 34.d0/3.d0*s2 -1.d0/c2))
         
        return
         end
c
c       This subroutine computes  the additional term to be added
c       to the Msbar 2 loop expression of Deltar^w
c       in units 
c       Nc*(alfa0/(4.d0*pi*s2))**2/(4.d0*zt*c2)
c       OUTPUT = drs2lewOS
c
        subroutine ew2ldeltarwOS(zt,drs2lewOS,s2,mz,mu)
        real*8 zt,drs2lewOS,s2,c2,mz,mu
        c2 = 1.d0 - s2  
        drs2lewOS = - (
     1   1.d0/(3.d0*c2) + 4.d0 - 
     2   (1.d0 + 11.d0*c2 + 12.d0*c2*c2)/(3.d0*c2*c2)*
     3   DSqrt(-1.d0 + 4.d0*c2)*DATan(Sqrt(-1.d0 + 4.d0*c2)) -
     4   DLog(c2)*(0.5d0/s2 +3.d0 - 1.5d0/c2 - 1.d0/(6.d0*c2*c2)))
         return
         end
c
c       This subroutine contains the 2 loop expressions of
c       -c/s A_{gamma Z}(mz^2/mz^2) and the vertex contribution
c       in units
c       Nc*(alfa0/(4.d0*pi*s2))**2.d0/(4*zt*c2)**2.d0
c       OUTPUT = k2lew
c 
        subroutine kappacur2l(zt,ht,k2lew,c2,mz,mu)
        real*8 zt,k2lew,phi,c2,mz,mz2,B0,gdeg,mu,mu2,ht,hht
        mz2 = mz*mz
        mu2=mu*mu
        hht = ht/4.d0
        k2lew = zt*(
     1  -8.d0*c2*c2*DLog(c2)/3.d0 + 4.d0*(30.d0*c2-48.d0*c2*c2+ 
     2  135.d0*c2*ht - 216.d0*c2*c2*ht - 50.d0*c2*ht*ht + 
     3  80.d0*c2*c2*ht*ht+5.d0*c2*ht**3.d0-8.d0*c2*c2*ht**3.d0)*
     4  DLog(ht)/(27.d0*(-4.d0 + ht)) + 
     5  (251.d0*c2 - 462.d0*c2*c2-40.d0*c2*ht+64.d0*c2*c2*ht + 
     6  (18.d0*c2 + 144.d0*c2*c2)*B0(mz2,mz2*c2,mz2*c2,mu2) 
     8  +(20.d0*c2 - 32.d0*c2*c2)*(ht-4.d0)*DSQRT(ht)*gdeg(ht) 
     9  -(186.d0*c2-240.d0*c2*c2)*(-DLog(zt)+Dlog(mz2/mu2)))/27.d0+ 
     1  8.d0*(c2 - 3.d0*c2*c2)*DLog(zt)/9.d0  
     1  +8.d0*phi(hht)*(5.d0*c2-8.d0*c2*c2)/(9.d0*(-4.d0 +ht)*ht)-
     1  8.d0*phi(hht)*(5.d0*c2- 8.d0*c2*c2)/(9.d0*(-4.d0 + ht)) 
     1    )
        return
        end
c
c       This subroutine should replace the one with the same name
c       Notice that in the subroutine statement there is one new
c       entry, i.e. i3qf.
c       i3qf is the product of the isospin of the fermion (i3)
c       times the electric charge (qf). The isospin is defined such that for
c       electron i3 = -1, therefore for electron i3qf = 1
c       for down quark i3qf = 1/3, for up quark i3qf = 2/3
c
c       This subroutine contains the additional piece to be
c       added to the MSbar two-loop contribution to
c       c/s A_{gamma Z}(mz^2/mz^2 and the vertex part
c       in units
c       Nc*(alfa0/(4.d0*pi*s2))**2.d0/(4*zt)
c     
        subroutine kappacur2lOS(zt,k2lewOS,c2w,mz,mu,sman,i3qf)
        real*8 zt,k2lewOS,s2w,c2w,mz,mz2,B0,mu,mu2,j1,sman,
     1         ug,revf,i3qf
        s2w = 1.d0-c2w
        mz2 = mz*mz
        mu2=mu*mu
c       c/s A_gz
c       top
        k2lewOS = -32.d0/3.d0*(j1(1.d0/zt,sman/mz2)+dlog(sman/mu2)/6.d0)
c       light quarks
        k2lewOS = k2lewOS + 44.d0/9.d0*(-dlog(sman/mu2)+5.d0/3.d0)
c       leptons
        k2lewOS = k2lewOS + 4.d0*(-dlog(sman/mu2)+5.d0/3.d0)
c       bosons
        k2lewOS = k2lewOS -( 4.d0*c2w*mz2/sman*dlog(c2w)+ 
     $                      4.d0*c2w*mz2/sman*dlog(mz2/mu2) +
     1           (3.d0*sman/mz2 + 4.d0*c2w*mz2/sman)*
     2           B0(sman,mz2*c2w,mz2*c2w,mu2))
c       vertex contribution
        k2lewOS = k2lewOS +2.d0*(ug(c2w*mz2/sman)+0.25d0) +
     $   (i3qf - 1.d0)*(revf(mz2*c2w/sman) + 
     $                   3.d0/2.d0/c2w/c2w*revf(mz2/sman)) -
     $    4.d0*s2w/c2w*(i3qf*i3qf-1.d0)*revf(mz2/sman) +
     1   (5.d0 - 16.d0*s2w + 8.d0*s2w*s2w)/4.d0/c2w/c2w*revf(mz2/sman)
     2       +2.d0*dlog(c2w) +2.d0*dlog(mz2/mu2)
        return
        end
c
c       In OS framework in units
c       Nc*(alfa/(4.d0*pi*s2))**2.d0*(mt^2/(4.d0 mw^2))
c       one has:
c 
c       eta^(2l) = 4.d0*eta2lew +eta2lewOS
c
c       This subroutine contains the 2 loop expressions of
c       Re [Azz (q^2) - Azz(mz^2)]/(q^2 - mz^2) |_mz^2
c       in units
c       Nc*(alfa0/(4.d0*pi*s2))**2.d0/(zt*c2)
c       OUTPUT = eta2lew
c
        subroutine ew2leta(zt,ht,eta2lew,c2,mz,mu)
        real*8 zt,ht,eta2lew,c2,mz,ln,mz2,B0,gdeg,phi,mu,mu2
        double precision gmu,repiggh0,mz0,alfa0,alfastr,delQCD,
     1                pi,mel,mmu,mtau,mb,gamz,Nc,zeta3
c
        common/datcom/gmu,repiggh0,mz0,alfa0,alfastr,delQCD,
     1                pi,mel,mmu,mtau,mb,gamz,Nc,zeta3
        mz2 = mz*mz
        mu2 = mu*mu
  
        if(dsqrt(ht).lt.0.57d0) then
c       LIGHT HIGGS CASE
        
        eta2lew = 
     -   ((-17.d0 + 40.d0*c2 - 32.d0*(c2*c2))*
     -   (-4.d0 + 2.d0*Sqrt(ht)*Pi))/(108.d0*c2) + 
     -  ((-24.d0 + 96.d0*c2)*ht*ht*ht + (144.d0 - 576.d0*c2)*ht*ht*zt + 
     -  ( -313.d0 +1345.d0*c2 + 349.d0*c2*c2 - 292.d0*c2**3)*ht*zt*zt + 
     -  (196.d0 - 1156.d0*c2 - 1396.d0*c2*c2 + 1168.d0*c2**3)*zt*zt*zt)/
     -  (216.d0*c2*(-1.d0 + 4.d0*c2)*(ht - 4.d0*zt)*zt*zt) + 
     -  ((-2.d0*ht*ht*ht + 13.d0*ht*ht*zt - 32.d0*ht*zt*zt + 
     -  36.d0*zt*zt*zt)*B0(mz2, ht*mz2/zt, mz2, mu2))/
     -  (18.d0*c2*(ht - 4.d0*zt)*zt*zt) + 
     -  ((-1.d0 + 4.d0*c2 - 44.d0*c2*c2 + 32.d0*c2**3)*
     -  B0(mz2, c2*mz2, c2*mz2,mu2))/(-24.d0*c2 + 96.d0*(c2*c2)) + 
     -  ((-1.d0-18.d0*c2+16.d0*c2*c2)*DLog(c2))/(2.d0*(-6.d0+24.d0*c2))+ 
     -  ((-2.d0*ht*ht*ht + 11.d0*ht*ht*zt - 24.d0*ht*zt*zt + 
     -  24.d0*zt*zt*zt)*DLog(ht))/(18.d0*c2*(ht - 4.d0*zt)*zt*zt) + 
     -  (((8.d0-32.d0*c2)*ht*ht*ht -52.d0*ht*ht*zt +208.d0*c2*ht*ht*zt + 
     -  159.d0*ht*zt*zt - 704.d0*c2*ht*zt*zt + 192.d0*c2*c2*ht*zt*zt - 
     -  112.d0*c2**3*ht*zt*zt + ( -268.d0 + 1344.d0*c2 - 
     -  768.d0*(c2*c2) + 448.d0*c2**3)*zt*zt*zt)*
     -  (Dlog(1.d0/zt) +DLog(mz2/mu2)))/
     -  (72.d0*c2*(-1.d0 + 4.d0*c2)*(ht - 4.d0*zt)*zt*zt) + 
     -  ((-4.d0*ht*ht + 16.d0*c2*ht*ht + 20.d0*ht*zt - 79.d0*c2*ht*zt - 
     -  70.d0*c2*c2*ht*zt+48.d0*c2**3*ht*zt-40.d0*zt*zt+156.d0*c2*zt*zt+ 
     -  280.d0*c2*c2*zt*zt - 192.d0*c2**3*zt*zt)*DLog(zt))/
     -  (36.d0*c2*(-1.d0 + 4.d0*c2)*(ht - 4.d0*zt)*zt)
c
        else
c
c       HEAVY HIGGS CASE
c
        eta2lew =
     -  (-1152.d0 + 4608.d0*c2 +50.d0*ht+2248.d0*c2*ht+976.d0*c2*c2*ht - 
     -  1600.d0*c2**3*ht+67.d0*ht*ht-880.d0*c2*ht*ht-244.d0*c2*c2*ht*ht+ 
     -  400.d0*c2**3*ht*ht)/(864.d0*c2*(1.d0 -4.d0*c2)*(-4.d0 + ht)*ht)+ 
     -  ((-1.d0 + 4.d0*c2 - 44.d0*c2*c2 + 32.d0*c2**3)*
     -  B0(mz2,c2*mz2,c2*mz2,mu2))/(-24.d0*c2 + 96.d0*(c2*c2)) + 
     -  (4.d0/3.d0 - 4.d0/ht - 
     -  (1.d0 -4.d0/ht)*Ln(-1.d0 + 4.d0/ht))/(12.d0*c2) + 
     -  ((-1.d0-18.d0*c2+16.d0*c2*c2)*DLog(c2))/(2.d0*(-6.d0+24.d0*c2))+ 
     -  ((-384.d0 - 202.d0*ht + 320.d0*c2*ht - 256.d0*c2*c2*ht + 
     -  55.d0*ht*ht + 80.d0*c2*ht*ht - 64.d0*c2*c2*ht*ht +3.d0*ht*ht*ht-
     -  40.d0*c2*ht*ht*ht + 32.d0*(c2*c2)*ht*ht*ht)*DLog(ht))/
     -  (144.d0*c2*(-4.d0 + ht)**2*ht) +
     -  ((-17.d0 + 40.d0*c2 - 32.d0*(c2*c2))*(-4.d0 + ht/2.d0 + 
     -  (1.d0 - ht/4.d0)*Sqrt(ht)*gdeg(ht) + 
     -  ((6.d0 - ht)*ht*Log(ht))/4.d0))/(108.d0*c2) + 
     -  ((-31.d0 + 192.d0*c2 - 192.d0*(c2*c2) + 112.d0*c2**3)*
     -  (Dlog(1.d0/zt) + DLog(mz2/mu2)))/(72.d0*c2 - 288.d0*(c2*c2)) + 
     -  ((2.d0 - 7.d0*c2 - 70.d0*c2*c2 + 48.d0*c2**3)*DLog(zt))/
     -  (-36.d0*c2 + 144.d0*(c2*c2))+ 
     -  ((-384.d0 - 10.d0*ht + 320.d0*c2*ht - 256.d0*(c2*c2)*ht + 
     -  238.d0*ht*ht - 400.d0*c2*ht*ht + 320.d0*c2*c2*ht*ht - 
     -  63.d0*ht*ht*ht + 80.d0*c2*ht*ht*ht - 
     -  64.d0*(c2*c2)*ht*ht*ht + 3.d0*ht**4)*phi(ht/4.d0))/
     -  (144.d0*c2*(-4.d0 + ht)**2*ht*ht)
        endif
c
        return
        end
c
c       This subroutine contains the additional piece to be
c       added to the MSbar two-loop contribution to
c       Re [Azz (q^2) - Azz(mz^2)]/(q^2 - mz^2) |_mz^2
c       in units
c       Nc*(alfa0/(4.d0*pi*s2))**2.d0/(4.d0*zt*c2)
c       OUTPUT = eta2lewOS
c      
        subroutine ew2letaOS(zt,ht,eta2lewOS,s2,mz,mu,sman,i3qf)
        real*8 zt,eta2lewOS,c2,mz,mz2,B0,mu,mu2,sman,ug,revf,i3qf,
     1         zs2,di,ln,hbar,j1,a1,a2,ht,s2,vertex
c
        mz2 = mz*mz 
        mu2=mu*mu
        zs2 =1.d0/zt        
        c2 = 1.d0 - s2
c       Azz
         eta2lewOS =  
     -   (197.d0 - 1378.d0*c2 + 1064.d0*c2*c2)/(27.d0*(-1.d0 +4.d0*c2))+ 
     -   (1.d0 + 16.d0*c2 - 20.d0*(c2*c2) + 48.d0*c2**3)*
     -   B0(mz2, c2*mz2, c2*mz2, mu2)/(-3.d0 + 12.d0*c2) + 
     -   (2.d0*c2*(1.d0 + 26.d0*c2 + 24.d0*(c2*c2))*DLog(c2))/
     -   (-3.d0 + 12.d0*c2) + 
     -   (2.d0*(-20.d0 + 113.d0*c2 - 102.d0*(c2*c2) + 24.d0*c2**3)*
     -   (Dlog(zs2) + DLog(mz2/mu2)))/(-3.d0 + 12.d0*c2) + 
     -   (2.d0*(-50.d0 + 283.d0*c2 - 242.d0*c2*c2 + 72.d0*c2**3)*
     -   DLog(zt))/(-9.d0 + 36.d0*c2)  

c       Vertex contribution 
        vertex = 
     -   4.d0*c2*(DLog(c2) + DLog(mz2/mu2)) +4.d0*c2*(ug(mz2*c2/sman)+
     -    0.25d0) - 2.d0*c2*(1.d0 - i3qf)*revf(mz2*c2/sman) -
     -    (1.d0 - 6.d0*I3qf + 12.d0*(1.d0-c2*c2)*i3qf*i3qf)/(2.d0*c2)*
     -    revf(mz2/sman) 
  
c       Contribution of eta due to overall shift
c 
c       hadronic contribution
        di=4.d0*zs2-1.d0
         a1=(6.d0*(zs2-4.d0*zs2*zs2/di*ln(di)+1.d0/6.d0+j1(zs2,1.d0))*
     1       (16.d0*s2*s2/9.d0-4.d0*s2/3.d0+0.5d0)-1.5d0*zs2 +
     1        6.d0*zs2*zs2/di*ln(di) -
     2        2.d0/3.d0*(2.5d0-14.d0/3.d0*s2+44.d0*s2*s2/9.d0)  +
     3       3.d0*(20.d0/9.d0*s2*s2 - 2.d0*s2 + 1.d0)*Dlog(mz2/mu2))/c2
c          leptonic contribution
        a1= a1 +(1.d0-2.d0*s2+4.d0*s2*s2)/c2*(Dlog(mz2/mu2)-2.d0/3.d0)
c          bosonic contribution
        a2=dsqrt(4.d0*c2-1.d0)
        a2=a2*datan(1.d0/a2)
        a2=a2*(1.d0/6.d0/c2-2.d0/3.d0+c2/c2/3.d0-4.d0/3.d0*c2
     1        +6.d0*c2*c2-6.d0*c2+c2/c2*(18.d0*c2**2-8.d0*c2*s2**2)
     2        /(4.d0*c2-1.d0))
c
        a2=a2+hbar(ht/zt)/2.d0/c2
        a2=a2+dlog(c2)*(1.d0/12.d0/c2-1.d0/3.d0-3.d0*c2)-4.d0*c2*c2
     1     +108.d0/36.d0*c2-8.d0*c2/3.d0+5.d0/9.d0-5.d0/36.d0/c2
     2     +5.d0/3.d0 -1.d0/6.d0*(19.d0*c2 - s2*s2/c2)*dlog(mz2/mu2)
        eta2lewOS = eta2lewOS + vertex + a1 + a2
c
        return
        end

c       function Hbar in d AZZ(q**2)/d q**2 (Degrassi-Sirlin)
        double precision function hbar(x)
        double precision x,a,h
        h=31.d0/18.d0-x+x*x/3.d0
        h=(1.d0-1.5d0*x+0.75d0*x*x-x*x*x/6.d0)*dlog(x)+h
        if(x.gt.4.d0) go to 330
        a=dsqrt(4.d0/x-1.d0)
        a=datan(a)
        a=a*dsqrt(4.d0*x-x*x)
        hbar=h+(-2.d0+2.d0/(4.d0-x)+5.d0*x/6.d0-x*x/3.d0)*a
        go to 340
330     continue
        a=dsqrt(1.d0-4.d0/x)
        a=dlog((1.d0-a)/(1.d0+a))
        a=dsqrt(x*x/4.d0-x)*a
        hbar=h+(-2.d0+2.d0/(4.d0-x)+5.d0/6.d0*x-x*x/3.d0)*a
340     continue
        return
        end
