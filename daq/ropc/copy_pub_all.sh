#!/bin/sh

scp ~/.ssh/id_rsa.pub svd01:~/; ssh svd01 'ssh 172.22.16.4; cat ~/id_rsa.pub > ~/.ssh/authorized_keys; chmod 600 ~/.ssh/authorized_keys;'
scp ~/.ssh/id_rsa.pub svd02:~/; ssh svd02 'ssh 172.22.16.4; cat ~/id_rsa.pub > ~/.ssh/authorized_keys; chmod 600 ~/.ssh/authorized_keys;'
scp ~/.ssh/id_rsa.pub svd03:~/; ssh svd03 'ssh 172.22.16.4; cat ~/id_rsa.pub > ~/.ssh/authorized_keys; chmod 600 ~/.ssh/authorized_keys;'
scp ~/.ssh/id_rsa.pub svd04:~/; ssh svd04 'ssh 172.22.16.4; cat ~/id_rsa.pub > ~/.ssh/authorized_keys; chmod 600 ~/.ssh/authorized_keys;'
scp ~/.ssh/id_rsa.pub svd05:~/; ssh svd05 'ssh 172.22.16.4; cat ~/id_rsa.pub > ~/.ssh/authorized_keys; chmod 600 ~/.ssh/authorized_keys;'
scp ~/.ssh/id_rsa.pub svd06:~/; ssh svd06 'ssh 172.22.16.4; cat ~/id_rsa.pub > ~/.ssh/authorized_keys; chmod 600 ~/.ssh/authorized_keys;'
scp ~/.ssh/id_rsa.pub svd07:~/; ssh svd07 'ssh 172.22.16.4; cat ~/id_rsa.pub > ~/.ssh/authorized_keys; chmod 600 ~/.ssh/authorized_keys;'
scp ~/.ssh/id_rsa.pub svd08:~/; ssh svd08 'ssh 172.22.16.4; cat ~/id_rsa.pub > ~/.ssh/authorized_keys; chmod 600 ~/.ssh/authorized_keys;'
scp ~/.ssh/id_rsa.pub svd09:~/; ssh svd09 'ssh 172.22.16.4; cat ~/id_rsa.pub > ~/.ssh/authorized_keys; chmod 600 ~/.ssh/authorized_keys;'
scp ~/.ssh/id_rsa.pub svd10:~/; ssh svd10 'ssh 172.22.16.4; cat ~/id_rsa.pub > ~/.ssh/authorized_keys; chmod 600 ~/.ssh/authorized_keys;'
scp ~/.ssh/id_rsa.pub svd11:~/; ssh svd11 'ssh 172.22.16.4; cat ~/id_rsa.pub > ~/.ssh/authorized_keys; chmod 600 ~/.ssh/authorized_keys;'
scp ~/.ssh/id_rsa.pub cdc01:~/; ssh cdc01 'ssh 172.22.16.4; cat ~/id_rsa.pub > ~/.ssh/authorized_keys; chmod 600 ~/.ssh/authorized_keys;'
scp ~/.ssh/id_rsa.pub cdc02:~/; ssh cdc02 'ssh 172.22.16.4; cat ~/id_rsa.pub > ~/.ssh/authorized_keys; chmod 600 ~/.ssh/authorized_keys;'
scp ~/.ssh/id_rsa.pub cdc03:~/; ssh cdc03 'ssh 172.22.16.4; cat ~/id_rsa.pub > ~/.ssh/authorized_keys; chmod 600 ~/.ssh/authorized_keys;'
scp ~/.ssh/id_rsa.pub cdc04:~/; ssh cdc04 'ssh 172.22.16.4; cat ~/id_rsa.pub > ~/.ssh/authorized_keys; chmod 600 ~/.ssh/authorized_keys;'
scp ~/.ssh/id_rsa.pub cdc05:~/; ssh cdc05 'ssh 172.22.16.4; cat ~/id_rsa.pub > ~/.ssh/authorized_keys; chmod 600 ~/.ssh/authorized_keys;'
scp ~/.ssh/id_rsa.pub cdc06:~/; ssh cdc06 'ssh 172.22.16.4; cat ~/id_rsa.pub > ~/.ssh/authorized_keys; chmod 600 ~/.ssh/authorized_keys;'
scp ~/.ssh/id_rsa.pub cdc07:~/; ssh cdc07 'ssh 172.22.16.4; cat ~/id_rsa.pub > ~/.ssh/authorized_keys; chmod 600 ~/.ssh/authorized_keys;'
scp ~/.ssh/id_rsa.pub cdc08:~/; ssh cdc08 'ssh 172.22.16.4; cat ~/id_rsa.pub > ~/.ssh/authorized_keys; chmod 600 ~/.ssh/authorized_keys;'
scp ~/.ssh/id_rsa.pub cdc09:~/; ssh cdc09 'ssh 172.22.16.4; cat ~/id_rsa.pub > ~/.ssh/authorized_keys; chmod 600 ~/.ssh/authorized_keys;'
scp ~/.ssh/id_rsa.pub cdc10:~/; ssh cdc10 'ssh 172.22.16.4; cat ~/id_rsa.pub > ~/.ssh/authorized_keys; chmod 600 ~/.ssh/authorized_keys;'

#scp ~/.ssh/id_rsa.pub top01:~/; ssh top01 'ssh 172.22.16.4; cat ~/id_rsa.pub > ~/.ssh/authorized_keys; chmod 600 ~/.ssh/authorized_keys;'
scp ~/.ssh/id_rsa.pub top02:~/; ssh top02 'ssh 172.22.16.4; cat ~/id_rsa.pub > ~/.ssh/authorized_keys; chmod 600 ~/.ssh/authorized_keys;'
scp ~/.ssh/id_rsa.pub top03:~/; ssh top03 'ssh 172.22.16.4; cat ~/id_rsa.pub > ~/.ssh/authorized_keys; chmod 600 ~/.ssh/authorized_keys;'
scp ~/.ssh/id_rsa.pub top04:~/; ssh top04 'ssh 172.22.16.4; cat ~/id_rsa.pub > ~/.ssh/authorized_keys; chmod 600 ~/.ssh/authorized_keys;'

scp ~/.ssh/id_rsa.pub arich01:~/; ssh arich01 'ssh 172.22.16.4; cat ~/id_rsa.pub > ~/.ssh/authorized_keys; chmod 600 ~/.ssh/authorized_keys;'
scp ~/.ssh/id_rsa.pub arich02:~/; ssh arich02 'ssh 172.22.16.4; cat ~/id_rsa.pub > ~/.ssh/authorized_keys; chmod 600 ~/.ssh/authorized_keys;'
scp ~/.ssh/id_rsa.pub arich03:~/; ssh arich03 'ssh 172.22.16.4; cat ~/id_rsa.pub > ~/.ssh/authorized_keys; chmod 600 ~/.ssh/authorized_keys;'
scp ~/.ssh/id_rsa.pub arich04:~/; ssh arich04 'ssh 172.22.16.4; cat ~/id_rsa.pub > ~/.ssh/authorized_keys; chmod 600 ~/.ssh/authorized_keys;'


scp ~/.ssh/id_rsa.pub ecl01:~/; ssh ecl01 'ssh 172.22.16.4; cat ~/id_rsa.pub > ~/.ssh/authorized_keys; chmod 600 ~/.ssh/authorized_keys;'
scp ~/.ssh/id_rsa.pub ecl02:~/; ssh ecl02 'ssh 172.22.16.4; cat ~/id_rsa.pub > ~/.ssh/authorized_keys; chmod 600 ~/.ssh/authorized_keys;'
scp ~/.ssh/id_rsa.pub ecl03:~/; ssh ecl03 'ssh 172.22.16.4; cat ~/id_rsa.pub > ~/.ssh/authorized_keys; chmod 600 ~/.ssh/authorized_keys;'
scp ~/.ssh/id_rsa.pub ecl04:~/; ssh ecl04 'ssh 172.22.16.4; cat ~/id_rsa.pub > ~/.ssh/authorized_keys; chmod 600 ~/.ssh/authorized_keys;'
scp ~/.ssh/id_rsa.pub ecl05:~/; ssh ecl05 'ssh 172.22.16.4; cat ~/id_rsa.pub > ~/.ssh/authorized_keys; chmod 600 ~/.ssh/authorized_keys;'
scp ~/.ssh/id_rsa.pub ecl06:~/; ssh ecl06 'ssh 172.22.16.4; cat ~/id_rsa.pub > ~/.ssh/authorized_keys; chmod 600 ~/.ssh/authorized_keys;'
scp ~/.ssh/id_rsa.pub ecl07:~/; ssh ecl07 'ssh 172.22.16.4; cat ~/id_rsa.pub > ~/.ssh/authorized_keys; chmod 600 ~/.ssh/authorized_keys;'

scp ~/.ssh/id_rsa.pub klm01:~/; ssh klm01 'ssh 172.22.16.4; cat ~/id_rsa.pub > ~/.ssh/authorized_keys; chmod 600 ~/.ssh/authorized_keys;'
scp ~/.ssh/id_rsa.pub klm02:~/; ssh klm02 'ssh 172.22.16.4; cat ~/id_rsa.pub > ~/.ssh/authorized_keys; chmod 600 ~/.ssh/authorized_keys;'
scp ~/.ssh/id_rsa.pub klm03:~/; ssh klm03 'ssh 172.22.16.4; cat ~/id_rsa.pub > ~/.ssh/authorized_keys; chmod 600 ~/.ssh/authorized_keys;'
scp ~/.ssh/id_rsa.pub klm04:~/; ssh klm04 'ssh 172.22.16.4; cat ~/id_rsa.pub > ~/.ssh/authorized_keys; chmod 600 ~/.ssh/authorized_keys;'


