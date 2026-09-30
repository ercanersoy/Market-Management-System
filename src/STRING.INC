; ============================================================================
;  STRING.INC - String and number routines
; ============================================================================
;
;  All strings are zero terminated and live in the program segment (DS=ES).
;  Prices are kept as 32-bit integers in cents; totals use 48-bit integers
;  (three words, least significant first).

; ----------------------------------------------------------------------------
; str_len - SI = string  ->  CX = length
; ----------------------------------------------------------------------------
str_len:
        push    ax
        push    si
        xor     cx,cx
.next:
        lodsb
        or      al,al
        jz      .done
        inc     cx
        jmp     .next
.done:
        pop     si
        pop     ax
        ret

; ----------------------------------------------------------------------------
; str_copy - copy string SI to DI (including terminator)
; ----------------------------------------------------------------------------
str_copy:
        push    ax
        push    si
        push    di
.next:
        lodsb
        stosb
        or      al,al
        jnz     .next
        pop     di
        pop     si
        pop     ax
        ret

; ----------------------------------------------------------------------------
; str_append - copy string SI to DI; DI is left on the new terminator
; ----------------------------------------------------------------------------
str_append:
        push    ax
        push    si
.next:
        lodsb
        stosb
        or      al,al
        jnz     .next
        dec     di
        pop     si
        pop     ax
        ret

; ----------------------------------------------------------------------------
; chr_append - store AL at DI and terminate; DI is left on the terminator
; ----------------------------------------------------------------------------
chr_append:
        stosb
        mov     byte [di],0
        ret

; ----------------------------------------------------------------------------
; str_cmp - compare SI and DI (case sensitive), ZF=1 if equal
; ----------------------------------------------------------------------------
str_cmp:
        push    ax
        push    si
        push    di
.next:
        mov     al,[si]
        cmp     al,[di]
        jne     .done
        or      al,al
        jz      .done
        inc     si
        inc     di
        jmp     .next
.done:
        pop     di
        pop     si
        pop     ax
        ret

; ----------------------------------------------------------------------------
; str_cmp_ci - compare SI and DI ignoring case, ZF=1 if equal
; ----------------------------------------------------------------------------
str_cmp_ci:
        push    ax
        push    si
        push    di
.next:
        mov     al,[di]
        call    to_upper
        mov     ah,al
        mov     al,[si]
        call    to_upper
        cmp     al,ah
        jne     .done
        or      al,al
        jz      .done
        inc     si
        inc     di
        jmp     .next
.done:
        pop     di
        pop     si
        pop     ax
        ret

; ----------------------------------------------------------------------------
; str_trim - remove leading and trailing blanks from string SI (in place)
; ----------------------------------------------------------------------------
str_trim:
        push    ax
        push    si
        push    di
        mov     di,si
.skip:
        cmp     byte [si],0
        je      .copy
        cmp     byte [si],' '
        ja      .copy
        inc     si
        jmp     .skip
.copy:
        lodsb
        stosb
        or      al,al
        jnz     .copy
        dec     di                      ; DI -> terminator
        pop     si
        push    si
.back:
        cmp     di,si
        je      .done
        cmp     byte [di-1],' '
        ja      .done
        dec     di
        mov     byte [di],0
        jmp     .back
.done:
        pop     di
        pop     si
        pop     ax
        ret

; ----------------------------------------------------------------------------
; str_upper - convert string SI to upper case (in place)
; ----------------------------------------------------------------------------
str_upper:
        push    ax
        push    si
.next:
        mov     al,[si]
        or      al,al
        jz      .done
        call    to_upper
        mov     [si],al
        inc     si
        jmp     .next
.done:
        pop     si
        pop     ax
        ret

; ----------------------------------------------------------------------------
; to_upper - convert AL to upper case (ASCII letters only)
; ----------------------------------------------------------------------------
to_upper:
        cmp     al,'a'
        jb      .done
        cmp     al,'z'
        ja      .done
        sub     al,'a'-'A'
.done:
        ret

; ----------------------------------------------------------------------------
; u32_mul10 - DX:AX = DX:AX * 10, CF=1 on overflow
; ----------------------------------------------------------------------------
u32_mul10:
        push    bx
        push    cx
        mov     bx,dx
        mov     cx,10
        mul     cx                      ; DX:AX = low * 10
        push    ax
        push    dx
        mov     ax,bx
        mul     cx                      ; DX:AX = high * 10
        or      dx,dx
        pop     dx
        jnz     .ovf_pop
        add     dx,ax
        pop     ax
        jc      .ovf
        clc
        jmp     .done
.ovf_pop:
        pop     ax
.ovf:
        stc
.done:
        pop     cx
        pop     bx
        ret

; ----------------------------------------------------------------------------
; parse_u32 - SI = decimal string  ->  DX:AX, CF=1 if invalid or empty
; ----------------------------------------------------------------------------
parse_u32:
        push    bx
        push    si
        xor     ax,ax
        xor     dx,dx
        cmp     byte [si],0
        je      .bad
.next:
        mov     bl,[si]
        or      bl,bl
        jz      .ok
        sub     bl,'0'
        cmp     bl,9
        ja      .bad
        call    u32_mul10
        jc      .bad
        xor     bh,bh
        add     ax,bx
        adc     dx,0
        jc      .bad
        inc     si
        jmp     .next
.ok:
        clc
        jmp     .done
.bad:
        stc
.done:
        pop     si
        pop     bx
        ret

; ----------------------------------------------------------------------------
; parse_qty - SI = decimal string  ->  AX (0..65535), CF=1 if invalid
; ----------------------------------------------------------------------------
parse_qty:
        push    dx
        call    parse_u32
        jc      .done
        or      dx,dx
        jz      .done
        stc
.done:
        pop     dx
        ret

; ----------------------------------------------------------------------------
; parse_price - SI = "123", "123.4" or "123.45"  ->  DX:AX = cents,
;               CF=1 if invalid or larger than MAX_PRICE
; ----------------------------------------------------------------------------
parse_price:
        push    bx
        push    cx
        push    si
        xor     ax,ax
        xor     dx,dx
        mov     cx,00FFh                ; CL = decimals (FFh: no point yet)
                                        ; CH = digit seen flag
.next:
        mov     bl,[si]
        inc     si
        or      bl,bl
        jz      .end
        cmp     bl,'.'
        je      .point
        cmp     bl,','
        je      .point
        sub     bl,'0'
        cmp     bl,9
        ja      .bad
        cmp     cl,0FFh
        je      .digit
        cmp     cl,2
        jae     .bad
        inc     cl
.digit:
        call    u32_mul10
        jc      .bad
        xor     bh,bh
        add     ax,bx
        adc     dx,0
        jc      .bad
        mov     ch,1
        jmp     .next
.point:
        cmp     cl,0FFh
        jne     .bad
        xor     cl,cl
        jmp     .next
.end:
        or      ch,ch
        jz      .bad
        cmp     cl,0FFh
        jne     .scale
        xor     cl,cl
.scale:
        cmp     cl,2
        jae     .check
        call    u32_mul10
        jc      .bad
        inc     cl
        jmp     .scale
.check:
        cmp     dx,MAX_PRICE shr 16
        ja      .bad
        jb      .ok
        cmp     ax,MAX_PRICE and 0FFFFh
        ja      .bad
.ok:
        clc
        jmp     .done
.bad:
        stc
.done:
        pop     si
        pop     cx
        pop     bx
        ret

; ----------------------------------------------------------------------------
; u32_to_str - DX:AX = number, BL = decimals (0 or 2), DI = output
;              DI is left on the terminator
; n48_to_str - same for the 48-bit number at SI
; u16_append - AX = number, DI = output (DI is left on the terminator)
; ----------------------------------------------------------------------------
u16_append:
        push    bx
        push    dx
        xor     dx,dx
        xor     bl,bl
        call    u32_to_str
        pop     dx
        pop     bx
        ret

u32_to_str:
        mov     [num_tmp],ax
        mov     [num_tmp+2],dx
        mov     word [num_tmp+4],0
        jmp     num_tmp_to_str

n48_to_str:
        push    ax
        mov     ax,[si]
        mov     [num_tmp],ax
        mov     ax,[si+2]
        mov     [num_tmp+2],ax
        mov     ax,[si+4]
        mov     [num_tmp+4],ax
        pop     ax

num_tmp_to_str:
        push    ax
        push    cx
        push    dx
        xor     cx,cx                   ; digit count
.divide:
        xor     dx,dx                   ; (zero high words are skipped)
        mov     ax,[num_tmp+4]
        or      ax,ax
        jz      .middle
        div     word [w_ten]
        mov     [num_tmp+4],ax
.middle:
        mov     ax,[num_tmp+2]
        or      ax,ax
        jnz     .div_middle
        or      dx,dx
        jz      .low
.div_middle:
        div     word [w_ten]
        mov     [num_tmp+2],ax
.low:
        mov     ax,[num_tmp]
        div     word [w_ten]
        mov     [num_tmp],ax
        add     dl,'0'
        push    dx
        inc     cx
        mov     ax,[num_tmp]
        or      ax,[num_tmp+2]
        or      ax,[num_tmp+4]
        jnz     .divide
        mov     al,bl
        xor     ah,ah
        cmp     cx,ax                   ; need at least decimals+1 digits
        jbe     .divide
.output:
        pop     ax
        stosb
        dec     cx
        jz      .done
        cmp     cl,bl
        jne     .output
        mov     al,'.'
        stosb
        jmp     .output
.done:
        mov     byte [di],0
        pop     dx
        pop     cx
        pop     ax
        ret

; ----------------------------------------------------------------------------
; dec2_append - AL = 0..99, stored as two digits at DI
; ----------------------------------------------------------------------------
dec2_append:
        push    ax
        aam                             ; AH = tens, AL = ones
        add     ax,3030h
        xchg    al,ah
        stosw
        mov     byte [di],0
        pop     ax
        ret

; ----------------------------------------------------------------------------
; hex_append - AL = byte, stored as two hex digits at DI
; ----------------------------------------------------------------------------
hex_append:
        push    ax
        push    cx
        mov     ah,al
        mov     cl,4
        shr     al,cl
        call    .digit
        mov     al,ah
        and     al,0Fh
        call    .digit
        mov     byte [di],0
        pop     cx
        pop     ax
        ret
.digit:
        add     al,'0'
        cmp     al,'9'
        jbe     .store
        add     al,'A'-'9'-1
.store:
        stosb
        ret

; ----------------------------------------------------------------------------
; hex_parse - SI = two hex digits  ->  AL, SI advanced, CF=1 if invalid
; ----------------------------------------------------------------------------
hex_parse:
        push    cx
        push    dx
        call    .nibble
        jc      .done
        mov     cl,4
        shl     al,cl
        mov     dl,al
        call    .nibble
        jc      .done
        or      al,dl
        clc
.done:
        pop     dx
        pop     cx
        ret
.nibble:
        lodsb
        call    to_upper
        sub     al,'0'
        jb      .bad
        cmp     al,9
        jbe     .ok
        sub     al,'A'-'0'-10
        cmp     al,10
        jb      .bad
        cmp     al,15
        ja      .bad
.ok:
        clc
        ret
.bad:
        stc
        ret

; ----------------------------------------------------------------------------
; 48-bit arithmetic (DI = destination, SI = source)
; ----------------------------------------------------------------------------
; zero48 - [DI] = 0
zero48:
        mov     word [di],0
        mov     word [di+2],0
        mov     word [di+4],0
        ret

; add48 - [DI] = [DI] + [SI]
add48:
        push    ax
        mov     ax,[si]
        add     [di],ax
        mov     ax,[si+2]
        adc     [di+2],ax
        mov     ax,[si+4]
        adc     [di+4],ax
        pop     ax
        ret

; sub48 - [DI] = [DI] - [SI], CF=1 on borrow (result negative)
sub48:
        push    ax
        mov     ax,[si]
        sub     [di],ax
        mov     ax,[si+2]
        sbb     [di+2],ax
        mov     ax,[si+4]
        sbb     [di+4],ax
        pop     ax
        ret

; mul32x16 - [DI] = DX:AX * BX
mul32x16:
        push    ax
        push    cx
        push    dx
        mov     cx,dx
        mul     bx
        mov     [di],ax
        mov     [di+2],dx
        mov     ax,cx
        mul     bx
        add     [di+2],ax
        adc     dx,0
        mov     [di+4],dx
        pop     dx
        pop     cx
        pop     ax
        ret
