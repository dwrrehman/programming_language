
#ifndef comments_h
#define comments_h

// ------------------ previous saves of the CSR prototype code i used ----------------------














/*
 
 
 
 
 
 
 
 
 if (at == head) {
     if (next < size) {
         if (tail < size) m[tail].queue_next = next;
         tail = next;
         struct unit u = {
             .index = m[at].index,
             .parent = this.parent,
             .begin = this.begin,
             .queue_next = size,
         };
         printf("pushed arg {%lu,%lu,%lu,%lu}\n", u.index, u.parent, u.begin, u.queue_next);
         m[next++] = u;
     }
 }
 
 
 
 
 
 
 
 
 
 
 
 
 
 string solve(struct unit* m, nat size) {
     nat head = size, tail = size, next = 0;
     do {
         {puts("queue"); debug_memory(m, size, head, tail, next);}
                 
         if (next < size) {
             if (tail < size) m[tail].queue_next = next;
             tail = next;
             struct unit u = {
                 .index = 0,
                 .parent = head,
                 .begin = head == size ? 0 : m[head].begin,
                 .queue_next = size
             };
             printf("pushed basis {%lu,%lu,%lu,%lu}\n", u.index, u.parent, u.begin, u.queue_next);
             m[next++] = u;
         }
                 
         head = head == size ? 0 : m[head].queue_next;
     
         if (head >= size) break;
         
         for (; m[head].index < strlen(context); m[head].index++) {
             {puts("index"); debug_memory(m, size, head, tail, next);}
             
             for (nat at = head; at < size; at = m[at].parent) {
                 {puts("parent"); debug_memory(m, size, head, tail, next);}
                 
                 nat error = 0;
                 struct unit this = m[at];
                 
                 for (; context[m[at].index] != '.'; m[at].index++) {
                     {display_signature(context, m[at].index); display_string_at_char(input, this.begin);}
                     
                     if (error) continue;
                     else if (context[m[at].index] == '_') {
                         
                         if (at == head) {
                             if (next < size) {
                                 if (tail < size) m[tail].queue_next = next;
                                 tail = next;
                                 struct unit u = {
                                     .index = m[at].index,
                                     .parent = this.parent,
                                     .begin = this.begin,
                                     .queue_next = size,
                                 };
                                 printf("pushed arg {%lu,%lu,%lu,%lu}\n", u.index, u.parent, u.begin, u.queue_next);
                                 m[next++] = u;
                             }
                         }
                         error = 1;
                     } else if (context[m[at].index] == input[this.begin]) this.begin++;
                     else error = 1;
                 }
                 
                 if (error) {
                     puts("signature was erroneous. moving to next index.");
                     break;
                 } else if (this.parent < size) {
                     puts("moving onto next parent...");
                     m[this.parent].index++;
                     m[this.parent].begin = this.begin;
                 } else if (this.begin == strlen(input)) return "success";
                 else {
                     puts("root parent was incomplete. moving to next index...");
                     break;
                 }
             }
         }
         nat save = head;
         head = m[head].queue_next;
         if (m[save].index == strlen(context)) m[save] = undefined;
     } while (head < size);
     return "failure";
 }

 
 
 
 
 
 
 
 
 //                if (!error) {
 //                    if (next < size) {
 ////                        if (tail < size) m[tail].queue_next = next;
 ////                        tail = next;
 //                        struct unit u = {
 //                            .index = m[at].index,
 //                            .parent = this.parent,
 //                            .begin = this.begin,
 //                            .queue_next = size,
 //                        };
 //                        printf("pushed sig {%lu,%lu,%lu,%lu}\n", u.index, u.parent, u.begin, u.queue_next);
 //                        m[next++] = u;
 //                    }
 //                }
 
 
 //static inline void solve(nat* m, nat size) {
 //    m[6] = 0;
 //
 ////    nat tail = 0, next = 4;
 ////
 ////    for (nat head = 4; head; head = m[head + 3]) {
 ////
 ////        debug_memory(m, size, head, tail);
 ////
 ////        m[tail + 3] = next;
 ////        m[next++] = 0;
 ////        m[next++] = head; // parent is head.
 ////        m[next++] = m[head + 2];
 ////        m[next++] = 0;
 ////
 ////        // { 0, head, m[head + 2], 0 }
 ////
 ////        break;
 ////
 ////    }
 //}
 
 
 
 
 
 
 
 
 
 
 void s(struct unit* m, nat size) {
     nat head = 4, tail = 4, next = 8;
     for (; head; head = m[head].queue_next) {
         for (; m[head].index < strlen(context); m[head].index++) {
             for (nat at = head; at; at = m[at].parent) {
                 nat bad = 0;
                 for (; context[m[at].index] != '.'; m[at].index++) {
                     if (bad) continue;
                     else if (context[m[at].index] == '_') {
                         bad = 1;
                     } else if (context[m[at].index] == input[m[at].begin]) m[at].begin++;
                     else bad = 1;
                 }
                 if (bad) break;
                 else if (m[at].parent) {
                     m[m[at].parent].index++;
                     m[m[at].parent].begin = m[at].begin;
                 } else if (m[at].begin == strlen(input)) return;
                 else break;
             }
         }
     }
 }

 
 
 
 
 
 
 void s(struct unit* m, nat size) {
     nat head = 4, tail = 4, next = 8;
     for (; head; head = m[head].queue_next) {
         for (; m[head].index < strlen(context); m[head].index++) {
             for (nat at = head; at; at = m[at].parent) {
                 nat bad = 0;
                 for (; context[m[at].index] != '.'; m[at].index++) {
                     if (bad) continue;
                     else if (context[m[at].index] == '_') {
                         if (at == head) {
                             
                         }
                         bad = 1;
                     } else if (context[m[at].index] == input[m[at].begin]) m[at].begin++;
                     else bad = 1;
                 }
                 if (bad) break;
                 else if (m[at].parent) {
                     m[m[at].parent].index++;
                     m[m[at].parent].begin = m[at].begin;
                 } else if (m[at].begin == strlen(input)) return;
                 else break;
             }
             // after all parents are done, and we havent returned..
         }
         // after we went through all indexes for this node...
     }
     // all possibilities have been tried. fail.
     return;
 }

 

 
 
 
 
 
 
 
 
 
 
 
 for (nat head = 4, tail = 0, next = 4; head; head = m[head + 3]) {
                                    //note: this /might/ be a do-while.... if we want head to go through on first iteration, initing it to zero, but we want to break on it equaling 0.... thats a weird idea...
 
 
 
 
 
 
 
 
 
 
 
 
 
 m[next++] = 0; // starting index of 0.
 m[next++] = head; // parent is head.
 m[next++] = m[head + 2];  // child uses the begin of its parent.
 m[next++] = 0; // we are pushing back this node into the queue, and thus, there is no next node in the queue.

 m[tail + 3] = next;
 // { 0, head, m[head + 2], 0 }
 
 
 
 before:
 
 
 //        m[next++] = 0;
 //        m[next++] = 0;
 //        m[next++] = 0;
 //        m[next++] = 0;
 //        /// { 0, head, m[head + 2], 0 }      <------- im pretty certain this is how you get a new child.... i just dont know where to put it...
 
    0 because 0 is the starting index for the context.
 
    head because our parent is always head- we are supplying a possible argument FOR HEAD- who then by definition is our parent.
 
    m[head + 2]   is saying "head->begin"  that is to say, the childs begin is the same as where the parent left off.    thats correct.
 
    and 0 for the queue next because this node is literally by definition at the back of the queue- ie, we are making this node the new tail! obviously, thats what this whole thing sis about. "what child do we push to the tail? for an arg"
 
 
 so yeah
 
 


 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 static inline void other_solve(nat* m, nat size) {
     string context = "-.hello.bubbles_there_.";
     string input = "bubbleshellotherehello";
     m[6] = 0;
     for (nat head = 4, tail = 4, next = 8; head; head = m[head + 3]) {
         puts("queue"); debug_memory(m, size, head, tail);
         if (tail + 4 > 4 * size) return;
         nat new = tail;
         m[tail++] = 0;
         m[tail++] = head;
         m[tail++] = m[head + 2];
         m[tail++] = 99999;
         puts("after push"); debug_memory(m, size, head, tail);
         for (; m[new + 0] < strlen(context); m[new + 0]++) {
             puts("index"); debug_memory(m, size, head, tail);
             for (nat at = new; at; at = m[at + 1]) {
                 puts("parent"); debug_memory(m, size, head, tail);
                 nat bad = 0;
                 for (; context[m[at + 0]] != '.'; m[at + 0]++) {
                     if (!bad) {
                         display_signature(context, m[at + 0]);
                         display_string_at_char(input, m[at + 2]);
                     }
                     if (bad) continue;
                     else if (context[m[at + 0]] == '_') {
                         bad = 1;
                         if (tail + 4 <= 4 * size) continue;
                         m[tail++] = m[at + 0];
                         m[tail++] = m[at + 1];
                         m[tail++] = m[at + 2];
                         m[tail++] = 99999;
                     } else if (context[m[at + 0]] == input[m[at + 2]]) m[at + 2]++;
                     else bad = 1;
                 }
                 
                 puts("signature done"); debug_memory(m, size, head, tail);
                 
                 if (bad) { puts("next index: invalid signature"); break; }
                 else if (m[at + 1]) { m[m[at + 1] + 0]++; m[m[at + 1] + 2] = m[at + 2]; }
                 else if (m[at + 2] == strlen(input)) {puts("success"); return; }
                 else { puts("next index: b ≠ e."); break; }
             }
         }
         m[new + 0] = 99999;
         m[new + 1] = 99999;
         m[new + 2] = 99999;
         m[new + 3] = 99999;
     }
     puts("failure");
 }

 
 
 
 
 m[new + 0] = 99999;
 m[new + 1] = 99999;
 m[new + 2] = 99999;
 m[new + 3] = 99999;
 
 
 
 
 
 
 
 
 

 
 
 
 
 
 
 
 
 
 
 
 
 
 static inline void solve(nat* m, nat size) {
     string context = "-.hello.bubbles_there_.";
     string input = "bubbleshellotherehello";
     m[6] = 0;
     for (nat head = 4, tail = 4, next = 8; head; head = m[head + 3]) {
         puts("queue"); debug_memory(m, size, head, tail);
         if (tail + 4 > 4 * size) return;
         nat new = tail;
         m[tail++] = 0;
         m[tail++] = head;
         m[tail++] = m[head + 2];
         m[tail++] = 99999;
         puts("after push"); debug_memory(m, size, head, tail);
         for (; m[new + 0] < strlen(context); m[new + 0]++) {
             puts("index"); debug_memory(m, size, head, tail);
             for (nat at = new; at; at = m[at + 1]) {
                 puts("parent"); debug_memory(m, size, head, tail);
                 nat bad = 0;
                 for (; context[m[at + 0]] != '.'; m[at + 0]++) {
                     if (!bad) {
                         display_signature(context, m[at + 0]);
                         display_string_at_char(input, m[at + 2]);
                     }
                     if (bad) continue;
                     else if (context[m[at + 0]] == '_') {
                         bad = 1;
                         if (tail + 4 <= 4 * size) continue;
                         m[tail++] = m[at + 0];
                         m[tail++] = m[at + 1];
                         m[tail++] = m[at + 2];
                         m[tail++] = 99999;
                     } else if (context[m[at + 0]] == input[m[at + 2]]) m[at + 2]++;
                     else bad = 1;
                 }
                 
                 puts("signature done"); debug_memory(m, size, head, tail);
                 
                 if (bad) { puts("next index: invalid signature"); break; }
                 else if (m[at + 1]) { m[m[at + 1] + 0]++; m[m[at + 1] + 2] = m[at + 2]; }
                 else if (m[at + 2] == strlen(input)) {puts("success"); return; }
                 else { puts("next index: b ≠ e."); break; }
             }
         }
         m[new + 0] = 99999;
         m[new + 1] = 99999;
         m[new + 2] = 99999;
         m[new + 3] = 99999;
     }
     puts("failure");
 }
 
 
 
 
 ///
 /// this is my second try at making a more memory efficient bfs csr algorithm.
 /// i reworked the data structures a ton, and am making things simpler, hopefully.

 

 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 static inline void solve(nat* m, nat size) {
     string context = "-.hello.bubbles_there_.";
     string input = "bubbleshellotherehello";
     m[6] = 0;
     for (nat head = 4, tail = 4, next = 8; head; head = m[head + 3]) {
         puts("queue"); debug_memory(m, size, head, tail);
         if (tail + 4 > 4 * size) return;
         nat new = tail;
         m[tail++] = 0;
         m[tail++] = head;
         m[tail++] = m[head + 2];
         m[tail++] = 99999;
         puts("after push"); debug_memory(m, size, head, tail);
         for (; m[new + 0] < strlen(context); m[new + 0]++) {
             puts("index"); debug_memory(m, size, head, tail);
             for (nat at = new; at; at = m[at + 1]) {
                 puts("parent"); debug_memory(m, size, head, tail);
                 nat bad = 0;
                 for (; context[m[at + 0]] != '.'; m[at + 0]++) {
                     if (!bad) {
                         display_signature(context, m[at + 0]);
                         display_string_at_char(input, m[at + 2]);
                     }
                     if (bad) continue;
                     else if (context[m[at + 0]] == '_') {
                         bad = 1;
                         if (tail + 4 <= 4 * size) continue;
                         m[tail++] = m[at + 0];
                         m[tail++] = m[at + 1];
                         m[tail++] = m[at + 2];
                         m[tail++] = 99999;
                     } else if (context[m[at + 0]] == input[m[at + 2]]) m[at + 2]++;
                     else bad = 1;
                 }
                 
                 puts("signature done"); debug_memory(m, size, head, tail);
                 
                 if (bad) { puts("next index: invalid signature"); break; }
                 else if (m[at + 1]) { m[m[at + 1] + 0]++; m[m[at + 1] + 2] = m[at + 2]; }
                 else if (m[at + 2] == strlen(input)) {puts("success"); return; }
                 else { puts("next index: b ≠ e."); break; }
             }
         }
         m[new + 0] = 99999;
         m[new + 1] = 99999;
         m[new + 2] = 99999;
         m[new + 3] = 99999;
     }
     puts("failure");
 }

 
 
 //static inline void verbose_solve() {
 //
 //    nat head = 4, tail = 4, next = 8;
 //
 //    for (; head; head = memory[head + 3]) {
 //
 //        puts("queue state: (before pushed new)"); debug_memory(head, tail);
 //
 //        if (tail + 4 > 4 * size) return;
 //
 //        nat new = tail;
 //        memory[tail++] = 0;
 //        memory[tail++] = head;
 //        memory[tail++] = memory[head + 2];
 //        memory[tail++] = undefined;
 //
 //        puts("queue state: (after pushed new)"); debug_memory(head, tail);
 //
 //        for (; memory[new + 0] < strlen(context); memory[new + 0]++) {
 //
 //            puts("index state: "); debug_memory(head, tail);
 //
 //            for (nat at = new; at; at = memory[at + 1]) {
 //
 //                puts("parent state:"); debug_memory(head, tail);
 //
 //                nat
 //                    bad = 0,
 //                    index = memory[at + 0],
 //                    parent = memory[at + 1],
 //                    begin = memory[at + 2],
 //                    qnext = memory[at + 3];
 //
 //                for (; context[index] != '.'; index++) {
 //                    if (bad) continue;
 //
 //                    display_signature(index);
 //                    display_string_at_char(begin);
 //                    printf("[LOCAL]: index = %lu, parent = %lu, begin = %lu\n\n", index, parent, begin);
 //
 //                    if (0) {}
 //                    else if (context[index] == '_') {
 //                        if (tail + 4 <= 4 * size) {
 //                            puts("ARG!\n");
 //
 //                            memory[tail++] = index;
 //                            memory[tail++] = parent;
 //                            memory[tail++] = begin;
 //                            memory[tail++] = undefined;
 //
 //                        } else puts("ARG PUSH FAILED.");
 //                        debug_memory(head, tail);
 //                        bad = 1;
 //
 //                    } else if (context[index] == input[begin]) begin++;
 //                    else { puts("character mismatch in signature"); bad = 1; }
 //                }
 //
 //                puts("reached end of signature: "); debug_memory(head, tail);
 //
 //                memory[at + 0] = index;
 //                memory[at + 1] = parent;
 //                memory[at + 2] = begin;
 //                memory[at + 3] = qnext;
 //
 //                puts("after updating (AT): "); debug_memory(head, tail);
 //
 //                if (bad) { puts("bad signature, moving to next index."); break; }
 //                else if (parent) {
 //                    memory[parent + 0]++;
 //                    memory[parent + 2] = begin;
 //                } else if (begin == strlen(input)) { puts("\n\t [parse successful]\n"); return; }
 //                else { puts("begin ≠ end & root. moving to next index."); break; }
 //            }
 //        }
 //        memory[new + 0] = undefined;
 //        memory[new + 1] = undefined;
 //        memory[new + 2] = undefined;
 //        memory[new + 3] = undefined;
 //    }
 //    puts("\nerror\n"); return;
 //}
 
 
 
 
 
 
 
 
 
 
 
 
 
 if (at == new) {
     puts("at == head: qnext = at\n");
     memory[head + 3] = at;
 } else {
     puts("at ≠ head: qnext = tail\n");
     memory[head + 3] = tail;
 }
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 static nat _4r_solve() {
     for (nat head = 0, tail = 3; head < 3 * size; head += 3) {
         
         puts("queue state:"); debug_memory(head, tail);
         
         for (nat index = 0; index < strlen(context); index++) {
             
             puts("index state:"); debug_memory(head, tail);
                         
                 nat invalid = 0,
                     parent = head,
                     begin = memory[head + 2];
                 
                 for (; context[index] != '.'; index++) {
                     
                     if (!invalid) {
                         display_signature(index);
                         display_string_at_char(begin);
                         printf("[LOCAL]: index = %lu, parent = %lu, begin = %lu\n\n",
                                index, parent, begin);
                     }
                     
                     if (invalid) { puts("signature was deemed invalid."); continue; }
                     
                     else if (context[index] == '_') {
                         if (tail + 3 <= 3 * size) {
                             memory[tail++] = index;
                             memory[tail++] = parent;
                             memory[tail++] = begin;
                         } else puts("ARG PUSH FAILED.");
                         debug_memory(head, tail);
                         invalid = 1;
                         
                     } else if (context[index] == input[begin]) begin++;
                     else { puts("character mismatch in signature"); invalid = 1; }
                 }
             
                 puts("reached end of signature: "); debug_memory(head, tail);
             
                 if (invalid) { puts("failing signature, moving to next index."); continue; }
                         
 //                else if (parent) {
 //                    if (tail + 3 <= 3 * size) {
 //                        memory[tail++] = index;
 //                        memory[tail++] = parent;
 //                        memory[tail++] = begin;
 //                    } else puts("AT PUSH FAILED.");
 //                    puts("moving to next parent");
 //                    memory[parent + 2] = begin;
 //                }
             
                 else if (parent && begin == strlen(input)) {
                     if (tail + 3 <= 3 * size) {
                         memory[tail++] = index;
                         memory[tail++] = parent;
                         memory[tail++] = begin;
                     } else puts("AT PUSH FAILED.");
                     puts("-> found solution");
                     return 1;
                 }
             
                 else { puts("skip to next index: no parents, length didnt match"); continue; }
             } // index loop
             
     } // queue loop
     return 0;
 //            for (nat at = head; at; at = memory[at + 1]) {
 //
 //                puts("parent state:"); debug_memory(head, tail);
 //
 //                nat invalid = 0,
 //                    index = at == head ? _index : memory[at],
 //                    parent = at,
 //                    begin = memory[at + 2];
 //
 //                for (; context[index] != '.'; index++) {
 //
 //                    if (!invalid) {
 //                        display_signature(index);
 //                        display_string_at_char(begin);
 //                        printf("[LOCAL]: index = %lu, parent = %lu, begin = %lu\n\n",
 //                               index, parent, begin);
 //                    }
 //
 //                    if (invalid) { puts("signature was deemed invalid."); continue; }
 //
 //                    else if (context[index] == '_') {
 //                        if (tail + 3 <= 3 * size) {
 //                            memory[tail++] = index;
 //                            memory[tail++] = parent;
 //                            memory[tail++] = begin;
 //                        } else puts("ARG PUSH FAILED.");
 //                        debug_memory(head, tail);
 //                        invalid = 1;
 //
 //                    } else if (context[index] == input[begin]) begin++;
 //                    else { puts("character mismatch in signature"); invalid = 1; }
 //                }
 //
 //                puts("reached end of signature: "); debug_memory(head, tail);
 //
 //                if (invalid) { puts("failing signature, moving to next index."); break; }
 //                else if (parent) {
 //                    if (tail + 3 <= 3 * size) {
 //                        memory[tail++] = index;
 //                        memory[tail++] = parent;
 //                        memory[tail++] = begin;
 //                    } else puts("AT PUSH FAILED.");
 //                    puts("moving to next parent");
 //                    memory[parent + 2] = begin;
 //                }
 //                else if (begin == strlen(input)) {
 //                    if (tail + 3 <= 3 * size) {
 //                        memory[tail++] = index;
 //                        memory[tail++] = parent;
 //                        memory[tail++] = begin;
 //                    } else puts("AT PUSH FAILED.");
 //                    puts("-> found solution");
 //                    return at;
 //                }
 //                else { puts("skip: no parents, length didnt match"); break; }
 //            } // parent loop
         
     
 } // solve()


//                else if (parent) {
//                    if (tail + 3 <= 3 * size) {
//                        memory[tail++] = index;
//                        memory[tail++] = parent;
//                        memory[tail++] = begin;
//                    } else puts("AT PUSH FAILED.");
//                    puts("moving to next parent");
//                    memory[parent + 2] = begin;
//                }
//            for (nat at = head; at; at = memory[at + 1]) {
//
//                puts("parent state:"); debug_memory(head, tail);
//
//                nat invalid = 0,
//                    index = at == head ? _index : memory[at],
//                    parent = at,
//                    begin = memory[at + 2];
//
//                for (; context[index] != '.'; index++) {
//
//                    if (!invalid) {
//                        display_signature(index);
//                        display_string_at_char(begin);
//                        printf("[LOCAL]: index = %lu, parent = %lu, begin = %lu\n\n",
//                               index, parent, begin);
//                    }
//
//                    if (invalid) { puts("signature was deemed invalid."); continue; }
//
//                    else if (context[index] == '_') {
//                        if (tail + 3 <= 3 * size) {
//                            memory[tail++] = index;
//                            memory[tail++] = parent;
//                            memory[tail++] = begin;
//                        } else puts("ARG PUSH FAILED.");
//                        debug_memory(head, tail);
//                        invalid = 1;
//
//                    } else if (context[index] == input[begin]) begin++;
//                    else { puts("character mismatch in signature"); invalid = 1; }
//                }
//
//                puts("reached end of signature: "); debug_memory(head, tail);
//
//                if (invalid) { puts("failing signature, moving to next index."); break; }
//                else if (parent) {
//                    if (tail + 3 <= 3 * size) {
//                        memory[tail++] = index;
//                        memory[tail++] = parent;
//                        memory[tail++] = begin;
//                    } else puts("AT PUSH FAILED.");
//                    puts("moving to next parent");
//                    memory[parent + 2] = begin;
//                }
//                else if (begin == strlen(input)) {
//                    if (tail + 3 <= 3 * size) {
//                        memory[tail++] = index;
//                        memory[tail++] = parent;
//                        memory[tail++] = begin;
//                    } else puts("AT PUSH FAILED.");
//                    puts("-> found solution");
//                    return at;
//                }
//                else { puts("skip: no parents, length didnt match"); break; }
//            } // parent loop
    


























 
 
 
 
 
 
 
 
 
 
 static nat solve() {
     for (nat head = 0, tail = 3; head < 3 * size; head += 3) {
         for (nat index = 0; index < strlen(context); index++) {
             
             nat
                 invalid = 0,
                 begin = memory[head + 2];
             
             for (; context[index] != '.'; index++) {
                 
                 if (!invalid) {
                     display_signature(index);
                     display_string_at_char(begin);
                     printf("[LOCAL]: index = %lu, parent = %lu, begin = %lu\n\n", index, head, begin);
                 }
                 
                 if (invalid) { puts("signature was deemed invalid."); continue; }
                 
                 else if (context[index] == '_') {
                     if (tail + 3 <= 3 * size) {
                         memory[tail++] = index;
                         memory[tail++] = head;
                         memory[tail++] = begin;
                         puts("arg pushed.\n");
                     } else puts("ARG PUSH FAILED.");
                     debug_memory(head, tail);
                     invalid = 1;
                 } else if (context[index] == input[begin]) begin++;
                 else { puts("character mismatch in signature"); invalid = 1; }
             }
             
             puts("reached end of signature: ");
             printf("[LOCAL]: index = %lu, parent = %lu, begin = %lu\n\n", index, head, begin);
             debug_memory(head, tail);
             
             if (invalid) {
                 puts("failing signature, moving to next index.");
                 continue;
             }
                         
             else if (head) {
                 printf("must complete parents!!\n");
                 
         
                 for (nat at = head {HERE MUST BE THE INDEX OF THE NODE WE JUST ADDED: WHICH HAS PARENT=HEAD.}; at; at = memory[at + 1]) {
//
//                     puts("parent state:"); debug_memory(head, tail);
//
//                     nat
//                         invalid = 0,
//                         index = memory[at + 0],
//                         parent = memory[at + 1],
//                         begin = memory[at + 2];
                         /// FACT:
                         ///     if we simply push a new node,
                     ///         before going into the index loop,
                     ///                 (and then pop it after we finish the index loop),
                         /// then we can use the same mechanism   (THIS LOOP     AND THE ABOVE CODE:
                     ///                                             i = m+0, p = m+1, b = m+2,,,
                     ///                                                               ie, clean code! :)
                     ///
                     ///                      this code is good, because its referencing memory-
                     ///                      we need to reference memory too!! its just we need to push
                     ///                      the new node, before we get here, so that things work out,
                     ///                      with respect to head, and what not.
                     ///
                     ///                      im thinking the node values that we push are as follows:
                     ///
                     ///                            new unit:{ i=0, p=head, b=memory[head + 2](ie, begin of head). }
                     ///
                     ///                     that is the data for the node that we need to push before we get
                     ///                     into the at/parent loop.    thiss is literally crucial.
                     ///
                     ///       another fact is:
                     ///
                     ///             that we need to actually index onto the new node, in the index loop.
                     ///             that is crucial, ie,
                     ///
                     ///
                     ///                     [note: NEW_IDX is the index of the new node. we need to know that.]
                     ///
                     ///
                     ///                     for (;memory[NEW_IDX] < context_length; memory[NEW_IDX]++) {
                     ///
                     ///                         // this loop is the true index loop. im pretty sure.
                     ///
                     ///                     }
                     ///
                     ///
                     ///
                     ///
                     ///             and then the parent loop is inside this loop, for sure.
//
//
//
//                     for (; context[index] != '.'; index++) {
//
//                     }
//                 }
//
//
                         
                 
                 
                 
                 
                 
                 
                 
                 
                 
                 
                 
                 
                 
                 
                 
                 
                 
                 
                 
                 
//
//
//             }
//
//             else if (begin == strlen(input)) {
//                 if (tail + 3 <= 3 * size) {
//                     memory[tail++] = index;
//                     memory[tail++] = head;
//                     memory[tail++] = begin;
//                     puts("at pushed.\n");
//                 } else puts("AT PUSH FAILED.");
//                 puts("-> found solution");
//                 return 1;
//             }
//
//             else {
//                 printf("skip to next index: no parents, length didnt match: [begin = %lu ≠ end = %lu]", begin, strlen(input));
//                 continue;
//             }
//
//             puts("going onto next index now.");
//         } // index
//     } // queue
//     return 0;
// }
//
//
//
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 static nat solve() {
     for (nat head = 0, tail = 3; head < 3 * size; head += 3) {
         
         puts("queue state:"); debug_memory(head, tail);
         
         for (nat index = 0; index < strlen(context); index++) {
             
             puts("index state:"); debug_memory(head, tail);
                         
                 nat invalid = 0,
                     parent = head,
                     begin = memory[head + 2];
                 
                 for (; context[index] != '.'; index++) {
                     
                     if (!invalid) {
                         display_signature(index);
                         display_string_at_char(begin);
                         printf("[LOCAL]: index = %lu, parent = %lu, begin = %lu\n\n",
                                index, parent, begin);
                     }
                     
                     if (invalid) { puts("signature was deemed invalid."); continue; }
                     
                     else if (context[index] == '_') {
                         if (tail + 3 <= 3 * size) {
                             memory[tail++] = index;
                             memory[tail++] = parent;
                             memory[tail++] = begin;
                         } else puts("ARG PUSH FAILED.");
                         debug_memory(head, tail);
                         invalid = 1;
                         
                     } else if (context[index] == input[begin]) begin++;
                     else { puts("character mismatch in signature"); invalid = 1; }
                 }
             
                 puts("reached end of signature: "); debug_memory(head, tail);
             
                 if (invalid) { puts("failing signature, moving to next index."); continue; }
                         
 //                else if (parent) {
 //                    if (tail + 3 <= 3 * size) {
 //                        memory[tail++] = index;
 //                        memory[tail++] = parent;
 //                        memory[tail++] = begin;
 //                    } else puts("AT PUSH FAILED.");
 //                    puts("moving to next parent");
 //                    memory[parent + 2] = begin;
 //                }
             
                 else if (parent && begin == strlen(input)) {
                     if (tail + 3 <= 3 * size) {
                         memory[tail++] = index;
                         memory[tail++] = parent;
                         memory[tail++] = begin;
                     } else puts("AT PUSH FAILED.");
                     puts("-> found solution");
                     return 1;
                 }
             
                 else { puts("skip to next index: no parents, length didnt match"); continue; }
             } // index loop
             
     } // queue loop
     return 0;
 //            for (nat at = head; at; at = memory[at + 1]) {
 //
 //                puts("parent state:"); debug_memory(head, tail);
 //
 //                nat invalid = 0,
 //                    index = at == head ? _index : memory[at],
 //                    parent = at,
 //                    begin = memory[at + 2];
 //
 //                for (; context[index] != '.'; index++) {
 //
 //                    if (!invalid) {
 //                        display_signature(index);
 //                        display_string_at_char(begin);
 //                        printf("[LOCAL]: index = %lu, parent = %lu, begin = %lu\n\n",
 //                               index, parent, begin);
 //                    }
 //
 //                    if (invalid) { puts("signature was deemed invalid."); continue; }
 //
 //                    else if (context[index] == '_') {
 //                        if (tail + 3 <= 3 * size) {
 //                            memory[tail++] = index;
 //                            memory[tail++] = parent;
 //                            memory[tail++] = begin;
 //                        } else puts("ARG PUSH FAILED.");
 //                        debug_memory(head, tail);
 //                        invalid = 1;
 //
 //                    } else if (context[index] == input[begin]) begin++;
 //                    else { puts("character mismatch in signature"); invalid = 1; }
 //                }
 //
 //                puts("reached end of signature: "); debug_memory(head, tail);
 //
 //                if (invalid) { puts("failing signature, moving to next index."); break; }
 //                else if (parent) {
 //                    if (tail + 3 <= 3 * size) {
 //                        memory[tail++] = index;
 //                        memory[tail++] = parent;
 //                        memory[tail++] = begin;
 //                    } else puts("AT PUSH FAILED.");
 //                    puts("moving to next parent");
 //                    memory[parent + 2] = begin;
 //                }
 //                else if (begin == strlen(input)) {
 //                    if (tail + 3 <= 3 * size) {
 //                        memory[tail++] = index;
 //                        memory[tail++] = parent;
 //                        memory[tail++] = begin;
 //                    } else puts("AT PUSH FAILED.");
 //                    puts("-> found solution");
 //                    return at;
 //                }
 //                else { puts("skip: no parents, length didnt match"); break; }
 //            } // parent loop
         
     
 } // solve()
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 static nat solve() {
     for (nat head = 3, tail = 6; head < 3 * size; head += 3) {
         
         puts("queue state:"); debug_memory(head, tail);
         
         for (nat _index = 0; _index < strlen(context); _index++) {
             
             puts("index state:"); debug_memory(head, tail);
             
             for (nat at = head; at; at = memory[at + 1]) {
                 
                 puts("parent state:"); debug_memory(head, tail);
                 
                 nat invalid = 0,
                     index = at == head ? _index : memory[at],
                     parent = at,
                     begin = memory[at + 2];
                 
                 for (; context[index] != '.'; index++) {
                     
                     if (!invalid) {
                         display_signature(index);
                         display_string_at_char(begin);
                         printf("[LOCAL]: index = %lu, parent = %lu, begin = %lu\n\n",
                                index, parent, begin);
                     }
                     
                     if (invalid) { puts("signature was deemed invalid."); continue; }
                     
                     else if (context[index] == '_') {
                         if (tail + 3 <= 3 * size) {
                             memory[tail++] = index;
                             memory[tail++] = parent;
                             memory[tail++] = begin;
                         } else puts("ARG PUSH FAILED.");
                         debug_memory(head, tail);
                         invalid = 1;
                         
                     } else if (context[index] == input[begin]) begin++;
                     else { puts("character mismatch in signature"); invalid = 1; }
                 }
                         
                 puts("reached end of signature: "); debug_memory(head, tail);
                                 
                 if (invalid) { puts("failing signature, moving to next index."); break; }
                 else if (parent) {
                     if (tail + 3 <= 3 * size) {
                         memory[tail++] = index;
                         memory[tail++] = parent;
                         memory[tail++] = begin;
                     } else puts("AT PUSH FAILED.");
                     puts("moving to next parent");
                     memory[parent + 2] = begin;
                 }
                 else if (begin == strlen(input)) {
                     if (tail + 3 <= 3 * size) {
                         memory[tail++] = index;
                         memory[tail++] = parent;
                         memory[tail++] = begin;
                     } else puts("AT PUSH FAILED.");
                     puts("-> found solution");
                     return at;
                 }
                 else { puts("skip: no parents, length didnt match"); break; }
                 
             } // parent loop
             
             
         } // index loop
         
     } // queue loop
     
     return 0;
 }

 */

 
 
 
 
 
 
 
 
 
 
 
 
 
 

static nat compact_solve() { // definitely wrong, thus far.
    for (nat head = 3, tail = 6; head < 3 * size; head += 3) {
        for (nat _index = memory[head]; _index < strlen(context); _index++) {
            for (nat at = head; at; at = memory[at + 1]) {
                nat bad = 0,
                    index = at == head ? _index : memory[at + 0],
                    parent = memory[at + 1],
                    begin = memory[at + 2];
                for (; context[index] != '.'; index++) {
                    if (bad) continue;
                    else if (context[index] == '_') {
                        if (tail + 3 <= 3 * size) {
                            memory[tail++] = index;
                            memory[tail++] = parent;
                            memory[tail++] = begin;
                        }
                        bad = 1;
                    } else if (context[index] == input[begin]) begin++;
                    else bad = 1;
                }
                _index = index;
                if (bad) break;
                else if (parent) memory[parent + 2] = begin;
                else if (begin == strlen(input)) return at;
                else break;
            }
        }
    }
    return 0;
}
 
 
 

 static nat solve() {
     for (nat head = 3, tail = 6; head < 3 * size; head += 3) {
         
         puts("queue state:"); debug_memory(head, tail);
         
         for (nat _index = memory[head]; _index < strlen(context); _index++) {
             
             puts("index state:"); debug_memory(head, tail);
             
             for (nat at = head; at; at = memory[at + 1]) {
                 
                 puts("parent state:"); debug_memory(head, tail);
                 
                 nat bad = 0,
                     index = at == head ? _index : memory[at + 0],
                     parent = memory[at + 1],
                     begin = memory[at + 2];
                 
                 for (; context[index] != '.'; index++) {
                     
                     if (!bad) {
                         display_signature(index);
                         display_string_at_char(begin);
                     }
                     
                     if (bad) { puts("signature was deemed invalid."); continue; }
                     else if (context[index] == '_') {
                         if (tail + 3 <= 3 * size) {
                             memory[tail++] = index;
                             memory[tail++] = parent;
                             memory[tail++] = begin;
                         } else puts("ARG FAILED.");
                         
                         debug_memory(head, tail);
                         bad = 1;
                     } else if (context[index] == input[begin]) begin++;
                     else { puts("character mismatch in signature"); bad = 1; }
                 }
                 
                 puts("reached end of signature: "); debug_memory(head, tail);
                 
                 printf("global_index = %lu, local_index = %lu\n", _index, index);
                 _index = index;
         
                 if (bad) { puts("failing signature, moving to next index."); break; }
                 else if (parent) { puts("moving to next parent"); memory[parent + 2] = begin; }
                 else if (begin == strlen(input)) { puts("-> found solution"); return at; }
                 else { puts("skip: no parents, length didnt match"); break; }
             }
         }
     }
     return 0;
 }


    memory[tail++] = 0;
    memory[tail++] = head;
    memory[tail++] = memory[head + 2];

    memory[at + 0] = index;
    memory[at + 1] = parent;
    memory[at + 2] = begin;






 

 static nat debug_solve(string given) {
     for (nat head = 3, tail = 6; head != tail; head += 3) {
         puts("queue state:");
         debug_memory(head, tail);
         for (nat index = memory[head]; index < context_length; index++) {
             puts("index state:");
             debug_memory(head, tail);
             for (nat at = head; at; at = memory[at + 1]) {
                 puts("parent state:");
                 debug_memory(head, tail);
                 nat parent = memory[at + 1], begin = memory[at + 2];
                 bool good = true;
                 for (; context[index]; ) {
                     
                     if (good) {
                         display_signature(index);
                         display_string_at_char(given, begin);
                     }
                     
                     nat c = context[index++];
                     
                     if (!good) { puts("signature was deemed invalid."); continue; }
                     else if (c >= 256) {
                         puts("found argument! moving on.");
                         memory[at + 0] = index;
                         memory[at + 1] = parent;
                         memory[at + 2] = begin;
                         
                         memory[tail + 0] = 0;
                         memory[tail + 1] = at;
                         memory[tail + 2] = begin;
                         tail += 3;
                         
                         good = false;
                         debug_memory(head, tail);
                     }
                     else if (c == (nat) given[begin]) { puts("char match..."); begin++; }
                     else { puts("character mismatch in signature"); good = false; }
                 }
                 debug_memory(head, tail);
                 if (!good) { puts("failing signature, moving to next index."); break; }
                 else if (parent) { puts("moving to next parent"); memory[parent + 2] = begin; }
                 else if (begin == strlen(given)) { puts("-> found solution"); return at; }
                 else { puts("skip: no parents, length didnt match"); break; }
             }
         }
     }
     return 0;
 }
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 ------------------------------ working (slow) csr ------------------------------------------
 

 static inline size_t parse(uint8_t* given, size_t length, size_t size,
                            struct resolved* memory, struct context* context) {
     for (size_t next = 2, head = 1, tail = 1; head; head = memory[head].queue_next) {
         for (; memory[head].index < context->name_count; memory[head].index++) {
             for (size_t at = head; at; at = memory[at].parent) {
                 struct resolved me = memory[at];
                 struct name name = context->names[me.index];
                 while (me.done < name.length) {
                     size_t c = name.signature[me.done++];
                     if (c >= 256 && next + 2 < size) {
                         struct resolved child = {.begin = me.begin, .parent = next + 1};
                         me.args[me.count++] = next;  // this is definitely wrong.
                         memory[tail].queue_next = next; tail = next;
                         memory[next++] = child;
                         memory[next++] = me;     // this seems wrong.
                     }
                     if (c >= 256 || c != (size_t) given[me.begin]) goto skip;
                     me.begin++;
                     if (me.begin > context->best) context->best = me.begin;
                 }
                 size_t p = me.parent;
                 if (!p) { if (me.begin == length) return at; else break; }
                 memory[p].begin = me.begin;
                 memory[p].args[memory[p].count - 1] = next;
                 if (next + 1 < size) memory[next++] = me;
             } skip: continue;
         }
         memory[head].index = 0;
     }
     return 0;
 }


 


static inline size_t parse(string given, nat length, nat size,
                           struct unit* memory, struct context* context) {
    size_t tail = 2;
     for (nat head = 1; head < size; head++) {
         for (; memory[head].index < context->name_count; memory[head].index++) {
             for (size_t at = head; at; at = memory[at].parent) {
                 struct resolved me = memory[at];
                 struct name name = context->names[me.index];
                 for (; me.done < name.length; ) {
                     size_t c = name.signature[me.done++];
                     if (c >= 256 && next + 2 < size) {
                         struct resolved child = {.begin = me.begin, .parent = next + 1};
                         me.args[me.count++] = next;
                         memory[tail].queue_next = next; tail = next;
                         memory[next++] = child; memory[next++] = me;
                     }
                     if (c >= 256 || c != (size_t) given[me.begin]) goto skip;
                     me.begin++; context->best = me.begin > context->best ? me.begin : context->best;
                 }
                 size_t p = me.parent;
                 if (!p) {
                     if (me.begin == length) return at; else break;
                 }
                 memory[p].begin = me.begin;
                 memory[p].args[memory[p].count - 1] = next;
                 if (next + 1 < size) memory[next++] = me;
             } skip: continue;
         }
     }
     return 0;
 }


static nat csr(string given) {
    nat tail = 2;
    for (nat head = 1; head != tail; head++) {
        for (nat at = head; at; at = memory[at].parent) {
            struct unit me = memory[at];
            while (me.index < context_length) {
                nat c = context[me.index++];
                if (c < 256) {
                    if (c != (nat) given[me.string]) goto skip;
                    else me.string++;
                } else {
                    struct unit child = { .string = me.string, .parent = at };
                    memory[tail++] = child;
                    goto skip;
                }
            }
            if (me.parent) memory[me.parent].string = me.string;
            else if (me.string == strlen(given)) return at;
            skip: continue;
        }
    }
    return 0;
}


 
        int printed = 0;
        if (given.string < context.length) {
            for (nat s = 0; s < context.names[given.index].length; s++) {
                nat c = context.names[given.index].signature[s];
                if (c >= 256) printf("_"); else printf("%c", (char) c);
                printed++;
            }
        } else {
            printf("////");
            printed += 4;
        }
        for (int j = 0; j < abs(max_name_length - printed); j++) printf(" ");





 
 
 
 

 static nat s(string given) {
     nat tail = 2;
     printf("in csr.\n");
     for (nat head = 1; head != tail; head++) {
         printf("head = %lu\n", head);
         for (nat at = head; at; at = memory[at].where) {
             printf("at = %lu\n", at);
             nat begin = memory[at].begin;
             nat where = memory[at].where;
             for (nat index = 0; index < context_length; index++) {
                 printf("me.index = %lu\n", index);
                 while (context[index]) {
                     
                     display_signature(context, context_length, index);
                     display_string_at_char(given, begin);
                     
                     nat c = context[index++];
                     
                     if (c < 256) {
                         if (c != (nat) given[begin]) {
                             printf("mismatch, going to next index.\n");
                             goto next_index;
                         } else {
                             begin++;
                         }
                     } else {
                         printf("found argument! going to next node.\n");
                         struct unit child = {
                             .index = index,
                             .begin = begin,
                             .where = at,
                         };
                         memory[tail++] = child;
                         goto next_node;
                     }
                 }
                 if (where) {
                     memory[where].begin = begin;
                     printf("going to next parent!\n");
                     goto next_parent;
                 } else if (begin == strlen(given)) {
                     printf("returning solution = %lu\n", at);
                     return at;
                 }
                 next_index:
                 while (context[index]) index++;
                 continue;
             } next_parent: continue;
         } next_node: continue;
     } return 0;
 }
 
 
 
 
 
 
 
 
 
 
 
 
 
 context:
 ---------------
 
 
 
 
 
 
 
 
                        h e l l o  \0  b u b b l e s  \0
 
                                    ^
 
 
 
 
 
 
 
 
 
 
static nat s(string given) {
    for (nat head = 3, tail = 6; head != tail; head++) {
        for (nat index = memory[head]; index < length; index++) {
            for (nat at = head; at; at = memory[at + 1]) {
                nat where = memory[at + 1], begin = memory[at + 2];
                bool good = true;
                for (; context[index]; ) {
                    nat c = context[index++];
                    if (!good) {}
                    else if (c >= 256) {}
                    else if (c == (nat) given[begin]) {}
                    else {}
                }
            }
        }
    }
    return 0;
}
 
 
 
 
 
 
 
 

 static nat debug_solve(string given) {
     for (nat head = 3, tail = 6; head != tail; head += 3) {
         puts("queue state:");
         debug_memory(head, tail);
         for (nat index = memory[head]; index < context_length; index++) {
             puts("index state:");
             debug_memory(head, tail);
             for (nat at = head; at; at = memory[at + 1]) {
                 puts("parent state:");
                 debug_memory(head, tail);
                 nat parent = memory[at + 1], begin = memory[at + 2];
                 bool good = true;
                 for (; context[index]; ) {
                     
                     if (good) {
                         display_signature(index);
                         display_string_at_char(given, begin);
                     }
                     
                     nat c = context[index++];
                     
                     if (!good) { puts("signature was deemed invalid."); continue; }
                     else if (c >= 256) {
                         puts("found argument! moving on.");
                         memory[at + 0] = index;
                         memory[at + 1] = parent;
                         memory[at + 2] = begin;
                         
                         memory[tail + 0] = 0;
                         memory[tail + 1] = at;
                         memory[tail + 2] = begin;
                         tail += 3;
                         
                         good = false;
                         debug_memory(head, tail);
                     }
                     else if (c == (nat) given[begin]) { puts("char match..."); begin++; }
                     else { puts("character mismatch in signature"); good = false; }
                 }
                 debug_memory(head, tail);
                 if (!good) { puts("failing signature, moving to next index."); break; }
                 else if (parent) { puts("moving to next parent"); memory[parent + 2] = begin; }
                 else if (begin == strlen(given)) { puts("-> found solution"); return at; }
                 else { puts("skip: no parents, length didnt match"); break; }
             } // parent loop
 //            memory[head + 1] = 0;
         } // index loop
 //        memory[head] = 0;
     } // queue loop
     return 0;
 }
 
 static nat s(string given) {
     for (nat head = 3, tail = 6; head != tail; head += 3) {
         for (nat index = memory[head]; index < context_length; index++) {
             for (nat at = head; at; at = memory[at + 1]) {
                 nat parent = memory[at + 1], begin = memory[at + 2];
                 bool good = true;
                 for (; context[index]; ) {
                     nat c = context[index++];
                     if (!good) continue;
                     else if (c >= 256) {
                         memory[at + 0] = index;
                         memory[at + 1] = parent;
                         memory[at + 2] = begin;
                         memory[tail + 0] = 0;
                         memory[tail + 1] = at;
                         memory[tail + 2] = begin;
                         tail += 3;
                         good = false;
                     }
                     else if (c == (nat) given[begin]) begin++;
                     else good = false;
                 }
                 if (!good) break;
                 else if (parent) memory[parent + 2] = begin;
                 else if (begin == strlen(given)) return at;
                 else break;
             }
         }
     }
     return 0;
 }

 




static inline size_t parse(string given, nat length, nat size,
                           struct unit* memory, struct context* context) {
    size_t tail = 2;
     for (nat head = 1; head < size; head++) {
         for (; memory[head].index < context->name_count; memory[head].index++) {
             for (size_t at = head; at; at = memory[at].parent) {
                 struct resolved me = memory[at];
                 struct name name = context->names[me.index];
                 for (; me.done < name.length; ) {
                     size_t c = name.signature[me.done++];
                     if (c >= 256 && next + 2 < size) {
                         struct resolved child = {.begin = me.begin, .parent = next + 1};
                         me.args[me.count++] = next;
                         memory[tail].queue_next = next; tail = next;
                         memory[next++] = child; memory[next++] = me;
                     }
                     if (c >= 256 || c != (size_t) given[me.begin]) goto skip;
                     me.begin++; context->best = me.begin > context->best ? me.begin : context->best;
                 }
                 size_t p = me.parent;
                 if (!p) {
                     if (me.begin == length) return at; else break;
                 }
                 memory[p].begin = me.begin;
                 memory[p].args[memory[p].count - 1] = next;
                 if (next + 1 < size) memory[next++] = me;
             } skip: continue;
         }
     }
     return 0;
 }


static nat csr(string given) {
    nat tail = 2;
    for (nat head = 1; head != tail; head++) {
        for (nat at = head; at; at = memory[at].parent) {
            struct unit me = memory[at];
            while (me.index < context_length) {
                nat c = context[me.index++];
                if (c < 256) {
                    if (c != (nat) given[me.string]) goto skip;
                    else me.string++;
                } else {
                    struct unit child = { .string = me.string, .parent = at };
                    memory[tail++] = child;
                    goto skip;
                }
            }
            if (me.parent) memory[me.parent].string = me.string;
            else if (me.string == strlen(given)) return at;
            skip: continue;
        }
    }
    return 0;
}




        int printed = 0;
        if (given.string < context.length) {
            for (nat s = 0; s < context.names[given.index].length; s++) {
                nat c = context.names[given.index].signature[s];
                if (c >= 256) printf("_"); else printf("%c", (char) c);
                printed++;
            }
        } else {
            printf("////");
            printed += 4;
        }
        for (int j = 0; j < abs(max_name_length - printed); j++) printf(" ");


 
 
 
 
 

 static nat s(string given) {
     nat tail = 2;
     printf("in csr.\n");
     for (nat head = 1; head != tail; head++) {
         printf("head = %lu\n", head);
         for (nat at = head; at; at = memory[at].where) {
             printf("at = %lu\n", at);
             nat begin = memory[at].begin;
             nat where = memory[at].where;
             for (nat index = 0; index < context_length; index++) {
                 printf("me.index = %lu\n", index);
                 while (context[index]) {
                     
                     display_signature(context, context_length, index);
                     display_string_at_char(given, begin);
                     
                     nat c = context[index++];
                     
                     if (c < 256) {
                         if (c != (nat) given[begin]) {
                             printf("mismatch, going to next index.\n");
                             goto next_index;
                         } else {
                             begin++;
                         }
                     } else {
                         printf("found argument! going to next node.\n");
                         struct unit child = {
                             .index = index,
                             .begin = begin,
                             .where = at,
                         };
                         memory[tail++] = child;
                         goto next_node;
                     }
                 }
                 if (where) {
                     memory[where].begin = begin;
                     printf("going to next parent!\n");
                     goto next_parent;
                 } else if (begin == strlen(given)) {
                     printf("returning solution = %lu\n", at);
                     return at;
                 }
                 next_index:
                 while (context[index]) index++;
                 continue;
             } next_parent: continue;
         } next_node: continue;
     } return 0;
 }
 
 

 
 
 
 context:
 ---------------
 
 

                        h e l l o  \0  b u b b l e s  \0
 
                                    ^


static nat other_s(string given) {
    nat tail = 2;
    for (nat head = 1; head != tail; head++) {
        for (nat at = head; at; at = memory[at].where) {
            nat begin = memory[at].begin, where = memory[at].where, index = memory[at].index;
            bool good = true;
            for (; index < context_length; index++) {
                nat c = context[index];
                if (!c && good) break;
                else if (!c && !good) good = true;
                else if (!good) continue;
                else if (c >= 256) {  } // arg
                else if (c == (nat) given[begin]) begin++;
                else good = false;
            }
            if (where) { memory[where].begin = begin; continue; }
            else if (begin == strlen(given)) return at;
        }
    }
    return 0;
}

 

 struct unit child = { .index = index, .begin = begin, .where = at };
 memory[tail++] = child;
 
 
 
 

static struct unit* memory = NULL;
// should be a nat* eventually, and use i + 0, i + 1, i + 2
// to access the various members.

























//
//
//    for later:   context best:
//
//        if (me.string > best) best = me.string;
//
//
//
//    clean up:
//
//        memory[head] = (struct unit){0};
//
 
 
 
 
 
 
 
 
 
 
 
 
             nat c = memory[head].index;
             printf("%5lu   : %5lu : %c \n", c, context[c], (char) context[c]);
 
 
 
 
 
 typedef uint32_t nat; // for later.
 typedef size_t nat;    // for now.
 typedef const char* string;

 static const nat memory_size = 1024;

 static struct unit* memory = NULL;
 static nat* context = NULL;
 static nat context_length = 0;

 struct unit {
     nat string;
     nat context;
     nat parent;
 };
 

 
 current partial draft of the new type of csr that is supposed to be more memory efficient.
 
 
 static nat csr(string given) {
     nat tail = 2;
     for (nat head = 1; head != tail; head++) {
         for (nat at = head; at; at = memory[at].parent) {
             struct unit me = memory[at];
             while (me.index < context_length) {
                 nat c = context[me.index++];
                 if (c < 256) {
                     if (c != (nat) given[me.string]) goto skip;
                     else me.string++;
                 } else {
                     struct unit child = { .string = me.string, .parent = at };
                     // memory.push(child);
                     goto skip;
                 }
             }
             if (me.parent) memory[me.parent].string = me.string;
             else if (me.string == strlen(given)) return at;
             skip: continue;
         }
     }
     return 0;
 }
 

 printf("r = %lu\n", solution);
     
 if (!count) fprintf(stderr, "n: %s: error: unresolved empty file\n\n", argv[a]);
 else if (!solution) {
     if (context.best == count) context.best--;
     fprintf(stderr,
             "n: %s:%u:%u: error: unresolved %c\n\n", argv[a],
             loc[2 * context.best], loc[2 * context.best + 1], tokens[context.best]);
 }
 
 
 
 
 
 
 static void clone(struct resolved me,
                   nat n, nat max,
                   nat* head, nat* next, nat* tail,
                   res_array memory) {

     printf("cloning parent stack...\n");

     if (*next + 2 < max) {
         struct resolved child = {
             .index = 0,
             .begin = me.begin,
             .done = 0,
             .queue_next = 0,
             .parent = *next + 1,
             .count = 0,
             .args = {0},
         };
         memory[*tail].queue_next = *next; *tail = *next;
         me.args[me.count++] = *next;
         memory[(*next)++] = child;
         memory[(*next)++] = me;
     
     } else { printf("error, ran out of memory.\n"); }
     
     debug_memory(memory, *head, *tail, *next, max);
 }

 static bool matches(nat n, string given, res_array memory,
                       nat max, nat* head, nat* next, nat* tail) {
     for (; n; n = memory[n].parent) {
         
         printf("trying me/parent = %lu\n", n);
         
         struct resolved me = memory[n];
         struct name name = context.names[me.index];
         
         printf("recognizing...\n");
         for (; me.done < name.length; ) {
             
             printf("looking at: \n");
             display_signature(name.signature, me.done, name.length);
             display_string_at_char(given, me.begin);
                         
             nat c = name.signature[me.done++];
             
             if (c >= 256) {
                 printf("[found argument, failing]\n");
                 clone(me, n, max, head, next, tail, memory);
                 return 0;
             } else if (c != (nat) given[me.begin]) {
                 printf("[character mismatch, failing]\n");
                 return 0;
             }
             me.begin++;
             best(me.begin);
         }
         printf("[signature succeeded]\n");
         
         if (!me.parent) {
             printf("at last parent, returning %s!\n\n", me.begin == strlen(given) ? "SUCCESS" : "FAILURE");
             return me.begin == strlen(given);
         } else {
             nat p = me.parent;
             memory[p].begin = me.begin;
             memory[p].args[memory[p].count - 1] = *next;
             
             if (*next + 1 < max) {
                     memory[(*next)++] = me;
             } else { printf("error, ran out of memory.\n"); }
             
             printf("continuing to next parent...\n");
         }
     }
     printf("[failing signature][out]\n");
     return 0;
 }

 static nat csr(string given, res_array memory, nat max) {
     nat next = 2, head = 1, tail = 1;
     for (; head; head = memory[head].queue_next) {
         printf("------------ moving on to head = %lu --------------\n", head);
         debug_memory(memory, head, tail, next, max);
         printf("trying indexes for head = %lu...\n", head);
         for (; memory[head].index < context.name_count; memory[head].index++) {
             printf("\n\ntrying index:  %lu \n", memory[head].index);
             if (matches(head, given, memory, max, &head, &next, &tail)) {
                 printf("found a match!\n");
                 return head;
             } else {
                 printf("didnt match, or found argument, trying next index...\n");
             }
         }
         printf("no index solved it. moving onto next head...\n");
     }
     printf("[NO MATCH FOUND]\n");
     if (next + 1 == max) printf("----> ERROR, ran out of memory!\n");
     return 0;
 }







static nat compacted_csr(string given, res_array memory, nat max) {
    for (nat next = 2, head = 1, tail = 1; head; head = memory[head].queue_next) {
        for (; memory[head].index < context.name_count; memory[head].index++) {
            for (nat at = head; at; at = memory[at].parent) {
                struct resolved me = memory[at];
                struct name name = context.names[me.index];
                for (; me.done < name.length; ) {
                    nat c = name.signature[me.done++];
                    if (c >= 256 && next + 2 < max) {
                        struct resolved child = {.begin = me.begin, .parent = next + 1};
                        me.args[me.count++] = next;
                        memory[tail].queue_next = next; tail = next;
                        memory[next++] = child; memory[next++] = me;
                    }
                    if (c >= 256 || c != (nat) given[me.begin]) goto skip;
                    me.begin++; context.best = me.begin > context.best ? me.begin : context.best;
                }
                nat p = me.parent;
                if (!p) {
                    if (me.begin == strlen(given)) return at; else break;
                }
                memory[p].begin = me.begin;
                memory[p].args[memory[p].count - 1] = next;
                if (next + 1 < max) memory[next++] = me;
            } skip: continue;
        }
    }
    return 0;
}




static inline size_t parse(uint8_t* given, size_t length, size_t size,
                           struct resolved* memory, struct context* context) {
     for (size_t next = 2, head = 1, tail = 1; head; head = memory[head].queue_next) {
         for (; memory[head].index < context->name_count; memory[head].index++) {
             for (size_t at = head; at; at = memory[at].parent) {
                 struct resolved me = memory[at];
                 struct name name = context->names[me.index];
                 for (; me.done < name.length; ) {
                     size_t c = name.signature[me.done++];
                     if (c >= 256 && next + 2 < size) {
                         struct resolved child = {.begin = me.begin, .parent = next + 1};
                         me.args[me.count++] = next;
                         memory[tail].queue_next = next; tail = next;
                         memory[next++] = child; memory[next++] = me;
                     }
                     if (c >= 256 || c != (size_t) given[me.begin]) goto skip;
                     me.begin++; context->best = me.begin > context->best ? me.begin : context->best;
                 }
                 size_t p = me.parent;
                 if (!p) {
                     if (me.begin == length) return at; else break;
                 }
                 memory[p].begin = me.begin;
                 memory[p].args[memory[p].count - 1] = next;
                 if (next + 1 < size) memory[next++] = me;
             } skip: continue;
         }
     }
     return 0;
 }

static void debug_resolved(struct resolved* memory, nat r, nat depth) {
    const struct resolved given = memory[r];
    for (size_t i = depth; i--;) printf(".   ");
    int printed = 0;
    if (r) {
        if (given.index >= context.name_count) printf("ERROR IN SIG INDEX");
        else for (nat s = 0; s < context.names[given.index].length; s++) {
            nat c = context.names[given.index].signature[s];
            if (c >= 256) printf("_"); else printf("%c", (char) c);
            printed++;
        }
    } else {
            printf("?");
            printed++;
        }
    for (int j = 0; j < abs(30 - printed); j++) printf(" ");
    print_resolved_data(given);
    for (size_t i = 0; i < given.count; i++) {
        for (size_t i = depth + 1; i--;) printf(".   ");
        printf("#%lu: \n", i);
        debug_resolved(memory, given.args[i], depth + 2);
    }
}

static void debug_context() {
    printf("\n----- context names: ------ \n{\n");
    for (size_t i = 0; i < context.name_count; i++) {
        printf("\t%6lu  :    ", i);
        struct name name = context.names[i];
        for (nat s = 0; s < name.length; s++)
            printf("%c",
                   name.signature[s] < 256
                    ? (char) name.signature[s]
                    : '_');
        puts("\n");
    }
    puts("}\n");
}






#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

typedef size_t nat;

struct resolved {
    size_t index;
    size_t begin;
    size_t done;
    size_t count;
    struct resolved* args;
    struct resolved* parent;
};

struct name {
    size_t* signature;
    size_t length;
};

struct context {
    size_t best;
    size_t name_count;
    struct name* names;
};

static struct context context = {0};
static const struct resolved failure = {0};

#define best() context.best = node.begin > context.best ? node.begin : context.best


static void prep(size_t d) {
    for (size_t i = d; i--;) printf(".   ");
}


static inline void represent
(size_t given, char* buffer, size_t limit,
 size_t* at) {
    if (given < 256) {
        buffer[(*at)++] = given;
        return;
    } else given -= 256;
    if (given >= context.name_count) return;
    struct name s = context.names[given];
    for (size_t i = 0; i < s.length; i++) {
        const size_t c = s.signature[i];
        if (c < 256) {
            buffer[(*at)++] = c;
        } else {
            buffer[(*at)++] = ' ';
            buffer[(*at)++] = '(';
            represent(c, buffer, limit, at);
            buffer[(*at)++] = ')';
            buffer[(*at)++] = ' ';
        }
    }
}

static void debug_resolved(struct resolved given, size_t depth) {
    char buffer[4096] = {0}; size_t index = 0;
    represent(given.index, buffer, sizeof buffer, &index);
    for (size_t i = depth; i--;) printf(".   ");
    if (given.index < 256) printf("(%s) :: %c:i=%lu:b=%lu:sd=%lu:c=%lu\n", buffer, (char) given.index, given.index, given.begin, given.done, given.count);
    else printf("(%s) :: i=%lu:b=%lu:sd=%lu:c=%lu\n", buffer, given.index, given.begin, given.done, given.count);
    for (size_t i = 0; i < given.count; i++) {
        for (size_t i = depth + 1; i--;) printf(".   ");
        printf("#%lu: \n", i);
        debug_resolved(given.args[i], depth + 1);
    }
}

static void debug_queue(struct resolved* queue, nat queue_count) {
    printf("----------------------------- debugging queue (queue count = %lu) ------------------------\n\n", queue_count);
    for (nat i = 0; i < queue_count; i++) {
        printf("\t----------------- queue node #%lu --------------- \n", i);
        struct resolved n = queue[i];
        debug_resolved(n, 2);
        printf("\t-------------------------------------------------\n\n");
    }
    printf("----------------------------------------------------------------------------------------\n");
}


static void debug_context() {
    printf("\n----- names: ------ \n{\n");
    for (size_t i = 0; i < context.name_count; i++) {
        char buffer[4096] = {0}; size_t index = 0;
        represent(i + 256, buffer, sizeof buffer, &index);
        printf("\t%6lu: %s\n\n", i, buffer);
    } puts("}\n");
}





static void construct_context() {
    context.best = 0;
    context.name_count = 5;
    context.names = calloc(context.name_count, sizeof(struct name));

    // _      param type.
    context.names[0].length = 1;
    context.names[0].signature = calloc(1, sizeof(size_t));
    context.names[0].signature[0] = '_';

    // hello
    context.names[1].length = 5;
    context.names[1].signature = calloc(5, sizeof(size_t));
    context.names[1].signature[0] = 'h';
    context.names[1].signature[1] = 'e';
    context.names[1].signature[2] = 'l';
    context.names[1].signature[3] = 'l';
    context.names[1].signature[4] = 'o';

    // (x) (y)
    context.names[2].length = 2;
    context.names[2].signature = calloc(2, sizeof(size_t));
    context.names[2].signature[0] = 256;
    context.names[2].signature[1] = 256;

    // (x) empty
    context.names[3].length = 6;
    context.names[3].signature = calloc(6, sizeof(size_t));
    context.names[3].signature[0] = 256;
    context.names[3].signature[1] = 'e';
    context.names[3].signature[2] = 'm';
    context.names[3].signature[3] = 'p';
    context.names[3].signature[4] = 't';
    context.names[3].signature[5] = 'y';

    // (x)
    context.names[4].length = 1;
    context.names[4].signature = calloc(1, sizeof(size_t));
    context.names[4].signature[0] = 256;

    debug_context();
}




static struct resolved queue_pop(struct resolved** v, size_t* count) {
    if (!*count) abort();
    struct resolved f = **v;
    --*count;
    memmove(*v, *v + 1, sizeof(struct resolved) * *count);
    *v = realloc(*v, sizeof(struct resolved) * *count);
    return f;
}

static void queue_push(struct resolved f, struct resolved** v, size_t* count) {
    *v = realloc(*v, sizeof(struct resolved) * (*count + 1));
    (*v)[(*count)++] = f;
}


static struct resolved old_csr(const uint8_t* given, nat end, nat max_depth) {

    nat queue_count = 0; struct resolved* queue = NULL;
    queue_push((struct resolved){256 + 1, 0, 0, 0, 0, 0}, &queue, &queue_count);

    while (queue_count) {
        struct resolved node = queue_pop(&queue, &queue_count);
        struct name name = context.names[node.index - 256];

        while (node.done < name.length) {
            nat c = name.signature[node.done];
            if (c < 256) {
                if (c == given[node.begin]) {
                    node.begin++; node.done++; best();
                } else goto next;
            } else {
                for (nat i = 0; i < context.name_count; i++) {

                    // push i to node's parameter list.

                    // push that tree to the queue.

                    // pop i from node's parameter list.         (undo)

                }
                goto next;
            }
        }

        if (node.parent) {
            node.parent->begin = node.begin;
            node.parent->done++;
            // ????

        } else if (node.begin == end) return node;

        next: continue;
    }
    return failure;
}


static void test_csr_with_string() {
    const char* input = "hello";
    struct resolved solution = old_csr((const uint8_t*) input, strlen(input), 2);
    debug_resolved(solution, 0);
    if (!solution.index) {
        printf("error: @ %lu: unexpected %c%s\n",
               context.best, input[context.best],
               input[context.best] ? "" : "end of expression");
    } else printf("\n\t--> parse successful.\n\n");
}

static void start() {
    construct_context();
    test_csr_with_string();
}



bool compact_matches(nat n, string given, res_array memory,
                      nat max, nat* next, nat* tail) {
    for (; n; n = memory[n].parent) {
        struct resolved me = memory[n];
        struct name name = context.names[me.index];
        for (; me.done < name.length;) {
            nat c = name.signature[me.done++];
            if (c >= 256) {
                struct resolved child = {0};
                child.begin = me.begin;
                child.parent = *next + 1;
                memory[*tail].queue_next = *next;
                *tail = *next;
                me.args[me.count++] = *next;
                if (*next < max) memory[(*next)++] = child;
                if (*next < max) memory[(*next)++] = me;
                return false;

            } else if (c != (nat) given[me.begin]) return false;
            me.begin++; best(me.begin);
        }
        if (!memory[n].parent) return me.begin == strlen(given);
    }
    return false;
}

 nat compact_csr(string given, res_array memory, nat max) {
     for (nat next = 1, head = 1, tail = 1; head; head = memory[head].queue_next)
         for (; memory[head].index < context.name_count; memory[head].index++)
             if (compact_matches(head, given, memory, max, &next, &tail))
                 return head;
     return 0;
 }




    nat new = 2, tail = 1;
        nat saved = memory[head].begin;
        for (; memory[head].index < context.name_count; memory[head].index++) {
            
                    if (memory[head].begin == strlen(given)) return head;
            
            }
    next_name: continue;
        }
next_node: continue;








static nat pruned_csr(const char* given, struct resolved* memory, nat memory_size) {
    nat new = 1;
    for (nat head = 1; head < memory_size; head++) {
        for (; memory[head].index < context.name_count; memory[head].index++) {
            for (nat me = head; me; me = memory[me].parent) {
                struct name name = context.names[memory[me].index];
                for (; memory[me].done < name.length; memory[me].done++) {
                    nat c = name.signature[memory[me].done];
                    if (c < 256) {
                        if (c == (nat) given[memory[me].begin]) {
                            memory[me].begin++; best(memory[me].begin);
                        } else {
                            goto next_name;
                        }
                    } else {
                        if (new < memory_size) {
                            memory[new++] = (struct resolved){0, memory[me].begin, 0, me, 0, {0}};
                        }
                        goto next_name;
                    }
                }
                nat p = memory[me].parent;
                if (p) {
                    memory[p].begin = memory[me].begin;
                    memory[p].done++;
                    memory[p].args[memory[p].count++] = me;
                } else {
                    if (memory[me].begin == strlen(given)) return me;
                }
            } next_name: continue;
        }
    } return 0;
}






 static nat csr(const char* given, struct resolved* memory, nat memory_size) {
     nat new = 2, tail = 1;
     for (nat head = 1; head; head = memory[head].queue_next) {
         debug_memory(memory, head, tail, new);
         nat saved = memory[head].begin;
         for (; memory[head].index < context.name_count; memory[head].index++) {
             memory[head].begin = saved;
             memory[head].done = 0;
             printf("for index: trying: %lu", memory[head].index);
             for (nat me = head; me; me = memory[me].parent) {
                 printf("for me: now recognizing: %lu", me);
                 struct name name = context.names[memory[me].index];
                 for (; memory[me].done < name.length; memory[me].done++) {
                     display_string_at_char(given, memory[me].begin);
                     display_signature(name.signature, memory[me].done, name.length);
                     nat c = name.signature[memory[me].done];
                     if (c < 256) {
                         if (c == (nat) given[memory[me].begin]) {
                             memory[me].begin++; best(memory[me].begin);
                         } else {
                             printf("---> character mismatch! n");
                             goto next_name;
                         }
                     } else {
                         if (new < memory_size) {
                             printf("found parameter: pushing new queue node! n");
                             memory[tail].queue_next = new;
                             tail = new;
                             memory[new++] = (struct resolved){0, memory[me].begin, 0, me, 0, 0, {0}};
                             printf("just pushed node:  n");
                             print_resolved_data(memory + new - 1);
                         }
                         printf("moving on to next node... n");
                         goto next_node;
                     }
                 }
                 nat p = memory[me].parent;
                 if (p) {
                     printf("advancing parent based on child... n");
                     memory[p].begin = memory[me].begin;
                     memory[p].done++;
                     printf("appending argument :: {%lu} n", me);
                     memory[p].args[memory[p].count++] = me;
                 } else {
                     if (memory[me].begin == strlen(given)) return me;
                 }
             } next_name: printf("<NEXT NAME> n"); continue;
         } next_node: printf("<NEXT NODE> n"); continue;
     } return 0;
 }






//    for head {
//
//        for index {
//
//            for parent {
//
//                for done {
//
//                }
//            }
//        }
//    }
//
 
 

 

 while (qhead) {                        // queue loop

     while (qnode) {                             // parent loop

         while (index < name_count) {        // context loop

             while (done < name.length) {    // signature loop

                 if (c < 256) {...}
                 else {...}

                 done++;
             }
             index++;
         }
         qnode = qnode.parent;
     }
     head = head.queue_next;
 }



nat csr(const char* given, struct resolved* memory) {
    nat length = strlen(given);
    pointer new = 2, head = 1, tail = 1;
    
    while (head) {
        debug_memory(memory, head, tail);
        
        pointer try = head, at = try;
        
        while (at) {
            
            printf("while(at): now recognizing: %lu\n", at);
            
            struct resolved me = memory[at];
            struct name name = context.names[memory[at].index - 256];
            
            while (memory[at].done < name.length) {
                
                display_string_at_char(given, memory[at].at);
                display_signature(name.signature, memory[at].done, name.length);
                
                nat c = name.signature[memory[at].done++];
                
                if (c < 256) {
                    if (c != (nat) given[memory[at].at]) {
                        printf("character mismatch!    "
                               " given[.begin]('%c') != sig[.done]('%c')   "
                               "  moving on.\n", given[memory[at].at], (char) c);
                        
                        goto next;
                    } else {
                        memory[at].at++;
                        best(memory[at].at);
                    }
                } else {
                    printf("found parameter: pushing signatures...\n");
                    
                    for (nat i = 0; i < context.name_count; i++) {
                        
                        if (new < memory_size) {
                            memory[tail].queue_next = new;
                            tail = new;
                            memory[new] = (struct resolved){i + 256, memory[at].at, 0, new + 1, 0, 0, {0}};
                            new++;
                        }
                        
                        nat n = at;
                        if (new < memory_size) {
                            memory[new] = memory[n];
                            memory[new].args[memory[new].count++] = new - 1;
                            memory[new].parent = new + 1;
                            new++;
                            n = memory[n].parent;
                        }
                        
                        /// theproblem is that we need to redo the arguments, for  each parent, now. not just the .parent's, but also every single last argument, i think. thats crucial. lets do it.
                        
                    
                        while (n && new < memory_size) {
                            memory[new] = memory[n];
                            memory[new].args[memory[new].count - 1] = new - 1;
                            memory[new].parent = new + 1;
                            new++;
                            n = memory[n].parent;
                        }
                        
                        memory[new - 1].parent = 0;
                    }
                    printf("pushing complete. moving on.\n");
                    goto next;
                }
            }
            
            printf("while(at): sig finished: \n");
            display_string_at_char(given, memory[at].at);
            display_signature(name.signature, memory[at].done, name.length);
            
            pointer parent = memory[at].parent;
            
            if (parent) {
                printf("while(at): parent is nonnull! pushing argument...\n");
                memory[parent].args[memory[parent].count++] = at;
                memory[parent].at = memory[at].at;
                
            } else {
                if (memory[at].at == length) {
                    printf("recognitition success: returning %lu\n", at);
                    return at;
                } else {
                    printf("recognitition failure: %lu != %lu  ...\n", memory[at].at, length);
                    goto next;
                }
            }
            at = parent;
            printf("while(at): recognitition success. moving to parent: %lu\n", parent);
        }
        printf("next: reached end of parent chain, and didnt return, moving on...\n");
    next:
        head = memory[try].queue_next;

        printf("next:   trying now: [head = %lu]\n", head);
    }
    return 0;
}

        struct resolved saved = memory[try]; //, saved_me = me;
        memory[try] = saved;

 
 



 nat pruned_csr(const char* given, struct resolved* memory) {
     nat length = strlen(given);
     pointer new = 2, head = 1, tail = 1;
     while (head) {
         pointer try = head, at = try;
         while (at) {
             struct name name = context.names[memory[at].index - 256];
             while (memory[at].done < name.length) {
                 nat c = name.signature[memory[at].done++];
                 if (c < 256) {
                     if (c != (nat) given[memory[at].at]) goto next;
                     memory[at].at++; best(memory[at].at);
                 } else {
                     for (nat i = 0; i < context.name_count; i++) {
                         if (new < memory_size) {
                             tail = memory[tail].queue_next = new;
                             memory[new++] = (struct resolved){i + 256, memory[at].at, 0, at, 0, 0, {0}};
                         }
                     }
                     goto next;
                 }
             }
             pointer parent = memory[at].parent;
             if (parent) { memory[parent].at = memory[at].at; at = parent; }
             else if (memory[at].at == length) return at;
             else goto next;
         }
         next: head = memory[try].queue_next;
     }
     return 0;
 }




//
//
// we are almost done!
//
//
// we just need to duplicate all parent trees, before pushing
// things,
//
//
//    so like, we duplicate the tree so that each node we append to the queue, has its own little version of the tree, that it can operate on..?
//
//
// i think so........
//
// that seemt o be the root of the priblem'
//
//
// now,
//
// @   after that, we have to figure out how to properly imple,ent paraemters!!!
//
//    we need to push and pop them, in a stacky way, so that we can backtrack from an argument list, properly..?
//
// thats important.
//
//
 

 
 
 
 
 struct resolved {
     nat index;
     nat begin;
     nat done;
     nat parent;
     nat head;
     nat tail;
     nat next;
     nat queue_next;
 };
 



static void debug_context() {
    printf("\n----- names: ------ \n{\n");
    for (size_t i = 0; i < context.name_count; i++) {
        char buffer[4096] = {0}; size_t index = 0;
        represent(i + 256, buffer, sizeof buffer, &index);
        printf("\t%6lu: %s\n\n", i, buffer);
    } puts("}\n");
}

static inline void represent
(size_t given, char* buffer, size_t limit,
 size_t* at) {
    if (given < 256) {
        buffer[(*at)++] = given;
        return;
    } else given -= 256;
    if (given >= context.name_count) return;
    struct name s = context.names[given];
    for (size_t i = 0; i < s.length; i++) {
        const size_t c = s.signature[i];
        if (c < 256) {
            buffer[(*at)++] = c;
        } else {
            buffer[(*at)++] = ' ';
            buffer[(*at)++] = '(';
            represent(c, buffer, limit, at);
            buffer[(*at)++] = ')';
            buffer[(*at)++] = ' ';
        }
    }
}




 void represent(size_t given, char* buffer, size_t limit, size_t* at, struct context* context) {
     if (given < 256) {
         buffer[(*at)++] = given;
         return;
     } else given -= 256;
     if (given >= context->name_count) return;
     struct name s = context->names[given];
     for (size_t i = 0; i < s.length; i++) {
         const size_t c = s.signature[i];
         if (c < 256) {
             buffer[(*at)++] = c;
         } else {
             buffer[(*at)++] = ' ';
             buffer[(*at)++] = '(';
             represent(c, buffer, limit, at, context);
             buffer[(*at)++] = ')';
             buffer[(*at)++] = ' ';
         }
     }
 }
 
 
 
 
 
 
 

 void debug_context(struct context* context) {
     printf("\n[best = %lu]\n", context->best);
     printf("---- debugging frames: ----\n");
     for (size_t i = 0; i < context->frame_count; i++)
         printf("\tframe # %lu  bp = %lu\n",
                i, context->frames[i]);
     printf("\n---- debugging indicies: ----\n");
     printf("\t\tidxs: { ");
     for (size_t i = 0; i < context->index_count; i++)
         printf("%lu ", context->indicies[i]);
     printf("}\n\n----- names: ------ \n{\n");
     for (size_t i = 0; i < context->name_count; i++) {
         char buffer[4096] = {0}; size_t index = 0;
         represent(i + 256, buffer, sizeof buffer,
                   &index, context);
         printf("\t%6lu: %s\n\n", i, buffer);
     } printf("}\n\n");
 }
 


 static void duplicate(nat me, const nat max_memory_size, nat* next, res_array memory) {

     struct resolved child = {
         .index = 0,
         .begin = memory[me].begin,
         .done = 0,
         .next = 0, /// ?
         .parent = *next + 1,  /// ?
         .count = 0,
         .args = {0},
     };

     
         if (*next < max_memory_size) {
             memory[tail].queue_next = new;
             tail = new;
             memory[new] = ;
             new++;
         }
         
     
         if (*next < max_memory_size) {
             memory[new] = memory[n];
             memory[new].args[memory[new].count++] = new - 1;
             memory[new].parent = new + 1;
             new++;
             n = memory[n].parent;
         }
         
        /// theproblem is that we need to redo the arguments,
        /// for  each parent, now. not just the .parent's, but also
        /// every single last argument, i think. thats crucial. lets do it.
         
     
         while (n && new < memory_size) {
             memory[new] = memory[n];
             memory[new].args[memory[new].count - 1] = new - 1;
             memory[new].parent = new + 1;
             new++;
             n = memory[n].parent;
         }
 
         memory[new - 1].parent = 0;
 }
 

 
     memory[(*next)++] = child;
 
     memory[me].args[memory[me].count++] = *next;
 
     memory[(*next)++] = child;
 
 
     assert(memory[me].queue_next == 0);
     


     memory[(*next)].queue_next = new;
     (*next) = (*next);
     memory[(*next)++] = ;

     memory[(*next)] = memory[n];
     memory[(*next)].args[memory[new].count++] = (*next) - 1;
     memory[(*next)++].parent = (*next) + 1;
     n = memory[n].parent;




     
     
     
     
     
     
     
        
      
//      [===== 0 =====]
//      ------ 1 ------
//      ------ 2 ------
//      [      3      ]
//      [      4      ]
//      [      5      ]
//      [      6      ]
//
            
//      next = 3
        
     
     
 
     if (*next < max_memory_size) {
         memory[me].queue_next = *next;
         memory[(*next)++] = child;
     }
 
     
     /// push(arg);      dont forget to add it to the args list of me.
     
     
      for each parent starting from me, and working our way to our greatest grandparent,
     while (me) {
         if (*next >= max_memory_size) { printf("error, ran out of memory."); return; }
 
          // push(me);
 
          // set memtop parent to be next;
          // set memtop argback to be next - 2;
 
         me = memory[me].parent;
     }
 
     memory[*next - 1].parent = 0;
 
 
     
     
         {
             memory[tail].queue_next = new;
             tail = new;
             memory[new] = ;
             new++;
         }
     
         if (*next < max_memory_size) {
             memory[new] = memory[n];
             memory[new].args[memory[new].count++] = new - 1;
             memory[new].parent = new + 1;
             new++;
             n = memory[n].parent;
         }

         while (n && new < memory_size) {
             memory[new] = memory[n];
             memory[new].args[memory[new].count - 1] = new - 1;
             memory[new].parent = new + 1;
             new++;
             n = memory[n].parent;
         }
 
         memory[new - 1].parent = 0;


 

        const nat save = memory[head].begin;
                              
             memory[head].done = 0;
             memory[head].begin = save;
 
 
 
 static void clone(nat me, const nat max, nat* next, nat* tail, res_array memory) {

     printf("cloning parent stack...\n");
         
     if (*next >= max) {
         printf("error, ran out of memory.\n");
         return;
     }
     
     struct resolved child = {
         .index = 0,
         .begin = memory[me].begin,
         .done = 0,
         .queue_next = 0,
         .parent = *next + 1,
         .count = 0,
         .args = {0},
     };
         
     memory[*tail].queue_next = *next;
     *tail = *next;
     
     if (*next < max) memory[(*next)++] = child;
     
     if (*next < max) memory[(*next)++] = memory[me];
         
     memory[*next - 1].args[memory[*next - 1].count++] = *next - 2;
     
     
     
 }

 static bool matches(nat n, string given, res_array memory,
                       nat max, nat* next, nat* tail) {
     for (; n; n = memory[n].parent) {
         
         struct name name = context.names[memory[n].index];
         
         for (; memory[n].done < name.length; ) {
             
             printf("looking at: \n");
             display_signature(name.signature, memory[n].done, name.length);
             display_string_at_char(given, memory[n].begin);
                         
             nat c = name.signature[memory[n].done++];
         
             if (c >= 256) clone(n, max, next, tail, memory);
             if (c >= 256) {
                 printf("[found argument, failing]\n");
                 return 0;
             }
             if (c != (nat) given[memory[n].begin]) {
                 printf("[character mismatch, failing]\n");
                 return 0;
             }
             memory[n].begin++;
             best(memory[n].begin);
         }
         
         printf("[signature succeeded]\n");
         if (!memory[n].parent) {
             printf("at last parent, returning %s", memory[n].begin == strlen(given) ? "SUCCESS" : "FAILURE");
             return memory[n].begin == strlen(given);
         }
         
         printf("continuing to next parent...\n");
     }
     printf("[failing signature][out]\n");
     return 0;
 }

 static nat csr(string given, res_array memory, nat max) {
     nat next = 1, head = 1, tail = 1;
     
     for (; head; head = memory[head].queue_next) {
                 
         printf("------------ moving on to head = %lu --------------\n", head);
         
         debug_memory(memory, head, next, max);
         
         const nat save = memory[head].begin;
         
         for (; memory[head].index < context.name_count; memory[head].index++) {
             
             printf("\n\ntrying index:  %lu \n", memory[head].index);
             
             memory[head].done = 0;
             memory[head].begin = save;
             
             if (matches(head, given, memory, max, &next, &tail)) {
                 printf("found a match!\n");
                 return head;
             } else {
                 printf("didnt match, trying next index...\n");
             }
         }
         
     }
     printf("[NO MATCH FOUND]\n");
     return 0;
 }






 
 
 
 nat compressed_broken_csr(const char* given, struct resolved* memory) {
     nat length = strlen(given);
     nat new = 2, head = 1, tail = 1;
     while (head) {
         nat try = head, n = try;
         struct resolved saved = memory[try], me;
         while (n) {
             me = memory[n];
             struct name name = context.names[me.index - 256];
             while (me.done < name.length) {
                 nat c = name.signature[me.done];
                 if (c < 256) {
                     if (c != (nat) given[me.at]) goto next;
                     me.at++; me.done++; best(me.at);
                 } else {
                     for (nat i = 0; i < context.name_count; i++) {
                         struct resolved arg = {i + 256, me.at, 0, new, 0, 0, {0}};
                         if (new < memory_size) memory[new++] = me;
                         if (new < memory_size) { memory[tail].queue_next = new; tail = new; memory[new++] = arg; }
                     }
                     goto next;
                 }
             }
             if (me.parent) {
                 memory[me.parent].args[memory[me.parent].count++] = n;
                 memory[me.parent].at = me.at;
                 memory[me.parent].done++;
             } else if (!me.parent && me.at == length) return n;
             n = me.parent;
         }
         next: head = memory[try].queue_next; memory[try] = saved;
     }
     return 0;
 }

 
 


 nat csr_using_gotos(const uint8_t* given, nat length, struct resolved* memory) {
     
     nat ptr = 2, head = 1, tail = 1;
     struct name name;
     struct resolved saved_N;
 top:
     debug_memory(memory, head, tail);
     nat try = head, current = try;
     saved_N = memory[try];
     
 recognize:
     name = context.names[memory[current].index - 256];
     while (memory[current].done < name.length) {
         
         nat c = name.signature[memory[current].done];
         display_string_at_char((const char*) given, memory[current].at);
         display_signature(name.signature, memory[current].done, name.length);
         
         if (c < 256) {
             if (c == given[memory[current].at]) {
                 memory[current].at++; memory[current].done++; best(memory[current].at);
             } else {
                 printf("character mismatch!     given[.begin]('%c') != sig[.done]('%c')     moving on.\n", given[memory[current].at], (char) c);
                 goto next;
             }
         } else {
             printf("found parameter: pushing signatures...\n");
             for (nat i = 0; i < context.name_count; i++) {
                 if (ptr < memory_size) {
                     memory[ptr] = (struct resolved){i + 256, memory[current].at, 0, current, 0, 0, {0}};
                     memory[tail].queue_next = ptr; tail = ptr++;
                 }
             }
             printf("pushing complete. moving on.\n");
             goto next;
         }
     }
     
     if (memory[current].parent) {
         nat parent = memory[current].parent;
         memory[parent].args[memory[parent].count++] = current;
         memory[parent].at = memory[current].at;
         memory[parent].done++;
         current = parent;
         printf("recognitition success: recognizing parent: %lu\n", parent);
         goto recognize;
     }
     
     if (memory[current].at == length) {
         printf("recognitition success: returning %lu\n", current);
         return current;
     }
     
 next:
     head = memory[try].queue_next;
     memory[try] = saved_N;
     printf("at next:   [head = %lu]\n", head);
     if (head) goto top;
     return 0;
 }




nat unused_csr(const char* given, struct resolved* memory) {
    nat length = strlen(given);
    pointer new = 2, head = 1, tail = 1;

    while (head) {

        debug_memory(memory, head, tail);

        pointer try = head, at = try;
        struct resolved saved = memory[try], current;

        while (at) {

            current = memory[at];
            struct name name = context.names[current.index - 256];

            display_string_at_char(given, current.at);
            display_signature(name.signature, current.done, name.length);

            while (current.done < name.length) {
                character c = name.signature[current.done];
                if (c < 256) {
                    if (c == (nat) given[current.at]) { current.at++; current.done++; best(current.at); }
                    else {
                        printf("character mismatch!     given[.begin]('%c') != sig[.done]('%c')     moving on.\n", given[current.at], (char) c);
                        goto next;
                    }
                } else {
                    printf("found parameter: pushing signatures...\n");
                    for (nat i = 0; i < context.name_count; i++) {
                        if (new < memory_size) {
                            memory[new] = (struct resolved){i + 256, current.at, 0, at, 0, 0, {0}};
                            memory[tail].queue_next = new; tail = new++;
                        }
                    }
                    printf("pushing complete. moving on.\n");
                    goto next;
                }
            }
            if (current.parent) {
                pointer parent = current.parent;
                memory[parent].args[memory[parent].count++] = at;
                memory[parent].at = current.at;
                memory[parent].done++;
                at = parent;
                printf("recognitition success: recognizing parent: %lu\n", parent);
            }
        }

        if (current.at == length) {
            printf("recognitition success: returning %lu\n", at);
            return at;
        }
    next:
        head = memory[try].queue_next;
        memory[try] = saved;
        printf("at next:   [head = %lu]\n", head);
    }
    return 0;
}













static void debug_resolved_next(struct resolved* memory, nat r, nat depth) {
    const struct resolved given = memory[r];
    prep(depth); printf("@%lu: { index: %lu, begin: %lu, done: %lu, parent: %lu, next: %lu, previous: %lu, count: %lu} \n", r, given.index, given.begin, given.done, given.parent, given.next, given.previous, given.count);
    for (size_t i = 0; i < given.count; i++) {
        prep(depth + 1); printf("#%lu: %lu\n", i, given.args[i]);
        debug_resolved_next(memory, given.args[i], depth + 1);
    }

    if (given.next) {
        prep(depth); printf("next: \n");
        debug_resolved_next(memory, given.next, depth);
    }
}

static void debug_resolved_previous(struct resolved* memory, nat r, nat depth) {
    const struct resolved given = memory[r];
    prep(depth); printf("@%lu: { index: %lu, begin: %lu, done: %lu, parent: %lu, next: %lu, previous: %lu, count: %lu} \n", r, given.index, given.begin, given.done, given.parent, given.next, given.previous, given.count);
    for (size_t i = 0; i < given.count; i++) {
        prep(depth + 1); printf("#%lu: %lu\n", i, given.args[i]);
        debug_resolved_previous(memory, given.args[i], depth + 1);
    }

    if (given.previous) {
        prep(depth); printf("previous: \n");
        debug_resolved_previous(memory, given.previous, depth);
    }
}


nat csr(const char* given, nat end, nat max_depth, struct resolved* memory, nat ptr) {
    
    
    memory[ptr] = (struct resolved){256 + 1, 0, 0, 0, 0, 0, 0};
    nat queue = ptr++;
                
    while (queue) {
        
        struct resolved node = queue_pop(&queue, &queue_count);
        struct name name = context.names[node.index - 256];
        
        while (node.done < name.length) {
            nat c = name.signature[node.done];
            if (c < 256) {
                if (c == given[node.begin]) {
                    node.begin++; node.done++;
                } else goto next;
            } else {
                for (nat i = 0; i < context.name_count; i++) {

                    // push i to node's parameter list.

                    // push that tree to the queue.

                    // pop i from node's parameter list.         (undo)

                }
                goto next;
            }
        }
        
        if (node.parent) {
            node.parent->begin = node.begin;
            node.parent->done++;
             ????
            
        } else if (node.begin == end) return node;
        next: continue;
    }
    
    return 0;
}






struct resolved queue_pop(struct resolved** v, size_t* count) {
    if (!*count) abort();
    struct resolved f = **v;
    --*count;
    memmove(*v, *v + 1, sizeof(struct resolved) * *count);
    *v = realloc(*v, sizeof(struct resolved) * *count);
    return f;
}

void queue_push(struct resolved f, struct resolved** v, size_t* count) {
    *v = realloc(*v, sizeof(struct resolved) * (*count + 1));
    (*v)[(*count)++] = f;
}



void debug_queue(struct resolved* queue, nat queue_count) {
    printf("----------------------------- debugging queue (queue count = %lu) ------------------------\n\n", queue_count);
    for (nat i = 0; i < queue_count; i++) {
        printf("\t----------------- queue node #%lu --------------- \n", i);
        struct resolved n = queue[i];
        debug_resolved(n, 2);
        printf("\t-------------------------------------------------\n\n");
    }
    printf("----------------------------------------------------------------------------------------\n");
}


























    do if (c < n) s[c++] = 0;
    else while (c && ++s[c - 1] == m) c--;
    while (c);



// go through the list of context names.

/// we need to append things to the queue.
/// right here. specfically, things related to the context names.

//                  DO NOT DO THIS: top.done++;
// do this later.




 void print_signature(size_t* signature, size_t length) {
     for (size_t i = 0; i < length; i++)
         printf("%c ",
                signature[i] < 256
                 ? (char) signature[i]
                 : '_');
 }
 
 
 
 
 
 
 
 void stack_push(struct resolved f, struct resolved** v, size_t* count) {
     *v = realloc(*v, sizeof(struct resolved) * (*count + 1));
     (*v)[(*count)++] = f;
 }


 
 
 
 void print_stack(int* stack, int stack_count) {
     printf("[ ");
     for (int i = 0; i < stack_count; i++)
         printf("%d ", stack[i]);
     puts("]");
 }




struct resolved duplicate(struct resolved r) {
    struct resolved s = {
        r.index, r.count, r.begin, r.done,
        calloc(r.count, sizeof(struct resolved))
    };

    for (size_t i = 0; i < r.count; i++)
        s.args[i] = duplicate(r.args[i]);

    return s;
}












 
 
 
 
 struct qnode {
     size_t depth;
     size_t stack_count;
     struct resolved* stack;
 };



 struct resolved csr(uint8_t* given, size_t end, size_t max_depth) {
     
     size_t Q_count = 0;
     struct qnode* Q = NULL;
     queue_push((struct qnode){0, 0, NULL}, &Q, &Q_count);
     stack_push((struct resolved){256 + 4, 0, 0, 0, 0}, &Q[0].stack, &Q[0].stack_count);
     
     while (Q_count) {
         struct qnode node = queue_pop(&Q, &Q_count);
         if (node.depth > max_depth) continue;
         
         while (node.stack_count) {
             struct resolved top = node.stack[node.stack_count - 1];
             struct name name = context.names[top.index - 256];
             
             while (top.done < name.length) {
                 size_t c = name.signature[top.done];
                 if (c < 256) {
                     if (c == given[top.begin]) {
                         top.begin++; top.done++;
                     } else {
                         // fail;
                     }
                 } else {
                     for (size_t i = 0; i < context.name_count; i++) {
                         
                         struct qnode new = {node.depth + 1, node.stack_count + 1, calloc(node.stack_count + 1, sizeof(struct resolved))};
                         for (size_t i = 0; i < node.stack_count; i++) {
                             new.stack[i] = duplicate(node.stack[i]);
                         }
                         new.stack[node.stack_count] = (struct resolved){256 + i, 0, top.begin, 0, 0};
                         queue_push(new, &Q, &Q_count);
                     } // names
                     // fail;
                 }
             } // sig
             
             node.stack_count--;
             if (node.stack_count == 0) {
                 if (top.begin == end) return top;
                 else break; // fail;
             }
         } // stack
         
     } // queue
     return (struct resolved){0};
 }



void c_possibilities(int m, int n) {

    int* s = calloc(n, sizeof(int));
    int c = 0;
    do if (c < n) s[c++] = 0;
    else while (c && ++s[c - 1] == m) c--;
    while (c);
}



void possibilities(int modulus, int length) {

    int* stack = calloc(length, sizeof(int));
    int count = 0;

    empty:
    printf("curr: "); print_stack(stack, count);
    if (count < length) {
        stack[count++] = 0;
        printf("push: "); print_stack(stack, count);
        goto empty;
    }
    full:
    if (count) {
        if (++stack[count - 1] == modulus) {
            count--;
            printf(" pop: "); print_stack(stack, count);
            goto full;
        } goto empty;
    }
}



     possibilities(2, 3);


     
     int n = 3, m = 2;
     
     int* s = calloc(n, sizeof(int));
     int c = 0;

     do if (c < n) s[c++] = 0;
     else while (c && ++s[c - 1] == m) c--;
     while (c);
    
     b: if (c < n) { s[c++] = 0; goto b; }
     e: if (c) {if (++s[c - 1] == m) { c--; goto e; } goto b; }
      
 


void new2_clean_possibilities(int modulus, int length) {

    int* stack = calloc(length, sizeof(int));
    int stack_count = 0;

    loop: if (stack_count < length) {
        stack[stack_count++] = 0; goto loop;
    }
    pop: if (stack_count) {
        if (++stack[stack_count - 1] == modulus) {
            stack_count--; goto pop;
        } goto loop;
    }
}


void clean_possibilities(int modulus, int length) {

    int* stack = calloc(length, sizeof(int));
    int stack_count = 0;

    loop:
    if (stack_count < length) {
        stack[stack_count++] = 0;
        goto loop;
    }

    pop:
    if (stack_count && stack[stack_count - 1] == modulus - 1) {
        stack_count--;
        goto pop;
    }

    if (stack_count) {
        stack[stack_count - 1]++;
        goto loop;
    }
}


void old_iterate(int* M, int n) {
    int L[n];
    memset(L, 0, sizeof L);

    int i = 0;
    while (i < n) {

        for (int i = 0; i < n; i++) printf("%d ", L[i]);
        puts("");

        i = 0;
        while (i < n && ++L[i] >= M[i])
            L[i++] = 0;
    }
}

void old_iterate(int name_count, int arg_count) {
    int L[arg_count];
    memset(L, 0, sizeof L);

    int i = 0;
    while (i < arg_count) {

        for (int i = 0; i < arg_count; i++) printf("%d ", L[i]); puts("");

        i = 0;
        while (i < arg_count && ++L[i] >= name_count) L[i++] = 0;
    }
}



 the expotential code.
 
    ie, how to generate all possibilities:    given a modulus, and a length.
 
 while (stack_count) {
     if (stack_count < length) {
         stack[stack_count++] = 0;
         print_int_stack(stack, stack_count);
     } else {
         while (stack_count && stack[stack_count - 1] == modulus - 1) {
             stack_count--;
             print_int_stack(stack, stack_count);
         }
         if (stack_count) stack[stack_count - 1]++;
         print_int_stack(stack, stack_count);
     }
 }
 
 
 
 
 now, using gotos:
 
 

 void clean_possibilities(int modulus, int length) {
     
     int* stack = calloc(length, sizeof(int));
     int stack_count = 0;
         
     loop:
     if (stack_count < length) {
         stack[stack_count++] = 0;
         goto loop;
     }
     
     pop:
     if (stack_count && stack[stack_count - 1] == modulus - 1) {
         stack_count--;
         goto pop;
     }

     if (stack_count) {
         stack[stack_count - 1]++;
         goto loop;
     }
 }
 


 void clean_possibilities(int modulus, int length) {
     
     int* stack = calloc(length, sizeof(int));
     int stack_count = 0;
         
     loop:
     if (stack_count < length) {
         stack[stack_count++] = 0;
         goto loop;
     }
     
     pop:
     if (stack_count && stack[stack_count - 1] == modulus - 1) {
         stack_count--;
         goto pop;
     }

     if (stack_count) {
         stack[stack_count - 1]++;
         goto loop;
     }
 }
 



    const int T = 3;


    int stack[bignumber] = {9};
    int count = 1;

    int remaining = 2;
    print_vector(stack, count);
    while (count) {
        print_vector(stack, count);
        while (remaining) {
            print_vector(stack, count);
            for (int i = 0; i < T; i++)  {
                printf("pushing i = %d\n", i);
                stack[count++] = i;
                print_vector(stack, count);
            }
            print_vector(stack, count);
            remaining--;
            print_vector(stack, count);
        }
        print_vector(stack, count);
        count--; // undo decision.
        print_vector(stack, count);
    }

 if (node.depth > max_depth) continue;


 const int T = 3;
 
 for (int i0 = 0; i0 < T; i0++) {
     for (int i1 = 0; i1 < T; i1++) {
         printf("%d, %d\n", i0,i1);
     }
 }
 




 struct resolved my_csr(uint8_t* given, size_t end, size_t max_depth) {
     
     size_t Q_count = 0;
     struct frame* Q = NULL;
     queue_push((struct frame){0, 0, NULL}, &Q, &Q_count);
     stack_push((struct resolved){256 + 4, 0, 0, 0, 0}, &Q[0].stack, &Q[0].stack_count);
     
     while (Q_count) {
         struct frame node = queue_pop(&Q, &Q_count);
         
         while (node.stack_count) {
             struct resolved top = node.stack[node.stack_count - 1];
             struct name name = context.names[top.index - 256];
             
             while (top.done < name.length) {
                 size_t c = name.signature[top.done];
                 if (c < 256) {
                     if (c == given[top.begin]) {
                         top.begin++; top.done++;
                     } else {
                         // fail;
                     }
                 } else {
                     for (size_t i = 0; i < context.name_count; i++) {
                         
                         struct frame new = {node.depth + 1, node.stack_count + 1, calloc(node.stack_count + 1, sizeof(struct resolved))};
                         for (size_t i = 0; i < node.stack_count; i++) {
                             new.stack[i] = duplicate(node.stack[i]);
                         }
                         new.stack[node.stack_count] = (struct resolved){256 + i, 0, top.begin, 0, 0};
                         queue_push(new, &Q, &Q_count);
                     } // names
                 }
             } // sig
             
             node.stack_count--;
             if (node.stack_count == 0) {
                 if (top.begin == end) return top;
                 else break;
             }
         } // stack
     } // queue
     return (struct resolved){0};
 }

 

struct resolved old_resolve
(uint8_t* given, size_t end, struct resolved* stack,
 size_t count, size_t depth, struct resolved sol)
{
    if (depth >= 5)
        return (struct resolved){0};

    struct name name = context.names[sol.index - 256];

    while (sol.done < name.length) {
        size_t c = name.signature[sol.done];

        if (c < 256) {
            if (c != given[sol.begin]) return (struct resolved){0};
            sol.begin++; sol.done++;
            context.best = sol.begin > context.best ? sol.begin : context.best;

        } else {

            struct resolved* extended = calloc(count + 1, sizeof(struct resolved));
            for (size_t i = 0; i < count; i++) extended[i] = stack[i];
            extended[count++] = sol;

            for (size_t i = 0; i < context.name_count; i++) {
                struct resolved parent =
                    old_resolve(given, end, extended, count, depth + 1,
                        (struct resolved){256 + i, 0, sol.begin, 0, 0});
                if (parent.index) return parent;
            }

            return (struct resolved){0};
        }
    }
    if (count == 0) {
        if (sol.begin == end) return sol;
        else return (struct resolved){0};

    } else {
        struct resolved top = duplicate(stack[--count]);
        top.begin = sol.begin;
        top.args = realloc(top.args, sizeof(struct resolved) * (top.count + 1));
        top.args[top.count++] = sol;
        top.done++;

        return old_resolve(given, end, stack, count, depth, top);
    }
}







    while (stack_count) {

        struct resolved sol = stack[stack_count - 1];
        struct name name = context.names[sol.index - 256];

        while (sol.done < name.length) {

            size_t c = name.signature[sol.done];

            if (c < 256) {
                if (c != given[sol.begin]) return (struct resolved){0}; // backtrack.
                sol.begin++; sol.done++;

            } else {
                for (size_t i = 0; i < context.name_count; i++) {

                    struct resolved arg = (struct resolved){256 + i, 0, sol.begin, 0, 0};

                    struct resolved* extended = calloc(stack_count + 1, sizeof(struct resolved));
                    for (size_t i = 0; i < stack_count; i++) extended[i] = stack[i];
                    extended[stack_count++] = arg;

                    struct resolved parent = CSR(given, end, extended, stack_count, depth + 1);

                    if (parent.index) return parent;
                }
                return (struct resolved){0}; // backtrack.
            }
        }

        struct resolved top = duplicate(stack[--stack_count]);
        top.begin = sol.begin;
        top.args = realloc(top.args, sizeof(struct resolved) * (top.count + 1));
        top.args[top.count++] = sol;
        top.done++;
    }

    return sol.begin == end ? sol : (struct resolved) {0};



void print_vector_nonewline(int* v, size_t count) {
    printf("vector %lu:{ ", count);
    for (size_t i = 0; i < count; i++) {
        printf("%d ", v[i]);
    }
    printf("}");
}



   for arity of 3 functions:
   
   
      
                      func(x, y, z)
   

          1.    T T T
   
          2.    T T NT
   
          3.    T NT T
   
          4.    T NT NT
   
          5.    NT T T
          
          6.    NT T NT
          
          7.    NT NT T
          
          8.    NT NT NT
        
      







/// the input string:

int given[] = {24, 35, 236, 46};
int given_count = 4;


/// the symbol table:
const char* T = "ab";
const char* NT = "ij";

int T_count = 2;
int NT_count = 2;








    if (!mode_count) {  // no more parameters, ie, this is a terminal.
        printf("%*s", d * 2, "");
        puts("a: no more parameters");
        //        return 1; // success
    }





struct parent {

    bool* mode_array;
    int mode_count;

    int* signature;
    int signature_length;

    int remaining_elements;     // kinda like the inverse of "done".

    struct resolved solution;
};

int try_signature
(
 int* signature, int length,
 int param_count,
 struct parent* parent_stack, int psc
 ) ;



int a(int* signature, int length,
      bool* mode_array, int mode_count,
      int d,
      struct parent* parent_stack, int psc) {

    if (!mode_count) { // empty signature base case.
        printf("%*s", d * 2, "");
        puts("a: ---");
        return 1; // success
    }

    int c = signature[length - 1];

    if (c < 256) {

        if (c == given[0]) {

            return a(signature, length - 1, mode_array, mode_count - 1, d + 1, parent_stack, psc);

        } else {
            return 0;
        }

    } else {
        int is_nonterminal = mode_array[mode_count - 1];

        if (!is_nonterminal) {
            for (int i = 0; i < T_count; i++) {

                printf("%*s", d * 2, "");
                printf("a: T: %c\n", T[i]);

                int signature[] = {1, 3, 4}; // get this from i, and context
                int length = 3; // get this from i, and context
                int param_count = 0; // get this from i, and context

                int successful = try_signature(signature, length, param_count, parent_stack, psc);
                if (successful) return 1;



                
//
//
//                 things we need on the parent stack:
//
//                    - the mode array and its count (which is known as "arguments_remaining")
//                        -
//
//                 then INSIDE OF the TRY RECOGNIZE, we recognize the rest of the signature, if try was successful.
//

                


            }
        } else {
            for (int i = 0; i < NT_count; i++) {

                printf("%*s", d * 2, "");
                printf("a: NT: %c\n", NT[i]);

                int rest_of_args = a(signature, length - 1, mode_array, mode_count - 1, d + 1);
                if (rest_of_args) return 1;
                if rest of args succeeded too, then we resolved the signature.
            }
        }
    }


    return 0;
}




int b(int* signature, int length,
       int argument_count, int d,
       bool* terminal_type_stack, int stack_count) {

    printf("%*s", d * 2, "");
    printf("b: #%d...\n", argument_count);

    if (!argument_count) {
        printf("%*s", d * 2, "");
        printf("b: --- ");
        print_vector_nonewline((int*) terminal_type_stack, stack_count);
        printf("\n");
        //         try all arguments, give this "terminalness-arrangement" for the arguments.

        a(signature, length, terminal_type_stack, stack_count, 0, parent_stack, parent_stack_count);
        return 1;

    } else {
        for (int i = 0; i < 2; i++) {//  trying terminals or not
            terminal_type_stack[stack_count] = i;
            b(signature, length, argument_count - 1, d + 1, terminal_type_stack, stack_count + 1, parent_stack, parent_stack_count);
        }
    }

    return 0;
}



int try_signature
 (
  int* signature, int length,
  int param_count,
  struct parent* parent_stack, int psc
  ) {

    bool args[bignumber] = {0};
    b(signature, 3, 0
        //  arg count
 , 0, args, 0);


    if (psc > 0) {

        struct parent top = parent_stack[psc - 1];

        return a(
                            top.signature, top.signature_length - 1,
                            top.mode_array, top.mode_count - 1,
                            0,
                            parent_stack, psc
                            );
        }

     }

    
     NO!! NO!! NO!!!!  the following call to a()     DOESSSS NNNNNOOOOTTTTTTTTTTTTTT      DOES NOT


                    DOES NOT


        happen here.


     it should happen deep inside the call to try_signature(). when we are prompted to essentially resolve our parents, one by one, off the stack. thats what we need to do.
      and then only when we realize that we have


            no more parents to recognize,

                ..do we check if solution.begin == end.        if so, succ.        if not, fail, and backtrack a hole bunch.



            oh, okay, so is a corrlaryl of this, simply that



                i need to actually push the binary vector onto the stack?











void other_test() {

    printf("testing t and nt loop structures....\n");

    for (int i = 0; i < T_count; i++) {
        for (int j = 0; j < T_count; j++) {
            printf("( %c , %c )\n", T[i], T[j]);
        }
    }


    for (int i = 0; i < T_count; i++) {
        for (int j = 0; j < NT_count; j++) {
            printf("( %c , %c )\n", T[i], NT[j]);
        }
    }

    for (int i = 0; i < NT_count; i++) {
        for (int j = 0; j < T_count; j++) {
            printf("( %c , %c )\n", NT[i], T[j]);
        }
    }

    for (int i = 0; i < NT_count; i++) {
        for (int j = 0; j < NT_count; j++) {
            printf("( %c , %c )\n", NT[i], NT[j]);
        }
    }
}






struct my_resolved {
    struct res* args;
    int count;
    int index;
    int begin;
    int done;
    int depth;
};

struct stack {
    struct res* parents;
    int count;
};




void my_csr() {


}



void iterate(int* M, int n) {
    int L[n];
    memset(L, 0, sizeof L);

    int i = 0;
    while (i < n) {

        for (int i = 0; i < n; i++) printf("%d ", L[i]);
        puts("");

        i = 0;
    while (i < n && ++L[i] >= M[i]) L[i++] = 0;
    }
}

    int d[10] = {0, 0};


    iterate(d, 2);

 
 
 
*/

#endif /* comments_h */
