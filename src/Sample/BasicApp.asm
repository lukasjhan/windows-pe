int WINAPI _tWinMain(HINSTANCE hInst, HINSTANCE hPrevInst,
                     PTSTR szCmdLine, int iCmdShow)
{
00411620		push	ebp  
00411621		mov 	ebp,esp  
00411623		sub 	esp,128h  
00411629		push	ebx  
0041162A		push	esi  
0041162B		push	edi  
0041162C		lea 	edi,[ebp-128h]  
00411632		mov 	ecx,4Ah  
00411637		mov 	eax,0CCCCCCCCh  
0041163C	rep stos	dword ptr es:[edi]  

	WNDCLASSEX wc;
	wc.cbSize		 = sizeof(wc);
0041163E		mov		dword ptr [wc],30h  
	wc.style		 = CS_HREDRAW | CS_VREDRAW;
00411645		mov		dword ptr [ebp-30h],3  
	wc.lpfnWndProc   = WndProc;
0041164C		mov		dword ptr [ebp-2Ch],41117Ch  
	wc.cbClsExtra    = 0;
00411653		mov		dword ptr [ebp-28h],0  
	wc.cbWndExtra    = 0;
0041165A		mov		dword ptr [ebp-24h],0  
	wc.hInstance	 = hInst;
00411661		mov		eax,dword ptr [hInst]  
00411664		mov		dword ptr [ebp-20h],eax  
	wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
00411667		mov 	esi,esp  
00411669		push	7F00h  
0041166E		push	0  
00411670		call	dword ptr ds:[41A194h]  
00411676		cmp 	esi,esp  
00411678		call	__RTC_CheckEsp (041116Dh)  
0041167D		mov 	dword ptr [ebp-1Ch],eax  
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
00411680		mov 	esi,esp  
00411682		push	7F00h  
00411687		push	0  
00411689		call	dword ptr ds:[41A198h]  
0041168F		cmp 	esi,esp  
00411691		call	__RTC_CheckEsp (041116Dh)  
00411696		mov 	dword ptr [ebp-18h],eax  
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
00411699		mov		esi,esp  
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
0041169B		push	0  
0041169D		call	dword ptr ds:[41A000h]  
004116A3		cmp 	esi,esp  
004116A5		call	__RTC_CheckEsp (041116Dh)  
004116AA		mov 	dword ptr [ebp-14h],eax  
	wc.lpszMenuName  = NULL;
004116AD		mov		dword ptr [ebp-10h],0  
	wc.hIconSm		 = NULL;
004116B4		mov		dword ptr [ebp-8],0  
	wc.lpszClassName = G_APP_CLS;
004116BB		mov		dword ptr [ebp-0Ch],419000h  
	if(!RegisterClassEx(&wc))
004116C2		mov 	esi,esp  
004116C4		lea 	eax,[wc]  
004116C7		push	eax  
004116C8		call	dword ptr ds:[41A1BCh]  
004116CE		cmp 	esi,esp  
004116D0		call	__RTC_CheckEsp (041116Dh)  
004116D5		movzx	ecx,ax  
004116D8		test	ecx,ecx  
004116DA		jne 	wWinMain+0E0h (0411700h)  
	{
		MessageBox(NULL, _T("RegisterClassEx API call failed!"),
			G_APP_CLS, MB_ICONERROR);
004116DC		mov 	esi,esp  
004116DE		push	10h  
004116E0		push	419000h  
004116E5		push	416858h  
004116EA		push	0  
004116EC		call	dword ptr ds:[41A19Ch]  
004116F2		cmp 	esi,esp  
004116F4		call	__RTC_CheckEsp (041116Dh)  
		return 0 ;
004116F9		xor		eax,eax  
004116FB		jmp		wWinMain+1BAh (04117DAh)  
	}

	HWND hWnd = CreateWindowEx
	(
		0, 
		G_APP_CLS,					// window class name
		_T("YHD Hello Program"),	// window caption
		WS_OVERLAPPEDWINDOW,		// window style
		CW_USEDEFAULT,              // initial x position
		CW_USEDEFAULT,              // initial y position
		400,						// initial x size
		150,						// initial y size
		NULL,                       // parent window handle
		NULL,                       // window menu handle
		hInst,						// program instance handle
		NULL						// creation parameters
	);
00411700		mov		esi,esp  
00411702		push	0  
	}

	HWND hWnd = CreateWindowEx
	(
		0, 
		G_APP_CLS,					// window class name
		_T("YHD Hello Program"),	// window caption
		WS_OVERLAPPEDWINDOW,		// window style
		CW_USEDEFAULT,              // initial x position
		CW_USEDEFAULT,              // initial y position
		400,						// initial x size
		150,						// initial y size
		NULL,                       // parent window handle
		NULL,                       // window menu handle
		hInst,						// program instance handle
		NULL						// creation parameters
	);
00411704		mov 	eax,dword ptr [hInst]  
00411707		push	eax  
00411708		push	0  
0041170A		push	0  
0041170C		push	96h  
00411711		push	190h  
00411716		push	80000000h  
0041171B		push	80000000h  
00411720		push	0CF0000h  
00411725		push	4168A8h  
0041172A		push	419000h  
0041172F		push	0  
00411731		call	dword ptr ds:[41A1B8h]  
00411737		cmp 	esi,esp  
00411739		call	__RTC_CheckEsp (041116Dh)  
0041173E		mov 	dword ptr [hWnd],eax  
	if (hWnd == NULL)
00411741		cmp		dword ptr [hWnd],0  
00411745		jne		wWinMain+148h (0411768h)  
	{
		MessageBox(NULL, _T("CreateWindowEx API call failed!"),
			G_APP_CLS, MB_ICONERROR);
00411747		mov 	esi,esp  
00411749		push	10h  
0041174B		push	419000h  
00411750		push	4168D8h  
00411755		push	0  
00411757		call	dword ptr ds:[41A19Ch]  
0041175D		cmp 	esi,esp  
0041175F		call	__RTC_CheckEsp (041116Dh)  
		return 0;
00411764		xor		eax,eax  
00411766		jmp		wWinMain+1BAh (04117DAh)  
	}
	ShowWindow(hWnd, iCmdShow);
00411768		mov 	esi, esp  
0041176A		mov 	eax, dword ptr [iCmdShow]  
0041176D		push	eax  
0041176E		mov 	ecx, dword ptr [hWnd]  
00411771		push	ecx  
00411772		call	dword ptr ds:[41A1B4h]  
00411778		cmp 	esi,esp  
0041177A		call	__RTC_CheckEsp (041116Dh)  
	UpdateWindow(hWnd);
0041177F		mov 	esi,esp  
00411781		mov 	eax,dword ptr [hWnd]  
00411784		push	eax  
00411785		call	dword ptr ds:[41A1ACh]  
0041178B		cmp 	esi,esp  
0041178D		call	__RTC_CheckEsp (041116Dh)  

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
00411792		mov 	esi,esp  
00411794		push	0  
00411796		push	0  
00411798		push	0  
0041179A		lea 	eax,[msg]  
0041179D		push	eax  
0041179E		call	dword ptr ds:[41A1D0h]  
004117A4		cmp 	esi,esp  
004117A6		call	__RTC_CheckEsp (041116Dh)  
004117AB		test	eax,eax  
004117AD		je  	wWinMain+1B7h (04117D7h)  
	{
		TranslateMessage(&msg);
004117AF		mov 	esi,esp  
004117B1		lea 	eax,[msg]  
004117B4		push	eax  
004117B5		call	dword ptr ds:[41A1CCh]  
004117BB		cmp 	esi,esp  
004117BD		call	__RTC_CheckEsp (041116Dh)  
		DispatchMessage(&msg);
004117C2		mov 	esi,esp  
004117C4		lea 	eax,[msg]  
004117C7		push	eax  
004117C8		call	dword ptr ds:[41A1C8h]  
004117CE		cmp 	esi,esp  
004117D0		call	__RTC_CheckEsp (041116Dh)  
	}
004117D5		jmp		wWinMain+172h (0411792h)  

	return (int)msg.wParam;
004117D7		mov         eax, dword ptr [ebp-5Ch]  
}
004117DA		push	edx  
004117DB		mov 	ecx, ebp  
004117DD		push	eax  
004117DE		lea 	edx, ds:[411804h]  
004117E4		call	@_RTC_CheckStackVars@8 (0411096h)  
004117E9		pop 	eax  
004117EA		pop 	edx  
004117EB		pop 	edi  
004117EC		pop 	esi  
004117ED		pop 	ebx  
004117EE		add 	esp,128h  
004117F4		cmp 	ebp, esp  
004117F6		call	__RTC_CheckEsp (041116Dh)  
004117FB		mov 	esp, ebp  
004117FD		pop 	ebp  
004117FE		ret 	10h  







LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
00411470		push		ebp  
00411471		mov			ebp,esp  
00411473		sub			esp,134h  
00411479		push		ebx  
0041147A		push		esi  
0041147B		push		edi  
0041147C		lea 		edi,[ebp-134h]  
00411482		mov 		ecx,4Dh  
00411487		mov 		eax,0CCCCCCCCh  
0041148C		rep stos	dword ptr es:[edi]  
0041148E		mov			eax,dword ptr ds:[00419090h]  
00411493		xor			eax,ebp  
00411495		mov			dword ptr [ebp-4],eax  
	static TCHAR szMsg[] = _T("The Most Simple Windows GUI Program by YHD.");
	switch(uMsg)
00411498		mov			eax,dword ptr [uMsg]  
0041149B		mov			dword ptr [ebp-134h],eax  
004114A1		cmp			dword ptr [ebp-134h],2  
004114A8		je 			WndProc+0CEh (041153Eh)  
004114AE		cmp			dword ptr [ebp-134h],0Fh  
004114B5		je 			WndProc+4Ch (04114BCh)  
004114B7		jmp			WndProc+0E3h (0411553h)  
	{
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			RECT        rc;
			HDC hDC = BeginPaint(hWnd, &ps);
004114BC		mov 		esi,esp  
004114BE		lea 		eax,[ebp-48h]  
004114C1		push		eax  
004114C2		mov 		ecx,dword ptr [hWnd]  
004114C5		push		ecx  
004114C6		call		dword ptr ds:[41A1A8h]  
004114CC		cmp 		esi,esp  
004114CE		call		__RTC_CheckEsp (041116Dh)  
004114D3		mov 		dword ptr [ebp-6Ch],eax  
			GetClientRect(hWnd, &rc);
004114D6		mov 		esi,esp  
004114D8		lea 		eax,[ebp-60h]  
004114DB		push		eax  
004114DC		mov 		ecx,dword ptr [hWnd]  
004114DF		push		ecx  
004114E0		call		dword ptr ds:[41A1A0h]  
004114E6		cmp 		esi,esp  
004114E8		call		__RTC_CheckEsp (041116Dh)  
			DrawText(hDC, szMsg, (int)_tcslen(szMsg),
				&rc, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
004114ED		mov 		esi,esp  
004114EF		push		25h  
004114F1		lea 		eax,[ebp-60h]  
004114F4		push		eax  
004114F5		mov 		edi,esp  
004114F7		push		419020h  
004114FC		call		dword ptr ds:[41A144h]  
00411502		add 		esp,4  
00411505		cmp 		edi,esp  
00411507		call		__RTC_CheckEsp (041116Dh)  
0041150C		push		eax  
0041150D		push		419020h  
00411512		mov 		ecx,dword ptr [ebp-6Ch]  
00411515		push		ecx  
00411516		call		dword ptr ds:[41A1B0h]  
0041151C		cmp 		esi,esp  
0041151E		call		__RTC_CheckEsp (041116Dh)  
			EndPaint(hWnd, &ps);
00411523		mov 		esi,esp  
00411525		lea 		eax,[ebp-48h]  
00411528		push		eax  
00411529		mov 		ecx,dword ptr [hWnd]  
0041152C		push		ecx  
0041152D		call		dword ptr ds:[41A1A4h]  
00411533		cmp 		esi,esp  
00411535		call		__RTC_CheckEsp (041116Dh)  
		}
		return 0;
0041153A		xor			eax,eax  
0041153C		jmp			WndProc+102h (0411572h)  

		case WM_DESTROY:
			PostQuitMessage(0);
0041153E		mov			esi,esp  
00411540		push		0  
00411542		call		dword ptr ds:[41A1C0h]  
00411548		cmp			esi,esp  
0041154A		call		__RTC_CheckEsp (041116Dh)  
		return 0;
0041154F		xor			eax,eax  
00411551		jmp			WndProc+102h (0411572h)  
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
00411553		mov         esi,esp  
00411555		mov         eax,dword ptr [lParam]  
00411558		push        eax  
00411559		mov         ecx,dword ptr [wParam]  
0041155C		push        ecx  
0041155D		mov         edx,dword ptr [uMsg]  
00411560		push        edx  
00411561		mov         eax,dword ptr [hWnd]  
00411564		push        eax  
00411565		call        dword ptr ds:[41A1C4h]  
0041156B		cmp         esi,esp  
0041156D		call        __RTC_CheckEsp (041116Dh)  
}
00411572		push        edx  
00411573		mov         ecx,ebp  
00411575		push        eax  
00411576		lea         edx,ds:[4115A4h]  
0041157C		call        @_RTC_CheckStackVars@8 (0411096h)  
00411581		pop         eax  
00411582		pop         edx  
00411583		pop         edi  
00411584		pop         esi  
00411585		pop         ebx  
}
00411586		mov         ecx,dword ptr [ebp-4]  
00411589		xor         ecx,ebp  
0041158B		call        @__security_check_cookie@4 (0411028h)  
00411590		add         esp,134h  
00411596		cmp         ebp,esp  
00411598		call        __RTC_CheckEsp (041116Dh)  
0041159D		mov         esp,ebp  
0041159F		pop         ebp  
004115A0		ret         10h  
