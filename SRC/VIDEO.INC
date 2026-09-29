; ============================================================================
;  VIDEO.INC - Text screen output (direct video memory, 80x25)
; ============================================================================
;
;  The MDA text buffer is at B000:0000.  If a colour adapter is found the
;  program falls back to B800:0000 so it can still be used for testing.
;  Every cell is a word: low byte = character, high byte = attribute.

; ----------------------------------------------------------------------------
; video_init - detect the adapter, remember the cursor shape, clear screen
; ----------------------------------------------------------------------------
video_init:
        push    ax
        push    bx
        push    es
        mov     ah,0Fh                  ; get current video mode
        int     10h
        cmp     al,7                    ; 7 = MDA 80x25 monochrome
        jne     .color
        mov     word [vid_seg],0B000h
        jmp     .shape
.color:
        mov     word [vid_seg],0B800h
        cmp     al,2
        je      .shape
        cmp     al,3
        je      .shape
        mov     ax,0003h                ; set 80x25 text mode
        int     10h
.shape:
        xor     ax,ax
        mov     es,ax
        mov     ax,[es:0460h]           ; BIOS cursor shape
        test    ah,20h                  ; cursor disabled?
        jz      .store
        mov     ax,0607h
        cmp     word [vid_seg],0B000h
        jne     .store
        mov     ax,0B0Ch
.store:
        mov     [cur_shape],ax
        pop     es
        call    cursor_hide
        call    clear_screen
        pop     bx
        pop     ax
        ret

; ----------------------------------------------------------------------------
; video_done - restore a clean DOS screen before exit
; ----------------------------------------------------------------------------
video_done:
        push    dx
        call    clear_screen
        xor     dx,dx
        call    set_cursor
        call    cursor_show
        pop     dx
        ret

; ----------------------------------------------------------------------------
; vid_offset - DH = row, DL = column  ->  DI = offset in video memory
; ----------------------------------------------------------------------------
vid_offset:
        push    ax
        mov     al,80
        mul     dh
        add     al,dl
        adc     ah,0
        shl     ax,1
        mov     di,ax
        pop     ax
        ret

; ----------------------------------------------------------------------------
; put_char - AL = character, BL = attribute, DH/DL = row/column
; ----------------------------------------------------------------------------
put_char:
        push    ax
        push    di
        push    es
        call    vid_offset
        mov     es,[vid_seg]
        mov     ah,bl
        stosw
        pop     es
        pop     di
        pop     ax
        ret

; ----------------------------------------------------------------------------
; fill_chars - AL = character, BL = attribute, CX = count, DH/DL = position
; ----------------------------------------------------------------------------
fill_chars:
        push    ax
        push    cx
        push    di
        push    es
        call    vid_offset
        mov     es,[vid_seg]
        mov     ah,bl
        rep     stosw
        pop     es
        pop     di
        pop     cx
        pop     ax
        ret

; ----------------------------------------------------------------------------
; put_str - SI = zero terminated string, BL = attribute, DH/DL = position
;           DL is advanced past the string
; ----------------------------------------------------------------------------
put_str:
        push    ax
        push    si
        push    di
        push    es
        call    vid_offset
        mov     es,[vid_seg]
        mov     ah,bl
.next:
        lodsb
        or      al,al
        jz      .done
        stosw
        inc     dl
        jmp     .next
.done:
        pop     es
        pop     di
        pop     si
        pop     ax
        ret

; ----------------------------------------------------------------------------
; put_field - SI = string, CX = field width, BL = attribute, DH/DL = position
;             Writes at most CX characters and pads the rest with spaces.
; ----------------------------------------------------------------------------
put_field:
        push    ax
        push    cx
        push    si
        push    di
        push    es
        call    vid_offset
        mov     es,[vid_seg]
        mov     ah,bl
        jcxz    .done
.copy:
        lodsb
        or      al,al
        jz      .pad
        stosw
        loop    .copy
        jmp     .done
.pad:
        mov     al,' '
        rep     stosw
.done:
        pop     es
        pop     di
        pop     si
        pop     cx
        pop     ax
        ret

; ----------------------------------------------------------------------------
; put_center - SI = string, BL = attribute, DH = row (centred on 80 columns)
; ----------------------------------------------------------------------------
put_center:
        push    cx
        push    dx
        call    str_len
        mov     dl,80
        sub     dl,cl
        shr     dl,1
        call    put_str
        pop     dx
        pop     cx
        ret

; ----------------------------------------------------------------------------
; draw_lines - SI = list of zero terminated strings ended by 0FFh,
;              DH/DL = position of the first line, BL = attribute.
;              A line starting with 01h is drawn bright (heading).
; ----------------------------------------------------------------------------
draw_lines:
        push    bx
        push    cx
        push    dx
        push    si
.line:
        cmp     byte [si],0FFh
        je      .done
        push    bx
        push    dx
        cmp     byte [si],01h
        jne     .put
        inc     si
        mov     bl,A_BRIGHT
.put:
        call    put_str
        pop     dx
        pop     bx
        call    str_len
        add     si,cx
        inc     si
        inc     dh
        jmp     .line
.done:
        pop     si
        pop     dx
        pop     cx
        pop     bx
        ret

; ----------------------------------------------------------------------------
; clear_rows - DH = first row, CL = number of rows
; ----------------------------------------------------------------------------
clear_rows:
        push    ax
        push    bx
        push    cx
        push    dx
        mov     al,80
        mul     cl
        mov     cx,ax
        xor     dl,dl
        mov     al,' '
        mov     bl,A_NORMAL
        call    fill_chars
        pop     dx
        pop     cx
        pop     bx
        pop     ax
        ret

; ----------------------------------------------------------------------------
; clear_work - clear the work area (rows ROW_TOP..ROW_BOTTOM)
; ----------------------------------------------------------------------------
clear_work:
        push    cx
        push    dx
        mov     dh,ROW_TOP
        mov     cl,ROW_BOTTOM-ROW_TOP+1
        call    clear_rows
        pop     dx
        pop     cx
        ret

; ----------------------------------------------------------------------------
; clear_screen - clear the whole screen
; ----------------------------------------------------------------------------
clear_screen:
        push    cx
        push    dx
        xor     dh,dh
        mov     cl,25
        call    clear_rows
        pop     dx
        pop     cx
        ret

; ----------------------------------------------------------------------------
; save_screen - copy the screen to DS:DI (4000 bytes)
; ----------------------------------------------------------------------------
save_screen:
        push    cx
        push    si
        push    di
        push    ds
        mov     cx,2000
        mov     ds,[vid_seg]
        xor     si,si
        rep     movsw
        pop     ds
        pop     di
        pop     si
        pop     cx
        ret

; ----------------------------------------------------------------------------
; restore_screen - copy DS:SI (4000 bytes) back to the screen
; ----------------------------------------------------------------------------
restore_screen:
        push    cx
        push    si
        push    di
        push    es
        mov     cx,2000
        mov     es,[vid_seg]
        xor     di,di
        rep     movsw
        pop     es
        pop     di
        pop     si
        pop     cx
        ret

; ----------------------------------------------------------------------------
; set_cursor - DH = row, DL = column
; ----------------------------------------------------------------------------
set_cursor:
        push    ax
        push    bx
        mov     ah,02h
        xor     bh,bh
        int     10h
        pop     bx
        pop     ax
        ret

; ----------------------------------------------------------------------------
; cursor_hide / cursor_show
; ----------------------------------------------------------------------------
cursor_hide:
        push    ax
        push    cx
        push    dx
        mov     ah,01h
        mov     cx,2000h
        int     10h
        mov     dx,1900h                ; also park it below the screen
        call    set_cursor
        mov     byte [cur_visible],0
        pop     dx
        pop     cx
        pop     ax
        ret

cursor_show:
        push    ax
        push    cx
        mov     ah,01h
        mov     cx,[cur_shape]
        int     10h
        mov     byte [cur_visible],1
        pop     cx
        pop     ax
        ret

; ----------------------------------------------------------------------------
; draw_box - single line box, DH/DL = top/left, CH/CL = bottom/right,
;            BL = attribute. The inside is filled with spaces.
; ----------------------------------------------------------------------------
draw_box:
        push    ax
        push    cx
        push    dx
        push    bp
        mov     bp,cx
        mov     al,cl
        sub     al,dl
        dec     al
        xor     ah,ah
        mov     [box_w],ax
        mov     ax,0DAC4h               ; AH = corner, AL = line
        mov     cx,0BFh
        call    .row
.mid:
        inc     dh
        mov     ax,bp
        cmp     dh,ah
        jae     .last
        mov     ax,0B320h
        mov     cx,0B3h
        call    .row
        jmp     .mid
.last:
        mov     ax,0C0C4h
        mov     cx,0D9h
        call    .row
        pop     bp
        pop     dx
        pop     cx
        pop     ax
        ret
; draw one box row: AH = left char, AL = fill char, CL = right char
.row:
        push    ax
        push    cx
        push    dx
        push    ax
        mov     al,ah
        call    put_char
        pop     ax
        inc     dl
        push    cx
        mov     cx,[box_w]
        call    fill_chars
        add     dl,cl
        pop     cx
        mov     al,cl
        call    put_char
        pop     dx
        pop     cx
        pop     ax
        ret
