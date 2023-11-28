//
// test18
//
// Code generation
// - boolean variables and expressions
//

module test18;

const
  N: integer = 10;

var
  a,b,c: boolean;
  B: boolean[N];
  i: integer;

begin
  a := true;
  b := false;

  c := (a && b) || (a || !b);
  if (c) then WriteInt(1)
  else WriteInt(0) end;
  WriteLn();

  i := 0;
  while (i < N) do
    B[i] := i = i / 2 * 2;
    i := i+1
  end;

  i := 0;
  while (i < N) do
    if (B[i]) then WriteInt(1)
    else WriteInt(0) end;
    WriteLn();
    i := i+1
  end
end test18.
