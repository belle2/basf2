      program fortranmain
      implicit none

      include "pjfry.f"

      REAL*8 ps1,ps2,ps3,ps4,ps5
      REAL*8 ms1,ms2,ms3,ms4,ms5
      REAL*8 s12,s23,s34,s45,s15 

!     set renormalization scale mu^2
      call psetmusq(36D0)
      write(*,*) "mu^2=", pgetmusq()

      write (*,*) "-----------------------------------------------"

      write (*,*) "2-mass point (m1 m1 -> m2 m2 0)"
      ps1=4
      ps2=4
      ps3=25
      ps4=25
      ps5=0
      s12= 0.10000000000000000D+05
      s23=-0.20042636597603382D+04
      s34= 0.10413130839415544D+04
      s45= 0.61341040415443358D+04
      s15=-0.33860497881996525D+04
      ms1=0
      ms2=4
      ms3=0
      ms4=25
      ms5=0


      print "(A5, e25.17)", "ps1 =", ps1
      print "(A5, e25.17)", "ps2 =", ps2
      print "(A5, e25.17)", "ps3 =", ps3
      print "(A5, e25.17)", "ps4 =", ps4
      print "(A5, e25.17)", "ps5 =", ps5

      print "(A5, e25.17)", "s12 =",s12
      print "(A5, e25.17)", "s23 =",s23
      print "(A5, e25.17)", "s34 =",s34
      print "(A5, e25.17)", "s45 =",s45
      print "(A5, e25.17)", "s15 =",s15

      print "(A5, e25.17)", "ms1 =",  ms1
      print "(A5, e25.17)", "ms2 =", ms2
      print "(A5, e25.17)", "ms3 =", ms3
      print "(A5, e25.17)", "ms4 =", ms4
      print "(A5, e25.17)", "ms5 =", ms5


      write (*,*) "E0(0)",PE0(ps1,ps2,ps3,ps4,ps5,
     & s12, s23, s34, s45, s15, ms1,ms2,ms3,ms4,ms5,0)
      write (*,*) "E0(-1)",PE0(ps1,ps2,ps3,ps4,ps5,
     & s12, s23, s34, s45, s15, ms1,ms2,ms3,ms4,ms5,1)
      write (*,*) "E0(-2)",PE0(ps1,ps2,ps3,ps4,ps5,
     & s12, s23, s34, s45, s15, ms1,ms2,ms3,ms4,ms5,2)
      write (*,*) "E1(0)",PE0i(1, ps1,ps2,ps3,ps4,ps5,
     & s12, s23, s34,s45, s15, ms1,ms2,ms3,ms4,ms5,0)
      write (*,*) "E2(0)",PE0i(2, ps1,ps2,ps3,ps4,ps5,
     & s12, s23, s34, s45, s15, ms1,ms2,ms3,ms4,ms5,0)

      write (*,*) "E00(0)",PE0ij(0,0, ps1,ps2,ps3,ps4,ps5,
     & s12, s23, s34, s45, s15, ms1,ms2,ms3,ms4,ms5,0)
      write (*,*) "E11(0)",PE0ij(1,1, ps1,ps2,ps3,ps4,ps5,
     & s12, s23, s34, s45, s15, ms1,ms2,ms3,ms4,ms5,0)
      write (*,*) "E12(0)",PE0ij(1,2, ps1,ps2,ps3,ps4,ps5,
     & s12, s23, s34, s45, s15, ms1,ms2,ms3,ms4,ms5,0)

      write (*,*) "E001(0)",PE0ijk(0,0,1,ps1,ps2,ps3,ps4,ps5,
     & s12, s23, s34, s45, s15, ms1,ms2,ms3,ms4,ms5,0)
      write (*,*) "E112(0)",PE0ijk(1,1,2,ps1,ps2,ps3,ps4,ps5,
     & s12, s23, s34, s45, s15, ms1,ms2,ms3,ms4,ms5,0)
      write (*,*) "E12(0)3",PE0ijk(1,2,3,ps1,ps2,ps3,ps4,ps5,
     & s12, s23, s34, s45, s15, ms1,ms2,ms3,ms4,ms5,0)

      write (*,*) "D111(0)",PD0ijk(1,1,1,s12,ps3,ps4,ps5,
     &s45,s34,ms1,ms3,ms4,ms5,0)
      write (*,*) "D111(-1)",PD0ijk(1,1,1,s12,ps3,ps4,ps5,
     &s45,s34,ms1,ms3,ms4,ms5,1)
      write (*,*) "D111(-2)",PD0ijk(1,1,1,s12,ps3,ps4,ps5,
     &s45,s34,ms1,ms3,ms4,ms5,2)
      write (*,*) "D1233(0)",PD0ijkl(1,2,3,3,s12,ps3,ps4,ps5,
     &s45,s34,ms1,ms3,ms4,ms5,0)

      write (*,*) "C122(0)",PC0ijk(1,2,2,s45,ps4,ps5,ms1,ms4,ms5,0)


      write (*,*) "-----------------------------------------------"

      write (*,*) "0 mass, small ()_4 point"
      ps1=0
      ps2=0
      ps3=0
      ps4=0
      ps5=0
      s12=25
      s23=-24.73938641761069D0
      s34=24.52647702751698D0
      s45=0.2605610444840831D0
      s15=-0.2145177937292013D0
      ms1=0
      ms2=0
      ms3=0
      ms4=0
      ms5=0

      print "(A5, e25.17)", "ps1 =", ps1
      print "(A5, e25.17)", "ps2 =", ps2
      print "(A5, e25.17)", "ps3 =", ps3
      print "(A5, e25.17)", "ps4 =", ps4
      print "(A5, e25.17)", "ps5 =", ps5

      print "(A5, e25.17)", "s12 =",s12
      print "(A5, e25.17)", "s23 =",s23
      print "(A5, e25.17)", "s34 =",s34
      print "(A5, e25.17)", "s45 =",s45
      print "(A5, e25.17)", "s15 =",s15

      print "(A5, e25.17)", "ms1 =",  ms1
      print "(A5, e25.17)", "ms2 =", ms2
      print "(A5, e25.17)", "ms3 =", ms3
      print "(A5, e25.17)", "ms4 =", ms4
      print "(A5, e25.17)", "ms5 =", ms5

      write (*,*) "E",PE0(ps1,ps2,ps3,ps4,ps5,
     & s12, s23, s34, s45, s15, ms1,ms2,ms3,ms4,ms5,0)
      write (*,*) "E1",PE0i(1,ps1,ps2,ps3,ps4,ps5,
     & s12, s23, s34, s45, s15, ms1,ms2,ms3,ms4,ms5,0)
      write (*,*) "E12",PE0ij(1,2,ps1,ps2,ps3,ps4,ps5,
     & s12, s23, s34, s45, s15, ms1,ms2,ms3,ms4,ms5,0)
      write (*,*) "E123",PE0ijk(1,2,3,ps1,ps2,ps3,ps4,ps5,
     & s12, s23, s34, s45, s15, ms1,ms2,ms3,ms4,ms5,0)
      write (*,*) "E1234",PE0ijkl(1,2,3,4,ps1,ps2,ps3,ps4,ps5,
     & s12, s23, s34, s45, s15, ms1,ms2,ms3,ms4,ms5,0)
      write (*,*) "E00234",PE0ijklm(0,0,2,3,4,ps1,ps2,ps3,ps4,ps5,
     & s12, s23, s34, s45, s15, ms1,ms2,ms3,ms4,ms5,0)

      end
