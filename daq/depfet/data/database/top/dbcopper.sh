function sed_config()
{
   sed "s/cpr5014/cpr${1}/g" cpr5014.conf | sed "s/CPR5014/CPR${1}/g" > cpr${1}.conf
   daqdbcreate cpr${1}.conf top
   #cat cpr${1}.conf
}

for i in $(seq 3001 3016)
do
  daqdbremove top CPR${i}@RC:cosmic:001
  sed_config $i
done

