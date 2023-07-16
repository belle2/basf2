##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################


from datetime import datetime, timedelta
import getpass
# requires pyjwt library which is not present by default yet
import jwt

filename = "./cdb_token"
username = "testuser"
validity = timedelta(hours=1)
secret = getpass.getpass("JWT Secret: ")

payload = {
    "sub": "testuser",
    "name": "Dr. Test User",
    "b2cdb:admin": [f"user_{username}_.*", "staging_online", "online"],
    "b2cdb:create_iov": ["test_creation.*"],
    "exp": datetime.utcnow() + validity
}

with open(filename, "bw") as f:
    token = jwt.encode(payload, secret)
    f.write(token)
