      SUBROUTINE tohad(ifhadm,ifhadp,PReco)
! This is modified koralw/interfaces/lundface
! * S. Jadach Jan. 1997
! * yet modified by W. Placzek, 7 Apr. 1998
! It implements some kind of simple color reconnection.
! It is activated when the input parameter PReco > 0.
! PReco - colour reconnection probability (to be supplied be the user).
      IMPLICIT NONE
      INTEGER ifhadm,ifhadp
! Colour Re-Connection probability
      DOUBLE PRECISION PReco

      INTEGER iflav(4)
      DOUBLE PRECISION   amdec(4)
      COMMON / decays / iflav, amdec

! This common can be everywhere, contains various switches
      COMMON / KeyKey /  KeyRad,KeyPhy,KeyTek,KeyMis,KeyDwm,KeyDwp
      INTEGER            KeyRad,KeyPhy,KeyTek,KeyMis,KeyDwm,KeyDwp

      REAL*4 PHEP,VHEP
      INTEGER NMXHEP,NEVHEP,NHEP,ISTHEP,IDHEP,JMOHEP,JDAHEP
      PARAMETER (NMXHEP=2000)
      COMMON/HEPEVT/NEVHEP,NHEP,ISTHEP(NMXHEP),IDHEP(NMXHEP),
     &JMOHEP(2,NMXHEP),JDAHEP(2,NMXHEP),PHEP(5,NMXHEP),VHEP(4,NMXHEP)

      INTEGER ijoin(2)
      INTEGER id1,id2,ireco,key4f
      REAL*4 sqrs1,sqrs2
      DOUBLE PRECISION drvec(1)
      DOUBLE PRECISION wt4f9
      INTEGER icwm,icwp
      SAVE
!
! lets first go to LUND and later hadronize
!
      CALL luhepc(2)
      DO  id1=5,nhep
         IF (idhep(id1).NE.22) THEN
            GOTO 8
         ENDIF
      ENDDO
 8    CONTINUE
      DO  id2=id1+2,nhep
         IF (idhep(id2).NE.22) THEN
            GOTO 18
         ENDIF
      ENDDO
 18   CONTINUE
!--------------------------
      IF (ifhadm .NE. 0 .OR. ifhadp .NE. 0) THEN
         ireco=0
         Key4f  = MOD(KeyMis,100)/10
         IF (key4f.ne.0) THEN 
! routine spdetx can make a random choice of two colour recombination
! pattern either WW or ZZ like. 
! It is necessary only for uudd ccss f.s.
            IF ( (abs(iflav(1)).eq.1).AND.(abs(iflav(4)).eq.1)
     $           .AND.(abs(iflav(2)).eq.2).and.(abs(iflav(3)).eq.2) 
     $           )  CALL spdetx(ireco)
            IF ( (abs(iflav(1)).eq.3).AND.(abs(iflav(4)).eq.3)
     $           .AND.(abs(iflav(2)).eq.4).and.(abs(iflav(3)).eq.4) 
     $           )  CALL spdetx(ireco)

c bug 2/20/99  
         icwm=iflav(1)
         icwp=iflav(3)
c***** This code should be executed only for qqQQ final states,
c***** add a protection for that. M.Verzocchi 15/3/99
        if (abs(iflav(1)).eq.abs(iflav(2)).and.
     @      abs(iflav(3)).eq.abs(iflav(4))) then
c***** End of the correction. There is an additional ENDIF below.
! select MIX non-diag. (uuss for example)
         IF(icwm.EQ.1.AND.icwp.EQ.4 .OR. icwm.EQ.4.AND.icwp.EQ.1 .OR.
!         ccdd
     @      icwm.EQ.2.AND.icwp.EQ.3 .OR. icwm.EQ.3.AND.icwp.EQ.2 .OR. 
!         uuss
     @      icwm.EQ.2.AND.icwp.EQ.5 .OR. icwm.EQ.5.AND.icwp.EQ.2 .OR. 
!         uubb
     @      icwm.EQ.4.AND.icwp.EQ.5 .OR. icwm.EQ.5.AND.icwp.EQ.4) THEN
!         ccbb

           CALL waga_ckm(1,wt4f9)  ! wt4f(9)=wtmod4f/(wtcc03+wtmod4f)

           call varran(drvec,1)
           IF(drvec(1).GT.wt4f9) THEN
! WW-type
             ireco=1
           ELSE
! ZZ-type
             ireco=0
           ENDIF
         ENDIF
c***** Terminate the ENDIF for the qqQQ check.
        endif
c***** End of correction.
c bug 2/20/99 

         ENDIF
!-------------------------Colour REconnection START
! Primitive ansatz, S.J. 29Dec96
! Corrected by W. Placzek, 7Apr98
! WP: Do "colour reconnection" only if 4 quarks in the final state!
         IF (abs(iflav(1)).LT.10 .AND. abs(iflav(2)).LT.10 .AND.
     $       abs(iflav(3)).LT.10 .AND. abs(iflav(4)).LT.10) THEN

            CALL varran(drvec,1)
            IF( drvec(1) .LT. PReco ) THEN
               IF(ireco.eq.0) THEN
                  ireco=1
               ELSE
                  ireco=0
               ENDIF
            ENDIF

         ENDIF
!-------------------------Colour REconnection END
         IF(ireco .EQ. 0) then  ! normal 1+2, 3+4 (WW or ZZ)
            IF (abs(iflav(1)) .LT. 10) then
               ijoin(1) = ID1
               ijoin(2) = ID1+1 
               call lujoin(2,ijoin)
            ENDIF
            IF (ABS(iflav(3)) .LT. 10) THEN
               ijoin(1) = ID2
               ijoin(2) = ID2+1 
               CALL lujoin(2,ijoin)
            ENDIF
!     That's it, shower and hadronize now
            sqrs1=phep(5,jmohep(1,id1))
            IF (ABS(iflav(1)).LT.10) CALL lushow(id1,id1+1,sqrs1)
            sqrs2=phep(5,jmohep(1,id2))
            IF (ABS(iflav(3)).LT.10) CALL lushow(id2,id2+1,sqrs2)
            CALL luexec
         ELSE  !  flipped (1+4, 2+3)
            ijoin(1) = id1
            ijoin(2) = id2+1 
            CALL lujoin(2,ijoin)
            ijoin(1) = id1+1
            ijoin(2) = id2 
            CALL lujoin(2,ijoin)
!     That's it, shower and hadronize now  
            sqrs1=(phep(4,id1)+phep(4,id2+1))**2
     $           -(phep(3,id1)+phep(3,id2+1))**2
     $           -(phep(2,id1)+phep(2,id2+1))**2
     $           -(phep(1,id1)+phep(1,id2+1))**2
            sqrs1=sqrt(abs(sqrs1))
            CALL lushow(id1,id2+1,sqrs1)
            sqrs2=(phep(4,id1+1)+phep(4,id2))**2
     $           -(phep(3,id1+1)+phep(3,id2))**2
     $           -(phep(2,id1+1)+phep(2,id2))**2
     $           -(phep(1,id1+1)+phep(1,id2))**2
            sqrs2=sqrt(abs(sqrs2))
            CALL lushow(id1+1,id2,sqrs2)
            CALL luexec
         ENDIF 
      ENDIF
      END

      subroutine waga_ckm(imode,waga)
*-----------------------------------------------------------------------
* storage of weight for ckm suppressed channels
* m.s. 3/16/99
* imode = -1 input 
*          1 output
*-----------------------------------------------------------------------
      implicit DOUBLE PRECISION (a-h,o-z)
      SAVE waga_lcl
      IF(imode.EQ.-1) THEN
        waga_lcl=waga
      ELSE
        waga=waga_lcl
      ENDIF
      END


