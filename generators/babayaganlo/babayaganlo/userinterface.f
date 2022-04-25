      subroutine userinterface
! written by CMCC, last modified 7/4/2006
      implicit double precision (a-b,d-h,o-z)
      implicit character*10 (c)
      logical istop
      character*20  c,ctmp
      character*100 cinput
      character*6   ord,arun,darkmod
      character*10  model,menu2,menud
      character*100 outfile,storefile
      character*80  path
      character*30  cx
      character*100 stringa
      character*3   eventlimiter,store
      character*50  programma,corrections
      double precision convfac
      integer*8 iwriteout
      character*12 col(20)
c      character*12 c1,c2,c3
      character*1 c1,c2,c3
      character*2 fs
      common/finalstate/fs
      common/colors/col
      common/ecms/ecms,ecmsnom
      common/nphot_mode/nphotmode
      common/expcuts/thmin,thmax,emin,zmax,egmin,thgmin,thgmax
      common/zparameters/zm,gz,ve,ae,rv,ra,wm,s2th,gfermi,sqrt2,um
      common/epssoft/eps
      common/energiabeam/ebeam
      common/parameters/ame,ammu,convfac,alpha,pi
      common/intinput/iwriteout,iseed,nsearch,iverbose
      common/qedORDER/ord
      common/charinput/model,eventlimiter,store,storefile,outfile
      character*100 vpuncout
      common/vpuncfile/vpuncout
      common/realinput/anpoints,sdifmax
      common/nmaxphalpha/nphmaxalpha

      common/ialpharunning/iarun
      common/rescaledhaderr/scal_err_dhad
      common/beamspreadsigmas/esig1,esig2
      
      common/idarkon/idarkon
      common/darkmatter/amassU,gammaU,gvectU,gaxU
      common/massainv/amassainvmin,amassainvmax
      character*7 setkey(36)

c$$$      
c$$$ for teubner routine, 2010/05/19
c$$$
      integer iteubn
      common/teubner/iteubn
c$$$
c$$$
c$$$ for Novosibirsk VP routine, 2015/01/22
c$$$
      integer inskvp
      common/nskvp/inskvp
c$$$
!       parameter (lux=4,k1=0,k2=0) ! for RANLUX

      data setkey/
     +     'ecms   ', !  1
     +     'thmin  ', !  2
     +     'thmax  ', !  3
     +     'zmax   ', !  4
     +     'emin   ', !  5
     +     'nev    ', !  6
     +     'path   ', !  7
     +     'ntuple ', !  8
     +     'menu2  ', !  9
     +     'seed   ', ! 10
     +     'ord    ', ! 11
     +     'model  ', ! 12
     +     'nphot  ', ! 13
     +     'mode   ', ! 14
     +     'eps    ', ! 15
     +     'nwrite ', ! 16
     +     'nsearch', ! 17
     +     'verbose', ! 18
     +     'sdmax  ', ! 19
     +     'legenda', ! 20
     +     'arun   ', ! 21
     +     'fs     ', ! 22
     +     'menud  ', ! 23
     +     'darkmod', ! 24
     +     'massU  ', ! 25
     +     'gammaU ', ! 26
     +     'k      ', ! 27
     +     'gaxU   ', ! 28
     +     'egmin  ', ! 29
     +     'thgmin ', ! 30 
     +     'thgmax ', ! 31
     +     'massmin', ! 32
     +     'massmax', ! 33
     +     'scEdhad', ! 34
     +     'sprb1  ', ! 35
     +     'sprb2  '  ! 36
     +     /

      character*62 welcome(23)
      data welcome/
     +'12345678901234567890123456789012345678901234567890123456789012',!1
     +' *************************************************************',!2
     +' **********                                         **********',!3
     +' ********                                             ********',!4
     +' ******             Welcome to BabaYaga                 ******',!5
     +' ****             ^^^^^^^^^^^^^^^^^^^^^^^                 ****',!6
     +' **       It is an event generator for QED processes        **',!7
     +' ****      at low energies, matching a QED PS with        ****',!8
     +' ******           exact order alpha corrections         ******',!9
     +' ********                                             ********',!10
     +' **********                                         **********',!11
     +' *************************************************************',!12
     +'  [[ it simulates: e+e- -->>  g  -->> e+e- or mu+mu- or gg ]] ',!13
     +'  [[             : e+e- -->> g,U -->> e+e-g or mu+mu-g     ]] ',!14
     +' *****     *      *****     *           *****   ******    *   ',!15
     +' *        * *     *        * *         *    *   *        * *  ',!16
     +' *        * *     *        * *         *    *   *        * *  ',!17
     +' ****    *   *    ****    *   *         *****   *       *   * ',!18
     +' *   *   *****    *   *   *****          *  *   *       ***** ',!19
     +' *   *  *     *   *   *  *     *        *   *   *      *     *',!20
     +' ****   *     *   ****   *     *        *   *   *      *     *',!21
     +' ',
     +' '/
      call initcolors
c ;-)    call system("mplayer /home/carlo/Desktop/Baba-Yaga.mp3
c ;-)  .     1>/dev/null 2>/dev/null &")
      do k = 2,14
         print*,welcome(k)
      enddo

      convfac = 0.389379660D6
      pi      = 3.1415926535897932384626433832795029D0 
      do i = 1,10
         model(i:i) = ' '
      enddo
      do k = 1,80
         path(k:k) = ' '
      enddo
      do i = 1,50
         programma(i:i)   = ' '
         corrections(i:i) = ' '
      enddo

      ame  =   0.51099906d-3
      ammu = 105.65836900d-3 

c      do ii = 1,1000
c         print*,'MASSA MU = MASSA E'
c      enddo

c----------------------------------
c      print*,' '
c      print*,'ENTER ELECTRON MASS (GEV): (< 0 --> default)'
c      read*,ametmp
c      if (ametmp.gt.0.d0) ame = ametmp
c      print*,'--> electron mass = ',ame,' GeV'
c      print*,' '
c----------------------------------

      alpha = 1.D0/137.0359895D0

** Z parameters...
      zm  = 91.1867D0
      wm  = 80.35d0 
      gz  = 2.4952D0
      s2th = 1.d0 - wm**2/zm**2

      gfermi = 1.16639d-5
      sqrt2  = sqrt(2.d0)
      gfermi = pi*alpha/2.d0/s2th/wm**2*sqrt2

      ve = -1.d0 + 4.d0*s2th
      ae = -1.d0
      rv = ve**2/(ae**2+ve**2)
      ra = ae**2/(ae**2+ve**2)
      um = sqrt(pi*alpha/sqrt2/gfermi)
** ...Z parameters

***** default input values
      fs   = 'ee' ! 'ee' or 'gg' or 'mm'
      path = 'test-run/'
      ecms = 1.02d0
      ord  = 'exp'
      nphotmode = -1
      model  = 'matched'
      thmin  =  20.d0
      thmax  = 160.d0
      zmax   =  10.d0
      emin   =   0.408d0
      anpoints = 10000000.d0

! OLD DEFAULT      
      eps    = 5.d-4
! NEW DEFAULT (it has to be much smaller than \Gamma/M for the M resonance...)
      eps    = 5.d-7

      egmin  = 0.02d0
      thgmin = thmin 
      thgmax = thmax

      iseed        = 700253512
      iwriteout    = 1000000
      nsearch      = 5000000
      iverbose     = 0
      eventlimiter = 'w'  ! 'unw' or 'w'
      store        = 'no'
      sdifmax      = 1.d-18
      arun         = 'on'

!     NEW DEFAULT: using Fedor Ignatov's VP parameterization
      arun         = 'nsk'

      darkmod      = 'off'
      amassU       = 0.4d0
      gammaU       = -1. 
      gvectU       = 1.d-3
      gaxU         = 0.d0
      menu2        = 'off'
      menud        = 'off'
      amassainvmin = 0.d0
      amassainvmax = ecms
      idarkon = 0
      iarun = 0
      if (arun.ne.'off') iarun = 1
      if (darkmod.eq.'on') idarkon = 1
      scal_err_dhad = 0.d0
      esig1  = 0.d0
      esig2  = 0.d0
***********

c      c1 = col(12)
c      c2 = col(10) ! 10 is white, 1 is black
c      c3 = col(15)
      c1 = ' '
      c2 = ' '
      c3 = ' '

      istop = .false.
      do while(.not.istop)
         print*,c2
         print*,'Principal Menu:'
         print*,' [ type "run" to start generation,'
         print*,'   "',setkey(20),'" for help or "quit" to quit ]'
         write(stringa,'(1x,A,A,A,A,A)')'['//c1,setkey(22),c2
     .//'] final state = '//c3,fs,c2//' '
         print*,stringa(1:79)
         write(stringa,'(1x,A,A,A,f7.3,A)')'['//c1,setkey(1),c2
     .//'] CoM energy    = '//c3,ecms,c2//' GeV'
         print*,stringa(1:79)
         write(stringa,'(1x,A,A,A,f7.3,A)')'['//c1,setkey(2),c2
     .//'] min. angle    = '//c3,thmin,c2//' deg'
         print*,stringa(1:79)
         write(stringa,'(1x,A,A,A,f7.3,A)')'['//c1,setkey(3),c2
     .//'] max. angle    = '//c3,thmax,c2//' deg'
         print*,stringa(1:79)
         write(stringa,'(1x,A,A,A,f7.3,A)')'['//c1,setkey(4),
     .c2//'] acollinearity = '//c3,zmax,c2//' deg'
         print*,stringa(1:79)
         write(stringa,'(1x,A,A,A,f7.3,A)')'['//c1,setkey(5),c2
     .//'] min. energy   = '//c3,emin,c2//' GeV'
         print*,stringa(1:79)
         write(stringa,'(1x,A,A,A,f12.0,A)')'['//c1,setkey(6),c2
     .//']'//c3,anpoints,c2//'events will be generated'
         print*,stringa(1:79)

         do k = 1,80
            if (path(k:k).ne.' ') lpath=k
         enddo
         if (path(lpath:lpath).ne.'/') then
            lpath = lpath+1
            path(lpath:lpath)='/'
         endif

         write(stringa,'(1x,A)')'['//c1//setkey(7)//c2
     .//'] files saved in'//c3//path(1:lpath)//c2
         print*,stringa(1:79)
         write(stringa,'(1x,A)')'['//c1//setkey(8)//c2
     .//'] ntuple creation'//c3//store//c2
         print*,stringa(1:79)
         write(stringa,'(1x,A)')'['//c1//setkey(9)//c2
     .//'] the second menu is'//c3//menu2//c2
         print*,stringa(1:79)

         write(stringa,'(1x,A)')'['//c1//setkey(23)//c2
     .//'] the dark matter menu is'//c3//menud//c2
         print*,stringa(1:79)

         write(stringa,'(1x,A,A,A,f9.7,A)')'['//c1,setkey(35),
     .c2//'] e- beam energy spread = '//c3,esig1,c2//' (GeV)'
         print*,stringa(1:79)

         write(stringa,'(1x,A,A,A,f9.7,A)')'['//c1,setkey(36),
     .c2//'] e+ beam energy spread = '//c3,esig2,c2//' (GeV)'
         print*,stringa(1:79)
                 
         if (menu2.eq.'on') then
*********
         print*,' '
         print*,'Second Menu (inner parameters):'
         write(stringa,'(1x,A)')'['//c1//setkey(21)//c2
     .//'] alpha running is'//c3//arun//c2
         print*,stringa(1:79)

         write(stringa,'(1x,A,f6.4,A)')'['//c1//setkey(34)//c2
     .//'] rescale factor for d_had error ='//c3,scal_err_dhad,c2
         print*,stringa(1:79)

         ctmp = 'unweighted'
         if (eventlimiter.eq.'w') ctmp = 'weighted'
         write(stringa,'(1x,A)')'['//c1//setkey(14)//c2
     .//'] requested evts. are'//c3//ctmp//c2
         print*,stringa(1:79)

         write(stringa,'(1x,A,f6.4,A)')'['//c1//setkey(15)//c2
     .//'] soft photon cutoff ='//c3,eps,c2
         print*,stringa(1:79)
         write(stringa,'(1x,A)')'['//c1//setkey(11)//c2
     .//'] corrections at'//c3//ord//c2//'order'
         print*,stringa(1:79)
         write(stringa,'(1x,A)')'['//c1//setkey(12)//c2
     .//'] model for corrections is'//c3//model//c2
         print*,stringa(1:79)
         write(stringa,'(1x,A,i9,A)')'['//c1//setkey(10)//c2
     .//'] seed for RANLUX'//c3,iseed,c2
         print*,stringa(1:79)
         write(stringa,'(1x,A,i3,A)')'['//c1//setkey(13)//c2
     .//'] max. number of photons mode is'//c3,nphotmode,c2
         print*,stringa(1:79)
         write(stringa,'(1x,A,i9,A)')'['//c1//setkey(16)//c2
     .//'] file(s) dumped every'//c3,iwriteout,c2//'events'
         print*,stringa(1:79)
         write(stringa,'(1x,A,i9,A)')'['//c1//setkey(17)//c2
     .//'] events for maximum searching'//c3,nsearch,c2
         print*,stringa(1:79)
         write(stringa,'(1x,A,i2,A)')'['//c1//setkey(18)//c2
     .//'] verbose mode (for debugging)'//c3,iverbose,c2
         print*,stringa(1:79)
         write(stringa,'(1x,A,e10.3,A)')'['//c1//setkey(19)//c2
     .//'] starting "sdifmax"'//c3,sdifmax,c2
         print*,stringa(1:79)
*********
         endif


         if (menud.eq.'on') then
*********
         print*,' '
         print*,'Dark matter Menu:'
         write(stringa,'(1x,A)')'['//c1//setkey(24)//c2
     .//'] The U channel is '//c3//darkmod//c2
         print*,stringa(1:79)
         write(stringa,'(1x,A,A,A,f6.3,A)')'['//c1,setkey(25),c2
     .//'] U mass  = '//c3,amassU,c2//' GeV'
         print*,stringa(1:79)
         write(stringa,'(1x,A,A,A,f6.3,A)')'['//c1,setkey(26),c2
     .//'] U width = '//c3,gammaU,c2//' GeV'
         print*,stringa(1:79)
         write(stringa,'(1x,A,A,A,f6.3,A)')'['//c1,setkey(27),c2
     .//'] vect g  = '//c3,gvectU,c2
         print*,stringa(1:79)
         write(stringa,'(1x,A,A,A,f6.3,A)')'['//c1,setkey(28),c2
     .//'] axial g = '//c3,gaxU,c2
         print*,stringa(1:79)
         write(stringa,'(1x,A,f7.3,A)')'['//c1//setkey(29)//c2
     .//'] photon min energy ='//c3,egmin,c2
         print*,stringa(1:79)
         write(stringa,'(1x,A,f7.3,A)')'['//c1//setkey(30)//c2
     .//'] photon min angle  ='//c3,thgmin,c2
         print*,stringa(1:79)
         write(stringa,'(1x,A,f7.3,A)')'['//c1//setkey(31)//c2
     .//'] photon max angle  ='//c3,thgmax,c2
         print*,stringa(1:79)
         write(stringa,'(1x,A,f7.3,A)')'['//c1//setkey(32)//c2
     .//'] min invariant mass='//c3,amassainvmin,c2
         print*,stringa(1:79)
         write(stringa,'(1x,A,f7.3,A)')'['//c1//setkey(33)//c2
     .//'] max invariant mass='//c3,amassainvmax,c2
         print*,stringa(1:79)
*********
         endif

         print*,' '
c         print*,'1        10        20        30        40        50 '//
c     .'       60        70        80     '
c         print*,'123456789*123456789*123456789*123456789*123456789*'//
c     .'123456789*123456789*123456789*123456789'
 111     write(*,'(''Insert "variable value": '',$)')
         read(*,'(A)')cinput
         k = index(cinput,' ')
         c = cinput(1:k-1)
         if (c.eq.'legenda') then 
            print*,' '
            call explainvalues(setkey)
            print*,' '
            goto 111
         endif
         if ( c.eq.'ord'.or.
     .        c.eq.'model'.or.
     .        c.eq.'path'.or.
     .        c.eq.'ntuple'.or.
     .        c.eq.'menu2'.or.
     .        c.eq.'arun'.or.
     .        c.eq.'mode'.or.
     .        c.eq.'fs'.or.
     .        c.eq.'menud'.or.
     .        c.eq.'darkmod') then
            read(cinput(k+1:),'(bn,A)')cx
            if (c.eq.'ord')    ord=cx
            if (c.eq.'model')  model=cx
            if (c.eq.'path')   path = cx
            if (c.eq.'ntuple') store = cx
            if (c.eq.'menu2')  menu2 = cx
            if (c.eq.'menud')  menud = cx
            if (c.eq.'darkmod') darkmod = cx
            if (c.eq.'arun')   arun = cx
            if (c.eq.'fs')     fs = cx
            idarkon = 0
            iarun = 0
            iteubn = 0
            inskvp = 0
            if (arun.eq.'hadr5') then
               iarun = 1
               iteubn = 0
            elseif (arun.eq.'hlmnt') then
               iarun = 1
               iteubn = 1
            elseif (arun.eq.'on'.or.arun.eq.'nsk') then
               iarun  = 1
               inskvp = 1
            else
               iarun = iarun
            endif
            if (darkmod.eq.'on') then
                idarkon = 1
                if(model.eq.'matched') model = 'ps'
            endif
            if (c.eq.'mode') then 
               if (cx.eq.'weighted')   eventlimiter = 'w'
               if (cx.eq.'unweighted') eventlimiter = 'unw'
            endif
         elseif (c.eq.'seed'.or.
     .           c.eq.'nsearch'.or.
     .           c.eq.'nwrite'.or.
     .           c.eq.'verbose'.or.
     .           c.eq.'nphot') then
            ctmp = cinput(k+1:)
            kk = index(ctmp,'.')
            if (kk.gt.0) ctmp = ctmp(:kk-1)
            read(ctmp,'(bn,i9)')ix
            if (c.eq.'seed')    iseed = ix
            if (c.eq.'nsearch') nsearch = ix
            if (c.eq.'nwrite')  iwriteout = ix
            if (c.eq.'verbose') iverbose = ix
            if (c.eq.'nphot')   nphotmode = ix
         else
            ctmp = cinput(k+1:)
            kk = index(ctmp,'.')
            read(cinput(k+1:),'(bn,f40.20)')x
            if (kk.eq.0) read(cinput(k+1:),'(bn,f40.0)')x
            if (c.eq.'ecms') then
                if(amassainvmax.eq.ecms) amassainvmax = x
                ecms = x
            endif
            if (c.eq.'thmin') then
                if(thgmin.eq.thmin) thgmin = x 
                thmin = x
            endif
            if (c.eq.'thmax') then
                if(thgmax.eq.thmax) thgmax = x 
                thmax = x
            endif
            if (c.eq.'zmax')   zmax = x
            if (c.eq.'emin')   emin = x
            if (c.eq.'nev')    anpoints = x
            if (c.eq.'eps')    eps = x
            if (c.eq.'sdmax')  sdifmax = x
            if (c.eq.'massU')  amassU = x
            if (c.eq.'gammaU') gammaU = x
            if (c.eq.'k')      gvectU = x
            if (c.eq.'gaxU')   gaxU = x
            if (c.eq.'egmin')  egmin = x
            if (c.eq.'thgmin') thgmin = x
            if (c.eq.'thgmax') thgmax = x
            if (c.eq.'massmin') amassainvmin = x
            if (c.eq.'massmax') amassainvmax = x
            if (c.eq.'scEdhad') scal_err_dhad = x
            if (c.eq.'sprb1') esig1 = x
            if (c.eq.'sprb2') esig2 = x
         endif
         if (c(1:3).eq.'run')  then
            istop = .true.
            print*,'                  '
            if (iarun.eq.1) then
               if (inskvp.eq.1) then
                  print*,'                  '
                  print*, 'Using Novosibirsk routine for Delta alpha.'
               else
               if (iteubn.lt.1) then
                  print*,'                  '
                  print*, 'Using HADR5N12 routine for Delta alpha.'
               else
                  print*,'                  '
                  print*,'Using HLMNT routine for Delta alpha.'//
     >                 ' [VP_HLMNT_v2_2 (version 2.2, 20 May 2015)]'
               endif
            endif
            endif
            print*,'                  '
            print*,'Starting generation...'
            print*,'                  '
         endif
         if (c(1:4).eq.'quit') stop
      enddo

      do k = 1,80
         if (path(k:k).ne.' ') lpath=k
      enddo
      if (path(lpath:lpath).ne.'/') then
         lpath = lpath+1
         path(lpath:lpath)='/'
      endif
      open(11,file=path,status='old',iostat=ierr)
      if (ierr.eq.0) then
         print*,' '
         print*,'WARNING:'
         print*,'The directory ',path(1:lpath),' already exists.'
         print*,'The files in it are going to be overwritten...'
         print*,' '
      endif
      close(11)
      call system('mkdir -p '//path)
      outfile  = path(:lpath)//'statistics.txt'
      vpuncout = path(:lpath)//'vpuncertainties.txt'

c     storefile = path(:lpath)//'ntuple.hbook'
      storefile = path(:lpath)//'events.dat'
      corrections=ord
      if (ord.eq.'alpha') corrections='oal'

      do i = 1,10
         if (model(i:i).ne.' ') then 
            j = i
            programma(i:i)=model(i:i)
         endif
      enddo
      programma(j+1:j+1) = '_'
      programma = path(:lpath)//programma

      ebeam = ecms/2.d0
      thmin = thmin * pi/180.d0
      thmax = thmax * pi/180.d0
      thgmin = thgmin * pi/180.d0
      thgmax = thgmax * pi/180.d0
      zmax  = zmax * pi/180.d0
c
      call setfilenames(programma,corrections)
c
      if (fs.eq.'gg'.or.fs.eq.'ee'.or.fs.eq.'mm') then
         continue
      else
         print*,'  '
         print*,'Wrong final-state selected (',fs,')'
         print*,'Select one of ''ee'',''gg'' or ''mm'''
         print*,'  '
         stop
      endif
c
!       call rluxgo(lux,iseed,k1,k2)
      nphmaxalpha = 6
c
      return
      end
cccccccccccccccccccccccccccccccccccccccccccccccc
      subroutine explainvalues(setkey)
! written by CMCC, last modified 5/3/2006
      character*(*) setkey(*)
      print*,setkey(22),'--> Final state (ee/gg/mm)'
      print*,setkey(1) ,'--> CoM energy (double)'
      print*,setkey(2) ,'--> min. l+ & l- or photons angle (double)'
      print*,setkey(3) ,'--> max. l+ & l- or photons angle (double)'
      print*,setkey(4) ,'--> max. l+l- or photons acollinearity'//
     .     ' (double)'
      print*,setkey(5) ,'--> min. l+ & l- or photons energy (double)'
      print*,setkey(6) ,'--> number of event to generate (double)'
      print*,setkey(7) ,'--> directory for outputs (char)'
      print*,setkey(8) ,'--> saving ntuple file (yes/no)'
      print*,setkey(9) ,'--> toggle second menu (on/off)'
      print*,setkey(23) ,'--> toggle dark matter menu (on/off)'
      print*,setkey(21),'--> choose alpha running routine'//
     .     ' (hadr5/hlmnt/nsk/off)'
      print*,setkey(14),'--> if nev corresponds to weighted/unweighted'
      print*,'           events (weighted/unweighted)'
      print*,setkey(15),'--> photon cutoff is k0 = eps*ebeam (double)'
      print*,setkey(11),'--> QED corrections at'//
     .     ' born/alpha/exp/struct(*)'
      print*,'           (*) struct only for darkmod on'
      print*,setkey(12),'--> model for corrections is matched/ps'
      print*,'           ("matched" means the best one)'
      print*,setkey(10),'--> seed for RANLUX (integer)'
      print*,setkey(13),'--> generates only a fixed number of photons'//
     .     ' (integer)'
      print*,'           (if < 0, an arbitrary number is generated)'
      print*,setkey(16),'--> each "nwrite" events, output is'//
     .     ' written (integer)'
      print*,setkey(17),'--> events before '//
     .     'starting also unweighted generation (integer)'
      print*,setkey(18),'--> verbose mode (0/1)'
      print*,setkey(19),'--> starting sdifmax (double)'
      print*,setkey(24),'--> activate the dark matter channel (on/off)'
      print*,setkey(25),'--> mass of the U boson (double)'
      print*,setkey(26),'--> U boson width (double)'
      print*,'           if set to -1 decay width into SM particles'//
     .' is automatically computed'
      print*,setkey(27),'--> vectorial coupling of the U boson'//
     .     ' (double)'
      print*,setkey(28),'--> axial coupling of the U boson (double)'
      print*,setkey(29),'--> min. gamma energy (double) (only for'//
     .'  darkmod on)'
      print*,setkey(30),'--> min. gamma angle (double) (only for'//
     . ' darkmod on)'
      print*,setkey(31),'--> max. gamma angle (double) (only for'//
     .     '  darkmod on)'
      print*,setkey(32),'--> min. lepton pair invariant mass (double)'//
     .     ' (only for darkmod on)'
      print*,setkey(33),'--> max. lepton pair invariant mass (double)'//
     .     ' (only for darkmod on)'
      print*,setkey(34),'--> n. of sigma variations for VP'//
     .     ' parametric uncertainty (double)'
      print*,setkey(35),'--> energy spread for electron beam (GeV)'
      print*,setkey(36),'--> energy spread for positron beam (GeV)'
      return
      end
c
      subroutine initcolors
! written by CMCC, last modified 5/3/2006
      character*12 col(20)
      common/colors/col
      data col /
     >     '\e[00;00;30m',      !1
     >     '\e[00;00;31m',      !2
     >     '\e[00;00;32m',      !3
     >     '\e[00;00;33m',      !4
     >     '\e[00;00;34m',      !5
     >     '\e[00;00;35m',      !6
     >     '\e[00;00;36m',      !7
     >     '\e[00;00;37m',      !8
     >     '\e[00;01;37m',      !9
     >     '\e[00;00;00m',      !10
     >     '\e[00;07;30m',      !11
     >     '\e[00;01;31m',      !12
     >     '\e[00;01;32m',      !13
     >     '\e[00;01;33m',      !14
     >     '\e[00;01;34m',      !15
     >     '\e[00;01;35m',      !16
     >     '\e[00;01;36m',      !17
     >     '\e[43;01;37m',      !18
     >     '\e[00;07;31m',      !19
     >     '\e[00;05;31m'/      !20
      return
      end
