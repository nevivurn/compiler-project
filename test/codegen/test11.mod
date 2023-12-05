//
// test11
//
// Code generation
// - global arrays
//

module test11;

var A: integer[10];
    i: integer;

begin
  i := 0;
  while (i < 10) do
    A[i] := i;
    i := i+1
  end;

  i := 0;
  while (i < 10) do
    WriteInt(A[i]); WriteLn();
    i := i + 1
  end
end test11.
