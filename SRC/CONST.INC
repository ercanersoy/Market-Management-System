; ============================================================================
;  CONST.INC - Constants
; ============================================================================

; ---- Program limits --------------------------------------------------------
MAX_REC         = 1000                  ; maximum number of products
REC_SIZE        = 64                    ; bytes per product record
REC_SHIFT       = 6                     ; log2(REC_SIZE)
REC_PARAS       = (MAX_REC*REC_SIZE+15)/16

BAR_LEN         = 20                    ; barcode length (characters)
NAME_LEN        = 30                    ; product name length
PRICE_LEN       = 9                     ; price input length ("999999.99")
QTY_LEN         = 5                     ; quantity input length (0..65535)
MAX_PRICE       = 99999999              ; maximum price in cents
STORE_LEN       = 30                    ; store name length
CUR_LEN         = 4                     ; currency symbol length
PASS_LEN        = 12                    ; password length
QUERY_LEN       = 30                    ; search text length
SALE_LEN        = 26                    ; "QTY*BARCODE" input length
MAX_CART        = 100                   ; maximum lines in one sale
MAX_SALE_QTY    = 9999                  ; maximum quantity typed in a sale

LINE_MAX        = 127                   ; maximum text file line length
FBUF_SIZE       = 2048                  ; file I/O buffer size
LB_SIZE         = 100                   ; line builder buffer size

; ---- Product record layout (in the record segment) -------------------------
R_BARCODE       = 0                     ; BAR_LEN+1 bytes, zero terminated
R_NAME          = 21                    ; NAME_LEN+1 bytes, zero terminated
R_PRICE         = 52                    ; dword, price in cents
R_QTY           = 56                    ; word, quantity in stock

; ---- Screen layout ---------------------------------------------------------
ROW_TITLE       = 0
ROW_SECTION     = 1
ROW_TOP         = 3                     ; first row of the work area
ROW_BOTTOM      = 21                    ; last row of the work area
ROW_STATUS      = 23
ROW_HINT        = 24

; ---- Text attributes (MDA) -------------------------------------------------
A_NORMAL        = 07h
A_BRIGHT        = 0Fh
A_REVERSE       = 70h
A_INPUT         = 70h

; ---- ASCII key codes -------------------------------------------------------
K_BS            = 08h
K_TAB           = 09h
K_ENTER         = 0Dh
K_ESC           = 1Bh
K_CTRL_Y        = 19h
K_CTRL_BS       = 7Fh

; ---- Keyboard scan codes (extended keys, AL = 0) ---------------------------
S_STAB          = 0Fh
S_F1            = 3Bh
S_F2            = 3Ch
S_F3            = 3Dh
S_F4            = 3Eh
S_F10           = 44h
S_HOME          = 47h
S_UP            = 48h
S_PGUP          = 49h
S_LEFT          = 4Bh
S_RIGHT         = 4Dh
S_END           = 4Fh
S_DOWN          = 50h
S_PGDN          = 51h
S_DEL           = 53h

; ---- input_line flags ------------------------------------------------------
IN_DIGITS       = 01h                   ; digits only
IN_PRICE        = 02h                   ; digits and decimal point
IN_UPPER        = 04h                   ; convert letters to upper case
IN_PASSWORD     = 08h                   ; show '*' instead of characters

; ---- Column layout of product lines (screen and STOCK.TXT) -----------------
COL_NO          = 0
W_NO            = 4
COL_BAR         = 5
W_BAR           = 20
COL_NAME        = 26
W_NAME          = 30
COL_PRICE       = 57
W_PRICE         = 11
COL_QTY         = 69
W_QTY           = 6
COL_ST          = 77
W_ST            = 3
COL_VAL         = 81                    ; report only
W_VAL           = 14
RPT_WIDTH       = COL_VAL+W_VAL

; ---- Column layout of sales (cart) lines -----------------------------------
CC_NO           = 0
CW_NO           = 3
CC_BAR          = 4
CW_BAR          = 20
CC_NAME         = 25
CW_NAME         = 24
CC_QTY          = 50
CW_QTY          = 5
CC_PRICE        = 56
CW_PRICE        = 10
CC_AMOUNT       = 67
CW_AMOUNT       = 12
CART_FIRST      = 6                     ; first cart row on screen
CART_ROWS       = 13                    ; visible cart rows

; ---- Settings value types --------------------------------------------------
T_STR           = 0
T_BOOL          = 1
T_BYTE          = 2                     ; 1..param
T_WORD          = 3                     ; 0..param
T_DRIVE         = 4
T_PASS          = 5

TICKS_PER_MIN   = 1092                  ; BIOS timer ticks per minute
