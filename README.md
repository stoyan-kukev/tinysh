# tinysh

tinysh is a minimal, POSIX-like Unix shell written from scratch in C.
It was built as an exercise in systems programming, memory management, and language parsing.
tinysh implements its own language front-end and execution engine to interact with the Linux kernel.

## Features

- Supports standard Unix plumbing (`|`, `<`, `>`, `>>`)
- Plus new routing based on combined streams (`!>`, `!>>`, `&>`, `&>>`, `&|`), where `!` is prefix for stderr and `&` is prefix for both stderr and stdout
- Utilizes a recursive descent parser as to correctly handle logical chaining (`||`, `&&`) and sequential/background execution (`&`, `;`)
- AST construction utilizes a custom Arena allocator to prevent memory leaks during REPL (Read-Eval-Print) loops.

## Building and Running
```bash
mkdir build && cd build
cmake ..
make
./tinysh
```

## Possible Future Development
- Configuration & Aliasing: Parsing a .tinyshrc file on startup to support aliasing (e.g. mapping `ls` to `ls --color=auto`)
- Environment Variables: Expanding `$VAR` syntax and implementing the `export` built-in command
- Line Editing & History: Integrating a library like `linenoise` or `readline` for command history and tab-completion
- Job Control: Implementing full process group management for `fg`, `bg`, and `jobs`
