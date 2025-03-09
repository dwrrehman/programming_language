lf foundation.s

ctdebug 32
ctdebug 4
ctdebug 2
ctdebug 1

df type_zero set type_zero 2
df type_neg set type_neg 0
df type_keep set type_keep 3
df width64 set width64 1
df width32 set width32 0




df limit set limit 4
df max_entry_count set max_entry_count 10 mul max_entry_count 10

mov limit max_entry_count 0 type_zero width64

mov limit max_entry_count 0 type_zero width64

svc

halt

nop

eoi



