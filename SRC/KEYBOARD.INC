; ============================================================================
;  KEYBOARD.INC - Keyboard input, idle tasks, PC speaker and screen saver
; ============================================================================
;
;  A barcode reader in "keyboard wedge" mode behaves exactly like a keyboard:
;  it types the code and presses Enter, so no special driver is needed.

; ----------------------------------------------------------------------------
; kbd_init - use the enhanced BIOS functions when a 101/102-key keyboard
;            is present (needed for F11/F12), otherwise the XT functions.
; ----------------------------------------------------------------------------
kbd_init:
        push    ax
        push    es
        xor     ax,ax
        mov     es,ax
        test    byte [es:0496h],10h     ; enhanced keyboard installed?
        jz      .xt
        mov     word [kb_funcs],1110h   ; AH=11h check, AH=10h read
.xt:
        pop     es
        pop     ax
        ret

; ----------------------------------------------------------------------------
; get_key - wait for a key while doing idle work (clock, screen saver)
;           Returns AL = ASCII code (0 for extended keys), AH = scan code
; ----------------------------------------------------------------------------
get_key:
.wait:
        mov     ah,byte [kb_funcs+1]    ; check for keystroke
        int     16h
        jnz     .have
        call    idle_tasks
        sti
        hlt                             ; sleep until the next interrupt
        jmp     .wait
.have:
        mov     ah,byte [kb_funcs]      ; read keystroke
        int     16h
        cmp     al,0E0h                 ; grey keys on enhanced keyboards
        jne     .done
        or      ah,ah
        jz      .done
        xor     al,al
.done:
        call    reset_idle
        ret

; ----------------------------------------------------------------------------
; get_ticks - DX:AX = BIOS timer tick count (18.2 per second)
; ----------------------------------------------------------------------------
get_ticks:
        push    ds
        xor     ax,ax
        mov     ds,ax
        cli
        mov     ax,[046Ch]
        mov     dx,[046Eh]
        sti
        pop     ds
        ret

; ----------------------------------------------------------------------------
; reset_idle - restart the screen saver countdown
; ----------------------------------------------------------------------------
reset_idle:
        push    ax
        push    dx
        call    get_ticks
        mov     [idle_lo],ax
        mov     [idle_hi],dx
        pop     dx
        pop     ax
        ret

; ----------------------------------------------------------------------------
; idle_tasks - called while waiting for a key
; ----------------------------------------------------------------------------
idle_tasks:
        push    ax
        push    bx
        push    dx
        cmp     byte [in_saver],0
        jne     .done
        call    update_clock
        cmp     byte [cfg_saver],0
        je      .done
        call    get_ticks
        sub     ax,[idle_lo]
        sbb     dx,[idle_hi]
        jc      .reset                  ; counter wrapped at midnight
        or      dx,dx
        jnz     .fire
        mov     bx,ax
        mov     al,[cfg_saver_min]
        xor     ah,ah
        mov     dx,TICKS_PER_MIN
        mul     dx
        cmp     bx,ax
        jb      .done
.fire:
        call    screen_saver
        jmp     .done
.reset:
        call    reset_idle
.done:
        pop     dx
        pop     bx
        pop     ax
        ret

; ----------------------------------------------------------------------------
; wait_ticks - wait CX timer ticks
; ----------------------------------------------------------------------------
wait_ticks:
        push    ax
        push    bx
        push    dx
        call    get_ticks
        mov     bx,ax
.wait:
        sti
        hlt
        call    get_ticks
        sub     ax,bx
        cmp     ax,cx
        jb      .wait
        pop     dx
        pop     bx
        pop     ax
        ret

; ----------------------------------------------------------------------------
; sound_tone - BX = PIT divisor (1193182 / frequency), CX = duration (ticks)
;              Does nothing when the PC speaker is disabled in the settings.
; ----------------------------------------------------------------------------
sound_tone:
        cmp     byte [cfg_sound],0
        je      .off
        push    ax
        mov     al,0B6h                 ; PIT channel 2, square wave
        out     43h,al
        mov     ax,bx
        out     42h,al
        mov     al,ah
        out     42h,al
        in      al,61h
        or      al,03h                  ; speaker on
        out     61h,al
        call    wait_ticks
        in      al,61h
        and     al,0FCh                 ; speaker off
        out     61h,al
        pop     ax
.off:
        ret

beep_ok:
        push    bx
        push    cx
        mov     bx,1193                 ; 1000 Hz
        mov     cx,2
        call    sound_tone
        pop     cx
        pop     bx
        ret

beep_err:
        push    bx
        push    cx
        mov     bx,5423                 ; 220 Hz
        mov     cx,5
        call    sound_tone
        pop     cx
        pop     bx
        ret

; ----------------------------------------------------------------------------
; rand16 - AX = pseudo random number
; ----------------------------------------------------------------------------
rand16:
        push    dx
        mov     ax,[rnd_seed]
        mov     dx,25173
        mul     dx
        add     ax,13849
        mov     [rnd_seed],ax
        xchg    al,ah
        pop     dx
        ret

; ----------------------------------------------------------------------------
; screen_saver - blank the screen and show a moving message until a key is
;                pressed. When password login is enabled the password must
;                be entered to return to the program.
; ----------------------------------------------------------------------------
screen_saver:
        push    ax
        push    bx
        push    cx
        push    dx
        push    si
        push    di
        mov     byte [in_saver],1
        mov     ah,03h                  ; save cursor position
        xor     bh,bh
        int     10h
        mov     [ss_cursor],dx
        mov     al,[cur_visible]
        mov     [ss_cur_vis],al
        mov     di,ss_buf
        call    save_screen
.show:
        call    cursor_hide
        call    saver_animate
        call    password_needed
        jc      .restore
        call    clear_screen
        call    ask_password
        jc      .show
        or      al,al
        jnz     .restore
        call    beep_err
        jmp     .show
.restore:
        mov     si,ss_buf
        call    restore_screen
        mov     dx,[ss_cursor]
        call    set_cursor
        cmp     byte [ss_cur_vis],0
        je      .nocursor
        call    cursor_show
.nocursor:
        mov     byte [in_saver],0
        call    reset_idle
        pop     di
        pop     si
        pop     dx
        pop     cx
        pop     bx
        pop     ax
        ret

; show the store name and a hint at random places until a key is pressed
saver_animate:
        push    ax
        push    bx
        push    cx
        push    dx
        push    si
.frame:
        call    clear_screen
        call    rand16
        xor     dx,dx
        mov     bx,24
        div     bx
        mov     [ss_row],dl
        mov     si,cfg_store
        call    str_len
        cmp     cx,msg_saver_len
        jae     .width
        mov     cx,msg_saver_len
.width:
        mov     bx,81
        sub     bx,cx
        call    rand16
        xor     dx,dx
        div     bx
        mov     dh,[ss_row]
        push    dx
        mov     bl,A_BRIGHT
        call    put_str
        pop     dx
        inc     dh
        mov     si,msg_saver
        mov     bl,A_NORMAL
        call    put_str
        call    get_ticks
        mov     [ss_t0],ax
.wait:
        mov     ah,byte [kb_funcs+1]
        int     16h
        jnz     .key
        sti
        hlt
        call    get_ticks
        sub     ax,[ss_t0]
        cmp     ax,91                   ; move every 5 seconds
        jb      .wait
        jmp     .frame
.key:
        mov     ah,byte [kb_funcs]      ; swallow the key
        int     16h
        pop     si
        pop     dx
        pop     cx
        pop     bx
        pop     ax
        ret
