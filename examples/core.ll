%"(_)" = type opaque



%"(__)" = type opaque
%"(_llvm)" = type opaque
%"(_none) (_)" = type opaque
%"(_a) (_)" = type opaque
%"(_b) (_)" = type opaque

@g = global i32 100, align 4


define void @"(_c) (_llvm)"() {
entry:
    ret void
}
define void @"(_d (() (_b) (_)) (() (_)) (() (_a) (_)) (() (_a) (_))) (_llvm)"(%"(_b) (_)", %"(_)", %"(_a) (_)", %"(_a) (_)") {
entry:
    ret void
}
define void @"(_e (() (_a) (_))) (_llvm)"(%"(_a) (_)") {
entry:
    ret void
}

declare void @types-no-discard(%"(_)", %"(__)", %"(_llvm)", %"(_none) (_)", %"(_a) (_)", %"(_b) (_)");




; ----------- user defined land --------------

%"(unit) (_)" = type {}

define %"(unit) (_)" @"() (unit) (_)"() {
entry:
    ret %"(unit) (_)" zeroinitializer
}

; chain abstraction.

define %"(_)" @"((() (_)) (() (_))) (_)"(%"(_)", %"(_)") {
entry:
    ret %"(_)" %1
}

; a ud type, hello.

define %"(_)" @hello() {
entry:
    ret %"(_)" zeroinitializer
}

