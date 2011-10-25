      SUBROUTINE MODEL1(MODE)
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
!       to formulae obtained by the authors (see routine xmatel).      !
!                                                                      !
!======================================================================!
!    Written by: Wieslaw Placzek                  Knoxville, May 1995  ! 
!    Last update: 08.12.2010       by: W.P.                            !
!----------------------------------------------------------------------!  
      IMPLICIT REAL*8(A-H,O-Z)
      PARAMETER( pi = 3.1415926535897932d0, alfinv=137.0359895d0)
      PARAMETER( alfpi=  1/pi/alfinv, alfa= 1d0/alfinv)
      PARAMETER( Gnanob=389.385D3 )
      COMMON / BHPAR1 / DEL,EPSCM,THMIN,XMIVIS
      COMMON / BHPAR2 / CMSENE,AMEL
      COMMON / BHPAR3 / KEYRAD,KEYOPT
      COMMON / BHCRUD / trmid,crufla,Zprof,sg01,sg02,sg03,sig0
      COMMON / TRANSR / TRAN,TRMIN,TRMAX
      COMMON / MOMSET / P1(4),Q1(4),P2(4),Q2(4),PHOT(100,4),NPHOT
      COMMON / WGTALL / WTMOD,WTCRUD,WTTRIG,WTSET(300)
!WP: photon radiation tags
      Common / RadTag / idrad(100)
      REAL*8 p1r0(4),q1r0(4),p2r0(4),q2r0(4)
      REAL*8 p1r(4),q1r(4),p2r(4),q2r(4),pk(4),pkr(4)
      REAL*8 p1e(4),q1e(4),qq(4),pp(4)
! Switch for some diagnostic printouts: 0/1 (OFF/ON)
      DATA KeyDia /1/
! Count bad weights if appear
      DATA ibadw0 /0/, ibadw1 /0/, ibadw2 /0/
      DATA icont /0/
      SAVE
!---
      icont=icont+1
      eps = EPSCM   
      ame = AMEL 
      s   = CMSENE**2
      t   =-TRAN
      u   = 4*ame**2 -s -t
      zeta = TRAN/s
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
        IF (KeyEWC.EQ.1) THEN
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
            call REDUZ1(qq,p1,q1,pk,p1r,q1r,pkr)
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
        thp1k = angvec(p1r,pkr)
        thq1k = angvec(q1r,pkr)
        thp2k = angvec(p2r,pkr)
        thq2k = angvec(q2r,pkr)
        delth = 1d-7
! Collinear approx. if angle between lepton and photon less than delth 
        IF (thp1k.LT.delth) THEN
          dis10 = 0
          z = pkr(4)/p1r(4)
!WP          beta1i = beta00 *z*(z-2)/2
          beta1i = beta00 *z**2/2/(1-z)
        ELSEIF (thq1k.LT.delth) THEN
          dis10 = 0
          z = pkr(4)/q1r(4)
!WP          beta1i = beta00 *z*(z-2)/2 
          beta1i = beta00 *z**2/2/(1-z)
        ELSEIF (thp2k.LT.delth) THEN
          dis10 = 0
          z = pkr(4)/(p2r(4)+pkr(4))
!WP          beta1i = beta00 *z*(z-2)/2
          beta1i = beta00 *z**2/2/(1-z)
        ELSEIF (thq2k.LT.delth) THEN
          dis10 = 0
          z = pkr(4)/(q2r(4)+pkr(4))
!WP          beta1i = beta00 *z*(z-2)/2
          beta1i = beta00 *z**2/2/(1-z)
        ELSE
! Soft factor 
          sfr = SoftFa(p1r,q1r,p2r,q2r,pkr)
! Single hard photon bremss. matrix element 
          dis10 = xmatel(p1r,q1r,p2r,q2r,pkr) /( 4*(4*pi*alfa)**3 )
! beta1 O(alpha1)
          beta1i = dis10/sfr - beta00
        ENDIF
! Sum of beta1 for all photons
        beta10 = beta10 + beta1i
      ENDDO
!
!          **************************************
!          **     Definitions of MC weights    **
!          **************************************
! All beta's:  O(alf0),O(alf1)
      wtset(10) =  beta00           /crude
      wtset(11) = (beta01 + beta10) /crude
!-> Pure QED corrections
      IF (KeyLib.EQ.2 .AND. KeyEWC.EQ.1)
     &     wtset(31) = (beta01qed + beta10)/crude
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
        dsne1 = dis10 /sfr
! YFS formfactor 
        fYFS = YFSfmf(eps) 
      ENDIF
!
!          **************************************
!          ****   Definitions of MC weights  ****
!          ****    Non-exponentiated case    ****
!          **************************************
! TOTAL O(alpha^0),O(alpha^1)
      wtset(20) = dsne0 /fYFS/crude
      wtset(21) = dsne1 /fYFS/crude
!
!---------------------------------------------------------------!
! Setting some "bad" weights to zero!
      wtx1 =-10.0
      wtx2 = 50.0
      wt0 = wtcrud*wttrig*wtset(10)
! This is less dangerous
      IF (wt0.lt.wtx1 .or. wt0.gt.wtx2) THEN
        ibadw0 = ibadw0 + 1
        IF (KeyDia.eq.1) THEN
          IObad = 6 
          WRITE(IObad,*)' '
          WRITE(IObad,*)'>>> BHWIDE WARNING !!! <<<'
          WRITE(IObad,*)'>>> MODEL1: BAD weight wt0 -> reset to zero!'
          WRITE(IObad,*)'iwt0,wt0=',ibadw0,wt0
          WRITE(IObad,*)'s,s1=',s,s1
          WRITE(IObad,*)'t,t1=',t,t1
          WRITE(IObad,*)'idrad=',(idrad(i),i=1,nphot)
          CALL dumps(IObad)
        ENDIF
        wtset(10) = 0
      ENDIF
      wt1 = wtcrud*wttrig*wtset(11)
! This is more dangerous
      IF (wt1.lt.wtx1 .or. wt1.gt.wtx2) THEN
        ibadw1 = ibadw1 + 1
        IF (KeyDia.eq.1) THEN
          IObad = 6
          WRITE(IObad,*)' '
          WRITE(IObad,*)'>>> BHWIDE WARNING !!! <<<'
          WRITE(IObad,*)'>>> MODEL1: BAD weight wt1 -> reset to zero!'
          WRITE(IObad,*)'iwt1,wt1=',ibadw1,wt1
          WRITE(IObad,*)'s,s1=',s,s1
          WRITE(IObad,*)'t,t1=',t,t1
          WRITE(IObad,*)'idrad=',(idrad(i),i=1,nphot)
          CALL dumps(IObad)
        ENDIF
        wtset(11) = 0
      ENDIF  
!+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++!
!+++++++++++++++++++ PRINCIPAL WEIGHT ++++++++++++++++++++++++++!
!+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++!
      wtset( 1) = wtset(11)
      END

      FUNCTION angvec(p,q)
*     ********************
!---------------------------------------------------------------!
! This function provides a value of an agle between 2 vectors.  !
! INPUT:  p(4),q(4) - two 4-vectors                             !
!---------------------------------------------------------------!
! Written by: Wieslaw Placzek              Knoxville, May 1995  !
! Last update: 19.07.1995       by: W.P.                        !
!---------------------------------------------------------------!
      IMPLICIT REAL*8 (a-h,o-z)
      REAL*8 p(4),q(4)
!
      pm = SQRT( p(1)**2 + p(2)**2 + p(3)**2 )
      qm = SQRT( q(1)**2 + q(2)**2 + q(3)**2 )
      pq = p(1)*q(1) + p(2)*q(2) + p(3)*q(3)
      costhe = pq/(pm*qm)
      IF (costhe.LT.-1d0) costhe =-1d0
      IF (costhe.GT. 1d0) costhe = 1d0
      angvec = ACOS(costhe)
      END      

      function SoftFa(p1,q1,p2,q2,pk)
*     *******************************
!---------------------------------------------------------------!
! This function provides a value of the soft factor S-tilde     !
! for massless fermions (factor (alpha/4pi^2) omitted!).        !
! INPUT:  p1(4),q1(4),p2(4),q2(4) - fermion 4-momenta           !
!                           pk(4) - photon  4-momentum          !
!---------------------------------------------------------------!
! Written by: Wieslaw Placzek              Knoxville, May 1995  !
! Last update: 23.05.1995       by: W.P.                        !
!---------------------------------------------------------------!
      implicit REAL*8 (a-h,o-z)
      COMMON / BHPAR2 / CMSENE,AMEL
      SAVE   / BHPAR2 /
      REAL*8 p1(4) ,q1(4) ,p2(4) ,q2(4) ,pk(4)
      REAL*8 p1r(4),q1r(4),p2r(4),q2r(4),pkr(4)
!
      ame = AMEL
! Scalar products of massive fermion 4-momenta
      p1km = p1(4)*pk(4)-p1(3)*pk(3)-p1(2)*pk(2)-p1(1)*pk(1)
      q1km = q1(4)*pk(4)-q1(3)*pk(3)-q1(2)*pk(2)-q1(1)*pk(1)
      p2km = p2(4)*pk(4)-p2(3)*pk(3)-p2(2)*pk(2)-p2(1)*pk(1)
      q2km = q2(4)*pk(4)-q2(3)*pk(3)-q2(2)*pk(2)-q2(1)*pk(1)
! Rescaling 4-momenta of all particles for the massless limit
      pmod = SQRT(p2(1)**2 +p2(2)**2 +p2(3)**2)
      qmod = SQRT(q2(1)**2 +q2(2)**2 +q2(3)**2) 
      refa = ( pmod + qmod + pk(4) )/( p1(4) + q1(4) )
      DO i = 1,4
         p1r(i) = p1(i)
         q1r(i) = q1(i)
         p2r(i) = p2(i)/refa
         q2r(i) = q2(i)/refa
         pkr(i) = pk(i)/refa
      ENDDO
      p1r(3) = p1r(4)
      q1r(3) =-q1r(4)
      p2r(4) = pmod/refa
      q2r(4) = qmod/refa
! Scalar products of massless fermion 4-momenta
      p1k  = p1r(4)*pkr(4)-p1r(3)*pkr(3)-p1r(2)*pkr(2)-p1r(1)*pkr(1)
      q1k  = q1r(4)*pkr(4)-q1r(3)*pkr(3)-q1r(2)*pkr(2)-q1r(1)*pkr(1)
      p2k  = p2r(4)*pkr(4)-p2r(3)*pkr(3)-p2r(2)*pkr(2)-p2r(1)*pkr(1)
      q2k  = q2r(4)*pkr(4)-q2r(3)*pkr(3)-q2r(2)*pkr(2)-q2r(1)*pkr(1)
      p1q1 = p1r(4)*q1r(4)-p1r(3)*q1r(3)-p1r(2)*q1r(2)-p1r(1)*q1r(1)
      p1p2 = p1r(4)*p2r(4)-p1r(3)*p2r(3)-p1r(2)*p2r(2)-p1r(1)*p2r(1)
      p1q2 = p1r(4)*q2r(4)-p1r(3)*q2r(3)-p1r(2)*q2r(2)-p1r(1)*q2r(1)
      q1p2 = q1r(4)*p2r(4)-q1r(3)*p2r(3)-q1r(2)*p2r(2)-q1r(1)*p2r(1)
      q1q2 = q1r(4)*q2r(4)-q1r(3)*q2r(3)-q1r(2)*q2r(2)-q1r(1)*q2r(1)
      p2q2 = p2r(4)*q2r(4)-p2r(3)*q2r(3)-p2r(2)*q2r(2)-p2r(1)*q2r(1)
! Massless terms 
      sfml = 2*( p1p2/(p1k*p2k) +p1q1/(p1k*q1k) -p1q2/(p1k*q2k)
     &          -q1p2/(q1k*p2k) +p2q2/(p2k*q2k) +q1q2/(q1k*q2k) )
! Mass terms
      sfmt = ame**2 *(1/p1km**2 +1/q1km**2 +1/p2km**2 +1/q2km**2)
      SoftFa = sfml - sfmt
      END

      SUBROUTINE REDUZ0(QQ,P1,P2,PR1,PR2) 
*     *********************************** 
!---------------------------------------------------------------!
! This routine comes originally from the program YFS3.          !
! Modified by: W. Placzek        Knoxville, May 1995            !
!---------------------------------------------------------------!
C reduction of momenta for beta0, second one                 
C I.E. WE MAPP:   P1,P2 ==> PR1,PR2       
C such that  PR1+PR2 = QQ                 
C Resulting PRi QRi are in QQ rest frame. 
C     *********************************** 
      IMPLICIT DOUBLE PRECISION (A-H,O-Z) 
      PARAMETER( EPS1 =1D-15)                  
      DIMENSION QQ(4),P1(4),P2(4),PR1(4),PR2(4)              
      DIMENSION PP(4),PX1(4),PX2(4),PPX(4)                   
C   
      DO 20 K=1,4      
 20   PP(K)=P1(K)+P2(K)                   
      IF((PP(1)**2+PP(2)**2+PP(3)**2)/PP(4)**2 .GT. EPS1) THEN 
C transform all momenta to QQ rest-frame  
         CALL BOSTDQ( 1,QQ,P1 ,PX1)       
         CALL BOSTDQ( 1,QQ,P2 ,PX2)       
         CALL BOSTDQ( 1,QQ,PP ,PPX)       
C transform all momenta to PP rest-frame  
         CALL BOSTDQ( 1,PPX,PX1,PX1)      
         CALL BOSTDQ( 1,PPX,PX2,PX2)      
!WP      CALL BOSTDQ( 1,PP,P1,PX1)      
!WP      CALL BOSTDQ( 1,PP,P2,PX2)      
      ELSE             
C do nothing if we are already in PP rest-frame              
         DO 23 K=1,4   
            PX1(K)=P1(K)                  
   23       PX2(K)=P2(K)                  
      ENDIF            
C construct reduced beam momenta PR1,PR2  
C note: they are understood to be in QQ rest-frame           
      SVAR1 = QQ(4)**2-QQ(3)**2-QQ(2)**2-QQ(1)**2            
      SVAR  = PP(4)**2-PP(3)**2-PP(2)**2-PP(1)**2            
      VV    = 1D0 -SVAR1/SVAR             
      IF(ABS(VV).GT. EPS1) THEN           
         AMEL2=  P1(4)**2-P1(3)**2-P1(2)**2-P1(1)**2         
         PR1(4)= SQRT(SVAR1)/2D0          
         PR2(4)= PR1(4)                   
         PXMOD = SQRT(PX1(1)**2+PX1(2)**2+PX1(3)**2)         
         PRMOD = SQRT(PR1(4)**2-AMEL2)    
         DO 30 K=1,3   
         PR1(K)= PX1(K)/PXMOD*PRMOD       
 30      PR2(K)= PX2(K)/PXMOD*PRMOD       
      ELSE             
         DO 40 K=1,4   
         PR1(K)= PX1(K)                   
 40      PR2(K)= PX2(K)                   
      ENDIF            
      END              
  
      SUBROUTINE REDUZ1(QQ,P1,P2,PH,PR1,PR2,PHR)             
*     ****************************************** 
!---------------------------------------------------------------!
! This routine comes originally from the program YFS3.          !
! Modified by: W. Placzek        Knoxville, May 1995            !
!---------------------------------------------------------------!           
C reduction of 4-momenta for beta1        
C           P1,P2,PH ==--> PR1,PR2,PHR    
C such that  PR1+PR2 = QQ+PHR             
C     *********************************** 
      IMPLICIT DOUBLE PRECISION (A-H,O-Z) 
      PARAMETER( EPS1 =1D-15)                   
      COMMON / INOUT  / NINP,NOUT   
      SAVE   / INOUT  /
      DIMENSION QQ(4), P1(4), P2(4), PH(4), PR1(4),PR2(4),PHR(4)
      DIMENSION PP(4),QQK(4),PPX(4), PPK(4)                  
      DIMENSION PX1(4),PX2(4),PHX(4)      
C   
      DO 20 K=1,4      
      PP(K)   = P1(K)+P2(K)               
      PPK(K)  = P1(K)+P2(K)-PH(K)         
 20   QQK(K)  = QQ(K)+PH(K)               
      SVAR  =  PP(4)**2 -PP(3)**2 -PP(2)**2 -PP(1)**2        
      SVAR1 =  QQ(4)**2 -QQ(3)**2 -QQ(2)**2 -QQ(1)**2        
      SS1   = PPK(4)**2-PPK(3)**2-PPK(2)**2-PPK(1)**2        
      SS2   = QQK(4)**2-QQK(3)**2-QQK(2)**2-QQK(1)**2        
      IF((PP(1)**2+PP(2)**2+PP(3)**2)/PP(4)**2 .GT. EPS1) THEN
C transform all momenta to QQ rest-frame  
         CALL BOSTDQ( 1,QQ,P1 ,PX1)       
         CALL BOSTDQ( 1,QQ,P2 ,PX2)       
         CALL BOSTDQ( 1,QQ,PH ,PHX)       
         CALL BOSTDQ( 1,QQ,PP ,PPX)       
C transform all momenta to PP rest-frame  
         CALL BOSTDQ( 1,PPX,PX1,PX1)      
         CALL BOSTDQ( 1,PPX,PX2,PX2)      
         CALL BOSTDQ( 1,PPX,PHX,PHX)      
!WP      CALL BOSTDQ( 1,PP,P1,PX1)      
!WP      CALL BOSTDQ( 1,PP,P2,PX2)      
!WP      CALL BOSTDQ( 1,PP,PH,PHX)      
      ELSE             
C do nothing if we are already in PP rest-frame              
         DO 23 K=1,4   
            PHX(K)=PH(K)                  
            PX1(K)=P1(K)                  
   23       PX2(K)=P2(K)                  
      ENDIF            
C construct reduced beam momenta PR1,PR2  
C note: they are understood to be in QQ rest-frame           
      VV2   = 1D0 - SS2/SVAR              
      IF(ABS(VV2).GT. EPS1) THEN          
CCCCC    PK    =  (PX1(4)+PX2(4))*PHX(4)  
CCCCC    XLAM= SQRT(SVAR1/SVAR+(PK/SVAR)**2)+PK/SVAR         
         XLAM= SQRT(SVAR1/SS1)            
         AMEL2=  P1(4)**2-P1(3)**2-P1(2)**2-P1(1)**2         
         PXMOD = SQRT(PX1(1)**2+PX1(2)**2+PX1(3)**2)         
         PX1(4)= PX1(4)*XLAM              
         PX2(4)= PX2(4)*XLAM              
CCC      PRMOD = SQRT(PX1(4)**2-AMEL2)    
         PRMOD =      PX1(4)**2-AMEL2     
         IF(PRMOD.LE.0D0) WRITE(NOUT,*) ' REDUZ1: PRMOD=', PRMOD
         IF(PRMOD.LE.0D0) WRITE(   6,*) ' REDUZ1: PRMOD=', PRMOD
         PRMOD = SQRT(ABS(PRMOD))         
         DO 30 K=1,3   
         PX1(K)= PX1(K)/PXMOD*PRMOD       
 30      PX2(K)= PX2(K)/PXMOD*PRMOD       
         DO 31 K=1,4   
 31      PHX(K)= PHX(K)*XLAM              
      ENDIF            
C then, boost away the three-vector part of P1+P2-PH         
C that is transform to QQ rest frame      
      DO 35 K=1,4      
 35   PP(K)= PX1(K)+PX2(K)-PHX(K)         
      CALL BOSTDQ( 1,PP,PX1,PR1)          
      CALL BOSTDQ( 1,PP,PX2,PR2)          
      CALL BOSTDQ( 1,PP,PHX,PHR)          
      END 
             
      SUBROUTINE boost5(qq,p1,p2,p3,p4,p5)
*     ************************************
!---------------------------------------------------------------!
! Boost of the 4-vectors p1,p2,p3,p4,p5 to the rest frame of qq.!
!---------------------------------------------------------------!
! Written by: Wieslaw Placzek              Knoxville, May 1995  !
!---------------------------------------------------------------!
      IMPLICIT REAL*8 (a-h,o-z)
      REAL*8 qq(4),p1(4),p2(4),p3(4),p4(4),p5(4)
      CALL bostdq( 1,qq,p1,p1)
      CALL bostdq( 1,qq,p2,p2)
      CALL bostdq( 1,qq,p3,p3)
      CALL bostdq( 1,qq,p4,p4)
      CALL bostdq( 1,qq,p5,p5)
      END

      SUBROUTINE rotat5(p1,q1,p2,q2,pk)
*     *********************************
!---------------------------------------------------------------!
! Rotation of the 4-vectors p1,q1,p2,q2,pk given in the rest    ! 
! frame of p1+q1 such that the resulting vector p1 points along !
! the +z axis.                                                  !
!---------------------------------------------------------------!
! Written by: Wieslaw Placzek              Knoxville, May 1995  !
!---------------------------------------------------------------! 
      IMPLICIT REAL*8 (a-h,o-z)
      PARAMETER ( pi = 3.1415926535897932d0 )
      REAL*8 p1(4),q1(4),p2(4),q2(4),pk(4)
!
      p1m = SQRT( p1(1)**2 + p1(2)**2 + p1(3)**2 )
      p1t = p1(1)**2 + p1(2)**2 
      IF (p1t.GT.1d-20) THEN
        phi = angfix(p1(1),p1(2))
        CALL rxtod3(-phi,p2,p2)
        CALL rxtod3(-phi,q2,q2)
        CALL rxtod3(-phi,pk,pk)
      ENDIF
      the = ACOS(p1(3)/p1m) 
      CALL rxtod2(-the,p2,p2)
      CALL rxtod2(-the,q2,q2)
      CALL rxtod2(-the,pk,pk)
      p1(3) = p1m
      p1(2) = 0
      p1(1) = 0
      q1(3) =-p1m
      q1(2) = 0
      q1(1) = 0
      END
