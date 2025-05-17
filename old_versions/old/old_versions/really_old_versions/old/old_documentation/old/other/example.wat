# compile me with:    wasm-tools parse example.wat -o out_file

(module
  (func $i (import "imports" "imported_func") (param i32))
  (func (export "exported_func")
    i32.const 42
    call $i
  )
 )
