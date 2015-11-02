if [ $# -ne 1 ];then 
echo "Usage : make_rcnode.sh <nodename>"
exit
fi

name=`perl -e "print lc(${1})"`
bigname=`perl -e "print uc(${1})"`
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
sed "s/NSMTemplate/$classname/g" ${dir}/nsmtemplated/include/NSMTemplateCallback.h | \
sed "s/nsmtemplate/${name}/g" > ${filename}

filename=${appdir}/src/${classname}Callback.cc 
echo "create source file : ${filename}"
sed "s/NSMTemplate/$classname/g" ${dir}/nsmtemplated/src/NSMTemplateCallback.cc | \
sed "s/nsmtemplate/${name}/g"  > ${filename}

filename=${appdir}/tools/${name}d.cc
echo "create main file : ${filename}"
sed "s/NSMTemplate/$classname/g" ${dir}/nsmtemplated/tools/nsmtemplated.cc | \
sed "s/nsmtemplate/${name}/g" > ${filename}

filename=${appdir}/Makefile
echo "create Makefile for library : ${filename}"
sed "s/nsmtemplate/${name}/g" ${dir}/nsmtemplated/Makefile > ${filename}

filename=${appdir}/SConscript
echo "create SConscript for library : ${filename}"
sed "s/nsmtemplate/${name}/g" ${dir}/nsmtemplated/SConscript > ${filename}

filename=${appdir}/tools/Makefile
echo "create Makefile for executable: ${filename}"
sed "s/nsmtemplate/${name}/g" ${dir}/nsmtemplated/tools/Makefile > ${filename}

filename=${appdir}/tools/SConscript
echo "create SConscript for executable: ${filename}"
sed "s/nsmtemplate/${name}/g" ${dir}/nsmtemplated/tools/SConscript > ${filename}

else 
echo "directory ${appdir} already exsists"
fi


dir=${BELLE2_DAQ_SLC}/data/config

filename=${dir}/${name}.conf
if [ -e ${filename} ]; then
echo "config file ${filename} alredy exsists"
else
echo "create NSM configuration file : ${filename}"
sed "s/nsmtemplate/${name}/g" ${dir}/nsmtemplate.conf | \
sed "s/NSMTEMPLATE/$bigname/g" > ${filename}
fi

echo "make_nsmnode: done"