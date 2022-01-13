      SUBROUTINE MODEL2(MODE)
!     ***********************
!======================================================================!
!                                                                      !
!    Matrix element for Bhabha scattering inluding O(alpha)            !
!    radiative corrections as defined in Yennie-Frautschi-Suura        !
!    formula by functions beta0 and beta1.                             !
!                                                                      !
!    1. Electroweak virtual and real soft photon corrections           !
!       are taken from the program BABAMC or the program               !
!       ALIBABA depending on the switch parameter KeyLib.              !
!    2. Hard photon radiation contribution is calculated according     !
!       to the matrix element given by CALKUL, F.A. Berends et al.,    !
!       Nucl. Phys. B206 (1982) 61.                                    !
!                                                                      !
!======================================================================!
!    Written by: Wieslaw Placzek                  Knoxville, July 1995 ! 
!    Last update: 08.12.2010       by: W.P.                            !
!----------------------------------------------------------------------!  
      IMPLICIT REAL*8(A-H,O-Z)
      PARAMETER( pi = 3.1415926535897932d0, alfinv=137.0359895d0)
      PARAMETER( alfpi=  1/pi/alfinv, alfa= 1d0/alfinv)
      PARAMETER( Gnanob=389.385D3 )
      COMMON / BHPAR1 / DEL,EPSCMS,THMIN,XMIVIS
      COMMON / BHPAR2 / CMSENE,AMEL
      COMMON / BHPAR3 / KEYRAD,KEYOPT
      COMMON / BHCRUD / trmid,crufla,Zprof,sg01,sg02,sg03,sig0
      COMMON / TRANSR / TRAN,TRMIN,TRMAX
      COMMON / MOMSET / P1(4),Q1(4),P2(4),Q2(4),PHOT(100,4),NPHOT
      COMMON / WGTALL / WTMOD,WTCRUD,WTTRIG,WTSET(300)
! Photon radiation tags (needed for reduction procedure)
      Common / RadTag / idrad(100)
      REAL*8 p1r0(4),q1r0(4),p2r0(4),q2r0(4)
      REAL*8 p1r(4),q1r(4),p2r(4),q2r(4),pk(4),pkr(4)
      REAL*8 p1e(4),q1e(4),qq(4),pp(4)
! Switch for some diagnostic printouts: 0/1 (OFF/ON)
      DATA KeyDia /1/
! Count bad weights if appear
      DATA ibadw0 /0/, ibadw1 /0/, ibadw2a /0/, ibadw2b /0/
      DATA icont /0/
      SAVE
!---
      icont=icont+1
      eps = EPSCMS   
      ame = AMEL 
      s   = CMSENE**2
      t   =-TRAN
      u   = 4*ame**2 -s -t
! Crude MC distribution 
      IF (TRAN.ge.trmid) THEN
        crude =  Zprof + (s/t)**2
      ELSE
! Flat distribution for very small values of TRAN
        crude = crufla
      ENDIF
! Calculate fermion effective 4-momenta at the interation point
      DO k = 1,4
        p1e(k) = p1(k)
        q1e(k) = q1(k)
      ENDDO
      DO i = 1,NPHOT
        ira = idrad(i)
        IF (ira.eq.1) THEN
          DO k =1,4
            p1e(k) = p1e(k) -PHOT(i,k)
          ENDDO
        ELSEIF (ira.eq.2) THEN
          DO k =1,4
            q1e(k) = q1e(k) -PHOT(i,k)
          ENDDO
        ENDIF
      ENDDO
      DO k = 1,4
        qq(k) = p1e(k) + q1e(k)
      ENDDO
      IF (NPHOT.eq.0) THEN
        DO k = 1,4
          p1r0(k) = p1(k)
          q1r0(k) = q1(k)
          p2r0(k) = p2(k)
          q2r0(k) = q2(k)
        ENDDO
      ELSE
! Reduction for beta0 (Born-like process)
        CALL REDUZ0(qq,p1,q1,p1r0,q1r0)
        CALL REDUZ0(qq,p2,q2,p2r0,q2r0)
      ENDIF
! Mandelstam variables
      s1 = qq(4)**2 -qq(3)**2 -qq(2)**2 -qq(1)**2
      t1 = (p1r0(4)-p2r0(4))**2 -(p1r0(3)-p2r0(3))**2
     $    -(p1r0(2)-p2r0(2))**2 -(p1r0(1)-p2r0(1))**2
      u1 = 4*ame**2 -s1 -t1
! Born matrix element
      xmebo = xmate0(s1,t1,u1)
      dis00 = xmebo /(4*pi*alfa)**2/4
! Electroweak O(alpha) virtual and real soft photon corrections:
      KeyEWC = MOD(KeyRad,10000)/1000
      KeyLib = MOD(KeyRad,1000)/100
      IF (KeyLib.EQ.2) THEN
! ... From ALIBABA
        xmqed = xmatvs(s1,t1,eps)
        alivs = xmebo + xmqed
        IF (KeyEWC .EQ. 1) THEN
          dis01qed = alivs /(4*pi*alfa)**2/4
          xweak = xmatwc(s1,t1)
          alivs = xweak + xmqed
        ENDIF
        dis01 = alivs /(4*pi*alfa)**2/4
      ELSEIF (KeyLib.EQ.1) THEN
! ... From BABAMC
!    dsig0 - Born cross section, dsig1 - O(alpha) cross section. 
        CALL babewc(s1,t1,eps,dsig0,dsig1)
        babvs = dsig1/dsig0 
        dis01 = dis00*babvs
      ENDIF
! Infrared factor
      finf = YFSirf(p1r0,q1r0,p2r0,q2r0,eps,ame)
!===================================================================
!          ########################################
!          #               beta0                  #
!          ######################################## 
      beta00 = dis00 
      beta01 = dis01  -finf*beta00 
      IF (KeyLib.EQ.2 .AND. KeyEWC.EQ.1)
     &     beta01qed = dis01qed  -finf*beta00 
!          ########################################
!          #               beta1                  # 
!          ########################################
      beta10 = 0
      DO i = 1,NPHOT
        ira = idrad(i)
        DO k = 1,4
          pk(k) = PHOT(i,k)
        ENDDO
        IF (NPHOT.eq.1) THEN
          DO k = 1,4
            p1r(k) = p1(k)
            q1r(k) = q1(k)
            p2r(k) = p2(k)
            q2r(k) = q2(k)
            pkr(k) = pk(k)
          ENDDO
        ELSE
! Initial state radiation
          IF (ira.eq.1 .or. ira.eq.2) THEN
! Reduction procedure (e+e- ---> e+e- + gamma like process)
            CALL REDUZ1(qq,p1,q1,pk,p1r,q1r,pkr)
            DO k = 1,4
              p2r(k) = p2r0(k)
              q2r(k) = q2r0(k)
              pp(k)  = p1r(k) + q1r(k)
            ENDDO
            CALL boost5(pp ,p1r,q1r,p2r,q2r,pkr)
            CALL rotat5(p1r,q1r,p2r,q2r,pkr)
! Final state radiation 
          ELSE
! Reduction procedure (e+e- ---> e+e- + gamma like process)
            DO k = 1,4
               pk(k) = -pk(k)
            ENDDO
            CALL REDUZ1(qq,p2,q2,pk,p2r,q2r,pkr)
            DO k = 1,4
              p1r(k) = p1r0(k)
              q1r(k) = q1r0(k)
              pkr(k) =-pkr(k)
            ENDDO
          ENDIF
        ENDIF
! Soft factor (for massive fermion 4-momenta) 
        sfr = SoftFm(p1r,q1r,p2r,q2r,pkr)
!Soft factor (for massless fermion 4-momenta) - for tests  
!WP        sfr = SoftFa(p1r,q1r,p2r,q2r,pkr)
! Single hard photon bremss. matrix element 
        dis10 = xmecal(p1r,q1r,p2r,q2r,pkr) /( 4*(4*pi*alfa)**3 )
! beta1 O(alpha1)
        beta1i = dis10/sfr - beta00
! Sum of beta1 for all photons
        beta10 = beta10 + beta1i
      ENDDO
!
!          **************************************
!          **     Definitions of MC weights    **
!          **************************************
! All beta's:  O(alf0),O(alf1)
      wtset(110) =  beta00           /crude
      wtset(111) = (beta01 + beta10) /crude
!-> Pure QED corrections
      IF (KeyLib.EQ.2 .AND. KeyEWC.EQ.1)
     &     wtset(131) = (beta01qed + beta10)/crude
!==================================================================
!==================================================================
!                  Non-exponentiated version                      !
!==================================================================
!==================================================================
! Entire 0,1-photon distributions
      dsne0 = 0
      dsne1 = 0
      fYFS  = 1
      IF(NPHOT.eq.0) THEN
! [0] No hard photons
! O(alpha^0,^1) entire distributions 
        dsne0 = dis00
        dsne1 = dis01
! YFS formfactor 
        fYFS = YFSfmf(eps)  
      ELSEIF(NPHOT.eq.1) THEN
! [1] One hard photon
! O(alpha^1) entire distribution
        dsne1 =  dis10 /sfr
! YFS formfactor 
        fYFS = YFSfmf(eps) 
      ENDIF
!
!          **************************************
!          ****   Definitions of MC weights  ****
!          ****    Non-exponentiated case    ****
!          **************************************
! TOTAL O(alpha^0),O(alpha^1)
      wtset(120) = dsne0 /fYFS/crude
      wtset(121) = dsne1 /fYFS/crude
!---------------------------------------------------------------!
! Setting some "bad" weights to zero!
      wtx1 =-10.0
      wtx2 = 50.0
      wt0 = wtcrud*wttrig*wtset(110)
! This is less dangerous
      IF (wt0.lt.wtx1 .or. wt0.gt.wtx2) THEN
        ibadw0 = ibadw0 + 1
        IF (KeyDia.EQ.2) THEN
          IObad = 6 
          WRITE(IObad,*)' '
          WRITE(IObad,*)'>>> BHWIDE WARNING !!! <<<'
          WRITE(IObad,*)'MODEL2: BAD weight wt0 -> reset to zero!'
          WRITE(IObad,*)'iwt0,wt0=',ibadw0,wt0
          WRITE(IObad,*)'s,s1=',s,s1
          WRITE(IObad,*)'t,t1=',t,t1
          WRITE(IObad,*)'idrad=',(idrad(i),i=1,nphot)
          CALL dumps(IObad)
        ENDIF
        wtset(110) = 0
      ENDIF
      wt1 = wtcrud*wttrig*wtset(111)
! This is more dangerous
      IF (wt1.lt.wtx1 .or. wt1.gt.wtx2) THEN
        ibadw1 = ibadw1 + 1
        IF (KeyDia.GT.0) THEN
          IObad = 6
          WRITE(IObad,*)' '
          WRITE(IObad,*)'>>> BHWIDE WARNING !!! <<<'
          WRITE(IObad,*)'>>> MODEL2: BAD weight wt1 -> reset to zero!'
          WRITE(IObad,*)'iwt1,wt1=',ibadw1,wt1
          WRITE(IObad,*)'s,s1=',s,s1
          WRITE(IObad,*)'t,t1=',t,t1
          WRITE(IObad,*)'idrad=',(idrad(i),i=1,nphot)
          WRITE(IObad,*)'beta00,beta10=',beta00,beta10
          CALL dumps(IObad)
        ENDIF
        wtset(111) = 0
      ENDIF  
!+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++!
!+++++++++++++++++++ PRINCIPAL WEIGHT ++++++++++++++++++++++++++!
!+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++!
      wtset(101) = wtset(111)
      END

      FUNCTION SoftFm(p1,q1,p2,q2,pk)
*     *******************************
!---------------------------------------------------------------!
! This function provides a value of the soft factor S-tilde     !
! for massless fermions (factor (alpha/4pi^2) omitted!).        !
! INPUT:  p1(4),q1(4),p2(4),q2(4) - fermion 4-momenta           !
!                           pk(4) - photon  4-momentum          !
!---------------------------------------------------------------!
! Written by: Wieslaw Placzek              Knoxville, May 1995  !
! Last update: 14.07.1995       by: W.P.                        !
!---------------------------------------------------------------!
      implicit REAL*8 (a-h,o-z)
      COMMON / BHPAR2 / CMSENE,AMEL
      SAVE   / BHPAR2 /
      REAL*8 p1(4) ,q1(4) ,p2(4) ,q2(4) ,pk(4)
!
      ame = AMEL
! Scalar products of various 4-momenta
      p1k = p1(4)*pk(4)-p1(3)*pk(3)-p1(2)*pk(2)-p1(1)*pk(1)
      q1k = q1(4)*pk(4)-q1(3)*pk(3)-q1(2)*pk(2)-q1(1)*pk(1)
      p2k = p2(4)*pk(4)-p2(3)*pk(3)-p2(2)*pk(2)-p2(1)*pk(1)
      q2k = q2(4)*pk(4)-q2(3)*pk(3)-q2(2)*pk(2)-q2(1)*pk(1)
      p1q1 = p1(4)*q1(4)-p1(3)*q1(3)-p1(2)*q1(2)-p1(1)*q1(1)
      p1p2 = p1(4)*p2(4)-p1(3)*p2(3)-p1(2)*p2(2)-p1(1)*p2(1)
      p1q2 = p1(4)*q2(4)-p1(3)*q2(3)-p1(2)*q2(2)-p1(1)*q2(1)
      q1p2 = q1(4)*p2(4)-q1(3)*p2(3)-q1(2)*p2(2)-q1(1)*p2(1)
      q1q2 = q1(4)*q2(4)-q1(3)*q2(3)-q1(2)*q2(2)-q1(1)*q2(1)
      p2q2 = p2(4)*q2(4)-p2(3)*q2(3)-p2(2)*q2(2)-p2(1)*q2(1)
! Soft factor
      sfml = 2*( p1p2/(p1k*p2k) +p1q1/(p1k*q1k) -p1q2/(p1k*q2k)
     &          -q1p2/(q1k*p2k) +p2q2/(p2k*q2k) +q1q2/(q1k*q2k) )
      sfmt = ame**2 *(1/p1k**2 +1/q1k**2 +1/p2k**2 +1/q2k**2)
      SoftFm = sfml - sfmt
      END
