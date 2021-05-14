#include <stdio.h>
#include <stdlib.h>
#include "implementer.c"
#include <string.h>

void append_arg(int** array, int type, int contents, int* size);

void decimal_to_binary(int x, int** arr){
    int i = 0;
    while (x > 0){
        (*arr)[i] = x % 2;
        x /= 2;
        i++;
    }
}

void array_append(int** array, struct instruction* instr, int* size, int* new_function, int* instructions){
    if (instr->oc == FUNC){
        *new_function = 0;
        int* arr = calloc(3,sizeof(int));
        decimal_to_binary(instr->contents1, &arr);
        (*array)[*size] = arr[2];
        (*array)[*size + 1] = arr[1];
        (*array)[*size + 2] = arr[0];
        free(arr);
        *size += 3;
    } else if (instr->oc == MOV){
        append_arg(array, instr->type2, instr->contents2, size);
        append_arg(array, instr->type1, instr->contents1, size);
        (*array)[*size] = 0;
        (*array)[*size + 1] = 0;
        (*array)[*size + 2] = 0;
        *size += 3;
        
    } else if (instr->oc == CAL){
        append_arg(array, instr->type1, instr->contents1, size);
        (*array)[*size] = 0;
        (*array)[*size + 1] = 0;
        (*array)[*size + 2] = 1;
        *size += 3;
    } else if (instr->oc == RET){
        (*array)[*size] = 0;
        (*array)[*size + 1] = 1;
        (*array)[*size + 2] = 0;
        *size += 3;
        int* arr = calloc(5, sizeof(int));
        decimal_to_binary(*instructions - 1, &arr);
        for (int i = 0; i < 5; i++){
            (*array)[*size + i] = arr[4-i];
        }
        free(arr);
        *size += 5;
        *new_function = 1;
        *instructions = 0;

    } else if (instr->oc == REF){
        append_arg(array, instr->type2, instr->contents2, size);
        append_arg(array, instr->type1, instr->contents1, size);
        (*array)[*size] = 0;
        (*array)[*size + 1] = 1;
        (*array)[*size + 2] = 1;
        *size += 3;
    } else if (instr->oc == ADD){
        append_arg(array, instr->type2, instr->contents2, size);
        append_arg(array, instr->type1, instr->contents1, size);
        (*array)[*size] = 1;
        (*array)[*size + 1] = 0;
        (*array)[*size + 2] = 0;
        *size += 3;
 
    } else if (instr->oc == PRINT){
        append_arg(array, instr->type1, instr->contents1, size);
        (*array)[*size] = 1;
        (*array)[*size + 1] = 0;
        (*array)[*size + 2] = 1;
        *size += 3;
        
    } else if (instr->oc == NOT){
        append_arg(array, instr->type1, instr->contents1, size);
        (*array)[*size] = 1;
        (*array)[*size + 1] = 1;
        (*array)[*size + 2] = 0;
        *size += 3;
        
    } else if (instr->oc == EQU){
        append_arg(array, instr->type1, instr->contents1, size);
        (*array)[*size] = 1;
        (*array)[*size + 1] = 1;
        (*array)[*size + 2] = 1;
        *size += 3;
    }
}

void append_arg(int** array, int type, int contents, int* size){
    int* arr1;
    if (type == STK || type == PTR){
        arr1 = calloc(5,sizeof(int));
        decimal_to_binary(contents, &arr1);
        for (int i = 0; i < 5; i++){
            (*array)[*size + i] = arr1[4-i];
        }
        free(arr1);
        *size += 5;
    } else if (type == REG){
        arr1 = calloc(5,sizeof(int));
        decimal_to_binary(contents, &arr1);
        for (int i = 0; i < 3; i++){
            (*array)[*size + i] = arr1[2-i];
        }
        free(arr1);
        *size += 3;
    } else if (type == VAL){
        arr1 = calloc(8,sizeof(int));
        decimal_to_binary(contents, &arr1);
        for (int i = 0; i < 8; i++){
            (*array)[*size + i] = arr1[7-i];
        }
        free(arr1);
        *size += 8;
    }
    arr1 = calloc(2,sizeof(int));
    decimal_to_binary(type, &arr1);
    (*array)[*size] = arr1[1];
    (*array)[*size + 1] = arr1[0];
    free(arr1);
    *size += 2;
}

int symbol_to_int(char c){
    if (c >= 65 && c <= 90){
        return c - 65;
    } else {
        return c - 71;
    }
}

void get_argument(char* type, char* contents, int* t1, int* c1){
    if (!strcmp(type, STK_)){
        *t1 = STK;
        *c1 = symbol_to_int(contents[0]);
    } else if (!strcmp(type, PTR_)){
        *t1 = PTR;
        *c1 = symbol_to_int(contents[0]);
    } else if (!strcmp(type, REG_)){
        *t1 = REG;
        sscanf(contents, "%d", c1);
    } else if (!strcmp(type, VAL_)){
        *t1 = VAL;
        sscanf(contents, "%d", c1);
    }
}

int main(int argc, char** argv){
    if (argc != 2){
        perror("Incorrect number of arguments, use the following format: ./ascii_to_binary <x2017_ascii.txt>\n");
        return -1;
    }

    FILE* file = fopen(argv[1], "r");

    if (file == NULL){
        perror("Unable to open file.");
        return -1;
    }

    struct words word_data = {.size = 0, .capacity = 100, .array = malloc(sizeof(char*)*DATA_BUFFER)};

    char buffer[100];
    char opcode[10], type1[4], contents1[30], type2[4], contents2[30];
    int oc, t1, c1, t2, c2;

    int function_start = 1;
    while (fgets(buffer, 100, file) != NULL){
        if (function_start){
            sscanf(buffer, "%s %s %d",contents1,contents1, &c1);
            words_append(create_instruction(FUNC, NONE, c1, NONE, NONE), &word_data);
            function_start = 0;
        } else {
            sscanf(buffer, "%s", opcode);
            if (!strcmp(opcode, MOV_)){
                oc = MOV;
                sscanf(buffer, "%s %s %s %s %s", opcode, type1, contents1, type2, contents2);
                get_argument(type1, contents1, &t1, &c1);
                get_argument(type2, contents2, &t2, &c2);
                words_append(create_instruction(oc,t1,c1,t2,c2), &word_data);
            } else if (!strcmp(opcode, REF_)){
                 oc = REF;
                sscanf(buffer, "%s %s %s %s %s", opcode, type1, contents1, type2, contents2);
                get_argument(type1, contents1, &t1, &c1);
                get_argument(type2, contents2, &t2, &c2);
                words_append(create_instruction(oc,t1,c1,t2,c2), &word_data);
            } else if (!strcmp(opcode, ADD_)){
                 oc = ADD;
                sscanf(buffer, "%s %s %s %s %s", opcode, type1, contents1, type2, contents2);
                get_argument(type1, contents1, &t1, &c1);
                get_argument(type2, contents2, &t2, &c2);
                words_append(create_instruction(oc,t1,c1,t2,c2), &word_data);
            } else if (!strcmp(opcode, CAL_)){
                sscanf(buffer, "%s %s %s", opcode, type1, contents1);
                get_argument(type1, contents1, &t1, &c1);
                words_append(create_instruction(CAL, t1, c1, NONE, NONE), &word_data);
            } else if (!strcmp(opcode, RET_)){
                function_start = 1;
                words_append(create_instruction(RET, NONE, NONE, NONE, NONE), &word_data);
            } else if (!strcmp(opcode,PRINT_)){
                sscanf(buffer, "%s %s %s", opcode, type1, contents1);
                get_argument(type1, contents1, &t1, &c1);
                words_append(create_instruction(PRINT, t1, c1, NONE, NONE), &word_data);
            } else if (!strcmp(opcode,NOT_)){
                sscanf(buffer, "%s %s %s", opcode, type1, contents1);
                get_argument(type1, contents1, &t1, &c1);
                words_append(create_instruction(NOT, t1, c1, NONE, NONE), &word_data);
            } else if (!strcmp(opcode,EQU_)){
                sscanf(buffer, "%s %s %s", opcode, type1, contents1);
                get_argument(type1, contents1, &t1, &c1);
                words_append(create_instruction(EQU, t1, c1, NONE, NONE), &word_data);
            }
        }
    }

    fclose(file);

    int capacity = 1000;
    int* array = (int*) malloc(sizeof(int)*capacity);
    int size = 0;
    int instructions = 0;
    function_start = 1;

    for (int i = 0; i < word_data.size; i++){
        if (size > capacity - 200){
            capacity *= 2;
            array = realloc(array ,sizeof(int)* capacity);
            if (array == NULL){
                perror("Realloc failed\n");
                return 0;
            }
        }
        instructions++;
        array_append(&array, word_data.array[i], &size, &function_start, &instructions);
    }

    int padding = 8 - (size % 8);
    if (padding == 8){
        padding = 0;
    }
    int* final = calloc(padding + size, sizeof(int));
    for (int i = 0; i < size; i++){
        final[padding + i] = array[i];
    }

    size += padding;

    char output_file[50];
    int x = 0;
    for (; argv[1][x] != '.'; x++){
        output_file[x] = argv[1][x]; 
    } 
    output_file[x] = '\0';
    strcat(output_file, ".x2017");

    FILE* filewrite = fopen(output_file, "wb");
    if (filewrite == NULL){
        perror("Error opening file");
        return 0;
    }


    for (int i = 0; i < size / 8; i++){
        unsigned char byte = 0;
        for (int j = 0; j < 8; j++){
            byte += final[i*8 + j] * pow(2, 7-j);
        }
        fwrite(&byte, 1, 1, filewrite);
     
    }

    printf("Binary file %s has been created!\n", output_file);

    fclose(filewrite);
    free_words(&word_data);
    free(array);
    free(final);
    return 0;
}