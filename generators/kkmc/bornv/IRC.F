*////////////////////////////////////////////////////////////////////////////////////
*//                                                                                //
*//            Customization by S. Jadach, April 1999                              //
*//                                                                                //
*//     Prefix IRC_ added to all subrogram names                                   //
*//     Name /circom/ replaced with /c_IRC/                                        //
*//                                                                                //
*//                                                                                //
*//                                                                                //
*////////////////////////////////////////////////////////////////////////////////////

c circe.f -- canonical beam spectra for linear collider physics
c   Copyright (C) 1996,1997 by Thorsten.Ohl@Physik.TH-Darmstadt.de
c
c   Circe is free software; you can redistribute it and/or modify it
c   under the terms of the GNU General Public License as published by
c   the Free Software Foundation; either version 2, or (at your option)
c   any later version.
c
c   Circe is distributed in the hope that it will be useful, but
c   WITHOUT ANY WARRANTY; without even the implied warranty of
c   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
c   GNU General Public License for more details.
c
c   You should have received a copy of the GNU General Public License
c   along with this program; if not, write to the Free Software
c   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

      DOUBLE PRECISION FUNCTION IRC_circe (x1, x2, p1, p2)
      IMPLICIT NONE
      DOUBLE PRECISION x1, x2
      INTEGER p1, p2
      DOUBLE PRECISION IRC_circee, IRC_circeg, IRC_circgg
      INTEGER electr, positr, photon
      PARAMETER (electr =  11)
      PARAMETER (positr = -11)
      PARAMETER (photon =  22)
      INTEGER magic0
      PARAMETER (magic0 = 1904 06 16)
      DOUBLE PRECISION x1m, x2m, roots
      COMMON /c_IRC/ x1m, x2m, roots
      DOUBLE PRECISION lumi
      COMMON /c_IRC/ lumi
      DOUBLE PRECISION a1(0:7)
      COMMON /c_IRC/ a1
      INTEGER acc, ver, rev, chat
      COMMON /c_IRC/ acc, ver, rev, chat
      INTEGER magic
      COMMON /c_IRC/ magic
      INTEGER e, r
      COMMON /c_IRC/ e, r
      SAVE /c_IRC/
      IF (magic  .NE.  magic0) THEN
         CALL IRC_circes (-1d0, -1d0, -1d0, -1, -1, -1, -1)
      ENDIF
      IRC_circe = -1.0
      IF (ABS(p1)  .EQ.  electr) THEN
         IF (ABS(p2)  .EQ.  electr) THEN
            IRC_circe = IRC_circee (x1, x2)
         ELSEIF (p2  .EQ.  photon) THEN
            IRC_circe = IRC_circeg (x1, x2)
         ENDIF
      ELSEIF (p1  .EQ.  photon) THEN
         IF (ABS(p2)  .EQ.  electr) THEN
            IRC_circe = IRC_circeg (x2, x1)
         ELSEIF (p2  .EQ.  photon) THEN
            IRC_circe = IRC_circgg (x1, x2)
         ENDIF
      ENDIF
      END
      SUBROUTINE IRC_circes (xx1m, xx2m, xroots, xacc, xver, xrev, xchat)
      IMPLICIT NONE
      DOUBLE PRECISION xx1m, xx2m, xroots
      INTEGER xacc, xver, xrev, xchat
      INTEGER sband, tesla, xband
      PARAMETER (sband  =  1, tesla  =  2, xband  =  3)
      INTEGER sbndee, teslee, xbndee
      PARAMETER (sbndee =  4, teslee =  5, xbndee =  6)
      INTEGER nacc
      PARAMETER (nacc = 6)
      INTEGER magic0
      PARAMETER (magic0 = 1904 06 16)
      DOUBLE PRECISION x1m, x2m, roots
      COMMON /c_IRC/ x1m, x2m, roots
      DOUBLE PRECISION lumi
      COMMON /c_IRC/ lumi
      DOUBLE PRECISION a1(0:7)
      COMMON /c_IRC/ a1
      INTEGER acc, ver, rev, chat
      COMMON /c_IRC/ acc, ver, rev, chat
      INTEGER magic
      COMMON /c_IRC/ magic
      INTEGER e, r
      COMMON /c_IRC/ e, r
      SAVE /c_IRC/
      CHARACTER*60 msgbuf
      CHARACTER*6 accnam(nacc)
      INTEGER zver
      SAVE zver
      INTEGER ver34
      INTEGER gev350, gev500, gev800, tev1, tev16
      PARAMETER (gev350 = 1, gev500 = 2, gev800 = 3,
     $           tev1 =   4, tev16  = 5)
      INTEGER a1negy, a1nrev
      PARAMETER (a1negy = 5, a1nrev = 5)
      INTEGER i
      REAL xa1lum(a1negy,nacc,0:a1nrev)
      REAL xa1(0:7,a1negy,nacc,0:a1nrev)
      INTEGER a3negy, a3nrev
      PARAMETER (a3negy = 5, a3nrev = 5)
      REAL xa3lum(a3negy,nacc,0:a3nrev)
      REAL xa3(0:7,a3negy,nacc,0:a3nrev)
      INTEGER a5negy, a5nrev
      PARAMETER (a5negy = 5, a5nrev = 1)
      REAL xa5lum(a5negy,nacc,0:a5nrev)
      REAL xa5(0:7,a5negy,nacc,0:a5nrev)
      DATA accnam(sband)  /'sband'/
      DATA accnam(tesla)  /'tesla'/
      DATA accnam(xband)  /'xband'/
      DATA accnam(sbndee) /'sbndee'/
      DATA accnam(teslee) /'teslee'/
      DATA accnam(xbndee) /'xbndee'/
      DATA zver / -1 /
      DATA xa1lum(gev500,sband,1) /  5.212299e+01 /
      DATA (xa1(i,gev500,sband,1),i=0,7) /
     $    .39192e+00,   .66026e+00,   .11828e+02,  -.62543e+00, 
     $    .52292e+00,  -.69245e+00,   .14983e+02,   .65421e+00 /
      DATA xa1lum(gev500,tesla,1) /  6.066178e+01 /
      DATA (xa1(i,gev500,tesla,1),i=0,7) /
     $    .30196e+00,   .12249e+01,   .21423e+02,  -.57848e+00, 
     $    .68766e+00,  -.69788e+00,   .23121e+02,   .78399e+00 /
      DATA xa1lum(gev500,xband,1) /  5.884699e+01 /
      DATA (xa1(i,gev500,xband,1),i=0,7) /
     $    .48594e+00,   .52435e+00,   .83585e+01,  -.61347e+00, 
     $    .30703e+00,  -.68804e+00,   .84109e+01,   .44312e+00 /
      DATA xa1lum(tev1,sband,1)   /  1.534650e+02 /
      DATA (xa1(i,tev1,sband,1),i=0,7) /
     $    .24399e+00,   .87464e+00,   .66751e+01,  -.56808e+00, 
     $    .59295e+00,  -.68921e+00,   .94232e+01,   .83351e+00 /
      DATA xa1lum(tev1,tesla,1)   /  1.253381e+03 /
      DATA (xa1(i,tev1,tesla,1),i=0,7) /
     $    .39843e+00,   .70097e+00,   .11602e+02,  -.61061e+00, 
     $    .40737e+00,  -.69319e+00,   .14800e+02,   .51382e+00 /
      DATA xa1lum(tev1,xband,1)   /  1.901783e+02 /
      DATA (xa1(i,tev1,xband,1),i=0,7) /
     $    .32211e+00,   .61798e+00,   .28298e+01,  -.54644e+00, 
     $    .45674e+00,  -.67301e+00,   .41703e+01,   .74536e+00 /
      DATA (xa1lum(gev350,i,1),i=1,nacc) / nacc*-1d0 /
      DATA (xa1lum(gev800,i,1),i=1,nacc) / nacc*-1d0 /
      DATA (xa1lum(gev500,i,1),i=sbndee,nacc) / 3*-1d0 /
      DATA (xa1lum(tev1,i,1),i=sbndee,nacc) / 3*-1d0 /
      DATA (xa1lum(tev16,i,1),i=1,nacc) / 6*-1d0 /
      DATA xa1lum(gev500,sband,2) /   .31057e+02 /
      DATA (xa1(i,gev500,sband,2),i=0,7) /
     $    .38504e+00,   .79723e+00,   .14191e+02,  -.60456e+00, 
     $    .53411e+00,  -.68873e+00,   .15105e+02,   .65151e+00 /
      DATA xa1lum(tev1,sband,2) /   .24297e+03 /
      DATA (xa1(i,tev1,sband,2),i=0,7) /
     $    .24374e+00,   .89466e+00,   .70242e+01,  -.56754e+00, 
     $    .60910e+00,  -.68682e+00,   .96083e+01,   .83985e+00 /
      DATA xa1lum(gev350,tesla,2) /   .73369e+02 /
      DATA (xa1(i,gev350,tesla,2),i=0,7) /
     $    .36083e+00,   .12819e+01,   .37880e+02,  -.59492e+00, 
     $    .69109e+00,  -.69379e+00,   .40061e+02,   .65036e+00 /
      DATA xa1lum(gev500,tesla,2) /   .10493e+03 /
      DATA (xa1(i,gev500,tesla,2),i=0,7) /
     $    .29569e+00,   .11854e+01,   .21282e+02,  -.58553e+00, 
     $    .71341e+00,  -.69279e+00,   .24061e+02,   .77709e+00 /
      DATA xa1lum(gev800,tesla,2) /   .28010e+03 /
      DATA (xa1(i,gev800,tesla,2),i=0,7) /
     $    .22745e+00,   .11265e+01,   .10483e+02,  -.55711e+00, 
     $    .69579e+00,  -.69068e+00,   .13093e+02,   .89605e+00 /
      DATA xa1lum(tev1,tesla,2) /   .10992e+03 /
      DATA (xa1(i,tev1,tesla,2),i=0,7) /
     $    .40969e+00,   .66105e+00,   .11972e+02,  -.62041e+00, 
     $    .40463e+00,  -.69354e+00,   .14669e+02,   .51281e+00 /
      DATA xa1lum(gev500,xband,2) /   .35689e+02 /
      DATA (xa1(i,gev500,xband,2),i=0,7) /
     $    .48960e+00,   .46815e+00,   .75249e+01,  -.62769e+00, 
     $    .30341e+00,  -.68754e+00,   .85545e+01,   .43453e+00 /
      DATA xa1lum(tev1,xband,2) /   .11724e+03 /
      DATA (xa1(i,tev1,xband,2),i=0,7) /
     $    .31939e+00,   .62415e+00,   .30763e+01,  -.55314e+00, 
     $    .45634e+00,  -.67089e+00,   .41529e+01,   .73807e+00 /
      DATA xa1lum(gev350,sband,2) / -1d0 /
      DATA xa1lum(gev350,xband,2) / -1d0 /
      DATA xa1lum(gev800,sband,2) / -1d0 /
      DATA xa1lum(gev800,xband,2) / -1d0 /
      DATA (xa1lum(gev350,i,2),i=sbndee,nacc) / 3*-1d0 /
      DATA (xa1lum(gev500,i,2),i=sbndee,nacc) / 3*-1d0 /
      DATA (xa1lum(gev800,i,2),i=sbndee,nacc) / 3*-1d0 /
      DATA (xa1lum(tev1,i,2),i=sbndee,nacc) / 3*-1d0 /
      DATA (xa1lum(tev16,i,2),i=1,nacc) / 6*-1d0 /
      DATA xa1lum(gev500,sband, 3) /   .31469e+02 /
      DATA (xa1(i,gev500,sband, 3),i=0,7) /
     $    .38299e+00,   .72035e+00,   .12618e+02,  -.61611e+00, 
     $    .51971e+00,  -.68960e+00,   .15066e+02,   .63784e+00 /
      DATA xa1lum(tev1,  sband, 3) /   .24566e+03 /
      DATA (xa1(i,tev1,  sband, 3),i=0,7) /
     $    .24013e+00,   .95763e+00,   .69085e+01,  -.55151e+00, 
     $    .59497e+00,  -.68622e+00,   .94494e+01,   .82158e+00 /
      DATA xa1lum(gev350,tesla, 3) /   .74700e+02 /
      DATA (xa1(i,gev350,tesla, 3),i=0,7) /
     $    .34689e+00,   .12484e+01,   .33720e+02,  -.59523e+00, 
     $    .66266e+00,  -.69524e+00,   .38488e+02,   .63775e+00 /
      DATA xa1lum(gev500,tesla, 3) /   .10608e+03 /
      DATA (xa1(i,gev500,tesla, 3),i=0,7) /
     $    .28282e+00,   .11700e+01,   .19258e+02,  -.58390e+00, 
     $    .68777e+00,  -.69402e+00,   .23638e+02,   .75929e+00 /
      DATA xa1lum(gev800,tesla, 3) /   .28911e+03 /
      DATA (xa1(i,gev800,tesla, 3),i=0,7) /
     $    .21018e+00,   .12039e+01,   .96763e+01,  -.54024e+00, 
     $    .67220e+00,  -.69083e+00,   .12733e+02,   .87355e+00 /
      DATA xa1lum(tev1,  tesla, 3) /   .10936e+03 /
      DATA (xa1(i,tev1,  tesla, 3),i=0,7) /
     $    .41040e+00,   .68099e+00,   .11610e+02,  -.61237e+00, 
     $    .40155e+00,  -.69073e+00,   .14698e+02,   .49989e+00 /
      DATA xa1lum(gev500,xband, 3) /   .36145e+02 /
      DATA (xa1(i,gev500,xband, 3),i=0,7) /
     $    .51285e+00,   .45812e+00,   .75135e+01,  -.62247e+00, 
     $    .30444e+00,  -.68530e+00,   .85519e+01,   .43062e+00 /
      DATA xa1lum(tev1,  xband, 3) /   .11799e+03 /
      DATA (xa1(i,tev1,  xband, 3),i=0,7) /
     $    .31241e+00,   .61241e+00,   .29938e+01,  -.55848e+00, 
     $    .44801e+00,  -.67116e+00,   .41119e+01,   .72753e+00 /
      DATA xa1lum(gev350,sband,3) / -1d0 /
      DATA xa1lum(gev350,xband,3) / -1d0 /
      DATA xa1lum(gev800,sband,3) / -1d0 /
      DATA xa1lum(gev800,xband,3) / -1d0 /
      DATA (xa1lum(gev350,i,3),i=sbndee,nacc) / 3*-1d0 /
      DATA (xa1lum(gev500,i,3),i=sbndee,nacc) / 3*-1d0 /
      DATA (xa1lum(gev800,i,3),i=sbndee,nacc) / 3*-1d0 /
      DATA (xa1lum(tev1,i,3),i=sbndee,nacc) / 3*-1d0 /
      DATA (xa1lum(tev16,i,3),i=1,nacc) / 6*-1d0 /
      DATA xa1lum(gev500,sband, 4) /   .31528e+02 /
      DATA (xa1(i,gev500,sband, 4),i=0,7) /
     $    .38169e+00,   .73949e+00,   .12543e+02,  -.61112e+00, 
     $    .51256e+00,  -.69009e+00,   .14892e+02,   .63314e+00 /
      DATA xa1lum(tev1,  sband, 4) /   .24613e+03 /
      DATA (xa1(i,tev1,  sband, 4),i=0,7) /
     $    .24256e+00,   .94117e+00,   .66775e+01,  -.55160e+00, 
     $    .57484e+00,  -.68891e+00,   .92271e+01,   .81162e+00 /
      DATA xa1lum(gev350,tesla, 4) /   .74549e+02 /
      DATA (xa1(i,gev350,tesla, 4),i=0,7) /
     $    .34120e+00,   .12230e+01,   .32932e+02,  -.59850e+00, 
     $    .65947e+00,  -.69574e+00,   .38116e+02,   .63879e+00 /
      DATA xa1lum(gev500,tesla, 4) /   .10668e+03 /
      DATA (xa1(i,gev500,tesla, 4),i=0,7) /
     $    .28082e+00,   .11074e+01,   .18399e+02,  -.59118e+00, 
     $    .68880e+00,  -.69375e+00,   .23463e+02,   .76073e+00 /
      DATA xa1lum(gev800,tesla, 4) /   .29006e+03 /
      DATA (xa1(i,gev800,tesla, 4),i=0,7) /
     $    .21272e+00,   .11443e+01,   .92564e+01,  -.54657e+00, 
     $    .66799e+00,  -.69137e+00,   .12498e+02,   .87571e+00 /
      DATA xa1lum(tev1,  tesla, 4) /   .11009e+03 /
      DATA (xa1(i,tev1,  tesla, 4),i=0,7) /
     $    .41058e+00,   .64745e+00,   .11271e+02,  -.61996e+00, 
     $    .39801e+00,  -.69150e+00,   .14560e+02,   .49924e+00 /
      DATA xa1lum(gev500,xband, 4) /   .36179e+02 /
      DATA (xa1(i,gev500,xband, 4),i=0,7) /
     $    .51155e+00,   .43313e+00,   .70446e+01,  -.63003e+00, 
     $    .29449e+00,  -.68747e+00,   .83489e+01,   .42458e+00 /
      DATA xa1lum(tev1,  xband, 4) /   .11748e+03 /
      DATA (xa1(i,tev1,  xband, 4),i=0,7) /
     $    .32917e+00,   .54322e+00,   .28493e+01,  -.57959e+00, 
     $    .39266e+00,  -.68217e+00,   .38475e+01,   .68478e+00 /
      DATA xa1lum(gev350,sband,4) / -1d0 /
      DATA xa1lum(gev350,xband,4) / -1d0 /
      DATA xa1lum(gev800,sband,4) / -1d0 /
      DATA xa1lum(gev800,xband,4) / -1d0 /
      DATA (xa1lum(gev350,i,4),i=sbndee,nacc) / 3*-1d0 /
      DATA (xa1lum(gev500,i,4),i=sbndee,nacc) / 3*-1d0 /
      DATA (xa1lum(gev800,i,4),i=sbndee,nacc) / 3*-1d0 /
      DATA (xa1lum(tev1,i,4),i=sbndee,nacc) / 3*-1d0 /
      DATA (xa1lum(tev16,i,4),i=1,nacc) / 6*-1d0 /
      DATA xa1lum(gev350,sband, 5) /  0.21897e+02 /
      DATA (xa1(i,gev350,sband, 5),i=0,7) /
     $   0.57183e+00,  0.53877e+00,  0.19422e+02, -0.63064e+00, 
     $   0.49112e+00, -0.69109e+00,  0.24331e+02,  0.52718e+00 /
      DATA xa1lum(gev500,sband, 5) /  0.31383e+02 /
      DATA (xa1(i,gev500,sband, 5),i=0,7) /
     $   0.51882e+00,  0.49915e+00,  0.11153e+02, -0.63017e+00, 
     $   0.50217e+00, -0.69113e+00,  0.14935e+02,  0.62373e+00 /
      DATA xa1lum(gev800,sband, 5) /  0.95091e+02 /
      DATA (xa1(i,gev800,sband, 5),i=0,7) /
     $   0.47137e+00,  0.46150e+00,  0.56562e+01, -0.61758e+00, 
     $   0.46863e+00, -0.68897e+00,  0.85876e+01,  0.67577e+00 /
      DATA xa1lum(tev1,sband, 5) /  0.11900e+03 /
      DATA (xa1(i,tev1,sband, 5),i=0,7) /
     $   0.43956e+00,  0.45471e+00,  0.42170e+01, -0.61180e+00, 
     $   0.48711e+00, -0.68696e+00,  0.67145e+01,  0.74551e+00 /
      DATA xa1lum(tev16,sband, 5) /  0.11900e+03 /
      DATA (xa1(i,tev16,sband, 5),i=0,7) /
     $   0.43956e+00,  0.45471e+00,  0.42170e+01, -0.61180e+00, 
     $   0.48711e+00, -0.68696e+00,  0.67145e+01,  0.74551e+00 /
      DATA xa1lum(gev350,tesla, 5) /  0.97452e+02 /
      DATA (xa1(i,gev350,tesla, 5),i=0,7) /
     $   0.39071e+00,  0.84996e+00,  0.17614e+02, -0.60609e+00, 
     $   0.73920e+00, -0.69490e+00,  0.28940e+02,  0.77286e+00 /
      DATA xa1lum(gev500,tesla, 5) /  0.10625e+03 /
      DATA (xa1(i,gev500,tesla, 5),i=0,7) /
     $   0.42770e+00,  0.71457e+00,  0.15284e+02, -0.61664e+00, 
     $   0.68166e+00, -0.69208e+00,  0.24165e+02,  0.73806e+00 /
      DATA xa1lum(gev800,tesla, 5) /  0.17086e+03 /
      DATA (xa1(i,gev800,tesla, 5),i=0,7) /
     $   0.36025e+00,  0.69118e+00,  0.76221e+01, -0.59440e+00, 
     $   0.71269e+00, -0.69077e+00,  0.13117e+02,  0.91780e+00 /
      DATA xa1lum(tev1,tesla, 5) /  0.21433e+03 /
      DATA (xa1(i,tev1,tesla, 5),i=0,7) /
     $   0.33145e+00,  0.67075e+00,  0.55438e+01, -0.58468e+00, 
     $   0.72503e+00, -0.69084e+00,  0.99992e+01,  0.10112e+01 /
      DATA xa1lum(tev16,tesla, 5) /  0.34086e+03 /
      DATA (xa1(i,tev16,tesla, 5),i=0,7) /
     $   0.49058e+00,  0.42609e+00,  0.50550e+01, -0.61867e+00, 
     $   0.39225e+00, -0.68916e+00,  0.75514e+01,  0.58754e+00 /
      DATA xa1lum(gev350,xband, 5) /  0.31901e+02 /
      DATA (xa1(i,gev350,xband, 5),i=0,7) /
     $   0.65349e+00,  0.31752e+00,  0.94342e+01, -0.64291e+00, 
     $   0.30364e+00, -0.68989e+00,  0.11446e+02,  0.40486e+00 /
      DATA xa1lum(gev500,xband, 5) /  0.36386e+02 /
      DATA (xa1(i,gev500,xband, 5),i=0,7) /
     $   0.65132e+00,  0.28728e+00,  0.69853e+01, -0.64440e+00, 
     $   0.28736e+00, -0.68758e+00,  0.83227e+01,  0.41492e+00 /
      DATA xa1lum(gev800,xband, 5) /  0.10854e+03 /
      DATA (xa1(i,gev800,xband, 5),i=0,7) /
     $   0.49478e+00,  0.36221e+00,  0.30116e+01, -0.61548e+00, 
     $   0.39890e+00, -0.68418e+00,  0.45183e+01,  0.67243e+00 /
      DATA xa1lum(tev1,xband, 5) /  0.11899e+03 /
      DATA (xa1(i,tev1,xband, 5),i=0,7) /
     $   0.49992e+00,  0.34299e+00,  0.26184e+01, -0.61584e+00, 
     $   0.38450e+00, -0.68342e+00,  0.38589e+01,  0.67408e+00 /
      DATA xa1lum(tev16,xband, 5) /  0.13675e+03 /
      DATA (xa1(i,tev16,xband, 5),i=0,7) /
     $   0.50580e+00,  0.30760e+00,  0.18339e+01, -0.61421e+00, 
     $   0.35233e+00, -0.68315e+00,  0.26708e+01,  0.67918e+00 /
      DATA xa1lum(gev500,sbndee, 0) /   .92914e+01 /
      DATA (xa1(i,gev500,sbndee, 0),i=0,7) /
     $    .34866e+00,   .78710e+00,   .10304e+02,  -.59464e+00, 
     $    .40234e+00,  -.69741e+00,   .20645e+02,   .47274e+00 /
      DATA xa1lum(tev1,  sbndee, 0) /   .45586e+02 /
      DATA (xa1(i,tev1,  sbndee, 0),i=0,7) /
     $    .21084e+00,   .99168e+00,   .54407e+01,  -.52851e+00, 
     $    .47493e+00,  -.69595e+00,   .12480e+02,   .64027e+00 /
      DATA xa1lum(gev350,teslee, 0) /   .15175e+02 /
      DATA (xa1(i,gev350,teslee, 0),i=0,7) /
     $    .33093e+00,   .11137e+01,   .25275e+02,  -.59942e+00, 
     $    .49623e+00,  -.70403e+00,   .60188e+02,   .44637e+00 /
      DATA xa1lum(gev500,teslee, 0) /   .21622e+02 /
      DATA (xa1(i,gev500,teslee, 0),i=0,7) /
     $    .27175e+00,   .10697e+01,   .14858e+02,  -.58418e+00, 
     $    .50824e+00,  -.70387e+00,   .36129e+02,   .53002e+00 /
      DATA xa1lum(gev800,teslee, 0) /   .43979e+02 /
      DATA (xa1(i,gev800,teslee, 0),i=0,7) /
     $    .22994e+00,   .10129e+01,   .81905e+01,  -.55751e+00, 
     $    .46551e+00,  -.70461e+00,   .19394e+02,   .58387e+00 /
      DATA xa1lum(tev1,  teslee, 0) /   .25465e+02 /
      DATA (xa1(i,tev1,  teslee, 0),i=0,7) /
     $    .37294e+00,   .67522e+00,   .87504e+01,  -.60576e+00, 
     $    .35095e+00,  -.69821e+00,   .18526e+02,   .42784e+00 /
      DATA xa1lum(gev500,xbndee, 0) /   .13970e+02 /
      DATA (xa1(i,gev500,xbndee, 0),i=0,7) /
     $    .47296e+00,   .46800e+00,   .58897e+01,  -.61689e+00, 
     $    .27181e+00,  -.68923e+00,   .10087e+02,   .37462e+00 /
      DATA xa1lum(tev1,  xbndee, 0) /   .41056e+02 /
      DATA (xa1(i,tev1,  xbndee, 0),i=0,7) /
     $    .27965e+00,   .74816e+00,   .27415e+01,  -.50491e+00, 
     $    .38320e+00,  -.67945e+00,   .47506e+01,   .62218e+00 /
      DATA xa1lum(gev350,sbndee,0) / -1d0 /
      DATA xa1lum(gev350,xbndee,0) / -1d0 /
      DATA xa1lum(gev800,sbndee,0) / -1d0 /
      DATA xa1lum(gev800,xbndee,0) / -1d0 /
      DATA xa1lum(gev500,sband, 0) /   .31528e+02 /
      DATA (xa1(i,gev500,sband, 0),i=0,7) /
     $    .38169e+00,   .73949e+00,   .12543e+02,  -.61112e+00, 
     $    .51256e+00,  -.69009e+00,   .14892e+02,   .63314e+00 /
      DATA xa1lum(tev1,  sband, 0) /   .24613e+03 /
      DATA (xa1(i,tev1,  sband, 0),i=0,7) /
     $    .24256e+00,   .94117e+00,   .66775e+01,  -.55160e+00, 
     $    .57484e+00,  -.68891e+00,   .92271e+01,   .81162e+00 /
      DATA xa1lum(gev350,tesla, 0) /   .74549e+02 /
      DATA (xa1(i,gev350,tesla, 0),i=0,7) /
     $    .34120e+00,   .12230e+01,   .32932e+02,  -.59850e+00, 
     $    .65947e+00,  -.69574e+00,   .38116e+02,   .63879e+00 /
      DATA xa1lum(gev500,tesla, 0) /   .10668e+03 /
      DATA (xa1(i,gev500,tesla, 0),i=0,7) /
     $    .28082e+00,   .11074e+01,   .18399e+02,  -.59118e+00, 
     $    .68880e+00,  -.69375e+00,   .23463e+02,   .76073e+00 /
      DATA xa1lum(gev800,tesla, 0) /   .29006e+03 /
      DATA (xa1(i,gev800,tesla, 0),i=0,7) /
     $    .21272e+00,   .11443e+01,   .92564e+01,  -.54657e+00, 
     $    .66799e+00,  -.69137e+00,   .12498e+02,   .87571e+00 /
      DATA xa1lum(tev1,  tesla, 0) /   .11009e+03 /
      DATA (xa1(i,tev1,  tesla, 0),i=0,7) /
     $    .41058e+00,   .64745e+00,   .11271e+02,  -.61996e+00, 
     $    .39801e+00,  -.69150e+00,   .14560e+02,   .49924e+00 /
      DATA xa1lum(gev500,xband, 0) /   .36179e+02 /
      DATA (xa1(i,gev500,xband, 0),i=0,7) /
     $    .51155e+00,   .43313e+00,   .70446e+01,  -.63003e+00, 
     $    .29449e+00,  -.68747e+00,   .83489e+01,   .42458e+00 /
      DATA xa1lum(tev1,  xband, 0) /   .11748e+03 /
      DATA (xa1(i,tev1,  xband, 0),i=0,7) /
     $    .32917e+00,   .54322e+00,   .28493e+01,  -.57959e+00, 
     $    .39266e+00,  -.68217e+00,   .38475e+01,   .68478e+00 /
      DATA xa1lum(gev350,sband,0) / -1d0 /
      DATA xa1lum(gev350,xband,0) / -1d0 /
      DATA xa1lum(gev800,sband,0) / -1d0 /
      DATA xa1lum(gev800,xband,0) / -1d0 /
      DATA xa3lum(gev800,tesla, 3) /   .17196e+03 /
      DATA (xa3(i,gev800,tesla, 3),i=0,7) /
     $    .21633e+00,   .11333e+01,   .95928e+01,  -.55095e+00, 
     $    .73044e+00,  -.69101e+00,   .12868e+02,   .94737e+00 /
      DATA xa3lum(gev800,tesla, 4) /   .16408e+03 /
      DATA (xa3(i,gev800,tesla, 4),i=0,7) /
     $    .41828e+00,   .72418e+00,   .14137e+02,  -.61189e+00, 
     $    .36697e+00,  -.69205e+00,   .17713e+02,   .43583e+00 /
      DATA xa3lum(gev350,tesla, 5) /  0.66447e+02 /
      DATA (xa3(i,gev350,tesla, 5),i=0,7) /
     $   0.69418e+00,  0.50553e+00,  0.48430e+02, -0.63911e+00, 
     $   0.34074e+00, -0.69533e+00,  0.55502e+02,  0.29397e+00 /
      DATA xa3lum(gev500,tesla, 5) /  0.95241e+02 /
      DATA (xa3(i,gev500,tesla, 5),i=0,7) /
     $   0.64882e+00,  0.45462e+00,  0.27103e+02, -0.64535e+00, 
     $   0.35101e+00, -0.69467e+00,  0.33658e+02,  0.35024e+00 /
      DATA xa3lum(gev800,tesla, 5) /  0.16974e+03 /
      DATA (xa3(i,gev800,tesla, 5),i=0,7) /
     $   0.58706e+00,  0.43771e+00,  0.13422e+02, -0.63804e+00, 
     $   0.35541e+00, -0.69467e+00,  0.17528e+02,  0.43051e+00 /
      DATA xa3lum(tev1,tesla, 5) /  0.21222e+03 /
      DATA (xa3(i,tev1,tesla, 5),i=0,7) /
     $   0.55525e+00,  0.42577e+00,  0.96341e+01, -0.63587e+00, 
     $   0.36448e+00, -0.69365e+00,  0.13161e+02,  0.47715e+00 /
      DATA xa3lum(tev16,tesla, 5) /  0.34086e+03 /
      DATA (xa3(i,tev16,tesla, 5),i=0,7) /
     $   0.49058e+00,  0.42609e+00,  0.50550e+01, -0.61867e+00, 
     $   0.39225e+00, -0.68916e+00,  0.75514e+01,  0.58754e+00 /
      DATA xa3lum(gev350,tesla, 0) /  0.66447e+02 /
      DATA (xa3(i,gev350,tesla, 0),i=0,7) /
     $   0.69418e+00,  0.50553e+00,  0.48430e+02, -0.63911e+00, 
     $   0.34074e+00, -0.69533e+00,  0.55502e+02,  0.29397e+00 /
      DATA xa3lum(gev500,tesla, 0) /  0.95241e+02 /
      DATA (xa3(i,gev500,tesla, 0),i=0,7) /
     $   0.64882e+00,  0.45462e+00,  0.27103e+02, -0.64535e+00, 
     $   0.35101e+00, -0.69467e+00,  0.33658e+02,  0.35024e+00 /
      DATA xa3lum(gev800,tesla, 0) /  0.16974e+03 /
      DATA (xa3(i,gev800,tesla, 0),i=0,7) /
     $   0.58706e+00,  0.43771e+00,  0.13422e+02, -0.63804e+00, 
     $   0.35541e+00, -0.69467e+00,  0.17528e+02,  0.43051e+00 /
      DATA xa3lum(tev1,tesla, 0) /  0.21222e+03 /
      DATA (xa3(i,tev1,tesla, 0),i=0,7) /
     $   0.55525e+00,  0.42577e+00,  0.96341e+01, -0.63587e+00, 
     $   0.36448e+00, -0.69365e+00,  0.13161e+02,  0.47715e+00 /
      DATA xa3lum(tev16,tesla, 0) /  0.34086e+03 /
      DATA (xa3(i,tev16,tesla, 0),i=0,7) /
     $   0.49058e+00,  0.42609e+00,  0.50550e+01, -0.61867e+00, 
     $   0.39225e+00, -0.68916e+00,  0.75514e+01,  0.58754e+00 /
      DATA xa5lum(gev350,tesla, 1) /  -1.0 /
      DATA xa5lum(gev500,tesla, 1) /  0.33980e+03 /
      DATA (xa5(i,gev500,tesla, 1),i=0,7) /
     $   0.49808e+00,  0.54613e+00,  0.12287e+02, -0.62756e+00, 
     $   0.42817e+00, -0.69120e+00,  0.17067e+02,  0.51143e+00 /
      DATA xa5lum(gev800,tesla, 1) /  0.35936e+03 /
      DATA (xa5(i,gev800,tesla, 1),i=0,7) /
     $   0.58751e+00,  0.43128e+00,  0.13324e+02, -0.64006e+00, 
     $   0.30682e+00, -0.69235e+00,  0.16815e+02,  0.37078e+00 /
      DATA xa5lum(tev1,  tesla, 1) /  -1.0 /
      DATA xa5lum(tev16, tesla, 1) /  -1.0 /
      DATA xa5lum(gev350,tesla, 0) /  -1.0 /
      DATA xa5lum(gev500,tesla, 0) /  0.33980e+03 /
      DATA (xa5(i,gev500,tesla, 0),i=0,7) /
     $   0.49808e+00,  0.54613e+00,  0.12287e+02, -0.62756e+00, 
     $   0.42817e+00, -0.69120e+00,  0.17067e+02,  0.51143e+00 /
      DATA xa5lum(gev800,tesla, 0) /  0.35936e+03 /
      DATA (xa5(i,gev800,tesla, 0),i=0,7) /
     $   0.58751e+00,  0.43128e+00,  0.13324e+02, -0.64006e+00, 
     $   0.30682e+00, -0.69235e+00,  0.16815e+02,  0.37078e+00 /
      DATA xa5lum(tev1,  tesla, 0) /  -1.0 /
      DATA xa5lum(tev16, tesla, 0) /  -1.0 /

      IF (magic  .NE.  1904 06 16) THEN
         magic = 1904 06 16
      x1m = 0d0
      x2m = 0d0
      roots = 500d0
      acc = tesla
      ver = 0
      rev = 0
      chat = 1
      IF (xchat  .NE.  0) THEN
         CALL IRC_circem ('message', 'starting up ...')
         CALL IRC_circem ('message',
     $        '$id: circe.nw,v 1.28 1998/05/05 10:37:32 ohl exp $')
      ENDIF
      ENDIF
      IF ((xchat  .GE.  0)  .AND.  (xchat  .NE.  chat)) THEN
         chat = xchat
         IF (chat  .GE.  1) THEN
            WRITE (msgbuf, 1000) 'chat', chat
 1000       FORMAT ('updating `', a, ''' to ', i2)
            CALL IRC_circem ('message', msgbuf)
         ENDIF
      ELSE
         IF (chat  .GE.  2) THEN
            WRITE (msgbuf, 1100) 'chat', chat
 1100       FORMAT ('keeping `', a, ''' at ', i2)
            CALL IRC_circem ('message', msgbuf)
         ENDIF
      ENDIF
      IF ((xx1m  .GE.  0d0)  .AND.  (xx1m  .NE.  x1m)) THEN
         x1m = xx1m
         IF (chat  .GE.  1) THEN
            WRITE (msgbuf, 1001) 'x1min', x1m
 1001       FORMAT ('updating `', a, ''' to ', e12.4)
            CALL IRC_circem ('message', msgbuf)
         ENDIF
      ELSE
         IF (chat  .GE.  2) THEN
            WRITE (msgbuf, 1101) 'x1min', x1m
 1101       FORMAT ('keeping `', a, ''' at ', e12.4)
            CALL IRC_circem ('message', msgbuf)
         ENDIF
      ENDIF
      IF ((xx2m  .GE.  0d0)  .AND.  (xx2m  .NE.  x2m)) THEN
         x2m = xx2m
         IF (chat  .GE.  1) THEN
            WRITE (msgbuf, 1001) 'x2min', x2m
            CALL IRC_circem ('message', msgbuf)
         ENDIF
      ELSE
         IF (chat  .GE.  2) THEN
            WRITE (msgbuf, 1101) 'x2min', x2m
            CALL IRC_circem ('message', msgbuf)
         ENDIF
      ENDIF
      IF ((xroots  .GE.  0d0)  .AND. (xroots  .NE.  roots)) THEN
         roots = xroots
         IF (chat  .GE.  1) THEN
            WRITE (msgbuf, 1002) 'roots', roots
 1002       FORMAT ('updating `', a, ''' to ', f6.1)
            CALL IRC_circem ('message', msgbuf)
         ENDIF
      ELSE
         IF (chat  .GE.  2) THEN
            WRITE (msgbuf, 1102) 'roots', roots
 1102       FORMAT ('keeping `', a, ''' at ', f6.1)
            CALL IRC_circem ('message', msgbuf)
         ENDIF
      ENDIF
      IF ((xacc  .GE.  0)  .AND. (xacc  .NE.  acc)) THEN
         IF ((xacc  .GE.  1)  .AND.  (xacc  .LE.  nacc)) THEN
            acc = xacc
            IF (chat  .GE.  1) THEN
               WRITE (msgbuf, 1003) 'acc', accnam(acc)
 1003          FORMAT ('updating `', a, ''' to ', a)
               CALL IRC_circem ('message', msgbuf)
            ENDIF
         ELSE
            WRITE (msgbuf, 1203) xacc
 1203       FORMAT ('invalid `acc'': ', i8)
            CALL IRC_circem ('error', msgbuf)
            WRITE (msgbuf, 1103) 'acc', accnam(acc)
 1103       FORMAT ('keeping `', a, ''' at ', a)
            CALL IRC_circem ('message', msgbuf)
         ENDIF
      ELSE
         IF (chat  .GE.  2) THEN
            WRITE (msgbuf, 1003) 'acc', accnam(acc)
            CALL IRC_circem ('message', msgbuf)
         ENDIF
      ENDIF
      IF ((acc  .EQ.  sbndee)  .OR.  (acc  .EQ.  teslee)
     $     .OR.  (acc  .EQ.  xbndee)) THEN
      CALL IRC_circem ('warning', '***********************************')
      CALL IRC_circem ('warning', '* the accelerator PARAMETERs have *')
      CALL IRC_circem ('warning', '* not been ENDorsed for use in    *')
      CALL IRC_circem ('warning', '* an e-e- collider yet!!!         *')
      CALL IRC_circem ('warning', '***********************************')
      ENDIF
      IF ((xver  .GE.  0)  .AND.  (xver  .NE.  zver)) THEN
         ver = xver
         zver = xver
         IF (chat  .GE.  1) THEN
            WRITE (msgbuf, 1000) 'ver', ver
            CALL IRC_circem ('message', msgbuf)
         ENDIF
      ELSE
         IF (chat  .GE.  2) THEN
            WRITE (msgbuf, 1100) 'ver', ver
            CALL IRC_circem ('message', msgbuf)
         ENDIF
      ENDIF
      IF ((xrev  .GE.  0)  .AND. (xrev  .NE.  rev)) THEN
         rev = xrev
         IF (chat  .GE.  1) THEN
            WRITE (msgbuf, 1004) 'rev', rev
 1004       FORMAT ('updating `', a, ''' to ', i8)
            CALL IRC_circem ('message', msgbuf)
         ENDIF
      ELSE
         IF (chat  .GE.  2) THEN
            WRITE (msgbuf, 1104) 'rev', rev
 1104       FORMAT ('keeping `', a, ''' at ', i8)
            CALL IRC_circem ('message', msgbuf)
         ENDIF
      ENDIF
      ver34 = 0
      IF ((ver  .EQ.  1)  .OR.  (ver  .EQ.  0)) THEN
      IF (rev  .EQ.  0) THEN
         r = 0
      CALL IRC_circem ('warning', '*************************************')
      CALL IRC_circem ('warning', '* this release is not official yet, *')
      CALL IRC_circem ('warning', '* DO not use it in publications!    *')
      CALL IRC_circem ('warning', '*************************************')
      ELSEIF (rev  .GE.  1997 04 17) THEN
         r = 5
      ELSEIF (rev  .GE.  1996 09 02) THEN
         r = 4
      ELSEIF (rev  .GE.  1996 07 29) THEN
         r = 3
      ELSEIF (rev  .GE.  1996 07 11) THEN
         r = 2
      ELSEIF (rev  .GE.  1996 04 01) THEN
         r = 1
      ELSEIF (rev  .LT.  1996 04 01) THEN
         CALL IRC_circem ('error',
     $        'no revision of version 1 before 96/04/01 available')
         CALL IRC_circem ('message', 'falling back to default')
         r = 1
      ENDIF
      IF (chat  .GE.  2) THEN
         WRITE (msgbuf, 2000) rev, r
 2000    FORMAT ('mapping date ', i8, ' to revision index ', i2)
         CALL IRC_circem ('message', msgbuf)
      ENDIF
      IF (roots  .EQ.  350d0) THEN
         e = gev350
      ELSEIF ((roots  .GE.  340d0)  .AND.  (roots  .LE.  370d0)) THEN
         WRITE (msgbuf, 2001) roots, 350d0
         CALL IRC_circem ('message', msgbuf)
         e = gev350
      ELSEIF (roots  .EQ.  500d0) THEN
         e = gev500
      ELSEIF ((roots  .GE.  480d0)  .AND.  (roots  .LE.  520d0)) THEN
         WRITE (msgbuf, 2001) roots, 500d0
         CALL IRC_circem ('message', msgbuf)
         e = gev500
      ELSEIF (roots  .EQ.  800d0) THEN
         e = gev800
      ELSEIF ((roots  .GE.  750d0)  .AND.  (roots  .LE.  850d0)) THEN
         WRITE (msgbuf, 2001) roots, 800d0
         CALL IRC_circem ('message', msgbuf)
         e = gev800
      ELSEIF (roots  .EQ.  1000d0) THEN
         e = tev1
      ELSEIF ((roots  .GE.  900d0)  .AND.  (roots  .LE.  1100d0)) THEN
         WRITE (msgbuf, 2001) roots, 1000d0
         CALL IRC_circem ('message', msgbuf)
         e = tev1
      ELSEIF (roots  .EQ.  1600d0) THEN
         e = tev16
      ELSEIF ((roots  .GE.  1500d0)  .AND.  (roots  .LE.  1700d0)) THEN
         WRITE (msgbuf, 2001) roots, 1600d0
         CALL IRC_circem ('message', msgbuf)
         e = tev16
      ELSE
         CALL IRC_circem ('error',
     $        'only roots = 350, 500, 800, 1000 and 1600gev available')
         CALL IRC_circem ('message', 'falling back to 500gev')
         e = gev500
      ENDIF
      IF (xa1lum(e,acc,r)  .LT.  0d0) THEN
         WRITE (msgbuf, 2002) roots, accnam(acc), r
         CALL IRC_circem ('error', msgbuf)
         CALL IRC_circem ('message', 'falling back to 500gev')
         e = gev500
      ENDIF
      IF (chat  .GE.  2) THEN
         WRITE (msgbuf, 2003) roots, e
         CALL IRC_circem ('message', msgbuf)
      ENDIF
      lumi = xa1lum (e,acc,r)
      DO 10 i = 0, 7
         a1(i) = xa1(i,e,acc,r)
 10   CONTINUE
      ELSEIF ((ver  .EQ.  3)  .OR.  (ver  .EQ.  4)) THEN
         ver34 = ver
         ver = 1
      IF (rev  .EQ.  0) THEN
         r = 0
      CALL IRC_circem ('warning', '*************************************')
      CALL IRC_circem ('warning', '* this release is not official yet, *')
      CALL IRC_circem ('warning', '* DO not use it in publications!    *')
      CALL IRC_circem ('warning', '*************************************')
      ELSEIF (rev  .GE.  1997 04 17) THEN
         r = 5
         IF (ver34  .EQ.  3) THEN
            CALL IRC_circem ('warning', 'version 3 retired after 97/04/17')
            CALL IRC_circem ('message', 'falling back to version 4')
         ENDIF
      ELSEIF (rev  .GE.  1996 10 22) THEN
         r = ver34
         IF ((roots  .NE.  800d0)  .OR.  (acc  .NE.  tesla)) THEN
            CALL IRC_circem ('error', 'versions 3 and 4 before 97/04/17')
            CALL IRC_circem ('error', 'apply to tesla at 800 gev only')
            CALL IRC_circem ('message', 'falling back to tesla at 800gev')
            acc = tesla
            e = gev800
         ENDIF
      ELSEIF (rev  .LT.  1996 10 22) THEN
         CALL IRC_circem ('error',
     $     'no revision of versions 3 and 4 available before 96/10/22')
         CALL IRC_circem ('message', 'falling back to default')
         r = 5
      ENDIF
      IF (chat  .GE.  2) THEN
         WRITE (msgbuf, 2000) rev, r
         CALL IRC_circem ('message', msgbuf)
      ENDIF
      IF (roots  .EQ.  350d0) THEN
         e = gev350
      ELSEIF ((roots  .GE.  340d0)  .AND.  (roots  .LE.  370d0)) THEN
         WRITE (msgbuf, 2001) roots, 350d0
         CALL IRC_circem ('message', msgbuf)
         e = gev350
      ELSEIF (roots  .EQ.  500d0) THEN
         e = gev500
      ELSEIF ((roots  .GE.  480d0)  .AND.  (roots  .LE.  520d0)) THEN
         WRITE (msgbuf, 2001) roots, 500d0
         CALL IRC_circem ('message', msgbuf)
         e = gev500
      ELSEIF (roots  .EQ.  800d0) THEN
         e = gev800
      ELSEIF ((roots  .GE.  750d0)  .AND.  (roots  .LE.  850d0)) THEN
         WRITE (msgbuf, 2001) roots, 800d0
         CALL IRC_circem ('message', msgbuf)
         e = gev800
      ELSEIF (roots  .EQ.  1000d0) THEN
         e = tev1
      ELSEIF ((roots  .GE.  900d0)  .AND.  (roots  .LE.  1100d0)) THEN
         WRITE (msgbuf, 2001) roots, 1000d0
         CALL IRC_circem ('message', msgbuf)
         e = tev1
      ELSEIF (roots  .EQ.  1600d0) THEN
         e = tev16
      ELSEIF ((roots  .GE.  1500d0)  .AND.  (roots  .LE.  1700d0)) THEN
         WRITE (msgbuf, 2001) roots, 1600d0
         CALL IRC_circem ('message', msgbuf)
         e = tev16
      ELSE
         CALL IRC_circem ('error',
     $        'only roots = 350, 500, 800, 1000 and 1600gev available')
         CALL IRC_circem ('message', 'falling back to 500gev')
         e = gev500
      ENDIF
      IF (xa3lum(e,acc,r)  .LT.  0d0) THEN
         WRITE (msgbuf, 2002) roots, accnam(acc), r
         CALL IRC_circem ('error', msgbuf)
         CALL IRC_circem ('message', 'falling back to 500gev')
         e = gev500
      ENDIF
      IF (chat  .GE.  2) THEN
         WRITE (msgbuf, 2003) roots, e
         CALL IRC_circem ('message', msgbuf)
      ENDIF
      lumi = xa3lum (e,acc,r)
      DO 20 i = 0, 7
         a1(i) = xa3(i,e,acc,r)
 20   CONTINUE
      ELSEIF (ver  .EQ.  5) THEN
         ver = 1
      IF (rev  .EQ.  0) THEN
         r = 0
      CALL IRC_circem ('warning', '*************************************')
      CALL IRC_circem ('warning', '* this release is not official yet, *')
      CALL IRC_circem ('warning', '* DO not use it in publications!    *')
      CALL IRC_circem ('warning', '*************************************')
      ELSEIF (rev  .GE.  1998 05 05) THEN
         r = 1
      ELSEIF (rev  .LT.  1998 05 05) THEN
         CALL IRC_circem ('error',
     $     'no revision of version 5 available before 98/05/05')
         CALL IRC_circem ('message', 'falling back to default')
         r = 1
      ENDIF
      IF (chat  .GE.  2) THEN
         WRITE (msgbuf, 2000) rev, r
         CALL IRC_circem ('message', msgbuf)
      ENDIF
      IF (acc  .NE.  tesla) THEN
         CALL IRC_circem ('error', 'versions 5 applies to tesla only')
         acc = tesla
      END IF
      IF (roots  .EQ.  350d0) THEN
         e = gev350
      ELSEIF ((roots  .GE.  340d0)  .AND.  (roots  .LE.  370d0)) THEN
         WRITE (msgbuf, 2001) roots, 350d0
         CALL IRC_circem ('message', msgbuf)
         e = gev350
      ELSEIF (roots  .EQ.  500d0) THEN
         e = gev500
      ELSEIF ((roots  .GE.  480d0)  .AND.  (roots  .LE.  520d0)) THEN
         WRITE (msgbuf, 2001) roots, 500d0
         CALL IRC_circem ('message', msgbuf)
         e = gev500
      ELSEIF (roots  .EQ.  800d0) THEN
         e = gev800
      ELSEIF ((roots  .GE.  750d0)  .AND.  (roots  .LE.  850d0)) THEN
         WRITE (msgbuf, 2001) roots, 800d0
         CALL IRC_circem ('message', msgbuf)
         e = gev800
      ELSEIF (roots  .EQ.  1000d0) THEN
         e = tev1
      ELSEIF ((roots  .GE.  900d0)  .AND.  (roots  .LE.  1100d0)) THEN
         WRITE (msgbuf, 2001) roots, 1000d0
         CALL IRC_circem ('message', msgbuf)
         e = tev1
      ELSEIF (roots  .EQ.  1600d0) THEN
         e = tev16
      ELSEIF ((roots  .GE.  1500d0)  .AND.  (roots  .LE.  1700d0)) THEN
         WRITE (msgbuf, 2001) roots, 1600d0
         CALL IRC_circem ('message', msgbuf)
         e = tev16
      ELSE
         CALL IRC_circem ('error',
     $        'only roots = 350, 500, 800, 1000 and 1600gev available')
         CALL IRC_circem ('message', 'falling back to 500gev')
         e = gev500
      ENDIF
      IF (xa5lum(e,acc,r)  .LT.  0d0) THEN
         WRITE (msgbuf, 2002) roots, accnam(acc), r
         CALL IRC_circem ('error', msgbuf)
         CALL IRC_circem ('message', 'falling back to 500gev')
         e = gev500
      ENDIF
      IF (chat  .GE.  2) THEN
         WRITE (msgbuf, 2003) roots, e
         CALL IRC_circem ('message', msgbuf)
      ENDIF
      lumi = xa5lum (e,acc,r)
      DO 30 i = 0, 7
         a1(i) = xa5(i,e,acc,r)
 30   CONTINUE
      ELSEIF (ver  .EQ.  2) THEN
      CALL IRC_circem ('panic', '*********************************')
      CALL IRC_circem ('panic', '* version 2 has been retired,   *')
      CALL IRC_circem ('panic', '* please use version 1 instead! *')
      CALL IRC_circem ('panic', '*********************************')
      RETURN
      ELSEIF (ver  .GT.  5) THEN
         CALL IRC_circem ('panic', 'versions >5 not available yet')
         RETURN
      ELSE
         CALL IRC_circem ('panic', 'version must be positive')
         RETURN
      ENDIF
 2001 FORMAT ('treating energy ', f6.1, 'gev as ',  f6.1, 'gev')
 2002 FORMAT ('energy ', f6.1, ' not available for ', a6,
     $        ' in revison ', i2)
 2003 FORMAT ('mapping energy ', f6.1, ' to energy index ', i2)
      END
      SUBROUTINE IRC_circel (l)
      IMPLICIT NONE
      DOUBLE PRECISION l
      INTEGER magic0
      PARAMETER (magic0 = 1904 06 16)
      DOUBLE PRECISION x1m, x2m, roots
      COMMON /c_IRC/ x1m, x2m, roots
      DOUBLE PRECISION lumi
      COMMON /c_IRC/ lumi
      DOUBLE PRECISION a1(0:7)
      COMMON /c_IRC/ a1
      INTEGER acc, ver, rev, chat
      COMMON /c_IRC/ acc, ver, rev, chat
      INTEGER magic
      COMMON /c_IRC/ magic
      INTEGER e, r
      COMMON /c_IRC/ e, r
      SAVE /c_IRC/
      l = lumi
      END
      DOUBLE PRECISION FUNCTION IRC_circee (x1, x2)
      IMPLICIT NONE
      DOUBLE PRECISION x1, x2
      INTEGER magic0
      PARAMETER (magic0 = 1904 06 16)
      DOUBLE PRECISION x1m, x2m, roots
      COMMON /c_IRC/ x1m, x2m, roots
      DOUBLE PRECISION lumi
      COMMON /c_IRC/ lumi
      DOUBLE PRECISION a1(0:7)
      COMMON /c_IRC/ a1
      INTEGER acc, ver, rev, chat
      COMMON /c_IRC/ acc, ver, rev, chat
      INTEGER magic
      COMMON /c_IRC/ magic
      INTEGER e, r
      COMMON /c_IRC/ e, r
      SAVE /c_IRC/
      DOUBLE PRECISION d1, d2
      IF (magic  .NE.  magic0) THEN
         CALL IRC_circes (-1d0, -1d0, -1d0, -1, -1, -1, -1)
      ENDIF
      IRC_circee = -1.0
      IF ((ver  .EQ.  1)  .OR.  (ver  .EQ.  0)) THEN
      IF (x1  .EQ.  1d0) THEN
         d1 = a1(0)
      ELSEIF (x1  .LT.  1d0  .AND.  x1  .GT.  0d0) THEN
         d1 = a1(1) * x1**a1(2) * (1d0 - x1)**a1(3)
      ELSEIF (x1  .EQ.  -1d0) THEN
         d1 = 1d0 - a1(0)
      ELSE
         d1 = 0d0
      ENDIF
      IF (x2  .EQ.  1d0) THEN
         d2 = a1(0)
      ELSEIF (x2  .LT.  1d0  .AND.  x2  .GT.  0d0) THEN
         d2 = a1(1) * x2**a1(2) * (1d0 - x2)**a1(3)
      ELSEIF (x2  .EQ.  -1d0) THEN
         d2 = 1d0 - a1(0)
      ELSE
         d2 = 0d0
      ENDIF
      IRC_circee = d1 * d2
      ELSEIF (ver  .EQ.  2) THEN
      CALL IRC_circem ('panic', '*********************************')
      CALL IRC_circem ('panic', '* version 2 has been retired,   *')
      CALL IRC_circem ('panic', '* please use version 1 instead! *')
      CALL IRC_circem ('panic', '*********************************')
      RETURN
      ELSEIF (ver  .GT.  5) THEN
         CALL IRC_circem ('panic', 'versions >5 not available yet')
         RETURN
      ELSE
         CALL IRC_circem ('panic', 'version must be positive')
         RETURN
      ENDIF
      END
      DOUBLE PRECISION FUNCTION IRC_circeg (x1, x2)
      IMPLICIT NONE
      DOUBLE PRECISION x1, x2
      INTEGER magic0
      PARAMETER (magic0 = 1904 06 16)
      DOUBLE PRECISION x1m, x2m, roots
      COMMON /c_IRC/ x1m, x2m, roots
      DOUBLE PRECISION lumi
      COMMON /c_IRC/ lumi
      DOUBLE PRECISION a1(0:7)
      COMMON /c_IRC/ a1
      INTEGER acc, ver, rev, chat
      COMMON /c_IRC/ acc, ver, rev, chat
      INTEGER magic
      COMMON /c_IRC/ magic
      INTEGER e, r
      COMMON /c_IRC/ e, r
      SAVE /c_IRC/
      DOUBLE PRECISION d1, d2
      IF (magic  .NE.  magic0) THEN
         CALL IRC_circes (-1d0, -1d0, -1d0, -1, -1, -1, -1)
      ENDIF
      IRC_circeg = -1.0
      IF ((ver  .EQ.  1)  .OR.  (ver  .EQ.  0)) THEN
      IF (x1  .EQ.  1d0) THEN
         d1 = a1(0)
      ELSEIF (x1  .LT.  1d0  .AND.  x1  .GT.  0d0) THEN
         d1 = a1(1) * x1**a1(2) * (1d0 - x1)**a1(3)
      ELSEIF (x1  .EQ.  -1d0) THEN
         d1 = 1d0 - a1(0)
      ELSE
         d1 = 0d0
      ENDIF
      IF (x2  .LT.  1d0  .AND.  x2  .GT.  0d0) THEN
         d2 = a1(4) * x2**a1(5) * (1d0 - x2)**a1(6)
      ELSEIF (x2  .EQ.  -1d0) THEN
         d2 = a1(7)
      ELSE
         d2 = 0d0
      ENDIF
      IRC_circeg = d1 * d2
      ELSEIF (ver  .EQ.  2) THEN
      CALL IRC_circem ('panic', '*********************************')
      CALL IRC_circem ('panic', '* version 2 has been retired,   *')
      CALL IRC_circem ('panic', '* please use version 1 instead! *')
      CALL IRC_circem ('panic', '*********************************')
      RETURN
      ELSEIF (ver  .GT.  5) THEN
         CALL IRC_circem ('panic', 'versions >5 not available yet')
         RETURN
      ELSE
         CALL IRC_circem ('panic', 'version must be positive')
         RETURN
      ENDIF
      END
      DOUBLE PRECISION FUNCTION IRC_circgg (x1, x2)
      IMPLICIT NONE
      DOUBLE PRECISION x1, x2
      INTEGER magic0
      PARAMETER (magic0 = 1904 06 16)
      DOUBLE PRECISION x1m, x2m, roots
      COMMON /c_IRC/ x1m, x2m, roots
      DOUBLE PRECISION lumi
      COMMON /c_IRC/ lumi
      DOUBLE PRECISION a1(0:7)
      COMMON /c_IRC/ a1
      INTEGER acc, ver, rev, chat
      COMMON /c_IRC/ acc, ver, rev, chat
      INTEGER magic
      COMMON /c_IRC/ magic
      INTEGER e, r
      COMMON /c_IRC/ e, r
      SAVE /c_IRC/
      DOUBLE PRECISION d1, d2
      IF (magic  .NE.  magic0) THEN
         CALL IRC_circes (-1d0, -1d0, -1d0, -1, -1, -1, -1)
      ENDIF
      IRC_circgg = -1.0
      IF ((ver  .EQ.  1)  .OR.  (ver  .EQ.  0)) THEN
      IF (x1  .LT.  1d0  .AND.  x1  .GT.  0d0) THEN
         d1 = a1(4) * x1**a1(5) * (1d0 - x1)**a1(6)
      ELSEIF (x1  .EQ.  -1d0) THEN
         d1 = a1(7)
      ELSE
         d1 = 0d0
      ENDIF
      IF (x2  .LT.  1d0  .AND.  x2  .GT.  0d0) THEN
         d2 = a1(4) * x2**a1(5) * (1d0 - x2)**a1(6)
      ELSEIF (x2  .EQ.  -1d0) THEN
         d2 = a1(7)
      ELSE
         d2 = 0d0
      ENDIF
      IRC_circgg = d1 * d2
      ELSEIF (ver  .EQ.  2) THEN
      CALL IRC_circem ('panic', '*********************************')
      CALL IRC_circem ('panic', '* version 2 has been retired,   *')
      CALL IRC_circem ('panic', '* please use version 1 instead! *')
      CALL IRC_circem ('panic', '*********************************')
      RETURN
      ELSEIF (ver  .GT.  5) THEN
         CALL IRC_circem ('panic', 'versions >5 not available yet')
         RETURN
      ELSE
         CALL IRC_circem ('panic', 'version must be positive')
         RETURN
      ENDIF
      END
      SUBROUTINE IRC_girce (x1, x2, p1, p2, rng)
      IMPLICIT NONE
      DOUBLE PRECISION x1, x2
      INTEGER p1, p2
      EXTERNAL rng
      INTEGER magic0
      PARAMETER (magic0 = 1904 06 16)
      DOUBLE PRECISION x1m, x2m, roots
      COMMON /c_IRC/ x1m, x2m, roots
      DOUBLE PRECISION lumi
      COMMON /c_IRC/ lumi
      DOUBLE PRECISION a1(0:7)
      COMMON /c_IRC/ a1
      INTEGER acc, ver, rev, chat
      COMMON /c_IRC/ acc, ver, rev, chat
      INTEGER magic
      COMMON /c_IRC/ magic
      INTEGER e, r
      COMMON /c_IRC/ e, r
      SAVE /c_IRC/
      DOUBLE PRECISION u, w, IRC_circgg
      INTEGER electr, positr, photon
      PARAMETER (electr =  11)
      PARAMETER (positr = -11)
      PARAMETER (photon =  22)
      IF (magic  .NE.  magic0) THEN
         CALL IRC_circes (-1d0, -1d0, -1d0, -1, -1, -1, -1)
      ENDIF
 99   CONTINUE
      w = 1d0 / (1d0 + IRC_circgg (-1d0, -1d0))
      CALL rng (u)
      IF (u*u  .LE.  w) THEN
         p1 = positr
      ELSE
         p1 = photon
      ENDIF
      CALL rng (u)
      IF (u*u  .LE.  w) THEN
         p2 = electr
      ELSE
         p2 = photon
      ENDIF
      IF (ABS(p1)  .EQ.  electr) THEN
         IF (ABS(p2)  .EQ.  electr) THEN
            CALL IRC_gircee (x1, x2, rng)
         ELSEIF (p2  .EQ.  photon) THEN
            CALL IRC_girceg (x1, x2, rng)
         ENDIF
      ELSEIF (p1  .EQ.  photon) THEN
         IF (ABS(p2)  .EQ.  electr) THEN
            CALL IRC_girceg (x2, x1, rng)
         ELSEIF (p2  .EQ.  photon) THEN
            CALL IRC_gircgg (x1, x2, rng)
         ENDIF
      ENDIF
      IF ((x1  .LT.  x1m)  .OR.  (x2  .LT.  x2m)) GOTO 99
      END
      SUBROUTINE IRC_gircee (x1, x2, rng)
      IMPLICIT NONE
      DOUBLE PRECISION x1, x2
      EXTERNAL rng
      INTEGER magic0
      PARAMETER (magic0 = 1904 06 16)
      DOUBLE PRECISION x1m, x2m, roots
      COMMON /c_IRC/ x1m, x2m, roots
      DOUBLE PRECISION lumi
      COMMON /c_IRC/ lumi
      DOUBLE PRECISION a1(0:7)
      COMMON /c_IRC/ a1
      INTEGER acc, ver, rev, chat
      COMMON /c_IRC/ acc, ver, rev, chat
      INTEGER magic
      COMMON /c_IRC/ magic
      INTEGER e, r
      COMMON /c_IRC/ e, r
      SAVE /c_IRC/
      DOUBLE PRECISION u, IRC_girceb
      IF (magic  .NE.  magic0) THEN
         CALL IRC_circes (-1d0, -1d0, -1d0, -1, -1, -1, -1)
      ENDIF
      IF ((ver  .EQ.  1)  .OR.  (ver  .EQ.  0)) THEN
      CALL rng (u)
      IF (u  .LE.  a1(0)) THEN
         x1 = 1d0
      ELSE
         x1 = 1d0 - IRC_girceb (0d0, 1d0-x1m, a1(3)+1d0, a1(2)+1d0, rng)
      ENDIF
      CALL rng (u)
      IF (u  .LE.  a1(0)) THEN
         x2 = 1d0
      ELSE
         x2 = 1d0 - IRC_girceb (0d0, 1d0-x2m, a1(3)+1d0, a1(2)+1d0, rng)
      ENDIF
      ELSEIF (ver  .EQ.  2) THEN
      CALL IRC_circem ('panic', '*********************************')
      CALL IRC_circem ('panic', '* version 2 has been retired,   *')
      CALL IRC_circem ('panic', '* please use version 1 instead! *')
      CALL IRC_circem ('panic', '*********************************')
      RETURN
      ELSEIF (ver  .GT.  5) THEN
         CALL IRC_circem ('panic', 'versions >5 not available yet')
         RETURN
      ELSE
         CALL IRC_circem ('panic', 'version must be positive')
         RETURN
      ENDIF
      END
      SUBROUTINE IRC_girceg (x1, x2, rng)
      IMPLICIT NONE
      DOUBLE PRECISION x1, x2
      EXTERNAL rng
      INTEGER magic0
      PARAMETER (magic0 = 1904 06 16)
      DOUBLE PRECISION x1m, x2m, roots
      COMMON /c_IRC/ x1m, x2m, roots
      DOUBLE PRECISION lumi
      COMMON /c_IRC/ lumi
      DOUBLE PRECISION a1(0:7)
      COMMON /c_IRC/ a1
      INTEGER acc, ver, rev, chat
      COMMON /c_IRC/ acc, ver, rev, chat
      INTEGER magic
      COMMON /c_IRC/ magic
      INTEGER e, r
      COMMON /c_IRC/ e, r
      SAVE /c_IRC/
      DOUBLE PRECISION u, IRC_girceb
      IF (magic  .NE.  magic0) THEN
         CALL IRC_circes (-1d0, -1d0, -1d0, -1, -1, -1, -1)
      ENDIF
      IF ((ver  .EQ.  1)  .OR.  (ver  .EQ.  0)) THEN
      CALL rng (u)
      IF (u  .LE.  a1(0)) THEN
         x1 = 1d0
      ELSE
         x1 = 1d0 - IRC_girceb (0d0, 1d0-x1m, a1(3)+1d0, a1(2)+1d0, rng)
      ENDIF
      x2 = IRC_girceb (x2m, 1d0, a1(5)+1d0, a1(6)+1d0, rng)
      ELSEIF (ver  .EQ.  2) THEN
      CALL IRC_circem ('panic', '*********************************')
      CALL IRC_circem ('panic', '* version 2 has been retired,   *')
      CALL IRC_circem ('panic', '* please use version 1 instead! *')
      CALL IRC_circem ('panic', '*********************************')
      RETURN
      ELSEIF (ver  .GT.  5) THEN
         CALL IRC_circem ('panic', 'versions >5 not available yet')
         RETURN
      ELSE
         CALL IRC_circem ('panic', 'version must be positive')
         RETURN
      ENDIF
      END
      SUBROUTINE IRC_gircgg (x1, x2, rng)
      IMPLICIT NONE
      DOUBLE PRECISION x1, x2
      EXTERNAL rng
      INTEGER magic0
      PARAMETER (magic0 = 1904 06 16)
      DOUBLE PRECISION x1m, x2m, roots
      COMMON /c_IRC/ x1m, x2m, roots
      DOUBLE PRECISION lumi
      COMMON /c_IRC/ lumi
      DOUBLE PRECISION a1(0:7)
      COMMON /c_IRC/ a1
      INTEGER acc, ver, rev, chat
      COMMON /c_IRC/ acc, ver, rev, chat
      INTEGER magic
      COMMON /c_IRC/ magic
      INTEGER e, r
      COMMON /c_IRC/ e, r
      SAVE /c_IRC/
      DOUBLE PRECISION IRC_girceb
      IF (magic  .NE.  magic0) THEN
         CALL IRC_circes (-1d0, -1d0, -1d0, -1, -1, -1, -1)
      ENDIF
      IF ((ver  .EQ.  1)  .OR.  (ver  .EQ.  0)) THEN
      x1 = IRC_girceb (x1m, 1d0, a1(5)+1d0, a1(6)+1d0, rng)
      x2 = IRC_girceb (x2m, 1d0, a1(5)+1d0, a1(6)+1d0, rng)
      ELSEIF (ver  .EQ.  2) THEN
      CALL IRC_circem ('panic', '*********************************')
      CALL IRC_circem ('panic', '* version 2 has been retired,   *')
      CALL IRC_circem ('panic', '* please use version 1 instead! *')
      CALL IRC_circem ('panic', '*********************************')
      RETURN
      ELSEIF (ver  .GT.  5) THEN
         CALL IRC_circem ('panic', 'versions >5 not available yet')
         RETURN
      ELSE
         CALL IRC_circem ('panic', 'version must be positive')
         RETURN
      ENDIF
      END
      DOUBLE PRECISION FUNCTION IRC_girceb (xmin, xmax, a, b, rng)
      IMPLICIT NONE
      DOUBLE PRECISION xmin, xmax, a, b
      EXTERNAL rng
      DOUBLE PRECISION t, p, u, umin, umax, x, w
      IF ((a  .GT.  1d0)  .OR.  (b  .LT.  1d0)) THEN
         IRC_girceb = -1d0
         CALL IRC_circem ('error', 'beta-distribution expects a<=1<=b')
         RETURN
      ENDIF
      t = (1d0 - a) / (b + 1d0 - a)
      p = b*t / (b*t + a * (1d0 - t)**b)
      IF (xmin  .LE.  0d0) THEN
         umin = 0d0
      ELSEIF (xmin  .LT.  t) THEN
         umin = p * (xmin/t)**a
      ELSEIF (xmin  .EQ.  t) THEN
         umin = p
      ELSEIF (xmin  .LT.  1d0) THEN
         umin = 1d0 - (1d0 - p) * ((1d0 - xmin)/(1d0 - t))**b
      ELSE
         umin = 1d0
      ENDIF
      IF (xmax  .GE.  1d0) THEN
         umax = 1d0
      ELSEIF (xmax  .GT.  t) THEN
         umax = 1d0 - (1d0 - p) * ((1d0 - xmax)/(1d0 - t))**b
      ELSEIF (xmax  .EQ.  t) THEN
         umax = p
      ELSEIF (xmax  .GT.  0d0) THEN
         umax = p * (xmax/t)**a
      ELSE
         umax = 0d0
      ENDIF
      IF (umax  .LT.  umin) THEN
         IRC_girceb = -1d0
         RETURN
      ENDIF
 10   CONTINUE
      CALL rng (u)
      u = umin + (umax - umin) * u
      IF (u  .LE.  p) THEN
         x = t * (u/p)**(1d0/a)
         w = (1d0 - x)**(b-1d0)
      ELSE
         x = 1d0 - (1d0 - t) * ((1d0 - u)/(1d0 - p))**(1d0/b)
         w = (x/t)**(a-1d0)
      ENDIF
         CALL rng (u)
      IF (w  .LE.  u) GOTO 10
      IRC_girceb = x
      END
      SUBROUTINE IRC_circem (errlvl, errmsg)
      IMPLICIT NONE
      CHARACTER*(*) errlvl, errmsg
      INTEGER magic0
      PARAMETER (magic0 = 1904 06 16)
      DOUBLE PRECISION x1m, x2m, roots
      COMMON /c_IRC/ x1m, x2m, roots
      DOUBLE PRECISION lumi
      COMMON /c_IRC/ lumi
      DOUBLE PRECISION a1(0:7)
      COMMON /c_IRC/ a1
      INTEGER acc, ver, rev, chat
      COMMON /c_IRC/ acc, ver, rev, chat
      INTEGER magic
      COMMON /c_IRC/ magic
      INTEGER e, r
      COMMON /c_IRC/ e, r
      SAVE /c_IRC/
      INTEGER errcnt
      SAVE errcnt
      DATA errcnt /0/
      IF (errlvl  .EQ.  'message') THEN
         print *, 'circe:message: ', errmsg
      ELSEIF (errlvl  .EQ.  'warning') THEN
         IF (errcnt  .LT.  100) THEN
            errcnt = errcnt + 1
            print *, 'circe:warning: ', errmsg
         ELSEIF (errcnt  .EQ.  100) THEN
            errcnt = errcnt + 1
            print *, 'circe:message: more than 100 messages' 
            print *, 'circe:message: turning warnings off' 
         ENDIF
      ELSEIF (errlvl  .EQ.  'error') THEN
         IF (errcnt  .LT.  200) THEN
            errcnt = errcnt + 1
            print *, 'circe:error:   ', errmsg
         ELSEIF (errcnt  .EQ.  200) THEN
            errcnt = errcnt + 1
            print *, 'circe:message: more than 200 messages' 
            print *, 'circe:message: turning error messages off' 
         ENDIF
      ELSEIF (errlvl  .EQ.  'panic') THEN
         IF (errcnt  .LT.  300) THEN
            errcnt = errcnt + 1
            print *, 'circe:panic:   ', errmsg
         ELSEIF (errcnt  .EQ.  300) THEN
            errcnt = errcnt + 1
            print *, 'circe:message: more than 300 messages' 
            print *, 'circe:message: turning panic messages off' 
         ENDIF
      ELSE
         print *, 'circe:panic:    invalid error code ', errlvl
      ENDIF
      END


      SUBROUTINE IRC_GetParamee (Paramee)
*////////////////////////////////////////////////////////////////////////////////////
*//                                                                                //
*//   This is clone of circee  by S.J.                                             //
*//   Instead of function it provides 3 constants to build the function            //
*//                                                                                //
*////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      DOUBLE PRECISION Paramee(0:3)
*
      DOUBLE PRECISION x1, x2
      INTEGER magic0
      PARAMETER (magic0 = 1904 06 16)
      DOUBLE PRECISION x1m, x2m, roots
      COMMON /c_IRC/ x1m, x2m, roots
      DOUBLE PRECISION lumi
      COMMON /c_IRC/ lumi
      DOUBLE PRECISION a1(0:7)
      COMMON /c_IRC/ a1
      INTEGER acc, ver, rev, chat
      COMMON /c_IRC/ acc, ver, rev, chat
      INTEGER magic
      COMMON /c_IRC/ magic
      INTEGER e, r
      COMMON /c_IRC/ e, r
      SAVE /c_IRC/
      DOUBLE PRECISION d1, d2
      IF (magic  .NE.  magic0) THEN
         CALL IRC_circes (-1d0, -1d0, -1d0, -1, -1, -1, -1)
      ENDIF
      IF ((ver  .EQ.  1)  .OR.  (ver  .EQ.  0)) THEN
*(((((((((((((
******   d2 = a1(1) * x2**a1(2) * (1d0 - x2)**a1(3)
         Paramee(0) = a1(0)     ! Normalization of delta part
         Paramee(1) = a1(1)     ! Normalization of beta part
         Paramee(2) = a1(2)     ! Power at x=0, big one
         Paramee(3) = a1(3)     ! Power at x=1, small one
*))))))))))))))
      ELSEIF (ver  .EQ.  2) THEN
      CALL IRC_circem ('panic', '*********************************')
      CALL IRC_circem ('panic', '* version 2 has been retired,   *')
      CALL IRC_circem ('panic', '* please use version 1 instead! *')
      CALL IRC_circem ('panic', '*********************************')
      RETURN
      ELSEIF (ver  .GT.  5) THEN
         CALL IRC_circem ('panic', 'versions >5 not available yet')
         RETURN
      ELSE
         CALL IRC_circem ('panic', 'version must be positive')
         RETURN
      ENDIF
      END

