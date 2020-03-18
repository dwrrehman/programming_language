





{ none, id, op, string, expr,
    action = 'x', exec = 'o', object = 'c', ir = 'i', assembly = 's',
    
    _undefined = 0,
    _init, _name, _number, _join,
    _declare,
    
    _type,
    _0, _lazy,
    
    _1, _2, _3, _4, _define, /// define (s: name) (t: init) (d: L t) (extern: number) -> init
    
    _i1, _i8, _i16, _i32, _i64, _i128, _x86_mmx, _f16, _f32, _f64, _f128,
    
    _5, _6, _pointer,           /// pointer (addrspace: number) (t: type)   -> type
    
    _7, _8, _vector,            /// vector (width: number) (t: type)  -> type
    _9, _10, _scalable,         /// scalable (width: number) (t: type)   -> type
    
    _11, _12, _array,           /// array (size: number) (t: type)   -> type
    
    _13, _opaque,               /// opaque (s: name) -> type
    _14, _15, _16, _structure,  /// struct (s: name) (d: name) (extern: number) -> type
    _17, _18, _19, _packed,     /// packed (s: name) (d: name) (extern: number) -> type
    
    _27, _function_type,         /// function (type: name)
    
    _label, _metadata, _token, _unit,
    _string, ///  string -> pointer 0 i8

    _unreachable,                   /// unreachable   -> unit
    _ret_void,                      /// ret void   -> unit
    _20, _21, _ret_value,           /// ret (t: type) (v: t)   -> unit
    
    _22, _create_label,             /// label (l: name)  -> unit
    _23, _uncond_branch,            /// jump (l: name)   -> unit
    _24, _25, _26, _cond_branch,    /// br (cond: i1) (1: name) (2: name)   -> unit
    
    _intrinsic_count
}
