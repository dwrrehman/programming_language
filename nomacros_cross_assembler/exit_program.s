lf foundation.s

set _targetarchitecture arm64_arch
set _outputformat macho_executable
set _shouldoverwrite true

mov syscallnumber system_exit shift_none type_zero width64

mov syscallarg0 42 shift_none type_zero width64

svc halt



