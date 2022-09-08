      SUBROUTINE decay(sprim,rndm,label)
****************************************
! this is 'buffor' routine that calls make_decay and then 
! fills up necessary commons for decay state
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      COMMON / decdat / amafin(20), br(20)
! Codes, br ratios and masses for final particles 
      COMMON / decays / iflav(4), amdec(4) 
      SAVE / decays /,/ decdat /
      SAVE

!!!!! M.Verzocchi bug fix !!!!!!!!!!!!!!!!!
cccc Include KW.inc to access m_Key4f......
      INCLUDE   'KW.inc'
!!!!! M.Verzocchi bug fix end !!!!!!!!!!!!!!!!!

      DIMENSION ipdg(4)
      CHARACTER*3 chuman(4)

! actual generation of final state type
      CALL make_decay(label,sprim,rndm)

! Assigning masses and codes to final particles according to 
! PDG coding convention (1992)
      CALL linear_to_pdg_label(1,label,ipdg,chuman)
      DO i=1,4
        iflav(i) = ipdg(i)
        amdec(i) = amafin( abs(iflav(i)))
      ENDDO

!!!!! M.Verzocchi bug fix !!!!!!!!!!!!!!!!!
        if (m_Key4f.eq.0) then
c CC03 matrix element. For the ussu, ubbu, cddc and cbbc
c final states assign correctely the quarks to the W bosons
c and don't make a ZZ final state for which anyhow the
c matrix element is computed as being WW. This is simply
c realised by swapping particles 2 and 4.
         if (label.eq.105.or.label.eq.115.or.
     +       label.eq.127.or.label.eq.129) then
          iflav2=iflav(2)
          amdec2=amdec(2)
          iflav(2)=iflav(4)
          amdec(2)=amdec(4)
          iflav(4)=iflav2
          amdec(4)=amdec2
         endif
        endif
!!!!! M.Verzocchi bug fix end!!!!!!!!!!!!!!!!!

      END

      FUNCTION get_decay_prob(sprim,label)
************************************************************
c gets the decay channel probability factor
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      INCLUDE 'decay.inc'

! we take value from nearest smaller energy. NO interpolation so far.
      estep=(emax-emin)/(max_e_bin-1)
      ielower=int( (dsqrt(sprim)-emin)/estep ) +1
      gdp=prob_chan(ielower,label)

      get_decay_prob=gdp

c      write(6,*)'get_decay_prob=',gdp

      END

      FUNCTION get_total_crude(sprim)
************************************************************
c gets the total (summed over channels) crude photonic distribution
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      INCLUDE 'decay.inc'

! we take value from nearest smaller energy. NO interpolation so far.
      estep=(emax-emin)/(max_e_bin-1)
      ielower=int( (dsqrt(sprim)-emin)/estep ) +1
      gtc=prob_e_total(ielower)

      get_total_crude=gtc

      END

      SUBROUTINE make_decay(label,sprim,rndm)
************************************************************
c chooses the decay channel
      IMPLICIT DOUBLE PRECISION (a-h,o-z)

      INCLUDE 'decay.inc'
!

! we take value from nearest smaller energy. NO interpolation so far.
      estep=(emax-emin)/(max_e_bin-1)
      ielower=int( (dsqrt(sprim)-emin)/estep ) +1
! highly inefficient, to be optimized later, 
      DO ilab=1,max_label
        IF (rndm .LE. prob_chan_cumul(ielower,ilab)) GOTO 12
      ENDDO
      WRITE(6,*)'make_decay=>sth. is wrong'
      STOP
 12   CONTINUE

c      if(ilab.gt.1) then
c      write(6,*)'prob_chan_cumul=',
c     $   prob_chan_cumul(ielower,ilab)-prob_chan_cumul(ielower,ilab-1)
c      else
c      write(6,*)'prob_chan_cumul=',
c     $   prob_chan_cumul(ielower,ilab)
c      endif

      label=ilab

      END

      SUBROUTINE linear_to_pdg_label(mode,label,ipdg,chuman)
********************************
! converts linear labels to pdg and human conventions (mode=1)
! DOES NOT work back (yet) but it would be a fairly good tool
! 1-81: WW Wp=1:1-9; 2:10-18..
! 82-202: ZZ Z1=1:82-92; 2:93-103..

      IMPLICIT DOUBLE PRECISION (a-h,o-z)

      DIMENSION ipdg(4)
      DIMENSION icod (20)
      DIMENSION icodz(20)
      CHARACTER*3 chuman(4)
      CHARACTER*2 clabl(20)

      SAVE

      DATA init /0/

      IF(mode.NE.1) THEN
        WRITE(6,*)'linear_to_pdg_label=> only mode=1 implemented'
        STOP
      ENDIF

      IF(init.EQ.0) THEN
        init=1
! Codes for WW final state flavors
        icod(1)=102
        icod(2)=104
        icod(3)=302
        icod(4)=304
        icod(5)=502
        icod(6)=504
        icod(7)=1112
        icod(8)=1314
        icod(9)=1516
! Codes for ZZ final state flavors
        icodz(1)=101
        icodz(2)=202
        icodz(3)=303
        icodz(4)=404
        icodz(5)=505
        icodz(6)=1111
        icodz(7)=1313
        icodz(8)=1515
        icodz(9)=1212
        icodz(10)=1414
        icodz(11)=1616

! human readable
        clabl(1) ='dq'
        clabl(2) ='uq'
        clabl(3) ='sq'
        clabl(4) ='cq'
        clabl(5) ='bq'
        clabl(6) ='tq'

        clabl(11) ='el'
        clabl(12) ='ne'
        clabl(13) ='mu'
        clabl(14) ='nm'
        clabl(15) ='ta'
        clabl(16) ='nt'

      ENDIF


      CALL linear_to_WZ_label(1,label,icwm,icwp,if_z,if_w)

! Assigning codes to final particles according to PDG
! coding convention (1992)
      IF(if_w .EQ. 1) THEN
!-- WW
          ipdg(1)= icod(icwm)/100
          ipdg(2)=-mod(icod(icwm),100)
          ipdg(3)= mod(icod(icwp),100)
          ipdg(4)=-icod(icwp)/100
      ELSEIF(if_z .EQ. 1) THEN
!-- ZZ
          ipdg(1)= icodz(icwm)/100
          ipdg(2)=-mod(icodz(icwm),100)
          ipdg(3)= mod(icodz(icwp),100)
          ipdg(4)=-icodz(icwp)/100
      ENDIF

! Assigning human labels
      DO i=1,4
        IF(ipdg(i).LT.0) THEN
          chuman(i)= '~'//clabl(abs(ipdg(i)))
        ELSE
          chuman(i)= ' '//clabl(abs(ipdg(i)))
        ENDIF
      ENDDO

      END




      SUBROUTINE linear_to_WZ_label(mode,label,icwm,icwp,ifznow,ifwnow)
********************************
! converts linear labels to KoralW.13x convention (mode=1) 
! and back (mode=-1) 
! 1-81: WW Wp=1:1-9; 2:10-18..
! 82-202: ZZ Z1=1:82-92; 2:93-103..

      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      IF(mode .EQ. 1) THEN
!--======================= 
!-- prepare W/Z type labelling
        IF(label .LE. 81) THEN
!-- WW
          ifwnow=1
          ifznow=0
          icwp=(label-1)/9 +1
          icwm=mod(label-1,9)+1
        ELSEIF(label .LE. 202) THEN
!-- ZZ
          ifznow=1
          ifwnow=0
          icwp=(label-81-1)/11 +1
          icwm=mod((label-81-1),11)+1
        ELSE
          WRITE(6,*)'linear_to_WZ_label=> stop 1',1/(mode-1)
          STOP
        ENDIF
      ELSEIF(mode .EQ. -1) THEN
!--======================= 
!-- prepare linear labelling
        IF(ifwnow .EQ. 1) THEN
          label=(icwp-1)*9+icwm
        ELSEIF(ifznow .EQ. 1) THEN
          label=81+(icwp-1)*11 +icwm
        ELSE
          WRITE(6,*)'linear_to_WZ_label=> stop 2'
          STOP
        ENDIF
      ELSE
!-- ======================
        WRITE(6,*)'linear_to_WZ_label=> stop 3'
        STOP
      ENDIF
!-- ======================

      END

      SUBROUTINE umask_init(user_umask)
*******************************************************
! lblmin,lblmax give the range of channels to be generated
! umask_lbl(max_label) is a user mask for suppressing certain final states on
! request. only 0 or 1 are expected.
! matrices to be filled:
! prob_chan(ienergy,label)
!   each bin contains value from its beginning. last bin=smax
! prob_chan_cumul(ienergy,label) is cumulative in index label starting
! from label=1  

      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      INCLUDE 'decay.inc'
      DIMENSION user_umask(*)


! This common can be everywhere, contains various switches
      COMMON / KeyKey /  KeyRad,KeyPhy,KeyTek,KeyMis,KeyDwm,KeyDwp
! I/O unit numbers
      COMMON / INOUT  / NINP,NOUT     

      CHARACTER*3 chuman(4)

      DIMENSION iflav(4)

      KeyZon = MOD(KeyMis,10000)/1000
      KeyWon = MOD(KeyMis,100000)/10000

      DO i=1,max_label
        umask_lbl(i)=0d0
      ENDDO

      IF( keywon .EQ. 1 .AND.
     $    keyzon .EQ. 1 .AND.
     $    keydwm .EQ. 0 .AND.
     $    keydwp .EQ. 0       ) THEN
! read user's umask only here
        DO i=1,202
          umask_lbl(i)=user_umask(i)
        ENDDO

      ELSEIF( keywon .EQ. 1 .AND.
     $        keyzon .EQ. 0 .AND.
     $        keydwm .EQ. 0 .AND.
     $        keydwp .EQ. 0       ) THEN
! WW inclusive
        DO i=1,81
          umask_lbl(i)=1d0
        ENDDO
!... mix must be added in ZZ part, but only: ussu ubbu cddc cbbb
        DO label=82,202
          CALL linear_to_pdg_label(1,label,iflav,chuman)

          DO I=1,2
            IA=iflav(2*I-1)
            IB=iflav(5-2*I)

            IF( 
     $        (IA.eq. 3.and.IB.eq. 2).or.  ! s   u
     $        (IA.eq. 5.and.IB.eq. 2).or.  ! b   u
     $        (IA.eq. 1.and.IB.eq. 4).or.  ! d   c
     $        (IA.eq. 5.and.IB.eq. 4)      ! b   c
     $        )  THEN
!-- mix detected
              umask_lbl(label)=1d0
            ENDIF
          ENDDO
        ENDDO
! end mix-adding
!... mix must be added in ZZ part, but only: ussu ubbu cddc cbbb
      ELSEIF( keywon .EQ. 0 .AND.
     $        keyzon .EQ. 1 .AND.
     $        keydwm .EQ. 0 .AND.
     $        keydwp .EQ. 0       ) THEN
! ZZ inclusive
        DO i=82,202
          umask_lbl(i)=1d0
        ENDDO
!... mix must be added in WW part, but only: uudd, ccss, llll
        DO ibra=7,9 ! leptons
          CALL linear_to_WZ_label(-1,label_mix,ibra,ibra,0,1)
          umask_lbl(label_mix)=1d0
        ENDDO
        ibra=1 ! uddu
        CALL linear_to_WZ_label(-1,label_mix,ibra,ibra,0,1)
        umask_lbl(label_mix)=1d0
        ibra=4 ! cssc
        CALL linear_to_WZ_label(-1,label_mix,ibra,ibra,0,1)
        umask_lbl(label_mix)=1d0
      ELSEIF( keywon .EQ. 1 .AND.
     $        keyzon .EQ. 0 .AND.
     $        keydwm .NE. 0 .AND.
     $        keydwp .NE. 0       ) THEN
! WW exclusive
          CALL linear_to_WZ_label(-1,label,keydwm,keydwp,keyzon,keywon)
          umask_lbl(label)=1d0
      ELSEIF( keywon .EQ. 0 .AND.
     $        keyzon .EQ. 1 .AND.
     $        keydwm .NE. 0 .AND.
     $        keydwp .NE. 0       ) THEN
! ZZ exclusive
          CALL linear_to_WZ_label(-1,label,keydwm,keydwp,keyzon,keywon)
          umask_lbl(label)=1d0
      ELSE
        WRITE(6,*)'umask_init=> do not know this flavor setting'
        STOP
      ENDIF

! now we kill the mix-type final states in ZZ configuration
      DO label=82,202
        CALL linear_to_pdg_label(1,label,iflav,chuman)

        DO I=1,2
          IA=iflav(2*I-1)
          IB=iflav(5-2*I)

          IF( 
     $        (IA.eq.11 .and. IB.eq.12) .or.  ! e   nue
     $        (IA.eq.13 .and. IB.eq.14) .or.  ! mu  numu
     $        (IA.eq.15 .and. IB.eq.16) .or.  ! tau nutau
     $        (IA.eq. 1 .and. IB.eq. 2) .or.  ! d   u
     $        (IA.eq. 3 .and. IB.eq. 4)       ! s   c
     $      )  THEN
!-- mix detected
            umask_lbl(label)=0d0
          ENDIF
        ENDDO
      ENDDO
! ...and in WW configurations...
      DO ibra=2,3 ! leptons
        CALL linear_to_WZ_label(-1,label_mix,ibra,ibra,0,1)
        umask_lbl(label_mix)=0d0
      ENDDO
      DO ibra=5,6 ! leptons
        CALL linear_to_WZ_label(-1,label_mix,ibra,ibra,0,1)
        umask_lbl(label_mix)=0d0
      ENDDO
! end mix-killing

! additional stuff from selecto !

* kill DOUBLE counting in different zz final states.
      DO label=82,202
        CALL linear_to_WZ_label(1,label,iwm,iwp,if_z,if_w)
        IF(iwm .GT. iwp) umask_lbl(label)=0d0
      ENDDO

* remove e+e-e+e- final state.  this is desactivated !!!!!
* it may be useful. will SAVE a lot of cpu time IF you DO not need
* these states.
*     CALL linear_to_WZ_label(-1,label,6,6,1,0)
*     umask_lbl(label)=0d0

! end selecto stuff !

! dump umask for control
      WRITE(6,*)'umask_init=>umask:'
      DO i=1,9
        WRITE(6,'(9f5.1)') (umask_lbl((i-1)*9+j),j=1,9)
      ENDDO
      DO i=1,11
        WRITE(6,'(11f5.1)') (umask_lbl(81+(i-1)*11+j),j=1,11)
      ENDDO
!!!      OPEN(unit=9,file='umask_actual')
      WRITE(nout,*)'umask_init=>umask:'
      DO i=1,9  
        WRITE(nout,'(9f5.1)') (umask_lbl((i-1)*9+j),j=1,9)
      ENDDO
      DO i=1,11
        WRITE(nout,'(11f5.1)') (umask_lbl(81+(i-1)*11+j),j=1,11)
      ENDDO
!!!      CLOSE(9)
! Check if open at least one channel
      mark=0
      DO i=1,202
        IF(umask_lbl(i).NE.0d0) mark=1
      ENDDO
      IF(mark.EQ.0) THEN
        WRITE(6,*)'umask_init=> There are no opened decay channels'
        WRITE(6,*)'umask_init=> Check your input cards, STOP'
        STOP  
      ENDIF

      END

      SUBROUTINE decay_prob_init(emi,ema,user_umask,
     @                           i_file,KeyWgt,keysmp)
******************************************************************
! umask_lbl(max_label) is a user mask for suppressing certain final states on
! request. only 0 or 1 are expected.
! matrices to be filled:
! prob_chan(ienergy,label)
!   each bin contains value from its beginning. last bin=emax
! prob_chan_cumul(ienergy,label) is cumulative in index label starting
! from label=1  
! this routine generates probabilities from the 
! -analytical function  phot_spec_crud (i_file=0)
! -pretabulated data (i_file=1)
!    (keywgt<>0 -pretabulated xsections)
!    (keywgt =0 -pretabulated wtmax)

      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      INCLUDE 'decay.inc'
      DIMENSION user_umask(202)

! fill in smin and smax
      emin=emi
      emax=ema
! read umask_lbl
      CALL umask_init(user_umask)

      estep=(emax-emin)/(max_e_bin-1)
      eloc=emin
      sloc=emin**2
      smax=emax**2
      DO iebin=1,max_e_bin
        prob_e_total(iebin)=0d0
        DO label=1,max_label 
          prob_chan(iebin,label) =
     $      give_phot_spec_crud(smax,sloc,label,i_file,KeyWgt,keysmp)
     $                           *umask_lbl(label)
          prob_e_total(iebin) =prob_e_total(iebin)
     $                        +prob_chan(iebin,label)
        ENDDO
        eloc=eloc+estep
        sloc=eloc**2
      ENDDO
c      write(6,*)'prob_e_total',prob_e_total
c      write(6,*)'prob_e_total',i_file,keywgt
c      write(6,*)'prob_e_total',umask_lbl
c      write(6,*)'eloc,estep,emax,emin',eloc,estep,ema,emi,max_e_bin
      IF( abs( (eloc-estep)/emax-1 ) .GT. 1d-8 ) THEN
        WRITE(6,*)'decay_prob_init=> sth. is wrong'
        STOP
      ENDIF

      DO iebin=1,max_e_bin
        prcum=0d0
        DO label=1,max_label
          prob_chan(iebin,label) =prob_chan(iebin,label)
     $                           /prob_e_total(iebin)
          prcum=prcum+prob_chan(iebin,label)
          prob_chan_cumul(iebin,label)=prcum
        ENDDO
        IF( abs( prcum-1 ) .GT. 1d-12 ) THEN
          WRITE(6,*)'decay_prob_init=> 2 sth. is wrong'
          STOP
        ENDIF
      ENDDO

      WRITE(6,*)'decay_prob_init=> photonic pretabulation done'

      END

      FUNCTION give_phot_spec_crud(svar,sprim,
     @                             label,i_file,KeyWgt,keysmp)
******************************************************************
! provides crude photonic spectrum either from 
! pretabulated file (i_file=1),     or from
!    (keywgt<>0 -pretabulated xsections)
!    (keywgt =0 -pretabulated wtmax)
! analytic function (i_file=0) as of v. 1.33

      IMPLICIT DOUBLE PRECISION (a-h,o-z)

      CHARACTER*250       path
      COMMON / datapath / path

      INCLUDE 'decay.inc'
      PARAMETER (n_ene=100) ! number of actual energy bins in file
                            ! do not confuse with internal binning
      DIMENSION xsec_lcl(n_ene,max_label)
      DIMENSION wtma_lcl(n_ene,max_label)
      CHARACTER*80 name_wt, name_xs
      CHARACTER*250 name_full_wt, name_full_xs

      DATA init /0/

      SAVE

      IF(init .EQ. 0 .AND. i_file .EQ. 1) THEN
        DO ilbl=1,max_label
          DO iene=1,n_ene
            xsec_lcl(iene,ilbl)=1
          ENDDO
        ENDDO
        init=1
c        IF(KeyWgt .EQ. 0) THEN
        IF(KeySmp .EQ. 1) THEN
          name_wt='data_wtmax.fit.smp1'
          name_xs='data_xsect.fit'
! tobedone        ELSEIF(KeySmp .EQ. 3) THEN
! tobedone          name_wt='data_wtmax.fit.smp3'
! tobedone          name_xs='data_xsect.fit'
        ELSE
          name_wt='data_wtmax.fit.smp2'
          name_xs='data_xsect.fit'
        ENDIF

! read wtmax
        name_full_wt = TRIM(path) // name_wt
        name_full_xs = TRIM(path) // name_xs

        CALL open_data(name_full_wt,name_wt,io_number)
        DO ilbl=1,max_label
          IF(umask_lbl(ilbl) .NE. 0d0) THEN
ccc            write(6,*)'ddd',umask_lbl(ilbl),ilbl
 200        CONTINUE
            READ(io_number,*) lbl_lcl,eminx,emaxx
            READ(io_number,*) ( wtma_lcl(iene,ilbl), iene=1,n_ene )
            IF(ilbl .GT. lbl_lcl) THEN
CC              WRITE(6,*)'give_phot_spec_crud=>wt skipped label ',lbl_lcl
              GOTO 200
            ELSEIF(ilbl .LT. lbl_lcl) THEN
CC              WRITE(6,*)'give_phot_spec_cr=>wt not found label ',ilbl
              STOP
            ENDIF
          ENDIF
        ENDDO
        CALL close_data(name_full_wt,name_wt,io_number)

CC        WRITE(6,*)' '

! read xsectn
        CALL open_data(name_full_xs,name_xs,io_number)
        DO ilbl=1,max_label
          IF(umask_lbl(ilbl) .NE. 0d0) THEN
ccc            write(6,*)'ddd',umask_lbl(ilbl),ilbl
 201         CONTINUE
            READ(io_number,*) lbl_lcl,eminx,emaxx
            READ(io_number,*) ( xsec_lcl(iene,ilbl), iene=1,n_ene )
            IF(ilbl .GT. lbl_lcl) THEN
CC              WRITE(6,*)'give_phot_spec_crud=>xs skipped label ',lbl_lcl
              GOTO 201
            ELSEIF(ilbl .LT. lbl_lcl) THEN
CC              WRITE(6,*)'give_phot_spec_cr=>xs not found label ',ilbl
              STOP
            ENDIF
          ENDIF
        ENDDO
        CALL close_data(name_full_xs,name_xs,io_number)

        DO ilbl=1,max_label
          IF(umask_lbl(ilbl) .NE. 0d0) THEN
            DO iene=1,n_ene
c              xsec_lcl(iene,ilbl)=dsqrt( 
c     #                  xsec_lcl(iene,ilbl)*wtma_lcl(iene,ilbl) )
c              xsec_lcl(iene,ilbl)=1/2d0*( 
c     #                  xsec_lcl(iene,ilbl)+wtma_lcl(iene,ilbl) )
              IF(KeySmp .EQ. 0) THEN
                xsec_lcl(iene,ilbl)=
     #                  xsec_lcl(iene,ilbl)
              ELSE
                xsec_lcl(iene,ilbl)=
     #                  wtma_lcl(iene,ilbl) 
              ENDIF
            ENDDO
          ENDIF
        ENDDO

        ebinx= (emaxx-eminx)/(n_ene-1)

      ENDIF

      IF(i_file .EQ. 0) THEN
        g_p_s_c=phot_spec_crud(svar,sprim,label)
      ELSEIF(i_file .EQ. 1) THEN
        ie_lcl=nint((dsqrt(sprim)-eminx)/ebinx)
        IF(ie_lcl .GT. n_ene) ie_lcl=n_ene
        IF(ie_lcl .LT. 1) ie_lcl=1
        g_p_s_c=xsec_lcl(ie_lcl,label)
      ENDIF

        give_phot_spec_crud=g_p_s_c

      END

      SUBROUTINE decay_monit(mode,wtmod,crud,svar,label,nout)
************************************************
! bookkeeping routine for monitoringg xsections in different flavor
! decay channels, used also to pre-tabulate the photonic spectra
! label_dumm later on to be converted into real label (when relocated
! to karludw. for now label comes from get_label routine
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      INCLUDE 'decay.inc'

      PARAMETER (label_max=202)
      DIMENSION num_phot(label_max)
      DIMENSION num_phot_non0(label_max)
      DIMENSION wgt_phot(label_max)
      DIMENSION wgt2_phot(label_max)
      DIMENSION wt_biggest(label_max)
      DIMENSION wt_smallest(label_max)
      DIMENSION ipdg(4)
      REAL*4 rewt_biggest(label_max)
      REAL*4 r0wt_biggest(label_max)
      REAL*4 rewgt_phot(label_max)
      CHARACTER*3 chuman(4)
! wtover is inactive until wtmax is settled
!      DIMENSION wgt_over(label_max)
      SAVE

!------------------------------
      IF(mode .EQ. -1) THEN
!------------------------------
        DO i=1,label_max
          num_phot(i)=0
          num_phot_non0(i)=0
          wgt_phot(i)=0d0
          wgt2_phot(i)=0d0
          wt_biggest(i)=0d0
        ENDDO
        ntot=0
!------------------------------
      ELSEIF(mode .EQ. 0) THEN
!------------------------------
        IF(label .GT. label_max) THEN
          WRITE(6,*)'phot_spec_book=> label_max too small:',label_max
          STOP
        ENDIF
        ntot=ntot+1
        IF( wtmod .GT. 0d0 ) 
     $     num_phot_non0(label) =num_phot_non0(label)+1
        num_phot(label) =num_phot(label)+1
        wgt_phot(label) =wgt_phot(label)+wtmod
        wgt2_phot(label)=wgt2_phot(label)+wtmod**2
        IF( wt_biggest(label).LT.wtmod ) wt_biggest(label)=wtmod
        IF( wt_smallest(label).GT.wtmod ) wt_smallest(label)=wtmod
!------------------------------
      ELSEIF(mode .EQ. 1) THEN
!------------------------------
! normalisation
c        DO i=1,label_max
c          wgt_phot(i ) =wgt_phot(i )*crud
c          wgt2_phot(i )=wgt2_phot(i )*crud**2
c          wt_biggest(i )=wt_biggest(i )*crud
c          wt_smallest(i )=wt_smallest(i )*crud
c        ENDDO
! printout 
        WRITE(nout,*)
        WRITE(nout,'(80a)') ' ',('*',iii=1,75)
        WRITE(nout,*)'                     '
     $      //'Decay Report on Different Channels '
        WRITE(nout,*)
        WRITE(nout,'(80a)')
     $    '                                            '
     $    //' wt_max  wt_max  nev_ch nev_non0'
        WRITE(nout,'(80a)')
     $    '  wm wp        human   sigma [pb] +- abs_err'
     $    //' ------ -------- ------ --------'
        WRITE(nout,'(80a)')
     $    '                                            '
     $    //'  <wt>  <wt_non0>   tot   nev_ch'
        WRITE(nout,*)
        xstot=0d0
        xstot2=0d0
        DO i=1,label_max
          IF( umask_lbl(i).GT.0d0 ) THEN
            xstot=xstot+wgt_phot(i)*crud
            xstot2=xstot2+wgt2_phot(i)*crud**2
            wgt_ph=wgt_phot(i)*crud/ntot
            wgt2_ph=wgt2_phot(i)*crud**2/ntot
            wterro=dsqrt((wgt2_ph -wgt_ph**2)/ntot)
            procevt=dble(num_phot(i))/dble(ntot)
c            procevt0=dble(num_phot_non0(i))/dble(ntot)
            IF( num_phot(i) .GT. 0d0) THEN
              procevt0=dble(num_phot_non0(i))/dble(num_phot(i))
            ELSE
              procevt0=0d0
            ENDIF
            IF(wgt_ph.GT.0d0) THEN
              wtma=wt_biggest(i)/wgt_phot(i) *num_phot(i)
              wtma_non0=wt_biggest(i)/wgt_phot(i) *num_phot_non0(i)
            ENDIF

            CALL linear_to_WZ_label(1,i,kwm,kwp,if_z,if_w)
            CALL linear_to_pdg_label(1,i,ipdg,chuman)

       WRITE(nout,
     $ '(a,2i3,a,4a3,a,e13.7,a,e8.2,a,e8.2,a,e8.2,a,f6.4,a,f6.4)')
     $        ' ',kwm,kwp,' ',
     $        chuman(1),chuman(2),chuman(3),chuman(4),
     $        ' ',real(wgt_ph),'+-',real(wterro),
     $        ' ',real(wtma),' ',real(wtma_non0),
     $        ' ',real(procevt),
     $        ' ',real(procevt0)
            WRITE(nout,*)' '
         ENDIF
        ENDDO
        xstot=xstot/ntot
        xstot2=xstot2/ntot
        ertot=dsqrt((xstot2 -xstot**2)/ntot)
        WRITE(nout,*)'   total xsection = ',xstot,' +- ',ertot,' [pb]'
        WRITE(nout,'(80a)') ' ',('*',iii=1,75)
        WRITE(nout,*)' '
!------------------------------
      ELSEIF(mode .EQ. 2) THEN
!------------------------------
! printout for pre-tabulation
        DO i=1,label_max
          rewgt_phot(i)=wgt_phot(i)*crud/ntot
!          procevt=dble(num_phot(i))/dble(ntot)
          IF(wgt_phot(i).GT.0d0) THEN
            r0wt_biggest(i)=wt_biggest(i)/wgt_phot(i) *num_phot_non0(i)
            rewt_biggest(i)=wt_biggest(i)/wgt_phot(i) *num_phot(i)
          ENDIF
        ENDDO
        OPEN(unit=19,file='spectrum',status='unknown')
        WRITE(19,*)svar,'  svar point'
        WRITE(19,*)rewgt_phot
        CLOSE(19)
        OPEN(unit=19,file='wtmax.non0',status='unknown')
        WRITE(19,*)svar,'  svar point'
        WRITE(19,*)r0wt_biggest
        CLOSE(19)
        OPEN(unit=19,file='wtmax',status='unknown')
        WRITE(19,*)svar,'  svar point'
        WRITE(19,*)rewt_biggest
        CLOSE(19)
!------------------------------
      ENDIF
!------------------------------

      END

      SUBROUTINE store_label(mode,label)
****************************************
! tempotary routine to carry label from karludw to koralw and wwborn 
! for the phot_spec_book and born only !!!!!
! later on I hope external matrix el. calculation will migrate
! to karludw also and then problem of label will disappear.
!
! NOT ANY MORE a temporary routine. Now it is used by few routines, in
! the place of / WorZ / common, M.S. 3/3/98, Knox. 
!
! mode 0=store
!      1=retrieve

      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      SAVE ilabel

      IF(mode .EQ. 0) THEN
        ilabel=label
      ELSEIF(mode .EQ. 1) THEN
        label=ilabel
      ELSE
        WRITE(6,*) 'store_label=>wrong mode ',mode
      ENDIF

      END
