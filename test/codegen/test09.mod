//
// test09
//
// Code generation
// - control flow expressions
//

module test09;

var a,b,c: integer;

begin
  a := 1;
  b := 2;
  c := 3;

  if (a+b < c) then WriteInt(a)
  else WriteInt(b)
  end;
  WriteLn()
end test09.
