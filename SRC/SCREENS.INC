; ============================================================================
;  SCREENS.INC - Main menu and program functions
; ============================================================================

MENU_TOP        = ROW_TOP+1
MENU_BOTTOM     = MENU_TOP+13
MENU_LEFT       = 19
MENU_RIGHT      = 60
MENU_COUNT      = 12

; ----------------------------------------------------------------------------
; main_menu - F1..F12 (or Up/Down and Enter) select a function
; ----------------------------------------------------------------------------
main_menu:
.redraw:
        mov     si,sec_main
        mov     di,hint_main
        call    draw_frame
        mov     dx,(MENU_TOP shl 8) + MENU_LEFT
        mov     cx,(MENU_BOTTOM shl 8) + MENU_RIGHT
        mov     bl,A_NORMAL
        call    draw_box
        call    menu_summary
.items:
        call    menu_items
.key:
        call    get_key
        or      al,al
        jnz     .ascii
        mov     al,ah
        cmp     al,S_F1
        jb      .nav
        cmp     al,S_F10
        ja      .nav
        sub     al,S_F1                 ; F1..F10 -> 0..9
        jmp     .run
.nav:
        cmp     al,S_F11
        je      .f11
        cmp     al,S_SF1
        je      .f11
        cmp     al,S_F12
        je      .f12
        cmp     al,S_SF2
        je      .f12
        cmp     al,S_UP
        je      .up
        cmp     al,S_DOWN
        je      .down
        cmp     al,S_HOME
        je      .home
        cmp     al,S_END
        je      .f12_sel
        jmp     .key
.f11:
        mov     al,10
        jmp     .run
.f12:
        mov     al,11
        jmp     .run
.up:
        mov     al,[menu_sel]
        dec     al
        jns     .select
.f12_sel:
        mov     al,MENU_COUNT-1
        jmp     .select
.down:
        mov     al,[menu_sel]
        inc     al
        cmp     al,MENU_COUNT
        jb      .select
.home:
        xor     al,al
.select:
        mov     [menu_sel],al
        jmp     .items
.ascii:
        cmp     al,K_ENTER
        jne     .esc
        mov     al,[menu_sel]
        jmp     .run
.esc:
        cmp     al,K_ESC
        jne     .key
        mov     al,MENU_COUNT-1
.run:
        mov     [menu_sel],al
        call    menu_items
        cmp     al,MENU_COUNT-1
        je      .exit
        xor     ah,ah
        mov     bx,ax
        shl     bx,1
        call    word [menu_handlers+bx]
        jmp     .redraw
.exit:
        mov     si,msg_exit_confirm
        call    ask_yn
        jnc     .jf8
        jmp     .key
.jf8:
        ret

; menu_items - draw the menu entries (selected one in reverse video)
menu_items:
        push    ax
        push    bx
        push    cx
        push    dx
        push    si
        xor     ax,ax
        mov     dx,((MENU_TOP+1) shl 8) + MENU_LEFT+2
.next:
        mov     si,ax
        shl     si,1
        mov     si,[menu_texts+si]
        mov     bl,A_NORMAL
        cmp     al,[menu_sel]
        jne     .put
        mov     bl,A_REVERSE
.put:
        mov     cx,MENU_RIGHT-MENU_LEFT-3
        call    put_field
        inc     dh
        inc     ax
        cmp     ax,MENU_COUNT
        jb      .next
        pop     si
        pop     dx
        pop     cx
        pop     bx
        pop     ax
        ret

; menu_summary - product counts below the menu
menu_summary:
        push    ax
        push    bx
        push    dx
        push    si
        push    di
        call    db_stats
        mov     di,msg_buf
        mov     si,str_sum_products
        call    str_append
        mov     ax,[db_count]
        call    u16_append
        mov     si,str_sum_max
        call    str_append
        mov     si,str_sum_low
        call    str_append
        mov     ax,[st_low]
        call    u16_append
        mov     si,str_sum_out
        call    str_append
        mov     ax,[st_out]
        call    u16_append
        mov     si,msg_buf
        mov     dh,MENU_BOTTOM+2
        mov     bl,A_NORMAL
        call    put_center
        pop     di
        pop     si
        pop     dx
        pop     bx
        pop     ax
        ret

; ----------------------------------------------------------------------------
; login - ask for the password (3 attempts). CF=1 if access is denied
; ----------------------------------------------------------------------------
login:
        push    ax
        push    si
        push    di
        mov     byte [login_tries],3
.try:
        mov     si,sec_login
        mov     di,hint_login
        call    draw_frame
        call    ask_password
        jc      .fail
        or      al,al
        jnz     .ok
        dec     byte [login_tries]
        jz      .denied
        mov     word [pending_msg],err_wrong_password
        call    beep_err
        jmp     .try
.denied:
        mov     word [exit_msg],msg_access_denied
.fail:
        stc
        jmp     .done
.ok:
        clc
.done:
        pop     di
        pop     si
        pop     ax
        ret

; ----------------------------------------------------------------------------
; save_database - write DATA.DAT; SI = message for success.
;                 The result is left in pending_msg (see show_pending).
; ----------------------------------------------------------------------------
save_database:
        call    db_save
        jc      .error
        mov     [pending_msg],si
        jmp     beep_ok
.error:
        mov     word [pending_msg],err_save
        jmp     beep_err

; show_pending - show the pending message now
show_pending:
        push    si
        mov     si,[pending_msg]
        or      si,si
        jz      .done
        mov     word [pending_msg],0
        call    show_info
.done:
        pop     si
        ret

; ----------------------------------------------------------------------------
; F1 - Help
; ----------------------------------------------------------------------------
do_help:
        mov     si,sec_help
        mov     di,hint_back
        call    draw_frame
        mov     dx,(ROW_TOP shl 8) + 1
        mov     si,help_text
        mov     bl,A_NORMAL
        call    draw_lines
        call    get_key
        ret

; ----------------------------------------------------------------------------
; F2 - Product list
; ----------------------------------------------------------------------------
do_list:
        xor     ax,ax
        mov     [lv_sel],ax
        mov     [lv_top],ax
.redraw:
        mov     si,sec_list
        mov     di,hint_list
        call    draw_frame
.refresh:
        mov     byte [lv_first],ROW_TOP+1
        mov     word [lv_rows],ROW_BOTTOM-ROW_TOP-1
        call    db_filter_all
.list:
        call    list_view
        cmp     al,K_ESC
        je      .done
        cmp     al,K_ENTER
        jne     .delete
        mov     ax,bx
        call    edit_record
        jmp     .redraw
.delete:
        or      al,al
        jnz     .list
        cmp     ah,S_DEL
        jne     .list
        mov     ax,bx
        call    delete_confirm
        call    show_pending
        jmp     .refresh
.done:
        ret

; ----------------------------------------------------------------------------
; F3 - Add product
; ----------------------------------------------------------------------------
do_add:
        mov     si,sec_add
        mov     di,hint_form
        call    draw_frame
.next:
        cmp     word [db_count],MAX_REC
        jb      .form
        mov     si,err_db_full
        call    show_error
        call    get_key
        ret
.form:
        xor     al,al
        mov     [ed_barcode],al
        mov     [ed_name],al
        mov     [ed_price],al
        mov     [ed_qty],al
        mov     word [form_self],0FFFFh
        call    product_form
        jc      .done
        call    db_append
        call    edit_to_rec
        mov     si,msg_added
        call    save_database
        call    show_pending
        jmp     .next
.done:
        ret

; ----------------------------------------------------------------------------
; F4 - Search product
; ----------------------------------------------------------------------------
do_search:
        mov     byte [query_buf],0
        mov     si,sec_search
        mov     di,hint_search
        call    draw_frame
        call    search_label
.input:
        mov     si,query_buf
        mov     cx,QUERY_LEN
        mov     dx,(ROW_TOP shl 8) + 10
        xor     bl,bl
        call    input_line
        cmp     al,K_ESC
        je      .done
        cmp     al,K_ENTER
        jne     .input
        xor     ax,ax
        mov     [lv_sel],ax
        mov     [lv_top],ax
.results:
        mov     si,query_buf
        call    str_trim
        call    db_filter
        mov     byte [lv_first],ROW_TOP+3
        mov     word [lv_rows],ROW_BOTTOM-ROW_TOP-3
        mov     cx,QUERY_LEN
        mov     dx,(ROW_TOP shl 8) + 10
        xor     bl,bl
        call    show_input
.list:
        call    list_view
        cmp     al,K_ESC
        jne     .enter
        mov     dh,ROW_TOP+1
        mov     cl,ROW_BOTTOM-ROW_TOP
        call    clear_rows
        jmp     .input
.enter:
        cmp     al,K_ENTER
        jne     .delete
        mov     ax,bx
        call    edit_record
        mov     si,sec_search
        mov     di,hint_search
        call    draw_frame
        call    search_label
        jmp     .results
.delete:
        or      al,al
        jnz     .list
        cmp     ah,S_DEL
        jne     .list
        mov     ax,bx
        call    delete_confirm
        call    show_pending
        jmp     .results
.done:
        ret

search_label:
        push    bx
        push    dx
        push    si
        mov     dx,(ROW_TOP shl 8) + 1
        mov     si,lbl_search
        mov     bl,A_NORMAL
        call    put_str
        pop     si
        pop     dx
        pop     bx
        ret

; ----------------------------------------------------------------------------
; F5 - Update product
; ----------------------------------------------------------------------------
do_update:
        mov     si,sec_update
        mov     di,hint_select
        call    draw_frame
        call    select_product
        jc      .done
        call    edit_record
        jmp     do_update
.done:
        ret

; edit_record - edit product AX with the product form and save it
edit_record:
        push    ax
        push    si
        push    di
        mov     [form_self],ax
        call    rec_to_edit
        mov     si,sec_update
        mov     di,hint_form
        call    draw_frame
        call    product_form
        jc      .done
        mov     ax,[form_self]
        call    edit_to_rec
        mov     si,msg_updated
        call    save_database
.done:
        pop     di
        pop     si
        pop     ax
        ret

; ----------------------------------------------------------------------------
; select_product - ask for a barcode or a part of a name.
;   An exact barcode match (or a single result) is selected directly,
;   otherwise the matching products are listed.
;   out: AX = record index, CF=1 if Esc was pressed
; ----------------------------------------------------------------------------
select_product:
        push    bx
        push    cx
        push    dx
        push    si
        mov     byte [query_buf],0
.ask:
        call    clear_work
        mov     dx,(ROW_TOP shl 8) + 1
        mov     si,lbl_find
        mov     bl,A_NORMAL
        call    put_str
.input:
        mov     si,query_buf
        mov     cx,QUERY_LEN
        mov     dx,(ROW_TOP shl 8) + 19
        xor     bl,bl
        call    input_line
        cmp     al,K_ESC
        je      .esc
        cmp     al,K_ENTER
        jne     .input
        mov     si,query_buf
        call    str_trim
        call    db_find_barcode
        jnc     .found
        call    db_filter
        mov     cx,[lv_count]
        jcxz    .none
        mov     ax,[idx_buf]
        cmp     cx,1
        je      .found
        mov     byte [lv_first],ROW_TOP+3
        mov     word [lv_rows],ROW_BOTTOM-ROW_TOP-3
        xor     ax,ax
        mov     [lv_sel],ax
        mov     [lv_top],ax
        mov     si,msg_select
        call    show_info
.list:
        call    list_view
        cmp     al,K_ESC
        je      .ask
        cmp     al,K_ENTER
        jne     .list
        mov     ax,bx
.found:
        call    clear_status
        clc
        jmp     .done
.none:
        mov     si,msg_not_found
        call    show_error
        jmp     .input
.esc:
        call    clear_status
        stc
.done:
        pop     si
        pop     dx
        pop     cx
        pop     bx
        ret

; ----------------------------------------------------------------------------
; F6 - Delete product
; ----------------------------------------------------------------------------
do_delete:
        mov     si,sec_delete
        mov     di,hint_select
        call    draw_frame
        call    select_product
        jc      .done
        mov     [form_self],ax
        call    rec_to_edit
        call    form_draw
        push    cx
        push    dx
        mov     dh,ROW_TOP+10
        mov     cl,ROW_BOTTOM-ROW_TOP-9
        call    clear_rows
        pop     dx
        pop     cx
        call    delete_confirm
        jmp     do_delete
.done:
        ret

; delete_confirm - ask and delete product AX. CF=0 if it was deleted
delete_confirm:
        push    ax
        push    si
        push    di
        push    ax
        mov     di,msg_buf
        mov     si,str_delete_q1
        call    str_append
        call    rec_name
        call    str_append
        mov     si,str_delete_q2
        call    str_append
        pop     ax
        mov     si,msg_buf
        call    ask_yn
        jc      .done
        call    db_delete
        mov     si,msg_deleted
        call    save_database
        clc
.done:
        pop     di
        pop     si
        pop     ax
        ret

; ----------------------------------------------------------------------------
; F7 - Sales (checkout)
; ----------------------------------------------------------------------------
do_sell:
.new:
        mov     word [cart_n],0
        mov     byte [sale_buf],0
        mov     si,sec_sell
        mov     di,hint_sell
        call    draw_frame
        call    sell_draw
.input:
        mov     si,sale_buf
        mov     cx,SALE_LEN
        mov     dx,(ROW_TOP shl 8) + 10
        xor     bl,bl
        call    input_line
        cmp     al,K_ENTER
        je      .add
        cmp     al,K_ESC
        je      .esc
        or      al,al
        jnz     .input
        cmp     ah,S_F2
        je      .pay
        cmp     ah,S_F3
        je      .remove
        jmp     .input
.add:
        call    sell_add
        jmp     .input
.remove:
        cmp     word [cart_n],0
        je      .empty
        dec     word [cart_n]
        call    sell_draw
        mov     si,msg_removed
        call    show_info
        jmp     .input
.pay:
        cmp     word [cart_n],0
        je      .empty
        call    sell_pay
        jc      .input
        jmp     .new
.empty:
        mov     si,err_cart_empty
        call    show_error
        jmp     .input
.esc:
        cmp     word [cart_n],0
        je      .done
        mov     si,msg_cancel_sale
        call    ask_yn
        jc      .input
.done:
        ret

; sell_draw - labels, cart lines and totals
sell_draw:
        push    ax
        push    bx
        push    cx
        push    dx
        push    si
        push    di
        mov     dx,(ROW_TOP shl 8) + 1
        mov     si,lbl_barcode_sale
        mov     bl,A_NORMAL
        call    put_str
        mov     si,sale_buf
        mov     cx,SALE_LEN
        mov     dl,10
        xor     bl,bl
        call    show_input
        mov     dx,((ROW_TOP+1) shl 8) + 10
        mov     si,msg_sale_tip
        mov     bl,A_NORMAL
        call    put_str
        ; heading
        call    lb_clear
        mov     al,1
        mov     si,hdr_no_short
        mov     dx,CC_NO
        mov     cx,CW_NO
        call    lb_put
        mov     si,hdr_qty
        mov     dx,CC_QTY
        mov     cx,CW_QTY
        call    lb_put
        mov     si,hdr_price
        mov     dx,CC_PRICE
        mov     cx,CW_PRICE
        call    lb_put
        mov     si,hdr_amount
        mov     dx,CC_AMOUNT
        mov     cx,CW_AMOUNT
        call    lb_put
        xor     al,al
        mov     si,hdr_barcode
        mov     dx,CC_BAR
        mov     cx,CW_BAR
        call    lb_put
        mov     si,hdr_name
        mov     dx,CC_NAME
        mov     cx,CW_NAME
        call    lb_put
        mov     dh,CART_FIRST-1
        mov     bl,A_BRIGHT
        call    lb_to_screen
        ; lines (the last CART_ROWS lines are visible)
        mov     ax,[cart_n]
        sub     ax,CART_ROWS
        jns     .first
        xor     ax,ax
.first:
        xor     cx,cx
.row:
        cmp     cx,CART_ROWS
        jae     .totals
        mov     dh,CART_FIRST
        add     dh,cl
        mov     bx,ax
        add     bx,cx
        cmp     bx,[cart_n]
        jae     .blank
        push    ax
        mov     ax,bx
        call    fmt_cart_line
        pop     ax
        mov     bl,A_NORMAL
        call    lb_to_screen
        jmp     .next_row
.blank:
        push    cx
        mov     cl,1
        call    clear_rows
        pop     cx
.next_row:
        inc     cx
        jmp     .row
.totals:
        call    sell_totals
        mov     dh,ROW_BOTTOM-1
        mov     cl,2
        call    clear_rows
        mov     di,msg_buf
        mov     si,str_lines
        call    str_append
        mov     ax,[cart_n]
        call    u16_append
        mov     si,str_items
        call    str_append
        mov     ax,[sale_qty]
        mov     dx,[sale_qty+2]
        xor     bl,bl
        call    u32_to_str
        mov     si,msg_buf
        mov     dx,(ROW_BOTTOM shl 8) + 1
        mov     bl,A_NORMAL
        call    put_str
        mov     di,msg_buf
        mov     si,str_total
        call    str_append
        mov     si,sale_total
        mov     bl,2
        call    n48_to_str
        call    currency_append
        mov     si,msg_buf
        call    str_len
        mov     dl,79
        sub     dl,cl
        mov     bl,A_BRIGHT
        call    put_str
        pop     di
        pop     si
        pop     dx
        pop     cx
        pop     bx
        pop     ax
        ret

; currency_append - append " " and the currency symbol at DI (if any)
currency_append:
        cmp     byte [cfg_currency],0
        je      .done
        push    ax
        push    si
        mov     al,' '
        call    chr_append
        mov     si,cfg_currency
        call    str_append
        pop     si
        pop     ax
.done:
        ret

; fmt_cart_line - AX = cart line  ->  lb
fmt_cart_line:
        push    ax
        push    bx
        push    cx
        push    dx
        push    si
        push    di
        call    lb_clear
        mov     bx,ax
        shl     bx,1
        inc     ax
        mov     di,num_str
        call    u16_append
        mov     dx,CC_NO
        mov     cx,CW_NO
        call    lb_num
        mov     ax,[cart_qty+bx]
        mov     [fpl_qty],ax
        mov     di,num_str
        call    u16_append
        mov     dx,CC_QTY
        mov     cx,CW_QTY
        call    lb_num
        mov     ax,[cart_rec+bx]
        call    rec_barcode
        mov     dx,CC_BAR
        mov     cx,CW_BAR
        call    lb_text
        call    rec_name
        mov     dx,CC_NAME
        mov     cx,CW_NAME
        call    lb_text
        call    rec_get
        mov     bx,[fpl_qty]
        mov     di,amount48
        call    mul32x16
        mov     di,num_str
        mov     bl,2
        call    u32_to_str
        mov     dx,CC_PRICE
        mov     cx,CW_PRICE
        call    lb_num
        mov     si,amount48
        mov     di,num_str
        call    n48_to_str
        mov     dx,CC_AMOUNT
        mov     cx,CW_AMOUNT
        call    lb_num
        pop     di
        pop     si
        pop     dx
        pop     cx
        pop     bx
        pop     ax
        ret

; sell_totals - sale_total (48-bit) and sale_qty (32-bit) of the cart
sell_totals:
        push    ax
        push    bx
        push    cx
        push    dx
        push    si
        push    di
        mov     di,sale_total
        call    zero48
        xor     ax,ax
        mov     [sale_qty],ax
        mov     [sale_qty+2],ax
        xor     si,si
.next:
        mov     ax,si
        cmp     ax,[cart_n]
        jae     .done
        push    si
        shl     si,1
        mov     bx,[cart_qty+si]
        add     [sale_qty],bx
        adc     word [sale_qty+2],0
        mov     ax,[cart_rec+si]
        call    rec_get
        mov     di,amount48
        call    mul32x16
        mov     si,amount48
        mov     di,sale_total
        call    add48
        pop     si
        inc     si
        jmp     .next
.done:
        pop     di
        pop     si
        pop     dx
        pop     cx
        pop     bx
        pop     ax
        ret

; sell_add - add the barcode typed in sale_buf ("QTY*BARCODE" allowed)
sell_add:
        push    ax
        push    bx
        push    cx
        push    dx
        push    si
        push    di
        mov     si,sale_buf
        call    str_trim
        cmp     byte [si],0
        jne     .parse
        jmp     .done
.parse:
        mov     word [sell_q],1
        mov     di,si
.find:
        mov     al,[di]
        or      al,al
        jz      .code
        cmp     al,'*'
        je      .star
        inc     di
        jmp     .find
.star:
        mov     byte [di],0
        call    parse_qty
        jnc     .jf9
        jmp     .bad_qty
.jf9:
        or      ax,ax
        jnz     .jf10
        jmp     .bad_qty
.jf10:
        cmp     ax,MAX_SALE_QTY
        jbe     .jf11
        jmp     .bad_qty
.jf11:
        mov     [sell_q],ax
        lea     si,[di+1]
        call    str_trim
.code:
        call    db_find_barcode
        jc      .not_found
        mov     [sell_rec],ax
        call    rec_get                 ; CX = quantity in stock
        xor     bx,bx
.scan:
        cmp     bx,[cart_n]
        jae     .new_line
        mov     di,bx
        shl     di,1
        mov     ax,[cart_rec+di]
        cmp     ax,[sell_rec]
        je      .existing
        inc     bx
        jmp     .scan
.existing:
        mov     ax,[cart_qty+di]
        add     ax,[sell_q]
        jc      .no_stock
        cmp     ax,cx
        ja      .no_stock
        mov     [cart_qty+di],ax
        jmp     .added
.new_line:
        cmp     bx,MAX_CART
        jae     .full
        mov     ax,[sell_q]
        cmp     ax,cx
        ja      .no_stock
        mov     di,bx
        shl     di,1
        mov     [cart_qty+di],ax
        mov     ax,[sell_rec]
        mov     [cart_rec+di],ax
        inc     word [cart_n]
.added:
        mov     byte [sale_buf],0
        call    sell_draw
        mov     di,msg_buf
        mov     si,str_added
        call    str_append
        mov     ax,[sell_rec]
        call    rec_name
        call    str_append
        mov     si,msg_buf
        call    show_ok
        jmp     .done
.bad_qty:
        mov     si,err_sale_qty
        jmp     .error
.full:
        mov     si,err_cart_full
        jmp     .error
.not_found:
        mov     di,msg_buf
        push    si
        mov     si,err_not_found_bc
        call    str_append
        pop     si
        call    str_append
        mov     si,msg_buf
        jmp     .error
.no_stock:
        mov     di,msg_buf
        mov     si,err_no_stock
        call    str_append
        mov     ax,cx
        call    u16_append
        mov     si,msg_buf
.error:
        call    show_error
.done:
        mov     byte [sale_buf],0
        pop     di
        pop     si
        pop     dx
        pop     cx
        pop     bx
        pop     ax
        ret

; sell_pay - ask for the amount paid, show the change and update the stock.
;            CF=1 if cancelled.
sell_pay:
        push    ax
        push    bx
        push    cx
        push    dx
        push    si
        push    di
        call    sell_totals
        mov     byte [pay_buf],0
        mov     dh,ROW_BOTTOM-1
        mov     cl,1
        call    clear_rows
        mov     dx,((ROW_BOTTOM-1) shl 8) + 1
        mov     si,lbl_paid
        mov     bl,A_BRIGHT
        call    put_str
        inc     dl
        mov     [pay_col],dl
.input:
        mov     si,pay_buf
        mov     cx,PRICE_LEN
        mov     dh,ROW_BOTTOM-1
        mov     dl,[pay_col]
        mov     bl,IN_PRICE
        call    input_line
        cmp     al,K_ESC
        jne     .jf12
        jmp     .cancel
.jf12:
        cmp     al,K_ENTER
        jne     .input
        mov     si,pay_buf
        call    str_trim
        cmp     byte [si],0
        jne     .amount
        mov     si,sale_total           ; empty: exact amount
        mov     di,pay_amount
        mov     cx,3
        rep     movsw
        jmp     .change
.amount:
        call    parse_price
        jnc     .valid
        mov     si,err_price
        call    show_error
        jmp     .input
.valid:
        mov     [pay_amount],ax
        mov     [pay_amount+2],dx
        mov     word [pay_amount+4],0
.change:
        mov     si,pay_amount
        mov     di,change48
        mov     cx,3
        rep     movsw
        mov     di,change48
        mov     si,sale_total
        call    sub48
        jnc     .enough
        mov     si,err_paid_less
        call    show_error
        jmp     .input
.enough:
        ; take the sold items out of the stock
        xor     si,si
.stock:
        cmp     si,[cart_n]
        jae     .save
        mov     bx,si
        shl     bx,1
        mov     ax,[cart_rec+bx]
        push    ax
        call    rec_get
        sub     cx,[cart_qty+bx]
        pop     ax
        call    rec_set_qty
        inc     si
        jmp     .stock
.save:
        mov     si,msg_sale_done
        call    save_database
        ; show amount paid and change
        mov     dh,ROW_BOTTOM-1
        mov     cl,1
        call    clear_rows
        mov     di,msg_buf
        mov     si,str_paid
        call    str_append
        mov     si,pay_amount
        mov     bl,2
        call    n48_to_str
        call    currency_append
        mov     si,str_change
        call    str_append
        mov     si,change48
        call    n48_to_str
        call    currency_append
        mov     si,msg_buf
        mov     dx,((ROW_BOTTOM-1) shl 8) + 1
        mov     bl,A_BRIGHT
        call    put_str
        mov     si,[pending_msg]
        mov     word [pending_msg],0
        cmp     si,msg_sale_done
        jne     .result
        mov     si,msg_sale_next
.result:
        call    show_info
        call    get_key
        mov     [pending_msg],si
        cmp     si,msg_sale_next
        jne     .ok
        mov     word [pending_msg],msg_sale_done
.ok:
        clc
        jmp     .done
.cancel:
        mov     dh,ROW_BOTTOM-1
        mov     cl,1
        call    clear_rows
        call    clear_status
        stc
.done:
        pop     di
        pop     si
        pop     dx
        pop     cx
        pop     bx
        pop     ax
        ret

; ----------------------------------------------------------------------------
; F8 - Stock report (STOCK.TXT)
; ----------------------------------------------------------------------------
do_report:
        mov     si,sec_report
        mov     di,hint_back
        call    draw_frame
        mov     si,msg_creating
        call    show_info
        call    make_report
        jnc     .ok
        mov     si,err_report
        call    show_error
        call    get_key
        ret
.ok:
        mov     dx,((ROW_TOP+1) shl 8) + 4
        mov     si,msg_report_saved
        mov     bl,A_BRIGHT
        call    put_str
        xor     al,al
        mov     dh,ROW_TOP+3
.line:
        call    rpt_summary_line
        mov     dl,4
        mov     si,msg_buf
        mov     bl,A_NORMAL
        call    put_str
        inc     dh
        inc     al
        cmp     al,RPT_LINES
        jb      .line
        mov     si,msg_report_done
        call    show_ok
        call    get_key
        ret

; make_report - write STOCK.TXT. CF=1 on error
make_report:
        push    ax
        push    bx
        push    cx
        push    dx
        push    si
        push    di
        mov     dx,fn_stock
        call    fw_create
        jnc     .created
        jmp     .done
.created:
        xor     ax,ax
        mov     [rpt_in],ax
        mov     [rpt_out],ax
        mov     [rpt_low],ax
        mov     [rpt_qty],ax
        mov     [rpt_qty+2],ax
        mov     di,rpt_value
        call    zero48
        ; heading
        cmp     byte [cfg_store],0
        je      .title
        mov     si,cfg_store
        call    fw_line
.title:
        mov     si,rpt_title
        call    fw_line
        mov     di,msg_buf
        mov     si,rpt_date
        call    str_append
        call    date_append
        mov     si,rpt_time
        call    str_append
        call    time_append
        mov     si,msg_buf
        call    fw_line
        call    fw_crlf
        ; products in stock
        mov     si,rpt_sec_in
        call    fw_line
        mov     byte [rpt_pass],1
        call    rpt_table
        ; products out of stock
        mov     si,rpt_sec_out
        call    fw_line
        mov     byte [rpt_pass],0
        call    rpt_table
        ; summary
        mov     si,rpt_sec_sum
        call    fw_line
        mov     al,'-'
        mov     cx,RPT_WIDTH
        call    fw_repeat
        call    fw_crlf
        xor     al,al
.sum:
        call    rpt_summary_line
        mov     si,msg_buf
        call    fw_line
        inc     al
        cmp     al,RPT_LINES
        jb      .sum
        call    fw_close
.done:
        pop     di
        pop     si
        pop     dx
        pop     cx
        pop     bx
        pop     ax
        ret

; rpt_table - write the product table for rpt_pass (1 = in stock, 0 = out)
rpt_table:
        push    ax
        push    bx
        push    cx
        push    dx
        push    si
        push    di
        call    fmt_header_line
        cmp     byte [rpt_pass],0
        jne     .head
        mov     di,lb+COL_VAL           ; no value column for empty stock
        mov     cx,W_VAL
        mov     al,' '
        rep     stosb
.head:
        call    lb_to_file
        call    rpt_rule
        xor     ax,ax                   ; record index
        xor     dx,dx                   ; line number
.next:
        cmp     ax,[db_count]
        jae     .end
        push    ax
        push    dx
        call    rec_get
        pop     dx
        pop     ax
        cmp     byte [rpt_pass],0
        je      .want_empty
        jcxz    .skip
        jmp     .take
.want_empty:
        or      cx,cx
        jnz     .skip
.take:
        inc     dx
        push    cx
        mov     cx,dx
        call    fmt_product_line
        pop     cx
        cmp     byte [rpt_pass],0
        je      .out
        ; value = price * quantity
        push    ax
        push    dx
        inc     word [rpt_in]
        add     [rpt_qty],cx
        adc     word [rpt_qty+2],0
        cmp     cx,[cfg_low]
        ja      .not_low
        inc     word [rpt_low]
.not_low:
        mov     ax,[fpl_price]
        mov     dx,[fpl_price+2]
        mov     bx,cx
        mov     di,amount48
        call    mul32x16
        mov     si,amount48
        mov     di,rpt_value
        call    add48
        mov     di,num_str
        mov     bl,2
        call    n48_to_str
        mov     dx,COL_VAL
        mov     cx,W_VAL
        call    lb_num
        pop     dx
        pop     ax
        jmp     .write
.out:
        inc     word [rpt_out]
.write:
        call    lb_to_file
.skip:
        inc     ax
        jmp     .next
.end:
        or      dx,dx
        jnz     .rule
        mov     si,rpt_none
        call    fw_line
.rule:
        call    rpt_rule
        call    fw_crlf
        pop     di
        pop     si
        pop     dx
        pop     cx
        pop     bx
        pop     ax
        ret

rpt_rule:
        push    ax
        push    cx
        mov     al,'-'
        mov     cx,RPT_WIDTH
        call    fw_repeat
        call    fw_crlf
        pop     cx
        pop     ax
        ret

; rpt_summary_line - AL = line number (0..RPT_LINES-1)  ->  msg_buf
RPT_LINES       = 6
rpt_summary_line:
        push    ax
        push    bx
        push    dx
        push    si
        push    di
        xor     ah,ah
        mov     bx,ax
        shl     bx,1
        mov     di,msg_buf
        mov     si,[rpt_labels+bx]
        call    str_append
        cmp     al,0
        jne     .l1
        mov     ax,[rpt_in]
        jmp     .word
.l1:
        cmp     al,1
        jne     .l2
        mov     ax,[rpt_out]
        jmp     .word
.l2:
        cmp     al,2
        jne     .l3
        mov     ax,[rpt_low]
        call    u16_append
        mov     si,str_low_limit
        call    str_append
        mov     ax,[cfg_low]
        call    u16_append
        mov     al,')'
        call    chr_append
        jmp     .done
.l3:
        cmp     al,3
        jne     .l4
        mov     ax,[rpt_qty]
        mov     dx,[rpt_qty+2]
        xor     bl,bl
        call    u32_to_str
        jmp     .done
.l4:
        cmp     al,4
        jne     .l5
        mov     si,rpt_value
        mov     bl,2
        call    n48_to_str
        call    currency_append
        jmp     .done
.l5:
        mov     ax,[db_count]
.word:
        call    u16_append
.done:
        pop     di
        pop     si
        pop     dx
        pop     bx
        pop     ax
        ret

; ----------------------------------------------------------------------------
; F9 - Backup (DATA.DAT and STOCK.TXT to another drive)
; ----------------------------------------------------------------------------
do_backup:
        mov     si,sec_backup
        mov     di,hint_backup
        call    draw_frame
        mov     dx,((ROW_TOP+1) shl 8) + 4
        mov     si,backup_text
        mov     bl,A_NORMAL
        call    draw_lines
        mov     ah,19h                  ; current drive
        int     21h
        add     al,'A'
        mov     [cur_drive],al
        mov     di,msg_buf
        mov     si,lbl_cur_drive
        call    str_append
        call    chr_append
        mov     al,':'
        call    chr_append
        mov     si,msg_buf
        mov     dx,((ROW_TOP+5) shl 8) + 4
        call    put_str
        mov     dx,((ROW_TOP+7) shl 8) + 4
        mov     si,lbl_target_drive
        call    put_str
        mov     al,[cfg_backup]
        mov     [drive_buf],al
        mov     byte [drive_buf+1],0
.input:
        mov     si,drive_buf
        mov     cx,1
        mov     dx,((ROW_TOP+7) shl 8) + 20
        mov     bl,IN_UPPER
        call    input_line
        cmp     al,K_ESC
        jne     .enter
        ret
.enter:
        cmp     al,K_ENTER
        jne     .input
        mov     al,[drive_buf]
        cmp     al,'A'
        jb      .bad
        cmp     al,'Z'
        ja      .bad
        cmp     al,[cur_drive]
        je      .same
        mov     dl,al
        sub     dl,'A'-1
        mov     ah,36h                  ; drive present and ready?
        int     21h
        cmp     ax,0FFFFh
        je      .not_ready
        call    clear_status
        mov     al,[drive_buf]
        cmp     al,[cfg_backup]
        je      .start
        mov     [cfg_backup],al
        call    cfg_save
.start:
        call    db_save                 ; make sure DATA.DAT is up to date
        mov     dx,fn_stock
        call    file_exists
        jnc     .copy
        call    make_report
        mov     si,msg_report_made
        mov     dx,((ROW_TOP+9) shl 8) + 4
        mov     bl,A_NORMAL
        call    put_str
.copy:
        mov     dh,ROW_TOP+11
        mov     si,fn_data
        call    backup_one
        sbb     cl,cl                   ; CL = FFh on error
        inc     dh
        mov     si,fn_stock
        call    backup_one
        sbb     ch,ch
        mov     si,msg_backup_ok
        or      cx,cx
        jz      .result
        mov     si,err_backup
        call    show_error
        jmp     .wait
.result:
        call    show_ok
.wait:
        call    get_key
        ret
.bad:
        mov     si,err_drive
        call    show_error
        jmp     .input
.same:
        mov     si,err_same_drive
        call    show_error
        jmp     .input
.not_ready:
        mov     si,err_not_ready
        call    show_error
        jmp     .input

; backup_one - SI = file name, DH = screen row. Copies the file to the
;              root of the target drive. CF=1 on error
backup_one:
        push    ax
        push    bx
        push    dx
        push    si
        push    di
        mov     di,path_buf
        mov     al,[drive_buf]
        stosb
        mov     al,':'
        stosb
        mov     al,'\'
        stosb
        call    str_append
        mov     di,msg_buf
        push    si
        mov     si,str_copying
        call    str_append
        pop     si
        call    str_append
        push    si
        mov     si,str_to
        call    str_append
        mov     si,path_buf
        call    str_append
        mov     si,str_dots
        call    str_append
        mov     si,msg_buf
        mov     dl,4
        mov     bl,A_NORMAL
        call    put_str
        pop     si
        mov     di,path_buf
        call    copy_file
        mov     si,str_ok
        jnc     .result
        mov     si,str_failed
.result:
        pushf
        mov     bl,A_BRIGHT
        call    put_str
        popf
        pop     di
        pop     si
        pop     dx
        pop     bx
        pop     ax
        ret

; ----------------------------------------------------------------------------
; F10 - Settings (MARKET.CFG)
; ----------------------------------------------------------------------------
SET_COUNT       = 9
SET_ROW         = ROW_TOP+2
SET_LCOL        = 4
SET_VCOL        = 36

do_settings:
        mov     byte [set_sel],0
        mov     byte [set_changed],0
.frame:
        mov     si,sec_settings
        mov     di,hint_settings
        call    draw_frame
        mov     dx,((SET_ROW+SET_COUNT+2) shl 8) + SET_LCOL
        mov     si,settings_help
        mov     bl,A_NORMAL
        call    draw_lines
.draw:
        call    set_draw
.key:
        call    get_key
        or      al,al
        jnz     .ascii
        cmp     ah,S_UP
        jne     .down
        cmp     byte [set_sel],0
        je      .key
        dec     byte [set_sel]
        jmp     .draw
.down:
        cmp     ah,S_DOWN
        jne     .key
        cmp     byte [set_sel],SET_COUNT-1
        jae     .key
        inc     byte [set_sel]
        jmp     .draw
.ascii:
        cmp     al,K_ESC
        je      .exit
        cmp     al,K_ENTER
        je      .edit
        cmp     al,' '
        jne     .key
.edit:
        call    set_edit
        jmp     .frame
.exit:
        cmp     byte [set_changed],0
        je      .done
        call    cfg_save
        mov     word [pending_msg],msg_settings_saved
        jnc     .done
        mov     word [pending_msg],err_cfg_save
        call    beep_err
.done:
        ret

; set_entry - AL = setting number  ->  BX = table entry, DH = screen row
set_entry:
        push    ax
        push    cx
        xor     ah,ah
        mov     bx,ax
        mov     cl,3
        shl     bx,cl
        add     bx,set_table
        add     al,SET_ROW
        mov     dh,al
        pop     cx
        pop     ax
        ret

; set_draw - draw all settings with their values
set_draw:
        push    ax
        push    bx
        push    cx
        push    dx
        push    si
        xor     al,al
.next:
        call    set_entry
        push    bx
        mov     si,[bx]
        mov     dl,SET_LCOL
        mov     bl,A_NORMAL
        cmp     al,[set_sel]
        jne     .label
        mov     bl,A_REVERSE
.label:
        mov     cx,SET_VCOL-SET_LCOL-2
        call    put_field
        pop     bx
        call    set_value_text
        mov     si,msg_buf
        mov     dl,SET_VCOL
        mov     cx,80-SET_VCOL
        push    bx
        mov     bl,A_NORMAL
        call    put_field
        pop     bx
        inc     al
        cmp     al,SET_COUNT
        jb      .next
        pop     si
        pop     dx
        pop     cx
        pop     bx
        pop     ax
        ret

; set_value_text - BX = table entry  ->  value as text in msg_buf
set_value_text:
        push    ax
        push    bx
        push    si
        push    di
        mov     di,msg_buf
        mov     byte [di],0
        mov     si,[bx+4]
        mov     ax,[bx+2]
        cmp     al,T_STR
        jne     .bool
        cmp     byte [si],0
        jne     .str
        mov     si,str_none
.str:
        call    str_append
        jmp     .done
.bool:
        cmp     al,T_BOOL
        jne     .byte
        cmp     byte [si],0
        mov     si,str_on
        jne     .str
        mov     si,str_off
        jmp     .str
.byte:
        cmp     al,T_BYTE
        jne     .word
        mov     al,[si]
        xor     ah,ah
        call    u16_append
        mov     si,str_minutes
        jmp     .str
.word:
        cmp     al,T_WORD
        jne     .drive
        mov     ax,[si]
        call    u16_append
        jmp     .done
.drive:
        cmp     al,T_DRIVE
        jne     .pass
        mov     al,[si]
        call    chr_append
        mov     al,':'
        call    chr_append
        jmp     .done
.pass:
        cmp     byte [si],0
        mov     si,str_pass_set
        jne     .str
        mov     si,str_pass_none
        jmp     .str
.done:
        pop     di
        pop     si
        pop     bx
        pop     ax
        ret

; set_edit - change the selected setting
set_edit:
        push    ax
        push    bx
        push    cx
        push    dx
        push    si
        push    di
        mov     al,[set_sel]
        call    set_entry
        mov     di,[bx+4]               ; variable
        mov     cx,[bx+6]               ; parameter
        mov     ax,[bx+2]               ; type
        mov     dl,SET_VCOL
        cmp     al,T_BOOL
        jne     .str
        cmp     di,cfg_login            ; enabling login needs a password
        jne     .toggle
        cmp     byte [di],0
        jne     .toggle
        cmp     byte [cfg_pass],0
        jne     .toggle
        call    set_password
        jnc     .jf13
        jmp     .done
.jf13:
        cmp     byte [cfg_pass],0
        jne     .jf14
        jmp     .done
.jf14:
.toggle:
        xor     byte [di],1
        jmp     .changed
.str:
        cmp     al,T_STR
        jne     .number
        mov     si,di
        mov     di,edit_buf
        call    str_copy
        mov     si,di
        push    bx
        xor     bl,bl
        call    input_line
        pop     bx
        cmp     al,K_ENTER
        je      .jf15
        jmp     .done
.jf15:
        call    str_trim
        mov     di,[bx+4]
        call    str_copy
        jmp     .changed
.number:
        cmp     al,T_BYTE
        je      .num
        cmp     al,T_WORD
        jne     .drive
.num:
        push    ax
        mov     ax,[di]
        cmp     byte [bx+2],T_BYTE
        jne     .num_text
        xor     ah,ah
.num_text:
        mov     di,edit_buf
        call    u16_append
        pop     ax
.num_input:
        mov     si,edit_buf
        push    cx
        mov     cx,2                    ; width: 2 digits (minutes) ...
        cmp     byte [bx+2],T_BYTE
        je      .num_width
        mov     cx,4                    ; ... or 4 digits (stock level)
.num_width:
        mov     bl,IN_DIGITS
        call    input_line
        pop     cx
        call    .restore                ; BX, DH/DL
        cmp     al,K_ENTER
        je      .num_check
        jmp     .done_clear
.num_check:
        call    parse_u32
        jc      .num_bad
        or      dx,dx
        jnz     .num_bad
        cmp     ax,cx
        ja      .num_bad
        mov     di,[bx+4]
        cmp     byte [bx+2],T_BYTE
        jne     .num_word
        or      ax,ax
        jz      .num_bad
        mov     [di],al
        jmp     .changed
.num_word:
        mov     [di],ax
        jmp     .changed
.num_bad:
        call    .restore
        mov     si,err_range
        call    show_error
        jmp     .num_input
; restore BX (table entry) and DH/DL (value position), AX is kept
.restore:
        push    ax
        mov     al,[set_sel]
        call    set_entry
        mov     dl,SET_VCOL
        pop     ax
        ret
.drive:
        cmp     al,T_DRIVE
        jne     .pass
        mov     al,[di]
        mov     [edit_buf],al
        mov     byte [edit_buf+1],0
.drive_input:
        mov     si,edit_buf
        mov     cx,1
        mov     bl,IN_UPPER
        call    input_line
        cmp     al,K_ENTER
        jne     .done_clear
        mov     al,[edit_buf]
        cmp     al,'A'
        jb      .drive_bad
        cmp     al,'Z'
        ja      .drive_bad
        mov     [di],al
        jmp     .changed
.drive_bad:
        mov     si,err_drive
        call    show_error
        jmp     .drive_input
.pass:
        call    set_password
        jmp     .done
.changed:
        mov     byte [set_changed],1
.done_clear:
        call    clear_status
.done:
        pop     di
        pop     si
        pop     dx
        pop     cx
        pop     bx
        pop     ax
        ret

; set_password - change the password (asks the current one first).
;                CF=1 if cancelled or failed. Messages go to pending_msg.
set_password:
        push    ax
        push    bx
        push    cx
        push    dx
        push    si
        push    di
        mov     dh,SET_ROW+SET_COUNT+1
        mov     cl,ROW_BOTTOM-SET_ROW-SET_COUNT
        call    clear_rows
        mov     bl,A_NORMAL
        cmp     byte [cfg_pass],0
        je      .new
        mov     dx,((SET_ROW+SET_COUNT+2) shl 8) + SET_LCOL
        mov     si,lbl_pw_current
        call    put_str
        mov     dh,SET_ROW+SET_COUNT+2
        mov     si,pw_buf
        call    pw_input
        jc      .cancel
        mov     di,cfg_pass
        call    str_cmp
        je      .new
        mov     word [pending_msg],err_wrong_password
        call    beep_err
        jmp     .cancel
.new:
        mov     dx,((SET_ROW+SET_COUNT+3) shl 8) + SET_LCOL
        mov     si,lbl_pw_new
        call    put_str
        mov     dh,SET_ROW+SET_COUNT+3
        mov     si,pw_buf
        call    pw_input
        jc      .cancel
        mov     dx,((SET_ROW+SET_COUNT+4) shl 8) + SET_LCOL
        mov     si,lbl_pw_again
        call    put_str
        mov     dh,SET_ROW+SET_COUNT+4
        mov     si,pw_buf2
        call    pw_input
        jc      .cancel
        mov     si,pw_buf
        mov     di,pw_buf2
        call    str_cmp
        je      .store
        mov     word [pending_msg],err_pw_mismatch
        call    beep_err
        jmp     .cancel
.store:
        mov     di,cfg_pass
        call    str_copy
        mov     byte [set_changed],1
        mov     word [pending_msg],msg_pw_changed
        cmp     byte [cfg_pass],0
        jne     .ok
        mov     byte [cfg_login],0
        mov     word [pending_msg],msg_pw_removed
.ok:
        clc
        jmp     .done
.cancel:
        stc
.done:
        pop     di
        pop     si
        pop     dx
        pop     cx
        pop     bx
        pop     ax
        ret

; pw_input - masked input of SI at row DH (column SET_VCOL). CF=1 on Esc
pw_input:
        push    ax
        push    bx
        push    cx
        push    dx
        mov     byte [si],0
.input:
        mov     cx,PASS_LEN
        mov     dl,SET_VCOL
        mov     bl,IN_PASSWORD
        call    input_line
        cmp     al,K_ESC
        je      .esc
        cmp     al,K_ENTER
        jne     .input
        clc
        jmp     .done
.esc:
        stc
.done:
        pop     dx
        pop     cx
        pop     bx
        pop     ax
        ret

; ----------------------------------------------------------------------------
; F11 - About
; ----------------------------------------------------------------------------
do_about:
        mov     si,sec_about
        mov     di,hint_back
        call    draw_frame
        mov     dx,((ROW_TOP+1) shl 8) + 9
        mov     cx,((ROW_BOTTOM-1) shl 8) + 70
        mov     bl,A_NORMAL
        call    draw_box
        mov     si,about_text
        mov     dh,ROW_TOP+3
        mov     bl,A_BRIGHT
.line:
        cmp     byte [si],0FFh
        je      .count
        call    put_center
        call    str_len
        add     si,cx
        inc     si
        inc     dh
        mov     bl,A_NORMAL
        jmp     .line
.count:
        mov     di,msg_buf
        mov     si,str_about_count
        call    str_append
        mov     ax,[db_count]
        call    u16_append
        mov     si,str_sum_max
        call    str_append
        mov     si,msg_buf
        inc     dh
        call    put_center
        call    get_key
        ret
