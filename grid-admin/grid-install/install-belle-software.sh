#!/bin/bash
#!/bin/sh
set -x
#
SOFTWARE_VERSION=belle-b20090127_0910-p31-cern-Linux_x86_64_glibc-2.5.tar.gz
VO_BELLE_SW_DIR=${VO_BELLE_SW_DIR:-$OSG_APP/belle}

if [ "$VO_BELLE_SW_DIR" == "" ]; then
 exit 1
fi

if [ -f /etc/redhat-release ]; then
  echo "/etc/redhat-release  -----------------"
  cat /etc/redhat-release
fi
echo "glite-version = `glite-version`"
echo "/bin/uname -a   -------------------"
/bin/uname -a
echo "I am `whoami`" 
echo "WN = `hostname`" 

echo "start ipcs--------------"
ipcs
echo "end ipcs--------------"

echo "df -h ----------------------------------" 
df -h 
echo "----------------------------------" 

echo "start ls --------------" 
ls 
echo "end ls --------------" 

echo "start ls -l $VO_BELLE_SW_DIR--------------" 
ls -l $VO_BELLE_SW_DIR 
echo "end ls -l $VO_BELLE_SW_DIR--------------" 
find $VO_BELLE_SW_DIR -maxdepth 3
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
echo "" 
echo "start gcc -v --------------" 
gcc -v
echo "end gcc -v -------------" 
echo "" 
echo "start gcc --version --------------" 
gcc --version
echo "end gcc --version -------------" 

echo "start env----------------------"
env
echo "end env----------------------"
cd $VO_BELLE_SW_DIR
rm $SOFTWARE_VERSION
rm -rf Linux_x86_64_glibc-2.5
lcg-cp -v lfn:/grid/belle/cloud/software/$SOFTWARE_VERSION $SOFTWARE_VERSION
tar -xvzf $SOFTWARE_VERSION
chmod -R 775 Linux_x86_64_glibc-2.5
rm $SOFTWARE_VERSION
ls -l 
