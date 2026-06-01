#ifndef __BOARD_CONFIG_H__
#define __BOARD_CONFIG_H__
#include <stdint.h>
#include <cstddef>


enum class TruthTableItem : int8_t {
    FALSE,
    TRUE,
    DONT_CARE,
};

struct ProblemConfig {
    // Bit mask for which inputs are used
    // Takes the form 0bABCD
    // These could be switched to use 0bDCBA without too much difficulty, if desired.
    uint8_t used_inputs;
    // This is indexed by output number, then by the inputs in 0bABCD form.
    TruthTableItem truth_table[4][16]; 
    // TODO: Add restrictions data, if desired.
};

// Takes a pointer to the loaded file in memory.
//   - We should probably test that it handles carriage returns (0xD)
//     On Windows, at least, it automatically removes them when loading using `fread`,
//     so it hasn't been thoroughly tested.
//   - It's just the full file in memory
// Also takes the read length of the file.
//   - BE CAREFUL. On Windows, because it automatically removes carriage returns,
//     the apparent file size when measured with `fseek` was actually longer than what was read.
//     In the main function in the file, I only use `fseek` to get the allocation size.
//     The "length" I pass is what is returned from `fread`
// `out` is just a pointer to a ProblemConfig struct.
//   - This function will initialize it.
// Returns the position in the file where there's an error, or -1 if no error.
//   - Previously returned 0 if no error, but that was indistinguishable from error on first char.
//   - Alternatively, we could have it output the position+1 (so the first character is 1).
int32_t parse_config(const char *file, size_t len, ProblemConfig *out);

#endif