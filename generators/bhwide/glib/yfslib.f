C  !!!! LOGBOOK of corrections since 24 Nov 91 !!!!!
C  
C * line in MARRAN to long ( in printout of ijkl)
C * CHBIN2 replaced by CHBIN1
C  !!!!!!!!!!!!!!

C Library of utilities for YFS and BHLUMI programs
C version 1.0 November 91
      SUBROUTINE CHBIN3(R,ALF,BET,X,XPRIM,DJAC)
C     *****************************************
C Written: Dec. 1991
C This routine mapps variable R into X, XPRIM=1-X.
C To be employed in the integration (either ordinary or Monte Carlo)
C of any distributions resambling the binomial distribution 
C             x**(alf-1)*(1-x)**(bet-1).
C with 1> alf,bet > 0. Variables R and X are  in (0,1) range.
C Djac is the Jacobian factor d(x)/d(r).
C Mapping is such that 1/djac is very close to
C binomial distribution x**(alf-1)*(1-x)**(bet-1).
C WARNING: 
C Mapping may fail very close to R=0 and R=1. User is recommended 
C to assure that: fleps**alf < R < 1-fleps**bet, 
C where fleps = 1.d-30.
C     ************************
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
      COMMON / INOUT  / NINP,NOUT
      SAVE   / INOUT  /
C
      IF( ALF.LE.1D-10 .OR. ALF .GT. 3D0 ) GOTO 900
      IF( BET.LE.1D-10 .OR. BET .GT. 3D0 ) GOTO 900
      X0=(1D0-ALF)/(2D0-ALF-BET)
      X0= MIN( MAX(X0, 0.001D0), 0.999D0)
      Q1=       X0**ALF            *BET*(1D0-X0)**(BET-1D0)
      Q2=       ALF*X0**(ALF-1D0)  *((1D0-X0)**BET)
      P1= Q1/(Q1+Q2)
      IF( R.LE.P1 ) THEN
         X    =  X0*(R/P1)**(1D0/ALF)
         XPRIM=  1D0-X
         DIST =  ALF* X**(ALF-1D0)  *BET*(1D0-X0)**(BET-1D0)
ccc      write(6,*) '3A:x,x1=',x,xprim
      ELSE
         XPRIM=  (1-X0)*((1D0-R)/(1D0-P1))**(1D0/BET)
         X    =  1D0- XPRIM
         DIST =  ALF*X0**(ALF-1D0)  *BET*XPRIM**(BET-1D0)
ccc      write(6,*) '3B:x,x1=',x,xprim
      ENDIF
      DJAC    =  (Q1+Q2)/DIST
      RETURN
  900 WRITE(NOUT,*) ' ++++ STOP IN CHBIN3: wrong parameters'
      WRITE(   6,*) ' ++++ STOP IN CHBIN3: wrong parameters'
      STOP
      END

      SUBROUTINE CHBIN1(R,ALF,BET,XMAX,X,DJAC)
C     ****************************************
C     last correction Dec. 91
c this mapps variable r into x.
c to be employed in the integration (either ordinary or monte carlo)
c of distributions resambling
c the binomial distribution x**(alf-1)*(1-x)**(bet-1)
c with alf > 0 and  bet arbitrary.
c variable r is in (0,1) range and x is within (0,xmax) range.
c djac is jacobian factor d(x)/d(r).
c mapping is such that 1/djac is very close to
c binomial distribution x**(alf-1)*(1-x)**(bet-1).
c WARNING: mapping may fail very close to R=0. Practically, one is
c recommended to obey: fleps**alf < r, where fleps = 1.d-30.
c Problems may also arise for very small xmax ( below 1.d-12 ).
C     ************************
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
      COMMON / INOUT  / NINP,NOUT
      save   / INOUT  / 
C
      IF( ALF.LE.0D0 ) GOTO 900
      X0=(ALF-1D0)/(ALF+BET-2D0)
      IF(X0.GT.XMAX) X0=XMAX
      X0= MAX(X0, 0D0)
      Q1= 1D0/ALF*X0**ALF  *(1D0-X0)**(BET-1D0)
      Q2= X0**(ALF-1D0) /BET*((1D0-X0)**BET-(1D0-XMAX)**BET)
      P1= Q1/(Q1+Q2)
      IF( R.LE.P1 ) THEN
         X=X0*(R/P1)**(1D0/ALF)
         DIST= X**(ALF-1D0)*(1D0-X0)**(BET-1D0)
      ELSE
         R1= (1D0-R)/(1D0-P1)
         X = (1D0-XMAX)**BET + ((1D0-X0)**BET-(1D0-XMAX)**BET)*R1
         X = 1D0 - X**(1D0/BET)
         DIST= X0**(ALF-1D0)*(1D0-X)**(BET-1D0)
      ENDIF
      DJAC=(Q1+Q2)/DIST
      RETURN
  900 WRITE(NOUT,*) ' ========= STOP IN CHBIN1: WRONG PARAMS'
      STOP
      END


      SUBROUTINE VESK1W(MMODE,FUNSKO,PAR1,PAR2,PAR3) 
C     **********************************************
C======================================================================
C======================================================================
C===================== V E S K 1 W ====================================
C==================S. JADACH  SEPTEMBER 1985=========================== 
C==================S. JADACH  November  1991=========================== 
C======================================================================
C ONE DIMENSIONAL MONTE CARLO  SAMPLER. 
C Vesrion with weighted events! 
C DOUBLE PRECISION  FUNCTION FUNSKO IS THE DISTRIBUTION TO BE GENERATED.
C JLIM1 IS THE NUMBER OF ENTRIES IN THE EQUIDISTANT LATICE WHICH
C IS FORMED IN THE FIRST STAGE AND JLIM2 IS THE TOTAL MAXIMUM 
C NUMBER OF ENTRIES IN THE LATICE, NOTE THAT DIMENSIONS OF
C MATRICES IN /CESK8A/ SHOULD BE AT LEAST JLIM2+1 . 
C FOR MILD FUNSKO JLIM2=128 IS ENOUGH.   
C TO CREATE AN INDEPENDENT VERSION REPLACE /ESK8A/=>/ESK8B/. 
C     ********************************** 
      IMPLICIT DOUBLE PRECISION (A-H,O-Z) 
      save
      COMMON / CESK1W / XX(1025),YY(1025),ZINT(1025),ZSUM,JMAX 
      COMMON / INOUT  / NINP,NOUT  
      DIMENSION RNUM(1)
      EXTERNAL FUNSKO
      DATA JLIM1,JLIM2/16,257/           
      DATA IWARM/0/                      
C                                        
      MODE=MMODE                         
      IF(MODE.EQ.-1) THEN                
C     ===================                
C INITIALISATION PART, SEE VINSKO FOR MORE COMMENTS
      INIRAN=1                           
      IWARM=1                            
      WT=0.                              
      SWT=0.                             
      SSWT=0.                            
      NEVS=0                             
C INITIALISATION PART, SAMPLING DISTRIBUTION FUNSKO
C AND FILLING MATRICES XX,YY,ZINT ETC.   
      JMAX=1                             
      XX(1)=0.                           
      XX(2)=1.                           
      YY(1)=FUNSKO(XX(1))                
      YY(2)=FUNSKO(XX(2))                
      IF(YY(1).LT.0.0.OR.YY(2).LT.0.0) GO TO 999 
      ZINT(1)=.5D0*(YY(2)+YY(1))*(XX(2)-XX(1))  
C
      JDIV=1                             
      DO 200 K=1,JLIM2-1                 
      IF(JMAX.LT.JLIM1) THEN             
C...    NOTE THAT DESK1W INCREMENTS JMAX=JMAX+1 IN EVERY CALL 
        CALL DESK1W(FUNSKO,JDIV)                
        JDIV=JDIV+2                      
        IF(JDIV.GT.JMAX) JDIV=1          
      ELSE                               
        JDIV=1                           
        ZMX=ZINT(1)                      
        DO 180 J=1,JMAX                  
        IF(ZMX.LT.ZINT(J)) THEN          
          ZMX=ZINT(J)                    
          JDIV=J                         
        ENDIF                            
  180   CONTINUE                         
        CALL DESK1W(FUNSKO,JDIV)                
      ENDIF                              
  200 CONTINUE                           
C                                        
C...  FINAL ADMINISTRATION, NORMALIZING ZINT ETC. 
      ZSUM1=0.                           
      ZSUM =0.                           
      DO 220 J=1,JMAX                    
      ZSUM1=ZSUM1+ZINT(J)                
      YMAX= MAX( YY(J+1),YY(J))          
      ZINT(J)=YMAX*(XX(J+1)-XX(J))       
  220 ZSUM=ZSUM+ZINT(J)                  
      SUM=0.                             
      DO 240 J=1,JMAX                    
      SUM=SUM+ZINT(J)                    
  240 ZINT(J)=SUM/ZSUM                   
C====>>>
C Crude x-section estimate
ccc      CINTEG=ZSUM 
ccc      ERRINT=0D0 
      PAR1=  ZSUM                      
      PAR2=  ZSUM
      PAR3=  ZSUM                      
C===<<<                                 
      ELSE IF(MODE.EQ.0) THEN            
C     =======================            
C GENERATION PART                        
      IF(IWARM.EQ.0) GOTO 901            
ccc  222 CONTINUE                           
ccc      IF( (WT-1D0).GT.1D-10) THEN                  
ccc        WT=WT-1.D0                       
ccc      ELSE                               
        CALL VARRAN(RNUM,1)
        RNUMB=RNUM(1)
        DO 215 J=1,JMAX                  
        JSTOP=J                          
  215   IF(ZINT(J).GT.RNUMB) GOTO 216    
  216   CONTINUE                         
        IF(JSTOP.EQ.1) THEN              
          D=RNUMB/ZINT(1)                
        ELSE                             
          D =(RNUMB-ZINT(JSTOP-1))/(ZINT(JSTOP)-ZINT(JSTOP-1))
        ENDIF                            
        X=XX(JSTOP)*(1.D0 -D )+XX(JSTOP+1)*D   
        FN=FUNSKO(X)                     
        IF(FN.LT.0.D0) GOTO 999            
        YYMAX=MAX(YY(JSTOP+1),YY(JSTOP)) 
        WT=FN/YYMAX                      
        NEVS=NEVS+1                      
        SWT=SWT+WT                       
        SSWT=SSWT+WT*WT                  
ccc      ENDIF                              
ccc      CALL VARRAN(RNUMB,1)
ccc      IF(RNUMB.GT.WT) GOTO 222           
      PAR1=  X                           
      PAR2=  FN
      PAR3=  WT                          
C                                        
      ELSE IF(MODE.EQ.1) THEN            
C     =======================            
C FINAL STATISTICS                       
C STJ 24.OCT.89                          
      CINTEG=0D0                         
      ERRINT=0D0                         
      IF(NEVS.GT.0) CINTEG=ZSUM*SWT/FLOAT(NEVS) 
      IF(NEVS.GT.0) ERRINT=SQRT(SSWT/SWT**2-1.D0/FLOAT(NEVS)) 
      PAR1=  CINTEG                      
      PAR2=  ERRINT
      PAR3=  ZSUM                      
C--
      ELSE                               
C     ====                               
      GOTO  902                          
      ENDIF                              
C     =====                              
C                                        
      RETURN                             
 901  WRITE(NOUT,9010)                   
 9010 FORMAT(' **** STOP IN VESK8A, LACK OF INITIALISATION') 
      STOP                               
 902  WRITE(NOUT,9020)                   
 9020 FORMAT(' **** STOP IN VESK8A, WRONG MODE ') 
      STOP                               
 999  WRITE(NOUT,9990)                   
 9990 FORMAT(' **** STOP IN VESK8A, NEGATIVE VALUE OF FUNSKO ') 
      STOP                               
      END                                
      SUBROUTINE DESK1W(FUNSKO,JDIV)            
C     ******************************            
C THIS ROUTINE BELONGS TO VESK8A PACKAGE 
C IT SUDIVIDES INTO TWO EQUAL PARTS THE INTERVAL 
C (XX(JDIV),XX(JDIV+1))  IN THE LATICE   
C     ***********************            
      IMPLICIT DOUBLE PRECISION (A-H,O-Z) 
      save
      COMMON / CESK1W / XX(1025),YY(1025),ZINT(1025),ZSUM,JMAX 
      COMMON / INOUT  / NINP,NOUT  
      EXTERNAL FUNSKO
C                                        
      XNEW=.5D0*(XX(JDIV) +XX(JDIV+1))   
      DO 100 J=JMAX,JDIV,-1              
      XX(J+2)  =XX(J+1)                  
      YY(J+2)  =YY(J+1)                  
  100 ZINT(J+1)=ZINT(J)                  
      XX(JDIV+1)= XNEW                   
      YY(JDIV+1)= FUNSKO(XNEW)           
      IF(YY(JDIV+1).LT.0.) GOTO 999      
      ZINT(JDIV)  =.5D0*(YY(JDIV+1)+YY(JDIV)  )*(XX(JDIV+1)-XX(JDIV)  ) 
      ZINT(JDIV+1)=.5D0*(YY(JDIV+2)+YY(JDIV+1))*(XX(JDIV+2)-XX(JDIV+1)) 
      JMAX=JMAX+1                        
      RETURN                             
  999 WRITE(NOUT,9000)                   
 9000 FORMAT(' **** STOP IN DESK1W, NEGATIVE VALUE OF FUNSKO ')
      STOP                               
      END                                


      SUBROUTINE VESK2W(MODE,FUNSKO,X,Y,WT)
C     *************************************
C=======================================================================
C=======================================================================
C=======================================================================
C===============TWO DIMENSIONAL SAMPLER VESK2W==========================
C=======================================================================
C=======================================================================
C=======================================================================
C                         VESK2W                                       C
C  GENERAL PURPOSE ROUTINE TO GENERATE AN ARBITRARY TWO DIMENSIONAL    C
C  DISTRIBUTION SUPPLIED BY USER IN A FORM OF FUNCTION FUNSKO(X,Y)     C
C                 WRITTEN NOVEMBER 1985                                C
C                    BY S. JADACH                                      C
C                 LAST UPDATE:  07.NOV.1990                            C
C                 version with weighted event....                      C
C======================================================================C
C VESKO2 GENERATES TWO DIMENSIONAL DISTRIBUTION DEFINED BY ARBITRARY
C FUNCTION FUNSKO(X,Y) WHERE X,Y BELONG  TO (0,1) RANGE.
C THE METHOD CONSISTS IN DIVIDING UNIT PLAQUET INTO CELLS USING
C SORT OF 'LIFE-GAME' METHOD IN WHICH THE DIVISION OF A CELLS IS MADE
C (DURING INITIALISATION) ALWAYS FOR THIS CELL WHICH CONTAINS
C A MAXIMUM VALUE OF THE INTEGRAL OVER FUNSKO IN THE CELL.
C RESULTING CELLS CONTAIN (USUALLY UP TO FACTOR TWO) EQUAL INTERGRAL
C VALUE. THE GENERATION CONSISTS IN CHOOSING RANDOMLY  A CELL
C ACCORDING TO ITS CONTENT AND THEN IN GENERATING X,Y WITHIN THE CELL.
C REJECTION METHOD IS APPLIED AT THE END OF THE PROCEDURE IN ORDER TO
C ASSURE THAT X,Y ARE DISTRIBUTED PRECISELY ACCORDING TO FUNSKO(X,Y)
C                    PARAMETERS
C -/ MODE = -1 INITIALISATION, NO (X,Y) GENERATED, CALL VESKO2(-1,D1,D2)
C    HAS TO BE MADE PRIOR  TO GENERATING FIRST (X,Y) PAIR
C -/ MODE =  0 GENERATION OF (X,Y) PAIR BY CALL VESKO2(0,X,Y)
C -/ MODE =  1 CALL VESKO2(1,VALINT,ERRINT) MAY BE DONE AFTER LAST
C    (X,Y) WAS GENERATED IN ORDER TO OBTAIN THE VALUE OF THE INTEGRAL
C    VALINT AND ITS ERROR ERRINT, INTEGRAL IS CALCULATED USING AVERAGE
C    WEIGHTS ENCOUTERED DURING GENERATION PHASE
C -/ X,Y  IF MODE=-1 THE THEY ARE DUMMY
C         IF MODE= 0 THE RESULT OF RANDOM GENERATION ACCORDING TO
C                    FUNCTION FUNSKO, X AND Y BELONG TO (0,1)
C         IF MODE= 1 X= VALUE OF INTEGRAL AND Y=ERROR (RELATIVE)
C                    WT = crude x-section
C ------------------------------------------------------------
      IMPLICIT DOUBLE PRECISION(A-H,O-Z)
      save
      PARAMETER( JLIM1 = 64, JLIM2 = 1000 , NOUT = 6 )
      COMMON / VESW2  / XX(JLIM2,2),DX(JLIM2,2),YY(JLIM2,2,2)
     $  ,YYMX(JLIM2),ZINT(JLIM2),ZSUM,LEV(JLIM2),JMAX
      DOUBLE PRECISION DRVEC(100)
      EXTERNAL FUNSKO
      DATA IWARM/77/

      IF(MODE) 100,200,300
C...  INITIALISATION PART, SEE VINSKO FOR MORE COMMENTS
  100 CALL VINSKW(FUNSKO)
      IWARM=0
      WT=0D0
      WTMAX = 1D0
      WTMXX = WTMAX
      NEVOV=0
      SWT=0D0
      SSWT=0D0
      NEVS=0
C(((((((((((((
C     CALL HBOOK1(1, 16H WT-VESKO2     $,75,0.0D0,1.5D0)
C     CALL HMINIM(1,0)
C     CALL HBOOK2(2,16H X-Y VESKO2    $, 64,0,1, 32,0,1,0)
C     CALL HSCALE(2)
C))))))))))))
      RETURN
C...
  200 CONTINUE
C...  GENERATION PART
      IF(IWARM.EQ.77) GO TO 980
cc    IF(WT.GT.WTMAX) THEN
cc      write(6,*) ' vesko2: ev. overweighted, dont worry, wt=',wt
cc      WT=WT-WTMAX
cc      NEVOV=NEVOV+1
cc    ELSE
        CALL VARRAN(DRVEC,3)
        R = DRVEC(1)
        DO 215 J=1,JMAX
        JSTOP=J
  215   IF(ZINT(J).GT.R) GOTO 216
  216   CONTINUE
        XR=XX(JSTOP,1)+DX(JSTOP,1)*DRVEC(2)
        YR=XX(JSTOP,2)+DX(JSTOP,2)*DRVEC(3)
        FN=FUNSKO(XR,YR)
        IF(FN.LT.0.) GOTO 999
        YYMAX=YYMX(JSTOP)
        WT=FN/YYMAX
        WTMXX = MAX(WTMXX,WT)
cc      IF(NEVS.LE.(4*JLIM2).AND.WT.GT.WTMAX) THEN
cc         WTMAX=WT*1.1D0
cc         WRITE(6,*) ' VESKO2: NEVS, new WTMAX= ',NEVS,WTMAX
cc      ENDIF
        NEVS=NEVS+1
        SWT=SWT+WT
        SSWT=SSWT+WT*WT
C((((((((((
C       CALL HFILL(1,WT,0D0,1D0)
C))))))))))
ccc   ENDIF
CCC    CALL VARRAN(DRVEC,1)
ccc    RN=DRVEC(1)
ccc   IF(WTMAX*RN.GT.WT) GOTO 200
      X=XR
      Y=YR
C((((((((((
C     CALL HFILL(2,XR,YR)
C))))))))))
      RETURN
C...
  300 CONTINUE
C THIS IS THE VALUE OF THE INTEGRAL
      CINTEG=ZSUM*SWT/NEVS
C AND ITS ERROR
      ERRINT=SQRT(SSWT/SWT**2-1D0/NEVS)
      X=CINTEG
      Y=ERRINT
      WT=ZSUM
C((((((((((
C     CALL HPRINT(1)
C     CALL HDELET(1)
C     CALL HPRINT(2)
C     CALL HDELET(2)
      PRINT 7000,NEVS,NEVOV,WTMAX,WTMXX
 7000 FORMAT(' VESK2W: NEVS,NEVOV,WTMAX,WTMXX= ',2I7,2F7.3)
C))))))))))
      RETURN
  980 WRITE(NOUT,9002)
 9002 FORMAT(' **** STOP IN VESK2W, LACK OF INITIALISATION   ')
      STOP
  999 WRITE(NOUT,9004)
 9004 FORMAT(' **** STOP IN VESK2W, NEGATIVE VALUE OF FUNSKO ')
      STOP
      END

      SUBROUTINE VINSKW(FUNSKO)
C     *************************
C THIS ROUTINE BELONGS TO VESKO2 PACKAGE
C JLIM1 IS THE NUMBER OF CELLS, DIVISION OF THE UNIT PLAQUE INTO CELLS
C IS MADE IN THE FIRST STAGE.    JLIM2 IS THE TOTAL MAXIMUM
C NUMBER OF CELLS, NOTE THAT DIMENSIONS OF
C MATRICES IN /VESKOA/ SHOULD BE AT LEAST JLIM2
C     **********************************
      IMPLICIT DOUBLE PRECISION(A-H,O-Z)
      save
C ------------------------------------------------------------
      PARAMETER( JLIM1 = 64, JLIM2 = 1000 , NOUT = 6 )
      COMMON / VESW2  / XX(JLIM2,2),DX(JLIM2,2),YY(JLIM2,2,2)
     $  ,YYMX(JLIM2),ZINT(JLIM2),ZSUM,LEV(JLIM2),JMAX
      EXTERNAL FUNSKO

C...  INITIALISATION PART, SAMPLING DISTRIBUTION FUNSKO
C...  AND FILLING MATRICES XX,YY,ZINT ETC.
      JMAX=1
      XX(1,1)=0D0
      XX(1,2)=0D0
      DX(1,1)=1D0
      DX(1,2)=1D0
      LEV(1)=1
      SUM=0D0
      DO 150 I=1,2
      DO 150 K=1,2
C... THIS IS NOT ELEGANT BUT SIMPLE
      YY(1,I,K)=FUNSKO(XX(1,1)+(I-1.)*DX(1,1),XX(1,2)+(K-1.)*DX(1,2))
      IF(YY(1,I,K).LT.0.0) GO TO 999
  150 SUM=SUM+YY(1,I,K)
      ZINT(1)=SUM*DX(1,1)*DX(1,2)/4D0

      JDIV=1
      DO 200 KK=1,JLIM2-1
      IF(JMAX.LT.JLIM1) THEN
C...    NOTE THAT DIVSKW INCREMENTS JMAX=JMAX+1 IN EVERY CALL
        CALL DIVSKW(JDIV,FUNSKO)
C(((((((((((
c      IF(JMAX.EQ.JLIM1) THEN
c      PRINT 9900,JMAX,(LEV(I),I=1,JMAX)
c 9900 FORMAT(///,' JMAX...  LEV LEV LEV LEV LEV',I10,/(24I5))
c      PRINT 9901,((XX(JD,I),I=1,2),JD=1,JMAX)
c 9901 FORMAT('  XX XX XX XX XX XX XX  ',/(10E12.5))
c      PRINT 9902,((DX(JD,I),I=1,2),JD=1,JMAX)
c 9902 FORMAT('  DX  DX DX DX DX DX ',/(10E12.5))
c      PRINT 9903,(((YY(JD,I,K),I=1,2),K=1,2),JD=1,JMAX)
c 9903 FORMAT('  YY  YY YY YY YY YY ',/(8E15.5))
c      PRINT 9904,(ZINT(I),I=1,JMAX)
c 9904 FORMAT('   ZINT ZINT ZINT ZINT ',/(10E12.5))
c      ENDIF
C))))))))))))
        JDIV=JDIV+2
        IF(JDIV.GT.JMAX) JDIV=1
      ELSE
        JDIV=1
        ZMX=ZINT(1)
        DO 180 J=1,JMAX
        IF(ZMX.LT.ZINT(J)) THEN
          ZMX=ZINT(J)
          JDIV=J
        ENDIF
  180   CONTINUE
        CALL DIVSKW(JDIV,FUNSKO)
      ENDIF
  200 CONTINUE

C(((((((((((
c      JPRN=64
c      PRINT 9910,JMAX,(LEV(I),I=1,JMAX)
c 9910 FORMAT(/,' JMAX...  LEV LEV LEV LEV LEV',I10,/(24I5))
c      IF(JMAX.LE.JPRN) PRINT 9911,((XX(JD,I),I=1,2),JD=1,JMAX)
c 9911 FORMAT('  XX XX XX XX XX XX XX  ',/(10E12.5))
c      IF(JMAX.LE.JPRN) PRINT 9912,((DX(JD,I),I=1,2),JD=1,JMAX)
c 9912 FORMAT('  DX  DX DX DX DX DX ',/(10E12.5))
c      IF(JMAX.LE.JPRN) PRINT 9913,(((YY(JD,I,K),I=1,2),K=1,2),JD=1,JMAX)
c 9913 FORMAT('  YY  YY YY YY YY YY ',/(8E15.5))
c      IF(JMAX.LE.JPRN) PRINT 9914,(ZINT(I),I=1,JMAX)
c 9914 FORMAT('   ZINT ZINT ZINT ZINT ',/(10E12.5))
C     DO 902 J=1,JMAX
C     Z=1D0*J-.5D0
C 902 CALL HFILL(202,Z,ZINT(J))
C))))))))))))
C...  FINAL ADMINISTRATION, NORMALIZING ZINT ETC.
      ZSUM1=0D0
      ZSUM =0D0
      DO 260 J=1,JMAX
      ZSUM1=ZSUM1+ZINT(J)
      YMAX= 0D0
      DO 250 I=1,2
      DO 250 K=1,2
  250 YMAX= MAX(YMAX,YY(J,I,K))
      YYMX(J)=YMAX
      ZINT(J)=YMAX*DX(J,1)*DX(J,2)
  260 ZSUM=ZSUM+ZINT(J)
C((((((((
      ZR=ZSUM1/ZSUM
      PRINT 7000,ZR
 7000 FORMAT(' /////// ZSUM1/ZSUM= ',F20.8)
C)))))))))
      SUM=0D0
      DO 240 J=1,JMAX
      SUM=SUM+ZINT(J)
  240 ZINT(J)=SUM/ZSUM
C(((((((((((
c     JPRN=64
c     PRINT 9932,JMAX
c9932 FORMAT(/'=====JMAX ZINT ZINT ZINT  ',I10)
c     IF(JMAX.LE.JPRN) PRINT 9935,(ZINT(I),I=1,JMAX)
c9935            FORMAT(10E12.5)
C     DO 901 J=2,JMAX
C 901 CALL HFILL(201,(ZINT(J)-ZINT(J-1))*JMAX)
C     CALL HFILL(201,ZINT(1)*JMAX)
C))))))))))))
      RETURN
  999 WRITE(NOUT,9000)
 9000 FORMAT(' **** STOP IN VINSKW, NEGATIVE VALUE OF FUNSKO ')
      STOP
      END

      SUBROUTINE DIVSKW(JD,FUNSKO)
C     ****************************
C THIS ROUTINE BELONGS TO VESKO2 PACKAGE
C IT SUBDIVIDES ONE CELL (NO. JD) INTO TWO EQUAL SIZE CELLS
C     **********************************
      IMPLICIT DOUBLE PRECISION(A-H,O-Z)
      save
C ------------------------------------------------------------
      PARAMETER( JLIM1 = 64, JLIM2 = 1000 , NOUT = 6 )
      COMMON / VESW2  / XX(JLIM2,2),DX(JLIM2,2),YY(JLIM2,2,2)
     $  ,YYMX(JLIM2),ZINT(JLIM2),ZSUM,LEV(JLIM2),JMAX
      EXTERNAL FUNSKO

C...  MOOVE TO MAKE A HOLE FOR A NEW ENTRY (ONE ADDITIONAL CELL)
      DO 100 J=JMAX,JD,-1
      ZINT(J+1)=ZINT(J)
      LEV(J+1)=LEV(J)
      DO 100 I=1,2
      XX(J+1,I)  =XX(J,I)
      DX(J+1,I)  =DX(J,I)
      DO 100 K=1,2
  100 YY(J+1,I,K)  =YY(J,I,K)
C...  CREATE TWO NEW CELLS AND STORE THEM
      LL= MOD(LEV(JD),2)+1
      DX(JD,LL)=DX(JD,LL)/2D0
      DX(JD+1,LL)=DX(JD+1,LL)/2D0
      XX(JD+1,LL)=XX(JD,LL)+DX(JD,LL)
      IF(LL.EQ.1) THEN
        DO 150 I=1,2
C... THIS IS NOT ELEGANT, PROBABLY COULD BE DONE BETTER
        YY(JD,2,I)=FUNSKO(XX(JD,1)+DX(JD,1),XX(JD,2)+(I-1.)*DX(JD,2))
  150   YY(JD+1,1,I)=YY(JD,2,I)
      ELSE
        DO 152 I=1,2
        YY(JD,I,2)=FUNSKO(XX(JD,1)+(I-1.)*DX(JD,1),XX(JD,2)+DX(JD,2))
  152   YY(JD+1,I,1)=YY(JD,I,2)
      ENDIF
C...  ESTIMATE THE INTEGRALS OVER NEW CELLS RESULTING FROM DIVISION
      DO 220 JDV=JD,JD+1
      LEV(JDV)=LEV(JDV)+1
      SUM=0D0
      DO 210 I=1,2
      DO 210 K=1,2
      IF(YY(JDV,I,K).LT.0.D0) GO TO 999
  210 SUM=SUM+YY(JDV,I,K)
  220 ZINT(JDV) =SUM*DX(JDV,1)*DX(JDV,2)/4D0
      JMAX=JMAX+1
      RETURN
  999 WRITE(NOUT,9000)
 9000 FORMAT(' **** STOP IN DIVSKW, NEGATIVE VALUE OF FUNSKO ')
      STOP
      END


      SUBROUTINE GAUSJD(FUN,AA,BB,EEPS,RESULT)
C     ****************************************
C Gauss integration by S. Jadach, Oct. 90.
C This is NON-ADAPTIVE (!!!!) UNOPTIMIZED (!!!) integration subprogram.
C     *************************
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
      DIMENSION WG(12),XX(12)
      COMMON / INOUT  / NINP,NOUT
      EXTERNAL FUN
      save /inout/,wg,xx,ITERMX 
      DATA WG
     $/0.101228536290376D0, 0.222381034453374D0, 0.313706645877887D0,
     $ 0.362683783378362D0, 0.027152459411754D0, 0.062253523938648D0,
     $ 0.095158511682493D0, 0.124628971255534D0, 0.149595988816577D0,
     $ 0.169156519395003D0, 0.182603415044924D0, 0.189450610455069D0/
      DATA XX
     $/0.960289856497536D0, 0.796666477413627D0, 0.525532409916329D0,
     $ 0.183434642495650D0, 0.989400934991650D0, 0.944575023073233D0,
     $ 0.865631202387832D0, 0.755404408355003D0, 0.617876244402644D0,
     $ 0.458016777657227D0, 0.281603550779259D0, 0.095012509837637D0/
      DATA ITERMX / 15/
      EPS=ABS(EEPS)
      A=AA
      B=BB
      NDIVI=1
C iteration over subdivisions terminated by precision requirement
      DO 400 ITER=1,ITERMX
      CALK8  =0D0
      CALK16 =0D0
C sum over DELTA subintegrals
      DO 200 K = 1,NDIVI
      DELTA = (B-A)/NDIVI
      X1    =  A + (K-1)*DELTA
      X2    =  X1+ DELTA
      XMIDLE= 0.5D0*(X2+X1)
      RANGE = 0.5D0*(X2-X1)
      SUM8 =0D0
      SUM16=0D0
C 8- and 12-point   Gauss integration over single DELTA subinterval
      DO 100 I=1,12
      XPLUS= XMIDLE+RANGE*XX(I)
      XMINU= XMIDLE-RANGE*XX(I)
      FPLUS=FUN(XPLUS)
      FMINU=FUN(XMINU)
      IF(I.LE.4) THEN
          SUM8 =SUM8  +(FPLUS+FMINU)*WG(I)/2D0
      ELSE
          SUM16=SUM16 +(FPLUS+FMINU)*WG(I)/2D0
      ENDIF
  100 CONTINUE
      CALK8 = CALK8 + SUM8 *(X2-X1)
      CALK16= CALK16+ SUM16*(X2-X1)
  200 CONTINUE
      ERABS = ABS(CALK16-CALK8)
      ERELA = 0D0
      IF(CALK16.NE.0D0) ERELA= ERABS/ABS(CALK16)
c     write(6,*) 'gausjd: CALK8,CALK16=',ITER,CALK8,CALK16,ERELA
C precision check to terminate integration
      IF(EEPS.GT.0D0) THEN
        IF(ERABS.LT. EPS) GOTO 800
      ELSE
        IF(ERELA.LT. EPS) GOTO 800
      ENDIF
  400 NDIVI=NDIVI*2
      WRITE(NOUT,*) ' +++++ GAUSJD:  REQUIRED PRECISION TO HIGH!'
      WRITE(NOUT,*) ' +++++ GAUSJD:  ITER,ERELA=',ITER,ERELA
  800 RESULT= CALK16
      END


      SUBROUTINE WMONIT(MODE,ID,WT,WTMAX,RN)
C     **************************************
C last correction 19 sept. 89
C Utility program for monitoring M.C. rejection weights.
C ID is weight idendifier, maximum IDMX (defined below).
C WT IS WEIGHT, WTMAX IS MAXIMUM WEIGHT AND RN IS RANDOM NUMBER.
C IF(MODE.EQ.-1) THEN
C          INITALIZATION IF ENTRY ID, OTHER ARGUMENTS ARE IGNORED
C ELSEIF(MODE.EQ.0) THEN
C          SUMMING UP WEIGHTS ETC. FOR A GIVEN EVENT FOR ENTRY ID
C        - WT IS CURRENT WEIGHT.
C        - WTMAX IS MAXIMUM WEIGHT USED FOR COUTING OVERWEIGHTED
C          EVENTS WITH WT>WTMAX.
C        - RN IS RANDOM NUMBER USED IN REJECTION, IT IS USED TO
C          COUNT NO. OF ACCEPTED (RN<WT/WTMAX) AND REJECTED
C          (WT>WT/WTMAX) EVENTS,
C          IF RO REJECTION THEN PUT RN=0D0.
C ELSEIF(MODE.EQ.1) THEN
C          IN THIS MODE WMONIT REPPORTS ON ACCUMULATED STATISTICS
C          AND THE INFORMATION IS STORED IN COMMON /CMONIT/
C        - AVERWT= AVERAGE WEIGHT WT COUNTING ALL EVENT
C        - ERRELA= RELATIVE ERROR OF AVERWT
C        - NEVTOT= TOTAL NIMBER OF ACCOUNTED EVENTS
C        - NEVACC= NO. OF ACCEPTED EVENTS (RN<WT\WTMAX)
C        - NEVNEG= NO. OF EVENTS WITH NEGATIVE WEIGHT (WT<0)
C        - NEVZER= NO. OF EVENTS WITH ZERO WEIGHT (WT.EQ.0D0)
C        - NEVOVE= NO. OF OVERWEGHTED EVENTS (WT>WTMAX)
C          AND IF YOU DO NOT WANT TO USE CMONIT THEN THE VALUE
C          The value of AVERWT is assigned to WT,
C          the value of ERRELA is assigned to WTMAX and
C          the value of WTMAX  is assigned to RN in this mode.
C ELSEIF(MODEE.EQ.2) THEN
C          ALL INFORMATION DEFINED FOR ENTRY ID DEFINED ABOVE
C          FOR MODE=2 IS JUST PRINTED OF UNIT NOUT
C ENDIF
C NOTE THAT OUTPUT REPPORT (MODE=1,2) IS DONE DYNAMICALLY JUST FOR A
C GIVEN ENTRY ID ONLY AND IT MAY BE REPEATED MANY TIMES FOR ONE ID AND
C FOR VARIOUS ID'S AS WELL.
C     ************************
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
      save
      PARAMETER(IDMX=100)
      COMMON / CMONIT/ AVERWT,ERRELA,NEVTOT,NEVACC,NEVNEG,NEVOVE,NEVZER
      COMMON / INOUT  / NINP,NOUT
      INTEGER NTOT(IDMX),NACC(IDMX),NNEG(IDMX),NOVE(IDMX),NZER(IDMX)
      DIMENSION SWT(IDMX),SSWT(IDMX),WWMX(IDMX)
      DATA NTOT /IDMX* -1/  SWT /IDMX*   0D0/
      DATA SSWT /IDMX*0D0/ WWMX /IDMX*-1D-20/
C
      IF(ID.LE.0.OR.ID.GT.IDMX) THEN
           WRITE(NOUT,*) ' =====WMONIT: WRONG ID',ID
           STOP
      ENDIF
      IF(MODE.EQ.-1) THEN
           NTOT(ID)=0
           NACC(ID)=0
           NNEG(ID)=0
           NZER(ID)=0
           NOVE(ID)=0
           SWT(ID)   =0D0
           SSWT(ID)  =0D0
           WWMX(ID)  = -1D-20
      ELSEIF(MODE.EQ.0) THEN
           IF(NTOT(ID).LT.0) THEN
              WRITE(NOUT,*) ' ==== WARNING FROM WMONIT: '
              WRITE(NOUT,*) ' LACK OF INITIALIZATION, ID=',ID
           ENDIF
           NTOT(ID)=NTOT(ID)+1
           SWT(ID)=SWT(ID)+WT
           SSWT(ID)=SSWT(ID)+WT**2
           WWMX(ID)= MAX(WWMX(ID),WT)
           IF(WT.EQ.0D0)   NZER(ID)=NZER(ID)+1
           IF(WT.LT.0D0)   NNEG(ID)=NNEG(ID)+1
           IF(WT.GT.WTMAX)      NOVE(ID)=NOVE(ID)+1
           IF(RN*WTMAX.LE.WT)   NACC(ID)=NACC(ID)+1
      ELSEIF(MODE.EQ.1) THEN
           IF(NTOT(ID).LT.0) THEN
              WRITE(NOUT,*) ' ==== WARNING FROM WMONIT: '
              WRITE(NOUT,*) ' LACK OF INITIALIZATION, ID=',ID
           ENDIF
           IF(NTOT(ID).LE.0.OR.SWT(ID).EQ.0D0)  THEN
              AVERWT=0D0
              ERRELA=0D0
           ELSE
              AVERWT=SWT(ID)/FLOAT(NTOT(ID))
              ERRELA=SQRT(ABS(SSWT(ID)/SWT(ID)**2-1D0/FLOAT(NTOT(ID))))
           ENDIF
           NEVTOT=NTOT(ID)
           NEVACC=NACC(ID)
           NEVNEG=NNEG(ID)
           NEVZER=NZER(ID)
           NEVOVE=NOVE(ID)
           WT=AVERWT
           WTMAX=ERRELA
           RN    =WWMX(ID)
      ELSEIF(MODE.EQ.2) THEN
           IF(NTOT(ID).LE.0.OR.SWT(ID).EQ.0D0)  THEN
              AVERWT=0D0
              ERRELA=0D0
           ELSE
              AVERWT=SWT(ID)/FLOAT(NTOT(ID))
              ERRELA=SQRT(ABS(SSWT(ID)/SWT(ID)**2-1D0/FLOAT(NTOT(ID))))
              WWMAX=WWMX(ID)
           ENDIF
           WRITE(NOUT,1003) ID, AVERWT, ERRELA, WWMAX
           WRITE(NOUT,1004) NTOT(ID),NACC(ID),NNEG(ID),NOVE(ID),NZER(ID)
           WT=AVERWT
           WTMAX=ERRELA
           RN    =WWMX(ID)
      ELSE
           WRITE(NOUT,*) ' =====WMONIT: WRONG MODE',MODE
           STOP
      ENDIF
 1003 FORMAT(
     $  ' =======================WMONIT========================'
     $/,'   ID           AVERWT         ERRELA            WWMAX'
     $/,    I5,           E17.7,         F15.9,           E17.7)
 1004 FORMAT(
     $  ' -----------------------------------------------------------'
     $/,'      NEVTOT      NEVACC      NEVNEG      NEVOVE      NEVZER'
     $/,   5I12)
      END

      SUBROUTINE WMONI2(MODE,ID,WT,WTMAX,RN)
C     **************************************
C -------------- SECOND COPY OF WMONIT ----------------
C last correction 19 sept. 89
C Utility program for monitoring M.C. rejection weights.
C ID is weight idendifier, maximum IDMX (defined below).
C WT IS WEIGHT, WTMAX IS MAXIMUM WEIGHT AND RN IS RANDOM NUMBER.
C IF(MODE.EQ.-1) THEN
C          INITALIZATION IF ENTRY ID, OTHER ARGUMENTS ARE IGNORED
C ELSEIF(MODE.EQ.0) THEN
C          SUMMING UP WEIGHTS ETC. FOR A GIVEN EVENT FOR ENTRY ID
C        - WT IS CURRENT WEIGHT.
C        - WTMAX IS MAXIMUM WEIGHT USED FOR COUTING OVERWEIGHTED
C          EVENTS WITH WT>WTMAX.
C        - RN IS RANDOM NUMBER USED IN REJECTION, IT IS USED TO
C          COUNT NO. OF ACCEPTED (RN<WT/WTMAX) AND REJECTED
C          (WT>WT/WTMAX) EVENTS,
C          IF RO REJECTION THEN PUT RN=0D0.
C ELSEIF(MODE.EQ.1) THEN
C          IN THIS MODE WMONIT REPPORTS ON ACCUMULATED STATISTICS
C          AND THE INFORMATION IS STORED IN COMMON /CMONIT/
C        - AVERWT= AVERAGE WEIGHT WT COUNTING ALL EVENT
C        - ERRELA= RELATIVE ERROR OF AVERWT
C        - NEVTOT= TOTAL NIMBER OF ACCOUNTED EVENTS
C        - NEVACC= NO. OF ACCEPTED EVENTS (RN<WT\WTMAX)
C        - NEVNEG= NO. OF EVENTS WITH NEGATIVE WEIGHT (WT<0)
C        - NEVZER= NO. OF EVENTS WITH ZERO WEIGHT (WT.EQ.0D0)
C        - NEVOVE= NO. OF OVERWEGHTED EVENTS (WT>WTMAX)
C          AND IF YOU DO NOT WANT TO USE CMONIT THEN THE VALUE
C          The value of AVERWT is assigned to WT,
C          the value of ERRELA is assigned to WTMAX and
C          the value of WTMAX  is assigned to RN in this mode.
C ELSEIF(MODEE.EQ.2) THEN
C          ALL INFORMATION DEFINED FOR ENTRY ID DEFINED ABOVE
C          FOR MODE=2 IS JUST PRINTED OF UNIT NOUT
C ENDIF
C NOTE THAT OUTPUT REPPORT (MODE=1,2) IS DONE DYNAMICALLY JUST FOR A
C GIVEN ENTRY ID ONLY AND IT MAY BE REPEATED MANY TIMES FOR ONE ID AND
C FOR VARIOUS ID'S AS WELL.
C     ************************
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
      save
      PARAMETER(IDMX=100)
      COMMON / CMONI2/ AVERWT,ERRELA,NEVTOT,NEVACC,NEVNEG,NEVOVE,NEVZER
      COMMON / INOUT  / NINP,NOUT
      INTEGER NTOT(IDMX),NACC(IDMX),NNEG(IDMX),NOVE(IDMX),NZER(IDMX)
      DIMENSION SWT(IDMX),SSWT(IDMX),WWMX(IDMX)
      DATA NTOT /IDMX* -1/  SWT /IDMX*   0D0/
      DATA SSWT /IDMX*0D0/ WWMX /IDMX*-1D-20/
C
      IF(ID.LE.0.OR.ID.GT.IDMX) THEN
           WRITE(NOUT,*) ' =====WMONI2: WRONG ID',ID
           STOP
      ENDIF
      IF(MODE.EQ.-1) THEN
           NTOT(ID)=0
           NACC(ID)=0
           NNEG(ID)=0
           NZER(ID)=0
           NOVE(ID)=0
           SWT(ID)   =0D0
           SSWT(ID)  =0D0
           WWMX(ID)  = -1D-20
      ELSEIF(MODE.EQ.0) THEN
           IF(NTOT(ID).LT.0) THEN
              WRITE(NOUT,*) ' ==== WARNING FROM WMONIT: '
              WRITE(NOUT,*) ' LACK OF INITIALIZATION, ID=',ID
           ENDIF
           NTOT(ID)=NTOT(ID)+1
           SWT(ID)=SWT(ID)+WT
           SSWT(ID)=SSWT(ID)+WT**2
           WWMX(ID)= MAX(WWMX(ID),WT)
           IF(WT.EQ.0D0)   NZER(ID)=NZER(ID)+1
           IF(WT.LT.0D0)   NNEG(ID)=NNEG(ID)+1
           IF(WT.GT.WTMAX)      NOVE(ID)=NOVE(ID)+1
           IF(RN*WTMAX.LE.WT)   NACC(ID)=NACC(ID)+1
      ELSEIF(MODE.EQ.1) THEN
           IF(NTOT(ID).LT.0) THEN
              WRITE(NOUT,*) ' ==== WARNING FROM WMONI2: '
              WRITE(NOUT,*) ' LACK OF INITIALIZATION, ID=',ID
           ENDIF
           IF(NTOT(ID).LE.0.OR.SWT(ID).EQ.0D0)  THEN
              AVERWT=0D0
              ERRELA=0D0
           ELSE
              AVERWT=SWT(ID)/FLOAT(NTOT(ID))
              ERRELA=SQRT(ABS(SSWT(ID)/SWT(ID)**2-1D0/FLOAT(NTOT(ID))))
           ENDIF
           NEVTOT=NTOT(ID)
           NEVACC=NACC(ID)
           NEVNEG=NNEG(ID)
           NEVZER=NZER(ID)
           NEVOVE=NOVE(ID)
           WT=AVERWT
           WTMAX=ERRELA
           RN    =WWMX(ID)
      ELSEIF(MODE.EQ.2) THEN
           IF(NTOT(ID).LE.0.OR.SWT(ID).EQ.0D0)  THEN
              AVERWT=0D0
              ERRELA=0D0
           ELSE
              AVERWT=SWT(ID)/FLOAT(NTOT(ID))
              ERRELA=SQRT(ABS(SSWT(ID)/SWT(ID)**2-1D0/FLOAT(NTOT(ID))))
              WWMAX=WWMX(ID)
           ENDIF
           WRITE(NOUT,1003) ID, AVERWT, ERRELA, WWMAX
           WRITE(NOUT,1004) NTOT(ID),NACC(ID),NNEG(ID),NOVE(ID),NZER(ID)
           WT=AVERWT
           WTMAX=ERRELA
           RN    =WWMX(ID)
      ELSE
           WRITE(NOUT,*) ' =====WMONI2: WRONG MODE',MODE
           STOP
      ENDIF
 1003 FORMAT(
     $  ' =======================WMONI2========================'
     $/,'   ID           AVERWT         ERRELA            WWMAX'
     $/,    I5,           E17.7,         F15.9,           E17.7)
 1004 FORMAT(
     $  ' -----------------------------------------------------------'
     $/,'      NEVTOT      NEVACC      NEVNEG      NEVOVE      NEVZER'
     $/,   5I12)
      END

      FUNCTION GAUS(F,A,B,EEPS)  
C     *************************   
C THIS IS ITERATIVE INTEGRATION PROCEDURE                             
C ORIGINATES  PROBABLY FROM CERN LIBRARY                              
C IT SUBDIVIDES INEGRATION RANGE UNTIL REQUIRED PRECISION IS REACHED  
C PRECISION IS A DIFFERENCE FROM 8 AND 16 POINT GAUSS ITEGR. RESULT   
C EEPS POSITIVE TREATED AS ABSOLUTE PRECISION                         
C EEPS NEGATIVE TREATED AS RELATIVE PRECISION                         
C     *************************              
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)    
      DIMENSION W(12),X(12)        
      COMMON / INOUT  / NINP,NOUT      
      EXTERNAL F                             
      DATA CONST /1.0D-19/     
      save     / INOUT/, CONST, W, X
      DATA W                       
     1/0.10122 85362 90376d0, 0.22238 10344 53374d0, 
     1 0.31370 66458 77887d0, 
     2 0.36268 37833 78362d0, 0.02715 24594 11754d0, 
     2 0.06225 35239 38648d0, 
     3 0.09515 85116 82493d0, 0.12462 89712 55534d0, 
     3 0.14959 59888 16577d0, 
     4 0.16915 65193 95003d0, 0.18260 34150 44924d0, 
     4 0.18945 06104 55069d0/ 
      DATA X                       
     1/0.96028 98564 97536d0, 0.79666 64774 13627d0, 
     1 0.52553 24099 16329d0, 
     2 0.18343 46424 95650d0, 0.98940 09349 91650d0, 
     1 0.94457 50230 73233d0, 
     3 0.86563 12023 87832d0, 0.75540 44083 55003d0, 
     1 0.61787 62444 02644d0, 
     4 0.45801 67776 57227d0, 0.28160 35507 79259d0, 
     1 0.09501 25098 37637d0/ 
      EPS=ABS(EEPS)                
      DELTA=CONST*ABS(A-B)         
      GAUS=0D0                     
      AA=A                         
    5 Y=B-AA                       
      IF(ABS(Y) .LE. DELTA) RETURN 
    2 BB=AA+Y                      
      C1=0.5D0*(AA+BB)             
      C2=C1-AA                     
      S8=0D0                       
      S16=0D0                      
      DO 1 I=1,4                   
      U=X(I)*C2                    
    1 S8=S8+W(I)*(F(C1+U)+F(C1-U)) 
      DO 3 I=5,12                  
      U=X(I)*C2                    
    3 S16=S16+W(I)*(F(C1+U)+F(C1-U))                                  
      S8=S8*C2                     
      S16=S16*C2                   
      IF(EEPS.LT.0D0) THEN         
        IF(ABS(S16-S8) .GT. EPS*ABS(S16)) GO TO 4                     
      ELSE             
        IF(ABS(S16-S8) .GT. EPS) GO TO 4                  
      ENDIF            
      GAUS=GAUS+S16    
      AA=BB            
      GO TO 5          
    4 Y=0.5D0*Y        
      IF(ABS(Y) .GT. DELTA) GOTO 2                        
      WRITE(NOUT,7)                          
      GAUS=0D0                
      RETURN                  
    7 FORMAT(1X,36HGAUS  ... TOO HIGH ACCURACY REQUIRED)         
      END                     

      FUNCTION GAUS2(F,A,B,EEPS)  
C     *************************   
C THIS IS ITERATIVE INTEGRATION PROCEDURE                             
C ORIGINATES  PROBABLY FROM CERN LIBRARY                              
C IT SUBDIVIDES INEGRATION RANGE UNTIL REQUIRED PRECISION IS REACHED  
C PRECISION IS A DIFFERENCE FROM 8 AND 16 POINT GAUSS ITEGR. RESULT   
C EEPS POSITIVE TREATED AS ABSOLUTE PRECISION                         
C EEPS NEGATIVE TREATED AS RELATIVE PRECISION                         
C     *************************              
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)    
      DIMENSION W(12),X(12)        
      COMMON / INOUT  / NINP,NOUT      
      EXTERNAL F                             
      DATA CONST /1.0D-19/     
      save     / INOUT/, CONST, W, X
      DATA W                       
     1/0.10122 85362 90376d0, 0.22238 10344 53374d0, 
     1 0.31370 66458 77887d0, 
     2 0.36268 37833 78362d0, 0.02715 24594 11754d0, 
     2 0.06225 35239 38648d0, 
     3 0.09515 85116 82493d0, 0.12462 89712 55534d0, 
     3 0.14959 59888 16577d0, 
     4 0.16915 65193 95003d0, 0.18260 34150 44924d0, 
     4 0.18945 06104 55069d0/ 
      DATA X                       
     1/0.96028 98564 97536d0, 0.79666 64774 13627d0, 
     1 0.52553 24099 16329d0, 
     2 0.18343 46424 95650d0, 0.98940 09349 91650d0, 
     1 0.94457 50230 73233d0, 
     3 0.86563 12023 87832d0, 0.75540 44083 55003d0, 
     1 0.61787 62444 02644d0, 
     4 0.45801 67776 57227d0, 0.28160 35507 79259d0, 
     1 0.09501 25098 37637d0/ 
      EPS=ABS(EEPS)                
      DELTA=CONST*ABS(A-B)         
      GAUS2=0D0                     
      AA=A                         
    5 Y=B-AA                       
      IF(ABS(Y) .LE. DELTA) RETURN 
    2 BB=AA+Y                      
      C1=0.5D0*(AA+BB)             
      C2=C1-AA                     
      S8=0D0                       
      S16=0D0                      
      DO 1 I=1,4                   
      U=X(I)*C2                    
    1 S8=S8+W(I)*(F(C1+U)+F(C1-U)) 
      DO 3 I=5,12                  
      U=X(I)*C2                    
    3 S16=S16+W(I)*(F(C1+U)+F(C1-U))                                  
      S8=S8*C2                     
      S16=S16*C2                   
      IF(EEPS.LT.0D0) THEN         
        IF(ABS(S16-S8) .GT. EPS*ABS(S16)) GO TO 4                     
      ELSE             
        IF(ABS(S16-S8) .GT. EPS) GO TO 4                  
      ENDIF            
      GAUS2=GAUS2+S16    
      AA=BB            
      GO TO 5          
    4 Y=0.5D0*Y        
      IF(ABS(Y) .GT. DELTA) GOTO 2                        
      WRITE(NOUT,7)                          
      GAUS2=0D0                
      RETURN                  
    7 FORMAT(1X,36HGAUS2 ... TOO HIGH ACCURACY REQUIRED)         
      END                     

      FUNCTION GAUS3(F,A,B,EEPS)  
C     *************************   
C THIS IS ITERATIVE INTEGRATION PROCEDURE                             
C ORIGINATES  PROBABLY FROM CERN LIBRARY                              
C IT SUBDIVIDES INEGRATION RANGE UNTIL REQUIRED PRECISION IS REACHED  
C PRECISION IS A DIFFERENCE FROM 8 AND 16 POINT GAUSS ITEGR. RESULT   
C EEPS POSITIVE TREATED AS ABSOLUTE PRECISION                         
C EEPS NEGATIVE TREATED AS RELATIVE PRECISION                         
C     *************************              
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)    
      DIMENSION W(12),X(12)        
      COMMON / INOUT  / NINP,NOUT      
      EXTERNAL F                             
      DATA CONST /1.0D-19/     
      save     / INOUT/, CONST, W, X
      DATA W                       
     1/0.10122 85362 90376d0, 0.22238 10344 53374d0, 
     1 0.31370 66458 77887d0, 
     2 0.36268 37833 78362d0, 0.02715 24594 11754d0, 
     2 0.06225 35239 38648d0, 
     3 0.09515 85116 82493d0, 0.12462 89712 55534d0, 
     3 0.14959 59888 16577d0, 
     4 0.16915 65193 95003d0, 0.18260 34150 44924d0, 
     4 0.18945 06104 55069d0/ 
      DATA X                       
     1/0.96028 98564 97536d0, 0.79666 64774 13627d0, 
     1 0.52553 24099 16329d0, 
     2 0.18343 46424 95650d0, 0.98940 09349 91650d0, 
     1 0.94457 50230 73233d0, 
     3 0.86563 12023 87832d0, 0.75540 44083 55003d0, 
     1 0.61787 62444 02644d0, 
     4 0.45801 67776 57227d0, 0.28160 35507 79259d0, 
     1 0.09501 25098 37637d0/ 
      EPS=ABS(EEPS)                
      DELTA=CONST*ABS(A-B)         
      GAUS3=0D0                     
      AA=A                         
    5 Y=B-AA                       
      IF(ABS(Y) .LE. DELTA) RETURN 
    2 BB=AA+Y                      
      C1=0.5D0*(AA+BB)             
      C2=C1-AA                     
      S8=0D0                       
      S16=0D0                      
      DO 1 I=1,4                   
      U=X(I)*C2                    
    1 S8=S8+W(I)*(F(C1+U)+F(C1-U)) 
      DO 3 I=5,12                  
      U=X(I)*C2                    
    3 S16=S16+W(I)*(F(C1+U)+F(C1-U))                                  
      S8=S8*C2                     
      S16=S16*C2                   
      IF(EEPS.LT.0D0) THEN         
        IF(ABS(S16-S8) .GT. EPS*ABS(S16)) GO TO 4                     
      ELSE             
        IF(ABS(S16-S8) .GT. EPS) GO TO 4                  
      ENDIF            
      GAUS3=GAUS3+S16    
      AA=BB            
      GO TO 5          
    4 Y=0.5D0*Y        
      IF(ABS(Y) .GT. DELTA) GOTO 2                        
      WRITE(NOUT,7)                          
      GAUS3=0D0                
      RETURN                  
    7 FORMAT(1X,36HGAUS3 ... TOO HIGH ACCURACY REQUIRED)         
      END                     

C  CORRECTIONS ST. JADACH   (STJ)
C    DOUBLE PRECISION,
C    THIS PROGRAM IS NOT REALY ABLE TO FIND INTEGRAL
C    WITH RELATIVE PRECISION, EPS IS NOW ABSOLUTE ERROR (INPUT ONLY!!)
C.......................................................................
C
C   PURPOSE           - INTEGRATE A FUNCTION F(X)
C   METHOD            - ADAPTIVE GAUSSIAN
C   USAGE             - CALL GADAP(A0,B0,F,EPS,SUM)
C   PARAMETERS  A0    - LOWER LIMIT (INPUT,REAL)
C               B0    - UPPER LIMIT (INPUT,REAL)
C               F     - FUNCTION F(X) TO BE INTEGRATED. MUST BE
C                       SUPPLIED BY THE USER. (INPUT,REAL FUNCTION)
C               EPS   - DESIRED RELATIVE ACCURACY. IF SUM IS SMALL EPS
C                       WILL BE ABSOLUTE ACCURACY INSTEAD. (INPUT,REAL)
C               SUM   - CALCULATED VALUE FOR THE INTEGRAL (OUTPUT,REAL)
C   PRECISION         - DOUBLE
C   REQ'D PROG'S      - F
C   AUTHOR            - THOMAS JOHANSSON, LDC,1973
C   REFERENCE(S)      - THE AUSTRALIAN COMPUTER JOURNAL,3 P.126 AUG. -71
C
C.......................................................................
      SUBROUTINE DGADAP(A0,B0,F,EPS1,SUM)
*     **********************************
      IMPLICIT DOUBLE PRECISION(A-H,O-Z)
      COMMON/GADAP1/ NUM,IFU
      EXTERNAL F
      DIMENSION A(300),B(300),F1(300),F2(300),F3(300),S(300),N(300)
    1 FORMAT(16H GADAP:I TOO BIG)
      DSUM(F1F,F2F,F3F,AA,BB)=5D0/18D0*(BB-AA)*(F1F+1.6D0*F2F+F3F)

      EPS=EPS1
      IF(EPS.LT.1D-15) EPS=1D-15
      RED=1.3D0
      L=1
      I=1
      SUM=0D0
      C=SQRT(15D0)/5D0
      A(1)=A0
      B(1)=B0
      F1(1)=F(0.5D0*(1D0+C)*A0+0.5D0*(1D0-C)*B0)
      F2(1)=F(0.5D0*(A0+B0))
      F3(1)=F(0.5D0*(1D0-C)*A0+0.5D0*(1D0+C)*B0)
      IFU=3
      S(1)=  DSUM(F1(1),F2(1),F3(1),A0,B0)
  100 CONTINUE
      L=L+1
      N(L)=3
      EPS=EPS*RED
      A(I+1)=A(I)+C*(B(I)-A(I))
      B(I+1)=B(I)
      A(I+2)=A(I)+B(I)-A(I+1)
      B(I+2)=A(I+1)
      A(I+3)=A(I)
      B(I+3)=A(I+2)
      W1=A(I)+(B(I)-A(I))/5D0
      U2=2D0*W1-(A(I)+A(I+2))/2D0
      F1(I+1)=F(A(I)+B(I)-W1)
      F2(I+1)=F3(I)
      F3(I+1)=F(B(I)-A(I+2)+W1)
      F1(I+2)=F(U2)
      F2(I+2)=F2(I)
      F3(I+2)=F(B(I+2)+A(I+2)-U2)
      F1(I+3)=F(A(I)+A(I+2)-W1)
      F2(I+3)=F1(I)
      F3(I+3)=F(W1)
      IFU=IFU+6
      IF(IFU.GT.5000) GOTO 130
      S(I+1)=  DSUM(F1(I+1),F2(I+1),F3(I+1),A(I+1),B(I+1))
      S(I+2)=  DSUM(F1(I+2),F2(I+2),F3(I+2),A(I+2),B(I+2))
      S(I+3)=  DSUM(F1(I+3),F2(I+3),F3(I+3),A(I+3),B(I+3))
      SS=S(I+1)+S(I+2)+S(I+3)
      I=I+3
      IF(I.GT.300)GOTO 120
      SOLD=S(I-3)
*STJ  IF(ABS(SOLD-SS).GT.EPS*(1D0+ABS(SS))/2D0) GOTO 100
      IF(ABS(SOLD-SS).GT.EPS/2D0) GOTO 100
      SUM=SUM+SS
      I=I-4
      N(L)=0
      L=L-1
  110 CONTINUE
      IF(L.EQ.1) GOTO 130
      N(L)=N(L)-1
      EPS=EPS/RED
      IF(N(L).NE.0) GOTO 100
      I=I-1
      L=L-1
      GOTO 110
  120 WRITE(6,1)
 130  CONTINUE
      END

C  CORRECTIONS ST. JADACH   (STJ)
C    DOUBLE PRECISION,
C    THIS PROGRAM IS NOT REALY ABLE TO FIND INTEGRAL
C    WITH RELATIVE PRECISION, EPS IS NOW ABSOLUTE ERROR (INPUT ONLY!!)
C.......................................................................
C
C   PURPOSE           - INTEGRATE A FUNCTION F(X)
C   METHOD            - ADAPTIVE GAUSSIAN
C   USAGE             - CALL GADAP(A0,B0,F,EPS,SUM)
C   PARAMETERS  A0    - LOWER LIMIT (INPUT,REAL)
C               B0    - UPPER LIMIT (INPUT,REAL)
C               F     - FUNCTION F(X) TO BE INTEGRATED. MUST BE
C                       SUPPLIED BY THE USER. (INPUT,REAL FUNCTION)
C               EPS   - DESIRED RELATIVE ACCURACY. IF SUM IS SMALL EPS
C                       WILL BE ABSOLUTE ACCURACY INSTEAD. (INPUT,REAL)
C               SUM   - CALCULATED VALUE FOR THE INTEGRAL (OUTPUT,REAL)
C   PRECISION         - DOUBLE
C   REQ'D PROG'S      - F
C   AUTHOR            - THOMAS JOHANSSON, LDC,1973
C   REFERENCE(S)      - THE AUSTRALIAN COMPUTER JOURNAL,3 P.126 AUG. -71
C
C.......................................................................
      SUBROUTINE DGADA2(A0,B0,F,EPS1,SUM)
*     **********************************
      IMPLICIT DOUBLE PRECISION(A-H,O-Z)
      COMMON/GADAP1/ NUM,IFU
      EXTERNAL F
      DIMENSION A(300),B(300),F1(300),F2(300),F3(300),S(300),N(300)
    1 FORMAT(16H GADAP:I TOO BIG)
      DSUM(F1F,F2F,F3F,AA,BB)=5D0/18D0*(BB-AA)*(F1F+1.6D0*F2F+F3F)

      EPS=EPS1
      IF(EPS.LT.1D-15) EPS=1D-15
      RED=1.3D0
      L=1
      I=1
      SUM=0D0
      C=SQRT(15D0)/5D0
      A(1)=A0
      B(1)=B0
      F1(1)=F(0.5D0*(1D0+C)*A0+0.5D0*(1D0-C)*B0)
      F2(1)=F(0.5D0*(A0+B0))
      F3(1)=F(0.5D0*(1D0-C)*A0+0.5D0*(1D0+C)*B0)
      IFU=3
      S(1)=  DSUM(F1(1),F2(1),F3(1),A0,B0)
  100 CONTINUE
      L=L+1
      N(L)=3
      EPS=EPS*RED
      A(I+1)=A(I)+C*(B(I)-A(I))
      B(I+1)=B(I)
      A(I+2)=A(I)+B(I)-A(I+1)
      B(I+2)=A(I+1)
      A(I+3)=A(I)
      B(I+3)=A(I+2)
      W1=A(I)+(B(I)-A(I))/5D0
      U2=2D0*W1-(A(I)+A(I+2))/2D0
      F1(I+1)=F(A(I)+B(I)-W1)
      F2(I+1)=F3(I)
      F3(I+1)=F(B(I)-A(I+2)+W1)
      F1(I+2)=F(U2)
      F2(I+2)=F2(I)
      F3(I+2)=F(B(I+2)+A(I+2)-U2)
      F1(I+3)=F(A(I)+A(I+2)-W1)
      F2(I+3)=F1(I)
      F3(I+3)=F(W1)
      IFU=IFU+6
      IF(IFU.GT.5000) GOTO 130
      S(I+1)=  DSUM(F1(I+1),F2(I+1),F3(I+1),A(I+1),B(I+1))
      S(I+2)=  DSUM(F1(I+2),F2(I+2),F3(I+2),A(I+2),B(I+2))
      S(I+3)=  DSUM(F1(I+3),F2(I+3),F3(I+3),A(I+3),B(I+3))
      SS=S(I+1)+S(I+2)+S(I+3)
      I=I+3
      IF(I.GT.300)GOTO 120
      SOLD=S(I-3)
*STJ  IF(ABS(SOLD-SS).GT.EPS*(1D0+ABS(SS))/2D0) GOTO 100
      IF(ABS(SOLD-SS).GT.EPS/2D0) GOTO 100
      SUM=SUM+SS
      I=I-4
      N(L)=0
      L=L-1
  110 CONTINUE
      IF(L.EQ.1) GOTO 130
      N(L)=N(L)-1
      EPS=EPS/RED
      IF(N(L).NE.0) GOTO 100
      I=I-1
      L=L-1
      GOTO 110
  120 WRITE(6,1)
 130  CONTINUE
      END

C  CORRECTIONS ST. JADACH   (STJ)
C    DOUBLE PRECISION,
C    THIS PROGRAM IS NOT REALY ABLE TO FIND INTEGRAL
C    WITH RELATIVE PRECISION, EPS IS NOW ABSOLUTE ERROR (INPUT ONLY!!)
C.......................................................................
C
C   PURPOSE           - INTEGRATE A FUNCTION F(X)
C   METHOD            - ADAPTIVE GAUSSIAN
C   USAGE             - CALL GADAP(A0,B0,F,EPS,SUM)
C   PARAMETERS  A0    - LOWER LIMIT (INPUT,REAL)
C               B0    - UPPER LIMIT (INPUT,REAL)
C               F     - FUNCTION F(X) TO BE INTEGRATED. MUST BE
C                       SUPPLIED BY THE USER. (INPUT,REAL FUNCTION)
C               EPS   - DESIRED RELATIVE ACCURACY. IF SUM IS SMALL EPS
C                       WILL BE ABSOLUTE ACCURACY INSTEAD. (INPUT,REAL)
C               SUM   - CALCULATED VALUE FOR THE INTEGRAL (OUTPUT,REAL)
C   PRECISION         - DOUBLE
C   REQ'D PROG'S      - F
C   AUTHOR            - THOMAS JOHANSSON, LDC,1973
C   REFERENCE(S)      - THE AUSTRALIAN COMPUTER JOURNAL,3 P.126 AUG. -71
C
C.......................................................................
      SUBROUTINE DGADA3(A0,B0,F,EPS1,SUM)
*     **********************************
      IMPLICIT DOUBLE PRECISION(A-H,O-Z)
      COMMON/GADAP1/ NUM,IFU
      EXTERNAL F
      DIMENSION A(300),B(300),F1(300),F2(300),F3(300),S(300),N(300)
    1 FORMAT(16H GADAP:I TOO BIG)
      DSUM(F1F,F2F,F3F,AA,BB)=5D0/18D0*(BB-AA)*(F1F+1.6D0*F2F+F3F)

      EPS=EPS1
      IF(EPS.LT.1D-15) EPS=1D-15
      RED=1.3D0
      L=1
      I=1
      SUM=0D0
      C=SQRT(15D0)/5D0
      A(1)=A0
      B(1)=B0
      F1(1)=F(0.5D0*(1D0+C)*A0+0.5D0*(1D0-C)*B0)
      F2(1)=F(0.5D0*(A0+B0))
      F3(1)=F(0.5D0*(1D0-C)*A0+0.5D0*(1D0+C)*B0)
      IFU=3
      S(1)=  DSUM(F1(1),F2(1),F3(1),A0,B0)
  100 CONTINUE
      L=L+1
      N(L)=3
      EPS=EPS*RED
      A(I+1)=A(I)+C*(B(I)-A(I))
      B(I+1)=B(I)
      A(I+2)=A(I)+B(I)-A(I+1)
      B(I+2)=A(I+1)
      A(I+3)=A(I)
      B(I+3)=A(I+2)
      W1=A(I)+(B(I)-A(I))/5D0
      U2=2D0*W1-(A(I)+A(I+2))/2D0
      F1(I+1)=F(A(I)+B(I)-W1)
      F2(I+1)=F3(I)
      F3(I+1)=F(B(I)-A(I+2)+W1)
      F1(I+2)=F(U2)
      F2(I+2)=F2(I)
      F3(I+2)=F(B(I+2)+A(I+2)-U2)
      F1(I+3)=F(A(I)+A(I+2)-W1)
      F2(I+3)=F1(I)
      F3(I+3)=F(W1)
      IFU=IFU+6
      IF(IFU.GT.5000) GOTO 130
      S(I+1)=  DSUM(F1(I+1),F2(I+1),F3(I+1),A(I+1),B(I+1))
      S(I+2)=  DSUM(F1(I+2),F2(I+2),F3(I+2),A(I+2),B(I+2))
      S(I+3)=  DSUM(F1(I+3),F2(I+3),F3(I+3),A(I+3),B(I+3))
      SS=S(I+1)+S(I+2)+S(I+3)
      I=I+3
      IF(I.GT.300)GOTO 120
      SOLD=S(I-3)
*STJ  IF(ABS(SOLD-SS).GT.EPS*(1D0+ABS(SS))/2D0) GOTO 100
      IF(ABS(SOLD-SS).GT.EPS/2D0) GOTO 100
      SUM=SUM+SS
      I=I-4
      N(L)=0
      L=L-1
  110 CONTINUE
      IF(L.EQ.1) GOTO 130
      N(L)=N(L)-1
      EPS=EPS/RED
      IF(N(L).NE.0) GOTO 100
      I=I-1
      L=L-1
      GOTO 110
  120 WRITE(6,1)
 130  CONTINUE
      END


      DOUBLE PRECISION FUNCTION DILOGX(X)
C-------------------------------------------- REMARKS ---------------
C DILOGARITHM FUNCTION: DILOG(X)=INT( -LN(1-Z)/Z ) , 0 < Z < X .
C THIS IS THE CERNLIB VERSION.
C--------------------------------------------------------------------
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
      Z=-1.644934066848226D0
      IF(X .LT.-1.D0) GO TO 1
      IF(X .LE. 0.5D0) GO TO 2
      IF(X .EQ. 1.D0) GO TO 3
      IF(X .LE. 2.D0) GO TO 4
      Z=3.289868133696453D0
    1 T=1.D0/X
      S=-0.5D0
      Z=Z-0.5D0*DLOG(DABS(X))**2
      GO TO 5
    2 T=X
      S=0.5D0
      Z=0.D0
      GO TO 5
    3 DILOGX=1.644934066848226D0
      RETURN
    4 T=1.D0-X
      S=-0.5D0
      Z=1.644934066848226D0-DLOG(X)*DLOG(DABS(T))
    5 Y=2.666666666666667D0*T+0.666666666666667D0
      B=      0.000000000000001D0
      A=Y*B  +0.000000000000004D0
      B=Y*A-B+0.000000000000011D0
      A=Y*B-A+0.000000000000037D0
      B=Y*A-B+0.000000000000121D0
      A=Y*B-A+0.000000000000398D0
      B=Y*A-B+0.000000000001312D0
      A=Y*B-A+0.000000000004342D0
      B=Y*A-B+0.000000000014437D0
      A=Y*B-A+0.000000000048274D0
      B=Y*A-B+0.000000000162421D0
      A=Y*B-A+0.000000000550291D0
      B=Y*A-B+0.000000001879117D0
      A=Y*B-A+0.000000006474338D0
      B=Y*A-B+0.000000022536705D0
      A=Y*B-A+0.000000079387055D0
      B=Y*A-B+0.000000283575385D0
      A=Y*B-A+0.000001029904264D0
      B=Y*A-B+0.000003816329463D0
      A=Y*B-A+0.000014496300557D0
      B=Y*A-B+0.000056817822718D0
      A=Y*B-A+0.000232002196094D0
      B=Y*A-B+0.001001627496164D0
      A=Y*B-A+0.004686361959447D0
      B=Y*A-B+0.024879322924228D0
      A=Y*B-A+0.166073032927855D0
      A=Y*A-B+1.935064300869969D0
      DILOGX=S*T*(A-B)+Z
      END

      DOUBLE PRECISION FUNCTION DILOGY(X)
C-------------------------------------------- REMARKS ---------------
C DILOGARITHM FUNCTION: DILOG(X)=INT( -LN(1-Z)/Z ) , 0 < Z < X .
C THIS IS THE CERNLIB VERSION.
C--------------------------------------------------------------------
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
      Z=-1.644934066848226D0
      IF(X .LT.-1.D0) GO TO 1
      IF(X .LE. 0.5D0) GO TO 2
      IF(X .EQ. 1.D0) GO TO 3
      IF(X .LE. 2.D0) GO TO 4
      Z=3.289868133696453D0
    1 T=1.D0/X
      S=-0.5D0
      Z=Z-0.5D0*DLOG(DABS(X))**2
      GO TO 5
    2 T=X
      S=0.5D0
      Z=0.D0
      GO TO 5
    3 DILOGY=1.644934066848226D0
      RETURN
    4 T=1.D0-X
      S=-0.5D0
      Z=1.644934066848226D0-DLOG(X)*DLOG(DABS(T))
    5 Y=2.666666666666667D0*T+0.666666666666667D0
      B=      0.000000000000001D0
      A=Y*B  +0.000000000000004D0
      B=Y*A-B+0.000000000000011D0
      A=Y*B-A+0.000000000000037D0
      B=Y*A-B+0.000000000000121D0
      A=Y*B-A+0.000000000000398D0
      B=Y*A-B+0.000000000001312D0
      A=Y*B-A+0.000000000004342D0
      B=Y*A-B+0.000000000014437D0
      A=Y*B-A+0.000000000048274D0
      B=Y*A-B+0.000000000162421D0
      A=Y*B-A+0.000000000550291D0
      B=Y*A-B+0.000000001879117D0
      A=Y*B-A+0.000000006474338D0
      B=Y*A-B+0.000000022536705D0
      A=Y*B-A+0.000000079387055D0
      B=Y*A-B+0.000000283575385D0
      A=Y*B-A+0.000001029904264D0
      B=Y*A-B+0.000003816329463D0
      A=Y*B-A+0.000014496300557D0
      B=Y*A-B+0.000056817822718D0
      A=Y*B-A+0.000232002196094D0
      B=Y*A-B+0.001001627496164D0
      A=Y*B-A+0.004686361959447D0
      B=Y*A-B+0.024879322924228D0
      A=Y*B-A+0.166073032927855D0
      A=Y*A-B+1.935064300869969D0
      DILOGY=S*T*(A-B)+Z
      END


      DOUBLE PRECISION FUNCTION DPGAMM(Z)
C     **********************************
C Double precision Gamma function
      DOUBLE PRECISION Z,Z1,X,X1,X2,D1,D2,S1,S2,S3,PI,C(20),CONST
      save C,PI,CONST
      DATA C( 1) / 8.3333333333333333333333333332D-02/
      DATA C( 2) /-2.7777777777777777777777777777D-03/
      DATA C( 3) / 7.9365079365079365079365079364D-04/
      DATA C( 4) /-5.9523809523809523809523809523D-04/
      DATA C( 5) / 8.4175084175084175084175084175D-04/
      DATA C( 6) /-1.9175269175269175269175269175D-03/
      DATA C( 7) / 6.4102564102564102564102564102D-03/
      DATA C( 8) /-2.9550653594771241830065359477D-02/
      DATA C( 9) / 1.7964437236883057316493849001D-01/
      DATA C(10) /-1.3924322169059011164274322169D+00/
      DATA C(11) / 1.3402864044168391994478951001D+01/
      DATA C(12) /-1.5684828462600201730636513245D+02/
      DATA C(13) / 2.1931033333333333333333333333D+03/
      DATA C(14) /-3.6108771253724989357173265219D+04/
      DATA C(15) / 6.9147226885131306710839525077D+05/
      DATA C(16) /-1.5238221539407416192283364959D+07/
      DATA C(17) / 3.8290075139141414141414141414D+08/
      DATA C(18) /-1.0882266035784391089015149165D+10/
      DATA C(19) / 3.4732028376500225225225225224D+11/
      DATA C(20) /-1.2369602142269274454251710349D+13/
      DATA PI    / 3.1415926535897932384626433832D+00/
      DATA CONST / 9.1893853320467274178032973641D-01/
      IF(Z.GT.5.75D 1)                                     GOTO  6666
      NN = Z
      IF (Z  -  DBLE(FLOAT(NN)))                 3,1,3
    1 IF (Z    .LE.    0.D 0)                    GOTO 6667
      DPGAMM = 1.D 0
      IF (Z    .LE.    2.D 0)                    RETURN
      Z1 = Z
    2 Z1 = Z1  -  1.D 0
      DPGAMM = DPGAMM * Z1
      IF (Z1  -  2.D 0)                          61,61,2
    3 IF (DABS(Z)    .LT.    1.D-29)             GOTO 60
      IF (Z    .LT.    0.D 0)                    GOTO 4
      X  = Z
      KK = 1
      GOTO 10
    4 X  = 1.D 0  -  Z
      KK = 2
   10 X1 = X
      IF (X    .GT.    19.D 0)                   GOTO 13
      D1 = X
   11 X1 = X1  +  1.D 0
      IF (X1    .GE.    19.D 0)                  GOTO 12
      D1 = D1 * X1
      GOTO 11
   12 S3 = -DLOG(D1)
      GOTO 14
   13 S3 = 0.D 0
   14 D1 = X1 * X1
      S1 = (X1  -  5.D-1) * DLOG(X1)  -  X1  +  CONST
      DO 20                  K=1,20
      S2 = S1  +  C(K)/X1
      IF (DABS(S2  -  S1)    .LT.    1.D-28)     GOTO 21
      X1 = X1 * D1
   20 S1 = S2
   21 S3 = S3  +  S2
      GOTO (50,22),    KK
   22 D2 = DABS(Z  -  NN)
      D1 = D2 * PI
      IF (D1    .LT.    1.D-15)                  GOTO 31
   30 X2 =  DLOG(PI/DSIN(D1))  -  S3
      GOTO 40
   31 X2 = -DLOG(D2)
   40 MM = DABS(Z)
      IF(X2      .GT.      1.74D2)                         GO TO 6666
      DPGAMM = DEXP(X2)
      IF (MM    .NE.    (MM/2) * 2)              RETURN
      DPGAMM = -DPGAMM
      RETURN
   50 IF(S3      .GT.      1.74D2)                         GO TO 6666
      DPGAMM = DEXP(S3)
      RETURN
 6666 PRINT *, 2000
      RETURN
 6667 PRINT *, 2001
      RETURN
   60 DPGAMM = 0.D 0
      IF(DABS(Z)   .LT.   1.D-77)   RETURN
      DPGAMM = 1.D 0/Z
   61 RETURN
 2000 FORMAT (/////, 2X, 32HDPGAMM ..... ARGUMENT TOO LARGE., /////)
 2001 FORMAT (/////, 2X, 32HDPGAMM ..... ARGUMENT IS A POLE., /////)
      END

      SUBROUTINE BOSTDQ(MODE,QQ,PP,R)        
C     *******************************        
C BOOST ALONG ARBITRARY AXIS (BY RONALD KLEISS).
C P BOOSTED INTO R  FROM ACTUAL FRAME TO REST FRAME OF Q  
C FORTH (MODE = 1) OR BACK (MODE = -1).      
C Q MUST BE A TIMELIKE, P MAY BE ARBITRARY.  
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)    
      PARAMETER ( NOUT =6 )                         
      DIMENSION QQ(*),PP(*),R(*)             
      DIMENSION Q(4),P(4)                    

      DO 10 K=1,4                            
      P(K)=PP(K)                             
   10 Q(K)=QQ(K)                             
      AMQ =DSQRT(Q(4)**2-Q(1)**2-Q(2)**2-Q(3)**2) 
      IF    (MODE.EQ.-1) THEN                
         R(4) = (P(1)*Q(1)+P(2)*Q(2)+P(3)*Q(3)+P(4)*Q(4))/AMQ 
         FAC  = (R(4)+P(4))/(Q(4)+AMQ)       
      ELSEIF(MODE.EQ. 1) THEN                
         R(4) =(-P(1)*Q(1)-P(2)*Q(2)-P(3)*Q(3)+P(4)*Q(4))/AMQ  
         FAC  =-(R(4)+P(4))/(Q(4)+AMQ)       
      ELSE                                   
         WRITE(NOUT,*) ' ++++++++ WRONG MODE IN BOOST3 ' 
         STOP                                
      ENDIF                                  
      R(1)=P(1)+FAC*Q(1)                     
      R(2)=P(2)+FAC*Q(2)                     
      R(3)=P(3)+FAC*Q(3)                     
      END                                    


C BOOST ALONG X AXIS, EXE=EXP(ETA), ETA= HIPERBOLIC VELOCITY.
      SUBROUTINE BOSTD1(EXE,PVEC,QVEC)
C     ********************************
      IMPLICIT DOUBLE PRECISION(A-H,O-Z)
      DOUBLE PRECISION PVEC(4),QVEC(4),RVEC(4)
      DO 10 I=1,4
  10  RVEC(I)=PVEC(I)
      RPL=RVEC(4)+RVEC(1)
      RMI=RVEC(4)-RVEC(1)
      QPL=RPL*EXE
      QMI=RMI/EXE
      QVEC(2)=RVEC(2)
      QVEC(3)=RVEC(3)
      QVEC(1)=(QPL-QMI)/2
      QVEC(4)=(QPL+QMI)/2
      END

C BOOST ALONG Z AXIS, EXE=EXP(ETA), ETA= HIPERBOLIC VELOCITY.
      SUBROUTINE BXSTD3(EXE,PVEC,QVEC)
C     ********************************
      IMPLICIT DOUBLE PRECISION(A-H,O-Z)
      DOUBLE PRECISION PVEC(4),QVEC(4),RVEC(4)
      DO 10 I=1,4
  10  RVEC(I)=PVEC(I)
      RPL=RVEC(4)+RVEC(3)
      RMI=RVEC(4)-RVEC(3)
      QPL=RPL*EXE
      QMI=RMI/EXE
      QVEC(1)=RVEC(1)
      QVEC(2)=RVEC(2)
      QVEC(3)=(QPL-QMI)/2
      QVEC(4)=(QPL+QMI)/2
      END

      SUBROUTINE RXTOD1(PH1,PVEC,QVEC)
C     ********************************
      IMPLICIT DOUBLE PRECISION(A-H,O-Z)
      DOUBLE PRECISION PVEC(4),QVEC(4),RVEC(4)
      PHI=PH1
      CS=COS(PHI)
      SN=SIN(PHI)
      DO 10 I=1,4
  10  RVEC(I)=PVEC(I)
      QVEC(1)=RVEC(1)
      QVEC(2)= CS*RVEC(2)-SN*RVEC(3)
      QVEC(3)= SN*RVEC(2)+CS*RVEC(3)
      QVEC(4)=RVEC(4)
      END

      SUBROUTINE RXTOD2(PH1,PVEC,QVEC)
C     ********************************
      IMPLICIT DOUBLE PRECISION(A-H,O-Z)
      DOUBLE PRECISION PVEC(4),QVEC(4),RVEC(4)
      PHI=PH1
      CS=COS(PHI)
      SN=SIN(PHI)
      DO 10 I=1,4
  10  RVEC(I)=PVEC(I)
      QVEC(1)= CS*RVEC(1)+SN*RVEC(3)
      QVEC(2)=RVEC(2)
      QVEC(3)=-SN*RVEC(1)+CS*RVEC(3)
      QVEC(4)=RVEC(4)
      END

      SUBROUTINE RXTOD3(PH1,PVEC,QVEC)
C     ********************************
      IMPLICIT DOUBLE PRECISION(A-H,O-Z)
      DOUBLE PRECISION PVEC(4),QVEC(4),RVEC(4)
      PHI=PH1
      CS=COS(PHI)
      SN=SIN(PHI)
      DO 10 I=1,4
  10  RVEC(I)=PVEC(I)
      QVEC(1)= CS*RVEC(1)-SN*RVEC(2)
      QVEC(2)= SN*RVEC(1)+CS*RVEC(2)
      QVEC(3)=RVEC(3)
      QVEC(4)=RVEC(4)
      END

      FUNCTION ANGFIX(X,Y)
C     *******************
* CALCULATES ANGLE IN (0,2*PI) RANGE OUT OF X-Y
*     ***********************
      IMPLICIT DOUBLE PRECISION(A-H,O-Z)
      DATA PI /3.1415926535897932D0/

      IF(ABS(Y).LT.ABS(X)) THEN
        THE=ATAN(ABS(Y/X))
        IF(X.LE.0D0) THE=PI-THE
      ELSE
        THE=ACOS(X/SQRT(X**2+Y**2))
      ENDIF
      IF(Y.LT.0D0) THE=2D0*PI-THE
      ANGFIX=THE
      END

      SUBROUTINE DUMPT(NUNIT,WORD,PP)        
C     *******************************        
      IMPLICIT DOUBLE PRECISION(A-H,O-Z)               
      CHARACTER*8 WORD                       
      DOUBLE PRECISION PP(4)                           
      AMS=PP(4)**2-PP(3)**2-PP(2)**2-PP(1)**2  
      IF(AMS.GT.0.0) AMS=SQRT(AMS)           
      WRITE(NUNIT,'(1X,A8,5(1X,F13.8))') WORD,(PP(I),I=1,4),AMS 
C====================================================================== 
C================END OF YFSLIB========================================= 
C====================================================================== 
      END 
