


// static inline struct expression clone(struct expression e, 
// 		struct context* context, struct stack_element* stack, nat top) { 

// 	struct expression new = e;
// 	new.args = calloc((size_t) e.count, sizeof(struct expression));

// 	for (nat i = 0; i < e.count; i++)
// 		new.args[i] = clone(e.args[i], context, stack, top);
	
// 	return new;
// }


// static inline void copy_replace(struct expression def, struct expression call, struct expression* out,
// 				struct context* context, struct stack_element* stack, nat top) {

// 	if (def.index >= call.index - call.count and def.index < call.index) { 
// 		*out = clone(call.args[call.count - (call.index - def.index)], context, stack, top);
// 		return;
// 	}
// 	*out = def;
// 	out->args = calloc((size_t) def.count, sizeof(struct expression));

// 	for (nat i = 0; i < def.count; i++) 
// 		copy_replace(def.args[i], call, out->args + i, context, stack, top);

// }

// static inline void expand_macro(struct context* context, struct stack_element* stack, nat top) {
// 	struct expression call = stack[top].data;	
// 	copy_replace(context->names[call.index].def, call, &stack[top].data, context, stack, top);
// }


