; ============================================================================
;  FILE.INC - Buffered text file I/O through MS-DOS handles
; ============================================================================

; ----------------------------------------------------------------------------
; fr_open - DX = file name, open for reading. CF=1 on error
; ----------------------------------------------------------------------------
fr_open:
        push    ax
        mov     ax,3D00h
        int     21h
        jc      .done
        mov     [fr_handle],ax
        xor     ax,ax
        mov     [fr_pos],ax
        mov     [fr_len],ax
        mov     [fr_eof],al
        clc
.done:
        pop     ax
        ret

; ----------------------------------------------------------------------------
; fr_getc - AL = next character, CF=1 at end of file (AH is destroyed)
; ----------------------------------------------------------------------------
fr_getc:
        push    bx
        push    cx
        push    dx
        push    si
        mov     si,[fr_pos]
        cmp     si,[fr_len]
        jb      .have
        cmp     byte [fr_eof],0
        jne     .eof
        mov     ah,3Fh
        mov     bx,[fr_handle]
        mov     cx,FBUF_SIZE
        mov     dx,fbuf
        int     21h
        jc      .set_eof
        or      ax,ax
        jz      .set_eof
        mov     [fr_len],ax
        xor     si,si
.have:
        mov     al,[fbuf+si]
        inc     si
        mov     [fr_pos],si
        cmp     al,1Ah                  ; Ctrl+Z ends a text file
        je      .set_eof
        clc
        jmp     .done
.set_eof:
        mov     byte [fr_eof],1
        mov     ax,[fr_len]
        mov     [fr_pos],ax
.eof:
        stc
.done:
        pop     si
        pop     dx
        pop     cx
        pop     bx
        ret

; ----------------------------------------------------------------------------
; fr_readline - read one line into DI (at most CX characters, the rest of a
;               long line is skipped). CF=1 at end of file.
; ----------------------------------------------------------------------------
fr_readline:
        push    ax
        push    bx
        push    dx
        xor     bx,bx                   ; characters stored
        xor     dl,dl                   ; anything read?
.next:
        call    fr_getc
        jc      .eof
        mov     dl,1
        cmp     al,0Ah
        je      .eol
        cmp     al,0Dh
        je      .next
        cmp     bx,cx
        jae     .next
        mov     [bx+di],al
        inc     bx
        jmp     .next
.eof:
        or      dl,dl
        jz      .none
.eol:
        mov     byte [bx+di],0
        clc
        jmp     .done
.none:
        mov     byte [di],0
        stc
.done:
        pop     dx
        pop     bx
        pop     ax
        ret

; ----------------------------------------------------------------------------
; fr_close - close the input file
; ----------------------------------------------------------------------------
fr_close:
        push    ax
        push    bx
        mov     ah,3Eh
        mov     bx,[fr_handle]
        int     21h
        pop     bx
        pop     ax
        ret

; ----------------------------------------------------------------------------
; fw_create - DX = file name, create/truncate for writing. CF=1 on error
; ----------------------------------------------------------------------------
fw_create:
        push    ax
        push    cx
        mov     ah,3Ch
        xor     cx,cx
        int     21h
        jc      .done
        mov     [fw_handle],ax
        mov     word [fw_len],0
        mov     byte [fw_err],0
        clc
.done:
        pop     cx
        pop     ax
        ret

; ----------------------------------------------------------------------------
; fw_putc - write character AL
; ----------------------------------------------------------------------------
fw_putc:
        push    bx
        mov     bx,[fw_len]
        mov     [fbuf+bx],al
        inc     bx
        mov     [fw_len],bx
        cmp     bx,FBUF_SIZE
        jb      .done
        call    fw_flush
.done:
        pop     bx
        ret

; ----------------------------------------------------------------------------
; fw_puts - write string SI;  fw_crlf - write end of line
; fw_line - write string SI followed by end of line
; fw_repeat - write character AL, CX times
; ----------------------------------------------------------------------------
fw_puts:
        push    ax
        push    bx
        push    si
        mov     bx,[fw_len]
.next:
        lodsb
        or      al,al
        jz      .done
        mov     [fbuf+bx],al
        inc     bx
        cmp     bx,FBUF_SIZE
        jb      .next
        mov     [fw_len],bx
        call    fw_flush
        xor     bx,bx
        jmp     .next
.done:
        mov     [fw_len],bx
        pop     si
        pop     bx
        pop     ax
        ret

; fw_puts_far - write the string at rec_seg:SI
fw_puts_far:
        push    ds
        mov     ds,[rec_seg]
        call    .copy
        pop     ds
        ret
.copy:                                  ; DS = record segment here, so the
        push    ax                      ; variables are reached through CS
        push    bx
        push    si
        mov     bx,[cs:fw_len]
.next:
        lodsb
        or      al,al
        jz      .done
        mov     [cs:fbuf+bx],al
        inc     bx
        cmp     bx,FBUF_SIZE
        jb      .next
        mov     [cs:fw_len],bx
        push    ds
        push    cs
        pop     ds
        call    fw_flush
        pop     ds
        xor     bx,bx
        jmp     .next
.done:
        mov     [cs:fw_len],bx
        pop     si
        pop     bx
        pop     ax
        ret

fw_crlf:
        push    ax
        mov     al,0Dh
        call    fw_putc
        mov     al,0Ah
        call    fw_putc
        pop     ax
        ret

fw_line:
        call    fw_puts
        jmp     fw_crlf

fw_repeat:
        push    cx
        jcxz    .done
.next:
        call    fw_putc
        loop    .next
.done:
        pop     cx
        ret

; ----------------------------------------------------------------------------
; fw_flush - write the buffer to the file
; ----------------------------------------------------------------------------
fw_flush:
        push    ax
        push    bx
        push    cx
        push    dx
        mov     cx,[fw_len]
        jcxz    .done
        mov     ah,40h
        mov     bx,[fw_handle]
        mov     dx,fbuf
        int     21h
        jc      .error
        cmp     ax,cx                   ; disk full?
        je      .written
.error:
        mov     byte [fw_err],1
.written:
        mov     word [fw_len],0
.done:
        pop     dx
        pop     cx
        pop     bx
        pop     ax
        ret

; ----------------------------------------------------------------------------
; fw_close - flush and close the output file. CF=1 if anything failed
; ----------------------------------------------------------------------------
fw_close:
        push    ax
        push    bx
        call    fw_flush
        mov     ah,3Eh
        mov     bx,[fw_handle]
        int     21h
        jc      .done
        cmp     byte [fw_err],0
        je      .done                   ; CF=0 from the compare
        stc
.done:
        pop     bx
        pop     ax
        ret

; ----------------------------------------------------------------------------
; file_exists - DX = file name, CF=1 if the file does not exist
; ----------------------------------------------------------------------------
file_exists:
        push    ax
        push    cx
        mov     ax,4300h
        int     21h
        pop     cx
        pop     ax
        ret

; ----------------------------------------------------------------------------
; copy_file - copy file SI to file DI. CF=1 on error
; ----------------------------------------------------------------------------
copy_file:
        push    ax
        push    bx
        push    cx
        push    dx
        mov     dx,si
        mov     ax,3D00h
        int     21h
        jc      .fail
        mov     [cp_src],ax
        mov     dx,di
        mov     ah,3Ch
        xor     cx,cx
        int     21h
        jc      .fail_src
        mov     [cp_dst],ax
.copy:
        mov     ah,3Fh
        mov     bx,[cp_src]
        mov     cx,FBUF_SIZE
        mov     dx,fbuf
        int     21h
        jc      .fail_both
        or      ax,ax
        jz      .end
        mov     cx,ax
        mov     ah,40h
        mov     bx,[cp_dst]
        mov     dx,fbuf
        int     21h
        jc      .fail_both
        cmp     ax,cx
        jne     .fail_both
        jmp     .copy
.end:
        mov     ah,3Eh
        mov     bx,[cp_dst]
        int     21h
        jc      .fail_dst_closed
        mov     ah,3Eh
        mov     bx,[cp_src]
        int     21h
        clc
        jmp     .done
.fail_both:
        mov     ah,3Eh
        mov     bx,[cp_dst]
        int     21h
.fail_dst_closed:
        mov     ah,41h                  ; remove the incomplete copy
        mov     dx,di
        int     21h
.fail_src:
        mov     ah,3Eh
        mov     bx,[cp_src]
        int     21h
.fail:
        stc
.done:
        pop     dx
        pop     cx
        pop     bx
        pop     ax
        ret
