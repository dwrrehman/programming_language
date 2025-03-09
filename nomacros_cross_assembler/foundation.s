.
  the foundation file for the language
  written on 1202503086.033529 by dwrr
.

df _targetarchitecture
zero _targetarchitecture

df 0 zero 0
df 1 set 1 0 incr 1
df 2 set 2 1 incr 2
df 3 set 3 2 incr 3
df 4 set 4 3 incr 4
df 5 set 5 4 incr 5
df 6 set 6 5 incr 6
df 7 set 7 6 incr 7
df 8 set 8 7 incr 8
df 9 set 9 8 incr 9
df 10 set 10 9 incr 10
df 11 set 11 10 incr 11
df 12 set 12 11 incr 12
df 13 set 13 12 incr 13
df 14 set 14 13 incr 14
df 15 set 15 14 incr 15

df 16 set 16 1 si 16 4
df 32 set 32 1 si 32 5
df 64 set 64 1 si 64 6
df 128 set 128 1 si 128 7
df 256 set 256 1 si 256 8
df 512 set 512 1 si 512 9
df 1024 set 1024 1 si 1024 10
df 2048 set 2048 1 si 2048 11
df 4096 set 4096 1 si 4096 12

df no_arch set no_arch 0
df arm64_arch set arm64_arch 1
df arm32_arch set arm32_arch 2
df rv64_arch set rv64_arch 3
df rv32_arch set rv32_arch 4



. ...........arm64............. . 

df width64 set width64 1
df width32 set width32 0

. arm64 mov instruction .
df type_zero set type_zero 2
df type_neg set type_neg 0
df type_keep set type_keep 3





