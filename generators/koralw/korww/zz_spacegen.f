      subroutine spacegen(mode,itype,svar,sprim,fakir,
     $                       bp1,bp2,bp3,bp4)
********************************************************
! ================================================================
! mode=0                                                         =
!        generates 4-momenta accordingly to itype of generation, =
!        calculates jacobian (out from 4-momenta)                =
! mode=1                                                         =
!        calculates jacobian (out from 4-momenta)                =
!        for itype generation branch                             =
! ================================================================
      implicit DOUBLE PRECISION (a-h,o-z)
      COMMON / WEKING / ENE,AMAZ,GAMMZ,AMEL,AMFIN,XK0,SINW2,IDE,IDF 
      COMMON / WEKIN2 / AMAW,GAMMW,GMU,ALPHAW 
      COMMON / MATPAR / PI,CEULER       
      COMMON / DECAYS / IFLAV(4), AMDEC(4) 
! This common can be everywhere, contains various switches
      COMMON / KeyKey/  KeyRad,KeyPhy,KeyTek,KeyMis,KeyDwm,KeyDwp
      save  / WEKING /,/ WEKIN2 /,/ MATPAR /,/ KeyKey/
      save  / DECAYS /
c ms      parameter (NRCH=100)
c ms      common /kanalarz/ fak,fakI(NRCH),ikan,MRCHAN,NRCHAN
c ms      save  /kanalarz/

      dimension bq1(4),bq2(4),bp1(4),bp2(4),bp3(4),bp4(4)
      common /nevik/ nevtru,ifprint
      save /nevik/
      SAVE
!
      ifprint=0
!      =====================================
!      ====================================
       IF (ITYPE .EQ. 70) THEN
!      =====================================
!      =====================================
! MS 2/20/99 fix for uuss WW part
         igcl=1
         CALL kinchce(IGCL,MODE,AmaW,gammW,svar,SPRIM,fakir,
     $     bP1,amdec(1),bP4,amdec(4),bP3,amdec(3),bP2,amdec(2))
c.. 4momenta for born, in effective CMS, z+ along qeff1
         if (mode.eq.0) then
           do i=1,4  ! bq-s not used any more, 
                     ! do not care how they are set
             bq1(i)=bp1(i)+bp2(i)
             bq2(i)=bp3(i)+bp4(i)
           enddo
         endif
         RETURN
       ENDIF
! MS 2/20/99 fix for uuss WW part END
!      =====================================
!      =====================================

!      =====================================
!      ====================================
       IF (ITYPE.GT.62.and.itype.lt.70) THEN
!      =====================================
!      =====================================
!
        IRODZ=ITYPE-62
        igcl=7
      IF     (IRODZ.EQ.1) THEN
       igcl=5
!       if (nevtru.eq.-236) ifprint=1
       CALL kinchce(IGCL,MODE,Amaz,gammz,svar,SPRIM,fakir,
     $     bP1,amdec(1),bP4,amdec(4),bP3,amdec(3),bP2,amdec(2))
       ifprint=0
      elseIF (IRODZ.EQ.2) THEN
       igcl=6
!       if (nevtru.eq.-269) ifprint=1
       CALL kinchce(IGCL,MODE,Amaz,gammz,svar,SPRIM,fakir,
     $     bP3,amdec(3),bP2,amdec(2),bP1,amdec(1),bP4,amdec(4))
       ifprint=0
      elseIF (IRODZ.EQ.3) THEN
       igcl=5
       CALL kinchce(IGCL,MODE,Amaz,gammz,svar,SPRIM,fakir,
     $     bP3,amdec(3),bP2,amdec(2),bP1,amdec(1),bP4,amdec(4))
      elseIF (IRODZ.EQ.4) THEN
       igcl=6
       CALL kinchce(IGCL,MODE,Amaz,gammz,svar,SPRIM,fakir,
     $     bP1,amdec(1),bP4,amdec(4),bP3,amdec(3),bP2,amdec(2))
      elseIF (IRODZ.EQ.5) THEN
       igcl=1
       CALL kinchce(IGCL,MODE,Amaw,gammw,svar,SPRIM,fakir,
     $     bP1,amdec(1),bP2,amdec(2),bP3,amdec(3),bP4,amdec(4))
      elseIF (IRODZ.EQ.6) THEN
       igcl=1
       CALL kinchce(IGCL,MODE,Amaz,gammz,svar,SPRIM,fakir,
     $     bP1,amdec(1),bP2,amdec(2),bP3,amdec(3),bP4,amdec(4))
      elseIF (IRODZ.EQ.7) THEN
       igcl=1
       CALL kinchce(IGCL,MODE,Amaz,amaz,svar,SPRIM,fakir,
     $     bP1,amdec(1),bP2,amdec(2),bP3,amdec(3),bP4,amdec(4))
      ENDIF
c.. 4momenta for born, in effective CMS, z+ along qeff1
      if (mode.eq.0) then
      do i=1,4
        bq1(i)=bp1(i)+bp2(i)
        bq2(i)=bp3(i)+bp4(i)
      enddo
      endif 
        RETURN
       ENDIF
!      =====================================
!      =====================================
!      =====================================
!      =====================================
!      =====================================
       IF (ITYPE.GT.58.and.itype.lt.63) THEN
!      =====================================
!      =====================================
!
        IRODZ=ITYPE-58
        igcl=10
      IF     (IRODZ.EQ.1) THEN
       CALL kinchce(IGCL,MODE,ADUM,GDUM,0.d0,SPRIM,fakir,
     $     bP2,amdec(2),bP3,amdec(3),bP1,amdec(1),bP4,amdec(4))
      ELSEIF (IRODZ.EQ.2) THEN
       if (nevtru.eq.-226) ifprint=1
       CALL kinchce(IGCL,MODE,ADUM,GDUM,1d0,SPRIM,fakir,
     $     bP3,amdec(3),bP2,amdec(2),bP1,amdec(1),bP4,amdec(4))
       ifprint=0
      ELSEIF (IRODZ.EQ.3) THEN
       CALL kinchce(IGCL,MODE,ADUM,GDUM,0d0,SPRIM,fakir,
     $     bP4,amdec(4),bP1,amdec(1),bP2,amdec(2),bP3,amdec(3))
      ELSEIF (IRODZ.EQ.4) THEN
       CALL kinchce(IGCL,MODE,ADUM,GDUM,1d0,SPRIM,fakir,
     $     bP1,amdec(1),bP4,amdec(4),bP2,amdec(2),bP3,amdec(3))
      ENDIF
c.. 4momenta for born, in effective CMS, z+ along qeff1
      if (mode.eq.0) then
      do i=1,4
        bq1(i)=bp1(i)+bp2(i)
        bq2(i)=bp3(i)+bp4(i)
      enddo
      endif 
        RETURN
       ENDIF
!      =====================================
!      =====================================

!      =====================================
!      =====================================
       IF (ITYPE.GT.54.and.itype.lt.59) THEN
!      =====================================
!      =====================================
!
        IRODZ=ITYPE-54
        igcl=9
      IF     (IRODZ.EQ.1) THEN
       CALL kinchce(IGCL,MODE,AMAZ,-GAMMZ,0D0,SPRIM,fakir,
     $     bP1,amdec(1),bP4,amdec(4),bP3,amdec(3),bP2,amdec(2))
      elseIF (IRODZ.EQ.2) THEN
       CALL kinchce(IGCL,MODE,AMAZ,-GAMMZ,1D0,SPRIM,fakir,
     $     bP4,amdec(4),bP1,amdec(1),bP3,amdec(3),bP2,amdec(2))
      ELSEIF (IRODZ.EQ.3) THEN
       if (nevtru.eq.-7) ifprint=1
       CALL kinchce(IGCL,MODE,AMAZ,-GAMMZ,0D0,SPRIM,fakir,
     $     bP3,amdec(3),bP2,amdec(2),bP1,amdec(1),bP4,amdec(4))
       ifprint=0
      ELSEIF (IRODZ.EQ.4) THEN
       if (nevtru.eq.-43026) ifprint=1
       CALL kinchce(IGCL,MODE,AMAZ,-GAMMZ,1D0,SPRIM,fakir,
     $     bP2,amdec(2),bP3,amdec(3),bP1,amdec(1),bP4,amdec(4))
       ifprint=0
      ENDIF
c.. 4momenta for born, in effective CMS, z+ along qeff1
      if (mode.eq.0) then
      do i=1,4
        bq1(i)=bp1(i)+bp2(i)
        bq2(i)=bp3(i)+bp4(i)
      enddo
      endif 
        RETURN
       ENDIF
!      =====================================
!      =====================================

!      ====================================
       IF (ITYPE.GT.50.and.itype.lt.55) THEN
!      =====================================
!      =====================================
!
        IRODZ=ITYPE-50
        igcl=7
      IF     (IRODZ.EQ.1) THEN
       igcl=5
!       if (nevtru.eq.-236) ifprint=1
       CALL kinchce(IGCL,MODE,Amaz,gammz,svar,SPRIM,fakir,
     $     bP3,amdec(3),bP4,amdec(4),bP1,amdec(1),bP2,amdec(2))
       ifprint=0
      elseIF (IRODZ.EQ.2) THEN
       igcl=6
!       if (nevtru.eq.-269) ifprint=1
       CALL kinchce(IGCL,MODE,Amaz,gammz,svar,SPRIM,fakir,
     $     bP1,amdec(1),bP2,amdec(2),bP3,amdec(3),bP4,amdec(4))
       ifprint=0
      elseIF (IRODZ.EQ.3) THEN
       igcl=5
       CALL kinchce(IGCL,MODE,Amaz,gammz,svar,SPRIM,fakir,
     $     bP1,amdec(1),bP2,amdec(2),bP3,amdec(3),bP4,amdec(4))
      elseIF (IRODZ.EQ.4) THEN
       igcl=6
       CALL kinchce(IGCL,MODE,Amaz,gammz,svar,SPRIM,fakir,
     $     bP3,amdec(3),bP4,amdec(4),bP1,amdec(1),bP2,amdec(2))
      ENDIF
c.. 4momenta for born, in effective CMS, z+ along qeff1
      if (mode.eq.0) then
      do i=1,4
        bq1(i)=bp1(i)+bp2(i)
        bq2(i)=bp3(i)+bp4(i)
      enddo
      endif 
        RETURN
       ENDIF
!      =====================================
!      =====================================

!      =====================================
!      =====================================
       IF (ITYPE.GT.46.and.itype.lt.51) THEN
!      =====================================
!      =====================================
!
        IRODZ=ITYPE-46
        igcl=8
      IF     (IRODZ.EQ.1) THEN
       CALL kinchce(IGCL,MODE,AMAZ,-gammz,0D0,SPRIM,fakir,
     $     bP1,amdec(1),bP2,amdec(2),bP3,amdec(3),bP4,amdec(4))
      elseIF (IRODZ.EQ.2) THEN
       CALL kinchce(IGCL,MODE,AMAZ,-gammz,1D0,SPRIM,fakir,
     $     bP2,amdec(2),bP1,amdec(1),bP3,amdec(3),bP4,amdec(4))
      ELSEIF (IRODZ.EQ.3) THEN
       CALL kinchce(IGCL,MODE,AMAZ,-gammz,0D0,SPRIM,fakir,
     $     bP3,amdec(3),bP4,amdec(4),bP1,amdec(1),bP2,amdec(2))
      ELSEIF (IRODZ.EQ.4) THEN
       CALL kinchce(IGCL,MODE,AMAZ,-gammz,1D0,SPRIM,fakir,
     $     bP4,amdec(4),bP3,amdec(3),bP1,amdec(1),bP2,amdec(2))
      ENDIF
c.. 4momenta for born, in effective CMS, z+ along qeff1
      if (mode.eq.0) then
      do i=1,4
        bq1(i)=bp1(i)+bp2(i)
        bq2(i)=bp3(i)+bp4(i)
      enddo
      endif 
        RETURN
       ENDIF
!      =====================================
!      =====================================
       IF (ITYPE.GT.42.and.itype.lt.47) THEN
!      =====================================
!      =====================================
!
        IRODZ=ITYPE-42
        igcl=10
      IF     (IRODZ.EQ.1) THEN
       CALL kinchce(IGCL,MODE,ADUM,GDUM,0.d0,SPRIM,fakir,
     $     bP4,amdec(4),bP3,amdec(3),bP1,amdec(1),bP2,amdec(2))
      ELSEIF (IRODZ.EQ.2) THEN
       if (nevtru.eq.-226) ifprint=1
       CALL kinchce(IGCL,MODE,ADUM,GDUM,1d0,SPRIM,fakir,
     $     bP3,amdec(3),bP4,amdec(4),bP1,amdec(1),bP2,amdec(2))
       ifprint=0
      ELSEIF (IRODZ.EQ.3) THEN
       CALL kinchce(IGCL,MODE,ADUM,GDUM,0d0,SPRIM,fakir,
     $     bP2,amdec(2),bP1,amdec(1),bP3,amdec(3),bP4,amdec(4))
      ELSEIF (IRODZ.EQ.4) THEN
       CALL kinchce(IGCL,MODE,ADUM,GDUM,1d0,SPRIM,fakir,
     $     bP1,amdec(1),bP2,amdec(2),bP3,amdec(3),bP4,amdec(4))
      ENDIF
c.. 4momenta for born, in effective CMS, z+ along qeff1
      if (mode.eq.0) then
      do i=1,4
        bq1(i)=bp1(i)+bp2(i)
        bq2(i)=bp3(i)+bp4(i)
      enddo
      endif 
        RETURN
       ENDIF
!      =====================================
!      =====================================

!      =====================================
!      =====================================
       IF (ITYPE.GT.38.and.itype.lt.43) THEN
!      =====================================
!      =====================================
!
        IRODZ=ITYPE-38
        igcl=9
      IF     (IRODZ.EQ.1) THEN
       if (nevtru.eq.-3320) ifprint=1
       CALL kinchce(IGCL,MODE,AMAZ,-GAMMZ,0D0,SPRIM,fakir,
     $     bP1,amdec(1),bP2,amdec(2),bP3,amdec(3),bP4,amdec(4))
       ifprint=0
      elseIF (IRODZ.EQ.2) THEN
       CALL kinchce(IGCL,MODE,AMAZ,-GAMMZ,1D0,SPRIM,fakir,
     $     bP2,amdec(2),bP1,amdec(1),bP3,amdec(3),bP4,amdec(4))
      ELSEIF (IRODZ.EQ.3) THEN
       if (nevtru.eq.-7) ifprint=1
       CALL kinchce(IGCL,MODE,AMAZ,-GAMMZ,0D0,SPRIM,fakir,
     $     bP3,amdec(3),bP4,amdec(4),bP1,amdec(1),bP2,amdec(2))
       ifprint=0
      ELSEIF (IRODZ.EQ.4) THEN
       if (nevtru.eq.-43026) ifprint=1
       CALL kinchce(IGCL,MODE,AMAZ,-GAMMZ,1D0,SPRIM,fakir,
     $     bP4,amdec(4),bP3,amdec(3),bP1,amdec(1),bP2,amdec(2))
       ifprint=0
      ENDIF
c.. 4momenta for born, in effective CMS, z+ along qeff1
      if (mode.eq.0) then
      do i=1,4
        bq1(i)=bp1(i)+bp2(i)
        bq2(i)=bp3(i)+bp4(i)
      enddo
      endif 
        RETURN
       ENDIF
!      =====================================
!      =====================================

!      =====================================
!      =====================================
       IF (ITYPE.GT.30.and.itype.lt.39) THEN
!      =====================================
!      =====================================
!
        IRODZ=ITYPE-30
        igcl=8
      IF     (IRODZ.EQ.1) THEN
       CALL kinchce(IGCL,MODE,AMAW,amaw,0D0,SPRIM,fakir,
     $     bP1,amdec(1),bP3,amdec(3),bP2,amdec(2),bP4,amdec(4))
      elseIF (IRODZ.EQ.2) THEN
       CALL kinchce(IGCL,MODE,AMAW,amaw,0D0,SPRIM,fakir,
     $     bP1,amdec(1),bP4,amdec(4),bP2,amdec(2),bP3,amdec(3))
      ELSEIF (IRODZ.EQ.3) THEN
       CALL kinchce(IGCL,MODE,AMAW,amaw,1D0,SPRIM,fakir,
     $     bP2,amdec(2),bP3,amdec(3),bP1,amdec(1),bP4,amdec(4))
      ELSEIF (IRODZ.EQ.4) THEN
       CALL kinchce(IGCL,MODE,AMAW,amaw,1D0,SPRIM,fakir,
     $     bP2,amdec(2),bP4,amdec(4),bP1,amdec(1),bP3,amdec(3))

      ELSEIF (IRODZ.EQ.5) THEN
         if (nevtru.eq.-80  ) ifprint=1
         if (nevtru.eq.-109856  ) write(*,*) nevtru
       CALL kinchce(IGCL,MODE,AMAW,amaw,0D0,SPRIM,fakir,
     $     bP3,amdec(3),bP1,amdec(1),bP4,amdec(4),bP2,amdec(2))
      ELSEIF (IRODZ.EQ.6) THEN
       CALL kinchce(IGCL,MODE,AMAW,amaw,0D0,SPRIM,fakir,
     $     bP3,amdec(3),bP2,amdec(2),bP4,amdec(4),bP1,amdec(1))
      ELSEIF (IRODZ.EQ.7) THEN
      if (nevtru.eq.-113526) write(*,*) 'nevtru=',nevtru
      if (nevtru.eq.-24) ifprint=1
       CALL kinchce(IGCL,MODE,AMAW,amaw,1D0,SPRIM,fakir,
     $     bP4,amdec(4),bP1,amdec(1),bP3,amdec(3),bP2,amdec(2))
      ELSEIF (IRODZ.EQ.8) THEN
!       if (nevtru.eq.-17170) ifprint=1
       if (nevtru.eq.-9743) ifprint=1
       CALL kinchce(IGCL,MODE,AMAW,amaw,1D0,SPRIM,fakir,
     $     bP4,amdec(4),bP2,amdec(2),bP3,amdec(3),bP1,amdec(1))
       ifprint=0
!      if (mode.eq.0) write(*,*) bp4
!      if (mode.eq.0) write(*,*) bp2
 !     if (mode.eq.0) write(*,*) bp3
!      if (mode.eq.0) write(*,*) bp1
  !    if (mode.eq.0) write(*,*) '==========================='
      ENDIF
c.. 4momenta for born, in effective CMS, z+ along qeff1
      if (mode.eq.0) then
      do i=1,4
        bq1(i)=bp1(i)+bp2(i)
        bq2(i)=bp3(i)+bp4(i)
      enddo
      endif 
        RETURN
       ENDIF
!      =====================================
!      =====================================
!      =====================================
!      ====================================
       IF (ITYPE.GT.20.and.itype.lt.31) THEN
!      =====================================
!      =====================================
!
        IRODZ=ITYPE-20
        igcl=7
      IF     (IRODZ.EQ.1) THEN
       igcl=3
       if (nevtru.eq.-8191) ifprint=1
       CALL kinchce(IGCL,MODE,Amaz,gammz,svar,SPRIM,fakir,
     $     bP1,amdec(1),bP2,amdec(2),bP3,amdec(3),bP4,amdec(4))
       ifprint=0
      elseIF (IRODZ.EQ.2) THEN
       igcl=3
       CALL kinchce(IGCL,MODE,Amaz,gammz,svar,SPRIM,fakir,
     $     bP1,amdec(1),bP4,amdec(4),bP2,amdec(2),bP3,amdec(3))
      ELSEIF (IRODZ.EQ.3) THEN
       CALL kinchce(IGCL,MODE,ADUM,GDUM,PDUM,SPRIM,fakir,
     $     bP4,amdec(4),bP3,amdec(3),bP1,amdec(1),bP2,amdec(2))
      ELSEIF (IRODZ.EQ.4) THEN
       CALL kinchce(IGCL,MODE,ADUM,GDUM,PDUM,SPRIM,fakir,
     $     bP3,amdec(3),bP4,amdec(4),bP1,amdec(1),bP2,amdec(2))
      ELSEIF (IRODZ.EQ.5) THEN
       CALL kinchce(IGCL,MODE,ADUM,GDUM,PDUM,SPRIM,fakir,
     $     bP2,amdec(2),bP1,amdec(1),bP3,amdec(3),bP4,amdec(4))
      ELSEIF (IRODZ.EQ.6) THEN
       CALL kinchce(IGCL,MODE,ADUM,GDUM,PDUM,SPRIM,fakir,
     $     bP1,amdec(1),bP2,amdec(2),bP3,amdec(3),bP4,amdec(4))
      ELSEIF (IRODZ.EQ.7) THEN
       CALL kinchce(IGCL,MODE,ADUM,GDUM,PDUM,SPRIM,fakir,
     $     bP2,amdec(2),bP3,amdec(3),bP4,amdec(4),bP1,amdec(1))
      ELSEIF (IRODZ.EQ.8) THEN
       CALL kinchce(IGCL,MODE,ADUM,GDUM,PDUM,SPRIM,fakir,
     $     bP3,amdec(3),bP2,amdec(2),bP4,amdec(4),bP1,amdec(1))
      ELSEIF (IRODZ.EQ.9) THEN
       CALL kinchce(IGCL,MODE,ADUM,GDUM,PDUM,SPRIM,fakir,
     $     bP4,amdec(4),bP1,amdec(1),bP2,amdec(2),bP3,amdec(3))
      ELSEIF (IRODZ.EQ.10) THEN
       CALL kinchce(IGCL,MODE,ADUM,GDUM,PDUM,SPRIM,fakir,
     $     bP1,amdec(1),bP4,amdec(4),bP2,amdec(2),bP3,amdec(3))
      ENDIF
c.. 4momenta for born, in effective CMS, z+ along qeff1
      if (mode.eq.0) then
      do i=1,4
        bq1(i)=bp1(i)+bp2(i)
        bq2(i)=bp3(i)+bp4(i)
      enddo
      endif 
        RETURN
       ENDIF
!      =====================================
!      =====================================

!      =====================================
!      =====================================
       IF (ITYPE.GT.10.and.itype.lt.21) THEN
!      =====================================
!      =====================================
        igcl=8
        IRODZ=ITYPE-10
      IF     (IRODZ.EQ.1) THEN
       CALL kinchce(IGCL,MODE,AMAW,GAMMW,0D0,SPRIM,fakir,
     $     bP1,amdec(1),bP2,amdec(2),bP3,amdec(3),bP4,amdec(4))
      ELSEIF (IRODZ.EQ.2) THEN
       CALL kinchce(IGCL,MODE,AMAZ,GAMMZ,0D0,SPRIM,fakir,
     $     bP1,amdec(1),bP4,amdec(4),bP2,amdec(2),bP3,amdec(3))
      ELSEIF (IRODZ.EQ.3) THEN
       CALL kinchce(IGCL,MODE,AMAW,amaw,0D0,SPRIM,fakir,
     $     bP4,amdec(4),bP3,amdec(3),bP2,amdec(2),bP1,amdec(1))
      ELSEIF (IRODZ.EQ.4) THEN
       CALL kinchce(IGCL,MODE,AMAW,amaw,0D0,SPRIM,fakir,
     $     bP1,amdec(1),bP3,amdec(3),bP2,amdec(2),bP4,amdec(4))
      ELSEIF (IRODZ.EQ.5) THEN
       CALL kinchce(IGCL,MODE,AMAW,amaw,0D0,SPRIM,fakir,
     $     bP1,amdec(1),bP4,amdec(4),bP2,amdec(2),bP3,amdec(3))

      ELSEIF (IRODZ.EQ.6) THEN
       CALL kinchce(IGCL,MODE,AMAW,GAMMW,1D0,SPRIM,fakir,
     $     bP4,amdec(4),bP3,amdec(3),bP2,amdec(2),bP1,amdec(1))
      ELSEIF (IRODZ.EQ.7) THEN
       CALL kinchce(IGCL,MODE,AMAZ,GAMMZ,1D0,SPRIM,fakir,
     $     bP4,amdec(4),bP1,amdec(1),bP3,amdec(3),bP2,amdec(2))
      ELSEIF (IRODZ.EQ.8) THEN
       CALL kinchce(IGCL,MODE,AMAW,amaw,1D0,SPRIM,fakir,
     $     bP1,amdec(1),bP2,amdec(2),bP3,amdec(3),bP4,amdec(4))
      ELSEIF (IRODZ.EQ.9) THEN
       CALL kinchce(IGCL,MODE,AMAW,amaw,1D0,SPRIM,fakir,
     $     bP4,amdec(4),bP2,amdec(2),bP3,amdec(3),bP1,amdec(1))
      ELSEIF (IRODZ.EQ.10) THEN
       CALL kinchce(IGCL,MODE,AMAW,amaw,1D0,SPRIM,fakir,
     $     bP4,amdec(4),bP1,amdec(1),bP3,amdec(3),bP2,amdec(2))

      ENDIF
c.. 4momenta for born, in effective CMS, z+ along qeff1
      if (mode.eq.0) then
      do i=1,4
        bq1(i)=bp1(i)+bp2(i)
        bq2(i)=bp3(i)+bp4(i)
      enddo
      endif
        RETURN
       ENDIF
!      =====================================
!      =====================================
       IF (ITYPE.GT. 6.and.itype.lt.11) THEN
!      =====================================
!      =====================================
        igcl=7
        IRODZ=ITYPE-6
      IF     (IRODZ.EQ.1) THEN
       CALL kinchce(IGCL,MODE,ADUM,GDUM,PDUM,SPRIM,fakir,
     $     bP1,amdec(1),bP4,amdec(4),bP2,amdec(2),bP3,amdec(3))
      ELSEIF (IRODZ.EQ.2) THEN
       CALL kinchce(IGCL,MODE,ADUM,GDUM,PDUM,SPRIM,fakir,
     $     bP4,amdec(4),bP1,amdec(1),bP2,amdec(2),bP3,amdec(3))
      ELSEIF (IRODZ.EQ.3) THEN
       CALL kinchce(IGCL,MODE,ADUM,GDUM,PDUM,SPRIM,fakir,
     $     bP2,amdec(2),bP3,amdec(3),bP4,amdec(4),bP1,amdec(1))
      ELSEIF (IRODZ.EQ.4) THEN
       CALL kinchce(IGCL,MODE,ADUM,GDUM,PDUM,SPRIM,fakir,
     $     bP3,amdec(3),bP2,amdec(2),bP4,amdec(4),bP1,amdec(1))
      ENDIF
c.. 4momenta for born, in effective CMS, z+ along qeff1
      if (mode.eq.0) then
      do i=1,4
        bq1(i)=bp1(i)+bp2(i)
        bq2(i)=bp3(i)+bp4(i)
      enddo
      endif 
        RETURN
       ENDIF
!      =====================================
!      =====================================
       IF (ITYPE.GT. 0.and.itype.lt. 7) THEN
!      =====================================
!      =====================================
!
        IRODZ=ITYPE
      IF     (IRODZ.EQ.1) THEN
       igcl=1    
       CALL kinchce(IGCL,MODE,Amaw,gammw,svar,SPRIM,fakir,
     $     bP1,amdec(1),bP2,amdec(2),bP3,amdec(3),bP4,amdec(4))
      elseIF (IRODZ.EQ.2) THEN
       igcl=2    
       CALL kinchce(IGCL,MODE,Amaw*5,gammw,svar,SPRIM,fakir,
     $     bP1,amdec(1),bP2,amdec(2),bP3,amdec(3),bP4,amdec(4))
      elseIF (IRODZ.EQ.3) THEN
       igcl=3
       CALL kinchce(IGCL,MODE,amaz,gammz,svar,SPRIM,fakir,
     $     bP1,amdec(1),bP4,amdec(4),bP3,amdec(3),bP2,amdec(2))
      elseIF (IRODZ.EQ.4) THEN
       igcl=4
       CALL kinchce(IGCL,MODE,amaw,gammw,svar,SPRIM,fakir,
     $     bP1,amdec(1),bP2,amdec(2),bP3,amdec(3),bP4,amdec(4))
      elseIF (IRODZ.EQ.5) THEN
       igcl=4
       CALL kinchce(IGCL,MODE,amaz,gammz,svar,SPRIM,fakir,
     $     bP1,amdec(1),bP4,amdec(4),bP3,amdec(3),bP2,amdec(2))
      elseIF (IRODZ.EQ.6) THEN
       igcl=4
       CALL kinchce(IGCL,MODE,amaz,gammz,svar,SPRIM,fakir,
     $     bP3,amdec(3),bP2,amdec(2),bP1,amdec(1),bP4,amdec(4))
      ENDIF
c.. 4momenta for born, in effective CMS, z+ along qeff1
      if (mode.eq.0) then
      do i=1,4
        bq1(i)=bp1(i)+bp2(i)
        bq2(i)=bp3(i)+bp4(i)
      enddo
      endif 
        RETURN
       ENDIF
!      =====================================
!      =====================================
   
       write(*,*) 'spacegen; wrong ITYPE=',itype
       stop
!!!
      end
