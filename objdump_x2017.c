// Disassembler for x2017 binaries
#include "implementer.c"

int main(int argc, char** argv){
    // Initialise file data object
    struct data file_data = {.size = 0,.capacity = DATA_BUFFER, .array = malloc(sizeof(unsigned char)*DATA_BUFFER)};
    // Read the file
    read_file(argc, argv, &file_data);

    // Initialise the word_data object
    struct words word_data = {.size = 0, .capacity = DATA_BUFFER, .array = malloc(sizeof(char*)*DATA_BUFFER)};
    parse_binary_data(&file_data, &word_data);
    rebuild_symbols(&word_data);
    
    // Print out the instructions in human readable format
    binary_to_human_printer(&word_data);
    free_data(&file_data);
    free_words(&word_data);
    return 0;
}