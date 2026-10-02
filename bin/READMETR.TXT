======================
Market Yînetim Sistemi
======================

MS-DOS altçnda, MDA, CGA, EGA veya VGA ekran kartlç PC'lerde áalçüan basit bir
yazç modu (TUI) market yînetim programçdçr. flat assembler (FASM)
(https://flatassembler.net/) iáin 8086 assembly diliyle yazçlmçütçr ve tek bir
MARKET.COM dosyasç olarak derlenir.

Program her zaman 80x25 yazç modunu kullançr; grafik modu kullançlmaz. CGA,
EGA ve VGA'da ekran siyah Åzeri yeüildir. MDA'da ekran her zamanki gibi tek
renklidir.

TÅm giriüler klavyeden yapçlçr. Klavye giriüli (keyboard wedge) barkod
okuyucu, kodu klavye gibi yazçp Enter'a bastçßç iáin sÅrÅcÅ gerektirmeden
áalçüçr.

ôzellikler
----------

Tuü  òülev                                Aáçklama
---  -----------------------------------  ------------------------------------
F1   Help and About (Yardçm ve Hakkçnda)  Tuü baüvurusu ve program bilgileri
F2   Products (örÅnler)                   Arama alanlç ÅrÅn listesi. Barkoda
                                          veya adçn herhangi bir bîlÅmÅne
                                          gîre, bÅyÅk/kÅáÅk harf ayçrt etmeden
                                          arar; boü arama tÅm ÅrÅnleri
                                          listeler. Listede Enter dÅzenler,
                                          Del siler.
F3   Add product (örÅn ekle)              Barkod, ad, fiyat, miktar
F4   Update product (örÅn gÅncelle)       Barkodu okutun veya ada gîre arayçn,
                                          sonra dÅzenleyin
F5   Delete product (örÅn sil)            Barkodu okutun veya ada gîre arayçn,
                                          sonra onaylayçn
F6   Sales (Satçü, kasa)                  örÅnleri okutun (3*BARKOD 3 adet
                                          satar), îdeme, para ÅstÅ; stok
                                          dÅüÅlÅr
F7   Stock report (Stok raporu)           STOCK.TXT dosyasçnç yazar (stokta
                                          olanlar / tÅkenenler / îzet)
F8   Backup (Yedekleme)                   DATA.DAT ve STOCK.TXT dosyalarçnç
                                          baüka bir sÅrÅcÅnÅn kîk dizinine
                                          kopyalar
F9   Settings (Ayarlar)                   MARKET.CFG dosyasçnda saklançr
F10  Exit (Äçkçü)                         DOS'a dîner (onaydan sonra)

Program arayÅzÅ òngilizcedir. Ana menÅ yalnçzca her PC/XT klavyesinde bulunan
F1-F10 iülev tuülarçyla kullançlçr.

Ayarlar (F9)
~~~~~~~~~~~~

  * Maßaza adç (baülçk áubußunda ve raporda gîsterilir)
  * Para birimi simgesi
  * PC hoparlîrÅ sesi aáçk/kapalç (onay ve hata sesleri)
  * Ekran koruyucu aáçk/kapalç ve bekleme sÅresi (1-60 dakika). Parolayla
    giriü aáçksa ekran koruyucu, parola girilene kadar programç da kilitler.
  * Parolayla giriü aáçk/kapalç ve parola deßiütirme
  * DÅüÅk stok seviyesi (bu seviyede veya altçndaki ÅrÅnler LOW olarak
    iüaretlenir)
  * Varsayçlan yedekleme sÅrÅcÅsÅ

Dosyalar
--------

TÅm dosyalar dÅz metindir ve geáerli dizinde tutulur.

Dosya       òáerik
----------  ------------------------------------------------------------------
DATA.DAT    örÅn veritabanç, her satçrda bir ÅrÅn: BARKOD;öRöN
            ADI;FòYAT;MòKTAR. ; ile baülayan satçrlar aáçklamadçr.
MARKET.CFG  ANAHTAR=DE¶ER satçrlarç olarak ayarlar. òlk baülatmada varsayçlan
            deßerlerle oluüturulur.
STOCK.TXT   Stok raporu; F7 ile (ve henÅz yoksa F8 ile) oluüturulur.

ôrnek DATA.DAT:

    ; BARCODE;PRODUCT NAME;PRICE;QUANTITY
    8690504000011;Ayran 1L;9.50;40
    8690504000028;Beyaz ekmek;12.00;3

DATA.DAT înce DATA.TMP dosyasçna yazçlçr, sonra yeniden adlandçrçlçr; bîylece
baüarçsçz bir yazma eski veritabançnç bozmaz. MARKET.CFG iáindeki parola ilk
bakçüta okunamayacak üekilde karçütçrçlçr. Bu bir üifreleme deßildir.

Sçnçrlar
--------

  * 1000 ÅrÅn. Barkodlar en fazla 20, adlar en fazla 30 karakter olabilir.
  * Fiyat: 0.00 - 999999.99; stok miktarç: 0 - 65535.
  * Satçü baüçna 100 satçr, satçr baüçna en fazla 9999 adet.

Gereksinimler
-------------

  * IBM PC/XT veya uyumlusu (8088 veya ÅstÅ). Yalnçzca 8086/8088 komutlarç
    kullançlçr.
  * ûu ekran kartlarçndan biri:
    * MDA: 80x25 tek renkli yazç modu (mod 7, B000h).
    * CGA, EGA veya VGA: 80x25 renkli yazç modu (mod 3, B800h), siyah Åzeri
      yeüil. Program aáçlçüta bu moda geáer.
  * MS-DOS 3.0 veya ÅstÅ. Yaklaüçk 130 KB boü bellek: program iáin 64 KB ve
    ÅrÅn kayçtlarç iáin 64 KB.

Daßçtçm
-------

bin dizini, programç áalçütçrmak iáin gereken her üeyi iáerir:

Dosya             òáerik
----------------  --------------------------------------
bin/MARKET.COM    Program
bin/READMETR.TXT  Bu belgenin TÅrkáesi (kod sayfasç 857)
bin/READMEUS.TXT  Bu belgenin Amerikan òngilizcesi
bin/READMEUK.TXT  Bu belgenin òngiliz òngilizcesi

Metin dosyalarç DOS (CR/LF) satçr sonlarçna ve en fazla 78 karakterlik
satçrlara sahiptir; bu yÅzden DOS'ta TYPE veya MORE ile okunabilir. TÅrkáe
karakterlerin doßru gîrÅnmesi iáin TÅrkáe kod sayfasç 857 (CHCP 857) gerekir.

Derleme
-------

Program bin dizinine derlenir.

DOS'ta FASM ile:

    BUILD.BAT

Linux'ta fasm ile:

    ./build.sh

Ya da doßrudan: cd src, ardçndan fasm MARKET.ASM ../bin/MARKET.COM.

Linux'ta build.sh, tools/md2txt.py ile README_*.md dosyalarçndan
bin/README*.TXT dosyalarçnç da oluüturur (bunun iáin Python 3 gerekir).

src/MACROS.INC tÅm koüullu atlamalarç kçsa biáime zorlar. 8088'de yakçn (near)
koüullu atlama yoktur; bu yÅzden erim dçüçndaki bir atlama, 386 kodu Åretmek
yerine derlemeyi bir hatayla durdurur.

Kaynak dÅzeni
-------------

Dosya             òáerik
----------------  ------------------------------------------------------------
src/MARKET.ASM    Giriü noktasç, baülatma, kesme iüleyicileri
src/CONST.INC     Sabitler ve ekran dÅzeni
src/VIDEO.INC     Ekran kartç algçlama, doßrudan gîrÅntÅ belleßi áçktçsç,
                  kutular, imleá
src/KEYBOARD.INC  Klavye, boüta áalçüan iüler, PC hoparlîrÅ, ekran koruyucu
src/STRING.INC    Dizgiler, sayç ayrçütçrma/biáimlendirme, 48 bit aritmetik
src/FILE.INC      Arabellekli metin dosyasç G/Ä, dosya kopyalama
src/DATABASE.INC  örÅn kayçtlarç, DATA.DAT yÅkleme/kaydetme, arama
src/CONFIG.INC    MARKET.CFG yÅkleme/kaydetme
src/UI.INC        Äeráeve, durum satçrç, giriü alanlarç, listeler, ÅrÅn formu
src/SCREENS.INC   Ana menÅ ve iülevler
src/DATA.INC      Metinler, tablolar ve deßiükenler

DOSBox'ta deneme
----------------

MDA iáin:

    [dosbox]
    machine=hercules
    [cpu]
    cputype=8086
    cycles=fixed 300

CGA, EGA veya VGA iáin machine=cga, machine=ega veya machine=svga_s3 kullançn.

bin dizinindeki MARKET.COM dosyasçnç bir dizine kopyalayçn, dizini baßlayçn
(mount) ve MARKET komutunu áalçütçrçn.

Lisans
------

MIT Lisansç. Bkz. LICENSE.

Not
---

  * IBM, IBM'in ticari markasçdçr.
  * Microsoft, Microsoft'un ticari markasçdçr.
  * MS-DOS, Microsoft'un ticari markasçdçr.
  * Windows, Microsoft'un ticari markasçdçr.
