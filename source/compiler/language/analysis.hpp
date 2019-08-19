//
//  analysis.hpp
//  language
//
//  Created by Daniel Rehman on 1901314.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#ifndef analysis_hpp
#define analysis_hpp

#include "arguments.hpp"
#include "nodes.hpp"
#include "llvm/IR/IRBuilder.h"

struct translation_unit_data {
    struct file file;
    llvm::Module* module;
    llvm::IRBuilder<>& builder;
};

struct flags {
    bool should_allow_undefined_signatures = false;    
    bool should_generate_code = false;
    bool is_at_top_level = false; 
    bool is_parsing_type = false;
    
    struct flags& allow_undefined() { should_allow_undefined_signatures = true; return *this; }
    struct flags& dont_allow_undefined() { should_allow_undefined_signatures = false; return *this; }    
    struct flags& generate_code() { should_generate_code = true; return *this; }
    struct flags& dont_generate_code() { should_generate_code = false; return *this; }    
    struct flags& at_top_level() { is_at_top_level = true; return *this; }
    struct flags& not_at_top_level() { is_at_top_level = false; return *this; }    
    struct flags& parsing_a_type() { is_parsing_type = true; return *this; }    
    struct flags& not_parsing_a_type() { is_parsing_type = false; return *this; }
    
    flags(bool given_should_allow_undefined_signatures = false,
          bool given_should_generate_code = false, 
          bool given_is_at_top_level = false,
          bool given_is_parsing_type = false
          ):  
    should_allow_undefined_signatures(given_should_allow_undefined_signatures), 
    should_generate_code(given_should_generate_code), 
    is_at_top_level(given_is_at_top_level), 
    is_parsing_type(given_is_parsing_type) {}    
}; 


using nat = size_t;

struct stack_frame {
    llvm::ValueSymbolTable* llvm = nullptr;
    std::vector<nat> indicies = {};   // index into master.   
};

struct signature_entry {
    expression signature = {};
    abstraction_definition definition = {};
    std::vector<nat> parents = {}; // index into master.
}; 

class symbol_table {
public:    
    std::vector<struct signature_entry> master = {};
    std::vector<struct stack_frame> frames = {};
    std::vector<nat> blacklist = {}; // index into master.
    
    void push() {frames.push_back({});}    
    void pop() {frames.pop_back();}
    std::vector<nat>& top() {return frames.back().indicies;}    
    expression& lookup(nat index) {return master[index].signature;}
    

    void define(expression signature, abstraction_definition definition, 
                nat stack_frame_index, std::vector<nat> parents = {} ) {
        
        /// if in blacklist, 
                    /// remove from blacklist.
        
        frames[frames.size() - 1 - stack_frame_index].indicies.push_back(master.size()); 
        master.push_back({signature, definition, parents});
    }
    
    void undefine(nat signature_index, nat stack_frame_index) {
        blacklist.push_back(signature_index);
        //frames[frames.size() - 1 - stack_frame_index].indicies.push_back(master.size());  // TODO: make this into a    "remove_if(erase(signature_index));"
        
    }
    
    void disclose(nat desired_signature, expression new_signature, 
                  nat source_abstraction, nat destination_frame) {
        
    }

    symbol_table() {        
        push();
    }
};

struct state {
    symbol_table& stack;    
    translation_unit_data& data;
    bool& error;
};

expression csr(expression given, size_t& index, const size_t depth, const size_t max_depth, state& state, flags flags); 
void adp(abstraction_definition& given, state& state, flags flags);
expression res(expression given, state& state, flags flags);

std::unique_ptr<llvm::Module> analyze(translation_unit unit, llvm::LLVMContext& context, struct file file);

#endif /* analysis_hpp */
