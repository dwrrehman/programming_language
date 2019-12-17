%"(_)" = type opaque
%"(__)" = type opaque
%"(_llvm)" = type opaque
%"(_none) (_)" = type opaque
%"(_a) (_)" = type opaque
%"(_b) (_)" = type opaque

define void @"(_)"() { entry: ret void }
define void @"(__)"() { entry: ret void }
define void @"(_llvm)"() { entry: ret void }
define %"(_)" @"(_none) (_)"() { entry: ret %"(_)" zeroinitializer }
define %"(_)" @"(_a) (_)"() { entry: ret %"(_)" zeroinitializer }
define %"(_)" @"(_b) (_)"() { entry: ret %"(_)" zeroinitializer }
define void @"(_c) (_llvm)"() { entry: ret void }
define void @"(_d (() (_b) (_)) (() (_)) (() (_a) (_)) (() (_a) (_))) (_llvm)"(%"(_b) (_)", %"(_)", %"(_a) (_)", %"(_a) (_)") { entry: ret void }
define void @"(_e (() (_a) (_))) (_llvm)"(%"(_a) (_)") { entry: ret void }

declare void @types-no-discard(%"(_)", %"(__)", %"(_llvm)", %"(_none) (_)", %"(_a) (_)", %"(_b) (_)")

; %"(unit) (_)" = type {}
; define %"(unit) (_)" @"() (unit) (_)"() { entry: ret %"(unit) (_)" zeroinitializer }
; define %"(_)" @"((() (_)) (() (_))) (_)"(%"(_)", %"(_)") { entry: ret %"(_)" %1 }
; define %"(_)" @hello() { entry: ret %"(_)" zeroinitializer }

