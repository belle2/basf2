      subroutine mm_spacegen(mode,itype,ambeam,svar,sprim,fakir,
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

      dimension amdet(4)
!!! MS new kinematics
!      dimension qeff1(4),qeff2(4)
      dimension bq1(4),bq2(4),bp1(4),bp2(4),bp3(4),bp4(4)
!      dimension pt1(4),pt2(4)
      dimension bq123(4),bq23(4)
      dimension wkbp1(4),wkbp2(4),wkbp3(4),wkbp4(4)
      dimension i_flav(4)
!!! MS new kinematics end
!#############################################################
!    GENERAL INITIALIZATION:                                 #
!    SETTING PRESAMPLER PARAMETERS AND MASSES IN ORDER       #
!    FOR PARTICULAR >ITYPE< SLOT-SPACE                       #
!    most of ITYPES differ by order of 4-vectors only        #
!#############################################################

      IF (ITYPE.EQ.1) THEN
        amx=amaw
        gamx=gammw
        amdet(1)=amdec(1)
        amdet(2)=amdec(2)
        amdet(3)=amdec(3)
        amdet(4)=amdec(4)
      ENDIF

      IF (MODE.EQ.1) THEN 
!##############################################
! INITIALIZATION FOR RECALCULATION MODE:      #
!  ANGLES AND S-i's FROM FOUR VECTORS         #
!##############################################
        IF(itype.EQ.1) THEN
          call invkin(ctn,fin,ct1n,fi1n,ct2n,fi2n,
     $                  amwmn,amwpn,  bp1,bp2,bp3,bp4)
          s1=amwmn**2
          s2=amwpn**2
        ELSE
          do i=1,4
            bq1(i)=bp1(i)+bp2(i)
            bq2(i)=bp3(i)+bp4(i)
            bq23(i)=bp2(i)+bp3(i)
            bq123(i)=bp1(i)+bp2(i)+bp3(i)
          enddo
          s1=dmas2(bq1)
          s2=dmas2(bq2)
          s12=s1
          s23=dmas2(bq23)
          s123=dmas2(bq123)
        ENDIF
      ENDIF

!(((((((((((((((((((((((
      IF(itype.EQ.1)THEN
!(((((((((((((((((((((((

!##############################################
! BASIC PART:                                 #
! MODE=0 GENERATION AND JACOBIAN CALCULATION  #
! MODE=1 JACOBIAN CALCULATION ONLY            #
!##############################################
!... s1,s2 subspace
        CALL RES2GN(mode,SVAR,SPRIM,AMx,GAMx,amdet,S1,S2,SSCRU)

!
! rejection
!m.s. 13.04      if(mode.eq.0.and.sscru.eq.0d0)  then
        if(sscru.eq.0d0)  then
!-- short-out ... 
          fakir=0D0
          return
        endif

! lambda factors, can be moved to res2/3-s or even out.
        x1=s1/sprim
        x2=s2/sprim
        bmain=sqrt( (1-x1-x2)**2 - 4*x1*x2 )
        xwm1=amdet(1)**2/s1
        xwm2=amdet(2)**2/s1
        bwm=sqrt( (1-xwm1-xwm2)**2 - 4*xwm1*xwm2 )
        xwp1=amdet(3)**2/s2
        xwp2=amdet(4)**2/s2
        bwp=sqrt( (1-xwp1-xwp2)**2 - 4*xwp1*xwp2 )
        wjac=bmain*bwp*bwm

!... production angles
        CALL cospro(mode,sprim,s1,s2,ctn,fin,xccos)

! decay angles
        CALL cosdec(mode,sprim,ct1n,fi1n,xccos1)
        CALL cosdec(mode,sprim,ct2n,fi2n,xccos2)

! for the overall jacobian
        fakp = xccos1*xccos2*xccos*sscru

c        write(6,*) 'OLD  fakp,wjac=',fakp,wjac
c        write(6,*) 'OLD crud c1,c2,c,ss',xccos1,xccos2,xccos,sscru
! security check
        if (fakp.eq.0d0 .or. wjac.eq.0d0) then
          write(6,*) 'spacegen: we have troubles;'
          write(6,*) '  fakp,wjac=',fakp,wjac
          write(6,*) 'crud c1,c2,c,ss',xccos1,xccos2,xccos,sscru
          write(6,*) 'itype,mode',itype,mode
          write(6,*) 'amdet',amdet
          write(6,*) 'sq(s1),sq(s2)',sqrt(s1),sqrt(s2)
!!         write(6,*) 1/fakp,1/wjac
!!         stop
        endif
       
        if (mode.eq.0) then 
!##############################################
!  KONSTRUCT FINAL 4-VECTORS etc:             #
!  MODE=0 ONLY                                #
!##############################################
!
          CALL kineww(sprim,ctn,fin,ct1n,fi1n,ct2n,fi2n,
     $           sqrt(s1),sqrt(s2),amdet,bq1,bq2,bp1,bp2,bp3,bp4)

c.. 4momenta for born, in effective CMS, z+ along qeff1
          do i=1,4
            bq1(i)=bp1(i)+bp2(i)
            bq2(i)=bp3(i)+bp4(i)
          enddo
        endif   

!((((((((((!((((((((((
      ENDIF
!((((((((((!((((((((((

!! ++++++++++++++++++++++++++++++++++
!! M.S. New Channels, new kinematics.

      DO i=1,4
        i_flav(i)=iflav(i)
      ENDDO


      IF(itype.GE.2 .AND. itype.LE.7) THEN
!     ========================
! extra slots fixed to parallel without permutation (ie 1234)
        CALL mm_permut_tab(itype,iperm,iflip)
        CALL mm_permut(1,iperm,amdec,amdet,
     $           bp1,bp2,bp3,bp4,wkbp1,wkbp2,wkbp3,wkbp4)
        CALL kinga_2parl(mode,iflip,sprim,ambeam,amdet,i_flav,
     @            itype,wkbp1,wkbp2,wkbp3,wkbp4,fakp,wjac)
        CALL mm_permut(-1,iperm,amdec,amdet,
     $           bp1,bp2,bp3,bp4,wkbp1,wkbp2,wkbp3,wkbp4)
c.. 4momenta for born, in effective CMS, z+ along qeff1
      do i=1,4
        bq1(i)=bp1(i)+bp2(i)
        bq2(i)=bp3(i)+bp4(i)
      enddo

      ELSEIF(itype.GE.8 .AND. itype.LE.13) THEN
!     =======================
! 8-10,11-13 parall. perm.
        CALL mm_permut_tab(itype,iperm,iflip)
        CALL mm_permut(1,iperm,amdec,amdet,
     $           bp1,bp2,bp3,bp4,wkbp1,wkbp2,wkbp3,wkbp4)
        CALL kinga_2parl(mode,iflip,sprim,ambeam,amdet,i_flav,
     @            itype,wkbp1,wkbp2,wkbp3,wkbp4,fakp,wjac)
        CALL mm_permut(-1,iperm,amdec,amdet,
     $           bp1,bp2,bp3,bp4,wkbp1,wkbp2,wkbp3,wkbp4)
c.. 4momenta for born, in effective CMS, z+ along qeff1
      do i=1,4
        bq1(i)=bp1(i)+bp2(i)
        bq2(i)=bp3(i)+bp4(i)
      enddo
! checks start
!      if(mode.eq.11 .and. fakp*wjac.ne.0d0) then
!        CALL kinga_2parl(1,iflip,sprim,ambeam,amdet,i_flav,
!     @            itype,bp1,bp2,bp3,bp4,fakp1,wjac1)
!        write(6,*)'jacek ',wjac/wjac1,fakp/fakp1
!      endif
! checks end

      ELSEIF(itype.GE.14 .AND. itype.LE.37) THEN
!     ========================
! 14-25,26-37 serl.1 perm.
        CALL mm_permut_tab(itype,iperm,iflip)
        CALL mm_permut(1,iperm,amdec,amdet,
     $           bp1,bp2,bp3,bp4,wkbp1,wkbp2,wkbp3,wkbp4)
        CALL kinga_2serl(mode,iflip,sprim,ambeam,amdet,i_flav,
     @            itype,wkbp1,wkbp2,wkbp3,wkbp4,fakp,wjac)
        CALL mm_permut(-1,iperm,amdec,amdet,
     $           bp1,bp2,bp3,bp4,wkbp1,wkbp2,wkbp3,wkbp4)
c.. 4momenta for born, in effective CMS, z+ along qeff1
      do i=1,4
        bq1(i)=bp1(i)+bp2(i)
        bq2(i)=bp3(i)+bp4(i)
      enddo

      ELSEIF(itype.GE.38 .AND. itype.LE.61) THEN
!     ========================
! 38-49,50-61 serl.2 perm.
        CALL mm_permut_tab(itype,iperm,iflip)
        CALL mm_permut(1,iperm,amdec,amdet,
     $           bp1,bp2,bp3,bp4,wkbp1,wkbp2,wkbp3,wkbp4)
        CALL kinga_2serl_2(mode,iflip,sprim,ambeam,amdet,i_flav,
     @            itype,wkbp1,wkbp2,wkbp3,wkbp4,fakp,wjac)
        CALL mm_permut(-1,iperm,amdec,amdet,
     $           bp1,bp2,bp3,bp4,wkbp1,wkbp2,wkbp3,wkbp4)
c.. 4momenta for born, in effective CMS, z+ along qeff1
      do i=1,4
        bq1(i)=bp1(i)+bp2(i)
        bq2(i)=bp3(i)+bp4(i)
      enddo

      ENDIF
!     =====

! rejection
      IF(mode.EQ.0 .AND. fakp.EQ.0d0)  THEN
!-- short-cut ... 
         fakir=0D0
         RETURN
      ENDIF

!... WE CALCULATE OVERALL JACOBIAN ...
      fak= 1D0/32D0*fakp*wjac
!...  EN_pi=(2*pi)**4/(2*(2*PI)**3)**(r;r=4) 
      ENPI=(2*PI)**4/(2*(2*PI)**3)**4
      fakir=fak*ENPI

      end


