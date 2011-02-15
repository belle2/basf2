######################################
#
# Clean all G4 envs
unset  CLHEP_BASE_DIR
unset  CLHEP_INCLUDE_DIR
unset  CLHEP_LIB
unset  CLHEP_LIB_DIR

unset  G4DEBUG
unset  G4INCLUDE
unset  G4INSTALL

unset  G4LEDATA
unset  G4LEVELGAMMADATA
unset  G4NEUTRONHPDATA
unset  G4RADIOACTIVEDATA
unset  G4ABLADATA
unset  G4REALSURFACEDATA

unset  G4LIB
unset  G4LIB_BUILD_G3TOG4
unset  G4LIB_BUILD_SHARED
unset  G4LIB_BUILD_STATIC
unset  G4LIB_BUILD_ZLIB
unset  G4LIB_BUILD_GDML
unset  G4LIB_USE_G3TOG4
unset  G4LIB_USE_GRANULAR
unset  G4LIB_USE_ZLIB

unset  G4SYSTEM

unset  G4UI_NONE
unset  G4UI_BUILD_WIN32_SESSION
unset  G4UI_BUILD_XAW_SESSION
unset  G4UI_BUILD_XM_SESSION
unset  G4UI_USE_TCSH
unset  G4UI_USE_WIN32
unset  G4UI_USE_XAW
unset  G4UI_USE_XM
unset  G4UI_USE_QT


unset  G4VIS_NONE
unset  G4VIS_BUILD_DAWN_DRIVER
unset  G4VIS_BUILD_OIWIN32_DRIVER
unset  G4VIS_BUILD_OIX_DRIVER
unset  G4VIS_BUILD_OPENGLWIN32_DRIVER
unset  G4VIS_BUILD_OPENGLXM_DRIVER
unset  G4VIS_BUILD_OPENGLX_DRIVER
unset  G4VIS_BUILD_RAYTRACERX_DRIVER
unset  G4VIS_BUILD_VRML_DRIVER
unset  G4VIS_BUILD_OPENGLQT_DRIVER

unset  G4VIS_USE_DAWN
unset  G4VIS_USE_OIWIN32
unset  G4VIS_USE_OIX
unset  G4VIS_USE_OPENGLWIN32
unset  G4VIS_USE_OPENGLX
unset  G4VIS_USE_OPENGLXM
unset  G4VIS_USE_RAYTRACERX
unset  G4VIS_USE_VRML
unset  G4VIS_USE_OPENGLQT


#
# g4system.U
#
#+
G4SYSTEM="Linux-g++"
export G4SYSTEM
if [ X$g4non_display = X ] ; then
echo "On this machine the G4SYSTEM=$G4SYSTEM"
fi

#
# g4dirs.U
#
#+
G4INSTALL="${BELLE2_EXTERNALS_DIR}/geant4"
export G4INSTALL
if [ X$g4non_display = X ] ; then
echo "On this machine the G4INSTALL=$G4INSTALL"
fi

#+
G4INCLUDE="${BELLE2_EXTERNALS_DIR}/include/geant4"
export G4INCLUDE
if [ X$g4non_display = X ] ; then
echo "On this machine the G4INCLUDE=$G4INCLUDE"
fi

#+
G4LIB="${BELLE2_EXTERNALS_DIR}/lib/${BELLE2_SUBDIR}"
export G4LIB
if [ X$g4non_display = X ] ; then
echo "On this machine the G4LIB=$G4LIB"
fi

#+
if [ X${BELLE2_EXTERNALS_DIR}/share/geant4/data/PhotonEvaporation2.0 != X ] ; then 
G4LEVELGAMMADATA="${BELLE2_EXTERNALS_DIR}/share/geant4/data/PhotonEvaporation2.0"
export G4LEVELGAMMADATA
if [ X$g4non_display = X ] ; then
echo "On this machine the G4LEVELGAMMADATA=$G4LEVELGAMMADATA"
fi
fi

#+
if [ X${BELLE2_EXTERNALS_DIR}/share/geant4/data/RadioactiveDecay3.2 != X ] ; then 
G4RADIOACTIVEDATA="${BELLE2_EXTERNALS_DIR}/share/geant4/data/RadioactiveDecay3.2"
export G4RADIOACTIVEDATA
if [ X$g4non_display = X ] ; then
echo "On this machine the G4RADIOACTIVEDATA=$G4RADIOACTIVEDATA"
fi
fi

#+
if [ X${BELLE2_EXTERNALS_DIR}/share/geant4/data/G4EMLOW6.9 != X ] ; then 
G4LEDATA="${BELLE2_EXTERNALS_DIR}/share/geant4/data/G4EMLOW6.9"
export G4LEDATA
if [ X$g4non_display = X ] ; then
echo "On this machine the G4LEDATA=$G4LEDATA"
fi
fi

#+
if [ X${BELLE2_EXTERNALS_DIR}/share/geant4/data/G4NDL3.13 != X ] ; then 
G4NEUTRONHPDATA="${BELLE2_EXTERNALS_DIR}/share/geant4/data/G4NDL3.13"
export G4NEUTRONHPDATA
if [ X$g4non_display = X ] ; then
echo "On this machine the G4NEUTRONHPDATA=$G4NEUTRONHPDATA"
fi
fi

#+
if [ X${BELLE2_EXTERNALS_DIR}/share/geant4/data/G4ABLA3.0 != X ] ; then 
G4ABLADATA="${BELLE2_EXTERNALS_DIR}/share/geant4/data/G4ABLA3.0"
export G4ABLADATA
if [ X$g4non_display = X ] ; then
echo "On this machine the G4ABLADATA=$G4ABLADATA"
fi
fi

#+
if [ X${BELLE2_EXTERNALS_DIR}/share/geant4/data/RealSurface1.0 != X ] ; then 
G4REALSURFACEDATA="${BELLE2_EXTERNALS_DIR}/share/geant4/data/RealSurface1.0"
export G4REALSURFACEDATA
if [ X$g4non_display = X ] ; then
echo "On this machine the G4REALSURFACEDATA=$G4REALSURFACEDATA"
fi
fi

#
# g4clhep.U
#
CLHEP_BASE_DIR="${BELLE2_EXTERNALS_DIR}"
export CLHEP_BASE_DIR
if [ X$g4non_display = X ] ; then
echo "On this machine the CLHEP_BASE_DIR=$CLHEP_BASE_DIR"
fi

#+
if [ X${BELLE2_EXTERNALS_DIR}/include != X ] ; then 
CLHEP_INCLUDE_DIR="${BELLE2_EXTERNALS_DIR}/include"
export CLHEP_INCLUDE_DIR
if [ X$g4non_display = X ] ; then
echo "On this machine the CLHEP_INCLUDE_DIR=$CLHEP_INCLUDE_DIR"
fi
fi

#+
if [ X${BELLE2_EXTERNALS_DIR}/lib/${BELLE2_SUBDIR} != X ] ; then 
CLHEP_LIB_DIR="${BELLE2_EXTERNALS_DIR}/lib/${BELLE2_SUBDIR}"
export CLHEP_LIB_DIR
if [ X$g4non_display = X ] ; then
echo "On this machine the CLHEP_LIB_DIR=$CLHEP_LIB_DIR"
fi
fi

#+
if [ XCLHEP != X ] ; then 
CLHEP_LIB="CLHEP"
export CLHEP_LIB
if [ X$g4non_display = X ] ; then
echo "On this machine the CLHEP_LIB=$CLHEP_LIB"
fi
fi

#+
#
# g4debug
#
if [ Xn = Xy ] ; then 
G4DEBUG=1
export G4DEBUG
if [ X$g4non_display = X ] ; then
echo "On this machine the G4DEBUG=$G4DEBUG"
fi
fi


#
# g4ui
#
#+
if [ Xn = Xy ] ; then
G4UI_NONE=1
export G4UI_NONE
if [ X$g4non_display = X ] ; then
echo "On this machine, G4UI_NONE is set, so no user interfaces are available"
fi
fi


# Check for Windows!
if test "X$G4SYSTEM" != "XWIN32-VC" -a "X$G4SYSTEM" != "XWIN32-VC7"; then
#+
#
# UI_USE_TCSH for Unix
#
if [ Xn != Xy ] ; then
G4UI_USE_TCSH=1
export G4UI_USE_TCSH
if [ X$g4non_display = X ] ; then
echo "On this machine the G4UI_USE_TCSH=$G4UI_USE_TCSH"
fi
fi
fi


#+
if [ Xn = Xy ] ; then
G4UI_BUILD_XAW_SESSION=1
export G4UI_BUILD_XAW_SESSION
if [ X$g4non_display = X ] ; then
echo "On this machine the G4UI_BUILD_XAW_SESSION=$G4UI_BUILD_XAW_SESSION"
fi
fi 

#+
if [ Xn = Xy ] ; then
G4UI_USE_XAW=1
export G4UI_USE_XAW
if [ X$g4non_display = X ] ; then
echo "On this machine the G4UI_USE_XAW=$G4UI_USE_XAW"
fi
fi 

#+
if [ Xn = Xy ] ; then
G4UI_BUILD_XM_SESSION=1
export G4UI_BUILD_XM_SESSION
if [ X$g4non_display = X ] ; then
echo "On this machine the G4UI_BUILD_XM_SESSION=$G4UI_BUILD_XM_SESSION"
fi
fi 

#+
if [ Xn = Xy ] ; then
G4UI_USE_XM=1
export G4UI_USE_XM
if [ X$g4non_display = X ] ; then
echo "On this machine the G4UI_USE_XM=$G4UI_USE_XM"
fi
fi 

#+
if [ Xn = Xy ] ; then
G4UI_BUILD_WIN32_SESSION=1
export G4UI_BUILD_WIN32_SESSION
if [ X$g4non_display = X ] ; then
echo "On this machine the G4UI_BUILD_WIN32_SESSION=$G4UI_BUILD_WIN32_SESSION"
fi
fi 

#+
if [ Xn = Xy ] ; then
G4UI_USE_WIN32=1
export G4UI_USE_WIN32
if [ X$g4non_display = X ] ; then
echo "On this machine the G4UI_USE_WIN32=$G4UI_USE_WIN32"
fi
fi 


#+
if [ Xn = Xy ] ; then
G4UI_BUILD_QT_SESSION=1
export G4UI_BUILD_QT_SESSION
if [ X$g4non_display = X ] ; then
echo "On this machine the G4UI_BUILD_QT_SESSION=$G4UI_BUILD_QT_SESSION"
fi
fi 

#+
if [ Xn = Xy ] ; then
G4UI_USE_QT=1
export G4UI_USE_QT
if [ X$g4non_display = X ] ; then
echo "On this machine the G4UI_USE_QT=$G4UI_USE_QT"
fi
fi 



#
# g4vis
#
#+
if [ Xn = Xy ] ; then
G4VIS_NONE=1
export G4VIS_NONE
if [ X$g4non_display = X ] ; then
echo "On this machine G4VIS_NONE is set, so no vis drivers are available"
fi
fi

#+
if [ Xn = Xy ] ; then
G4VIS_BUILD_DAWN_DRIVER=1
export G4VIS_BUILD_DAWN_DRIVER
if [ X$g4non_display = X ] ; then
echo "On this machine the G4VIS_BUILD_DAWN_DRIVER=$G4VIS_BUILD_DAWN_DRIVER"
fi
fi 

#+
if [ Xn = Xy ] ; then
G4VIS_BUILD_OPENGLX_DRIVER=1
export G4VIS_BUILD_OPENGLX_DRIVER
if [ X$g4non_display = X ] ; then
echo "On this machine the G4VIS_BUILD_OPENGLX_DRIVER=$G4VIS_BUILD_OPENGLX_DRIVER"
fi
fi 

#+
if [ Xn = Xy ] ; then
G4VIS_BUILD_OPENGLXM_DRIVER=1
export G4VIS_BUILD_OPENGLXM_DRIVER
if [ X$g4non_display = X ] ; then
echo "On this machine the G4VIS_BUILD_OPENGLXM_DRIVER=$G4VIS_BUILD_OPENGLXM_DRIVER"
fi
fi 

#+
if [ Xn = Xy ] ; then
G4VIS_BUILD_OPENGLWIN32_DRIVER=1
export G4VIS_BUILD_OPENGLWIN32_DRIVER
if [ X$g4non_display = X ] ; then
echo "On this machine the G4VIS_BUILD_OPENGLWIN32_DRIVER=$G4VIS_BUILD_OPENGLWIN32_DRIVER"
fi
fi 

#+
if [ Xn = Xy ] ; then
G4VIS_BUILD_OIX_DRIVER=1
export G4VIS_BUILD_OIX_DRIVER
if [ X$g4non_display = X ] ; then
echo "On this machine the G4VIS_BUILD_OIX_DRIVER=$G4VIS_BUILD_OIX_DRIVER"
fi
fi 

#+
if [ Xn = Xy ] ; then
G4VIS_BUILD_RAYTRACERX_DRIVER=1
export G4VIS_BUILD_RAYTRACERX_DRIVER
if [ X$g4non_display = X ] ; then
echo "On this machine the G4VIS_BUILD_RAYTRACERX_DRIVER=$G4VIS_BUILD_RAYTRACERX_DRIVER"
fi
fi 

#+
if [ Xn = Xy ] ; then
G4VIS_BUILD_OIWIN32_DRIVER=1
export G4VIS_BUILD_OIWIN32_DRIVER
if [ X$g4non_display = X ] ; then
echo "On this machine the G4VIS_BUILD_OIWIN32_DRIVER=$G4VIS_BUILD_OIWIN32_DRIVER"
fi
fi 

#+
if [ Xn = Xy ] ; then
G4VIS_BUILD_VRML_DRIVER=1
export G4VIS_BUILD_VRML_DRIVER
if [ X$g4non_display = X ] ; then
echo "On this machine the G4VIS_BUILD_VRML_DRIVER=$G4VIS_BUILD_VRML_DRIVER"
fi
fi 

#+
if [ Xn = Xy ] ; then
G4VIS_BUILD_OPENGLQT_DRIVER=1
export G4VIS_BUILD_OPENGLQT_DRIVER
if [ X$g4non_display = X ] ; then
echo "On this machine the G4VIS_BUILD_OPENGLQT_DRIVER=$G4VIS_BUILD_OPENGLQT_DRIVER"
fi
fi 



#+
if [ Xn = Xy ] ; then
G4VIS_USE_DAWN=1
export G4VIS_USE_DAWN
if [ X$g4non_display = X ] ; then
echo "On this machine the G4VIS_USE_DAWN=$G4VIS_USE_DAWN"
fi
fi 

#+
if [ Xn = Xy ] ; then
G4VIS_USE_OPENGLX=1
export G4VIS_USE_OPENGLX
if [ X$g4non_display = X ] ; then
echo "On this machine the G4VIS_USE_OPENGLX=$G4VIS_USE_OPENGLX"
fi
fi 

#+
if [ Xn = Xy ] ; then
G4VIS_USE_OPENGLXM=1
export G4VIS_USE_OPENGLXM
if [ X$g4non_display = X ] ; then
echo "On this machine the G4VIS_USE_OPENGLXM=$G4VIS_USE_OPENGLXM"
fi
fi 

#+
if [ Xn = Xy ] ; then
G4VIS_USE_OPENGLWIN32=1
export G4VIS_USE_OPENGLWIN32
if [ X$g4non_display = X ] ; then
echo "On this machine the G4VIS_USE_OPENGLWIN32=$G4VIS_USE_OPENGLWIN32"
fi
fi 

#+
if [ Xn = Xy ] ; then
G4VIS_USE_OIX=1
export G4VIS_USE_OIX
if [ X$g4non_display = X ] ; then
echo "On this machine the G4VIS_USE_OIX=$G4VIS_USE_OIX"
fi
fi 

#+
if [ Xn = Xy ] ; then
G4VIS_USE_RAYTRACERX=1
export G4VIS_USE_RAYTRACERX
if [ X$g4non_display = X ] ; then
echo "On this machine the G4VIS_USE_RAYTRACERX=$G4VIS_USE_RAYTRACERX"
fi
fi 

#+
if [ Xn = Xy ] ; then
G4VIS_USE_OIWIN32=1
export G4VIS_USE_OIWIN32
if [ X$g4non_display = X ] ; then
echo "On this machine the G4VIS_USE_OIWIN32=$G4VIS_USE_OIWIN32"
fi
fi 

#+
if [ Xn = Xy ] ; then
G4VIS_USE_VRML=1
export G4VIS_USE_VRML
if [ X$g4non_display = X ] ; then
echo "On this machine the G4VIS_USE_VRML=$G4VIS_USE_VRML"
fi
fi 

#+
if [ Xn = Xy ] ; then
G4VIS_USE_OPENGLQT=1
export G4VIS_USE_OPENGLQT
if [ X$g4non_display = X ] ; then
echo "On this machine the G4VIS_USE_OPENGLQT=$G4VIS_USE_OPENGLQT"
fi
fi



#+
if [ X != X ] ; then
OGLHOME=""
export OGLHOME
if [ X$g4non_display = X ] ; then
echo "On this machine the OGLHOME=$OGLHOME"
fi
fi 

#+
if [ X != X ] ; then
OIVHOME=""
export OIVHOME
if [ X$g4non_display = X ] ; then
echo "On this machine the OIVHOME=$OIVHOME"
fi
fi 



#+
if [ Xn != X ] ; then
XMFLAGS=""
export XMFLAGS
if [ X$g4non_display = X ] ; then
echo "On this machine the XMFLAGS=$XMFLAGS"
fi
fi 

#+
if [ Xn != X ] ; then
XMLIBS=""
export XMLIBS
if [ X$g4non_display = X ] ; then
echo "On this machine the XMLIBS=$XMLIBS"
fi
fi 

#+
if [ Xn != X ] ; then
XMFLAGS=""
export XMFLAGS
if [ X$g4non_display = X ] ; then
echo "On this machine the XMFLAGS=$XMFLAGS"
fi
fi 

#+
if [ X != X ] ; then
XMLIBS=""
export XMLIBS
if [ X$g4non_display = X ] ; then
echo "On this machine the XMLIBS=$XMLIBS"
fi
fi 

#+
if [ Xn != X ] ; then
XAWFLAGS=""
export XAWFLAGS
if [ X$g4non_display = X ] ; then
echo "On this machine the XAWFLAGS=$XAWFLAGS"
fi
fi 

#+
if [ Xn != X ] ; then
XAWLIBS=""
export XAWLIBS
if [ X$g4non_display = X ] ; then
echo "On this machine the XAWLIBS=$XAWLIBS"
fi
fi 


#
# Qt Flags and Libs, messy, but needed for backward compatibility
#+
if [ "Xn" = "Xy" -o "Xn" = "Xy" ] ; then
QTFLAGS=""
QTLIBS=""
QTMOC=""
export QTFLAGS
export QTLIBS
export QTMOC
if test "x$g4non_display" = "x" ; then
echo "On this machine the QTFLAGS=$QTFLAGS"
echo "On this machine the QTLIBS=$QTLIBS"
echo "On this machine the QTMOC=$QTMOC"
fi
fi

if [ "Xn" = "Xy" -o "Xn" = "Xy" ] ; then
if [ "X$QTFLAGS" = "X" ] ; then
QTFLAGS=" "
else
QTFLAGS="$QTFLAGS "
fi
if [ "X$QTMOC" = "X" ] ; then
QTMOC=""
fi
GLQTLIBS=" "
export QTFLAGS
export QTMOC
export GLQTLIBS
if test "x$g4non_display" = "x" ; then
echo "On this machine the QTFLAGS=$QTFLAGS"
echo "On this machine the GLQTLIBS=$GLQTLIBS"
echo "On this machine the QTMOC=$QTMOC"
fi
fi



#
# Use GDML module
#
#+
if [ Xn = Xy ] ; then
G4LIB_BUILD_GDML=1
export G4LIB_BUILD_GDML
if test "x$g4non_display" = "x" ; then
echo "On this machine the G4LIB_BUILD_GDML=$G4LIB_BUILD_GDML"
fi
fi 

if [ Xn = Xy ] ; then
XERCESCROOT=
export XERCESCROOT
if test "x$g4non_display" = "x" ; then
echo "On this machine the XERCESCROOT=$XERCESCROOT"
fi
fi 



#
# Use G3TOG4 module
#
#+
if [ Xn = Xy ] ; then
G4LIB_BUILD_G3TOG4=1
export G4LIB_BUILD_G3TOG4
if [ X$g4non_display = X ] ; then
echo "On this machine the G4LIB_BUILD_G3TOG4=$G4LIB_BUILD_G3TOG4"
fi
fi 

if [ Xn = Xy ] ; then
G4LIB_USE_G3TOG4=1
export G4LIB_USE_G3TOG4
if [ X$g4non_display = X ] ; then
echo "On this machine the G4LIB_USE_G3TOG4=$G4LIB_USE_G3TOG4"
fi
fi 

#
# Use ZLIB module
#
#+
if [ Xn = Xy ] ; then
G4LIB_BUILD_ZLIB=1
export G4LIB_BUILD_ZLIB
if [ X$g4non_display = X ] ; then
echo "On this machine the G4LIB_BUILD_ZLIB=$G4LIB_BUILD_ZLIB"
fi
fi 

if [ Xn = Xy ] ; then
G4LIB_USE_ZLIB=1
export G4LIB_USE_ZLIB
if [ X$g4non_display = X ] ; then
echo "On this machine the G4LIB_USE_ZLIB=$G4LIB_USE_ZLIB"
fi
fi 


#+
#
# g4shared
#
if [ Xy = Xy ] ; then
G4LIB_BUILD_SHARED=1
export G4LIB_BUILD_SHARED
if [ X$g4non_display = X ] ; then
echo "On this machine the G4LIB_BUILD_SHARED=$G4LIB_BUILD_SHARED"
fi
fi 

if [ Xn = Xy ] ; then
G4LIB_BUILD_STATIC=1
export G4LIB_BUILD_STATIC
if [ X$g4non_display = X ] ; then
echo "On this machine the G4LIB_BUILD_STATIC=$G4LIB_BUILD_STATIC"
fi
fi 

#+
#
# g4granular
#
if [ Xn = Xy ] ; then
G4LIB_USE_GRANULAR=1
export G4LIB_USE_GRANULAR
if [ X$g4non_display = X ] ; then
echo "On this machine the G4LIB_USE_GRANULAR=$G4LIB_USE_GRANULAR"
fi
fi 


#####################################################################



#+
#
# G4WORKDIR
#
if [ $G4WORKDIR ] ; then

if [ X\$g4non_display = X ] ; then
echo "In your environment you have the G4WORKDIR=$G4WORKDIR"
fi

else

# Check for Windows!
if test "X$G4SYSTEM" = "XWIN32-VC" -o "X$G4SYSTEM" = "XWIN32-VC7"; then

if [ X\$g4non_display = X ] ; then
echo "G4WORKDIR will be set to c:/geant4 (in "native" Windows syntax)"
fi

G4WORKDIR="c:/geant4"
export G4WORKDIR

else # if Unix

if [ X\$g4non_display = X ] ; then
echo "G4WORKDIR will be set to HOME/geant4=$HOME/geant4"
fi

G4WORKDIR=$HOME/geant4
export G4WORKDIR

fi # if platforms

fi




#
# Shared libraries
#
if [ $G4LIB_BUILD_SHARED ] ; then
#
# Warning about LD_LIBRARY_PATH unless g4non_display is set!
#
if test "x\$g4non_display" = "x" ; then
    echo ''
    echo 'LD_LIBRARY_PATH is set to include CLHEP and Geant4 libraries. '
    echo ''
    echo 'NOTE : verify that the correct path for the CLHEP library'
    echo '- $CLHEP_BASE_DIR/lib - is included in the $LD_LIBRARY_PATH'
    echo 'variable in your environment, and no other outdated installations'
    echo 'of CLHEP are by chance referenced through $LD_LIBRARY_PATH and'
    echo 'present in your system !'
    echo ''
fi

if [ $LD_LIBRARY_PATH ] ; then
LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${G4LIB}/${G4SYSTEM}
LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${CLHEP_LIB_DIR}

if [ $G4LIB_BUILD_GDML ] ; then
   LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${XERCESCROOT}/lib
fi
export LD_LIBRARY_PATH

else
LD_LIBRARY_PATH=${G4LIB}/${G4SYSTEM}
LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${CLHEP_LIB_DIR}

if [ $G4LIB_BUILD_GDML ] ; then
   LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${XERCESCROOT}/lib
fi

export LD_LIBRARY_PATH
fi
fi

#
# Shared libraries for Mac
#
if test "X$G4SYSTEM" = "XDarwin-g++" ; then

if [ $G4LIB_BUILD_SHARED ] ; then
if [ $DYLD_LIBRARY_PATH ] ; then
DYLD_LIBRARY_PATH=${DYLD_LIBRARY_PATH}:${G4LIB}/${G4SYSTEM}
DYLD_LIBRARY_PATH=${DYLD_LIBRARY_PATH}:${CLHEP_LIB_DIR}

if [ $G4LIB_BUILD_GDML ] ; then
   DYLD_LIBRARY_PATH=${DYLD_LIBRARY_PATH}:${XERCESCROOT}/lib
fi

export DYLD_LIBRARY_PATH
else
DYLD_LIBRARY_PATH=${G4LIB}/${G4SYSTEM}
DYLD_LIBRARY_PATH=${DYLD_LIBRARY_PATH}:${CLHEP_LIB_DIR}

if [ $G4LIB_BUILD_GDML ] ; then
   DYLD_LIBRARY_PATH=${DYLD_LIBRARY_PATH}:${XERCESCROOT}/lib
fi

export DYLD_LIBRARY_PATH
fi
fi

fi

#
# For BIN
#
PATH=${PATH}:${G4WORKDIR}/bin/${G4SYSTEM}
export PATH

