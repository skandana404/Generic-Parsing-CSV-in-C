#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>              

#define MAX_LINE_LEN 4096         

typedef struct {
    char* key;
    char* value;
} CSVPair;  


typedef struct {
    CSVPair* pairs;
    size_t size;
} CSVRow; 

typedef struct {
    CSVRow* rows;
    size_t size;
} CSVData; 

// Trim whitespace at the starting and end of a string

char* trim(char* str) {
    while (isspace((unsigned char)*str)) str++;  
    if (*str == 0) return str;                  
    char* end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    *(end + 1) = '\0';                
    return str;
}

// Function to parse a CSV line and return an array of tokens

char** ParseCSVLine(FILE* fp, char delimiter, char text_qualifier, int* out_count) {     
    char* line = malloc(MAX_LINE_LEN);
    char** tokens = NULL;
    size_t size = 0;
    char* token = malloc(MAX_LINE_LEN);
    size_t token_len = 0;
    int inside_quotes = 0;
    *out_count = 0;      

    while (fgets(line, MAX_LINE_LEN, fp)) {
        size_t len = strlen(line);
        for (size_t i = 0; i < len; i++) {
            char current = line[i];
            if (current == text_qualifier) {
                if (inside_quotes && i + 1 < len && line[i + 1] == text_qualifier) {  
                    token[token_len++] = text_qualifier;
                    i++;
                }
                else {
                    inside_quotes = !inside_quotes;
                }
            }
            else if (current == delimiter && !inside_quotes) { 
                token[token_len] = '\0';
                tokens = realloc(tokens, sizeof(char*) * (size + 1));
                tokens[size++] = _strdup(trim(token));  
                token_len = 0;
            }
            else {
                token[token_len++] = current;
            }
        }
        if (!inside_quotes) break;    
        if (token_len > 0 && token[token_len - 1] != ' ' && token[token_len - 1] != '\n') { 
            token[token_len++] = ' ';
        }


    }

    token[token_len] = '\0';        
    tokens = realloc(tokens, sizeof(char*) * (size + 1));   
    tokens[size++] = _strdup(trim(token));

    free(token);
    free(line);
    *out_count = (int)size;
    return tokens;

}

// Function read the header and values rows
CSVData parse_csv(const char* filepath, char delimiter, char text_qualifier) {
    FILE* file = NULL;
    CSVData data;
    data.rows = NULL;
    data.size = 0;

    if (fopen_s(&file, filepath, "r") != 0 || !file) {
        fprintf(stderr, "Error: Cannot open file '%s'.It might be open in Excel(close it if open and execute the code again).\n", filepath);
        return data;
    }

    int header_count = 0;
    char** headers = NULL;       
    while (!feof(file)) {
        int is_valid = 0;
        headers = ParseCSVLine(file, delimiter, text_qualifier, &header_count);

        
        for (int i = 0; i < header_count; i++) { 
            if (headers[i]) {
                if (strlen(headers[i]) > 0) {
                    is_valid = 1;
				}
				else {
					free(headers[i]);
					headers[i] = NULL; 
            }
        }
		}
        if (!is_valid)continue;  

        
        for (int i = 0; i < header_count; i++) {
            if (!headers[i] || strlen(headers[i]) == 0) {
                printf("Warning: Missing header at column %d\n", i + 1);
            }
        }

        // Found the header line break the loop
        break;
    }

	// read Values

    while (!feof(file)) {
        int value_count = 0;
        char** values = ParseCSVLine(file, delimiter, text_qualifier, &value_count);

        CSVRow row;
        row.size = header_count;
        row.pairs = (CSVPair*)malloc(sizeof(CSVPair) * row.size);

        for (int i = 0; i < header_count; i++) {
            row.pairs[i].key = NULL;  
            row.pairs[i].value = NULL;  

            row.pairs[i].key = _strdup(headers[i]);
            if (i < value_count && values[i] != NULL) {
                row.pairs[i].value = _strdup(values[i]);
            }
            else {
               row.pairs[i].value = _strdup(""); 
            }
        }

        data.rows = (CSVRow*)realloc(data.rows, sizeof(CSVRow) * (data.size + 1)); 
        data.rows[data.size] = row;  
        data.size = data.size + 1;


    }
    return data;
}


int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <filepath> [delimiter] [text_qualifier]\n", argv[0]);
        return 1;
    }

    char delimiter = (argc >= 3) ? argv[2][0] : ',';
    char text_qualifier = (argc >= 4) ? argv[3][0] : '"';

    CSVData csv = parse_csv(argv[1], delimiter, text_qualifier); 
    for (size_t i = 0; i < csv.size; i++) {
        for (size_t j = 0; j < csv.rows[i].size; j++) {
            printf("%s: %s  ", csv.rows[i].pairs[j].key, csv.rows[i].pairs[j].value);
        }
        printf("\n");
    }    
    

    return 0;
}