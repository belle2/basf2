*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//                                                                                 //
*//                         Pseudo-CLASS  GPS                                       //
*//         This will become a separate class in the near future                    //
*//                                                                                 //
*//       Purpose:  Calculation of spin amplitudes using spinor methods             //
*//                                                                                 //
*//                 Library of basic tools used in CEEX.f                           //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////

 
      SUBROUTINE GPS_PhelStart(nphot,last)
*///////////////////////////////////////////////////////////////////////////////
*//                                                                           //
*//    Not Used                                                               //
*//    Initialize first photon helicity combination                           //
*//                                                                           //
*///////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GPS.h'
      INTEGER   nphot,last
      INTEGER   i
*----------  Initialize first partition -----------------
      last=0
      DO i = 1,nphot
         m_Phel(i) = 0
      ENDDO
      END

      SUBROUTINE GPS_PhelPlus(nphot,last)
*///////////////////////////////////////////////////////////////////////////////
*//                                                                           //
*//    Not Used                                                               //
*//   update m_Phel, check if it is last combination                          //
*//                                                                           //
*///////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GPS.h'
      INTEGER   nphot,last
      INTEGER   i
*
      IF(nphot .EQ. 1) last=1
      m_Phel(1)=m_Phel(1)+1       !!!
      DO i=1,nphot
         IF( m_Phel(i).EQ. 2 ) THEN
            m_Phel(i)=0          !!!
            m_Phel(i+1)=m_Phel(i+1)+1 !!!
            IF( m_Phel(nphot) .EQ. 2 ) last=2
         ENDIF
      ENDDO
      END

      SUBROUTINE GPS_PhelRandom(nphot)
*///////////////////////////////////////////////////////////////////////////////
*//                                                                           //
*//    Not Used                                                               //
*//   Generate photon helicities randomly                                     //
*//                                                                           //
*///////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GPS.h'
      INTEGER   nphot
      INTEGER   i
      REAL                 rvec(m_phmax)
* 
      CALL PseuMar_MakeVec(rvec,nphot)
      DO i=1,nphot
         IF( rvec(i) .GT. 0.5d0 ) THEN
            m_Phel(i)=0
         ELSE
            m_Phel(i)=1
         ENDIF
      ENDDO
      END

      SUBROUTINE GPS_PartitionStart(nphot,last)
*///////////////////////////////////////////////////////////////////////////////
*//   Initialize first partition                                              //
*///////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GPS.h'
      INTEGER   nphot,last
      INTEGER   i
*----------
      IF(     (m_KeyISR. EQ. 1) .AND. (m_HasFSR. EQ. 1) ) THEN
         IF(      m_KeyINT .NE. 0) THEN
*     Normal case, ISR+FSR
            DO i = 1,nphot
               m_isr(i) = 0     ! Start with all FSR
            ENDDO
            last=0              ! Run through all partitions
         ELSE
*     INTerference OFF, Copy partition from crude MC
            CALL KK2f_GetIsr(m_isr)
            last=1              ! Exit next time
         ENDIF
      ELSEIF( (m_KeyISR. EQ. 1) .AND. (m_HasFSR. EQ. 0) ) THEN
*     Special test, ISR ONLY
         DO i = 1,nphot
            m_isr(i) = 1        ! Start with all ISR
         ENDDO
         last=1                 ! Exit next time
      ELSEIF( (m_KeyISR. EQ. 0) .AND. (m_HasFSR. EQ. 1) ) THEN
*     Special test, FSR ONLY
         DO i = 1,nphot
            m_isr(i) = 0        ! Start with all FSR
         ENDDO
         last=1                 ! Exit next time
      ELSE
         WRITE(*,*) '#### GPS_PartitionStart: Wrong KeyISR,HasFSR = ',m_KeyISR,m_HasFSR
         STOP
      ENDIF
      END

      SUBROUTINE GPS_PartitionPlus(nphot,last)
*///////////////////////////////////////////////////////////////////////////////
*//                                                                           //
*//   update m_isr, check if it is last partition                             //
*//                                                                           //
*///////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GPS.h'
      INTEGER   nphot,last
      INTEGER   i
*
      IF(nphot .EQ. 1) last=1   !!! Exit next time
      m_isr(1)=m_isr(1)+1
      DO i=1,nphot
         IF( m_isr(i).EQ. 2 ) THEN
            m_isr(i)=0
            m_isr(i+1)=m_isr(i+1)+1
            IF( m_isr(nphot) .EQ. 2 ) last=2 !!! Immediate exit
         ENDIF
      ENDDO
      END


      SUBROUTINE GPS_BornZero(AmpBorn)
*//////////////////////////////////////////////////////////////////////////////////
*//   Set AmpBorn to zero                                                        //
*//////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GPS.h'
*
      DOUBLE COMPLEX  AmpBorn(2,2,2,2)
      INTEGER    j1,j2,j3,j4
*
      DO j1 = 1,2
         DO j2 = 1,2
            DO j3 = 1,2
               DO j4 = 1,2
                  AmpBorn(j1,j2,j3,j4) = DCMPLX(0d0,0d0)
               ENDDO                  
            ENDDO
         ENDDO
      ENDDO
      END                       !!!GPS_BornZero!!!

      SUBROUTINE GPS_BornCopy(AmpBorn,AmpBorn2)
*//////////////////////////////////////////////////////////////////////////////////
*//   Copy AmpBorn into AmpBorn2                                                 //
*//////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GPS.h'
*
      DOUBLE COMPLEX  AmpBorn(2,2,2,2),AmpBorn2(2,2,2,2)
      INTEGER    j1,j2,j3,j4
*
      DO j1 = 1,2
         DO j2 = 1,2
            DO j3 = 1,2
               DO j4 = 1,2
                  AmpBorn2(j1,j2,j3,j4) = AmpBorn(j1,j2,j3,j4)
               ENDDO                  
            ENDDO
         ENDDO
      ENDDO
      END                       !!!GPS_BornCopy!!!

      SUBROUTINE GPS_BornSumSq(AmpBorn,Sum)
*//////////////////////////////////////////////////////////////////////////////////
*//   Sum up Born amplitudes squared                                             //
*//////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GPS.h'
*
      DOUBLE COMPLEX  AmpBorn(2,2,2,2)
      DOUBLE PRECISION      Sum
*
      INTEGER    j1,j2,j3,j4
*----------------------------------
      Sum = 0d0
      DO j1 = 1,2
         DO j2 = 1,2
            DO j3 = 1,2
               DO j4 = 1,2
                  Sum = Sum +AmpBorn(j1,j2,j3,j4)*DCONJG(AmpBorn(j1,j2,j3,j4))
               ENDDO                  
            ENDDO
         ENDDO
      ENDDO
      END                       !!!GPS_BornSumSq!!!


      SUBROUTINE GPS_TralorPrepare(QQ,id)
*///////////////////////////////////////////////////////////////////////////////
*//                                                                           //
*//   Prepares transformation for Tralor, according to GPS rules              //
*//   The resulting Lorenz transformation matrix is stored for multiple use!  //
*//   This organization saves time in the case of multiple calls for many     //
*//   decay products.                                                         //
*//   Ident id=1,2 for two fermions only, later on may be extended.           //
*//   Note that Tralor is a good candidate for separate class!                //
*//                                                                           //
*///////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GPS.h'
      INTEGER id                ! Ident of fermion, id=1,2 for 2 fermions
      DOUBLE PRECISION   QQ(4)             ! Fermion 4-momentum in LAB
      DOUBLE PRECISION   Lorenz(4,4),Lorinv(4,4)
      DOUBLE PRECISION   Rot(4,4),   Rotinv(4,4)
      DOUBLE PRECISION   EQ,MQ,sum,a,b
      DOUBLE PRECISION   xi(4),eta(4)
      INTEGER k,l
*--------------------------------------------------
* Construct Lorenz transformation from/to QQ rest frame (Jackson style)
      CALL KinLib_DefBostQQ(-1,QQ,Lorenz) ! from ferm. rest frame to LAB
      CALL KinLib_DefBostQQ( 1,QQ,Lorinv) ! from LAB to ferm. rest frame

* Additional 3-rotation according to GPS rules
      CALL KinLib_VecTrasform(Lorinv, m_Xi,  xi)
      CALL KinLib_VecTrasform(Lorinv, m_Eta, eta)
      CALL GPS_GPS(xi,eta,Rot)
* Inverse matrix is just transpose of Rot
      CALL KinLib_LorCopy(Rot,Rotinv)
      CALL KinLib_RotTranspose(Rotinv)
* Consctruct total GPS transformation
      CALL KinLib_LorMult(Lorenz,      Rot,  Lorenz) ! towards LAB
      CALL KinLib_LorMult(Rotinv,   Lorinv,  Lorinv)
* Memorize transformation matrices for further use
      IF( id .EQ. 1) THEN
         CALL KinLib_LorCopy(Lorenz,m_Loren1) ! towards LAB
         CALL KinLib_LorCopy(Lorinv,m_Lorin1)
      ELSEIF(id .EQ. 2) THEN
         CALL KinLib_LorCopy(Lorenz,m_Loren2) ! towards LAB
         CALL KinLib_LorCopy(Lorinv,m_Lorin2)
      ELSE
         GOTO 900
      ENDIF
      RETURN
 901  WRITE(*,*) '++++++++ GPS_TralorMake: WRONG QQ, not timelike'
      STOP
 900  WRITE(*,*) '++++++++ GPS_TralorMake: WRONG id=',id
      STOP
      END


      SUBROUTINE GPS_GPS(xi,eta,Rot)
*///////////////////////////////////////////////////////////////////////////////
*//                                                                           //
*// Defines basis vectors e1,e2,e2 from xi and eta                            //
*// Columns in Rot are e1,e2,e2                                               //
*// Called in GPS_TralorPrepare                                               //
*//                                                                           //
*///////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GPS.h'
      DOUBLE PRECISION   xi(4),eta(4)
      DOUBLE PRECISION   Rot(4,4)
      DOUBLE PRECISION   e1(4),e2(4),e3(4),a
      INTEGER k
*---------------------------------------
* GPS rule 1: z-axis antiparalel to xi
      a = SQRT(xi(1)**2 +xi(2)**2 +xi(3)**2 )
      e3(1) = -xi(1)/a
      e3(2) = -xi(2)/a
      e3(3) = -xi(3)/a
      e3(4) = 0d0
* GPS rule 2: x-axis in plane (+eta, -xi),
* that is y-axis perpendicular to (eta,xi), i.e. e2 = eta X xi
      e2(1) =  eta(2)*xi(3) -eta(3)*xi(2)
      e2(2) = -eta(1)*xi(3) +eta(3)*xi(1)
      e2(3) =  eta(1)*xi(2) -eta(2)*xi(1)
      a = SQRT(e2(1)**2 +e2(2)**2 +e2(3)**2 )
      e2(1) = e2(1)/a
      e2(2) = e2(2)/a
      e2(3) = e2(3)/a
      e2(4) = 0d0
      e1(1) = e2(2)*e3(3) -e2(3)*e3(2)
      e1(2) =-e2(1)*e3(3) +e2(3)*e3(1)
      e1(3) = e2(1)*e3(2) -e2(2)*e3(1)
      e1(4) = 0d0
* Define additional rotation matrix from GPS (e1,e2,e3) frame 
* to actual fermion rest frame defined with matrix Lorenz 
      CALL KinLib_RotColumn(e1,e2,e3,Rot)
      END


      SUBROUTINE GPS_TralorDoIt(id,pp,q)
*///////////////////////////////////////////////////////////////////////////////
*//                                                                           //
*//   Transform pp from rest frame of fermion id to LAB, q is result          //
*//   It uses Lorenz transfromation prepared and memorized in                 //
*//   the subprogram GPS_TralorPrepare, which has to be called first!         //
*//   This organization saves time in the case of multiple calls for many     //
*//   decay products.                                                         //
*//                                                                           //
*///////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GPS.h'
      INTEGER id                ! Ident of fermion, id=1,2 for 2 fermions
      DOUBLE PRECISION   pp(4),q(4)        ! fermion moms to be transformed TO LAB
      INTEGER k,l
      DOUBLE PRECISION   p(4),sum
*-----------------
      DO k=1,4
         p(k)=pp(k)
      ENDDO
* Transform vector p, i.e. multiply by matrix Loren1 or Loren2
      IF( id .EQ. 1) THEN
         DO k=1,4
            sum = 0d0
            DO l=1,4
               sum=sum+ m_Loren1(k,l)*p(l)
            ENDDO
            q(k) = sum
         ENDDO
      ELSEIF( id .EQ. 2) THEN
         DO k=1,4
            sum = 0d0
            DO l=1,4
               sum=sum+ m_Loren2(k,l)*p(l)
            ENDDO
            q(k) = sum
         ENDDO
      ELSE
         GOTO 900
      ENDIF
*-----------
      RETURN
 900  WRITE(*,*) '++++++++ WRONG id in GPS_TralorDoIt =',id
      STOP
      END

      SUBROUTINE GPS_TralorUnDo(id,pp,q)
*///////////////////////////////////////////////////////////////////////////////
*//                                                                           //
*//               Inverse of GPS_TralorDoIt                                   //
*//                                                                           //
*//   Transform pp from LAB to rest frame of fermion, q is result             //
*//   It uses Lorenz transfromation prepared and memorized in                 //
*//   the subprogram GPS_TralorPrepare, which has to be called first!         //
*//                                                                           //
*///////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GPS.h'
      INTEGER id                ! Ident of fermion, id=1,2 for 2 fermions
      DOUBLE PRECISION   pp(4),q(4)        ! fermion moms to be transformed TO LAB
      INTEGER k,l
      DOUBLE PRECISION   p(4),sum
*-----------------
      DO k=1,4
         p(k)=pp(k)
      ENDDO
* Transform vector p, i.e. multiply by matrix Loren1 or Loren2
      IF( id .EQ. 1) THEN
         DO k=1,4
            sum = 0d0
            DO l=1,4
               sum=sum+ m_Lorin1(k,l)*p(l)
            ENDDO
            q(k) = sum
         ENDDO
      ELSEIF( id .EQ. 2) THEN
         DO k=1,4
            sum = 0d0
            DO l=1,4
               sum=sum+ m_Lorin2(k,l)*p(l)
            ENDDO
            q(k) = sum
         ENDDO
      ELSE
         GOTO 900
      ENDIF
*-----------
      RETURN
 900  WRITE(*,*) '++++++++ WRONG id in GPS_TralorUnDo =',id
      STOP
      END

      SUBROUTINE GPS_TraJacobWick(Mode,QQ,pp,rr)
*///////////////////////////////////////////////////////////////////////////////
*//                                                                           //
*//   Tralor-type transformation for Jacob Wick quantization axies.           //
*//   Not optimized, mainly for tests.                                        //
*//                                                                           //
*//   Mode =-1 from ferm_rest to LAB  (normal mode for KORALB Tralor)         //
*//   Mode =+1 from LAB to ferm_rest                                          //
*//                                                                           //
*///////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GPS.h'
      INTEGER Mode
      DOUBLE PRECISION   QQ(4)             ! Fermion 4-momentum in LAB
      DOUBLE PRECISION   pp(4)             ! input momentum
      DOUBLE PRECISION   rr(4)             ! transformed momentum
      DOUBLE PRECISION   MQ,pQ, exe, Theta, Phi
      DOUBLE PRECISION   Lor(4,4),Bos3(4,4),Rot2(4,4),Rot3(4,4)
      INTEGER i,j
      DOUBLE PRECISION   KinLib_AngPhi
*--------------------------------
      pQ = DSQRT(QQ(1)**2 +QQ(2)**2 +QQ(3)**2)
      MQ = QQ(4)**2-pQ**2
      IF(MQ .LE. 0d0 ) GOTO 901
      MQ  = DSQRT(MQ)
      exe   = (QQ(4)+pQ)/MQ     ! exe=exp(hiper_velocity), exe>1 for q(3)>0
* Completely standard Theta, Phi
      Theta = KinLib_AngPhi(QQ(3),DSQRT(QQ(1)**2+QQ(2)**2) ) ! range (0,pi)
      Phi   = KinLib_AngPhi(QQ(1),QQ(2))                     ! range (0,2pi)
      IF(Mode .EQ. 1) THEN
         exe   = 1d0/exe
         Theta = -Theta
         Phi   = -Phi
      ENDIF
* Define matrices for elementary boosts/rotations
      CALL KinLib_DefBoost(3,     exe,Bos3) ! Calculate Boost3 matrix
      CALL KinLib_DefRotor(3,1, Theta,Rot2) ! Rotation x-z plane, around y axis
      CALL KinLib_DefRotor(1,2,   Phi,Rot3) ! Rotation x-y plane, around z axis
* Define the Entire transformation matrix (slow but secure)
      IF(Mode .EQ. -1) THEN
*     Mode =-1 from ferm_rest to LAB  (normal mode in KORALB Tralor)
         CALL KinLib_LorCopy(Bos3,     Lor) ! Lor=Bos3(exe)
         CALL KinLib_LorMult(Rot2,Lor, Lor) ! Lor=Rot2(Theta)*Bos3(exe)
         CALL KinLib_LorMult(Rot3,Lor, Lor) ! Lor=Rot3(Phi)*Rot2(Theta)*Bos3(exe)
      ELSEIF( Mode .EQ. 1) THEN
*     Mode =+1 from LAB to ferm_rest
         CALL KinLib_LorCopy(Rot3,     Lor) ! Lor=Rot3(-Phi)
         CALL KinLib_LorMult(Rot2,Lor, Lor) ! Lor=Rot2(-Theta)*Rot3(-Phi)
         CALL KinLib_LorMult(Bos3,Lor, Lor) ! Lor=Bos3(1/exe)*Rot2(-Theta)*Rot3(-Phi)
      ELSE
         GOTO 902
      ENDIF
* Transform vector
      CALL KinLib_VecTrasform(Lor,pp,rr)
      RETURN
 901  WRITE(*,*) '++++++++GPS_TraJacobWick: WRONG QQ, not timelike'
      STOP
 902  WRITE(*,*) '++++++++GPS_TraJacobWick: WRONG Mode= ', Mode
      STOP
      END


      SUBROUTINE GPS_RmatMake
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//                                                                                 //
*//   Translates Born spin amplitudes into double spin density matrix               //
*//                m_AmpBorn  -----> R_{ab}                                         //
*//                                                                                 //
*//   Notes:                                                                        //
*//   Polarizations for beams not included but it will be strightforward...         //
*//   m_Rmat( k, l) is realy REAL, we keep it complex to be able to xcheck it.      //
*//   One should remember to use DREAL( m_Rmat( k, l) ) in the calculations!        //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GPS.h'
*
      INTEGER    k,l
      INTEGER    i1,i2,i3,i4
      INTEGER    j1,j2,j3,j4
      INTEGER    Hel1,Hel2,Hel3,Hel4
      DOUBLE COMPLEX  Sum,cx

      CALL GPS_Initialize
*
* No polarization yet for initial state spins
* Loop over 4*4*2**6=16*64=1024 indices!
* The formula is eq. (2.14) in Acta. Phys. Pol. B16 (1985) 1007.
      DO k = 0,3
         DO l = 0,3
            Sum = DCMPLX(0d0,0d0)
            DO i1 = 1,2
               DO i2 = 1,2
                  DO i3 = 1,2
                     DO i4 = 1,2
                        DO j3 = 1,2
                           DO j4 = 1,2
                              Sum= Sum+
     $                                      m_AmpBorn( i1,i2, i3,i4)
     $                             *DCONJG( m_AmpBorn( i1,i2, j3,j4) )
     $                             *m_Pauli( k,j3,i3)
     $                             *m_Pauli( l,j4,i4)
                           ENDDO
                        ENDDO
                     ENDDO
                  ENDDO
               ENDDO
            ENDDO
            m_Rmat( k, l) = Sum
         ENDDO
      ENDDO
      cx = m_Rmat( 0, 0)
      DO k = 0,3
         DO l = 0,3
            m_Rmat( k, l) = m_Rmat( k, l)/cx
         ENDDO
      ENDDO
      END                       !Rmat


      SUBROUTINE GPS_RmatMake2
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//    !!!!!!!!!!!!!!!!!!!!!!! TEST TEST !!!!!!!!!!!!!!!!!!!!!!!                    //
*//                                                                                 //
*//             m_AmpBorn2 instead of m_AmpBorn                                     //
*//                m_AmpBorn  -----> R_{ab}                                         //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GPS.h'
*
      INTEGER    k,l
      INTEGER    i1,i2,i3,i4
      INTEGER    j1,j2,j3,j4
      INTEGER    Hel1,Hel2,Hel3,Hel4
      DOUBLE COMPLEX  Sum,cx

      CALL GPS_Initialize
*
* No polarization yet for initial state spins
* Loop over 4*4*2**6=16*64=1024 indices!
* The formula is eq. (2.14) in Acta. Phys. Pol. B16 (1985) 1007.
      DO k = 0,3
         DO l = 0,3
            Sum = DCMPLX(0d0,0d0)
            DO i1 = 1,2
               DO i2 = 1,2
                  DO i3 = 1,2
                     DO i4 = 1,2
                        DO j3 = 1,2
                           DO j4 = 1,2
                              Sum= Sum+
     $                                      m_AmpBorn2( i1,i2, i3,i4)
     $                             *DCONJG( m_AmpBorn2( i1,i2, j3,j4) )
     $                             *m_Pauli( k,j3,i3)
     $                             *m_Pauli( l,j4,i4)
                           ENDDO
                        ENDDO
                     ENDDO
                  ENDDO
               ENDDO
            ENDDO
            m_Rmat( k, l) = Sum
         ENDDO
      ENDDO
      cx = m_Rmat( 0, 0)
      DO k = 0,3
         DO l = 0,3
            m_Rmat( k, l) = m_Rmat( k, l)/cx
         ENDDO
      ENDDO
      END                       !Rmat


      SUBROUTINE GPS_RmatMult(Rot1,Rot2)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//   Multiplies tensor m_Rmat (spin correlations joint density matrix)             //
*//   with two rotatios Rot1 and Rot2.                                              //
*//   Transposed Rot1 and Rot2 are realy employed.                                  //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GPS.h'
      DOUBLE PRECISION      Rot1(4,4),Rot2(4,4)
      INTEGER    i,j,k,l
      DOUBLE PRECISION      sum,sum1,Rmat(0:3,0:3)
*-----------------------------------
* Save old result
      DO i=0,3
         DO j=0,3
            Rmat(i,j)=m_Rmat(i,j)
         ENDDO
      ENDDO
* Transform first index in tensor Rmat
      DO i=1,3                  !active
         DO j=0,3               !passive
            sum  = 0d0
            DO k=1,3
               sum  = sum + Rmat(k,j) *Rot1(i,k)
            ENDDO
            m_Rmat(i,j) = sum
         ENDDO
      ENDDO
* Save partial result
      DO i=0,3
         DO j=0,3
            Rmat(i,j)=m_Rmat(i,j)
         ENDDO
      ENDDO
* Transform second index in Rmat
      DO i=0,3                  !passive
         DO j=1,3               !active
            sum  = 0d0
            DO k=1,3
               sum  = sum + Rmat(i,k) *Rot2(j,k)
            ENDDO
            m_Rmat(i,j) = sum
         ENDDO
      ENDDO
      END


      SUBROUTINE GPS_Hini(KFi,KFf,PX, p1,m1,p2,m2,p3,m3,p4,m4,ph,mph, AmHarIsr)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//   IR-finite part od 1-photon amplitudes for ISR                                 //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GPS.h'
*
      INTEGER    KFi,KFf
      DOUBLE PRECISION      PX(4),p1(4),p2(4),p3(4),p4(4),ph(4)
      DOUBLE PRECISION      m1,m2,m3,m4,mph

      DOUBLE COMPLEX  AmHarIsr(2,2,2,2,2)
      DOUBLE COMPLEX  AmpBornU(2,2,2,2)
      DOUBLE COMPLEX  AmpBornV(2,2,2,2)
      DOUBLE COMPLEX  Csum1,Csum2,U(2,2),V(2,2)
      INTEGER    j,j1,j2,j3,j4,k,Sig
      DOUBLE PRECISION      pr1,pr2,Fleps
*----------------------------------------
      Fleps =  1d-100
* ISR non-infrared two parts: (1) p1 -> photon, contracted with U-matrix
*                             (2) p2 -> photon, contracted with V-matrix
      CALL GPS_Born(KFi,KFf,PX, ph,mph,    p2,-Fleps,  p3,m3, p4,-m4, AmpBornU)
      CALL GPS_Born(KFi,KFf,PX, p1,Fleps,  ph,-mph,    p3,m3, p4,-m4, AmpBornV)
***   CALL GPS_BPrint(6,'Bo(k234)',AmpBornU)
***   CALL GPS_BPrint(6,'Bo(1k34)',AmpBornV)
* Fermion propagarotors
      pr1 = 1d0/(p1(4)*ph(4)-p1(3)*ph(3)-p1(2)*ph(2)-p1(1)*ph(1))/2d0
      pr2 =-1d0/(p2(4)*ph(4)-p2(3)*ph(3)-p2(2)*ph(2)-p2(1)*ph(1))/2d0
      DO k=1,2
         Sig = 3-2*k
         IF( m_KeyArb .EQ. 0 ) THEN
            CALL GPS_MakeU(ph,Sig,  ph,mph,  p1,m1,    U)
            CALL GPS_MakeV(ph,Sig,  p2,m2,   ph,mph,   V)
         ELSE
            CALL GPS_MakeUb(ph,Sig, ph,mph,  p1,m1,    U)
            CALL GPS_MakeVb(ph,Sig, p2,m2,   ph,mph,   V)
         ENDIF
***      WRITE(*,*) ' ///// sig = ', sig
***      CALL GPS_UPrint(6,' U(k,p1)    ',U)
***      CALL GPS_UPrint(6,' V(p2,k)    ',V)
         DO j1=1,2
            DO j2=1,2
               DO j3=1,2
                  DO j4=1,2
                     Csum1=DCMPLX(0d0,0d0)
                     Csum2=DCMPLX(0d0,0d0)
                     DO j=1,2
                        Csum1=Csum1 +U(j,j1)*pr1 *AmpBornU( j,j2,j3,j4)
                        Csum2=Csum2 +V(j2,j)*pr2 *AmpBornV(j1, j,j3,j4)
                     ENDDO
                     AmHarIsr(j1,j2,j3,j4,k) =  Csum1+Csum2
                  ENDDO
               ENDDO
            ENDDO
         ENDDO
      ENDDO
      END                       !!! GPS_Hini


      SUBROUTINE GPS_Hfin(KFi,KFf,PP, p1,m1,p2,m2,p3,m3,p4,m4,ph,mph, AmHarFsr)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//   IR-finite part od 1-photon amplitudes for FSR                                 //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GPS.h'
*
      INTEGER    KFi,KFf
      DOUBLE PRECISION      PP(4),p1(4),p2(4),p3(4),p4(4),ph(4)
      DOUBLE PRECISION      m1,m2,m3,m4,mph
*
      DOUBLE COMPLEX  AmHarFsr(2,2,2,2,2)
      DOUBLE COMPLEX  AmpBornU(2,2,2,2)
      DOUBLE COMPLEX  AmpBornV(2,2,2,2)
      DOUBLE COMPLEX  Csum1,Csum2,U(2,2),V(2,2)
      INTEGER    j,j1,j2,j3,j4,k,Sig
      DOUBLE PRECISION      pr1,pr2,Fleps
*----------------------------------------
      Fleps =  1d-100
* FSR non-infrared two parts: (1) p1 -> photon, contracted with U-matrix
*                             (2) p2 -> photon, contracted with V-matrix
      CALL GPS_Born(KFi,KFf,PP, p1,Fleps, p2,-Fleps, ph,mph, p4,-m4,   AmpBornU)
      CALL GPS_Born(KFi,KFf,PP, p1,Fleps, p2,-Fleps, p3,m3,  ph,-mph,  AmpBornV)
***   CALL GPS_BPrint(6,'Bo(12k4)',AmpBornU)
***   CALL GPS_BPrint(6,'Bo(123k)',AmpBornV)
* Fermion propagarotors
      pr1 = 1d0/(p3(4)*ph(4)-p3(3)*ph(3)-p3(2)*ph(2)-p3(1)*ph(1))/2d0
      pr2 =-1d0/(p4(4)*ph(4)-p4(3)*ph(3)-p4(2)*ph(2)-p4(1)*ph(1))/2d0
      DO k=1,2
         Sig = 3-2*k
         IF( m_KeyArb .EQ. 0 ) THEN
            CALL GPS_MakeU(ph,Sig,    p3,m3,  ph,mph,   U)
            CALL GPS_MakeV(ph,Sig,    ph,mph, p4,m4,    V)
         ELSE
            CALL GPS_MakeUb(ph,Sig,   p3,m3,  ph,mph,   U)
            CALL GPS_MakeVb(ph,Sig,   ph,mph, p4,m4,    V)
         ENDIF
***      WRITE(*,*) ' ///// sig = ', sig
***      CALL GPS_UPrint(6,' U(k,p3)    ',U)
***      CALL GPS_UPrint(6,' V(p4,k)    ',V)
         DO j1=1,2
            DO j2=1,2
               DO j3=1,2
                  DO j4=1,2
                     Csum1=DCMPLX(0d0,0d0)
                     Csum2=DCMPLX(0d0,0d0)
                     DO j=1,2
                        Csum1=Csum1 +U(j3,j)*pr1* AmpBornU(j1,j2, j,j4)
                        Csum2=Csum2 +V(j,j4)*pr2* AmpBornV(j1,j2,j3, j)
                     ENDDO
                     AmHarFsr(j1,j2,j3,j4,k) =  Csum1+Csum2
                  ENDDO
               ENDDO
            ENDDO
         ENDDO
      ENDDO
      END                       !!! GPS_Hfin


      SUBROUTINE GPS_MakeBorn(KFi,KFf,PX,p1,p2,p3,p4,Born)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//   Define spin amplitudes and calculate Born x-section.                          //
*//   Resulting spin amplitudes stored in m_AmpBorn                                 //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GPS.h'
      INTEGER    KFi,KFf
      DOUBLE PRECISION      PX(4),p1(4),p2(4),p3(4),p4(4),Born
*
      INTEGER    k,j1,j2,j3,j4
      DOUBLE PRECISION      Massf,Mbeam,Sum,Fleps
      DOUBLE PRECISION      svarX,svarQ,BetaFin
      INTEGER    NCf,BornV_GetColor
      DOUBLE PRECISION      BornV_GetMass
      DOUBLE PRECISION      PQ(4)
*----------------------------------
      CALL GPS_Initialize
      Mbeam  =  BornV_GetMass(KFi)
      Massf =  BornV_GetMass(KFf)
      Fleps = 1d-100
      Mbeam  = Fleps             ! electron mass almost zero
      DO k=1,4
         PQ(k)=p3(k)+p4(k)
      ENDDO
      svarX=PX(4)**2-PX(3)**2-PX(2)**2-PX(1)**2
      svarQ=PQ(4)**2-PQ(3)**2-PQ(2)**2-PQ(1)**2
      IF(svarX .LE. 4*Massf**2) GOTO 900

* Calculate Born spin amplitudes
      CALL GPS_Born(KFi,KFf,PX,p1,Mbeam,p2,-Mbeam,p3,Massf,p4,-Massf,m_AmpBorn)
* Calculate total x-section
      CALL GPS_BornSumSq(m_AmpBorn,Sum)
* Phase Space factor, Lorenz Beta due to final state 2-body phase space
      BetaFin = SQRT(1d0 -4d0*Massf**2/svarQ)
      Born = Sum*BetaFin
* Color factor
      NCf = BornV_GetColor(KFf)
      Born = NCf*Born
      RETURN
 900  Born = 0d0
      END


      SUBROUTINE GPS_MakeBorn2(KFi,KFf,p1,p2,p3,p4,Born)
*//////////////////////////////////////////////////////////////////////////////////
*//  !!!!!  TEST !!!!!   !!!!!  TEST !!!!! !!!!!  TEST !!!!! !!!!!  TEST !!!     //
*//                                                                              //
*//   Clasical, massive/massles, in terms of cos(theta), similar to KORALB       //
*//   We use here KORALB spin amplitudes for pure s-chanel photons               //
*//   Note that for KORALB x-axis is perpendicular to reaction plane             //
*//   while in JacobWick y-axis is perpendicular to reaction plane.              //
*//   We expect in Rmat, for pure s-chanel photon exchange,                      //
*//   strictly zero correlations between in-plane  and                           //
*//   perpendicular-to-plane components, see APP B15 (1984) p. 1154, eq. (2.6)   //
*//                                                                              //
*//////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GPS.h'
*
      INTEGER KFi,KFf
      DOUBLE PRECISION   p1(4),p2(4),p3(4),p4(4)
      DOUBLE PRECISION   Born
*
      DOUBLE PRECISION    T3e,Qe,Ve,Ae
      DOUBLE PRECISION    T3f,Qf,Af,Vf
      INTEGER  NCf,NCe
      DOUBLE PRECISION    BetaFin
      DOUBLE PRECISION    svar,CosTheta
*-----------------------------------------------------------------------------
      DOUBLE COMPLEX  PropGam,PropZet
      DOUBLE COMPLEX  AmpGam(2,2,2,2),AmpZet(2,2,2,2)
      INTEGER    i,j,k,l
      INTEGER    j1,j2,j3,j4
      INTEGER    Hel1,Hel2,Hel3,Hel4
      DOUBLE COMPLEX  s31,s24,s14,s32
      DOUBLE PRECISION      CupGam,CupZet,CupZet1,CupZet2
      DOUBLE COMPLEX  HeliFactor
      DOUBLE COMPLEX  GPS_iProd1
      DOUBLE COMPLEX  GPS_iProd2
      DOUBLE PRECISION      Massf,Mbeam,Mfin,SinTheta
      DOUBLE PRECISION      Sum,Fleps
*-----------------------------------------------------------------------------
      CALL GPS_Initialize
      CALL BornV_GetParticle(KFf, Massf,Qf,T3f,NCf)
      CALL BornV_GetParticle(KFi, Mbeam, Qe,T3e,NCe)
      svar= (p1(4)+p2(4))**2 -(p1(3)+p2(3))**2 -(p1(2)+p2(2))**2 -(p1(1)+p2(1))**2
      Mfin = Massf*2d0/sqrt(svar)
      IF(svar .LE. 4*Massf**2) GOTO 900
      CosTheta= (p1(3)*p3(3)+p1(2)*p3(2)+p1(1)*p3(1))
     $     /SQRT(p1(3)*p1(3)+p1(2)*p1(2)+p1(1)*p1(1))
     $     /SQRT(p3(3)*p3(3)+p3(2)*p3(2)+p3(1)*p3(1))
      IF(abs(CosTheta) .GT. 1d0) WRITE(*,*) ' BornV: CosTheta=',CosTheta
      SinTheta = SQRT(1d0-CosTheta**2)
      WRITE(*,*) ' GPS_MakeBorn2: svar,CosTheta=',svar,CosTheta
* Couplings
      Ve    = (2*T3e -4*Qe*m_Sw2)/DSQRT(m_Sw2*(1d0-m_Sw2))/4d0
      Ae    =  2*T3e             /DSQRT(m_Sw2*(1d0-m_Sw2))/4d0
      Vf    = (2*T3f -4*Qf*m_Sw2)/DSQRT(m_Sw2*(1d0-m_Sw2))/4d0
      Af    =  2*T3f             /DSQRT(m_Sw2*(1d0-m_Sw2))/4d0
      IF(m_KeyZet .LE. 0) THEN
         Ve=0d0
         Ae=0d0
      ENDIF
      IF(m_KeyZet .EQ. 9) THEN
         Qe=0d0
         Qf=0d0
      ENDIF
*=============================================================
* Propagators
      PropGam =    DCMPLX(  1d0/svar,  0d0)
      PropZet =    1d0/DCMPLX(svar-m_MZ**2, m_GammZ*svar/m_MZ)
      IF(m_KeyZet .EQ.-1) PropZet =  1d0/DCMPLX(Svar-m_MZ**2, m_GammZ*m_MZ)
*=============================================================
* Clean
      DO j1 = 1,2
         DO j2 = 1,2
            DO j3 = 1,2
               DO j4 = 1,2
                  AmpGam( j1,j2,j3,j4) = DCMPLX(0d0,0d0)
                  AmpZet( j1,j2,j3,j4) = DCMPLX(0d0,0d0)
                  m_AmpBorn2( j1,j2,j3,j4) = DCMPLX(0d0,0d0)
               ENDDO
            ENDDO
         ENDDO
      ENDDO
* Clasical, massles
      DO j1 = 1,2
         DO j3 = 1,2
            DO j4 = 1,2
               Hel1 = 3-2*j1
               Hel3 = 3-2*j3
               Hel4 = 3-2*j4
               Hel2 = -Hel1     ! helicity conservation
               j2  = (3-Hel2)/2
***[[ With finite Massf this part is valid strictly if you switch off Z exchange
               IF(Hel3 .EQ. -Hel4) THEN
                  HeliFactor = -DCMPLX( 0d0, Hel1*Hel3 -CosTheta )
               ELSE         
                  HeliFactor = -DCMPLX(Mfin*SinTheta, 0d0)
***  Activate line below if you want to get rid of mass effects completely!!!
***               HeliFactor = -DCMPLX(0d0, 0d0)
               ENDIF
***]]
               CupGam = Qe*Qf
               CupZet = (Ve +Hel2*Ae)*(Vf +Hel3*Af)
               AmpGam( j1,j2,j3,j4) = HeliFactor*PropGam*CupGam
               AmpZet( j1,j2,j3,j4) = HeliFactor*PropZet*CupZet
               m_AmpBorn2(j1,j2,j3,j4) =
     $              (AmpGam(j1,j2,j3,j4)+AmpZet(j1,j2,j3,j4))*svar/2
            ENDDO
         ENDDO
      ENDDO
* Spin sumation/averaging
      CALL GPS_BornSumSq(m_AmpBorn2,Sum)
      Born = Sum
* Lorenz Beta due to final state 2-body phase space
      BetaFin = SQRT(1d0 -4d0*Massf**2/svar)
      Born = Born*BetaFin
      Born = NCf*Born           ! Color factor
      RETURN
 900  Born = 0d0
      END      !GPS_MakeBorn2


      SUBROUTINE GPS_MakeBorn1(KFi,KFf,p1,p2,p3,p4,Born)
*///////////////////////////////////////////////////////////////////////////////
*//  !!!!!  TEST !!!!!   !!!!!  TEST !!!!! !!!!!  TEST !!!!! !!!!!  TEST !!!  //
*//                                                                           //
*//  Born  spin amplitudes, spinor method, massles, based on Chisholm         //
*//                                                                           //
*///////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GPS.h'
*
      INTEGER KFi,KFf
      DOUBLE PRECISION   p1(4),p2(4),p3(4),p4(4)
      DOUBLE PRECISION   Born
*
      DOUBLE PRECISION    T3e,Qe,Ve,Ae
      DOUBLE PRECISION    T3f,Qf,Af,Vf
      INTEGER  NCf,NCe
      DOUBLE PRECISION    BetaFin, svar
*-----------------------------------------------------------------------------
      DOUBLE COMPLEX  PropGam,PropZet
      DOUBLE COMPLEX  AmpGam(2,2,2,2),AmpZet(2,2,2,2)
      INTEGER    i,j,k,l
      INTEGER    j1,j2,j3,j4
      INTEGER    Hel1,Hel2,Hel3,Hel4
      DOUBLE COMPLEX  s31,s24,s14,s32
      DOUBLE PRECISION      CupGam,CupZet
      DOUBLE COMPLEX  HeliFactor
      DOUBLE COMPLEX  GPS_iProd1
      DOUBLE COMPLEX  GPS_iProd2
      DOUBLE PRECISION      Massf,Mbeam
      DOUBLE PRECISION      Sum
*-----------------------------------------------------------------------------
      CALL GPS_Initialize
      CALL BornV_GetParticle(KFf, Massf,Qf,T3f,NCf)
      CALL BornV_GetParticle(KFi, Mbeam, Qe,T3e,NCe)
      svar= (p1(4)+p2(4))**2 -(p1(3)+p2(3))**2 -(p1(2)+p2(2))**2 -(p1(1)+p2(1))**2
      IF(svar .LE. 4*Massf**2) GOTO 900
* Couplings
      Ve    = (2*T3e -4*Qe*m_Sw2)/DSQRT(m_Sw2*(1d0-m_Sw2))/4d0
      Ae    =  2*T3e             /DSQRT(m_Sw2*(1d0-m_Sw2))/4d0
      Vf    = (2*T3f -4*Qf*m_Sw2)/DSQRT(m_Sw2*(1d0-m_Sw2))/4d0
      Af    =  2*T3f             /DSQRT(m_Sw2*(1d0-m_Sw2))/4d0
      IF(m_KeyZet .LE. 0) THEN
         Ve=0d0
         Ae=0d0
      ENDIF
      IF(m_KeyZet .EQ. 9) THEN
         Qe=0d0
         Qf=0d0
      ENDIF
*=============================================================
* Propagators
      PropGam =    DCMPLX(  1d0/svar,  0d0)
      PropZet =    1d0/DCMPLX(svar-m_MZ**2, m_GammZ*svar/m_MZ)
      IF(m_KeyZet .EQ.-1) PropZet =  1d0/DCMPLX(Svar-m_MZ**2, m_GammZ*m_MZ)
*=============================================================
* Clean
      DO j1 = 1,2
         DO j2 = 1,2
            DO j3 = 1,2
               DO j4 = 1,2
                  AmpGam( j1,j2,j3,j4) = DCMPLX(0d0,0d0)
                  AmpZet( j1,j2,j3,j4) = DCMPLX(0d0,0d0)
                  m_AmpBorn1( j1,j2,j3,j4) = DCMPLX(0d0,0d0)
               ENDDO
            ENDDO
         ENDDO
      ENDDO
* Spinor, massles, based on Chisholm
      DO j1 = 1,2
         DO j3 = 1,2
            Hel1 = 3-2*j1
            Hel3 = 3-2*j3
            Hel2 = -Hel1        ! helicity conservation initial state
            Hel4 = -Hel3        ! helicity conservation initial state
            j2  = (3-Hel2)/2
            j4  = (3-Hel4)/2
            CupGam = Qe*Qf
            IF( Hel1 .EQ. -Hel3 ) THEN
               s31 = GPS_iProd1( -Hel1,p3,p1)
               s24 = GPS_iProd1(  Hel1,p2,p4)
               HeliFactor = s31*s24
               CupZet = (Ve +Hel2*Ae)*(Vf +Hel1*Af)
            ELSE
               s14 = GPS_iProd1( -Hel1,p1,p4)
               s32 = GPS_iProd1(  Hel1,p3,p2)
               HeliFactor = s14*s32
               CupZet = (Ve +Hel2*Ae)*(Vf +Hel2*Af)
            ENDIF
            AmpGam( j1,j2,j3,j4) = HeliFactor*PropGam*CupGam
            AmpZet( j1,j2,j3,j4) = HeliFactor*PropZet*CupZet
            m_AmpBorn1(j1,j2,j3,j4) = AmpGam(j1,j2,j3,j4)+AmpZet(j1,j2,j3,j4)
         ENDDO
      ENDDO
* Spin sumation/averaging
      CALL GPS_BornSumSq(m_AmpBorn1,Sum)
* Lorenz Beta due to final state 2-body phase space
      BetaFin = SQRT(1d0 -4d0*Massf**2/svar)
      Born = Sum*BetaFin
      Born = NCf*Born           ! Color factor
      RETURN
 900  Born = 0d0
      END                       !!!!!!  GPS_MakeBorn1


      SUBROUTINE GPS_BornSimple(KFi,KFf,svar,costhe,Born)
*///////////////////////////////////////////////////////////////////////////////
*//                                                                           //
*//   Simple Born similar as in BornV                                         //
*//   Limitation: final mass terms exact for photon exchange!                 //
*//                                                                           //
*///////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GPS.h'
*
      INTEGER  KFi,KFf
      DOUBLE PRECISION    svar,costhe
      DOUBLE PRECISION    Born
*
      DOUBLE PRECISION    ss,T3e,Qe,deno,Ve,Ae
      DOUBLE PRECISION    ye,yf,xf,rechi,xe,amx2
      DOUBLE PRECISION    thresh,ff0,ff1,chi2
      DOUBLE PRECISION    t3f,af,vf,sum,qf
      DOUBLE PRECISION    Massf,Mbeam
      DOUBLE PRECISION    BWD
      INTEGER  NCf,NCe
      DOUBLE PRECISION    BetaFin
*-----------------------------------------------------------------
      CALL GPS_Initialize
      CALL BornV_GetParticle(KFi, Mbeam, Qe,T3e,NCe)
      CALL BornV_GetParticle(KFf, Massf,Qf,T3f,NCf)

      IF(abs(costhe) .GT. 1d0) WRITE(*,*) ' BornV: costhe=',costhe

      ss = svar
      amx2=4d0*Massf**2/svar
* Z and gamma couplings to beams (electrons)
      deno  = 4d0*sqrt(m_Sw2*(1d0-m_Sw2))
      Ve    = (2*T3e -4*Qe*m_Sw2)/deno
      Ae    =  2*T3e             /deno
      Vf    = (2*T3f -4*Qf*m_Sw2)/deno
      Af    =  2*T3f             /deno

      IF(m_KeyZet .LE. 0) THEN
         Ve=0d0
         Ae=0d0
      ENDIF
      IF(m_KeyZet .EQ. 9) THEN
         Qe=0d0
         Qf=0d0
      ENDIF
      BWD = (ss-m_MZ**2)**2 + (m_GammZ*ss/m_MZ)**2
      IF(m_KeyZet .EQ.-1) BWD = (ss-m_MZ**2)**2 + (m_GammZ*m_MZ)**2
      chi2 = ss**2        /BWD
      rechi=(ss-m_MZ**2)*ss /BWD
      xe= Ve**2 +Ae**2
      xf= Vf**2 +Af**2
      ye= 2*Ve*Ae
      yf= 2*Vf*Af
      ff0= qe**2*qf**2 +2*rechi*qe*qf*Ve*Vf +chi2*xe*xf
      ff1=             +2*rechi*qe*qf*Ae*Af +chi2*ye*yf
      Born    = (1d0+ costhe**2 +amx2*(1d0-costhe**2))*ff0 +2d0*costhe*ff1
*     Colour factor
      Born = NCf*Born

      thresh = BetaFin
      IF(    svar .LE.  4d0*Massf**2) THEN
         thresh=0d0
      ELSE
         BetaFin = SQRT(1d0 -4d0*Massf**2/svar)
         thresh  = BetaFin
      ENDIF
c[[[
c      thresh=1d0
c]]]
      Born= Born*thresh
      END                       !!!! GPS_BornSimple


      SUBROUTINE GPS_MakeUW(Norm,ph,sigma,p1,m1,p2,m2,U)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*// like GPS_MakeU  but with W v-a coupling included                                //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INTEGER     sigma,l1,l2
      DOUBLE PRECISION       ph(4),p1(4),m1,p2(4),m2
      DOUBLE COMPLEX   U(2,2),UW0(2,2),WC(2,2),Cnor,Norm
         Cnor=Norm
         CALL GPS_MakeU(ph,Sigma,  p1,m1,   p2,m2,    UW0)
         CALL  GPS_MatrWm(WC)             ! W-e-nu couplingss
         CALL  GPS_times(Cnor,WC,UW0,U) ! we add v-a coupl
      END

      SUBROUTINE GPS_MakeVW(Norm,ph,sigma,p1,m1,p2,m2,V)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*// like GPS_MakeV  but with W v-a coupling included                                //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INTEGER     sigma,l1,l2
      DOUBLE PRECISION       ph(4),p1(4),m1,p2(4),m2
      DOUBLE COMPLEX   V(2,2),VW0(2,2),WC(2,2),Cnor,Norm
         Cnor=Norm
         CALL GPS_MakeV(ph,Sigma,  p1,m1,   p2,m2,    VW0)
         CALL  GPS_MatrW(WC)             ! W-e-nu couplingss
         CALL  GPS_times(Cnor,VW0,WC,V) ! we add v-a coupl
      END

 
      SUBROUTINE GPS_MakeUWb(Norm,ph,sigma,p1,m1,p2,m2,U)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*// like GPS_MakeUb but with W v-a coupling included                                //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INTEGER     sigma,l1,l2
      DOUBLE PRECISION       ph(4),p1(4),m1,p2(4),m2
      DOUBLE COMPLEX   U(2,2),UW0(2,2),WC(2,2),Cnor,Norm
         Cnor=Norm
         CALL GPS_MakeUb(ph,Sigma,  p1,m1,   p2,m2,    UW0)
         CALL  GPS_MatrWm(WC)             ! W-e-nu couplingss
         CALL  GPS_times(Cnor,WC,UW0,U) ! we add v-a coupl
      END

      SUBROUTINE GPS_MakeVWb(Norm,ph,sigma,p1,m1,p2,m2,V)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*// like GPS_MakeVb but with W v-a coupling included                                //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////

      IMPLICIT NONE
      INTEGER     sigma,l1,l2
      DOUBLE PRECISION       ph(4),p1(4),m1,p2(4),m2
      DOUBLE COMPLEX   V(2,2),VW0(2,2),WC(2,2),Cnor,Norm
         Cnor=Norm
         CALL GPS_MakeVb(ph,Sigma,  p1,m1,   p2,m2,    VW0)
         CALL  GPS_MatrW(WC)             ! W-e-nu couplingss
         CALL  GPS_times(Cnor,VW0,WC,V) ! we add v-a coupl
      END

      SUBROUTINE GPS_MakeU(ph,sigma,p1,m1,p2,m2,U)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//   Transition matrix U, (epsilon-slash sandwiched between ubar-u spinors)        //
*//   ph      = photon  4-momentum                                                  //
*//   sigma   = photon polarization (+1,-1)                                         //
*//   p1,p2   = fermion 4-momenta                                                   //
*//   m1,m2   = fermion masses                                                      //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GPS.h'
*
      INTEGER     sigma,l1,l2
      DOUBLE PRECISION       ph(4),p1(4),m1,p2(4),m2
      DOUBLE COMPLEX   U(2,2)
      DOUBLE COMPLEX   GPS_iProd1
      DOUBLE PRECISION       GPS_XiProd
      DOUBLE PRECISION       Sqr2
*-----------
      Sqr2 = DSQRT(2d0)
      CALL GPS_Initialize
      IF(     sigma. EQ. 1 ) THEN
         U(1,1) =  Sqr2*( GPS_XiProd(p2,ph) *GPS_iProd1(1,ph,p1))           ! (++)
         U(2,2) =  Sqr2*( GPS_XiProd(p1,ph) *GPS_iProd1(1,ph,p2))           ! (--)
         U(1,2) =  0d0                                                      ! (+-)
         U(2,1) =  Sqr2*( -m1*GPS_XiProd(p2,p1) +m2*GPS_XiProd(p1,p2))      ! (-+)
      ELSEIF(sigma. EQ. -1 ) THEN
         U(1,1) =  Sqr2*( GPS_XiProd(p1,ph) *GPS_iProd1(-1,ph,p2))          ! (++)
         U(2,2) =  Sqr2*( GPS_XiProd(p2,ph) *GPS_iProd1(-1,ph,p1))          ! (--)
         U(2,1) =  0d0                                                      ! (-+)
         U(1,2) =  Sqr2*( -m1*GPS_XiProd(p2,p1) +m2*GPS_XiProd(p1,p2))      ! (+-)
      ELSE
         Write(*,*) '++++++++ GPS_MakeU: WRONG sigma= ',sigma
      ENDIF
      END                       !!!! GPS_MakeU

      SUBROUTINE GPS_MakeV(ph,sigma,p1,m1,p2,m2,V)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//   Transition matrix V, (epsilon-slash sandwiched between v vbar spinors)        //
*//   ph      = photon  4-momentum                                                  //
*//   sigma   = photon polarization (+1,-1)                                         //
*//   p1,p2   = fermion 4-momenta                                                   //
*//   m1,m2   = fermion masses                                                      //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GPS.h'
*
      INTEGER     sigma,l1,l2
      DOUBLE PRECISION       ph(4),p1(4),m1,p2(4),m2
      DOUBLE COMPLEX   V(2,2)
      DOUBLE COMPLEX   GPS_iProd1
      DOUBLE PRECISION       GPS_XiProd
      DOUBLE PRECISION       Sqr2
*-----------
      Sqr2 = DSQRT(2d0)
      CALL GPS_Initialize
      IF(     sigma. EQ. 1 ) THEN
         V(2,2) =  Sqr2*( GPS_XiProd(p2,ph) *GPS_iProd1(1,ph,p1))            !(--)
         V(1,1) =  Sqr2*( GPS_XiProd(p1,ph) *GPS_iProd1(1,ph,p2))            !(++)
         V(2,1) =  0d0                                                       !(-+)
         V(1,2) =  Sqr2*( m1*GPS_XiProd(p2,p1) -m2*GPS_XiProd(p1,p2))        !(+-)
      ELSEIF(sigma. EQ. -1 ) THEN
         V(2,2) =  Sqr2*( GPS_XiProd(p1,ph) *GPS_iProd1(-1,ph,p2))           !(--)
         V(1,1) =  Sqr2*( GPS_XiProd(p2,ph) *GPS_iProd1(-1,ph,p1))           !(++)
         V(1,2) =  0d0                                                       !(+-)
         V(2,1) =  Sqr2*( m1*GPS_XiProd(p2,p1) -m2*GPS_XiProd(p1,p2))        !(-+)
      ELSE
         Write(*,*) '++++++++ GPS_MakeV: WRONG sigma= ',sigma
      ENDIF
      END                       !!!! GPS_MakeV


      SUBROUTINE GPS_MakeUb(ph,sigma,p1,m1,p2,m2,U)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//   Transition matrix U, (epsilon-slash sandwiched between ubar-u spinors)        //
*//   dependend of vector auxial-gauge vector b=beta=m_b !!!                        //
*//   ph      = photon  4-momentum                                                  //
*//   sigma   = photon polarization (+1,-1)                                         //
*//   p1,p2   = fermion 4-momenta                                                   //
*//   m1,m2   = fermion masses                                                      //
*//   m_b     = auxiliary lightlike vector IS IMPLICIT INPUT                        //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GPS.h'
      INTEGER     sigma,l1,l2
      DOUBLE PRECISION       ph(4),p1(4),m1,p2(4),m2
      DOUBLE COMPLEX   U(2,2)
      DOUBLE COMPLEX   GPS_iProd1
      DOUBLE PRECISION       GPS_XiProd
      DOUBLE COMPLEX   Cnor
*-----------
      CALL GPS_Initialize
      IF(     sigma. EQ. 1 ) THEN
         Cnor   = DCMPLX(DSQRT(2d0),0d0)/GPS_iProd1(-1,ph,m_b)
         U(1,1) = Cnor*(     GPS_iProd1(  1,p1, ph)*GPS_iProd1(-1,m_b,p2)    !(++)
     $                +m1*m2*GPS_XiProd(    m_b,p1)*GPS_XiProd(   ph, p2) )
         U(2,2) = Cnor*(     GPS_iProd1( -1,p1,m_b)*GPS_iProd1( 1,ph, p2)    !(--)
     $                +m1*m2*GPS_XiProd(    m_b,p2)*GPS_XiProd(   ph, p1) )
         U(1,2) = Cnor*(    +m1*GPS_XiProd( m_b,p1)*GPS_iProd1( 1,ph, p2)    !(+-)
     $                      +m2*GPS_XiProd( m_b,p2)*GPS_iProd1( 1,p1, ph) )
         U(2,1) = Cnor*(    +m1*GPS_XiProd(  ph,p1)*GPS_iProd1(-1,m_b,p2)    !(-+)
     $                      +m2*GPS_XiProd(  ph,p2)*GPS_iProd1(-1,p1,m_b) )
      ELSEIF(sigma. EQ. -1 ) THEN
         Cnor   = DCMPLX(DSQRT(2d0),0d0)/GPS_iProd1( 1,ph,m_b)
         U(1,1) = Cnor*(     GPS_iProd1(  1,p1,m_b)*GPS_iProd1(-1,ph, p2)    !(++)
     $                +m1*m2*GPS_XiProd(    m_b,p2)*GPS_XiProd(   ph, p1) )
         U(2,2) = Cnor*(     GPS_iProd1( -1,p1, ph)*GPS_iProd1( 1,m_b,p2)    !(--)
     $                +m1*m2*GPS_XiProd(    m_b,p1)*GPS_XiProd(   ph, p2) )
         U(2,1) = Cnor*(    +m1*GPS_XiProd( m_b,p1)*GPS_iProd1(-1,ph, p2)    !(-+)
     $                      +m2*GPS_XiProd( m_b,p2)*GPS_iProd1(-1,p1, ph) )
         U(1,2) = Cnor*(    +m1*GPS_XiProd(  ph,p1)*GPS_iProd1( 1,m_b,p2)    !(+-)
     $                      +m2*GPS_XiProd(  ph,p2)*GPS_iProd1( 1,p1,m_b) )
      ELSE
         Write(*,*) '++++++++ GPS_MakeUb: WRONG sigma= ',sigma
      ENDIF
      END                       !!!! GPS_MakeUb

      SUBROUTINE GPS_MakeVb(ph,sigma,p1,m1,p2,m2,V)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//   Transition matrix V, (epsilon-slash sandwiched between vbar-v spinors)        //
*//   dependend of vector auxial-gauge vector b=beta=m_b !!!                        //
*//   ph      = photon  4-momentum                                                  //
*//   sigma   = photon polarization (+1,-1)                                         //
*//   p1,p2   = fermion 4-momenta                                                   //
*//   m1,m2   = fermion masses                                                      //
*//   m_b     = auxiliary lightlike vector IS IMPLICIT INPUT                        //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GPS.h'
      INTEGER     sigma,l1,l2
      DOUBLE PRECISION       ph(4),p1(4),m1,p2(4),m2
      DOUBLE COMPLEX   V(2,2)
      DOUBLE COMPLEX   GPS_iProd1
      DOUBLE PRECISION       GPS_XiProd
      DOUBLE COMPLEX   Cnor
*-----------
      CALL GPS_Initialize
      IF(     sigma. EQ. 1 ) THEN
         Cnor   = DCMPLX(DSQRT(2d0),0d0)/GPS_iProd1(-1,ph,m_b)
         V(2,2) = Cnor*(     GPS_iProd1(  1,p1, ph)*GPS_iProd1(-1,m_b,p2)    !(--)
     $                +m1*m2*GPS_XiProd(    m_b,p1)*GPS_XiProd(   ph, p2) )
         V(1,1) = Cnor*(     GPS_iProd1( -1,p1,m_b)*GPS_iProd1( 1,ph, p2)    !(++)
     $                +m1*m2*GPS_XiProd(    m_b,p2)*GPS_XiProd(   ph, p1) )
         V(2,1) = Cnor*(    -m1*GPS_XiProd( m_b,p1)*GPS_iProd1( 1,ph, p2)    !(-+)
     $                      -m2*GPS_XiProd( m_b,p2)*GPS_iProd1( 1,p1, ph) )
         V(1,2) = Cnor*(    -m1*GPS_XiProd(  ph,p1)*GPS_iProd1(-1,m_b,p2)    !(+-)
     $                      -m2*GPS_XiProd(  ph,p2)*GPS_iProd1(-1,p1,m_b) )
      ELSEIF(sigma. EQ. -1 ) THEN
         Cnor   = DCMPLX(DSQRT(2d0),0d0)/GPS_iProd1( 1,ph,m_b)
         V(2,2) = Cnor*(     GPS_iProd1(  1,p1,m_b)*GPS_iProd1(-1,ph, p2)    !(--)
     $                +m1*m2*GPS_XiProd(    m_b,p2)*GPS_XiProd(   ph, p1) )
         V(1,1) = Cnor*(     GPS_iProd1( -1,p1, ph)*GPS_iProd1( 1,m_b,p2)    !(++)
     $                +m1*m2*GPS_XiProd(    m_b,p1)*GPS_XiProd(   ph, p2) )
         V(1,2) = Cnor*(    -m1*GPS_XiProd( m_b,p1)*GPS_iProd1(-1,ph, p2)    !(+-)
     $                      -m2*GPS_XiProd( m_b,p2)*GPS_iProd1(-1,p1, ph) )
         V(2,1) = Cnor*(    -m1*GPS_XiProd(  ph,p1)*GPS_iProd1( 1,m_b,p2)    !(-+)
     $                      -m2*GPS_XiProd(  ph,p2)*GPS_iProd1( 1,p1,m_b) )
      ELSE
         Write(*,*) '++++++++ GPS_MakeVb: WRONG sigma= ',sigma
      ENDIF
      END                       !!!! GPS_MakeVb




      SUBROUTINE GPS_MakeUX(Cn,ph,mh,p1,m1,p2,m2,U)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//   Transition matrix U, (ph-slash sandwiched between ubar-u spinors)             //
*//   mass terms not tested   !!!                                                   //
*//   ph      = photon  4-momentum                                                  //
*//   p1,p2   = fermion 4-momenta                                                   //
*//   m1,m2   = fermion masses                                                      //
*//   m_b     = auxiliary lightlike vector IS IMPLICIT INPUT                        //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GPS.h'
      INTEGER     sigma,l1,l2
      DOUBLE PRECISION       ph(4),mh,p1(4),m1,p2(4),m2
      DOUBLE COMPLEX   U(2,2),A(2,2),B(2,2),AB(2,2),C(2,2)
      DOUBLE COMPLEX   GPS_iProd1
      DOUBLE PRECISION       GPS_XiProd
      DOUBLE COMPLEX   Cn,Cnor
*-----------
      CALL GPS_Initialize
      Cnor=1D0
*-----------
      CALL GPS_Initialize
      CALL  GPS_MatrS(p1,m1,ph,mh,A)
      CALL  GPS_MatrWm(C)
      CALL  GPS_MatrS(ph,mh,p2,m2,B)
      CALL  GPS_times(Cnor,A,B,AB)
      CALL  GPS_times(Cn,C,AB,U)
      END                       !!!! GPS_MakeUb

      SUBROUTINE GPS_MakeVX(Cn,ph,mh,p1,m1,p2,m2,V)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//   Transition matrix V, (ph-slash sandwiched between vbar-v spinors)             //
*//   mass terms not studied !!!                                                    //
*//   ph      = photon  4-momentum                                                  //
*//   mh      = photon  mass                                                        //
*//   p1,p2   = fermion 4-momenta                                                   //
*//   m1,m2   = fermion masses                                                      //
*//   massles limit                                                                 //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GPS.h'
      INTEGER     sigma,l1,l2
      DOUBLE PRECISION       ph(4),mh,p1(4),m1,p2(4),m2
      DOUBLE COMPLEX   V(2,2),A(2,2),B(2,2),C(2,2),AB(2,2)
      DOUBLE COMPLEX   GPS_iProd1
      DOUBLE PRECISION       GPS_XiProd
      DOUBLE COMPLEX   Cn,Cnor,XX
      Cnor=1D0
*-----------
      CALL GPS_Initialize
      CALL  GPS_MatrS(p1,m1,ph,mh,A)
      CALL  GPS_MatrWm(C)
      CALL  GPS_MatrS(ph,mh,p2,m2,B)
      CALL  GPS_times(Cnor,A,B,AB)
      CALL  GPS_times(Cn,AB,C,V)
      XX=V(1,1)
      V(1,1)=V(2,2)
      V(2,2)=XX
      END                       !!!! GPS_MakeVb




      SUBROUTINE GPS_MatrU(Cfact,ph,sigma,p1,m1,p2,m2,U)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//   Transition matrix U, (epsilon-slash sandwiched between ubar-u spinors)        //
*//   ph      = photon  4-momentum                                                  //
*//   sigma   = photon polarization (+1,-1)                                         //
*//   p1,p2   = fermion 4-momenta                                                   //
*//   m1,m2   = fermion masses                                                      //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GPS.h'
*
      INTEGER           sigma,l1,l2
      DOUBLE PRECISION  ph(4),p1(4),m1,p2(4),m2
      DOUBLE COMPLEX    Cfact,U(2,2)
      DOUBLE COMPLEX    GPS_iProd1
      DOUBLE PRECISION  GPS_XiProd
      DOUBLE COMPLEX    Norm
*-----------
      Norm = DSQRT(2d0)*Cfact
      CALL GPS_Initialize
      IF(     sigma. EQ. 1 ) THEN
         U(1,1) =  Norm*( GPS_XiProd(p2,ph) *GPS_iProd1(1,ph,p1))           ! (++)
         U(2,2) =  Norm*( GPS_XiProd(p1,ph) *GPS_iProd1(1,ph,p2))           ! (--)
         U(1,2) =  0d0                                                      ! (+-)
         U(2,1) =  Norm*( -m1*GPS_XiProd(p2,p1) +m2*GPS_XiProd(p1,p2))      ! (-+)
      ELSEIF(sigma. EQ. -1 ) THEN
         U(1,1) =  Norm*( GPS_XiProd(p1,ph) *GPS_iProd1(-1,ph,p2))          ! (++)
         U(2,2) =  Norm*( GPS_XiProd(p2,ph) *GPS_iProd1(-1,ph,p1))          ! (--)
         U(2,1) =  0d0                                                      ! (-+)
         U(1,2) =  Norm*( -m1*GPS_XiProd(p2,p1) +m2*GPS_XiProd(p1,p2))      ! (+-)
      ELSE
         Write(*,*) '++++++++ GPS_MatrU: WRONG sigma= ',sigma
      ENDIF
      END                       !!!! GPS_MatrU

      SUBROUTINE GPS_MatrV(Cfact,ph,sigma,p1,m1,p2,m2,V)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//   Transition matrix V, (epsilon-slash sandwiched between v vbar spinors)        //
*//   ph      = photon  4-momentum                                                  //
*//   sigma   = photon polarization (+1,-1)                                         //
*//   p1,p2   = fermion 4-momenta                                                   //
*//   m1,m2   = fermion masses                                                      //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GPS.h'
*
      INTEGER           sigma,l1,l2
      DOUBLE PRECISION  ph(4),p1(4),m1,p2(4),m2
      DOUBLE COMPLEX    Cfact,V(2,2)
      DOUBLE COMPLEX    GPS_iProd1
      DOUBLE PRECISION  GPS_XiProd
      DOUBLE COMPLEX    Norm
*-----------
      Norm = DSQRT(2d0)*Cfact
      CALL GPS_Initialize
      IF(     sigma. EQ. 1 ) THEN
         V(2,2) =  Norm*( GPS_XiProd(p2,ph) *GPS_iProd1(1,ph,p1))            !(--)
         V(1,1) =  Norm*( GPS_XiProd(p1,ph) *GPS_iProd1(1,ph,p2))            !(++)
         V(2,1) =  0d0                                                       !(-+)
         V(1,2) =  Norm*( m1*GPS_XiProd(p2,p1) -m2*GPS_XiProd(p1,p2))        !(+-)
      ELSEIF(sigma. EQ. -1 ) THEN
         V(2,2) =  Norm*( GPS_XiProd(p1,ph) *GPS_iProd1(-1,ph,p2))           !(--)
         V(1,1) =  Norm*( GPS_XiProd(p2,ph) *GPS_iProd1(-1,ph,p1))           !(++)
         V(1,2) =  0d0                                                       !(+-)
         V(2,1) =  Norm*( m1*GPS_XiProd(p2,p1) -m2*GPS_XiProd(p1,p2))        !(-+)
      ELSE
         Write(*,*) '++++++++ GPS_MatrV: WRONG sigma= ',sigma
      ENDIF
      END                       !!!! GPS_MatrV


      SUBROUTINE GPS_MatrUb(Cfact,ph,sigma,p1,m1,p2,m2,U)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//   Transition matrix U, (epsilon-slash sandwiched between ubar-u spinors)        //
*//   dependend of vector auxial-gauge vector b=beta=m_b !!!                        //
*//   ph      = photon  4-momentum                                                  //
*//   sigma   = photon polarization (+1,-1)                                         //
*//   p1,p2   = fermion 4-momenta                                                   //
*//   m1,m2   = fermion masses                                                      //
*//   m_b     = auxiliary lightlike vector IS IMPLICIT INPUT                        //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GPS.h'
      INTEGER           sigma,l1,l2
      DOUBLE PRECISION  ph(4),p1(4),m1,p2(4),m2
      DOUBLE COMPLEX    Cfact,U(2,2)
      DOUBLE COMPLEX    GPS_iProd1
      DOUBLE PRECISION  GPS_XiProd
      DOUBLE COMPLEX    Cnor
*-----------
      CALL GPS_Initialize
      IF(     sigma. EQ. 1 ) THEN
         Cnor   = DCMPLX(DSQRT(2d0),0d0)/GPS_iProd1(-1,ph,m_b)*Cfact
         U(1,1) = Cnor*(     GPS_iProd1(  1,p1, ph)*GPS_iProd1(-1,m_b,p2)    !(++)
     $                +m1*m2*GPS_XiProd(    m_b,p1)*GPS_XiProd(   ph, p2) )
         U(2,2) = Cnor*(     GPS_iProd1( -1,p1,m_b)*GPS_iProd1( 1,ph, p2)    !(--)
     $                +m1*m2*GPS_XiProd(    m_b,p2)*GPS_XiProd(   ph, p1) )
         U(1,2) = Cnor*(    +m1*GPS_XiProd( m_b,p1)*GPS_iProd1( 1,ph, p2)    !(+-)
     $                      +m2*GPS_XiProd( m_b,p2)*GPS_iProd1( 1,p1, ph) )
         U(2,1) = Cnor*(    +m1*GPS_XiProd(  ph,p1)*GPS_iProd1(-1,m_b,p2)    !(-+)
     $                      +m2*GPS_XiProd(  ph,p2)*GPS_iProd1(-1,p1,m_b) )
      ELSEIF(sigma. EQ. -1 ) THEN
         Cnor   = DCMPLX(DSQRT(2d0),0d0)/GPS_iProd1( 1,ph,m_b)*Cfact
         U(1,1) = Cnor*(     GPS_iProd1(  1,p1,m_b)*GPS_iProd1(-1,ph, p2)    !(++)
     $                +m1*m2*GPS_XiProd(    m_b,p2)*GPS_XiProd(   ph, p1) )
         U(2,2) = Cnor*(     GPS_iProd1( -1,p1, ph)*GPS_iProd1( 1,m_b,p2)    !(--)
     $                +m1*m2*GPS_XiProd(    m_b,p1)*GPS_XiProd(   ph, p2) )
         U(2,1) = Cnor*(    +m1*GPS_XiProd( m_b,p1)*GPS_iProd1(-1,ph, p2)    !(-+)
     $                      +m2*GPS_XiProd( m_b,p2)*GPS_iProd1(-1,p1, ph) )
         U(1,2) = Cnor*(    +m1*GPS_XiProd(  ph,p1)*GPS_iProd1( 1,m_b,p2)    !(+-)
     $                      +m2*GPS_XiProd(  ph,p2)*GPS_iProd1( 1,p1,m_b) )
      ELSE
         Write(*,*) '++++++++ GPS_MatrUb: WRONG sigma= ',sigma
      ENDIF
      END                       !!!! GPS_MatrUb

      SUBROUTINE GPS_MatrVb(Cfact,ph,sigma,p1,m1,p2,m2,V)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//   Transition matrix V, (epsilon-slash sandwiched between vbar-v spinors)        //
*//   dependend of vector auxial-gauge vector b=beta=m_b !!!                        //
*//   ph      = photon  4-momentum                                                  //
*//   sigma   = photon polarization (+1,-1)                                         //
*//   p1,p2   = fermion 4-momenta                                                   //
*//   m1,m2   = fermion masses                                                      //
*//   m_b     = auxiliary lightlike vector IS IMPLICIT INPUT                        //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GPS.h'
      INTEGER           sigma,l1,l2
      DOUBLE PRECISION  ph(4),p1(4),m1,p2(4),m2
      DOUBLE COMPLEX    Cfact,V(2,2)
      DOUBLE COMPLEX    GPS_iProd1
      DOUBLE PRECISION  GPS_XiProd
      DOUBLE COMPLEX    Cnor
*-----------
      CALL GPS_Initialize
      IF(     sigma. EQ. 1 ) THEN
         Cnor   = DCMPLX(DSQRT(2d0),0d0)/GPS_iProd1(-1,ph,m_b)*Cfact
         V(2,2) = Cnor*(     GPS_iProd1(  1,p1, ph)*GPS_iProd1(-1,m_b,p2)    !(--)
     $                +m1*m2*GPS_XiProd(    m_b,p1)*GPS_XiProd(   ph, p2) )
         V(1,1) = Cnor*(     GPS_iProd1( -1,p1,m_b)*GPS_iProd1( 1,ph, p2)    !(++)
     $                +m1*m2*GPS_XiProd(    m_b,p2)*GPS_XiProd(   ph, p1) )
         V(2,1) = Cnor*(    -m1*GPS_XiProd( m_b,p1)*GPS_iProd1( 1,ph, p2)    !(-+)
     $                      -m2*GPS_XiProd( m_b,p2)*GPS_iProd1( 1,p1, ph) )
         V(1,2) = Cnor*(    -m1*GPS_XiProd(  ph,p1)*GPS_iProd1(-1,m_b,p2)    !(+-)
     $                      -m2*GPS_XiProd(  ph,p2)*GPS_iProd1(-1,p1,m_b) )
      ELSEIF(sigma. EQ. -1 ) THEN
         Cnor   = DCMPLX(DSQRT(2d0),0d0)/GPS_iProd1( 1,ph,m_b)*Cfact
         V(2,2) = Cnor*(     GPS_iProd1(  1,p1,m_b)*GPS_iProd1(-1,ph, p2)    !(--)
     $                +m1*m2*GPS_XiProd(    m_b,p2)*GPS_XiProd(   ph, p1) )
         V(1,1) = Cnor*(     GPS_iProd1( -1,p1, ph)*GPS_iProd1( 1,m_b,p2)    !(++)
     $                +m1*m2*GPS_XiProd(    m_b,p1)*GPS_XiProd(   ph, p2) )
         V(1,2) = Cnor*(    -m1*GPS_XiProd( m_b,p1)*GPS_iProd1(-1,ph, p2)    !(+-)
     $                      -m2*GPS_XiProd( m_b,p2)*GPS_iProd1(-1,p1, ph) )
         V(2,1) = Cnor*(    -m1*GPS_XiProd(  ph,p1)*GPS_iProd1( 1,m_b,p2)    !(-+)
     $                      -m2*GPS_XiProd(  ph,p2)*GPS_iProd1( 1,p1,m_b) )
      ELSE
         Write(*,*) '++++++++ GPS_MatrVb: WRONG sigma= ',sigma
      ENDIF
      END                       !!!! GPS_MatrVb

      SUBROUTINE GPS_MatrS(p1,m1,p2,m2,V)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//   S matrix for spinor products (matrix version of  GPS_iProd1                   //
*//   p1,p2   = fermion 4-momenta                                                   //
*//   m1,m2   = fermion masses   (not used at the moment massless case taken        //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GPS.h'
      INTEGER           sigma,l1,l2
      DOUBLE PRECISION  ph(4),p1(4),m1,p2(4),m2
      DOUBLE COMPLEX    Cfact,V(2,2)
      DOUBLE COMPLEX    GPS_iProd1
      DOUBLE PRECISION  GPS_XiProd
      DOUBLE COMPLEX    Cnor
*-----------
      CALL GPS_Initialize
         V(1,2) =      GPS_iProd1(  1,p1, p2)   !(+-)
         V(2,1) =      GPS_iProd1( -1,p1,p2)    !(-+)
         V(2,2) = 0D0                            !(--)
         V(1,1) = 0D0                            !(++)

      END                       !!!! GPS_MatrS
      SUBROUTINE GPS_MatrW(V)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//   W coupling vertes in spinor indices   V-case                                  //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GPS.h'
      INTEGER           sigma,l1,l2
      DOUBLE PRECISION  ph(4),p1(4),m1,p2(4),m2
      DOUBLE COMPLEX    Cfact,V(2,2)
      DOUBLE COMPLEX    GPS_iProd1
      DOUBLE PRECISION  GPS_XiProd
      DOUBLE COMPLEX    Cnor
*-----------
      CALL GPS_Initialize
         Cnor   = sqrt(1.d0/2.d0/m_Sw2)
         V(1,2) = Cnor*0D0 !(+-)
         V(2,1) = Cnor*0D0 !(-+)
         V(2,2) = Cnor*0D0 !(--)
         V(1,1) = Cnor*1D0 !(++)

      END                       !!!! GPS_MatrW
      SUBROUTINE GPS_MatrWm(V)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//   W coupling vertes in spinor indices U-case                                    //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GPS.h'
      INTEGER           sigma,l1,l2
      DOUBLE PRECISION  ph(4),p1(4),m1,p2(4),m2
      DOUBLE COMPLEX    Cfact,V(2,2)
      DOUBLE COMPLEX    GPS_iProd1
      DOUBLE PRECISION  GPS_XiProd
      DOUBLE COMPLEX    Cnor
*-----------
      CALL GPS_Initialize
         Cnor   = sqrt(1.d0/2.d0/m_Sw2)
         V(1,2) = Cnor*0D0 !(+-)
         V(2,1) = Cnor*0D0 !(-+)
         V(2,2) = Cnor*1D0 !(--)
         V(1,1) = Cnor*0D0 !(++)

      END                       !!!! GPS_MatrWm




      SUBROUTINE GPS_MakeUE(ph,sigma,p1,m1,p2,m2,U)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//   Alternative construction of U using MultiE, essentialy for tests.             //
*//   Note that one cannot input Xi into MultiE                                     //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      INCLUDE 'GPS.h'
      INTEGER     sigma,l1,l2
      DOUBLE PRECISION       ph(4),p1(4),m1,p2(4),m2
      DOUBLE COMPLEX   U(2,2)
      DOUBLE COMPLEX   A1(2,2),A2(2,2)
      DOUBLE PRECISION       eps
      INTEGER     i,j,k
      DOUBLE COMPLEX   Csum,Norm
      DOUBLE COMPLEX   GPS_iProd1
*-----------
      CALL GPS_Initialize
      eps = 1d-100
      CALL GPS_MultiE(      1,  p1, m1,  p1, m1,   sigma, m_b,eps,  ph,eps,   A1)
      CALL GPS_MultiE(  sigma,  ph,eps, m_b,eps,       1,  p2, m2,  p2, m2,   A2)
      Norm = DSQRT(2d0) /GPS_iProd1(-sigma,ph,m_b)
      CALL  GPS_times(Norm,A1,A2,U)
      END                       !!!!  GPS_MakeUE

      SUBROUTINE GPS_MakeVE(ph,sigma,p1,m1,p2,m2,V)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//   Alternative construction of V using MultiE, essentialy for tests.             //
*//   Note that one cannot input Xi into MultiE                                     //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      INCLUDE 'GPS.h'
      INTEGER     sigma,l1,l2
      DOUBLE PRECISION       ph(4),p1(4),m1,p2(4),m2
      DOUBLE COMPLEX   V(2,2)
      DOUBLE COMPLEX   A1(2,2),A2(2,2)
      DOUBLE PRECISION       eps
      INTEGER     i,j,k
      DOUBLE COMPLEX   Csum,Norm
      DOUBLE COMPLEX   GPS_iProd1
*-----------
      CALL GPS_Initialize
      eps = 1d-100
      CALL GPS_MultiE(      1,  p1,-m1,  p1,-m1,   sigma, m_b,eps,  ph,eps,   A1)
      CALL GPS_MultiE(  sigma,  ph,eps, m_b,eps,       1,  p2,-m2,  p2,-m2,   A2)
      Norm = DSQRT(2d0) /GPS_iProd1(-sigma,ph,m_b)
      CALL  GPS_times(Norm,A1,A2,V)
      END                       !!!!  GPS_MakeVE

      SUBROUTINE GPS_times(C,A1,A2,Result)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//   Matrix multiplication, Warning: Result cannot be the same as A1 or A2 !!!!    //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      DOUBLE COMPLEX   C,A1(2,2),A2(2,2),Result(2,2)
      INTEGER     i,j,k
      DOUBLE COMPLEX   Csum
*-----------
      DO i=1,2
         DO j=1,2
            Csum= DCMPLX(0d0,0d0)
            DO k=1,2
               Csum= Csum +A1(i,k)*A2(k,j)
            ENDDO
            Result(i,j)=Csum*C
         ENDDO
      ENDDO
      END                       !!!!  GPS_times

      SUBROUTINE GPS_sum(C,A1,A2,Result)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//   Matrix summation, Warning: C multiplies second matrix                         //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      DOUBLE COMPLEX  A1(2,2),A2(2,2),Result(2,2)
      INTEGER     i,j,k
      DOUBLE PRECISION   C
*-----------
      DO i=1,2
         DO j=1,2
            Result(i,j)=A1(i,j)+ A2(i,j)*C
         ENDDO
      ENDDO
      END                       !!!!  GPS_sum


      SUBROUTINE GPS_MultiE(rho, a,ma,b,mb, mu,c,mc,d,md, Matrix)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//   Clever routine Multi introduced by Elzbieta Richter-Was.                      //
*//   Simplified version.                                                           //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INTEGER     rho,mu
      DOUBLE PRECISION       a(4),ma, b(4),mb, c(4),mc, d(4),md
      DOUBLE COMPLEX   Matrix(2,2)
      DOUBLE COMPLEX   GPS_iProd2
*------------------------------
      Matrix(1,1) = GPS_iProd2(  rho,a,ma,   mu,c,mc)
      Matrix(1,2) = GPS_iProd2(  rho,a,ma,  -mu,d,md)
      Matrix(2,1) = GPS_iProd2( -rho,b,mb,   mu,c,mc)
      Matrix(2,2) = GPS_iProd2( -rho,b,mb,  -mu,d,md)
*------------------------------
      END                       !!!! GPS_MultiE



      DOUBLE COMPLEX FUNCTION GPS_Sof1(sigma,ph,pf)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//   Single soft photon contribution to Soft factor at amplitude level             //
*//   sigma   = photon polarization (+1,-1)                                         //
*//   ph      = photon  4-momentum                                                  //
*//   pf      = fermion 4-momenta                                                   //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INTEGER              sigma
      DOUBLE PRECISION     ph(4),pf(4)
      DOUBLE COMPLEX       GPS_iProd1
      DOUBLE PRECISION     GPS_XiProd
*
      GPS_Sof1 =  
     $     DSQRT(2d0)*GPS_iProd1(sigma,ph,pf)*GPS_XiProd(pf,ph)
     $     /(2d0*(pf(4)*ph(4)-pf(3)*ph(3)-pf(2)*ph(2)-pf(1)*ph(1))) !
      END

      DOUBLE COMPLEX FUNCTION GPS_Sof1b(sigma,ph,pf,mf)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//   Single soft photon contribution to Soft factor at amplitude level             //
*//   dependend of vector auxial-gauge vector b=beta=m_b !!!                        //
*//   sigma   = photon polarization (+1,-1)                                         //
*//   ph      = photon  4-momentum                                                  //
*//   pf      = fermion 4-momentum                                                  //
*//   mf      = fermion masses                                                      //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INTEGER           sigma
      DOUBLE PRECISION  ph(4),pf(4),mf
      DOUBLE COMPLEX    GPS_bfacb
*
      GPS_Sof1b =
     $     GPS_bfacb(sigma,ph,pf,mf)
     $     /(2*( pf(4)*ph(4)-pf(3)*ph(3)-pf(2)*ph(2)-pf(1)*ph(1) ))
      END

      DOUBLE COMPLEX FUNCTION GPS_Sof1x(sigma,ph,pf)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//   Single soft photon contribution to Soft factor at amplitude level             //
*//   no propagator                                                                 //
*//   sigma   = photon polarization (+1,-1)                                         //
*//   ph      = photon  4-momentum                                                  //
*//   pf      = fermion 4-momenta                                                   //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INTEGER              sigma
      DOUBLE PRECISION     ph(4),pf(4)
      DOUBLE COMPLEX       GPS_iProd1
      DOUBLE PRECISION     GPS_XiProd
*
      GPS_Sof1x =  
     $     DSQRT(2d0)*GPS_iProd1(sigma,ph,pf)*GPS_XiProd(pf,ph)
      END

      DOUBLE COMPLEX FUNCTION GPS_Sof1bx(sigma,ph,pf,mf)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//   Single soft photon contribution to Soft factor at amplitude level             //
*//   no propagator                                                                 //
*//   dependend of vector auxial-gauge vector b=beta=m_b !!!                        //
*//   sigma   = photon polarization (+1,-1)                                         //
*//   ph      = photon  4-momentum                                                  //
*//   pf      = fermion 4-momentum                                                  //
*//   mf      = fermion masses                                                      //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INTEGER           sigma
      DOUBLE PRECISION  ph(4),pf(4),mf
      DOUBLE COMPLEX    GPS_bfacb
*
      GPS_Sof1bx =
     $     GPS_bfacb(sigma,ph,pf,mf)
!     $     /(2*( pf(4)*ph(4)-pf(3)*ph(3)-pf(2)*ph(2)-pf(1)*ph(1) ))
      END




      DOUBLE COMPLEX  FUNCTION GPS_soft(sigma,ph,p1,p2)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//   2-fermion Soft factor at amplitude level                                      //
*//   sigma   = photon polarization (+1,-1)                                         //
*//   ph      = photon  4-momentum                                                  //
*//   p1,p2   = fermion 4-momenta                                                   //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INTEGER     sigma
      DOUBLE PRECISION       ph(4),p1(4),p2(4)
      DOUBLE COMPLEX   GPS_iProd1
      DOUBLE PRECISION       GPS_XiProd
      DOUBLE COMPLEX   Soft,bf1,bf2
      DOUBLE PRECISION       pk1,pk2
      DOUBLE PRECISION       Sqr2
*------
      Sqr2 = DSQRT(2d0)
      CALL GPS_Initialize
      pk1 = p1(4)*ph(4)-p1(3)*ph(3)-p1(2)*ph(2)-p1(1)*ph(1)
      pk2 = p2(4)*ph(4)-p2(3)*ph(3)-p2(2)*ph(2)-p2(1)*ph(1)
      bf1 =  Sqr2*GPS_iProd1(sigma,ph,p1)*GPS_XiProd(p1,ph) !!! =GPS_bfact(sigma,ph,p1)
      bf2 =  Sqr2*GPS_iProd1(sigma,ph,p2)*GPS_XiProd(p2,ph) !!! =GPS_bfact(sigma,ph,p2)
      Soft = -bf1/(2*pk1) +bf2/(2*pk2)
      GPS_soft = Soft
      END

      DOUBLE COMPLEX  FUNCTION GPS_bfact(sigma,phot,pferm)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//     diagonal element of U-matrix for massive fermion = denominator in s-factor  //
*//     sigma  = photon polarization (+1,-1)                                        //
*//     phot   = photon 4-momentum                                                  //
*//     pferm  = fermion 4-momentum                                                 //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INTEGER     sigma
      DOUBLE PRECISION       phot(4), pferm(4)
      DOUBLE COMPLEX   GPS_iProd1
      DOUBLE PRECISION       GPS_XiProd
*---------------------------
      CALL GPS_Initialize
      GPS_bfact =  DSQRT(2d0)*GPS_iProd1(sigma,phot,pferm)*GPS_XiProd(pferm,phot)
      END


      DOUBLE COMPLEX  FUNCTION GPS_softb(sigma,ph,p1,m1,p2,m2)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//   Soft factor at amplitude level                                                //
*//   dependend of vector auxial-gauge vector b=beta=m_b !!!                        //
*//   sigma   = photon polarization (+1,-1)                                         //
*//   ph      = photon  4-momentum                                                  //
*//   p1,p2   = fermion 4-momenta                                                   //
*//   m1,m2   = fermion masses                                                      //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INTEGER     sigma
      DOUBLE PRECISION       ph(4),p1(4),m1,p2(4),m2
      DOUBLE COMPLEX   GPS_bfacb
      DOUBLE COMPLEX   bf1,bf2
      DOUBLE PRECISION       pk1,pk2
*------
      CALL GPS_Initialize
      pk1 = p1(4)*ph(4)-p1(3)*ph(3)-p1(2)*ph(2)-p1(1)*ph(1)
      pk2 = p2(4)*ph(4)-p2(3)*ph(3)-p2(2)*ph(2)-p2(1)*ph(1)
      bf1 = GPS_bfacb(sigma,ph,p1,m1)
      bf2 = GPS_bfacb(sigma,ph,p2,m2)
      GPS_softb = -bf1/(2*pk1) +bf2/(2*pk2)
      END


      DOUBLE COMPLEX  FUNCTION GPS_bfacb(sigma,phot,pferm,mass)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//   diagonal element of U-matrix for massive fermion = denominator in s-factor    //
*//   dependend of vector auxial-gauge vector b=beta=m_b !!!                        //
*//   sigma  = photon polarization (+1,-1)                                          //
*//   phot   = photon 4-momentum                                                    //
*//   pferm  = fermion 4-momentum                                                   //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GPS.h'
      INTEGER     sigma
      DOUBLE PRECISION       phot(4), pferm(4), mass
      DOUBLE COMPLEX   GPS_iProd1
      DOUBLE PRECISION       GPS_XiProd
*---------------------------
      CALL GPS_Initialize
      GPS_bfacb = DCMPLX(DSQRT(2d0),0d0)/GPS_iProd1(-sigma,phot,m_b)
     $        *(     GPS_iProd1( -sigma,m_b,pferm) *GPS_iProd1( sigma, pferm, phot)
     $          +mass**2 *GPS_XiProd(   m_b,pferm) *GPS_XiProd(        phot, pferm) )
      END

      DOUBLE COMPLEX  FUNCTION GPS_iProd1(L,p,q)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*// This is basic inner s-product of spinors s_{L}(p,q)=ubar_{L}(p)*u_{-L}(q)       //
*// We exploit identity s_{-}(p,q) = -[s_{+}(p,q)]^*                                //
*// Four-vectors p,q are in principle massless, however, massive p,q                //
*// are also meaningfull. Implicit projection on xi is then assumed.                //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GPS.h'
*
      INTEGER     L
      DOUBLE PRECISION       p(4), q(4)
      DOUBLE COMPLEX   Prod

      IF(     L .EQ.  1 ) THEN
         Prod=
     $        -DSQRT( (p(4)-p(1)) / (q(4)-q(1)) ) *DCMPLX(q(2),q(3))
     $        +DSQRT( (q(4)-q(1)) / (p(4)-p(1)) ) *DCMPLX(p(2),p(3))
      ELSEIF( L .EQ. -1 ) THEN
         Prod=
     $        -DSQRT( (q(4)-q(1)) / (p(4)-p(1)) ) *DCMPLX(p(2),p(3))
     $        +DSQRT( (p(4)-p(1)) / (q(4)-q(1)) ) *DCMPLX(q(2),q(3))
         Prod= DCONJG(Prod)
      ELSE
         WRITE(*,*) '##### GPS_iProd1: Wrong L= ', L
      ENDIF
      GPS_iProd1 =  Prod
      END       

      DOUBLE COMPLEX  FUNCTION GPS_iProd2(Lamp,p,mp,Lamq,q,mq)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*// General spinor product s_{l1,l2}(p,q) for massive spinors u and/or v            //
*// mp and mq are masses of p and q. Negative mass means antiparticle!              //
*//                                                                                 //
*// NOTES:                                                                          //
*// Antiparticle, v-spinor, is recognized according to sign of mass.                //
*// Spin sign is flipped for v-spinor.                                              //
*// This requires attention for zero-mass because zero mass particle is recognized  //
*// as u-spinor (no flip of spin).                                                  //
*// The way out is to give to antiparticle very small mass like -1e-150.            //
*//                                                                                 //
*// Note also that mp=0 or mq=0 for massive p, q, mean that p, q                    //
*// are projected on xi.                                                            //
*//                                                                                 //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INTEGER     Lamp,Lamq
      DOUBLE PRECISION       p(4),q(4),mp,mq
      DOUBLE COMPLEX   GPS_iProd1
      DOUBLE PRECISION       GPS_XiProd
      DOUBLE COMPLEX   Prod
      INTEGER     Lp,Lq

      Lp = Lamp
      Lq = Lamq
* Helicity for v-spinor is fliped
      IF( mp .LT. 0d0) Lp = -Lp
      IF( mq .LT. 0d0) Lq = -Lq
* Helicity conservation and non-conservation separately
      IF(     Lp .EQ. -Lq ) THEN
         Prod = GPS_iProd1(Lp,p,q)
      ELSEIF( Lp .EQ.  Lq ) THEN
         Prod = DCMPLX( mp*GPS_XiProd(q,p) +mq*GPS_XiProd(p,q), 0d0)
      ELSE
         WRITE(*,*) '##### GPS_iProd2: Wrong Lp,Lq= ', Lp,Lq
      ENDIF
      GPS_iProd2 = Prod
      END       

      DOUBLE PRECISION  FUNCTION GPS_XiProd(p,q)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//   auxiliary function called in GPS_iProd2                                       //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      DOUBLE PRECISION  p(4),q(4)

      GPS_XiProd = DSQRT( (p(4)-p(1)) / (q(4)-q(1)) )
      END

      SUBROUTINE GPS_BornPrint(nout,Mode)
*///////////////////////////////////////////////////////////////////////////////
*//                                                                           //
*//   Printout of Born spin amplitudes ina a nice format                      //
*//   Mode=0    prints AmpBorn, principal Born spin amplitudes                //
*//   Mode=1,2  for test printouts (to be kicked out!!!!!)                    //
*//                                                                           //
*///////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GPS.h'
      INTEGER nout,Mode
*
      INTEGER j1,j2,j3,j4
*----------------------------------------
      WRITE(nout,*) ' '
      WRITE(nout,'(5a)') '++++++++++++++++++++++++++++++++++++',
     $                        ' Born spin amplitudes ',
     $                   '++++++++++++++++++++++++++++++++++++'
      DO j1=1,2
         DO j2=1,2
            WRITE(nout,'(a,4(a,4i2,a))')  '     ',
     $           (('{', 3-2*j1, 3-2*j2, 3-2*j3 , 3-2*j4 ,'}  ', j3=1,2),j4=1,2)
         ENDDO
      ENDDO
      DO j1=1,2
         DO j2=1,2
            IF(Mode .EQ. 0) THEN
               WRITE(nout,'(4(a,2f10.6,a))') 
     $              (('[',m_AmpBorn(j1,j2,j3,j4),'] ', j3=1,2),j4=1,2)
            ELSEIF(Mode .EQ. 1) THEN
               WRITE(nout,'(4(a,2f10.6,a))') 
     $              (('[',m_AmpBorn1(j1,j2,j3,j4),'] ', j3=1,2),j4=1,2)
            ELSEIF(Mode .EQ. 2) THEN
               WRITE(nout,'(4(a,2f10.6,a))') 
     $              (('[',m_AmpBorn2(j1,j2,j3,j4),'] ', j3=1,2),j4=1,2)
            ENDIF
         ENDDO
      ENDDO
      WRITE(nout,'(120i1)') (mode, j1=1,91)
      END

      SUBROUTINE GPS_BPrint(nout,word,AmpBorn)
*///////////////////////////////////////////////////////////////////////////////
*//                                                                           //
*//   Printout of Born 16 spin amplitudes in  a nice format                   //
*//                                                                           //
*///////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GPS.h'
      INTEGER      nout
      CHARACTER*8  word
      DOUBLE COMPLEX    AmpBorn(2,2,2,2)
      DOUBLE PRECISION        Sum
*
      INTEGER     j1,j2,j3,j4
*----------------------------------------
      WRITE(nout,'(a)') ' '
      WRITE(nout,'(4a)') '+++++++++++++++++++++++++++++++++++++++++++++',
     $                             ' Born amplits: ', word,
     $                  ' +++++++++++++++++++++++++++++++++++++++++++++'
***      DO j1=1,2
***         DO j2=1,2
***            WRITE(*,'(a,4(a,4i2,a))')  '     ',
***     $           (('{', 3-2*j1, 3-2*j2, 3-2*j3 , 3-2*j4 ,'}  ', j3=1,2),j4=1,2)
***         ENDDO
***      ENDDO
      DO j1=1,2
         DO j2=1,2
*@@@@       WRITE(nout,'(4(a,2f10.6,a))') 
            WRITE(nout,'(4(a,2g14.6,a))') 
     $           (('[',AmpBorn(j1,j2,j3,j4),'] ', j3=1,2),j4=1,2)
         ENDDO
      ENDDO
      Sum=0d0
      DO j1=1,2
         DO j2=1,2
            DO j3=1,2
               DO j4=1,2
                  Sum=Sum+CDABS(AmpBorn(j1,j2,j3,j4))**2
               ENDDO
            ENDDO
         ENDDO
      ENDDO
      WRITE(nout,'(a,g20.12)') '++++++++++ Sum= ',Sum
      END

      SUBROUTINE GPS_RmatPrint(nout,word)
*///////////////////////////////////////////////////////////////////////////////
*//                                                                           //
*//   Born similar as in BornV, mass terms exact for photon exchange!         //
*//                                                                           //
*///////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GPS.h'
      INTEGER      nout
      CHARACTER*8  word
*
      INTEGER    k,l
*----------------
      WRITE(nout,'(a)') ' '
      WRITE(nout,'(4a)') '#####################################',
     $                    ' R-matrix ', word,
     $                   ' #####################################'
      DO k=0,3
         WRITE(nout,'(4(a,2f10.6,a))')  ('[',m_Rmat(k,l),'] ', l=0,3)
      ENDDO
*----------------
      END

      SUBROUTINE GPS_UPrint(nout,word,U)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//   Print U-matrix in a nice form                                                 //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      DOUBLE COMPLEX  U(2,2)
      INTEGER k,l
      CHARACTER*8  word
*
      WRITE(nout,'(a)') '  '
      WRITE(nout,'(4a)') '////////////////////////////////////////',
     $                             ' T-matrix: ',word,
     $                  ' ////////////////////////////////////////'
      DO k=1,2
         WRITE(nout,'(2(a,2f20.12,a))')  ('[',U(k,l),' ]     ', l=1,2)
      ENDDO
      END


      SUBROUTINE GPS_Amp1Print(nout,word,Amp1Phot)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//   Print 1-photon 32 spin amplitudes in a readible format on unit nout           //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      DOUBLE COMPLEX    Amp1Phot(2,2,2,2,2)
      CHARACTER*8  word
      INTEGER      j1,j2,j3,j4,k
      DOUBLE PRECISION        Sum
*
      WRITE(nout,'(a)') ' '
      WRITE(nout,'(4a)') '++++++++++++++++++++++++++++++++++',
     $                    ' 1-phot amplits: ',word,
     $                  ' ++++++++++++++++++++++++++++++++++'
      DO j1=1,2
         DO j2=1,2
            WRITE(*,'(a,4(a,4i2,a))')  '     ',
     $           (('{', 3-2*j1, 3-2*j2, 3-2*j3 , 3-2*j4 ,'}  ', j3=1,2),j4=1,2)
         ENDDO
      ENDDO
      DO k=1,2
         DO j1=1,2
            DO j2=1,2
****               WRITE(nout,'(4(a,2f14.8,a))') 
               WRITE(nout,'(4(a,2g14.6,a))') 
     $              (('[',Amp1Phot(j1,j2,j3,j4,k),'] ', j3=1,2),j4=1,2)
            ENDDO
         ENDDO
      ENDDO
      Sum=0d0
      DO k=1,2
         DO j1=1,2
            DO j2=1,2
               DO j3=1,2
                  DO j4=1,2
                     Sum=Sum+ CDABS(Amp1Phot(j1,j2,j3,j4,k))**2
                  ENDDO
               ENDDO
            ENDDO
         ENDDO
      ENDDO
***      WRITE(nout,'(a,f20.12)') '++++++++++ Sum= ',Sum
      WRITE(nout,'(a,g20.12)') '++++++++++ Sum= ',Sum
      END
            
*/////////////////////////////////////////////////////////////////////////////////////
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//           Setters and Getters of CLASS  GPS                                     //
*//           Setters and Getters of CLASS  GPS                                     //
*//           Setters and Getters of CLASS  GPS                                     //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
*/////////////////////////////////////////////////////////////////////////////////////

      SUBROUTINE GPS_GetXi(xi,eta)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//   xi is basic lightlike vector in LAB frame entering definition of all spinors  //
*//   called k0 in Kleiss Stirling papers                                           //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GPS.h'
      DOUBLE PRECISION  xi(4),eta(4)
      INTEGER k
*--------------
      DO k=1,4
         xi(k)  =  m_Xi(k)
         eta(k) =  m_Eta(k)
      ENDDO
      END

      SUBROUTINE GPS_SetKeyArb(KeyArb)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//  KeyArb is for switching on/off the use of m_b,   KeyArb=0 means b=Xi           //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GPS.h'
      INTEGER KeyArb
      m_KeyArb = KeyArb
      END

      SUBROUTINE GPS_GetKeyArb(KeyArb)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//  KeyArb is for switching on/off the use of m_b,   KeyArb=0 means b=Xi           //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GPS.h'
      INTEGER KeyArb
      KeyArb = m_KeyArb
      END

      SUBROUTINE GPS_Setb1
*/////////////////////////////////////////////////////////////////////////////////////
*//   b1-->b                                                                        //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GPS.h'
      INTEGER k
      DO k=1,4
         m_b( k) = m_b1( k)
      ENDDO
      END

      SUBROUTINE GPS_Setb2
*/////////////////////////////////////////////////////////////////////////////////////
*//   b2-->b                                                                        //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GPS.h'
      INTEGER k
      DO k=1,4
         m_b( k) = m_b2( k)
      ENDDO
      END

      SUBROUTINE GPS_Setb3
*/////////////////////////////////////////////////////////////////////////////////////
*//   b3-->b                                                                        //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GPS.h'
      INTEGER k
      DO k=1,4
         m_b( k) = m_b3( k)
      ENDDO
      END

      SUBROUTINE GPS_GetRmat(Rmat)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//   Export m_Rmat to outside world                                                //
*//   Note that Rmat is REAL while m_Rmat is still COMPLEX                          //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GPS.h'
      DOUBLE PRECISION      sum,sum1,Rmat(0:3,0:3)
      INTEGER    i,j
*-----------------------------------
* Save old result
      DO i=0,3
         DO j=0,3
            Rmat(i,j)=m_Rmat(i,j)
         ENDDO
      ENDDO
      END

      SUBROUTINE GPS_GetDebg(j,y)
*////////////////////////////////////////////////////////////////////////////////
*//                                                                            //
*////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GPS.h'
*
      DOUBLE PRECISION  y
      INTEGER j
*---------------------------------------------------------------
      y = m_Debg(j)
      END   ! GPS_GetDebg


      SUBROUTINE GPS_SetDebg(j,y)
*////////////////////////////////////////////////////////////////////////////////
*//                                                                            //
*////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GPS.h'
*
      DOUBLE PRECISION  y
      INTEGER j
*---------------------------------------------------------------
      IF(j.LT.0 .OR. j.GT.200) THEN
         WRITE(*,*) ' STOP in GPS_SetDebg: j= ',j
         STOP
      ENDIF
      m_Debg(j) =y
      END   ! GPS_SetDebg


      SUBROUTINE GPS_ZeroWtSet
*/////////////////////////////////////////////////////////////////////////////////////
*//   Zeroing weght list                                                            //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GPS.h'
      INTEGER  j
*
      DO j=1,m_lenwt
         m_WtSet(j)=0d0
      ENDDO
      END   ! GPS_ZeroWtSet

      SUBROUTINE GPS_GetWtSet(WtBest,WtSet)
*/////////////////////////////////////////////////////////////////////////////////////
*//   Export list of weights                                                        //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      DOUBLE PRECISION    WtBest,WtSet(*)
      INCLUDE 'GPS.h'
*
      INTEGER  j
*--------------------------------------------------------------
      WtBest = m_WtBest
* collection of all weights
      DO j=1,m_lenwt
         WtSet(j)= m_WtSet(j)
      ENDDO
      END                       !!!GPS_GetWtSet!!!

      SUBROUTINE GPS_SetKeyINT(KeyINT)
*/////////////////////////////////////////////////////////////////////////////////////
*//   Set INTerference ISR*FSR switch                                               //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GPS.h'
      INTEGER KeyINT
*
      m_KeyINT = KeyINT
      END


      SUBROUTINE GPS_SetPolBeams(PolBeam1,PolBeam2)
*/////////////////////////////////////////////////////////////////////////////////////
*//   Seting beam POLARIZATION vectors                                              //
*//   Dont forget Wigner rotation to GPS frame!!!!                                  //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GPS.h'
      INTEGER i,j,k
      DOUBLE PRECISION  PolBeam1(4),PolBeam2(4)
*------------------------------------------------------------------------------------
      DO k=1,4
         m_PolBeam1( k) = PolBeam1(k)
         m_PolBeam2( k) = PolBeam2(k)
      ENDDO
* Define spin density matriced
      DO i=1,2
         DO j=1,2
            m_SDMat1(i,j)=0D0
            m_SDMat2(i,j)=0D0
            DO k=1,4
               m_SDMat1(i,j)=m_SDMat1(i,j)+m_Pauli4( k,i,j) *PolBeam1(k)
               m_SDMat2(i,j)=m_SDMat2(i,j)+m_Pauli4( k,i,j) *PolBeam2(k)
            ENDDO
         ENDDO
      ENDDO
      END

      SUBROUTINE GPS_SetHvectors(HvecFer1,HvecFer2)
*/////////////////////////////////////////////////////////////////////////////////////
*//   Seting final fermion POLARIMETER vectors                                      //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GPS.h'
      INTEGER i,j,k
      DOUBLE PRECISION  HvecFer1(4),HvecFer2(4)
*-------------------------------------------------------------------------------------
      DO k=1,4
         m_HvecFer1( k) = HvecFer1(k)
         m_HvecFer2( k) = HvecFer2(k)
      ENDDO
* Define immediately polarimeter density matriced
      DO i=1,2
         DO j=1,2
            m_SDMat3(i,j)=0D0
            m_SDMat4(i,j)=0D0
            DO k=1,4
               m_SDMat3(i,j)=m_SDMat3(i,j)+m_Pauli4( k,i,j) *HvecFer1(k)
               m_SDMat4(i,j)=m_SDMat4(i,j)+m_Pauli4( k,i,j) *HvecFer2(k)
            ENDDO
         ENDDO
      ENDDO
      END


      SUBROUTINE GPS_GetPolBeams(PolBeam1,PolBeam2)
*/////////////////////////////////////////////////////////////////////////////////////
*//   !!!!!!!!!!!!!!!!! TEMPORARY, no getters for these variables !!!!!!!!!!!!!!!!! //
*//   Geting beam POLARIZATION vectors                                              //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GPS.h'
      INTEGER k
      DOUBLE PRECISION  PolBeam1(4),PolBeam2(4)
*
      DO k=1,4
         PolBeam1( k) = m_PolBeam1(k)
         PolBeam2( k) = m_PolBeam2(k)
      ENDDO
      END

      SUBROUTINE GPS_GetHvectors(HvecFer1,HvecFer2)
*/////////////////////////////////////////////////////////////////////////////////////
*//   !!!!!!!!!!!!!!!!! TEMPORARY, no getters for these variables !!!!!!!!!!!!!!!!! //
*//   Geting final fermion POLARIMETER vectors                                      //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GPS.h'
      INTEGER k
      DOUBLE PRECISION  HvecFer1(4),HvecFer2(4)
*
      DO k=1,4
         HvecFer1( k) = m_HvecFer1(k)
         HvecFer2( k) = m_HvecFer2(k)
      ENDDO
      END



      SUBROUTINE GPS_Make_eps(ph,Sig,eps)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//  photon polarization 4-vector explicitelly calculated                           //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GPS.h'
      SAVE                      !!! <-- necessary !!!
*
      DOUBLE PRECISION      k1(4),k2(4),k3(4),k4(4),ph(4)
      DOUBLE PRECISION      k1m(4),k2m(4),k3m(4),k4m(4)
*
      DOUBLE PRECISION      Fleps
*-----------------------------------------------------------------------------
      INTEGER    j,k
      INTEGER    Sig
      DOUBLE COMPLEX  GPS_Sof1x,GPS_Sof1bx
      DOUBLE COMPLEX  eps(4)
      DOUBLE COMPLEX  x1,x2,x3,x1m,x2m,x3m
*-----------------------------------------------------------------------------
        DO k=1,4
         k1(k)=0D0
         k2(k)=0D0
         k3(k)=0D0
         k4(k)=0D0
         k1m(k)=0D0
         k2m(k)=0D0
         k3m(k)=0D0
         k4m(k)=0D0
        ENDDO
         k=4
         k1(k)=sqrt(2D0)
         k2(k)=1D0
         k3(k)=1D0
         k4(k)=1D0
         k1m(k)=sqrt(2D0)
         k2m(k)=1D0
         k3m(k)=1D0
         k4m(k)=1D0

         k1(1)=1D0
         k1(2)=1D0
         k2(2)=1D0
         k3(3)=1D0
         k4(4)=1D0
         k1m(1)=-1D0
         k1m(2)= 1D0
         k2m(2)=-1D0
         k3m(3)=-1D0
         k4m(4)=-1D0

      Fleps =  1d-100

      IF( m_KeyArb  .EQ.  0 ) THEN
         x1 =   GPS_Sof1x( 1,ph,k1 )
         x1m=   GPS_Sof1x( 1,ph,k1m)
         x2 =   GPS_Sof1x( 1,ph,k2 )
         x2m=   GPS_Sof1x( 1,ph,k2m)
         x3 =   GPS_Sof1x( 1,ph,k3 )
         x3m=   GPS_Sof1x( 1,ph,k3m)
      ELSE
         x1 =   GPS_Sof1bx( 1,ph,k1 ,Fleps)
         x1m=   GPS_Sof1bx( 1,ph,k1m,Fleps)
         x2 =   GPS_Sof1bx( 1,ph,k2 ,Fleps)
         x2m=   GPS_Sof1bx( 1,ph,k2m,Fleps)
         x3 =   GPS_Sof1bx( 1,ph,k3 ,Fleps)
         x3m=   GPS_Sof1bx( 1,ph,k3m,Fleps)
      ENDIF
         eps(4)=(x3+x3m)/2D0
         eps(3)=-(x3-x3m)/2D0
         eps(2)=-(x2-x2m)/2D0
         eps(1)=-(x1-x1m)/2D0

         DO j=1,4
          eps(j)=eps(j)/2d0
         ENDDO

         IF (sig.LT.0d0) THEN
           DO j=1,4
             eps(j)=-DCONJG(eps(j))
           ENDDO
         ENDIF
         RETURN

         write(*,*) 'vec  ph=',ph
         write(*,*) 'sig=',sig
         write(*,*) 'eps(i)=',eps

         write(*,*) 'consistency check; product=',
     $               ((eps(4)*ph(4)-eps(3)*ph(3)-eps(2)*ph(2)-eps(1)*ph(1)))
         write(*,*) 'consistency check; square=',
     $               eps(4)*DCONJG(eps(4))-eps(3)*DCONJG(eps(3))-eps(2)*DCONJG(eps(2))-eps(1)*DCONJG(eps(1))


      END                       !!!GPS_make_eps!!!


*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//                      End of CLASS  GPS                                          //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////

      SUBROUTINE GPS_BornWPlusT(KFi,KFf,s,t,u,p1,m1,p2,m2,p3,m3,p4,m4,AmpBorn,AmpBornT)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//                                                                                 //
*//   Tests of BornW spin amplitudes using ortogonality relations:: to be removed?  //
*//                                                                                 //
*//   Input:                                                                        //
*//   KFi, Kff = beam and final fermion flavour codes (to define charges)           //
*//   PX       = s-chanel momentum for W propagator (not for spinors)               //
*//   pi,mi    are for spinors, not for W propagator                                //
*//   p1,m1    =fermion momentum and mass (beam)                                    //
*//   p2,m2    =fermion momentum and mass (beam)                                    //
*//   p3,m3    =fermion momentum and mass final state                               //
*//   p4,m4    =fermion momentum and mass final state                               //
*//                                                                                 //
*//   Output:                                                                       //
*//   AmpBornT     = amplitude                                                      //
*//                                                                                 //
*//                                                                                 //
*//   Notes:                                                                        //
*//   Electron mass neglected in spinors, this is why we may use Chisholm!          //
*//   Final fermion mass kept exactly. I doubt                                      //
*//   W in t-chanel.                                                                //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GPS.h'
      SAVE                      !!! <-- necessary !!!
*
      INTEGER    Mode,KFi,KFf,Level,JakKoralZ
      DOUBLE PRECISION      p1(4),p2(4),p3(4),p4(4),k1(4),k2(4),k3(4),k4(4)
      DOUBLE PRECISION      k1m(4),k2m(4),k3m(4),k4m(4)
      DOUBLE PRECISION      m1,m2,m3,m4,Xborn
*
      DOUBLE PRECISION      BornSum
      DOUBLE PRECISION      s,t,u
      DOUBLE PRECISION      Fleps,m_MW, m_GammW
*-----------------------------------------------------------------------------
      INTEGER    i,j,k,l
      INTEGER    j1,j2,j3,j4
      INTEGER    Hel1,Hel2,Hel3,Hel4
      DOUBLE COMPLEX  Cfac,AmpBornW
      DOUBLE COMPLEX  PropW
      DOUBLE COMPLEX  s31,s24,s14,s32
      DOUBLE COMPLEX  AmpBorn(2,2,2,2),AmpBornT(2,2,2,2)
      DOUBLE COMPLEX  Coef, IntIR
      DOUBLE COMPLEX  TT,UU
      DOUBLE COMPLEX  UWX1(2,2),UWX1m(2,2),VWX1(2,2),VWX1m(2,2)
      DOUBLE COMPLEX  UWX2(2,2),UWX2m(2,2),VWX2(2,2),VWX2m(2,2)
      DOUBLE COMPLEX  UWX3(2,2),UWX3m(2,2),VWX3(2,2),VWX3m(2,2)
      DOUBLE COMPLEX  GPS_iProd2
*-----------------------------------------------------------------------------
* Electroweak
      INTEGER      NCf,NCe
      DOUBLE PRECISION        T3e,Qe
      DOUBLE PRECISION        T3f,Qf
      DOUBLE COMPLEX    Cn,Ve,Ae,Vf,Af, VVCor, WVPi
*-----------------------------------------------------------------------------
      DOUBLE PRECISION        dummy
c[[[[[[[[[[!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
c      INTEGER    icont
c      DATA       icont /0/
c]]]]]]]]]]!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
*-----------------------------------------------------------------------------
      IF (ABS(KFf) .NE. 12)  RETURN
        return
        DO k=1,4
         k1(k)=0D0
         k2(k)=0D0
         k3(k)=0D0
         k4(k)=0D0
         k1m(k)=0D0
         k2m(k)=0D0
         k3m(k)=0D0
         k4m(k)=0D0
        ENDDO
         k=4
         k1(k)=sqrt(2D0)
         k2(k)=1D0
         k3(k)=1D0
         k4(k)=1D0
         k1m(k)=sqrt(2D0)
         k2m(k)=1D0
         k3m(k)=1D0
         k4m(k)=1D0

         k1(1)=1D0
         k1(2)=1D0
         k2(2)=1D0
         k3(3)=1D0
         k4(4)=1D0
         k1m(1)=-1D0
         k1m(2)= 1D0
         k2m(2)=-1D0
         k3m(3)=-1D0
         k4m(4)=-1D0

      JakKoralZ=0  ! warning: defined in 2 places ! to fix a potential problem .eq.1 like KORALZ .eq.0 possiby OK
      Fleps =  1d-100
      Cn=1D0
         CALL GPS_MakeUX(Cn,k1,Fleps, p3,m3,   p1,m1,    UWX1) ! v-a inside
         CALL GPS_MakeVX(Cn,k1,Fleps, p2,m2,   p4,m4,    VWX1) ! v-a inside
         CALL GPS_MakeUX(Cn,k1m,Fleps, p3,m3,   p1,m1,    UWX1m) ! v-a inside
         CALL GPS_MakeVX(Cn,k1m,Fleps, p2,m2,   p4,m4,    VWX1m) ! v-a inside

         CALL GPS_MakeUX(Cn,k2,Fleps, p3,m3,   p1,m1,    UWX2) ! v-a inside
         CALL GPS_MakeVX(Cn,k2,Fleps, p2,m2,   p4,m4,    VWX2) ! v-a inside
         CALL GPS_MakeUX(Cn,k2m,Fleps, p3,m3,   p1,m1,    UWX2m) ! v-a inside
         CALL GPS_MakeVX(Cn,k2m,Fleps, p2,m2,   p4,m4,    VWX2m) ! v-a inside
           write(*,*) k1
           write(*,*) fleps,m3,m1
           write(*,*) p3
           write(*,*) p1
!           CALL GPS_UPrint(0,' UWX1    ',UWX2)
!           CALL GPS_UPrint(0,' UWX2    ',UWX2)


         CALL GPS_MakeUX(Cn,k3,Fleps, p3,m3,   p1,m1,    UWX3) ! v-a inside
         CALL GPS_MakeVX(Cn,k3,Fleps, p2,m2,   p4,m4,    VWX3) ! v-a inside
         CALL GPS_MakeUX(Cn,k3m,Fleps, p3,m3,   p1,m1,    UWX3m) ! v-a inside
         CALL GPS_MakeVX(Cn,k3m,Fleps, p2,m2,   p4,m4,    VWX3m) ! v-a inside

        DO j1 = 1,2
           DO j2 = 1,2
              DO j3 = 1,2
                 DO j4 = 1,2
                    AmpBornT( j1,j2,j3,j4) = 0D0
                 ENDDO                  
              ENDDO
           ENDDO
        ENDDO



*=============================================================
* Get charges, izospin, color
         CALL BornV_GetParticle(KFi, dummy, Qe,T3e,NCe)
         CALL BornV_GetParticle(KFf, dummy, Qf,T3f,NCf)
*=============================================================
      Coef  =1.d0/2.d0/m_Sw2

      IF(JakKoralZ.eq.1) then
*       W Propagator: it looks crazy in this case ....
        CALL GPS_EWFFactW(KFi,KFf,s,u,PropW,WVPi)
      ELSE
        CALL GPS_EWFFactW(KFi,KFf,s,t,PropW,WVPi)
      ENDIF
        DO j1 = 1,2
           DO j2 = 1,2
              DO j3 = 1,2
                 DO j4 = 1,2
                    AmpBornT( j1,j2,j3,j4) = PropW/4d0    *( ! factor of 1/4 is due to versor norm
     $              (UWX2(j3,j1)+UWX2m(j3,j1))*(VWX2(j2,j4)+VWX2m(j2,j4))
     $             -(UWX1(j3,j1)-UWX1m(j3,j1))*(VWX1(j2,j4)-VWX1m(j2,j4))
     $             -(UWX2(j3,j1)-UWX2m(j3,j1))*(VWX2(j2,j4)-VWX2m(j2,j4)) ! g_\mu_\nu matrix in versor repr.
     $             -(UWX3(j3,j1)-UWX3m(j3,j1))*(VWX3(j2,j4)-VWX3m(j2,j4))
     $                                                  )/(-2D0) ! arbitrary fix-up 
                 ENDDO                  
              ENDDO
           ENDDO
        ENDDO
      write(*,*) ' ========= test =========='
      write(*,*) ' we compare Born ME using:'
      write(*,*) ' '
      write(*,*) ' (1) standard BORN of KK '
      Call GPS_BPrint(0,' BORN   ',AmpBorn)
      write(*,*) ' '
      write(*,*) ' (2) BORN from orthogonality and  buiding blocks for nunu:'
      Call GPS_BPrint(0,' BORN-T   ',AmpBornT)
      stop
*////////////////////////////////////////////////////////////////////////////////////////////
*//     Primitives formfactor-type for construction of spin amplitudes                     //
*////////////////////////////////////////////////////////////////////////////////////////////
      END                       !!!GPS_BornWPlusT!!!

      SUBROUTINE GPS_MakeUT(KFf,ph,Sig, p2,m2,   p4,m4,  p3,m3,   p1,m1,UW,VW)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//  Test of UW,VW transition matrices using UWX, VWX and orthogonality             //
*//  also, photon polarization 4-vector explicitelly calculated                     //
*//  to be kept?                                                                               //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'GPS.h'
      SAVE                      !!! <-- necessary !!!
*
      INTEGER    Mode,KFi,KFf,Level,JakKoralZ
      DOUBLE PRECISION      p1(4),p2(4),p3(4),p4(4),k1(4),k2(4),k3(4),k4(4),ph(4)
      DOUBLE PRECISION      k1m(4),k2m(4),k3m(4),k4m(4)
      DOUBLE PRECISION      m1,m2,m3,m4,Xborn
*
      DOUBLE PRECISION      BornSum
      DOUBLE PRECISION      s,t,u
      DOUBLE PRECISION      Fleps,m_MW, m_GammW
*-----------------------------------------------------------------------------
      INTEGER    i,j,k,l
      INTEGER    j1,j2,j3,j4
      INTEGER    Hel1,Hel2,Hel3,Hel4,Sig
      DOUBLE COMPLEX  Cfac,AmpBornW
      DOUBLE COMPLEX  PropW,GPS_Sof1x,GPS_Sof1bx
      DOUBLE COMPLEX  s31,s24,s14,s32
      DOUBLE COMPLEX  Cn,Coef, IntIR
      DOUBLE COMPLEX  TT,UU
      DOUBLE COMPLEX  UWX1(2,2),UWX1m(2,2),VWX1(2,2),VWX1m(2,2),UW(2,2),UWT(2,2)
      DOUBLE COMPLEX  UWX2(2,2),UWX2m(2,2),VWX2(2,2),VWX2m(2,2),VW(2,2),VWT(2,2)
      DOUBLE COMPLEX  UWX3(2,2),UWX3m(2,2),VWX3(2,2),VWX3m(2,2)
      DOUBLE COMPLEX  GPS_iProd2,eps(4)
      DOUBLE COMPLEX  x1,x2,x3,x1m,x2m,x3m,xp1
*-----------------------------------------------------------------------------
* Electroweak
      INTEGER      NCf,NCe
      DOUBLE PRECISION        T3e,Qe
      DOUBLE PRECISION        T3f,Qf
      DOUBLE COMPLEX    Ve,Ae,Vf,Af, VVCor, WVPi
*-----------------------------------------------------------------------------
      DOUBLE PRECISION        dummy
c[[[[[[[[[[!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
c      INTEGER    icont
c      DATA       icont /0/
c]]]]]]]]]]!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
*-----------------------------------------------------------------------------
      IF (ABS(KFf) .NE. 12)  RETURN
       if (sig.lt.0d0) return
         return
        DO k=1,4
         k1(k)=0D0
         k2(k)=0D0
         k3(k)=0D0
         k4(k)=0D0
         k1m(k)=0D0
         k2m(k)=0D0
         k3m(k)=0D0
         k4m(k)=0D0
        ENDDO
         k=4
         k1(k)=sqrt(2D0)
         k2(k)=1D0
         k3(k)=1D0
         k4(k)=1D0
         k1m(k)=sqrt(2D0)
         k2m(k)=1D0
         k3m(k)=1D0
         k4m(k)=1D0

         k1(1)=1D0
         k1(2)=1D0
         k2(2)=1D0
         k3(3)=1D0
         k4(4)=1D0
         k1m(1)=-1D0
         k1m(2)= 1D0
         k2m(2)=-1D0
         k3m(3)=-1D0
         k4m(4)=-1D0

      JakKoralZ=0  ! warning: defined in 2 places ! to fix a potential problem .eq.1 like KORALZ .eq.0 possiby OK
      Fleps =  1d-100

      IF( m_KeyArb  .EQ.  0 ) THEN
         x1 =   GPS_Sof1x( 1,ph,k1 )
         x1m=   GPS_Sof1x( 1,ph,k1m)
         x2 =   GPS_Sof1x( 1,ph,k2 )
         x2m=   GPS_Sof1x( 1,ph,k2m)
         x3 =   GPS_Sof1x( 1,ph,k3 )
         x3m=   GPS_Sof1x( 1,ph,k3m)
         xp1=   GPS_Sof1x( 1,ph,p1) 
      ELSE
         x1 =   GPS_Sof1bx( 1,ph,k1 ,Fleps)
         x1m=   GPS_Sof1bx( 1,ph,k1m,Fleps)
         x2 =   GPS_Sof1bx( 1,ph,k2 ,Fleps)
         x2m=   GPS_Sof1bx( 1,ph,k2m,Fleps)
         x3 =   GPS_Sof1bx( 1,ph,k3 ,Fleps)
         x3m=   GPS_Sof1bx( 1,ph,k3m,Fleps)
         xp1=   GPS_Sof1bx( 1,ph,p1, Fleps)
      ENDIF
         eps(4)=(x3+x3m)/2D0
         eps(3)=-(x3-x3m)/2D0
         eps(2)=-(x2-x2m)/2D0
         eps(1)=-(x1-x1m)/2D0
         write(*,*) '  >>>>  TEST of UWX, VWX <<<<'
         write(*,*) 'vec  ph=',ph
         write(*,*) 'sig=',sig
         write(*,*) 'raw eps(i)=',eps

         write(*,*) 'consistency check on Sof1(b)x; ratio=',
     $               ((eps(4)*p1(4)-eps(3)*p1(3)-eps(2)*p1(2)-eps(1)*p1(1))/xp1)
         write(*,*) 'we got however from Sof1(b)x and standard othogonality game twice photon polarization 4-vector.'
         do j1=1,4
          eps(j1)=eps(j1)/2d0
         enddo

         write(*,*) '    eps(i)=',eps
         Cn=1D0
         CALL GPS_MakeUX(Cn,k1,Fleps, p3,m3,   p1,m1,    UWX1) ! v-a inside
         CALL GPS_MakeVX(Cn,k1,Fleps, p2,m2,   p4,m4,    VWX1) ! v-a inside
         CALL GPS_MakeUX(Cn,k1m,Fleps, p3,m3,   p1,m1,    UWX1m) ! v-a inside
         CALL GPS_MakeVX(Cn,k1m,Fleps, p2,m2,   p4,m4,    VWX1m) ! v-a inside

         CALL GPS_MakeUX(Cn,k2,Fleps, p3,m3,   p1,m1,    UWX2) ! v-a inside
         CALL GPS_MakeVX(Cn,k2,Fleps, p2,m2,   p4,m4,    VWX2) ! v-a inside
         CALL GPS_MakeUX(Cn,k2m,Fleps, p3,m3,   p1,m1,    UWX2m) ! v-a inside
         CALL GPS_MakeVX(Cn,k2m,Fleps, p2,m2,   p4,m4,    VWX2m) ! v-a inside

         CALL GPS_MakeUX(Cn,k3,Fleps, p3,m3,   p1,m1,    UWX3) ! v-a inside
         CALL GPS_MakeVX(Cn,k3,Fleps, p2,m2,   p4,m4,    VWX3) ! v-a inside
         CALL GPS_MakeUX(Cn,k3m,Fleps, p3,m3,   p1,m1,    UWX3m) ! v-a inside
         CALL GPS_MakeVX(Cn,k3m,Fleps, p2,m2,   p4,m4,    VWX3m) ! v-a inside

        DO j1 = 1,2
           DO j2 = 1,2
              DO j3 = 1,2
                 DO j4 = 1,2
                 ENDDO                  
              ENDDO
           ENDDO
        ENDDO

      

      Coef  =1.d0/2.d0/m_Sw2

        DO j1 = 1,2
           DO j2 = 1,2
               UWT(j1,j2)=(1+sig)/2*(
     $                     eps(4)*(uwx3(j1,j2)+uwx3m(j1,j2))/2
     $                    +eps(3)*(uwx3(j1,j2)-uwx3m(j1,j2))/2
     $                    +eps(2)*(uwx2(j1,j2)-uwx2m(j1,j2))/2
     $                    +eps(1)*(uwx1(j1,j2)-uwx1m(j1,j2))/2
     $                     ) ! not fully tested, case of sig= 1
               UWT(j1,j2)=UWT(j1,j2)+(1-sig)/2D0*( 
     $                    -dconjg(eps(4))*(uwx3(j1,j2)+uwx3m(j1,j2))/2
     $                    -dconjg(eps(3))*(uwx3(j1,j2)-uwx3m(j1,j2))/2
     $                    -dconjg(eps(2))*(uwx2(j1,j2)-uwx2m(j1,j2))/2
     $                    -dconjg(eps(1))*(uwx1(j1,j2)-uwx1m(j1,j2))/2
     $                     )    ! tested to the end case of sig= -1
               VWT(j1,j2)=(1+sig)/2*(
     $                     eps(4)*(vwx3(j1,j2)+vwx3m(j1,j2))/2
     $                    +eps(3)*(vwx3(j1,j2)-vwx3m(j1,j2))/2
     $                    +eps(2)*(vwx2(j1,j2)-vwx2m(j1,j2))/2
     $                    +eps(1)*(vwx1(j1,j2)-vwx1m(j1,j2))/2
     $                     ) ! 
               VWT(j1,j2)=VWT(j1,j2)+(1-sig)/2D0*( 
     $                    -dconjg(eps(4))*(vwx3(j1,j2)+vwx3m(j1,j2))/2
     $                    -dconjg(eps(3))*(vwx3(j1,j2)-vwx3m(j1,j2))/2
     $                    -dconjg(eps(2))*(vwx2(j1,j2)-vwx2m(j1,j2))/2
     $                    -dconjg(eps(1))*(vwx1(j1,j2)-vwx1m(j1,j2))/2
     $                     )    ! 
           ENDDO
        ENDDO

           CALL GPS_UPrint(0,' UW    ',UW)
           CALL GPS_UPrint(0,' UW-T    ',UWT)
           write(*,*) 'CONCLUSION of two above being equal:'
           write(*,*) 'MakeUX acts on a 4-vector identically as combination of MakeUb MatrWm _times'
           write(*,*) 'acts on normalized to -1 polarization vector of photon'
           CALL GPS_UPrint(0,' VW    ',VW)
           CALL GPS_UPrint(0,' VW-T    ',VWT)
           write(*,*) 'CONCLUSION of two above being equal:'
           write(*,*) 'MakeVX acts on a 4-vector identically as combination of MakeVb MatrWm _times'
           write(*,*) 'acts on normalized to -1 polarization vector of photon'

       stop
*////////////////////////////////////////////////////////////////////////////////////////////
*//     Primitives formfactor-type for construction of spin amplitudes                     //
*////////////////////////////////////////////////////////////////////////////////////////////
      END                       !!!GPS_BornWPlusT!!!







