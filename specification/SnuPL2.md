# SnuPL/2 Language Specification

[[_TOC_]]

## EBNF
    module            = "module" ident ";"
                        { constDeclaration | varDeclaration | subroutineDecl }
                        [ "begin" statSequence ] "end" ident ".".
    
    letter            = "A".."Z" | "a".."z" | "_".
    digit             = "0".."9".
    hexdigit          = digit | "A".."F" | "a".."f".
    character         = LATIN1 char | "\n" | "\t" | "\"" | "\'" | "\\" | hexencoded.
    hexedcoded        = "\x" hexdigit hexdigit.
    char              = "'" character  | "\0" "'".
    string            = '"' { character } '"'.
    
    ident             = letter { letter | digit }.
    number            = digit { digit } [ "L" ].
    boolean           = "true" | "false".
    type              = basetype | type "[" [ simpleexpr ] "]".
    basetype          = "boolean" | "char" | "integer" | "longint".
    
    qualident         = ident { "[" simpleexpr "]" }.
    factOp            = "*" | "/" | "&&".
    termOp            = "+" | "-" | "||".
    relOp             = "=" | "#" | "<" | "<=" | ">" | ">=".
    
    factor            = qualident | number | boolean | char | string |
                       "(" expression ")" | subroutineCall | "!" factor.
    term              = factor { factOp factor }.
    simpleexpr        = ["+"|"-"] term { termOp term }.
    expression        = simpleexpr [ relOp simplexpr ].
    
    assignment        = qualident ":=" expression.
    subroutineCall    = ident "(" [ expression {"," expression} ] ")".
    ifStatement       = "if" "(" expression ")" "then" statSequence
                        [ "else" statSequence ] "end".
    whileStatement    = "while" "(" expression ")" "do" statSequence "end".
    returnStatement   = "return" [ expression ].
    
    statement         = assignment | subroutineCall | ifStatement
                        | whileStatement | returnStatement.
    statSequence      = [ statement { ";" statement } ].
    
    constDeclaration  = [ "const" constDeclSequence ].
    constDeclSequence = constDecl ";" { constDecl ";" }
    constDecl         = varDecl "=" expression.
    
    varDeclaration    = [ "var" varDeclSequence ";" ].
    varDeclSequence   = varDecl { ";" varDecl }.
    varDecl           = ident { "," ident } ":" type.
    
    subroutineDecl    = (procedureDecl | functionDecl)
                        ( "extern" | subroutineBody ident ) ";".
    procedureDecl     = "procedure" ident [ formalParam ] ";".
    functionDecl      = "function" ident [ formalParam ] ":" type ";".
    formalParam       = "(" [ varDeclSequence ] ")".
    subroutineBody    = constDeclaration varDeclaration
                        "begin" statSequence "end".
    
    comment           = "//" {[^\n]} \n.
    whitespace        = { " " | \t | \n }.




## Type System

### Scalar Types
SnuPL/2 supports four scalar types: `boolean`, `character`, `integer`, and `longint` types.

#### Storage size and alignment
The storage size, the alignment requirements and the value range are given in the table below:

| Type | Storage Size | Alignment | Value Range |
|:-----|:-------------|:----------|:------------|
| `boolean` | 1 byte | 1 byte | true, false |
| `char` | 1 byte | 1 byte | ISO 8859-1 characters  |
| `integer` | 4 bytes | 4 bytes | -2<sup>31</sup> .. 2<sup>31</sup>-1  |
| `longint` | 8 bytes | 8 bytes | -2<sup>63</sup> .. 2<sup>63</sup>-1 |

#### Operations
SnuPL/2 defines the following operations on its types:

<table>
<thead>
<tr>
  <th style="text-align: center;">Operator</th>
  <th style="text-align: center;">Boolean type</th>
  <th style="text-align: center;">Character type</th>
  <th style="text-align: center;">Integer Types</th></tr>
</thead>
<tbody style="text-align: center;">
<tr><td colspan="4" style="font-weight: bold; text-align: center;">Arithmetic operations</td></tr>
<tr><td>+</td><td>n/a</td><td>n/a</td><td>binary: &lt;int&gt; &larr; &lt;int&gt; + &lt;int&gt;<br>unary: &lt;int&gt; &larr; &lt;int&gt;</td></tr>
<tr><td>-</td><td>n/a</td><td>n/a</td><td>binary: &lt;int&gt; &larr; &lt;int&gt; - &lt;int&gt;<br>unary: &lt;int&gt; &larr; -&lt;int&gt;</td></tr>
<tr><td>*</td><td>n/a</td><td>n/a</td><td>&lt;int&gt; &larr; &lt;int&gt; * &lt;int&gt;</td></tr>
<tr><td>/</td><td>n/a</td><td>n/a</td><td>&lt;int&gt; &larr; &lt;int&gt; / &lt;int&gt;<br> (round towards zero)</td></tr>

<tr><td colspan="4" style="font-weight: bold; text-align: center;">Logical operations</td></tr>
<tr><td>&&</td><td>&lt;bool&gt; &larr; &lt;bool&gt; &and; &lt;bool&gt;</td><td>n/a</td><td>n/a</td></tr>
<tr><td>||</td><td>&lt;bool&gt; &larr; &lt;bool&gt; &or; &lt;bool&gt;</td><td>n/a</td><td>n/a</td></tr>
<tr><td>!</td><td>&lt;bool&gt; &larr; &lt;bool&gt; &not; &lt;bool&gt;</td><td>n/a</td><td>n/a</td></tr>

<tr><td colspan="4" style="font-weight: bold; text-align: center;">Equality and relational operations</td></tr>
<tr><td>=</td><td>&lt;bool&gt; &larr; &lt;bool&gt; &equals; &lt;bool&gt;</td><td>&lt;bool&gt; &larr; &lt;char&gt; &equals; &lt;char&gt;</td><td>&lt;bool&gt; &larr; &lt;int&gt; &equals; &lt;int&gt;</td></tr>
<tr><td>#</td><td>&lt;bool&gt; &larr; &lt;bool&gt; &ne; &lt;bool&gt;</td><td>&lt;bool&gt; &larr; &lt;char&gt; &ne; &lt;char&gt;</td><td>&lt;bool&gt; &larr; &lt;int&gt; &ne; &lt;int&gt;</td></tr>
<tr><td>&lt;</td><td>n/a</td><td>&lt;bool&gt; &larr; &lt;char&gt; &lt; &lt;char&gt;</td><td>&lt;bool&gt; &larr; &lt;int&gt; &lt; &lt;int&gt;</td></tr>
<tr><td>&lt;=</td><td>n/a</td><td>&lt;bool&gt; &larr; &lt;char&gt; &le; &lt;char&gt;</td><td>&lt;bool&gt; &larr; &lt;int&gt; &le; &lt;int&gt;</td></tr>
<tr><td>&gt;=</td><td>n/a</td><td>&lt;bool&gt; &larr; &lt;char&gt; &ge; &lt;char&gt;</td><td>&lt;bool&gt; &larr; &lt;int&gt; &ge; &lt;int&gt;</td></tr>
<tr><td>&gt;</td><td>n/a</td><td>&lt;bool&gt; &larr; &lt;char&gt; &gt; &lt;char&gt;</td><td>&lt;bool&gt; &larr; &lt;int&gt; &gt; &lt;int&gt;</td></tr>

</tbody>
</table>

Boolean, character, and integer types are not compatible and no type conversion/casting is
supported. The two integer types (integer/longint) are compatible. The value is truncated/expanded to the type of the LHS/parameter in assignments. Mixed expressions are converted to the larger type.

Numerical constants comprising only digits are of type integer. Longint constants are generated if the numerical constant ends with the character 'L':
```
const i : integer = 0;
      l : longint = 0L;
```


### Array Types
SnuPL/2 supports multidimensional arrays of scalar types. The declaration of the array requires the dimensions to be specified as a simpleexpr that can be evaluated to an integer value at compile-time:
```
const N : integer = 1024;
var a : longint[128];
    b : integer[N][12*12];
```
The valid index range is from 0 to N-1. Dereferencing an array variable is possible by specifying the indices in brackets:
```
l := a[8];
i := b[1][127];
```

In parameter definitions, open arrays are allowed as follows:
```
procedure WriteStr(str: char[]);
procedure foo(m: integer[][]);
```

The dimensions of open arrays can be queried using `DIM(array, dimension)` (see “Predefined
Procedures and Functions” below.)
```
procedure print(matrix: integer[][]);
var i,j,N,M: integer;
begin
  N := DIM(matrix, 1);
  M := DIM(matrix, 2);
  i := 0;

  while (i < N) do
    j := 0;
    while (j < M) do
      WriteInt(matrix[i][j]); WriteChar('\t')
    end;
    WriteLn()
  end
end print;
```

Support for open arrays and at-runtime querying of array dimensions requires the implementation of arrays to carry the necessary information (i.e., number of dimensions and size per dimension). You are free to choose a memory layout that suits your needs.

One possible implementation is provided with the reference compiler. The reference implementation stores the number of dimensions and the size of each dimension at the beginning of the array. As a consequence, it is not possible to pass sub-arrays as full arrays because the necessary array meta-data is missing. Consider:
```
procedure foo(b: integer[][]);
var n: integer[5][5];
    m: integer[16][16][16];
begin
  foo(n);       // valid
  foo(m);       // invalid: dimension mismatch
  foo(m[1])     // valid, but not supported by reference implementation
end
```


### Characters and Strings
The 8-bit `char` data type represents a single character. Strings are implemented as (constant) character arrays and are null-terminated. SnuPL/2 characters and strings support the ISO-8859-1 standard (latin1). Non-printable characters (0 – 31 and 0x7f) must be escaped. Internally, all unprintable and characters ≥ 0x80 are escaped. The following escape sequences are defined:

| Escape sequence | Character | Remarks |
|:---------------:|:----------|:--------|
| `\n` | newline |
| `\t` | tabulator |
| `\0` | NULL | not allowed within strings |
| `\"` | double quote | only required within double-quoted strings |
| `\'` | single quote | only required within a single-quoted character |
| `\\` | backslash |
| `\xHH` | ASCII character HH | specified in hexadecimal notation |


The NULL character (`\0`) is only allowed in character constants but not within strings. Special rules apply to the printable characters backslash (`\`), double quotes (`"`), and single quotes (`'`):
* Backslash always has to be escaped  
  Example: `"This is a backslash: \\"`
* Double quotes must be escaped in a string and can be escaped in a character constant  
  Example: `"He said, \"That's what she said.\""`  
  Example: `c = '"'; c = '\"';`
* Single quotes must be escaped in a character constant and can be escaped in a string  
  Example: `c = '\'';`  
  Example: `"To which she replied, \'That's what he said!'"`

The enclosing quotes are not part of the string/character constant itself.

Computations with variables of data type `char` are not allowed , i.e., unlike in C, the `char` data type is not a numerical character type. Relational operators are allowed, however; the order of the characters follows the [ISO-8859-1 standard](https://www.iso.org/standard/28245.html) (for a free version, refer to [this page](https://kb.iu.edu/d/aepu)).

Hint: to save files in the latin1 character encoding from within VIM, use
```
:w ++enc=latin1 <filename>
```


### Assignments to Compound Types
The reference implementation of SnuPL/2 does not support assignments to arrays. You are free to lift this limitation in your implementation.

Example:
```
var c: char[32];
begin
  c := "Hello, world!";
```
results in
```
parse error: assignments to compound types are not supported.
```


### Symbolic constants
SnuPL/2 allows the specification of symbolic constants for notational convenience. The following example illustrates the idea:
```
module constants;

const
  K: integer = 1024;
  M: integer = 1024 * K;
  Message: char[] = "Size converter:";

procedure PrintSize(size: integer);
const a,b,c: integer = 1;
     K     : integer = 1000;
var i,j,k  : integer;
begin
  WriteStr(Message); WriteLn();
  WriteStr("Size: ");
  WriteInt(size);
  WriteStr("bytes = "); WriteInt(size / K);
  WriteStr("KB = ");
  WriteInt(size / M);
  WriteStr("MB"); WriteLn()
end PrintSize;

begin
  PrintSize(1111)
end constants.
```

The following rules apply when defining symbolic constants:
* Symbolic constants are typed and defined by a constant expression.
* The constant expression must only contain constant values and previously defined symbolic
constants.
* The type of the constant and the expression must match.
* Variables and constants share the same name space, i.e., constant and variable names must
be unique within the same scope.
* Array constants are not supported with the exception of constant character arrays (i.e.,
strings). The size of the constant array is computed from the length of the constant string.




## External and Predefined Procedures and Functions

SnuPL/2 supports external subroutine declarations and defines a few built-in procedures and functions that are predefined in the compiler (i.e., your compiler must be able to deal with them without throwing an unknown identifier error).


### External subroutines
SnuPL/2 allows the declaration of subroutines with the “extern” keyword. External subroutines can be used normally but an implementation must be provided when linking the executable.
```
module Extern;

function fork(void): integer; extern;

var pid: integer;
begin
  pid := fork();
  WriteStr("my pid: "); WriteInt(pid); WriteLn()
end Extern.
```


### Open arrays
The predefined functions DIM/DOFS are used to deal with open arrays. The functionality can be implemented directly into the compiler or as an external library.
* `function DIM(array: pointer to array; dim: integer): integer;`  
  returns the size of the 'dim'-th array dimension of 'array'.
* `function DOFS(array: pointer to array): integer;`  
  returns the number of bytes from the starting address of the array to the first data element.

Example usage is provided above (Type System – Array Types)


### Input/Output
The following predefined low-level I/O routines read/write integers, characters, and strings. An implementation is provided and can be linked to the compiled code.
* `function ReadInt(): integer;`  
  `function ReadLong(): longint;`  
  read and return an integer/longint value from stdin.
* `procedure WriteInt(v: integer);`  
  `procedure WriteLong(v: longint);`  
  print integer/longint value ‘v’ to stdout.
* `procedure WriteChar(c: char);`  
  write a single character to stdout.
* `procedure WriteStr(string: char[]);`  
  write string ‘string’ to stdout. No newline is added.
* `procedure WriteLn();`  
  write a newline sequence to stdout.

Examples are provided throughout the text in this document.




## Parameter Passing and Backend ABIs
Scalar arguments are passed by value, array arguments by reference.


### x86-64 Backend
The x86-64 backend follows the [ELF x86-64-ABI psABI](https://gitlab.com/x86-psABIs/x86-64-ABI). The first six parameter are passed in registers, parameters >= 7 are passed on the stack in reverse order. The result is returned in %rax. 

| Register | Function | Save | Register | Function | Save |
|:--------:|:---------|:-----|:--------:|:---------|:-----|
| `rax` | return value | caller | `r8` | argument #5 | caller |
| `rcx` | argument #4 | caller | `r9` | argument #6 | caller |
| `rdx` | argument #3 | caller | `r10` | | caller |
| `rbx` | | callee | `r11` | | caller |
| `rsi` | argument #2 | callee | `r12` | | callee |
| `rdi` | argument #1 | callee | `r13` | | callee |
| `rsp` | stack pointer | (callee) | `r14` | | callee |
| `rbp` | | callee | `r15` | | callee |


### IA32 Backend
The IA32 backend follows the [System V ABI for Intel386 Architectures](http://www.sco.com/developers/devspecs/abi386-4.pdf). Parameters are passed on the stack in reverse order, results returned in %eax.

| Register | Function | Save | Register | Function | Save |
|:--------:|:---------|:-----|:--------:|:---------|:-----|
| `eax` | return value | caller | `esi` | | callee |
| `ecx` |  | caller | `edi` | | callee |
| `edx` | | caller | `esp` | stack pointer | (callee) |
| `ebx` | | callee | `ebp` | frame pointer | callee |

The reference IA32 backend does not support the longint data type.




## Example
The following shows a valid test program exercising (almost) all features of the language.

    //
    // Matrix Add
    //
    // A valid SnuPL/2 test program
    //
    
    module MatrixAdd;
    
    const
      N : integer = 2*(2+2);
      ProgramName : char[] = "Matrix\t Adder\n\n";
    
    procedure minit(m: integer[][]);
    var x,y,v: integer;
    begin
      v := 1;
      while (y < N) do
        x := 0;
        while (x < N) do
          m[y][x] := v;
          v := v+1;
          if (v = 10) then v := 0 end;
          x := x+1
        end;
        y := y+1
      end
    end minit;
    
    procedure madd(sum: integer[N][N]; a,b: integer[N][N]);
    var x,y: integer;
    begin
      y := 0;
      while (y < N) do
        x := 0;
        while (x < N) do
          sum[y][x] := a[y][x] + b[y][x];
          x := x+1
        end;
        y := y+1
      end
    end madd;
    
    procedure mprint(m: integer[][]; title: char[]);
    const MStr : char[] = ". Matrix ";
    var N,M,x,y: integer;
    begin
      M := DIM(m, 1);
      N := DIM(m, 2);
    
      WriteStr(title); WriteStr(MStr); WriteInt(M); WriteChar('x'); WriteInt(N); 
      WriteLn();
      WriteStr("[\n");
    
      while (y < M) do
        WriteStr("  "); WriteInt(y); WriteStr(":  [   ");
    
        x := 0;
        while (x < N) do
          WriteInt(m[y][x]); WriteStr("   ");
          x := x+1
        end;
    
        WriteStr("]\n");
        y := y+1
      end;
    
      WriteStr("]\n\n")
    end mprint;
    
    var
      A, B, C : integer[N][N];
    begin
      WriteStr(ProgramName); WriteLn();
    
      minit(A);
      minit(B);
      minit(C);
    
      mprint(A, "A");
      mprint(B, "B");
    
      madd(C, A, B);
    
      mprint(C, "C = A+B")
    end MatrixAdd.

