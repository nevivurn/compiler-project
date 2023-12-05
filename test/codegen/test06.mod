//
// test06
//
// Code generation
// - procedure with parameters
//

module test06;

procedure print(a, b, c: integer);
begin
  WriteInt(a); WriteLn();
  WriteInt(b); WriteLn();
  WriteInt(c); WriteLn()
end print;

begin
  print(1, 2, 3)
end test06.
