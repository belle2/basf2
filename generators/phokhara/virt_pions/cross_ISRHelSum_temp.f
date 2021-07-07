       subroutine cross_ISRHelSum(ME,MU,p1,p2,p3,p4,p5,psi_p5_ME,barps
     -   i_p2_ME,mup1,musq,comp,result,resultn,resultb)
c ************************************************************************************
c Author: Francisco Campanario
c E-mail: francam@particle.uni-karlsruhe.de
c Date: 17/2/2010
c Modified:12/10/2017
c ************************************************************************************
c determine the  finite virtual corrections along the lepton line i.e 
c the sum of all virtual corrections to the Born diagram 
c psi(p1,ME)    ---->------->------->---- bar_psi(p4,ME)
c                   $       $                    
c                   $       $                     
c                   $       $                     
c                   V       V                     
c                   $       $                     
c                   $       $                     
c                  p2       p3                   
c                 mu_p2    mu_p3              
c Note: The vertices are just Gamma^(mu_p2)..terms. So the correct
c factor should be added by hand for each boson. i.e, Without adding
c anything it represents ee->2 photons up to the electromagnetic charge 
c For W and Z,
c we have to account for the Diracgamma_5 that are missing in the vertex
c Note: To make it shorter in the promgram: mu_p2,...->mup2,... 
c Notation of External momenta: p1+p2+p3+p4=0 
c mu_p2,mu_p3, should be think as external current 
c musq is the renormalization scale energy  
c comp: integer value.The first time called with p1...p4, comp=1
c ATTENTION: ONLY!!!If you have to call the subroutine consecutively 
c with the same arguments(p1,p2,p3,p4). Then, comp=-1 (it safes some lines of code) 
c OUTPUT:
c        result:result of the boxline 
c        resulb:result of the born amplitude 
c************************************************************************************
c************************************************************************************
c************************************************************************************
c************************************************************************************
c************************************************************************************
c************************************************************************************
c************************************************************************************
c************************************************************************************
c    Declaration of variables 
c************************************************************************************
c************************************************************************************
c************************************************************************************
c************************************************************************************
c************************************************************************************
c************************************************************************************
c************************************************************************************
c************************************************************************************
       IMPLICIT NONE
       complex*16 qlI1,qlI2,qlI3,qlI4,Ival(-2:0)
       Real*16 p1(0:3), p2(0:3), p3(0:3), p4(0:3), p5(0:3)
       Real*16   p1p2, p1p3, p1p4, p1p5, p1sq, p2p3, p2p4, p2p5, 
     -          p2sq, p3p4, p3p5, p3sq, p4p5, p4sq, p5sq
       Real*16   s12, s13, s14, s15, s23, s24, s25, s34, s35, 
     -          s45
       Real*8   p1p2d, p1p3d, p1p4d, p1p5d, p1sqd, p2p3d, p2p4d, p2p5d, 
     -          p2sqd, p3p4d, p3p5d, p3sqd, p4p5d, p4sqd, p5sqd
       Real*8   s12d, s13d, s14d, s15d, s23d, s24d, s25d, s34d, s35d, 
     -          s45d
       Complex*32   p1mup1, p1mup2, p1mup3, p2mup1, p2mup2, p2mup3, 
     -          p3mup1, p3mup2, p3mup3, p4mup1, p4mup2, p4mup3, 
     -          p5mup1, p5mup2, p5mup3
       Complex*32 mup2mup3,mup1(0:3)
       Real*16 dotrr_QUAD
       Complex*32 dotrc_QUAD,dotcc_QUAD
      EXTERNAL dotrr_QUAD,dotrc_QUAD,dotcc_QUAD
       Complex*32 result,resultn(6),resultb
       Complex*32 A0finG1_QUAD,A0finG2_QUAD
       Complex*32 B0finG1_QUAD,B0finG2_QUAD,B0finG3_QUAD
       Complex*32 B0finG4_QUAD,B0finG5_QUAD,B0finG6_QUAD
       Complex*32 C0finG1_QUAD,C0finG2_QUAD
       Complex*32 D0finG1_QUAD,D0finG2_QUAD,D0finG3_QUAD
       Complex*32 E0finG_QUAD
       External A0finG1_QUAD,A0finG2_QUAD
       External B0finG1_QUAD,B0finG2_QUAD,B0finG3_QUAD
       External B0finG4_QUAD,B0finG5_QUAD,B0finG6_QUAD
       External C0finG1_QUAD,C0finG2_QUAD
       External D0finG1_QUAD,D0finG2_QUAD,D0finG3_QUAD
       External E0finG_QUAD
       Complex*16 A01,A02,A03,A04,A05
       Complex*32 A01q,A02q,A03q,A04q,A05q
      Real*16 A01R,A02R,A03R,A04R,A05R
      Real*16 A01I,A02I,A03I,A04I,A05I
      Complex*16 B012,B013,B014,B015,B023,B024,B025,B034,B035,B045
      Complex*32 B012q,B013q,B014q,B015q,B023q,B024q,B025q,B034q,
     1 B035q,B045q
      Real*16 B012R,B013R,B014R,B015R,B023R,B024R,B025R,B034R,
     - B035R,B045R
      Real*16 B012I,B013I,B014I,B015I,B023I,B024I,B025I,B034I,
     - B035I,B045I
      Real*16 Bij12R,Bij13R,Bij14R,Bij15R,Bij23R,Bij24R,Bij25R,Bij34R,
     - Bij35R,Bij45R
      Real*16 Bij12I,Bij13I,Bij14I,Bij15I,Bij23I,Bij24I,Bij25I,Bij34I,
     - Bij35I,Bij45I
       Complex*16 C0123,C0124,C0125,C0134,C0135,C0145,C0234,C0235,C024
     -   5,C0345
       Complex*32 C0123q,C0124q,C0125q,C0134q,C0135q,C0145q,C0234q,
     - C0235q,C0245q,C0345q
       Real*16 C0123R,C0124R,C0125R,C0134R,C0135R,C0145R,C0234R,C0235R
     -   ,C0245R,C0345R
       Real*16 C0123I,C0124I,C0125I,C0134I,C0135I,C0145I,C0234I,C0235I
     -   ,C0245I,C0345I
       Real*16 Cij123R(4,2),Cij124R(4,2),Cij125R(4,2),Cij134R(4,2),Cij
     -   135R(4,2),Cij145R(4,2),Cij234R(4,2),Cij235R(4,2),Cij245R(4,2
     -   ),Cij345R(4,2)
       Real*16 Cij123I(4,2),Cij124I(4,2),Cij125I(4,2),Cij134I(4,2),Cij
     -   135I(4,2),Cij145I(4,2),Cij234I(4,2),Cij235I(4,2),Cij245I(4,2
     -   ),Cij345I(4,2)
       Complex*16 D01234,D01235,D01245,D01345,D02345
       Complex*32 D01234q,D01235q,D01245q,D01345q,D02345q,EE0
        Real*16 D01234R,D01235R,D01245R,D01345R,D02345R,EE0R
        Real*16 D01234I,D01235I,D01245I,D01345I,D02345I,EE0I
        Real*16 Dij1234R(13,3),Dij1235R(13,3),Dij1245R(13,3),Dij1345R(1
     -   3,3),Dij2345R(13,3),EijR(46,4)
        Real*16 Dij1234I(13,3),Dij1235I(13,3),Dij1245I(13,3),Dij1345I(1
     -   3,3),Dij2345I(13,3),EijI(46,4)
       Complex*32 SMB(8), SMB1(16),F(49),Fa(8),K(0)
       Real*16 FR(49),FI(49),KR(0),KI(0)
       Complex*32 barpsi_p2_ME(4),psi_p5_ME(4),mup2(0:3),mup3(0:3) 
       Complex*32 barpsi_p2_ME_M(2),psi_p5_ME_M(2)
       Complex*32 barpsi_p2_ME_P(2),psi_p5_ME_P(2)
       Complex*32 SC0_QUAD,SC1c_QUAD,SC1r_QUAD,SC2cc_QUAD,SC2rr_QUAD
     -  ,SC2rc_QUAD,SC3rcc_QUAD,SC3rrc_QUAD
       Real*16 ME,MU,musq, P(79) 
       Real*16 MESQ,ME3SQ,ME4SQ,ME6SQ,MU2SQ,MU3SQ,MU4SQ,MU6SQ,INVs12M,
     -   INVs15M
       real*8 MESQd,MU2SQd,musqd
       Complex*32 ssav(42),ssa(20),hel(7)
       EXTERNAL   SC0_QUAD,SC1c_QUAD,SC1r_QUAD,SC2cc_QUAD,SC2rr_QUAD
     -            ,SC2rc_QUAD,SC3rcc_QUAD,SC3rrc_QUAD
       Integer comp 
       common/cross_ISRHelSumFfunctionsmm/F,P
       SAVE/cross_ISRHelSumFfunctionsmm/
      Complex*32 Fs,Fq
       common/fsfq/fs,fq
c************************************************************************************
c************************************************************************************
c************************************************************************************
c************************************************************************************
c************************************************************************************
c************************************************************************************
c************************************************************************************
c************************************************************************************
c       Definition of the scalar products. Not inlcueded the contraction of the
c       moments with the external currents  
c************************************************************************************
c************************************************************************************
c************************************************************************************
c************************************************************************************
c************************************************************************************
c************************************************************************************
c************************************************************************************
       p1sq = dotrr_QUAD(p1,p1)
       p1p2 = dotrr_QUAD(p1,p2)
       p1p3 = dotrr_QUAD(p1,p3)
       p1p4 = dotrr_QUAD(p1,p4)
       p1p5 = dotrr_QUAD(p1,p5)
       p2sq = dotrr_QUAD(p2,p2)
       p2p3 = dotrr_QUAD(p2,p3)
       p2p4 = dotrr_QUAD(p2,p4)
       p2p5 = dotrr_QUAD(p2,p5)
       p3sq = dotrr_QUAD(p3,p3)
       p3p4 = dotrr_QUAD(p3,p4)
       p3p5 = dotrr_QUAD(p3,p5)
       p4sq = dotrr_QUAD(p4,p4)
       p4p5 = dotrr_QUAD(p4,p5)
       p5sq = dotrr_QUAD(p5,p5)
       s12 = (p1sq +p2sq+ 2*p1p2) 
       s13 = (p1sq +p3sq+ 2*p1p3) 
       s14 = (p1sq +p4sq+ 2*p1p4) 
       s15 = (p1sq +p5sq+ 2*p1p5) 
       s23 = (p2sq +p3sq+ 2*p2p3) 
       s24 = (p2sq +p4sq+ 2*p2p4) 
       s25 = (p2sq +p5sq+ 2*p2p5) 
       s34 = (p3sq +p4sq+ 2*p3p4) 
       s35 = (p3sq +p5sq+ 2*p3p5) 
       s45 = (p4sq +p5sq+ 2*p4p5) 

       p1sqd = dble(p1sq)
       p1p2d = dble(p1p2)
       p1p3d = dble(p1p3)
       p1p4d = dble(p1p4)
       p1p5d = dble(p1p5)
       p2sqd = dble(p2sq)
       p2p3d = dble(p2p3)
       p2p4d = dble(p2p4)
       p2p5d = dble(p2p5)
       p3sqd = dble(p3sq)
       p3p4d = dble(p3p4)
       p3p5d = dble(p3p5)
       p4sqd = dble(p4sq)
       p4p5d = dble(p4p5)
       p5sqd = dble(p5sq)
       s12d = (p1sqd +p2sqd+ 2*p1p2d) 
       s13d = (p1sqd +p3sqd+ 2*p1p3d) 
       s14d = (p1sqd +p4sqd+ 2*p1p4d) 
       s15d = (p1sqd +p5sqd+ 2*p1p5d) 
       s23d = (p2sqd +p3sqd+ 2*p2p3d) 
       s24d = (p2sqd +p4sqd+ 2*p2p4d) 
       s25d = (p2sqd +p5sqd+ 2*p2p5d) 
       s34d = (p3sqd +p4sqd+ 2*p3p4d) 
       s35d = (p3sqd +p5sqd+ 2*p3p5d) 
       s45d = (p4sqd +p5sqd+ 2*p4p5d)

c       print*,'-----------------CROSS ISR----------------'
c       print*, 'p1sqd=',p1sqd
c       print*, 'p2sqd=',p2sqd
c       print*, 'p3sqd=',p3sqd
c       print*, 'p4sqd=',p4sqd
c       print*, 'p5sqd=',p5sqd   
c       print*, 's12d=',s12d
c       print*, 's13d=',s13d
c       print*, 's14d=',s14d
c       print*, 's15d=',s15d
c       print*, 's23d=',s23d
c       print*, 's24d=',s24d
c       print*, 's25d=',s25d
c       print*, 's34d=',s34d
c       print*, 's35d=',s35d
c       print*, 's45d=',s45d

c       print*, 'p1sq=',p1sq
c       print*, 'p2sq=',p2sq
c       print*, 'p3sq=',p3sq
c       print*, 'p4sq=',p4sq
c       print*, 'p5sq=',p5sq   
c       print*, 's12=',s12
c       print*, 's13=',s13
c      print*, 's14=',s14
c       print*, 's15=',s15
c       print*, 's23=',s23
c       print*, 's24=',s24
c       print*, 's25=',s25
c       print*, 's34=',s34
c       print*, 's35=',s35
c      print*, 's45=',s45

       MESQ=ME**2
       ME3SQ=ME**3
       ME4SQ=ME**4
       ME6SQ=ME**6
       MU2SQ=MU**2
       MU3SQ=MU**3
       MU4SQ=MU**4
       MU6SQ=MU**6
       MESQd=dble(MESQ)
       MU2SQd=dble(MU2SQ)
       musqd=dble(musq)
       INVs12M=1q0/(-MESQ+s12)
       INVs15M=1q0/(-MESQ+s15)
c       Write(*,'(a5,F20.10)')," p1sq ", p1sq 
c       Write(*,'(a5,F20.10)')," p1p2 ", p1p2
c       Write(*,'(a5,F20.10)')," p1p3 ", p1p3
c       Write(*,'(a5,F20.10)')," p1p4 ", p1p4
c       Write(*,'(a5,F20.10)')," p2sq ", p2sq 
c       Write(*,'(a5,F20.10)')," p2p3 ", p2p3
c       Write(*,'(a5,F20.10)')," p2p4 ", p2p4
c       Write(*,'(a5,F20.10)')," p3sq ", p3sq 
c       Write(*,'(a5,F20.10)')," p3p4 ", p3p4
c       Write(*,'(a5,F20.10)')," p4sq ", p4sq 
c       Write(*,'(a5,F20.10)')," s12", s12 
c       Write(*,'(a5,F20.10)')," s13", s13 
c       Write(*,'(a5,F20.10)')," s14", s14 
c       Write(*,'(a5,F20.10)')," s23", s23 
c       Write(*,'(a5,F20.10)')," s24", s24 
c       Write(*,'(a5,F20.10)')," s34", s34 
      If(comp.gt.0) then
c************************************************************************************
c************************************************************************************
c************************************************************************************
c************************************************************************************
c    Calling C_ij,D_ij Functions    
c************************************************************************************
c************************************************************************************
c************************************************************************************
c************************************************************************************
c       A01=A0finG2(ME,musq)!A0finG(ME,musq) 
       A01=qlI1(MESQd,musqd,0)
c       A02=A0finG2(ME,musq)!A0finG(ME,musq) 
       A02=qlI1(MESQd,musqd,0)
c       A03=A0finG1(musq) !A0finG(0d0,musq)  
       A03=qlI1(0.d0,musqd,0)
c       A04=A0finG2(MU,musq) !A0finG(MU,musq) 
       A04=qlI1(MU2SQd,musqd,0)
c       A05=A0finG1(musq) !A0finG(0d0,musq)  
       A05=qlI1(0.d0,musqd,0)
       A01q=A01
       A02q=A02
       A03q=A03
       A04q=A04
       A05q=A05
       A01R=REALPART(A01q) 
       A02R=REALPART(A02q)          
       A03R=REALPART(A03q) 
       A04R=REALPART(A04q)   
       A05R=REALPART(A05q)           
       A01I=IMAGPART(A01q)
       A02I=IMAGPART(A02q)
       A03I=IMAGPART(A03q)
       A04I=IMAGPART(A04q)
       A05I=IMAGPART(A05q)
c************************************************************************************
c************************************************************************************
c************************************************************************************
c       B012q=B0finG6_QUAD(ME,musq) !B0finG(ME,ME,p1sq=0q0,musq)  
       B012= qlI2(p1sqd,MESQd,MESQd,musqd,0)
c       B013q=B0finG2_QUAD(ME,s12,musq)!B0finG(ME,0q0,s12,musq)  
       B013= qlI2(s12d,MESQd,0.d0,musqd,0)
c       B014q=B0finG5_QUAD(ME,MU,s45,musq)!B0finG(ME,MU,s45,musq) 
       B014=  qlI2(s45d,MESQd,MU2SQd,musqd,0)
c       B015q=B0finG3_QUAD(ME,musq)!B0finG(ME,0d0,p5sq,musq) 
       B015=qlI2(p5sqd,MESQd,0.d0,musqd,0)
c       B023q=B0finG3_QUAD(ME,musq) !B0finG(ME,0,p2sq=MESQ,musq)  
       B023=qlI2(p2sqd,MESQd,0.d0,musqd,0)
c       B024q=B0finG5_QUAD(ME,MU,s23,musq)!B0finG(ME,MU,s23,musq)   
       B024=qlI2(s23d,MESQd,MU2SQd,musqd,0)
c       B025q=B0finG2_QUAD(ME,s15,musq)!B0finG(ME,0d0,s15,musq)   
       B025=qlI2(s15d,MESQd,0.d0,musqd,0)
c       B034q=B0finG3_QUAD(MU,musq)!B0finG(0,MU,p3sq=MU,musq)  
       B034=qlI2(p3sqd,0.d0,MU2SQd,musqd,0)
c       B035q=B0finG1_QUAD(s34,musq)!B0finG(0d0,0d0,s34,musq) 
       B035=qlI2(s34d,0.d0,0.d0,musqd,0) 
c       B045q=B0finG3_QUAD(MU,musq)!B0finG(MU,0d0,p4sq=MU,musq)  
       B045=qlI2(p4sqd,MU2SQd,0.d0,musqd,0)  

       B012q=B012
       B013q=B013
       B014q=B014
       B015q=B015
       B023q=B023
       B024q=B024
       B025q=B025
       B034q=B034
       B035q=B035
       B045q=B045

       B012R=REALPART(B012q)          
       B013R=REALPART(B013q) 
       B014R=REALPART(B014q)   
       B015R=REALPART(B015q)           
       B023R=REALPART(B023q ) 
       B024R=REALPART( B024q)  
       B025R=REALPART(B025q)   
       B034R=REALPART(B034q)  
       B035R=REALPART(B035q)  
       B045R=REALPART(B045q)  
       B012I=IMAGPART(B012q)          
       B013I=IMAGPART(B013q) 
       B014I=IMAGPART(B014q)   
       B015I=IMAGPART(B015q)           
       B023I=IMAGPART(B023q) 
       B024I=IMAGPART(B024q)  
       B025I=IMAGPART(B025q)   
       B034I=IMAGPART(B034q)  
       B035I=IMAGPART(B035q)  
       B045I=IMAGPART(B045q)  
c************************************************************************************
       call tens_red2_new_Re_Com_QUAD(ME,ME,p1sq,A02R,A02I,A01R,A01I,B
     -   012R,B012I,Bij12R,Bij12I) 
       call tens_red2_new_Re_Com_QUAD(ME,0q0,s12,A03R,A03I,A01R,A01I,B
     -   013R,B013I,Bij13R,Bij13I) 
       call tens_red2_new_Re_Com_QUAD(ME,MU,s45,A04R,A04I,A01R,A01I,B0
     -   14R,B014I,Bij14R,Bij14I) 
       call tens_red2_new_Re_Com_QUAD(ME,0q0,p5sq,A05R,A05I,A01R,A01I,
     -   B015R,B015I,Bij15R,Bij15I) 
       call tens_red2_new_Re_Com_QUAD(ME,0q0,p2sq,A03R,A03I,A02R,A02I,
     -   B023R,B023I,Bij23R,Bij23I) 
       call tens_red2_new_Re_Com_QUAD(ME,MU,s23,A04R,A04I,A02R,A02I,B0
     -   24R,B024I,Bij24R,Bij24I) 
       call tens_red2_new_Re_Com_QUAD(ME,0q0,s15,A05R,A05I,A02R,A02I,B
     -   025R,B025I,Bij25R,Bij25I) 
       call tens_red2_new_Re_Com_QUAD(0q0,MU,p3sq,A04R,A04I,A03R,A03I,
     -   B034R,B034I,Bij34R,Bij34I) 
       call tens_red2_new_Re_Com_QUAD(0q0,0q0,s34,A05R,A05I,A03R,A03I,
     -   B035R,B035I,Bij35R,Bij35I) 
       call tens_red2_new_Re_Com_QUAD(MU,0q0,p4sq,A05R,A05I,A04R,A04I,
     -   B045R,B045I,Bij45R,Bij45I) 
c************************************************************************************
c************************************************************************************
c       C0123q=C0finG2_QUAD(ME,0.q0,ME,s12,p2sq,p1sq,musq)!C0finG(ME,ME,0,p1sq,p2sq,s12,musq)   
       C0123=qlI3(p1sqd,p2sqd,s12d,MESQd,MESQd,0.d0,musqd,0)
c       C0124q=C0finG2_QUAD(ME,MU,ME,s45,s23,p1sq,musq)  !C0finG(ME,ME,MU,p1sq,s23,s45,musq)   
       C0124=qlI3(p1sqd,s23d,s45d,MESQd,MESQd,MU2SQd,musqd,0)
c       C0125q=C0finG2_QUAD(ME,0.d0,ME,p5sq,s15,p1sq,musq)!C0finG(ME,ME,0d0,p1sq,s15,p5sq,musq)   
       C0125=qlI3(p1sqd,s15d,p5sqd,MESQd,MESQd,0.d0,musqd,0)
c       C0134q=C0finG2_QUAD(ME,0q0,MU,s12,p3sq,s45,musq)!C0finG(ME,0,MU,s12,p3sq,s45,musq)   
       C0134=qlI3(s12d,p3sqd,s45d,MESQd,0.d0,MU2SQd,musqd,0)
c       C0135q=C0finG2_QUAD(ME,0.d0,0.d0,s12,s34,p5sq,musq)!C0finG(ME,0d0,0d0,s12,s34,p5sq,musq)   
       C0135=qlI3(s12d,s34d,p5sqd,MESQd,0.d0,0.d0,musqd,0)
c       C0145q=C0finG1_QUAD(ME,MU,s45,musq) !C0finG(ME,MU,0d0,s45,p4sq,p5sq,musq)
       C0145=qlI3(s45d,p4sqd,p5sqd,MESQd,MU2SQd,0.d0,musqd,0)
       C0145=C0145-qlI3(s45d,p4sqd,p5sqd,MESQd,MU2SQd,0.d0,musqd,-1)*
     1 Log(musq/s25)
c       C0234q=C0finG1_QUAD(ME,MU,s23,musq)!C0finG(ME,0d0,MU,p2sq,p3sq,s23,musq) 
       C0234=  qlI3(p2sqd,p3sqd,s23d,MESQd,0.d0,MU2SQd,musqd,0)
       C0234=C0234-qlI3(p2sqd,p3sqd,s23d,MESQd,0.d0,MU2SQd,musqd,-1)*
     1 Log(musq/s25)
c       C0235q=C0finG2_QUAD(ME,0.d0,0.d0,p2sq,s34,s15,musq)!C0finG(ME,0d0,0d0,p2sq,s34,s15,musq)   
       C0235=qlI3(p2sqd,s34d,s15d,MESQd,0.d0,0.d0,musqd,0)
c       C0245q=C0finG2_QUAD(ME,MU,0.d0,s23,p4sq,s15,musq)!C0finG(ME,MU,0d0,s23,p4sq,s15,musq)   
       C0245=qlI3(s23d,p4sqd,s15d,MESQd,MU2SQd,0.d0,musqd,0)
c       C0345q=C0finG2_QUAD(0.d0,MU,0.d0,p3sq,p4sq,s34,musq)!C0finG(0d0,MU,0d0,p3sq,p4sq,s34,musq)  
       C0345= qlI3(p3sqd,p4sqd,s34d,0.d0,MU2SQd,0.d0,musqd,0) 
c       print*,s12d,s23d,s45d,s15d,s34d

       C0123q=C0123
       C0124q=C0124
       C0125q=C0125
       C0134q=C0134
       C0135q=C0135
       C0145q=C0145
       C0234q=C0234
       C0235q=C0235
       C0245q=C0245
       C0345q=C0345
c$$$       Print*,"C0123",C0123
c$$$       Print*,"C0124",C0124
c$$$       Print*,"C0125",C0125
c$$$       Print*,"C0134",C0134,MU,MU,ME,0.d0,s45,s12,musq
c$$$       Print*,"C0135",C0135
c$$$       Print*,"C0145",C0145
c$$$       Print*,"C0234",C0234
c$$$       Print*,"C0235",C0235
c$$$       Print*,"C0245",C0245
c$$$       Print*,"C0345",C0345

c************************************************************************************
c************************************************************************************
       call tens_red3_new_Re_Com_G_QUAD(ME,ME,0q0,p1sq,p2sq,s12,B023R,
     -   B013R,B012R,B023I,B013I,B012I,Bij23R,Bij13R,Bij12R,Bij23I,Bi
     -   j13I,Bij12I,C0123q,C0123R,C0123I,Cij123R,Cij123I) 
       call tens_red3_new_Re_Com_G_QUAD(ME,ME,MU,p1sq,s23,s45,B024R,B0
     -   14R,B012R,B024I,B014I,B012I,Bij24R,Bij14R,Bij12R,Bij24I,Bij1
     -   4I,Bij12I,C0124q,C0124R,C0124I,Cij124R,Cij124I) 
       call tens_red3_new_Re_Com_G_QUAD(ME,ME,0q0,p1sq,s15,p5sq,B025R,
     -   B015R,B012R,B025I,B015I,B012I,Bij25R,Bij15R,Bij12R,Bij25I,Bi
     -   j15I,Bij12I,C0125q,C0125R,C0125I,Cij125R,Cij125I) 
       call tens_red3_new_Re_Com_G_QUAD(ME,0q0,MU,s12,p3sq,s45,B034R,B
     -   014R,B013R,B034I,B014I,B013I,Bij34R,Bij14R,Bij13R,Bij34I,Bij
     -   14I,Bij13I,C0134q,C0134R,C0134I,Cij134R,Cij134I) 
       call tens_red3_new_Re_Com_G_QUAD(ME,0q0,0q0,s12,s34,p5sq,B035R,
     -   B015R,B013R,B035I,B015I,B013I,Bij35R,Bij15R,Bij13R,Bij35I,Bi
     -   j15I,Bij13I,C0135q,C0135R,C0135I,Cij135R,Cij135I) 
       call tens_red3_new_Re_Com_G_QUAD(ME,MU,0q0,s45,p4sq,p5sq,B045R,
     -   B015R,B014R,B045I,B015I,B014I,Bij45R,Bij15R,Bij14R,Bij45I,Bi
     -   j15I,Bij14I,C0145q,C0145R,C0145I,Cij145R,Cij145I) 
       call tens_red3_new_Re_Com_G_QUAD(ME,0q0,MU,p2sq,p3sq,s23,B034R,
     -   B024R,B023R,B034I,B024I,B023I,Bij34R,Bij24R,Bij23R,Bij34I,Bi
     -   j24I,Bij23I,C0234q,C0234R,C0234I,Cij234R,Cij234I) 
       call tens_red3_new_Re_Com_G_QUAD(ME,0q0,0q0,p2sq,s34,s15,B035R,
     -   B025R,B023R,B035I,B025I,B023I,Bij35R,Bij25R,Bij23R,Bij35I,Bi
     -   j25I,Bij23I,C0235q,C0235R,C0235I,Cij235R,Cij235I) 
       call tens_red3_new_Re_Com_G_QUAD(ME,MU,0q0,s23,p4sq,s15,B045R,B
     -   025R,B024R,B045I,B025I,B024I,Bij45R,Bij25R,Bij24R,Bij45I,Bij
     -   25I,Bij24I,C0245q,C0245R,C0245I,Cij245R,Cij245I) 
       call tens_red3_new_Re_Com_G_QUAD(0q0,MU,0q0,p3sq,p4sq,s34,B045R
     -   ,B035R,B034R,B045I,B035I,B034I,Bij45R,Bij35R,Bij34R,Bij45I,B
     -   ij35I,Bij34I,C0345q,C0345R,C0345I,Cij345R,Cij345I) 
c************************************************************************************
c************************************************************************************
c       D02345q=D0finG2_QUAD(ME,MU,s34,s23,s15,musq)!D0finG(ME,0d0,MU,0d0,s23,s34,p2sq,p3sq,p4sq,s15,musq)
       D02345=qlI4(p2sqd,p3sqd,p4sqd,s15d,s23d,s34d,MESQd,0.d0,
     1 MU2SQd,0.d0,musqd,0)
       D02345=D02345-qlI4(p2sqd,p3sqd,p4sqd,s15d,s23d,s34d,MESQd
     1 ,0.d0,MU2SQd,0.d0,musqd,-1)*Log(musq/s25)
c       D01345q=D0finG2_QUAD(ME,MU,s34,s45,s12,musq)!D0finG(ME,0d0,MU,0d0,s45,s34,s12,p3sq,p4sq,p5sq,musq)
       D01345=qlI4(s12d,p3sqd,p4sqd,p5sqd,s45d,s34d,MESQd,0.d0,
     1 MU2SQd,0.d0,musqd,0)
       D01345=D01345-qlI4(s12d,p3sqd,p4sqd,p5sqd,s45d,s34d,MESQd,0.d0,
     1 MU2SQd,0.d0,musqd,-1)*Log(musq/s25)
c       D01245q=D0finG1_QUAD(MU,ME,s15,s45,s23,musq)!D0finG(ME,ME,MU,0d0,s45,s15,p1sq,s23,p4sq,p5sq,musq)
       D01245=qlI4(p1sqd,s23d,p4sqd,p5sqd,s45d,s15d,MESQd,MESQd,MU2SQd,
     1 0.d0,musqd,0)
       D01245=D01245-qlI4(p1sqd,s23d,p4sqd,p5sqd,s45d,s15d,MESQd,MESQd,
     1 MU2SQd,0.d0,musqd,-1)*Log(musq/s25)
c       D01235q=D0finG3_QUAD(ME,s12,s15,s34,musq)!D0finG(ME,ME,0d0,0d0,s12,s15,p1sq,p2sq,s34,p5sq,musq)
       D01235=qlI4(p1sqd,p2sqd,s34d,p5sqd,s12d,s15d,MESQd,MESQd,
     1 0.d0,0.d0,musqd,0)
c       D01234q=D0finG1_QUAD(MU,ME,s12,s23,s45,musq)!D0finG(ME,ME,0d0,MU,s12,s23,p1sq,p2sq,p3sq,s45,musq)
       D01234=qlI4(p1sqd,p2sqd,p3sqd,s45d,s12d,s23d,MESQd,MESQd,
     1 0.d0,MU2SQd,musqd,0)
       D01234=D01234-qlI4(p1sqd,p2sqd,p3sqd,s45d,s12d,s23d,MESQd,MESQd,
     1 0.d0,MU2SQd,musqd,-1)*Log(musq/s25)

       D02345q=D02345
       D01345q=D01345
       D01245q=D01245
       D01235q=D01235
       D01234q=D01234

c$$$       Print*," D02345",D02345
c$$$       Print*," D01345",D01345
c$$$       Print*," D01245",D01245
c$$$       Print*," D01235",D01235
c$$$       Print*," D01234",D01234
c************************************************************************************
c************************************************************************************
       call tens_red4_new_Re_Com_G_QUAD(ME,0q0,MU,0q0,p2sq,p3sq,p4sq,p
     -   2p3,p2p4,p3p4,C0345R,C0245R,C0235R,C0234R,Cij345R,Cij245R,Ci
     -   j235R,Cij234R,C0345I,C0245I,C0235I,C0234I,Cij345I,Cij245I,Ci
     -   j235I,Cij234I,D02345q,D02345R,D02345I,Dij2345R,Dij2345I)
        call tens_red4_new_Re_Com_G_QUAD(ME,0q0,MU,0q0,s12,p3sq,p4sq,p1
     -   p3+p2p3,p1p4+p2p4,p3p4,C0345R,C0145R,C0135R,C0134R,Cij345R,C
     -   ij145R,Cij135R,Cij134R,C0345I,C0145I,C0135I,C0134I,Cij345I,C
     -   ij145I,Cij135I,Cij134I,D01345q,D01345R,D01345I,Dij1345R,Dij13
     -   45I)
        call tens_red4_new_Re_Com_G_QUAD(ME,ME,MU,0q0,p1sq,s23,p4sq,p1p
     -   2+p1p3,p1p4,p2p4+p3p4,C0245R,C0145R,C0125R,C0124R,Cij245R,Ci
     -   j145R,Cij125R,Cij124R,C0245I,C0145I,C0125I,C0124I,Cij245I,Ci
     -   j145I,Cij125I,Cij124I,D01245q,D01245R,D01245I,Dij1245R,Dij124
     -   5I)
        call tens_red4_new_Re_Com_G_QUAD(ME,ME,0q0,0q0,p1sq,p2sq,s34,p1
     -   p2,p1p3+p1p4,p2p3+p2p4,C0235R,C0135R,C0125R,C0123R,Cij235R,C
     -   ij135R,Cij125R,Cij123R,C0235I,C0135I,C0125I,C0123I,Cij235I,C
     -   ij135I,Cij125I,Cij123I,D01235q,D01235R,D01235I,Dij1235R,Dij12
     -   35I)
        call tens_red4_new_Re_Com_G_QUAD(ME,ME,0q0,MU,p1sq,p2sq,p3sq,p1
     -   p2,p1p3,p2p3,C0234R,C0134R,C0124R,C0123R,Cij234R,Cij134R,Cij
     -   124R,Cij123R,C0234I,C0134I,C0124I,C0123I,Cij234I,Cij134I,Cij
     -   124I,Cij123I,D01234q,D01234R,D01234I,Dij1234R,Dij1234I)
c************************************************************************************
c************************************************************************************
       EE0=E0finG_QUAD(ME,ME,0q0,MU,0q0,p1sq,p2sq,p3sq,p4sq,p5sq,s12,s
     -   23,s34,s45,s15,D02345q,D01345q,D01245q,D01235q,D01234q)
       EE0R=REALPART(EE0) 
       EE0I=IMAGPART(EE0) 
c************************************************************************************
c************************************************************************************
       call tens_red5_new_Re_Com_G_QUAD(ME,ME,0q0,MU,0q0,p1sq,p2sq,p3s
     -   q,p4sq,p1p2,p1p3,p1p4,p2p3,p2p4,p3p4,D02345R,D01345R,D01245R
     -   ,D01235R,D01234R,Dij2345R,Dij1345R,Dij1245R,Dij1235R,Dij1234
     -   R,D02345I,D01345I,D01245I,D01235I,D01234I,Dij2345I,Dij1345I,
     -   Dij1245I,Dij1235I,Dij1234I,EijR,EijI)
c************************************************************************************
c************************************************************************************
c************************************************************************************
c************************************************************************************
c************************************************************************************
c************************************************************************************
c************************************************************************************
c************************************************************************************
c************************************************************************************
c************************************************************************************
c************************************************************************************
c************************************************************************************
c       Definition of the F,P functions:Independent of the currents    
c************************************************************************************
c************************************************************************************
c************************************************************************************
c************************************************************************************
c************************************************************************************
c************************************************************************************
c************************************************************************************
c************************************************************************************
       P(1) = -MESQ+s15
       P(2) = MESQ+s12
       P(3) = MESQ-s15
       P(4) = -MESQ+s12
       P(5) = -MESQ+s45
       P(6) = MESQ-s45
       P(7) = MESQ+s12-2*s45
       P(8) = -MESQ+s15-s23+s45
       P(9) = -s12+s15-s23+s45
       P(10) = MESQ-s15+s23-s45
       P(11) = MU2SQ+s12-s15+s23-3*s45
       P(12) = s15-s23+s45
       P(13) = 2*ME4SQ+s45*P(9)+MESQ*P(11)-MU2SQ*P(12)
       P(14) = s12-s15+s23-3*s45
       P(15) = -s15+s23+s45
       P(16) = -3*ME4SQ-MESQ*P(14)+s12*P(15)
       P(17) = 2*MU2SQ+s15-s23
       P(18) = s15-s23
       P(19) = s12-2*s45
       P(20) = -2*MU2SQ*s45+MESQ*P(17)+P(18)*P(19)
       P(21) = -s15+s23+s34
       P(22) = -3*MU2SQ+2*P(21)
       P(23) = MU2SQ+s15-s23-s34
       P(24) = MU2SQ*s12+MESQ*P(22)+2*s45*P(23)
       P(25) = MESQ+MU2SQ-s45
       P(26) = MESQ-s12
       P(27) = MESQ+s15-2*s23
       FR(1) = 4*Invs15M*ME*(D02345R+Dij2345R(1,1))*P(27)
       FI(1) = 4*Invs15M*ME*(D02345I+Dij2345I(1,1))*P(27)
       F(1)=COMPLEX(FR(1),FI(1))
       P(28) = MESQ+MU2SQ-s23
       P(29) = MESQ-s23
       FR(2) = -(Invs15M*P(3)*(C0345R+Cij345R(1,1)+2*(C0235R-MU2SQ*Di
     -   j2345R(3,1)+D02345R*P(28)+Dij2345R(2,1)*P(29))))
       FI(2) = -(Invs15M*P(3)*(C0345I+Cij345I(1,1)+2*(C0235I-MU2SQ*Di
     -   j2345I(3,1)+D02345I*P(28)+Dij2345I(2,1)*P(29))))
       F(2)=COMPLEX(FR(2),FI(2))
       P(30) = 2*MESQ+MU2SQ-s23
       P(31) = MESQ-MU2SQ-s23
       P(32) = -MESQ+MU2SQ+s23-s34
       FR(3) = 2*Invs15M*(Cij345R(1,1)-Cij345R(2,1)+4*(C0235R+MESQ*Di
     -   j2345R(1,1)+D02345R*P(30))-2*(C0245R-C0345R-Dij2345R(2,1)*P(
     -   31)-Dij2345R(3,1)*P(32)))
       FI(3) = 2*Invs15M*(Cij345I(1,1)-Cij345I(2,1)+4*(C0235I+MESQ*Di
     -   j2345I(1,1)+D02345I*P(30))-2*(C0245I-C0345I-Dij2345I(2,1)*P(
     -   31)-Dij2345I(3,1)*P(32)))
       F(3)=COMPLEX(FR(3),FI(3))
       FR(4) = 2*Invs15M*ME*(D02345R+Dij2345R(1,1))*P(27)
       FI(4) = 2*Invs15M*ME*(D02345I+Dij2345I(1,1))*P(27)
       F(4)=COMPLEX(FR(4),FI(4))
       FR(5) = 2*Invs15M*ME*(D02345R+Dij2345R(1,1))*P(1)
       FI(5) = 2*Invs15M*ME*(D02345I+Dij2345I(1,1))*P(1)
       F(5)=COMPLEX(FR(5),FI(5))
       FR(6) = Invs15M*(Cij345R(1,1)-Cij345R(2,1)+4*(C0235R+MESQ*Dij2
     -   345R(1,1)+D02345R*P(30))-2*(C0245R-C0345R-Dij2345R(2,1)*P(31
     -   )-Dij2345R(3,1)*P(32)))
       FI(6) = Invs15M*(Cij345I(1,1)-Cij345I(2,1)+4*(C0235I+MESQ*Dij2
     -   345I(1,1)+D02345I*P(30))-2*(C0245I-C0345I-Dij2345I(2,1)*P(31
     -   )-Dij2345I(3,1)*P(32)))
       F(6)=COMPLEX(FR(6),FI(6))
       FR(7) = 4*Invs12M*ME*(D01345R+Dij1345R(1,1))*P(4)
       FI(7) = 4*Invs12M*ME*(D01345I+Dij1345I(1,1))*P(4)
       F(7)=COMPLEX(FR(7),FI(7))
       FR(8) = 8*Invs12M*ME*(D01345R+Dij1345R(1,1))*P(5)
       FI(8) = 8*Invs12M*ME*(D01345I+Dij1345I(1,1))*P(5)
       F(8)=COMPLEX(FR(8),FI(8))
       FR(9) = Invs12M*(Cij345R(1,1)*P(9)+Cij345R(2,1)*P(10)+4*(C0145
     -   R*P(6)+C0135R*P(8)-D01345R*P(13))-2*(C0345R*P(7)-Dij1345R(1,
     -   1)*P(16)-Dij1345R(2,1)*P(20)-Dij1345R(3,1)*P(24)))
       FI(9) = Invs12M*(Cij345I(1,1)*P(9)+Cij345I(2,1)*P(10)+4*(C0145
     -   I*P(6)+C0135I*P(8)-D01345I*P(13))-2*(C0345I*P(7)-Dij1345I(1,
     -   1)*P(16)-Dij1345I(2,1)*P(20)-Dij1345I(3,1)*P(24)))
       F(9)=COMPLEX(FR(9),FI(9))
       FR(10) = 2*Invs12M*(Cij345R(1,1)-Cij345R(2,1)+4*(C0135R+Dij134
     -   5R(3,1)*P(5)+D01345R*P(25))+2*(Dij1345R(2,1)*P(7)+Dij1345R(1
     -   ,1)*P(26)))
       FI(10) = 2*Invs12M*(Cij345I(1,1)-Cij345I(2,1)+4*(C0135I+Dij134
     -   5I(3,1)*P(5)+D01345I*P(25))+2*(Dij1345I(2,1)*P(7)+Dij1345I(1
     -   ,1)*P(26)))
       F(10)=COMPLEX(FR(10),FI(10))
       FR(11) = -2*Invs12M*(Cij345R(1,1)-Cij345R(2,1)-2*(Dij1345R(1,1
     -   )*P(4)-Dij1345R(2,1)*P(7))+4*(C0135R-Dij1345R(3,1)*P(6)+D013
     -   45R*P(25)))
       FI(11) = -2*Invs12M*(Cij345I(1,1)-Cij345I(2,1)-2*(Dij1345I(1,1
     -   )*P(4)-Dij1345I(2,1)*P(7))+4*(C0135I-Dij1345I(3,1)*P(6)+D013
     -   45I*P(25)))
       F(11)=COMPLEX(FR(11),FI(11))
       FR(12) = 4*Invs12M*ME*(D01345R+Dij1345R(1,1))*P(6)
       FI(12) = 4*Invs12M*ME*(D01345I+Dij1345I(1,1))*P(6)
       F(12)=COMPLEX(FR(12),FI(12))
       FR(13) = 2*Invs12M*ME*(D01345R+Dij1345R(1,1))*P(26)
       FI(13) = 2*Invs12M*ME*(D01345I+Dij1345I(1,1))*P(26)
       F(13)=COMPLEX(FR(13),FI(13))
       FR(14) = Invs12M*(Cij345R(1,1)-Cij345R(2,1)+4*(C0135R+Dij1345R
     -   (3,1)*P(5)+D01345R*P(25))+2*(Dij1345R(2,1)*P(7)+Dij1345R(1,1
     -   )*P(26)))
       FI(14) = Invs12M*(Cij345I(1,1)-Cij345I(2,1)+4*(C0135I+Dij1345I
     -   (3,1)*P(5)+D01345I*P(25))+2*(Dij1345I(2,1)*P(7)+Dij1345I(1,1
     -   )*P(26)))
       F(14)=COMPLEX(FR(14),FI(14))
       FR(15) = 8*ME*(EijR(2,1)+EijR(2,2)-EijR(3,1)-EijR(8,2))*P(18)
       FI(15) = 8*ME*(EijI(2,1)+EijI(2,2)-EijI(3,1)-EijI(8,2))*P(18)
       F(15)=COMPLEX(FR(15),FI(15))
       FR(16) = 8*ME*(EijR(1,1)-EijR(3,1)+EijR(5,2)-EijR(8,2))*P(18)
       FI(16) = 8*ME*(EijI(1,1)-EijI(3,1)+EijI(5,2)-EijI(8,2))*P(18)
       F(16)=COMPLEX(FR(16),FI(16))
       FR(17) = 4*ME*(D01345R+Dij1345R(1,1)+2*((EE0R+EijR(2,1))*P(3)+(
     -   -EijR(3,1)+EijR(4,1)-EijR(8,2)+EijR(9,2))*P(18)))
       FI(17) = 4*ME*(D01345I+Dij1345I(1,1)+2*((EE0I+EijI(2,1))*P(3)+(
     -   -EijI(3,1)+EijI(4,1)-EijI(8,2)+EijI(9,2))*P(18)))
       F(17)=COMPLEX(FR(17),FI(17))
       FR(18) = 4*ME*(D02345R+Dij2345R(1,1)+2*((EE0R+EijR(2,1))*P(10)-
     -   (EijR(3,1)+EijR(8,2))*P(18)))
       FI(18) = 4*ME*(D02345I+Dij2345I(1,1)+2*((EE0I+EijI(2,1))*P(10)-
     -   (EijI(3,1)+EijI(8,2))*P(18)))
       F(18)=COMPLEX(FR(18),FI(18))
       P(33) = s15-2*s23
       P(34) = 2*MU2SQ+s12-3*s15+4*s23-6*s45
       P(35) = s12-s15+s23
       P(36) = s45**2-MU2SQ*P(12)-s45*P(35)
       P(37) = 3*ME4SQ-s12*P(33)+MESQ*P(34)+2*P(36)
       P(38) = s23-s45
       P(39) = MESQ-s15+2*P(38)
       P(40) = 3*MESQ+MU2SQ-s23
       P(41) = -2*MU2SQ-s15+s23+s34
       P(42) = s12+s15
       P(43) = s15-2*P(38)
       P(44) = ME4SQ+2*P(12)*P(38)-MESQ*P(42)+s12*P(43)
       P(45) = -MESQ+s15-2*P(38)
       P(46) = MU2SQ+s15+s23
       P(47) = ME4SQ+s15*s23+MU2SQ*P(43)-MESQ*P(46)
       P(48) = s15-s23-s34
       P(49) = s15-4*P(38)
       P(50) = MESQ*P(23)-P(43)*P(48)-MU2SQ*P(49)
       P(51) = -s15+s23
       FR(19) = -C0345R+2*(-C0235R+C0245R+MU2SQ*(Dij1345R(3,1)+Dij2345
     -   R(2,1))+Dij1345R(1,1)*P(10)+Dij1345R(2,1)*P(18)+D01345R*P(27
     -   )-EE0R*P(37)+D01245R*P(39)-D02345R*P(40)+Dij2345R(3,1)*P(41)
     -   +EijR(1,1)*P(44)-2*(MESQ*Dij2345R(1,1)-D01235R*P(8)-MESQ*Eij
     -   R(2,1)*P(45))-EijR(3,1)*P(47)-EijR(4,1)*P(50)+4*EijR(11,2)*P
     -   (51))
       FI(19) = -C0345I+2*(-C0235I+C0245I+MU2SQ*(Dij1345I(3,1)+Dij2345
     -   I(2,1))+Dij1345I(1,1)*P(10)+Dij1345I(2,1)*P(18)+D01345I*P(27
     -   )-EE0I*P(37)+D01245I*P(39)-D02345I*P(40)+Dij2345I(3,1)*P(41)
     -   +EijI(1,1)*P(44)-2*(MESQ*Dij2345I(1,1)-D01235I*P(8)-MESQ*Eij
     -   I(2,1)*P(45))-EijI(3,1)*P(47)-EijI(4,1)*P(50)+4*EijI(11,2)*P
     -   (51))
       F(19)=COMPLEX(FR(19),FI(19))
       P(52) = 3*MESQ+s12-2*s45
       P(53) = MU2SQ+s12-s23+2*P(6)
       P(54) = -3*MU2SQ-s15+s23+s34
       P(55) = 3*MU2SQ+s15-s23-s34
       FR(20) = 4*(Dij1245R(1,1)-Dij1245R(2,1)-Dij1345R(1,1)+Dij1345R(
     -   2,1)-2*(MESQ*EijR(5,2)-MESQ*EijR(8,2))-EijR(1,2)*P(7)-EijR(3
     -   ,2)*P(28)+(-EijR(1,1)+EijR(3,1))*P(52)+EijR(6,2)*P(53)+EijR(
     -   7,2)*P(54)+EijR(10,2)*P(55))
       FI(20) = 4*(Dij1245I(1,1)-Dij1245I(2,1)-Dij1345I(1,1)+Dij1345I(
     -   2,1)-2*(MESQ*EijI(5,2)-MESQ*EijI(8,2))-EijI(1,2)*P(7)-EijI(3
     -   ,2)*P(28)+(-EijI(1,1)+EijI(3,1))*P(52)+EijI(6,2)*P(53)+EijI(
     -   7,2)*P(54)+EijI(10,2)*P(55))
       F(20)=COMPLEX(FR(20),FI(20))
       FR(21) = 4*(-Dij1345R(1,1)+Dij1345R(2,1)-2*(MESQ*EijR(2,2)+EijR
     -   (11,2))+(-EijR(5,2)+EijR(6,2))*P(7)-EijR(3,2)*P(28)+EijR(8,2
     -   )*P(40)+(-EijR(2,1)+EijR(3,1))*P(52)+EijR(9,2)*P(54)+EijR(10
     -   ,2)*P(55))
       FI(21) = 4*(-Dij1345I(1,1)+Dij1345I(2,1)-2*(MESQ*EijI(2,2)+EijI
     -   (11,2))+(-EijI(5,2)+EijI(6,2))*P(7)-EijI(3,2)*P(28)+EijI(8,2
     -   )*P(40)+(-EijI(2,1)+EijI(3,1))*P(52)+EijI(9,2)*P(54)+EijI(10
     -   ,2)*P(55))
       F(21)=COMPLEX(FR(21),FI(21))
       P(56) = 2*MESQ+MU2SQ+s12-s45
       P(57) = -MESQ+s12+s23-s45
       P(58) = MU2SQ+s45
       P(59) = 3*MESQ+s12-2*P(58)
       P(60) = s15-s23-s34+s45
       P(61) = 3*MESQ-4*MU2SQ+s12-2*P(60)
       P(62) = MESQ+4*MU2SQ+s15-2*s23-s34
       FR(22) = 4*(-Dij1245R(2,1)+Dij1245R(3,1)-Dij1345R(1,1)-Dij1345R
     -   (3,1)-4*EijR(11,2)+(EijR(6,2)-EijR(7,2))*P(7)-EijR(3,2)*P(28
     -   )+EijR(4,2)*P(54)+2*(D01235R-D01245R+D01345R+Dij1345R(2,1)+M
     -   ESQ*(2*EijR(2,1)+EijR(8,2)-EijR(9,2))+EE0R*P(56)+EijR(1,1)*P
     -   (57))+EijR(3,1)*P(59)-EijR(4,1)*P(61)+EijR(10,2)*P(62))
       FI(22) = 4*(-Dij1245I(2,1)+Dij1245I(3,1)-Dij1345I(1,1)-Dij1345I
     -   (3,1)-4*EijI(11,2)+(EijI(6,2)-EijI(7,2))*P(7)-EijI(3,2)*P(28
     -   )+EijI(4,2)*P(54)+2*(D01235I-D01245I+D01345I+Dij1345I(2,1)+M
     -   ESQ*(2*EijI(2,1)+EijI(8,2)-EijI(9,2))+EE0I*P(56)+EijI(1,1)*P
     -   (57))+EijI(3,1)*P(59)-EijI(4,1)*P(61)+EijI(10,2)*P(62))
       F(22)=COMPLEX(FR(22),FI(22))
       P(63) = -MU2SQ+s12+s23+2*P(6)
       FR(23) = 4*(-D01245R+D01345R-Dij1245R(2,1)+Dij1345R(2,1)+2*MESQ
     -   *(EijR(2,1)+EijR(8,2))+EE0R*P(2)+EijR(1,1)*P(4)+EijR(6,2)*P(
     -   7)-EijR(3,2)*P(28)+(EijR(4,1)+EijR(10,2))*P(55)+EijR(3,1)*P(
     -   63))
       FI(23) = 4*(-D01245I+D01345I-Dij1245I(2,1)+Dij1345I(2,1)+2*MESQ
     -   *(EijI(2,1)+EijI(8,2))+EE0I*P(2)+EijI(1,1)*P(4)+EijI(6,2)*P(
     -   7)-EijI(3,2)*P(28)+(EijI(4,1)+EijI(10,2))*P(55)+EijI(3,1)*P(
     -   63))
       F(23)=COMPLEX(FR(23),FI(23))
       P(64) = MU2SQ-s23
       P(65) = 3*MESQ+s12+2*P(64)
       P(66) = -MESQ+MU2SQ+s23
       P(67) = MU2SQ+s12+s23
       P(68) = 8*MESQ-4*P(67)
       P(69) = MU2SQ-s15+s23-s34
       P(70) = -MU2SQ+s15-s23+s34
       FR(24) = 8*(Dij1235R(1,1)-Dij1235R(3,1)+MESQ*EijR(5,2)-MESQ*Eij
     -   R(8,2))+16*EijR(11,2)+EijR(6,2)*P(68)-4*(Dij1245R(1,1)-Dij12
     -   45R(2,1)-EijR(1,2)*P(4)-(EijR(1,1)-EijR(3,1))*P(65)-EijR(3,2
     -   )*P(66)-EijR(7,2)*P(69)-EijR(10,2)*P(70))
       FI(24) = 8*(Dij1235I(1,1)-Dij1235I(3,1)+MESQ*EijI(5,2)-MESQ*Eij
     -   I(8,2))+16*EijI(11,2)+EijI(6,2)*P(68)-4*(Dij1245I(1,1)-Dij12
     -   45I(2,1)-EijI(1,2)*P(4)-(EijI(1,1)-EijI(3,1))*P(65)-EijI(3,2
     -   )*P(66)-EijI(7,2)*P(69)-EijI(10,2)*P(70))
       F(24)=COMPLEX(FR(24),FI(24))
       P(71) = MESQ+MU2SQ+s23
       FR(25) = 4*(2*(Dij1235R(2,1)-Dij1235R(3,1)+MESQ*EijR(2,2)+EijR(
     -   11,2))+EijR(5,2)*P(4)+EijR(6,2)*P(26)+(EijR(2,1)-EijR(3,1))*
     -   P(65)+EijR(3,2)*P(66)+EijR(9,2)*P(69)+EijR(10,2)*P(70)-EijR(
     -   8,2)*P(71))
       FI(25) = 4*(2*(Dij1235I(2,1)-Dij1235I(3,1)+MESQ*EijI(2,2)+EijI(
     -   11,2))+EijI(5,2)*P(4)+EijI(6,2)*P(26)+(EijI(2,1)-EijI(3,1))*
     -   P(65)+EijI(3,2)*P(66)+EijI(9,2)*P(69)+EijI(10,2)*P(70)-EijI(
     -   8,2)*P(71))
       F(25)=COMPLEX(FR(25),FI(25))
       P(72) = MU2SQ+s15-s23
       P(73) = MESQ+s12+2*P(72)
       P(74) = MU2SQ+s23
       P(75) = MESQ+s15+s34-2*P(74)
       FR(26) = 4*(Dij1245R(2,1)-Dij1245R(3,1)+Dij1345R(2,1)-Dij1345R(
     -   3,1)+2*(D01345R-MESQ*EijR(8,2)+MESQ*EijR(9,2))+EijR(7,2)*P(4
     -   )+EijR(6,2)*P(26)-EijR(3,1)*P(65)+EijR(3,2)*P(66)+EijR(4,2)*
     -   P(69)+EijR(4,1)*P(73)+EijR(10,2)*P(75))
       FI(26) = 4*(Dij1245I(2,1)-Dij1245I(3,1)+Dij1345I(2,1)-Dij1345I(
     -   3,1)+2*(D01345I-MESQ*EijI(8,2)+MESQ*EijI(9,2))+EijI(7,2)*P(4
     -   )+EijI(6,2)*P(26)-EijI(3,1)*P(65)+EijI(3,2)*P(66)+EijI(4,2)*
     -   P(69)+EijI(4,1)*P(73)+EijI(10,2)*P(75))
       F(26)=COMPLEX(FR(26),FI(26))
       FR(27) = 4*(Dij1245R(2,1)+Dij1345R(2,1)-Dij2345R(3,1)+2*(D01345
     -   R-Dij1235R(3,1)-MESQ*EijR(8,2)+EijR(4,1)*P(8))+EijR(6,2)*P(2
     -   6)-EijR(3,1)*P(65)+EijR(3,2)*P(66)+EijR(10,2)*P(70))
       FI(27) = 4*(Dij1245I(2,1)+Dij1345I(2,1)-Dij2345I(3,1)+2*(D01345
     -   I-Dij1235I(3,1)-MESQ*EijI(8,2)+EijI(4,1)*P(8))+EijI(6,2)*P(2
     -   6)-EijI(3,1)*P(65)+EijI(3,2)*P(66)+EijI(10,2)*P(70))
       F(27)=COMPLEX(FR(27),FI(27))
       FR(28) = 2*ME*(D02345R+Dij2345R(1,1)+2*(EE0R+EijR(2,1))*P(6))
       FI(28) = 2*ME*(D02345I+Dij2345I(1,1)+2*(EE0I+EijI(2,1))*P(6))
       F(28)=COMPLEX(FR(28),FI(28))
       FR(29) = 2*ME*(-D01345R+D02345R-Dij1345R(1,1)+Dij2345R(1,1)+2*(
     -   EE0R+EijR(2,1))*P(38))
       FI(29) = 2*ME*(-D01345I+D02345I-Dij1345I(1,1)+Dij2345I(1,1)+2*(
     -   EE0I+EijI(2,1))*P(38))
       F(29)=COMPLEX(FR(29),FI(29))
       FR(30) = 8*ME*(EijR(2,1)+EijR(2,2)-EijR(3,1)-EijR(8,2))
       FI(30) = 8*ME*(EijI(2,1)+EijI(2,2)-EijI(3,1)-EijI(8,2))
       F(30)=COMPLEX(FR(30),FI(30))
       FR(31) = 8*ME*(EijR(1,1)-EijR(3,1)+EijR(5,2)-EijR(8,2))
       FI(31) = 8*ME*(EijI(1,1)-EijI(3,1)+EijI(5,2)-EijI(8,2))
       F(31)=COMPLEX(FR(31),FI(31))
       FR(32) = -8*ME*(EE0R+EijR(2,1)+EijR(3,1)-EijR(4,1)+EijR(8,2)-Ei
     -   jR(9,2))
       FI(32) = -8*ME*(EE0I+EijI(2,1)+EijI(3,1)-EijI(4,1)+EijI(8,2)-Ei
     -   jI(9,2))
       F(32)=COMPLEX(FR(32),FI(32))
       FR(33) = -8*ME*(EE0R+EijR(2,1)+EijR(3,1)+EijR(8,2))
       FI(33) = -8*ME*(EE0I+EijI(2,1)+EijI(3,1)+EijI(8,2))
       F(33)=COMPLEX(FR(33),FI(33))
       P(76) = MU2SQ-s45
       P(77) = 3*MESQ+s12+2*P(76)
       P(78) = -MESQ+s12+2*P(38)
       P(79) = MU2SQ+s15-s23-s34-2*P(6)
       FR(34) = 2*(-D01245R+D01345R-Dij1345R(1,1)+Dij1345R(2,1)-Dij234
     -   5R(3,1)+2*(D01235R+MESQ*EijR(2,1))-4*EijR(11,2)+EijR(3,1)*P(
     -   31)+EE0R*P(77)+EijR(1,1)*P(78)+EijR(4,1)*P(79))
       FI(34) = 2*(-D01245I+D01345I-Dij1345I(1,1)+Dij1345I(2,1)-Dij234
     -   5I(3,1)+2*(D01235I+MESQ*EijI(2,1))-4*EijI(11,2)+EijI(3,1)*P(
     -   31)+EE0I*P(77)+EijI(1,1)*P(78)+EijI(4,1)*P(79))
       F(34)=COMPLEX(FR(34),FI(34))
       FR(35) = 4*Invs15M*ME*(2*C0235R+Cij235R(1,1))
       FI(35) = 4*Invs15M*ME*(2*C0235I+Cij235I(1,1))
       F(35)=COMPLEX(FR(35),FI(35))
       FR(36) = P(1)
       FI(36) =0
       F(36)=COMPLEX(FR(36),FI(36))
       FR(37) = Cij235R(2,1)
       FI(37) = Cij235I(2,1)
       F(37)=COMPLEX(FR(37),FI(37))
       FR(38) = 2*Invs15M*ME*(2*C0235R+Cij235R(1,1))
       FI(38) = 2*Invs15M*ME*(2*C0235I+Cij235I(1,1))
       F(38)=COMPLEX(FR(38),FI(38))
       FR(39) = -4*Invs12M*ME*(2*C0135R+Cij135R(1,1))
       FI(39) = -4*Invs12M*ME*(2*C0135I+Cij135I(1,1))
       F(39)=COMPLEX(FR(39),FI(39))
       FR(40) = 2*Invs12M*(B015R-B035R+s34*Cij135R(2,1)-(C0135R+Cij13
     -   5R(1,1))*P(2))
       FI(40) = 2*Invs12M*(B015I-B035I+s34*Cij135I(2,1)-(C0135I+Cij13
     -   5I(1,1))*P(2))
       F(40)=COMPLEX(FR(40),FI(40))
       FR(41) = 2*Invs12M*ME*(2*C0135R+Cij135R(1,1))
       FI(41) = 2*Invs12M*ME*(2*C0135I+Cij135I(1,1))
       F(41)=COMPLEX(FR(41),FI(41))
       FR(42) = 4*ME*(Dij1235R(2,2)+2*(Dij1235R(2,1)-Dij1235R(3,1))-Di
     -   j1235R(6,2))
       FI(42) = 4*ME*(Dij1235I(2,2)+2*(Dij1235I(2,1)-Dij1235I(3,1))-Di
     -   j1235I(6,2))
       F(42)=COMPLEX(FR(42),FI(42))
       FR(43) = 4*ME*(D01235R+Dij1235R(2,1)+2*(Dij1235R(1,1)-Dij1235R(
     -   3,1))+Dij1235R(4,2)-Dij1235R(6,2))
       FI(43) = 4*ME*(D01235I+Dij1235I(2,1)+2*(Dij1235I(1,1)-Dij1235I(
     -   3,1))+Dij1235I(4,2)-Dij1235I(6,2))
       F(43)=COMPLEX(FR(43),FI(43))
       FR(44) = -4*ME*(2*Dij1235R(3,1)+Dij1235R(6,2))
       FI(44) = -4*ME*(2*Dij1235I(3,1)+Dij1235I(6,2))
       F(44)=COMPLEX(FR(44),FI(44))
       FR(45) = 2*(C0235R-2*Dij1235R(7,2)+Dij1235R(3,1)*P(3))
       FI(45) = 2*(C0235I-2*Dij1235I(7,2)+Dij1235I(3,1)*P(3))
       F(45)=COMPLEX(FR(45),FI(45))
       FR(46) = Dij1235R(3,2)+Dij1235R(4,2)-Dij1235R(5,2)-Dij1235R(6,2
     -   )
       FI(46) = Dij1235I(3,2)+Dij1235I(4,2)-Dij1235I(5,2)-Dij1235I(6,2
     -   )
       F(46)=COMPLEX(FR(46),FI(46))
       FR(47) = -4*(Dij1235R(1,1)+Dij1235R(1,2)-Dij1235R(3,1)+Dij1235R
     -   (3,2)-2*Dij1235R(5,2))
       FI(47) = -4*(Dij1235I(1,1)+Dij1235I(1,2)-Dij1235I(3,1)+Dij1235I
     -   (3,2)-2*Dij1235I(5,2))
       F(47)=COMPLEX(FR(47),FI(47))
       FR(48) = 4*(Dij1235R(3,1)-Dij1235R(3,2)+Dij1235R(5,2))
       FI(48) = 4*(Dij1235I(3,1)-Dij1235I(3,2)+Dij1235I(5,2))
       F(48)=COMPLEX(FR(48),FI(48))
       FR(49) = -2*ME*Dij1235R(2,1)
       FI(49) = -2*ME*Dij1235I(2,1)
       F(49)=COMPLEX(FR(49),FI(49))
       endif 
c       Amplitude                         
c************************************************************************************
c************************************************************************************
c************************************************************************************
c************************************************************************************
c************************************************************************************
c************************************************************************************
c************************************************************************************
c************************************************************************************


       hel(1) = (2*Fs)/(2*MESQ-s12-s15+s34)+(Fq*(INVs15M*(MESQ-s15)+IN
     -   Vs12M*(MESQ+s12-2*(s15-s23+s45))))/s34
       hel(2) = (-2*Fq*(INVs12M+INVs15M))/s34
       hel(3) = (-4*Fq*INVs12M)/s34-(4*Fs)/((2*MESQ-s12-s15+s34)*(MESQ
     -   -s15+s23-s45))
       hel(4) = 4*((Fq*INVs12M)/s34+Fs/((2*MESQ-s12-s15+s34)*(-MESQ+s1
     -   2+s23-s45)))
       hel(5) = 4*((Fq*INVs12M)/s34+Fs/((2*MESQ-s12-s15+s34)*(-MESQ+s1
     -   2+s23-s45)))
       hel(6) = (4*Fs*(-2*MESQ+s12+s15))/((2*MESQ-s12-s15+s34)*(MESQ-s
     -   15+s23-s45)*(MESQ-s12-s23+s45))
       hel(7) = (-4*Fq*INVs15M)/s34+(4*Fs)/((2*MESQ-s12-s15+s34)*(-MES
     -   Q+s12+s23-s45))
       ssa(1) = 4*MESQ-s12-s15+s34
       ssa(2) = 3*ME4SQ+s12*s15+2*s15**2+MU2SQ*(s12+s15)-s12*s23-2*s15
     -   *s23-2*s15*s34+s23*s34+MESQ*(-2*MU2SQ-6*s15+5*s23+2*s34-3*s4
     -   5)+s15*s45-s34*s45
       ssa(3) = ME4SQ+MU2SQ*(s12-s15)+s12*s15-s12*s23+s23*s34+MESQ*(-2
     -   *s15+s23-s45)+s15*s45-s34*s45
       ssa(4) = MESQ+2*MU2SQ+s15-s23-s34-s45
       ssa(5) = MU4SQ+MESQ*(3*MU2SQ+s15-s23-s34)+(-s15+s23+s34)*s45-MU
     -   2SQ*(s12+s23+s45)
       ssa(6) = -MESQ-2*MU2SQ-s15+s23+s34+s45
       ssa(7) = ME6SQ+ME4SQ*(2*MU2SQ-3*s15+2*s23-2*s45)-(s15-s23+s45)*
     -   (s12*(s15-s23)+s23*s34+s15*s45-s34*s45)+MESQ*(s12*s15+2*s15*
     -   *2-MU2SQ*(s12+3*s15)-s12*s23-3*s15*s23+s23**2+s23*s34+4*s15*
     -   s45-2*s23*s45-s34*s45+s45**2)+MU2SQ*(s12*(s15+s23-s45)+s15*(
     -   s15-s23+s45))
       ssa(8) = MESQ-s12
       ssa(9) = MESQ-s15
       ssa(10) = MESQ-s15+s23-s45
       ssa(11) = MU2SQ+s15-s23-s34
       ssa(12) = MESQ+MU2SQ-s45
       ssa(13) = MESQ*(2*MU2SQ+s15-s23-s34)-(s15-s23-s34)*(s15-s23+s45
     -   )-MU2SQ*(s12+s15-s23+s45)
       ssa(14) = -MU2SQ-s15+s23+s34
       ssa(15) = MU4SQ*(s12+s15)+ME4SQ*(-MU2SQ-s15+s23+s34)-MESQ*(2*MU
     -   4SQ-(s15-s23-s34)*(2*s12+s23+s45)-MU2SQ*(6*s12-2*s15+3*s23+s
     -   45))-(s15-s23-s34)*(s23*s34+(s15-s34)*s45+s12*(s15-s23+2*s45
     -   ))-MU2SQ*(2*s12**2-s15**2+s34*(s23-s45)+s15*(s23+s34+s45)+s1
     -   2*(2*s15-s34+2*s45))
       ssa(16) = ME4SQ*(5*MU2SQ+2*s15-2*s23-2*s34)+2*(s15-s23-s34)*s45
     -   *(s15-s23+s45)-MU4SQ*(s12+s15-2*s23+2*s45)+MU2SQ*(-(s23*(-2*
     -   s15+2*s23+s34))+(s15+s34)*s45+2*s45**2+s12*(s15-s23+2*s45))+
     -   MESQ*(2*MU4SQ-2*(s15-s23-s34)*(s15-s23+2*s45)-MU2SQ*(2*s12+4
     -   *s15-3*s23+7*s45))
       ssa(17) = MESQ*(3*MU2SQ+s15-s23-s34)-(s15-s23-s34)*(s15-s23+s45
     -   )-MU2SQ*(s12+2*s15-s23+s45)
       ssa(18) = s12+s15-s34
       ssa(19) = -MU4SQ+MESQ*(-3*MU2SQ-s15+s23+s34)+(s15-s23-s34)*s45+
     -   MU2SQ*(s12+s23+s45)
       ssa(20) = -MESQ+s15-s23+s45
       ssav(1) = 2*ME*(-2*(MESQ+s12)+s34)*CONJG(F(39))+4*(4*MESQ-s12-s
     -   15+s34)*CONJG(F(40))+4*ME*(3*MESQ-2*s12-s15)*CONJG(F(41))
       ssav(2) = -(ME*(ME4SQ-s12*s15+MU2SQ*(s12+s15)+s12*s23-s23*s34-2
     -   *s12*s45-s15*s45+s34*s45-MESQ*(2*MU2SQ-2*s12+2*s15-3*s23+s45
     -   ))*CONJG(F(39)))+2*(3*ME4SQ+s12*s15+2*s15**2+MU2SQ*(s12+s15)
     -   -s12*s23-2*s15*s23-2*s15*s34+s23*s34+MESQ*(-2*MU2SQ-6*s15+5*
     -   s23+2*s34-3*s45)+s15*s45-s34*s45)*CONJG(F(40))+4*ME*(MESQ-s1
     -   5)*(MESQ-s15+s23-s45)*CONJG(F(41))
       ssav(3) = -(ME*(s12-s15)*(MESQ+MU2SQ-s34-s45)*CONJG(F(39)))+(ME
     -   4SQ+MU2SQ*(s12-s15)+s12*s15-s12*s23+s23*s34+MESQ*(-2*s15+s23
     -   -s45)+s15*s45-s34*s45)*CONJG(F(40))+2*ME*(MESQ-s12)*(MESQ-s1
     -   5+s23-s45)*CONJG(F(41))
       ssav(4) = ME*(MESQ-s12)*(MESQ+2*MU2SQ+s15-s23-s34-s45)*CONJG(F(
     -   39))+(ME4SQ+MU2SQ*(s12-s15)+s12*s15-s12*s23+s23*s34+MESQ*(-2
     -   *s15+s23-s45)+s15*s45-s34*s45)*CONJG(F(40))
       ssav(5) = ME*(-((MESQ+s12)*(MESQ+2*MU2SQ+s15-s23-s34-s45)*CONJG
     -   (F(39)))+2*ME*(MESQ+2*MU2SQ+s15-s23-s34-s45)*CONJG(F(40))+(3
     -   *ME4SQ-s12*s15-MU2SQ*(3*s12+s15)+s12*s23+2*s12*s34+s23*s34+M
     -   ESQ*(4*MU2SQ-2*s12-s23-2*s34-3*s45)+(2*s12+s15-s34)*s45)*CON
     -   JG(F(41)))
       ssav(6) = -(ME*(MESQ+MU2SQ-s34-s45)*(MESQ+2*MU2SQ+s15-s23-s34-s
     -   45)*CONJG(F(39)))+2*(MU4SQ+MESQ*(3*MU2SQ+s15-s23-s34)+(-s15+
     -   s23+s34)*s45-MU2SQ*(s12+s23+s45))*CONJG(F(40))-2*ME*(ME4SQ+M
     -   U2SQ*(s12+s23-s45)+s45*(s15-s23+s45)-MESQ*(MU2SQ+s15-s23+2*s
     -   45))*CONJG(F(41))
       ssav(7) = ME*((MESQ+s12-s34)*(MESQ+2*MU2SQ+s15-s23-s34-s45)*CON
     -   JG(F(39))+2*ME*(-MESQ-2*MU2SQ-s15+s23+s34+s45)*CONJG(F(40))+
     -   (ME4SQ+MU2SQ*(s12-s15)+s12*s15-s12*s23+s23*s34+MESQ*(-2*s15+
     -   s23-s45)+s15*s45-s34*s45)*CONJG(F(41)))
       ssav(8) = 2*(-(ME*(2*MESQ+s12+s15-s34)*CONJG(F(42)))+ME*(-s12+s
     -   15)*CONJG(F(43))+ME*(2*MESQ+s12+s15-s34)*CONJG(F(44))+2*(4*M
     -   ESQ-s12-s15+s34)*CONJG(F(45))+4*MESQ*(-s12+s15)*CONJG(F(46))
     -   -(MESQ-s12)*(MESQ-s15)*CONJG(F(47))+MESQ*(-s12+s15)*CONJG(F(
     -   48))+2*ME*(3*MESQ-2*s12-s15)*CONJG(F(49)))
       ssav(9) = -(ME*(ME4SQ-s12*s15+MU2SQ*(s12+s15)+s12*s23-s23*s34-2
     -   *s12*s45-s15*s45+s34*s45-MESQ*(2*MU2SQ-2*s12+2*s15-3*s23+s45
     -   ))*CONJG(F(42)))+ME*(3*ME4SQ-s12*s15-2*s15**2-MU2SQ*(s12+s15
     -   )+s12*s23+2*s15*s23+2*s15*s34-s23*s34+MESQ*(2*MU2SQ+s23-2*s3
     -   4-3*s45)-s15*s45+s34*s45)*CONJG(F(44))+2*(3*ME4SQ+s12*s15+2*
     -   s15**2+MU2SQ*(s12+s15)-s12*s23-2*s15*s23-2*s15*s34+s23*s34+M
     -   ESQ*(-2*MU2SQ-6*s15+5*s23+2*s34-3*s45)+s15*s45-s34*s45)*CONJ
     -   G(F(45))-4*(ME6SQ+ME4SQ*(2*MU2SQ+s12-s23-2*s34-s45)+s12*(2*s
     -   15**2+MU2SQ*(s12+s15)+s12*(s15-s23)-2*s15*s23-2*s15*s34+s23*
     -   s34+s15*s45-s34*s45)+MESQ*(-5*s12*s15-MU2SQ*(3*s12+s15)+4*s1
     -   2*s23+2*s12*s34+2*s15*s34-s23*s34+(-s12+s15+s34)*s45))*CONJG
     -   (F(46))-2*MESQ*(MESQ-s15)*(MESQ-s15+s23-s45)*CONJG(F(48))+4*
     -   ME*(MESQ-s15)*(MESQ-s15+s23-s45)*CONJG(F(49))
       ssav(10) = ME*(s12-s15)*(-MU2SQ-s15+s23+s34)*CONJG(F(42))+ME*(s
     -   12-s15)*(-MESQ+s15-s23+s45)*CONJG(F(43))+ME*(s12-s15)*(MESQ+
     -   MU2SQ-s45)*CONJG(F(44))+(ME4SQ+MU2SQ*(s12-s15)+s12*s15-s12*s
     -   23+s23*s34+MESQ*(-2*s15+s23-s45)+s15*s45-s34*s45)*CONJG(F(45
     -   ))+4*(MESQ-s12)*(MESQ-s15)*(MU2SQ+s15-s23-s34)*CONJG(F(46))+
     -   (MESQ-s12)*(MESQ-s15)*(-MESQ+s15-s23+s45)*CONJG(F(47))+(MESQ
     -   -s12)*(MESQ-s15)*(MESQ+MU2SQ-s45)*CONJG(F(48))+2*ME*(MESQ-s1
     -   2)*(MESQ-s15+s23-s45)*CONJG(F(49))
       ssav(11) = ME*(MESQ-s12)*(MESQ+2*MU2SQ+s15-s23-s34-s45)*CONJG(F
     -   (42))+ME*(MESQ-s15)*(MESQ+2*MU2SQ+s15-s23-s34-s45)*CONJG(F(4
     -   4))+(ME4SQ+MU2SQ*(s12-s15)+s12*s15-s12*s23+s23*s34+MESQ*(-2*
     -   s15+s23-s45)+s15*s45-s34*s45)*CONJG(F(45))+2*(MESQ-s12)*(ME4
     -   SQ+MU2SQ*(s12-s15)+s12*s15-s12*s23+s23*s34+MESQ*(-2*s15+s23-
     -   s45)+s15*s45-s34*s45)*CONJG(F(46))-((MESQ-s15)*(ME4SQ+MU2SQ*
     -   (s12-s15)+s12*s15-s12*s23+s23*s34+MESQ*(-2*s15+s23-s45)+s15*
     -   s45-s34*s45)*CONJG(F(48)))/2q0
       ssav(12) = 2*ME3SQ*(-MESQ-2*MU2SQ-s15+s23+s34+s45)*CONJG(F(42))
     -   +ME*(MESQ-s12)*(MESQ+2*MU2SQ+s15-s23-s34-s45)*CONJG(F(43))+M
     -   E*(s12+s15-s34)*(MESQ+2*MU2SQ+s15-s23-s34-s45)*CONJG(F(44))+
     -   2*MESQ*(MESQ+2*MU2SQ+s15-s23-s34-s45)*CONJG(F(45))-4*MESQ*(M
     -   E4SQ+MU2SQ*(s12-s15)+s12*s15-s12*s23+s23*s34+MESQ*(-2*s15+s2
     -   3-s45)+s15*s45-s34*s45)*CONJG(F(46))-((MESQ-s12)*(ME4SQ+MU2S
     -   Q*(s12-s15)+s12*s15-s12*s23+s23*s34+MESQ*(-2*s15+s23-s45)+s1
     -   5*s45-s34*s45)*CONJG(F(47)))/2q0-((s12+s15-s34)*(ME4SQ+MU2SQ
     -   *(s12-s15)+s12*s15-s12*s23+s23*s34+MESQ*(-2*s15+s23-s45)+s15
     -   *s45-s34*s45)*CONJG(F(48)))/2q0+ME*(3*ME4SQ-s12*s15-MU2SQ*(3
     -   *s12+s15)+s12*s23+2*s12*s34+s23*s34+MESQ*(4*MU2SQ-2*s12-s23-
     -   2*s34-3*s45)+(2*s12+s15-s34)*s45)*CONJG(F(49))
       ssav(13) = ME*(-MU2SQ-s15+s23+s34)*(MESQ+2*MU2SQ+s15-s23-s34-s4
     -   5)*CONJG(F(42))-ME*(MESQ-s15+s23-s45)*(MESQ+2*MU2SQ+s15-s23-
     -   s34-s45)*CONJG(F(43))+ME*(MESQ+MU2SQ-s45)*(MESQ+2*MU2SQ+s15-
     -   s23-s34-s45)*CONJG(F(44))+2*(MU4SQ+MESQ*(3*MU2SQ+s15-s23-s34
     -   )+(-s15+s23+s34)*s45-MU2SQ*(s12+s23+s45))*CONJG(F(45))+2*(-M
     -   U2SQ-s15+s23+s34)*(ME4SQ+MU2SQ*(s12-s15)+s12*s15-s12*s23+s23
     -   *s34+MESQ*(-2*s15+s23-s45)+s15*s45-s34*s45)*CONJG(F(46))+((M
     -   ESQ-s15+s23-s45)*(ME4SQ+MU2SQ*(s12-s15)+s12*s15-s12*s23+s23*
     -   s34+MESQ*(-2*s15+s23-s45)+s15*s45-s34*s45)*CONJG(F(47)))/2q0
     -   -((MESQ+MU2SQ-s45)*(ME4SQ+MU2SQ*(s12-s15)+s12*s15-s12*s23+s2
     -   3*s34+MESQ*(-2*s15+s23-s45)+s15*s45-s34*s45)*CONJG(F(48)))/2
     -   q0-2*ME*(ME4SQ+MU2SQ*(s12+s23-s45)+s45*(s15-s23+s45)-MESQ*(M
     -   U2SQ+s15-s23+2*s45))*CONJG(F(49))
       ssav(14) = ME*(s12+s15-s34)*(MESQ+2*MU2SQ+s15-s23-s34-s45)*CONJ
     -   G(F(42))+ME*(MESQ-s15)*(MESQ+2*MU2SQ+s15-s23-s34-s45)*CONJG(
     -   F(43))+2*ME3SQ*(-MESQ-2*MU2SQ-s15+s23+s34+s45)*CONJG(F(44))+
     -   2*MESQ*(-MESQ-2*MU2SQ-s15+s23+s34+s45)*CONJG(F(45))+2*(s12+s
     -   15-s34)*(ME4SQ+MU2SQ*(s12-s15)+s12*s15-s12*s23+s23*s34+MESQ*
     -   (-2*s15+s23-s45)+s15*s45-s34*s45)*CONJG(F(46))-((MESQ-s15)*(
     -   ME4SQ+MU2SQ*(s12-s15)+s12*s15-s12*s23+s23*s34+MESQ*(-2*s15+s
     -   23-s45)+s15*s45-s34*s45)*CONJG(F(47)))/2q0+MESQ*(ME4SQ+MU2SQ
     -   *(s12-s15)+s12*s15-s12*s23+s23*s34+MESQ*(-2*s15+s23-s45)+s15
     -   *s45-s34*s45)*CONJG(F(48))+ME*(ME4SQ+MU2SQ*(s12-s15)+s12*s15
     -   -s12*s23+s23*s34+MESQ*(-2*s15+s23-s45)+s15*s45-s34*s45)*CONJ
     -   G(F(49))
       ssav(15) = 2*ME*(-MESQ-2*MU2SQ-s15+s23+s34+s45)*CONJG(F(7))+2*M
     -   E*(-2*(MESQ+s12)+s34)*CONJG(F(8))+4*(4*MESQ-s12-s15+s34)*CON
     -   JG(F(9))+(ME4SQ+MU2SQ*(s12-s15)+s12*s15-s12*s23+s23*s34+MESQ
     -   *(-2*s15+s23-s45)+s15*s45-s34*s45)*CONJG(F(10))+(3*ME4SQ+MU2
     -   SQ*(s12-s15)+s12*s15-s12*s23+s23*s34+MESQ*(4*MU2SQ-s23-2*s34
     -   -3*s45)+s15*s45-s34*s45)*CONJG(F(11))+4*ME*(3*MESQ-2*s12-s15
     -   )*CONJG(F(12))+4*ME*(MESQ-MU2SQ-2*s15+2*(s23+s34)-s45)*CONJG
     -   (F(13))+2*(3*ME4SQ+s12*s15+2*s15**2+MU2SQ*(s12+s15)-s12*s23-
     -   2*s15*s23-2*s15*s34+s23*s34+MESQ*(-2*MU2SQ-6*s15+5*s23+2*s34
     -   -3*s45)+s15*s45-s34*s45)*CONJG(F(14))
       ssav(16) = -2*ME*(ME4SQ+MESQ*(2*MU2SQ-s34-2*s45)-(s15-s23-s34-s
     -   45)*(s15-s23+s45)-MU2SQ*(s12+s15-2*s23+2*s45))*CONJG(F(7))-M
     -   E*(ME4SQ-s12*s15+MU2SQ*(s12+s15)+s12*s23-s23*s34-2*s12*s45-s
     -   15*s45+s34*s45-MESQ*(2*MU2SQ-2*s12+2*s15-3*s23+s45))*CONJG(F
     -   (8))+2*(3*ME4SQ+s12*s15+2*s15**2+MU2SQ*(s12+s15)-s12*s23-2*s
     -   15*s23-2*s15*s34+s23*s34+MESQ*(-2*MU2SQ-6*s15+5*s23+2*s34-3*
     -   s45)+s15*s45-s34*s45)*CONJG(F(9))+(ME6SQ+ME4SQ*(2*MU2SQ-3*s1
     -   5+2*s23-2*s45)-(s15-s23+s45)*(s12*(s15-s23)+s23*s34+s15*s45-
     -   s34*s45)+MESQ*(s12*s15+2*s15**2-MU2SQ*(s12+3*s15)-s12*s23-3*
     -   s15*s23+s23**2+s23*s34+4*s15*s45-2*s23*s45-s34*s45+s45**2)+M
     -   U2SQ*(s12*(s15+s23-s45)+s15*(s15-s23+s45)))*CONJG(F(10))+(MU
     -   4SQ*(s12+s15)+ME4SQ*(-MU2SQ-s15+s23+s34)-MESQ*(2*MU4SQ-(s15-
     -   s23-s34)*(2*s12+s23+s45)-MU2SQ*(6*s12-2*s15+3*s23+s45))-(s15
     -   -s23-s34)*(s23*s34+(s15-s34)*s45+s12*(s15-s23+2*s45))-MU2SQ*
     -   (2*s12**2-s15**2+s34*(s23-s45)+s15*(s23+s34+s45)+s12*(2*s15-
     -   s34+2*s45)))*CONJG(F(11))+4*ME*(MESQ-s15)*(MESQ-s15+s23-s45)
     -   *CONJG(F(12))+4*ME*(MESQ*(-3*MU2SQ+2*(-s15+s23+s34))+2*(s15-
     -   s23-s34)*(s15-s23+s45)+MU2SQ*(2*s12+s15-2*s23+2*s45))*CONJG(
     -   F(13))-4*(MESQ-s15)*(MESQ*(2*MU2SQ+s15-s23-s34)-(s15-s23-s34
     -   )*(s15-s23+s45)-MU2SQ*(s12+s15-s23+s45))*CONJG(F(14))
       ssav(17) = 2*ME*MU2SQ*(-s12+s15)*CONJG(F(7))-ME*(s12-s15)*(MESQ
     -   +MU2SQ-s34-s45)*CONJG(F(8))+(ME4SQ+MU2SQ*(s12-s15)+s12*s15-s
     -   12*s23+s23*s34+MESQ*(-2*s15+s23-s45)+s15*s45-s34*s45)*CONJG(
     -   F(9))-2*MU2SQ*(MESQ-s12)*(MESQ-s15)*CONJG(F(10))+((MESQ+MU2S
     -   Q-s34-s45)*(ME4SQ+MU2SQ*(s12-s15)+s12*s15-s12*s23+s23*s34+ME
     -   SQ*(-2*s15+s23-s45)+s15*s45-s34*s45)*CONJG(F(11)))/2q0+2*ME*
     -   (MESQ-s12)*(MESQ-s15+s23-s45)*CONJG(F(12))+2*ME*MU2SQ*(-s12+
     -   s15)*CONJG(F(13))+(ME6SQ+ME4SQ*(2*MU2SQ-3*s15+2*s23-2*s45)-(
     -   s15-s23+s45)*(s12*(s15-s23)+s23*s34+s15*s45-s34*s45)+MESQ*(s
     -   12*s15+2*s15**2-MU2SQ*(s12+3*s15)-s12*s23-3*s15*s23+s23**2+s
     -   23*s34+4*s15*s45-2*s23*s45-s34*s45+s45**2)+MU2SQ*(s12*(s15+s
     -   23-s45)+s15*(s15-s23+s45)))*CONJG(F(14))
       ssav(18) = -(ME*(MESQ-s15+s23-s45)*(MESQ+2*MU2SQ+s15-s23-s34-s4
     -   5)*CONJG(F(7)))+ME*(MESQ-s12)*(MESQ+2*MU2SQ+s15-s23-s34-s45)
     -   *CONJG(F(8))+(ME4SQ+MU2SQ*(s12-s15)+s12*s15-s12*s23+s23*s34+
     -   MESQ*(-2*s15+s23-s45)+s15*s45-s34*s45)*CONJG(F(9))+((MESQ-s1
     -   5+s23-s45)*(ME4SQ+MU2SQ*(s12-s15)+s12*s15-s12*s23+s23*s34+ME
     -   SQ*(-2*s15+s23-s45)+s15*s45-s34*s45)*CONJG(F(10)))/2q0-(MESQ
     -   -s12)*(MU4SQ+MESQ*(3*MU2SQ+s15-s23-s34)+(-s15+s23+s34)*s45-M
     -   U2SQ*(s12+s23+s45))*CONJG(F(11))-2*ME*(MESQ*(3*MU2SQ+s15-s23
     -   -s34)-(s15-s23-s34)*(s15-s23+s45)-MU2SQ*(s12+2*s15-s23+s45))
     -   *CONJG(F(13))
       ssav(19) = ME*(-MU2SQ-s15+s23+s34)*(MESQ+2*MU2SQ+s15-s23-s34-s4
     -   5)*CONJG(F(7))-ME*(MESQ+s12)*(MESQ+2*MU2SQ+s15-s23-s34-s45)*
     -   CONJG(F(8))+2*MESQ*(MESQ+2*MU2SQ+s15-s23-s34-s45)*CONJG(F(9)
     -   )+((MU2SQ+s15-s23-s34)*(ME4SQ+MU2SQ*(s12-s15)+s12*s15-s12*s2
     -   3+s23*s34+MESQ*(-2*s15+s23-s45)+s15*s45-s34*s45)*CONJG(F(10)
     -   ))/2q0+(MESQ+s12)*(MU4SQ+MESQ*(3*MU2SQ+s15-s23-s34)+(-s15+s2
     -   3+s34)*s45-MU2SQ*(s12+s23+s45))*CONJG(F(11))+ME*(3*ME4SQ-s12
     -   *s15-MU2SQ*(3*s12+s15)+s12*s23+2*s12*s34+s23*s34+MESQ*(4*MU2
     -   SQ-2*s12-s23-2*s34-3*s45)+(2*s12+s15-s34)*s45)*CONJG(F(12))+
     -   2*ME*(2*MESQ*MU2SQ-MU4SQ-MU2SQ*(s12+3*s15-2*s23-3*s34)-(-s15
     -   +s23+s34)**2)*CONJG(F(13))+(MU4SQ*(s12+s15)+ME4SQ*(-MU2SQ-s1
     -   5+s23+s34)-MESQ*(2*MU4SQ-(s15-s23-s34)*(2*s12+s23+s45)-MU2SQ
     -   *(6*s12-2*s15+3*s23+s45))-(s15-s23-s34)*(s23*s34+(s15-s34)*s
     -   45+s12*(s15-s23+2*s45))-MU2SQ*(2*s12**2-s15**2+s34*(s23-s45)
     -   +s15*(s23+s34+s45)+s12*(2*s15-s34+2*s45)))*CONJG(F(14))
       ssav(20) = 2*ME*MU2SQ*(-MESQ-2*MU2SQ-s15+s23+s34+s45)*CONJG(F(7
     -   ))-ME*(MESQ+MU2SQ-s34-s45)*(MESQ+2*MU2SQ+s15-s23-s34-s45)*CO
     -   NJG(F(8))+2*(MU4SQ+MESQ*(3*MU2SQ+s15-s23-s34)+(-s15+s23+s34)
     -   *s45-MU2SQ*(s12+s23+s45))*CONJG(F(9))+MU2SQ*(ME4SQ+MU2SQ*(s1
     -   2-s15)+s12*s15-s12*s23+s23*s34+MESQ*(-2*s15+s23-s45)+s15*s45
     -   -s34*s45)*CONJG(F(10))+(MESQ+MU2SQ-s34-s45)*(MU4SQ+MESQ*(3*M
     -   U2SQ+s15-s23-s34)+(-s15+s23+s34)*s45-MU2SQ*(s12+s23+s45))*CO
     -   NJG(F(11))-2*ME*(ME4SQ+MU2SQ*(s12+s23-s45)+s45*(s15-s23+s45)
     -   -MESQ*(MU2SQ+s15-s23+2*s45))*CONJG(F(12))+2*ME*MU2SQ*(-MESQ-
     -   2*MU2SQ-s15+s23+s34+s45)*CONJG(F(13))+(ME4SQ*(5*MU2SQ+2*s15-
     -   2*s23-2*s34)+2*(s15-s23-s34)*s45*(s15-s23+s45)-MU4SQ*(s12+s1
     -   5-2*s23+2*s45)+MU2SQ*(-(s23*(-2*s15+2*s23+s34))+(s15+s34)*s4
     -   5+2*s45**2+s12*(s15-s23+2*s45))+MESQ*(2*MU4SQ-2*(s15-s23-s34
     -   )*(s15-s23+2*s45)-MU2SQ*(2*s12+4*s15-3*s23+7*s45)))*CONJG(F(
     -   14))
       ssav(21) = ME*(MESQ+MU2SQ-s45)*(MESQ+2*MU2SQ+s15-s23-s34-s45)*C
     -   ONJG(F(7))+ME*(MESQ+s12-s34)*(MESQ+2*MU2SQ+s15-s23-s34-s45)*
     -   CONJG(F(8))+2*MESQ*(-MESQ-2*MU2SQ-s15+s23+s34+s45)*CONJG(F(9
     -   ))-((MESQ+MU2SQ-s45)*(ME4SQ+MU2SQ*(s12-s15)+s12*s15-s12*s23+
     -   s23*s34+MESQ*(-2*s15+s23-s45)+s15*s45-s34*s45)*CONJG(F(10)))
     -   /2q0+(MESQ+s12-s34)*(-MU4SQ+MESQ*(-3*MU2SQ-s15+s23+s34)+(s15
     -   -s23-s34)*s45+MU2SQ*(s12+s23+s45))*CONJG(F(11))+ME*(ME4SQ+MU
     -   2SQ*(s12-s15)+s12*s15-s12*s23+s23*s34+MESQ*(-2*s15+s23-s45)+
     -   s15*s45-s34*s45)*CONJG(F(12))+2*ME*(MU4SQ+MESQ*(3*MU2SQ+s15-
     -   s23-s34)+(-s15+s23+s34)*s45-MU2SQ*(s12+s23+s45))*CONJG(F(13)
     -   )-2*MESQ*(MESQ*(3*MU2SQ+s15-s23-s34)-(s15-s23-s34)*(s15-s23+
     -   s45)-MU2SQ*(s12+2*s15-s23+s45))*CONJG(F(14))
       ssav(22) = 2*(ME*(2*MESQ+s12+s15-s34)*CONJG(F(1))+2*(4*MESQ-s12
     -   -s15+s34)*CONJG(F(2))+MESQ*(-MESQ-2*MU2SQ-s15+s23+s34+s45)*C
     -   ONJG(F(3))+2*ME*(3*MESQ-2*s12-s15)*CONJG(F(4))+2*ME*(MESQ-MU
     -   2SQ-2*s15+2*(s23+s34)-s45)*CONJG(F(5))+(3*ME4SQ+s12*s15+2*s1
     -   5**2+MU2SQ*(s12+s15)-s12*s23-2*s15*s23-2*s15*s34+s23*s34+MES
     -   Q*(-2*MU2SQ-6*s15+5*s23+2*s34-3*s45)+s15*s45-s34*s45)*CONJG(
     -   F(6)))
       ssav(23) = ME*(3*ME4SQ-s12*s15-2*s15**2-MU2SQ*(s12+s15)+s12*s23
     -   +2*s15*s23+2*s15*s34-s23*s34+MESQ*(2*MU2SQ+s23-2*s34-3*s45)-
     -   s15*s45+s34*s45)*CONJG(F(1))+2*(3*ME4SQ+s12*s15+2*s15**2+MU2
     -   SQ*(s12+s15)-s12*s23-2*s15*s23-2*s15*s34+s23*s34+MESQ*(-2*MU
     -   2SQ-6*s15+5*s23+2*s34-3*s45)+s15*s45-s34*s45)*CONJG(F(2))-2*
     -   MESQ*(MESQ*(3*MU2SQ+s15-s23-s34)-(s15-s23-s34)*(s15-s23+s45)
     -   -MU2SQ*(s12+2*s15-s23+s45))*CONJG(F(3))+4*ME*(MESQ-s15)*(MES
     -   Q-s15+s23-s45)*CONJG(F(4))+4*ME*(MESQ*(-3*MU2SQ+2*(-s15+s23+
     -   s34))+2*(s15-s23-s34)*(s15-s23+s45)+MU2SQ*(2*s12+s15-2*s23+2
     -   *s45))*CONJG(F(5))-4*(MESQ-s15)*(MESQ*(2*MU2SQ+s15-s23-s34)-
     -   (s15-s23-s34)*(s15-s23+s45)-MU2SQ*(s12+s15-s23+s45))*CONJG(F
     -   (6))
       ssav(24) = ME*(s12-s15)*(MESQ+MU2SQ-s45)*CONJG(F(1))+(ME4SQ+MU2
     -   SQ*(s12-s15)+s12*s15-s12*s23+s23*s34+MESQ*(-2*s15+s23-s45)+s
     -   15*s45-s34*s45)*CONJG(F(2))-((MESQ+MU2SQ-s45)*(ME4SQ+MU2SQ*(
     -   s12-s15)+s12*s15-s12*s23+s23*s34+MESQ*(-2*s15+s23-s45)+s15*s
     -   45-s34*s45)*CONJG(F(3)))/2q0+2*ME*(MESQ-s12)*(MESQ-s15+s23-s
     -   45)*CONJG(F(4))+2*ME*MU2SQ*(-s12+s15)*CONJG(F(5))+(ME6SQ+ME4
     -   SQ*(2*MU2SQ-3*s15+2*s23-2*s45)-(s15-s23+s45)*(s12*(s15-s23)+
     -   s23*s34+s15*s45-s34*s45)+MESQ*(s12*s15+2*s15**2-MU2SQ*(s12+3
     -   *s15)-s12*s23-3*s15*s23+s23**2+s23*s34+4*s15*s45-2*s23*s45-s
     -   34*s45+s45**2)+MU2SQ*(s12*(s15+s23-s45)+s15*(s15-s23+s45)))*
     -   CONJG(F(6))
       ssav(25) = ME*(MESQ-s15)*(MESQ+2*MU2SQ+s15-s23-s34-s45)*CONJG(F
     -   (1))+(ME4SQ+MU2SQ*(s12-s15)+s12*s15-s12*s23+s23*s34+MESQ*(-2
     -   *s15+s23-s45)+s15*s45-s34*s45)*CONJG(F(2))-(MESQ-s15)*(MU4SQ
     -   +MESQ*(3*MU2SQ+s15-s23-s34)+(-s15+s23+s34)*s45-MU2SQ*(s12+s2
     -   3+s45))*CONJG(F(3))-2*ME*(MESQ*(3*MU2SQ+s15-s23-s34)-(s15-s2
     -   3-s34)*(s15-s23+s45)-MU2SQ*(s12+2*s15-s23+s45))*CONJG(F(5))
       ssav(26) = ME*(s12+s15-s34)*(MESQ+2*MU2SQ+s15-s23-s34-s45)*CONJ
     -   G(F(1))+2*MESQ*(MESQ+2*MU2SQ+s15-s23-s34-s45)*CONJG(F(2))+(s
     -   12+s15-s34)*(-MU4SQ+MESQ*(-3*MU2SQ-s15+s23+s34)+(s15-s23-s34
     -   )*s45+MU2SQ*(s12+s23+s45))*CONJG(F(3))+ME*(3*ME4SQ-s12*s15-M
     -   U2SQ*(3*s12+s15)+s12*s23+2*s12*s34+s23*s34+MESQ*(4*MU2SQ-2*s
     -   12-s23-2*s34-3*s45)+(2*s12+s15-s34)*s45)*CONJG(F(4))+2*ME*(2
     -   *MESQ*MU2SQ-MU4SQ-MU2SQ*(s12+3*s15-2*s23-3*s34)-(-s15+s23+s3
     -   4)**2)*CONJG(F(5))+(MU4SQ*(s12+s15)+ME4SQ*(-MU2SQ-s15+s23+s3
     -   4)-MESQ*(2*MU4SQ-(s15-s23-s34)*(2*s12+s23+s45)-MU2SQ*(6*s12-
     -   2*s15+3*s23+s45))-(s15-s23-s34)*(s23*s34+(s15-s34)*s45+s12*(
     -   s15-s23+2*s45))-MU2SQ*(2*s12**2-s15**2+s34*(s23-s45)+s15*(s2
     -   3+s34+s45)+s12*(2*s15-s34+2*s45)))*CONJG(F(6))
       ssav(27) = ME*(MESQ+MU2SQ-s45)*(MESQ+2*MU2SQ+s15-s23-s34-s45)*C
     -   ONJG(F(1))+2*(MU4SQ+MESQ*(3*MU2SQ+s15-s23-s34)+(-s15+s23+s34
     -   )*s45-MU2SQ*(s12+s23+s45))*CONJG(F(2))-(MESQ+MU2SQ-s45)*(MU4
     -   SQ+MESQ*(3*MU2SQ+s15-s23-s34)+(-s15+s23+s34)*s45-MU2SQ*(s12+
     -   s23+s45))*CONJG(F(3))-2*ME*(ME4SQ+MU2SQ*(s12+s23-s45)+s45*(s
     -   15-s23+s45)-MESQ*(MU2SQ+s15-s23+2*s45))*CONJG(F(4))+2*ME*MU2
     -   SQ*(-MESQ-2*MU2SQ-s15+s23+s34+s45)*CONJG(F(5))+(ME4SQ*(5*MU2
     -   SQ+2*s15-2*s23-2*s34)+2*(s15-s23-s34)*s45*(s15-s23+s45)-MU4S
     -   Q*(s12+s15-2*s23+2*s45)+MU2SQ*(-(s23*(-2*s15+2*s23+s34))+(s1
     -   5+s34)*s45+2*s45**2+s12*(s15-s23+2*s45))+MESQ*(2*MU4SQ-2*(s1
     -   5-s23-s34)*(s15-s23+2*s45)-MU2SQ*(2*s12+4*s15-3*s23+7*s45)))
     -   *CONJG(F(6))
       ssav(28) = ME*(2*MESQ*(-MESQ-2*MU2SQ-s15+s23+s34+s45)*CONJG(F(1
     -   ))+2*ME*(-MESQ-2*MU2SQ-s15+s23+s34+s45)*CONJG(F(2))+2*ME*(MU
     -   4SQ+MESQ*(3*MU2SQ+s15-s23-s34)+(-s15+s23+s34)*s45-MU2SQ*(s12
     -   +s23+s45))*CONJG(F(3))+(ME4SQ+MU2SQ*(s12-s15)+s12*s15-s12*s2
     -   3+s23*s34+MESQ*(-2*s15+s23-s45)+s15*s45-s34*s45)*CONJG(F(4))
     -   +2*(MU4SQ+MESQ*(3*MU2SQ+s15-s23-s34)+(-s15+s23+s34)*s45-MU2S
     -   Q*(s12+s23+s45))*CONJG(F(5))-2*ME*(MESQ*(3*MU2SQ+s15-s23-s34
     -   )-(s15-s23-s34)*(s15-s23+s45)-MU2SQ*(s12+2*s15-s23+s45))*CON
     -   JG(F(6)))
       ssav(29) = -2*ME*(2*MESQ+s12+s15-s34)*CONJG(F(15))+2*ME*(-s12+s
     -   15)*CONJG(F(16))+2*ME*(-MESQ-2*MU2SQ-s15+s23+s34+s45)*CONJG(
     -   F(17))+2*ME*(2*MESQ+s12+s15-s34)*CONJG(F(18))+4*(4*MESQ-s12-
     -   s15+s34)*CONJG(F(19))-2*(MESQ-s12)*(MESQ-s15)*CONJG(F(20))+2
     -   *MESQ*(s12-s15)*CONJG(F(21))+(ME4SQ+MU2SQ*(s12-s15)+s12*s15-
     -   s12*s23+s23*s34+MESQ*(-2*s15+s23-s45)+s15*s45-s34*s45)*CONJG
     -   (F(22))+2*MESQ*(-s12+s15)*CONJG(F(23))+(ME4SQ+MU2SQ*(s12-s15
     -   )+s12*s15-s12*s23+s23*s34+MESQ*(-2*s15+s23-s45)+s15*s45-s34*
     -   s45)*CONJG(F(24))+2*MESQ*(MESQ+2*MU2SQ+s15-s23-s34-s45)*CONJ
     -   G(F(25))+2*(MU4SQ+MESQ*(3*MU2SQ+s15-s23-s34)+(-s15+s23+s34)*
     -   s45-MU2SQ*(s12+s23+s45))*CONJG(F(26))+2*MESQ*(-MESQ-2*MU2SQ-
     -   s15+s23+s34+s45)*CONJG(F(27))+4*ME*(3*MESQ-2*s12-s15)*CONJG(
     -   F(28))+4*ME*(MESQ-MU2SQ-2*s15+2*(s23+s34)-s45)*CONJG(F(29))+
     -   ME*(-3*ME4SQ+s12*s15+2*s15**2+MU2SQ*(s12+s15)-s12*s23-2*s15*
     -   s23-2*s15*s34+s23*s34-MESQ*(2*MU2SQ+s23-2*s34-3*s45)+s15*s45
     -   -s34*s45)*CONJG(F(30))-2*ME*(MESQ-s15)*(MESQ-s15+s23-s45)*CO
     -   NJG(F(31))-2*ME*(MESQ*(3*MU2SQ+s15-s23-s34)-(s15-s23-s34)*(s
     -   15-s23+s45)-MU2SQ*(s12+2*s15-s23+s45))*CONJG(F(32))+ME*(3*ME
     -   4SQ-s12*s15-2*s15**2-MU2SQ*(s12+s15)+s12*s23+2*s15*s23+2*s15
     -   *s34-s23*s34+MESQ*(2*MU2SQ+s23-2*s34-3*s45)-s15*s45+s34*s45)
     -   *CONJG(F(33))+2*(3*ME4SQ+s12*s15+2*s15**2+MU2SQ*(s12+s15)-s1
     -   2*s23-2*s15*s23-2*s15*s34+s23*s34+MESQ*(-2*MU2SQ-6*s15+5*s23
     -   +2*s34-3*s45)+s15*s45-s34*s45)*CONJG(F(34))
       ssav(30) = -(ME*(ME4SQ-s12*s15+MU2SQ*(s12+s15)+s12*s23-s23*s34-
     -   2*s12*s45-s15*s45+s34*s45-MESQ*(2*MU2SQ-2*s12+2*s15-3*s23+s4
     -   5))*CONJG(F(15)))-2*ME*(ME4SQ+MESQ*(2*MU2SQ-s34-2*s45)-(s15-
     -   s23-s34-s45)*(s15-s23+s45)-MU2SQ*(s12+s15-2*s23+2*s45))*CONJ
     -   G(F(17))+ME*(3*ME4SQ-s12*s15-2*s15**2-MU2SQ*(s12+s15)+s12*s2
     -   3+2*s15*s23+2*s15*s34-s23*s34+MESQ*(2*MU2SQ+s23-2*s34-3*s45)
     -   -s15*s45+s34*s45)*CONJG(F(18))+2*(3*ME4SQ+s12*s15+2*s15**2+M
     -   U2SQ*(s12+s15)-s12*s23-2*s15*s23-2*s15*s34+s23*s34+MESQ*(-2*
     -   MU2SQ-6*s15+5*s23+2*s34-3*s45)+s15*s45-s34*s45)*CONJG(F(19))
     -   +(ME6SQ+ME4SQ*(2*MU2SQ+s12-s23-2*s34-s45)+s12*(2*s15**2+MU2S
     -   Q*(s12+s15)+s12*(s15-s23)-2*s15*s23-2*s15*s34+s23*s34+s15*s4
     -   5-s34*s45)+MESQ*(-5*s12*s15-MU2SQ*(3*s12+s15)+4*s12*s23+2*s1
     -   2*s34+2*s15*s34-s23*s34+(-s12+s15+s34)*s45))*CONJG(F(21))+(M
     -   E6SQ+ME4SQ*(2*MU2SQ-3*s15+2*s23-2*s45)-(s15-s23+s45)*(s12*(s
     -   15-s23)+s23*s34+s15*s45-s34*s45)+MESQ*(s12*s15+2*s15**2-MU2S
     -   Q*(s12+3*s15)-s12*s23-3*s15*s23+s23**2+s23*s34+4*s15*s45-2*s
     -   23*s45-s34*s45+s45**2)+MU2SQ*(s12*(s15+s23-s45)+s15*(s15-s23
     -   +s45)))*CONJG(F(22))-2*MESQ*(MESQ-s15)*(MESQ-s15+s23-s45)*CO
     -   NJG(F(23))+(MU4SQ*(s12+s15)+ME4SQ*(-MU2SQ-s15+s23+s34)-MESQ*
     -   (2*MU4SQ-(s15-s23-s34)*(2*s12+s23+s45)-MU2SQ*(6*s12-2*s15+3*
     -   s23+s45))-(s15-s23-s34)*(s23*s34+(s15-s34)*s45+s12*(s15-s23+
     -   2*s45))-MU2SQ*(2*s12**2-s15**2+s34*(s23-s45)+s15*(s23+s34+s4
     -   5)+s12*(2*s15-s34+2*s45)))*CONJG(F(25))+(ME4SQ*(5*MU2SQ+2*s1
     -   5-2*s23-2*s34)+2*(s15-s23-s34)*s45*(s15-s23+s45)-MU4SQ*(s12+
     -   s15-2*s23+2*s45)+MU2SQ*(-(s23*(-2*s15+2*s23+s34))+(s15+s34)*
     -   s45+2*s45**2+s12*(s15-s23+2*s45))+MESQ*(2*MU4SQ-2*(s15-s23-s
     -   34)*(s15-s23+2*s45)-MU2SQ*(2*s12+4*s15-3*s23+7*s45)))*CONJG(
     -   F(26))-2*MESQ*(MESQ*(3*MU2SQ+s15-s23-s34)-(s15-s23-s34)*(s15
     -   -s23+s45)-MU2SQ*(s12+2*s15-s23+s45))*CONJG(F(27))+4*ME*(MESQ
     -   -s15)*(MESQ-s15+s23-s45)*CONJG(F(28))+4*ME*(MESQ*(-3*MU2SQ+2
     -   *(-s15+s23+s34))+2*(s15-s23-s34)*(s15-s23+s45)+MU2SQ*(2*s12+
     -   s15-2*s23+2*s45))*CONJG(F(29))+2*ME*(MESQ-s12)*(MESQ*(2*MU2S
     -   Q+s15-s23-s34)-(s15-s23-s34)*(s15-s23+s45)-MU2SQ*(s12+s15-s2
     -   3+s45))*CONJG(F(30))-2*ME*(MESQ-s15+s23-s45)*(MESQ*(2*MU2SQ+
     -   s15-s23-s34)-(s15-s23-s34)*(s15-s23+s45)-MU2SQ*(s12+s15-s23+
     -   s45))*CONJG(F(32))+2*ME*(MESQ-s15)*(MESQ*(2*MU2SQ+s15-s23-s3
     -   4)-(s15-s23-s34)*(s15-s23+s45)-MU2SQ*(s12+s15-s23+s45))*CONJ
     -   G(F(33))-4*(MESQ-s15)*(MESQ*(2*MU2SQ+s15-s23-s34)-(s15-s23-s
     -   34)*(s15-s23+s45)-MU2SQ*(s12+s15-s23+s45))*CONJG(F(34))
       ssav(31) = ME*(s12-s15)*(-MU2SQ-s15+s23+s34)*CONJG(F(15))+ME*(s
     -   12-s15)*(-MESQ+s15-s23+s45)*CONJG(F(16))+2*ME*MU2SQ*(-s12+s1
     -   5)*CONJG(F(17))+ME*(s12-s15)*(MESQ+MU2SQ-s45)*CONJG(F(18))+(
     -   ME4SQ+MU2SQ*(s12-s15)+s12*s15-s12*s23+s23*s34+MESQ*(-2*s15+s
     -   23-s45)+s15*s45-s34*s45)*CONJG(F(19))+(MESQ-s12)*(MESQ-s15)*
     -   (-MESQ+s15-s23+s45)*CONJG(F(20))+(MESQ-s12)*(MESQ-s15)*(-MU2
     -   SQ-s15+s23+s34)*CONJG(F(21))-2*MU2SQ*(MESQ-s12)*(MESQ-s15)*C
     -   ONJG(F(22))+(MESQ-s12)*(MESQ-s15)*(MESQ+MU2SQ-s45)*CONJG(F(2
     -   3))+((MESQ-s15+s23-s45)*(ME4SQ+MU2SQ*(s12-s15)+s12*s15-s12*s
     -   23+s23*s34+MESQ*(-2*s15+s23-s45)+s15*s45-s34*s45)*CONJG(F(24
     -   )))/2q0+((MU2SQ+s15-s23-s34)*(ME4SQ+MU2SQ*(s12-s15)+s12*s15-
     -   s12*s23+s23*s34+MESQ*(-2*s15+s23-s45)+s15*s45-s34*s45)*CONJG
     -   (F(25)))/2q0+MU2SQ*(ME4SQ+MU2SQ*(s12-s15)+s12*s15-s12*s23+s2
     -   3*s34+MESQ*(-2*s15+s23-s45)+s15*s45-s34*s45)*CONJG(F(26))-((
     -   MESQ+MU2SQ-s45)*(ME4SQ+MU2SQ*(s12-s15)+s12*s15-s12*s23+s23*s
     -   34+MESQ*(-2*s15+s23-s45)+s15*s45-s34*s45)*CONJG(F(27)))/2q0+
     -   2*ME*(MESQ-s12)*(MESQ-s15+s23-s45)*CONJG(F(28))+2*ME*MU2SQ*(
     -   -s12+s15)*CONJG(F(29))+ME*(MESQ-s15)*(-MU2SQ-s15+s23+s34)*(M
     -   ESQ-s15+s23-s45)*CONJG(F(30))-ME*(MESQ-s15)*(MESQ-s15+s23-s4
     -   5)**2*CONJG(F(31))-2*ME*MU2SQ*(MESQ-s15)*(MESQ-s15+s23-s45)*
     -   CONJG(F(32))+ME*(MESQ-s15)*(MESQ+MU2SQ-s45)*(MESQ-s15+s23-s4
     -   5)*CONJG(F(33))+(ME6SQ+ME4SQ*(2*MU2SQ-3*s15+2*s23-2*s45)-(s1
     -   5-s23+s45)*(s12*(s15-s23)+s23*s34+s15*s45-s34*s45)+MESQ*(s12
     -   *s15+2*s15**2-MU2SQ*(s12+3*s15)-s12*s23-3*s15*s23+s23**2+s23
     -   *s34+4*s15*s45-2*s23*s45-s34*s45+s45**2)+MU2SQ*(s12*(s15+s23
     -   -s45)+s15*(s15-s23+s45)))*CONJG(F(34))
       ssav(32) = ME*(MESQ-s12)*(MESQ+2*MU2SQ+s15-s23-s34-s45)*CONJG(F
     -   (15))-ME*(MESQ-s15+s23-s45)*(MESQ+2*MU2SQ+s15-s23-s34-s45)*C
     -   ONJG(F(17))+ME*(MESQ-s15)*(MESQ+2*MU2SQ+s15-s23-s34-s45)*CON
     -   JG(F(18))+(ME4SQ+MU2SQ*(s12-s15)+s12*s15-s12*s23+s23*s34+MES
     -   Q*(-2*s15+s23-s45)+s15*s45-s34*s45)*CONJG(F(19))-((MESQ-s12)
     -   *(ME4SQ+MU2SQ*(s12-s15)+s12*s15-s12*s23+s23*s34+MESQ*(-2*s15
     -   +s23-s45)+s15*s45-s34*s45)*CONJG(F(21)))/2q0+((MESQ-s15+s23-
     -   s45)*(ME4SQ+MU2SQ*(s12-s15)+s12*s15-s12*s23+s23*s34+MESQ*(-2
     -   *s15+s23-s45)+s15*s45-s34*s45)*CONJG(F(22)))/2q0-((MESQ-s15)
     -   *(ME4SQ+MU2SQ*(s12-s15)+s12*s15-s12*s23+s23*s34+MESQ*(-2*s15
     -   +s23-s45)+s15*s45-s34*s45)*CONJG(F(23)))/2q0-(MESQ-s12)*(MU4
     -   SQ+MESQ*(3*MU2SQ+s15-s23-s34)+(-s15+s23+s34)*s45-MU2SQ*(s12+
     -   s23+s45))*CONJG(F(25))+(-MESQ+s15-s23+s45)*(-MU4SQ+MESQ*(-3*
     -   MU2SQ-s15+s23+s34)+(s15-s23-s34)*s45+MU2SQ*(s12+s23+s45))*CO
     -   NJG(F(26))-(MESQ-s15)*(MU4SQ+MESQ*(3*MU2SQ+s15-s23-s34)+(-s1
     -   5+s23+s34)*s45-MU2SQ*(s12+s23+s45))*CONJG(F(27))-2*ME*(MESQ*
     -   (3*MU2SQ+s15-s23-s34)-(s15-s23-s34)*(s15-s23+s45)-MU2SQ*(s12
     -   +2*s15-s23+s45))*CONJG(F(29))+ME*(MESQ-s12)*(MESQ*(3*MU2SQ+s
     -   15-s23-s34)-(s15-s23-s34)*(s15-s23+s45)-MU2SQ*(s12+2*s15-s23
     -   +s45))*CONJG(F(30))-ME*(MESQ-s15+s23-s45)*(MESQ*(3*MU2SQ+s15
     -   -s23-s34)-(s15-s23-s34)*(s15-s23+s45)-MU2SQ*(s12+2*s15-s23+s
     -   45))*CONJG(F(32))+ME*(MESQ-s15)*(MESQ*(3*MU2SQ+s15-s23-s34)-
     -   (s15-s23-s34)*(s15-s23+s45)-MU2SQ*(s12+2*s15-s23+s45))*CONJG
     -   (F(33))
       ssav(33) = 2*ME3SQ*(-MESQ-2*MU2SQ-s15+s23+s34+s45)*CONJG(F(15))
     -   +ME*(MESQ-s12)*(MESQ+2*MU2SQ+s15-s23-s34-s45)*CONJG(F(16))+M
     -   E*(-MU2SQ-s15+s23+s34)*(MESQ+2*MU2SQ+s15-s23-s34-s45)*CONJG(
     -   F(17))+ME*(s12+s15-s34)*(MESQ+2*MU2SQ+s15-s23-s34-s45)*CONJG
     -   (F(18))+2*MESQ*(MESQ+2*MU2SQ+s15-s23-s34-s45)*CONJG(F(19))-(
     -   (MESQ-s12)*(ME4SQ+MU2SQ*(s12-s15)+s12*s15-s12*s23+s23*s34+ME
     -   SQ*(-2*s15+s23-s45)+s15*s45-s34*s45)*CONJG(F(20)))/2q0+MESQ*
     -   (ME4SQ+MU2SQ*(s12-s15)+s12*s15-s12*s23+s23*s34+MESQ*(-2*s15+
     -   s23-s45)+s15*s45-s34*s45)*CONJG(F(21))+((MU2SQ+s15-s23-s34)*
     -   (ME4SQ+MU2SQ*(s12-s15)+s12*s15-s12*s23+s23*s34+MESQ*(-2*s15+
     -   s23-s45)+s15*s45-s34*s45)*CONJG(F(22)))/2q0-((s12+s15-s34)*(
     -   ME4SQ+MU2SQ*(s12-s15)+s12*s15-s12*s23+s23*s34+MESQ*(-2*s15+s
     -   23-s45)+s15*s45-s34*s45)*CONJG(F(23)))/2q0-(MESQ-s12)*(MU4SQ
     -   +MESQ*(3*MU2SQ+s15-s23-s34)+(-s15+s23+s34)*s45-MU2SQ*(s12+s2
     -   3+s45))*CONJG(F(24))+2*MESQ*(MU4SQ+MESQ*(3*MU2SQ+s15-s23-s34
     -   )+(-s15+s23+s34)*s45-MU2SQ*(s12+s23+s45))*CONJG(F(25))+(-MU2
     -   SQ-s15+s23+s34)*(-MU4SQ+MESQ*(-3*MU2SQ-s15+s23+s34)+(s15-s23
     -   -s34)*s45+MU2SQ*(s12+s23+s45))*CONJG(F(26))+(s12+s15-s34)*(-
     -   MU4SQ+MESQ*(-3*MU2SQ-s15+s23+s34)+(s15-s23-s34)*s45+MU2SQ*(s
     -   12+s23+s45))*CONJG(F(27))+ME*(3*ME4SQ-s12*s15-MU2SQ*(3*s12+s
     -   15)+s12*s23+2*s12*s34+s23*s34+MESQ*(4*MU2SQ-2*s12-s23-2*s34-
     -   3*s45)+(2*s12+s15-s34)*s45)*CONJG(F(28))+2*ME*(2*MESQ*MU2SQ-
     -   MU4SQ-MU2SQ*(s12+3*s15-2*s23-3*s34)-(-s15+s23+s34)**2)*CONJG
     -   (F(29))+2*ME3SQ*(MESQ*(-3*MU2SQ-s15+s23+s34)+(s15-s23-s34)*(
     -   s15-s23+s45)+MU2SQ*(s12+2*s15-s23+s45))*CONJG(F(30))+ME*(MES
     -   Q-s12)*(MESQ*(3*MU2SQ+s15-s23-s34)-(s15-s23-s34)*(s15-s23+s4
     -   5)-MU2SQ*(s12+2*s15-s23+s45))*CONJG(F(31))+ME*(-MU2SQ-s15+s2
     -   3+s34)*(MESQ*(3*MU2SQ+s15-s23-s34)-(s15-s23-s34)*(s15-s23+s4
     -   5)-MU2SQ*(s12+2*s15-s23+s45))*CONJG(F(32))+ME*(s12+s15-s34)*
     -   (MESQ*(3*MU2SQ+s15-s23-s34)-(s15-s23-s34)*(s15-s23+s45)-MU2S
     -   Q*(s12+2*s15-s23+s45))*CONJG(F(33))+(MU4SQ*(s12+s15)+ME4SQ*(
     -   -MU2SQ-s15+s23+s34)-MESQ*(2*MU4SQ-(s15-s23-s34)*(2*s12+s23+s
     -   45)-MU2SQ*(6*s12-2*s15+3*s23+s45))-(s15-s23-s34)*(s23*s34+(s
     -   15-s34)*s45+s12*(s15-s23+2*s45))-MU2SQ*(2*s12**2-s15**2+s34*
     -   (s23-s45)+s15*(s23+s34+s45)+s12*(2*s15-s34+2*s45)))*CONJG(F(
     -   34))
       ssav(34) = ME*(-MU2SQ-s15+s23+s34)*(MESQ+2*MU2SQ+s15-s23-s34-s4
     -   5)*CONJG(F(15))-ME*(MESQ-s15+s23-s45)*(MESQ+2*MU2SQ+s15-s23-
     -   s34-s45)*CONJG(F(16))+2*ME*MU2SQ*(-MESQ-2*MU2SQ-s15+s23+s34+
     -   s45)*CONJG(F(17))+ME*(MESQ+MU2SQ-s45)*(MESQ+2*MU2SQ+s15-s23-
     -   s34-s45)*CONJG(F(18))+2*(MU4SQ+MESQ*(3*MU2SQ+s15-s23-s34)+(-
     -   s15+s23+s34)*s45-MU2SQ*(s12+s23+s45))*CONJG(F(19))+((MESQ-s1
     -   5+s23-s45)*(ME4SQ+MU2SQ*(s12-s15)+s12*s15-s12*s23+s23*s34+ME
     -   SQ*(-2*s15+s23-s45)+s15*s45-s34*s45)*CONJG(F(20)))/2q0+((MU2
     -   SQ+s15-s23-s34)*(ME4SQ+MU2SQ*(s12-s15)+s12*s15-s12*s23+s23*s
     -   34+MESQ*(-2*s15+s23-s45)+s15*s45-s34*s45)*CONJG(F(21)))/2q0+
     -   MU2SQ*(ME4SQ+MU2SQ*(s12-s15)+s12*s15-s12*s23+s23*s34+MESQ*(-
     -   2*s15+s23-s45)+s15*s45-s34*s45)*CONJG(F(22))-((MESQ+MU2SQ-s4
     -   5)*(ME4SQ+MU2SQ*(s12-s15)+s12*s15-s12*s23+s23*s34+MESQ*(-2*s
     -   15+s23-s45)+s15*s45-s34*s45)*CONJG(F(23)))/2q0+(-MESQ+s15-s2
     -   3+s45)*(-MU4SQ+MESQ*(-3*MU2SQ-s15+s23+s34)+(s15-s23-s34)*s45
     -   +MU2SQ*(s12+s23+s45))*CONJG(F(24))+(-MU2SQ-s15+s23+s34)*(-MU
     -   4SQ+MESQ*(-3*MU2SQ-s15+s23+s34)+(s15-s23-s34)*s45+MU2SQ*(s12
     -   +s23+s45))*CONJG(F(25))+2*MU2SQ*(MU4SQ+MESQ*(3*MU2SQ+s15-s23
     -   -s34)+(-s15+s23+s34)*s45-MU2SQ*(s12+s23+s45))*CONJG(F(26))-(
     -   MESQ+MU2SQ-s45)*(MU4SQ+MESQ*(3*MU2SQ+s15-s23-s34)+(-s15+s23+
     -   s34)*s45-MU2SQ*(s12+s23+s45))*CONJG(F(27))-2*ME*(ME4SQ+MU2SQ
     -   *(s12+s23-s45)+s45*(s15-s23+s45)-MESQ*(MU2SQ+s15-s23+2*s45))
     -   *CONJG(F(28))+2*ME*MU2SQ*(-MESQ-2*MU2SQ-s15+s23+s34+s45)*CON
     -   JG(F(29))+ME*(-MU2SQ-s15+s23+s34)*(MESQ*(3*MU2SQ+s15-s23-s34
     -   )-(s15-s23-s34)*(s15-s23+s45)-MU2SQ*(s12+2*s15-s23+s45))*CON
     -   JG(F(30))-ME*(MESQ-s15+s23-s45)*(MESQ*(3*MU2SQ+s15-s23-s34)-
     -   (s15-s23-s34)*(s15-s23+s45)-MU2SQ*(s12+2*s15-s23+s45))*CONJG
     -   (F(31))+2*ME*MU2SQ*(MESQ*(-3*MU2SQ-s15+s23+s34)+(s15-s23-s34
     -   )*(s15-s23+s45)+MU2SQ*(s12+2*s15-s23+s45))*CONJG(F(32))+ME*(
     -   MESQ+MU2SQ-s45)*(MESQ*(3*MU2SQ+s15-s23-s34)-(s15-s23-s34)*(s
     -   15-s23+s45)-MU2SQ*(s12+2*s15-s23+s45))*CONJG(F(33))+(ME4SQ*(
     -   5*MU2SQ+2*s15-2*s23-2*s34)+2*(s15-s23-s34)*s45*(s15-s23+s45)
     -   -MU4SQ*(s12+s15-2*s23+2*s45)+MU2SQ*(-(s23*(-2*s15+2*s23+s34)
     -   )+(s15+s34)*s45+2*s45**2+s12*(s15-s23+2*s45))+MESQ*(2*MU4SQ-
     -   2*(s15-s23-s34)*(s15-s23+2*s45)-MU2SQ*(2*s12+4*s15-3*s23+7*s
     -   45)))*CONJG(F(34))
       ssav(35) = ME*(s12+s15-s34)*(MESQ+2*MU2SQ+s15-s23-s34-s45)*CONJ
     -   G(F(15))+ME*(MESQ-s15)*(MESQ+2*MU2SQ+s15-s23-s34-s45)*CONJG(
     -   F(16))+ME*(MESQ+MU2SQ-s45)*(MESQ+2*MU2SQ+s15-s23-s34-s45)*CO
     -   NJG(F(17))+2*ME3SQ*(-MESQ-2*MU2SQ-s15+s23+s34+s45)*CONJG(F(1
     -   8))+2*MESQ*(-MESQ-2*MU2SQ-s15+s23+s34+s45)*CONJG(F(19))-((ME
     -   SQ-s15)*(ME4SQ+MU2SQ*(s12-s15)+s12*s15-s12*s23+s23*s34+MESQ*
     -   (-2*s15+s23-s45)+s15*s45-s34*s45)*CONJG(F(20)))/2q0-((s12+s1
     -   5-s34)*(ME4SQ+MU2SQ*(s12-s15)+s12*s15-s12*s23+s23*s34+MESQ*(
     -   -2*s15+s23-s45)+s15*s45-s34*s45)*CONJG(F(21)))/2q0-((MESQ+MU
     -   2SQ-s45)*(ME4SQ+MU2SQ*(s12-s15)+s12*s15-s12*s23+s23*s34+MESQ
     -   *(-2*s15+s23-s45)+s15*s45-s34*s45)*CONJG(F(22)))/2q0+MESQ*(M
     -   E4SQ+MU2SQ*(s12-s15)+s12*s15-s12*s23+s23*s34+MESQ*(-2*s15+s2
     -   3-s45)+s15*s45-s34*s45)*CONJG(F(23))-(MESQ-s15)*(MU4SQ+MESQ*
     -   (3*MU2SQ+s15-s23-s34)+(-s15+s23+s34)*s45-MU2SQ*(s12+s23+s45)
     -   )*CONJG(F(24))+(s12+s15-s34)*(-MU4SQ+MESQ*(-3*MU2SQ-s15+s23+
     -   s34)+(s15-s23-s34)*s45+MU2SQ*(s12+s23+s45))*CONJG(F(25))-(ME
     -   SQ+MU2SQ-s45)*(MU4SQ+MESQ*(3*MU2SQ+s15-s23-s34)+(-s15+s23+s3
     -   4)*s45-MU2SQ*(s12+s23+s45))*CONJG(F(26))+2*MESQ*(MU4SQ+MESQ*
     -   (3*MU2SQ+s15-s23-s34)+(-s15+s23+s34)*s45-MU2SQ*(s12+s23+s45)
     -   )*CONJG(F(27))+ME*(ME4SQ+MU2SQ*(s12-s15)+s12*s15-s12*s23+s23
     -   *s34+MESQ*(-2*s15+s23-s45)+s15*s45-s34*s45)*CONJG(F(28))+2*M
     -   E*(MU4SQ+MESQ*(3*MU2SQ+s15-s23-s34)+(-s15+s23+s34)*s45-MU2SQ
     -   *(s12+s23+s45))*CONJG(F(29))+ME*(s12+s15-s34)*(MESQ*(3*MU2SQ
     -   +s15-s23-s34)-(s15-s23-s34)*(s15-s23+s45)-MU2SQ*(s12+2*s15-s
     -   23+s45))*CONJG(F(30))+ME*(MESQ-s15)*(MESQ*(3*MU2SQ+s15-s23-s
     -   34)-(s15-s23-s34)*(s15-s23+s45)-MU2SQ*(s12+2*s15-s23+s45))*C
     -   ONJG(F(31))+ME*(MESQ+MU2SQ-s45)*(MESQ*(3*MU2SQ+s15-s23-s34)-
     -   (s15-s23-s34)*(s15-s23+s45)-MU2SQ*(s12+2*s15-s23+s45))*CONJG
     -   (F(32))+2*ME3SQ*(MESQ*(-3*MU2SQ-s15+s23+s34)+(s15-s23-s34)*(
     -   s15-s23+s45)+MU2SQ*(s12+2*s15-s23+s45))*CONJG(F(33))-2*MESQ*
     -   (MESQ*(3*MU2SQ+s15-s23-s34)-(s15-s23-s34)*(s15-s23+s45)-MU2S
     -   Q*(s12+2*s15-s23+s45))*CONJG(F(34))
       ssav(36) = 2*ME*(2*MESQ+s12+s15-s34)*CONJG(F(35))+8*Invs15M*(4
     -   *MESQ-s12-s15+s34)*CONJG(F(36))*CONJG(F(37))+4*ME*(3*MESQ-2*
     -   s12-s15)*CONJG(F(38))
       ssav(37) = ME*(3*ME4SQ-s12*s15-2*s15**2-MU2SQ*(s12+s15)+s12*s23
     -   +2*s15*s23+2*s15*s34-s23*s34+MESQ*(2*MU2SQ+s23-2*s34-3*s45)-
     -   s15*s45+s34*s45)*CONJG(F(35))+4*Invs15M*(3*ME4SQ+s12*s15+2*
     -   s15**2+MU2SQ*(s12+s15)-s12*s23-2*s15*s23-2*s15*s34+s23*s34+M
     -   ESQ*(-2*MU2SQ-6*s15+5*s23+2*s34-3*s45)+s15*s45-s34*s45)*CONJ
     -   G(F(36))*CONJG(F(37))+4*ME*(MESQ-s15)*(MESQ-s15+s23-s45)*CON
     -   JG(F(38))
       ssav(38) = ME*(s12-s15)*(MESQ+MU2SQ-s45)*CONJG(F(35))+2*Invs15M
     -   *(ME4SQ+MU2SQ*(s12-s15)+s12*s15-s12*s23+s23*s34+MESQ*(-2*s1
     -   5+s23-s45)+s15*s45-s34*s45)*CONJG(F(36))*CONJG(F(37))+2*ME*(
     -   MESQ-s12)*(MESQ-s15+s23-s45)*CONJG(F(38))
       ssav(39) = ME*(MESQ-s15)*(MESQ+2*MU2SQ+s15-s23-s34-s45)*CONJG(F
     -   (35))+2*Invs15M*(ME4SQ+MU2SQ*(s12-s15)+s12*s15-s12*s23+s23*
     -   s34+MESQ*(-2*s15+s23-s45)+s15*s45-s34*s45)*CONJG(F(36))*CONJ
     -   G(F(37))
       ssav(40) = ME*((s12+s15-s34)*(MESQ+2*MU2SQ+s15-s23-s34-s45)*CON
     -   JG(F(35))+4*Invs15M*ME*(MESQ+2*MU2SQ+s15-s23-s34-s45)*CONJG
     -   (F(36))*CONJG(F(37))+(3*ME4SQ-s12*s15-MU2SQ*(3*s12+s15)+s12*
     -   s23+2*s12*s34+s23*s34+MESQ*(4*MU2SQ-2*s12-s23-2*s34-3*s45)+(
     -   2*s12+s15-s34)*s45)*CONJG(F(38)))
       ssav(41) = ME*(MESQ+MU2SQ-s45)*(MESQ+2*MU2SQ+s15-s23-s34-s45)*C
     -   ONJG(F(35))+4*Invs15M*(MU4SQ+MESQ*(3*MU2SQ+s15-s23-s34)+(-s
     -   15+s23+s34)*s45-MU2SQ*(s12+s23+s45))*CONJG(F(36))*CONJG(F(37
     -   ))-2*ME*(ME4SQ+MU2SQ*(s12+s23-s45)+s45*(s15-s23+s45)-MESQ*(M
     -   U2SQ+s15-s23+2*s45))*CONJG(F(38))
       ssav(42) = ME*(2*MESQ*(-MESQ-2*MU2SQ-s15+s23+s34+s45)*CONJG(F(3
     -   5))+4*Invs15M*ME*(-MESQ-2*MU2SQ-s15+s23+s34+s45)*CONJG(F(36
     -   ))*CONJG(F(37))+(ME4SQ+MU2SQ*(s12-s15)+s12*s15-s12*s23+s23*s
     -   34+MESQ*(-2*s15+s23-s45)+s15*s45-s34*s45)*CONJG(F(38)))
       resultn(1) = hel(1)*ssav(22)+hel(2)*ssav(23)+hel(3)*ssav(24)+he
     -   l(4)*ssav(25)+hel(5)*ssav(26)+hel(6)*ssav(27)+hel(7)*ssav(28
     -   )
       resultn(2) = hel(1)*ssav(15)+hel(2)*ssav(16)+hel(3)*ssav(17)+he
     -   l(4)*ssav(18)+hel(5)*ssav(19)+hel(6)*ssav(20)+hel(7)*ssav(21
     -   )
       resultn(3) = hel(1)*ssav(29)+hel(2)*ssav(30)+hel(3)*ssav(31)+he
     -   l(4)*ssav(32)+hel(5)*ssav(33)+hel(6)*ssav(34)+hel(7)*ssav(35
     -   )
       resultn(4) = hel(1)*ssav(36)+hel(2)*ssav(37)+hel(3)*ssav(38)+he
     -   l(4)*ssav(39)+hel(5)*ssav(40)+hel(6)*ssav(41)+hel(7)*ssav(42
     -   )
       resultn(5) = hel(1)*ssav(1)+hel(2)*ssav(2)+hel(3)*ssav(3)+hel(4
     -   )*ssav(4)+hel(5)*ssav(5)+hel(6)*ssav(6)+hel(7)*ssav(7)
       resultn(6) = hel(1)*ssav(8)+hel(2)*ssav(9)+hel(3)*ssav(10)+hel(
     -   4)*ssav(11)+hel(5)*ssav(12)+hel(6)*ssav(13)+hel(7)*ssav(14)
       result = hel(1)*ssav(1)+hel(2)*ssav(2)+hel(3)*ssav(3)+hel(4)*ss
     -   av(4)+hel(5)*ssav(5)+hel(6)*ssav(6)+hel(7)*ssav(7)+hel(1)*ss
     -   av(8)+hel(2)*ssav(9)+hel(3)*ssav(10)+hel(4)*ssav(11)+hel(5)*
     -   ssav(12)+hel(6)*ssav(13)+hel(7)*ssav(14)+hel(1)*ssav(15)+hel
     -   (2)*ssav(16)+hel(3)*ssav(17)+hel(4)*ssav(18)+hel(5)*ssav(19)
     -   +hel(6)*ssav(20)+hel(7)*ssav(21)+hel(1)*ssav(22)+hel(2)*ssav
     -   (23)+hel(3)*ssav(24)+hel(4)*ssav(25)+hel(5)*ssav(26)+hel(6)*
     -   ssav(27)+hel(7)*ssav(28)+hel(1)*ssav(29)+hel(2)*ssav(30)+hel
     -   (3)*ssav(31)+hel(4)*ssav(32)+hel(5)*ssav(33)+hel(6)*ssav(34)
     -   +hel(7)*ssav(35)+hel(1)*ssav(36)+hel(2)*ssav(37)+hel(3)*ssav
     -   (38)+hel(4)*ssav(39)+hel(5)*ssav(40)+hel(6)*ssav(41)+hel(7)*
     -   ssav(42)
c The Finite virtual piece should be multiplied for (-1)  since 
c I have multiplied by (-I) to get the F's and k's without (I) factor
c . The factorization from the B_ij is Fact=(I/(4\[Pi])^2 (4 \[Pi])^Eps Gamma[1+Eps] (musq)^(-Eps))
c 
c So, I*I=(-1)!!!
       result =result
       resultb = hel(1)*(4*CONJG(hel(1))*ssa(1)+2*CONJG(hel(2))*ssa(2)
     -   +CONJG(hel(3))*ssa(3)+CONJG(hel(4))*ssa(3)+2*ME**2*CONJG(hel
     -   (5))*ssa(4)+2*CONJG(hel(6))*ssa(5)+2*ME**2*CONJG(hel(7))*ssa
     -   (6))+(hel(3)*(2*CONJG(hel(1))*ssa(3)+2*MU**2*CONJG(hel(6))*s
     -   sa(3)+2*CONJG(hel(2))*ssa(7)-4*MU**2*CONJG(hel(3))*ssa(8)*ss
     -   a(9)+CONJG(hel(4))*ssa(3)*ssa(10)+CONJG(hel(5))*ssa(3)*ssa(1
     -   1)-CONJG(hel(7))*ssa(3)*ssa(12)))/2.+hel(2)*(2*CONJG(hel(1))
     -   *ssa(2)+CONJG(hel(3))*ssa(7)-4*CONJG(hel(2))*ssa(9)*ssa(13)+
     -   CONJG(hel(5))*ssa(15)+CONJG(hel(6))*ssa(16)-2*ME**2*CONJG(he
     -   l(7))*ssa(17))+hel(7)*(2*ME**2*CONJG(hel(7))*ssa(5)+2*ME**2*
     -   CONJG(hel(1))*ssa(6)-CONJG(hel(4))*ssa(5)*ssa(9)-(CONJG(hel(
     -   3))*ssa(3)*ssa(12))/2.-CONJG(hel(6))*ssa(5)*ssa(12)-2*ME**2*
     -   CONJG(hel(2))*ssa(17)+CONJG(hel(5))*ssa(18)*ssa(19))+hel(5)*
     -   (2*ME**2*CONJG(hel(1))*ssa(4)+2*ME**2*CONJG(hel(5))*ssa(5)-C
     -   ONJG(hel(4))*ssa(5)*ssa(8)+(CONJG(hel(3))*ssa(3)*ssa(11))/2.
     -   +CONJG(hel(2))*ssa(15)+CONJG(hel(6))*ssa(14)*ssa(19)+CONJG(h
     -   el(7))*ssa(18)*ssa(19))+hel(6)*(MU**2*CONJG(hel(3))*ssa(3)+2
     -   *CONJG(hel(1))*ssa(5)+2*MU**2*CONJG(hel(6))*ssa(5)-CONJG(hel
     -   (7))*ssa(5)*ssa(12)+CONJG(hel(2))*ssa(16)+CONJG(hel(5))*ssa(
     -   14)*ssa(19)+CONJG(hel(4))*ssa(19)*ssa(20))+hel(4)*(CONJG(hel
     -   (1))*ssa(3)-CONJG(hel(5))*ssa(5)*ssa(8)-CONJG(hel(7))*ssa(5)
     -   *ssa(9)+(CONJG(hel(3))*ssa(3)*ssa(10))/2.+CONJG(hel(6))*ssa(
     -   19)*ssa(20))
c************************************************************************************
c************************************************************************************
c************************************************************************************
c************************************************************************************
c************************************************************************************
c************************************************************************************
c************************************************************************************
       Return
       End
