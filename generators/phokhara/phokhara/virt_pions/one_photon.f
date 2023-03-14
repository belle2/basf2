       real*8 function NLOpions(qq)
       include '../phokhara_10.0.inc'     
      real*8 qq,rk1(4),q(4),dps,amp_h,ampNLO
      complex*16 gam(4),gammu(4,2,2),v1(2,2),v2(2,2),up1(2,2),up2(2,2)
     1          ,gammu_ma(4,2,2)
      integer i1,ic1,ic2
      common/pedf/rk1,dps
c
      call gam1(gam,gammu,gammu_ma,v1,v2,up1,up2,qq)
c
      do i1=1,4
         rk1(i1) = momenta(3,i1-1)
         q(i1)   = momenta(5,i1-1)
      enddo


       dps = (1.d0-qq/Sp)/(32.d0*pi*pi)        ! Phase space factors
       dps = dps*dSqrt(1.d0-4.d0*mpi*mpi/qq)/(32.d0*pi*pi) 
       amp_h = 
     1 ((4.d0*pi*alpha)**3)*ampNLO(qq,rk1,gam,q)/4.d0
      
      NLOpions=amp_h*dps

      end

      real*8 function ampNLO(qq,rk1,gam,q)
      include '../phokhara_10.0.inc'     
      complex*16 p1eck1,p1eck2,p2eck1,p2eck2,p1gam,p2gam,cvac_qq,dggvap
      complex*16 epsk1(2,4),epsk2(2,4),BW_om,vacpol_and_nr
      complex*16 gam(4),gam_ma(4),eck1(4),eck2(4)
      complex*16 ma(2,2),mb(2,2),ddpl(2,2),ddmi(2,2)
     1          ,ma_ma(2,2),mb_ma(2,2)
      complex*16 uupp1(1,2),uupp2(1,2),vv1(2,1),vv2(2,1)
      integer i
      real*8 qq,dme,el_m2,ebppb,vacuumpolarization,vertLO,ver_f
      real*8 rk1(4),rk2(4),p1(4),p2(4),q(4),aa_phi,mm_ph,gg_ph
      real*8 amp_fullLO,amp_add
      real*8 softint_p1p2,softint_pjpj,softint_q1q2,softint_qjqj,
     1 softint_sum
      real*8 t3,t2,q2,m2,coll,FSRamp,ISRamp
c
      common/iloczs1/p1eck1,p1eck2,p2eck1,p2eck2,p1gam,p2gam
      common/matri1/ma,mb,ma_ma,mb_ma
      common /cp1p2/p1,p2,dme,el_m2
      common/param_PFF/aa_phi,mm_ph,gg_ph
c     
      ampNLO = 0.d0
      amp_fullLO=0.d0
      FSRamp=0.d0
      ISRamp=0.d0
      call pol_vec(rk1,epsk1)
      call skalar1LO(rk1)
      call skalar1aLO(gam,gam_ma)
      ebppb = p1(1)+p1(4)

      if(fsrnlo.eq.1)then
       ver_f = vertLO(qq)
      endif
c
c vacuum polarization
c
      cvac_qq = vacpol_and_nr(qq)
c
c sum over photon polarizations
c
      do i=1,2
                  eck1(1)=epsk1(i,1)
                  eck1(2)=epsk1(i,2)
                  eck1(3)=epsk1(i,3)
                  eck1(4)=epsk1(i,4)
c
           call skalar2LO(rk1,eck1)
           call blocksLO(qq)
           call ddvec(rk1,eck1,uupp1,uupp2,vv1,vv2,ddpl,ddmi,qq)


         amp_fullLO=amp_fullLO+( (dme*cdabs(mb(1,1)-ma(1,1)))**2 !ISR
     1          +(dme*cdabs(mb(2,2)-ma(2,2)))**2
     2          +(cdabs(-ebppb*ma(2,1)+el_m2/ebppb*mb(2,1)))**2
     3          +(cdabs(ebppb*mb(1,2)-el_m2/ebppb*ma(1,2)))**2 )
     4          *cdabs(cvac_qq)**2 
c         ISRamp=ISRamp+( (dme*cdabs(mb(1,1)-ma(1,1)))**2 !ISR
c     1          +(dme*cdabs(mb(2,2)-ma(2,2)))**2
c     2          +(cdabs(-ebppb*ma(2,1)+el_m2/ebppb*mb(2,1)))**2
c     3          +(cdabs(ebppb*mb(1,2)-el_m2/ebppb*ma(1,2)))**2 )
c     4          *cdabs(cvac_qq)**2 

        amp_fullLO=amp_fullLO+ ((dme*cdabs(ddpl(1,1)-ddmi(1,1)))**2 !FSR
     1         +(dme*cdabs(ddpl(2,2)-ddmi(2,2)))**2
     2         +(cdabs(-ebppb*ddmi(2,1)+el_m2/ebppb*ddpl(2,1)))**2
     3         +(cdabs(ebppb*ddpl(1,2)-el_m2/ebppb*ddmi(1,2)))**2)
     4         *cdabs(cvac_s)**2                

          amp_fullLO=amp_fullLO + 2.d0*dreal(                    !INT
     5    dconjg(dme*(mb(1,1)-ma(1,1))*cvac_qq)
     5              *dme*(ddpl(1,1)-ddmi(1,1))*cvac_s
     6   +dconjg(dme*(mb(2,2)-ma(2,2))*cvac_qq)
     6              *dme*(ddpl(2,2)-ddmi(2,2))*cvac_s
     7   +dconjg((-ebppb*ma(2,1)+el_m2/ebppb*mb(2,1))*cvac_qq)*
     8    (-ebppb*ddmi(2,1)+el_m2/ebppb*ddpl(2,1))*cvac_s
     9   +dconjg((ebppb*mb(1,2)-el_m2/ebppb*ma(1,2))*cvac_qq)*
     1    (ebppb*ddpl(1,2)-el_m2/ebppb*ddmi(1,2))*cvac_s )


c       FSRamp=FSRamp+ ((dme*cdabs(ddpl(1,1)-ddmi(1,1)))**2 !FSR
c     1         +(dme*cdabs(ddpl(2,2)-ddmi(2,2)))**2
c     2         +(cdabs(-ebppb*ddmi(2,1)+el_m2/ebppb*ddpl(2,1)))**2
c     3         +(cdabs(ebppb*ddpl(1,2)-el_m2/ebppb*ddmi(1,2)))**2)
c     4         *cdabs(cvac_s)**2

      enddo


             call softintegral_p1p2(softint_p1p2)
             call softintegral_pjpj(softint_pjpj)
             call softintegral_q1q2_sub(softint_q1q2)
             call softintegral_qjqj_sub(softint_qjqj)
             call softintegral_interference(softint_sum)

         
         ampNLO=amp_fullLO*2.d0*
     1  ( softint_pjpj - softint_p1p2
     1  + softint_qjqj - softint_q1q2 
     2  + softint_sum)


      
      return
      end
