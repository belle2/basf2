      character*3 LEPTONflag
      integer iLEP,iwarnings,LFLAG
      real *8 pi,pi2,ln2,zeta2,zeta3,zeta5
      real *8 alp,adp,adp2,adp3,ml(3)
      real *8 small,large,large_3
      real *8 st2,als,mtop
      real *8 dalept,dahadr,daltop
      common /consts/pi,pi2,ln2,zeta2,zeta3,zeta5
      common /params/ALP,adp,adp2,adp3,ml
      common /switch/small,large,large_3
      common /lepton/iLEP,iwarnings,LEPTONflag
      common /parm/st2,als,mtop
      common /resu/dalept,dahadr,daltop
