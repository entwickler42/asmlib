import DnsRecordListFree    dnsapi.dll
import DnsQuery_A           dnsapi.dll

extern DnsRecordListFree
extern DnsQuery_A

;------------------------------------------------------------------------------
;   PARMS:
;           eax = offset host
;           ebx = offset return_value

%define HOST            ebp-0x04
%define RETURN_VALUE    ebp-0x08
%define PRR             ebp-0x0C

resolve_mx:
    push    ebp
    mov     ebp, esp
    sub     esp, 0x0C
    
    mov     [HOST], eax             ; store parameter
    mov     [RETURN_VALUE], ebx
    
    push    dword NULL
    lea     eax, [PRR]
    push    eax
    push    dword NULL
    push    dword 0x00
    push    dword 0x0F
    mov     eax, [HOST]
    push    eax
    call    [DnsQuery_A]
    cmp     eax, dword 0x0
    jne     near .resolve_err
    
    mov     eax, [PRR]
    mov     esi, [eax+0x18]
    mov     edi, [RETURN_VALUE]    
    xor     eax, eax    
.resolve_mx_genret:
    lodsb
    stosb
    cmp     al, 0x0
    jne     near .resolve_mx_genret
    mov     eax, dword 0x0      

.resolve_mx_cleanup:
    push    dword 0x01
    mov     eax, [PRR]
    push    eax    
    call    [DnsRecordListFree]
    jmp     near .resolve_mx_exit
    
.resolve_err:
    mov     eax, dword 0x0    

.resolve_mx_exit:
    add     esp, 0x0C
    pop     ebp
    ret
    
%undef HOST
%undef RETURN_VALUE
%undef PRR    