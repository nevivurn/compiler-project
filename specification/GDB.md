# GNU Debugger (GDB)

The GNU debugger, `gdb`, is an excellent tool to debug programs. While it seems to be rather crude, it offers lots and lots of functions that many people are not aware of. To start debugging your program using gdb, type
```bash
  $ gdb ./primes
```
at the prompt. GDB greets you with
```
GNU gdb (Gentoo 13.2 vanilla) 13.2
Copyright (C) 2023 Free Software Foundation, Inc.
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
...

For help, type "help".
Type "apropos word" to search for commands related to "word".
(gdb)
```

From there, commands will help you run/stop/break your program and inspect/modify values. The following table contains a list of handy commands that you might use when debugging your program. For a complete list, type help and follow the instructions on the screen.

| Command             | Description |
|:--------------------|:---------------------|
| `r(un)`             | run the program until it ends, crashes, or hits a breakpoint |
| `c(ontinue)`        | continue a stopped program until it ends, crashes, or hits the next breakpoint |
| `quit`              | exit GDB |
| `break *address`    | set a breakpoint at address |
| `break name`        | set a breakpoint at name |
| `stepi`             | execute one assembly instruction |
| `stepi n`           | execute n assembly instructions |
| `disas`             | disassemble around current program counter |
| `disas name`        | disassemble at name |
| `p(rint)[/format] expression` | inspect expression |
| `display /5i $pc    | disassemble the next 5 instructions at pc at every stop |
| `display $<reg>`    | display the value of reg at every stop |
| `<Enter>`           | repeat (re-execute) last command |

Especially the display command together with stepi will be very helpful when stepping through your program. 
The [GDB documentation](https://sourceware.org/gdb/current/onlinedocs/gdb/index.html) and many [GDB cheat sheets](https://darkdust.net/files/GDB%20Cheat%20Sheet.pdf) are helpful to familiarize yourself with GDB. 

GDB supports a textual user interface (TUI) that is most helpful when debugging code. Refer to the [GDB TUI documentation](https://sourceware.org/gdb/current/onlinedocs/gdb/TUI.html#TUI) to learn more about it.
