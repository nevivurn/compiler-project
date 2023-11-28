//
// test15
//
// Code generation
// - open array parameters
//

module test15;

var A: integer[3][7];
    B: integer[5][3];

procedure init(a: integer[][]);
var i,j,c: integer;
    N,M: integer;
begin
  N := DIM(a, 1);
  M := DIM(a, 2);

  c := 0;
  i := 0;
  while (i < N) do
    j := 0;
    while (j < M) do
      a[i][j] := c;
      c := c+1;
      j := j+1
    end;
    i := i+1
  end
end init;

procedure print(a: integer[][]);
var i,j: integer;
    N,M: integer;
begin
  N := DIM(a, 1);
  M := DIM(a, 2);

  i := 0;
  while (i < N) do
    j := 0;
    while (j < M) do
      WriteInt(a[i][j]); WriteChar(' ');
      j := j+1
    end;
    WriteLn();
    i := i+1
  end
end print;

begin
  init(A);
  print(A);

  init(B);
  print(B)
end test15.
