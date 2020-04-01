#include "llvm/AsmParser/Parser.h"              // n: a n3zqx2l compiler written in C++.
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Linker/Linker.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/SourceMgr.h"
const bool debug=true;enum {__,_i,_0,_1,_j,_s,_2,_d,_C}; typedef size_t N;typedef std::vector<std::vector<N>> V; struct F{const char*n=0;char* t=0;N l=0;};
struct L{N i=0;N s=0;N l=0;N c=0;};struct t{N v=0;const char*s=0;N l = 0; N c=0;}; struct e{t t={};std::vector<e> s={};N e=0;};struct r{N i=0;std::vector<r> a={};t t={};N e=0;}; struct E{std::vector<r> s={};r t={};r d={};};typedef std::vector<E> W;
static t n(L&l,F&f){t t{};for(N&i=l.i;i<f.l;i++){
        N c=f.t[i];if(c=='\"'&&!l.s)t={l.s=1,f.t+i+1,l.l,l.c};
        else if(!isspace(c)&&!l.s){i++;return{c,"",l.l,l.c++};}else if(c=='\"'&&l.s){l.s=false;l.c++;f.t[i++]='\0';return t;}if (c=='\n'){l.l++;l.c=1;}else l.c++;
    } if(l.s)printf("n3zqx2l: %s:%ld:%ld: error: expected \"\n\n", f.n, l.l, l.c); return t;
} static e p(L&s,F&f){ e l={};L S=s;t t=n(s,f);while(t.v&&t.v!=')'){
        if(t.v==1||!strchr("()",t.v))l.s.push_back({t});else if(t.v=='('){e e=p(s,f);
            if(n(s,f).v!=')')printf("n3zqx2l: %s:%ld:%ld: error: expected )\n\n",f.n,t.l,t.c); l.s.push_back(e);t.v=0;l.s.back().t=t;
        }S=s;t=n(s,f);}s=S;return l;
} static std::string sr(const r& g, const W& E) {
    std::string o="";N i=0;for(r s:E[g.i].s)if(s.t.v==1)o+="\""+std::string(s.t.s)+"\""; else if(s.t.v)o+=s.t.v;else o+="("+sr(g.a.size()?g.a[i++]:s,E)+")"; if (E[g.i].t.i) o += " " + sr(E[g.i].t, E); return o;
} static void d(const std::vector<r>& s, const r& t, const r& d, W& E, V& S) {
    S.back().push_back(E.size()); E.push_back({s, t, d});
    std::stable_sort(S.back().begin(), S.back().end(), [&](N a, N b) {return E[a].s.size()>E[b].s.size();});
    std::stable_sort(S.back().begin(), S.back().end(), [&](N a, N b) {return E[a].s.size()&&!E[a].s.front().t.v;});
} static N in(N c,N t,const V& I){return std::find(I[c].begin(),I[c].end(),t)!=I[c].end();}
static N ne(const r& a,const r& b,const std::vector<N>& f) {
    if(std::find(f.begin(),f.end(),a.i)==f.end())return 0;if(a.i!=b.i||a.a.size()!=b.a.size())return 1; for(N i=0;i<a.a.size();i++)if(ne(a.a[i],b.a[i],f))return 1;return 0;
} static r R(const e&,const r&,W&,V&,V&, const F&,N);
static r ra(const e&g,const r&t,N&i,N&b,N d,N m,W&E,V&S,V&I,const F&f) {
    if(d>m)return{0,{},{},1}; if(i<g.s.size()&&!g.s[i].t.v)return R(g.s[i++],t,E,S,I,f,m);
    if(i<g.s.size()&&in(_s,t.i,I))return {t.i,{},{g.s[i++].t.v}};
    N si=i;V Z=S;for(N s:Z.back()){ b=std::max(i,b);i=si;S=Z;std::vector<r> A={};
        if(ne(E[s].t,t,S.back()))goto c; for(N j = 0;j<E[s].s.size();j++){
            if(i>=g.s.size()) {if(A.size()&&j==1)return A[0];else goto c;}
            if(!E[s].s[j].t.v) { r a = ra(g,E[E[s].s[j].i].t,i,b,d+1,m,E,S,I,f);if(a.e||ne(E[s].t,t,S.back()))goto c;else A.push_back({a});
            }else if(E[s].s[j].t.v!=g.s[i].t.v)goto c;else i++;
        }return{s,A};c:continue;}return{0,{},{},1};
} static r R(const e& g,const r& T,W&E,V&S,V&I,const F& f,N m) {
    N i = 0, b = 0; r s = ra(g,T,i,b,0,m,E,S,I,f); if (i < g.s.size()) s.e = 1; if (s.e) { t B = b < g.s.size()?g.s[b].t:g.t; printf("n3zqx2l: %s:%ld:%ld: error: %s: unresolved %c\n", f.n,B.l,B.c, sr(T, E).c_str(), (char)B.v); } return s;
} static void sd(std::unique_ptr<llvm::Module>& m) {
    m->setTargetTriple(llvm::sys::getDefaultTargetTriple()); std::string e = "";
    auto t = llvm::TargetRegistry::lookupTarget(m->getTargetTriple(), e)->createTargetMachine(m->getTargetTriple(), "generic", "", {}, {}, {}); ///TODO: make this not generic!
    m->setDataLayout(t->createDataLayout());
} static llvm::Value* generate_expression(const r& g, W& E, V& S, llvm::Module* m, llvm::Function* f, llvm::IRBuilder<>& b) {
    return 0;
} static std::unique_ptr<llvm::Module> g(const r& g, W&E,V&S,V&I,const F& f, llvm::LLVMContext& C, N is_main) {
    if (debug) printf("\n\n\t%s\n\n", sr(g, E).c_str());
    auto m = llvm::make_unique<llvm::Module>(f.n, C); return m;
} static std::unique_ptr<llvm::Module> optimize(std::unique_ptr<llvm::Module>& m) {
    if (debug) {
        printf("\n\n\n\n-------- printing the final state of the module before output:------ \n\n");
        m->print(llvm::errs(), nullptr);
    } std::string er = "";
    if (llvm::verifyModule(*m, &(llvm::raw_string_ostream(er) << ""))) printf("llvm: %s: error: %s\n", "init.n", er.c_str()); return std::move(m);
} static void interpret(std::unique_ptr<llvm::Module> m, const std::vector<std::string> a) {
    auto e = llvm::EngineBuilder(std::move(m)).setEngineKind(llvm::EngineKind::JIT).create(); e->finalizeObject();
    if (auto main = e->FindFunctionNamed("main"); main) exit(e->runFunctionAsMain(main, a, nullptr));
    else printf("n: error: could not find entry point\n");
} static std::string G(std::unique_ptr<llvm::Module> m, const char* n, llvm::TargetMachine::CodeGenFileType type) {
    std::error_code error; if (type == llvm::TargetMachine::CGFT_Null) {
        llvm::raw_fd_ostream dest(std::string(n) + ".ll", error, llvm::sys::fs::F_None); m->print(dest, nullptr); return "";
    } std::string le = ""; auto tm = llvm::TargetRegistry::lookupTarget(m->getTargetTriple(), le)->createTargetMachine(m->getTargetTriple(), "generic", "", {}, {}, {}); ///TODO: make this not generic!
    auto of = std::string(n) + (type == llvm::TargetMachine::CGFT_AssemblyFile ? ".s" : ".o");
    llvm::raw_fd_ostream dest(of, error, llvm::sys::fs::F_None);
    llvm::legacy::PassManager pass; tm->addPassesToEmitFile(pass, dest, nullptr, type); pass.run(*m); dest.flush(); return of;
} static void ee(const std::string& o, const std::string& e) {
    std::system(std::string("ld -macosx_version_min 10.15 -lSystem -lc -o " + e + " " + o).c_str()); std::remove(o.c_str());
} static void output(N o, const char* n, const std::vector<std::string>& a, std::unique_ptr<llvm::Module>&& m) {
    if (o == 0) interpret(std::move(m), a); if (o == 'i') G(std::move(m), n, llvm::TargetMachine::CGFT_Null);
    if (o == 'c') G(std::move(m), n, llvm::TargetMachine::CGFT_ObjectFile); if (o == 's') G(std::move(m), n, llvm::TargetMachine::CGFT_AssemblyFile);
    if (o == 'o') ee(G(std::move(m), n, llvm::TargetMachine::CGFT_ObjectFile), n);
} int main(const int ac, const char** av) {
    llvm::InitializeAllTargetInfos();llvm::InitializeAllTargets();llvm::InitializeAllTargetMCs();llvm::InitializeAllAsmParsers();llvm::InitializeAllAsmPrinters();
    llvm::LLVMContext C;auto m=llvm::make_unique<llvm::Module>("",C);
    N o=0,M=100,ea=0,nf=1;const char*en="";std::vector<std::string>A={};
    for(long i=1;i<ac;i++){if(av[i][0]=='-'){
            N c=av[i][1];if(ea)A.push_back(av[i]);else if(c=='-')ea=1;
            else if(c=='u'){puts("usage: n [-u/-v] [-o <exe>/-c <object>/-i <ir>/-s <assembly>] {[-d <depth>] <.n/.ll/.o/.s>}* [-- {<argv>}*]");exit(0);}
            else if(c=='v'){puts("n3zqx2l: 0.0.4 \tn: 0.0.4");exit(0);}
            else if(c=='d'&&i+1<ac)M=atol(av[++i]);else if(strchr("ocis",c)&&i+1<ac){o=c;en=av[++i];}
            else{printf("n: error: bad option: %s\n",av[i]);continue;}
        }else{const char* ex = strrchr(av[i], '.');if(ex&&!strcmp(ex, ".n")) {
                FILE* w = fopen(av[i], "r");if(!w){printf("n: %s: error: %s\n",av[i],strerror(errno));continue;}
                fseek(w,0,SEEK_END);N l=ftell(w);char* T=(char*)calloc(l+1,sizeof(char));
                fseek(w,0,SEEK_SET);fread(T,sizeof(char),l,w);F f={av[i],T,l};L L{0,0,1,1};
                std::vector<E>E{{}};V S{{}},I(_C,std::vector<N>{});for(N i=__;i<_C;i++)I[i].push_back(i);
                d({{0,{},{'i'}}},{},{},E,S);d({{0,{},{'x'}}},{_i},{},E,S);d({{0,{},{'y'}}},{_i},{},E,S);d({{0,{},{'j'}},{_0},{_1}},{_i},{},E,S);d({{0,{},{'s'}}},{_i},{},E,S);d({{0,{},{'p'}}},{_s},{},E,S);d({{0,{},{'d'}},{_2}},{_i},{},E,S);d({},{_i},{},E,S);
                if (llvm::Linker::linkModules(*m,g(R(p(L,f),{_i},E,S,I,f,M),E,S,I,f,C,nf)))continue;
            }else if (ex&&!strcmp(ex, ".ll")) {
                llvm::SMDiagnostic er; std::string es = "";
                auto _m = llvm::parseAssemblyFile(av[i], er, C);
                if(!_m) {er.print("llvm",llvm::errs());continue;}else sd(_m);
                if(llvm::verifyModule(*_m,&(llvm::raw_string_ostream(es)<<""))){printf("llvm: %s: error: %s\n", av[i], es.c_str()); continue; }
                if(llvm::Linker::linkModules(*m,std::move(_m))) continue;
            }else{printf("n: error: cannot process file \"%s\" with extension \"%s\"\n",av[i],ex);continue;}nf=0;
        }}if(nf)printf("n: error: no input files\n");else output(o,en,A,optimize(m));
}
