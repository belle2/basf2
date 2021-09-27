!=======================================================
!====== central library of phase-space routines  =======
!=======================================================
! (Re)Written by: M.Skrzypek        date: 
! Last update:             by:  
!=======================================================
! Notice,
! This is an (almost) COMMON-free environment.
! Please respect it and neither introduce new COMMONs
! nor modify the unfortunate ones that still survived.
!=======================================================
      SUBROUTINE mm_permut_tab(itype,iperm,iflip)
!     **************************************************
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      DIMENSION itable_par(6),itable_ser(24),itable_ser2(24)
      SAVE
      DATA init /0/

      IF(init.EQ.0) THEN
        init = 1
        itable_par(1)=1234
        itable_par(2)=1324
        itable_par(3)=1423

        itable_ser(1)=1234
        itable_ser(2)=1324
        itable_ser(3)=2314

        itable_ser(4)=1243
        itable_ser(5)=1423
        itable_ser(6)=2413

        itable_ser(7)=1342
        itable_ser(8)=1432
        itable_ser(9)=3412

        itable_ser(10)=2341
        itable_ser(11)=2431
        itable_ser(12)=3421

        itable_ser2(1)=1234
        itable_ser2(2)=1243
        itable_ser2(3)=1342

        itable_ser2(4)=2134
        itable_ser2(5)=2143
        itable_ser2(6)=2341

        itable_ser2(7)=3124
        itable_ser2(8)=3142
        itable_ser2(9)=3241

        itable_ser2(10)=4123
        itable_ser2(11)=4132
        itable_ser2(12)=4231

        DO i=1,3
          itable_par(3+i)=itable_par(i)
        ENDDO
        DO i=1,12
!!          itable_ser(12+i)=ifliper(itable_ser(i))
          itable_ser(12+i)=itable_ser(i)
          itable_ser2(12+i)=itable_ser2(i)
        ENDDO
      ENDIF

      iflip = 1

      IF(itype.GE.2 .AND. itype.LE.7) THEN
! extra slots fixed to parallel without permutation (ie 1234)
        iperm=itable_par(1)

      ELSEIF(itype.GE.8 .AND. itype.LE.13) THEN
! 8-10,11-13 parall. perm.
        iperm=itable_par(itype-7)
        IF(itype.GE.11) iflip = -1 

      ELSEIF(itype.GE.14 .AND. itype.LE.37) THEN
! 14-25,26-37 serl.1 perm.
        iperm=itable_ser(itype-13)
        IF(itype.GE.26) iflip = -1 

      ELSEIF(itype.GE.38 .AND. itype.LE.61) THEN
! 38-49,50-61 serl.2 perm.
        iperm=itable_ser2(itype-37)
        IF(itype.GE.50) iflip = -1 
      ENDIF

      END

      FUNCTION ifliper(iperm)
!     ***************************************************
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      SAVE

      it4= mod(iperm,10)
      it3= mod(iperm,100)/10
      it2= mod(iperm,1000)/100
      it1= mod(iperm,10000)/1000

      ifliper = 1000*it4 +100*it3 +10*it2 + it1

      END

      SUBROUTINE mm_permut(mode,iperm,amdec,wkamdec,
     $           bp1,bp2,bp3,bp4,wkbp1,wkbp2,wkbp3,wkbp4)
!     ***************************************************
! mode =  1    amdec -> wkamdec
!              bp*   -> wkbp*
! mode = -1    wkbp* -> bp*
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      DIMENSION bp1(4),bp2(4),bp3(4),bp4(4)
      DIMENSION wkbp1(4),wkbp2(4),wkbp3(4),wkbp4(4)
      DIMENSION amdec(4),wkamdec(4)
      DIMENSION it(4),pmtrx(4,4)
      SAVE

      it(4)= mod(iperm,10)
      it(3)= mod(iperm,100)/10
      it(2)= mod(iperm,1000)/100
      it(1)= mod(iperm,10000)/1000

      IF(mode.EQ.1) THEN
        DO i=1,4
          wkamdec(i)=amdec(it(i))

          pmtrx(1,i)=bp1(i)
          pmtrx(2,i)=bp2(i)
          pmtrx(3,i)=bp3(i)
          pmtrx(4,i)=bp4(i)

          wkbp1(i)=pmtrx(it(1),i)
          wkbp2(i)=pmtrx(it(2),i)
          wkbp3(i)=pmtrx(it(3),i)
          wkbp4(i)=pmtrx(it(4),i)
        ENDDO

c        write(6,*)'permut,mode',iperm,mode
c        write(6,*)bp1(4),bp2(4),bp3(4),bp4(4)
c        write(6,*)wkbp1(4),wkbp2(4),wkbp3(4),wkbp4(4)
c        write(6,*)amdec
c        write(6,*)wkamdec

      ELSEIF(mode.EQ.-1) THEN
        DO i=1,4
          pmtrx(it(1),i)=wkbp1(i)
          pmtrx(it(2),i)=wkbp2(i)
          pmtrx(it(3),i)=wkbp3(i)
          pmtrx(it(4),i)=wkbp4(i)

          bp1(i)=pmtrx(1,i)
          bp2(i)=pmtrx(2,i)
          bp3(i)=pmtrx(3,i)
          bp4(i)=pmtrx(4,i)
        ENDDO
c        write(6,*)'permut,mode',iperm,mode
c        write(6,*)wkbp1(4),wkbp2(4),wkbp3(4),wkbp4(4)
c        write(6,*)bp1(4),bp2(4),bp3(4),bp4(4)
      ENDIF

      END

      SUBROUTINE reader(itypek,qeff1,qeff2,dbp1,dbp2,dbp3,dbp4) 
!     *********************************************
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)   
      DIMENSION dbp1(4),dbp2(4),dbp3(4),dbp4(4),qeff1(4),qeff2(4) 
      read(13,*) itypek
      read(13,*) qeff1
      read(13,*) qeff2

c      read(13,*) dbp1  !original
c      read(13,*) dbp2  !original
c      read(13,*) dbp3  !original
c      read(13,*) dbp4  !original
c
c      read(13,*) dbp4  !conjug. ch.
c      read(13,*) dbp3  !conjug. ch.
c      read(13,*) dbp2  !conjug. ch.
c      read(13,*) dbp1  !conjug. ch.

      read(13,*) dbp1 
      read(13,*) dbp2 
      read(13,*) dbp3 
      read(13,*) dbp4 

      END

      SUBROUTINE writer(nout,itypek,qeff1,qeff2,dbp1,dbp2,dbp3,dbp4)
!     *********************************************
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)   
      DIMENSION dbp1(4),dbp2(4),dbp3(4),dbp4(4),qeff1(4),qeff2(4)
      write(nout,*) itypek
      write(nout,*) qeff1
      write(nout,*) qeff2
      write(nout,*) dbp1
      write(nout,*) dbp2
      write(nout,*) dbp3
      write(nout,*) dbp4
      END

      FUNCTION wlambda(s,s1,s2)
!     ********************************
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      x1=s1/s
      x2=s2/s
      wlambda=dsqrt( (1-x1-x2)**2 - 4*x1*x2 )
      END

      SUBROUTINE set_eff_beams(sprim,ambeam,qeff1,qeff2)
!     ***************************************************
!-- sets effective beams (CMS')
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      DIMENSION qeff1(4),qeff2(4)
      SAVE
        qeff1(1)=0d0
        qeff1(2)=0d0
        qeff1(3)=dsqrt(sprim/4d0  - ambeam**2)
        qeff1(4)=dsqrt(sprim/4d0)

        qeff2(1)=0d0
        qeff2(2)=0d0
        qeff2(3)=-dsqrt(sprim/4d0  - ambeam**2)
        qeff2(4)=dsqrt(sprim/4d0)

      END

      SUBROUTINE set_param(prob_ang,prob_mass,rmas,rgam,
     $                    amreg2_mas,amreg2_ang,itype,i_flav)
!     *************************************************
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      SAVE
      PARAMETER (n_ang=9,n_mass=7,n_m_res=2,m_step=2)
      DIMENSION prob_ang(0:m_step,n_ang)
      DIMENSION rmas(n_m_res),rgam(n_m_res),prob_mass(n_mass)
      DIMENSION i_flav(4)

! common / WorZ / is replaced by  store_label  routine !!!!!!
! convert linear labels to KoralW.13x convention (mode=1)
! these routines comes from the decay.f package !!! 
      CALL store_label(1,label)
      CALL linear_to_WZ_label(1,label,icwm,icwp,ifznow,ifwnow)

        IF(ifwnow.EQ.1) THEN
          CALL set_param_WW(prob_ang,prob_mass,rmas,rgam,
     $                      amreg2_mas,amreg2_ang,itype,i_flav)
        ELSE
          CALL set_param_ZZ(prob_ang,prob_mass,rmas,rgam,
     $                      amreg2_mas,amreg2_ang,itype,i_flav)
        ENDIF

      END

      FUNCTION flip_flop(itype)
!     **************************
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      IF(itype.LT.8) THEN
        itpe_loc = itype
      ELSEIF(itype.GE.8 .AND. itype.LE.10) THEN
        itpe_loc = itype+3
      ELSEIF(itype.GE.11 .AND. itype.LE.13) THEN
        itpe_loc = itype-3
      ELSEIF(itype.GE.14 .AND. itype.LE.25) THEN
        itpe_loc = 37 -(itype-14)
      ELSEIF(itype.GE.26 .AND. itype.LE.37) THEN
        itpe_loc = 37 -(itype-14)
      ELSEIF(itype.GE.38 .AND. itype.LE.49) THEN
        itpe_loc = 61 -(itype-38)
      ELSEIF(itype.GE.50 .AND. itype.LE.61) THEN
        itpe_loc = 61 -(itype-38)
      ELSE
        write(6,*)'flip_flop=> wrong itype: ',itype
        stop
      ENDIF
      flip_flop=itpe_loc
      END

      SUBROUTINE kinga_2serl(mode,iflip,sprim,ambeam,amdet,i_flav,
     @            itype,dp1,dp2,dp3,dp4,fakp,wjac)
!     ***************************************************
!-- 1 x 3 channel  [4x(3x(1,2))]
! generation of angles (hidden) 
! construction of kinematics
! 4moms are inputs in mode 1
! iflip 1 - normal, -1 - mirror configuration (qeff1 <-> qeff2)
! prob_mass(2)=prob_inv
! prob_mass(1)=prob_flat
! prob_mass(2+i)=prob_bw(i)
! prob_ang(mkrok,5): 1 - flat
!                    2 - 1/t
!                    3 - 1/u
!                    4 - ln t/t
!                    5 - ln u/u
!     ***************************************************
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      DIMENSION amdet(4),qeff1(4),qeff2(4),pt1(4),pt2(4)
      DIMENSION bq12(4),bq123(4),bp1(4),bp2(4),bp3(4),bp4(4)
      DIMENSION dq12(4),dq123(4),dp1(4),dp2(4),dp3(4),dp4(4)
      DIMENSION i_flav(4)

      PARAMETER (n_ang=9,n_mass=7,n_m_res=2,m_step=2)
      DIMENSION prob_ang(0:m_step,n_ang),crd_ang(0:m_step,n_ang)
      DIMENSION mtype(0:m_step),crd_mass(m_step,n_mass)
      DIMENSION rmas(n_m_res),rgam(n_m_res),prob_mass(n_mass)
      SAVE

      kindmp = 0
!-- set effective beams (CMS)
      IF(iflip.EQ.1) THEN
        CALL set_eff_beams(sprim,ambeam,qeff1,qeff2)
      ELSEIF(iflip.EQ.-1) THEN
        CALL set_eff_beams(sprim,ambeam,qeff2,qeff1)
      ELSE
        WRITE(6,*) 'kinga_ ==> wrong iflip ',iflip
      ENDIF

      CALL set_param(prob_ang,prob_mass,rmas,rgam,
     $               amreg2_mas,amreg2_ang,itype,i_flav)

      smi=(amdet(1)+amdet(2))**2
      IF(sprim.LT.(amdet(1)+amdet(2)+amdet(3)+amdet(4))**2) THEN
        fakp = 0d0
        wjac = 0d0
        RETURN
      ENDIF

      IF(mode.EQ.1) THEN
        DO i=1,4
          bq12(i)=dp1(i)+dp2(i)
          bq123(i)=bq12(i)+dp3(i)
          bp1(i)=dp1(i)
          bp2(i)=dp2(i)
          bp3(i)=dp3(i)
          bp4(i)=dp4(i)
        ENDDO
        s12  = dmas2(bq12)
        s123 = dmas2(bq123)
      ENDIF

      CALL mass_2serl_gen(mode,sprim,
     @     smi,prob_mass,rmas,rgam,amreg2_mas,
     @     s12,s123,sscru,mtype,crd_mass)
!------- check if outside phase-space
      IF(s12.LT.smi .OR. s123.LT.(dsqrt(s12)+amdet(3))**2
     @    .OR. sprim.LT.(dsqrt(s123)+amdet(4))**2 
     @    .OR. sscru.EQ.0d0) THEN 
        fakp = 0d0
        wjac = 0d0
        RETURN
      ENDIF

!------- generate primary angle and construct 4-vects
      do i=1,4
        pt1(i) = qeff1(i)
        pt2(i) = qeff2(i)
      enddo
      mkrok=0
      call kin_step(mode,amreg2_ang,prob_ang,sprim,s123,amdet(4)**2,
     $  mtype,mkrok,pt1,pt2, bq123,bp4, xccos,crd_ang)
      IF(xccos.EQ.0d0) THEN 
        fakp=0d0
        wjac=0d0
        RETURN
      ENDIF

      IF(KINDMP.EQ.1)THEN
        write(6,*) 'bq123, bp4'
        CALL
     $    DUMPL(6,bP1,bP2,bP3,bP4,QEFF1,QEFF2,SPHOT,nphot) 
      ENDIF
!------- generate secondary angles and construct 4-vects
      do i=1,4
        pt1(i) = -qeff1(i) +bq123(i)
      enddo
      mkrok=1
      call kin_step(mode,amreg2_ang,prob_ang, s123,s12,amdet(3)**2,
     $       mtype,mkrok,qeff1,pt1, bq12,bp3, xccos1,crd_ang)
      IF(xccos1.EQ.0d0) THEN 
        fakp=0d0
        wjac=0d0
        RETURN
      ENDIF

      IF(KINDMP.EQ.1)THEN
        write(6,*)'masses bq123,bq12,s123,s12 '
        write(6,*) dsqrt(dmas2(bq123)),dsqrt(dmas2(bq12))
        write(6,*) dsqrt(s123),dsqrt(s12)
        write(6,*) 'bq123, bq12, bp3, bp4'
        CALL
     $      DUMPL(6,bP1,bP2,bP3,bP4,QEFF1,QEFF2,SPHOT,nphot)
      ENDIF
      do i=1,4
        pt1(i) = -qeff1(i) +bq12(i)
      enddo
      mkrok=2
      call kin_step(mode,amreg2_ang,prob_ang,
     $     s12,amdet(1)**2,amdet(2)**2, 
     $     mtype,mkrok,qeff1,pt1, bp1,bp2, xccos2,crd_ang)
      IF(xccos2.EQ.0d0) THEN 
        fakp=0d0
        wjac=0d0
        RETURN
      ENDIF

      IF(KINDMP.EQ.1)THEN
        write(6,*) 'in CMSeff, all'
        CALL
     $      DUMPL(6,bP1,bP2,bP3,bP4,QEFF1,QEFF2,SPHOT,nphot) 
      ENDIF
      IF(KINDMP.EQ.1)THEN
!----- consistency checks 
!-- require smineww.unused.f file --!
!        IF(mode.EQ.0) 
!     $  call kinebr(sprim,sprim,costhe,phi,cosde1,phi1,cosde2,phi2,
!     $  ambeam,dsqrt(s12),dsqrt(s123),amdet,bq12,bq123,bp1,bp2,bp3,bp4)
!        write(6,*) 'with kinebr'
!       CALL DUMPL(6,bP1,bP2,bP3,bP4,QEFF1,QEFF2,SPHOT,nphot)
!----- end consistency checks
      ENDIF

cccc m.s.25.04     CALL norm_2ser(crd_mass,prob_mass,
cccc m.s.25.04     $     crd_ang,prob_ang,fakp)

      fakp = xccos1*xccos2*xccos*sscru

! lambda factors

      bmain = wlambda(sprim,s123,amdet(4)**2)
      bwm   = wlambda(s123,s12,amdet(3)**2)
      bwp   = wlambda(s12,amdet(1)**2,amdet(2)**2)

      wjac=bmain*bwp*bwm

      IF(mode.EQ.0) THEN
        DO i=1,4
          dq12(i)=bq12(i)
          dq123(i)=bq123(i)
          dp1(i)=bp1(i)
          dp2(i)=bp2(i)
          dp3(i)=bp3(i)
          dp4(i)=bp4(i)
        ENDDO
      ENDIF

      END

      SUBROUTINE norm_2parl(crd_mass_p,crd_mass_g,prob_mass,
     $     crd_ang,prob_ang,   fakp)
!     ***************************************************
!!!!!!!!!!!!!! THIS ROUTINE IS WRONG !!!!!!!!!!!!
! IT IS NOT CORRECTED FOR THE NEW MEANING OF prob_ang !
!!!!!!!!!!!!!! THIS ROUTINE IS WRONG !!!!!!!!!!!!

      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      PARAMETER (n_ang=9,n_mass=7,n_m_res=2,m_step=2)
      DIMENSION prob_ang(0:m_step,n_ang),crd_ang(0:m_step,n_ang)
      DIMENSION crd_mass_g(m_step,n_mass),crd_mass_p(m_step,n_mass)
      DIMENSION prob_mass(n_mass)

      wta0=0d0
      DO k=1,n_ang
        wta0= wta0 +prob_ang(0,k)*crd_ang(0,k)
      ENDDO

      wt_=1d0
      wt_norm=1d0
      DO i=1,m_step
        wt_i=0d0
        wt_i_norm=0d0
        DO j=1,n_mass
          wt_ji=0d0
          DO k=1,n_ang
            wt_ji= wt_ji +prob_ang(j,k)*crd_ang(i,k)
          ENDDO
          wt_i= wt_i +wt_ji*prob_mass(j)*crd_mass_g(i,j)
          wt_i_norm= wt_i_norm 
     $              +prob_mass(j)*crd_mass_g(i,j)/crd_mass_p(i,j)
        ENDDO
        wt_= wt_ *wt_i 
        wt_norm = wt_norm *wt_i_norm
      ENDDO

      wt_norm = 1d0 - wt_norm

      fakp = wt_norm/(wt_*wta0)

      END

      SUBROUTINE norm_2ser(crd_mass,prob_mass,
     $     crd_ang,prob_ang,   fakp)
!     ***************************************************
!!!!!!!!!!!!!! THIS ROUTINE IS WRONG !!!!!!!!!!!!
! IT IS NOT CORRECTED FOR THE NEW MEANING OF prob_ang !
!!!!!!!!!!!!!! THIS ROUTINE IS WRONG !!!!!!!!!!!!

      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      PARAMETER (n_ang=9,n_mass=7,n_m_res=2,m_step=2)
      DIMENSION prob_ang(0:m_step,n_ang),crd_ang(0:m_step,n_ang)
      DIMENSION crd_mass(m_step,n_mass),prob_mass(n_mass)

      wta0=0d0
      DO k=1,n_ang
        wta0= wta0 +prob_ang(0,k)*crd_ang(0,k)
      ENDDO

      wt_=1d0
      wt_norm=1d0
      DO i=1,m_step
        wt_i=0d0
        DO j=1,n_mass
          wt_ji=0d0
          DO k=1,n_ang
            wt_ji= wt_ji +prob_ang(j,k)*crd_ang(i,k)
          ENDDO
          wt_i= wt_i +wt_ji*prob_mass(j)*crd_mass(i,j)
        ENDDO
        wt_norm=wt_norm/i
        wt_= wt_ *wt_i 
      ENDDO

      fakp = wt_norm/(wt_*wta0)

      END

      SUBROUTINE kinga_2serl_2(mode,iflip,sprim,ambeam,amdet,i_flav,
     @            itype,dp1,dp2,dp3,dp4,fakp,wjac)
!     ***************************************************
!-- 1 x 3 channel  [4x((3x2)x1)]
! generation of angles, construction of kinematics
! (angles, s23, s123) TOGETHER with 4moms are inputs in mode 1
! in mode 0 (angles, s23, s123) are redundant!
! prob_mass(2)=prob_inv
! prob_mass(1)=prob_flat
! prob_mass(2+i)=prob_bw(i)
!  prob_ang(mkrok,5): 1 - flat
!                     2 - 1/t
!                     3 - 1/u
!                     4 - ln t/t
!                     5 - ln u/u
!     ***************************************************
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      DIMENSION amdet(4),qeff1(4),qeff2(4),pt1(4),pt2(4)
      DIMENSION bq23(4),bq123(4),bp1(4),bp2(4),bp3(4),bp4(4)
      DIMENSION dq23(4),dq123(4),dp1(4),dp2(4),dp3(4),dp4(4)
      DIMENSION i_flav(4)

      PARAMETER (n_ang=9,n_mass=7,n_m_res=2,m_step=2)
      DIMENSION prob_ang(0:m_step,n_ang),crd_ang(0:m_step,n_ang)
      DIMENSION mtype(0:m_step),crd_mass(m_step,n_mass)
      DIMENSION rmas(n_m_res),rgam(n_m_res),prob_mass(n_mass)
      SAVE

      kindmp = 0

!-- set effective beams (CMS)
      IF(iflip.EQ.1) THEN
        CALL set_eff_beams(sprim,ambeam,qeff1,qeff2)
      ELSEIF(iflip.EQ.-1) THEN
        CALL set_eff_beams(sprim,ambeam,qeff2,qeff1)
      ELSE
        WRITE(6,*) 'kinga_ ==> wrong iflip ',iflip
      ENDIF

      CALL set_param(prob_ang,prob_mass,rmas,rgam,
     $               amreg2_mas,amreg2_ang,itype,i_flav)

      smi=(amdet(2)+amdet(3))**2
      IF(sprim.LT.(amdet(1)+amdet(2)+amdet(3)+amdet(4))**2) THEN
        fakp = 0d0
        wjac = 0d0
        RETURN
      ENDIF

      IF(mode.EQ.1) THEN
        DO i=1,4
          bq23(i)=dp2(i)+dp3(i)
          bq123(i)=bq23(i)+dp1(i)
          bp1(i)=dp1(i)
          bp2(i)=dp2(i)
          bp3(i)=dp3(i)
          bp4(i)=dp4(i)
        ENDDO
        s23  = dmas2(bq23)
        s123 = dmas2(bq123)
      ENDIF

      CALL mass_2serl_gen(mode,sprim,
     @     smi,prob_mass,rmas,rgam,amreg2_mas,
     @     s23,s123,sscru,mtype,crd_mass)
!------- check if outside phase-space
      IF(s23.LT.smi .OR. s123.LT.(dsqrt(s23)+amdet(1))**2
     @    .OR. sprim.LT.(dsqrt(s123)+amdet(4))**2 
     @    .OR. sscru.EQ.0d0) THEN 
        fakp = 0d0
        wjac = 0d0
        RETURN
      ENDIF

!------- generate primary angle and construct 4-vects
      do i=1,4
        pt1(i) = qeff1(i)
        pt2(i) = qeff2(i)
      enddo
      mkrok=0
      call kin_step(mode,amreg2_ang,prob_ang, sprim,s123,amdet(4)**2,
     $  mtype,mkrok,pt1,pt2, bq123,bp4, xccos,crd_ang)
      IF(xccos.EQ.0d0) THEN 
        fakp=0d0
        wjac=0d0
        RETURN
      ENDIF

      IF(KINDMP.EQ.1)THEN
        write(6,*) 'bq123, bp4'
        CALL
     $    DUMPL(6,bP1,bP2,bP3,bP4,QEFF1,QEFF2,SPHOT,nphot) 
      ENDIF
!------- generate secondary angles and construct 4-vects
      do i=1,4
        pt1(i) =-qeff1(i) +bq123(i)
      enddo
      mkrok=1
      call kin_step(mode,amreg2_ang,prob_ang, s123,amdet(1)**2,s23,
     $  mtype,mkrok,qeff1,pt1, bp1,bq23, xccos1,crd_ang)
      IF(xccos1.EQ.0d0) THEN 
        fakp=0d0
        wjac=0d0
        RETURN
      ENDIF

      IF(KINDMP.EQ.1)THEN
        write(6,*)'masses bq123,bq23,s123,s23 '
        write(6,*) dsqrt(dmas2(bq123)),dsqrt(dmas2(bq23))
        write(6,*) dsqrt(s123),dsqrt(s23)
        write(6,*) 'fakp,wjac,xccos,xccos1,xccos2'
        write(6,*) fakp,wjac,xccos,xccos1,xccos2
        write(6,*) 'bq123, bq23, bp1, bp4'
        CALL
     $      DUMPL(6,bP1,bP2,bP3,bP4,QEFF1,QEFF2,SPHOT,nphot)
      ENDIF
      do i=1,4
        pt1(i) = qeff1(i) -bp1(i)
        pt2(i) = -bp4(i) +qeff2(i)
      enddo
      mkrok=2
      call kin_step(mode,amreg2_ang,prob_ang,
     $     s23,amdet(2)**2,amdet(3)**2, 
     $     mtype,mkrok,pt1,pt2,bp2,bp3, xccos2,crd_ang)
      IF(xccos2.EQ.0d0) THEN 
        fakp=0d0
        wjac=0d0
        RETURN
      ENDIF

      IF(KINDMP.EQ.1)THEN
        write(6,*) 'in CMSeff, all'
        CALL
     $      DUMPL(6,bP1,bP2,bP3,bP4,QEFF1,QEFF2,SPHOT,nphot) 
      ENDIF

ccc m.s. 25.04      CALL norm_2ser(crd_mass,prob_mass,
ccc m.s. 25.04     $     crd_ang,prob_ang,fakp)

      fakp = xccos1*xccos2*xccos*sscru

! lambda factors

      bmain = wlambda(sprim,s123,amdet(4)**2)
      bwm   = wlambda(s123,s23,amdet(1)**2)
      bwp   = wlambda(s23,amdet(2)**2,amdet(3)**2)

      wjac=bmain*bwp*bwm

      IF(mode.EQ.0) THEN
        DO i=1,4
          dq23(i)=bq23(i)
          dq123(i)=bq123(i)
          dp1(i)=bp1(i)
          dp2(i)=bp2(i)
          dp3(i)=bp3(i)
          dp4(i)=bp4(i)
        ENDDO
      ENDIF

      END


      SUBROUTINE kinga_2parl(mode,iflip,sprim,ambeam,amdet,i_flav,
     @            itype,dp1,dp2,dp3,dp4,fakp,wjac)
!     ***************************************************
!-- 2 x 2 channel [(1,2)x(3,4)]
! generation of angles, construction of kinematics
! (angles, s1, s2) TOGETHER with 4moms are inputs in mode 1
! in mode 0 (angles, s1, s2) are redundant!
! prob_mass(2)=prob_inv
! prob_mass(1)=prob_flat
! prob_mass(2+i)=prob_bw(i)
! prob_ang(mkrok,5): 1 - flat
!                    2 - 1/t
!                    3 - 1/u
!                    4 - ln t/t
!                    5 - ln u/u
!     ***************************************************
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      DIMENSION amdet(4),qeff1(4),qeff2(4),pt1(4),pt2(4)
      DIMENSION bq1(4),bq2(4),bp1(4),bp2(4),bp3(4),bp4(4)
      DIMENSION dq1(4),dq2(4),dp1(4),dp2(4),dp3(4),dp4(4)
      DIMENSION i_flav(4)

      PARAMETER (n_ang=9,n_mass=7,n_m_res=2,m_step=2)
      DIMENSION prob_ang(0:m_step,n_ang),crd_ang(0:m_step,n_ang)
      DIMENSION mtype(0:m_step)
      DIMENSION crd_mass_p(m_step,n_mass),crd_mass_g(m_step,n_mass)
      DIMENSION rmas(n_m_res),rgam(n_m_res),prob_mass(n_mass)
      SAVE

      kindmp = 0

!-- set effective beams (CMS)
      IF(iflip.EQ.1) THEN
        CALL set_eff_beams(sprim,ambeam,qeff1,qeff2)
      ELSEIF(iflip.EQ.-1) THEN
        CALL set_eff_beams(sprim,ambeam,qeff2,qeff1)
      ELSE
        WRITE(6,*) 'kinga_ ==> wrong iflip ',iflip
      ENDIF

      CALL set_param(prob_ang,prob_mass,rmas,rgam,
     $               amreg2_mas,amreg2_ang,itype,i_flav)

      smi1=(amdet(1)+amdet(2))**2
      smi2=(amdet(3)+amdet(4))**2
      IF(sprim.LT.(amdet(1)+amdet(2)+amdet(3)+amdet(4))**2) THEN
        fakp = 0d0
        wjac = 0d0
        RETURN
      ENDIF

      IF(mode.EQ.1) THEN
        DO i=1,4
          bq1(i)=dp1(i)+dp2(i)
          bq2(i)=dp3(i)+dp4(i)
          bp1(i)=dp1(i)
          bp2(i)=dp2(i)
          bp3(i)=dp3(i)
          bp4(i)=dp4(i)
        ENDDO
        s1  = dmas2(bq1)
        s2  = dmas2(bq2)
      ENDIF

      CALL mass_2parl_gen(mode,sprim,
     @  smi1,prob_mass,rmas,rgam,amreg2_mas,
     @  smi2,prob_mass,rmas,rgam,amreg2_mas,
     @  s1,s2,sscru,mtype,crd_mass_p,crd_mass_g)

      IF(KINDMP.EQ.1)THEN
        write(6,*)'s1,s2 ',s1,s2
      ENDIF
!------- check if outside phase-space
      IF(s1.LT.smi1 .OR. s2.LT.smi2
     @   .OR. sprim.LT.(dsqrt(s1)+dsqrt(s2))**2 
     @   .OR. sscru .EQ. 0d0) THEN
        fakp=0d0
        wjac=0d0
        RETURN
      ENDIF
!------- generate primary angle and construct 4-vects
      do i=1,4
        pt1(i) = qeff1(i)
        pt2(i) = qeff2(i)
      enddo
      mkrok=0
      call kin_step(mode,amreg2_ang,prob_ang, sprim,s1,s2,
     $    mtype,mkrok,pt1,pt2, bq1,bq2, xccos,crd_ang)
      IF(KINDMP.EQ.1)THEN
        write(6,*)'bq1, bq2 ',costhe,phi
cc        CALL DUMPL(6,bP1,bP2,bP3,bP4,QEFF1,QEFF2,SPHOT,nphot)
      ENDIF
      do i=1,4
        pt1(i) = -qeff1(i) +bq1(i)
        pt2(i) = -pt1(i)
      enddo
!------- generate secondary angles and construct 4-vects
      mkrok=1
      call kin_step(mode,amreg2_ang,prob_ang,s1,amdet(1)**2,amdet(2)**2,
     $     mtype,mkrok,qeff1,pt1, bp1,bp2, xccos1,crd_ang)
      IF(KINDMP.EQ.1)THEN
cc        write(6,*)'bp1, bp2 ',cosde1,phi1
        write(6,*)'bp1, bp2 ',costhe,phi
cc        CALL DUMPL(6,bP1,bP2,bP3,bP4,QEFF1,QEFF2,SPHOT,nphot)
      ENDIF
      mkrok=2
      call kin_step(mode,amreg2_ang,prob_ang,s2,amdet(4)**2,amdet(3)**2,  
     $    mtype,mkrok,qeff2,pt2,bp4,bp3, xccos2,crd_ang)
      IF(KINDMP.EQ.1)THEN
        write(6,*)'CMS-eff (bp3,bp4)',costhe,phi
cc        write(6,*)'CMS-eff (bp3,bp4)',cosde2,phi2
cc        CALL DUMPL(6,bP1,bP2,bP3,bP4,QEFF1,QEFF2,SPHOT,nphot)
      ENDIF

      IF(xccos1.EQ.0 .OR. xccos2.EQ.0 .OR. xccos.EQ.0 
     $    .OR. sscru.EQ.0) THEN 
        fakp=0d0
        wjac=0d0
        RETURN
      ENDIF

ccc m.s. 25.04      CALL norm_2parl(crd_mass_p,crd_mass_g,prob_mass,
ccc m.s. 25.04     $      crd_ang,prob_ang,fakp)

      fakp = xccos1*xccos2*xccos*sscru

! lambda factors

      bmain=wlambda(sprim,s1,s2)
      bwm=  wlambda(s1,amdet(1)**2,amdet(2)**2)
      bwp=  wlambda(s2,amdet(3)**2,amdet(4)**2)


      wjac=bmain*bwp*bwm
c      write(6,*) 'PAR fakp,wjac=',fakp,wjac
c      write(6,*) 'PAR crud c1,c2,c,ss',xccos1,xccos2,xccos,sscru

      IF(mode.EQ.0) THEN
        DO i=1,4
          dq1(i)=bq1(i)
          dq2(i)=bq2(i)
          dp1(i)=bp1(i)
          dp2(i)=bp2(i)
          dp3(i)=bp3(i)
          dp4(i)=bp4(i)
        ENDDO
      ENDIF

      END

      SUBROUTINE mass_1_gen
     @  (mode,smi,sma,prob_mass,
     @   rmas,rgam,amr2,sout,imtype,crd_mass0,wt)
!     *******************************************************
! Basic generation of 1-dim ds distribution from smi to sma
! Shape: prob_in/(s+amr2) +\sum^nres prob_bw_i/BW_i + prob_fl
! BW = (sout-rmas(i)^2)^2 + (rmas(i)*rgam(i))^2
! prob_fl    = prob_mass(1)
! prob_in    = prob_mass(2)
! prob_in*ln = prob_mass(3)
! prob_in-s  = prob_mass(4)
! prob_bw(i) = prob_mass(i+4)
! prob_fl +\sum prob_bw(i) + prob_in = 1 assumed !
! crd_mass0(i) = f(i) / \int f(i),  f - i-th branch distrib.
!---------------------
! for mode=0
! OUTPUT: sout  : s variable
!         wt    : wt=1/ (\sum prob_i f_i/F_i); F_i=\int f_i
!---------------------
! for mode=1
! INPUT:  sout  - no generation,  just calculation of weight wt. 
!---------------------
!
! (Re)Written by: M. Skrzypek            date: 6/12/96
!
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      COMMON / matpar / pi,ceuler
      SAVE / matpar / 
      DIMENSION drvec(100)

      PARAMETER (n_ang=9,n_mass=7,n_m_res=2,m_step=2)
!      DIMENSION prob_ang(0:m_step,n_ang),crd_ang(0:m_step,n_ang)
      DIMENSION crd_mass0(n_mass)
      DIMENSION rmas(n_m_res),rgam(n_m_res),prob_mass(n_mass)
      DIMENSION alp_ma(n_m_res),alp_mi(n_m_res)
      DIMENSION pr(0:n_mass)
      SAVE

      IF(smi.LT.1d-12 .AND. amr2.LT.1d-13) THEN
        WRITE(6,*)'mass_1_gen==> Expected troubles at s=0: ',smi,amr2
        STOP
      ENDIF

      pr(0)= 0d0
      DO i=1,n_mass
        pr(i)=pr(i-1) +prob_mass(i)
      ENDDO

      DO i=1,n_m_res
        rmrg = rmas(i)*rgam(i)
        alp_ma(i)=atan( (sma-rmas(i)**2)/rmrg )
        alp_mi(i)=atan( (smi-rmas(i)**2)/rmrg )
      ENDDO

      biglog =dlog((sma+amr2)/(smi+amr2))
      biglog2=dlog((sma-smi+amr2)/amr2)
! set strenght of log. singul.
      n_log=1
! setting strenght of power sing.
      n_inv=2
! end setting strenght
      nlo=n_log+1
      ninv=n_inv-1

      sma1=-1/( ninv*(sma+amr2)**ninv )
      smi1=-1/( ninv*(smi+amr2)**ninv )

!     ====================
      IF (mode.EQ.0) THEN
!     ====================
!
 10   CALL varran(drvec,2)
      r1=drvec(1)
      r2=drvec(2)
      IF(r1.LE.pr(1)) THEN      
!-- flat s
        sout=(sma-smi)*r2+smi 
        imtype=1
      ELSEIF(r1.LE.pr(2)) then 
!-- 1/(s+amr2)
        sout=(smi+amr2)*exp(r2*biglog) -amr2
        imtype=2
      ELSEIF(r1.LE.pr(3)) then 
!-- ln(s+amr2/sma)**n_log/(s+amr2)
        ymax= -biglog**nlo/nlo
        ymin=  0d0
        y1=r2*(ymax-ymin)+ymin
        sout=(sma+amr2)*exp( -(-nlo*y1)**(1d0/dble(nlo)) ) -amr2
        imtype=3
      ELSEIF(r1.LE.pr(4)) then 
!-- 1/(sma-s+amr2)
        sout=(    amr2)*exp(r2*biglog2) -amr2
        sout=sma-sout
        if(sout.gt.sma) sout=sma
        imtype=4
      ELSEIF(r1.LE.pr(5)) then 
!-- 1/(s+amr2)**n_inv
        y1=r2*(sma1-smi1) +smi1
        sout=(-1/(ninv*y1))**(1d0/dble(ninv)) -amr2
        imtype=5
      ELSE
!-- resonance 
        DO i=1,n_m_res
          IF(r1.LE.pr(n_mass-n_m_res+i)) then
            alp=alp_mi(i)+r2*(alp_ma(i)-alp_mi(i))
            sout=rmas(i)**2+rmas(i)*rgam(i)*tan(alp)
            imtype=i+n_mass-n_m_res
            GOTO 11
          ENDIF
        ENDDO
        WRITE(6,*) 'mass_1_gen==> Total probability below 1: ',sum
        STOP
      ENDIF
 11   CONTINUE

!     =====
      ENDIF
!     =====

      IF(sout.LT.0d0 ) THEN
        WRITE(6,*)'mass_1_gen==> Negative sout: ',sout
        STOP
      ENDIF

! Normalisation
!-- flat 
      crd_mass0(1) = 1/(sma -smi)
!-- 1/(s+amr2)
      crd_mass0(2) = 1/( (sout+amr2)*biglog )
!-- -log(s+amr2/sma+amr2)^n/(s+amr2)
      crd_mass0(3) = (-dlog((sout+amr2)/(sma+amr2)))**n_log/(sout+amr2) 
     @                 /(biglog**nlo/nlo) 
!-- 1/(sma-s+amr2)
      crd_mass0(4) = 1/( (sma-sout+amr2)*biglog2 )
!-- 1/(s+amr2)**n_inv
      crd_mass0(5) = 1/(sout+amr2)**n_inv /(sma1-smi1) 
!-- resonances
      DO i=1,n_m_res
        rmrg = rmas(i)*rgam(i)
        crd_mass0(n_mass-n_m_res+i) =1/( ((sout-rmas(i)**2)**2+rmrg**2)
     @              *(alp_ma(i)-alp_mi(i))/rmrg )
      ENDDO

      ph_total=0d0
      DO i=1,n_mass
        ph_total = ph_total +prob_mass(i)*crd_mass0(i)
      ENDDO

      wt=1/ph_total

      END

      SUBROUTINE mass_2parl_gen(mode,smax,
     @  smi1,prob_mass1,rmas1,rgam1,amr21,
     @  smi2,prob_mass2,rmas2,rgam2,amr22,
     @  sout1,sout2,wt,mtype,crd_mass_p,crd_mass_g)
!     ***************************************************************
! Generation of ds_1ds_2 distribution of 2 parallel branches 
! within phase space boundaries using weighted (pre-sampled) events
! branches can have different coefficients and limits
!---------------------
! for mode=0
! OUTPUT: sout1, sout2  
!         wt    : wt=1/ (\sum prob_i f_i/F_i); F_i=\int f_i
!---------------------
! for mode=1
! INPUT:  sout1, sout2  - no generation,  just calculation of weight. 
!---------------------
!
! (Re)Written by: M. Skrzypek            date: 6/12/96
!
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      COMMON / matpar / pi,ceuler
      SAVE / matpar / 

      PARAMETER (n_ang=9,n_mass=7,n_m_res=2,m_step=2)
!      DIMENSION prob_ang(0:m_step,n_ang),crd_ang(0:m_step,n_ang)
      DIMENSION mtype(0:m_step)
      DIMENSION crd_mass_p(m_step,n_mass),crd_mass_g(m_step,n_mass)
      DIMENSION rmas1(n_m_res),rgam1(n_m_res),prob_mass1(n_mass)
      DIMENSION rmas2(n_m_res),rgam2(n_m_res),prob_mass2(n_mass)
      DIMENSION crd_grd1(n_mass),crd_grd2(n_mass)
      DIMENSION crd_ptt1(n_mass),crd_ptt2(n_mass)

      SAVE


      sxsec1 = smax/4d0
      IF(sxsec1.LT.smi1) sxsec1 = smi1
      sxsec2 = smax/4d0
      IF(sxsec2.LT.smi2) sxsec2 = smi2

! Generation, Normalization

 12   CONTINUE

      CALL mass_1_gen
     @  (mode,smi1,smax,prob_mass1,
     @   rmas1,rgam1,amr21,sout1,imtype1,crd_grd1,wt1_grd)

      CALL mass_1_gen
     @  (mode,smi2,smax,prob_mass2,
     @   rmas2,rgam2,amr22,sout2,imtype2,crd_grd2,wt2_grd)

!-- rejection
      IF(mode.EQ.0 .AND. sout1.GT.sxsec1 .AND. sout2.GT.sxsec2) GOTO 12

      IF( dsqrt(sout1)+dsqrt(sout2) .GT. dsqrt(smax) ) THEN 
!-- check if event outside of phase-space
        wt = 0d0
        RETURN
      ENDIF


! Total normalisation

      CALL mass_1_gen
     @  (1,sxsec1,smax,prob_mass1,
     @   rmas1,rgam1,amr21,sout1,imtype,crd_ptt1,wt1_ptt)

      CALL mass_1_gen
     @  (1,sxsec2,smax,prob_mass2,
     @   rmas2,rgam2,amr22,sout2,imtype,crd_ptt2,wt2_ptt)

      DO i=1,n_mass
        crd_mass_g(1,i)=crd_grd1(i)
        crd_mass_g(2,i)=crd_grd2(i)
        crd_mass_p(1,i)=crd_ptt1(i)
        crd_mass_p(2,i)=crd_ptt2(i)
      ENDDO
      mtype(0)=0
      mtype(1)=imtype1
      mtype(2)=imtype2

      crd1 = 0d0
      crd1_norm = 0d0
      DO i=1,n_mass
        crd1 = crd1 +prob_mass1(i)*crd_grd1(i)
        crd1_norm = crd1_norm +prob_mass1(i)*crd_grd1(i)/crd_ptt1(i)
      ENDDO

      crd2 = 0d0
      crd2_norm = 0d0
      DO i=1,n_mass
        crd2 = crd2 +prob_mass2(i)*crd_grd2(i)
        crd2_norm = crd2_norm +prob_mass2(i)*crd_grd2(i)/crd_ptt2(i)
      ENDDO

      crd_norm=1d0 -crd2_norm*crd1_norm

      wt = crd_norm/(crd1*crd2)

      END


      SUBROUTINE mass_2serl_gen_1(mode,smax,
     @  smi,prob_mass,rmas,rgam,amr2,
     @  sout12,sout123,wt,mtype,crd_mass)
!     ***************************************************************
! ORIGINAL ROUTINE OF v 1.40
! Generation of ds_1ds_2 distribution of 2 serial (nested) branches 
! within phase space boundaries using weighted (pre-sampled) events
! s1 ist the outer variable, s2 the inner one.
! Branches have identical coefficients and limits.
! Limits: smax > s123 > s12 > smi.
!---------------------
! for mode=0
! OUTPUT: sout12, sout123  
!         wt    : wt=1/ (\sum prob_i f_i/F_i); F_i=\int f_i
!---------------------
! for mode=1
! INPUT:  sout12, sout123  - no generation,  just calculation of weight. 
!---------------------
!
! (Re)Written by: M. Skrzypek            date: 6/12/96
!
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      COMMON / matpar / pi,ceuler
      SAVE / matpar / 

      PARAMETER (n_ang=9,n_mass=7,n_m_res=2,m_step=2)
      DIMENSION mtype(0:m_step),crd_mass(m_step,n_mass)
      DIMENSION rmas(n_m_res),rgam(n_m_res),prob_mass(n_mass)
      DIMENSION crd_mass1(n_mass),crd_mass2(n_mass)
      SAVE

! Generation

      CALL mass_1_gen
     @  (mode,smi,smax,prob_mass,
     @   rmas,rgam,amr2,sout123,imtype123,crd_mass1,wt1_grd)

      CALL mass_1_gen
     @  (mode,smi,smax,prob_mass,
     @   rmas,rgam,amr2,sout12,imtype12,crd_mass2,wt2_grd)


      IF(mode.EQ.0 .AND. sout12.GT.sout123) THEN
        sou=sout12
        sout12=sout123
        sout123=sou
        mtype(0)=0
        mtype(1)=imtype12
        mtype(2)=imtype123
        DO i=1,n_mass
          crd_mass(1,i)=crd_mass2(i)
          crd_mass(2,i)=crd_mass1(i)
        ENDDO
      ELSE
        mtype(0)=0
        mtype(1)=imtype123
        mtype(2)=imtype12
        DO i=1,n_mass
          crd_mass(1,i)=crd_mass1(i)
          crd_mass(2,i)=crd_mass2(i)
        ENDDO
      ENDIF


! Normalisation

      wt = wt1_grd*wt2_grd 
      wt=wt/2               ! to account for ordering

      END

      SUBROUTINE mass_2serl_gen_2(mode,smax,
     @  smi,prob_mass,rmas,rgam,amr2,
     @  sout12,sout123,wt,mtype,crd_mass)
!     ***************************************************************
! SECOND VERSION !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
! Generation of ds_1ds_2 distribution of 2 serial (nested) branches 
! within phase space boundaries using weighted (pre-sampled) events
! s1 ist the outer variable, s2 the inner one.
! Branches have identical coefficients and limits.
! Limits: smax > s123 > s12 > smi.
!---------------------
! for mode=0
! OUTPUT: sout12, sout123  
!         wt    : wt=1/ (\sum prob_i f_i/F_i); F_i=\int f_i
!---------------------
! for mode=1
! INPUT:  sout12, sout123  - no generation,  just calculation of weight. 
!---------------------
!
! (Re)Written by: M. Skrzypek            date: 6/12/96
!
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      COMMON / matpar / pi,ceuler
      SAVE / matpar / 

      PARAMETER (n_ang=9,n_mass=7,n_m_res=2,m_step=2)
      DIMENSION mtype(0:m_step),crd_mass(m_step,n_mass)
      DIMENSION rmas(n_m_res),rgam(n_m_res),prob_mass(n_mass)
      DIMENSION crd_mass1(n_mass),crd_mass2(n_mass)
      SAVE

! Generation

      CALL mass_1_gen
     @  (mode,smi,smax,prob_mass,
     @   rmas,rgam,amr2,sout123,imtype123,crd_mass1,wt1_grd)

      CALL mass_1_gen
     @  (mode,smi,sout123,prob_mass,
     @   rmas,rgam,amr2,sout12,imtype12,crd_mass2,wt2_grd)


      IF(mode.EQ.0 ) THEN
        mtype(0)=0
        mtype(1)=imtype123
        mtype(2)=imtype12
        DO i=1,n_mass
          crd_mass(1,i)=crd_mass1(i)
          crd_mass(2,i)=crd_mass2(i)
        ENDDO
      ENDIF


! Normalisation

      wt = wt1_grd*wt2_grd 

      END

      SUBROUTINE mass_2serl_gen(mode,smax,
     @  smi,prob_mass,rmas,rgam,amr2,
     @  sout12,sout123,wt,mtype,crd_mass)
!     ***************************************************************
! SECOND VERSION !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
! Generation of ds_1ds_2 distribution of 2 serial (nested) branches 
! within phase space boundaries using weighted (pre-sampled) events
! s1 ist the outer variable, s2 the inner one.
! Branches have identical coefficients and limits.
! Limits: smax > s123 > s12 > smi.
!---------------------
! for mode=0
! OUTPUT: sout12, sout123  
!         wt    : wt=1/ (\sum prob_i f_i/F_i); F_i=\int f_i
!---------------------
! for mode=1
! INPUT:  sout12, sout123  - no generation,  just calculation of weight. 
!---------------------
!
! (Re)Written by: M. Skrzypek            date: 6/12/96
!
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      COMMON / matpar / pi,ceuler
      SAVE / matpar / 

      PARAMETER (n_ang=9,n_mass=7,n_m_res=2,m_step=2)
      DIMENSION mtype(0:m_step),crd_mass(m_step,n_mass)
      DIMENSION rmas(n_m_res),rgam(n_m_res),prob_mass(n_mass)
      SAVE

c      CALL mass_2serl_gen_2(mode,smax,
c     @  smi,prob_mass,rmas,rgam,amr2,
c     @  sout12,sout123,wt,mtype,crd_mass)

      CALL mass_2serl_gen_1(mode,smax,
     @  smi,prob_mass,rmas,rgam,amr2,
     @  sout12,sout123,wt,mtype,crd_mass)

      END

      SUBROUTINE kin_step(mode,amreg2_ang,prob_ang,s12,s1,s2, 
     $      mtype,mkrok,pinb1,pinb2, pout1,pout2, wt,crd_ang)
*     ************************************************************
! generates and constructs kinematics of a single 'decay'
c INPUT:  
!         mode = 0 generation
!                1 weight calculation based on 4-moms
!         amreg         auxiliary mass for regularization
!         prob_ang(6): 1 - flat
!                      2 - 1/t
!                      3 - 1/u
c         s12    inv mass of 1+2 system
c         s1,s2  squared outgoing masses
c         pinb1,pinb2   reference momenta to span angles on them
c                       in certain reference frame Cref
c         imtype: generated mass channel 
c                 0 - none
c                 1 - 1/s
c                 2 - 1
c                 n+2 - BW(n)
c OUTPUT:
c         pout1(4)  4-momentum of 1 in Cref (INPUT for mode = 1)
c         pout2(4)  4-momentum of 2 in Cref (INPUT for mode = 1)
c         wt        weight
c
c (Re)Written by: M. Skrzypek              date: 1/20/97
c
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      DIMENSION pout1(4),pout2(4)
      DIMENSION pinb1(4),pinb2(4)
      PARAMETER (n_ang=9,n_mass=7,n_m_res=2,m_step=2)
      DIMENSION prob_ang(0:m_step,n_ang),crd_ang(0:m_step,n_ang)
      DIMENSION mtype(0:m_step) !,crd_mass(m_step,n_mass)
!      DIMENSION rmas(n_m_res),rgam(n_m_res),prob_mass(n_mass)
!      real*16 costhe,phi
      SAVE

      CALL cosdec_br(mode,amreg2_ang,mtype,mkrok,prob_ang,
     $  pinb1,pinb2, pout1,pout2,s12,s1,s2,  costhe,phi,wt,crd_ang)
      IF(wt.EQ.0) RETURN
      IF(mode.EQ.0) CALL kin_dec(pinb1,pinb2,costhe,phi,
     $                           s12,s1,s2, pout1,pout2) 

      END

      subroutine kin_dec(pinb1,pinb2,costhe,fi,
     $                           s12,am1sq,am2sq, q1,q2) 
*     ************************************************************
! corresponds to kinett of v.121
c Construction of kinematics for cosdec_br.
c INPUT:  
c         pb1,pb2   reference momenta to span angles on them
c                   in certain reference frame Cref
c         costhe,fi -  production angles 
c         s12 - inv mass of 1+2 system
c         am1sq,am2sq  squared masses of q1 and q2
c OUTPUT:
c         q1(4)        - four-momentum of 1 in Cref
c         q2(4)        - four-momentum of 2 in Cref
c
c (Re)Written by: M. Skrzypek              date: 7/6/96
c
      implicit DOUBLE PRECISION (a-h,o-z)
      save
      dimension q1(4),q2(4)
      dimension b12(4),pb1(4),pb2(4),pinb1(4),pinb2(4)

!      real*16 costhe,fi
!

!!      WRITE(6,*)'kindec: costhe,fi=',costhe,fi

      sinthe =sqrt(max(0d0,((1d0-costhe)*(1d0+costhe))))
!!      if(sinthe.eq.0d0)  write(6,*)'kin_dec==> sinthe=0'
      do i=1,4
        pb1(i)=pinb1(i)
        pb2(i)=pinb2(i)
      enddo

!!      WRITE(6,*)'kindec: pinb1,pinb2=',pb1,pb2

!... q1, q2 in their own rest frame
!... Momentum q1 
      qq=dsqrt( (s12-am1sq-am2sq)**2 -4*am1sq*am2sq )/(2*sqrt(s12))
      q1(1)=qq*sinthe*cos(fi)
      q1(2)=qq*sinthe*sin(fi)
      q1(3)=qq*costhe
      q1(4)=dsqrt(am1sq+q1(1)**2+q1(2)**2+q1(3)**2)
!!... Momentum q2 

!!      WRITE(6,*)'kindec: q1 bef. boost=',q1

      do i=1,4
        b12(i)=pb1(i)+pb2(i)     
      enddo

!... boost pb1 from Cref to b12 rest frame
      call boostv(1,b12,pb1,pb1)
!... rotate q1,q2 to frame parallel to pb1 (in b12 rest fr)
      call rotatv(1,pb1,q1,q1)
!... boost q1, q2 from b12 rest frame to Cref
      call boostv(-1,b12,q1,q1)

!!      WRITE(6,*)'kindec: q1 aft. boost=',q1

      do i=1,3
        q2(i)=b12(i)-q1(i)     
      enddo

!!      WRITE(6,*)'kindec: q2=',q2

!... fine tuning on masses
      q1(4)=dsqrt(am1sq+q1(1)**2+q1(2)**2+q1(3)**2)
      q2(4)=dsqrt(am2sq+q2(1)**2+q2(2)**2+q2(3)**2)
      
      end

      subroutine cosdec_br(mode,amreg2_ang,
     $          mtype,mkrok,prob_ang,pinb1,pinb2,
     $          pout1,pout2,s12,s1,s2,  costhe,phi,wt,crd_ang)
*     *********************************************************
c This is cosdec_br to generate decay angles
! version based fully on invariants !!
c INPUT:  
!    mode     0-generation
!             1-xccos of given cdec
!    prob_ang(mkrok,5): 1 - flat
!                       2 - 1/t
!                       3 - 1/u
!                       4 - ln t/t
!                       5 - ln u/u
!    pout1,pout2   only for mode=1, outgoing momenta,
!                  to construct weight.
c    pinb1,pinb2   reference momenta to span angles on them
c                  in certain reference frame Cref
!    WARNING now both pinb1&2 are incoming !!!
c    s12 - inv masses of 1+2 system
c    s1,s2  squared masses of q1 and q2
c OUTPUT:
c         costhe,phi -  production angles 
c         wt - weight
c
c (Re)Written by: M. Skrzypek              date: 7/6/96
c Last update: 9/19/96 M.S.

      implicit DOUBLE PRECISION (a-h,o-z)
c      implicit real*16 (a-h,o-z)
c      DOUBLE PRECISION amreg2_ang
c      DOUBLE PRECISION prob_ang,pinb1,pinb2
c      DOUBLE PRECISION pout1,pout2,s12,s1,s2 ,wt,crd_ang
c      DOUBLE PRECISION pi,ceuler,drvec
c      DOUBLE PRECISION dmas2
      common / matpar / pi,ceuler     
      dimension drvec(100),pb1(4),pb2(4),b12(4),pinb1(4),pinb2(4)
      dimension pout1(4),pout2(4)
      PARAMETER (n_ang=9,n_mass=7,n_m_res=2,m_step=2)
      DIMENSION prob_ang(0:m_step,n_ang),crd_ang(0:m_step,n_ang)
      DIMENSION mtype(0:m_step) !,crd_mass(m_step,n_mass)
!      DIMENSION rmas(n_m_res),rgam(n_m_res),prob_mass(n_mass)
      DIMENSION pr(0:n_ang)
      SAVE

! we will flaten  (BUT NOT CUT) distribution at 
      amflt2 = amreg2_ang

      do i=1,4
        pb1(i)=pinb1(i)
        pb2(i)=pinb2(i)
        b12(i)=pb1(i) +pb2(i) 
      enddo

      t1=dmas2(pb1)
      t2=dmas2(pb2)
      dt=dot(pb1,pb2)
      s=s12

!=========
      x01 = (s12+s1-s2)/2/dsqrt(s12)
      x02 = (s12+s2-s1)/2/dsqrt(s12)
      ex01=x01**2 -s1
      ex02=x02**2 -s2
      IF(ex01.GT.ex02) THEN
        xx= sqrt(max(0d0,ex01))
      ELSE
        xx= sqrt(max(0d0,ex02))
      ENDIF
      IF(xx.EQ.0d0)THEN
        wt=0d0
        write(6,*)'cosdec_br==> xx=0'
        RETURN
      ENDIF

!((((((((((((( replace this with perhaps more stable
!      pp=sqrt( max( 0d0,(dt**2-t1*t2)/s12 ) )
!      IF(pp.EQ.0d0)THEN
!        wt=0d0
!        write(6,*)'cosdec_br  ==>  pp=0'
!        RETURN
!      ENDIF
!      p0=(t1 +dt)/sqrt(s12)
!)))))))))))))) 3/24/97 m.s.
      px01 = (s12+t1-t2)/2/dsqrt(s12)
      px02 = (s12+t2-t1)/2/dsqrt(s12)
      pex01=px01**2 -t1
      pex02=px02**2 -t2
      IF(pex01.LE.0d0 .AND. pex02.LE.0d0)THEN
        wt=0d0
        write(6,*)'cosdec_br  ==>  pp=0'
        write(6,*)'pp2,pex01,pex02',(dt**2-t1*t2)/s12,pex01,pex02
        RETURN
      ENDIF
      IF(pex01.GT.pex02) THEN
        pp= sqrt(max(0d0,pex01))
      ELSE
        pp= sqrt(max(0d0,pex02))
      ENDIF
      p0=px01
!!!!!!!!!!!!!!!

      bb = -xx*pp

!=========
!... first fermion (in b12 rest fr.)
      x01 = sqrt(xx**2+s1)
      aa1 =  x01*p0   -1/2d0*(t1 +s1)
      IF(aa1.lt.-bb)THEN
!        write(6,*)'cosdec_br=>aa1, -bb=',aa1,-bb
        aa1=-bb
      ENDIF
      aa1=aa1+amflt2/2


      ymi1=log(aa1-bb)
      yma1=log(aa1+bb)
      IF(yma1.EQ.ymi1)THEN
        wt=0d0
        write(6,*)'cosdec_br==> ymi1=yma1'
        RETURN
      ENDIF

!      tmima1=((s1-t1)*(s2-t2) +(t2*s1-t1*s2)*(s1-t1-s2+t2)/s12)/4
!      eyma1 =tmima1/(x01*p0 -1/2d0*(t1 +s1) +xx*pp) +amflt2/2d0
!      yma1=log(eyma1)
!      write(6,*)'c_br     =>eyma1,aa1+bb=',eyma1,aa1+bb

!... second fermion (in b12 rest fr.)
      x02 = sqrt(xx**2+s2)
! is this a bug ??? 1.07.97 ms
! 1.07.97 ms      aa2 =  x02*p0   -1/2d0*(t1 +s2) +amflt2/2d0
      aa2 =  x02*p0   -1/2d0*(t1 +s2) 
! is this a bug ??? 1.07.97 ms
      IF(aa2.lt.-bb)THEN
!        write(6,*)'cosdec_br=>aa2, -bb=',aa2,-bb
        aa2=-bb
      ENDIF
      aa2=aa2+amflt2/2

      ymi2=log(aa2-bb)
      yma2=log(aa2+bb)
      IF(yma2.EQ.ymi2)THEN
        wt=0d0
        write(6,*)'cosdec_br==> ymi2=yma2'
        RETURN
      ENDIF

!      tmima2=((s2-t1)*(s1-t2) +(t2*s2-t1*s1)*(s2-t1-s1+t2)/s12)/4
!      eyma2 =tmima2/(x02*p0 -1/2d0*(t1 +s2) +xx*pp) +amflt2/2d0
!      yma2=log(eyma2)

      pr(0)= 0d0
      DO i=1,n_ang
        pr(i)=pr(i-1) +prob_ang(mkrok,i)
      ENDDO

! set singul. strenght
! logarithmic (log t)**n_log/t
      n_log =1   ! original 
      n_log2=2   ! original 
cc      n_log =1   ! !!!!!!!!!!
cc      n_log2=3   ! !!!!!!!!!! 
! power 1/t**n_inv
      n_inv =2      ! original  
ccc      n_inv =3    !!!!!!!!!!!!!!!!!!!!!!!!!!    
! end setting

      nlo =n_log+1
      nlo2=n_log2+1
      ninv=n_inv-1

      tma1=-1/((aa1+bb)**ninv*bb*ninv)
      tmi1=-1/((aa1-bb)**ninv*bb*ninv)
      tma2=-1/((aa2+bb)**ninv*bb*ninv)
      tmi2=-1/((aa2-bb)**ninv*bb*ninv)

      IF(mode.EQ.0) THEN

        call varran(drvec,3)

        r3=drvec(3)
        drv2=drvec(2)
        drv1=drvec(1)

        IF(r3.le.pr(1)) THEN
          costhe= 2*drv1-1  
        ELSEIF(r3.le.pr(2)) THEN
          ymax=yma1/bb
          ymin=ymi1/bb
          y1=drv1*(ymax-ymin)+ymin
          costhe= (exp(bb*y1)-aa1)/bb
        ELSEIF(r3.le.pr(3)) THEN
          ymax=yma2/bb
          ymin=ymi2/bb
          y2=drv1*(ymax-ymin)+ymin
          costhe= -(exp(bb*y2)-aa2)/bb
        ELSEIF(r3.le.pr(4)) THEN
          ymax= -(ymi1-yma1)**nlo/(bb*nlo)
          ymin=  0d0
          y1=drv1*(ymax-ymin)+ymin
          tt=(aa1-bb)* exp( -(-nlo*bb*y1)**(1d0/dble(nlo)) )
          costhe= (tt -aa1)/bb
        ELSEIF(r3.le.pr(5)) THEN
          ymax= -(ymi2-yma2)**nlo/(bb*nlo)
          ymin=  0d0
          y1=drv1*(ymax-ymin)+ymin
          tt=(aa2-bb)* exp( -(-nlo*bb*y1)**(1d0/dble(nlo)) )
          costhe= -(tt -aa2)/bb
        ELSEIF(r3.le.pr(6)) THEN
          ymax= -(ymi1-yma1)**nlo2/(bb*nlo2)
          ymin=  0d0
          y1=drv1*(ymax-ymin)+ymin
          tt=(aa1-bb)* exp( -(-nlo2*bb*y1)**(1d0/dble(nlo2)) )
          costhe= (tt -aa1)/bb
        ELSEIF(r3.le.pr(7)) THEN
          ymax= -(ymi2-yma2)**nlo2/(bb*nlo2)
          ymin=  0d0
          y1=drv1*(ymax-ymin)+ymin
          tt=(aa2-bb)* exp( -(-nlo2*bb*y1)**(1d0/dble(nlo2)) )
          costhe= -(tt -aa2)/bb
        ELSEIF(r3.le.pr(8)) THEN
          y1=drv1*(tma1-tmi1) +tmi1
c          costhe= -(1/(y1*bb)+aa1)/bb
          ttt=(-1/(bb*ninv*y1))**(1d0/dble(ninv))
          costhe= -( -ttt +aa1)/bb
        ELSEIF(r3.le.pr(9)) THEN
          y1=drv1*(tma2-tmi2) +tmi2
c          costhe=  (1/(y1*bb)+aa2)/bb
          ttt=(-1/(bb*ninv*y1))**(1d0/dble(ninv))
          costhe=  ( -ttt +aa2)/bb
        ELSE
          write(6,*)'cosdec_br=> wrong total probability=',pr(7)
          stop
        ENDIF

        phi=2*pi*drv2

      IF(abs(costhe).GT.1d0) THEN
        WRITE(6,*)'TROUBLE in cosdec_br ==> costhe > 1 '
        WRITE(6,*)'cosdec_br: costhe,phi,p0=',costhe,phi,p0
        WRITE(6,*)'pp=',pp
        WRITE(6,*)'pinb1',pinb1
        WRITE(6,*)'pinb2',pinb2
        WRITE(6,*)'b12  ',b12
        WRITE(6,*)'dmas2(pinb1)',dmas2(pinb1),dmas2(pinb2)
        WRITE(6,*)'s12,s12,s1,s2',s12,dmas2(b12),s1,s2

        STOP
      ENDIF

      ENDIF
      IF(mode.EQ.0) THEN
        tpr1 = aa1+bb*costhe
        tpr2 = aa2-bb*costhe
C        write(6,*)'0 tpr1-2 ',tpr1,tpr2,costhe
      ELSEIF(mode.EQ.1) THEN
        do i=1,4
          pb1(i)=pinb1(i)-pout1(i)
          pb2(i)=pinb1(i)-pout2(i)
        enddo
        tpr1 = -dmas2(pb1)/2+amflt2/2d0
        tpr2 = -dmas2(pb2)/2+amflt2/2d0
        IF(tpr1.lt.amflt2/2) tpr1=amflt2/2
        IF(tpr2.lt.amflt2/2) tpr2=amflt2/2
C        write(6,*)'1 tpr1-2 ',tpr1,tpr2
      ENDIF

      crd_ang(mkrok,1) = 1d0 /(4*pi)
      crd_ang(mkrok,2) = 2/tpr1 /((yma1-ymi1)/bb)  /(4*pi)
      crd_ang(mkrok,3) = 2/tpr2 /((yma2-ymi2)/bb)  /(4*pi)
      crd_ang(mkrok,4) = 2*(ymi1-log(tpr1))**n_log/tpr1 
     @                    /((ymi1-yma1)**nlo/(-nlo*bb))  /(4*pi)
      crd_ang(mkrok,5) = 2*(ymi2-log(tpr2))**n_log/tpr2 
     @                    /((ymi2-yma2)**nlo/(-nlo*bb))  /(4*pi)
      crd_ang(mkrok,6) = 2*(ymi1-log(tpr1))**n_log2/tpr1 
     @                    /((ymi1-yma1)**nlo2/(-nlo2*bb))  /(4*pi)
      crd_ang(mkrok,7) = 2*(ymi2-log(tpr2))**n_log2/tpr2 
     @                    /((ymi2-yma2)**nlo2/(-nlo2*bb))  /(4*pi)
      crd_ang(mkrok,8) = 2/tpr1**n_inv /(tma1-tmi1)  /(4*pi)
      crd_ang(mkrok,9) = 2/tpr2**n_inv /(tma2-tmi2)  /(4*pi)

      xccos =0d0
      DO i=1,n_ang
        xccos= xccos +prob_ang(mkrok,i)*crd_ang(mkrok,i) 
      ENDDO

      wt= 1d0/xccos

      end

