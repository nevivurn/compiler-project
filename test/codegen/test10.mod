//
// test10
//
// Code generation
// - control flow expressions
//

module test10;

var a,b,c: integer;

begin
  a := 1;
  b := 2;
  c := 3;

  while (a < c) do
    WriteInt(a); WriteLn();
    a := a + 1
  end
end test10.
