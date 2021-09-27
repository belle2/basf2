       subroutine Pentabox_PIFSRHelSum(ME,MU,p1,p2,p3,p4,p5,psi_p5_ME,
     -   barpsi_p1_ME,mup3,musq,comp,result,resultn,resultb)
c ************************************************************************************
c Author: Francisco Campanario
c E-mail: francam@particle.uni-karlsruhe.de
c Date: 17/2/2010
c Modified:10/10/2017
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
       Complex*32 result,resultn(7),resultb
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
       Complex*32 SMB(8), SMB1(16),F(35),Fa(10),K(0)
       Real*16 FR(35),FI(35),KR(0),KI(0)
       Complex*32 barpsi_p1_ME(4),psi_p5_ME(4),mup2(0:3),mup3(0:3) 
       Complex*32 barpsi_p1_ME_M(2),psi_p5_ME_M(2)
       Complex*32 barpsi_p1_ME_P(2),psi_p5_ME_P(2)
       Complex*32 SC0_QUAD,SC1c_QUAD,SC1r_QUAD,SC2cc_QUAD,SC2rr_QUAD
     -  ,SC2rc_QUAD,SC3rcc_QUAD,SC3rrc_QUAD
       Real*16 ME,MU,musq, P(24) 
       Real*16 MESQ,ME3SQ,ME4SQ,ME6SQ,MU2SQ,MU3SQ,MU4SQ,MU6SQ,INVs12M,
     -   INVs15M,Invs23mu,Invs24,Invs34mu,Invs35mmesq,Invs13mmesq
       real*8 MESQd,MU2SQd,musqd
       Complex*32 ssav(25),ssa(9),hel(4)
       EXTERNAL   SC0_QUAD,SC1c_QUAD,SC1r_QUAD,SC2cc_QUAD,SC2rr_QUAD
     -            ,SC2rc_QUAD,SC3rcc_QUAD,SC3rrc_QUAD
       Integer comp,ii,ll 
       common/Pentabox_PIFSRHelSumFfunctionsmm/F,P
       SAVE/Pentabox_PIFSRHelSumFfunctionsmm/
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
       Invs23mu=1q0/(-MU2SQ+s23)
       Invs24=1q0/(2*MU2SQ+s15-s23-s34)
       Invs34mu=1q0/(-MU2SQ+s34)
       Invs35mmesq=1q0/(MU2SQ+s12-s34-s45)
       Invs13mmesq=1q0/(MU2SQ-s12-s23+s45)
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
c       A02=A0finG1(musq) !A0finG(0d0,musq)  
       A02=qlI1(0.d0,musqd,0)
c       A03=A0finG2(MU,musq) !A0finG(MU,musq) 
       A03=qlI1(MU2SQd,musqd,0)
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
c       B012=B0finG3(ME,musq)!B0finG(ME,0d0,p1sq,musq)   
       B012=qlI2(p1sqd,MESQd,0.d0,musqd,0)
c       B013=B0finG5(ME,MU,s12,musq)!B0finG(ME,MU,s12,musq) 
       B013= qlI2(s12d,MESQd,MU2SQd,musqd,0) 
c       B014=B0finG5(ME,MU,s45,musq)!B0finG(ME,MU,s45,musq)   
       B014=qlI2(s45d,MESQd,MU2SQd,musqd,0)
c       B015=B0finG3(ME,musq)!B0finG(ME,0d0,p5sq,musq) 
       B015=qlI2(p5sqd,MESQd,0.d0,musqd,0)
c       B023=B0finG3(MU,musq)!B0finG(0d0,MU,p2sq,musq)  
       B023=qlI2(p2sqd,0.d0,MU2SQd,musqd,0)
c       B024=B0finG2(MU,s23,musq)!B0finG(0d0,MU,s23,musq)  
       B024= qlI2(s23d,0.d0,MU2SQd,musqd,0)
c       B025=B0finG1(s15,musq)!B0finG(0d0,0d0,s15,musq)   
       B025=qlI2(s15d,0.d0,0.d0,musqd,0)
c       B034=B0finG6(MU,musq)!B0finG(MU,MU,p3sq,musq)  
       B034=qlI2(p3sqd,MU2SQd,MU2SQd,musqd,0)
c       B035=B0finG2(MU,s34,musq)!B0finG(MU,0d0,s34,musq)  
       B035=qlI2(s34d,MU2SQd,0.d0,musqd,0)
c       B045=B0finG3(MU,musq)!B0finG(MU,0d0,p4sq,musq)  
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
       call tens_red2_new_Re_Com_QUAD(ME,0q0,p1sq,A02R,A02I,A01R,A01I,
     -   B012R,B012I,Bij12R,Bij12I) 
       call tens_red2_new_Re_Com_QUAD(ME,MU,s12,A03R,A03I,A01R,A01I,B0
     -   13R,B013I,Bij13R,Bij13I) 
       call tens_red2_new_Re_Com_QUAD(ME,MU,s45,A04R,A04I,A01R,A01I,B0
     -   14R,B014I,Bij14R,Bij14I) 
       call tens_red2_new_Re_Com_QUAD(ME,0q0,p5sq,A05R,A05I,A01R,A01I,
     -   B015R,B015I,Bij15R,Bij15I) 
       call tens_red2_new_Re_Com_QUAD(0q0,MU,p2sq,A03R,A03I,A02R,A02I,
     -   B023R,B023I,Bij23R,Bij23I) 
       call tens_red2_new_Re_Com_QUAD(0q0,MU,s23,A04R,A04I,A02R,A02I,B
     -   024R,B024I,Bij24R,Bij24I) 
       call tens_red2_new_Re_Com_QUAD(0q0,0q0,s15,A05R,A05I,A02R,A02I,
     -   B025R,B025I,Bij25R,Bij25I) 
       call tens_red2_new_Re_Com_QUAD(MU,MU,p3sq,A04R,A04I,A03R,A03I,B
     -   034R,B034I,Bij34R,Bij34I) 
       call tens_red2_new_Re_Com_QUAD(MU,0q0,s34,A05R,A05I,A03R,A03I,B
     -   035R,B035I,Bij35R,Bij35I) 
       call tens_red2_new_Re_Com_QUAD(MU,0q0,p4sq,A05R,A05I,A04R,A04I,
     -   B045R,B045I,Bij45R,Bij45I) 
c************************************************************************************
c************************************************************************************
c       C0123=C0finG1(ME,MU,s12,musq)!C0finG(ME,0d0,MU,p1sq,p2sq,s12,musq) 
       C0123=  qlI3(p1sqd,p2sqd,s12d,MESQd,0.d0,MU2SQd,musqd,0)
       C0123=C0123-qlI3(p1sqd,p2sqd,s12d,MESQd,0.d0,MU2SQd,musqd,-1)*
     1 Log(musq/s15)
c       print*,'C0123=',C0123
c       C0124=C0finG2(ME,0.d0,MU,p1sq,s23,s45,musq)!C0finG(ME,0d0,MU,p1sq,s23,s45,musq)  
       C0124= qlI3(p1sqd,s23d,s45d,MESQd,0.d0,MU2SQd,musqd,0)
c       print*,'C0124=',C0124
c       C0125=C0finG2(ME,0.d0,0.d0,p1sq,s15,p5sq,musq)!C0finG(ME,0d0,0d0,p1sq,s15,p5sq,musq)   
       C0125=qlI3(p1sqd,s15d,p5sqd,MESQd,0.d0,0.d0,musqd,0)
c       print*,'C0125=',C0125
c       C0134=C0finG2(MU,ME,MU,s45,s12,p3sq,musq)!C0finG(ME,MU,MU,s12,p3sq,s45,musq)   
       C0134=qlI3(s12d,p3sqd,s45d,MESQd,MU2SQd,MU2SQd,musqd,0)
c       print*,'C0134=',C0134
c       C0135=C0finG2(ME,MU,0.d0,s12,s34,p5sq,musq)!C0finG(ME,MU,0d0,s12,s34,p5sq,musq)   
       C0135=qlI3(s12d,s34d,p5sqd,MESQd,MU2SQd,0.d0,musqd,0)
c       print*,'C0135=',C0135
c       C0145=C0finG1(ME,MU,s45,musq)!C0finG(ME,MU,0d0,s45,p4sq,p5sq,musq)   
       C0145=qlI3(s45d,p4sqd,p5sqd,MESQd,MU2SQd,0.d0,musqd,0)
       C0145=C0145-qlI3(s45d,p4sqd,p5sqd,MESQd,MU2SQd,0.d0,musqd,-1)*
     2 Log(musq/s15)
c       print*,'C0145=',C0145
c       C0234=C0finG2(MU,0.d0,MU,s23,p2sq,p3sq,musq)!C0finG(0d0,MU,MU,p2sq,p3sq,s23,musq)   
       C0234=qlI3(p2sqd,p3sqd,s23d,0.d0,MU2SQd,MU2SQd,musqd,0)
c       print*,'C0234=',C0234
c       C0235=C0finG2(0.d0,MU,0.d0,p2sq,s34,s15,musq)!C0finG(0d0,MU,0d0,p2sq,s34,s15,musq) 
       C0235=  qlI3(p2sqd,s34d,s15d,0.d0,MU2SQd,0.d0,musqd,0)
c       print*,'C0235=',C0235
c       C0245=C0finG2(0.d0,MU,0.d0,s23,p4sq,s15,musq)!C0finG(0d0,MU,0d0,s23,p4sq,s15,musq)   
       C0245=qlI3(s23d,p4sqd,s15d,0.d0,MU2SQd,0.d0,musqd,0)
c       print*,'C0245=',C0245
c       C0345=C0finG2(MU,0.d0,MU,p4sq,s34,p3sq,musq)!C0finG(MU,MU,0d0,p3sq,p4sq,s34,musq)   
       C0345=qlI3(p3sqd,p4sqd,s34d,MU2SQd,MU2SQd,0.d0,musqd,0) 
c       print*,'C0345=',C0345 
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
       
       call tens_red3_new_Re_Com_G_QUAD(ME,0q0,MU,p1sq,p2sq,s12,B023R,
     -   B013R,B012R,B023I,B013I,B012I,Bij23R,Bij13R,Bij12R,Bij23I,Bi
     -   j13I,Bij12I,C0123q,C0123R,C0123I,Cij123R,Cij123I) 
       call tens_red3_new_Re_Com_G_QUAD(ME,0q0,MU,p1sq,s23,s45,B024R,B
     -   014R,B012R,B024I,B014I,B012I,Bij24R,Bij14R,Bij12R,Bij24I,Bij
     -   14I,Bij12I,C0124q,C0124R,C0124I,Cij124R,Cij124I) 
       call tens_red3_new_Re_Com_G_QUAD(ME,0q0,0q0,p1sq,s15,p5sq,B025R
     -   ,B015R,B012R,B025I,B015I,B012I,Bij25R,Bij15R,Bij12R,Bij25I,B
     -   ij15I,Bij12I,C0125q,C0125R,C0125I,Cij125R,Cij125I) 
       call tens_red3_new_Re_Com_G_QUAD(ME,MU,MU,s12,p3sq,s45,B034R,B0
     -   14R,B013R,B034I,B014I,B013I,Bij34R,Bij14R,Bij13R,Bij34I,Bij1
     -   4I,Bij13I,C0134q,C0134R,C0134I,Cij134R,Cij134I) 
       call tens_red3_new_Re_Com_G_QUAD(ME,MU,0q0,s12,s34,p5sq,B035R,B
     -   015R,B013R,B035I,B015I,B013I,Bij35R,Bij15R,Bij13R,Bij35I,Bij
     -   15I,Bij13I,C0135q,C0135R,C0135I,Cij135R,Cij135I) 
       call tens_red3_new_Re_Com_G_QUAD(ME,MU,0q0,s45,p4sq,p5sq,B045R,
     -   B015R,B014R,B045I,B015I,B014I,Bij45R,Bij15R,Bij14R,Bij45I,Bi
     -   j15I,Bij14I,C0145q,C0145R,C0145I,Cij145R,Cij145I) 
       call tens_red3_new_Re_Com_G_QUAD(0q0,MU,MU,p2sq,p3sq,s23,B034R,
     -   B024R,B023R,B034I,B024I,B023I,Bij34R,Bij24R,Bij23R,Bij34I,Bi
     -   j24I,Bij23I,C0234q,C0234R,C0234I,Cij234R,Cij234I) 
       call tens_red3_new_Re_Com_G_QUAD(0q0,MU,0q0,p2sq,s34,s15,B035R,
     -   B025R,B023R,B035I,B025I,B023I,Bij35R,Bij25R,Bij23R,Bij35I,Bi
     -   j25I,Bij23I,C0235q,C0235R,C0235I,Cij235R,Cij235I) 
       call tens_red3_new_Re_Com_G_QUAD(0q0,MU,0q0,s23,p4sq,s15,B045R,
     -   B025R,B024R,B045I,B025I,B024I,Bij45R,Bij25R,Bij24R,Bij45I,Bi
     -   j25I,Bij24I,C0245q,C0245R,C0245I,Cij245R,Cij245I) 
       call tens_red3_new_Re_Com_G_QUAD(MU,MU,0q0,p3sq,p4sq,s34,B045R,
     -   B035R,B034R,B045I,B035I,B034I,Bij45R,Bij35R,Bij34R,Bij45I,Bi
     -   j35I,Bij34I,C0345q,C0345R,C0345I,Cij345R,Cij345I) 
c************************************************************************************
c************************************************************************************
       
c       D02345=D0finG3(MU,s34,s23,s15,musq)!D0finG(0d0,MU,MU,0d0,s23,s34,p2sq,p3sq,p4sq,s15,musq)
       D02345=qlI4(p2sqd,p3sqd,p4sqd,s15d,s23d,s34d,0.d0,MU2SQd,
     1 MU2SQd,0.d0,musqd,0)
c       print*,'D02345=',D02345
c       D01345=D0finG1(ME,MU,s34,s45,s12,musq)!D0finG(ME,MU,MU,0d0,s45,s34,s12,p3sq,p4sq,p5sq,musq)
       D01345=qlI4(s12d,p3sqd,p4sqd,p5sqd,s45d,s34d,MESQd,MU2SQd,
     1 MU2SQd,0.d0,musqd,0)
       D01345=D01345-qlI4(s12d,p3sqd,p4sqd,p5sqd,s45d,s34d,MESQd,MU2SQd,
     1 MU2SQd,0.d0,musqd,-1)*Log(musq/s15)
c       print*,'D01345=',D01345
c       D01245=D0finG2(MU,ME,s15,s45,s23,musq)!D0finG(ME,0d0,MU,0d0,s45,s15,p1sq,s23,p4sq,p5sq,musq)
       D01245=qlI4(p1sqd,s23d,p4sqd,p5sqd,s45d,s15d,MESQd,
     1 0.d0,MU2SQd,0.d0,musqd,0)
       D01245=D01245-qlI4(p1sqd,s23d,p4sqd,p5sqd,s45d,s15d,MESQd,
     1 0.d0,MU2SQd,0.d0,musqd,-1)*Log(musq/s15)
c       print*,'D01245=',D01245
c       D01235=D0finG2(MU,ME,s15,s12,s34,musq)!D0finG(ME,0d0,MU,0d0,s12,s15,p1sq,p2sq,s34,p5sq,musq)
       D01235=qlI4(p1sqd,p2sqd,s34d,p5sqd,s12d,s15d,MESQd,0.d0,
     1 MU2SQd,0.d0,musqd,0)
       D01235=D01235-qlI4(p1sqd,p2sqd,s34d,p5sqd,s12d,s15d,MESQd,0.d0,
     1 MU2SQd,0.d0,musqd,-1)*Log(musq/s15)
c       print*,'D01235=',D01235
c       D01234=D0finG1(ME,MU,s23,s12,s45,musq)!D0finG(ME,0d0,MU,MU,s12,s23,p1sq,p2sq,p3sq,s45,musq)
       D01234=qlI4(p1sqd,p2sqd,p3sqd,s45d,s12d,s23d,MESQd,0.d0,
     1 MU2SQd,MU2SQd,musqd,0)
       D01234=D01234-qlI4(p1sqd,p2sqd,p3sqd,s45d,s12d,s23d,MESQd,0.d0,
     1 MU2SQd,MU2SQd,musqd,-1)*Log(musq/s15)
c       print*,'D01234=',D01234
            
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
       call tens_red4_new_Re_Com_G_QUAD(0q0,MU,MU,0q0,p2sq,p3sq,p4sq,p
     -   2p3,p2p4,p3p4,C0345R,C0245R,C0235R,C0234R,Cij345R,Cij245R,Ci
     -   j235R,Cij234R,C0345I,C0245I,C0235I,C0234I,Cij345I,Cij245I,Ci
     -   j235I,Cij234I,D02345q,D02345R,D02345I,Dij2345R,Dij2345I)
        call tens_red4_new_Re_Com_G_QUAD(ME,MU,MU,0q0,s12,p3sq,p4sq,p1p
     -   3+p2p3,p1p4+p2p4,p3p4,C0345R,C0145R,C0135R,C0134R,Cij345R,Ci
     -   j145R,Cij135R,Cij134R,C0345I,C0145I,C0135I,C0134I,Cij345I,Ci
     -   j145I,Cij135I,Cij134I,D01345q,D01345R,D01345I,Dij1345R,Dij134
     -   5I)
        call tens_red4_new_Re_Com_G_QUAD(ME,0q0,MU,0q0,p1sq,s23,p4sq,p1
     -   p2+p1p3,p1p4,p2p4+p3p4,C0245R,C0145R,C0125R,C0124R,Cij245R,C
     -   ij145R,Cij125R,Cij124R,C0245I,C0145I,C0125I,C0124I,Cij245I,C
     -   ij145I,Cij125I,Cij124I,D01245q,D01245R,D01245I,Dij1245R,Dij12
     -   45I)
        call tens_red4_new_Re_Com_G_QUAD(ME,0q0,MU,0q0,p1sq,p2sq,s34,p1
     -   p2,p1p3+p1p4,p2p3+p2p4,C0235R,C0135R,C0125R,C0123R,Cij235R,C
     -   ij135R,Cij125R,Cij123R,C0235I,C0135I,C0125I,C0123I,Cij235I,C
     -   ij135I,Cij125I,Cij123I,D01235q,D01235R,D01235I,Dij1235R,Dij12
     -   35I)
        call tens_red4_new_Re_Com_G_QUAD(ME,0q0,MU,MU,p1sq,p2sq,p3sq,p1
     -   p2,p1p3,p2p3,C0234R,C0134R,C0124R,C0123R,Cij234R,Cij134R,Cij
     -   124R,Cij123R,C0234I,C0134I,C0124I,C0123I,Cij234I,Cij134I,Cij
     -   124I,Cij123I,D01234q,D01234R,D01234I,Dij1234R,Dij1234I)
c************************************************************************************
c************************************************************************************
       EE0=E0finG_QUAD(ME,0q0,MU,MU,0q0,p1sq,p2sq,p3sq,p4sq,p5sq,s12,s
     -   23,s34,s45,s15,D02345q,D01345q,D01245q,D01235q,D01234q)
       EE0R=REALPART(EE0) 
       EE0I=IMAGPART(EE0) 
c************************************************************************************
c************************************************************************************
       call tens_red5_new_Re_Com_G_QUAD(ME,0q0,MU,MU,0q0,p1sq,p2sq,p3s
     -   q,p4sq,p1p2,p1p3,p1p4,p2p3,p2p4,p3p4,D02345R,D01345R,D01245R
     -   ,D01235R,D01234R,Dij2345R,Dij1345R,Dij1245R,Dij1235R,Dij1234
     -   R,D02345I,D01345I,D01245I,D01235I,D01234I,Dij2345I,Dij1345I,
     -   Dij1245I,Dij1235I,Dij1234I,EijR,EijI)

c       write(83,*),'-----------------------FSR-------------------------'

c       do ii=1,46
c         do ll=1,4
c             write(83,*)'ER=',ii,ll, EijR(ii,ll)
c             write(83,*)'EI',ii,ll, EijI(ii,ll)
             
c         enddo
c       enddo

c       do ii=1,13
c         do ll=1,2
c             write(83,*) 'DR=',ii,ll,Dij1234R(ii,ll)
c             write(83,*) 'DI=',ii,ll,Dij1234I(ii,ll)
c             write(83,*) 'DR=',ii,ll,Dij1235R(ii,ll)
c             write(83,*) 'DI=',ii,ll,Dij1235I(ii,ll)
c             write(83,*) 'DR=',ii,ll,Dij1245R(ii,ll)
c             write(83,*) 'DI=',ii,ll,Dij1245I(ii,ll)
c             write(83,*) 'DR=',ii,ll,Dij1345R(ii,ll)
c             write(83,*) 'DI=',ii,ll,Dij1345I(ii,ll)
c             write(83,*) 'DR=',ii,ll,Dij2345R(ii,ll)
c             write(83,*) 'DI=',ii,ll,Dij2345I(ii,ll)
c         enddo
c       enddo

c       do ii=1,4
c         do ll=1,2
c             write(83,*) 'CR=',ii,ll,Cij123R(ii,ll)
c             write(83,*) 'CI=',ii,ll,Cij123I(ii,ll)
c             write(83,*) 'CR=',ii,ll,Cij124R(ii,ll)
c             write(83,*) 'CI=',ii,ll,Cij124I(ii,ll)
c             write(83,*) 'CR=',ii,ll,Cij125R(ii,ll)
c             write(83,*) 'CI=',ii,ll,Cij125I(ii,ll)
c             write(83,*) 'CR=',ii,ll,Cij134R(ii,ll)
c             write(83,*) 'CI=',ii,ll,Cij134I(ii,ll)
c             write(83,*) 'CR=',ii,ll,Cij135R(ii,ll)
c             write(83,*) 'CI=',ii,ll,Cij135I(ii,ll)
c             write(83,*) 'CR=',ii,ll,Cij145R(ii,ll)
c             write(83,*) 'CI=',ii,ll,Cij145I(ii,ll)
c             write(83,*) 'CR=',ii,ll,Cij234R(ii,ll)
c             write(83,*) 'CI=',ii,ll,Cij234I(ii,ll)
c             write(83,*) 'CR=',ii,ll,Cij235R(ii,ll)
c             write(83,*) 'CI=',ii,ll,Cij235I(ii,ll)
c             write(83,*) 'CR=',ii,ll,Cij245R(ii,ll)
c             write(83,*) 'CI=',ii,ll,Cij245I(ii,ll)
c             write(83,*) 'CR=',ii,ll,Cij345R(ii,ll)
c             write(83,*) 'CI=',ii,ll,Cij345I(ii,ll)
c         enddo
c       enddo

c             write(83,*) 'BR12',Bij12R
c             write(83,*) 'BI12',Bij12I
c             write(83,*) 'BR13',Bij13R
c             write(83,*) 'BI13',Bij13I
c             write(83,*) 'BR14',Bij14R
c             write(83,*) 'BI14',Bij14I
c             write(83,*) 'BR15',Bij15R
c             write(83,*) 'BI15',Bij15I
c             write(83,*) 'BR23',Bij23R
c             write(83,*) 'BI23',Bij23I
c             write(83,*) 'BR24',Bij24R
c             write(83,*) 'BI24',Bij24I
c             write(83,*) 'BR25',Bij25R
c             write(83,*) 'BI25',Bij25I
c             write(83,*) 'BR34',Bij34R
c             write(83,*) 'BI34',Bij34I
c             write(83,*) 'BR35',Bij35R
c             write(83,*) 'BI35',Bij35I
c             write(83,*) 'BR45',Bij45R
c             write(83,*) 'BI45',Bij45I
c       write(83,*) '==================================='
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
       P(1) = MESQ-s12
       FR(1) = 4*ME*(D01235R+Dij1235R(1,1))*P(1)
       FI(1) = 4*ME*(D01235I+Dij1235I(1,1))*P(1)
       F(1)=COMPLEX(FR(1),FI(1))
       P(2) = ME3SQ-ME*s12
       FR(2) = -4*(D01235R+Dij1235R(1,1))*P(2)
       FI(2) = -4*(D01235I+Dij1235I(1,1))*P(2)
       F(2)=COMPLEX(FR(2),FI(2))
       P(3) = 3*MESQ+MU2SQ-s12
       P(4) = MU2SQ-s15
       FR(3) = Cij235R(1,1)-Cij235R(2,1)+8*MESQ*Dij1235R(1,1)+4*(C0125
     -   R-C0135R+C0235R-MU2SQ*Dij1235R(2,1)+D01235R*P(3)+Dij1235R(3,
     -   1)*P(4))
       FI(3) = Cij235I(1,1)-Cij235I(2,1)+8*MESQ*Dij1235I(1,1)+4*(C0125
     -   I-C0135I+C0235I-MU2SQ*Dij1235I(2,1)+D01235I*P(3)+Dij1235I(3,
     -   1)*P(4))
       F(3)=COMPLEX(FR(3),FI(3))
       P(5) = -3*MESQ-MU2SQ+s12
       P(6) = -MU2SQ+s15
       FR(4) = -Cij235R(1,1)+Cij235R(2,1)-8*MESQ*Dij1235R(1,1)-4*(C012
     -   5R-C0135R+C0235R-MU2SQ*Dij1235R(2,1)-D01235R*P(5)-Dij1235R(3
     -   ,1)*P(6))
       FI(4) = -Cij235I(1,1)+Cij235I(2,1)-8*MESQ*Dij1235I(1,1)-4*(C012
     -   5I-C0135I+C0235I-MU2SQ*Dij1235I(2,1)-D01235I*P(5)-Dij1235I(3
     -   ,1)*P(6))
       F(4)=COMPLEX(FR(4),FI(4))
       P(7) = MESQ-s45
       FR(5) = 4*ME*(D01245R+Dij1245R(1,1))*P(7)
       FI(5) = 4*ME*(D01245I+Dij1245I(1,1))*P(7)
       F(5)=COMPLEX(FR(5),FI(5))
       P(8) = ME3SQ-ME*s45
       FR(6) = -4*(D01245R+Dij1245R(1,1))*P(8)
       FI(6) = -4*(D01245I+Dij1245I(1,1))*P(8)
       F(6)=COMPLEX(FR(6),FI(6))
       P(9) = MESQ-s23+s45
       FR(7) = -8*D01245R*MESQ-Cij245R(1,1)+Cij245R(2,1)-4*(C0245R+s23
     -   *Dij1245R(2,1)-MU2SQ*Dij1245R(3,1)+Dij1245R(1,1)*P(9))
       FI(7) = -8*D01245I*MESQ-Cij245I(1,1)+Cij245I(2,1)-4*(C0245I+s23
     -   *Dij1245I(2,1)-MU2SQ*Dij1245I(3,1)+Dij1245I(1,1)*P(9))
       F(7)=COMPLEX(FR(7),FI(7))
       FR(8) = 8*D01245R*MESQ+Cij245R(1,1)-Cij245R(2,1)+4*(C0245R+s23*
     -   Dij1245R(2,1)-MU2SQ*Dij1245R(3,1)+Dij1245R(1,1)*P(9))
       FI(8) = 8*D01245I*MESQ+Cij245I(1,1)-Cij245I(2,1)+4*(C0245I+s23*
     -   Dij1245I(2,1)-MU2SQ*Dij1245I(3,1)+Dij1245I(1,1)*P(9))
       F(8)=COMPLEX(FR(8),FI(8))
       P(10) = MESQ+MU2SQ-s12
       P(11) = MESQ+MU2SQ-s12-s15+s34
       P(12) = -MESQ+s45
       P(13) = s15-s23-s34
       FR(9) = 4*ME*(EE0R+3*EijR(1,1)-2*(-EijR(1,2)+EijR(3,1)+EijR(6,2
     -   )))*P(13)
       FI(9) = 4*ME*(EE0I+3*EijI(1,1)-2*(-EijI(1,2)+EijI(3,1)+EijI(6,2
     -   )))*P(13)
       F(9)=COMPLEX(FR(9),FI(9))
       FR(10) = 4*ME*(EE0R+EijR(1,1)+2*(EijR(2,1)-EijR(3,1)+EijR(5,2)-
     -   EijR(6,2)))*P(13)
       FI(10) = 4*ME*(EE0I+EijI(1,1)+2*(EijI(2,1)-EijI(3,1)+EijI(5,2)-
     -   EijI(6,2)))*P(13)
       F(10)=COMPLEX(FR(10),FI(10))
       P(14) = -s15+s23+s34
       FR(11) = 4*ME*(EE0R+EijR(1,1)+2*(EijR(3,1)-EijR(4,1)+EijR(6,2)-
     -   EijR(7,2)))*P(14)
       FI(11) = 4*ME*(EE0I+EijI(1,1)+2*(EijI(3,1)-EijI(4,1)+EijI(6,2)-
     -   EijI(7,2)))*P(14)
       F(11)=COMPLEX(FR(11),FI(11))
       FR(12) = 4*ME*(EE0R+EijR(1,1)+2*(EijR(3,1)+EijR(6,2)))*P(14)
       FI(12) = 4*ME*(EE0I+EijI(1,1)+2*(EijI(3,1)+EijI(6,2)))*P(14)
       F(12)=COMPLEX(FR(12),FI(12))
       FR(13) = -2*Dij2345R(7,2)+8*EijR(11,2)*P(14)
       FI(13) = -2*Dij2345I(7,2)+8*EijI(11,2)*P(14)
       F(13)=COMPLEX(FR(13),FI(13))
       P(15) = -MESQ+MU2SQ+s45
       FR(14) = Dij2345R(1,1)+3*Dij2345R(2,1)+2*(D02345R-Dij2345R(2,2)
     -   +Dij2345R(4,2)-Dij2345R(5,2)+Dij2345R(6,2)-2*(Dij2345R(3,1)+
     -   MU2SQ*EijR(4,1)-4*EijR(11,2)+(EijR(3,1)+2*EijR(3,2))*P(7)-2*
     -   (MU2SQ*EijR(4,2)-EijR(10,2)*P(15))))
       FI(14) = Dij2345I(1,1)+3*Dij2345I(2,1)+2*(D02345I-Dij2345I(2,2)
     -   +Dij2345I(4,2)-Dij2345I(5,2)+Dij2345I(6,2)-2*(Dij2345I(3,1)+
     -   MU2SQ*EijI(4,1)-4*EijI(11,2)+(EijI(3,1)+2*EijI(3,2))*P(7)-2*
     -   (MU2SQ*EijI(4,2)-EijI(10,2)*P(15))))
       F(14)=COMPLEX(FR(14),FI(14))
       FR(15) = -5*(Dij2345R(1,1)-Dij2345R(2,1))-2*(D02345R+Dij2345R(1
     -   ,2)+Dij2345R(2,2)-2*(Dij2345R(4,2)+MU2SQ*(EijR(4,1)+2*(EijR(
     -   9,2)-EijR(10,2)))))+4*(EijR(3,1)-2*(EijR(3,2)-EijR(8,2)))*P(
     -   7)
       FI(15) = -5*(Dij2345I(1,1)-Dij2345I(2,1))-2*(D02345I+Dij2345I(1
     -   ,2)+Dij2345I(2,2)-2*(Dij2345I(4,2)+MU2SQ*(EijI(4,1)+2*(EijI(
     -   9,2)-EijI(10,2)))))+4*(EijI(3,1)-2*(EijI(3,2)-EijI(8,2)))*P(
     -   7)
       F(15)=COMPLEX(FR(15),FI(15))
       FR(16) = Dij2345R(1,1)+3*Dij2345R(2,1)+2*(D02345R-Dij2345R(2,2)
     -   +Dij2345R(4,2))+8*(MU2SQ*EijR(7,2)-MU2SQ*EijR(10,2))+4*(MU2S
     -   Q*EijR(4,1)+(EijR(3,1)-2*(EijR(3,2)-EijR(6,2)))*P(7))
       FI(16) = Dij2345I(1,1)+3*Dij2345I(2,1)+2*(D02345I-Dij2345I(2,2)
     -   +Dij2345I(4,2))+8*(MU2SQ*EijI(7,2)-MU2SQ*EijI(10,2))+4*(MU2S
     -   Q*EijI(4,1)+(EijI(3,1)-2*(EijI(3,2)-EijI(6,2)))*P(7))
       F(16)=COMPLEX(FR(16),FI(16))
       FR(17) = Dij2345R(1,1)+3*Dij2345R(2,1)+2*(D02345R-Dij2345R(2,2)
     -   +Dij2345R(4,2)-2*(MU2SQ*(EijR(4,1)+2*EijR(10,2))+(EijR(3,1)+
     -   2*EijR(3,2))*P(7)))
       FI(17) = Dij2345I(1,1)+3*Dij2345I(2,1)+2*(D02345I-Dij2345I(2,2)
     -   +Dij2345I(4,2)-2*(MU2SQ*(EijI(4,1)+2*EijI(10,2))+(EijI(3,1)+
     -   2*EijI(3,2))*P(7)))
       F(17)=COMPLEX(FR(17),FI(17))
       P(16) = -2*s23+6*P(10)
       P(17) = -s15+s23+s34-2*P(10)
       P(18) = MESQ+MU2SQ-s12-s23
       P(19) = MESQ+MU2SQ-s12+s15-2*s23-s34
       FR(18) = -5*(Dij2345R(2,1)-Dij2345R(3,1))-4*(D01245R-D01345R-MU
     -   2SQ*EijR(2,1)+EE0R*P(10))-2*(D02345R+Dij2345R(2,2)+Dij2345R(
     -   3,2)+EijR(3,1)*P(16)-2*(Dij2345R(6,2)-EijR(4,1)*P(17))+4*(Di
     -   j1345R(3,1)-MU2SQ*EijR(8,2)+MU2SQ*EijR(9,2)+EijR(3,2)*P(18))
     -   )-8*(Dij1245R(2,1)-Dij1245R(3,1)-Dij1345R(2,1)-EijR(4,2)*P(1
     -   4)-EijR(10,2)*P(19))
       FI(18) = -5*(Dij2345I(2,1)-Dij2345I(3,1))-4*(D01245I-D01345I-MU
     -   2SQ*EijI(2,1)+EE0I*P(10))-2*(D02345I+Dij2345I(2,2)+Dij2345I(
     -   3,2)+EijI(3,1)*P(16)-2*(Dij2345I(6,2)-EijI(4,1)*P(17))+4*(Di
     -   j1345I(3,1)-MU2SQ*EijI(8,2)+MU2SQ*EijI(9,2)+EijI(3,2)*P(18))
     -   )-8*(Dij1245I(2,1)-Dij1245I(3,1)-Dij1345I(2,1)-EijI(4,2)*P(1
     -   4)-EijI(10,2)*P(19))
       F(18)=COMPLEX(FR(18),FI(18))
       P(20) = MU2SQ+2*P(1)
       P(21) = MESQ+MU2SQ-s12+s23
       P(22) = -MESQ-MU2SQ+s12+s23
       P(23) = MESQ+2*MU2SQ-s12-s23
       FR(19) = -8*(Dij1345R(1,1)-Dij1345R(2,1))+4*(D01245R-D01345R+Di
     -   j2345R(1,1))-3*Dij2345R(2,1)-Dij2345R(3,1)+2*(D02345R-Dij234
     -   5R(2,2)+Dij2345R(4,2)-Dij2345R(5,2)+Dij2345R(6,2)+2*(-4*EijR
     -   (11,2)+EE0R*P(10)+(EijR(4,1)+2*(EijR(9,2)-EijR(10,2)))*P(14)
     -   +EijR(2,1)*P(20)-EijR(3,1)*P(21)-2*(MU2SQ*EijR(2,2)-EijR(3,2
     -   )*P(22)-EijR(8,2)*P(23))))
       FI(19) = -8*(Dij1345I(1,1)-Dij1345I(2,1))+4*(D01245I-D01345I+Di
     -   j2345I(1,1))-3*Dij2345I(2,1)-Dij2345I(3,1)+2*(D02345I-Dij234
     -   5I(2,2)+Dij2345I(4,2)-Dij2345I(5,2)+Dij2345I(6,2)+2*(-4*EijI
     -   (11,2)+EE0I*P(10)+(EijI(4,1)+2*(EijI(9,2)-EijI(10,2)))*P(14)
     -   +EijI(2,1)*P(20)-EijI(3,1)*P(21)-2*(MU2SQ*EijI(2,2)-EijI(3,2
     -   )*P(22)-EijI(8,2)*P(23))))
       F(19)=COMPLEX(FR(19),FI(19))
       FR(20) = 4*(D01245R-D01345R)+8*(Dij1245R(1,1)-Dij1245R(2,1)-Dij
     -   1345R(1,1)+Dij1345R(2,1))-5*Dij2345R(2,1)+Dij2345R(3,1)-2*(D
     -   02345R+Dij2345R(2,2)-Dij2345R(6,2)-2*(-(MU2SQ*(EijR(2,1)+2*(
     -   EijR(5,2)-EijR(8,2))))+(EE0R+2*EijR(1,1))*P(10)+(EijR(4,1)+2
     -   *(EijR(7,2)-EijR(10,2)))*P(14)-EijR(3,1)*P(21)+2*(EijR(6,2)*
     -   P(18)+EijR(3,2)*P(22))))
       FI(20) = 4*(D01245I-D01345I)+8*(Dij1245I(1,1)-Dij1245I(2,1)-Dij
     -   1345I(1,1)+Dij1345I(2,1))-5*Dij2345I(2,1)+Dij2345I(3,1)-2*(D
     -   02345I+Dij2345I(2,2)-Dij2345I(6,2)-2*(-(MU2SQ*(EijI(2,1)+2*(
     -   EijI(5,2)-EijI(8,2))))+(EE0I+2*EijI(1,1))*P(10)+(EijI(4,1)+2
     -   *(EijI(7,2)-EijI(10,2)))*P(14)-EijI(3,1)*P(21)+2*(EijI(6,2)*
     -   P(18)+EijI(3,2)*P(22))))
       F(20)=COMPLEX(FR(20),FI(20))
       P(24) = s23-3*P(10)
       FR(21) = -4*(D01245R-D01345R)-8*(Dij1245R(2,1)-Dij1345R(2,1))-5
     -   *Dij2345R(2,1)+Dij2345R(3,1)-2*(D02345R+Dij2345R(2,2)-Dij234
     -   5R(6,2)-2*(MU2SQ*EijR(2,1)-EE0R*P(10)-(EijR(4,1)+2*EijR(10,2
     -   ))*P(14)+2*(MU2SQ*EijR(8,2)+EijR(3,2)*P(22))+EijR(3,1)*P(24)
     -   ))
       FI(21) = -4*(D01245I-D01345I)-8*(Dij1245I(2,1)-Dij1345I(2,1))-5
     -   *Dij2345I(2,1)+Dij2345I(3,1)-2*(D02345I+Dij2345I(2,2)-Dij234
     -   5I(6,2)-2*(MU2SQ*EijI(2,1)-EE0I*P(10)-(EijI(4,1)+2*EijI(10,2
     -   ))*P(14)+2*(MU2SQ*EijI(8,2)+EijI(3,2)*P(22))+EijI(3,1)*P(24)
     -   ))
       F(21)=COMPLEX(FR(21),FI(21))
       FR(22) = -4*ME*(EE0R+3*EijR(1,1)-2*(-EijR(1,2)+EijR(3,1)+EijR(6
     -   ,2)))
       FI(22) = -4*ME*(EE0I+3*EijI(1,1)-2*(-EijI(1,2)+EijI(3,1)+EijI(6
     -   ,2)))
       F(22)=COMPLEX(FR(22),FI(22))
       FR(23) = -4*ME*(EE0R+EijR(1,1)+2*(EijR(2,1)-EijR(3,1)+EijR(5,2)
     -   -EijR(6,2)))
       FI(23) = -4*ME*(EE0I+EijI(1,1)+2*(EijI(2,1)-EijI(3,1)+EijI(5,2)
     -   -EijI(6,2)))
       F(23)=COMPLEX(FR(23),FI(23))
       FR(24) = 4*ME*(EE0R+EijR(1,1)+2*(EijR(3,1)-EijR(4,1)+EijR(6,2)-
     -   EijR(7,2)))
       FI(24) = 4*ME*(EE0I+EijI(1,1)+2*(EijI(3,1)-EijI(4,1)+EijI(6,2)-
     -   EijI(7,2)))
       F(24)=COMPLEX(FR(24),FI(24))
       FR(25) = 4*ME*(EE0R+EijR(1,1)+2*(EijR(3,1)+EijR(6,2)))
       FI(25) = 4*ME*(EE0I+EijI(1,1)+2*(EijI(3,1)+EijI(6,2)))
       F(25)=COMPLEX(FR(25),FI(25))
       FR(26) = EijR(11,2)
       FI(26) = EijI(11,2)
       F(26)=COMPLEX(FR(26),FI(26))
       FR(27) = -8*ME*(D01245R+Dij1245R(1,1))
       FI(27) = -8*ME*(D01245I+Dij1245I(1,1))
       F(27)=COMPLEX(FR(27),FI(27))
       FR(28) = 2*(C0245R-2*(MU2SQ*Dij1245R(3,1)-Dij1245R(2,1)*P(12)))
       FI(28) = 2*(C0245I-2*(MU2SQ*Dij1245I(3,1)-Dij1245I(2,1)*P(12)))
       F(28)=COMPLEX(FR(28),FI(28))
       FR(29) = -8*(Dij1245R(1,1)-Dij1245R(2,1))
       FI(29) = -8*(Dij1245I(1,1)-Dij1245I(2,1))
       F(29)=COMPLEX(FR(29),FI(29))
       FR(30) = 4*ME*(D01245R+Dij1245R(1,1))
       FI(30) = 4*ME*(D01245I+Dij1245I(1,1))
       F(30)=COMPLEX(FR(30),FI(30))
       FR(31) = 2*(C0235R+2*(C0125R-C0135R-MU2SQ*Dij1235R(2,1)+D01235R
     -   *P(10)+Dij1235R(3,1)*P(11)))
       FI(31) = 2*(C0235I+2*(C0125I-C0135I-MU2SQ*Dij1235I(2,1)+D01235I
     -   *P(10)+Dij1235I(3,1)*P(11)))
       F(31)=COMPLEX(FR(31),FI(31))
       FR(32) = Dij1235R(3,1)
       FI(32) = Dij1235I(3,1)
       F(32)=COMPLEX(FR(32),FI(32))
       FR(33) = 4*ME*(D01235R+Dij1235R(1,1))
       FI(33) = 4*ME*(D01235I+Dij1235I(1,1))
       F(33)=COMPLEX(FR(33),FI(33))
       FR(34) = 4*ME*(2*C0125R+Cij125R(1,1))
       FI(34) = 4*ME*(2*C0125I+Cij125I(1,1))
       F(34)=COMPLEX(FR(34),FI(34))
       FR(35) = -4*ME*(2*C0125R+Cij125R(1,1))
       FI(35) = -4*ME*(2*C0125I+Cij125I(1,1))
       F(35)=COMPLEX(FR(35),FI(35))
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


       hel(1) = (2*Fs+Fq*Invs24*s15*(Invs13mmesq*(-2*MESQ-MU2SQ+s12+2*
     -   s15-s23-2*s34+s45)+Invs35mmesq*(-2*MESQ-MU2SQ+s12+2*s15-2*s2
     -   3-s34+s45)))/s15
       hel(2) = -2*Fq*Invs24*(Invs13mmesq+Invs35mmesq)
       hel(3) = -4*Fq*Invs13mmesq*Invs24+(4*Fs*Invs34mu)/s15
       hel(4) = 4*(Fq*Invs13mmesq*Invs24+(Fs*Invs23mu)/s15)
       ssa(1) = -2*ME4SQ-MU4SQ-s12*s15+s12*s23-s23*s34-MESQ*(2*MU2SQ-2
     -   *s12-2*s15+s23+s34-2*s45)-2*s12*s45-s15*s45+s34*s45+MU2SQ*(s
     -   12+s45)
       ssa(2) = s15-s23-s34
       ssa(3) = MU4SQ*(2*s12-s15+s23-s34)+2*ME4SQ*(-MU2SQ-s15+s23+s34)
     -   -MU2SQ*(s12*(2*s12+s15+s23-s34)+(-s15+s23+s34)*s45)-(s15-s23
     -   -s34)*(s23*s34+(s15-s34)*s45+s12*(s15-s23+2*s45))+MESQ*(-2*M
     -   U4SQ+2*MU2SQ*(2*s12+s23)+(s15-s23-s34)*(2*s12+2*s15-s23-s34+
     -   2*s45))
       ssa(4) = (MESQ-s12)*(MESQ+MU2SQ-s12-s15)+(MESQ+MU2SQ-s12)*s34
       ssa(5) = 2*ME4SQ+MU4SQ+s12*s15-s12*s23+s23*s34+MESQ*(2*MU2SQ-2*
     -   s12-2*s15+s23+s34-2*s45)+(2*s12+s15-s34)*s45-MU2SQ*(s12+s45)
       ssa(6) = 2*MESQ+s15
       ssa(7) = MU4SQ-s12*s15-2*s15**2+s12*s23+2*s15*s23+2*s15*s34-s23
     -   *s34+MESQ*(2*MU2SQ+s23+s34)-s15*s45+s34*s45-MU2SQ*(s12+s45)
       ssa(8) = ME4SQ+MESQ*(MU2SQ-s15+s23-2*s45)+MU2SQ*(s23-s45)+s45*(
     -   s15-s23+s45)
       ssa(9) = ME4SQ*(2*MU2SQ+s15-s23-s34)+(s12+s15-s34)*(s15-s23-s34
     -   )*(s15-s23+s45)-MU4SQ*(s12+s15-s23-s34+s45)+MU2SQ*(s12**2+s1
     -   2*(s15-s34)+s45*(s15-s23+s45))+MESQ*(4*MU4SQ+MU2SQ*(-2*s12-2
     -   *s15+s23+s34-2*s45)-(s15-s23-s34)*(s12+2*s15-s23-s34+s45))
       ssav(1) = 2*MESQ+MU2SQ-s15+s23-2*s45
       ssav(2) = s15-s23-s34
       ssav(3) = -MU4SQ+MU2SQ*(2*s12+s23)+2*MESQ*(-MU2SQ-s15+s23+s34)+
     -   (s15-s23-s34)*(s15-s23+2*s45)
       ssav(4) = 2*MESQ+MU2SQ-2*s12-s15+s34
       ssav(5) = -s15+s23+s34
       ssav(6) = Invs34MU*(2*ME*(2*MESQ+MU2SQ-2*s12-s15+s34)*CONJG(F(1
     -   ))-2*ME*(2*MESQ+MU2SQ-s15+s23-2*s45)*CONJG(F(2))-(2*ME4SQ+MU
     -   4SQ+s12*s15-s12*s23+s23*s34+MESQ*(2*MU2SQ-2*s12-2*s15+s23+s3
     -   4-2*s45)+(2*s12+s15-s34)*s45-MU2SQ*(s12+s45))*CONJG(F(3))+2*
     -   ((MESQ-s12)*(MESQ+MU2SQ-s12-s15)+(MESQ+MU2SQ-s12)*s34)*CONJG
     -   (F(4)))
       ssav(7) = Invs34MU*(2*ME*(-((s15-s23)*(s12+s15-s34))+MESQ*(2*s1
     -   5-s23-s34)+(-s15+s34)*s45+MU2SQ*(-s12+s15-s23+s45))*CONJG(F(
     -   1))+2*ME*(-MU4SQ+MU2SQ*(2*s12+s23)+2*MESQ*(-MU2SQ-s15+s23+s3
     -   4)+(s15-s23-s34)*(s15-s23+2*s45))*CONJG(F(2))-(2*ME4SQ*(MU2S
     -   Q+s15-s23-s34)+MU4SQ*(-2*s12+s15-s23+s34)+MU2SQ*(s12*(2*s12+
     -   s15+s23-s34)+(-s15+s23+s34)*s45)+(s15-s23-s34)*(s23*s34+(s15
     -   -s34)*s45+s12*(s15-s23+2*s45))+MESQ*(2*MU4SQ-2*MU2SQ*(2*s12+
     -   s23)-(s15-s23-s34)*(2*s12+2*s15-s23-s34+2*s45)))*CONJG(F(3))
     -   +(-(MU4SQ*s12)+ME4SQ*(2*s15-s23-s34)+MU2SQ*(s12**2-s12*(2*s1
     -   5-2*s23+s45)+2*s34*(s15-s23+s45))+s12*(2*s15**2+s12*(s15-s23
     -   )+s34*(s23-s45)+s15*(-2*(s23+s34)+s45))+MESQ*(MU4SQ-2*s15**2
     -   +2*s15*s23+2*s15*s34-s23*s34+s12*(-3*s15+2*s23+s34)-s15*s45+
     -   s34*s45+MU2SQ*(-s12+2*s15-2*(s23+s34)+s45)))*CONJG(F(4)))
       ssav(8) = Invs34MU*(-(ME*(MU2SQ+s34)*(2*MESQ+MU2SQ-2*s12-s15+s3
     -   4)*CONJG(F(1)))+2*ME*MU2SQ*(2*MESQ+MU2SQ-2*s12-s15+s34)*CONJ
     -   G(F(2))+2*MU2SQ*((MESQ-s12)*(MESQ+MU2SQ-s12-s15)+(MESQ+MU2SQ
     -   -s12)*s34)*CONJG(F(3))-(MU2SQ+s34)*((MESQ-s12)*(MESQ+MU2SQ-s
     -   12-s15)+(MESQ+MU2SQ-s12)*s34)*CONJG(F(4)))
       ssav(9) = (Invs34MU*(-2*ME*(MU2SQ-s15+s34)*(2*MESQ+MU2SQ-s15+s2
     -   3-2*s45)*CONJG(F(1))+2*ME*(-s15+s23+s34)*(2*MESQ+MU2SQ-s15+s
     -   23-2*s45)*CONJG(F(2))-(s15-s23-s34)*(2*ME4SQ+MU4SQ+s12*s15-s
     -   12*s23+s23*s34+MESQ*(2*MU2SQ-2*s12-2*s15+s23+s34-2*s45)+(2*s
     -   12+s15-s34)*s45-MU2SQ*(s12+s45))*CONJG(F(3))-(MU2SQ-s15+s34)
     -   *(2*ME4SQ+MU4SQ+s12*s15-s12*s23+s23*s34+MESQ*(2*MU2SQ-2*s12-
     -   2*s15+s23+s34-2*s45)+(2*s12+s15-s34)*s45-MU2SQ*(s12+s45))*CO
     -   NJG(F(4))))/2q0
       ssav(10) = Invs23MU*(2*ME*(2*MESQ+MU2SQ-2*s12-s15+s34)*CONJG(F(
     -   5))-2*ME*(2*MESQ+MU2SQ-s15+s23-2*s45)*CONJG(F(6))+2*(ME4SQ+M
     -   ESQ*(MU2SQ-s15+s23-2*s45)+MU2SQ*(s23-s45)+s45*(s15-s23+s45))
     -   *CONJG(F(7))-(2*ME4SQ+MU4SQ+s12*s15-s12*s23+s23*s34+MESQ*(2*
     -   MU2SQ-2*s12-2*s15+s23+s34-2*s45)+(2*s12+s15-s34)*s45-MU2SQ*(
     -   s12+s45))*CONJG(F(8)))
       ssav(11) = Invs23MU*(-2*ME*MU2SQ*(2*MESQ+MU2SQ-s15+s23-2*s45)*C
     -   ONJG(F(5))+2*ME*((s12+s23)*(MU2SQ-s15+s23)+MESQ*(s23+s34)-(M
     -   U2SQ-s15+2*s23+s34)*s45)*CONJG(F(6))-(-MU6SQ+MU4SQ*(s12+s15+
     -   s23)+ME4SQ*(s23+s34)+MESQ*(-MU4SQ-s12*s15+s12*s23-s15*s23+s2
     -   3**2-s15*s34+MU2SQ*(s12+3*s23+s34-s45)+(s15-3*s23-2*s34)*s45
     -   )+(s15-s23+s45)*(s12*(s15-s23)+s23*s34+(-s15+2*s23+s34)*s45)
     -   +MU2SQ*((s23-s45)*(-2*s15+2*s23+s34-s45)-s12*(2*s15+s45)))*C
     -   ONJG(F(7))+2*MU2SQ*(ME4SQ+MESQ*(MU2SQ-s15+s23-2*s45)+MU2SQ*(
     -   s23-s45)+s45*(s15-s23+s45))*CONJG(F(8)))
       ssav(12) = (Invs23MU*(2*ME*(s15-s23-s34)*(2*MESQ+MU2SQ-2*s12-s1
     -   5+s34)*CONJG(F(5))+2*ME*(MU2SQ-s15+s23)*(2*MESQ+MU2SQ-2*s12-
     -   s15+s34)*CONJG(F(6))-(MU2SQ-s15+s23)*(2*ME4SQ+MU4SQ+s12*s15-
     -   s12*s23+s23*s34+MESQ*(2*MU2SQ-2*s12-2*s15+s23+s34-2*s45)+(2*
     -   s12+s15-s34)*s45-MU2SQ*(s12+s45))*CONJG(F(7))-(s15-s23-s34)*
     -   (2*ME4SQ+MU4SQ+s12*s15-s12*s23+s23*s34+MESQ*(2*MU2SQ-2*s12-2
     -   *s15+s23+s34-2*s45)+(2*s12+s15-s34)*s45-MU2SQ*(s12+s45))*CON
     -   JG(F(8))))/2q0
       ssav(13) = Invs23MU*(-2*ME*MU2SQ*(2*MESQ+MU2SQ-s15+s23-2*s45)*C
     -   ONJG(F(5))+ME*(MU2SQ+s23)*(2*MESQ+MU2SQ-s15+s23-2*s45)*CONJG
     -   (F(6))-(MU2SQ+s23)*(ME4SQ+MESQ*(MU2SQ-s15+s23-2*s45)+MU2SQ*(
     -   s23-s45)+s45*(s15-s23+s45))*CONJG(F(7))+2*MU2SQ*(ME4SQ+MESQ*
     -   (MU2SQ-s15+s23-2*s45)+MU2SQ*(s23-s45)+s45*(s15-s23+s45))*CON
     -   JG(F(8)))
       ssav(14) = -2*ME*(2*MESQ+MU2SQ-s15+s23-2*s45)*CONJG(F(27))+4*(2
     -   *MESQ+s15)*CONJG(F(28))+2*MESQ*(2*MESQ+MU2SQ-s15+s23-2*s45)*
     -   CONJG(F(29))+4*ME*(-MESQ+MU2SQ+2*s15-2*s23+s45)*CONJG(F(30))
       ssav(15) = 2*ME*(-MU4SQ+MU2SQ*(2*s12+s23)+2*MESQ*(-MU2SQ-s15+s2
     -   3+s34)+(s15-s23-s34)*(s15-s23+2*s45))*CONJG(F(27))-2*(MU4SQ-
     -   s12*s15-2*s15**2+s12*s23+2*s15*s23+2*s15*s34-s23*s34+MESQ*(2
     -   *MU2SQ+s23+s34)-s15*s45+s34*s45-MU2SQ*(s12+s45))*CONJG(F(28)
     -   )+(-(ME4SQ*(2*MU2SQ+s23+s34))-MU4SQ*(s15+s23-s45)-(s15-s23+s
     -   45)*(s12*(s15-s23)+s23*s34+(s15-s34)*s45)+MU2SQ*(s12*(s15+s2
     -   3-s45)-s45*(s15-s23+s45))+MESQ*(MU4SQ+s12*s15-s12*s23+s15*s2
     -   3-s23**2+s15*s34+(s15+s23)*s45+MU2SQ*(s12+2*s15-2*(s23+s34)+
     -   3*s45)))*CONJG(F(29))-4*ME*(2*MU4SQ+MESQ*(3*MU2SQ+2*s15-2*s2
     -   3-2*s34)+MU2SQ*(-3*s12-s15+s34)-2*(s15-s23-s34)*(s15-s23+s45
     -   ))*CONJG(F(30))
       ssav(16) = 2*ME*MU2SQ*(2*MESQ+MU2SQ-2*s12-s15+s34)*CONJG(F(27))
     -   +(-2*ME4SQ-MU4SQ-s12*s15+s12*s23-s23*s34-MESQ*(2*MU2SQ-2*s12
     -   -2*s15+s23+s34-2*s45)-2*s12*s45-s15*s45+s34*s45+MU2SQ*(s12+s
     -   45))*CONJG(F(28))-((MESQ-s15+s23-s45)*(2*ME4SQ+MU4SQ+s12*s15
     -   -s12*s23+s23*s34+MESQ*(2*MU2SQ-2*s12-2*s15+s23+s34-2*s45)+(2
     -   *s12+s15-s34)*s45-MU2SQ*(s12+s45))*CONJG(F(29)))/2q0+2*ME*MU
     -   2SQ*(2*MESQ+MU2SQ-2*s12-s15+s34)*CONJG(F(30))
       ssav(17) = ME*(-s15+s23+s34)*(2*MESQ+MU2SQ-s15+s23-2*s45)*CONJG
     -   (F(27))+(-2*ME4SQ-MU4SQ-s12*s15+s12*s23-s23*s34-MESQ*(2*MU2S
     -   Q-2*s12-2*s15+s23+s34-2*s45)-2*s12*s45-s15*s45+s34*s45+MU2SQ
     -   *(s12+s45))*CONJG(F(28))-(MESQ+MU2SQ-s12)*(ME4SQ+MESQ*(MU2SQ
     -   -s15+s23-2*s45)+MU2SQ*(s23-s45)+s45*(s15-s23+s45))*CONJG(F(2
     -   9))-2*ME*(MU4SQ+MU2SQ*(s15-s34)+(s15-s23-s34)*(MESQ-s15+s23-
     -   s45))*CONJG(F(30))
       ssav(18) = (-8*ME3SQ+2*ME*s15)*CONJG(F(9))+2*ME*(2*MESQ+MU2SQ-2
     -   *s12-s15+s34)*CONJG(F(10))-2*ME*(2*MESQ+MU2SQ-s15+s23-2*s45)
     -   *CONJG(F(11))+(8*ME3SQ-2*ME*s15)*CONJG(F(12))+4*(2*MESQ+s15)
     -   *CONJG(F(13))+(-2*ME4SQ-MU4SQ-s12*s15+s12*s23-s23*s34-MESQ*(
     -   2*MU2SQ-2*s12-2*s15+s23+s34-2*s45)-2*s12*s45-s15*s45+s34*s45
     -   +MU2SQ*(s12+s45))*CONJG(F(14))+(2*(MESQ-s12)*(MESQ+MU2SQ-s12
     -   -s15)+2*(MESQ+MU2SQ-s12)*s34)*CONJG(F(15))-2*MESQ*(2*MESQ+MU
     -   2SQ-2*s12-s15+s34)*CONJG(F(16))+2*MESQ*(2*MESQ+MU2SQ-2*s12-s
     -   15+s34)*CONJG(F(17))+2*(ME4SQ+MESQ*(MU2SQ-s15+s23-2*s45)+MU2
     -   SQ*(s23-s45)+s45*(s15-s23+s45))*CONJG(F(18))+(-2*ME4SQ-MU4SQ
     -   -s12*s15+s12*s23-s23*s34-MESQ*(2*MU2SQ-2*s12-2*s15+s23+s34-2
     -   *s45)-2*s12*s45-s15*s45+s34*s45+MU2SQ*(s12+s45))*CONJG(F(19)
     -   )+2*MESQ*(2*MESQ+MU2SQ-s15+s23-2*s45)*CONJG(F(20))-2*MESQ*(2
     -   *MESQ+MU2SQ-s15+s23-2*s45)*CONJG(F(21))+ME*(-MU4SQ+s12*s15+2
     -   *s15**2-s12*s23-2*s15*s23-2*s15*s34+s23*s34+MESQ*(-2*MU2SQ-6
     -   *s15+5*(s23+s34))+s15*s45-s34*s45+MU2SQ*(s12+s45))*CONJG(F(2
     -   2))+2*ME*(MU4SQ+MU2SQ*(s15-s23)+(s15-s23-s34)*(MESQ-s12-s15+
     -   s34))*CONJG(F(23))-2*ME*(MU4SQ+MU2SQ*(s15-s34)+(s15-s23-s34)
     -   *(MESQ-s15+s23-s45))*CONJG(F(24))+ME*(MU4SQ-s12*s15-2*s15**2
     -   +s12*s23+2*s15*s23+2*s15*s34-s23*s34+MESQ*(2*MU2SQ+6*s15-5*(
     -   s23+s34))-s15*s45+s34*s45-MU2SQ*(s12+s45))*CONJG(F(25))-16*(
     -   MU4SQ-s12*s15-2*s15**2+s12*s23+2*s15*s23+2*s15*s34-s23*s34+M
     -   ESQ*(2*MU2SQ+s23+s34)-s15*s45+s34*s45-MU2SQ*(s12+s45))*CONJG
     -   (F(26))
       ssav(19) = ME*(MU4SQ-s12*s15+s12*s23-s23*s34+MESQ*(2*MU2SQ-2*s1
     -   5+3*(s23+s34))-s15*s45+s34*s45-MU2SQ*(s12+s45))*CONJG(F(9))-
     -   2*ME*MU2SQ*(2*MESQ+MU2SQ-s15+s23-2*s45)*CONJG(F(10))+2*ME*(-
     -   MU4SQ+MU2SQ*(2*s12+s23)+2*MESQ*(-MU2SQ-s15+s23+s34)+(s15-s23
     -   -s34)*(s15-s23+2*s45))*CONJG(F(11))+ME*(MU4SQ-s12*s15-2*s15*
     -   *2+s12*s23+2*s15*s23+2*s15*s34-s23*s34+MESQ*(2*MU2SQ+6*s15-5
     -   *(s23+s34))-s15*s45+s34*s45-MU2SQ*(s12+s45))*CONJG(F(12))-2*
     -   (MU4SQ-s12*s15-2*s15**2+s12*s23+2*s15*s23+2*s15*s34-s23*s34+
     -   MESQ*(2*MU2SQ+s23+s34)-s15*s45+s34*s45-MU2SQ*(s12+s45))*CONJ
     -   G(F(13))+(MU4SQ*(2*s12-s15+s23-s34)+2*ME4SQ*(-MU2SQ-s15+s23+
     -   s34)-MU2SQ*(s12*(2*s12+s15+s23-s34)+(-s15+s23+s34)*s45)-(s15
     -   -s23-s34)*(s23*s34+(s15-s34)*s45+s12*(s15-s23+2*s45))+MESQ*(
     -   -2*MU4SQ+2*MU2SQ*(2*s12+s23)+(s15-s23-s34)*(2*s12+2*s15-s23-
     -   s34+2*s45)))*CONJG(F(14))-MU2SQ*(2*ME4SQ+MU4SQ+s12*s15-s12*s
     -   23+s23*s34+MESQ*(2*MU2SQ-2*s12-2*s15+s23+s34-2*s45)+(2*s12+s
     -   15-s34)*s45-MU2SQ*(s12+s45))*CONJG(F(15))+(MU6SQ+ME4SQ*(2*MU
     -   2SQ+s23+s34)-MU4SQ*(2*s12+s45)+MU2SQ*(s12**2-s12*s15+s12*s23
     -   +2*s15*s34-s23*s34+(s12+s15+s34)*s45)+MESQ*(MU4SQ-2*s15*s34+
     -   s23*s34+2*s34**2-s12*(s15+s34)-s15*s45+s34*s45-MU2SQ*(3*s12+
     -   2*s15-s23+s34+s45))+s12*(2*s15**2+s12*(s15-s23)+s34*(s23-s45
     -   )+s15*(-2*(s23+s34)+s45)))*CONJG(F(16))+2*MESQ*(MU4SQ+MU2SQ*
     -   (s15-s23)+(s15-s23-s34)*(MESQ-s12-s15+s34))*CONJG(F(17))+(MU
     -   6SQ+2*ME4SQ*(MU2SQ+s15-s23-s34)-MU4SQ*(s12+s45)+2*(s15-s23-s
     -   34)*s45*(s15-s23+s45)+MU2SQ*(-((2*s23+s34)*(s23-s45))+s15*(2
     -   *s23-s45)+s12*(s15-s23+2*s45))+MESQ*(2*MU4SQ-2*(s15-s23-s34)
     -   *(s15-s23+2*s45)-MU2SQ*(2*s12+s23+s34+2*s45)))*CONJG(F(18))+
     -   2*MU2SQ*(ME4SQ+MESQ*(MU2SQ-s15+s23-2*s45)+MU2SQ*(s23-s45)+s4
     -   5*(s15-s23+s45))*CONJG(F(19))+(-(ME4SQ*(2*MU2SQ+s23+s34))-MU
     -   4SQ*(s15+s23-s45)-(s15-s23+s45)*(s12*(s15-s23)+s23*s34+(s15-
     -   s34)*s45)+MU2SQ*(s12*(s15+s23-s45)-s45*(s15-s23+s45))+MESQ*(
     -   MU4SQ+s12*s15-s12*s23+s15*s23-s23**2+s15*s34+(s15+s23)*s45+M
     -   U2SQ*(s12+2*s15-2*(s23+s34)+3*s45)))*CONJG(F(20))-2*MESQ*(MU
     -   4SQ+MU2SQ*(s15-s34)+(s15-s23-s34)*(MESQ-s15+s23-s45))*CONJG(
     -   F(21))+2*ME*(MU6SQ+MU4SQ*(-2*s12+s15)+ME4SQ*(2*MU2SQ+s15-s23
     -   -s34)+s12*(s15-s23-s34)*(s15-s23+s45)+MU2SQ*(s12**2+(s15-s23
     -   +s45)*(s34+s45))-MESQ*(2*MU4SQ+(s15-s23-s34)*(s12+s15-s23+s4
     -   5)+MU2SQ*(2*s12+s15-s23+s34+2*s45)))*CONJG(F(22))-2*ME*MU2SQ
     -   *(MU4SQ+MU2SQ*(s15-s34)+(s15-s23-s34)*(MESQ-s15+s23-s45))*CO
     -   NJG(F(23))-2*ME*(MU6SQ+MU4SQ*(2*s15-2*s23-s34)+MU2SQ*(MESQ-s
     -   12)*(s15-s23-s34)+(-s15+s23+s34)**2*(MESQ-s15+s23-s45))*CONJ
     -   G(F(24))+2*ME*(ME4SQ*(-2*MU2SQ-s15+s23+s34)-(s12+s15-s34)*(s
     -   15-s23-s34)*(s15-s23+s45)+MU4SQ*(s12+s15-s23-s34+s45)-MU2SQ*
     -   (s12**2+s12*(s15-s34)+s45*(s15-s23+s45))+MESQ*(2*MU4SQ+(s15-
     -   s23-s34)*(s12+2*s15-s23-s34+s45)+MU2SQ*(2*s12+2*s15-s23-s34+
     -   2*s45)))*CONJG(F(25))+32*(ME4SQ*(2*MU2SQ+s15-s23-s34)+(s12+s
     -   15-s34)*(s15-s23-s34)*(s15-s23+s45)-MU4SQ*(s12+s15-s23-s34+s
     -   45)+MU2SQ*(s12**2+s12*(s15-s34)+s45*(s15-s23+s45))+MESQ*(4*M
     -   U4SQ+MU2SQ*(-2*s12-2*s15+s23+s34-2*s45)-(s15-s23-s34)*(s12+2
     -   *s15-s23-s34+s45)))*CONJG(F(26))
       ssav(20) = ME*(2*MESQ+MU2SQ-2*s12-s15+s34)*(MESQ-s15+s23-s45)*C
     -   ONJG(F(9))+ME*(s15-s23-s34)*(2*MESQ+MU2SQ-2*s12-s15+s34)*CON
     -   JG(F(10))+2*ME*MU2SQ*(2*MESQ+MU2SQ-2*s12-s15+s34)*CONJG(F(11
     -   ))-ME*(2*MESQ+MU2SQ-2*s12-s15+s34)*(MESQ+MU2SQ-s45)*CONJG(F(
     -   12))+(-2*ME4SQ-MU4SQ-s12*s15+s12*s23-s23*s34-MESQ*(2*MU2SQ-2
     -   *s12-2*s15+s23+s34-2*s45)-2*s12*s45-s15*s45+s34*s45+MU2SQ*(s
     -   12+s45))*CONJG(F(13))+2*MU2SQ*((MESQ-s12)*(MESQ+MU2SQ-s12-s1
     -   5)+(MESQ+MU2SQ-s12)*s34)*CONJG(F(14))+(-s15+s23+s34)*(-((MES
     -   Q-s12)*(MESQ+MU2SQ-s12-s15))-(MESQ+MU2SQ-s12)*s34)*CONJG(F(1
     -   5))+((MESQ-s12)*(MESQ+MU2SQ-s12-s15)+(MESQ+MU2SQ-s12)*s34)*(
     -   MESQ-s15+s23-s45)*CONJG(F(16))-((MESQ-s12)*(MESQ+MU2SQ-s12-s
     -   15)+(MESQ+MU2SQ-s12)*s34)*(MESQ+MU2SQ-s45)*CONJG(F(17))-MU2S
     -   Q*(2*ME4SQ+MU4SQ+s12*s15-s12*s23+s23*s34+MESQ*(2*MU2SQ-2*s12
     -   -2*s15+s23+s34-2*s45)+(2*s12+s15-s34)*s45-MU2SQ*(s12+s45))*C
     -   ONJG(F(18))-((s15-s23-s34)*(2*ME4SQ+MU4SQ+s12*s15-s12*s23+s2
     -   3*s34+MESQ*(2*MU2SQ-2*s12-2*s15+s23+s34-2*s45)+(2*s12+s15-s3
     -   4)*s45-MU2SQ*(s12+s45))*CONJG(F(19)))/2q0-((MESQ-s15+s23-s45
     -   )*(2*ME4SQ+MU4SQ+s12*s15-s12*s23+s23*s34+MESQ*(2*MU2SQ-2*s12
     -   -2*s15+s23+s34-2*s45)+(2*s12+s15-s34)*s45-MU2SQ*(s12+s45))*C
     -   ONJG(F(20)))/2q0+((MESQ+MU2SQ-s45)*(2*ME4SQ+MU4SQ+s12*s15-s1
     -   2*s23+s23*s34+MESQ*(2*MU2SQ-2*s12-2*s15+s23+s34-2*s45)+(2*s1
     -   2+s15-s34)*s45-MU2SQ*(s12+s45))*CONJG(F(21)))/2q0+ME*(-MU4SQ
     -   +MU2SQ*(-s15+s23)+(-MESQ+s12+s15-s34)*(s15-s23-s34))*(-MESQ+
     -   s15-s23+s45)*CONJG(F(22))+ME*(-MU4SQ+MU2SQ*(-s15+s23)+(-MESQ
     -   +s12+s15-s34)*(s15-s23-s34))*(-s15+s23+s34)*CONJG(F(23))+2*M
     -   E*MU2SQ*(MU4SQ+MU2SQ*(s15-s23)+(s15-s23-s34)*(MESQ-s12-s15+s
     -   34))*CONJG(F(24))+ME*(-MU4SQ+MU2SQ*(-s15+s23)+(-MESQ+s12+s15
     -   -s34)*(s15-s23-s34))*(MESQ+MU2SQ-s45)*CONJG(F(25))-8*(2*ME4S
     -   Q*(MU2SQ+s15-s23-s34)+MU4SQ*(-2*s12+s15-s23+s34)+MU2SQ*(s12*
     -   (2*s12+s15+s23-s34)+(-s15+s23+s34)*s45)+(s15-s23-s34)*(s23*s
     -   34+(s15-s34)*s45+s12*(s15-s23+2*s45))+MESQ*(2*MU4SQ-2*MU2SQ*
     -   (2*s12+s23)-(s15-s23-s34)*(2*s12+2*s15-s23-s34+2*s45)))*CONJ
     -   G(F(26))
       ssav(21) = ME*(MESQ+MU2SQ-s12)*(2*MESQ+MU2SQ-s15+s23-2*s45)*CON
     -   JG(F(9))-2*ME*MU2SQ*(2*MESQ+MU2SQ-s15+s23-2*s45)*CONJG(F(10)
     -   )+ME*(-s15+s23+s34)*(2*MESQ+MU2SQ-s15+s23-2*s45)*CONJG(F(11)
     -   )+ME*(-MESQ+s12+s15-s34)*(2*MESQ+MU2SQ-s15+s23-2*s45)*CONJG(
     -   F(12))+(-2*ME4SQ-MU4SQ-s12*s15+s12*s23-s23*s34-MESQ*(2*MU2SQ
     -   -2*s12-2*s15+s23+s34-2*s45)-2*s12*s45-s15*s45+s34*s45+MU2SQ*
     -   (s12+s45))*CONJG(F(13))-((s15-s23-s34)*(2*ME4SQ+MU4SQ+s12*s1
     -   5-s12*s23+s23*s34+MESQ*(2*MU2SQ-2*s12-2*s15+s23+s34-2*s45)+(
     -   2*s12+s15-s34)*s45-MU2SQ*(s12+s45))*CONJG(F(14)))/2q0-MU2SQ*
     -   (2*ME4SQ+MU4SQ+s12*s15-s12*s23+s23*s34+MESQ*(2*MU2SQ-2*s12-2
     -   *s15+s23+s34-2*s45)+(2*s12+s15-s34)*s45-MU2SQ*(s12+s45))*CON
     -   JG(F(15))+((MESQ+MU2SQ-s12)*(2*ME4SQ+MU4SQ+s12*s15-s12*s23+s
     -   23*s34+MESQ*(2*MU2SQ-2*s12-2*s15+s23+s34-2*s45)+(2*s12+s15-s
     -   34)*s45-MU2SQ*(s12+s45))*CONJG(F(16)))/2q0-((MESQ-s12-s15+s3
     -   4)*(2*ME4SQ+MU4SQ+s12*s15-s12*s23+s23*s34+MESQ*(2*MU2SQ-2*s1
     -   2-2*s15+s23+s34-2*s45)+(2*s12+s15-s34)*s45-MU2SQ*(s12+s45))*
     -   CONJG(F(17)))/2q0+(s15-s23-s34)*(ME4SQ+MESQ*(MU2SQ-s15+s23-2
     -   *s45)+MU2SQ*(s23-s45)+s45*(s15-s23+s45))*CONJG(F(18))+2*MU2S
     -   Q*(ME4SQ+MESQ*(MU2SQ-s15+s23-2*s45)+MU2SQ*(s23-s45)+s45*(s15
     -   -s23+s45))*CONJG(F(19))-(MESQ+MU2SQ-s12)*(ME4SQ+MESQ*(MU2SQ-
     -   s15+s23-2*s45)+MU2SQ*(s23-s45)+s45*(s15-s23+s45))*CONJG(F(20
     -   ))+(MESQ-s12-s15+s34)*(ME4SQ+MESQ*(MU2SQ-s15+s23-2*s45)+MU2S
     -   Q*(s23-s45)+s45*(s15-s23+s45))*CONJG(F(21))+ME*(MESQ+MU2SQ-s
     -   12)*(MU4SQ+MU2SQ*(s15-s34)+(s15-s23-s34)*(MESQ-s15+s23-s45))
     -   *CONJG(F(22))-2*ME*MU2SQ*(MU4SQ+MU2SQ*(s15-s34)+(s15-s23-s34
     -   )*(MESQ-s15+s23-s45))*CONJG(F(23))+ME*(-s15+s23+s34)*(MU4SQ+
     -   MU2SQ*(s15-s34)+(s15-s23-s34)*(MESQ-s15+s23-s45))*CONJG(F(24
     -   ))+ME*(-MESQ+s12+s15-s34)*(MU4SQ+MU2SQ*(s15-s34)+(s15-s23-s3
     -   4)*(MESQ-s15+s23-s45))*CONJG(F(25))+16*MU2SQ*(ME4SQ+MESQ*(MU
     -   2SQ-s15+s23-2*s45)+MU2SQ*(s23-s45)+s45*(s15-s23+s45))*CONJG(
     -   F(26))
       ssav(22) = 4*((2*MESQ+s15)*CONJG(F(31))-4*MESQ*(2*MESQ+MU2SQ-2*
     -   s12-s15+s34)*CONJG(F(32))+ME*(MESQ+2*MU2SQ-s12+s15-s34)*CONJ
     -   G(F(33)))
       ssav(23) = -2*(MU4SQ-s12*s15-2*s15**2+s12*s23+2*s15*s23+2*s15*s
     -   34-s23*s34+MESQ*(2*MU2SQ+s23+s34)-s15*s45+s34*s45-MU2SQ*(s12
     -   +s45))*CONJG(F(31))-16*MESQ*(MU4SQ+MU2SQ*(s15-s23)+(s15-s23-
     -   s34)*(MESQ-s12-s15+s34))*CONJG(F(32))-4*ME*(MU4SQ-(s12+s15-s
     -   34)*(s15-s23-s34)+MESQ*(3*MU2SQ+s15-s23-s34)+MU2SQ*(-2*s15+2
     -   *s23-3*s45))*CONJG(F(33))
       ssav(24) = (-2*ME4SQ-MU4SQ-s12*s15+s12*s23-s23*s34-MESQ*(2*MU2S
     -   Q-2*s12-2*s15+s23+s34-2*s45)-2*s12*s45-s15*s45+s34*s45+MU2SQ
     -   *(s12+s45))*CONJG(F(31))+8*((MESQ-s12)*(MESQ+MU2SQ-s12-s15)+
     -   (MESQ+MU2SQ-s12)*s34)*(MESQ+MU2SQ-s45)*CONJG(F(32))+2*ME*(-M
     -   U4SQ+(MESQ-s12)*(s15-s23-s34)-MU2SQ*s34)*CONJG(F(33))
       ssav(25) = (-2*ME4SQ-MU4SQ-s12*s15+s12*s23-s23*s34-MESQ*(2*MU2S
     -   Q-2*s12-2*s15+s23+s34-2*s45)-2*s12*s45-s15*s45+s34*s45+MU2SQ
     -   *(s12+s45))*CONJG(F(31))+4*(MESQ-s12-s15+s34)*(2*ME4SQ+MU4SQ
     -   +s12*s15-s12*s23+s23*s34+MESQ*(2*MU2SQ-2*s12-2*s15+s23+s34-2
     -   *s45)+(2*s12+s15-s34)*s45-MU2SQ*(s12+s45))*CONJG(F(32))-2*ME
     -   *MU2SQ*(2*MESQ+MU2SQ-s15+s23-2*s45)*CONJG(F(33))
       resultn(1) = hel(1)*ssav(6)+hel(2)*ssav(7)+hel(3)*ssav(8)+hel(4
     -   )*ssav(9)
       resultn(2) = hel(1)*ssav(10)+hel(2)*ssav(11)+hel(3)*ssav(12)+he
     -   l(4)*ssav(13)
       resultn(3) = hel(1)*ssav(18)+hel(2)*ssav(19)+hel(3)*ssav(20)+he
     -   l(4)*ssav(21)
       resultn(4) = hel(1)*ssav(14)+hel(2)*ssav(15)+hel(3)*ssav(16)+he
     -   l(4)*ssav(17)
       resultn(5) = hel(1)*ssav(22)+hel(2)*ssav(23)+hel(3)*ssav(24)+he
     -   l(4)*ssav(25)
       resultn(6) = -2*Invs23MU*ME*MU**2*CONJG(F(34))*hel(2)*ssav(1)-2
     -   *Invs23MU*ME*MU**2*CONJG(F(34))*hel(4)*ssav(1)+2*Invs23MU*ME
     -   *CONJG(F(34))*hel(1)*ssav(4)+Invs23MU*ME*CONJG(F(34))*hel(3)
     -   *ssav(2)*ssav(4)
       resultn(7) = -2*Invs34MU*ME*CONJG(F(35))*hel(1)*ssav(1)+2*Invs3
     -   4MU*ME*CONJG(F(35))*hel(2)*ssav(3)+2*Invs34MU*ME*MU**2*CONJG
     -   (F(35))*hel(3)*ssav(4)+Invs34MU*ME*CONJG(F(35))*hel(4)*ssav(
     -   1)*ssav(5)
       result = -2*Invs34MU*ME*CONJG(F(35))*hel(1)*ssav(1)-2*Invs23MU*
     -   ME*MU**2*CONJG(F(34))*hel(2)*ssav(1)-2*Invs23MU*ME*MU**2*CON
     -   JG(F(34))*hel(4)*ssav(1)+2*Invs34MU*ME*CONJG(F(35))*hel(2)*s
     -   sav(3)+2*Invs23MU*ME*CONJG(F(34))*hel(1)*ssav(4)+2*Invs34MU*
     -   ME*MU**2*CONJG(F(35))*hel(3)*ssav(4)+Invs23MU*ME*CONJG(F(34)
     -   )*hel(3)*ssav(2)*ssav(4)+Invs34MU*ME*CONJG(F(35))*hel(4)*ssa
     -   v(1)*ssav(5)+hel(1)*ssav(6)+hel(2)*ssav(7)+hel(3)*ssav(8)+he
     -   l(4)*ssav(9)+hel(1)*ssav(10)+hel(2)*ssav(11)+hel(3)*ssav(12)
     -   +hel(4)*ssav(13)+hel(1)*ssav(14)+hel(2)*ssav(15)+hel(3)*ssav
     -   (16)+hel(4)*ssav(17)+hel(1)*ssav(18)+hel(2)*ssav(19)+hel(3)*
     -   ssav(20)+hel(4)*ssav(21)+hel(1)*ssav(22)+hel(2)*ssav(23)+hel
     -   (3)*ssav(24)+hel(4)*ssav(25)
c The Finite virtual piece should be multiplied for (-1)  since 
c I have multiplied by (-I) to get the F's and k's without (I) factor
c . The factorization from the B_ij is Fact=(I/(4\[Pi])^2 (4 \[Pi])^Eps Gamma[1+Eps] (musq)^(-Eps))
c 
c So, I*I=(-1)!!!
       result =result
       resultb = hel(3)*(CONJG(hel(1))*ssa(1)+CONJG(hel(2))*ssa(3)+2*M
     -   U**2*CONJG(hel(3))*ssa(4)-(CONJG(hel(4))*ssa(2)*ssa(5))/2.)+
     -   hel(1)*(CONJG(hel(3))*ssa(1)+CONJG(hel(4))*ssa(1)+4*CONJG(he
     -   l(1))*ssa(6)-2*CONJG(hel(2))*ssa(7))+(hel(4)*(2*CONJG(hel(1)
     -   )*ssa(1)-CONJG(hel(3))*ssa(2)*ssa(5)+4*MU**2*(CONJG(hel(2))+
     -   CONJG(hel(4)))*ssa(8)))/2.+hel(2)*(CONJG(hel(3))*ssa(3)-2*CO
     -   NJG(hel(1))*ssa(7)+2*MU**2*CONJG(hel(4))*ssa(8)+4*CONJG(hel(
     -   2))*ssa(9))
c************************************************************************************
c************************************************************************************
c************************************************************************************
c************************************************************************************
c************************************************************************************
c************************************************************************************
c************************************************************************************
       Return
       End
