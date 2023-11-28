//
// test14
//
// Code generation
// - array parameters
//

module test14;

var A: integer[3][7];
    i,j,c: integer;

procedure print(a: integer[3][7]);
var i,j: integer;
begin
  i := 0;
  while (i < 3) do
    j := 0;
    while (j < 7) do
      WriteInt(a[i][j]); WriteChar(' ');
      j := j+1
    end;
    WriteLn();
    i := i+1
  end
end print;

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

  print(A)
end test14.
