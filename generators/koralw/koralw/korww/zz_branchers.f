      subroutine brancher(sprim,itype)
!     ******************************************************    
! ###########################################
! sets probablilities of channels, can be   #
! function of sprim and iflav(4)            #
! ###########################################
      implicit DOUBLE PRECISION (a-h,o-z)

      INCLUDE 'zz_phsp.inc'

      COMMON / DECAYS / IFLAV(4), AMDEC(4) 
      COMMON / INOUT  / NINP,NOUT 
      COMMON / BXFMTS / BXOPE,BXCLO,BXTXT,BXL1I,BXL1F,BXL2F,BXL1G,BXL2G
      CHARACTER*80      BXOPE,BXCLO,BXTXT,BXL1I,BXL1F,BXL2F,BXL1G,BXL2G
! This common can be everywhere, contains various switches
      COMMON / KeyKey /  KeyRad,KeyPhy,KeyTek,KeyMis,KeyDwm,KeyDwp
      save / DECAYS /,/ INOUT  /,/ BXFMTS /   
      DIMENSION drvec(1)
c ms      parameter (NRCH=100)
c ms      DIMENSION PROB(NRCH)
      CHARACTER*5 XI(NRCH)
c ms      common /kanalarz/ fak,fakI(NRCH),ikan,MRCHAN,NRCHAN
c ms      save  /kanalarz/
      logical ifnu,ifnnu,ifnel,ifel
      save
      data istart /0/
      ifnu(k)= (abs(k).eq.12). or.(abs(k).eq.14). or.(abs(k).eq.16)
      ifnnu(k)=(abs(k).ne.12).and.(abs(k).ne.14).and.(abs(k).ne.16)
      ifel(k)= (abs(k).eq.11)
      ifnel(k)=(abs(k).ne.11)
! random choice of generation branch as function of sprime, and final state chosen.
! should be emptied from rn. generation to preserve generation series. 
! you can adopt your choice of the presampling type on sprim and iflav.
! may be one should coonect it with placer? For choice of presampling resonances?
       if (istart.eq.0) then
         MRCHAN=1
         NRCHAN=69
! MS 2/20/99 fix for uuss WW part
         NRCHAN=70
! MS 2/20/99 fix for uuss WW part END
         nrstor=nrchan 
         
         KeySmp = MOD(KeyTek,1000)/100
         XI(1)='  1  ' 
         XI(2)='  2  ' 
         XI(3)='  3  ' 
         XI(4)='  4  ' 
         XI(5)='  5  '   
         XI(6)='  6  '   
         XI(7)='  7  ' 
         XI(8)='  8  ' 
         XI(9)='  9  ' 
         XI(10)=' 10  ' 
         XI(11)=' 11  ' 
         XI(12)=' 12  ' 
         XI(13)=' 13  ' 
         XI(14)=' 14  ' 
         XI(15)=' 15  ' 
         XI(16)=' 16  ' 
         XI(17)=' 17  ' 
         XI(18)=' 18  ' 
         XI(19)=' 19  ' 
         XI(20)=' 20  '
         XI(21)=' 21  '
         XI(22)=' 22  '
         XI(23)=' 23  '
         XI(24)=' 24  '
         XI(25)=' 25  '
         XI(26)=' 26  '
         XI(27)=' 27  '
         XI(28)=' 28  '
         XI(29)=' 29  '
         XI(30)=' 30  '
         XI(31)=' 31  '
         XI(32)=' 32  '
         XI(33)=' 33  '
         XI(34)=' 34  '
         XI(35)=' 35  '
         XI(36)=' 36  '
         XI(37)=' 37  '
         XI(38)=' 38  '
         XI(39)=' 39  '
         XI(40)=' 40  '
         XI(41)=' 41  '
         XI(42)=' 42  '
         XI(43)=' 43  '
         XI(44)=' 44  '
         XI(45)=' 45  '
         XI(46)=' 46  '
         XI(47)=' 47  '
         XI(48)=' 48  '
         XI(49)=' 49  '
         XI(50)=' 50  '
         XI(51)=' 51  '
         XI(52)=' 52  '
         XI(53)=' 53  '
         XI(54)=' 54  '
         XI(55)=' 55  '
         XI(56)=' 56  '
         XI(57)=' 57  '
         XI(58)=' 58  '
         XI(59)=' 59  '
         XI(60)=' 60  '
         XI(61)=' 61  '
         XI(62)=' 62  '
         XI(63)=' 63  '
         XI(64)=' 64  '
         XI(65)=' 65  '
         XI(66)=' 66  '
         XI(67)=' 67  '
         XI(68)=' 68  '
         XI(69)=' 69  '
         XI(70)=' 70  '
       endif   
         if (KeySmp.eq.0) then
           prob(1)= 1d0
           DO k=2,nrstor
             prob(k)=.0d0
           ENDDO
           mrchan=1
           nrchan=1
         else
          prob(1)=0.40d0
          prob(2)=0!.20d0     ! zero
          prob(3)=0.80d0    ! (Z+gam)*(Z+gam) + ini pairs ? 
          prob(4)=0!.200d0    ! zero
          prob(5)=0!.20d0    ! zero
          prob(6)=0!.20d0    ! zero
          prob(7)=0.03d0    ! brem 2+3 from 4
          prob(8)=0.03d0    ! brem 2+3 from 1
          prob(9)=0.10d0    ! brem 1+4 from 3
          prob(10)=0.10d0   ! brem 1+4 from 2

          prob(11)=0.20d0  ! multiperif 1 + W-res in 3-4
          prob(12)=0.20d0  ! multiperif 1 + 4  Z-res in 2-3
          prob(13)=0!.050d0 ! out ?
          prob(14)=0.6d0   ! multiperif 1 + 3
          prob(15)=0.1d0   ! multiperif 1 + 4
          prob(16)=0.20d0   ! multiperif 4 + W-res in 3-4
          prob(17)=0.20d0   ! multiperif 4 + 1  Z-res in 3-2
          prob(18)=0!.050d0  ! out ?
          prob(19)=0.6d0    ! multiperif 4 + 2
          prob(20)=0.1d0    ! multiperif 4 + 1
          prob(21)=0.6d0    ! for zz  z(1+2) z(3+4) 
          prob(22)=0.6d0    ! for zz  z(1+4) z(2+3) 

          prob(23)=0.05d0    ! for zz  (1+2)  from 3
          prob(24)=0.05d0    ! for zz  (1+2)  from 4
          prob(25)=0.05d0    ! for zz  (3+4)  from 1 
          prob(26)=0.05d0    ! for zz  (3+4)  from 2 

          prob(27)=0.05d0    ! for zz  (1+4)  from 3
          prob(28)=0.05d0    ! for zz  (1+4)  from 2
          prob(29)=0.05d0    ! for zz  (2+3)  from 1 
          prob(30)=0.05d0    ! for zz  (2+3)  from 4 

          prob(31)=0.1       ! for zz  multiperif 1+3
          prob(32)=0.1       ! for zz  multiperif 1+4
          prob(33)=0.1       ! for zz  multiperif 2+3
          prob(34)=0.1       ! for zz  multiperif 2+4

          prob(35)=0.1       ! for zz  multiperif 3+1
          prob(36)=0.1       ! for zz  multiperif 3+2
          prob(37)=0.1       ! for zz  multiperif 4+1
          prob(38)=0.1       ! for zz  multiperif 4+2

          prob(39)=0.1       ! for zz  rev-multiperif 1+2 + Z (gamma?)
          prob(40)=0.1       ! for zz  rev-multiperif 2+1 + Z (gamma?)
          prob(41)=0.1       ! for zz  rev-multiperif 3+4 + Z (gamma?)
          prob(42)=0.1       ! for zz  rev-multiperif 4+3 + Z (gamma?)

          prob(43)=0.15d0    ! for zz  (1+2)  from 3 (e+
          prob(44)=0.15d0    ! for zz  (1+2)  from 4 (e-
          prob(45)=0.15d0    ! for zz  (3+4)  from 1 (e+
          prob(46)=0.15d0    ! for zz  (3+4)  from 2 (e-

          prob(47)=0.1       ! for zz  multiperif 1+2
          prob(48)=0.1       ! for zz  multiperif 2+1
          prob(49)=0.1       ! for zz  multiperif 3+4
          prob(50)=0.1       ! for zz  multiperif 4+3

          prob(51)=0.35d0    ! for zz  z(3+4) z(1+2) i.e. t at 3 
          prob(52)=0.35d0    ! for zz  z(1+2) z(3+4) i.e. t at 4 
          prob(53)=0.35d0    ! for zz  z(1+2) z(3+4) i.e. t at 1 
          prob(54)=0.35d0    ! for zz  z(3+4) z(1+2) i.e. t at 2 

          prob(55)=0.1       ! for zz  rev-multiperif 1+4 + Z (gamma?)
          prob(56)=0.1       ! for zz  rev-multiperif 4+1 + Z (gamma?)
          prob(57)=0.1       ! for zz  rev-multiperif 3+2 + Z (gamma?)
          prob(58)=0.1       ! for zz  rev-multiperif 2+3 + Z (gamma?)

          prob(59)=0.15d0    ! for zz  (1+4)  from 3 (e+
          prob(60)=0.15d0    ! for zz  (1+4)  from 2 (e-
          prob(61)=0.15d0    ! for zz  (2+3)  from 1 (e+
          prob(62)=0.15d0    ! for zz  (2+3)  from 4 (e-

          prob(63)=0.35d0    ! for zz  z(1+4) z(3+2) i.e. t at 1 
          prob(64)=0.35d0    ! for zz  z(3+2) z(1+4) i.e. t at 4 
          prob(65)=0.35d0    ! for zz  z(3+2) z(1+4) i.e. t at 3 
          prob(66)=0.35d0    ! for zz  z(1+4) z(3+2) i.e. t at 2 

          prob(67)=0.35d0    ! for tests for cc03 ...
          prob(68)=0.35d0    ! for tests for cc03 but zz res...
          prob(69)=0.35d0    ! for tests for cc03 zz flattened...
! MS 2/20/99 fix for uuss WW part
          prob(70)=0.1d0    ! for MIX (zz)  W(1+4) W(3+2) 
! MS 2/20/99 fix for uuss WW part END
         endif
!         do k=1,42
!         prob(k)=0
!         enddo
!         do k=55,66
!         prob(k)=0
!         enddo
       if (istart.eq.0) then
         istart=1
        WRITE(NOUT,BXOPE) 
        WRITE(NOUT,BXTXT) '                Window X                '
        WRITE(NOUT,BXTXT) '            Brancher report             '
        if (KeySmp.eq.0)
     $  WRITE(NOUT,BXTXT) ' WARNING: KeySmp =0 Brancher is off !   '
        if (KeySmp.ne.0)
     $  WRITE(NOUT,BXTXT) '          Brancher is on now            '
         DO I=1,NRCHAN       
      WRITE(NOUT,BXL1F) prob(I),' wgt. for branch NR: ',XI(I),'X1'
         ENDDO
        WRITE(NOUT,BXTXT) ' some wgts zeroed in funct of channel   '
        WRITE(NOUT,BXTXT) '                                        '
cc ms        WRITE(NOUT,BXL1F)facwel,'effic factor for W->e-nu',' ','X4'
cc ms        WRITE(NOUT,BXL1F)faczel,'effic factor for Z->e-e ',' ','X5'
        WRITE(NOUT,BXCLO)         
       endif
!#########################################################
!#########################################################
!! here some channels can be switched off as a function of iflav(1..4)
!#########################################################
!#########################################################
!       +++++++++++++++++++++
        if (KeySmp.ne.0) then
!       +++++++++++++++++++++
!! ---  WW channels
        if (iflav(1)+iflav(2).ne.0) then
          nrchan=20
          mrchan=1
          do k=nrchan+1,nrstor
            prob(k)=0
          enddo
          if (iflav(1)+iflav(4).ne.0)                   prob( 3)=0
 
          if (ifnu(iflav(2)).or.(iflav(2).ne.-iflav(3))) prob( 7)=0
          if (ifnu(iflav(2)).or.(iflav(2).ne.-iflav(3))) prob( 8)=0
          if (ifnu(iflav(3)).or.(iflav(1).ne.-iflav(4))) prob( 9)=0
          if (ifnu(iflav(2)).or.(iflav(1).ne.-iflav(4))) prob(10)=0

          if (ifnel(iflav(1)))        then
                                       do k=11,15
                                         prob(k)=0
                                       enddo
                                      endif
          if (ifnu(iflav(3)))         prob(14)=0
 
          if (ifnel(iflav(4)))        then
                                       do k=16,20
                                          prob(k)=0
                                       enddo
                                      endif
          if (ifnu(iflav(2)))         prob(19)=0
 !! --- ZZ channels
        else
          nrchan=nrstor
          mrchan=21 ! <<<
          do k=1,20
            prob(k)=0
          enddo
          if (iflav(1)+iflav(4).ne.0) prob(22)=0
          if (ifnu(iflav(2)).or.ifnu(iflav(3))) then
            prob(23)=0
            prob(24)=0
            prob(25)=0
            prob(26)=0 

            prob(43)=0
            prob(44)=0
            prob(45)=0
            prob(46)=0
            prob(47)=0
            prob(48)=0
            prob(49)=0
            prob(50)=0
          elseif (ifel(iflav(2)).or.ifel(iflav(3))) then
            prob(23)=0
            prob(24)=0
            prob(25)=0
            prob(26)=0
            if (ifnel(iflav(3))) then
              prob(43)=0
              prob(44)=0
              prob(49)=0
              prob(50)=0
            endif
            if (ifnel(iflav(2))) then
              prob(45)=0
              prob(46)=0
              prob(47)=0
              prob(48)=0
            endif
          else
            prob(43)=0
            prob(44)=0
            prob(45)=0
            prob(46)=0
            prob(47)=0
            prob(48)=0
            prob(49)=0
            prob(50)=0
          endif
          if (ifnu(iflav(1)).or.ifnu(iflav(3)).or.
     $       (iflav(2)+iflav(3).ne.0)            ) then
            prob(27)=0
            prob(28)=0
            prob(29)=0
            prob(30)=0
          endif
          if (ifnel(iflav(1)))then
            prob(31)=0
            prob(32)=0
            prob(33)=0
            prob(34)=0
            prob(53)=0
            prob(54)=0
!  ---          prob(39)=0
!    - --      prob(40)=0
          endif
          if (ifnel(iflav(3)))then
            prob(35)=0
            prob(36)=0
            prob(37)=0
            prob(38)=0
            prob(51)=0
            prob(52)=0
!   --        prob(41)=0
!--          prob(42)=0

          endif

!          if ((ifel(iflav(1))).or.(ifel(iflav(3)))) then
!            prob(21)=0
!            prob(22)=0
!          endif
          if ((ifel(iflav(2))).and.(ifel(iflav(3)))) then 
            do k=27,30
              prob(k)=0
            enddo 
          else
            do k=55,66
              prob(k)=0
            enddo 
          endif
! 2/20/99 ms fix for uuss 
          IF( iflav(1).EQ.1.AND.iflav(3).EQ.4 .OR. !  ccdd
     @        iflav(1).EQ.4.AND.iflav(3).EQ.1 .OR. 
     @        iflav(1).EQ.2.AND.iflav(3).EQ.3 .OR. !  uuss
     @        iflav(1).EQ.3.AND.iflav(3).EQ.2 .OR. 
     @        iflav(1).EQ.2.AND.iflav(3).EQ.5 .OR. !  uubb
     @        iflav(1).EQ.5.AND.iflav(3).EQ.2 .OR. 
     @        iflav(1).EQ.4.AND.iflav(3).EQ.5 .OR. !  ccbb
     @        iflav(1).EQ.5.AND.iflav(3).EQ.4 ) THEN
            CONTINUE ! empty
          ELSE
            prob(70) =0d0
          ENDIF
! 2/20/99 ms fix for uuss END
        endif
!       +++++++++++++++++++++
        endif
!       +++++++++++++++++++++

!#########################################################
!#########################################################
!#########################################################
!#########################################################

         prtot=0d0
         DO I=MRCHAN,NRCHAN
           prtot=prtot+prob(I)
         ENDDO
         DO I=MRCHAN,NRCHAN
           prob(I)=prob(i)/prtot
         ENDDO
!
        if(Keysmp.eq.0) then
!       ====================
         itype=1
!
        else
!       ====================
!
         CALL varran(drvec,1)
         PROBI=0D0
         DO I=MRCHAN,NRCHAN
          PROBI=PROBI+PROB(I)
          if(drvec(1).lt.probI) THEN
           itype=I
           GOTO 10
          ENDIF
         enddo
        write(*,*) 
     $ 'brancher has problem prob=',prtot
 10     continue
!
        endif
!       =====================
      end 
