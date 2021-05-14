#ifndef OBJDUMP_H_   /* Include guard */
#define OBJDUMP_H_

// Enum for the equivalent opcode value when converted to a number
// Treat FUNCTION LABEL as an opcode for convinience
typedef enum{
    MOV,
    CAL,
    RET,
    REF,
    ADD,
    PRINT,
    NOT,
    EQU,
    FUNC
} opcode;

// Enum for the equivalent types when they are converted to a number
typedef enum{
    VAL,
    REG,
    STK,
    PTR,
    NONE // If the argument doesnt exist
} type;

// Buffer size for the unsigned char when reading it in
#define BUFFER_SIZE 1

// Must be at least 8
#define DATA_BUFFER 100
#define STRING_SIZE 5

// String constants for all the opcodes and types
#define MOV_ "MOV"
#define CAL_ "CAL"
#define RET_ "RET"
#define REF_ "REF"
#define ADD_ "ADD"
#define PRINT_ "PRINT"
#define NOT_ "NOT"
#define EQU_ "EQU"

#define VAL_ "VAL"
#define STK_ "STK"
#define PTR_ "PTR"
#define REG_ "REG"
#define FUNCTION_LABEL "FUNC LABEL"

// Array storing the types in order of their corresponding values
const char* type_as_string[4] = {VAL_, REG_, STK_, PTR_};

// Stores the binary data from the .x2017 file
struct data {
    unsigned char* array;
    int size;
    int capacity;
};

// Stores the parsed data, where every instruction is a line in the .asm file
struct words {
    struct instruction** array;
    int size;
    int capacity;
};

// Stores the instruction. If there is 0 or 1 arguments then the rest of the values will get filled with NONE
struct instruction {
    opcode oc;
    type type1;
    int contents1;
    type type2;
    int contents2;
};

// Reads the given binary file and stores it into a file_data object. Relevant errors raised if file does not exist.
int read_file(int argc, char** argv, struct data* file_data);

// Returns a char from A-Z, and a-e basec on the value of the symbol. 
char index_to_symbol(int index);
// Frees everything in the struct and itself
void free_data(struct data* file_data);
void free_words(struct words* word_data);

// Creates an instruction object in the heap and returns a pointer to it.
struct instruction* create_instruction(opcode oc, type type1, int contents1, type type2, int contents2);
// Takes an instruction and appends it to the array in word_data
void words_append(struct instruction* instr, struct words* word_data);
// Takes a char and appends it to the array in file_data
void data_append(unsigned char c, struct data* file_data);

// Takes in a file_data, and returns the equivalent int value of the specified number of bits at the index location
int request_bits(int n, int* byte_index, int* bit_index, struct data* file_data);

// Converts the file_data data into instructions and appends it to word_data
void parse_binary_data(struct data* file_data, struct words* word_data);
// Prints out the contents of word_data in the .asm human readable format
void binary_to_human_printer(struct words* word_data);

// For each function, changes the binary value of the stack symbols to start from 00000
void rebuild_symbols(struct words* word_data);
// Used in rebuild_symbols, it returns the new integer corresponding to a stack symbol in a given function.
int change_symbol(int (*symbols)[32], int* size, int value);
// From the given file_data and indexes, updates the value of contents and type based on the requested data
void get_arg(int* type, int* contents, struct data* file_data, int* byte_index, int* bit_index);
// Reverse the array in word_data. This is done so the program counter can start from 0 and increment instead
// of starting from the end and decrementing.
void reverse_instructions(struct words* word_data);

#endif