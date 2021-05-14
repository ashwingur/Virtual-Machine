#ifndef VM
#define VM

// 2^8 RAM space, and 8 registers
#define RAM_SIZE 256
#define NUMBER_OF_REGISTERS 8

// 8 Registers
typedef enum {
    REG_0,
    REG_1,
    REG_2,
    REG_3,
    REG_4,// Stores the address of the last called function's return address (Stored the end of the RAM)
    REG_5,// Stores pointer to return address of previous stack frame variables
    REG_6,// Stores the pointer to last called function address of previous function
    REG_7 // Stores the program counter 
} register_type;

// This stores the state of a virtual machine
struct cpu{
    // Unsigned char because it is 1 byte
    unsigned char ram[RAM_SIZE];
    unsigned char registers[NUMBER_OF_REGISTERS];

    // This is where the parsed instructions are stored
    struct words* word_data;
};

// Initialise registers to 0
void initialise_registers(struct cpu* vm);
// Based on the program counter, perform the next instruction on the VM
int perform_instruction(struct cpu* vm);
// Takes in a function label and returns it's index location in terms of instruction number.
// Returns 8 if function does not exist (Since 0-7 are valid functions)
unsigned char find_function(struct cpu* vm, int function_label);
// Pushes a function onto the stack, and at this new stack address store the address to the previous function
void push_function(struct cpu* vm, unsigned char return_address);
// Go back to the previous function that was called (The current function's data is not legally accessible)
int pop_function(struct cpu* vm);
// Updates the value of a stack symbol in the current stack frame
void update_stack_symbol(struct cpu* vm, int local_address, int new_value);
// Gets the value of a stack symbol in the current stack frame
int get_symbol_value(struct cpu* vm, int local_address);
// Performs a dereference on a stack symbol and gets the value at the address stored in the stack symbol
int get_global_value(struct cpu* vm, int local_address);
// Starts the virtual machine execution
void start_execution(struct cpu* vm);


#endif