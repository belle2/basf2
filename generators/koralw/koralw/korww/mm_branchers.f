      subroutine mm_brancher(sprim,itype,prob)
!     ******************************************************    
! ###########################################
! sets probablilities of channels, can be   #
! function of sprim and iflav(4)            #
! ###########################################
      implicit DOUBLE PRECISION (a-h,o-z)

      PARAMETER (mm_nrchan=65)
      DIMENSION prob(mm_nrchan)

      SAVE

! common / WorZ / is replaced by  store_label  routine !!!!!!
! convert linear labels to KoralW.13x convention (mode=1)
! these routines comes from the decay.f package !!! 
      CALL store_label(1,label)
      CALL linear_to_WZ_label(1,label,icwm,icwp,ifznow,ifwnow)

        IF(ifwnow.EQ.1) THEN
          CALL mm_brancher_WW(sprim,itype,prob)
        ELSE
          CALL mm_brancher_ZZ(sprim,itype,prob)
        ENDIF
      END


      subroutine mm_brancher_WW(sprim,itype,prob)
!     ******************************************************    
! ###########################################
! sets probablilities of channels, can be   #
! function of sprim and iflav(4)            #
! ###########################################
      implicit DOUBLE PRECISION (a-h,o-z)
      COMMON / DECAYS / IFLAV(4), AMDEC(4) 
      COMMON / INOUT  / NINP,NOUT 
      COMMON / BXFMTS / BXOPE,BXCLO,BXTXT,BXL1I,BXL1F,BXL2F,BXL1G,BXL2G
      CHARACTER*80      BXOPE,BXCLO,BXTXT,BXL1I,BXL1F,BXL2F,BXL1G,BXL2G
      CHARACTER*80      BX11F
! This common can be everywhere, contains various switches
      COMMON / KeyKey /  KeyRad,KeyPhy,KeyTek,KeyMis,KeyDwm,KeyDwp
      save / DECAYS /,/ INOUT  /,/ BXFMTS / 
      DIMENSION drvec(1)
      parameter (mm_nrchan=65)
      DIMENSION PROB(mm_nrchan)
      data istart /0/
      save
      BX11F =  '(1X,1H*,F17.8,               16X, A20,I12,A7, 1X,1H*)' 
! random choice of generation branch as function of sprime, and final state chosen.
! should be emptied from rn. generation to preserve generation series. 
! you can adopt your choice of the presampling type on sprim and iflav.
! may be one should coonect it with placer? For choice of presampling resonances?
        KeySmp = MOD(KeyTek,1000)/100
        IF (KeySmp.eq.0) THEN
          prob(1)= 1d0
          do i=2,mm_nrchan
            prob(i)=0d0
          enddo
        ELSE
          do i=1,7
            prob(i)=0d0
          enddo
ccc          prob(1)=1d-5
          do i=8,mm_nrchan
ccc            prob(i)=1.0d-5
            prob(i)=0d0
ccc            prob(i)=.02d0
          enddo
          prob(62)=0d0
          prob(63)=0d0
          prob(64)=0d0
          prob(65)=0d0


          IF(abs(iflav(1)).NE.11 .AND. abs(iflav(2)).NE.11 .AND. 
     @       abs(iflav(3)).NE.11 .AND. abs(iflav(4)).NE.11 ) THEN 
!  1-1 type channels
            prob(  1 )= 0.5d0  
            prob( 10 )= 0.1d0  
            prob( 13 )= 0.26d0
            prob( 14 )= 0.4d-1
            prob( 28 )= 0.2d-1
            prob( 38 )= 0.3d-1 !fsr .3d-1
            prob( 42 )= 0.2d-1
            prob( 45 )= 0.3d-1 !fsr .3d-1
            prob( 49 )= 0.2d-1
            prob( 50 )= 0.2d-1
            prob( 54 )= 0.2d-1
            prob( 61 )= 0.2d-1
          ELSEIF(abs(iflav(1)).EQ.11 .AND. abs(iflav(2)).NE.11 .AND. 
     @           abs(iflav(3)).NE.11 .AND. abs(iflav(4)).NE.11 ) THEN 
!  7-1 type channels (e,nu u,d)
            prob(  1 )= .6d0
            prob(  8 )= .1d0 !
            prob(  9 )= .3d-1 ! it was 46
            prob( 17 )= .2d0 
            prob( 18 )= .5d-1 !
            prob( 25 )= .1d0 
            prob( 37 )= .3d-1 !
            prob( 38 )= .3d-1 
            prob( 41 )= .3d-1 
            prob( 43 )= .2d0 !
            prob( 55 )= .1d0 
            prob( 58 )= .5d-1 
          ELSEIF(abs(iflav(1)).NE.11 .AND. abs(iflav(2)).NE.11 .AND. 
     @           abs(iflav(3)).NE.11 .AND. abs(iflav(4)).EQ.11 ) THEN 
! 1-7 type channels (u,d e,nu) 
            prob(  1 )= .6d0
            prob( 11 )= .1d0  !8
                              !9
            prob( 34 )= .2d0  !17
            prob( 33 )= .5d-1 !18
            prob( 26 )= .1d0  !25
            prob( 14 )= .3d-1 !37
                              !38
                              !41
            prob( 56 )= .2d0  !43
            prob( 44 )= .1d0  !55
                              !58
          ELSEIF(abs(iflav(1)).EQ.11 .AND. abs(iflav(2)).NE.11 .AND. 
     @           abs(iflav(3)).NE.11 .AND. abs(iflav(4)).EQ.11 ) THEN 
! 7-7 type channels (e,nu e,nu)


            prob(  1 )= .30d0
            prob( 10 )= 0.2d0  
            prob( 13 )= 0.2d0
            prob( 17 )= .5d-1 
            prob( 18 )= .8d-1 !
            prob( 33 )= .3d-1 !18
            prob( 34 )= .1d0  !17
            prob( 38 )= 0.1d0 !fsr .3d-1
            prob( 42 )= .1d0 !
            prob( 43 )= .1d0 !
!            prob( 44 )= .5d-1  !55
!            prob( 50 )= .6d-1 
            prob( 54 )= .1d0 
            prob( 56 )= .1d0  !43
            prob( 57 )= .6d-1 
            prob( 61 )= .1d0

       prob( 1)= .1528066401360303 !max= 5.126895446892595E-02
       prob( 10)= .123819037133896 !max= .4567310012216404
       prob( 13)= .13 !max= .4700347361740743
       prob( 17)= 2.401704334716281E-02 !max= .2876706027685168
       prob( 18)= 2.919633584784117E-02 !max= .1806467520754505
       prob( 33)= 2.875826008779435E-02 !max= .1422278918002859
       prob( 34)= 3.956069899541728E-02 !max= .3848683236527629
       prob( 38)= 5.530102479893887E-02 !max= .1747483992128647
       prob( 42)= 2.878073109354603E-02 !max= .3837148649525202
       prob( 43)= 6.778407127531376E-02 !max= 6.724114199142956E-02
       prob( 54)= 7.187468560675748E-02 !max= .7073283431479492
       prob( 56)= 8.208749745836423E-02 !max= .1134218362601451
       prob( 57)= 3.559681491376532E-02 !max= .1169604669046596
       prob( 61)= .175781241844806 !max= 1.0
          ELSE
            WRITE(6,*)'mm_brancher_WW=>unexpected iflav:',iflav
            STOP
          ENDIF
        ENDIF

!!!!!!!!!!!!!!!!!!!!!!!!!!!! danger !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!! danger !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!          prob(1)=1.0d-5
!          do i=8,mm_nrchan
!            prob(i)=1.0d-5
!          enddo
!          prob(62)=0d0
!          prob(63)=0d0
!!!!!!!!!!!!!!!!!!!!!!!!!!!! danger !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!! danger !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

        prtot=0d0
        DO I=1,mm_nrchan
          prtot=prtot+prob(I)
        ENDDO
        DO I=1,mm_nrchan  
          prob(I)=prob(I)/prtot
        ENDDO
!     ---------------------
      IF (istart.eq.0) THEN
!     ---------------------
        istart=1
        WRITE(NOUT,BXOPE) 
        WRITE(NOUT,BXTXT) '             Window X_WW                '
        WRITE(NOUT,BXTXT) '          mm_brancher_WW report         '
        if (KeySmp.eq.0) then
        WRITE(NOUT,BXTXT) ' WARNING: KeySmp =0 Brancher is off !   '
        else
        WRITE(NOUT,BXTXT) '          mm_brancher_WW  is on         '
        DO I=1,mm_nrchan  
          WRITE(NOUT,BX11F) prob(I),'prob. for branch NR: ',I,'X1'
        ENDDO
        endif
        WRITE(NOUT,BXCLO)         
!     -----
      ENDIF
!     -----

      if(Keysmp.eq.0) then
!     ====================
        itype=1
      else
!     ====================
        CALL varran(drvec,1)
        PROBI=0D0
        DO I=1,mm_nrchan
          PROBI=PROBI+PROB(I)
          if(drvec(1).lt.probI) THEN
            itype=I
            GOTO 10
           ENDIF
        enddo
        write(6,*) 
     $ 'mm_brancher_WW has problem prob=',prtot
        stop
 10     continue
      endif
!     =====================
      end 


      subroutine mm_brancher_ZZ(sprim,itype,prob)
!     ******************************************************    
! ###########################################
! sets probablilities of channels, can be   #
! function of sprim and iflav(4)            #
! ###########################################
      implicit DOUBLE PRECISION (a-h,o-z)
      COMMON / DECAYS / IFLAV(4), AMDEC(4) 
      COMMON / INOUT  / NINP,NOUT 
      COMMON / BXFMTS / BXOPE,BXCLO,BXTXT,BXL1I,BXL1F,BXL2F,BXL1G,BXL2G
      CHARACTER*80      BXOPE,BXCLO,BXTXT,BXL1I,BXL1F,BXL2F,BXL1G,BXL2G
      CHARACTER*80      BX11F
! This common can be everywhere, contains various switches
      COMMON / KeyKey /  KeyRad,KeyPhy,KeyTek,KeyMis,KeyDwm,KeyDwp
      save / DECAYS /,/ INOUT  /,/ BXFMTS /
      DIMENSION drvec(1)
      parameter (mm_nrchan=65)
      DIMENSION PROB(mm_nrchan)
      data istart /0/
      save
      BX11F =  '(1X,1H*,F17.8,               16X, A20,I12,A7, 1X,1H*)' 
! random choice of generation branch as function of sprime, and final state chosen.
! should be emptied from rn. generation to preserve generation series. 
! you can adopt your choice of the presampling type on sprim and iflav.
! may be one should coonect it with placer? For choice of presampling resonances?
        KeySmp = MOD(KeyTek,1000)/100
        IF (KeySmp.eq.0) THEN
          prob(1)= 1d0
          do i=2,mm_nrchan
            prob(i)=0d0
          enddo
        ELSE
          do i=1,7
            prob(i)=0d0
          enddo
ccc          prob(1)=1d-5
          do i=8,mm_nrchan
            prob(i)=0d0
ccc            prob(i)=1.0d-5
ccc            prob(i)=.02d0
          enddo
          prob(62)=0d0
          prob(63)=0d0
          prob(64)=0d0
          prob(65)=0d0


          IF(abs(iflav(1)).NE.11 .AND. abs(iflav(2)).NE.11 .AND. 
     @       abs(iflav(3)).NE.11 .AND. abs(iflav(4)).NE.11 .AND. 
     @       abs(iflav(1)).NE.abs(iflav(4)) ) THEN 
          IF( iflav(1).EQ.1.AND.iflav(3).EQ.4 .OR. !  ccdd
     @        iflav(1).EQ.4.AND.iflav(3).EQ.1 .OR. 
     @        iflav(1).EQ.2.AND.iflav(3).EQ.3 .OR. !  uuss
     @        iflav(1).EQ.3.AND.iflav(3).EQ.2 .OR. 
     @        iflav(1).EQ.2.AND.iflav(3).EQ.5 .OR. !  uubb
     @        iflav(1).EQ.5.AND.iflav(3).EQ.2 .OR. 
     @        iflav(1).EQ.4.AND.iflav(3).EQ.5 .OR. !  ccbb
     @        iflav(1).EQ.5.AND.iflav(3).EQ.4 ) THEN
c 2/20/98 fix for uuss... WW resonance
            prob( 10 )= 0.05d0   ! fix for uuss
            prob( 13 )= 0.05d0   ! fix for uuss
          ENDIF
       prob( 1 )= 0.105942985204660847  !max= 0.422468563492594429
       prob( 8 )= 0.104089346949555211  !max= 0.961147585632993628
       prob( 11 )= 0.195082323650622030  !max= 1.00000000000000000

       prob( 14 )= 0.248228255805263420E-01  !max= 0.101387425860385891
       prob( 17 )= 0.271209922326381137E-01  !max= 0.208362394638424042
       prob( 22 )= 0.30E-01 
       prob( 26 )= 0.247906173207529008E-01  !max= 0.128756761020349664
       prob( 29 )= 0.303159280499325189E-01  !max= 0.389546245736929753
       prob( 34 )= 0.27E-01  
       prob( 37 )= 0.228127925024490461E-01  !max= 0.157216473763183984
       prob( 40 )= 0.208963525281628468E-01  !max= 0.983818184116975469E-01
       prob( 43 )= 0.551212725876649504E-01  !max= 0.333970332517378732
       prob( 44 )= 0.482877207213861098E-01  !max= 0.200693014877614762
       prob( 47 )= 0.781729255217893798E-01  !max= 0.573873279622014842
       prob( 52 )= 0.235323459040677439E-01  !max= 0.134825656145201422
       prob( 55 )= 0.294615817248881640E-01  !max= 0.304665728733879870
       prob( 56 )= 0.436209697611461625E-01  !max= 0.173587530792248423
       prob( 59 )= 0.409452531335373071E-01  !max= 0.925181790377090085E-01

          ELSEIF(abs(iflav(1)).NE.11 .AND. abs(iflav(2)).NE.11 .AND. 
     @       abs(iflav(3)).NE.11 .AND. abs(iflav(4)).NE.11 ) THEN 
!  1-1 type channels(u,u,u,u)
            prob(  1 )= 0.1d0  
            prob( 10 )= 0.1d0  
            prob( 13 )= 0.26d0
            prob( 14 )= 0.4d-1
            prob( 28 )= 0.2d-1
            prob( 38 )= 0.3d-1 !fsr .3d-1
            prob( 42 )= 0.2d-1
            prob( 45 )= 0.3d-1 !fsr .3d-1
            prob( 49 )= 0.2d-1
            prob( 50 )= 0.2d-1
            prob( 54 )= 0.2d-1
            prob( 61 )= 0.2d-1

            prob(  8 )= 0.2d0
            prob( 11 )= 0.2d0
            prob( 29 )= 0.2d-1
            prob( 59 )= 0.1d0
          ELSEIF(abs(iflav(1)).EQ.11 .AND. abs(iflav(2)).EQ.11 .AND. 
     @           (abs(iflav(3)).EQ.14 .OR. abs(iflav(3)).EQ.16) ) THEN 
!  6-10 type channels (e,e,nm,nm)
!!            prob(  1 )= .6d0
            prob(  8 )= .1d0 !
            prob(  9 )= .3d-1 ! it was 46
            prob( 17 )= .2d0 
            prob( 18 )= .5d-1 !
            prob( 25 )= .1d0 
            prob( 37 )= .5d-1 !
            prob( 38 )= .3d-1 
            prob( 41 )= .3d-1 
            prob( 43 )= .2d0 !
            prob( 55 )= .1d0 
            prob( 58 )= .5d-1 

            prob( 10 )= 1d-1 !
            prob( 11 )= .1d0 !
            prob( 20 )= .5d-1 !
            prob( 21 )= .5d-1 !
            prob( 22 )= .5d-1 !
            prob( 32 )= .5d-1 !
            prob( 33 )= .5d-1 !
            prob( 34 )= .5d-1 !
            prob( 35 )= .5d-1 !
            prob( 36 )= .5d-1 !
            prob( 40 )= .1d0 !
            prob( 42 )= .5d-1 !
            prob( 44 )= .5d-1 !
            prob( 45 )= .5d-1 !
            prob( 50 )= .5d-1 !
            prob( 52 )= .5d-1 !
            prob( 56 )= .5d-1 !
            prob( 57 )= .5d-1 !
            prob( 59 )= .5d-1 !
            prob( 60 )= .5d-1 !

          ELSEIF( (abs(iflav(1)).EQ.11 .AND. abs(iflav(2)).EQ.11 .AND.
     @           abs(iflav(3)).NE.11 .AND. abs(iflav(4)).NE.11 )
     @           .OR.
     @            (abs(iflav(1)).EQ.11 .AND. abs(iflav(2)).EQ.11 .AND. 
     @           abs(iflav(3)).EQ.11 .AND. abs(iflav(4)).EQ.11 )
     @          ) THEN 
!  6-1 type channels (e,e,u,u)
!  6-6 type channels (e,e,e,e)

! jacob. optym.
            prob(  8 )= .5d-1 !c
            prob(  9 )= .3d-1 ! it was 46
            prob( 11 )= .5d-1 !c
            prob( 17 )= .5d-1 !c
            prob( 18 )= .5d-1 !
            prob( 25 )= .2d0 !c
            prob( 37 )= .5d-1 !
            prob( 38 )= .3d-1 
            prob( 41 )= .3d-1 
            prob( 43 )= .5d-1 !c
            prob( 55 )= .5d-1 !c 
            prob( 58 )= .5d-1 

            prob( 20 )= .5d-1 !
            prob( 21 )= .5d-1 !
            prob( 22 )= 1d-1 !c
            prob( 33 )= .5d-1 !
            prob( 34 )= 1.5d-1 !c
            prob( 36 )= 1.5d-1 !c
            prob( 40 )= 3.5d-1 !c
            prob( 42 )= .5d-1 !
            prob( 45 )= .5d-1 !
            prob( 50 )= .5d-1 !
            prob( 52 )= .5d-1 !
            prob( 57 )= .5d-1 !
            prob( 60 )= .5d-1 !

            prob( 10 )= 1d-1 !rev dodac
            prob( 23 )= .5d-1 !rev dodac
            prob( 35 )= 1d-1 !rev dodac
            prob( 46 )= .5d-1 !rev dodac
            prob( 49 )= .5d-1 !rev dodac

          ELSEIF( (abs(iflav(1)).EQ.14 .OR. abs(iflav(1)).EQ.16) .AND. 
     @           abs(iflav(3)).EQ.11 .AND. abs(iflav(4)).EQ.11 ) THEN 
! 10-6 type channels (nm,nm e,e) 
            prob( 11 )= .1d0 !
            prob( 12 )= .3d-1 ! it was 46
            prob( 34 )= .2d0 
            prob( 33 )= .5d-1 !
            prob( 26 )= .1d0 
            prob( 14 )= .5d-1 !
            prob( 61 )= .3d-1 
            prob( 58 )= .3d-1 
            prob( 56 )= .2d0 !
            prob( 44 )= .1d0 
            prob( 41 )= .5d-1 

            prob( 13 )= .1d0 !eenn
            prob( 8 )= .1d0 !eenn
            prob( 31 )= .5d-1 !
            prob( 30 )= .5d-1 !
            prob( 29 )= .5d-1 !
            prob( 19 )= .5d-1 !
            prob( 18 )= .5d-1 !
            prob( 17 )= .5d-1 !
            prob( 16 )= .5d-1 !
            prob( 15 )= .5d-1 !
            prob( 59 )= .1d0 !eenn
            prob( 57 )= .5d-1 !
            prob( 55 )= .5d-1 !eenn
            prob( 54 )= .5d-1 !
            prob( 49 )= .5d-1 !
            prob( 47 )= .5d-1 !
            prob( 43 )= .5d-1 !
            prob( 42 )= .5d-1 !
            prob( 40 )= .5d-1 !
            prob( 39 )= .5d-1 !

          ELSEIF(abs(iflav(1)).NE.11 .AND. abs(iflav(2)).NE.11 .AND. 
     @           abs(iflav(3)).EQ.11 .AND. abs(iflav(4)).EQ.11 ) THEN 
! 1-6 type channels (u,u e,e) 

! jacob. optym.
            prob( 11 )= .5d-1 !c
            prob( 12 )= .3d-1 ! it was 46
            prob(  8 )= .5d-1 !c
            prob( 34 )= .5d-1 !c
            prob( 33 )= .5d-1 !
            prob( 26 )= .2d0 !c
            prob( 14 )= .5d-1 !
            prob( 61 )= .3d-1 
            prob( 58 )= .3d-1 
            prob( 56 )= .5d-1 !c
            prob( 44 )= .5d-1 !c 
            prob( 41 )= .5d-1 

            prob( 31 )= .5d-1 !
            prob( 30 )= .5d-1 !
            prob( 29 )= 1d-1 !c
            prob( 18 )= .5d-1 !
            prob( 17 )= 1.5d-1 !c
            prob( 15 )= 1.5d-1 !c
            prob( 59 )= 3.5d-1 !c
            prob( 57 )= .5d-1 !
            prob( 54 )= .5d-1 !
            prob( 49 )= .5d-1 !
            prob( 47 )= .5d-1 !
            prob( 42 )= .5d-1 !
            prob( 39 )= .5d-1 !

            prob( 13 )= 1d-1 !rev dodac
            prob( 28 )= .5d-1 !rev dodac
            prob( 16 )= 1d-1 !rev dodac
            prob( 53 )= .5d-1 !rev dodac
            prob( 50 )= .5d-1 !rev dodac


          ENDIF
        ENDIF

!!!!!!!!!!!!!!!!!!!!!!!!!!!! danger !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!! danger !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!          prob(1)=1.0d-5
!          do i=8,mm_nrchan
!            prob(i)=1.0d-5
!          enddo
!          prob(62)=0d0
!          prob(63)=0d0
!!!!!!!!!!!!!!!!!!!!!!!!!!!! danger !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!! danger !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

!!!!!!!!!!!!! flat distr. for Mariusz begin !!!!!!!!!!!!!!!!!!!!
!          prob(2)=.5d0
!!!!!!!!!!!!! flat distr. for Mariusz end !!!!!!!!!!!!!!!!!!!!




        prtot=0d0
        DO I=1,mm_nrchan
          prtot=prtot+prob(I)
        ENDDO
        DO I=1,mm_nrchan  
          prob(I)=prob(I)/prtot
        ENDDO
!     ---------------------
      IF (istart.eq.0) THEN
!     ---------------------
        istart=1
        WRITE(NOUT,BXOPE) 
        WRITE(NOUT,BXTXT) '             Window X_ZZ                '
        WRITE(NOUT,BXTXT) '          mm_brancher_ZZ report         '
        if (KeySmp.eq.0) then
        WRITE(NOUT,BXTXT) ' WARNING: KeySmp =0 Brancher is off !   '
        else
        WRITE(NOUT,BXTXT) '          mm_brancher_ZZ  is on         '
        DO I=1,mm_nrchan  
          WRITE(NOUT,BX11F) prob(I),'prob. for branch NR: ',I,'X1'
        ENDDO
!        WRITE(NOUT,BXL1F) prtot,'total probability ','XX'
        WRITE(NOUT,BXTXT) '                                        '
        endif
        WRITE(NOUT,BXCLO)         
!     -----
      ENDIF
!     -----

      if(Keysmp.eq.0) then
!     ====================
        itype=1
      else
!     ====================
        CALL varran(drvec,1)
        PROBI=0D0
        DO I=1,mm_nrchan
          PROBI=PROBI+PROB(I)
          if(drvec(1).lt.probI) THEN
            itype=I
            GOTO 10
           ENDIF
        enddo
        write(6,*) 
     $ 'mm_brancher_ZZ has problem prob=',prtot
        stop
 10     continue
      endif
!     =====================
      end 
