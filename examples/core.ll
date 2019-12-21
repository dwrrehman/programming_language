; outdated, now lives in the compiler source itself, in string form.

%"(_)" = type opaque
%"(_0) (_)" = type opaque
%"(_1) (_)" = type opaque
%"(_2) (_)" = type opaque

declare void @"__intrinsic_no_discard"(%"(_)", %"(_0) (_)", %"(_1) (_)", %"(_2) (_)")

define void @"(_)"() { entry: ret void }
define %"(_)" @"(_0) (_)"() { entry: ret %"(_)" zeroinitializer }
define %"(_)" @"(_1) (_)"() { entry: ret %"(_)" zeroinitializer }
define %"(_)" @"(_2) (_)"() { entry: ret %"(_)" zeroinitializer }

define void @"(_3 (() (_1))) (`.void`.) (_)" ( %"(_1) (_)" ) { entry: ret void }
define void @"(_4 (() (_2)  (_)) (() (_1) (_)) (() (_1) (_))) (`.void`.) (_)" ( %"(_2) (_)", %"(_1) (_)", %"(_1) (_)" ) { entry: ret void }
