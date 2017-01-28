BITS 32

;------------------------------------------------------------------------------
;AUTOR  :   Stefan Brück <mailbockx@freenet.de> 
;DATE   :   040310                        
;------------------------------------------------------------------------------

;------------------------------------------------------------------------------
section data use32 class=data

ABOUT   db  "String manipulation library written my Stefan Brück in March 2004",0x0
RELEASE db  "1.0",0x0    

;------------------------------------------------------------------------------
section code use32 class=code

global strlen
global strncmp
global strncpy
global strncat
global memset

;------------------------------------------------------------------------------
;PARAMETER          :   eax = offset text 
;RETURN VALUE(s)    :   eax = size of text
;------------------------------------------------------------------------------
strlen:
    push    edi
    push    ecx
    
    mov     edi, eax
    xor     eax, eax
    xor     ecx, ecx
    
.strlen_count:
    inc     ecx
    scasb    
    jne     near .strlen_count  
    
    dec     ecx
    mov     eax, ecx    
                     
    pop     ecx
    pop     edi
    
    ret
    
;------------------------------------------------------------------------------
;PARAMETER          :   eax = offset 1st text
;                       ebx = offset 2nd text
;                       ecx = number of bytes to compare 
;RETURN VALUE(s)    :   eax = 0 if equal or 1 if not equal 
;------------------------------------------------------------------------------
strncmp:
    push    esi
    push    edi
    push    ecx
    
    mov     esi, eax
    mov     edi, ebx
    
    repe    cmpsb
    mov     eax, ecx
    
    pop     ecx
    pop     edi
    pop     esi

    ret
    
strcmp:
    
    
    ret   
    
;------------------------------------------------------------------------------
;PARAMETER          :   eax = offset source text
;                       ebx = offset destination text
;                       ecx = number of byte to copy
;RETURN VALUE(s)    :   eax = offset text
;------------------------------------------------------------------------------
strncpy:
    push    esi
    push    edi
    push    ecx
    push    ebx
    
    mov     esi, eax
    mov     edi, ebx
    
    rep     movsb 
    xor     eax, eax
    stosb
       
    pop     eax    
    pop     ecx
    pop     edi
    pop     esi    
    
    ret
    
;------------------------------------------------------------------------------
;PARAMETER          :   eax = destination string
;                       ebx = string to attach
;                       ecx = size of string to attach
;RETURN VALUE(s)    :
;------------------------------------------------------------------------------
strncat:
    push    eax        
    push    ebx
    push    ecx
    push    esi
    push    edi
              
    mov     edi, eax        ; attach string   
    call    strlen          
    add     edi, eax                         
    inc     ecx             
    mov     esi, ebx         
    rep     movsb
                
    pop     edi
    pop     esi
    pop     ecx
    pop     ebx
    pop     eax       

    ret

;------------------------------------------------------------------------------
;PARAMETER          :   eax = destination
;                       bl  = fillchar
;                       ecx = number of bytes to fill 
;------------------------------------------------------------------------------
memset:
    push    eax
    push    ecx
    push    ebx
    push    edi
    
    mov     edi, eax
    xor     eax, eax
    mov     al, bl
    rep     stosb
    
    pop     edi
    pop     ebx
    pop     ecx
    pop     eax
        
    ret
    
    

    