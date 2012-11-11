#!/bin/bash
# basf2helper - performs top and tail work to allow basf2 to function well 
#               in a grid environment
# $1 = steering file
# $2 = release
# Tom Fifield - 2010-01
# Yanliang Han -2011-11
##############################################################################
# Initialisation
##############################################################################

ls /cvmfs/belle.cern.ch > cvmfs.error 2>&1
if [ $? -ne 0 ]; then
  VO_BELLE_SW_DIR=${VO_BELLE_SW_DIR:-$OSG_APP/belle}
else
  VO_BELLE_SW_DIR=${VO_BELLE_SW_DIR:-/cvmfs/belle.cern.ch}
fi

unset VO_BELLE2_SW_DIR


#avoid basf2 tools check
export BELLE2_NO_TOOLS_CHECK=1

. ${VO_BELLE_SW_DIR}/belle2/tools/setup_belle2.sh
WORKDIR=$PWD

############setup central release
cd ${VO_BELLE_SW_DIR}/belle2/releases/$2
setuprel $2


##set up local release
###mkdir release
###cd release
###cp ${VO_BELLE_SW_DIR}/belle2/releases/$2/.release .
###cp ${VO_BELLE_SW_DIR}/belle2/releases/$2/.externals .
###cp ${VO_BELLE_SW_DIR}/belle2/releases/$2/site_scons . -r
###ln -s site_scons/SConstruct SConstruct
###setuprel
###
#####copy source code to local release directory and compile it
###cp ${WORKDIR}/$3-inputsandbox.tar.bz2 .
###tar -jxvf $3-inputsandbox.tar.bz2
###scons --local

cd $WORKDIR

# Modify output filename of parametric job
GBASF2_PARAMETER=${GBASF2_PARAMETER:--1}
if [ $GBASF2_PARAMETER -ge 0 ]; then
cat<<EOF>tmp.awk
\$0 ~ "outputFileName" {sfx=\$2;
sub(/\.[A-Za-z0-9]+')$/,"-"$GBASF2_PARAMETER,\$2);
sub(/^.+\./,".",sfx);
print \$1 \$2 sfx}
\$0 !~ "outputFileName" {print \$0}
EOF
awk -f tmp.awk $1 > tmp.$1
mv -f tmp.$1 $1
rm tmp.awk
fi

basf2 --info > basf2.error 2>&1
if [ $? -ne 0 ]; then
  cat basf2.error
  echo "basf2 not installed correctly"
  exit 1
fi

##############################################################################
# Runtime
##############################################################################
basf2 $1


##############################################################################
# Work with output files, metadata
##############################################################################
if [ $? -eq 0 ] 
  then
    ./gbasf2output.py -s $1
  else
    cat basf2.error
    echo "basf2 execution occur error"
fi
