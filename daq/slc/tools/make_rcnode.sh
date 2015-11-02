if [ $# -ne 1 ];then 
echo "Usage : make_rcnode.sh <nodename>"
exit
fi

name=`perl -e "print lc(${1})"`
bigname=`perl -e "print lcfirst(${1})"`
classname=`perl -e "print ucfirst(${name})"`

dir=${BELLE2_DAQ_SLC}/apps

appdir=${dir}/${name}d

if [ ! -e  ${appdir} ]; then

mkdir ${appdir}/
mkdir ${appdir}/include
mkdir ${appdir}/src
mkdir ${appdir}/tools

filename=${appdir}/include/${classname}Callback.h 
echo "create header file : ${filename}"
sed "s/RCTemplate/${classname}/g" ${dir}/rctemplated/include/RCTemplateCallback.h | \
sed "s/rctemplate/${name}/g" > ${filename}

filename=${appdir}/src/${classname}Callback.cc 
echo "create source file : ${filename}"
sed "s/RCTemplate/${classname}/g" ${dir}/rctemplated/src/RCTemplateCallback.cc | \
sed "s/rctemplate/${name}/g"  > ${filename}

filename=${appdir}/tools/${name}d.cc
echo "create main file : ${filename}"
sed "s/RCTemplate/${classname}/g" ${dir}/rctemplated/tools/rctemplated.cc | \
sed "s/rctemplate/${name}/g" > ${filename}

filename=${appdir}/Makefile
echo "create Makefile for library : ${filename}"
sed "s/rctemplate/${name}/g" ${dir}/rctemplated/Makefile > ${filename}

filename=${appdir}/SConscript
echo "create SConscript for library : ${filename}"
sed "s/rctemplate/${name}/g" ${dir}/rctemplated/SConscript > ${filename}

filename=${appdir}/tools/Makefile
echo "create Makefile for executable: ${filename}"
sed "s/rctemplate/${name}/g" ${dir}/rctemplated/tools/Makefile > ${filename}

filename=${appdir}/tools/SConscript
echo "create SConscript for executable : ${filename}"
sed "s/rctemplate/${name}/g" ${dir}/rctemplated/tools/SConscript > ${filename}

else 
echo "directory ${appdir} already exsists"
fi

dir=${BELLE2_DAQ_SLC}/data/config
filename=${dir}/${name}.conf
echo "create NSM configuration file : ${filename}"
sed "s/rctemplate/${name}/g" ${dir}/rctemplate.conf | \
sed "s/RCTEMPLATE/$bigname/g" > ${filename}

dir=${BELLE2_DAQ_SLC}/data/database
filename=${dir}/${name}d.conf
echo "create database input file : ${filename}"
sed "s/rctemplate/${name}/g" ${dir}/rctemplated.conf | \
sed "s/RCTEMPLATE/$bigname/g" > ${filename}

echo "make_rcnode: done"