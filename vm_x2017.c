#include <stdio.h>
#include <stdlib.h>
#include "implementer.c"
#include "vm_x2017.h"

// Inintialise all registers to 0
void initialise_registers(struct cpu* vm){
    for (int i = 0; i < NUMBER_OF_REGISTERS; i++){
        vm->registers[i] = 0;
    }
}

// Takes in a function label and returns it's index location. Since the program counter is limited
// To 8bits, an unsigned char is returned.
unsigned char find_function(struct cpu* vm, int function_label){
    for (int i = 0; i < vm->word_data->size; i++){
        if (vm->word_data->array[i]->oc == FUNC){
            if (vm->word_data->array[i]->contents1 == function_label){
                vm->registers[REG_7] = (unsigned char) i;
                return function_label;
            }
        }
    }
    // No function could be found, return 8 because only 0-7 is valid
    return (unsigned char) 8;
}

// Pushes a function onto the stack. The value at this stack address is the location of the previous
// Function in the stack so it will know where to return to once it has finished.
void push_function(struct cpu* vm, unsigned char return_address){
    // The value at the address in reg 5 is set to the value in reg 6
    vm->registers[REG_5]++;
    vm->ram[vm->registers[REG_5]] = vm->registers[REG_6];
    // The value in reg 6 is set to the value of reg5
    vm->registers[REG_6] = vm->registers[REG_5];

    vm->ram[vm->registers[REG_4]] = return_address;
    vm->registers[REG_4]--;
}

// Removes a function from the stack frame and goes to the previous function that was called.
int pop_function(struct cpu* vm){
    vm->registers[REG_5] = vm->registers[REG_6] - 1;
    vm->registers[REG_6] = vm->ram[vm->registers[REG_6]];
    vm->registers[REG_4]++;
    vm->registers[REG_7] = vm->ram[vm->registers[REG_4]];
    return 0;
}

// Takes in the local address of a stack symbol and updates it to the new value
void update_stack_symbol(struct cpu* vm, int local_address, int new_value){
    if (local_address + vm->registers[REG_6] + 1 >= vm->registers[REG_5]){
        vm->registers[REG_5]++;
    }
    vm->ram[vm->registers[REG_6] + 1 + local_address] = new_value;
}

// Takes in the local address of a stack symbol and gets its value
int get_symbol_value(struct cpu* vm, int local_address){
    return vm->ram[vm->registers[REG_6] + local_address + 1];
}

// Takes in the local address of the stack symbol and dereferences the value in it
int get_global_value(struct cpu* vm, int local_address){
    return (vm->ram[get_symbol_value(vm, local_address)]);
}

// Do all the necessary initialisation steps for the various registers and then begin code execution
void start_execution(struct cpu* vm){
    // Initialise program counter to function 0 location
    if (find_function(vm, 0) == 8){
        printf("Program entry function 0 could not be found.\n");
        exit(1);
    }

    // Set register 4 to the end of the stack. It will keep track of the instruction
    // that the program returns to after exiting from a function
    vm->registers[REG_4] = RAM_SIZE - 1;
    // Store the return address of function 0 to be 0x000
    vm->registers[REG_6] = 0;
    push_function(vm, vm->registers[REG_6]);

    // Now we do every instruction until the program ends
    while (1){
        // Test for no more memory left
        if (vm->registers[REG_5] >= vm->registers[REG_4]){
            printf("Ran out of memory!\n");
            exit(1);
        }
 
        if (perform_instruction(vm) == 0){
            break;
        }
    }
}

// Takes in a virtual machine and performs the next instruction based on its program counter
int perform_instruction(struct cpu* vm){
    // Declare the instruction data here to improve readability
    int oc = vm->word_data->array[vm->registers[REG_7]]->oc;
    int type1 = vm->word_data->array[vm->registers[REG_7]]->type1;
    int type2 = vm->word_data->array[vm->registers[REG_7]]->type2;
    int contents1 = vm->word_data->array[vm->registers[REG_7]]->contents1;
    int contents2 = vm->word_data->array[vm->registers[REG_7]]->contents2;

    // Increment program counter before executing the instruction
    vm->registers[REG_7]++;
    if (oc == MOV){
        if (type1 == STK){
            if (type2 == VAL){
                update_stack_symbol(vm, contents1, contents2);
            } else if (type2 == STK){
                update_stack_symbol(vm, contents1, get_symbol_value(vm, contents2));
            } else if (type2 == PTR){
                update_stack_symbol(vm, contents1, get_global_value(vm, contents2));
            } else if (type2 == REG){
                update_stack_symbol(vm, contents1, vm->registers[contents2]);
            }
        } else if (type1 == PTR){
            if (type2 == VAL){
                vm->ram[get_symbol_value(vm, contents1)] = contents2;
            } else if (type2 == STK){
                vm->ram[get_symbol_value(vm, contents1)] = get_symbol_value(vm, contents2);
            } else if (type2 == PTR){
                vm->ram[get_symbol_value(vm, contents1)] = vm->ram[get_symbol_value(vm, contents2)];
            } else if (type2 == REG){
                vm->ram[get_symbol_value(vm, contents1)] = vm->registers[contents2];
            }
        } else if (type1 == REG){
            if (type2 == VAL){
                vm->registers[contents1] = contents2;
            } else if (type2 == STK){
                vm->registers[contents1] = get_symbol_value(vm, contents2);
            } else if (type2 == REG){
                vm->registers[contents1] = vm->registers[contents2];
            } else if (type2 == PTR){
                vm->registers[contents1] = vm->ram[get_symbol_value(vm, contents2)];
            }
        } else {
            printf("Incorrect destination type for MOV.\n");
            exit(1);
        }
    } else if (oc == CAL){
        if (type1 != VAL){
            printf("CAL must be given a value type.\n");
            exit(1);
        }
        push_function(vm, vm->registers[REG_7]);
        if (find_function(vm, contents1) != contents1){
            printf("Function does not exist.\n");
            exit(1);
        }
        
        // We are at the function label which does not do anything, so go to first instruction
        vm->registers[REG_7]++;
    } else if (oc == RET){
        // Pop here
        pop_function(vm);
        if (vm->registers[REG_6] == 0){
            return vm->registers[REG_6];
        }
    } else if (oc == REF){
        if (type1 == VAL || type2 == VAL || type2 == REG){
            printf("REF: The first agument must be a stack address and the second argument must be a stack symbol.\n");
            exit(1);
        }
        if (type1 == STK){
            if (type2 == STK){
                update_stack_symbol(vm, contents1, vm->registers[REG_6] + contents2 + 1);
            } else if (type2 == PTR){
                update_stack_symbol(vm, contents1, get_symbol_value(vm, contents2));
            }
        } else if (type1 == PTR){
            if (type2 == STK){
                vm->ram[get_symbol_value(vm, contents1)] = vm->registers[REG_6] + 1 + contents2;
            } else if (type2 == PTR){
                vm->ram[get_symbol_value(vm, contents1)] = get_symbol_value(vm, contents2);
            }
        } else if (type1 == REG){
            if (type2 == STK){
                vm->registers[contents1] = vm->registers[REG_6] + 1 + contents2;
            } else if (type2 == PTR){
                vm->registers[contents1] = get_symbol_value(vm, contents2);
            }
        }
    } else if (oc == ADD){
        if (type1 != REG && type2 != REG){
            printf("The arguments for ADD must be of register type.\n");
            exit(1);
        }
        vm->registers[contents1] = vm->registers[contents1] + vm->registers[contents2];
    } else if (oc == PRINT){
        if (type1 == REG){
            printf("%d\n", vm->registers[contents1]);
        } else if (type1 == STK){
            printf("%d\n", get_symbol_value(vm, contents1));
        } else if (type1 == PTR){
            printf("%d\n", get_global_value(vm, contents1));
        } else if (type1 == VAL){
            printf("%d\n", contents1);
        } else {
            printf("No PRINT type given.\n");
            exit(1);
        }
    } else if (oc == NOT){
        if (type1 != REG){
            printf("The argument for NOT must be of register type.\n");
            exit(1);
        }
        vm->registers[contents1] = ~vm->registers[contents1];
    } else if (oc == EQU){
        if (type1 != REG){
            printf("The argument for EQU must be of register type.\n");
            exit(1);
        }
        if (vm->registers[contents1] == 0){
            vm->registers[contents1] = 1;
        } else {
            vm->registers[contents1] = 0;
        }
    }
    return 1;
}


int main(int argc, char** argv){

    struct data file_data = {.size = 0,.capacity = DATA_BUFFER, .array = malloc(sizeof(unsigned char)*DATA_BUFFER)};
    read_file(argc, argv, &file_data);
    
    struct words word_data = {.size = 0, .capacity = DATA_BUFFER, .array = malloc(sizeof(char*)*DATA_BUFFER)};
    parse_binary_data(&file_data, &word_data);
    rebuild_symbols(&word_data);
    reverse_instructions(&word_data);
    
    struct cpu vm = {.word_data = &word_data};
    initialise_registers(&vm);

    start_execution(&vm);

    free_data(&file_data);
    free_words(&word_data);
    return 0;
    }