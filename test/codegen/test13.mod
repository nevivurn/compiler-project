//
// test13
//
// Code generation
// - local arrays
// - character constants
//

module test13;

procedure local();
var A: integer[3][7];
    i,j,c: integer;
begin
  c := 0;

  i := 0;
  while (i < 3) do
    j := 0;
    while (j < 7) do
      A[i][j] := c;
      c := c+1;
      j := j+1
    end;
    i := i+1
  end;

  i := 0;
  while (i < 3) do
    j := 0;
    while (j < 7) do
      WriteInt(A[i][j]); WriteChar(' ');
      j := j+1
    end;
    WriteLn();
    i := i+1
  end
end local;

begin
  local()
end test13.
