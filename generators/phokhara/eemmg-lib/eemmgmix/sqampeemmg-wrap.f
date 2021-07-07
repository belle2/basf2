! Valery Yundin 2011
!
! Process: q1(e+) q2(e-) -> q3(mu+) q4(mu-) q5(gamma)
! contrubution of Re(|mixed triangles|*|tree|^+)
!
! Wrapper function for eemmgsqampmix, do channel crossing, etc
!
!  to get phokhara 'amplit' as in amplit = Matrix(Leptonic,Hadronic)
!  do the following:
!       call eemmgloopmix(loop,tree,p1,p2,p3,p4,p5,mle2,mlm2,mu2,eps)
!       dps1=(1.d0-qq/Sp)/(32.d0*pi*pi)
!       dps2=dSqrt(1.d0-4.d0*mmu*mmu/qq)/(32.d0*pi*pi)
!       loop=loop*0.25D0*dps1*dps2*(4*pi*alpha)**3*(alpha/(2*pi))
!       tree=tree*0.25D0*dps1*dps2*(4*pi*alpha)**3
!
! 0.25            - spin average
! dps1*dps2       - phase space factors
! (4*pi*alpha)**3 - LO factor
! (alpha/(2*pi))  - NLO factor
!
! OUTPUT:
! loop, tree
!
! INPUT:
! q1,q2,q3,q4,q5 particle momenta
! mle2 - m_e^2 electrom mass squared
! mlm2 - m_mu^2 muon mass squared
! mu2  - mu_R^2 regularization scale
! eps 0 - finite part, 1 - pole
!
      subroutine eemmgloopmix(loop,tree,q1,q2,q3,q4,q5,mle2,mlm2,
     &mu2,eps)
      implicit none
      double complex loop,tree
      double precision q1(0:3),q2(0:3),q3(0:3),q4(0:3),q5(0:3)
      double precision mle2,mlm2,mu2
      integer eps,i

      double precision p1(0:3),p2(0:3),p3(0:3),p4(0:3),p5(0:3)
      double complex amp1,amp0

!     eemmgsqamp takes 0 -> p1(e-),p2(e+),p3(mu-),p4(gamma),p5(mu+)
!     we cross e- and e+
!     p1(e-)=-q1(e+), p2(e+)=-q1(e-), p3=q4(mu-), p4=q5(gamma), p5=q3(mu+)
      do i=0,3
      p1(i)=-q1(i)
      p2(i)=-q2(i)
      p3(i)=q4(i)
      p4(i)=q5(i)
      p5(i)=q3(i)
      enddo

      call eemmgsqampmix(amp1,amp0,p1,p2,p3,p4,p5,mle2,mlm2,mu2,eps)

      loop=amp1
      tree=amp0

      end
