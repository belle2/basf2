c =================================================
c --- convert output to four vectors for basf2 ----
c =================================================
c
      subroutine belle2event()
      include 'phokhara_10.0.inc'
           
      integer ii
      integer BNPHOT,BNHAD
      double precision BP1(0:3),BQ1(0:3),BP2(0:9,0:5),BPHOT(0:1,0:3)
      COMMON / MOMSET / BP1,BQ1,BP2,BPHOT,BNPHOT,BNHAD
      
      BNPHOT = 0
      BNHAD = 0
      
!       electron (e-) (+z direction)
      do ii=0,3
        BP1(ii) = momenta(1,ii)
      enddo
!       positron (e+) (-z direction)
      do ii=0,3
        BQ1(ii) = momenta(2,ii)
      enddo      
      
!       photon 1
      if(momenta(3,0).gt.0.0)then !(ii,0) is energy, (ii,3) is pz!
        do ii=0,3
          BPHOT(0,ii) = momenta(3,ii)
        enddo
        BNPHOT = BNPHOT + 1
      endif
!       photon 2
      if(momenta(4,0).gt.0.0)then
        do ii=0,3
          BPHOT(1,ii) = momenta(4,ii)
        enddo
        BNPHOT = BNPHOT + 1
      endif
            
!       all the rest...
      if (pion.eq.0) then !mu+ mu-
        do ii=0,3
          BP2(0,ii) = momenta(6,ii)
          BP2(1,ii) = momenta(7,ii)
        enddo
        BP2(0,4) = -13 !mu+
        BP2(1,4) = 13  !mu-
        BP2(0,5) = -1
        BP2(1,5) = -1
        BNHAD = 2
        
      elseif(pion.eq.1)then
        do ii=0,3
          BP2(0,ii) = momenta(6,ii)
          BP2(1,ii) = momenta(7,ii)
        enddo
        BP2(0,4) = 211  !pi+
        BP2(1,4) = -211 !pi-
        BP2(0,5) = -1
        BP2(1,5) = -1
        BNHAD = 2
                
      elseif(pion.eq.2)then
        do ii=0,3
          BP2(0,ii) = momenta(6,ii)
          BP2(1,ii) = momenta(7,ii)
          BP2(2,ii) = momenta(8,ii)
          BP2(3,ii) = momenta(9,ii)
        enddo
        BP2(0,4) = 111  !pi0
        BP2(1,4) = 111  !pi0
        BP2(2,4) = -211 !pi-
        BP2(3,4) = 211  !pi+
        BP2(0,5) = -1
        BP2(1,5) = -1
        BP2(2,5) = -1
        BP2(3,5) = -1
        BNHAD = 4
        
      elseif(pion.eq.3)then
        do ii=0,3
          BP2(0,ii) = momenta(6,ii)
          BP2(1,ii) = momenta(7,ii)
          BP2(2,ii) = momenta(8,ii)
          BP2(3,ii) = momenta(9,ii)
        enddo
        BP2(0,4) = 211  !pi+
        BP2(1,4) = -211 !pi-
        BP2(2,4) = -211 !pi-
        BP2(3,4) = 211  !pi+
        BP2(0,5) = -1
        BP2(1,5) = -1
        BP2(2,5) = -1
        BP2(3,5) = -1
        BNHAD = 4

      elseif(pion.eq.4)then
        do ii=0,3
          BP2(0,ii) = momenta(6,ii)
          BP2(1,ii) = momenta(7,ii)
        enddo
        BP2(0,4) = -2212  !Pbar
        BP2(1,4) = 2212 !P
        BP2(0,5) = -1
        BP2(1,5) = -1
        BNHAD = 2
        
      elseif(pion.eq.5)then
        do ii=0,3
          BP2(0,ii) = momenta(6,ii)
          BP2(1,ii) = momenta(7,ii)
        enddo
        BP2(0,4) = -2112  !Nbar
        BP2(1,4) = 2112 !N
        BP2(0,5) = -1
        BP2(1,5) = -1
        BNHAD = 2
        
      elseif(pion.eq.6)then
        do ii=0,3
          BP2(0,ii) = momenta(6,ii)
          BP2(1,ii) = momenta(7,ii)
        enddo
        BP2(0,4) = 321  !K+
        BP2(1,4) = -321 !K-
        BP2(0,5) = -1
        BP2(1,5) = -1
        BNHAD = 2

      elseif(pion.eq.7)then
C       Kevin Varvell - Output KL and KS rather than K0 K0bar
        do ii=0,3
          BP2(0,ii) = momenta(6,ii)
          BP2(1,ii) = momenta(7,ii)
        enddo
        BP2(0,4) = 130  !KL
        BP2(1,4) = 310  !KS
        BP2(0,5) = -1
        BP2(1,5) = -1
        BNHAD = 2
        
      elseif(pion.eq.8)then
        do ii=0,3
          BP2(0,ii) = momenta(6,ii)
          BP2(1,ii) = momenta(7,ii)
          BP2(2,ii) = momenta(8,ii)
        enddo
        BP2(0,4) = 211  !Pi+
        BP2(1,4) = -211 !Pi-
        BP2(2,4) = 111 !Pi0
        BP2(0,5) = -1
        BP2(1,5) = -1
        BP2(2,5) = -1
        BNHAD = 3
        
      elseif(pion.eq.9)then
        do ii=0,3
          BP2(0,ii) = momenta(6,ii)
          BP2(1,ii) = momenta(7,ii)
          BP2(2,ii) = momenta(8,ii)
          BP2(3,ii) = momenta(9,ii)
          BP2(4,ii) = momenta(10,ii)
          BP2(5,ii) = momenta(11,ii)
        enddo
        BP2(0,4) = -3122  !Lambda bar
        BP2(1,4) = 3122 !Lambda
        BP2(2,4) = 211 !pi+
        BP2(3,4) = -2212 !Pbar
        BP2(4,4) = -211 !pi-
        BP2(5,4) = 2212 !P
        BP2(0,5) = -1
        BP2(1,5) = -1
        BP2(2,5) = 0
        BP2(3,5) = 0
        BP2(4,5) = 1
        BP2(5,5) = 1
        BNHAD = 6
        
      elseif(pion.eq.10)then
        do ii=0,3
          BP2(0,ii) = momenta(6,ii)
          BP2(1,ii) = momenta(7,ii)
          BP2(2,ii) = momenta(8,ii)
        enddo
        BP2(0,4) = 221  !eta
        BP2(1,4) = 211  !pi+
        BP2(2,4) = -211 !pi-
        BP2(0,5) = -1
        BP2(1,5) = -1
        BP2(2,5) = -1
        BNHAD = 3

      elseif(pion.eq.11)then
        do ii=0,3
          BP2(0,ii) = momenta(6,ii)
          BP2(1,ii) = momenta(7,ii)
          BP2(2,ii) = momenta(8,ii)
        enddo
        BP2(0,4) = -13  !mu+
        BP2(1,4) = 13  !mu-
        BP2(0,5) = -1
        BP2(1,5) = -1
        BNHAD = 2
        
      elseif(pion.eq.12)then
        do ii=0,3
          BP2(0,ii) = momenta(6,ii)
          BP2(1,ii) = momenta(7,ii)
          BP2(2,ii) = momenta(8,ii)
        enddo
        BP2(0,4) = -13  !mu+
        BP2(1,4) = 13  !mu-
        BP2(0,5) = -1
        BP2(1,5) = -1
        BNHAD = 2
        
      elseif(pion.eq.13)then
        do ii=0,3
          BP2(0,ii) = momenta(6,ii)
          BP2(1,ii) = momenta(7,ii)
          BP2(2,ii) = momenta(8,ii)
        enddo
        BP2(0,4) = 111  !pi0
        BP2(1,4) = 22  !gamma
        BP2(0,5) = -1
        BP2(1,5) = -1
        BNHAD = 2     
        
        
      elseif(pion.eq.14)then
        do ii=0,3
          BP2(0,ii) = momenta(6,ii)
          BP2(1,ii) = momenta(7,ii)
          BP2(2,ii) = momenta(8,ii)
        enddo
        BP2(0,4) = 221  !eta
        BP2(1,4) = 22  !gamma
        BP2(0,5) = -1
        BP2(1,5) = -1
        BNHAD = 2

        
      elseif(pion.eq.15)then
        do ii=0,3
          BP2(0,ii) = momenta(6,ii)
          BP2(1,ii) = momenta(7,ii)
          BP2(2,ii) = momenta(8,ii)
        enddo
        BP2(0,4) = 331  !etaP
        BP2(1,4) = 22  !gamma
        BP2(0,5) = -1
        BP2(1,5) = -1
        BNHAD = 2
      else
        continue
      endif
      
      return
      end
