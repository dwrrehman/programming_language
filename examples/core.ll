%"(__llvm)" = type opaque
%"(_)" = type opaque
%"(_0) (_)" = type opaque
%"(_1) (_)" = type opaque
%"(_2) (_)" = type opaque

define void @"(__llvm)"() { entry: ret void }
define void @"(_)"() { entry: ret void }
define %"(_)" @"(_0) (_)"() { entry: ret %"(_)" zeroinitializer }
define %"(_)" @"(_1) (_)"() { entry: ret %"(_)" zeroinitializer }
define %"(_)" @"(_2) (_)"() { entry: ret %"(_)" zeroinitializer }

define void @"(_3 (() (_1))) (`.void`.) (_)" ( %"(_1) (_)" ) { entry: ret void }

define void @"(_4 (() (_2)  (_)) (() (_1) (_)) (() (_1) (_))) (`.void`.) (_)" ( %"(_2) (_)", %"(_1) (_)", %"(_1) (_)" ) { entry: ret void }

declare void @"__intrinsic_no_discard"(%"(_)", %"(__llvm)", %"(_0) (_)", %"(_1) (_)", %"(_2) (_)")
