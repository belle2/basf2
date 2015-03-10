name=`echo ${1,,}`
bigname=`echo ${1^}`
classname=`echo ${name^^}`
id=$2

dir=${BELLE2_LOCAL_DIR}/daq/slc/copper

mkdir $dir/${name}/
mkdir $dir/${name}/include
mkdir $dir/${name}/src
mkdir $dir/${name}/tools

sed "s/TEMPLATE/$classname/g" $dir/template/include/TEMPLATEFEE.h | \
sed "s/template/${name}/g" > $dir/${name}/include/${classname}FEE.h 

sed "s/TEMPLATE/$classname/g" $dir/template/src/TEMPLATEFEE.cc | \
sed "s/template/${name}/g"  > $dir/${name}/src/${classname}FEE.cc 

dir=${BELLE2_LOCAL_DIR}/daq/slc/data/config

sed "s/TEMPLATE/$classname/g" $dir/cpr001.conf | \
sed "s/template/${name}/g" | sed "s/CPR001/CPR${id}/g"  > $dir/cpr${id}.conf

dir=${BELLE2_LOCAL_DIR}/daq/slc/data/database

mkdir -p $dir/${name}

sed "s/Template/$bigname/g" $dir/example/fee.reg.conf | \
sed "s/template/${name}/g"  > $dir/${name}/${name}.reg.conf

sed "s/Template/$bigname/g" $dir/example/fee.par.cpr001.a.conf | \
sed "s/cpr001/cpr${id}/g" | sed "s/template/${name}/g"  > $dir/${name}/${name}.par.cpr${id}.a.conf

sed "s/Template/$bigname/g" $dir/example/fee.par.cpr001.b.conf | \
sed "s/cpr001/cpr${id}/g" | sed "s/template/${name}/g"  > $dir/${name}/${name}.par.cpr${id}.b.conf

sed "s/Template/$bigname/g" $dir/example/fee.par.cpr001.c.conf | \
sed "s/cpr001/cpr${id}/g" | sed "s/template/${name}/g"  > $dir/${name}/${name}.par.cpr${id}.c.conf

sed "s/Template/$bigname/g" $dir/example/fee.par.cpr001.d.conf | \
sed "s/cpr001/cpr${id}/g" | sed "s/template/${name}/g"  > $dir/${name}/${name}.par.cpr${id}.d.conf

sed "s/Template/$bigname/g" $dir/example/copper.conf | \
sed "s/cpr001/cpr${id}/g" | sed "s/CPR001/CPR${id}/g" | \
sed "s/template/${name}/g" > $dir/${name}/cpr${id}.conf

