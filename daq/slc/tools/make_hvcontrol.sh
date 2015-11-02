if [ $# -ne 1 ];then 
echo "Usage : make_rcnode.sh <nodename>"
exit
fi

name=`perl -e "print lc(${1})"`
bigname=`perl -e "print uc(${1})"`
classname=`perl -e "print ucfirst(${name})"`

dir=${BELLE2_DAQ_SLC}/hvcontrol

appdir=${dir}/${name}

if [ ! -e  ${appdir} ]; then

mkdir ${appdir}/
mkdir ${appdir}/include
mkdir ${appdir}/src
mkdir ${appdir}/tools

filename=${appdir}/include/${classname}ControlCallback.h 
echo "create header file : ${filename}"
sed "s/TemplateHV/${classname}/g" ${dir}/template/include/TemplateHVControlCallback.h | \
sed "s/template/${name}/g" > ${filename}

filename=${appdir}/src/${classname}ControlCallback.cc 
echo "create source file : ${filename}"
sed "s/TemplateHV/${classname}/g" ${dir}/template/src/TemplateHVControlCallback.cc | \
sed "s/template/${name}/g"  > ${filename}

filename=${appdir}/tools/${name}hvd.cc
echo "create main file : ${filename}"
sed "s/TemplateHV/${classname}/g" ${dir}/template/tools/templatehvd.cc | \
sed "s/template/${name}/g" > ${appdir}/tools/${name}hvd.cc

filename=${appdir}/Makefile
echo "create Makefile for library : ${filename}"
sed "s/template/${name}/g" ${dir}/template/Makefile > ${filename}

filename=${appdir}/tools/Makefile
echo "create Makefile for executable: ${filename}"
sed "s/template/${name}/g" ${dir}/template/tools/Makefile > ${filename}
filename=${appdir}/tools/SConscript
echo "create SConscript for executable: ${filename}"
sed "s/template/${name}/g" ${dir}/template/tools/SConscript > ${filename}

else 
echo "directory ${appdir} already exsists"
fi

dir=${BELLE2_DAQ_SLC}/data/hvcontrol

filename=${dir}/${name}.conf
if [ -e ${filename} ]; then
echo "config file ${filename} alredy exsists"
else
echo "create NSM configuration file : ${filename}"
sed "s/template/${name}/g" ${dir}/template.conf | \
sed "s/TEMPLATE/$bigname/g" > ${filename}
fi

filename=${dir}/${name}-input.conf
if [ -e ${filename} ]; then
echo "dbinput file ${filename} alredy exsists"
else
echo "create database input file : ${filename}"
sed "s/template/${name}/g" ${dir}/template-input.conf | \
sed "s/TEMPLATE/$bigname/g" > ${filename}
fi

echo "make_hvcontrol: done"