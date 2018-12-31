//========================================================================
// simpleVM.c
//========================================================================
// a machine code program for my simple Virtual Machine
// define basic data representation, data structures, and the VM's execution loop, culminating in a successful "Hello world!" program.
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/** OBJECT */
typedef struct OBJECT_t {
    uint8_t type;
    union {
        uint8_t u8;
        int8_t i8;
        uint32_t u32;
        int32_t i32;
        void    *ptr;
    };
} OBJECT;

/** STACK: single stack to pass values to functions calls, and to store return pointers  */
typedef struct STACK_t {
    int top;
    int size;
    OBJECT *stack;
} STACK;

// an current(avoid the change of location of instruction pointer) instruction type function pointor returns a pointer of uint8 type 
typedef uint8_t* (*instruction)(uint8_t *, STACK *);

STACK stack_new(int size){
    STACK s;
    s.top   = 0;
    s.size  = size;
    s.stack = (OBJECT *)malloc(sizeof(OBJECT) * size);
    return s;
}

int stack_push(STACK *s, OBJECT o){
    s -> stack[s -> top] = o;
    s -> top++;
    return s -> top;
}

OBJECT stack_pop(STACK *s){
    s -> top--;
    return s -> stack[s -> top];
}

OBJECT stack_peek(STACK *s){
    return s -> stack[s -> top - 1];
}

void usage(){
    printf("usage: simpleVM <file>\n");
    exit(1);
}

uint8_t *load_file(char *filename){
    FILE *f;
    int size;
    uint8_t *code = NULL;
    struct stat st;

    if((f = fopen(filename, "r"))){
        fstat(fileno(f), &st);
        code = (uint8_t *)malloc(st.st_size);
        fread((void *)code, 1, st.st_size, f);
    }
    else{
        printf("ERROR: cannot open this file %s \n");
        usage();
    }
    return code;
}

/** create first upcode : nop instruction */
// instruction nop: no operation, dont do anything
uint8_t *op_nop(uint8_t *ip, STACK *s){
    return ip + 1; // go to next instruction
}

/** OUTPUT:  take off a char pop from the stack and put out to standard output stream */
uint8_t *op_emit(uint8_t *ip, STACK *s){
    OBJECT o = stack_pop(s);
    putchar(o.u8);
    return ip + 1;
}

/** instruction to push a char to the stack */
uint8_t *op_push_char(uint8_t *ip, STACK *s){
    OBJECT o;
    o.type  = 'c'; //TYPE_CHAR
    o.u8    = *(ip + 1); //increment 1 location memory to the next instruction
    stack_push(s, o);
    return ip + 2; //scape the literal value;
}

int main (int argc, char **argv){
    uint8_t *code;
    uint8_t *ip;
    STACK data;
    // an array of function pointers, stores the 256 locations which this VM can jump to 
    instruction ops[256]; //use single bytecode for upcode, so represent all posibble upcodes by array with 256 elements, range from [0 - 255]


    if(argc != 2){
        usage();
    }

    for(int i = 0; i < 256; i++){
        ops[i] = op_nop; // initialize all upcodes in this VM are not do anything
    }
    ops['c']    = op_push_char;
    ops['e']    = op_emit;

    code    = load_file(argv[1]);
    data    = stack_new(1024);
    ip      = code;

    //HALT_INSTRUCTION, tells VM we have done any instruction, so execute it
    while(*ip != 'h'){ 
        // load the instruction which the ip points to, and call associated functions in function table ops[256]
        ip = ops[*ip](ip, &data);  // read instruction,find function pointer ops[*ip], call the current stage of this VM(&data). then use the response of the stage to update the instruction pointer ip 
    }

    /** visulize working code */

    return 0;
}

