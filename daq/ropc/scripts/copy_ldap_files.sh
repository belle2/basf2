#!/bin/sh
scp /etc/ldap.conf root@klm02:/etc
scp /etc/openldap/ldap.conf root@klm02:/etc/openldap
scp /etc/pam.d/system-auth-ac root@klm02:/etc/pam.d
scp /etc/nsswitch.conf root@klm02:/etc
