      SUBROUTINE karlud(mode,par1,par2,par3)
*     **************************************
* low level  monte-carlo generator
* administrates directly generation of v-variable
* and indirectly of all other variables.
*     **************************************
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      PARAMETER (nmax= 40)
      COMMON / matpar / pi,ceuler
      COMMON / phypar / alfinv,gpicob
      COMMON / weking / ene,amaz,gammz,amel,amfin,xk0,sinw2,ide,idf
      COMMON / wekin2 / amaw,gammw,gmu,alphaw
! user mask on final states
      COMMON /cumask/ user_umask(202)
* this COMMON can be everywhere, contains various switches
      COMMON / keykey/  keyrad,keyphy,keytek,keymis,keydwm,keydwp
      COMMON / inout  / ninp,nout
      COMMON / bxfmts / bxope,bxclo,bxtxt,bxl1i,bxl1f,bxl2f,bxl1g,bxl2g
      CHARACTER*80      bxope,bxclo,bxtxt,bxl1i,bxl1f,bxl2f,bxl1g,bxl2g

      COMMON / momset / qeff1(4),qeff2(4),sphum(4),sphot(100,4),nphot
      COMMON / momdec / q1(4),q2(4),p1(4),p2(4),p3(4),p4(4)
      COMMON / cms_eff_momdec /
     $      effbeam1(4),effbeam2(4),effp1(4),effp2(4),effp3(4),effp4(4)
      COMMON / wgtgen / wtves,wtyfs,wtborn
      COMMON / wgtall / wtcrud,wtmod,wtset(100)

! this common breaks modularity. Should be removed somehow.
* communicates with vesko/rhosko
      COMMON / vvrec  / vvmin,vvmax,vv,beti

      EXTERNAL rhosko
      DIMENSION drvec(1)

      DIMENSION effq1(4),effq2(4)

      SAVE

*     ==================================================================
*     ===================initialization=================================
*     ==================================================================
      IF(mode .EQ. -1) THEN
*     ==================================================================
      KeyMas = MOD(KeyPhy,100)/10
      IF(  KeyMas .EQ. 0 ) THEN
        ambeam=0d0
      ELSE
        ambeam=amel
      ENDIF

      svar = 4d0*ene**2
* this is "pointer" for internal monitoring histograms/averages
      idyfs = 0


* 4f m-type phase space debugg mode, 
* READs 4vects and ikan from the disk,
* activated with msdump =1
* (requires keyisr=0 and keysmp=2 or 3)
      msdump=0

* m probability in m/z mixing of 4f phase space generators
      prob_ps_m=0.5d0

!-- yfs internal tests, activated with i_yfs=1
      i_yfs=0

*-- initialize decays
! what should emin be ??????? some additional cut-off ?????
      emin=1d0
      emax=2*ene
! i_file=1 requests xsection data from the file 
!          (50-250GeV for now, flat elsewhere)
! i_file=0 requests xsection data from analytic function
      i_file=1
      KeyWgt = MOD(KeyTek,10)
      KeySmp = MOD(KeyTek,1000)/100 
      CALL decay_prob_init(emin,emax,user_umask,i_file,KeyWgt,keysmp)
*-- initialize decays end

! is beti needed by / vvrec / ????
      beti = 2d0/alfinv/pi*(dlog(4d0*ene**2/amel**2)-1d0)

*-- initialization of qed part
      keyisr = MOD(keyrad,10)
*-- calculation of crude normalization
      IF( keyisr .NE. 0)THEN
         CALL vesk1w(-1,rhosko,dum1,dum2,xcvesk)
         CALL gifyfs(svar,amel,fyfs)
         xcrude = xcvesk*fyfs

*-- initialize internal tests of yfs
         IF(i_yfs. EQ. 1) 
     @     CALL yfs_tests(-1,amel,ene,idyfs,wtves,xcvesk,wt1,wt2,wt3)
      ELSE
! note, for no ISR xcrude is completely dummy, it will be divided out
! later on. can be set to 1 as well...
         sprim=svar
         xcrude=get_total_crude(sprim)
         fyfs=1d0
         xcvesk=0d0
      ENDIF
*-- outputs
      par1=xcrude
      par2=xcvesk
      par3=xcrude
      keysmp = MOD(keytek,1000)/100

* ==================================================================
* ====================generation====================================
* ==================================================================
      ELSEIF( mode  .EQ.  0) THEN
* ==================================================================

      CALL cleanup

      wtves=1d0
      wtyfs=1d0
* generate vv
      IF( keyisr .NE. 0 ) THEN
         CALL vesk1w( 0,rhosko,dum1,dum2,wtves)
* low-level multiphoton generator
         CALL yfsgen(vv,vvmin,nmax,wt1,wt2,wt3)

*-- internal tests of yfs (should that be before or after next line???)
         IF(i_yfs. EQ. 1) 
     @      CALL yfs_tests(0,amel,ene,idyfs,wtves,xcvesk,wt1,wt2,wt3)
*-- photons under ir cut treated as 0
         IF( vv  .LT.  vvmin) vv=0d0
      ELSE
         vv=0d0
         wt1=1d0
         wt2=1d0
         wt3=1d0
      ENDIF
      wtyfs=wt1*wt2*wt3
      IF( wtyfs  .EQ.  0d0) THEN
         wtkarl=0d0
         wtborn=0d0
         GOTO 150
      ENDIF

      sprim=svar*(1-vv)

*-- generate decay channel
      CALL varran(drvec,1)
      rndm=drvec(1)
      CALL decay(sprim,rndm,label)
*-- store label for future use from phot_spec_book ! temporary !!
      CALL store_label(0,label)

*======================================================
*=========== 4-body phase space begin =================
*======================================================

*-- make a choice of kinematical branch
*-- generate 4f hyper-point
      IF( keysmp .EQ. 2 ) THEN
*-- m branch
         CALL make_phsp_point
     $        (msdump,label,ambeam,svar,sprim,fak_phsp,
     $          effbeam1,effbeam2,effp1,effp2,effp3,effp4)
      ELSEIF( keysmp .LE. 1 ) THEN
*-- z branch
         CALL make_phsp_point_z
     $      (msdump,ambeam,svar,sprim,fak_phsp,
     $       effbeam1,effbeam2,effp1,effp2,effp3,effp4)
      ELSEIF( keysmp .EQ. 3 ) THEN
        WRITE(6,*)'karludw=> KeySmp=3 is a test option. Disabled'
        WRITE(6,*)'If you really want to play with it contact authors'
        STOP
*-- mixed branch
         CALL make_phsp_point_mz
     $      (prob_ps_m,msdump,ambeam,svar,sprim,fak_phsp,i_m,
     $       effbeam1,effbeam2,effp1,effp2,effp3,effp4)
      ELSE
         WRITE(6,*)'karludw=>wrong keysmp=',keysmp
         STOP
      ENDIF

*-- event outside physical phase space?
      IF(fak_phsp .EQ. 0d0)  THEN
         wtkarl=0d0
         wtborn=0d0
         GOTO 150
      ENDIF

! set auxilliary effq-i
      DO i=1,4
        effq1(i)=effp1(i)+effp2(i)
        effq2(i)=effp3(i)+effp4(i)
      ENDDO

* transform everything back to cms (lab)
      CALL from_cms_eff(1,svar,sprim,ambeam,sphum,sphot,nphot,
     $      effq1,effq2,effp1,effp2,effp3,effp4,
     $      qeff1,qeff2,q1,q2,p1,p2,p3,p4)

*... place selecto on some phase space regions to zero them
      fak_sel=1d0
      CALL selecto(p1,p2,p3,p4,qeff1,qeff2,fak_sel)
*... place users cuts on some phase space regions to zero them
      user_fak_sel=1d0
      CALL user_selecto(p1,p2,p3,p4,qeff1,qeff2,user_fak_sel)

      fak_sel=fak_sel*user_fak_sel

*... event outside physical phase space?
      IF(fak_sel .EQ. 0d0)  THEN
         fak=0d0
         wtkarl=0d0
         wtborn=0d0
         GOTO 150
      ENDIF

*-- calculate total jacobian (summed over branches)
      IF( keysmp  .EQ.  2 ) THEN
         CALL get_phsp_weight
     $      (label,ambeam,svar,sprim,fak,
     $       effbeam1,effbeam2,effp1,effp2,effp3,effp4)
      ELSEIF( keysmp  .LE.  1 ) THEN
         CALL get_phsp_weight_z
     $      (svar,sprim,fak,
     $       effbeam1,effbeam2,effp1,effp2,effp3,effp4)
      ELSEIF( keysmp .EQ. 3 ) THEN
         CALL get_phsp_weight_mz
     $      (prob_ps_m,ambeam,svar,sprim,fak,i_m,
     $       effbeam1,effbeam2,effp1,effp2,effp3,effp4)
      ENDIF

      IF(fak .EQ. 0d0)  THEN
         wtkarl=0d0
         wtborn=0d0
         GOTO 150
      ENDIF

*...  statistical factors for identical particles
      CALL selstfac(stat_fac)
      fak=fak*stat_fac

*======================================================
*=========== 4-body phase space END ===================
*======================================================

*------------------------------------------------------
*------------------ END generation --------------------
*------------------------------------------------------

* first come weights from vesko, inverse of the dummy
* born used in photon spectrum and yfs.
c ms      xcborn=1d0/tot_born_crud(svar,sprim)
      xcborn=1d0/get_total_crude(sprim)
      wtkarl=wtves*xcborn*wtyfs

*-- weight for phase space  volume
       wtkarl=wtkarl *fak

!============================
      IF(wtkarl .NE. 0d0)THEN
!============================

! decay channel probability normalisation factor 
         wmp= get_decay_prob(sprim,label)
         IF(wmp .LE. 0d0) THEN
           WRITE(6,*)'karludw=>generated channel with 0 probability',wmp
           STOP
         ELSE
           wtkarl=wtkarl/wmp
         ENDIF

!-- we move born and other weights up to koralw, as they need the 4fermion
!-- matr. el also

!==========
      ENDIF
!==========

      IF(wtkarl .LT. 0d0) THEN
         WRITE(*,*) 'ujemna waga itype=',itype
         WRITE(*,*) wtkarl
         WRITE(*,*) 'aa'
         WRITE(*,*) faki
         WRITE(*,*) 'bb'
         WRITE(*,*) p1
         WRITE(*,*) p2
         WRITE(*,*) p3
         WRITE(*,*) p4
      ENDIF

 150  CONTINUE

      par1=xcborn
      par2=wtkarl
      par3=0d0

*     ======================
      ELSEIF(mode .EQ. 1) THEN
*     ======================

*-- crude xsection xcvesk and yfs formfactor fyfs
      IF( keyisr .NE. 0 )THEN
         CALL vesk1w( 1,rhosko,xsve,erelve,xcvesk)
         CALL gifyfs(svar,amel,fyfs)
         xcrude=xcvesk*fyfs

*-- internal tests of yfs
         IF(i_yfs. EQ. 1) 
     @     CALL yfs_tests(1,amel,ene,idyfs,wtves,xcvesk,wt1,wt2,wt3)
      ELSE
c ms         xcrude=tot_born_crud(svar,sprim)
         xcrude=get_total_crude(sprim)
         fyfs=1d0
         xcvesk=0d0
      ENDIF
*-- outputs
      par1=xcrude
      par2=xcvesk
      par3=xcrude

* ============
      ELSE
* ============

*-- internal tests of yfs
      IF( keyisr .NE. 0 ) THEN
        IF(i_yfs.EQ.1) 
     @    CALL yfs_tests(mode,amel,ene,idyfs,wtves,xcvesk,wt1,wt2,wt3)
      ENDIF

*-- born xsection, total!
      IF( keyisr .NE. 0 ) THEN
        CALL vesk1w( 1,rhosko,xsve,erelve,xcvesk)
        CALL gifyfs(svar,amel,fyfs)
        xcrude=xcvesk*fyfs
      ELSE
c ms        xcrude=tot_born_crud(svar,sprim)
        xcrude=get_total_crude(sprim)
        fyfs=1d0
        xcvesk=0d0
      ENDIF
      CALL gmonit(1,idyfs+58,averw,errel,parm3)
      xsborn  = xcrude*averw
      erborn  = xsborn*errel
      par1=0d0
      par2=xsborn
      par3=erborn
* ==========
      ENDIF
* ==========
      END

      SUBROUTINE from_cms_eff(mode,svar,sprim,ambeam,sphum,sphot,
     $      nphot, bq1,bq2,bp1,bp2,bp3,bp4,
     $      qeff1,qeff2,q1,q2,p1,p2,p3,p4)
*     **********************************************************
* this routine transforms 4momenta between cms (lab) and
*      effective cms (p_final)
*
* mode =  1   from cms_eff to cms
* mode = -1   from cms to cms_eff
* bq1,bq2,bp1,bp2,bp3,bp4   arbitrary 4momenta to be transformed
*
* output
*  q1, q2, p1, p2, p3, p4   transformed 4momenta
*  qeff1,qeff2              effective beams in cms (mode=1)
*                                           in cms_eff (mode=-1)
*
* written by: m. skrzypek              date: 7/6/96
*
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      DIMENSION qeff1(4),qeff2(4),sphum(4),sphot(100,4)
      DIMENSION bq1(4),bq2(4),bp1(4),bp2(4),bp3(4),bp4(4)
      DIMENSION q1(4),q2(4),p1(4),p2(4),p3(4),p4(4)
      DIMENSION qsu(4),ef1(4),ef2(4),photd(4),sphotd(100,4)
      DIMENSION def1(4),def2(4)

* to activate dumps kindmp=1
      kindmp=0

      DO 124, i=1,4
      q1(i)=bq1(i)
      q2(i)=bq2(i)
      p1(i)=bp1(i)
      p2(i)=bp2(i)
      p3(i)=bp3(i)
 124  p4(i)=bp4(i)


      IF(nphot .EQ. 0) THEN
*     ===================

* define effective beams (massive) in cms''

        qeff1(4)= sqrt(svar)/2d0
        qeff1(3)= dsqrt(svar/4d0-ambeam**2)
        qeff1(2)= 0d0
        qeff1(1)= 0d0
        qeff2(4)= sqrt(svar)/2d0
        qeff2(3)=-dsqrt(svar/4d0-ambeam**2)
        qeff2(2)= 0d0
        qeff2(1)= 0d0

      ELSEIF(nphot .GE. 1) THEN
*     =======================

* effective beams (in lab)
        ef1(4)= sqrt(svar)/2d0
        ef1(3)= dsqrt(svar/4d0-ambeam**2)
        ef1(2)= 0d0
        ef1(1)= 0d0
        ef2(4)= sqrt(svar)/2d0
        ef2(3)=-dsqrt(svar/4d0-ambeam**2)
        ef2(2)= 0d0
        ef2(1)= 0d0
        DO 11 ii=1,nphot
        IF(sphot(ii,3) .GE. 0d0) THEN
          DO 12 jj=1,4
 12       ef1(jj)= ef1(jj)-sphot(ii,jj)
        ELSE
          DO 13 jj=1,4
 13       ef2(jj)= ef2(jj)-sphot(ii,jj)
        ENDIF
 11     CONTINUE

      IF(kindmp .EQ. 1)THEN
        WRITE(6,*)'__lab0, ef1,ef2'
        CALL dumpl(6,p1,p2,p3,p4,ef1,ef2,sphot,nphot)
      ENDIF

* qsu is 4momentum of decay products (p1-p4) in cms
        DO 110 k=1,4
  110 qsu(k)=-sphum(k)
        qsu(4)=qsu(4)+sqrt(svar)

* transform ef1,2 to rest frame (cms')
* cms' is rotated cms_eff (z+ not along qeff1) !!!
        CALL boostv(1,qsu,ef1,ef1)
        CALL boostv(1,qsu,ef2,ef2)

      IF(kindmp .EQ. 1)THEN
        WRITE(6,*)'__cms ef1,2, phots lab'
        CALL dumpl(6,p1,p2,p3,p4,ef1,ef2,sphot,nphot)
        DO 70 i=1,nphot
        DO 71 j=1,4
 71     photd(j)=sphot(i,j)
*.. photons to cms'
        CALL boostv( 1,qsu,photd,photd)
*.. photons to cms''
        CALL rotatv(1,ef1,photd,photd)
        DO 72 j=1,4
 72     sphotd(i,j)=photd(j)
 70     CONTINUE
        WRITE(6,*)'cms ef1,2 phots cmsbis'
        CALL dumpl(6,p1,p2,p3,p4,ef1,ef2,sphotd,nphot)
* control
        CALL rotatv(-1,ef1,ef2,def2)
        WRITE(6,*)'__control'
        CALL dumpl(6,p1,p2,p3,p4,ef1,def2,sphot,nphot)
        CALL rotatv( 1,ef1,ef2,def2)
        WRITE(6,*)'__control'
        CALL dumpl(6,p1,p2,p3,p4,ef1,def2,sphot,nphot)
      ENDIF

*++++++++++++++++++++++++
      IF(mode .EQ. 1) THEN
*       from cms_eff to cms
*++++++++++++++++++++++++

* define effective beams (massive) in cms'' (cms_eff, z+ along qeff1)
        qeff1(4)= sqrt(sprim)/2d0
        qeff1(3)= dsqrt(sprim/4d0-ambeam**2)
        qeff1(2)= 0d0
        qeff1(1)= 0d0
        qeff2(4)= sqrt(sprim)/2d0
        qeff2(3)=-dsqrt(sprim/4d0-ambeam**2)
        qeff2(2)= 0d0
        qeff2(1)= 0d0

        IF(kindmp .EQ. 1)THEN
          WRITE(6,*)'___cmsbis, qeff1,qeff2'
          CALL dumpl(6,p1,p2,p3,p4,qeff1,qeff2,sphotd,nphot)
        ENDIF

* rotate from cms'' (z along ef1) to cms' (z along e- beam)
          CALL rotatv(-1,ef1,qeff1,qeff1)
          CALL rotatv(-1,ef1,qeff2,qeff2)
          CALL rotatv(-1,ef1,q1,q1)
          CALL rotatv(-1,ef1,q2,q2)
          CALL rotatv(-1,ef1,p1,p1)
          CALL rotatv(-1,ef1,p2,p2)
          CALL rotatv(-1,ef1,p3,p3)
          CALL rotatv(-1,ef1,p4,p4)

        IF(kindmp .EQ. 1)THEN
          WRITE(6,*)'__cmsprim'
          CALL dumpl(6,p1,p2,p3,p4,qeff1,qeff2,sphot,nphot)
        ENDIF

* transform back to lab
          CALL boostv(-1,qsu,qeff1,qeff1)
          CALL boostv(-1,qsu,qeff2,qeff2)
          CALL boostv(-1,qsu,q1,q1)
          CALL boostv(-1,qsu,q2,q2)
          CALL boostv(-1,qsu,p1,p1)
          CALL boostv(-1,qsu,p2,p2)
          CALL boostv(-1,qsu,p3,p3)
          CALL boostv(-1,qsu,p4,p4)
*.. fine tuning on masses
          qeff1(4)=dsqrt(ambeam**2+qeff1(1)**2+qeff1(2)**2+qeff1(3)**2)
          qeff2(4)=dsqrt(ambeam**2+qeff2(1)**2+qeff2(2)**2+qeff2(3)**2)

        IF(kindmp .EQ. 1)THEN
          WRITE(6,*)'__lab, qeff'
          CALL dumpl(6,p1,p2,p3,p4,qeff1,qeff2,sphot,nphot)
* transform ef1 back to lab
          CALL boostv(-1,qsu,ef1,def1)
          CALL boostv(-1,qsu,ef2,def2)
          WRITE(6,*)'__lab all, ef'
          CALL dumpl(6,p1,p2,p3,p4,def1,def2,sphot,nphot)
        ENDIF
*++++++++++++++++++++++++
        ELSEIF(mode .EQ. -1) THEN
*       to cms_eff from cms
*++++++++++++++++++++++++
          CALL boostv(1,qsu,q1,q1)
          CALL boostv(1,qsu,q2,q2)
          CALL boostv(1,qsu,p1,p1)
          CALL boostv(1,qsu,p2,p2)
          CALL boostv(1,qsu,p3,p3)
          CALL boostv(1,qsu,p4,p4)
          CALL rotatv(1,ef1,q1,q1)
          CALL rotatv(1,ef1,q2,q2)
          CALL rotatv(1,ef1,p1,p1)
          CALL rotatv(1,ef1,p2,p2)
          CALL rotatv(1,ef1,p3,p3)
          CALL rotatv(1,ef1,p4,p4)
*++++++++++++++++++++++++
        ELSE
*++++++++++++++++++++++++
          WRITE(6,*)'from_cms_eff==> wrong mode: ',mode
          STOP
*++++++++++++++++++++++++
        ENDIF
*++++++++++++++++++++++++
      ELSE
*     ====
        WRITE(6,*)'from_cms_eff==> wrong no of phots: ',nphot
        STOP
      ENDIF
*     =====
      END

      SUBROUTINE selecto(p1,p2,p3,p4,p5,p6,wt)
* #################################################
* #        mask on phase space regions            #
* #             NOT TO BE MODIFIED                #
* #      use user_selecto for users cuts          #
* #################################################
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      COMMON /articut/ arbitr,arbitr1,themin,arbitr2
      COMMON / decays / IFlav(4), amdec(4) 
* this COMMON can be everywhere, contains various switches
      COMMON / keykey/  keyrad,keyphy,keytek,keymis,keydwm,keydwp
      SAVE /articut/,/ decays /,/ keykey/
      SAVE
      DIMENSION p1(4),p2(4),p3(4),p4(4),p5(4),p6(4)

      IF (wt .EQ. 0d0) RETURN
      angarb = SIN(themin)**2

* remove section of e+e-ch+ch- phase space where grace has numerical
* problem for config. with two nearly massless high p_t photons and electrons
* in final state.
!WP: tests ...
c         IF ((iflav(2) .EQ. -11) .OR. (iflav(3) .EQ. 11))THEN
c      IF (qadra(p1,p2)+qadra(p3,p4) .LE. arbitr1) wt=0d0
c         ENDIF
c         IF ((iflav(2) .EQ. -11) .AND. (iflav(3) .EQ. 11))THEN
c      IF (qadra(p1,p4)+qadra(p2,p3) .LE. arbitr1) wt=0d0
c         ENDIF
c      IF (wt .EQ. 0d0) RETURN

         IF ((iflav(2) .EQ. -11) .OR. (iflav(3) .EQ. 11))THEN
      IF (qadra(p1,p2).LE.arbitr1 .OR. qadra(p3,p4).LE.arbitr1) wt=0d0
         ENDIF
         IF ((iflav(2) .EQ. -11) .AND. (iflav(3) .EQ. 11))THEN
      IF (qadra(p1,p4).LE.arbitr1 .OR. qadra(p2,p3).LE.arbitr1) wt=0d0
         ENDIF
      IF (wt .EQ. 0d0) RETURN

!!!!!!!!!!!! additional cut on angle...temporary fixup

      IF(abs(iflav(1)) .NE. 12  .AND.  abs(iflav(1)) .NE. 14
     $  .AND.  abs(iflav(1)) .NE. 16  .AND.
     $ (p1(1)**2+p1(2)**2)/(p1(1)**2+p1(2)**2+p1(3)**2) .LT. angarb )
     $  wt=0d0

      IF(abs(iflav(2)) .NE. 12  .AND.  abs(iflav(2)) .NE. 14
     $  .AND.  abs(iflav(2)) .NE. 16  .AND.
     $ (p2(1)**2+p2(2)**2)/(p2(1)**2+p2(2)**2+p2(3)**2) .LT. angarb )
     $  wt=0d0

      IF(abs(iflav(3)) .NE. 12  .AND.  abs(iflav(3)) .NE. 14
     $  .AND.  abs(iflav(3)) .NE. 16  .AND.
     $ (p3(1)**2+p3(2)**2)/(p3(1)**2+p3(2)**2+p3(3)**2) .LT. angarb )
     $  wt=0d0

      IF(abs(iflav(4)) .NE. 12  .AND.  abs(iflav(4)) .NE. 14
     $  .AND.  abs(iflav(4)) .NE. 16  .AND.
     $ (p4(1)**2+p4(2)**2)/(p4(1)**2+p4(2)**2+p4(3)**2) .LT. angarb )
     $  wt=0d0

      IF (wt .EQ. 0d0) RETURN

!!!!!!!!!!!!!

      pt2=0
      IF(abs(iflav(1)) .NE. 12  .AND.  abs(iflav(1)) .NE. 14
     $  .AND.  abs(iflav(1)) .NE. 16  .AND.
     $ (p1(1)**2+p1(2)**2)/(p1(1)**2+p1(2)**2+p1(3)**2) .GT. angarb )
     $  pt2=pt2+p1(1)**2+p1(2)**2

      IF(abs(iflav(2)) .NE. 12  .AND.  abs(iflav(2)) .NE. 14
     $  .AND.  abs(iflav(2)) .NE. 16  .AND.
     $ (p2(1)**2+p2(2)**2)/(p2(1)**2+p2(2)**2+p2(3)**2) .GT. angarb )
     $  pt2=pt2+p2(1)**2+p2(2)**2

      IF(abs(iflav(3)) .NE. 12  .AND.  abs(iflav(3)) .NE. 14
     $  .AND.  abs(iflav(3)) .NE. 16  .AND.
     $ (p3(1)**2+p3(2)**2)/(p3(1)**2+p3(2)**2+p3(3)**2) .GT. angarb )
     $  pt2=pt2+p3(1)**2+p3(2)**2

      IF(abs(iflav(4)) .NE. 12  .AND.  abs(iflav(4)) .NE. 14
     $  .AND.  abs(iflav(4)) .NE. 16  .AND.
     $ (p4(1)**2+p4(2)**2)/(p4(1)**2+p4(2)**2+p4(3)**2) .GT. angarb )
     $  pt2=pt2+p4(1)**2+p4(2)**2

*!!!!!      IF(pt2 .LE. arbitr) wt=0d0
* set  .LT.  to have a chance to `see' electron neutrinos.
      IF(pt2 .LT. arbitr) wt=0d0

* this is dirty trick to get rid of events with high p_t photons
* which spoil e+e-xx final states.
* begin ==========================
       pt3=0
      IF(abs(iflav(2)) .EQ. 11 .OR. abs(iflav(3)) .EQ. 11) THEN
        pt3=pt3 +(p1(1)+p2(1)+p3(1)+p4(1))**2
        pt3=pt3 +(p1(2)+p2(2)+p3(2)+p4(2))**2
        IF(pt3 .GT. arbitr2) wt=0d0
      ENDIF
* end   =========================
* the following is the cut as used by v. 1.3x . It is practically
* identical to the above one that uses fermion four-momenta 
* instead of effective beams. also its numeric value was glued to the
* fermionic cut.
* begin ==========================
c       pt3=0
c      IF(abs(iflav(2)) .EQ. 11 .OR. abs(iflav(3)) .EQ. 11) THEN
c       pt3=pt3+p5(1)**2+p5(2)**2
c       pt3=pt3+p6(1)**2+p6(2)**2
c       IF(pt3 .GT. arbitr/2) wt=0d0
c      ENDIF
* end   =========================
* this is the END of routine!!
      RETURN


*rubbish kept `in any case'
*
*      IF ((abs(iflav(1)) .EQ. 11) .AND. (abs(iflav(4)) .EQ. 11)) THEN
*      IF (  (    ((p4(3)/p4(4))**2 .GT. 0.96d0)
*     $        .AND. ((p1(3)/p1(4))**2 .GT. 0.96d0) ) .OR.
*     $ ((p4(1)**2+p4(2)**2+p1(1)**2+p1(2)**2) .LE. 49d0)) wt=0d0
*      ENDIF
*!!!      IF (qadra(p1,p2) .LE. 1d0) wt=0d0
*      IF (qadra(p1,p3) .LE. arbitr) wt=0d0
*      IF (qadra(p1,p4) .LE. arbitr) wt=0d0
*      IF (qadra(p1,p5) .LE. arbitr) wt=0d0
*      IF (qadra(p1,p6) .LE. arbitr) wt=0d0
*      IF (qadra(p2,p3) .LE. arbitr) wt=0d0
*      IF (qadra(p2,p4) .LE. arbitr) wt=0d0
*      IF (qadra(p2,p5) .LE. arbitr) wt=0d0
*      IF (qadra(p2,p6) .LE. arbitr) wt=0d0
*      IF (qadra(p3,p4) .LE. arbitr) wt=0d0
*      IF (qadra(p3,p5) .LE. arbitr) wt=0d0
*      IF (qadra(p3,p6) .LE. arbitr) wt=0d0
*      IF (qadra(p4,p5) .LE. arbitr) wt=0d0
*      IF (qadra(p4,p6) .LE. arbitr) wt=0d0
*      IF (qadra(p5,p6) .LE. arbitr) wt=0d0
*       IF (wt .EQ. 0d0) RETURN
*      IF ((p1(1)**2+p1(2)**2) .LE. arbitr) wt=0d0
*      IF ((p2(1)**2+p2(2)**2) .LE. arbitr) wt=0d0
*      IF ((p3(1)**2+p3(2)**2) .LE. arbitr) wt=0d0
*      IF ((p4(1)**2+p4(2)**2) .LE. arbitr) wt=0d0

*       IF (wt .EQ. 0d0) RETURN
*      IF ((p1(1)**2+p1(2)**2)/p1(4)**2 .LE. angarb) wt=0d0
*      IF ((p2(1)**2+p2(2)**2)/p2(4)**2 .LE. angarb) wt=0d0
*      IF ((p3(1)**2+p3(2)**2)/p3(4)**2 .LE. angarb) wt=0d0
*      IF ((p4(1)**2+p4(2)**2)/p4(4)**2 .LE. angarb) wt=0d0
*       WRITE(*,*) p1
*       WRITE(*,*) p2
*       WRITE(*,*) p3
*       WRITE(*,*) p4
*       WRITE(*,*) '---------'
*       WRITE(*,*) p5
*       WRITE(*,*) p6
      END

      SUBROUTINE selstfac(wt)
* ######################################
* introduces statistical factor for identical particles
      IMPLICIT DOUBLE PRECISION (a-h,o-z)

      COMMON / decays / IFlav(4), amdec(4) 

      IF(iflav(1) .EQ. iflav(3)) THEN
        wt=1/4d0
      ELSE
        wt=1d0
      ENDIF

      END

      FUNCTION qadra(p1,p2)
*     ***************************************
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      DIMENSION p1(4),p2(4),pp(4)
      DO k=1,4
        pp(k)=p1(k)+p2(k)
      ENDDO
      qadra=dmas2(pp)
      END

      SUBROUTINE cleanup
!     ******************
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)   
      COMMON / MOMSET / QEFF1(4),QEFF2(4),SPHUM(4),SPHOT(100,4),NPHOT 
      COMMON / MOMDEC / Q1(4),Q2(4),P1(4),P2(4),P3(4),P4(4)

      DO i=1,4
        DO j=1,nphot
          SPHOT(j,i)=0d0
        ENDDO
        sphum(i)=0d0
        qeff1(i)=0d0
        qeff2(i)=0d0
        q1(i)=0d0
        q2(i)=0d0
        p1(i)=0d0
        p2(i)=0d0
        p3(i)=0d0
        p4(i)=0d0
      ENDDO
      nphot=0

      END

      SUBROUTINE yfs_tests(mode,amel,ene,idyfs,wtves,xcvesk,wt1,wt2,wt3)
!     ******************************************************************
! yfs related tests
! WARNING these tests use random numbers generator. 
! That means they will alter the series.
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)  
      COMMON / matpar / pi,ceuler
      COMMON / phypar / alfinv,gpicob
 
      COMMON / bxfmts / bxope,bxclo,bxtxt,bxl1i,bxl1f,bxl2f,bxl1g,bxl2g
      CHARACTER*80      bxope,bxclo,bxtxt,bxl1i,bxl1f,bxl2f,bxl1g,bxl2g

* communicates with vesko/rhosko
      COMMON / vvrec  / vvmin,vvmax,vv,beti

      EXTERNAL rhosko

      COMMON / inout  / ninp,nout

      DIMENSION drvec(100)

      SAVE

* ============
      IF(mode .EQ. -1) THEN
* ============

        svar=4*ene**2

        beti = 2d0/alfinv/pi*(dlog(4d0*ene**2/amel**2)-1d0)
        beti2= 2d0/alfinv/pi* dlog(4d0*ene**2/amel**2)
        gamfap =1d0-pi**2*beti**2/12d0
        gamfac =exp(-ceuler*beti)/dpgamm(1d0+beti)
        gamfa2 =exp(-ceuler*beti2)/dpgamm(1d0+beti2)

        prec = 1d-7
        xcgaus =bremkf(1,prec)
        xdel = xcvesk/xcgaus-1
        WRITE(nout,bxtxt) '  vesko initialisation report'
        WRITE(nout,bxtxt) '          mode -1            '
        WRITE(nout,bxl1f) xcvesk,'approx xs_crude  vesko','xcvesk','v1'
        WRITE(nout,bxl1f) xcgaus,'exact  xs_crude  gauss','xcgaus','v2'
        WRITE(nout,bxl1f) xdel   ,'xcvesk/xcgaus-1      ','      ','v3'
        WRITE(nout,bxtxt) 'initialize karlud  END '
        WRITE(nout,bxclo)

*-- initialize control histos for yfsgen
        DO k=51,57
          CALL gmonit(-1,idyfs+k,0d0,1d0,1d0)
        ENDDO

* ============
      ELSEIF(mode .EQ. 0) THEN
* ============

         ref  = vvrho(50,svar,amel,vv,vvmin)
         wtr  = ref/vvrho(1,svar,amel,vv,vvmin)
         CALL varran(drvec,1)
         CALL gmonit(0,idyfs+56,wtr,1d0,drvec(1))
* pseudorejection in order to introduce reference xsection
         IF(drvec(1) .GT. wtr) GOTO 110
         wf1  = wt1*vvrho(51,svar,amel,vv,vvmin)/ref
         wf2  = wt2*vvrho(52,svar,amel,vv,vvmin)/ref
         wf3  = wt3
         wf13 = wf1*wf3
         wf123= wf1*wf2*wf3
         CALL gmonit(0,idyfs+51,wf1,  1d0,1d0)
         CALL gmonit(0,idyfs+52,wf2,  1d0,1d0)
         CALL gmonit(0,idyfs+53,wf3,  1d0,1d0)
         CALL gmonit(0,idyfs+54,wf13, 1d0,1d0)
         CALL gmonit(0,idyfs+55,wf123,1d0,1d0)
 110     CONTINUE
         CALL gmonit(0,idyfs+57,wtves,  1d0,1d0)

* ============
      ELSEIF(mode .EQ. 1) THEN
* ============

         CALL vesk1w( 1,rhosko,xsve,erelve,xcves)
         CALL gmonit(1,idyfs+57,wtnoss,ernoss,dumm3)
         prec   = 1d-7
         xsgs   = bremkf(1,prec)
         ergs   = xsgs*prec
         erve   = xsve*erelve
         ddv    = xsve/xsgs-1d0
         ddr    = erelve + 1d-6
         xdel   = xcves/xsgs-1
         WRITE(nout,bxope)
         WRITE(nout,bxtxt) '            window v           '
         WRITE(nout,bxtxt) '       vesko final report      '
         WRITE(nout,bxtxt) '             mode  1           '
        WRITE(nout,bxl1f)xcves ,   'approx xs_crude vesko','xcvesk','v4'
        WRITE(nout,bxl2f)xsve,erve,'exact  xs_crude vesko','xsve  ','v5'
        WRITE(nout,bxl2f)xsgs,ergs,'exact  xs_crude gauss','xsgs  ','v6'
        WRITE(nout,bxl1f) xdel    ,'xcvesk_appr/xsgs-1   ','      ','v7'
        WRITE(nout,bxl2f)ddv,ddr,  ' xsve_exact/xsgs-1   ','      ','v8'
         WRITE(nout,bxclo)
* ============
      ELSE
* ============

      CALL gmonit(1,idyfs+51,del1,dwt1,parm3)
      del1   = del1-1d0
      CALL gmonit(1,idyfs+52,awf2,dwt2,parm3)
      CALL gmonit(1,idyfs+53,awf3,dwt3,parm3)
      del3   = awf3-gamfa2
      CALL gmonit(1,idyfs+54,awf4,dwt4,parm3)
      del4   = awf4-gamfac
      WRITE(nout,bxope)
      WRITE(nout,bxtxt) '     karlud  final  report     '
      WRITE(nout,bxtxt) '         window b              '
      WRITE(nout,bxl2f) del1,dwt1,  '<wf1>-1  mass wt   ','del1  ','b1'
      WRITE(nout,bxl2f) awf2,dwt2,  '<wf2> dilat. weight','awf2  ','b2'
      WRITE(nout,bxl2f) awf3,dwt3,  '<wf3> dilat. weight','awf3  ','b3'
      WRITE(nout,bxl2f) del3,dwt3,  '<wf3>-ygf(beti2)   ','del3  ','b4'
      WRITE(nout,bxl2f) awf4,dwt4,  '<wf1*wf3>          ','awf4  ','b5'
      WRITE(nout,bxl2f) del4,dwt4,  '<wf1*wf3>-ygf(beti)','del4  ','b6'
      WRITE(nout,bxclo)
*     ==================================================================
      CALL gmonit(1,idyfs+59,wtkarl,erkarl,parm3)
      CALL vesk1w( 1,rhosko,xsve,erelve,xcves)
      xskr=xcves*wtkarl
      CALL gmonit(1,idyfs+55,awf5,dwt5,parm3)
      del5   = awf5-gamfac
      CALL gmonit(1,idyfs+56,awf6,parm2,parm3)
      prec = 1d-6
      xrefer = bremkf(50,prec)
      delkar = xrefer*awf5/xskr  -1d0
      delref = xcves*awf6/xrefer-1d0
      WRITE(nout,bxope)
      WRITE(nout,bxtxt) '     karlud  final  report cont.   '
      WRITE(nout,bxtxt) '         window c                  '
      WRITE(nout,bxtxt) 'beti= 2*alfa/pi*(log(s/mel**2)-1)       '
      WRITE(nout,bxtxt) 'gamfap= 1-pi**2*beti**2/12              '
      WRITE(nout,bxtxt) 'gamfac=exp(-ceuler*beti)/gamma(1+beti)  '
      WRITE(nout,bxtxt) 'gamfa2=exp(-ceuler*beti2)/gamma(1+beti2)'
      WRITE(nout,bxl1f)  beti,        '                =','beti  ','c1'
      WRITE(nout,bxl1f)  gamfap,      '                =','gamfap','c2'
      WRITE(nout,bxl1f)  gamfac,      '                =','gamfac','c3'
      WRITE(nout,bxl1f)  gamfa2,      '                =','gamfa2','c4'
      WRITE(nout,bxl2f) awf5,dwt5, ' <wf1*wf3*wf4>      ','awf5  ','c5'
      WRITE(nout,bxl2f) del5,dwt5, ' <wf1*wf3>-ygf(beti)','del5  ','c6'
      WRITE(nout,bxtxt) 'delkar=xrefer*aver(wf1*wf1*wf3)/xskarl-1'
      WRITE(nout,bxtxt) 'delref=xcrude*aver(wtr)/xrefer-1        '
      WRITE(nout,bxl1f) xrefer,    'reference x_sect.   ','xrefer','c7'
      WRITE(nout,bxl1f) delkar,    'xrefer*awf5/xskr  -1','delkar','c8'
      WRITE(nout,bxl1f) delref,    'xcvesk*awf6/xrefer-1','delref','c9'
      WRITE(nout,bxclo)

* ============
      ENDIF
* ============

      END

