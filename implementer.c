#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "implementer.h"

int read_file(int argc, char** argv, struct data* file_data){
    // Invalid number of arguments
    if (argc != 2){
        printf("Incorrect number of arguments. This program takes in a single .x2017 file.\n");
        exit(1);
    }

    FILE* file = fopen(argv[1], "r");
    // File not found
    if (file == NULL){
        printf("Unable to open file.\n");
        exit(1);
    }

    // Assign a buffer to collect the data
    unsigned char buffer[BUFFER_SIZE];

    // Read in one byte at a time
    while (fread(buffer, sizeof(unsigned char),1,file)){
        // Append the char that was just read in, into file_data
        data_append(buffer[0], file_data);
    }
    fclose(file);
    return 0;
}

char index_to_symbol(int index){
    // Conversion to ascii
    if (index >= 0 && index <= 25){
        // index is between A - Z
        return index + 65;
    } else {
        // index is between a - f
        return index + 71;
    }

}

void free_data(struct data* file_data){
    free(file_data->array);
    file_data->array = NULL;
    file_data = NULL;
}

void free_words(struct words* word_data){
    // Need to free every instruction inside word_data because it points to a location on the heap
    for (int i = 0; i < word_data->size; i++){
       free(word_data->array[i]);
    }
    // Now free the array itself
    free(word_data->array);
    word_data->array = NULL;
    word_data = NULL;

}

struct instruction* create_instruction(opcode oc, type type1, int contents1, type type2, int contents2){
    // Create the struct on the heap
    struct instruction* ptr = malloc(sizeof(struct instruction));
    ptr->oc = oc;
    ptr->type1 = type1;
    ptr->type2 = type2;
    ptr->contents1 = contents1;
    ptr->contents2 = contents2;
    return ptr;
}

void words_append(struct instruction* instr, struct words* word_data){
    if (word_data->size == word_data->capacity){
        // Reallocate more memory if the instruction size was reached
        word_data->array = realloc(word_data->array, word_data->capacity * 2);
        if (word_data == NULL){
            printf("Not enough memory.\n");
            exit(1);
        }
        word_data->capacity *= 2;
    }

    word_data->array[word_data->size] = instr;
    word_data->size++;
}


void data_append(unsigned char c, struct data* file_data){
    if (file_data->size >= file_data->capacity){
        // Reallocate more memory if the instruction size was reached
        file_data->array = realloc(file_data->array, file_data->capacity * 2 * sizeof(unsigned char));
        if (file_data == NULL){
            printf("Not enough memory.\n");
            exit(1);
        }
        file_data->capacity *= 2;
    }

    file_data->array[file_data->size] = c;
    file_data->size++;

}


// Request n amount of bits from binary stream, and return it's integer equivalent
int request_bits(int n, int* byte_index, int* bit_index, struct data* file_data){
    int value = 0;
    // Bit index tells us how many bits from this byte has already been taken.
    if (n <= 8 - *bit_index){
        // We can just take the right-most n bits
        value = file_data->array[*byte_index] & ((1 << n) - 1);

        *bit_index += n;
        if (*bit_index == 8){
            // We are done with the current byte, go to the next byte (going backwards)
            *byte_index -=1;
            *bit_index = 0;
        } else {
            // Shift everything to the right by n bits so the rest of the bits in the byte can be read more easily
            file_data->array[*byte_index] = file_data->array[*byte_index] >> n;
        }
    } else {
        // Case where the requested number of bits overlaps 2 bytes
        // We have to take 8 - bit_index bits from this byte and then n-bit_index bits from next byte
        // Note that there can never be a request for more than 8 bits because of the system architecture
        value = file_data->array[*byte_index] & ((1 << (8 - *bit_index)) - 1);
        *byte_index -= 1;

        // Bits left to read from the next byte
        int bits_remaining = n - (8 - *bit_index);
        *bit_index = 0;
        for (int i = 0; i < bits_remaining; i++){
            // Converting the binary to int
            value += (file_data->array[*byte_index] & 1) * pow(2, n - bits_remaining + i);
            file_data->array[*byte_index] = file_data->array[*byte_index] >> 1;
            (*bit_index)++;
        }

    }
    return value;
}

void parse_binary_data(struct data* file_data, struct words* word_data){
    int byte_index = file_data->size - 1;
    int bit_index = 0;
    // Since we do not know the amount of padding, we will start parsing from the end of the data.
    while (byte_index > 0){
        // We are in a new function, get the amount of instructions it has.
        int instructions_left = request_bits(5, &byte_index, &bit_index, file_data);
        for (int i = 0; i < instructions_left; i++){
            // We are at a new instruction, get the opcode
            int opcode = request_bits(3, &byte_index, &bit_index, file_data);
            // Declaring the arguments
            int contents1;
            int contents2;
            int type1;
            int type2;
            // Depending on the opcode, gets 0,1 or 2 arguments and then appen it to word_data
            if (opcode == MOV){
                get_arg(&type1, &contents1, file_data, &byte_index, &bit_index);
                get_arg(&type2, &contents2, file_data, &byte_index, &bit_index);
                words_append(create_instruction(MOV, type1, contents1, type2, contents2), word_data);
            } else if (opcode == CAL){
                get_arg(&type1, &contents1, file_data, &byte_index, &bit_index);
                words_append(create_instruction(CAL, type1, contents1, type2, contents2), word_data);
            } else if (opcode == RET){
                words_append(create_instruction(RET, NONE, NONE, NONE, NONE) , word_data);
            } else if (opcode == REF){
                get_arg(&type1, &contents1, file_data, &byte_index, &bit_index);
                get_arg(&type2, &contents2, file_data, &byte_index, &bit_index);
                words_append(create_instruction(REF, type1, contents1, type2, contents2), word_data);
            } else if (opcode == ADD){
                get_arg(&type1, &contents1, file_data, &byte_index, &bit_index);
                get_arg(&type2, &contents2, file_data, &byte_index, &bit_index);
                words_append(create_instruction(ADD, type1, contents1, type2, contents2), word_data);
            } else if (opcode == PRINT){
                get_arg(&type1, &contents1, file_data, &byte_index, &bit_index);
                words_append(create_instruction(PRINT, type1, contents1, NONE, NONE), word_data);
            } else if (opcode == NOT){
                get_arg(&type1, &contents1, file_data, &byte_index, &bit_index);
                words_append(create_instruction(NOT, type1, contents1, type2, contents2), word_data);
            } else if (opcode == EQU){
                get_arg(&type1, &contents1, file_data, &byte_index, &bit_index);
                words_append(create_instruction(EQU, type1, contents1, type2, contents2), word_data);
            }
        }
        // The instructions in the function have been parsed and now function label has been reached
        int contents1 = request_bits(3, &byte_index, &bit_index, file_data);
        words_append(create_instruction(FUNC, NONE, contents1, NONE, NONE), word_data);
    }    
}

void get_arg(int* type, int* contents, struct data* file_data, int* byte_index, int* bit_index){
    // Request 2 bits to get the type
    int arg = request_bits(2, byte_index, bit_index, file_data);
    // Based on the type, get the relevant amount of bits for the argument value and update contents and type
    if (arg == STK){
        *type = STK;
        *contents = request_bits(5, byte_index, bit_index, file_data);
    } else if (arg == REG){
        *type = REG;
        *contents = request_bits(3, byte_index, bit_index, file_data);
    } else if (arg == PTR){
        *type = PTR;
        *contents = request_bits(5, byte_index, bit_index, file_data);
    } else if (arg == VAL){
        *type = VAL;
        *contents = request_bits(8, byte_index, bit_index, file_data);
    } else{
        printf("The given type does not exist.\n");
        exit(1);
    }
}

void binary_to_human_printer(struct words* word_data){
    // Loop backwards
    int i = word_data->size - 1;
    while (i >= 0){
        // Based on the opcode, print the relevant instruction data in the .asm format
        switch (word_data->array[i]->oc)
        {
        case FUNC:
             printf("%s %d\n", FUNCTION_LABEL, word_data->array[i]->contents1);
             break;
        case MOV:
            printf("    %s %s ", MOV_, type_as_string[word_data->array[i]->type1]);
            if (word_data->array[i]->type1 == STK || word_data->array[i]->type1 == PTR){
                printf("%c ", index_to_symbol(word_data->array[i]->contents1));
            } else {
                printf("%d ", word_data->array[i]->contents1);
            }
            printf("%s ", type_as_string[word_data->array[i]->type2]);
            if (word_data->array[i]->type2 == STK || word_data->array[i]->type2 == PTR){
                printf("%c\n", index_to_symbol(word_data->array[i]->contents2));
            } else {
                printf("%d\n", word_data->array[i]->contents2);
            }
            break;
        case CAL:
            printf("    %s %s %d\n", CAL_, type_as_string[word_data->array[i]->type1], word_data->array[i]->contents1);
            break;
        case RET:
            printf("    %s\n", RET_);
            break;
        case REF:
            printf("    %s %s ", REF_, type_as_string[word_data->array[i]->type1]);
            if (word_data->array[i]->type1 == STK || word_data->array[i]->type1 == PTR){
                printf("%c ", index_to_symbol(word_data->array[i]->contents1));
            } else {
                printf("%d ", word_data->array[i]->contents1);
            }
            printf("%s ", type_as_string[word_data->array[i]->type2]);
            if (word_data->array[i]->type2 == STK || word_data->array[i]->type2 == PTR){
                printf("%c\n", index_to_symbol(word_data->array[i]->contents2));
            } else {
                printf("%d\n", word_data->array[i]->contents2);
            }
            break;
        case ADD:
            printf("    %s %s %d %s %d\n", ADD_, REG_, word_data->array[i]->contents1, REG_, word_data->array[i]->contents2);
            break;
        case PRINT:
            printf("    %s %s ", PRINT_, type_as_string[word_data->array[i]->type1]);
            if (word_data->array[i]->type1 == STK || word_data->array[i]->type1 == PTR){
                printf("%c\n", index_to_symbol(word_data->array[i]->contents1));
            } else {
                printf("%d\n", word_data->array[i]->contents1);
            }
            break;
        case NOT:
            printf("    %s REG %d\n", NOT_, word_data->array[i]->contents1);
            break;
        case EQU:
            printf("    %s REG %d\n", EQU_, word_data->array[i]->contents1);
            break;
        }
        i--;      
    }
}

void rebuild_symbols(struct words* word_data){
    int symbols[32];
    int size = 0;
    // Loop backwards
    for (int i = word_data->size - 1; i >= 0; i--){
        if (word_data->array[i]->oc == FUNC){
            // At a new function so 'reset' the symbols array by setting size to 0 again
            size = 0;
        } else {
            // If there is a symbol in the instruction, then update its value to follow the A-Z, a-e specification
            if (word_data->array[i]->type1 == STK || word_data->array[i]->type1 == PTR){
                word_data->array[i]->contents1 = change_symbol(&symbols, &size, word_data->array[i]->contents1);
            }
            if (word_data->array[i]->type2 == STK || word_data->array[i]->type2 == PTR){
                word_data->array[i]->contents2 = change_symbol(&symbols, &size, word_data->array[i]->contents2);
            }
        }
    }
}

int change_symbol(int (*symbols)[32], int* size, int value){
    // If the symbol already exists in the function, return its new value which is the index in which it appears
    for (int i = 0; i < *size; i++){
        if ((*symbols)[i] == value){
            return i;
        }
    }
    // Otherwise add it to the symbols array and then return it
    (*symbols)[*size] = value;
    *size += 1;
    return *size - 1;
}

void reverse_instructions(struct words* word_data){
    // Reverses the array by swapping elements on opposite sizes
    for (int i = 0; i < word_data->size / 2; i++){
        struct instruction* temp = word_data->array[i];
        word_data->array[i] = word_data->array[word_data->size - i - 1];
        word_data->array[word_data->size - i - 1] = temp;
    }
}