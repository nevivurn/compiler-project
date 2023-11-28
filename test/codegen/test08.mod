//
// test08
//
// Code generation
// - integer arithmetic expressions
//

module test08;

var a,b,c: integer;

begin
  a := 1;
  b := 2;
  c := 3;

  c := a + b;
  WriteInt(c); WriteLn();

  c := a - b;
  WriteInt(c); WriteLn();

  c := a * b;
  WriteInt(c); WriteLn();

  c := (a+b) / c;
  WriteInt(c); WriteLn()
end test08.
