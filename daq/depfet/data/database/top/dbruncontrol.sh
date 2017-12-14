function echo_config()
{
echo "nodename           : ${1}"
echo "config             : RC:cosmic:0001"
}

function echo_cpr()
{
echo "node[${1}].name       : ${2}"
echo "node[${1}].used       : true"
echo "node[${1}].sequential : false"
echo "node[${1}].rcconfig   : object(RC:cosmic:)"
}

index=0
echo_config ECL01 > ecl01.conf
echo_cpr ${index} ROPC501 >> ecl01.conf
index=$(($index + 1))
for i in $(seq 5001 5013)
do
  index=$(($index + 1))
  echo_cpr ${index} CPR${i} >> ecl01.conf
done
daqdbcreate ecl01.conf ecl

index=0
echo_config ECL04 > ecl04.conf
echo_cpr ${index} ROPC504 >> ecl04.conf
index=$(($index + 1))
for i in $(seq 5014 5018)
do
  index=$(($index + 1))
  echo_cpr ${index} CPR${i} >> ecl04.conf
done
daqdbcreate ecl04.conf ecl

