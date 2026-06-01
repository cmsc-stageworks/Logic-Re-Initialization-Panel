#include "board_config.h"

typedef uint8_t bool8;

#define RPN_STACK_SIZE 32

// TODO: My idea was to have just a static array, with zeros as a terminator
// I'm leaving this here to show that
#define MAX_RESTRICTIONS 8

bool8 is_char_in(char c, const char *charset) {
    size_t i = 0;
    while (charset[i]) {
        if (charset[i] == c)
            return true;
        i += 1;
    }
    return false;
}

bool8 validate_rpn_and_advance(const char *file, size_t len, size_t *pos) {
    int stack_size = 0;
    bool8 added_item = false;
    while (*pos < len && file[*pos] != ';') {
        char c = file[*pos];
        if (c <= 0x20) // Whitespace
            stack_size += 0;
        else if (is_char_in(c, "aAbBcCdDfFtT")) // Pushables
            stack_size += 1;
        else if (is_char_in(c, "!~")) // Unary operator
            stack_size += 0;
        else if (is_char_in(c, "&|^")) // Binary Operators
            stack_size -= 1;
        else
            return false;
        
        if (stack_size > 0) {
            added_item = true;
        }
        else if (added_item && stack_size < 1 || stack_size > RPN_STACK_SIZE) {
            return false;
        }
        
        *pos += 1;
    }
    
    if (*pos == len)
        return false;
    
    if (stack_size < 1 || stack_size > 2)
        return false;
    
    *pos += 1;
    return true;
}

TruthTableItem eval_rpn(bool8 a, bool8 b, bool8 c, bool8 d, const char *expr) {
    bool8 stack[RPN_STACK_SIZE]; // Stack of booleans. Ensure TRUE is 1.
    int stackp = 0;
    
    #define PUSH(item) { \
        stack[stackp] = (item); \
        stackp += 1;\
    }
    #define CASE_PUSH(lower, upper, item) \
        case (lower): \
        case (upper): { \
            PUSH(item) \
        } break; 
    #define OPERATOR(operator, operand_count, operation) \
        case operator:  { \
            stackp -= (operand_count - 1); \
            bool8 *args = &stack[stackp-1]; \
            stack[stackp-1] = operation; \
        } break;
    
    for (int i = 0;; i++) {
        char instruction = expr[i];
        bool done = false;
        
        if (instruction <= 0x20)
            continue;
        
        switch (instruction) {
            CASE_PUSH('f', 'F', 0)
            CASE_PUSH('t', 'T', 1)
            CASE_PUSH('a', 'A', a)
            CASE_PUSH('b', 'B', b)
            CASE_PUSH('c', 'C', c)
            CASE_PUSH('d', 'D', d)
            
            case '~': // Both '~' and '!' should work.
            OPERATOR('!', 1, !args[0])
            OPERATOR('&', 2, args[0] && args[1])
            OPERATOR('|', 2, args[0] || args[1])
            OPERATOR('^', 2, args[0] ^ args[1])
            
            case ';':
                done = true;
                break;
        }
        if (done)
            break;
    }
    
    
    
    if (stackp == 2 && !stack[1]) {
        return TruthTableItem::DONT_CARE;
    }
    
    return stack[0] ? TruthTableItem::TRUE : TruthTableItem::FALSE;

    #undef PUSH
    #undef CASE_PUSH
    #undef OPERATOR
}

#define CASE_BIT 0x20
// Note: Assumes `string` is all lower-case, and zero-terminated.
//       Intended for string literals.
bool8 try_parsestr_caseless(const char *string, const char *file, size_t file_len, size_t *pos) {
    int i = 0;
    while (string[i] != 0) {
        if (*pos + i >= file_len)
            return false;
        
        char c = file[*pos + i];
        if ('A' <= c && c <= 'Z')
            c |= CASE_BIT;
        
        if (string[i] != c)
            return false;
        
        i += 1;
    }
    
    *pos += i;
    return true;
}

bool8 skip_whitespace(const char *file, size_t len, size_t *pos) {
    while (true) {
        if (*pos >= len)
            return false;
        
        char c = file[*pos];
        if (c > 0x20)
            return true;
        
        *pos += 1;
    }
}

inline TruthTableItem charToTTItem(char c) {
    switch (c) {
        case 't':
        case 'T':
        case '1':
        return TruthTableItem::TRUE;
        
        case 'f':
        case 'F':
        case '0':
        return TruthTableItem::FALSE;
        
        case 'x':
        case 'X':
        return TruthTableItem::DONT_CARE;
        
        default:
        return (TruthTableItem) -1;
    }
}

int32_t parse_config(const char *file, size_t len, ProblemConfig *out) {
    size_t pos = 0;
    { // Parse used inputs line
        size_t sub_pos = pos;
        while (pos < len && file[pos] != '\n')
            pos += 1;

        out->used_inputs = 0;
        while (sub_pos < pos) {
            char c = file[sub_pos];
            char upper = c & ~CASE_BIT;
            sub_pos += 1;
            
            if (c == '-' || c == ' ')
                continue;
            else if ('A' <= upper && upper <= 'D')
                out->used_inputs |= 1 << (3 - (upper - 'A'));
            else // Unrecognized character, invalid syntax
                return sub_pos-1;
        }
    }

    if (!skip_whitespace(file, len, &pos)) return pos;
    
    // Fill truth table with don't-cares
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j <= 0xf; j++) {
            out->truth_table[i][j] = TruthTableItem::DONT_CARE;
        }
    }
    
    // TODO: Initialize restrictions to none (however that may happen)
    for (int i = 0; i < MAX_RESTRICTIONS; i++) {
        // out->restrictions[i] = 0;
    }
    
    while (pos < len) {
        if (try_parsestr_caseless("outputs:\n", file, len, &pos)) {
            if (!skip_whitespace(file, len, &pos))
                return pos;
            while (pos < len && '1' <= file[pos] && file[pos] <= '4') {
                int output_num = file[pos++] - '1';
                if (file[pos] != ':') return pos;
                pos++;
                if (file[pos] != ' ') return pos;
                pos++;
                
                if (try_parsestr_caseless("rpn\n", file, len, &pos)) {
                    skip_whitespace(file, len, &pos);
                    size_t begin = pos;
                    if (!validate_rpn_and_advance(file, len, &pos))
                        return pos;
                    
                    for (int i = 0; i <= 0xf; i++) {
                        bool8 a = (i & 0x8) >> 3;
                        bool8 b = (i & 0x4) >> 2;
                        bool8 c = (i & 0x2) >> 1;
                        bool8 d = (i & 0x1) >> 0;
                        out->truth_table[output_num][i] = eval_rpn(a, b, c, d, file + begin);
                    }
                }
                else if (try_parsestr_caseless("truth table\n", file, len, &pos)) {
                    skip_whitespace(file, len, &pos);
                    int i = 0;
                    while (pos < len && file[pos] != ';') {
                        char c = file[pos];
                        
                        if (c <= 0x20) {
                            pos += 1;
                            continue;
                        }
                        
                        if (i > 0xf)
                            return pos;
                        
                        TruthTableItem parsed = charToTTItem(c);
                        if (((int8_t) parsed) == -1)
                            return pos;
                        
                        out->truth_table[output_num][i] = parsed;
                        i += 1;
                        pos += 1;
                    }
                        
                    if (file[pos] != ';')
                        return pos;
                    
                    pos += 1;
                }
                else if (try_parsestr_caseless("list\n", file, len, &pos)) {
                    skip_whitespace(file, len, &pos);
                    while (pos < len && file[pos] != ';') {
                        uint8_t maskbits = 0;
                        uint8_t truebits = 0;
                        TruthTableItem output;
                        
                        int i = 0;
                        // TODO: Could consider adding enforced whitespace here.
                        // Currently, it just gets the next five characters in "0Tt1FfXx".
                        while (i < 5) {
                            char c = file[pos];
                            
                            if (c <= 0x20) {
                                pos += 1;
                                continue;
                            }
                            
                            TruthTableItem parsed = charToTTItem(c);
                            if (((int8_t) parsed) == -1)
                                return pos;
                            
                            if (i < 4) { // first four are inputs
                                maskbits <<= 1;
                                truebits <<= 1;
                                if (parsed == TruthTableItem::TRUE) {
                                    maskbits |= 1;
                                    truebits |= 1;
                                }
                                else if (parsed == TruthTableItem::FALSE) {
                                    maskbits |= 1;
                                    // truebits |= 0, but doesn't do anything
                                }
                                else if (parsed == TruthTableItem::DONT_CARE) {
                                    // maskbits |= 0, but doesn't do anything
                                }
                                else {
                                    // Shouldn't be possible.
                                }
                            }
                            else { // last one is output
                                output = parsed;
                            }
                            
                            i += 1;
                            pos += 1;
                        }
                                                
                        for (uint8_t inputbits = 0; inputbits <= 0xf; inputbits++) {
                            if ((inputbits & maskbits) == truebits)
                                out->truth_table[output_num][inputbits] = output;
                        }
                        
                        skip_whitespace(file, len, &pos);
                    }
                        
                    if (file[pos] != ';')
                        return pos;
                    
                    pos += 1;
                }
                else return pos;
                
                skip_whitespace(file, len, &pos);
            }
        }
        else if (try_parsestr_caseless("restrictions:\n", file, len, &pos)) {
            skip_whitespace(file, len, &pos);
            int numRestrictions = 0;
            while (pos < len && file[pos] != ';') {
                // TODO: Write the code to fill out the restrictions.
                // TODO: Ensure only one restrictions section (if integration requires it)
                if (try_parsestr_caseless("and\n", file, len, &pos)) {
                    
                } else if (try_parsestr_caseless("or\n", file, len, &pos)) {
                    
                } else if (try_parsestr_caseless("not\n", file, len, &pos)) {
                    
                } else if (try_parsestr_caseless("xor\n", file, len, &pos)) {
                    
                } else if (try_parsestr_caseless("nand\n", file, len, &pos)) {
                    
                } else {
                    return pos;
                }
            }
        }
        else return pos;
    }
    
    return -1;
}



#ifdef BOARD_CONFIG_MAIN
#include <stdio.h>
#include <stdlib.h>
void print_config(ProblemConfig *conf) {
    for (int i = 0; i < 4; i++) {
        if (conf->used_inputs & (1 << (3-i)))
            printf("%c", 'A' + i);
        else
            printf("-");
    }
    printf("\n\n");
    
    for (int i = 0; i < 4; i++) {
        printf("Output %d\n", i+1);
        printf("    CC\n");
        printf("   D D\n");
        
        for (int j = 0; j < 4; j++) {
            if (j & 2)
                printf("A");
            else
                printf(" ");
            
            if (j & 1)
                printf("B");
            else
                printf(" ");
            
            for (int k = 0; k < 4; k++) {
                TruthTableItem value = conf->truth_table[i][(j<<2) | k];
                switch (value) {
                    case TruthTableItem::TRUE:
                        printf("1"); break;
                        
                    case TruthTableItem::FALSE:
                        printf("0"); break;
                        
                    case TruthTableItem::DONT_CARE:
                        printf(" "); break;
                    
                    default:
                        printf("?"); break;
                }
            }
            printf("\n");
        }
        
        printf("\n");
    }
}

int main(int argc, char *argv[]) {
    FILE *f = fopen("test_conf.txt", "r");
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    char *file_buf = (char*) malloc(size+1);
    size_t read = fread(file_buf, 1, size, f);
    file_buf[read] = 0;
    
    ProblemConfig conf;
    int result = parse_config(file_buf, read, &conf);
    printf("Parse result: %d\n\n", result);
    print_config(&conf);
}
#endif