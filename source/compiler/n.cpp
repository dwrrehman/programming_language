#include "llvm/AsmParser/Parser.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LegacyPassManager.h"  // n: a n3zqx2l compiler written in C++.
#include "llvm/IR/Verifier.h"
#include "llvm/Linker/Linker.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"

const bool debug = 1;

enum { _o, _i, _s, _n, _j0, _j1, _j, _a0, _a, _d0, _d1, _d, _C };

typedef size_t N;

typedef std::vector<std::vector<N>> V;

struct F{const char *n,*t;};
struct t{N v,l,c;};
struct e{std::vector<e>s;t t;};
/// TODO: merge these two structs.
struct r {
    N i;
    std::vector<r> a;
    t t;
    N error;
};

struct E {
    std::vector<r> s;
    r t, d;
};

typedef std::vector<E> W;

static t n(t&l,F&f){while(isspace(f.t[l.v])){if(f.t[l.v++]!=10)l.c++;else{l.l++;l.c=1;}}return{(N)f.t[l.v++],l.l,l.c++};}

static e p(t &s, F &f) {
    e l{};
    t S = s;
    t t = n(s, f);
    while (t.v && t.v != ')') {
        if (t.v == '(') {
            e e = p(s, f);
            if (n(s, f).v != ')')
                printf("n3zqx2l: %s:%ld:%ld: error: expected )\n\n",
                       f.n, t.l, t.c);
            e.t.l = t.l, e.t.c = t.c, e.t.v = 0;
            l.s.push_back(e);
        } else l.s.push_back({{}, t});
        if (t.v) t = n(S = s, f);
    }
    s = S;
    return l;
}

static std::string sr(const r &g, const W &E) {
    std::string o = "";
    N i = 0;
    for (r s : E[g.i].s)
        if (s.t.v)
            o += s.t.v;
        else
            o += "(" + sr(g.a.size() ? g.a[i++] : s, E) + ")";

    if (E[g.i].t.i) o += " " + sr(E[g.i].t, E);

    return o;
}

static void d(const std::vector<r> &s, const r &t, const r &d, W &E, V &S) {
    E.push_back({s, t, d});
    S.back().insert(
        std::upper_bound(S.back().begin(), S.back().end(), E.size() - 1,
                         [E](N ai, N bi) {
                             if (E[ai].s.size() &&
                                 !E[ai].s[0].t.v !=
                                     (E[bi].s.size() && !E[bi].s[0].t.v))
                                 return E[ai].s.size() && !E[ai].s[0].t.v;
                             return E[ai].s.size() > E[bi].s.size();
                         }),
        E.size() - 1);
}

static N in(N c, N t, const V &I) {
    return std::find(I[c].begin(), I[c].end(), t) != I[c].end();
}

static N ne(const r &a, const r &b, const V &S) {
    if (std::find(S.back().begin(), S.back().end(), a.i) == S.back().end())
        return 0;

    if (a.i != b.i || a.a.size() != b.a.size())
        return 1;

    for (N i = 0; i < a.a.size(); i++)
        if (ne(a.a[i], b.a[i], S))
            return 1;
    return 0;
}
static r R(const e &, const r &, W &, V &, V &, const F &, N);
static r O(const char *n, const r &T, W &E, V &S, V &I, N m) {
    
    FILE *w = fopen(n, "r");
    
    if (!w) {
        printf("n: %s: error: %s\n", n, strerror(errno));
        return {0, {}, {}, 1};
    }
    
    fseek(w, 0, 2);
    N l = ftell(w);
    char *B = (char *)calloc(l + 1, sizeof(char));
    fseek(w, 0, 0);
    fread(B, sizeof(char), l, w);
    fclose(w);
    
    F f {n, B};
    t L {0, 1, 1};
    return R(p(L, f), T, E, S, I, f, m);
}
static r u(const e &g, const r &t, N &i, N &b, N D, N m, W &E, V &S, V &I,
           const F &f) {
    if (D > m) return {0, {}, {}, 1};

    if (i < g.s.size() && !g.s[i].t.v) return R(g.s[i++], t, E, S, I, f, m);

    if (i < g.s.size() && in(_s, t.i, I)) return {t.i, {}, g.s[i++].t, 0};

    N si = i;
    V Z = S;

    for (N s : Z.back()) {
        b = fmax(i, b);
        i = si;
        S = Z;
        std::vector<r> A = {};
        if (ne(E[s].t, t, S)) goto c;

        for (N j = 0; j < E[s].s.size(); j++) {
            if (i >= g.s.size()) {
                if (A.size() && j == 1)
                    return A[0];
                else
                    goto c;
            }

            if (!E[s].s[j].t.v) {
                r a = u(g, E[E[s].s[j].i].t, i, b, D + 1, m, E, S, I, f);
                if (a.error)  // ||ne(E[s].t,t,S)
                    goto c; ///TODO: CURRENT ERROR RIGHT HERE!
                A.push_back({a});
            } else if (E[s].s[j].t.v != g.s[i].t.v)
                goto c;
            else
                i++;
        }
        if (in(_d, s, I)) {
            auto ccc = A[0].t.v;
            if (ccc - 42 < _C && ccc >= 42)
                I[ccc - 42].push_back(E.size());
            
//            d({{0, {}, {'K', 0, 0, 0}}}, {_i}, {}, E, S);
        }
        return {s, A, {}, 0};
    c:
        continue;
    }
    return {0, {}, {}, 1};
}
static r R(const e &g, const r &T, W &E, V &S, V &I, const F &f, N m) {
    N i = 0, b = 0;
    r s = u(g, T, i, b, 0, m, E, S, I, f);

    if (i < g.s.size()) s.error = 1;

    if (s.error) {
        t B = b < g.s.size() ? g.s[b].t : g.t;
        printf("n3zqx2l: %s:%ld:%ld: error: %s: unresolved %c\n", f.n, B.l, B.c,
               sr(T, E).c_str(), (char)B.v);
    }
    return s;
}
static void sd(std::unique_ptr<llvm::Module> &m) {
    m->setTargetTriple(llvm::sys::getDefaultTargetTriple());
    std::string e = "";
    auto t = llvm::TargetRegistry::lookupTarget(m->getTargetTriple(), e)
                 ->createTargetMachine(m->getTargetTriple(), "generic", "", {},
                                       {}, {});
    m->setDataLayout(t->createDataLayout());
}

//////////// DEBUG CODE ////////////////

void prep(N d) {
    for (N i = 0; i < d; i++) printf(".   ");
}

static inline void debug_intrinsics(V intrinsics) {
    if (not debug) return;
    printf("\n---- debugging intrinsics: ----\n");
    for (N i = 0; i < intrinsics.size(); i++) {
        if (intrinsics[i].empty()) continue;
        printf("\t ----- INTRINSIC ID # %lu ---- \n\t\tsignatures: { ", i);
        for (auto index : intrinsics[i]) printf("%lu ", index);
        printf("}\n\n");
    }
    printf("\n--------------------------------\n");
}

static inline void debug_resolved(r e, W entries, N depth = 0) {
    if (not debug) return;
    if (e.error) {
        prep(depth);
        printf("[ERROR]\n");
    }
    prep(depth);
    printf("%lu :   index = %lu  ::   %s\n", depth, e.i,
           sr({e.i, {}, {}, 0}, entries).c_str());
    if (e.t.v) {
        prep(depth);
        printf("literal:   %lu : '%c'  @(%lu, %lu)\n", e.t.v, (char)e.t.v,
               e.t.l, e.t.c);
    }
    prep(depth);
    puts("");
    N i = 0;
    for (auto arg : e.a) {
        prep(depth + 1);
        printf("#%lu: \n", i++);
        debug_resolved(arg, entries, depth + 1);
    }
}

static inline void debug_stack(W entries, V stack) {
    if (not debug) return;

    printf("\n---- debugging stack: ----\n");
    printf("printing frames: \n");

    for (N i = 0; i < stack.size(); i++) {
        printf("\t ----- FRAME # %lu ---- \n\t\tidxs: { ", i);

        for (auto index : stack[i]) printf("%lu ", index);

        puts("}");
    }
    printf("\nmaster: {\n");

    N j = 0;
    for (auto entry : entries) {
        printf("\t%6lu: ", j);

        printf("%s", sr({j, {}, {}, 0}, entries).c_str());

        auto def = sr(entry.d, entries);
        if (def.size() and entry.d.i) printf("      =    %s", def.c_str());

        puts("\n");
        j++;
    }
    puts("}");
}

///////////////////////////////////////

static std::unique_ptr<llvm::Module> g(const r &g, W &E, V &S, V &I,
                                       const char *n, llvm::LLVMContext &C,
                                       N nf) {
    if (debug) {
        printf("\n\ndebugging resolved:\n");
        debug_resolved(g, E);
        printf("\n\nrepresenting resolved:\n\t");
        puts(sr(g, E).c_str());
        printf("\n\nprinting entries/stack:\n");
        debug_stack(E, S);
        printf("\n\nprinting intrinsics:\n");
        debug_intrinsics(I);
        printf("\n\n");
    }
    if (debug) printf("\n\n\t%s\n\n", sr(g, E).c_str());
    if (g.error) exit(1);

    auto m = llvm::make_unique<llvm::Module>(n, C);
    return m;
}

static std::unique_ptr<llvm::Module> optimize(
    std::unique_ptr<llvm::Module> &m) {
    if (debug) {
        printf(
            "\n\n\n\n-------- printing the final state of the module before "
            "output:------ \n\n");
        m->print(llvm::errs(), nullptr);
    }
    std::string er = "";
    if (llvm::verifyModule(*m, &(llvm::raw_string_ostream(er) << "")))
        printf("llvm: %s: error: %s\n", "init.n", er.c_str());
    return std::move(m);
}

static void x(std::unique_ptr<llvm::Module> m,
              const std::vector<std::string> a) {
    auto e = llvm::EngineBuilder(std::move(m))
                 .setEngineKind(llvm::EngineKind::JIT)
                 .create();
    e->finalizeObject();

    if (auto main = e->FindFunctionNamed("main"); main)
        exit(e->runFunctionAsMain(main, a, nullptr));
    else
        printf("n: error: could not find entry point\n");
}

static std::string G(std::unique_ptr<llvm::Module> m, const char *n,
                     llvm::TargetMachine::CodeGenFileType t) {
    std::error_code er;
    if (t == llvm::TargetMachine::CGFT_Null) {
        llvm::raw_fd_ostream d(std::string(n) + ".ll", er,
                               llvm::sys::fs::F_None);
        m->print(d, nullptr);
        return "";
    }
    std::string le = "";
    auto tm =
        llvm::TargetRegistry::lookupTarget(m->getTargetTriple(), le)
            ->createTargetMachine(m->getTargetTriple(), "generic", "", {}, {},
                                  {});  /// TODO: make this not generic!
    auto of = std::string(n) +
              (t == llvm::TargetMachine::CGFT_AssemblyFile ? ".s" : ".o");
    llvm::raw_fd_ostream d(of, er, llvm::sys::fs::F_None);
    llvm::legacy::PassManager p;
    tm->addPassesToEmitFile(p, d, nullptr, t);
    p.run(*m);
    d.flush();
    return of;
}

static void ee(const std::string &o, const std::string &e) {
    std::system(std::string("ld -macosx_version_min 10.15 -lSystem -lc -o " +
                            e + " " + o)
                    .c_str());
    std::remove(o.c_str());
}

static void output(N o, const char *n, const std::vector<std::string> &a,
                   std::unique_ptr<llvm::Module> &&m) {
    if (!o) x(std::move(m), a);
    if (o == 'i') G(std::move(m), n, llvm::TargetMachine::CGFT_Null);
    if (o == 'c') G(std::move(m), n, llvm::TargetMachine::CGFT_ObjectFile);
    if (o == 's') G(std::move(m), n, llvm::TargetMachine::CGFT_AssemblyFile);
    if (o == 'o')
        ee(G(std::move(m), n, llvm::TargetMachine::CGFT_ObjectFile), n);
}

int main(int ac, const char **av) {
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();
    llvm::LLVMContext C;
    auto m = llvm::make_unique<llvm::Module>("", C);
    N o = 0, M = 100, ea = 0, nf = 1;
    const char *en = "";
    std::vector<std::string> A = {};
    for (long i = 1; i < ac; i++) {
        if (ea) {
            while (i < ac)
                A.push_back(av[i++]);
        
        } else if (av[i][0] == '-') {
            N c = av[i][1];
           
            if (c == '-')
                ea = 1;
            else if (c == 'u') {
                puts(
                    "usage: n [-u/-v] [-o <exe>/-c <object>/-i <ir>/-s "
                    "<assembly>] "
                    "{[-d <depth>] <.n/.ll/.o/.s>}* [-- {<argv>}*]");
                exit(0);
            } else if (c == 'v') {
                puts("n3zqx2l: 0.0.4 \tn: 0.0.4");
                exit(0);
            } else if (c == 'd' && i + 1 < ac)
                M = atol(av[++i]);
            else if (strchr("ocis", c) && i + 1 < ac) {
                o = c;
                en = av[++i];
            } else {
                printf("n: error: bad option: %s\n", av[i]);
                continue;
            }
        } else {
            const char *ex = strrchr(av[i], '.');
            if (ex && !strcmp(ex, ".n")) {
                std::vector<E> E {};
                V S{{}}, I(_C, std::vector<N>{});

                for (N i = 0; i < _C; i++) I[i].push_back(i);

//                d({{0, {}, {'o', 0, 0, 0}}}, {}, {}, E, S);
                
                d({{0, {}, {'o', 0, 0}, 0}}, {}, {}, E, S);
                d({{0, {}, {'i', 0, 0}, 0}}, {}, {}, E, S);
                d({{0, {}, {'s', 0, 0}, 0}}, {_i, {}, {}, 0}, {}, E, S);
//                d({{0, {}, {'i', 0, 0, 0}}}, {}, {}, E, S);
                
                
                
//                d({{0, {}, {'n', 0, 0, 0}}}, {_i}, {}, E, S);  // ?
//
//                d({{0, {}, {'x', 0, 0, 0}}}, {_i}, {}, E, S);
//                d({{0, {}, {'y', 0, 0, 0}}}, {_i}, {}, E, S);
//                d({/*{0,{},{'j'}},*/ {_j0}, {_j1}}, {_i}, {}, E, S);
//
//                d({{0, {}, {'w', 0, 0, 0}}}, {_s}, {}, E, S);
//                d({{_a0}}, {_i}, {}, E, S);
//
//                d({{0, {}, {'u', 0, 0, 0}}}, {_s}, {}, E, S);
//                d({{0, {}, {'v', 0, 0, 0}}}, {_i}, {}, E, S);
//                d({{0, {}, {'b', 0, 0, 0}}, {_d0}, {_d1}}, {_i}, {}, E, S);

                if (llvm::Linker::linkModules(*m, g(O(av[i], {_i, {}, {}, 0}, E, S, I, M),
                                                    E, S, I, av[i], C, nf)))
                    continue;

            } else if (ex && !strcmp(ex, ".ll")) {
                llvm::SMDiagnostic er;
                std::string es = "";

                auto _m = llvm::parseAssemblyFile(av[i], er, C);
                if (!_m) {
                    er.print("llvm", llvm::errs());
                    continue;
                } else
                    sd(_m);

                if (llvm::verifyModule(*_m,
                                       &(llvm::raw_string_ostream(es) << "")) ||
                    llvm::Linker::linkModules(*m, std::move(_m))) {
                    printf("llvm: %s: error: %s\n", av[i], es.c_str());
                    continue;
                }
            } else {
                printf(
                    "n: error: cannot process file \"%s\" with extension "
                    "\"%s\"\n",
                    av[i], ex);
                continue;
            }
            nf = 0;
        }
    }
    if (nf)
        printf("n: error: no input files\n");
    else
        output(o, en, A, optimize(m));
}

//        if (in(_d,s,I)) d({},A[1],A[2],E,S);
//        if (in(_l,s,I)) return lf(A[0].t.s,A[1],E,S,I,m);
