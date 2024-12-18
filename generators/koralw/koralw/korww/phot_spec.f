      FUNCTION phot_spec_crud(svar,sprim,label)
**********************************************************
! NOTICE 
! This function is used ONLY to fill the prob_crud matrix!!!
! It should not be called directly by any other routines.
! The prob_crud matrix can be used instead!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!
! this is becoming an unbelievably nasty function... ms.
!
! convention for prob_chan(numb):
! 1-81: WW Wp=1:1-9; 2:10-18..
! 82-202: ZZ Z1=1:82-92; 2:93-103..
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
! the following common is introduced temporarily, to ensure adiabatic
! transition, to be removed later on
      COMMON / DECDAT / AMdumm(20), BR(20)
! This common can be everywhere, contains various switches
      COMMON / KeyKey /  KeyRad,KeyPhy,KeyTek,KeyMis,KeyDwm,KeyDwp
!   -- cuts for selecto
      common /articut/ arbitr,arbitr1,themin,arbitr2
! end commons
      DIMENSION br_crud(20),brz_crud(20)
      SAVE

      DATA init /0/
      SAVE init,factor_z

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
      IF(init .EQ. 0) THEN
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        init=1
! br_crud is the TEMPORARY fundamental DUMMY quantity to construct
! crude probabilities of decay channels, to be ramoved later on.
        DO i=1,20
          br_crud(i)=br(i)
        ENDDO
! stuff from filexp=================start
        Key4f  = MOD(KeyMis,100)/10
        FACWEL=1d0
        FACZEL=1d0
        if(key4f.NE.0)  then
          FACWEL=7d0
          br_crud(7)=br_crud(7)*FACWEL

          FACZEL=48d0!21d0
          IF(arbitr.LT.500d0 .AND.  arbitr.GT.0.1d0) THEN
            faczel=faczel *(.5d0*500d0/arbitr +.5d0)
          ELSEIF(arbitr.LE.0.1d0) THEN
! this is unexploited region, faczel set to an arbitrary number
            faczel=faczel * 2500d0
          ENDIF
        endif

        probw=0
        do k=1,9
          probw=probw+br_crud(k)
        enddo
        do k=1,9
          br_crud(k)=br_crud(k)/probw
        enddo
!
        probw=1
        probh=0.5d0-1.5d0*br_crud(8)
        probh=0.5d0*(1d0-br_crud(7)-br_crud(8)-br_crud(9))

        probz=6d0*br_crud(8)+5d0*probh
        if(key4f.NE.0) probz=(5d0+FACZEL)*br_crud(8)+5d0*probh
        do k=6,11
          brz_crud(k)=br_crud(8)/probz
        enddo
        if(key4f.NE.0)  brz_crud(6)=brz_crud(6)*FACZEL
        do k=1,5
          brz_crud(k)=probh/probz
        enddo
        probz=probz**2
        probw=probw/(probw+probz)
        probz=1d0-probw
! stuff from filexp=================end

        braw=0d0
        braz=0d0
        DO i=1,11
          DO j=1,11
            braw=braw+br_crud(i)*br_crud(j)
            braz=braz+brz_crud(i)*brz_crud(j)
          ENDDO
        ENDDO
        factor_z=braw/braz*(probz/probw)
c        write(6,*)'braw/braz,probw/probz',braw,braz,probw/probz
! end backw. compat
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
       ENDIF
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

      IF(label .GT. 202) THEN
        phot_spec_crud=0d0
        RETURN
      ENDIF

! energy dependence
      phot_spec= tot_born_crud(svar,sprim)
! flavor dependence (this is a routine from decay.f)
      CALL linear_to_WZ_label(1,label,iwm,iwp,if_z,if_w)
c      write(6,*)'1 label,iwm,iwp,if_z,if_w',label,iwm,iwp,if_z,if_w
!-- find the overall W/Z normalisation
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!! disabled for now !!!!!!!!!!!!!!!!!!!!!!!!
ccc      goto 11
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!   
      IF(if_w .EQ. 1) THEN
!-- WW
        phot_spec= phot_spec* br_crud(iwm)*br_crud(iwp)
      ELSEIF(if_z .EQ. 1) THEN
!-- ZZ
        phot_spec= phot_spec* brz_crud(iwm)*brz_crud(iwp)
! factor_z for backw. comp.
        phot_spec= phot_spec *factor_z
      ENDIF
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 11   continue
      goto 12
!!!!!!!!!!!!!!!!!! disabled for now !!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
      IF(if_w .EQ. 1) THEN
!-- WW
        brcm=1
        brcp=1
        IF(iwm.eq.7) brcm=1
        IF(iwp.eq.7) brcp=1 
        phot_spec= phot_spec* brcm *brcp
      ELSEIF(if_z .EQ. 1) THEN
!-- ZZ
        brcm=1
        brcp=1
        IF(iwm.eq.6) brcm=7 
        IF(iwp.eq.6) brcp=7 
        phot_spec= phot_spec* brcm *brcp
      ENDIF
!!!!!!!!!!!!!!!!!! added for now end !!!!!!!!!!!!!!!!!!!!!!!
 12   continue
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
      phot_spec_crud = phot_spec

      END


      FUNCTION tot_born_crud(svar,sprim)
*     ***********************************
* total crude born xsection for photonic spectra
* 
*     ***********************************
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      COMMON / matpar / pi,ceuler
      COMMON / phypar / alfinv,gpicob
      COMMON / weking / ene,amaz,gammz,amel,amfin,xk0,sinw2,ide,idf
      COMMON / wekin2 / amaw,gammw,gmu,alphaw
* this COMMON can be everywhere, contains various switches
      COMMON / keykey /  keyrad,keyphy,keytek,keymis,keydwm,keydwp

      keysmp = MOD(keytek,1000)/100

      IF (keysmp .EQ. 0) THEN
! good old CC03
!--     S1-S2 integral over L area
        CALL resspec(svar,sprim,amaw,gammw,prnorm)
        born_crud=prnorm *bornscrud(sprim,2)
      ELSE
! full matr. el.
!--     S1-S2 integral over L area
        CALL resspec(svar,sprim,amaw,gammw,prnorm)
        born_crud=bornscrud(svar,2) *prnorm
* some trick of Zbyszeks
        svarxx=sprim
        IF(sprim .GT. amaz**2+gammz**2) THEN
          xls =log((svar-amaz**2)/gammz**2)
          xlsi=log((sprim-amaz**2)/gammz**2)
          xlsix=xls*(xlsi/xls)**2
          svarxx=amaz**2+gammz**2*exp(xlsix)
        ENDIF
        xsvarx=1/sprim+sprim/((svarxx-amaz**2)**2+(sprim*gammz/amaz)**2)
        svarix=svar
        xsvari=1/svar+svar/((svarix-amaz**2)**2+(svar*gammz/amaz)**2)
        born_crud=born_crud*xsvarx/xsvari
      ENDIF

      tot_born_crud=born_crud

      END

      SUBROUTINE resspec(svar,sprim,rmas,rgam,prnorm)
*     ***************************************************************
*crude FUNCTION for spectrum normalization
*prnorm calculates the value of integral
* ds_1d_s2 w(s_1)w(s_2) over theta crude region (see koralw 1.02) manual.
*its results cancels out in final results.
* this FUNCTION is arbitrary up to the problems with maximum weight and
* algorithm efficiency.
* note: both resonances have the same mass distribution FUNCTION
*         svar    - max sprim
*         sprim   - actual s
*         rmas    - central value of a resonance mass distribution
*         rgam    - width of a resonance
*         prnorm  - value of the integral of crude distr.
*
* written by: m. skrzypek            date: 2/16/95
* last update: 5/07/96                        by: z.was
*
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      COMMON / matpar / pi,ceuler
      SAVE a,b,winf,wmi,wma,wsqr,wrec,prsqr,prrec
      SAVE uma,umi,uinf,usqr,urec
      SAVE
*
*        WRITE(6,*)'resms2',rmas,rgam
      a=rmas**2
      b=rmas*rgam
* arctg
      winf = 1/b*atan((svar   -a)/b)
      wma  = 1/b*atan((sprim/4d0-a)/b)
      wmi  = 1/b*atan(        -a /b)
* logarithm
      uinf =1/2d0/a*dlog((svar   -a)**2 +b**2)
      uma  =1/2d0/a*dlog((sprim/4d0-a)**2 +b**2)
      umi  =1/2d0/a*dlog(                b**2)
* thetas
      thespr=1d0
      thesvr=1d0
      IF((sprim/4d0) .LT. a) thespr=0d0
      IF( svar       .LT. a) thesvr=0d0
      ulo= thespr*uma +(1d0-thespr)*umi
* normalisations
      wsqr=wma-wmi
      usqr=thespr*(uma-umi)
      prsqr=(wsqr+usqr)**2
      wrec=winf-wma
      urec=thesvr*(uinf -ulo)
      prrec=(wsqr+usqr)*(wrec+urec)
      prnorm=prsqr+2*prrec
      END

      FUNCTION bornscrud(svari,mode)
*     ***********************************
* this routine provides born crude cross section
* mode = 1 : normalized to total(s) (not used i.e. museum ?
*        2 : not normalized
*        difference is only in normalization !!!!?????
*     ***********************************
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      COMMON / matpar / pi,ceuler
      COMMON / phypar / alfinv,gpicob
      COMMON / weking / ene,amaz,gammz,amel,amfin,xk0,sinw2,ide,idf
      COMMON / wekin2 / amaw,gammw,gmu,alphaw
      SAVE / weking /,/ wekin2 /
      SAVE
      bss= pi/alfinv**2 /2d0 /sinw2**2
      bbwign=amaw**2/alfinv/pi/sinw2
      thr=1d0/svari/4d0
      IF(svari .GT. 8*amaw**2) THEN
        bs=1d0/svari*(1+4*amaw**2/svari)/2d0*log(svari/amaw**2)
      ELSEIF(svari .GT. 4*amaw**2) THEN
        beta=dsqrt(1-4*amaw**2/svari)
        IF(beta .LT. pi/alfinv) beta=pi/alfinv ! coulomb!
        bsr=1d0/svari*(1+4*amaw**2/svari)/2d0*log(svari/amaw**2)
        bs=bsr*(thr/bsr+beta*(1-thr/bsr))
      ELSE
        bs=thr*svari/4/amaw**2
      ENDIF
      bs=bs*(1+svari/sqrt((svari-amaz**2)**2+(svari*gammz/amaz)**2))/2d0
      bnorto=bss*gpicob*bbwign**2
      IF(mode .EQ. 2) THEN
        bornscrud=bs*4*amaw**2 *2
      ELSE
        WRITE(6,*)'bornsc=> mode <> 2:',mode
        STOP
      ENDIF
***      IF(mode .EQ. 1) bornscrud=bs*4*amaw**2 *2 *bnorto

      END

