peak:004
Guard-Bias : all
HV : panel#4,6
S1-1,2,3,4
S2-1,2
S4-1,2,3,4
S5-1,2




peak:005
Guard-Bias : all
HV : panel#5,7
S2-3,4
S3-1,2,3,4
S5-3,4
S6-1,2,3,4


peak:006
HV : all
Guard-bias : no

peak:007
HV : no
Guard-bias : crate1

peak:008
HV : no
Guard-bias : crate2

peak:009
HV : no
Guard-bias : crate3


peak:sec1:010
for sector 1

peak:sec2:010
for sector 2

peak:sec3:010
for sector 3

peak:sec4:010
for sector 4

peak:sec5:010
for sector 5

peak:sec6:010
for sector 6
you need move one HV module.
Please ask Yonenaga(yonenaga@hepmail.phys.se.tmu.ac.jp)

peak:012
full sector
mask parameter is changed as bellow 
bool("true" or "false") -> string("masked" or "unmasked")

peak:011
temp

peak:013,peak:014
for temp. test

peak:017
for all sectors.
mask parameter is changed as bellow 
bool("true" or "false") -> string("masked" or "unmasked")

peak:020
for all sectors.
bad bias->the channels are masked
bad HV->only HV channel is masked. bias and guard are NOT masked.
bad guard->HV,bias are also masked

peak:021
for all sectors.
mask:
bad bias->the channels are masked
bad HV->bias,guard are also masked
bad guard->HV,bias are also masked
