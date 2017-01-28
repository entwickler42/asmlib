BITS 32

;------------------------------------------------------------------------------
import  ExitProcess Kernel32.dll
extern  ExitProcess

extern  strlen
extern  strncat

;------------------------------------------------------------------------------
section data use32 class=data

text_a  db  "alles fitt im schritt ?",0x0
text_b  db  "Hello World, ",0x0   
resb    100   

;------------------------------------------------------------------------------
section code use32 class=code

..start:
    push    ebp
    mov     ebp, esp        
            
    mov     eax, text_a
    call    strlen
    mov     ecx, eax   
    mov     ebx, text_a                     
    mov     eax, text_b
    call    strncat    
        
.exit:
    mov     esp, ebp
    pop     ebp    

    push    dword 0x0
    call    [ExitProcess]