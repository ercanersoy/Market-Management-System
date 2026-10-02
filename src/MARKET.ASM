; ============================================================================
;
;  MARKET.COM - Market Management System
;
;  Target   : IBM PC/XT compatible (Intel 8088), MDA, CGA, EGA or VGA,
;             MS-DOS 3.0 or later
;  Assembler: flat assembler (FASM)
;  Build    : fasm MARKET.ASM ../bin/MARKET.COM
;
;  Files used by the program (in the current directory):
;      DATA.DAT    product database (text)
;      MARKET.CFG  settings (text)
;      STOCK.TXT   stock report (text, created on request)
;
;  Copyright (c) 2026 Ercan Ersoy - released under the MIT License
;
; ============================================================================

        format  binary as 'COM'
        use16
        org     100h

include 'MACROS.INC'
include 'CONST.INC'

; ----------------------------------------------------------------------------
; Program entry
; ----------------------------------------------------------------------------
start:
        cld
        ; MS-DOS 3.0 or later is needed (critical error "Fail" support)
        mov     ah,30h
        int     21h
        cmp     al,3
        jae     .dos_ok
        mov     dx,msg_need_dos
        mov     ah,09h
        int     21h
        int     20h
.dos_ok:
        ; keep 64 KB for the program, allocate a segment for the products
        mov     ah,4Ah                  ; ES = PSP
        mov     bx,1000h
        int     21h
        jc      .no_memory
        mov     ah,48h
        mov     bx,REC_PARAS
        int     21h
        jnc     .memory_ok
.no_memory:
        mov     dx,msg_no_memory
        mov     ah,09h
        int     21h
        int     20h
.memory_ok:
        mov     [rec_seg],ax
        push    cs
        pop     es
        ; Ctrl+Break and critical error handlers (restored by DOS on exit)
        mov     ax,2523h
        mov     dx,int23_handler
        int     21h
        mov     ax,2524h
        mov     dx,int24_handler
        int     21h
        ; initialise
        call    get_ticks
        mov     [rnd_seed],ax
        call    kbd_init
        call    video_init
        call    cfg_load
        call    db_load
        call    load_warning
        call    reset_idle
        ; password login
        call    password_needed
        jc      .menu
        call    login
        jc      .quit
.menu:
        call    main_menu
.quit:
        call    video_done
        mov     dx,[exit_msg]
        or      dx,dx
        jz      .exit
        mov     ah,09h
        int     21h
.exit:
        mov     ax,4C00h
        int     21h

; ----------------------------------------------------------------------------
; load_warning - prepare a message if DATA.DAT had problems
; ----------------------------------------------------------------------------
load_warning:
        push    ax
        push    si
        push    di
        mov     si,msg_db_full_load
        cmp     byte [load_full],0
        jne     .set
        mov     ax,[load_bad]
        or      ax,ax
        jz      .done
        mov     di,warn_buf
        mov     si,msg_bad_lines1
        call    str_append
        call    u16_append
        mov     si,msg_bad_lines2
        call    str_append
        mov     si,warn_buf
.set:
        mov     [pending_msg],si
.done:
        pop     di
        pop     si
        pop     ax
        ret

; ----------------------------------------------------------------------------
; Interrupt handlers
; ----------------------------------------------------------------------------
int23_handler:                          ; Ctrl+Break / Ctrl+C: ignore
        iret

int24_handler:                          ; critical error: "Fail" the call
        mov     al,3
        iret

; ----------------------------------------------------------------------------
; Modules
; ----------------------------------------------------------------------------
include 'VIDEO.INC'
include 'KEYBOARD.INC'
include 'STRING.INC'
include 'FILE.INC'
include 'DATABASE.INC'
include 'CONFIG.INC'
include 'UI.INC'
include 'SCREENS.INC'

code_end:

include 'DATA.INC'
