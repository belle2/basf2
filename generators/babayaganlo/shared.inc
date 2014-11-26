      implicit real*8 (a-h,o-z)
      parameter (nbin   = 200)
      parameter (ndistr =  15)
      common/ecms/ecms
      common/epssoft/eps
      common/expcuts/thmin,thmax,emin,zmax,egmin,thgmin,thgmax
      common/angularranges/thmine,thmaxe,thminp,thmaxp
      dimension p3(0:3),p4(0:3),qph(40,0:3)

