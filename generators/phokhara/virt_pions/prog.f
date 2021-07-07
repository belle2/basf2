c      program testheh
      subroutine testheh()
      implicit none
c      include '/home/szymontracz/PH9.2/eemmg-lib/ql/qlconstants.f'
      double complex qlI2
      double precision a1,a2,a3,a4,mpi,me,MESQ,musq
      integer ep
      
c      call qlinit
      mpi=0.13957018d0
      me=0.51099906d-3
      musq=1.02d0**2
      MESQ=ME**2
      a1=1000.d0
      a2=50.d0
      a3=80.d0
      a4=1.d0
      ep=0
      print*, 'a4=',a4,musq
      print*, qlI2(0.d0,MESQ,MESQ,musq,0)
      print*, qlI2(a1,a2,a3,a4,ep)


      end
