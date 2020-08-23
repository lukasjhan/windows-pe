.code
GetTibPtr PROC
	mov rax, gs:[30h]
	ret
GetTibPtr ENDP
End
