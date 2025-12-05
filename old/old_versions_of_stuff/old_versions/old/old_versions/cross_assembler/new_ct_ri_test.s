lf arm64.s

set d 3
mov d 4 0 2 1
svc

nop
nop
nop

br d 0 0
br d 0 1
br d 1 0
br d 1 1

halt



