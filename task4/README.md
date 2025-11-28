# Unix Commands Implementation in C

A collection of basic Unix commands implemented in C programming language.

## Project Structure

```
unix-commands-c/
├── Makefile
├── echo.c
├── pwd.c
├── ls.c
├── mv.c
├── sort.c
├── cat.c
└── create_test_files.sh
```

## Commands Documentation

### 1. echo

**File:** `echo.c`

**Purpose:** Display a line of text

**Implementation:**
```c
#include <stdio.h>

int main(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        printf("%s", argv[i]);
        if (i < argc - 1) {
            printf(" ");
        }
    }
    printf("\n");
    return 0;
}
```

**Input:** Command line arguments
**Output:** Arguments printed to stdout with spaces and newline
**Return:** 0 on success

**Usage:**
```bash
./echo Hello World
./echo "Multiple words"
```

---

### 2. pwd

**File:** `pwd.c`

**Purpose:** Print working directory

**Implementation:**
```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

int main() {
    char *cwd = getcwd(NULL, 0);
    if (cwd == NULL) {
        fprintf(stderr, "Error getting current directory\n");
        return 1;
    }
    printf("%s\n", cwd);
    free(cwd);
    return 0;
}
```

**Input:** None
**Output:** Absolute path of current directory
**Return:** 0 on success, 1 on error

**Usage:**
```bash
./pwd
```

---

### 3. ls

**File:** `ls.c`

**Purpose:** List directory contents

**Key Functions:**

- `get_permissions()` - Convert file mode to permission string
- `format_time()` - Format timestamp for display
- `print_long_format()` - Display file details
- `compare_by_permissions()` - Sorting function for -l flag
- `collect_files()` - Read directory contents
- `print_files()` - Output files in specified format
- `list_directory()` - Handle recursive directory traversal

**Flags:**
- `-l`: Long format with detailed information
- `-g`: Group format display
- `-R`: Recursive directory traversal

**Usage:**
```bash
./ls
./ls -l
./ls -R /path
./ls -l -R
```

---

### 4. mv

**File:** `mv.c`

**Purpose:** Move or rename files

**Implementation Logic:**
1. Check if destination is existing directory
2. If yes: move source inside directory keeping original name
3. If no: rename/move to specified path
4. Use `rename()` system call for the operation

**Usage:**
```bash
./mv oldname newname
./mv file.txt directory/
./mv dir1 dir2
```

---

### 5. sort

**File:** `sort.c`

**Purpose:** Sort lines of text

**Data Structure:**
```c
typedef struct {
    int reverse;
    int case_insensitive;
    int numeric;
    int skip_lines;
} sort_flags;
```

**Key Functions:**

- `read_lines()` - Read input into array of strings
- `compare_strings()` - String comparison with flags
- `compare_numbers()` - Numeric comparison for -n flag

**Flags:**
- `-r`: Reverse sort order
- `-f`: Case-insensitive sorting
- `-n`: Numeric sort
- `+n`: Skip first n lines

**Usage:**
```bash
./sort file.txt
./sort -r file.txt
./sort -n numbers.txt
./sort +2 file.txt
```

---

### 6. cat

**File:** `cat.c`

**Purpose:** Concatenate and display files

**Key Functions:**

- `print_file()` - Output file content with optional numbering
- Uses pointer to line counter for continuous numbering across files

**Flags:**
- `-n`: Number all output lines

**Usage:**
```bash
./cat file.txt
./cat -n file1.txt file2.txt
echo "text" | ./cat -n
```

---

## Compilation

### Individual Compilation:
```bash
gcc -o echo echo.c
gcc -o pwd pwd.c
gcc -o ls ls.c
gcc -o mv mv.c
gcc -o sort sort.c
gcc -o cat cat.c
```

### Using Makefile:
```bash
make        # Compile all commands
make clean  # Remove compiled binaries
```

---

## Testing

Create test files:
```bash
chmod +x create_test_files.sh
./create_test_files.sh
```

Run tests:
```bash
./echo "Test Message"
./pwd
./ls -l
./cat -n file1.txt
./sort -r file1.txt
./mv file1.txt renamed.txt
```

---

## Features

- **Modular Design**: Each command is a separate program
- **Error Handling**: Comprehensive error checking and reporting
- **Memory Management**: Proper allocation and freeing of resources
- **Standard Compliance**: Follows Unix command behavior standards
- **Flag Support**: Implements common command-line flags

## Technical Details

- **Language**: C99 standard
- **System Calls**: Uses POSIX functions for file operations
- **Libraries**: Standard C library + system-specific headers
- **Compatibility**: Unix-like systems (Linux, macOS, BSD)

This implementation demonstrates core systems programming concepts including file I/O, directory operations, memory management, and command-line argument processing.
