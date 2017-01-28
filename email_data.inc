EMAIL_SERVER:
    istruc sockaddr_in
        at sockaddr_in.sin_family,  dw AF_INET
        at sockaddr_in.sin_port,    dw 0x1900       ; htons(25) ...
        at sockaddr_in.sin_addr,    dd 0x00
        at sockaddr_in.sin_zero,    db 0,0,0,0,0,0,0,0
    iend    

T_EHLO          db "EHLO ",0x0
T_MAIL_FROM     db "MAIL FROM:<",0x0
T_RCPT_TO       db "RCPT TO:<",0x0
T_DATA          db "DATA",0x0D,0x0A,0x0
T_QUIT          db "QUIT",0x0D,0x0A,0x0
T_ENDL          db 0x0D,0x0A,0x0
T_BRK_CLOSE     db ">",0x0D,0x0A,0x0
T_FROM          db "From: ",0x0
T_SUBJECT       db "Subject: ",0x0
T_CONTENT_A     db "Content-Type: multipart/mixed;",0x0D,0x0A," boundary=",0x22,"------------001",0x22,0x0D,0x0A,"--------------001",0x0D,0x0A,0x0D,0x0A,0x0
T_CONTENT_B     db "Content-Type: application/octet-stream;",0x0D,0x0A," name=",0x0
T_CONTENT_C     db "Content-Transfer-Encoding: base64",0x0D,0x0A,"Content-Disposition: attachment;",0x0D,0x0A," filename=",0x0
T_CONTENT_D     db "",0x0
T_BONDARY       db "--------------001",0x0D,0x0A,0x0
T_BONDARY_END   db "--------------001--",0x0
T_MAIL_END      db 0x0D,0x0A,0x2E,0x0D,0x0A,0x0