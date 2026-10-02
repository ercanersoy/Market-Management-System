# Market Management System

A simple text-mode (TUI) market management program for the **PC** with an
**MDA**, **CGA**, **EGA** or **VGA** display, running under **MS-DOS**. It is
written in 8086 assembly language for the
[flat assembler (FASM)](https://flatassembler.net/) and builds to a single
`MARKET.COM` file.

The program always uses the 80x25 text mode; no graphics mode is used. On
CGA, EGA and VGA the screen is green on black. On MDA the screen is
monochrome as usual.

All input comes from the keyboard. A keyboard-wedge **barcode reader** works
without a driver because it types the code and presses Enter like a keyboard.

## Features

| Key | Function | Description |
|-----|----------|-------------|
| F1  | Help and About | Key reference and program information |
| F2  | Products | Product list with a search field. Search by barcode or any part of the name, ignoring case; an empty search lists every product. In the list, Enter edits and Del deletes. |
| F3  | Add product | Barcode, name, price, quantity |
| F4  | Update product | Scan a barcode or search by name, then edit |
| F5  | Delete product | Scan a barcode or search by name, then confirm |
| F6  | Sales (checkout) | Scan items (`3*BARCODE` sells 3), payment, change; stock is reduced |
| F7  | Stock report | Writes `STOCK.TXT` (in stock / out of stock / summary) |
| F8  | Backup | Copies `DATA.DAT` and `STOCK.TXT` to the root of another drive |
| F9  | Settings | Stored in `MARKET.CFG` |
| F10 | Exit | Back to DOS (after confirmation) |

The main menu is operated only with the function keys F1-F10, which every
PC/XT keyboard has.

### Settings (F9)

* Store name (shown in the title bar and in the report)
* Currency symbol
* PC speaker sound on/off (confirmation and error beeps)
* Screen saver on/off and delay (1-60 minutes). When password login is on,
  the screen saver also locks the program until the password is entered.
* Password login on/off, and changing the password
* Low stock level (products at or below it are marked `LOW`)
* Default backup drive

## Files

All files are plain text and are kept in the current directory.

| File | Contents |
|------|----------|
| `DATA.DAT`   | Product database, one product per line: `BARCODE;PRODUCT NAME;PRICE;QUANTITY`. Lines starting with `;` are comments. |
| `MARKET.CFG` | Settings as `KEY=VALUE` lines. It is created with default values on the first start. |
| `STOCK.TXT`  | Stock report, created by F7 (and by F8 when it does not exist yet). |

Example `DATA.DAT`:

```
; BARCODE;PRODUCT NAME;PRICE;QUANTITY
8690504000011;Milk 1L;24.50;40
8690504000028;White bread;12.00;3
```

`DATA.DAT` is written to `DATA.TMP` first and then renamed, so a failed
write does not destroy the old database. The password in `MARKET.CFG` is
scrambled so it cannot be read at a glance. This is not encryption.

## Limits

* 1000 products. Barcodes can be up to 20 characters and names up to 30.
* Price: 0.00 - 999999.99; quantity in stock: 0 - 65535.
* 100 lines per sale, up to 9999 pieces per line.

## Requirements

* IBM PC/XT or compatible (8088 or better). Only 8086/8088 instructions
  are used.
* One of these display adapters:
  * MDA: 80x25 monochrome text mode (mode 7, B000h).
  * CGA, EGA or VGA: 80x25 color text mode (mode 3, B800h), green on black.
    The program switches to this mode at start-up.
* MS-DOS 3.0 or later. About 130 KB of free memory: 64 KB for the program
  and 64 KB for the product records.

## Distribution

The `bin` directory contains everything needed to run the program:

| File | Contents |
|------|----------|
| `bin/MARKET.COM`   | The program |
| `bin/READMETR.TXT` | This documentation in Turkish (code page 857) |
| `bin/READMEUS.TXT` | This documentation in American English |
| `bin/READMEUK.TXT` | This documentation in British English |

The text files have DOS (CR/LF) line endings and lines of at most 78
characters, so they can be read with `TYPE` or `MORE` in DOS.

## Building

The program is built into the `bin` directory.

With FASM on DOS:

```
BUILD.BAT
```

With fasm on Linux:

```
./build.sh
```

Or directly: `cd src` and then `fasm MARKET.ASM ../bin/MARKET.COM`.

On Linux, `build.sh` also creates the `bin/README*.TXT` files from the
`README_*.md` files with `tools/md2txt.py` (Python 3 is needed for this).

`src/MACROS.INC` forces all conditional jumps to the short form. The 8088
has no near conditional jumps, so an out-of-range jump stops the build with
an error instead of producing 386 code.

## Source layout

| File | Contents |
|------|----------|
| `src/MARKET.ASM`   | Entry point, start-up, interrupt handlers |
| `src/CONST.INC`    | Constants and screen layout |
| `src/VIDEO.INC`    | Display adapter detection, direct video memory output, boxes, cursor |
| `src/KEYBOARD.INC` | Keyboard, idle work, PC speaker, screen saver |
| `src/STRING.INC`   | Strings, number parsing/formatting, 48-bit arithmetic |
| `src/FILE.INC`     | Buffered text file I/O, file copy |
| `src/DATABASE.INC` | Product records, `DATA.DAT` load/save, search |
| `src/CONFIG.INC`   | `MARKET.CFG` load/save |
| `src/UI.INC`       | Frame, status line, input fields, lists, product form |
| `src/SCREENS.INC`  | Main menu and the functions |
| `src/DATA.INC`     | Texts, tables and variables |

## Trying it in DOSBox

For MDA:

```
[dosbox]
machine=hercules
[cpu]
cputype=8086
cycles=fixed 300
```

For CGA, EGA or VGA, use `machine=cga`, `machine=ega` or `machine=svga_s3`.

Copy `MARKET.COM` from the `bin` directory to a directory, mount it and run
`MARKET`.

## License

MIT License. See [LICENSE](LICENSE).

## Note

* IBM is a trademark of IBM.
* Microsoft is a trademark of Microsoft.
* MS-DOS is a trademark of Microsoft.
* Windows is a trademark of Microsoft.
