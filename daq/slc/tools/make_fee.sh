if [ $# -ne 2 ];then 
echo "Usage : make_fee.sh <fee_name> <copperid>"
exit
fi

name=`perl -e "print lc(${1})"`
bigname=`perl -e "print lcfirst(${1})"`
classname=`perl -e "print uc(${1})"`
id=$2

dir=${BELLE2_DAQ_SLC}/copper

if [ ! -e  ${dir}/${name} ]; then
mkdir $dir/${name}/
mkdir $dir/${name}/include
mkdir $dir/${name}/src
mkdir $dir/${name}/tools

filename=${dir}/${name}/include/${classname}FEE.h
echo "create header file : ${filename}"
sed "s/TEMPLATE/$classname/g" $dir/template/include/TEMPLATEFEE.h | \
sed "s/template/${name}/g" > ${filename}

filename=${dir}/${name}/src/${classname}FEE.cc
echo "create source file : ${filename}"
sed "s/TEMPLATE/$classname/g" $dir/template/src/TEMPLATEFEE.cc | \
sed "s/template/${name}/g"  > ${filename}

filename=${dir}/${name}/Makefile
echo "create Makefile : ${filename}"
sed "s/TEMPLATE/$classname/g" $dir/template/Makefile | \
sed "s/template/${name}/g"  > ${filename}

else 
echo "directory ${dir}/${name} already exsists"
fi

dir=${BELLE2_DAQ_SLC}/data/config

filename=$dir/cpr${id}.conf
echo "create NSM configuration file : ${filename}"
sed "s/TEMPLATE/$classname/g" $dir/cpr_template.conf | \
sed "s/template/${name}/g" | sed "s/CPR001/CPR${id}/g"  > $dir/cpr${id}.conf

dir=${BELLE2_DAQ_SLC}/data/database

mkdir -p $dir/${name}

filename=$dir/${name}/${name}.cpr${id}.a.conf
echo "create dbinput file for parameter (HSLB-a) : ${filename}"
sed "s/Template/$bigname/g" $dir/example/fee.cpr001.a.conf | \
sed "s/cpr001/cpr${id}/g" | sed "s/template/${name}/g"  > ${filename}

filename=$dir/${name}/${name}.cpr${id}.b.conf
echo "create dbinput file for parameter (HSLB-b) : ${filename}"
sed "s/Template/$bigname/g" $dir/example/fee.cpr001.b.conf | \
sed "s/cpr001/cpr${id}/g" | sed "s/template/${name}/g"  > ${filename}

filename=$dir/${name}/${name}.cpr${id}.c.conf
echo "create dbinput file for parameter (HSLB-c) : ${filename}"
sed "s/Template/$bigname/g" $dir/example/fee.cpr001.c.conf | \
sed "s/cpr001/cpr${id}/g" | sed "s/template/${name}/g"  > ${filename}

filename=$dir/${name}/${name}.cpr${id}.d.conf
echo "create dbinput file for parameter (HSLB-d) : ${filename}"
sed "s/Template/$bigname/g" $dir/example/fee.cpr001.d.conf | \
sed "s/cpr001/cpr${id}/g" | sed "s/template/${name}/g"  > ${filename}

filename=$dir/${name}/cpr${id}.conf
echo "create dbinput file for COPPER RC client : ${filename}"
sed "s/Template/$bigname/g" $dir/example/copper.conf | \
sed "s/cpr001/cpr${id}/g" | sed "s/CPR001/CPR${id}/g" | \
sed "s/template/${name}/g" > ${filename}

echo "make_fee: done"