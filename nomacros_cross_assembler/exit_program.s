lf foundation.s

set _targetarchitecture arm64_arch

mov syscallnumber system_exit shift_none type_zero width64

mov syscallarg0 42 shift_none type_zero width64

svc halt



