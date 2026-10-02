; ============================================================================
;  UI.INC - Text user interface building blocks
; ============================================================================
;
;  Screen layout:
;      row 0      title bar (program, store name, date and time)
;      row 1      section title
;      row 2      separator
;      rows 3-21  work area
;      row 22     separator
;      row 23     status / message line
;      row 24     key hints

; ----------------------------------------------------------------------------
; draw_frame - clear the screen and draw the common frame
;              SI = section title, DI = key hint text
; ----------------------------------------------------------------------------
draw_frame:
        push    ax
        push    bx
        push    cx
        push    dx
        push    si
        call    cursor_hide
        call    clear_screen
        call    draw_title
        mov     dx,(ROW_SECTION shl 8) + 1
        mov     bl,A_BRIGHT
        call    put_str
        mov     al,0C4h
        mov     bl,A_NORMAL
        mov     cx,80
        mov     dx,(ROW_TOP-1) shl 8
        call    fill_chars
        mov     dx,(ROW_BOTTOM+1) shl 8
        call    fill_chars
        mov     si,di
        call    draw_hint
        mov     si,[pending_msg]
        or      si,si
        jz      .done
        mov     word [pending_msg],0
        call    show_info
.done:
        pop     si
        pop     dx
        pop     cx
        pop     bx
        pop     ax
        ret

; ----------------------------------------------------------------------------
; draw_hint - SI = key hint text for the bottom line
; ----------------------------------------------------------------------------
draw_hint:
        push    ax
        push    bx
        push    cx
        push    dx
        mov     dx,ROW_HINT shl 8
        mov     al,' '
        mov     bl,A_REVERSE
        mov     cx,80
        call    fill_chars
        inc     dl
        call    put_str
        pop     dx
        pop     cx
        pop     bx
        pop     ax
        ret

; ----------------------------------------------------------------------------
; draw_title - title bar with the store name and the clock
; ----------------------------------------------------------------------------
draw_title:
        push    ax
        push    bx
        push    cx
        push    dx
        push    si
        mov     dx,ROW_TITLE shl 8
        mov     al,' '
        mov     bl,A_REVERSE
        mov     cx,80
        call    fill_chars
        inc     dl
        mov     si,str_app_title
        call    put_str
        cmp     byte [cfg_store],0
        je      .clock
        mov     si,str_title_sep
        call    put_str
        mov     si,cfg_store
        call    put_str
.clock:
        mov     word [clock_last],0FFFFh
        call    update_clock
        pop     si
        pop     dx
        pop     cx
        pop     bx
        pop     ax
        ret

; ----------------------------------------------------------------------------
; update_clock - redraw date and time when the minute has changed
; ----------------------------------------------------------------------------
update_clock:
        push    ax
        push    bx
        push    cx
        push    dx
        push    si
        push    di
        mov     ah,2Ch
        int     21h
        cmp     cx,[clock_last]
        je      .done
        mov     [clock_last],cx
        mov     di,clk_buf
        call    date_append
        mov     al,' '
        call    chr_append
        call    time_append
        mov     si,clk_buf
        mov     dx,(ROW_TITLE shl 8) + 63
        mov     bl,A_REVERSE
        call    put_str
.done:
        pop     di
        pop     si
        pop     dx
        pop     cx
        pop     bx
        pop     ax
        ret

; ----------------------------------------------------------------------------
; date_append - "YYYY-MM-DD" at DI;  time_append - "HH:MM" at DI
; ----------------------------------------------------------------------------
date_append:
        push    ax
        push    cx
        push    dx
        mov     ah,2Ah
        int     21h
        mov     ax,cx
        call    u16_append
        mov     al,'-'
        stosb
        mov     al,dh
        call    dec2_append
        mov     al,'-'
        stosb
        mov     al,dl
        call    dec2_append
        pop     dx
        pop     cx
        pop     ax
        ret

time_append:
        push    ax
        push    cx
        push    dx
        mov     ah,2Ch
        int     21h
        mov     al,ch
        call    dec2_append
        mov     al,':'
        stosb
        mov     al,cl
        call    dec2_append
        pop     dx
        pop     cx
        pop     ax
        ret

; ----------------------------------------------------------------------------
; Status line messages (SI = text)
;   show_info  - normal message
;   show_ok    - normal message with a short high beep
;   show_error - bright message with a low beep
;   clear_status
; ----------------------------------------------------------------------------
show_info:
        push    bx
        mov     bl,A_NORMAL
        call    status_msg
        pop     bx
        ret

show_ok:
        call    show_info
        jmp     beep_ok

show_error:
        push    bx
        mov     bl,A_BRIGHT
        call    status_msg
        pop     bx
        jmp     beep_err

clear_status:
        push    si
        mov     si,empty_str
        call    show_info
        pop     si
        ret

status_msg:
        push    cx
        push    dx
        mov     dh,ROW_STATUS
        mov     cl,1
        call    clear_rows
        mov     dl,1
        call    put_str
        pop     dx
        pop     cx
        ret

; ----------------------------------------------------------------------------
; ask_yn - SI = question (shown on the status line). CF=0 for Yes
; ----------------------------------------------------------------------------
ask_yn:
        push    ax
        push    bx
        mov     bl,A_BRIGHT
        call    status_msg
.key:
        call    get_key
        call    to_upper
        cmp     al,'Y'
        je      .yes
        cmp     al,'N'
        je      .no
        cmp     al,K_ESC
        je      .no
        jmp     .key
.yes:
        call    clear_status
        clc
        jmp     .done
.no:
        call    clear_status
        stc
.done:
        pop     bx
        pop     ax
        ret

; ----------------------------------------------------------------------------
; wait_any_key - show "Press any key" on the status line and wait
; ----------------------------------------------------------------------------
wait_any_key:
        push    ax
        push    si
        mov     si,msg_any_key
        call    show_info
        call    get_key
        call    clear_status
        pop     si
        pop     ax
        ret

; ----------------------------------------------------------------------------
; input_line - edit a text field
;   in : SI = buffer (zero terminated, initial text), CX = maximum length,
;        DH/DL = position, BL = IN_* flags
;   out: AX = key that ended the input (Enter, Esc, Tab, Up, Down, function
;        keys and other extended keys). The buffer holds the text.
; ----------------------------------------------------------------------------
input_line:
        push    bx
        push    cx
        push    dx
        push    si
        push    di
        mov     [in_buf],si
        mov     [in_max],cx
        mov     [in_rowcol],dx
        mov     [in_flags],bl
        call    str_len
        cmp     cx,[in_max]
        jbe     .len_ok
        mov     cx,[in_max]
        mov     bx,si
        add     bx,cx
        mov     byte [bx],0
.len_ok:
        mov     [in_len],cx
        mov     [in_cur],cx
        call    cursor_show
.redraw:
        call    in_draw
.key:
        call    get_key
        or      al,al
        jnz     .jf5
        jmp     .ext
.jf5:
        cmp     al,K_ENTER
        jne     .jf2
        jmp     .done
.jf2:
        cmp     al,K_ESC
        jne     .jf3
        jmp     .done
.jf3:
        cmp     al,K_TAB
        jne     .jf4
        jmp     .done
.jf4:
        cmp     al,K_BS
        je      .backspace
        cmp     al,K_CTRL_Y
        je      .clear
        cmp     al,K_CTRL_BS
        je      .clear
        cmp     al,' '
        jb      .key
        cmp     al,0FFh
        je      .key
        call    in_filter
        jc      .reject
        mov     cx,[in_len]
        cmp     cx,[in_max]
        jae     .reject
        ; make room at the cursor position
        mov     di,[in_buf]
        mov     bx,di
        add     bx,cx                   ; BX -> terminator
        add     di,[in_cur]             ; DI -> insert position
.shift:
        mov     ah,[bx]
        mov     [bx+1],ah
        cmp     bx,di
        je      .insert
        dec     bx
        jmp     .shift
.insert:
        mov     [di],al
        inc     word [in_len]
        inc     word [in_cur]
        jmp     .redraw
.reject:
        call    beep_err
        jmp     .key
.backspace:
        cmp     word [in_cur],0
        je      .key
        dec     word [in_cur]
        jmp     .delete
.clear:
        mov     si,[in_buf]
        mov     byte [si],0
        xor     ax,ax
        mov     [in_len],ax
        mov     [in_cur],ax
        jmp     .redraw
.ext:
        cmp     ah,S_LEFT
        jne     .ext1
        cmp     word [in_cur],0
        jne     .jf6
        jmp     .key
.jf6:
        dec     word [in_cur]
        jmp     .redraw
.ext1:
        cmp     ah,S_RIGHT
        jne     .ext2
        mov     cx,[in_cur]
        cmp     cx,[in_len]
        jb      .jf7
        jmp     .key
.jf7:
        inc     word [in_cur]
        jmp     .redraw
.ext2:
        cmp     ah,S_HOME
        jne     .ext3
        mov     word [in_cur],0
        jmp     .redraw
.ext3:
        cmp     ah,S_END
        jne     .ext4
        mov     cx,[in_len]
        mov     [in_cur],cx
        jmp     .redraw
.ext4:
        cmp     ah,S_DEL
        jne     .done
        mov     cx,[in_cur]
        cmp     cx,[in_len]
        jb      .delete
        jmp     .key
.delete:
        mov     si,[in_buf]
        add     si,[in_cur]
.del_next:
        mov     cl,[si+1]
        mov     [si],cl
        inc     si
        or      cl,cl
        jnz     .del_next
        dec     word [in_len]
        jmp     .redraw
.done:
        call    cursor_hide
        pop     di
        pop     si
        pop     dx
        pop     cx
        pop     bx
        ret

; in_filter - check/convert typed character AL for the current field.
;             CF=1 if the character is not allowed.
in_filter:
        cmp     al,';'                  ; field separator of DATA.DAT
        je      .bad
        test    byte [in_flags],IN_DIGITS
        jz      .price
        cmp     al,'0'
        jb      .bad
        cmp     al,'9'
        ja      .bad
        jmp     .ok
.price:
        test    byte [in_flags],IN_PRICE
        jz      .upper
        cmp     al,','
        jne     .point
        mov     al,'.'
.point:
        cmp     al,'.'
        je      .ok
        cmp     al,'0'
        jb      .bad
        cmp     al,'9'
        ja      .bad
        jmp     .ok
.upper:
        test    byte [in_flags],IN_UPPER
        jz      .ok
        call    to_upper
.ok:
        clc
        ret
.bad:
        stc
        ret

; in_draw - draw the field being edited and place the cursor
in_draw:
        push    ax
        push    cx
        push    dx
        push    si
        push    di
        push    es
        mov     dx,[in_rowcol]
        call    vid_offset
        mov     es,[vid_seg]
        mov     si,[in_buf]
        mov     cx,[in_max]
        inc     cx
        mov     ah,A_INPUT
        and     ah,[attr_mask]
.next:
        lodsb
        or      al,al
        jz      .pad
        test    byte [in_flags],IN_PASSWORD
        jz      .put
        mov     al,'*'
.put:
        stosw
        loop    .next
        jmp     .cursor
.pad:
        mov     al,' '
        rep     stosw
.cursor:
        pop     es
        add     dl,byte [in_cur]
        call    set_cursor
        pop     di
        pop     si
        pop     dx
        pop     cx
        pop     ax
        ret

; show_input - draw a field without editing it
;              SI = text, CX = maximum length, DH/DL = position, BL = flags
show_input:
        push    ax
        push    bx
        push    cx
        push    dx
        inc     cx                      ; fields are one cell wider
        test    bl,IN_PASSWORD
        jz      .plain
        push    cx
        call    str_len
        mov     ax,cx
        mov     bl,A_INPUT
        push    ax
        mov     al,'*'
        call    fill_chars
        pop     ax
        pop     cx
        add     dl,al
        sub     cx,ax
        mov     al,' '
        call    fill_chars
        jmp     .done
.plain:
        mov     bl,A_INPUT
        call    put_field
.done:
        pop     dx
        pop     cx
        pop     bx
        pop     ax
        ret

; ----------------------------------------------------------------------------
; Line builder: product lines are built in "lb" and then written either to
; the screen or to STOCK.TXT.
; ----------------------------------------------------------------------------
; lb_clear - fill the line buffer with spaces
lb_clear:
        push    ax
        push    cx
        push    di
        mov     di,lb
        mov     cx,LB_SIZE
        mov     al,' '
        rep     stosb
        mov     byte [di],0
        pop     di
        pop     cx
        pop     ax
        ret

; lb_put - SI = text, DX = column, CX = width, AL = 0 left / 1 right align
lb_put:
        push    bx
        push    cx
        push    si
        push    di
        mov     bx,cx
        call    str_len
        cmp     cx,bx
        jbe     .fits
        mov     cx,bx
.fits:
        mov     di,lb
        add     di,dx
        or      al,al
        jz      .copy
        add     di,bx
        sub     di,cx
.copy:
        rep     movsb
        pop     di
        pop     si
        pop     cx
        pop     bx
        ret

; lb_num - put the number in num_str right aligned (DX = column, CX = width)
lb_num:
        push    ax
        push    si
        mov     si,num_str
        mov     al,1
        call    lb_put
        pop     si
        pop     ax
        ret

; lb_text - put SI left aligned (DX = column, CX = width)
lb_text:
        push    ax
        xor     al,al
        call    lb_put
        pop     ax
        ret

; lb_to_screen - DH = row, BL = attribute (first 80 columns)
lb_to_screen:
        push    cx
        push    dx
        push    si
        mov     si,lb
        mov     cx,80
        xor     dl,dl
        call    put_field
        pop     si
        pop     dx
        pop     cx
        ret

; lb_to_file - write the line buffer without trailing blanks
lb_to_file:
        push    ax
        push    cx
        push    si
        mov     cx,LB_SIZE
.trim:
        jcxz    .write
        mov     si,cx
        cmp     byte [lb+si-1],' '
        jne     .write
        dec     cx
        jmp     .trim
.write:
        mov     si,lb
        jcxz    .eol
.next:
        lodsb
        call    fw_putc
        loop    .next
.eol:
        call    fw_crlf
        pop     si
        pop     cx
        pop     ax
        ret

; ----------------------------------------------------------------------------
; fmt_header_line - column headings of the product list
; ----------------------------------------------------------------------------
fmt_header_line:
        push    ax
        push    cx
        push    dx
        push    si
        call    lb_clear
        mov     al,1
        mov     si,hdr_no
        mov     dx,COL_NO
        mov     cx,W_NO
        call    lb_put
        mov     si,hdr_price
        mov     dx,COL_PRICE
        mov     cx,W_PRICE
        call    lb_put
        mov     si,hdr_qty
        mov     dx,COL_QTY
        mov     cx,W_QTY
        call    lb_put
        mov     si,hdr_value
        mov     dx,COL_VAL
        mov     cx,W_VAL
        call    lb_put
        xor     al,al
        mov     si,hdr_barcode
        mov     dx,COL_BAR
        mov     cx,W_BAR
        call    lb_put
        mov     si,hdr_name
        mov     dx,COL_NAME
        mov     cx,W_NAME
        call    lb_put
        mov     si,hdr_status
        mov     dx,COL_ST
        mov     cx,W_ST
        call    lb_put
        pop     si
        pop     dx
        pop     cx
        pop     ax
        ret

; ----------------------------------------------------------------------------
; fmt_product_line - AX = record index, CX = line number  ->  lb
;                    Also leaves the price in fpl_price and qty in fpl_qty.
; ----------------------------------------------------------------------------
fmt_product_line:
        push    ax
        push    bx
        push    cx
        push    dx
        push    si
        push    di
        call    lb_clear
        push    ax
        mov     ax,cx
        mov     di,num_str
        call    u16_append
        mov     dx,COL_NO
        mov     cx,W_NO
        call    lb_num
        pop     ax
        call    rec_barcode
        mov     dx,COL_BAR
        mov     cx,W_BAR
        call    lb_text
        call    rec_name
        mov     dx,COL_NAME
        mov     cx,W_NAME
        call    lb_text
        call    rec_get
        mov     [fpl_price],ax
        mov     [fpl_price+2],dx
        mov     [fpl_qty],cx
        mov     di,num_str
        mov     bl,2
        call    u32_to_str
        mov     dx,COL_PRICE
        mov     cx,W_PRICE
        call    lb_num
        mov     ax,[fpl_qty]
        mov     di,num_str
        call    u16_append
        mov     dx,COL_QTY
        mov     cx,W_QTY
        call    lb_num
        mov     si,str_out
        or      ax,ax
        jz      .status
        mov     si,str_low
        cmp     ax,[cfg_low]
        jbe     .status
        mov     si,empty_str
.status:
        mov     dx,COL_ST
        mov     cx,W_ST
        call    lb_text
        pop     di
        pop     si
        pop     dx
        pop     cx
        pop     bx
        pop     ax
        ret

; ----------------------------------------------------------------------------
; list_view - scrollable product list
;   uses: idx_buf/lv_count (items), lv_first (first screen row),
;         lv_rows (visible rows), lv_sel/lv_top (state)
;   out : AX = key (Enter, Esc, Del or another extended key)
;         BX = selected record index (only when lv_count > 0)
; ----------------------------------------------------------------------------
list_view:
        push    cx
        push    dx
        mov     byte [lv_focus],1
        call    lv_fix
.draw:
        call    lv_draw
.key:
        call    get_key
        cmp     al,K_ESC
        je      .return
        cmp     word [lv_count],0
        je      .key
        cmp     al,K_ENTER
        je      .return
        or      al,al
        jnz     .key
        mov     cx,[lv_sel]
        cmp     ah,S_UP
        jne     .k1
        dec     cx
        jmp     .move
.k1:
        cmp     ah,S_DOWN
        jne     .k2
        inc     cx
        jmp     .move
.k2:
        cmp     ah,S_PGUP
        jne     .k3
        sub     cx,[lv_rows]
        jmp     .move
.k3:
        cmp     ah,S_PGDN
        jne     .k4
        add     cx,[lv_rows]
        jmp     .move
.k4:
        cmp     ah,S_HOME
        jne     .k5
        xor     cx,cx
        jmp     .move
.k5:
        cmp     ah,S_END
        jne     .return
        mov     cx,[lv_count]
        dec     cx
.move:
        mov     [lv_sel],cx
        call    lv_fix
        jmp     .draw
.return:
        mov     byte [lv_focus],0
        xor     bx,bx
        cmp     word [lv_count],0
        je      .done
        mov     bx,[lv_sel]
        shl     bx,1
        mov     bx,[idx_buf+bx]
.done:
        pop     dx
        pop     cx
        ret

; lv_fix - keep lv_sel and lv_top inside the list
lv_fix:
        push    ax
        push    cx
        mov     cx,[lv_sel]
        or      cx,cx
        jns     .not_neg
        xor     cx,cx
.not_neg:
        cmp     cx,[lv_count]
        jl      .in_list
        mov     cx,[lv_count]
        dec     cx
        jns     .in_list
        xor     cx,cx
.in_list:
        mov     [lv_sel],cx
        mov     ax,[lv_top]
        cmp     cx,ax
        jge     .top_ok
        mov     ax,cx
.top_ok:
        push    ax
        add     ax,[lv_rows]
        cmp     cx,ax
        pop     ax
        jl      .bottom_ok
        mov     ax,cx
        sub     ax,[lv_rows]
        inc     ax
.bottom_ok:
        mov     [lv_top],ax
        pop     cx
        pop     ax
        ret

; lv_draw - draw heading, visible rows and the position line
lv_draw:
        push    ax
        push    bx
        push    cx
        push    dx
        push    si
        push    di
        call    fmt_header_line
        mov     dh,[lv_first]
        dec     dh
        mov     bl,A_BRIGHT
        call    lb_to_screen
        xor     cx,cx
.row:
        cmp     cx,[lv_rows]
        jae     .info
        mov     dh,[lv_first]
        add     dh,cl
        mov     ax,[lv_top]
        add     ax,cx
        cmp     ax,[lv_count]
        jae     .blank
        push    cx
        mov     cx,ax
        inc     cx
        mov     si,ax
        shl     si,1
        mov     bl,A_NORMAL
        cmp     byte [lv_focus],0       ; selection bar only in the list
        je      .attr
        cmp     ax,[lv_sel]
        jne     .attr
        mov     bl,A_REVERSE
.attr:
        mov     ax,[idx_buf+si]
        call    fmt_product_line
        call    lb_to_screen
        pop     cx
        jmp     .next_row
.blank:
        push    cx
        mov     cl,1
        call    clear_rows
        pop     cx
.next_row:
        inc     cx
        jmp     .row
.info:
        mov     dh,[lv_first]
        add     dh,byte [lv_rows]
        mov     cl,1
        call    clear_rows
        mov     si,msg_no_products
        cmp     word [lv_count],0
        je      .show
        mov     di,msg_buf
        mov     si,str_product
        call    str_append
        mov     ax,[lv_sel]
        inc     ax
        call    u16_append
        mov     si,str_of
        call    str_append
        mov     ax,[lv_count]
        call    u16_append
        mov     si,msg_buf
.show:
        mov     dl,1
        mov     bl,A_NORMAL
        call    put_str
        pop     di
        pop     si
        pop     dx
        pop     cx
        pop     bx
        pop     ax
        ret

; ----------------------------------------------------------------------------
; Product form (barcode, name, price, quantity)
; ----------------------------------------------------------------------------
FORM_COL        = 22
FORM_FIELDS     = 4

; form_info - AL = field number  ->  SI = buffer, CX = length, BL = flags,
;                                     DH/DL = position
form_info:
        push    ax
        push    di
        xor     ah,ah
        mov     di,ax
        mov     cl,3
        shl     di,cl
        add     di,form_table
        mov     si,[di]
        mov     cx,[di+2]
        mov     bl,[di+4]
        mov     dh,[di+6]
        mov     dl,FORM_COL
        pop     di
        pop     ax
        ret

; form_draw - labels and current values of all fields
form_draw:
        push    ax
        push    bx
        push    cx
        push    dx
        push    si
        call    clear_work
        xor     al,al
.field:
        call    form_info
        call    show_input
        push    si
        mov     dl,4
        mov     bl,A_NORMAL
        xor     ah,ah
        mov     si,ax
        shl     si,1
        mov     si,[form_labels+si]
        call    put_str
        pop     si
        inc     al
        cmp     al,FORM_FIELDS
        jb      .field
        ; currency after the price field
        mov     dx,((ROW_TOP+6) shl 8) + FORM_COL+PRICE_LEN+2
        mov     si,cfg_currency
        mov     bl,A_NORMAL
        call    put_str
        mov     dx,((ROW_TOP+11) shl 8) + 4
        mov     si,form_help
        call    draw_lines
        pop     si
        pop     dx
        pop     cx
        pop     bx
        pop     ax
        ret

; product_form - edit the ed_* buffers. form_self = record being edited
;                (0FFFFh for a new product). CF=1 if cancelled.
product_form:
        push    ax
        push    bx
        push    cx
        push    dx
        push    si
        mov     byte [form_field],0
        call    form_draw
.edit:
        mov     al,[form_field]
        call    form_info
        call    input_line
        cmp     al,K_ESC
        je      .cancel
        cmp     al,K_ENTER
        je      .enter
        cmp     al,K_TAB
        je      .next
        or      al,al
        jnz     .edit
        cmp     ah,S_DOWN
        je      .next
        cmp     ah,S_UP
        je      .prev
        cmp     ah,S_STAB
        je      .prev
        jmp     .edit
.next:
        cmp     byte [form_field],FORM_FIELDS-1
        jae     .edit
        inc     byte [form_field]
        jmp     .edit
.prev:
        cmp     byte [form_field],0
        je      .edit
        dec     byte [form_field]
        jmp     .edit
.enter:
        mov     al,[form_field]
        call    form_validate
        jc      .edit
        call    clear_status
        cmp     al,FORM_FIELDS-1
        jb      .next
        xor     al,al                   ; last field: check everything
.check:
        call    form_validate
        jc      .invalid
        inc     al
        cmp     al,FORM_FIELDS
        jb      .check
        clc
        jmp     .done
.invalid:
        mov     [form_field],al
        jmp     .edit
.cancel:
        call    clear_status
        stc
.done:
        pop     si
        pop     dx
        pop     cx
        pop     bx
        pop     ax
        ret

; form_validate - AL = field number, CF=1 (and a message) if invalid
form_validate:
        push    ax
        push    bx
        push    cx
        push    dx
        push    si
        call    form_info
        call    str_trim
        call    show_input              ; show the trimmed text
        cmp     al,0
        jne     .name
        cmp     byte [si],0
        mov     si,err_barcode_empty
        je      .bad
        mov     si,ed_barcode
        call    db_find_barcode
        jc      .ok
        cmp     ax,[form_self]
        je      .ok
        mov     si,err_barcode_exists
        jmp     .bad
.name:
        cmp     al,1
        jne     .price
        cmp     byte [si],0
        mov     si,err_name_empty
        je      .bad
        jmp     .ok
.price:
        cmp     al,2
        jne     .qty
        call    parse_price
        mov     si,err_price
        jc      .bad
        jmp     .ok
.qty:
        call    parse_qty
        mov     si,err_qty
        jc      .bad
.ok:
        clc
        jmp     .done
.bad:
        call    show_error
        stc
.done:
        pop     si
        pop     dx
        pop     cx
        pop     bx
        pop     ax
        ret

; ----------------------------------------------------------------------------
; password_needed - CF=0 if the program is protected by a password
; ----------------------------------------------------------------------------
password_needed:
        cmp     byte [cfg_login],0
        je      .no
        cmp     byte [cfg_pass],0
        je      .no
        clc
        ret
.no:
        stc
        ret

; ----------------------------------------------------------------------------
; ask_password - password box in the middle of the screen
;   out: CF=1 if Esc was pressed, otherwise AL=1 correct / AL=0 wrong
; ----------------------------------------------------------------------------
ask_password:
        push    bx
        push    cx
        push    dx
        push    si
        push    di
        mov     dx,(9 shl 8) + 20
        mov     cx,(15 shl 8) + 59
        mov     bl,A_NORMAL
        call    draw_box
        mov     dx,(10 shl 8) + 23
        mov     si,msg_enter_password
        mov     bl,A_BRIGHT
        call    put_str
        mov     dx,(12 shl 8) + 23
        mov     si,lbl_password
        mov     bl,A_NORMAL
        call    put_str
        mov     byte [pw_buf],0
.input:
        mov     si,pw_buf
        mov     cx,PASS_LEN
        mov     dx,(12 shl 8) + 34
        mov     bl,IN_PASSWORD
        call    input_line
        cmp     al,K_ESC
        je      .esc
        cmp     al,K_ENTER
        jne     .input
        mov     si,pw_buf
        mov     di,cfg_pass
        call    str_cmp
        mov     al,1
        je      .done
        xor     al,al
        jmp     .done
.esc:
        stc
        jmp     .exit
.done:
        clc
.exit:
        pop     di
        pop     si
        pop     dx
        pop     cx
        pop     bx
        ret
