#ifndef analysis_hpp
#define analysis_hpp

#include "analysis_ds.hpp"

llvm_module analyze(expression_list unit, const file& file, llvm::LLVMContext& context);

#endif /* analysis_hpp */
