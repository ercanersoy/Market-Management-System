; ============================================================================
;  DATA.INC - Texts, tables and variables
; ============================================================================

CR_LF           equ 13,10

; ----------------------------------------------------------------------------
; DOS messages ('$' terminated)
; ----------------------------------------------------------------------------
msg_need_dos        db 'Market Management System needs MS-DOS 3.0 or later.',CR_LF,'$'
msg_no_memory       db 'Not enough memory.',CR_LF,'$'
msg_access_denied   db 'Access denied: wrong password.',CR_LF,'$'

; ----------------------------------------------------------------------------
; File names
; ----------------------------------------------------------------------------
fn_data             db 'DATA.DAT',0
fn_tmp              db 'DATA.TMP',0
fn_cfg              db 'MARKET.CFG',0
fn_stock            db 'STOCK.TXT',0

data_header         db '; DATA.DAT - Market Management System product database',CR_LF
                    db '; BARCODE;PRODUCT NAME;PRICE;QUANTITY',CR_LF,0

; ----------------------------------------------------------------------------
; Settings file
; ----------------------------------------------------------------------------
cfg_header          db '; MARKET.CFG - Market Management System settings',CR_LF
                    db '; STORE     = store name shown in the title bar and in the report',CR_LF
                    db '; CURRENCY  = currency symbol',CR_LF
                    db '; SOUND     = PC speaker sound (ON/OFF)',CR_LF
                    db '; SAVER     = screen saver (ON/OFF), SAVERTIME = delay in minutes (1-60)',CR_LF
                    db '; LOGIN     = ask for the password at start-up (ON/OFF)',CR_LF
                    db '; PASSWORD  = encoded password, change it on the Settings screen (F10)',CR_LF
                    db '; LOWSTOCK  = products with this quantity or less are "low stock"',CR_LF
                    db '; BACKUP    = default backup drive letter',CR_LF,0

def_store           db 'MY MARKET',0
def_currency        db 'TL',0

key_store           db 'STORE',0
key_currency        db 'CURRENCY',0
key_sound           db 'SOUND',0
key_saver           db 'SAVER',0
key_savertime       db 'SAVERTIME',0
key_login           db 'LOGIN',0
key_password        db 'PASSWORD',0
key_lowstock        db 'LOWSTOCK',0
key_backup          db 'BACKUP',0

; key, type, variable, parameter
cfg_table           dw key_store,     T_STR,   cfg_store,     STORE_LEN
                    dw key_currency,  T_STR,   cfg_currency,  CUR_LEN
                    dw key_sound,     T_BOOL,  cfg_sound,     0
                    dw key_saver,     T_BOOL,  cfg_saver,     0
                    dw key_savertime, T_BYTE,  cfg_saver_min, 60
                    dw key_login,     T_BOOL,  cfg_login,     0
                    dw key_password,  T_PASS,  cfg_pass,      PASS_LEN
                    dw key_lowstock,  T_WORD,  cfg_low,       9999
                    dw key_backup,    T_DRIVE, cfg_backup,    0
                    dw 0

str_on              db 'ON',0
str_off             db 'OFF',0

; ----------------------------------------------------------------------------
; General texts
; ----------------------------------------------------------------------------
str_app_title       db 'MARKET MANAGEMENT SYSTEM',0
str_title_sep       db ' - ',0
empty_str           db 0
str_semicolon       db ';',0
msg_any_key         db 'Press any key to continue.',0
msg_saver           db 'Press any key to continue.',0
msg_saver_len       = $ - msg_saver - 1

; ---- Section titles --------------------------------------------------------
sec_main            db 'MAIN MENU',0
sec_login           db 'LOGIN',0
sec_help            db 'HELP AND ABOUT',0
sec_products        db 'PRODUCTS',0
sec_add             db 'ADD PRODUCT',0
sec_update          db 'UPDATE PRODUCT',0
sec_delete          db 'DELETE PRODUCT',0
sec_sell            db 'SALES (CHECKOUT)',0
sec_report          db 'STOCK REPORT',0
sec_backup          db 'BACKUP',0
sec_settings        db 'SETTINGS',0

; ---- Key hints (bottom line) -----------------------------------------------
hint_main           db 'Press F1-F10 to select a function',0
hint_login          db 'Enter = Log in   Esc = Exit to DOS',0
hint_back           db 'Press any key to return to the main menu',0
hint_prod_find      db 'Enter/Down = Search and go to the list (empty = all)   Esc = Back',0
hint_form           db 'Enter = Next/Save   Up/Down = Field   Ctrl+Y = Clear field   Esc = Cancel',0
hint_select         db 'Type or scan a barcode (or a part of a name) and press Enter   Esc = Back',0
hint_prod_list      db 'Up/Down/PgUp/PgDn = Move   Enter = Edit   Del = Delete   Esc = Search',0
hint_sell           db 'Enter = Add item   F2 = Payment   F3 = Remove last item   Esc = Cancel sale',0
hint_backup         db 'Enter = Start backup   Esc = Back',0
hint_settings       db 'Up/Down = Select   Enter/Space = Change   Esc = Save and return',0

; ---- Main menu -------------------------------------------------------------
menu_texts          dw mt_help, mt_products, mt_add, mt_update, mt_delete
                    dw mt_sell, mt_report, mt_backup, mt_settings, mt_exit
menu_handlers       dw do_help, do_products, do_add, do_update, do_delete
                    dw do_sell, do_report, do_backup, do_settings

mt_help             db 'F1    Help and About',0
mt_products         db 'F2    Products (list and search)',0
mt_add              db 'F3    Add product',0
mt_update           db 'F4    Update product',0
mt_delete           db 'F5    Delete product',0
mt_sell             db 'F6    Sales (checkout)',0
mt_report           db 'F7    Stock report (STOCK.TXT)',0
mt_backup           db 'F8    Backup',0
mt_settings         db 'F9    Settings',0
mt_exit             db 'F10   Exit',0

str_sum_products    db 'Products: ',0
str_sum_max         db ' / 1000',0
str_sum_low         db '     Low stock: ',0
str_sum_out         db '     Out of stock: ',0
msg_exit_confirm    db 'Exit Market Management System? (Y/N)',0

; ---- Password --------------------------------------------------------------
msg_enter_password  db 'Please enter the password.',0
lbl_password        db 'Password:',0
err_wrong_password  db 'Wrong password!',0

; ---- Help ------------------------------------------------------------------
help_text           db 1,'ABOUT',0
                    db '  MARKET MANAGEMENT SYSTEM 1.0 - stock, sales and reports for small markets',0
                    db '  For IBM PC/XT (8088), MDA/CGA/EGA/VGA and MS-DOS 3.0+, written with FASM.',0
                    db '  Copyright (c) 2026 Ercan Ersoy. Released under the MIT License.',0
                    db 1,'MAIN MENU FUNCTION KEYS',0
                    db '  F1 Help and About    F4 Update product   F7 Stock report   F10 Exit',0
                    db '  F2 Products          F5 Delete product   F8 Backup',0
                    db '  F3 Add product       F6 Sales            F9 Settings',0
                    db 1,'TEXT FIELDS',0
                    db '  Left/Right/Home/End move the cursor, Backspace/Del delete, Ctrl+Y clears.',0
                    db '  Enter confirms, Esc cancels. Up/Down or Tab/Shift+Tab change the field.',0
                    db 1,'PRODUCTS',0
                    db '  Type a barcode or a part of a name and press Enter (empty = all products).',0
                    db '  In the list Up/Down/PgUp/PgDn/Home/End move, Enter edits, Del deletes.',0
                    db 1,'SALES AND BARCODE READERS',0
                    db '  Scan or type a barcode and press Enter (3*BARCODE sells 3 pieces).',0
                    db '  F2 = payment, F3 = remove the last item, Esc = cancel the sale.',0
                    db 1,'FILES',0
                    db '  DATA.DAT = products, MARKET.CFG = settings, STOCK.TXT = stock report',0
                    db 0FFh

; ---- Product list and form -------------------------------------------------
msg_no_products     db 'No products found.',0
str_product         db 'Product ',0
str_of              db ' of ',0
hdr_no              db 'No',0
hdr_no_short        db '#',0
hdr_barcode         db 'Barcode',0
hdr_name            db 'Product name',0
hdr_price           db 'Price',0
hdr_qty             db 'Qty',0
hdr_status          db 'St',0
hdr_value           db 'Value',0
hdr_amount          db 'Amount',0
str_out             db 'OUT',0
str_low             db 'LOW',0

; buffer, length, flags, row
form_table          dw ed_barcode, BAR_LEN,   0,           ROW_TOP+2
                    dw ed_name,    NAME_LEN,  0,           ROW_TOP+4
                    dw ed_price,   PRICE_LEN, IN_PRICE,    ROW_TOP+6
                    dw ed_qty,     QTY_LEN,   IN_DIGITS,   ROW_TOP+8
form_labels         dw lbl_f_barcode, lbl_f_name, lbl_f_price, lbl_f_qty
lbl_f_barcode       db 'Barcode',0
lbl_f_name          db 'Product name',0
lbl_f_price         db 'Price',0
lbl_f_qty           db 'Quantity in stock',0
form_help           db 'The barcode can be typed or scanned with a barcode reader.',0
                    db 'Enter the price with a decimal point, for example 12.50',0
                    db 'Enter moves to the next field; Enter on the last field saves.',0
                    db 0FFh

err_barcode_empty   db 'Please enter a barcode.',0
err_barcode_exists  db 'A product with this barcode already exists.',0
err_name_empty      db 'Please enter a product name.',0
err_price           db 'Invalid price (maximum 999999.99).',0
err_qty             db 'Invalid quantity (0 - 65535).',0
err_db_full         db 'The database is full (1000 products).',0
err_save            db 'ERROR: DATA.DAT could not be saved!',0
msg_added           db 'Product saved. Enter the next product or press Esc.',0
msg_updated         db 'Product updated.',0
msg_deleted         db 'Product deleted.',0

; ---- Search / select -------------------------------------------------------
lbl_search          db 'Search:',0
lbl_find            db 'Barcode or name:',0
msg_select          db 'More than one product matches. Select one and press Enter.',0
msg_not_found       db 'No product found.',0
str_delete_q1       db 'Delete "',0
str_delete_q2       db '"? (Y/N)',0

; ---- Sales -----------------------------------------------------------------
lbl_barcode_sale    db 'Barcode:',0
msg_sale_tip        db 'Tip: type 3*BARCODE to sell 3 pieces of a product.',0
msg_removed         db 'Last item removed.',0
err_cart_empty      db 'There are no items in this sale.',0
msg_cancel_sale     db 'Cancel this sale? (Y/N)',0
str_lines           db 'Lines: ',0
str_items           db '     Items: ',0
str_total           db 'TOTAL: ',0
str_added           db 'Added: ',0
err_sale_qty        db 'Invalid quantity (1 - 9999).',0
err_cart_full       db 'This sale is full (100 lines).',0
err_not_found_bc    db 'Product not found: ',0
err_no_stock        db 'Not enough stock. In stock: ',0
lbl_paid            db 'Amount paid (Enter = exact amount):',0
err_paid_less       db 'The amount paid is less than the total.',0
msg_sale_done       db 'Sale completed.',0
msg_sale_next       db 'Sale completed. Press any key to start a new sale.',0
str_paid            db 'Paid: ',0
str_change          db '     Change: ',0

; ---- Stock report ----------------------------------------------------------
msg_creating        db 'Creating STOCK.TXT ...',0
err_report          db 'ERROR: STOCK.TXT could not be written!',0
msg_report_saved    db 'The stock report has been saved to STOCK.TXT.',0
msg_report_done     db 'Report created. Press any key to return.',0
rpt_title           db 'STOCK REPORT - Market Management System',0
rpt_date            db 'Date: ',0
rpt_time            db '   Time: ',0
rpt_sec_in          db 'PRODUCTS IN STOCK',0
rpt_sec_out         db 'PRODUCTS OUT OF STOCK',0
rpt_sec_sum         db 'SUMMARY',0
rpt_none            db '  (none)',0
rpt_labels          dw rl_in, rl_out, rl_low, rl_qty, rl_value, rl_total
rl_in               db 'Products in stock     : ',0
rl_out              db 'Products out of stock : ',0
rl_low              db 'Low stock products    : ',0
rl_qty              db 'Total quantity        : ',0
rl_value            db 'Total stock value     : ',0
rl_total            db 'Products in database  : ',0
str_low_limit       db ' (quantity <= ',0

; ---- Backup ----------------------------------------------------------------
backup_text         db 'The backup copies DATA.DAT (products) and STOCK.TXT (stock report)',0
                    db 'to the root directory of another drive, for example a floppy disk.',0
                    db 'If STOCK.TXT does not exist yet, a new stock report is created first.',0
                    db 0FFh
lbl_cur_drive       db 'Current drive : ',0
lbl_target_drive    db 'Target drive  :',0
msg_report_made     db 'STOCK.TXT was not found, a new stock report has been created.',0
msg_backup_ok       db 'Backup completed successfully. Press any key to return.',0
err_backup          db 'Backup FAILED! Check the disk and try again. Press any key.',0
err_drive           db 'Please enter a drive letter (A-Z).',0
err_same_drive      db 'The target must be a different drive than the current one.',0
err_not_ready       db 'The drive does not exist or is not ready.',0
str_copying         db 'Copying ',0
str_to              db ' to ',0
str_dots            db ' ... ',0
str_ok              db 'OK',0
str_failed          db 'FAILED',0

; ---- Settings --------------------------------------------------------------
; label, type, variable, parameter
set_table           dw ls_store,    T_STR,   cfg_store,     STORE_LEN
                    dw ls_currency, T_STR,   cfg_currency,  CUR_LEN
                    dw ls_sound,    T_BOOL,  cfg_sound,     0
                    dw ls_saver,    T_BOOL,  cfg_saver,     0
                    dw ls_savertime,T_BYTE,  cfg_saver_min, 60
                    dw ls_login,    T_BOOL,  cfg_login,     0
                    dw ls_password, T_PASS,  cfg_pass,      PASS_LEN
                    dw ls_lowstock, T_WORD,  cfg_low,       9999
                    dw ls_backup,   T_DRIVE, cfg_backup,    0
ls_store            db ' Store name',0
ls_currency         db ' Currency symbol',0
ls_sound            db ' PC speaker sound',0
ls_saver            db ' Screen saver',0
ls_savertime        db ' Screen saver delay (1-60)',0
ls_login            db ' Password login',0
ls_password         db ' Password',0
ls_lowstock         db ' Low stock level (0-9999)',0
ls_backup           db ' Default backup drive',0
settings_help       db 'Up/Down selects a setting, Enter or Space changes it.',0
                    db 'The settings are saved to MARKET.CFG when you press Esc.',0
                    db 0FFh
str_none            db '(none)',0
str_minutes         db ' minute(s)',0
str_pass_set        db '******** (set)',0
str_pass_none       db '(not set)',0
err_range           db 'Value out of range.',0
msg_settings_saved  db 'Settings saved to MARKET.CFG.',0
err_cfg_save        db 'ERROR: MARKET.CFG could not be saved!',0
lbl_pw_current      db 'Current password:',0
lbl_pw_new          db 'New password (empty = none):',0
lbl_pw_again        db 'Type the new password again:',0
err_pw_mismatch     db 'The passwords do not match. Password not changed.',0
msg_pw_changed      db 'Password changed.',0
msg_pw_removed      db 'Password removed, password login is off.',0

; ---- Start-up warnings -----------------------------------------------------
msg_db_full_load    db 'Warning: DATA.DAT has more than 1000 products, the rest was not loaded.',0
msg_bad_lines1      db 'Warning: ',0
msg_bad_lines2      db ' invalid line(s) in DATA.DAT were skipped.',0

; ----------------------------------------------------------------------------
; Variables with initial values
; ----------------------------------------------------------------------------
w_ten               dw 10
vid_seg             dw 0B000h
attr_mask           db 0FFh
cur_shape           dw 0B0Ch
cur_visible         db 0
kb_funcs            dw 0100h            ; high = check, low = read function
clock_last          dw 0FFFFh
pending_msg         dw 0
exit_msg            dw 0
in_saver            db 0
lv_focus            db 0
db_count            dw 0
cart_n              dw 0

; ----------------------------------------------------------------------------
; Uninitialised data (not stored in MARKET.COM)
; ----------------------------------------------------------------------------
rec_seg             rw 1
box_w               rw 1
idle_lo             rw 1
idle_hi             rw 1
rnd_seed            rw 1
load_bad            rw 1
load_full           rb 1
pk_tmp              rb 1

cfg_store           rb STORE_LEN+1
cfg_currency        rb CUR_LEN+1
cfg_sound           rb 1
cfg_saver           rb 1
cfg_saver_min       rb 1
cfg_login           rb 1
cfg_pass            rb PASS_LEN+1
cfg_backup          rb 1
                    rb 1
cfg_low             rw 1

fr_handle           rw 1
fr_pos              rw 1
fr_len              rw 1
fr_eof              rb 1
fw_err              rb 1
fw_handle           rw 1
fw_len              rw 1
cp_src              rw 1
cp_dst              rw 1

ss_cursor           rw 1
ss_t0               rw 1
ss_cur_vis          rb 1
ss_row              rb 1

num_tmp             rw 3

in_buf              rw 1
in_max              rw 1
in_rowcol           rw 1
in_len              rw 1
in_cur              rw 1
in_flags            rb 1
lv_first            rb 1
lv_count            rw 1
lv_sel              rw 1
lv_top              rw 1
lv_rows             rw 1

st_low              rw 1
st_out              rw 1
fpl_price           rw 2
fpl_qty             rw 1
form_self           rw 1
form_field          rb 1
login_tries         rb 1

sale_total          rw 3
sale_qty            rw 2
amount48            rw 3
pay_amount          rw 3
change48            rw 3
sell_q              rw 1
sell_rec            rw 1
pay_col             rb 1
rpt_pass            rb 1
rpt_in              rw 1
rpt_out             rw 1
rpt_low             rw 1
rpt_qty             rw 2
rpt_value           rw 3

cur_drive           rb 1
set_sel             rb 1
set_changed         rb 1
drive_buf           rb 2
path_buf            rb 20
fld_ptr             rw 4

ed_barcode          rb BAR_LEN+1
ed_name             rb NAME_LEN+1
ed_price            rb PRICE_LEN+1
ed_qty              rb QTY_LEN+1
query_buf           rb QUERY_LEN+1
flt_needle          rb 32
sale_buf            rb SALE_LEN+1
pay_buf             rb PRICE_LEN+1
edit_buf            rb 40
pw_buf              rb PASS_LEN+1
pw_buf2             rb PASS_LEN+1
tmp_str             rb 64
num_str             rb 32
clk_buf             rb 24
msg_buf             rb 160
warn_buf            rb 80
line_buf            rb LINE_MAX+1
lb                  rb LB_SIZE+1

cart_rec            rw MAX_CART
cart_qty            rw MAX_CART
idx_buf             rw MAX_REC
fbuf                rb FBUF_SIZE
ss_buf              rb 4000

bss_end:
