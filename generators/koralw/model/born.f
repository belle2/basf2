c......==================== BORN ==========================
      function wwborn_massless(bp1,bp2,bp3,bp4,keyacc_lcl)
*     *******************************************
c This function provides a value of a differential born cross section
c for W+W- pair production and decay in e+e- scattering. 
c INPUT: sprim   - CMS-eff energy squared (in GeV**2)
c        costhe - cosine of the W- polar angle in the CMS
c                 of the incoming e+e- with z-axis pointing 
c                 in the e- direction
c Written by: Wieslaw Placzek            date: 20.07.1994
c Last update: 27.07.1994                by: W.P.
c
!
! wwborn is massless strictly speaking, but it works for massive 
! 4vects as well and the buffor routine wwborn_massive is in fact
! redundant, m.s. 
!
      implicit DOUBLE PRECISION (a-h,o-z)
      common / matpar / pi,ceuler     
      common / weking / ene,amaz,gammz,amel,amfin,xk0,sinw2,ide,idf 
      common / wekin2 / amaw,gammw,gmu,alphaw   
! This common can be everywhere, contains various switches
      COMMON / KeyKey/  KeyRad,KeyPhy,KeyTek,KeyMis,KeyDwm,KeyDwp
!  br - for normalisation
      COMMON / decdat / amafin(20), br(20)
! polarization amplitudes for WW production for left and right 
! handed electrons
      DOUBLE COMPLEX awwel(3,3),awwer(3,3)
! polarization amplitudes for W-decays
      DOUBLE COMPLEX adwm(3),adwp(3)
! auxilliary
      DOUBLE COMPLEX amwwr, amwwl,BWEXA1,BWEXA2
      dimension bq1(4),bq2(4),bp1(4),bp2(4),bp3(4),bp4(4)
      save   / weking /,/ wekin2 /,/ matpar / 
      save   / decdat /,/ keykey /
      save

      KeySpn = MOD(KeyPhy,10000)/1000
      KeyRed = MOD(KeyPhy,100000)/10000
c      KeyAcc = MOD(KeyMis,1000)/100
      KeyAcc = keyacc_lcl
      KeyWu  = MOD(KeyPhy,1000000)/100000
      Key4f  = MOD(KeyMis,100)/10

! convert linear labels to KoralW.13x convention (mode=1)
! these routines come from the decay.f package !!! 
      CALL store_label(1,label)
      CALL linear_to_WZ_label(1,label,icwm,icwp,ifznow,ifwnow)

      IF(ifwnow.EQ.0) THEN
!-- for ZZ Born is set to 0
!--     except for MIX doubly CKM-non-diagonal, ms 10/7/98
!--     in this case to get cross-section we cheat by 
!--     re-setting icwm & icwp from ZZ to WW values
        IF(icwm.EQ.1.AND.icwp.EQ.4 .OR. icwm.EQ.4.AND.icwp.EQ.1) THEN
!         cdcd
          icwm=2
          icwp=2
        ELSEIF(icwm.EQ.2.AND.icwp.EQ.3 .OR. icwm.EQ.3.AND.icwp.EQ.2)THEN 
!         usus
          icwm=3
          icwp=3
        ELSEIF(icwm.EQ.2.AND.icwp.EQ.5 .OR. icwm.EQ.5.AND.icwp.EQ.2)THEN 
!         ubub
          icwm=5
          icwp=5
        ELSEIF(icwm.EQ.4.AND.icwp.EQ.5 .OR. icwm.EQ.5.AND.icwp.EQ.4)THEN 
!         cbcb
          icwm=6
          icwp=6
        ELSE
!         all others
          wwborn_massless = 0
          RETURN
        ENDIF
      ENDIF

      IF(key4f.EQ.2) THEN
!-- Born suppressed (in case of external Matr el.)
!-- ( to restore ZZ, remember to fix br. ratios - br(icwm).. )
        wwborn_massless=1d0
        IF(ifwnow.EQ.1) THEN
!-- to have non-diag CKM zeroed if requested
          brel=br(7)  
          dfwmwp=br(icwm)*br(icwp)/brel**2
          wwborn_massless=wwborn_massless*dfwmwp
        ENDIF
        RETURN
      ENDIF

      wwborn_massless = 0  

      DO i=1,4
        bq1(i)=bp1(i)+bp2(i)
        bq2(i)=bp3(i)+bp4(i)
      ENDDO
!.. cms-eff mass
      sprim=(bp1(4)+bp2(4)+bp3(4)+bp4(4))**2
!.. resonance masses
      s1=dmas2(bq1)
      s2=dmas2(bq2)
!.. cos theta
      qq=dsqrt(bq1(1)**2+bq1(2)**2+bq1(3)**2)
      costhe=bq1(3)/qq

!! here was a bug A. Vallasi 30.06.96
!ms      wlambd=abs(sprim**2+s1**2+s2**2-2*sprim*s1-2*sprim*s2-2*s1*s2)
      wlambd=max(0d0,abs((sprim-s1-s2)**2 -4*s1*s2))
!      tvar=-(sprim-s1-s2-dsqrt(wlambd)*costhe)/2

      tvar=-1d0/2d0*(dsqrt(wlambd)*(1d0-costhe)
     $              +4d0*s1*s2/(sprim-s1-s2+dsqrt(wlambd)) ) 
!... Amplitudes for WW production
      IF(keyacc.EQ.0) THEN 
        call wwprod(sprim,tvar,bq1,bq2,awwel,awwer) 
      ELSE
        call wwamgc(sprim,tvar,bq1,bq2,awwel,awwer) 
      ENDIF
!... Amplitudes for W-decays
      call wdecay(bq1,bp1,bp2,adwm)  
      call wdecay(bq2,bp3,bp4,adwp) 
!... Exact W-propagators:
      IF(KeyWu.EQ.0) THEN
        BWEXA1=1D0/dcmplx((S1-AMAW**2),(S1/AMAW*GAMMW))
        BWEXA2=1D0/dcmplx((S2-AMAW**2),(S2/AMAW*GAMMW))
      ELSEIF(KeyWu.EQ.1) THEN
        BWEXA1=1D0/dcmplx((S1-AMAW**2),(AMAW*GAMMW))
        BWEXA2=1D0/dcmplx((S2-AMAW**2),(AMAW*GAMMW))
      ELSEIF(KeyWu.EQ.2) THEN
        BWEXA1=1D0/dcmplx((S1-AMAW**2),0D0)
        BWEXA2=1D0/dcmplx((S2-AMAW**2),0D0)
      ELSE
        WRITE(6,*)'BWIGN==> Wrong KeyWu=',keywu
        STOP
      ENDIF


!... Polarization amplitudes for WW production and decay
      if(keyspn.eq.0)then
        xmatr=0
        do 10 l2=1,3
        do 10 l1=1,3

          amwwl=awwel(l1,l2)*adwm(l1)*adwp(l2)*BWEXA1*BWEXA2 !ms,zw
          amwwr=awwer(l1,l2)*adwm(l1)*adwp(l2)*BWEXA1*BWEXA2 !ms,zw
          xmatr=xmatr + amwwl*dconjg(amwwl) !ms
          xmatr=xmatr + amwwr*dconjg(amwwr) !ms
 10     continue
      elseif(keyspn.eq.1)then
        xmatr=0
        amwwl=(0,0)
        amwwr=(0,0)
        do 20 l2=1,3
        do 20 l1=1,3
          amwwl=amwwl+awwel(l1,l2)*adwm(l1)*adwp(l2)*BWEXA1*BWEXA2 !ms,zw
          amwwr=amwwr+awwer(l1,l2)*adwm(l1)*adwp(l2)*BWEXA1*BWEXA2 !ms,zw
 20     continue
        xmatr=xmatr + amwwl*dconjg(amwwl) !ms
        xmatr=xmatr + amwwr*dconjg(amwwr) !ms
      endif
      fkin=1D0
      wwborn_massless=fkin*xmatr 
! include normalisation due to branching ratios (various WW channels)
! 10/7/98 ms      IF(ifwnow.EQ.1) THEN
        brel=br(7)  
        dfwmwp=br(icwm)*br(icwp)/brel**2
        wwborn_massless=wwborn_massless*dfwmwp
! 10/7/98 ms      ENDIF
! include spin average 1/4
      wwborn_massless=wwborn_massless/4d0

      end

      subroutine wwprod(s,t,q1,q2,awwel,awwer)
*     ****************************************
c This routine calculates polarization amplitudes for the process
c e+e- --> W+W-, for on-shell W's in Born approximation. Calculation
c is done in the CMS of e+e- with z-axis pointing along e- direction. 
c It is based on the paper: 
c K. Kolodziej, M. Zralek, Phys. Rev. D43 (1991) 43;
c INPUT: s   - center mass energy squared (in GeV**2)
c        t   - transfer (in GeV**2)
c        q1(4) - four-momentum of W- 
c        q2(4) - four-momentum of W+
c OUTPUT: awwel(3,3) - complex array containing polarization amplitudes 
c                     for left-handed electron 
c                     {M_0(-,+,l1,l2) in eq. (31)}
c         awwel(3,3) - complex array containing polarization amplitudes 
c                     for right-handed electron 
c                     {M_0(+,-,l1,l2) in eq. (31)}
c
c Written by: Wieslaw Placzek            date: 01.07.1994
c Last update: 02.08.1994                by: W.P.
c
      implicit DOUBLE PRECISION (a-h,o-z)
      DOUBLE PRECISION q1(4),q2(4)
      DOUBLE COMPLEX awwel(3,3),awwer(3,3)
      common / matpar / pi,ceuler     
      common / weking / ene,amaz,gammz,amel,amfin,xk0,sinw2,ide,idf 
      common / wekin2 / amaw,gammw,gmu,alphaw   
! This common can be everywhere, contains various switches
      COMMON / KeyKey/  KeyRad,KeyPhy,KeyTek,KeyMis,KeyDwm,KeyDwp
! polarization vectors of W-bosons, eq. (9)
      DOUBLE PRECISION eps1(4,3),eps2(4,3)
      DOUBLE PRECISION e1(4),e2(4),p1(4),pmq(4)
      DOUBLE COMPLEX f1e1,f1e2,f1q2,fkz3
      data init /0/
      save init,zetl,etal,etar
      save
!
!...initialization
!--------------------------------------------------------------------- 
      if (init.eq.0) then
        init=1 
        KeyZet = MOD(KeyPhy,1000)/100
!... electroweak coefficient factors, eq. (30)
        zetl=0.5d0/sinw2
        etal=1-zetl
        etar=1
      endif  
!... calculation
!=====================================================================
!... four-momentum of the incoming electron in CMS 
!    (+z axis along the electron beam)
      ecm=dsqrt(s)   
      p1(1)=0d0
      p1(2)=0d0
      p1(3)=ecm/2d0
      p1(4)=p1(3)
!... calculation of polarization vectors of W-bosons, eq. (9)
      call polvec(q1,eps1)
      call polvec(q2,eps2)
!... calculation of the polarization amplitudes, eq. (31)
!    {note: we use different normalization!}
!    (note: electric charge squared equals: 4*pi*alphaw}
      wsp=-4*pi*alphaw*ecm
      do 10 k=1,4
 10   pmq(k)=p1(k)-q1(k)
      do 20 l2=1,3
      do 20 l1=1,3
        do 25 k=1,4
          e1(k)=eps1(k,l1)
 25       e2(k)=eps2(k,l2)
        f1e1=dcmplx(e1(1),-e1(2))
        f1e2=dcmplx(e2(1),-e2(2))
        f1q2=dcmplx(q2(1),-q2(2))
        e1e2=prodm(e1,e2)
        e1q2=prodm(e1,q2)
        e2q1=prodm(e2,q1) 
        if(keyzet.eq.0)then
        awwel(l1,l2)=( 2*(1/s - etal/dcmplx(s-amaz**2,s/amaz*gammz))*
     &                   (e1q2*f1e2 - e1e2*f1q2 - e2q1*f1e1) + 
     &                 zetl/t*fkz3(e2,pmq,e1) ) *wsp   
        awwer(l1,l2)= -2*(1/s - etar/dcmplx(s-amaz**2,s/amaz*gammz))*
     &                   (e1q2*conjg(f1e2) - e1e2*conjg(f1q2) -
     &                    e2q1*conjg(f1e1)) *wsp
        elseif(keyzet.eq.1)then
        awwel(l1,l2)=( 2*(1/s - etal/dcmplx(s-amaz**2,amaz*gammz))*
     &                   (e1q2*f1e2 - e1e2*f1q2 - e2q1*f1e1) + 
     &                 zetl/t*fkz3(e2,pmq,e1) ) *wsp   
        awwer(l1,l2)= -2*(1/s - etar/dcmplx(s-amaz**2,amaz*gammz))*
     &                   (e1q2*conjg(f1e2) - e1e2*conjg(f1q2) -
     &                    e2q1*conjg(f1e1)) *wsp
        elseif(keyzet.eq.2)then
        awwel(l1,l2)=( 2*(1/s - etal/(s-amaz**2))*
     &                   (e1q2*f1e2 - e1e2*f1q2 - e2q1*f1e1) + 
     &                 zetl/t*fkz3(e2,pmq,e1) ) *wsp   
        awwer(l1,l2)= -2*(1/s - etar/(s-amaz**2))*
     &                   (e1q2*conjg(f1e2) - e1e2*conjg(f1q2) -
     &                    e2q1*conjg(f1e1)) *wsp
        else
          write(6,*)'wrong KEYZET:',keyzet
        endif
 20   continue
      end  

      function prodm(p,q)
*     *******************
c Scalar product of the four-vectors p and q in Minkowski space;
c note: p_0=p(4), q_0=q(4)
c 
      implicit DOUBLE PRECISION (a-h,o-z)
      DOUBLE PRECISION p(4),q(4)
      prodm=p(4)*q(4)-p(3)*q(3)-p(2)*q(2)-p(1)*q(1)
      end

      subroutine polvec(q,eps)
*     **************************
c Calculation of polarization vectors of a vector boson 
c in the rectangular basis, see eq. (9); see also K. Hagiwara 
c and D. Zeppenfeld, Nucl. Phys. B274 (1986) 1, eq. (3.47).
c     INPUT:  q(4)     - four-momentum of the vector boson
c     OUTPUT: eps(4,3) - three polarization four-vector
c
c Written by: Wieslaw Placzek            date: 01.07.1994
c Last update: 27.07.1994                by: W.P.
c
      implicit DOUBLE PRECISION (a-h,o-z)
      DOUBLE PRECISION q(4),eps(4,3)   
      save
!
      qt2=q(1)**2+q(2)**2
      qt =sqrt(qt2)
      aq2=qt2+q(3)**2
      aq =sqrt(aq2)
      am =sqrt(q(4)**2-aq2) 
      do 10 l=1,3
      do 10 k=1,4
   10 eps(k,l)=0
      if(aq.lt.1d-10) then
        eps(1,1)=1
        eps(2,2)=1
        if (am.gt.1d-10) eps(3,3)=1 
      elseif (qt2.lt.1d-10) then
        eps(1,1)=q(3)/aq
        eps(2,2)=q(3)/aq
        if (am.gt.1d-10) then
          eps(3,3)=q(4)/am/aq*q(3)
          eps(4,3)=aq/am
        endif
      else
        ws1=1/aq/qt
        eps(1,1)= ws1*q(1)*q(3)
        eps(2,1)= ws1*q(2)*q(3)
        eps(3,1)=-ws1*qt2
        eps(4,1)= 0
        ws2=1/qt 
        eps(1,2)=-ws2*q(2)
        eps(2,2)= ws2*q(1)
        eps(3,2)= 0
        eps(4,2)= 0
        if (am.gt.1d-10) then
          ws3=q(4)/am/aq 
          eps(1,3)=ws3*q(1)
          eps(2,3)=ws3*q(2)
          eps(3,3)=ws3*q(3)
          eps(4,3)=ws3*aq2/q(4)
        endif
      endif  
      end

      function fkz3(a,b,c)
*     *******************************
c Function F_3 of four-vectors contracted with Dirac matrices; 
c see eq. (19)
c
c Written by: Wieslaw Placzek            date: 01.07.1994
c
      DOUBLE PRECISION a(4),b(4),c(4)
      DOUBLE COMPLEX fkz3
      fkz3=(a(4) +a(3)) * ((b(4) -b(3))*dcmplx(c(1),-c(2)) -
     &                     dcmplx(b(1),-b(2))*(c(4) -c(3)))+ 
     &     dcmplx(a(1),-a(2)) * ((b(4) +b(3))*(c(4) -c(3)) -
     &                     dcmplx(b(1), b(2))*dcmplx(c(1),-c(2)))
      end        

      subroutine wdecay(q,p1,p2,adw)
*     ******************************
c This routine calculates polarization amplitudes for W decays 
c into massless fermions. It is based on the paper: 
c K. Hagiwara et al., Nucl. Phys. B282 (1987) 253; see Appendix C.
c No CKM-mixing matrix elements incuded here.
c INPUT: q(4)        - four-momentum of W  
c        p1(4),p2(4) - four-momenta of decay products
c OUTPUT: adw(3) - complex array containing W decay amplitudes 
c                   {M(lambda,sigma_1,sigma_2) in eq. (C.16)}
c
c Written by: Wieslaw Placzek            date: 20.07.1994
c Last update: 02.08.1994                by: W.P.
c
      implicit DOUBLE PRECISION (a-h,o-z)
      common / matpar / pi,ceuler
      common / weking / ene,amaz,gammz,amel,amfin,xk0,sinw2,ide,idf 
      common / wekin2 / amaw,gammw,gmu,alphaw   
      save / matpar /, / weking /, / wekin2 /
      DOUBLE PRECISION q(4),p1(4),p2(4)
      DOUBLE COMPLEX adw(3)
      DOUBLE PRECISION eps(4,3),e(4)
      DOUBLE COMPLEX sfunhz
!
!... calculation of polarization vectors of W in rectangular basis
      call polvec(q,eps)
!... calculation of the W decay amplitudes
      do 10 l=1,3
        do 15 k=1,4
 15       e(k)=eps(k,l)
        adw(l)=  sqrt(4*pi*alphaw) /sqrt(2*sinw2)
     $        *2*sqrt(p1(4)*p2(4)) *sfunhz(p1,e,p2)
 10   continue
      end

      function sfunhz(p1,a,p2)
*     ********************************** *
c Spinorial string S(pi,a,pf) for massless spinors chi(pi), chi(pf);
c a(4) - given four-vector, 
c see K. Hagiwara et al., Nucl. Phys. B282 (1987) 253; Appendix C. 
c
c Written by: Wieslaw Placzek            date: 20.07.1994
c Last update: 01.08.1994                by: W.P.
c
      implicit DOUBLE PRECISION (a-h,o-z)
      DOUBLE PRECISION a(4),p1(4),p2(4)
      DOUBLE COMPLEX z1p,z2m,zam,zap
      DOUBLE COMPLEX sfunhz
!
      x1p=p1(3)+p1(4)
      x2p=p2(3)+p2(4)
      z1p=dcmplx(p1(1), p1(2))
      z2m=dcmplx(p2(1),-p2(2))
      xam=a(4)-a(3)
      xap=a(4)+a(3)
      zam=dcmplx(a(1),-a(2))
      zap=dcmplx(a(1), a(2))
      if (x1p.gt.1d-20 .and. x2p.gt.1d-20) then
        fac=0.5/sqrt(p1(4)*p2(4)*x1p*x2p)
        sfunhz=fac *( x1p*(x2p*xam-z2m*zap) + z1p*(z2m*xap-x2p*zam) )
      elseif (x1p.gt.1d-20) then
        sfunhz= (x1p*zap - z1p*xap)/sqrt(2*p1(4)*x1p)
      elseif (x2p.gt.1d-20) then
        sfunhz= (z2m*xap - x2p*zam)/sqrt(2*p2(4)*x2p)
      else
        sfunhz=xap
      endif
      end

!======================================================================
!============= Born Version with Anomalous Couplings ==================
!======================================================================

      subroutine WWamgc(s,t,q1,q2,awwel,awwer)
*     ****************************************
!----------------------------------------------------------------------!
! This routine calculates polarization amplitudes for the process:     !
!              e-(p1) e+(p2) ---> W-(q1) W+(q2)                        !
! Calculation  is done in the CMS of e+e- with z-axis pointing along   !
! the e- direction. These amplitudes include general type three boson  !
! coupling constant as given in the paper:                             !
!     K. Hagiwara, R.D. Peccei, D. Zeppenfeld and K. Hikasa,           !
!                 Nucl. Phys. B282 (1987) 253.                         !
! The same formalism as in the subroutine wwprod (where only the SM    !
! coupling constant are included) is used here.                        !
! INPUT: s   - center mass energy squared (in GeV**2)                  !
!        t   - transfer momentum squared (in GeV**2)                   !
!        q1(4) - four-momentum of W-                                   !
!        q2(4) - four-momentum of W+                                   !
! OUTPUT: awwel(3,3) - complex array containing polarization           !
!                      amplitudes for left-handed electron,            !
!                      M_0(-,+,l1,l2);                                 !
!         awwel(3,3) - complex array containing polarization           ! 
!                      amplitudes for right-handed electron,           ! 
!                      M_0(+,-,l1,l2).                                 !
! Note: Before first use of this routine general type coupling         !
!       constant have to be set up in the routine setacc.              !
!----------------------------------------------------------------------! 
! Written by: Wieslaw Placzek                 Knoxville, November 1995 !
! Last update: 16.11.1995            by: W.P.                          !
!----------------------------------------------------------------------!
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      DOUBLE COMPLEX zi
      PARAMETER ( zi = (0d0,1d0) )
      DOUBLE PRECISION q1(4),q2(4)
      DOUBLE COMPLEX awwel(3,3),awwer(3,3)
      COMMON / weking / ene,amaz,gammz,amel,amfin,xk0,sinw2,ide,idf 
      COMMON / wekin2 / amaw,gammw,gmu,alphaw 
      common / matpar / pi,ceuler  
! This common can be everywhere, contains various switches
      COMMON / KeyKey /  KeyRad,KeyPhy,KeyTek,KeyMis,KeyDwm,KeyDwp
      SAVE / weking /, / wekin2 /, / KeyKey /, / matpar /
! Polarization vectors of W-bosons          
      DOUBLE PRECISION eps1(4,3),eps2(4,3)
      DOUBLE PRECISION e1(4),e2(4),p1(4),pmq(4)
! General type 3-boson coupling constants
      DOUBLE COMPLEX GamV(4,2)
      DOUBLE COMPLEX dZ,Gagm,Gagp,GaZm,GaZp,fkz3
      DATA init /0/
      SAVE init,KeyZet,zetl,etal,etar
!--------------------------------------------------------------------- 
! Initialization ...
      IF (init.eq.0) THEN
        init=1 
        KeyZet = MOD(KeyPhy,1000)/100
! Electroweak coefficient factors
        zetl=0.5/sinw2
        etal=1-zetl
        etar=1
! Set up anomaluos couplings constants
!!        CALL setacc ! MOVED TO FILEXP
      ENDIF  
!--------------------------------------------------------------------- 
! Calculation ...
! Options for Z-boson width
! a) running Z-width
      IF (KeyZet.eq.0) THEN
        dZ = DCMPLX(s-amaz**2,s/amaz*gammz)
! b) constant Z-width
      ELSEIF (KeyZet.eq.1) THEN
        dZ = DCMPLX(s-amaz**2,amaz*gammz)
! c) zero Z-width
      ELSEIF (KeyZet.eq.2) THEN
        dZ = s-amaz**2
      ELSE
         WRITE(6,*)'>>> Wrong KeyZet:',KeyZet
      ENDIF
! Four-momentum of the incoming electron in CMS (+z axis along e-)
      p1(1) = 0
      p1(2) = 0
      p1(3) = SQRT(s)/2
      p1(4) = p1(3)
! Calculation of W polarization vectors
      CALL polvec(q1,eps1)
      CALL polvec(q2,eps2)
      DO k = 1,4
        pmq(k) = p1(k) - q1(k)
      ENDDO
      wsp =-4*pi*alphaw*SQRT(s)
! Calculation of the polarization amplitudes
      DO l2 = 1,3
        DO l1 = 1,3
          DO k = 1,4
            e1(k)=eps1(k,l1)
            e2(k)=eps2(k,l2)
          ENDDO
! Calculation of the 3-boson couplings
          CALL WWVgcc(s,amaw,q1,e1,q2,e2,GamV)
! WWgamma vertex
          Gagm = GamV(1,1) - zi*GamV(2,1)
          Gagp = GamV(1,1) + zi*GamV(2,1)          
! WWZ vertex
          GaZm = GamV(1,2) - zi*GamV(2,2)
          GaZp = GamV(1,2) + zi*GamV(2,2) 
! Polarization amplitudes         
          awwel(l1,l2) = wsp*( Gagm/s - GaZm*etal/dZ 
     &                       + zetl/t *fkz3(e2,pmq,e1) )
          awwer(l1,l2) = wsp*( Gagp/s - GaZp*etar/dZ )
        ENDDO
      ENDDO
      END

      subroutine WWVgcc(s,amW,q1,eps1,q2,eps2,GamV)
*     *********************************************
!----------------------------------------------------------------------!
! This routine calculates four-vector Gamma_V^mu including general     !
! type 3-boson WWV couplings (V=gamma,Z) as defined in the paper:      !
!     K. Hagiwara, R.D. Peccei, D. Zeppenfeld and K. Hikasa,           !
!                 Nucl. Phys. B282 (1987) 253.                         !
! INPUT: s       - center mass energy squared (in GeV**2)              !
!        amW     - W-boson mass                                        !
!        q1(4)   - four-momentum of W-                                 !
!        eps1(4) - polarizarion vector of W-                           !
!        q2(4)   - four-momentum of W+                                 !
!        eps1(4) - polarizarion vector of W+                           !
! OUTPUT: GamV(4,2) - 2 complex number four-vectors Gamma_V^mu:        !
!                     GamV(4,1) for WWgamma vertex,                    !
!                     GamV(4,2) for WWZ vertex.                        !
!----------------------------------------------------------------------! 
! Written by: Wieslaw Placzek                 Knoxville, November 1995 !
! Last update: 15.11.1995            by: W.P.                          !
!----------------------------------------------------------------------!
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      DOUBLE COMPLEX zi
      PARAMETER ( zi = (0d0,1d0) )
      DOUBLE PRECISION q1(4),eps1(4),q2(4),eps2(4)
      DOUBLE COMPLEX GamV(4,2)
      COMMON / ancoco / g1(2),kap(2),lam(2),g4(2),g5(2),kapt(2),lamt(2)
      DOUBLE COMPLEX g1,kap,lam,g4,g5,kapt,lamt
      SAVE  / ancoco /
! 7 form factors f_i^V
      DOUBLE COMPLEX f1(2),f2(2),f3(2),f4(2),f5(2),f6(2),f7(2)
      DOUBLE PRECISION P(4),Q(4),Peps(4),Qeps(4)
!
      amW2 = amW**2
! Set up form factors
      DO i = 1,2
         f1(i) = g1(i) + s/(2*amW2) *lam(i)
         f2(i) = lam(i)
         f3(i) = g1(i) + kap(i) + lam(i)
         f4(i) = g4(i)
         f5(i) = g5(i)
         f6(i) = kapt(i) - lamt(i)
         f7(i) = -0.5*lamt(i)
      ENDDO
! Sum and difference of W's 4-momenta
      DO k = 1,4
         P(k) = q1(k) + q2(k)
         Q(k) = q1(k) - q2(k)
      ENDDO
! Scalar products of various 4-vectors
      e1e2 = prodm(eps1,eps2)
      Pe1  = prodm(P,eps1)
      Pe2  = prodm(P,eps2)
! Coefficients for f5 and f6 
      CALL epsabc(P,eps1,eps2,Peps)
      CALL epsabc(Q,eps1,eps2,Qeps)
! Coefficient for f7
      PQeps = prodm(P,Qeps)
! Calculate Gamma_V^mu
      DO i = 1,2
        DO k = 1,4
          GamV(k,i) = ( f1(i)*e1e2 - f2(i)/amW2*Pe1*Pe2 
     &                - f7(i)/amW2*PQeps )*Q(k)
     &              + (-f3(i) + zi*f4(i) )*Pe2*eps1(k)   
     &              + ( f3(i) + zi*f4(i) )*Pe1*eps2(k)   
     &              + zi*f5(i)*Qeps(k) - f6(i)*Peps(k)
        ENDDO
      ENDDO
      END

      subroutine epsabc(a,b,c,q)
*     **************************
!----------------------------------------------------------------------!
! This routine calculates four-vector q according to the formula:      !
!                                                                      !
!  q^mu = epsilon^{mu,alpha,beta,gamma} a_alpha b_beta c_gamma,        !
!                                                                      !
! where epsilon is a totally antisymmetric tensor in Bjorken & Drell   !
! convention, and a, b, c are four-vectors.                            !
!----------------------------------------------------------------------! 
! Written by: Wieslaw Placzek                 Knoxville, November 1995 !
! Last update: 16.11.1995            by: W.P.                          !
!----------------------------------------------------------------------!
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      DOUBLE PRECISION a(4),b(4),c(4),q(4)
!
      b1c2 = b(1)*c(2) - b(2)*c(1)
      b1c4 = b(1)*c(4) - b(4)*c(1)
      b2c3 = b(2)*c(3) - b(3)*c(2)
      b2c4 = b(2)*c(4) - b(4)*c(2)
      b3c1 = b(3)*c(1) - b(1)*c(3)
      b3c4 = b(3)*c(4) - b(4)*c(3)
! 4-vector q^mu
      q(1) = a(2)*b3c4 - a(3)*b2c4 + a(4)*b2c3
      q(2) =-a(1)*b3c4 + a(3)*b1c4 + a(4)*b3c1
      q(3) = a(1)*b2c4 - a(2)*b1c4 + a(4)*b1c2  
      q(4) = a(1)*b2c3 + a(2)*b3c1 + a(3)*b1c2
      END


!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! UNUSED !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

      subroutine setacc_UNUSED
*     *****************
!----------------------------------------------------------------------!
! Setting up the anomalous couplings constants as given in the paper:  !
!     K. Hagiwara, R.D. Peccei, D. Zeppenfeld and K. Hikasa,           !
!                 Nucl. Phys. B282 (1987) 253.                         !
! Note: This subroutine has to be called prior first use of the        !
! subroutine WWamgc.                                                   !
! The variables used in this routine correspond to the following       !
! contants defined in the above paper:                                 !
!           constant name     corresponding variable                   ! 
!                g_1^V                g1(2)                            !
!                kappa_V              kap(2)                           !
!                lambda_V             lam(2)                           !
!                g_4^V                g4(2)                            !
!                g_5^V                g5(2)                            !
!                kappa-tilde_V        kapt(2)                          !
!                lambda-tilde_V       lamt(2)                          ! 
!----------------------------------------------------------------------! 
! Written by: Wieslaw Placzek                 Knoxville, November 1995 !
! Last update: 14.11.1995            by: W.P.                          !
!----------------------------------------------------------------------!
      COMMON / ancoco / g1(2),kap(2),lam(2),g4(2),g5(2),kapt(2),lamt(2)
      DOUBLE COMPLEX g1,kap,lam,g4,g5,kapt,lamt
      SAVE  / ancoco /
! Set up the constants (within SM: g1=kap=1, lam=g4=g5=kapt=lamt=0):
! 1) for WWgamma vertex
      g1(1)   = (1.0, 0.0)
      kap(1)  = (1.0, 0.0)
      lam(1)  = (0.0, 0.0)
      g4(1)   = (0.0, 0.0)
      g5(1)   = (0.0, 0.0)
      kapt(1) = (0.0, 0.0)
      lamt(1) = (0.0, 0.0)
! 2) for WWZ vertex
      g1(2)   = (1.0, 0.0)
      kap(2)  = (1.0, 0.0)
      lam(2)  = (0.0, 0.0)
      g4(2)   = (0.0, 0.0)
      g5(2)   = (0.0, 0.0)
      kapt(2) = (0.0, 0.0)
      lamt(2) = (0.0, 0.0)
      END

      subroutine kinold_unused(bp1,bp2,bp3,bp4,
     $  s1,s2,costhe,phi,cosde1,phi1,cosde2,phi2)
!  zw 17.06.96 wrong arguments were used   ctn,fin,ct1n,fi1n,ct2n,fi2n)
! this routine sets back principal angular variables for matrix element.
! matrix element is not calculated so far from born-like 4-momenta, 
! but from these angles. 
      implicit DOUBLE PRECISION (a-h,o-z)
      dimension bp1(4),bp2(4),bp3(4),bp4(4)

      call invkin(ctn,fin,ct1n,fi1n,ct2n,fi2n,
     $                  amwmn,amwpn,  bp1,bp2,bp3,bp4)
      s1=amwmn**2
      s2=amwpn**2
      costhe=ctn
      phi=fin
      cosde1=ct1n
! zw 17.06.96 next line was phi2.
      phi1=fi1n
      cosde2=ct2n
      phi2=fi2n

      end

      function bornex(s)
*     *********************************
! exact, on-shell born(s)
      implicit DOUBLE PRECISION (a-h,o-z)
      common / matpar / pi,ceuler     
      common / phypar / alfinv,gpicob     
      common / weking / ene,amaz,gammz,amel,amfin,xk0,sinw2,ide,idf 
      common / wekin2 / amaw,gammw,gmu,alphaw   
      save   / weking /,/ wekin2 /
      save
   
      IF(s.le.4*amaw**2) THEN
        bornex=0d0
        return
      ENDIF
       
      ams= amaw**2/s
      bet=sqrt(1-4*ams)
      cc1=amaz**2*(1-2*sinw2)/(s-amaz**2)
      cc2=amaz**4*(8*sinw2**2-4*sinw2+1)*bet**2/(s-amaz**2)**2/48d0

      sigma=  (1+2*ams+2*ams**2)/bet*log((1+bet)/(1-bet)) -5d0/4d0
     @       +cc1*( 2*(ams**2+2*ams)/bet*log((1+bet)/(1-bet))
     @             -1/ams/12d0 -5d0/3d0 -ams )
     @       +cc2*( 1/ams**2 +20/ams +12)

      bornex=pi/alfinv**2*bet/2d0/sinw2**2/s *gpicob
      end


      FUNCTION BORNKD_unused(ep1,ep2,ep3,ep4)
*******************************************************
! ep-i are massive 4momenta of produced fermions in cms-eff
! buffor routine, in order to include additional effects in born
! (QCD, Coulomb) as well as total normalisation
      IMPLICIT DOUBLE PRECISION (A-H,O-Z) 
      common / phypar / alfinv,gpicob     

      dimension ep1(4),ep2(4),ep3(4),ep4(4)
      dimension bq1(4),bq2(4)

!      born=wwborn_massive(ep1,ep2,ep3,ep4)
! wwborn is massless strictly speaking, but it works for massive 
! 4vects as well and the buffor routine wwborn_massive is in fact
! redundant, m.s. 

      born=wwborn(ep1,ep2,ep3,ep4,keyac)

! invariants for normalisation
      DO i=1,4
        bq1(i)=ep1(i)+ep2(i)
        bq2(i)=ep3(i)+ep4(i)
      ENDDO

cc      s1=dmas2(bq1)
cc      s2=dmas2(bq2)
      sprim=(bq1(4)+bq2(4))**2

C now include born level flux factor1/2s', spin average factor 1/4
      BORN=1D0/(2D0*SPRIM)*(1D0/4D0)*BORN
! picobarns
      born=born*gpicob
!-- Coulomb corr.
c moved to karlud 11/7      cc=CulMC(sprim,s1,s2)
c moved to karlud 11/7      BORN=BORN*CC
!-- Naive QCD to be added here.....

      bornkd=born
      BORNKD_unused=bornkd
      END 


      FUNCTION WWBORN(ep1,ep2,ep3,ep4,keyacc_lcl)
*********************************************************
! ep-i are 4momenta of produced fermions in cms-eff
! this routine converts massive 4vects into massless ones 
! and calls the massless WWBORN
! also sets born to 1 on request (key4f=2)
      IMPLICIT DOUBLE PRECISION (A-H,O-Z) 
! This common can be everywhere, contains various switches
      COMMON / KeyKey/  KeyRad,KeyPhy,KeyTek,KeyMis,KeyDwm,KeyDwp
      SAVE /keykey/           

      dimension ep1(4),ep2(4),ep3(4),ep4(4)
      dimension boq1(4),boq2(4),bop1(4),bop2(4),bop3(4),bop4(4)
      dimension amdec0(4)
      dimension qeff1(4),qeff2(4)
      dimension bq1(4),bq2(4)

      KeyRed = MOD(KeyPhy,100000)/10000
      Key4f  = MOD(KeyMis,100)/10
      idebug=0

      DO i=1,4
        bq1(i)=ep1(i)+ep2(i)
        bq2(i)=ep3(i)+ep4(i)
      ENDDO

      IF(key4f.EQ.2) THEN
!     1=================1
!-- Born suppressed (in case of external Matr el.)
        IF(idebug.eq.1) THEN
          write(6,*)'born_massive suppressed, keyred=',keyred
        ENDIF
        born=1d0
      ELSE
!     1=================1
!-- standard matrix element squared
        IF(idebug.eq.1) THEN
          write(6,*)'born_massive dumps before, keyred=',keyred
          write(6,*)'born_massive dumps before, sq(sprim)=',sqrt(sprim)
          call dumpl(6,eP1,eP2,eP3,eP4,QEFF1,QEFF2,SPHOT,nphot)
        ENDIF
c.. make decay products massless
        IF(KeyRed.eq.0) THEN
!       2==================2
c.. sophisticated fermion mass reduction to 0
! re-construct angles
          call invkin(costhe,phi,cosde1,phi1,cosde2,phi2,
     $                  amwmn,amwpn,  ep1,ep2,ep3,ep4)

! make sure it is OK
          qq=dsqrt(bq1(1)**2+bq1(2)**2+bq1(3)**2)
          ctn=bq1(3)/qq
          IF(abs(ctn/costhe-1) .GT. 1d-14) THEN
            WRITE(6,*)'cosinusy ',ctn/costhe
          ENDIF
          sprim=(bq1(4)+bq2(4))**2
          
          DO i=1,4
            amdec0(i)=0d0
          ENDDO
! build again 4vects with 0 masses
          call kineww(sprim,costhe,phi,cosde1,phi1,cosde2,phi2,
     $    amwmn,amwpn,amdec0,boq1,boq2,bop1,bop2,bop3,bop4)
        ELSEIF(KeyRed.eq.1) THEN
!       2======================2
c.. brute force fermion mass reduction to 0, no 4 mom conserv.
          bp1mod=ep1(4)/dsqrt(ep1(1)**2+ep1(2)**2+ep1(3)**2)
          bp2mod=ep2(4)/dsqrt(ep2(1)**2+ep2(2)**2+ep2(3)**2)
          bp3mod=ep3(4)/dsqrt(ep3(1)**2+ep3(2)**2+ep3(3)**2)
          bp4mod=ep4(4)/dsqrt(ep4(1)**2+ep4(2)**2+ep4(3)**2)
          do i=1,3
            bop1(i)=ep1(i)*bp1mod !*(1-1d-15)
            bop2(i)=ep2(i)*bp2mod !*(1-1d-15)
            bop3(i)=ep3(i)*bp3mod !*(1-1d-15)
            bop4(i)=ep4(i)*bp4mod !*(1-1d-15)
          enddo
          bop1(4)=ep1(4)
          bop2(4)=ep2(4)
          bop3(4)=ep3(4)
          bop4(4)=ep4(4)
        ELSEIF(KeyRed.eq.2) THEN
!       2======================2
c.. NO reduction at all...
          DO i=1,4
            bop1(i)=ep1(i)
            bop2(i)=ep2(i)
            bop3(i)=ep3(i)
            bop4(i)=ep4(i)
          ENDDO
        ELSE
!       2================2
          write(6,*)'born_massive==>wrong KEYRED:',keyred
          stop
        ENDIF
!       2================2
        IF(idebug.eq.1) THEN
          write(6,*)'born_massive dumps after'
          call dumpl(6,boP1,boP2,boP3,boP4,QEFF1,QEFF2,SPHOT,nphot)
          write(6,*)'masses',sqrt(dmas2(bop1)),sqrt(dmas2(bop2)),
     $                       sqrt(dmas2(bop3)),sqrt(dmas2(bop4))
        ENDIF
        BORN= WWBORN_massless(bop1,bop2,bop3,bop4,keyacc_lcl)
      ENDIF
!     1=================1

      wwborn=born
      END 



