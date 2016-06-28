      implicit none
      double complex qlI1,qlI2,qlI3,qlI4,Ival(-2:0)
      double precision p1sq,p2sq,p3sq,p4sq,s12,s23,
     . m1sq,m2sq,m3sq,m4sq,musq 
      integer ep
      call qlinit
      musq=1.1d0
      m1sq=3d0
      m2sq=5d0
      m3sq=9d0
      m4sq=1.3d0
      p1sq=1.5d0
      p2sq=-1.7d0
      p3sq=2.3d0
      p4sq=2.9d0
      s12=37d0
      s23=-15.7d0
      do ep=-2,0
      Ival(ep)=qlI1(m1sq,musq,ep)
      write(6,*) 'ep,qlI1(m1sq,musq,ep)',ep,Ival(ep)
      enddo
      write(6,*)
      do ep=-2,0
      Ival(ep)=qlI2(p1sq,m1sq,m2sq,musq,ep)
      write(6,*) 'ep,qlI2(p1sq,m1sq,m2sq,musq,ep)',
     .   ep,Ival(ep)
      enddo
      write(6,*)
      do ep=-2,0
      Ival(ep)=qlI3(p1sq,p2sq,p3sq,m1sq,m2sq,m3sq,musq,ep)      
      write(6,*) 'ep,qlI3(p1sq,p2sq,p3sq,m1sq,m2sq,m3sq,musq,ep)',
     . ep,Ival(ep)
      enddo
      write(6,*)
      do ep=-2,0
      Ival(ep)=
     . qlI4(p1sq,p2sq,p3sq,p4sq,s12,s23,m1sq,m2sq,m3sq,m4sq,musq,ep)
      write(6,*) 
     .'ep,qlI4(p1sq,p2sq,p3sq,p4sq,s12,s23,m1sq,m2sq,m3sq,m4sq,musq,ep)'
     . ,ep,Ival(ep)
      enddo


      write(6,*)
      write(6,*) 'test of divergent boxes'
      do ep=-2,0
      Ival(ep)=
     . qlI4(0d0,0d0,p3sq,p4sq,s12,s23,0d0,0d0,0d0,m4sq,musq,ep)
      write(6,*) 
     .'ep,qlI4(0d0,0d0,p3sq,p4sq,s12,s23,0d0,0d0,0d0,m4sq,musq,ep)'
     . ,ep,Ival(ep)
      enddo

      stop
      end
