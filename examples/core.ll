%"(_)" = type opaque
%"(_0) (_)" = type opaque
%"(_1) (_)" = type opaque
%"(_2) (_)" = type opaque

%"(__llvm)" = type opaque
%"(__infered)" = type opaque

define void @"(_)"() { entry: ret void }
define %"(_)" @"(_0) (_)"() { entry: ret %"(_)" zeroinitializer }
define %"(_)" @"(_1) (_)"() { entry: ret %"(_)" zeroinitializer }
define %"(_)" @"(_2) (_)"() { entry: ret %"(_)" zeroinitializer }





define void @"" () { entry: ret void }




define void @"(__infered)"() { entry: ret void }
define void @"(__llvm)"() { entry: ret void }








define void @"(_3 (() (_1))) (`.void`.) (_)"
( %"(_1) (_)" ) { entry: ret void }

define void
@"(_4 (() (_2) (_)) (() (_1) (_)) (() (_1) (_))) (`.void`.) (_)"
(
%"(_2) (_)",
%"(_1) (_)",
%"(_1) (_)"
) { entry: ret void }










declare void @"__intrinsic-types-no-discard"(%"(_)", %"(__infered)", %"(__llvm)", %"(_0) (_)", %"(_1) (_)", %"(_2) (_)")

; ------ throw away code -------
; %"(unit) (_)" = type {}
; define %"(unit) (_)" @"() (unit) (_)"() { entry: ret %"(unit) (_)" zeroinitializer }
; define %"(_)" @"(((f) (_)) (() (_))) (_)"(%"(_)", %"(_)") { entry: ret %"(_)" %1 }
; define %"(_)" @hello() { entry: ret %"(_)" zeroinitializer }
