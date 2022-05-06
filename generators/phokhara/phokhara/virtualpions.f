      real*16 function virt_pions(qq,p1,q1,p2,q2,p4,check) 
      include 'phokhara_10.0.inc'
      real*16    q1(0:3),q2(0:3),p1(0:3),p2(0:3),p3(0:3),p4(0:3)
      real*8    q1d(0:3),q2d(0:3),p1d(0:3),p2d(0:3),p3d(0:3),p4d(0:3),
     1 musqd
      real*16    musq,dps,qq,delta_2,factor1,factor2
      real*16    rk1(4),p3fsr(0:3)
      real*16    ISR_Born,born_tr
      complex*32 hel_test,tr_test
      real*16 trace_result,rel_diff,soft   
      complex*32 barpsi_q2_MuT(4),psi_q1_MuT(4)
      complex*32 barpsi_p1_MET(4),psi_p4_MET(4)
      complex*32 virt_finite,Fs,Fq
      complex*32 result_penta_ISR,resultn_penta_ISR(6)
      complex*32 result_penta_ISRcr,resultn_penta_ISRcr(6)
      complex*32 result_penta_FSR,resultn_penta_FSR(7)
      complex*32 result_penta_FSRcr,resultn_penta_FSRcr(7)
      complex*32 resultB_penta_FSR,resultB_penta_ISR
      complex*16 result_vertex_FSR,resultn_vertex_FSR(12),
     1 resultb_vertex_FSR
      complex*16 result_vertex_FSRcr,resultn_vertex_FSRcr(12),
     1 resultb_vertex_FSRcr,result_virtual_FSR
      complex*32 mup2(0:3)
      complex*32 result_penta_ISR_div,
     1 resultn_penta_ISR_div,resultB_penta_ISR_div
      complex*32 result_penta_ISRcr_div,
     1 resultn_penta_ISRcr_div,resultB_penta_ISRcr_div
      complex*32 result_penta_FSR_div,
     1 resultn_penta_FSR_div,resultB_penta_FSR_div,
     1 result_penta_FSRcr_div,
     1 resultn_penta_FSRcr_div,resultB_penta_FSRcr_div
      complex*32 dummy
      complex*16 PionFormFactor
      real*8 check
      integer ic1,ic2,i,ii1,ii2,i1,k,jlk,jkl
c Declarations for helicity amplitudes
      integer elh(2),posh(2),phoh(2),nom,nom2
      real*8 rk1d(4)
      Complex*16 epsk1(2,4),uel1(2,2),uel2(2,2),vel1(2,2),vel2(2,2),
     1 barpsi_p2_MET(4),psi_p3_MET(4),temp1,temp,eck1(4)
      Complex*16 mup3(0:3)
      Complex*16 result_virtual_FSR_mat_born(2,2,2),
     1 result_virtual_FSR_mat(2,2,2),full_res_virtual_FSR,full_res_born
      Complex*16 result_vertex_FSR2,resultn_vertex_FSR2(12),
     1 resultb_vertex_FSR2
      Complex*16 result_virtual_FSRcr_mat_born(2,2,2),
     1 result_virtual_FSRcr_mat(2,2,2)
      Complex*16 result_vertex_FSR2cr,resultn_vertex_FSR2cr(12),
     1 resultb_vertex_FSR2cr
      real*8 dotestu(2,2,2),dotestu_fin
      complex*16 pol_sum
c QUAD
      Complex*32 epsk1q(2,4),uel1q(2,2),uel2q(2,2),vel1q(2,2),
     1 vel2q(2,2),
     1 barpsi_p2_METq(4),psi_p3_METq(4),temp1q,tempq,eck1q(4)
      Complex*32 mup3q(0:3)
      Complex*32 result_virtual_FSR_mat_bornq(2,2,2),
     1 result_virtual_FSR_matq(2,2,2),full_res_virtual_FSRq,
     1 full_res_bornq
      Complex*32 result_vertex_FSR2q,resultn_vertex_FSR2q(12),
     1 resultb_vertex_FSR2q
      Complex*16 vacpol_and_nr
  


c---------------------------------------------------------------------
       common/fsfq/Fs,Fq



       Fs=PionFormFactor(Sp)*vacpol_and_nr(Sp)!COMPLEX(1.q0,0.q0)!
       Fq=PionFormFactor(Dble(qq))*vacpol_and_nr(Dble(qq))!COMPLEX(1.q0,0.q0)!
c       Fq=COMPLEX(0.q0,0.q0)!


c be careful with photon signs - minus beceause all                                                                  
c pion lines are outgoing

c q1 - pi+
      q1(0)=-q1(0)
      q1(1)=-q1(1)
      q1(2)=-q1(2)
      q1(3)=-q1(3)
c
c  q2 - pi-
      q2(0)=-q2(0)
      q2(1)=-q2(1)
      q2(2)=-q2(2)
      q2(3)=-q2(3)

!       photon momenta p2:
      p2(0)=-p2(0)
      p2(1)=-p2(1)
      p2(2)=-p2(2)
      p2(3)=-p2(3)




      rk1(1)=-p2(0)
      rk1(2)=-p2(1)
      rk1(3)=-p2(2)
      rk1(4)=-p2(3)



!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
c ATTENTION mu scale 
      musq=Sp_q
c      musq=20.q0
c      
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

      dps = (1.q0-qq/Sp_q)/(32.q0*piq*piq)
      dps=dps*sqrt(1.q0-4.q0*mpi_q*mpi_q/qq)/(32.q0*piq*piq)
      

       factor1=alpha_q/4.q0/piq
       call Pentabox_PIFSRHelSum(me_q,mpi_q,p1,q1,p2,q2,p4,psi_p4_MET,
     2 barpsi_p1_MET
     1 ,mup2,musq,1,result_penta_FSR,
     1 resultn_penta_FSR,resultB_penta_FSR)

       call cross_FSRHelSum(me_q,mpi_q,p1,q2,p2,q1,p4,psi_p4_MET                                                  
     &   ,barpsi_p1_MET                         
     &   ,mup2
     &   ,musq,1
     &   ,result_penta_FSRcr,resultn_penta_FSRcr,dummy)

      call Pentabox_full_isrHelSum(me_q,mpi_q,p2,p1,q1,q2,p4,psi_p4_MET,
     2 barpsi_p1_MET
     1 ,mup2,musq,1,result_penta_ISR,
     1 resultn_penta_ISR,resultB_penta_ISR)

      call cross_ISRHelSum(me_q,mpi_q,p2,p1,q2,q1,p4,psi_p4_MET,
     2 barpsi_p1_MET
     1 ,mup2,musq,1,result_penta_ISRcr,
     1 resultn_penta_ISRcr,dummy)

      do jkl=0,3
        q1d(jkl)=dble(q1(jkl))
        q2d(jkl)=dble(q2(jkl))
        p2d(jkl)=dble(p2(jkl))
        p1d(jkl)=dble(p1(jkl))
        p4d(jkl)=dble(p4(jkl))
      enddo

      musqd=dble(musq)

      call virtual_FSRHelSum(me,mpi,q2d,p1d,p4d,p2d,q1d
     1   ,musqd,1,0,result_vertex_FSR
     2,resultn_vertex_FSR,resultb_vertex_FSR)

c cross
        call virtual_FSRHelSumcr(me,mpi,q1d,p1d,p4d,p2d,q2d
     1   ,musqd,1,1,result_vertex_FSRcr
     2,resultn_vertex_FSRcr,resultb_vertex_FSRcr)

      result_virtual_FSR=result_vertex_FSR+result_vertex_FSRcr


       trace_result=(4.q0*piq*alpha_q)**3*factor1*
     1 2.q0*REALPART((result_penta_FSRcr+result_penta_FSR)*CONJG(Fs)+
     2 (result_penta_ISR+result_penta_ISRcr)*CONJG(Fq))/4.q0
     3 +REALPART(result_virtual_FSR)*CONJG(Fs)*32.q0*piq**2*alpha_q**4
c       print*,'penta=',trace_result

       
       virt_pions=trace_result*dps

      return
      end

c-------------------------------------------------------------------
