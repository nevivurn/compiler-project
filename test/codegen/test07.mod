//
// test07
//
// Code generation
// - function with parameters
//

module test07;

var a: integer;

function foo(x, y, z: integer): integer;
begin
  return z
end foo;

begin
  a := foo(1, 2, 3);
  WriteInt(a); WriteLn()
end test07.
