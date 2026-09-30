; ============================================================================
;  DATABASE.INC - Product database (DATA.DAT)
; ============================================================================
;
;  DATA.DAT is a plain text file, one product per line:
;
;      BARCODE;PRODUCT NAME;PRICE;QUANTITY
;
;  PRICE is written with two decimals (for example 12.50). Lines starting
;  with ';' are comments. In memory the products are stored as fixed size
;  records (REC_SIZE bytes) in a separate 64 KB segment (rec_seg).

; ----------------------------------------------------------------------------
; rec_offset - AX = record index  ->  BX = offset in the record segment
; ----------------------------------------------------------------------------
rec_offset:
        push    ax
        push    cx
        mov     cl,REC_SHIFT
        shl     ax,cl
        mov     bx,ax
        pop     cx
        pop     ax
        ret

; ----------------------------------------------------------------------------
; far_to_near - copy string rec_seg:SI to DS:DI, at most CX characters
; ----------------------------------------------------------------------------
far_to_near:
        push    ax
        push    cx
        push    si
        push    di
        push    ds
        mov     ds,[rec_seg]
.next:
        jcxz    .end
        lodsb
        or      al,al
        jz      .end
        stosb
        dec     cx
        jmp     .next
.end:
        xor     al,al
        stosb
        pop     ds
        pop     di
        pop     si
        pop     cx
        pop     ax
        ret

; ----------------------------------------------------------------------------
; near_to_far - copy string DS:SI to rec_seg:DI; CX = field size including
;               the terminator. The rest of the field is filled with zeros.
; ----------------------------------------------------------------------------
near_to_far:
        push    ax
        push    cx
        push    si
        push    di
        push    es
        mov     es,[rec_seg]
        dec     cx
.next:
        jcxz    .fill
        lodsb
        or      al,al
        jz      .fill
        stosb
        dec     cx
        jmp     .next
.fill:
        inc     cx
        xor     al,al
        rep     stosb
        pop     es
        pop     di
        pop     si
        pop     cx
        pop     ax
        ret

; ----------------------------------------------------------------------------
; rec_get - AX = record index  ->  DX:AX = price, CX = quantity
; ----------------------------------------------------------------------------
rec_get:
        push    bx
        push    es
        call    rec_offset
        mov     es,[rec_seg]
        mov     cx,[es:bx+R_QTY]
        mov     ax,[es:bx+R_PRICE]
        mov     dx,[es:bx+R_PRICE+2]
        pop     es
        pop     bx
        ret

; ----------------------------------------------------------------------------
; rec_set_qty - AX = record index, CX = new quantity
; ----------------------------------------------------------------------------
rec_set_qty:
        push    bx
        push    es
        call    rec_offset
        mov     es,[rec_seg]
        mov     [es:bx+R_QTY],cx
        pop     es
        pop     bx
        ret

; ----------------------------------------------------------------------------
; rec_name - AX = record index  ->  copy of the name in tmp_str (SI=tmp_str)
; rec_barcode - same for the barcode
; ----------------------------------------------------------------------------
rec_name:
        push    bx
        push    cx
        push    di
        call    rec_offset
        lea     si,[bx+R_NAME]
        mov     cx,NAME_LEN
        jmp     rec_str_copy

rec_barcode:
        push    bx
        push    cx
        push    di
        call    rec_offset
        lea     si,[bx+R_BARCODE]
        mov     cx,BAR_LEN

rec_str_copy:
        mov     di,tmp_str
        call    far_to_near
        mov     si,di
        pop     di
        pop     cx
        pop     bx
        ret

; ----------------------------------------------------------------------------
; rec_to_edit - copy record AX into the edit buffers (ed_*)
; ----------------------------------------------------------------------------
rec_to_edit:
        push    ax
        push    bx
        push    cx
        push    dx
        push    si
        push    di
        push    ax
        call    rec_offset
        lea     si,[bx+R_BARCODE]
        mov     di,ed_barcode
        mov     cx,BAR_LEN
        call    far_to_near
        lea     si,[bx+R_NAME]
        mov     di,ed_name
        mov     cx,NAME_LEN
        call    far_to_near
        pop     ax
        call    rec_get
        mov     di,ed_price
        mov     bl,2
        call    u32_to_str
        mov     ax,cx
        mov     di,ed_qty
        call    u16_append
        pop     di
        pop     si
        pop     dx
        pop     cx
        pop     bx
        pop     ax
        ret

; ----------------------------------------------------------------------------
; edit_to_rec - store the (validated) edit buffers into record AX
; ----------------------------------------------------------------------------
edit_to_rec:
        push    ax
        push    bx
        push    cx
        push    dx
        push    si
        push    di
        push    es
        call    rec_offset
        mov     si,ed_barcode
        lea     di,[bx+R_BARCODE]
        mov     cx,BAR_LEN+1
        call    near_to_far
        mov     si,ed_name
        lea     di,[bx+R_NAME]
        mov     cx,NAME_LEN+1
        call    near_to_far
        mov     es,[rec_seg]
        mov     si,ed_price
        call    parse_price
        mov     [es:bx+R_PRICE],ax
        mov     [es:bx+R_PRICE+2],dx
        mov     si,ed_qty
        call    parse_qty
        mov     [es:bx+R_QTY],ax
        pop     es
        pop     di
        pop     si
        pop     dx
        pop     cx
        pop     bx
        pop     ax
        ret

; ----------------------------------------------------------------------------
; db_find_barcode - SI = barcode  ->  AX = record index, CF=1 if not found
; ----------------------------------------------------------------------------
db_find_barcode:
        push    bx
        push    cx
        push    dx
        push    si
        push    di
        push    es
        call    make_needle             ; SI = upper case copy
        mov     es,[rec_seg]
        xor     ax,ax
        mov     bx,R_BARCODE
.next:
        cmp     ax,[db_count]
        jae     .missing
        mov     si,flt_needle
        mov     di,bx
.cmp:
        mov     dl,[si]
        mov     cl,[es:di]
        cmp     cl,'a'
        jb      .upper
        cmp     cl,'z'
        ja      .upper
        sub     cl,'a'-'A'
.upper:
        cmp     cl,dl
        jne     .differ
        or      cl,cl
        jz      .found
        inc     si
        inc     di
        jmp     .cmp
.differ:
        add     bx,REC_SIZE
        inc     ax
        jmp     .next
.found:
        clc
        jmp     .done
.missing:
        stc
.done:
        pop     es
        pop     di
        pop     si
        pop     dx
        pop     cx
        pop     bx
        ret

; ----------------------------------------------------------------------------
; make_needle - copy string SI to flt_needle in upper case (SI = flt_needle)
; ----------------------------------------------------------------------------
make_needle:
        push    di
        mov     di,flt_needle
        call    str_copy
        mov     si,di
        call    str_upper
        pop     di
        ret

; ----------------------------------------------------------------------------
; far_contains - CF=0 if flt_needle (upper case, not empty) occurs in the
;                string at ES:DI (ignoring case)
; ----------------------------------------------------------------------------
far_contains:
        push    ax
        push    bx
        push    si
        push    di
        mov     bl,[flt_needle]
.outer:
        mov     al,[es:di]
        or      al,al
        jz      .missing
        inc     di
        cmp     al,'a'
        jb      .first
        cmp     al,'z'
        ja      .first
        sub     al,'a'-'A'
.first:
        cmp     al,bl
        jne     .outer
        push    di                      ; first character matches
        mov     si,flt_needle+1
.inner:
        mov     ah,[si]
        or      ah,ah
        jz      .found
        mov     al,[es:di]
        cmp     al,'a'
        jb      .cmp
        cmp     al,'z'
        ja      .cmp
        sub     al,'a'-'A'
.cmp:
        cmp     al,ah
        jne     .again
        inc     si
        inc     di
        jmp     .inner
.again:
        pop     di
        jmp     .outer
.found:
        pop     di
        clc
        jmp     .done
.missing:
        stc
.done:
        pop     di
        pop     si
        pop     bx
        pop     ax
        ret

; ----------------------------------------------------------------------------
; db_append - AX = index of a new record at the end, CF=1 if database full
; ----------------------------------------------------------------------------
db_append:
        mov     ax,[db_count]
        cmp     ax,MAX_REC
        jae     .full
        inc     word [db_count]
        clc
        ret
.full:
        stc
        ret

; ----------------------------------------------------------------------------
; db_delete - remove record AX
; ----------------------------------------------------------------------------
db_delete:
        push    ax
        push    bx
        push    cx
        push    si
        push    di
        push    ds
        push    es
        dec     word [db_count]
        mov     cx,[db_count]
        sub     cx,ax                   ; records after the deleted one
        call    rec_offset
        mov     di,bx
        lea     si,[bx+REC_SIZE]
        mov     ax,cx
        mov     cl,REC_SHIFT-1          ; words to move
        shl     ax,cl
        mov     cx,ax
        mov     ax,[rec_seg]
        mov     es,ax
        mov     ds,ax
        rep     movsw
        pop     es
        pop     ds
        pop     di
        pop     si
        pop     cx
        pop     bx
        pop     ax
        ret

; ----------------------------------------------------------------------------
; db_filter - SI = search text. Fills idx_buf with the records whose
;             barcode or name contains the text (lv_count = matches).
; ----------------------------------------------------------------------------
db_filter:
        push    ax
        push    bx
        push    cx
        push    si
        push    di
        push    es
        call    make_needle
        mov     es,[rec_seg]
        xor     ax,ax
        xor     cx,cx
        xor     bx,bx                   ; record offset
.next:
        cmp     ax,[db_count]
        jae     .done
        cmp     byte [flt_needle],0     ; empty text matches everything
        je      .match
        lea     di,[bx+R_BARCODE]
        call    far_contains
        jnc     .match
        lea     di,[bx+R_NAME]
        call    far_contains
        jc      .skip
.match:
        mov     di,cx
        shl     di,1
        mov     [idx_buf+di],ax
        inc     cx
.skip:
        add     bx,REC_SIZE
        inc     ax
        jmp     .next
.done:
        mov     [lv_count],cx
        pop     es
        pop     di
        pop     si
        pop     cx
        pop     bx
        pop     ax
        ret

; ----------------------------------------------------------------------------
; db_filter_all - idx_buf = all records
; ----------------------------------------------------------------------------
db_filter_all:
        push    si
        mov     si,empty_str
        call    db_filter
        pop     si
        ret

; ----------------------------------------------------------------------------
; db_stats - count low stock (st_low) and out of stock (st_out) products
; ----------------------------------------------------------------------------
db_stats:
        push    ax
        push    cx
        push    dx
        xor     ax,ax
        mov     [st_low],ax
        mov     [st_out],ax
.next:
        cmp     ax,[db_count]
        jae     .done
        push    ax
        call    rec_get
        pop     ax
        jcxz    .out
        cmp     cx,[cfg_low]
        ja      .skip
        inc     word [st_low]
        jmp     .skip
.out:
        inc     word [st_out]
.skip:
        inc     ax
        jmp     .next
.done:
        pop     dx
        pop     cx
        pop     ax
        ret

; ----------------------------------------------------------------------------
; db_load - read DATA.DAT (a missing file means an empty database)
; ----------------------------------------------------------------------------
db_load:
        push    ax
        push    cx
        push    dx
        push    si
        push    di
        xor     ax,ax
        mov     [db_count],ax
        mov     [load_bad],ax
        mov     [load_full],al
        mov     dx,fn_data
        call    fr_open
        jc      .done
.line:
        mov     di,line_buf
        mov     cx,LINE_MAX
        call    fr_readline
        jc      .eof
        mov     si,di
        call    str_trim
        cmp     byte [si],0
        je      .line
        cmp     byte [si],';'           ; comment
        je      .line
        call    parse_data_line
        jnc     .line
        inc     word [load_bad]
        jmp     .line
.eof:
        call    fr_close
.done:
        pop     di
        pop     si
        pop     dx
        pop     cx
        pop     ax
        ret

; ----------------------------------------------------------------------------
; parse_data_line - SI = "BARCODE;NAME;PRICE;QTY", add it to the database.
;                   CF=1 if the line is invalid.
; ----------------------------------------------------------------------------
parse_data_line:
        push    ax
        push    bx
        push    cx
        push    dx
        push    si
        push    di
        mov     [fld_ptr],si
        mov     bx,2
.scan:
        lodsb
        or      al,al
        jz      .split
        cmp     al,';'
        jne     .scan
        cmp     bx,8
        jb      .field
        jmp     .bad
.field:
        mov     byte [si-1],0
        mov     [fld_ptr+bx],si
        inc     bx
        inc     bx
        jmp     .scan
.split:
        cmp     bx,8
        je      .fields_ok
.bad_far:
        jmp     .bad
.fields_ok:
        ; barcode: 1..BAR_LEN characters
        mov     si,[fld_ptr]
        call    str_trim
        call    str_len
        jcxz    .bad_far
        cmp     cx,BAR_LEN
        ja      .bad_far
        ; name: 1..NAME_LEN characters (longer names are cut)
        mov     si,[fld_ptr+2]
        call    str_trim
        call    str_len
        jcxz    .bad_far
        cmp     cx,NAME_LEN
        jbe     .name_ok
        mov     byte [si+NAME_LEN],0
.name_ok:
        ; quantity
        mov     si,[fld_ptr+6]
        call    str_trim
        call    parse_qty
        jc      .bad_far
        mov     cx,ax
        ; price
        mov     si,[fld_ptr+4]
        call    str_trim
        call    parse_price
        jc      .bad_far
        ; store
        push    ax
        call    db_append
        pop     ax
        jc      .full
        push    ax
        push    dx
        mov     ax,[db_count]
        dec     ax
        call    rec_set_qty
        call    rec_offset
        mov     si,[fld_ptr]
        lea     di,[bx+R_BARCODE]
        mov     cx,BAR_LEN+1
        call    near_to_far
        mov     si,[fld_ptr+2]
        lea     di,[bx+R_NAME]
        mov     cx,NAME_LEN+1
        call    near_to_far
        pop     dx
        pop     ax
        push    es
        mov     es,[rec_seg]
        mov     [es:bx+R_PRICE],ax
        mov     [es:bx+R_PRICE+2],dx
        pop     es
        clc
        jmp     .done
.full:
        mov     byte [load_full],1
        clc
        jmp     .done
.bad:
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
; db_save - write DATA.DAT (through DATA.TMP). CF=1 on error
; ----------------------------------------------------------------------------
db_save:
        push    ax
        push    bx
        push    cx
        push    dx
        push    si
        push    di
        mov     dx,fn_tmp
        call    fw_create
        jc      .done
        mov     si,data_header
        call    fw_puts
        xor     ax,ax
.next:
        cmp     ax,[db_count]
        jae     .close
        call    rec_offset
        lea     si,[bx+R_BARCODE]
        call    fw_puts_far
        mov     si,str_semicolon
        call    fw_puts
        lea     si,[bx+R_NAME]
        call    fw_puts_far
        mov     si,str_semicolon
        call    fw_puts
        push    ax
        call    rec_get                 ; DX:AX = price, CX = quantity
        mov     di,num_str
        mov     bl,2
        call    u32_to_str
        mov     al,';'
        call    chr_append
        mov     ax,cx
        call    u16_append
        mov     si,num_str
        call    fw_line
        pop     ax
        inc     ax
        jmp     .next
.close:
        call    fw_close
        jc      .failed
        mov     ah,41h                  ; delete old DATA.DAT
        mov     dx,fn_data
        int     21h
        mov     ah,56h                  ; rename DATA.TMP -> DATA.DAT
        mov     dx,fn_tmp
        mov     di,fn_data
        int     21h
        jmp     .done
.failed:
        mov     ah,41h
        mov     dx,fn_tmp
        int     21h
        stc
.done:
        pop     di
        pop     si
        pop     dx
        pop     cx
        pop     bx
        pop     ax
        ret
