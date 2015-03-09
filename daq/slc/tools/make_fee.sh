name=`echo ${1,,}`
bigname=`echo ${1^^}`
classname=`echo ${name^^}`

dir=${BELLE2_LOCAL_DIR}/daq/slc/copper

mkdir $dir/${name}/
mkdir $dir/${name}/include
mkdir $dir/${name}/src
mkdir $dir/${name}/tools

sed "s/TEMPLATE/$classname/g" $dir/template/include/TEMPLATEFEE.h | \
sed "s/template/${name}/g" > $dir/${name}/include/${classname}FEE.h 

sed "s/TEMPLATE/$classname/g" $dir/template/src/TEMPLATEFEE.cc | \
sed "s/template/${name}/g"  > $dir/${name}/src/${classname}FEE.cc 

