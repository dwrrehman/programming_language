%"(_)" = type opaque ; compiler defined.

%"(_0) (_)" = type {}
%"(_1) (_)" = type {}
%"(_2) (_)" = type {}

define %"(_)" @"(_0) (_)"() { entry: ret %"(_)" undef }
define %"(_)" @"(_1) (_)"() { entry: ret %"(_)" undef }
define %"(_)" @"(_2) (_)"() { entry: ret %"(_)" undef }

define %"(_)" @"(_3 (() (_1))) (_)" ( %"(_1) (_)" ) { entry: ret %"(_)" undef  }

define %"(_)" @"(_4 (() (_2) (_)) (() (_1) (_)) (() (_1) (_))) (_)" ( %"(_2) (_)", %"(_1) (_)", %"(_1) (_)" ) { entry: ret %"(_)" undef  }
