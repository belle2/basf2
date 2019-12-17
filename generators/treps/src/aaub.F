      SUBROUTINE R3EXEC(E,XM,PHI3EX)
CAAUBRN.  THE EVALUATION OF THE VOLUME OF THE PHASE SPACE OF N PARTICLESAAUB0000
C1   KAJANTIE, K., KARIMAKI, V.                                         AAUB0000
CREF. IN COMP. PHYS. COMMUN. 2 (1971) 207                               AAUB0000
C-----------------------------------------------------------------------AAUB0001
C     KB0013/VEIKKO KARIM#KI AND KEIJO KAJANTIE                         AAUB0002
C-----------------------------------------------------------------------AAUB0003
C
CC########      SUBROUTINE R3EXAC(E,XM,PHI3EX)                                    AAUB0071
C     CALLED FROM MAIN PROGRAM                                          AAUB0072
C     COMPUTE EXACT 3-PARTICLE PHASE SPACE VOLUME WHEN                  AAUB0073
C     EITHER BOTH OF THE MASSES XM(2) AND XM(3) ARE ZERO                AAUB0074
C     OR MASS XM(3) IS ZERO                                             AAUB0075
      DIMENSION XM(3)                                                   AAUB0076
      PI=3.14159265                                                     AAUB0077
      IF (XM(2)+XM(3)) 1,1,2                                            AAUB0078
C                                                                       AAUB0079
    1 S=E**2                                                            AAUB0080
      PHI3EX=PI**2*(S-XM(1)**4/S+2.*XM(1)**2*ALOG(XM(1)**2/S))/8.       AAUB0081
      RETURN                                                            AAUB0082
C                                                                       AAUB0083
    2 X1=XM(1)/E                                                        AAUB0084
      X2=XM(2)/E                                                        AAUB0085
      SQRLAM=SQRT((1.-(X1+X2)**2)*(1.-(X1-X2)**2))                      AAUB0086
      Y1=X1*X1                                                          AAUB0087
      Y2=X2*X2                                                          AAUB0088
      PHI3EX=E*E*.25*PI**2*(.5*(1.+Y1+Y2)*SQRLAM+ABS(Y1-Y2)*ALOG((Y1+Y2-AAUB0089
     2(Y1-Y2)**2+(Y1-Y2)*SQRLAM)/2./X1/X2)-(Y1+Y2-2.*Y1*Y2)*            AAUB0090
     3ALOG((1.-Y1-Y2+SQRLAM)/2./X1/X2))                                 AAUB0091
      RETURN                                                            AAUB0092
      END                                                               AAUB0093
      SUBROUTINE AAUBRN(E,XM,N,XRN)                                     AAUB0094
C     RN CALCULATES THE N PARTICLE PHASE SPACE VOLUME                   AAUB0095
C     E=TOTAL CM ENERGY, XRN IS THE VALUE OF THE PHASE SPACE INTEGRAL   AAUB0096
C     THE CALLING PROGRAM SHOULD CONTAIN THE MASSES IN THE VECTOR XM(N) AAUB0097
C     E AND XM(N) IN GEV UNITS                                          AAUB0098
C                                                                       AAUB0099
      DIMENSION XM(N),BETAST(10)                                        AAUB0100
      R2(X,Y,Z)=1.570796*SQRT((X**2-(Y-Z)**2)*(X**2-(Y+Z)**2))/X**2     AAUB0101
C                                                                       AAUB0102
C     LOGARITHM OF GAMMA FUNCTION                                       AAUB0103
C                                                                       AAUB0104
      ALNGAM(X)=-X+(X-.5)*ALOG(X)+.91894+ALOG(1.+1./12./X+1./288./X**2  AAUB0105
     2-139./51840./X**3)                                                AAUB0106
      PI=3.14159265                                                     AAUB0107
      AN=FLOAT(N)                                                       AAUB0108
      AL2PI=ALOG(2.*PI)                                                 AAUB0109
      PIM=1.                                                            AAUB0110
      SUMM=0.                                                           AAUB0111
C                                                                       AAUB0112
C     TEST THAT N, E AND THE MASSES ARE PHYSICAL                        AAUB0113
C                                                                       AAUB0114
      IF (N-2) 3,4,1                                                    AAUB0115
    1 DO 2 I=1,N                                                        AAUB0116
      IF (XM(I).LT.0.0) GO TO 3                                         AAUB0117
      PIM=PIM*XM(I)                                                     AAUB0118
    2 SUMM=SUMM+XM(I)                                                   AAUB0119
      IF (E-SUMM.LE.0.0) GO TO 3                                        AAUB0120
      IF (SUMM.LE.0.00001) SUMM=.0                                      AAUB0121
      IF (SUMM) 3,5,6                                                   AAUB0122
    3 XRN=.0                                                            AAUB0123
      RETURN                                                            AAUB0124
C                                                                       AAUB0125
C     USE EXACT FORMULA WHEN N=2                                        AAUB0126
C                                                                       AAUB0127
    4 XRN =R2(E,XM(1),XM(2))                                            AAUB0128
      RETURN                                                            AAUB0129
C                                                                       AAUB0130
C     USE EXACT FORMULA WHEN ALL MASSES ARE ZERO                        AAUB0131
C                                                                       AAUB0132
    5 ALGXRN=(AN-1.)*(AL2PI-1.3862944)+(2.*AN-4.)*ALOG(E)               AAUB0133
     2-2.*ALNGAM(AN)+ALOG(AN-1.)                                        AAUB0134
      GO TO 22                                                          AAUB0135
C                                                                       AAUB0136
C     INITIAL VALUE FOR BETA = ROUGHLY THE INVERSE OF ENERGY PER PAR-   AAUB0137
C     TICLE                                                             AAUB0138
C                                                                       AAUB0139
    6 BETA=1.5*AN/(E-SUMM)                                              AAUB0140
C                                                                       AAUB0141
C     LARGE BETA IMPLIES ONE IS NEAR THRESHOLD (E=SUMM). FOR SUMM*BETA  AAUB0142
C     .GT. 2000 AND ALL MASSES POSITIVE (PIM .GT. 0) THE EXACT NONRELA- AAUB0143
C     TIVISTIC FORM IS USED. NORMALLY THE PROGRAM GOES TO 9.            AAUB0144
C                                                                       AAUB0145
      IF (SUMM*BETA-2000.) 9,9,7                                        AAUB0146
    7 IF (PIM) 3,9,8                                                    AAUB0147
    8 ALGXRN=.5*ALOG(PIM)+1.5*(AN-1.)*AL2PI+(1.5*AN-2.5)*ALOG(E-SUMM)   AAUB0148
     2-.69315*AN-ALNGAM(1.5*(AN-1.))-1.5*ALOG(SUMM)                     AAUB0149
      GO TO 22                                                          AAUB0150
C                                                                       AAUB0151
C     SOLVE BETA BY NEWTON'S METHOD FROM THE TRANSCENDENTAL EQUATION    AAUB0152
C     FNBETA=0 IN LOOP 15. DFNBET IS THE DERIVATIVE OF FNBETA.          AAUB0153
C                                                                       AAUB0154
    9 Q=0.                                                              AAUB0155
      DO 15 J=1,10                                                      AAUB0156
      SUMR=0.                                                           AAUB0157
      SUMDR=0.                                                          AAUB0158
      BETAST(J)=BETA                                                    AAUB0159
      DO 12 I=1,N                                                       AAUB0160
      IF (XM(I).LT.0.00001) GO TO 12                                    AAUB0161
      IF (I.EQ.1) GO TO 10                                              AAUB0162
C                                                                       AAUB0163
C     IF XM(I)=XM(I+1) R AND DR ARE UNCHANGED                           AAUB0164
C                                                                       AAUB0165
      IF (XM(I)-XM(I-1).EQ.0.0) GO TO 11                                AAUB0166
   10 X=XM(I)*BETA                                                      AAUB0167
C                                                                       AAUB0168
C     RR AND XLNK1 ARE THE FUNCTIONS 1-K0(X)/K1(X) AND LN(X*K1(X)),     AAUB0169
C     WHERE K0 AND K1 ARE THE MODIFIED BESSEL FUNCTIONS OF ORDER 0 AND 1AAUB0170
C                                                                       AAUB0171
      CALL BKR(X,RR,XLNK1)                                              AAUB0172
      R=X*(1.-RR)                                                       AAUB0173
      DR=(2.*R+R**2)/BETA-XM(I)**2*BETA                                 AAUB0174
   11 SUMR=SUMR+R                                                       AAUB0175
      SUMDR=SUMDR+DR                                                    AAUB0176
   12 CONTINUE                                                          AAUB0177
      FNBETA=(2.*AN+SUMR)/BETA-E                                        AAUB0178
      DFNBET=(SUMDR-FNBETA-E)/BETA                                      AAUB0179
      BETA=BETA-FNBETA/DFNBET                                           AAUB0180
C                                                                       AAUB0181
C     SET NEW INITIAL VALUE FOR BETA IF BETA NEGATIVE OR ZERO           AAUB0182
C                                                                       AAUB0183
      IF (BETA) 13,13,14                                                AAUB0184
   13 Q=Q+1.                                                            AAUB0185
      BETA=Q/10.**IFIX(Q)                                               AAUB0186
      GO TO 15                                                          AAUB0187
C                                                                       AAUB0188
C     TEST FOR CONVERGENCE                                              AAUB0189
C                                                                       AAUB0190
   14 IF (ABS((BETAST(J)-BETA)/BETA).LT.0.000025) GO TO 16              AAUB0191
   15 CONTINUE                                                          AAUB0192
C                                                                       AAUB0193
C     INSERT BETA TO OBTAIN XRN. THE LOGARITHM OF THE ZEROTH ORDER      AAUB0194
C     APPROXIMATION TO THE VOLUME OF THE PHASE SPACE (ALNOM0) NEEDS THE AAUB0195
C     QUANTITIES Y,A AND SUMK1 = THE LOGARITHM OF THE PRODUCT OF THE    AAUB0196
C     QUANTITIES XM(I)*BETA*K1(XM(I)*BETA). THE REMAINING ARITHMETIC    AAUB0197
C     OPERATIONS ARE NEEDED TO CALCULATE GN GIVING THE FIRST ORDER      AAUB0198
C     APPROXIMATION EXP(ALGXRN)                                         AAUB0199
C                                                                       AAUB0200
   16 Y=BETA*E                                                          AAUB0201
      SUMK1=0.                                                          AAUB0202
      S1=0.                                                             AAUB0203
      S2=0.                                                             AAUB0204
      S3=0.                                                             AAUB0205
      S4=0.                                                             AAUB0206
      S5=0.                                                             AAUB0207
      S6=0.                                                             AAUB0208
      DO 19 I=1,N                                                       AAUB0209
      IF (XM(I).LT.0.00001) GO TO 19                                    AAUB0210
      IF (I.EQ.1) GO TO 17                                              AAUB0211
      IF (XM(I)-XM(I-1).EQ.0.0) GO TO 18                                AAUB0212
   17 X=XM(I)*BETA                                                      AAUB0213
      CALL BKR(X,RR,XLNK1)                                              AAUB0214
      W=RR*RR                                                           AAUB0215
      Z=XM(I)*XM(I)                                                     AAUB0216
   18 SUMK1=SUMK1+XLNK1                                                 AAUB0217
      S1=S1+Z*(2.*RR-W)                                                 AAUB0218
      S2=S2+Z*(-2.+6.*RR-3.*W)                                          AAUB0219
      S3=S3+Z*(1.-RR)*XM(I)*(2.*RR-W)                                   AAUB0220
      S4=S4+Z*(4.-6.*RR+3.*W)                                           AAUB0221
      S5=S5+Z*XM(I)*(1.-RR)*(.5-3.*RR+1.5*W)                            AAUB0222
      S6=S6+Z*Z*(RR-3.5*W+3.*RR*W-.75*W*W)                              AAUB0223
   19 CONTINUE                                                          AAUB0224
      A=4.*AN-Y+BETA*BETA*S1                                            AAUB0225
      IF (SUMM*BETA.GT.500.) GO TO 20                                   AAUB0226
      B=-4.*AN+BETA*BETA*S2+2.*BETA**3*S3                               AAUB0227
      C=12.*AN-BETA*BETA*S4-8.*BETA**3*S5+8.*BETA**4*S6                 AAUB0228
      F=5.*B*B/A**3+45.*(A+Y)**2/(Y*Y*A)-18.*(A+Y)*B/(Y*A*A)            AAUB0229
      D=C/A/A+6.*(3.*Y+3.*A-B)/Y/A+(A+Y)*(15.*A-6.*Y)/(A*Y*Y)           AAUB0230
      GN=D/8.-F/24.                                                     AAUB0231
      GO TO 21                                                          AAUB0232
   20 GN=-47./36./AN                                                    AAUB0233
   21 ALNOM0=(AN-2.)*AL2PI+2.5*ALOG(Y)+Y+SUMK1-4.*ALOG(E)               AAUB0234
     2-2.*AN*ALOG(BETA)-.5*ALOG(A)                                      AAUB0235
      ALGXRN=ALNOM0+ALOG(1.+GN)                                         AAUB0236
   22 XRN=EXP(ALGXRN)                                                   AAUB0237
      RETURN                                                            AAUB0238
      END                                                               AAUB0239
      SUBROUTINE BKR(X,RR,XLNK1)                                        AAUB0240
C     CALLED FROM SUBROUTINE RN                                         AAUB0241
C     SUBPROGRAM COMPUTES FOR THE ARGUMENT X THE FUNCTIONS (K1-K0)/K1   AAUB0242
C     AND LN(X*K1), WHERE K0 AND K1 ARE THE MODIFIED BESSEL             AAUB0243
C     FUNCTIONS OF ORDER 0 AND 1 RESPECTIVELY                           AAUB0244
C                                                                       AAUB0245
      DIMENSION T(12)                                                   AAUB0246
      IF (X-1.) 3,3,1                                                   AAUB0247
    1 B=1./X                                                            AAUB0248
      T(1)=B                                                            AAUB0249
      DO 2 L=2,12                                                       AAUB0250
    2 T(L)=T(L-1)*B                                                     AAUB0251
C                                                                       AAUB0252
C     COMPUTE (K1-K0)/K1 AND LN(X*K1) USING POLYNOMIAL APPROXIMATION    AAUB0253
C                                                                       AAUB0254
      G0=-.1566641816*T(1)+.0881112782*T(2)-.091390954*T(3)             AAUB0255
     2+.13445962*T(4)-.22998503*T(5)+.37924097*T(6)-.52472773*T(7)      AAUB0256
     3+.55753684*T(8)-.42626329*T(9)+.21845181*T(10)-.066809767*T(11)   AAUB0257
     4+.009189383*T(12)                                                 AAUB0258
C                                                                       AAUB0259
      G1=+.4699927013*T(1)-.146858296*T(2)+.12804266*T(3)               AAUB0260
     2-.17364316*T(4)+.28476181*T(5)-.45943421*T(6)+.62833807*T(7)      AAUB0261
     3-.66322954*T(8)+.50502386*T(9)-.25813038*T(10)+.078800012*T(11)   AAUB0262
     4-.010824177*T(12)                                                 AAUB0263
C                                                                       AAUB0264
      RR=(G1-G0)/(1.2533141373+G1)                                      AAUB0265
      XLNK1=-X+.5*ALOG(X)+ALOG(1.2533141373+G1)                         AAUB0266
      RETURN                                                            AAUB0267
C                                                                       AAUB0268
C     COMPUTE (K1-K0)/K1 AND LN(X*K1) USING SERIES EXPANSION FOR        AAUB0269
C     SMALL ARGUMENTS                                                   AAUB0270
C                                                                       AAUB0271
    3 B=X/2.                                                            AAUB0272
      A=.57721566+ALOG(B)                                               AAUB0273
      C=B*B                                                             AAUB0274
      G0=-A                                                             AAUB0275
      X2J=1.                                                            AAUB0276
      FACT=1.                                                           AAUB0277
      HJ=.0                                                             AAUB0278
      DO 4 J=1,6                                                        AAUB0279
      RJ=1./FLOAT(J)                                                    AAUB0280
      X2J=X2J*C                                                         AAUB0281
      FACT=FACT*RJ*RJ                                                   AAUB0282
      HJ=HJ+RJ                                                          AAUB0283
    4 G0=G0+X2J*FACT*(HJ-A)                                             AAUB0284
C                                                                       AAUB0285
      X2J=B                                                             AAUB0286
      FACT=1.                                                           AAUB0287
      HJ=1.                                                             AAUB0288
      G1=1./X+X2J*(.5+A-HJ)                                             AAUB0289
      DO 5 J=2,8                                                        AAUB0290
      X2J=X2J*C                                                         AAUB0291
      RJ=1./FLOAT(J)                                                    AAUB0292
      HJ=HJ+RJ                                                          AAUB0293
      FACT=FACT*RJ*RJ                                                   AAUB0294
    5 G1=G1+X2J*FACT*(.5+(A-HJ)*FLOAT(J))                               AAUB0295
C                                                                       AAUB0296
      RR=(G1-G0)/G1                                                     AAUB0297
      XLNK1=ALOG(X*G1)                                                  AAUB0298
      RETURN                                                            AAUB0299
      END                                                               AAUB0300
                                                                        AAUB****

