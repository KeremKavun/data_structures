Contributing to Data Structures in C 🚀

First off, thank you for considering contributing! This project aims to provide high-quality, educational, and modular C implementations.
I used Gemini for architectural discussions and API design and Claude for generating edge-case unit tests, so feel free to use any AI tools,
but ensure all code adheres to our Manual Review standards below.

🛠️ How to Contribute

1. Reporting Bugs

Check the Issues tab to see if the bug has already been reported.

If not, open a new issue. Please include:

    The specific data structure involved.

    A small code snippet to reproduce the crash/error.

    Your compiler (gcc/clang) and OS.

2. Suggesting Enhancements

We are currently prioritizing:

    Minimalism: Reducing code bloat and maximizing code reuse.

    New Structures: Skip Lists, Tries, Hash Sets, or advanced Graph implementations.

    Safety: More rigorous pointer and bounds checking.

    Performance: Algorithmic optimizations for existing modules.

3. Submission Process

    1. Fork the repository.

    2. Create a branch for your feature (git checkout -b feature/AmazingFeature).

    3. Your code must pass all existing tests (and any new tests you've added) before a PR is accepted.

    4. Commit your changes (git commit -m 'Add some AmazingFeature').

    5. Push to the branch (git push origin feature/AmazingFeature).

    6. Open a Pull Request.

📏 Coding Standards

To maintain consistent codebase, please adhere to these C-specific rules:

Memory Management

    Zero Leak Policy: All implementations must be memory-safe.

    Stack-Allocatable: Provide <ds>_init and <ds>_deinit functions.

    Opaque Types: Provide <ds>_create and <ds>_destroy functions.

Architecture & Style

    Modularity: Keep logic in .c files and definitions in .h. No global variables.

    Documentation: We use Doxygen. Every public function must follow Doxygen style.

    Inlining: Use static inline in headers only for 1-line logic (excluding assertions).

    Naming: Use snake_case for all functions, variables, and files, name of functions that implements a struct must
    be prefixed by the struct's name followed by a dash, for namespacing.

    Consistency: Functions in a module must complement each other.

        Example: If a Singly Linked List module uses pointer-to-pointer for insertion, the search function should return a pointer-to-pointer.

    Reuse: Always check include/ to see if an existing utility or algorithm can be used before writing a new one.

📧 Questions?

If you're unsure where to start, reach me at 209keremkavun@gmail.com.