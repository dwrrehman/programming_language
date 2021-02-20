/// Daniel Rehman,
/// CE202008285.121444
/// modified on 2010095.113110

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef size_t nat;
typedef const char* string;
static string context = "-.hello.bubbles_there_.bob.";
static string input = "bubbleshellotherehello";

struct unit {
    nat index;  // ranges in context
    nat parent; // ranges in list
    nat begin;  // ranges in input
};

struct node {
    nat self; // ranges in list
    nat next; // ranges in queue
};

static const nat undefined_value = 13744632839234567870UL;
static const nat parent_of_root = 999999999;
static const nat end_of_queue = 9999999000;
static const struct unit undefined_unit = {undefined_value, undefined_value, undefined_value};
static const struct node undefined_node = {undefined_value, undefined_value};

// i == head ? 'H' : ' ', i == tail ? 'T' : ' ', i == next ? 'N' : ' ',

static inline void debug_list(struct unit* memory, nat size) {
    puts("LIST: {\n");
    for (nat i = 0; i < size; i++) {
        printf("   %5lu       i:%-5lu    p:%-5lu    b:%-5lu   \n", i, memory[i].index, memory[i].parent, memory[i].begin);
    }
    puts("}\n");
}

static inline void debug_queue(struct node* queue, nat size) {
    puts("QUEUE: {\n");
    for (nat i = 0; i < size; i++) {
        printf("   %5lu       s:%-5lu    n:%-5lu    \n", i, queue[i].self, queue[i].next);
    }
    puts("}\n");
}

static inline void display_signature(string context, nat at) {
    printf("\n         signature: ");
    for (nat i = 0; i < strlen(context) + 1; i++)
        printf(i == at ? "[%c] " : "%c ", i == strlen(context) ? '?' : context[i]);
    puts("");
}

static inline void display_string_at_char(string input, nat at) {
    printf("\n            string:  ");
    for (nat i = 0; i < strlen(input) + 1; i++)
        printf(i == at ? "[%c] " : "%c ", input[i]);
    puts("\n");
}



void queue_push
(struct node* queue,
 nat queue_size,
 nat* tail, nat* queue_next,
 struct node element) {
    if (*queue_next >= queue_size) abort();
    if (*tail != end_of_queue)
        queue[*tail].next = *queue_next;
    *tail = *queue_next;
    queue[*queue_next] = element;
    ++*queue_next;                      // how to find the next open spot?
}

//struct node queue_pop_front
//(struct node* queue,
// nat queue_size,
// nat* head) {
//    if (*head == end_of_queue) abort();
//
//    return n;
//}

void list_push(struct unit* list,
                nat list_size,
                nat* next,
                struct unit element) {
    if (*next >= list_size) abort();
    list[*next] = element;
    ++*next;
}

string solve(struct unit* list, nat list_size,
             struct node* queue, nat queue_size) {
    
    nat head = 0, tail = 0, list_next = 0, queue_next = 0;
    queue_push(queue, queue_size, &tail, &queue_next, (struct node) {parent_of_root, end_of_queue});
    queue) {

        { puts("------------------------queue------------------------"); debug_list(list, list_size); debug_queue(queue, queue_size); }
        
        nat init = list_next;
        
        list_push(list, list_size, &list_next, (struct unit) {
            0, queue[head].self,
            queue[head].self == parent_of_root
                ? 0
                : list[queue[head].self].begin
        });
    
        nat save = list[init].begin;
        
        for (; list[init].index < strlen(context); list[init].index++) {
        
            list[init].begin = save;
            
            { puts("---------------index----------------"); debug_list(list, list_size); debug_queue(queue, queue_size); }
            
            
            for (nat at = init; at != parent_of_root; at = list[at].parent) {
                
                { puts("-------------parent-------------"); debug_list(list, list_size); debug_queue(queue, queue_size); }
                
                                
                nat error = 0;
                for (; context[list[at].index] != '.'; list[at].index++) {
                    
                    {display_signature(context, list[at].index); display_string_at_char(input, list[at].begin);}

                    if (error) continue;
                    else if (context[list[at].index] == '_') {
                        
                        puts("pushed arg.");
                                                
                        queue_push(queue, queue_size, &tail, &queue_next, (struct node) {
                            list_next, end_of_queue
                        });
                        list_push(list, list_size, &list_next, list[at]);
                        
                        error = 1;
                        
                    } else if (context[list[at].index] == input[list[at].begin]) list[at].begin++;
                    else error = 1;
                }
                
                //            puts("pushed finished sig.");
                //            list_push(list, list_size, &next, list[init]);
                
                if (error) break;
                else if (list[at].parent != parent_of_root) {
                    list[list[at].parent].index++;
                    list[list[at].parent].begin = list[at].begin;
                } else if (list[at].begin == strlen(input)) return "success";
                else break;
            }
        }
        nat previous = head;
        head = queue[head].next;
        queue[previous] = undefined_node;
        list[init] = undefined_unit;
    }
    return "failure";
}

void start() {
    nat ms = 10, qs = 10;
    struct unit* m = malloc(ms * sizeof(struct unit));
    struct node* q = malloc(qs * sizeof(struct node));
    puts(solve(m, ms, q, qs));
    debug_list(m, ms);
    debug_queue(q, qs);
    free(m);
}







































/*
 
 
 
 
 
 
 
 
 nat
     head = 0,   // ranges in queue
     tail = 0,   // ranges in queue
     queue_next = 0,      // ranges in queue
     list_next = 0;       // ranges in list
     
 list[list_next++] = (struct unit) {0, parent_of_root, 0};
 queue[queue_next++] = (struct node) {0, end_of_queue};
 
 do {
     
     //            .parent = head == end_of_queue ? parent_of_root : queue[head].self,
     //            .begin = head == end_of_queue ? 0 : list[queue[head].self].begin,
     
     nat initial = list_next;
     
     struct unit new = {
         .index = 0,
         .parent = queue[head].self,
         .begin = list[queue[head].self].begin
     };
     
     struct node new_node = {
         .self = initial,
         .next = end_of_queue
     };

     
     list[list_next++] = new;
     queue[queue_next++] = new_node;
         
     nat begin_save = new.begin;
     
     for (; new.index < strlen(context); new.index++) {
         
         new.begin = begin_save;
         
         for (nat at = initial; at != parent_of_root; at = list[at].parent) {
             
             struct unit this = list[at];
             
             nat error = 0;
             for (; context[this.index] != '.'; this.index++) {
                 if (error) continue;
                 else if (context[this.index] == '_') error = 1;
                 else if (context[this.index] == input[this.begin]) this.begin++;
                 else error = 1;
             }
             if (error) break;
             else if (this.parent) {
                 list[this.parent].index++;
                 list[this.parent].begin = this.begin;
             } else if (this.begin == strlen(input)) return "success";
             else break;
         }
     }
     head = head == end_of_queue ? 0 : queue[head].next;
 } while (head != end_of_queue);
 return "failure";
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 string solve(struct unit* list, nat list_size, struct node* queue, nat queue_size) {
     nat
         head = 0,   // ranges in queue
         tail = 0,   // ranges in queue
         queue_next = 0,      // ranges in queue
         list_next = 0;       // ranges in list
         
     list[list_next++] = (struct unit) {0, parent_of_root, 0};
     queue[queue_next++] = (struct node) {0, end_of_queue};
     
     do {
         
         //            .parent = head == end_of_queue ? parent_of_root : queue[head].self,
         //            .begin = head == end_of_queue ? 0 : list[queue[head].self].begin,
         
         nat initial = list_next;
         
         struct unit new = {
             .index = 0,
             .parent = queue[head].self,
             .begin = list[queue[head].self].begin
         };
         
         struct node new_node = {
             .self = initial,
             .next = end_of_queue
         };

         
         list[list_next++] = new;
         queue[queue_next++] = new_node;
             
         nat begin_save = new.begin;
         
         for (; new.index < strlen(context); new.index++) {
             
             new.begin = begin_save;
             
             for (nat at = initial; at != parent_of_root; at = list[at].parent) {
                 
                 struct unit this = list[at];
                 
                 nat error = 0;
                 for (; context[this.index] != '.'; this.index++) {
                     if (error) continue;
                     else if (context[this.index] == '_') error = 1;
                     else if (context[this.index] == input[this.begin]) this.begin++;
                     else error = 1;
                 }
                 if (error) break;
                 else if (this.parent) {
                     list[this.parent].index++;
                     list[this.parent].begin = this.begin;
                 } else if (this.begin == strlen(input)) return "success";
                 else break;
             }
         }
         head = head == end_of_queue ? 0 : queue[head].next;
     } while (head != end_of_queue);
     return "failure";
 }

 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 string old_solve(struct unit* m, nat* q, nat msize, nat qsize) {
     nat head = qsize, tail = qsize, qnext = 0, mnext = 0;
     do {
         struct unit new = {
             .index = 0,
             .parent = q[head],
             .begin = head == qsize ? 0 : m[head].begin,
         };
         nat begin_save = new.begin;
         for (; new.index < strlen(context); new.index++) {
             new.begin = begin_save;
             nat error = 0;
             for (; context[new.index] != '.'; new.index++) {
                 if (error) continue;
                 else if (context[new.index] == '_') {
                     struct unit u = {
                         .index = new.index,
                         .parent = new.parent,
                         .begin = new.begin,
                     };
                     m[qnext++] = u;
                     error = 1;
                 } else if (context[new.index] == input[new.begin]) new.begin++;
                 else error = 1;
             }
             if (error) continue;
             if (qnext < qsize) m[mnext++] = new;
             if (new.parent < qsize) {
                 m[new.parent].index++;
                 m[new.parent].begin = new.begin;
             } else if (new.begin == strlen(input)) return "success";
             struct unit this = new;
             
             for (nat at = head; at < qsize; at = m[at].parent) {
                 nat error = 0;
                 this = m[at];
                 for (; context[this.index] != '.'; this.index++) {
                     if (error) continue;
                     else if (context[this.index] == '_') {
                         m[at].index = this.index;
                         m[at].begin = this.begin;
                         error = 1;
                     }
                     else if (context[this.index] == input[this.begin]) this.begin++;
                     else error = 1;
                 }
                 if (error) break;
                 else if (this.parent < qsize) {
                     m[this.parent].index++;
                     m[this.parent].begin = this.begin;
                 } else if (this.begin == strlen(input)) return "success";
             }
         }
     } while (head < qsize);
     return "failure";
 }


 
 
 
 
 
 
 assert(new.index < strlen(context) + 1 && new.parent < size + 1 && new.begin < strlen(input) + 1 && new.queue_next < size + 1);
 assert(u.index < strlen(context) + 1 && u.parent < size + 1 && u.begin < strlen(input) + 1 && u.queue_next < size + 1);
 assert(new.index < strlen(context) + 1 && new.parent < size + 1 && new.begin < strlen(input) + 1 && new.queue_next < size + 1);
 
 printf("queue: NEW = {%lu,%lu,%lu,%lu}\n", new.index, new.parent, new.begin, new.queue_next);
 printf("NOTE: strlen(context) = %lu, strlen(input) = %lu\n", strlen(context), strlen(input));
 printf("index: NEW = {%lu,%lu,%lu,%lu}\n", new.index, new.parent, new.begin, new.queue_next);
 printf("sig NEW = {%lu,%lu,%lu,%lu}\n", new.index, new.parent, new.begin, new.queue_next);
 printf("pushed arg {%lu,%lu,%lu,%lu}\n", u.index, u.parent, u.begin, u.queue_next);
 printf("pushed NEW {%lu,%lu,%lu,%lu}\n", new.index, new.parent, new.begin, new.queue_next);
 
 {puts("queue"); debug_memory(m, size, head, tail, next); debug_memory(k, size, khead, ktail, knext);}
 {puts("index"); debug_memory(m, size, head, tail, next);debug_memory(k, size, khead, ktail, knext);}
 {puts("parent"); debug_memory(m, size, head, tail, next);debug_memory(k, size, khead, ktail, knext);}

 {display_signature(context, new.index); display_string_at_char(input, new.begin);}
 {display_signature(context, this.index); display_string_at_char(input, this.begin);}



 
 
 
 
 
 
 
 
 
 
 
 so after breaking up the .queue_next into a seperate queue data strucutre,
 
 
        of which, the list (containiing the actual data for the nodes, is    looking an aweful lot like a stack now... loll
 anywways
 
        
 
 
    
                i realized that we actually have to push new to the list!
 
            and that way, we can make it ingegrated into the parent traversal system! / loop!
 
 
            and thus we minimize code duplication, and its clearer, and more correct!
 
            i love it.
 
 
                    and, the initializaation of the first node goes as follows:
    
 
 struct unit new = {
     .index = 0,                                                     //answers: where do we begin in the context string?
     .parent = head == queue_size ? list_size : queue[head].self,    //answers: what is the parent of root?
     .begin = head == queue_size ? 0 : list[queue[head].self].begin, //answers: where do we begin in the input string?
 };
 nat initial = list_next;
 list[list_next++] = new;
 
 
 
            and then the last two lines are where we push it obviously-
 
 
    
 
    
    see how there ar conditionals here! for initof parent and begin?
 
 
        those are answering the root questions which i was confused about earilier, for a bit-
 
 
    
 
        basically,        who is the parent of root?
 
 
 
 
        obviously its an erroneous question--- but we still have to answer it!
 
        so i just give a dummy value of "list_size" to make sure that we dont ACTUALLY try to derefence it lol
 
 
        or that we stop our parent train, when we reach that value.
 
 
        in fact, that value is used in the at loop!
 
 
    
        for (at = initial; at != list_size; at = list[at].parent) {
 
        }
 
 
 
    thats basically the general form.
 
 
    see how list_size shows up in this loop?
 
 
    it could also be:
 
 
 
 
 
 
 struct unit new = {
     .index = 0,
     .parent = head == queue_size ? 999999 : queue[head].self,
     .begin = head == queue_size ? 0 : list[queue[head].self].begin,
 };
 
 
                        as well as
 
 
 
    for (at = initial; at != 99999; at = list[at].parent) {
 
    }

 
 
                    and i think everything would still work.    ie, that value can be anything. doesnt have to be list_size.
 
 
 
 
                    i love this!
 
 
 
 
 
 
 
 
 

 string solve(struct unit* m, struct unit* k, nat size) {
     
     printf("NOTE: strlen(context) = %lu, strlen(input) = %lu\n", strlen(context), strlen(input));
     
     nat head = size, tail = size, next = 0;
     nat khead = size, ktail = size, knext = 0;
     
     do {
         
         {puts("queue"); debug_memory(m, size, head, tail, next); debug_memory(k, size, khead, ktail, knext);}
                         
         struct unit new = {
             .index = 0,
             .parent = head,
             .begin = head == size ? 0 : m[head].begin,
             .queue_next = size
         };
         
         assert(new.index < strlen(context) + 1 && new.parent < size + 1 && new.begin < strlen(input) + 1 && new.queue_next < size + 1);
         
         printf("queue: NEW = {%lu,%lu,%lu,%lu}\n", new.index, new.parent, new.begin, new.queue_next);
         
         nat begin_save = new.begin;
         
         for (; new.index < strlen(context); new.index++) {
             
             
             {puts("index"); debug_memory(m, size, head, tail, next);debug_memory(k, size, khead, ktail, knext);}
                              
             new.begin = begin_save;
             
             printf("index: NEW = {%lu,%lu,%lu,%lu}\n", new.index, new.parent, new.begin, new.queue_next);
             
             nat error = 0;
             
             for (; context[new.index] != '.'; new.index++) {
                 
                 printf("sig NEW = {%lu,%lu,%lu,%lu}\n", new.index, new.parent, new.begin, new.queue_next);
                 
                 {display_signature(context, new.index); display_string_at_char(input, new.begin);}
                 
                 if (error) continue;
                 else if (context[new.index] == '_') {
                     if (next < size) {
                         if (tail < size) m[tail].queue_next = next;
                         tail = next;
                         struct unit u = {
                             .index = new.index,
                             .parent = new.parent,
                             .begin = new.begin,
                             .queue_next = size,
                         };
                         printf("pushed arg {%lu,%lu,%lu,%lu}\n", u.index, u.parent, u.begin, u.queue_next);
                         assert(u.index < strlen(context) + 1 && u.parent < size + 1 && u.begin < strlen(input) + 1 && u.queue_next < size + 1);
                         m[next++] = u;
                     } else return "out-of-memory";
                     error = 1;
                 } else if (context[new.index] == input[new.begin]) new.begin++;
                 else error = 1;
             }
             
             if (error) { puts(""); continue; }
             
             if (knext < size) {
 //                if (tail < size) m[tail].queue_next = next;
 //                tail = next;
                 printf("pushed NEW {%lu,%lu,%lu,%lu}\n", new.index, new.parent, new.begin, new.queue_next);
                 assert(new.index < strlen(context) + 1 && new.parent < size + 1 && new.begin < strlen(input) + 1 && new.queue_next < size + 1);
                 k[knext++] = new;
             } else return "out-of-memory";
             
             if (new.parent < size) {
                 puts("...child is updating parent...");
                 m[new.parent].index++;
                 m[new.parent].begin = new.begin;
             } else {
                 puts("no parents!\n");
                 if (new.begin == strlen(input)) return "success";
                 else {
                     puts("new: begin didnt match. undoing push of NEW.");
                     k[--knext] = undefined;
                 }
             }
             

             

             printf("-------- parents ---------\n");

             struct unit this = new;
             
             for (nat at = head; at < size; at = m[at].parent) {
                 
                 {puts("parent"); debug_memory(m, size, head, tail, next);debug_memory(k, size, khead, ktail, knext);}
                 
                 nat error = 0;
                 
                 this = m[at];
                 
                 for (; context[this.index] != '.'; this.index++) {
                     
                     {display_signature(context, this.index); display_string_at_char(input, this.begin);}
                     
                     if (error) continue;
                     else if (context[this.index] == '_') {
                     
                         if (tail < size) m[tail].queue_next = at;
                         tail = at;
                         m[at].index = this.index;
                         m[at].begin = this.begin;
                         
                         error = 1;
                     }
                     else if (context[this.index] == input[this.begin]) this.begin++;
                     else error = 1;
                 }
                 
                 if (error) { puts("sig erroneous");  break; }
                 else if (this.parent < size) {
                     puts("moving to next parent...");
                     m[this.parent].index++;
                     m[this.parent].begin = this.begin;
                 } else if (this.begin == strlen(input)) return "success";
                 else {
                     puts("begin didnt match. undoing push of NEW.");
                     m[--next] = undefined;
                 }
             }
         }
         
 //        nat save = head;
         head = head == size ? 0 : m[head].queue_next;
 //        if (m[save].index == strlen(context)) m[save] = undefined;

         
     } while (head < size);
     return "failure";
 }

 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 void s(struct unit* m, nat size) {
     nat head = 0, tail = 0, next = 1;
     do {
         for (; m[head].index < strlen(context); m[head].index++) {
             for (nat at = head; at; at = m[at].parent) {
                 nat error = 0;
                 for (; context[m[at].index] != '.'; m[at].index++) {
                     if (error) continue;
                     else if (context[m[at].index] == '_') error = 1;
                     else if (context[m[at].index] == input[m[at].begin]) m[at].begin++;
                     else error = 1;
                 }
                 if (error) break;
                 else if (m[at].parent) {
                     m[m[at].parent].index++;
                     m[m[at].parent].begin = m[at].begin;
                 } else if (m[at].begin == strlen(input)) return;
                 else break;
             }
         }
         head = m[head].queue_next;
     } while (head);
 }
 
 
 
 */
