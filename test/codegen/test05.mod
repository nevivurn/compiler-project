//
// test05
//
// Code generation
// - global integer variables
// - simple assignments
// - procedure definitions
// - function/procedure calls
//

module test05;

var a,b,c: integer;

procedure print();
begin
  WriteInt(a); WriteLn();
  WriteInt(b); WriteLn();
  WriteInt(c); WriteLn()
end print;

begin
  a := 1;
  b := 2;
  c := 3;

  print()
end test05.
