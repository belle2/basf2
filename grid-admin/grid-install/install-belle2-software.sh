#!/bin/bash
set -x

##############################################################################
# Configuration and initialisation
##############################################################################

#First, determine the platform we're working on and verify it is supported
PLATFORM=`./dirac-platform.py`

if [ "$PLATFORM" != "Linux_x86_64_glibc-2.5" ]; then
  echo "$PLATFORM not supported."
  exit 1
fi

# Set software version and install location

SOFTWARE_VERSION=${1:-"build_2010-12-13"}
EXTERNALS_VERSION=v00-00-03
SOFTWARE_TAR=belle2-$SOFTWARE_VERSION-$PLATFORM.tgz

VO=belle
VO_BELLE_SW_DIR=${VO_BELLE_SW_DIR:-$OSG_APP/belle}

if [ "$VO_BELLE_SW_DIR" == "" ]; then
 echo "We couldn't find where to install the software"
 exit 1
fi

CE=${OSG_HOSTNAME:-`edg-brokerinfo getCE || glite-brokerinfo getCE`}
CE=`echo $CE | awk -F: '{ print $1 }'`

if [ "$CE" == "" ]; then
  echo "We could not determine the CE hostname"
  exit 1
fi

if [ "$LCG_GFAL_INFOSYS" == "is.grid.iu.edu" ] || [ "$LCG_GFAL_INFOSYS" == "" ]; then
  export LCG_GFAL_INFOSYS=lcg-bdii.cern.ch
fi

##############################################################################
# Debugging information
##############################################################################

if [ -f /etc/redhat-release ]; then
  echo "/etc/redhat-release  -----------------"
  cat /etc/redhat-release
fi
echo "glite-version = `glite-version`"
echo "/bin/uname -a   -------------------"
/bin/uname -a
echo "I am `whoami`" 
echo "WN = `hostname`" 

echo "df -h ----------------------------------" 
df -h 
echo "----------------------------------" 

echo "start ls --------------" 
ls 
echo "end ls --------------" 

echo "start ls -l $VO_BELLE_SW_DIR--------------" 
ls -l $VO_BELLE_SW_DIR 
echo "end ls -l $VO_BELLE_SW_DIR--------------" 
ls -l $VO_BELLE_SW_DIR/../

echo "start cpuinfo --------------" 
if [ -f /proc/cpuinfo ]; then
  cat /proc/cpuinfo
else
  echo "/proc/cpuinfo not found"
fi
echo "end cpuinfo--------------" 
echo "" 
echo "start meminfo --------------" 
if [ -f /proc/meminfo ]; then
  cat /proc/meminfo
else
  echo "/proc/meminfo not found"
fi
echo "end meminfo--------------" 

echo "start env----------------------"
env
echo "end env----------------------"

##############################################################################
# Installation steps
##############################################################################
if [ ! -d $VO_BELLE_SW_DIR/belle2 ]; then
  mkdir $VO_BELLE_SW_DIR/belle2
fi

chown `whoami` $VO_BELLE_SW_DIR/belle2
chmod 775 $VO_BELLE_SW_DIR/belle2
cd $VO_BELLE_SW_DIR/belle2

rm $SOFTWARE_TAR
rm -rf releases/$SOFTWARE_VERSION
 
lcg-cp  -n 1 lfn:/grid/$VO/software/$SOFTWARE_TAR $SOFTWARE_TAR
if [ $? -ne 0 ]; then
  echo "Problems downloading tar"
  exit 1
fi

tar -xzf $SOFTWARE_TAR
if [ $? -ne 0 ]; then
  echo "Problems untarring"
  exit 1
fi


chown -R `whoami` *
chmod -R 775 *
lcg-ManageVOTag -host $CE  -vo $VO --add -tag VO-$VO-$SOFTWARE_VERSION VO-$VO-Externals-$EXTERNALS_VERSION VO-$VO-Platform-$PLATFORM 
if [ $? -ne 0 ]; then
    lcg-tags --ce $CE  --vo $VO --add --tags VO-$VO-$SOFTWARE_VERSION,VO-$VO-Externals-$EXTERNALS_VERSION,VO-$VO-Platform-$PLATFORM 
fi

##############################################################################
# Cleanup
##############################################################################
rm $SOFTWARE_TAR
ls -l 
