import LocalAlloc       kernel32.dll
import LocalFree        kernel32.dll
import GetFileSize      kernel32.dll

extern LocalAlloc
extern LocalFree
extern GetFileSize

;------------------------------------------------------------------------------
;   PARMS:
;           eax:    binary size
;
;           ( binary size / 3 ) * 4 = base64 bytes 
;           ( binary size % 3 ) != 0 base64 bytes + 4

base64_size:
    push    edx
    push    ecx
    
    xor     edx, edx
    mov     ecx, dword 0x03
    div     ecx
    push    edx    
    xor     edx, edx
    mov     ecx, dword 0x04
    mul     ecx       
    pop     edx    
    cmp     edx, 0x0
    je      near .base64_size_ret
    
    mov     ecx, dword 0x04
    add     eax, ecx
       
.base64_size_ret:    
    pop     ecx
    pop     edx
    ret

;------------------------------------------------------------------------------
;   PARMS:
;           eax:    offset of buffer to encode    
;           ebx:    size of buffer 

%define SOURCE              ebp-0x04
%define SIZE                ebp-0x08
%define CIPHER              ebp-0x0C
%define CIPHER_SIZE         ebp-0x10
%define SOURCE_BUFF         ebp-0x14
%define SOURCE_BUFF_SIZE    ebp-0x18

encode_b64:
    push    ebp
    mov     ebp, esp
    sub     esp, 0x18

    mov     [SOURCE], eax                 ; save parameters    
    mov     [SIZE], ebx
            
    mov     eax, ebx                      ; allocate memory for cipher
    call    base64_size
    mov     [CIPHER_SIZE], eax
    push    eax
    push    dword LPTR    
    call    [LocalAlloc]
    cmp     eax, NULL
    je      near .encode_b64_ret
    mov     [CIPHER], eax
    
    mov     eax, [SIZE]                  ; determ nessesary source buffer size
    mov     [SOURCE_BUFF_SIZE], eax    
.find_source_buff_size:
    mov     edx, dword 0x0
    mov     eax, [SOURCE_BUFF_SIZE]
    mov     ecx, dword 0x03
    div     ecx
    cmp     edx, 0x0
    je      near .found_source_buff_size    
    mov     eax, [SOURCE_BUFF_SIZE]
    add     eax, 0x01
    mov     [SOURCE_BUFF_SIZE], eax
    jmp     near .find_source_buff_size
    
.found_source_buff_size:        
    mov     eax, [SOURCE_BUFF_SIZE]      ; alloc memory for source buffer               
    push    eax                     
    push    dword LPTR
    call    [LocalAlloc]    
    mov     [SOURCE_BUFF], eax
    cmp     eax, NULL
    je      near .free_on_error
            
    mov     esi, [SOURCE]                 ; copy source to source buffer
    mov     edi, [SOURCE_BUFF]
    mov     ecx, [SIZE]
.copy_source:
    lodsb   
    stosb
    dec     ecx
    cmp     ecx, 0x0
    jne     near .copy_source
    mov     eax, [SIZE]
    mov     ecx, [SOURCE_BUFF_SIZE]
    sub     ecx, ebx
    mov     al, 0x0
    rep     stosb                     
             
    mov     esi, [SOURCE_BUFF]           ; encode source
    mov     edi, [CIPHER]    
    mov     ecx, [SOURCE_BUFF_SIZE]
    
.read_block:
    lodsb   
    mov     [CBUF], al       
    lodsb   
    mov     [CBUF+0x01], al
    lodsb   
    mov     [CBUF+0x02], al
    
    xor     ebx, ebx                     ; encode 1st byte
    mov     bl, byte [CBUF]
    shr     bl, 0x02
    mov     al, [B64_TABLE+ebx]
    stosb
    
    xor     ebx, ebx                     ; encode 2nd byte
    xor     edx, edx
    mov     dl, byte [CBUF]
    and     dl, 00000011b
    shl     dl, 0x04
    mov     bl, byte [CBUF+0x01]
    shr     bl, 0x04
    add     bl, dl    
    mov     al, [B64_TABLE+ebx]
    stosb
    
    xor     ebx, ebx                     ; encode 3rd byte
    xor     edx, edx
    mov     dl, byte [CBUF+0x01]
    and     dl, 00001111b
    shl     dl, 0x02
    mov     bl, byte [CBUF+0x02]
    shr     bl, 0x06
    add     bl, dl
    mov     al, [B64_TABLE+ebx]
    stosb
    
    xor     ebx, ebx                     ; encode 4th byte
    mov     bl, byte [CBUF+0x02]
    and     bl, 00111111b
    mov     al, [B64_TABLE+ebx]
    stosb           
    
    sub     ecx, 0x03
    cmp     ecx, 0x00
    jne     near .read_block
    
    mov     eax, [SIZE]                  ; if nessesary append filling char('s)
    mov     ecx, [SOURCE_BUFF_SIZE]
    sub     ecx, eax
    cmp     ecx, 0x0
    je      near .encode_b64_ret
    mov     edi, [CIPHER]
    mov     eax, [CIPHER_SIZE]
    sub     eax, ecx
    add     edi, eax
    mov     al, '='
    rep     stosb         
    
    jmp     near .encode_b64_ret
    
.free_on_error:  
    mov     eax, [CIPHER]                ; free memory an prepare to return 0 values  
    push    eax
    call    [LocalFree]
    mov     [CIPHER], eax
    mov     [CIPHER_SIZE], dword 0x0            
        
.encode_b64_ret:
    mov     eax, [SOURCE_BUFF]
    push    eax
    call    [LocalFree]

    mov     eax, [CIPHER]
    mov     ebx, [CIPHER_SIZE]
   
    add     esp, 0x18
    pop     ebp 
    ret
    
%undef SOURCE   
%undef SIZE   
%undef CIPHER
%undef CIPHER_SIZE 
%undef SOURCE_BUFF
%undef SOURCE_BUFF_SIZE
    
;------------------------------------------------------------------------------
;   PARMS:
;           eax:    offset of filepath

%define hFILE        ebp-0x04
%define SIZE         ebp-0x08
%define fBUFF        ebp-0x0C
%define fCIPHER      ebp-0x10
%define fCIPHER_SIZE ebp-0x14

encode_b64_file:
    push    ebp
    mov     ebp, esp
    sub     esp, 0x14            
	                                    ; Open Source File
	push	dword NULL					; handle to file with attributes to copy
	push	dword FILE_ATTRIBUTE_NORMAL	; file attributes
	push	dword OPEN_EXISTING			; how to create
	push	dword NULL					; security attributes			
	push 	dword FILE_SHARE_READ		; share mode		
	push	dword GENERIC_READ			; access mode
	push	eax							; address of filename			
	call 	[CreateFileA]		
	mov		[hFILE], eax
	
    cmp     eax,0x0        
    je      near .exit_encode_b64_file
    
    push    NULL                        ; get filesize
    mov     eax, [hFILE]
    push    eax
    call    [GetFileSize]  
    mov     [SIZE], eax  
    cmp     eax, 0xFFFFFFFF    
    je      near .exit_encode_b64_file
    
    mov     eax, [SIZE]                 ; alloc memory for file               
    push    eax                     
    push    dword LPTR
    call    [LocalAlloc]
    mov     [fBUFF], eax          
    cmp     eax, NULL    
    je      near .exit_encode_b64_file    
        
    mov     eax, [SIZE]                 ; read file to memory
    push    eax
    mov     eax, [fBUFF]
    push    eax
    mov     eax, [hFILE]
    push    eax
    call    [_lread]
    
    mov     eax, [hFILE]                ; close filehandle
    push    eax
    call    [CloseHandle]

    mov     eax, [fBUFF]                ; encode base64
    mov     ebx, [SIZE]
    call    encode_b64
    
    mov     [fCIPHER], eax
    mov     [fCIPHER_SIZE], ebx
    
.exit_encode_b64_file    
    mov     eax, [fBUFF]                ; free memory used for plain file
    push    eax
    call    [LocalFree]
    
    mov     eax, [fCIPHER]
    mov     ebx, [fCIPHER_SIZE]
    
    add     esp, 0x14
    pop     ebp
    ret
    
%undef hFILE
%undef SIZE        
%undef fBUFF
%undef fCIPHER
%undef fCIPHER_SIZE