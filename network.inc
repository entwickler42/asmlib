import  gethostbyname   ws2_32.dll
import  inet_addr       ws2_32.dll

extern  gethostbyname
extern  inet_addr

;------------------------------------------------------------------------------
;   PARMS:
;           eax =   offset of IP String or Hostname String

%define HOST ebp-0x04

inet_resolve:
    push    ebp
    mov     ebp, esp
    sub     esp, 0x04
    
    mov     [HOST], eax                 ; save offset of hostname/ip
    
    push    eax                         ; try to get address my ip
    call    [inet_addr]
    
    cmp     eax, INADDR_NONE
    jne     near .return_resolve    

    mov     eax, [HOST]
    push    eax                         ; try to get address by name
    call    [gethostbyname]
    
    cmp     eax, NULL                   ; on Error try to resolve by IP
    je      near .return_resolve
                               
    mov     eax, dword [eax+0x0C]       ; readout of server address
    mov     eax, [eax]                   
    mov     eax, [eax]
                  
.return_resolve:     
    add     esp, 0x04                       
    pop     ebp        
    ret
    
%undef HOST
              