*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//                     Pseudo-CLASS  HepEvt                                 //
*//                                                                          //
*//  Purpose:  keep and serve event in HEPEVT format                         //
*//                                                                          //
*//  Output of KK2f is encoded in double precission /d_hepevt/               //
*//  which is double precision version of /hepevt/                           //
*//  It was necessary to rename /hepevt/                                     //
*//  because older Jetset uses single precision version of /hepevt/          //
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////


      SUBROUTINE HepEvt_Fill
*//////////////////////////////////////////////////////////////////////////////////////
*//                                                                                  //
*//  Filling HEP COMMON block using HepEvt_Fil1 routine                              //
*//                                                                                  //
*//  INPUT:                                                                          //
*//     KFfin          final particle code       (KarLud)                            //
*//     pf1,pf2        beam (electron) momenta   (KarLud)                            //
*//     nphox          photon multiplicity       (KarLud)                            //
*//     xphot          photon momenta            (KarLud)                            //
*//     qf1,qf2        final momenta             (Karfin)                            //
*//     nphoy          photon multiplicity       (KarFin)                            //
*//     yphot          photon momenta            (KarFin)                            //
*//                                                                                  //
*//  !!!!!! To be disscussed with the users. !!!!!!                                  //
*//  In present version we add beamstrahlung photons to the record                   //
*//  along with the ISR photons. It can be missleading!                              //
*//  May be they should be rather listed                                             //
*//  as the initial state particles, together with beams?                            //
*//                                                                                  //
*//////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      DOUBLE PRECISION  pf1(4),pf2(4),qf1(4),qf2(4),xphot(100,4),yphot(100,4)
      DOUBLE PRECISION  aph(5), bph(5)
      DOUBLE PRECISION  amel,amfin,BornV_GetMass
      DOUBLE PRECISION  Psum(4), Etot
      DOUBLE PRECISION  WtMain,WtCrud
      INTEGER           KFfin,ip,kstat,j,i,nphoy,nphox,kfbeam
      DOUBLE PRECISION  pbos(4),xmtmp ! &&&
*------------------------------------------------------------------------------
* Actual KFcode of final fermion
      CALL KarLud_GetKFfin(KFfin)
* Beams and ISR photons
      CALL KarLud_GetPhotons(nphox,xphot)
      CALL KarLud_GetBeams(pf1,pf2)
* Final fermions and FSR photons
      CALL KarFin_GetPhotons(nphoy,yphot)
      CALL KarFin_GetFermions(qf1,qf2)
      DO j=1,4
         Psum(j)=qf1(j)+qf2(j)
      ENDDO
*
      KFbeam = 11    ! KF=11 denotes electron
      amel   =BornV_GetMass(KFbeam)
* Beams
      ip=1                      !(&&&
      CALL HepEvt_Fil1(ip,3, 11, 0,0,0,0, pf1,amel,.FALSE.)
      ip=2
      CALL HepEvt_Fil1(ip,3,-11, 0,0,0,0, pf2,amel,.FALSE.)
* Now mother of ffbar and photons added in event record 
*   1,2               - beams
*   3                 - Z/gamma*
*   4,...,4+nphot     - photons 
*   4+nphot,5+nphot   - final fermions   
      DO j=1,4
         pbos(j)=qf1(j)+qf2(j)
      ENDDO
      xmtmp=pbos(4)**2-pbos(1)**2-pbos(2)**2-pbos(3)**2
      xmtmp=DSQRT(ABS(xmtmp))
      ip=ip+1
      CALL HepEvt_Fil1(ip,2,23,1,2,0,0,pbos,xmtmp,.FALSE.)

* Radiative photons (4 ... 3+nphot) (pdg-code for gamma is 22)
* Note that both ISR and FSR photons have beams as parents.
* This is because (a) JETSET does not tolerate FSR photons attached to quarks
*                 (b) photons for CEEX model cannot be split into ISR and FSR
      CALL HepEvt_SetPhotStart(ip) ! remeber position of the 1-st photon
      IF(nphox .NE. 0) THEN
         DO i=1,nphox
            DO j=1,4
               aph(j) = xphot(i,j)
               Psum(j)= Psum(j)+xphot(i,j)
            END DO
            ip=ip+1
            CALL HepEvt_Fil1(ip,1,22, 1,2,0,0, aph,0d0,.FALSE.) ! ISR
         END DO
      END IF
      IF(nphoy .NE. 0) THEN
         DO i=1,nphoy
            DO j=1,4
               aph(j) = yphot(i,j)
               Psum(j)= Psum(j)+yphot(i,j)
            END DO
            ip=ip+1
c$$$           CALL HepEvt_Fil1(4+ip,1,22, 2,1,0,0, aph,0d0,.FALSE.) ! FSR
            CALL HepEvt_Fil1(ip,1,22, 1,2,0,0, aph,0d0,.FALSE.) ! FSR
         END DO
      END IF
* add beamstrahlung photons to the record along with ISR photons, can be missleading!!
      CALL KarLud_GetBeasts( aph, bph)
      IF( aph(4).NE.0d0) THEN
         ip=ip+1
         CALL HepEvt_Fil1(ip,1,22, 1,2,0,0, aph,0d0,.FALSE.)
         DO j=1,4
            Psum(j)= Psum(j)+aph(j)
         ENDDO
      ENDIF
      IF( bph(4).NE.0d0) THEN
         ip=ip+1
         CALL HepEvt_Fil1(ip,1,22, 1,2,0,0, bph,0d0,.FALSE.)
         DO j=1,4
            Psum(j)= Psum(j)+bph(j)
         ENDDO
      ENDIF
      CALL HepEvt_SetPhotEnd(ip) ! remeber position of the last photon

*  Final state fermions (&&& moved after photons
      IF( KFfin .EQ. 0  ) RETURN
      amfin  =BornV_GetMass(KFfin)
      KStat = 1
      ip=ip+1
      CALL HepEvt_Fil1(ip,KStat, KFfin, 3,3,0,0, qf1,amfin,.FALSE.) ! &&& parent=3
      CALL HepEvt_SetF(ip)
      ip=ip+1
      CALL HepEvt_Fil1(ip,KStat,-KFfin, 3,3,0,0, qf2,amfin,.FALSE.) ! &&& parent=3
      CALL HepEvt_SetFbar(ip)

      Etot= SQRT(ABS(Psum(4)**2 -Psum(3)**2 -Psum(2)**2 -Psum(1)**2))
* Check on total 4-momentum conservation
      IF( ABS(Etot/(pf1(4)+pf2(4)+aph(4)+bph(4))-1d0) .GT.1d-4) THEN
         WRITE(*,*) '++++ HepEvt_Fill: something wrong with Etot=',Etot
      ENDIF
      IF( ABS(Psum(4)/(pf1(4)+pf2(4)+aph(4)+bph(4))-1d0) .GT.1d-4) THEN
         WRITE(*,*) '++++ HepEvt_Fill: something wrong with Psum(4)=',Psum(4)
      ENDIF
* Finaly fill also LUND common block
      CALL pyhepc(2)
      END



      SUBROUTINE HepEvt_Hadronize(HadMin)
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//          Aranging jets and hadronization                                 //
*//                                                                          //
*//     VERY close to HepEvt, but contain a switch for the RRes_HADGEN       //
*//     RRes_HADGEN falls back to Jetset if qq mass > 2 GeV                  //
*//     Maarten Boonekamp, sept. 2001                                        //
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE '../KK2f/HepEvt.fi'
*
      INTEGER  ijoin(2)
      DOUBLE PRECISION    HadMin
      DOUBLE PRECISION    sqrs1
      INTEGER  ih1,ih2
C (M.B.)
      INTEGER KeyRes
      DOUBLE PRECISION QQMOM(4)
C end (M.B.)
* ----------------------------------------------------------------------

* Quarks only, KeyHad=1 required
c$$$      ih1=3  ! fermion is here      &&&
c$$$      ih2=4  ! antifermion is here  &&&
      Call HepEvt_GetF(   ih1)  ! fermion is here
      Call HepEvt_GetFbar(ih2)  ! antifermion is here
C (M.B.) Use RRes_HADGEN if requested for KeyRes=1
      CALL BornV_GetKeyRes(KeyRes)
      IF(KeyRes.EQ.0) THEN
        IF ( ABS(idhep(ih1)) .LT. 10 ) THEN 
** Explicit string arangement:
          ijoin(1) = ih1
          ijoin(2) = ih2
** q-qbar effective mass
          sqrs1=(phep(4,ih1)+phep(4,ih2))**2
     $         -(phep(3,ih1)+phep(3,ih2))**2
     $         -(phep(2,ih1)+phep(2,ih2))**2
     $         -(phep(1,ih1)+phep(1,ih2))**2
          sqrs1=sqrt(abs(sqrs1))
* Showering < HadMas cut-off value (this also deals with WT=0 events)
* see also bornv
          IF( sqrs1 .GT. HadMin**2) THEN
            CALL pyjoin(2,ijoin)
            CALL pyshow(ih1,ih2,sqrs1)
            CALL pyexec
* Finaly fill also HepEvt common block
            CALL pyhepc(1)  ! Pythia-->HepEvt
          ENDIF
        ENDIF
      ELSE ! RRes_HADGEN falls back to Jetset if qq mass > 2 GeV
        IF ( ABS(idhep(ih1)) .LT. 10 ) THEN 
          QQMOM(1) = phep(1,ih1)+phep(1,ih2)
          QQMOM(2) = phep(2,ih1)+phep(2,ih2)
          QQMOM(3) = phep(3,ih1)+phep(3,ih2)
          QQMOM(4) = phep(4,ih1)+phep(4,ih2)
          CALL RRes_HADGEN(QQMOM,ih1,ih2,0)  ! PHOTOS inside!
          CALL pyexec
* Finaly fill also HepEvt common block
          CALL pyhepc(1)   ! Pythia-->HepEvt
        ENDIF
      ENDIF
C end (M.B.)
      END


      SUBROUTINE HepEvt_Hadronize_ori(HadMin)
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//          Aranging jets and hadronization                                 //
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'HepEvt.fi'
*
      INTEGER  ijoin(2)
      DOUBLE PRECISION    HadMin
****      REAL                sqrs1      ! corrections of B.B.
      DOUBLE PRECISION    sqrs1
      INTEGER  ih1,ih2
* ----------------------------------------------------------------------

* Quarks only, KeyHad=1 required
c$$$      ih1=3  ! fermion is here      &&&
c$$$      ih2=4  ! antifermion is here  &&&
      Call HepEvt_GetF(   ih1)  ! fermion is here
      Call HepEvt_GetFbar(ih2)  ! antifermion is here
      IF ( ABS(idhep(ih1)) .LT. 10 ) THEN 
** Explicit string arangement:
         ijoin(1) = ih1
         ijoin(2) = ih2
** q-qbar effective mass
         sqrs1=(phep(4,ih1)+phep(4,ih2))**2
     $        -(phep(3,ih1)+phep(3,ih2))**2
     $        -(phep(2,ih1)+phep(2,ih2))**2
     $        -(phep(1,ih1)+phep(1,ih2))**2
         sqrs1=sqrt(abs(sqrs1))
* Showering < HadMas cut-off value (this also deals with WT=0 events)
* see also bornv
         IF( sqrs1 .GT. HadMin**2) THEN
            CALL pyjoin(2,ijoin)
            CALL pyshow(ih1,ih2,sqrs1)
            CALL pyexec
         ENDIF
      ENDIF
      
      END


      SUBROUTINE HepEvt_Fil1( n,ist,id,jmo1,jmo2,jda1,jda2,p4,pinv,phflag)
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*// This subroutine fills one entry into the hepevt common                   //
*// and updates the information for affected mother entries                  //
*// WRITTEN by Martin W. Gruenewald (91/01/28)                               //
*// Re-Edited by S. Jadach, 6 july 97                                        //
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'HepEvt.fi'
* input
      INTEGER           n,ist,id,jmo1,jmo2,jda1,jda2
      DOUBLE PRECISION  p4(4),pinv
      LOGICAL           phflag
* locals
      INTEGER           ip,i,ihep
      SAVE
* ----------------------------------------------------------------------
*
* check address mode
      IF (n .EQ. 0) THEN
* append mode
        ihep=nhep+1
      ELSE IF (n .GT. 0) THEN
* absolute position
        ihep=n
      ELSE
* relative position
        ihep=nhep+n
      END IF
*
* check on ihep
      IF ((ihep .LE. 0) .OR. (ihep .GT. nmxhep)) RETURN
*
* add entry
      nhep=ihep
      isthep(ihep)   =ist
      idhep(ihep)    =id
      jmohep(1,ihep) =jmo1
      IF(jmo1 .LT. 0)  jmohep(1,ihep)=jmohep(1,ihep)+ihep
      jmohep(2,ihep) =jmo2
      IF(jmo2 .LT. 0)  jmohep(2,ihep)=jmohep(2,ihep)+ihep
      jdahep(1,ihep) =jda1
      jdahep(2,ihep) =jda2
*
      DO i=1,4
         phep(i,ihep)=p4(i)
* KORAL-B and KORAL-Z do not provide vertex and/or lifetime informations
         vhep(i,ihep)=0d0
      END DO
      phep(5,ihep)=pinv
* flag for photos...
      qedrad(ihep)=phflag
* update process:
      DO ip=jmohep(1,ihep),jmohep(2,ihep)
         IF(ip .GT. 0)THEN
* IF there is a daughter at ihep, mother entry at ip has decayed
            IF(isthep(ip) .EQ. 1)isthep(ip)=2
* and daughter pointers of mother entry must be updated
            IF(jdahep(1,ip) .EQ. 0)THEN
               jdahep(1,ip)=ihep
               jdahep(2,ip)=ihep
            ELSE
               jdahep(2,ip)=max(ihep,jdahep(2,ip))
            END IF
         END IF
      END DO
*
      END

      SUBROUTINE HepEvt_SetPhotosFlagTrue(ihep)
*//////////////////////////////////////////////////////////////////////////////
* flag for PHOTOS set true
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'HepEvt.fi'
      INTEGER ihep
      qedrad(ihep)=.TRUE.
      END


      SUBROUTINE HepEvt_GetKFfin(KFfin)
*//////////////////////////////////////////////////////////////////////////////
*// Purpose:  Get KFcode of final fermion out of /hepevt/                    //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'HepEvt.fi'
      INTEGER KFfin
      INTEGER ih1 ! pjh
* ----------------------------------------------------------------------
      CALL HepEvt_GetF(ih1) ! pjh
      KFfin = idhep(ih1) ! pjh
      END ! GetKFfin

      SUBROUTINE HepEvt_GetBeams(pf1,pf2)
*//////////////////////////////////////////////////////////////////////////////
*// Purpose:  Get Beam momenta out of /hepevt/                               //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'HepEvt.fi'
      DOUBLE PRECISION  pf1(4),pf2(4)
      INTEGER k
* ----------------------------------------------------------------------
      DO k=1,4
         pf1(k) =phep(k,1)
         pf2(k) =phep(k,2)
      ENDDO
      END ! GetBeams
      SUBROUTINE HepEvt_GetParticle(Id,Istart,Iadress,pf1)
*//////////////////////////////////////////////////////////////////////////////
*// Purpose:  Get Id-partcle momenta out of /hepevt/                         //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'HepEvt.fi'
      DOUBLE PRECISION  pf1(4)
      INTEGER k,l,Id,Istart,Iadress
* ----------------------------------------------------------------------
      Iadress=-1
      DO l=Istart,nhep
        IF(Idhep(l).eq.Id) then
          Iadress=l
          DO k=1,4
             pf1(k) =phep(k,l)
          ENDDO
          RETURN
        ENDIF
      ENDDO
      END ! GetParticle

      SUBROUTINE HepEvt_GetFfins(pf1,pf2)
*//////////////////////////////////////////////////////////////////////////////
*// Purpose:  Get final fermion momenta out of /hepevt/                      //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'HepEvt.fi'
      DOUBLE PRECISION  pf1(4),pf2(4)
      INTEGER k
      INTEGER ih1,ih2 ! pjh
* ----------------------------------------------------------------------
      CALL HepEvt_GetF(ih1) ! pjh
      CALL HepEvt_GetFbar(ih2) ! pjh
      DO k=1,4
         pf1(k) =phep(k,ih1) ! pjh
         pf2(k) =phep(k,ih2) ! pjh
      ENDDO
      END ! GetFfins

      SUBROUTINE HepEvt_GetNPhot(nphot)
*//////////////////////////////////////////////////////////////////////////////
*// Purpose:  Get number of bremsstrahlung photons  out of /hepevt/          //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'HepEvt.fi'
      INTEGER nphot,kf,j,ih
      INTEGER ih1,ifot ! pjh
*
      nphot=0
      CALL HepEvt_GetPhotStart(ifot) ! pjh
      DO j=1,100
         ih = ifot+j ! pjh
         kf = idhep(ih)
* STOP if /hepevt/ ended or non-photon found
         IF(ih .GT. nhep) GOTO 110
         IF(kf .NE. 22)   GOTO 110
* Bremsstrahlung photons have by convention 1-st parent being 1-st beam (ISR)
* at ih=1 or first final fermion at (FSR)
         CALL HepEvt_GetF(ih1) ! pjh
         IF(jmohep(1,ih).EQ.1 .OR. jmohep(1,ih).EQ.ih1) nphot=nphot+1 ! pjh
      ENDDO
      RETURN
 110  CONTINUE
      END


      SUBROUTINE HepEvt_GetPhotAll(nphot,phot)
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*// Purpose:  Get ALL photons out of /hepevt/                                //
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'HepEvt.fi'
      DOUBLE PRECISION  phot(100,4)
      INTEGER nphot,j,k,ih,kf
      INTEGER ih1,ifot ! pjh

* ----------------------------------------------------------------------
      nphot=0
      CALL HepEvt_GetPhotStart(ifot) ! pjh
      DO j=1,100
         ih = ifot+j ! pjh
         kf = idhep(ih)
* STOP if /hepevt/ ended or non-photon found
         IF(ih .GT. nhep) GOTO 110
         IF(kf .NE. 22)   GOTO 110
* Bremsstrahlung photons have by convention 1-st parent being 1-st beam (ISR)
* at ih=1 or first final fermion  (FSR)
         CALL HepEvt_GetF(ih1) ! pjh
         IF(jmohep(1,ih).EQ.1 .OR. jmohep(1,ih).EQ.ih1) THEN ! pjh
            nphot=nphot+1
            DO  k=1,4
               phot(nphot,k) =phep(k,ih)
            ENDDO
         ENDIF
      ENDDO
      GOTO 900
 110  CONTINUE
      RETURN
 900  WRITE(*,*) '++++ HepEvt_GetPhotAll: To many photons!!!'
      END ! HepEvt_GetPhotAll

      SUBROUTINE HepEvt_GetPhotIni(nphot,phot)
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*// Purpose:  Get ISR photons out of /hepevt/                                //
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'HepEvt.fi'
      DOUBLE PRECISION  phot(100,4)
      INTEGER nphot,j,k,ih,kf
      INTEGER ifot ! pjh
* ----------------------------------------------------------------------
      nphot=0
      CALL HepEvt_GetPhotStart(ifot)
      DO j=1,100
         ih = ifot+j               ! pjh
         kf = idhep(ih)
* STOP if /hepevt/ ended or non-photon found
         IF(ih .GT. nhep) GOTO 110
         IF(kf .NE. 22)   GOTO 110
* ISR photons have by convention 1-st parent being 1-st beam (convention)
         IF(jmohep(1,ih) .EQ. 1) THEN
            nphot=nphot+1
            DO  k=1,4
               phot(nphot,k) =phep(k,ih)
            ENDDO
         ENDIF
      ENDDO
      GOTO 900
 110  CONTINUE
      RETURN
 900  WRITE(*,*) '++++ HepEvt_GetPhotIni: To many photons!!!'
      END ! HepEvt_GetPhotIni

      SUBROUTINE HepEvt_GetPhotFin(nphot,phot)
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*// Purpose:  Get FSR photons out of /hepevt/                                //
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'HepEvt.fi'
      DOUBLE PRECISION  phot(100,4)
      INTEGER nphot,j,k,ih,kf
      INTEGER ifot ! pjh
* ----------------------------------------------------------------------
      nphot=0
      CALL HepEvt_GetPhotStart(ifot) ! pjh
      DO j=1,100
         ih = ifot+j               ! pjh
         kf = idhep(ih)
* STOP if /hepevt/ ended or non-photon found
         IF(ih .GT. nhep) GOTO 110
         IF(kf .NE. 22)   GOTO 110
* FSR photons have 1-st parent being 2-nd beam (convention)
         IF(jmohep(1,ih) .EQ. 2) THEN
            nphot=nphot+1
            DO  k=1,4
               phot(nphot,k) =phep(k,ih)
            ENDDO
         ENDIF
      ENDDO
      GOTO 900
 110  CONTINUE
      RETURN
 900  WRITE(*,*) '++++ HepEvt_GetPhotFin: To many photons!!!'
      END ! HepEvt_GetPhotFin

      SUBROUTINE HepEvt_GetPhotBst(nphot,phot)
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*// Purpose:  Get Beamstrahlung photons out of /hepevt/                      //
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'HepEvt.fi'
      DOUBLE PRECISION  phot(100,4)
      INTEGER nphot,j,k,ih,kf
      INTEGER ifot ! pjh
* ----------------------------------------------------------------------
      nphot=0
      CALL HepEvt_GetPhotStart(ifot) ! pjh
      DO j=1,100
         ih = ifot+j               ! pjh
         kf = idhep(ih)
* STOP if /hepevt/ ended or non-photon found
         IF(ih .GT. nhep) GOTO 110
         IF(kf .NE. 22)   GOTO 110
* Beamsstrahlung photons have pT exactly zero
         IF( (phep(1,ih)**2 +phep(2,ih)) .EQ. 0d0) THEN
            nphot=nphot+1
            DO  k=1,4
               phot(nphot,k) =phep(k,ih)
            ENDDO
         ENDIF
      ENDDO
      GOTO 900
 110  CONTINUE
      RETURN
 900  WRITE(*,*) '++++ HepEvt_GetPhotBst: To many photons!!!'
      END ! HepEvt_GetPhotBst


      SUBROUTINE HepEvt_SetPhotStart(Posn)
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*// Purpose:  Set start position of photons in HepEvt common                 //
*//           First photon is at StartPosn+1                                 //
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'HepEvt.fi'
      INTEGER Posn
      m_PhotStart=Posn
      END

      SUBROUTINE HepEvt_GetPhotStart(Posn)
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*// Purpose:  Get start position of photons in HepEvt common                 //
*//           First photon is at StartPosn+1                                 //
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'HepEvt.fi'
      INTEGER Posn
      Posn=m_PhotStart
      END

      SUBROUTINE HepEvt_SetPhotEnd(Posn)
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*// Purpose:  Get end position of photons in HepEvt common                   //
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'HepEvt.fi'
      INTEGER Posn
      m_PhotEnd=Posn
      END

      SUBROUTINE HepEvt_GetPhotEnd(Posn)
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*// Purpose:  Get end position of photons in HepEvt common                   //
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'HepEvt.fi'
      INTEGER Posn
      Posn=m_PhotEnd
      END

      SUBROUTINE HepEvt_GetPhotNumb(Numb)
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*// Purpose:  Get number of photons in HepEvt common                         //
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'HepEvt.fi'
      INTEGER Numb
      Numb=m_PhotEnd-m_PhotStart
      END

      SUBROUTINE HepEvt_SetF(Posn)
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*// Purpose:  Set position of final state fermion in HepEvt common           //
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'HepEvt.fi'
      INTEGER Posn
      m_PosnF=Posn
      END

      SUBROUTINE HepEvt_GetF(Posn)
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*// Purpose:  Set position of final state fermion in HepEvt common           //
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'HepEvt.fi'
      INTEGER Posn
      Posn=m_PosnF
      END

      SUBROUTINE HepEvt_SetFbar(Posn)
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*// Purpose:  Set position of final state anti-fermion in HepEvt common      //
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'HepEvt.fi'
      INTEGER Posn
      m_PosnFbar=Posn
      END

      SUBROUTINE HepEvt_GetFbar(Posn)
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*// Purpose:  Set position of final state anti-fermion in HepEvt common      //
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'HepEvt.fi'
      INTEGER Posn
      Posn=m_PosnFbar
      END

      SUBROUTINE HepEvt_GetNhep(nnhep)
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*// Purpose:  Get no of particles in HepEvt common                           //
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'HepEvt.fi'
      INTEGER nnhep
      nnhep= nhep
      END

*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//                      End of CLASS  HepEvt                                //
*//////////////////////////////////////////////////////////////////////////////
