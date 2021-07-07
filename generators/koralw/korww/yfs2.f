      SUBROUTINE gifyfs(svar,amel,fyfs)       
C     *********************************       
C YFS formfactor       
C     *********************************       
      IMPLICIT DOUBLE PRECISION (A-H,O-Z) 
      COMMON / MATPAR / PI,CEULER     
      COMMON / PHYPAR / ALFINV,GPICOB     
! This common can be everywhere, contains various switches
      COMMON / KeyKey/  KeyRad,KeyPhy,KeyTek,KeyMis,KeyDwm,KeyDwp
      SAVE

      KeyNLL = MOD(KeyRad,1000)/100

      alf1  =  1d0/alfinv/pi

      bilg  =  dlog(svar/amel**2)             
      beta  =  2*alf1*(bilg-1)
      IF(KeyNLL .EQ. 0) THEN
         delb  =  beta/4d0
      ELSEIF( KeyNLL .EQ. 1) THEN
         delb  =  beta/4d0 + alf1*( -.5d0  +pi**2/3d0)
      ELSE
         WRITE(6,*) '+++++ STOP in gifyfs, wrong KeyNLL= ',KeyNLL
      ENDIF
      fyfs  =  exp(delb)                  
      END              

      FUNCTION RHOSKO(R)                    
C     ********************                  
C CALLED IN VESK1W        
C PROVIDES V OR K DISTRIBUTION TO BE GENERATED                
C     ********************                  
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)   
      PARAMETER (FLEPS = 1D-35)
      COMMON / MATPAR / PI,CEULER     
      COMMON / PHYPAR / ALFINV,GPICOB     
      COMMON / WEKING / ENE,AMAZ,GAMMZ,AMEL,AMFIN,XK0,SINW2,IDE,IDF
      COMMON / WEKIN2 / AMAW,GAMMW,GMU,ALPHAW   
      COMMON / VVREC  / VVMIN,VVMAX,VV,BETI                   
      SAVE
C MAPPING  R => VV CHANGE  TO IMPROVE ON EFFICIENCY
C Note that the replacement below makes program more precise
C and bulet-proof with respect numerical instabilities close to VV=0    
      ALF1   = 1D0/PI/ALFINV
      SVAR   = 4D0*ENE**2
      BILG   = DLOG(SVAR/AMEL**2)           
      BETI   = 2D0*ALF1*(BILG-1D0)          
      X = MAX(R,FLEPS**BETI)                
      BBT = -0.5D0 
cc      write(6,*) amaw,gammw
      CALL CHBIN1(X,BETI,BBT,VVMAX,VV,RJAC)               
C BORN XSECTION           
      SVAR1  = SVAR*(1D0-VV)                 
c ms 11/17/97
      xcrude=get_total_crude(svar1)
c ms      xcrude1=tot_born_crud(svar,svar1)
c ms      IF( abs(xcrude1/xcrude -1) .GT. 0.2d0)
c ms     $ WRITE(6,*)'1 print', xcrude1/xcrude

      DILAT=1D0           
      IF(VV.GT.VVMIN) DILAT=(1D0+1D0/SQRT(1D0-VV))/2D0        
      BETI2  = 2D0*ALF1*BILG                
      DAMEL=1D0           
      IF(VV.GT.VVMIN) DAMEL=BETI2/BETI*(VV/VVMIN)**(BETI2-BETI)
      DISTR= BETI*VV**(BETI-1D0)*DILAT*DAMEL       
      RHOSKO = RJAC*xcrude*DISTR
c      RHOSKO = RJAC*VVRHO(1,SVAR,AMEL,VV,VVMIN)
      END                 

      SUBROUTINE YFSGEN(VV,VMIN,NMAX,WT1,WT2,WT3)               
C     *******************************************               
C======================================================================
C================== Y F S G E N =======================================
C======================================================================
C*********INPUT                 
C VV    = V VARIABLE            
C VMIN  = MINIMUM V VARIABLE (INFRARED CUTOFF)  
C NMAX  = MAXIMUM PHOTON MULTIPLICITY           
C*********OUTPUT                
C WT1  = WEIGHT DUE TO NEGLECTED MASS TERMS     
C WT2  = WEIGHT DUE TO DILATATION OF PHOTON MOMENTA             
C WT3  = ANOTHER DILATATION WEIGHT              
C OTHER OUTPUT RESULTS IN /MOMSET/              
C*****************************                  
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)       
      COMMON / MATPAR / PI,CEULER     
      COMMON / PHYPAR / ALFINV,GPICOB     
      COMMON / WEKING / ENE,AMAZ,GAMMZ,AMEL,AMFIN,XK0,SINW2,IDE,IDF
      COMMON / WEKIN2 / AMAW,GAMMW,GMU,ALPHAW   
      COMMON / MOMSET / QEFF1(4),QEFF2(4),SPHUM(4),SPHOT(100,4),NPHOT 
! This common can be everywhere, contains various switches
      COMMON / KeyKey/  KeyRad,KeyPhy,KeyTek,KeyMis,KeyDwm,KeyDwp   
      SAVE / WEKING /,/ MOMSET /,/ KeyKey /,/ WEKIN2 /
      DIMENSION XPH(100),RR(100)                
      SAVE
C               
C HERE BETI2 MUST BE USED INSTEAD OF BETI (MASS TERM NEGLECTED) 
      BETI2 = 2D0/ALFINV/PI*DLOG(4D0*ENE**2/AMEL**2)            
      AM2=(AMEL/ENE)**2         
      DO 10 I=1,NMAX            
      XPH(I)=0D0                
      DO 10 J=1,4               
   10 SPHOT(I,J)=0D0            
      IF(VV.LE.VMIN) THEN       
C NO PHOTON ABOVE DETECTABILITY THRESHOLD       
         WT1=1D0                
         WT2=1D0                
         WT3=1D0                
         NPHOT=0                
      ELSE      
C ONE OR MORE PHOTONS, GENERATE PHOTON MULTIPLICITY             
C NPHOT = POISSON(WITH AVERAGE = AVERG) + 1     
         AVERG=BETI2*DLOG(VV/VMIN)              
  100    CALL POISSG(AVERG,NMAX,MULTP,RR)       
         NPHOT = MULTP+1        
C This is for tests of program at fixed multiplicity (for adv. users)
! switch off the fixed multiplicity by hand !!!!!!!!!!!!!!!
!         NPHFIX =  MOD(KEYBRM,10000)/1000       
         nphfix = 0
! switch off the fixed multiplicity by hand !!!!!!!!!!!!!!!
         IF(NPHFIX.NE.0.AND.NPHOT.NE.NPHFIX) GOTO 100           
         IF(NPHOT.EQ.1) THEN    
            XPH(1)=VV           
            CALL BREMUL(XPH,AM2,WT1)            
            DJAC0=(1D0+1D0/SQRT(1D0-VV))/2D0    
            WT2  = 1D0/DJAC0    
            WT3  = 1D0          
         ELSE                   
            XPH(1)=VV           
            DO 200 I=2,NPHOT    
  200       XPH(I)=VV*(VMIN/VV)**RR(I-1)        
            CALL BREMUL(XPH,AM2,WT1)            
            CALL RESOLH(VV,EXPY,DJAC)           
            DJAC0=(1D0+1D0/SQRT(1D0-VV))/2D0    
            WT2  = DJAC/DJAC0   
            WT3  = 1D0          
C SCALE DOWN PHOTON ENERGIES AND MOMENTA        
            DO 300 I=1,NPHOT    
            DO 300 K=1,4        
  300       SPHOT(I,K)=SPHOT(I,K)/EXPY          
C CHECK ON LOWER ENERGY CUT-OFF                 
            IF(SPHOT(NPHOT,4).LT.VMIN) WT3 =0D0                 
         ENDIF                  
      ENDIF     
C PHOTON MOMENTA IN GEV UNITS   
      DO 420 J=1,4              
  420 SPHUM(J)=0D0              
      DO 480 I=1,NPHOT          
      DO 480 J=1,4              
      SPHOT(I,J)=SPHOT(I,J)*ENE                 
  480 SPHUM(J)=SPHUM(J)+SPHOT(I,J)              

C DEFINE FERMION MOMENTA        
C..      CALL KINEKR    ! MOVED OUTSIDE YFSGEN           
      END       
      SUBROUTINE RESOLH(VV,EXPY,DJAC)           
C     *******************************           
C THIS SOLVES CONSTRAINT EQUATION ON PHOTON MOMENTA             
C ALSO CALCULATES CORRESPONDING JACOBIAN FACTOR                 
C INPUT:  VV    = COSTRAINT PARAMETER V         
C OUTPUT  EXPY  = RESCALING FACTOR - A SOLUTION OF THE EQUATION 
C         DJAC  = JACOBIAN FACTOR               
C     ************************                  
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)       
      DIMENSION PP(4),PK(4)     
      COMMON / MOMSET / QEFF1(4),QEFF2(4),SPHUM(4),SPHOT(100,4),NPHOT
      SAVE   / MOMSET /
      SAVE
C               
      DO 210 K=1,4              
      PK(K)=0D0                 
  210 PP(K)=0D0                 
      PP(4)=2D0                 
      DO 215 I=1,NPHOT          
      DO 215 K=1,4              
  215 PK(K)=PK(K)+SPHOT(I,K)    
      PPDPP=PP(4)**2-PP(3)**2-PP(2)**2-PP(1)**2                 
      PKDPK=PK(4)**2-PK(3)**2-PK(2)**2-PK(1)**2                 
      PPDPK=PP(4)*PK(4)-PP(3)*PK(3)-PP(2)*PK(2)-PP(1)*PK(1)     
      AA=PPDPP*PKDPK/(PPDPK)**2                 
      EXPY=2D0*PPDPK/PPDPP/VV   
C SOLUTION FOR CONSTRAINT ON PHOTON FOUR MOMENTA                
      EXPY=EXPY*.5D0*(1D0+SQRT(1D0-VV*AA))      
C JACOBIAN FACTOR               
      DJAC=(1D0+1D0/SQRT(1D0-VV*AA))/2D0        
      END       
      SUBROUTINE BREMUL(XPH,AM2,WT)             
C     *****************************             
C PROVIDES PHOTON FOURMOMENTA   
C INPUT  : XPH    = LIST OF PHOTON ENERGIES     
C OUTPUT : SPHOT  = LIST OF PHPTON FOUR-MOMENTA                 
C          WT     = WEIGHT DUE TO MASS TERMS    
C     ************************                  
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)       
      COMMON / MATPAR / PI,CEULER     
      COMMON / MOMSET / QEFF1(4),QEFF2(4),SPHUM(4),SPHOT(100,4),NPHOT
      SAVE   / MOMSET /
      SAVE
      DIMENSION XPH(*),rn(1),rnumb(1)          

      WT=1D0    
      DO 100 I=1,NPHOT          
      XK=XPH(I)                 
      CALL VARRAN(RN,1)
      CALL ANGBRE(RN(1),AM2,CG,SG,DIST0,DIST1)     
      WTM   =DIST1/DIST0        
      WT    =WT*WTM             
      CALL VARRAN(RNUMB,1)
      PHI=2D0*PI*RNUMB(1)          
      SPHOT(I,1)=XK*SG*COS(PHI)                 
      SPHOT(I,2)=XK*SG*SIN(PHI)                 
      SPHOT(I,3)=XK*CG          
      SPHOT(I,4)=XK             
  100 CONTINUE                  
C======================================================================
C==================END OF YFSGEN=======================================
C======================================================================
      END       

      SUBROUTINE POISSG(AVERG,NMAX,MULT,RR)
C     ************************************** 
C Last corr. Nov. 91              
C This generates photon multipl. NPHOT according to Poisson distr. 
C INPUT:  AVERG = AVERAGE MULTIPLICITY   
C         NMAX  = MAXIMUM MULTIPLICITY   
C OUTPUT: MULT = GENERATED MULTIPLICITY 
C         RR(1:100) LIST OF ORDERED UNIFORM RANDOM NUMBERS, 
C         A BYPRODUCT RESULT, TO BE EVENTUALLY USED FOR SOME FURTHER
C         PURPOSE (I.E.  GENERATION OF PHOTON ENERGIES). 
C     ************************           
      IMPLICIT DOUBLE PRECISION (A-H,O-Z) 
      DIMENSION RR(*),rn(1)                    
      COMMON / INOUT  / NINP,NOUT  
      SAVE   / INOUT  /
      SAVE
      DATA NFAIL/0/                      
   50 NN=0                               
      SUM=0D0                            
      DO 100 IT=1,NMAX                   
      CALL VARRAN(RN,1)
      Y= LOG(RN(1))                         
      SUM=SUM+Y                          
      NN=NN+1                            
      RR(NN)=SUM/(-AVERG)                
      IF(SUM.LT.-AVERG) GOTO 130         
  100 CONTINUE                           
      NFAIL=NFAIL+1                      
      IF(NFAIL.GT.100) GOTO 900          
      GOTO 50                            
  130 MULT=NN-1                         
      RETURN                             
  900 WRITE(NOUT,*) ' POISSG: TO SMALL NMAX'
      STOP                               
      END                                

      SUBROUTINE ANGBRE(RN1,AM2,COSTHG,SINTHG,DIST0,DIST1)
C     **************************************************** 
C THIS ROUTINE GENERATES PHOTON ANGULAR DISTRIBUTION 
C IN THE REST FRAME OF THE FERMION PAIR. 
C THE DISTRIBUTION IS TAKEN IN THE INFRARED LIMIT.
C GENERATES WEIGHTED EVENTS              
C INPUT:  AM2 = 4*MASSF**2/S WHERE MASSF IS FERMION MASS
C         AND S IS FERMION PAIR EFFECTIVE MASS.
C OUTPUT: COSTHG, SINTHG, COS AND SIN OF THE PHOTON 
C         ANGLE WITH RESPECT TO FERMIONS DIRECTION 
C         DIST0 = distribution  generated without m**2/(kp)**2 terms
C         DIST1 = distribution  with m**2/(kp)**2 terms 
C     *************************************** 
      IMPLICIT DOUBLE PRECISION (A-H,O-Z) 
      DIMENSION RN2(1)
      SAVE
      BETA=SQRT(1.D0-AM2)                
      EPS=AM2/(1.D0+SQRT(1.D0-AM2))      
      DEL1=(2.D0-EPS)*(EPS/(2.D0-EPS))**RN1 
      DEL2=2.D0-DEL1                     
C SYMMETRIZATION                         
      CALL VARRAN(RN2,1)
      IF(RN2(1).LE.0.5D0) THEN              
        A=DEL1                           
        DEL1=DEL2                        
        DEL2=A                           
      ENDIF                              
      DIST0=1D0/DEL1/DEL2                
      DIST1=DIST0-EPS/2.D0*(1D0/DEL1**2+1D0/DEL2**2)
C CALCULATION OF SIN AND COS THETA FROM INTERNAL VARIABLES 
      COSTHG=(1.D0-DEL1)/BETA            
      SINTHG=SQRT(DEL1*DEL2-AM2)/BETA    
      END                                

      FUNCTION BREMKF(KEY,EREL)                  
C     *************************         
C NON-MONTECARLO INTEGRATION OF THE V-DISTRIBUTION            
C GAUSS METHOD, CHANGE OF VARIABLES WITH HELP OF CHBIN1       
C SEE VVDISB              
C KEY= 1,2,3,...FOR VARIOUS DISTRIBUTIONS   
C KEY= 3 FOR MC GENERATION, OTHER FOR TESTS                   
C FOR KEYFIX=1, EXEPTIONALLY, IT PROVIDES INTEGRAND AT VV=VVMAX 
C WITH BORN OMITTED       
C     ************************              
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)   
! This common can be everywhere, contains various switches
      COMMON / KeyKey/  KeyRad,KeyPhy,KeyTek,KeyMis,KeyDwm,KeyDwp  
C COMMON KEYDST COMMUNICATES ONLY WITH VVDISB - INTEGRAND FUNCTION 
      COMMON / KEYDST / KEYDIS              
      COMMON / WEKING / ENE,AMAZ,GAMMZ,AMEL,AMFIN,XK0,SINW2,IDE,IDF 
      COMMON / VVREC  / VVMIN,VVMAX,VV,BETI                   
      SAVE / KeyKey /,/ KEYDST /,/ WEKING /,/ VVREC  /
      SAVE
      EXTERNAL VVDISB     
      DATA KEYFIX /0/
C       
      KEYDIS=KEY          
      IF(KEYFIX.EQ.0) THEN                  
! ms10/21/97 bornsc is going to be redefined !!!
! ms10/21/97         XBORN  =BORNSC(4D0*ENE**2,2)   
! ms10/21/97         write(6,*)'======bremkf=>xborn;',xborn       
! ms10/21/97         PREC=  XBORN*EREL                  
         PREC=  EREL
! ms10/21/97  
         XA= 0D0          
         XB= 1D0
cc         CALL GAUSJD(VVDISB,XA,XB,PREC,RESULT) ! switched to ADAPTIVE etc
         result =GAUS(VVDISB,XA,XB,PREC)
cc      call DGADAP(XA,XB,VVDISB,PREC,RESULT)
         BREMKF=RESULT          
      ELSE                
         SVAR  = 4D0*ENE**2
         BREMKF= VVRHO(KEYDIS,SVAR,AMEL,VVMAX,VVMIN)
     $          /VVRHO(     9,SVAR,AMEL,VVMAX,VVMIN)          
      ENDIF               
      END                 
      FUNCTION VVDISB(R)                    
C     ******************                    
C INTEGRAND FOR BREMKF    
C MAPPING XX => VV CHANGE  TO IMPROVE ON EFFICIENCY           
C     ************************              
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)   
      PARAMETER( FLEPS =1D-35)              
      COMMON / WEKING / ENE,AMAZ,GAMMZ,AMEL,AMFIN,XK0,SINW2,IDE,IDF
      COMMON / WEKIN2 / AMAW,GAMMW,GMU,ALPHAW   
      COMMON / VVREC  / VVMIN,VVMAX,VV,BETI                   
      COMMON / KEYDST / KEYDIS              
      SAVE / WEKING /,/ VVREC  /,/ KEYDST /
      SAVE
C       
      KEYD=KEYDIS        
      X = MAX(R,FLEPS**BETI)                
      ALF=  BETI          
      BET=  1D0           
C ...SPECIAL CASES        
C ...Monte Carlo crude distr                
      IF    (KEYD.EQ.1)  THEN               
        BET=  -0.5D0      
C ...YFS exponentiation beta0,1,2 contribs  
      ELSEIF(KEYD.EQ.310)  THEN              
        ALF=  BETI        
      ELSEIF(KEYD.EQ.311)  THEN             
        ALF=  BETI +1     
      ELSEIF(KEYD.EQ.320)  THEN              
        ALF=  BETI        
      ELSEIF(KEYD.EQ.321)  THEN             
        ALF=  BETI +1     
      ELSEIF(KEYD.EQ.322)  THEN             
        ALF=  BETI +2     
C ...Reference distr including dilatation factor DAMEL        
      ELSEIF(KEYD.EQ.12) THEN               
        BET=  -0.5        
      ENDIF               
      CALL CHBIN1(X,ALF,BET,VVMAX,VV,RJAC) 
C BORN XSECTION           
      SVAR   = 4D0*ENE**2
      SVAR1  = SVAR*(1D0-VV)                 
cc      write(6,*)'vvdisb',alf,bet,svar,svar1,vv,ene,vvmax       
c ms 11/17/97
      xcrude=get_total_crude(svar1)
c ms      xcrude1=tot_born_crud(svar,svar1)
c ms      IF( abs(xcrude1/xcrude -1) .GT. 0.2d0)
c ms     $ WRITE(6,*)'2 print', xcrude1/xcrude
      VVDISB = VVRHO(KEYD,SVAR,AMEL,VV,VVMIN) *RJAC*xcrude        
      END            
     
      FUNCTION VVRHO(KEYDIS,SVAR,AMEL,VV,VVMIN) 
C     *****************************************
C-------------------------------------------------------------
C Convention for KEYDIS      
C     KEYDIS   =  1      crude distribution for initial state MC
C     KEYDIS   =  9      reference distr.  of YFS2 CPC paper 
C     KEYDIS   =  50-52  obsolete test distr. for YFS2 CPC paper 
C-------------------------------------------------------------
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
      COMMON / MATPAR / PI,CEULER     
      COMMON / PHYPAR / ALFINV,GPICOB     
      COMMON / INOUT  / NINP,NOUT     
      SAVE   / INOUT  /
      SAVE
C           
      ALF1   = 1D0/PI/ALFINV
      KEYD = KEYDIS           
      BILG   = DLOG(SVAR/AMEL**2)           
      BETI   = 2D0*ALF1*(BILG-1D0)          
C===================================================================
C ---------------------- KEYD = 1 ----------------------------------
C ---- Crude distribution in YFS2 initial state Monte Carlo --------
C ------------------------------------------------------------------
c dilat is related to dilatation jacobian in yfsgen                
c damel is responsible for modification of photon ang. distribution
c see also weight wt=wt1 in   angbre                               
      IF(KEYD.GE.1.AND.KEYD.LT.100) THEN
         DILAT=1D0           
         IF(VV.GT.VVMIN) DILAT=(1D0+1D0/SQRT(1D0-VV))/2D0        
         BETI2  = 2D0*ALF1*BILG                
         DAMEL=1D0           
         IF(VV.GT.VVMIN) DAMEL=BETI2/BETI*(VV/VVMIN)**(BETI2-BETI)
C---------
         IF    (KEYD.EQ.1)  THEN               
            DISTR= BETI*VV**(BETI-1D0)*DILAT*DAMEL       
C ...Reference distribution used in YFS2 paper --------------------
         ELSEIF(KEYD.EQ. 9)  THEN   
            DISTR= BETI*VV**(BETI-1D0)*(1+(1-VV)**2)/2               
C basic reference distribution  xrefer=sigma-ref                
         ELSEIF(KEYD.EQ.50) THEN   
            DISTR= BETI*VV**(BETI-1D0)             
C XREFER TIMES DAMEL            
         ELSEIF(KEYD.EQ.51) THEN   
            DISTR= BETI*VV**(BETI-1D0)*DAMEL       
C XREFER TIMES DILATATION FACTOR DILAT          
         ELSEIF(KEYD.EQ.52) THEN   
            DISTR= BETI*VV**(BETI-1D0)*DILAT
         ENDIF       
      ELSE       
         GOTO 900             
      ENDIF      
      VVRHO = DISTR                
      RETURN    
 900  WRITE(6,*) ' ===--->  WRONG KEYDIS IN VVRHO',KEYD 
      STOP       
      END        

