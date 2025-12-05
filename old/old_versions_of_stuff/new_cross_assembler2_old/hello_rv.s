file library/core.s

st compiler_target rv32_arch
st compiler_format hex_array
st compiler_should_overwrite true

ri rv_imm rv_add rv_system_number 0 rv_system_write
ri rv_imm rv_add rv_system_arg0 0 stdout
ru rv_auipc rv_system_arg1 string
ri rv_imm rv_add rv_system_arg1 rv_system_arg1 string
st compiler_length string ld c0 compiler_length
ri rv_imm rv_add rv_system_arg2 0 c0
ri rv_ecall 0 0 0 0

ri rv_imm rv_add rv_system_number 0 rv_system_exit
ri rv_imm rv_add rv_system_arg0 0 10001
ri rv_ecall 0 0 0 0

at string
str .hello, world!
.

eoi

1202507222.002822
hello world program for risc-v!





