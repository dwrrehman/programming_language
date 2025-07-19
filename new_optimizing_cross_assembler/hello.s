file library/core.s

st compiler_target rv32_arch
st compiler_format hex_array
st compiler_should_overwrite true

set rv_auipc_op1 1110_100
set rv_ecall_op1 1100_111

set length 101

ri rv_addi_op1 rv_addi_op2  rv_system_number 0  rv_system_write
ri rv_addi_op1 rv_addi_op2  rv_system_arg0 0  stdout

ru rv_auipc_op1 rv_system_arg1 hello
ri rv_addi_op1 rv_addi_op2  rv_system_arg1 rv_system_arg1  hello

ri rv_addi_op1 rv_addi_op2  rv_system_arg2 0  length

ri rv_ecall_op1 0 0 0 0

ri rv_addi_op1 rv_addi_op2 rv_system_number 0 rv_system_exit
ri rv_addi_op1 rv_addi_op2 rv_system_arg0 0 0011

ri rv_ecall_op1 0 0 0 0


at hello
str "hello"

eoi
