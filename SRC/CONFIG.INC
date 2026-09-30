; ============================================================================
;  CONFIG.INC - Settings file (MARKET.CFG)
; ============================================================================
;
;  MARKET.CFG is a plain text file with KEY=VALUE lines, for example:
;
;      SOUND=ON
;      SAVER=ON
;      SAVERTIME=5
;
;  Lines starting with ';' are comments. The password is stored encoded
;  (hexadecimal, XOR scrambled) so it cannot be read at a glance.

; ----------------------------------------------------------------------------
; cfg_defaults - built-in default settings
; ----------------------------------------------------------------------------
cfg_defaults:
        push    si
        push    di
        mov     byte [cfg_store],0      ; no store name until one is set
        mov     si,def_currency
        mov     di,cfg_currency
        call    str_copy
        mov     byte [cfg_sound],1
        mov     byte [cfg_saver],1
        mov     byte [cfg_saver_min],5
        mov     byte [cfg_login],0
        mov     byte [cfg_pass],0
        mov     word [cfg_low],5
        mov     byte [cfg_backup],'A'
        pop     di
        pop     si
        ret

; ----------------------------------------------------------------------------
; cfg_load - read MARKET.CFG; the file is created when it does not exist
; ----------------------------------------------------------------------------
cfg_load:
        push    ax
        push    bx
        push    cx
        push    dx
        push    si
        push    di
        call    cfg_defaults
        mov     dx,fn_cfg
        call    fr_open
        jnc     .line
        call    cfg_save
        jmp     .done
.line:
        mov     di,line_buf
        mov     cx,LINE_MAX
        call    fr_readline
        jc      .eof
        mov     si,di
        call    str_trim
        cmp     byte [si],';'
        je      .line
        ; split at '='
.find:
        lodsb
        or      al,al
        jz      .line                   ; no '=' -> ignore the line
        cmp     al,'='
        jne     .find
        mov     byte [si-1],0
        mov     dx,si                   ; DX = value
        mov     si,di
        call    str_trim
        call    str_upper
        mov     si,dx
        call    str_trim
        ; look the key up in the table
        mov     bx,cfg_table
.key:
        mov     si,[bx]
        or      si,si
        jz      .line                   ; unknown key -> ignore
        mov     di,line_buf
        call    str_cmp
        je      .found
        add     bx,8
        jmp     .key
.found:
        mov     si,dx
        call    cfg_set_value
        jmp     .line
.eof:
        call    fr_close
        cmp     byte [cfg_pass],0       ; no password -> no login
        jne     .done
        mov     byte [cfg_login],0
.done:
        pop     di
        pop     si
        pop     dx
        pop     cx
        pop     bx
        pop     ax
        ret

; ----------------------------------------------------------------------------
; cfg_set_value - BX = table entry, SI = value text
; ----------------------------------------------------------------------------
cfg_set_value:
        push    ax
        push    cx
        push    dx
        push    si
        push    di
        mov     di,[bx+4]               ; variable
        mov     cx,[bx+6]               ; parameter
        mov     ax,[bx+2]               ; type
        cmp     al,T_STR
        jne     .bool
        push    di
.copy:
        jcxz    .copy_end
        lodsb
        or      al,al
        jz      .copy_end
        cmp     al,';'
        je      .copy_next
        stosb
        dec     cx
.copy_next:
        jmp     .copy
.copy_end:
        mov     byte [di],0
        pop     si
        call    str_trim
        jmp     .done
.bool:
        cmp     al,T_BOOL
        jne     .byte
        mov     al,[si]
        call    to_upper
        mov     byte [di],1
        cmp     al,'1'
        je      .done1
        cmp     al,'Y'
        je      .done1
        cmp     al,'T'
        je      .done1
        cmp     al,'O'
        jne     .false
        mov     al,[si+1]
        call    to_upper
        cmp     al,'N'
        je      .done1
.false:
        mov     byte [di],0
.done1:
        jmp     .done
.byte:
        cmp     al,T_BYTE
        jne     .word
        call    parse_u32
        jc      .done
        or      dx,dx
        jnz     .done
        or      ax,ax
        jz      .done
        cmp     ax,cx
        ja      .done
        mov     [di],al
        jmp     .done
.word:
        cmp     al,T_WORD
        jne     .drive
        call    parse_u32
        jc      .done
        or      dx,dx
        jnz     .done
        cmp     ax,cx
        ja      .done
        mov     [di],ax
        jmp     .done
.drive:
        cmp     al,T_DRIVE
        jne     .pass
        mov     al,[si]
        call    to_upper
        cmp     al,'A'
        jb      .done
        cmp     al,'Z'
        ja      .done
        mov     [di],al
        jmp     .done
.pass:
        ; decode hexadecimal, XOR scrambled password
        xor     dx,dx                   ; DL = position
.pchar:
        cmp     byte [si],0
        je      .pend
        cmp     dl,PASS_LEN
        jae     .pend
        call    hex_parse
        jc      .pbad
        call    pass_key
        xor     al,ah
        or      al,al
        jz      .pbad
        stosb
        inc     dl
        jmp     .pchar
.pbad:
        mov     di,[bx+4]               ; invalid -> no password
.pend:
        mov     byte [di],0
.done:
        pop     di
        pop     si
        pop     dx
        pop     cx
        pop     ax
        ret

; pass_key - DL = position  ->  AH = scramble key for that position
pass_key:
        push    ax
        mov     al,17
        mul     dl
        add     al,5Ah
        mov     [pk_tmp],al
        pop     ax
        mov     ah,[pk_tmp]
        ret

; ----------------------------------------------------------------------------
; cfg_save - write MARKET.CFG. CF=1 on error
; ----------------------------------------------------------------------------
cfg_save:
        push    ax
        push    bx
        push    cx
        push    dx
        push    si
        push    di
        mov     dx,fn_cfg
        call    fw_create
        jnc     .jf1
        jmp     .done
.jf1:
        mov     si,cfg_header
        call    fw_puts
        mov     bx,cfg_table
.entry:
        mov     si,[bx]
        or      si,si
        jz      .close
        call    fw_puts
        mov     al,'='
        call    fw_putc
        mov     si,[bx+4]
        mov     ax,[bx+2]
        cmp     al,T_STR
        jne     .bool
        call    fw_puts
        jmp     .eol
.bool:
        cmp     al,T_BOOL
        jne     .byte
        push    si
        cmp     byte [si],0
        mov     si,str_on
        jne     .bool_put
        mov     si,str_off
.bool_put:
        call    fw_puts
        pop     si
        jmp     .eol
.byte:
        cmp     al,T_BYTE
        jne     .word
        mov     al,[si]
        xor     ah,ah
        jmp     .number
.word:
        cmp     al,T_WORD
        jne     .drive
        mov     ax,[si]
.number:
        mov     di,num_str
        call    u16_append
        mov     si,num_str
        call    fw_puts
        jmp     .eol
.drive:
        cmp     al,T_DRIVE
        jne     .pass
        mov     al,[si]
        call    fw_putc
        jmp     .eol
.pass:
        xor     dx,dx
.pchar:
        mov     al,[si]
        or      al,al
        jz      .eol
        call    pass_key
        xor     al,ah
        mov     di,num_str
        call    hex_append
        push    si
        mov     si,num_str
        call    fw_puts
        pop     si
        inc     si
        inc     dl
        jmp     .pchar
.eol:
        call    fw_crlf
        add     bx,8
        jmp     .entry
.close:
        call    fw_close
.done:
        pop     di
        pop     si
        pop     dx
        pop     cx
        pop     bx
        pop     ax
        ret
