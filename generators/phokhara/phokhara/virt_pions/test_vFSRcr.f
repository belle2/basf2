       subroutine virtual_FSRHelSumcr(ME,MU,p1,p2,p3,p4,p5,
     -   musq,comp,perm,result,resultn,resultb)
c ************************************************************************************
c Author: Francisco Campanario
c E-mail: francam@particle.uni-karlsruhe.de
c Date: 17/2/2010
c Modified:30/5/2018
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
      Complex*16 qlI1,qlI2,qlI3,qlI4,Ival(-2:0)
        Real*8 p1(0:3), p2(0:3), p3(0:3), p4(0:3), p5(0:3)
       Real*8   p1p2, p1p3, p1p4, p1p5, p1sq, p2p3, p2p4, p2p5, 
     -          p2sq, p3p4, p3p5, p3sq, p4p5, p4sq, p5sq
       Real*8   s12, s13, s14, s15, s23, s24, s25, s34, s35, s45
       Complex*16   p1mup1, p1mup2, p1mup3, p2mup1, p2mup2, p2mup3, 
     -          p3mup1, p3mup2, p3mup3, p4mup1, p4mup2, p4mup3, 
     -          p5mup1, p5mup2, p5mup3
       Complex*16 mup2mup3,mup1(0:3)
       Real*8 dotrr
       Complex*16 dotrc,dotcc
      EXTERNAL dotrr,dotrc,dotcc
       Complex*16 result,resultn(12),resultb,result_counter,diag3,
     - counter4,counter7,counter99,dB0,C0count
       Complex*16 A0finG1,A0finG2
       Complex*16 B0finG1,B0finG2,B0finG3
       Complex*16 B0finG4,B0finG5,B0finG6
       Complex*16 C0finG1,C0finG2
       Complex*16 D0finG1,D0finG2,D0finG3
       Complex*16 E0finG
       External A0finG1,A0finG2
       External B0finG1,B0finG2,B0finG3
       External B0finG4,B0finG5,B0finG6
       External C0finG1,C0finG2
       External D0finG1,D0finG2,D0finG3
       External E0finG
       Complex*16 A01,A02,A03,A04,A05
      Real*8 A01R,A02R,A03R,A04R,A05R
      Real*8 A01I,A02I,A03I,A04I,A05I
      Complex*16 B012,B013,B014,B015,B023,B024,B025,B034,B035,B045
      Real*8 B012R,B013R,B014R,B015R,B023R,B024R,B025R,B034R,
     - B035R,B045R
      Real*8 B012I,B013I,B014I,B015I,B023I,B024I,B025I,B034I,
     - B035I,B045I
      Real*8 Bij12R,Bij13R,Bij14R,Bij15R,Bij23R,Bij24R,Bij25R,Bij34R,
     - Bij35R,Bij45R
      Real*8 Bij12I,Bij13I,Bij14I,Bij15I,Bij23I,Bij24I,Bij25I,Bij34I,
     - Bij35I,Bij45I
       Complex*16 C0123,C0124,C0125,C0134,C0135,C0145,C0234,C0235,C024
     -   5,C0345
       Real*8 C0123R,C0124R,C0125R,C0134R,C0135R,C0145R,C0234R,C0235R,
     -   C0245R,C0345R
       Real*8 C0123I,C0124I,C0125I,C0134I,C0135I,C0145I,C0234I,C0235I,
     -   C0245I,C0345I
       Real*8 Cij123R(4,2),Cij124R(4,2),Cij125R(4,2),Cij134R(4,2),Cij1
     -   35R(4,2),Cij145R(4,2),Cij234R(4,2),Cij235R(4,2),Cij245R(4,2)
     -   ,Cij345R(4,2)
       Real*8 Cij123I(4,2),Cij124I(4,2),Cij125I(4,2),Cij134I(4,2),Cij1
     -   35I(4,2),Cij145I(4,2),Cij234I(4,2),Cij235I(4,2),Cij245I(4,2)
     -   ,Cij345I(4,2)
       Complex*16 D01234,D01235,D01245,D01345,D02345,EE0
        Real*8 D01234R,D01235R,D01245R,D01345R,D02345R,EE0R
        Real*8 D01234I,D01235I,D01245I,D01345I,D02345I,EE0I
        Real*8 Dij1234R(13,3),Dij1235R(13,3),Dij1245R(13,3),Dij1345R(13
     -   ,3),Dij2345R(13,3),EijR(46,4)
        Real*8 Dij1234I(13,3),Dij1235I(13,3),Dij1245I(13,3),Dij1345I(13
     -   ,3),Dij2345I(13,3),EijI(46,4)
       Complex*16 SMB(3), SMB1(8),F(24),Fa(9),K(0)
       Real*8 FR(24),FI(24),KR(0),KI(0)
       Complex*16 barpsi_p1_ME(4),psi_p5_ME(4),mup2(0:3),mup3(0:3) 
       Complex*16 barpsi_p1_ME_M(2),psi_p5_ME_M(2)
       Complex*16 barpsi_p1_ME_P(2),psi_p5_ME_P(2)
       Complex*16 SC0,SC1c,SC1r,SC2cc,SC2rr
     -  ,SC2rc,SC3rcc,SC3rrc
       Real*8 ME,MU,musq, P(5) 
       Real*8 MESQ,ME3SQ,ME4SQ,ME6SQ,MU2SQ,MU3SQ,MU4SQ,MU6SQ,Invs23,In
     -   vs45MU
       Complex*16 ssav(27),ssa(9),hel(4)
       EXTERNAL   SC0,SC1c,SC1r,SC2cc,SC2rr
     -            ,SC2rc,SC3rcc,SC3rrc
       Integer comp,perm 
       common/virtual_FSRHelSumFfunctionsmm/F,P
       SAVE/virtual_FSRHelSumFfunctionsmm/
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
       p1sq = dotrr(p1,p1)
       p1p2 = dotrr(p1,p2)
       p1p3 = dotrr(p1,p3)
       p1p4 = dotrr(p1,p4)
       p1p5 = dotrr(p1,p5)
       p2sq = dotrr(p2,p2)
       p2p3 = dotrr(p2,p3)
       p2p4 = dotrr(p2,p4)
       p2p5 = dotrr(p2,p5)
       p3sq = dotrr(p3,p3)
       p3p4 = dotrr(p3,p4)
       p3p5 = dotrr(p3,p5)
       p4sq = dotrr(p4,p4)
       p4p5 = dotrr(p4,p5)
       p5sq = dotrr(p5,p5)
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
       MESQ=ME**2
       ME3SQ=ME**3
       ME4SQ=ME**4
       ME6SQ=ME**6
       MU2SQ=MU**2
       MU3SQ=MU**3
       MU4SQ=MU**4
       MU6SQ=MU**6
       Invs23=1d0/s23
       Invs45MU=1d0/(-MU2SQ+s45)
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
       A01=qlI1(0.d0,musq,0) !A0finG(0d0,musq) 
       A02=qlI1(MU2SQ,musq,0)!A0finG(MU,musq) 
       A04=qlI1(MU2SQ,musq,0) !A0finG(MU,musq)  
       A05=qlI1(MU2SQ,musq,0) !A0finG(MU,musq) 
       A01R=DBLE(A01) 
       A02R=DBLE(A02)          
       A04R=DBLE(A04)   
       A05R=DBLE(A05)           
       A01I=DIMAG(A01)
       A02I=DIMAG(A02)
       A04I=DIMAG(A04)
       A05I=DIMAG(A05)
c************************************************************************************
c************************************************************************************
c************************************************************************************
       B012=qlI2(p1sq,0.d0,MU2SQ,musq,0)!B0finG(ME,0d0,p1sq,musq)   
       B014=qlI2(s45,0.d0,MU2SQ,musq,0)!B0finG(ME,MU,s45,musq)   
       B015=qlI2(p5sq,0.d0,MU2SQ,musq,0)!B0finG(ME,0d0,p5sq,musq) 
       B024=qlI2(s23,MU2SQ,MU2SQ,musq,0)!B0finG(0d0,MU,s23,musq)   
       B025=qlI2(s15,MU2SQ,MU2SQ,musq,0)!B0finG(0d0,0d0,s15,musq)   
       B045=qlI2(p4sq,MU2SQ,MU2SQ,musq,0)!B0finG(MU,0d0,p4sq,musq)  
       B012R=DBLE(B012)          
       B014R=DBLE(B014)   
       B015R=DBLE(B015)           
       B024R=DBLE( B024)  
       B025R=DBLE(B025)   
       B045R=DBLE(B045)  
       B012I=DIMAG(B012)          
       B014I=DIMAG(B014)   
       B015I=DIMAG(B015)           
       B024I=DIMAG(B024)  
       B025I=DIMAG(B025)   
       B045I=DIMAG(B045)  
c************************************************************************************
       call tens_red2_new_Re_Com(0d0,MU,p1sq,A02R,A02I,A01R,A01I,B012R
     -   ,B012I,Bij12R,Bij12I) 
       call tens_red2_new_Re_Com(0d0,MU,s45,A04R,A04I,A01R,A01I,B014R,
     -   B014I,Bij14R,Bij14I) 
       call tens_red2_new_Re_Com(0d0,MU,p5sq,A05R,A05I,A01R,A01I,B015R
     -   ,B015I,Bij15R,Bij15I) 
       call tens_red2_new_Re_Com(MU,MU,s23,A04R,A04I,A02R,A02I,B024R,B
     -   024I,Bij24R,Bij24I) 
       call tens_red2_new_Re_Com(MU,MU,s15,A05R,A05I,A02R,A02I,B025R,B
     -   025I,Bij25R,Bij25I) 
       call tens_red2_new_Re_Com(MU,MU,p4sq,A05R,A05I,A04R,A04I,B045R,
     -   B045I,Bij45R,Bij45I) 
c************************************************************************************
c************************************************************************************
       C0124=qlI3(p1sq,s23,s45,0.d0,MU2SQ,MU2SQ,musq,0)!C0finG(ME,0d0,MU,p1sq,s23,s45,musq)   
       C0125=qlI3(p1sq,s15,p5sq,0.d0,MU2SQ,MU2SQ,musq,0)!C0finG(ME,0d0,0d0,p1sq,s15,p5sq,musq)   
       C0125=C0125-qlI3(p1sq,s15,p5sq,0.d0,MU2SQ,MU2SQ,musq,-1)*
     1 Log(musq/s15)
       C0145=qlI3(s45,p4sq,p5sq,0.d0,MU2SQ,MU2SQ,musq,0)!C0finG(ME,MU,0d0,s45,p4sq,p5sq,musq)   
       C0245=qlI3(s23,p4sq,s15,MU2SQ,MU2SQ,MU2SQ,musq,0)!C0finG(0d0,MU,0d0,s23,p4sq,s15,musq)   
c************************************************************************************
c************************************************************************************
       call tens_red3_new_Re_Com_G(0d0,MU,MU,p1sq,s23,s45,B024R,B014R,
     -   B012R,B024I,B014I,B012I,Bij24R,Bij14R,Bij12R,Bij24I,Bij14I,B
     -   ij12I,C0124,C0124R,C0124I,Cij124R,Cij124I) 
       call tens_red3_new_Re_Com_G(0d0,MU,MU,p1sq,s15,p5sq,B025R,B015R
     -   ,B012R,B025I,B015I,B012I,Bij25R,Bij15R,Bij12R,Bij25I,Bij15I,
     -   Bij12I,C0125,C0125R,C0125I,Cij125R,Cij125I) 
       call tens_red3_new_Re_Com_G(0d0,MU,MU,s45,p4sq,p5sq,B045R,B015R
     -   ,B014R,B045I,B015I,B014I,Bij45R,Bij15R,Bij14R,Bij45I,Bij15I,
     -   Bij14I,C0145,C0145R,C0145I,Cij145R,Cij145I) 
       call tens_red3_new_Re_Com_G(MU,MU,MU,s23,p4sq,s15,B045R,B025R,B
     -   024R,B045I,B025I,B024I,Bij45R,Bij25R,Bij24R,Bij45I,Bij25I,Bi
     -   j24I,C0245,C0245R,C0245I,Cij245R,Cij245I) 
c************************************************************************************
c************************************************************************************
       D01245=qlI4(p1sq,s23,p4sq,p5sq,s45,s15
     - ,0.d0,MU2SQ,MU2SQ,MU2SQ,musq,0)!D0finG(ME,0d0,MU,0d0,s45,s15,p1sq,s23,p4sq,p5sq,musq)
       D01245=D01245-qlI4(p1sq,s23,p4sq,p5sq,s45,s15
     - ,0.d0,MU2SQ,MU2SQ,MU2SQ,musq,-1)*Log(musq/s15)
c$$$       Print*," D01245",D01245
c************************************************************************************
c************************************************************************************
       call tens_red4_new_Re_Com_G(0d0,MU,MU,MU,p1sq,s23,p4sq,p1p2+p1p
     -   3,p1p4,p2p4+p3p4,C0245R,C0145R,C0125R,C0124R,Cij245R,Cij145R
     -   ,Cij125R,Cij124R,C0245I,C0145I,C0125I,C0124I,Cij245I,Cij145I
     -   ,Cij125I,Cij124I,D01245,D01245R,D01245I,Dij1245R,Dij1245I)
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
       P(1) = 4*MU2SQ-2*s15
       FR(1) = (INVs23*(2*B012R-B025R+C0125R*P(1)))/2.
       FI(1) = (INVs23*(2*B012I-B025I+C0125I*P(1)))/2.
       F(1)=DCMPLX(FR(1),FI(1))
       FR(2) = -(INVs23*(Cij124R(4,2)+Cij145R(4,2)-Cij245R(4,2)+4*MU2S
     -   Q*Dij1245R(7,2)-2*s15*Dij1245R(7,2)))
       FI(2) = -(INVs23*(Cij124I(4,2)+Cij145I(4,2)-Cij245I(4,2)+4*MU2S
     -   Q*Dij1245I(7,2)-2*s15*Dij1245I(7,2)))
       F(2)=DCMPLX(FR(2),FI(2))
       P(2) = 3*MU2SQ+s45
       P(3) = 3*MU2SQ-2*s23+s45
       P(4) = MU2SQ+s45
       P(5) = 2*MU2SQ-s15
       FR(3) = -(INVs23*(Cij124R(2,1)+Cij124R(3,2)+Cij145R(1,1)+Cij145
     -   R(1,2)-Cij145R(2,1)+Cij145R(2,2)-Cij245R(2,2)+Cij245R(3,2)-2
     -   *(Cij145R(3,2)-(Dij1245R(2,1)-Dij1245R(3,1)+Dij1245R(3,2)+Di
     -   j1245R(4,2)-Dij1245R(5,2)-Dij1245R(6,2))*P(5))))
       FI(3) = -(INVs23*(Cij124I(2,1)+Cij124I(3,2)+Cij145I(1,1)+Cij145
     -   I(1,2)-Cij145I(2,1)+Cij145I(2,2)-Cij245I(2,2)+Cij245I(3,2)-2
     -   *(Cij145I(3,2)-(Dij1245I(2,1)-Dij1245I(3,1)+Dij1245I(3,2)+Di
     -   j1245I(4,2)-Dij1245I(5,2)-Dij1245I(6,2))*P(5))))
       F(3)=DCMPLX(FR(3),FI(3))
       FR(4) = INVs23*(C0124R+C0145R+Cij124R(1,1)+Cij145R(1,1)-Cij145R
     -   (2,2)+Cij145R(3,2)+Cij245R(2,1)+Cij245R(2,2)+2*(D01245R+Dij1
     -   245R(1,1)-Dij1245R(3,2)+Dij1245R(5,2))*P(5))
       FI(4) = INVs23*(C0124I+C0145I+Cij124I(1,1)+Cij145I(1,1)-Cij145I
     -   (2,2)+Cij145I(3,2)+Cij245I(2,1)+Cij245I(2,2)+2*(D01245I+Dij1
     -   245I(1,1)-Dij1245I(3,2)+Dij1245I(5,2))*P(5))
       F(4)=DCMPLX(FR(4),FI(4))
       FR(5) = -(INVs23*(Cij124R(1,1)+Cij124R(1,2)+Cij145R(1,1)+Cij145
     -   R(1,2)-Cij145R(2,1)+Cij145R(2,2)-Cij245R(2,1)-Cij245R(2,2)-2
     -   *(Cij145R(3,2)-(Dij1245R(1,1)+Dij1245R(1,2)-Dij1245R(3,1)+Di
     -   j1245R(3,2)-2*Dij1245R(5,2))*P(5))))
       FI(5) = -(INVs23*(Cij124I(1,1)+Cij124I(1,2)+Cij145I(1,1)+Cij145
     -   I(1,2)-Cij145I(2,1)+Cij145I(2,2)-Cij245I(2,1)-Cij245I(2,2)-2
     -   *(Cij145I(3,2)-(Dij1245I(1,1)+Dij1245I(1,2)-Dij1245I(3,1)+Di
     -   j1245I(3,2)-2*Dij1245I(5,2))*P(5))))
       F(5)=DCMPLX(FR(5),FI(5))
       FR(6) = INVs23*(-Cij145R(2,2)+Cij145R(3,2)+Cij245R(2,2)-Cij245R
     -   (3,2)+2*(-Dij1245R(3,2)+Dij1245R(6,2))*P(5))
       FI(6) = INVs23*(-Cij145I(2,2)+Cij145I(3,2)+Cij245I(2,2)-Cij245I
     -   (3,2)+2*(-Dij1245I(3,2)+Dij1245I(6,2))*P(5))
       F(6)=DCMPLX(FR(6),FI(6))
       FR(7) = INVs23*(-Cij145R(2,2)+Cij145R(3,2)+Cij245R(2,1)+Cij245R
     -   (2,2)+2*(-Dij1245R(3,2)+Dij1245R(5,2))*P(5))
       FI(7) = INVs23*(-Cij145I(2,2)+Cij145I(3,2)+Cij245I(2,1)+Cij245I
     -   (2,2)+2*(-Dij1245I(3,2)+Dij1245I(5,2))*P(5))
       F(7)=DCMPLX(FR(7),FI(7))
       FR(8) = INVs23*(-Cij145R(2,1)-Cij145R(2,2)+Cij245R(2,1)+Cij245R
     -   (2,2)-2*(Dij1245R(3,1)+Dij1245R(3,2))*P(5))
       FI(8) = INVs23*(-Cij145I(2,1)-Cij145I(2,2)+Cij245I(2,1)+Cij245I
     -   (2,2)-2*(Dij1245I(3,1)+Dij1245I(3,2))*P(5))
       F(8)=DCMPLX(FR(8),FI(8))
       FR(9) = Cij145R(4,2)
       FI(9) = Cij145I(4,2)
       F(9)=DCMPLX(FR(9),FI(9))
       FR(10) = Cij145R(1,2)+Cij145R(2,2)-2*Cij145R(3,2)
       FI(10) = Cij145I(1,2)+Cij145I(2,2)-2*Cij145I(3,2)
       F(10)=DCMPLX(FR(10),FI(10))
       FR(11) = INVs23*(-Cij145R(1,1)+Cij145R(2,1)+Cij145R(2,2)-Cij145
     -   R(3,2))
       FI(11) = INVs23*(-Cij145I(1,1)+Cij145I(2,1)+Cij145I(2,2)-Cij145
     -   I(3,2))
       F(11)=DCMPLX(FR(11),FI(11))
       FR(12) = INVs23*(-2*(Cij145R(1,1)-Cij145R(2,1))+Cij145R(2,2)-Ci
     -   j145R(3,2))
       FI(12) = INVs23*(-2*(Cij145I(1,1)-Cij145I(2,1))+Cij145I(2,2)-Ci
     -   j145I(3,2))
       F(12)=DCMPLX(FR(12),FI(12))
       FR(13) = INVs23*(2*C0145R+3*Cij145R(2,1)+Cij145R(2,2))
       FI(13) = INVs23*(2*C0145I+3*Cij145I(2,1)+Cij145I(2,2))
       F(13)=DCMPLX(FR(13),FI(13))
       FR(14) = -(INVs23*INVs45MU*(B045R+Bij14R+Bij45R+(Cij1
     -   45R(1,1)-Cij145R(2,1))*P(2)))
       FI(14) = -(INVs23*INVs45MU*(B045I+Bij14I+Bij45I+(Cij1
     -   45I(1,1)-Cij145I(2,1))*P(2)))
       F(14)=DCMPLX(FR(14),FI(14))
       FR(15) = INVs23*INVs45MU*(B012R+B014R-B045R+Bij12R-Bij45R
     -   +(C0145R+Cij145R(2,1))*P(2))
       FI(15) = INVs23*INVs45MU*(B012I+B014I-B045I+Bij12I-Bij45I
     -   +(C0145I+Cij145I(2,1))*P(2))
       F(15)=DCMPLX(FR(15),FI(15))
       FR(16) = -(INVs23*INVs45MU*(2*B012R+Bij12R))
       FI(16) = -(INVs23*INVs45MU*(2*B012I+Bij12I))
       F(16)=DCMPLX(FR(16),FI(16))
       FR(17) = Cij124R(4,2)
       FI(17) = Cij124I(4,2)
       F(17)=DCMPLX(FR(17),FI(17))
       FR(18) = INVs23*(2*C0124R+3*Cij124R(1,1)+Cij124R(1,2))
       FI(18) = INVs23*(2*C0124I+3*Cij124I(1,1)+Cij124I(1,2))
       F(18)=DCMPLX(FR(18),FI(18))
       FR(19) = INVs23*(Cij124R(2,1)+Cij124R(3,2))
       FI(19) = INVs23*(Cij124I(2,1)+Cij124I(3,2))
       F(19)=DCMPLX(FR(19),FI(19))
       FR(20) = INVs23*INVs45MU*(B012R+B014R+Bij12R+Bij14R+(
     -   C0124R+Cij124R(1,1))*P(3))
       FI(20) = INVs23*INVs45MU*(B012I+B014I+Bij12I+Bij14I+(
     -   C0124I+Cij124I(1,1))*P(3))
       F(20)=DCMPLX(FR(20),FI(20))
       FR(21) = B014R
       FI(21) = B014I
       F(21)=DCMPLX(FR(21),FI(21))
       FR(22) = -(INVs23*INVs45MU*(2*B014R+Bij14R))
       FI(22) = -(INVs23*INVs45MU*(2*B014I+Bij14I))
       F(22)=DCMPLX(FR(22),FI(22))
       FR(23) = INVs23*INVs45MU**2*(-A02R+2*(A01R+B014R*P(4)))
       FI(23) = INVs23*INVs45MU**2*(-A02I+2*(A01I+B014I*P(4)))
       F(23)=DCMPLX(FR(23),FI(23))
       FR(24) = INVs23*INVs45MU*(B014R+Bij14R)
       FI(24) = INVs23*INVs45MU*(B014I+Bij14I)
       F(24)=DCMPLX(FR(24),FI(24))
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


       hel(1) = (2*(Fs*Invs23*s15*(MESQ-s34)*(MESQ+s15-s23-s34)-Fq*(s1
     -   2*s15+s15**2-s12*s23-s15*s23-s15*s34+s23*s34+MU2SQ*(-2*s15+2
     -   *s23+s34)-s34*s45+MESQ*(-MU2SQ+s45))))/(s15*(MESQ-s34)*(MESQ
     -   +s15-s23-s34))
       hel(2) = (2*Fq*(s15-s23))/(s15*(MESQ-s34)*(MESQ+s15-s23-s34))
       hel(3) = 4*(Fs*Invs23*INVs45MU-Fq/(s15*(MESQ+s15-s23-s34)))
       hel(4) = (4*Fq)/(s15*(MESQ+s15-s23-s34))+(4*Fs*Invs23)/(MU2SQ-
     -   s15+s23-s45)

       ssa(1) = -2*s12**2-s12*s15-s12*s23+2*s12*s34+s23*s34+MU2SQ*(2*s
     -   12-s23-s34-2*s45)+(2*s12+s15-s34)*s45-MESQ*(MU2SQ-2*s12-s15+
     -   2*s34+s45)
       ssa(2) = (s12+s15-s34)*(s12-s34-s45)+MU2SQ*(-s12+s23+s34+s45)
       ssa(3) = 2*MU2SQ-s15
       ssa(4) = 2*s12**2+s12*s15+s12*s23-2*s12*s34-s23*s34-2*s12*s45-s
     -   15*s45+s34*s45+MESQ*(MU2SQ-2*s12-s15+2*s34+s45)+MU2SQ*(-2*s1
     -   2+s23+s34+2*s45)
       ssa(5) = -2*s15*s23-s12*(s15+s23)+s23*s34+MU2SQ*(3*s23+s34)+s15
     -   *s45-s34*s45+MESQ*(3*MU2SQ-s15+s45)
       ssa(6) = ME4SQ*MU2SQ+MU4SQ*(2*s23+s34)+s15*(s12+s15-s34)*(s12+s
     -   23-s45)+MESQ*(MU4SQ-s15*(s12+s15-s34)+MU2SQ*(3*s15-s23-2*s34
     -   +s45))-MU2SQ*(s12*(2*s15+s23)+s15*(3*s23+s34-2*s45)+s34*(-2*
     -   s23-s34+s45))
       ssa(7) = -2*ME4SQ*MU2SQ+MESQ*(MU2SQ*(-3*s15+2*s23+4*s34)+s15*(2
     -   *s12+s15-2*s34-s45))+2*MU4SQ*(-s12+s23+s34+s45)+s15*(-2*s12*
     -   *2+s23*s34+(s15-s34)*s45-s12*(s15+s23-2*(s34+s45)))+MU2SQ*(2
     -   *s12**2+2*s34*(-s23+s45)-s15*(s23+s34+4*s45)+s12*(4*s15-2*(2
     -   *s34+s45)))
       ssa(8) = 2*MESQ+s23
       ssa(9) = (MESQ-s12)*(MESQ+MU2SQ-s12-s23)+(MESQ+MU2SQ-s12)*s45
       ssav(1) = 2*MESQ+s23
       ssav(2) = -2*s15*s23-s12*(s15+s23)+s23*s34+MU2SQ*(3*s23+s34)+s1
     -   5*s45-s34*s45+MESQ*(3*MU2SQ-s15+s45)
       ssav(3) = 2*s12**2+s12*s15+s12*s23-2*s12*s34-s23*s34-2*s12*s45-
     -   s15*s45+s34*s45+MESQ*(MU2SQ-2*s12-s15+2*s34+s45)+MU2SQ*(-2*s
     -   12+s23+s34+2*s45)
       ssav(4) = -2*s12**2-s12*s15-s12*s23+2*s12*s34+s23*s34+MU2SQ*(2*
     -   s12-s23-s34-2*s45)+(2*s12+s15-s34)*s45-MESQ*(MU2SQ-2*s12-s15
     -   +2*s34+s45)
       ssav(5) = -2*ME4SQ*MU2SQ+MESQ*(MU2SQ*(-3*s15+2*s23+4*s34)+s15*(
     -   2*s12+s15-2*s34-s45))+2*MU4SQ*(-s12+s23+s34+s45)+s15*(-2*s12
     -   **2+s23*s34+(s15-s34)*s45-s12*(s15+s23-2*(s34+s45)))+MU2SQ*(
     -   2*s12**2+2*s34*(-s23+s45)-s15*(s23+s34+4*s45)+s12*(4*s15-2*(
     -   2*s34+s45)))
       ssav(6) = (MESQ-s12)*(MESQ+MU2SQ-s12-s23)+(MESQ+MU2SQ-s12)*s45
       ssav(7) = 2*MU2SQ-s15
       ssav(8) = INVs23*INVs45MU*(2*s12**2+s12*s15+s12*s23-2*s12*s34-s
     -   23*s34-2*s12*s45-s15*s45+s34*s45+MESQ*(MU2SQ-2*s12-s15+2*s34
     -   +s45)+MU2SQ*(-2*s12+s23+s34+2*s45))*CONJG(F(21))+(2*(MESQ-s1
     -   2)*(MESQ+MU2SQ-s12-s23)+2*(MESQ+MU2SQ-s12)*s45)*CONJG(F(24))
       ssav(9) = INVs23*INVs45MU*(2*ME4SQ*MU2SQ+2*MU4SQ*(s12-s23-s34-s
     -   45)+MESQ*(MU2SQ*(3*s15-2*(s23+2*s34))+s15*(-2*s12-s15+2*s34+
     -   s45))+MU2SQ*(-2*s12**2+2*s34*(s23-s45)+2*s12*(-2*s15+2*s34+s
     -   45)+s15*(s23+s34+4*s45))+s15*(2*s12**2-s23*s34+(-s15+s34)*s4
     -   5+s12*(s15+s23-2*(s34+s45))))*CONJG(F(21))+(-((MESQ-s12)*(2*
     -   MU4SQ+MESQ*(MU2SQ-s15)+s12*s15+s12*s23+2*s15*s23-s23*s34+MU2
     -   SQ*(-2*s12-2*s15-3*s23+s34)))+(ME4SQ-2*MU4SQ+2*MU2SQ*(s12+s1
     -   5-s34)+s12*(-s15+s34)-MESQ*(s12-s15+s34))*s45)*CONJG(F(24))
       ssav(10) = ((MESQ-s12)*(MESQ+MU2SQ-s12-s23)+(MESQ+MU2SQ-s12)*s4
     -   5)*(-2*INVs23*INVs45MU*MU2SQ*CONJG(F(21))-(MU2SQ+s45)*CONJG(
     -   F(24)))
       ssav(11) = ((2*s12**2+s12*s15+s12*s23-2*s12*s34-s23*s34-2*s12*s
     -   45-s15*s45+s34*s45+MESQ*(MU2SQ-2*s12-s15+2*s34+s45)+MU2SQ*(-
     -   2*s12+s23+s34+2*s45))*(INVs23*INVs45MU*(-2*MU2SQ+s15)*CONJG(
     -   F(21))-(MU2SQ-s23+s45)*CONJG(F(24))))/2d0
       ssav(12) = (2*(MESQ-s12)*(MESQ+MU2SQ-s12-s23)+2*(MESQ+MU2SQ-s12
     -   )*s45)*CONJG(F(14))+(-2*s12**2-s12*s15-s12*s23+2*s12*s34+s23
     -   *s34+MU2SQ*(2*s12-s23-s34-2*s45)+(2*s12+s15-s34)*s45-MESQ*(M
     -   U2SQ-2*s12-s15+2*s34+s45))*CONJG(F(15))
       ssav(13) = (-((MESQ-s12)*(2*MU4SQ+MESQ*(MU2SQ-s15)+s12*s15+s12*
     -   s23+2*s15*s23-s23*s34+MU2SQ*(-2*s12-2*s15-3*s23+s34)))+(ME4S
     -   Q-2*MU4SQ+2*MU2SQ*(s12+s15-s34)+s12*(-s15+s34)-MESQ*(s12-s15
     -   +s34))*s45)*CONJG(F(14))+(-2*ME4SQ*MU2SQ+MESQ*(MU2SQ*(-3*s15
     -   +2*s23+4*s34)+s15*(2*s12+s15-2*s34-s45))+2*MU4SQ*(-s12+s23+s
     -   34+s45)+s15*(-2*s12**2+s23*s34+(s15-s34)*s45-s12*(s15+s23-2*
     -   (s34+s45)))+MU2SQ*(2*s12**2+2*s34*(-s23+s45)-s15*(s23+s34+4*
     -   s45)+s12*(4*s15-2*(2*s34+s45))))*CONJG(F(15))
       ssav(14) = ((MESQ-s12)*(MESQ+MU2SQ-s12-s23)+(MESQ+MU2SQ-s12)*s4
     -   5)*(-((MU2SQ+s45)*CONJG(F(14)))+2*MU2SQ*CONJG(F(15)))
       ssav(15) = ((2*s12**2+s12*s15+s12*s23-2*s12*s34-s23*s34-2*s12*s
     -   45-s15*s45+s34*s45+MESQ*(MU2SQ-2*s12-s15+2*s34+s45)+MU2SQ*(-
     -   2*s12+s23+s34+2*s45))*(-((MU2SQ-s23+s45)*CONJG(F(14)))+(2*MU
     -   2SQ-s15)*CONJG(F(15))))/2d0
       ssav(16) = 4*(2*MESQ+s23)*CONJG(F(2))+(-2*s12**2-s12*s15-s12*s2
     -   3+2*s12*s34+s23*s34+MU2SQ*(2*s12-s23-s34-2*s45)+(2*s12+s15-s
     -   34)*s45-MESQ*(MU2SQ-2*s12-s15+2*s34+s45))*CONJG(F(4))+(2*(ME
     -   SQ-s12)*(MESQ+MU2SQ-s12-s23)+2*(MESQ+MU2SQ-s12)*s45)*CONJG(F
     -   (5))+(-2*s12**2-s12*s15-s12*s23+2*s12*s34+s23*s34+MU2SQ*(2*s
     -   12-s23-s34-2*s45)+(2*s12+s15-s34)*s45-MESQ*(MU2SQ-2*s12-s15+
     -   2*s34+s45))*CONJG(F(7))+2*((s12+s15-s34)*(s12-s34-s45)+MU2SQ
     -   *(-s12+s23+s34+s45))*CONJG(F(8))
       ssav(17) = -2*(-2*s15*s23-s12*(s15+s23)+s23*s34+MU2SQ*(3*s23+s3
     -   4)+s15*s45-s34*s45+MESQ*(3*MU2SQ-s15+s45))*CONJG(F(2))+(MU4S
     -   Q*(s23+s34)+ME4SQ*(-MU2SQ+s15+s45)+s12*(2*s15*s23+s12*(s15+s
     -   23)-s23*s34-s15*s45+s34*s45)-MU2SQ*(s12*(s15+2*s23+s34)-s15*
     -   s45+s34*(s23+s45))+MESQ*(-MU4SQ-2*s15*s23+s23*s34+s15*s45-s3
     -   4*s45-s12*(2*s15+s23+s45)+MU2SQ*(s12+s15+3*s23+s34+s45)))*CO
     -   NJG(F(3))+(-2*ME4SQ*MU2SQ+MESQ*(MU2SQ*(-3*s15+2*s23+4*s34)+s
     -   15*(2*s12+s15-2*s34-s45))+2*MU4SQ*(-s12+s23+s34+s45)+s15*(-2
     -   *s12**2+s23*s34+(s15-s34)*s45-s12*(s15+s23-2*(s34+s45)))+MU2
     -   SQ*(2*s12**2+2*s34*(-s23+s45)-s15*(s23+s34+4*s45)+s12*(4*s15
     -   -2*(2*s34+s45))))*CONJG(F(4))-MU2SQ*(2*s12**2+s12*s15+s12*s2
     -   3-2*s12*s34-s23*s34-2*s12*s45-s15*s45+s34*s45+MESQ*(MU2SQ-2*
     -   s12-s15+2*s34+s45)+MU2SQ*(-2*s12+s23+s34+2*s45))*CONJG(F(5))
     -   +(-(MU4SQ*(s23+s34))-(s12+s15-s34)*(s12*(s15+s23)-s23*s34+(-
     -   s15+s34)*s45)+MU2SQ*(s12*(s15+2*s23+s34)+s34*(-s34+s45)-s15*
     -   (s23-s34+s45))+MESQ*(MU4SQ+(s12+s15-s34)*(s15+s45)-MU2SQ*(s1
     -   2+2*s15+2*s23-s34+s45)))*CONJG(F(6))+2*MU2SQ*((s12+s15-s34)*
     -   (s12-s34-s45)+MU2SQ*(-s12+s23+s34+s45))*CONJG(F(7))+(MU4SQ*(
     -   2*s12-3*(s23+s34)-2*s45)+2*s15*(s12+s15-s34)*(s12-s34-s45)+M
     -   ESQ*MU2SQ*(MU2SQ-2*s12-s15+2*s34+s45)+MU2SQ*(-2*s12**2+s12*(
     -   -5*s15+s23+6*s34+2*s45)-s34*(s23+4*s34+3*s45)+s15*(2*s23+6*s
     -   34+5*s45)))*CONJG(F(8))
       ssav(18) = (-2*s12**2-s12*s15-s12*s23+2*s12*s34+s23*s34+MU2SQ*(
     -   2*s12-s23-s34-2*s45)+(2*s12+s15-s34)*s45-MESQ*(MU2SQ-2*s12-s
     -   15+2*s34+s45))*CONJG(F(2))+(MU2SQ-s15-s45)*((MESQ-s12)*(MESQ
     -   +MU2SQ-s12-s23)+(MESQ+MU2SQ-s12)*s45)*CONJG(F(3))+2*MU2SQ*((
     -   MESQ-s12)*(MESQ+MU2SQ-s12-s23)+(MESQ+MU2SQ-s12)*s45)*CONJG(F
     -   (4))-(2*MU2SQ-s15)*((MESQ-s12)*(MESQ+MU2SQ-s12-s23)+(MESQ+MU
     -   2SQ-s12)*s45)*CONJG(F(5))-((MU2SQ-s15-s45)*(2*s12**2+s12*s15
     -   +s12*s23-2*s12*s34-s23*s34-2*s12*s45-s15*s45+s34*s45+MESQ*(M
     -   U2SQ-2*s12-s15+2*s34+s45)+MU2SQ*(-2*s12+s23+s34+2*s45))*CONJ
     -   G(F(6)))/2d0+((2*MU2SQ-s15)*(2*s12**2+s12*s15+s12*s23-2*s12*
     -   s34-s23*s34-2*s12*s45-s15*s45+s34*s45+MESQ*(MU2SQ-2*s12-s15+
     -   2*s34+s45)+MU2SQ*(-2*s12+s23+s34+2*s45))*CONJG(F(7)))/2d0-MU
     -   2SQ*(2*s12**2+s12*s15+s12*s23-2*s12*s34-s23*s34-2*s12*s45-s1
     -   5*s45+s34*s45+MESQ*(MU2SQ-2*s12-s15+2*s34+s45)+MU2SQ*(-2*s12
     -   +s23+s34+2*s45))*CONJG(F(8))
       ssav(19) = (-2*s12**2-s12*s15-s12*s23+2*s12*s34+s23*s34+MU2SQ*(
     -   2*s12-s23-s34-2*s45)+(2*s12+s15-s34)*s45-MESQ*(MU2SQ-2*s12-s
     -   15+2*s34+s45))*CONJG(F(2))+((MU2SQ+s23-s45)*(2*s12**2+s12*s1
     -   5+s12*s23-2*s12*s34-s23*s34-2*s12*s45-s15*s45+s34*s45+MESQ*(
     -   MU2SQ-2*s12-s15+2*s34+s45)+MU2SQ*(-2*s12+s23+s34+2*s45))*CON
     -   JG(F(3)))/2d0+((2*MU2SQ-s15)*(2*s12**2+s12*s15+s12*s23-2*s12
     -   *s34-s23*s34-2*s12*s45-s15*s45+s34*s45+MESQ*(MU2SQ-2*s12-s15
     -   +2*s34+s45)+MU2SQ*(-2*s12+s23+s34+2*s45))*CONJG(F(4)))/2d0-M
     -   U2SQ*(2*s12**2+s12*s15+s12*s23-2*s12*s34-s23*s34-2*s12*s45-s
     -   15*s45+s34*s45+MESQ*(MU2SQ-2*s12-s15+2*s34+s45)+MU2SQ*(-2*s1
     -   2+s23+s34+2*s45))*CONJG(F(5))-(MU2SQ+s23-s45)*((s12+s15-s34)
     -   *(s12-s34-s45)+MU2SQ*(-s12+s23+s34+s45))*CONJG(F(6))+2*MU2SQ
     -   *((s12+s15-s34)*(s12-s34-s45)+MU2SQ*(-s12+s23+s34+s45))*CONJ
     -   G(F(7))+(2*MU2SQ-s15)*(-((s12+s15-s34)*(s12-s34-s45))+MU2SQ*
     -   (s12-s23-s34-s45))*CONJG(F(8))
       ssav(20) = 4*INVs23*(2*MESQ+s23)*CONJG(F(17))+(2*(MESQ-s12)*(ME
     -   SQ+MU2SQ-s12-s23)+2*(MESQ+MU2SQ-s12)*s45)*CONJG(F(18))
       ssav(21) = -2*INVs23*(-2*s15*s23-s12*(s15+s23)+s23*s34+MU2SQ*(3
     -   *s23+s34)+s15*s45-s34*s45+MESQ*(3*MU2SQ-s15+s45))*CONJG(F(17
     -   ))-MU2SQ*(2*s12**2+s12*s15+s12*s23-2*s12*s34-s23*s34-2*s12*s
     -   45-s15*s45+s34*s45+MESQ*(MU2SQ-2*s12-s15+2*s34+s45)+MU2SQ*(-
     -   2*s12+s23+s34+2*s45))*CONJG(F(18))+(MU4SQ*(s23+s34)+ME4SQ*(-
     -   MU2SQ+s15+s45)+s12*(2*s15*s23+s12*(s15+s23)-s23*s34-s15*s45+
     -   s34*s45)-MU2SQ*(s12*(s15+2*s23+s34)-s15*s45+s34*(s23+s45))+M
     -   ESQ*(-MU4SQ-2*s15*s23+s23*s34+s15*s45-s34*s45-s12*(2*s15+s23
     -   +s45)+MU2SQ*(s12+s15+3*s23+s34+s45)))*CONJG(F(19))
       ssav(22) = -(INVs23*(2*s12**2+s12*s15+s12*s23-2*s12*s34-s23*s34
     -   -2*s12*s45-s15*s45+s34*s45+MESQ*(MU2SQ-2*s12-s15+2*s34+s45)+
     -   MU2SQ*(-2*s12+s23+s34+2*s45))*CONJG(F(17)))-(2*MU2SQ-s15)*((
     -   MESQ-s12)*(MESQ+MU2SQ-s12-s23)+(MESQ+MU2SQ-s12)*s45)*CONJG(F
     -   (18))+(MU2SQ-s15-s45)*((MESQ-s12)*(MESQ+MU2SQ-s12-s23)+(MESQ
     -   +MU2SQ-s12)*s45)*CONJG(F(19))
       ssav(23) = ((2*s12**2+s12*s15+s12*s23-2*s12*s34-s23*s34-2*s12*s
     -   45-s15*s45+s34*s45+MESQ*(MU2SQ-2*s12-s15+2*s34+s45)+MU2SQ*(-
     -   2*s12+s23+s34+2*s45))*(-2*INVs23*CONJG(F(17))-2*MU2SQ*CONJG(
     -   F(18))+(MU2SQ+s23-s45)*CONJG(F(19))))/2d0
       ssav(24) = 4*INVs23*(2*MESQ+s23)*CONJG(F(9))+2*INVs23*((MESQ-s1
     -   2)*(MESQ+MU2SQ-s12-s23)+(MESQ+MU2SQ-s12)*s45)*CONJG(F(10))+(
     -   -2*s12**2-s12*s15-s12*s23+2*s12*s34+s23*s34+MU2SQ*(2*s12-s23
     -   -s34-2*s45)+(2*s12+s15-s34)*s45-MESQ*(MU2SQ-2*s12-s15+2*s34+
     -   s45))*CONJG(F(11))+(-2*s12**2-s12*s15-s12*s23+2*s12*s34+s23*
     -   s34+MU2SQ*(2*s12-s23-s34-2*s45)+(2*s12+s15-s34)*s45-MESQ*(MU
     -   2SQ-2*s12-s15+2*s34+s45))*CONJG(F(12))+2*((s12+s15-s34)*(s12
     -   -s34-s45)+MU2SQ*(-s12+s23+s34+s45))*CONJG(F(13))
       ssav(25) = -2*INVs23*(-2*s15*s23-s12*(s15+s23)+s23*s34+MU2SQ*(3
     -   *s23+s34)+s15*s45-s34*s45+MESQ*(3*MU2SQ-s15+s45))*CONJG(F(9)
     -   )+(-(INVs23*(MESQ-s12)*(2*MU4SQ+MESQ*(MU2SQ-s15)+s12*s15+s12
     -   *s23+2*s15*s23-s23*s34+MU2SQ*(-2*s12-2*s15-3*s23+s34)))+INVs
     -   23*(ME4SQ-2*MU4SQ+2*MU2SQ*(s12+s15-s34)+s12*(-s15+s34)-MESQ*
     -   (s12-s15+s34))*s45)*CONJG(F(10))+(-2*ME4SQ*MU2SQ+MESQ*(MU2SQ
     -   *(-3*s15+2*s23+4*s34)+s15*(2*s12+s15-2*s34-s45))+2*MU4SQ*(-s
     -   12+s23+s34+s45)+s15*(-2*s12**2+s23*s34+(s15-s34)*s45-s12*(s1
     -   5+s23-2*(s34+s45)))+MU2SQ*(2*s12**2+2*s34*(-s23+s45)-s15*(s2
     -   3+s34+4*s45)+s12*(4*s15-2*(2*s34+s45))))*CONJG(F(11))+(MU4SQ
     -   *(-2*s12+s23+s34+2*s45)-(s12+s15-s34)*(s12*(s15+s23)-s23*s34
     -   +(-s15+s34)*s45)+MESQ*(MU4SQ+(s12+s15-s34)*(s15+s45)-MU2SQ*(
     -   s12+2*s15+2*s23-s34+s45))+MU2SQ*(2*s12**2+s12*(3*s15+2*s23-3
     -   *s34-2*s45)+s34*(s34+3*s45)-s15*(s23+s34+3*s45)))*CONJG(F(12
     -   ))+(MU4SQ*(2*s12-3*(s23+s34)-2*s45)+2*s15*(s12+s15-s34)*(s12
     -   -s34-s45)+MESQ*MU2SQ*(MU2SQ-2*s12-s15+2*s34+s45)+MU2SQ*(-2*s
     -   12**2+s12*(-5*s15+s23+6*s34+2*s45)-s34*(s23+4*s34+3*s45)+s15
     -   *(2*s23+6*s34+5*s45)))*CONJG(F(13))
       ssav(26) = -(INVs23*(2*s12**2+s12*s15+s12*s23-2*s12*s34-s23*s34
     -   -2*s12*s45-s15*s45+s34*s45+MESQ*(MU2SQ-2*s12-s15+2*s34+s45)+
     -   MU2SQ*(-2*s12+s23+s34+2*s45))*CONJG(F(9)))-INVs23*(MU2SQ+s45
     -   )*((MESQ-s12)*(MESQ+MU2SQ-s12-s23)+(MESQ+MU2SQ-s12)*s45)*CON
     -   JG(F(10))+2*MU2SQ*((MESQ-s12)*(MESQ+MU2SQ-s12-s23)+(MESQ+MU2
     -   SQ-s12)*s45)*CONJG(F(11))+((MU2SQ+s45)*(2*s12**2+s12*s15+s12
     -   *s23-2*s12*s34-s23*s34-2*s12*s45-s15*s45+s34*s45+MESQ*(MU2SQ
     -   -2*s12-s15+2*s34+s45)+MU2SQ*(-2*s12+s23+s34+2*s45))*CONJG(F(
     -   12)))/2d0-MU2SQ*(2*s12**2+s12*s15+s12*s23-2*s12*s34-s23*s34-
     -   2*s12*s45-s15*s45+s34*s45+MESQ*(MU2SQ-2*s12-s15+2*s34+s45)+M
     -   U2SQ*(-2*s12+s23+s34+2*s45))*CONJG(F(13))
       ssav(27) = -(INVs23*(2*s12**2+s12*s15+s12*s23-2*s12*s34-s23*s34
     -   -2*s12*s45-s15*s45+s34*s45+MESQ*(MU2SQ-2*s12-s15+2*s34+s45)+
     -   MU2SQ*(-2*s12+s23+s34+2*s45))*CONJG(F(9)))-(INVs23*(MU2SQ-s2
     -   3+s45)*(2*s12**2+s12*s15+s12*s23-2*s12*s34-s23*s34-2*s12*s45
     -   -s15*s45+s34*s45+MESQ*(MU2SQ-2*s12-s15+2*s34+s45)+MU2SQ*(-2*
     -   s12+s23+s34+2*s45))*CONJG(F(10)))/2d0+((2*MU2SQ-s15)*(2*s12*
     -   *2+s12*s15+s12*s23-2*s12*s34-s23*s34-2*s12*s45-s15*s45+s34*s
     -   45+MESQ*(MU2SQ-2*s12-s15+2*s34+s45)+MU2SQ*(-2*s12+s23+s34+2*
     -   s45))*CONJG(F(11)))/2d0+(MU2SQ-s23+s45)*((s12+s15-s34)*(s12-
     -   s34-s45)+MU2SQ*(-s12+s23+s34+s45))*CONJG(F(12))+(2*MU2SQ-s15
     -   )*(-((s12+s15-s34)*(s12-s34-s45))+MU2SQ*(s12-s23-s34-s45))*C
     -   ONJG(F(13))
       resultn(1) = 4*CONJG(F(1))*hel(1)*ssav(1)-2*CONJG(F(1))*hel(2)*
     -   ssav(2)+CONJG(F(1))*hel(3)*ssav(4)+CONJG(F(1))*hel(4)*ssav(4
     -   )
       resultn(2) = hel(1)*ssav(16)+hel(2)*ssav(17)+hel(3)*ssav(18)+he
     -   l(4)*ssav(19)
       resultn(3) = hel(1)*ssav(24)+hel(2)*ssav(25)+hel(3)*ssav(26)+he
     -   l(4)*ssav(27)
       resultn(4) = hel(1)*ssav(12)+hel(2)*ssav(13)+hel(3)*ssav(14)+he
     -   l(4)*ssav(15)
       resultn(5) = CONJG(F(16))*hel(1)*ssav(4)+CONJG(F(16))*hel(2)*ss
     -   av(5)+2*MU**2*CONJG(F(16))*hel(3)*ssav(6)+(CONJG(F(16))*hel(
     -   4)*ssav(3)*ssav(7))/2.
       resultn(6) = hel(1)*ssav(20)+hel(2)*ssav(21)+hel(3)*ssav(22)+he
     -   l(4)*ssav(23)
       resultn(7) = CONJG(F(20))*hel(1)*ssav(4)+CONJG(F(20))*hel(2)*ss
     -   av(5)+2*MU**2*CONJG(F(20))*hel(3)*ssav(6)+(CONJG(F(20))*hel(
     -   4)*ssav(3)*ssav(7))/2.
       resultn(8) = CONJG(F(16))*hel(1)*ssav(4)+CONJG(F(16))*hel(2)*ss
     -   av(5)+2*MU**2*CONJG(F(16))*hel(3)*ssav(6)+(CONJG(F(16))*hel(
     -   4)*ssav(3)*ssav(7))/2.
       resultn(9) = -4*INVs23*CONJG(F(21))*hel(1)*ssav(1)+2*INVs23*CON
     -   JG(F(21))*hel(2)*ssav(2)+INVs23*CONJG(F(21))*hel(3)*ssav(3)+
     -   INVs23*CONJG(F(21))*hel(4)*ssav(3)
       resultn(10) = CONJG(F(22))*hel(1)*ssav(4)+CONJG(F(22))*hel(2)*s
     -   sav(5)+2*MU**2*CONJG(F(22))*hel(3)*ssav(6)+(CONJG(F(22))*hel
     -   (4)*ssav(3)*ssav(7))/2.
       resultn(11) = CONJG(F(23))*hel(1)*ssav(4)+CONJG(F(23))*hel(2)*s
     -   sav(5)+2*MU**2*CONJG(F(23))*hel(3)*ssav(6)+(CONJG(F(23))*hel
     -   (4)*ssav(3)*ssav(7))/2.
       resultn(12) = hel(1)*ssav(8)+hel(2)*ssav(9)+hel(3)*ssav(10)+hel
     -   (4)*ssav(11)
       If(perm.eq.0) then

c perm=0: then 4-vert evaluated
       result = 4*CONJG(F(1))*hel(1)*ssav(1)-4*INVs23*CONJG(F(21))*hel
     -   (1)*ssav(1)-2*CONJG(F(1))*hel(2)*ssav(2)+2*INVs23*CONJG(F(21
     -   ))*hel(2)*ssav(2)+INVs23*CONJG(F(21))*hel(3)*ssav(3)+INVs23*
     -   CONJG(F(21))*hel(4)*ssav(3)+2*CONJG(F(16))*hel(1)*ssav(4)+CO
     -   NJG(F(20))*hel(1)*ssav(4)+CONJG(F(22))*hel(1)*ssav(4)+CONJG(
     -   F(23))*hel(1)*ssav(4)+CONJG(F(1))*hel(3)*ssav(4)+CONJG(F(1))
     -   *hel(4)*ssav(4)+2*CONJG(F(16))*hel(2)*ssav(5)+CONJG(F(20))*h
     -   el(2)*ssav(5)+CONJG(F(22))*hel(2)*ssav(5)+CONJG(F(23))*hel(2
     -   )*ssav(5)+4*MU**2*CONJG(F(16))*hel(3)*ssav(6)+2*MU**2*CONJG(
     -   F(20))*hel(3)*ssav(6)+2*MU**2*CONJG(F(22))*hel(3)*ssav(6)+2*
     -   MU**2*CONJG(F(23))*hel(3)*ssav(6)+CONJG(F(16))*hel(4)*ssav(3
     -   )*ssav(7)+(CONJG(F(20))*hel(4)*ssav(3)*ssav(7))/2.+(CONJG(F(
     -   22))*hel(4)*ssav(3)*ssav(7))/2.+(CONJG(F(23))*hel(4)*ssav(3)
     -   *ssav(7))/2.+hel(1)*ssav(8)+hel(2)*ssav(9)+hel(3)*ssav(10)+h
     -   el(4)*ssav(11)+hel(1)*ssav(12)+hel(2)*ssav(13)+hel(3)*ssav(1
     -   4)+hel(4)*ssav(15)+hel(1)*ssav(16)+hel(2)*ssav(17)+hel(3)*ss
     -   av(18)+hel(4)*ssav(19)+hel(1)*ssav(20)+hel(2)*ssav(21)+hel(3
     -   )*ssav(22)+hel(4)*ssav(23)+hel(1)*ssav(24)+hel(2)*ssav(25)+h
     -   el(3)*ssav(26)+hel(4)*ssav(27)
       else

c perm=!=0: 4-vert evaluated set to zero 

        result=resultn(2)+resultn(3)+resultn(4)+resultn(5)+
     -   resultn(6)+resultn(7)+resultn(8)+resultn(9)
     -   +resultn(10)+resultn(11)+resultn(12)
       endif

c The Finite virtual piece should be multiplied for (-1)  since 
c I have multiplied by (-I) to get the F's and k's without (I) factor
c . The factorization from the B_ij is Fact=(I/(4\[Pi])^2 (4 \[Pi])^Eps Gamma[1+Eps] (musq)^(-Eps))
c 
c So, I*I=(-1)!!!
       resultb = hel(4)*(CONJG(hel(1))*ssa(1)+2*MU**2*CONJG(hel(2))*ss
     -   a(2)+2*MU**2*CONJG(hel(4))*ssa(2)+(CONJG(hel(3))*ssa(3)*ssa(
     -   4))/2.)+hel(2)*(2*MU**2*CONJG(hel(4))*ssa(2)-2*CONJG(hel(1))
     -   *ssa(5)+4*CONJG(hel(2))*ssa(6)+CONJG(hel(3))*ssa(7))+hel(1)*
     -   (CONJG(hel(3))*ssa(1)+CONJG(hel(4))*ssa(1)-2*CONJG(hel(2))*s
     -   sa(5)+4*CONJG(hel(1))*ssa(8))+hel(3)*(CONJG(hel(1))*ssa(1)+(
     -   CONJG(hel(4))*ssa(3)*ssa(4))/2.+CONJG(hel(2))*ssa(7)+2*MU**2
     -   *CONJG(hel(3))*ssa(9))
c counter terms
       diag3=-(hel(1)*ssav(4)+hel(2)*ssav(5)+2*MU**2*hel(3)*ssav(6)
     - +(hel(4)*ssav(3)*ssav(7))/2.)
       C0count=qlI3(MU2SQ,0.d0,MU2SQ,0.d0,MU2SQ,MU2SQ,musq,0)-
     - qlI3(MU2SQ,0.d0,MU2SQ,0.d0,MU2SQ,MU2SQ,musq,-1)*Log(musq/s15)
       dB0=-1.d0/MU2SQ - 1.d0/MU2SQ*Log(s15/MU2SQ)/2.d0
       counter99=-CONJG(INVs23*INvs45MU**2*(A02+4.d0*MU2SQ*dB0* 
     - (MU2SQ-s45)-2.d0*B012*(MU2SQ+s45)))*diag3
       counter4=-2.d0*CONJG(INVs23*INVs45MU*(B045
     - -2.d0*MU2SQ*C0count))*diag3
       counter7=-2.d0*CONJG(INVs23*INVs45MU*(B045
     - -2.d0*MU2SQ*C0count))*diag3
       result_counter=counter4+counter7+counter99
c       print*, '4+7+99=',result_counter
       result=result+result_counter
c************************************************************************************
c************************************************************************************
c************************************************************************************
c************************************************************************************
c************************************************************************************
c************************************************************************************
c************************************************************************************
       Return
       End
