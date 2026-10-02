# Market Yönetim Sistemi

**MS-DOS** altında, **MDA**, **CGA**, **EGA** veya **VGA** ekran kartlı
**PC**'lerde çalışan basit bir yazı modu (TUI) market yönetim programıdır.
[flat assembler (FASM)](https://flatassembler.net/) için 8086 assembly
diliyle yazılmıştır ve tek bir `MARKET.COM` dosyası olarak derlenir.

Program her zaman 80x25 yazı modunu kullanır; grafik modu kullanılmaz. CGA,
EGA ve VGA'da ekran siyah üzeri yeşildir. MDA'da ekran her zamanki gibi tek
renklidir.

Tüm girişler klavyeden yapılır. Klavye girişli (keyboard wedge) **barkod
okuyucu**, kodu klavye gibi yazıp Enter'a bastığı için sürücü gerektirmeden
çalışır.

## Özellikler

| Tuş | İşlev | Açıklama |
|-----|-------|----------|
| F1  | Help and About (Yardım ve Hakkında) | Tuş başvurusu ve program bilgileri |
| F2  | Products (Ürünler) | Arama alanlı ürün listesi. Barkoda veya adın herhangi bir bölümüne göre, büyük/küçük harf ayırt etmeden arar; boş arama tüm ürünleri listeler. Listede Enter düzenler, Del siler. |
| F3  | Add product (Ürün ekle) | Barkod, ad, fiyat, miktar |
| F4  | Update product (Ürün güncelle) | Barkodu okutun veya ada göre arayın, sonra düzenleyin |
| F5  | Delete product (Ürün sil) | Barkodu okutun veya ada göre arayın, sonra onaylayın |
| F6  | Sales (Satış, kasa) | Ürünleri okutun (`3*BARKOD` 3 adet satar), ödeme, para üstü; stok düşülür |
| F7  | Stock report (Stok raporu) | `STOCK.TXT` dosyasını yazar (stokta olanlar / tükenenler / özet) |
| F8  | Backup (Yedekleme) | `DATA.DAT` ve `STOCK.TXT` dosyalarını başka bir sürücünün kök dizinine kopyalar |
| F9  | Settings (Ayarlar) | `MARKET.CFG` dosyasında saklanır |
| F10 | Exit (Çıkış) | DOS'a döner (onaydan sonra) |

Program arayüzü İngilizcedir. Ana menü yalnızca her PC/XT klavyesinde
bulunan F1-F10 işlev tuşlarıyla kullanılır.

### Ayarlar (F9)

* Mağaza adı (başlık çubuğunda ve raporda gösterilir)
* Para birimi simgesi
* PC hoparlörü sesi açık/kapalı (onay ve hata sesleri)
* Ekran koruyucu açık/kapalı ve bekleme süresi (1-60 dakika). Parolayla
  giriş açıksa ekran koruyucu, parola girilene kadar programı da kilitler.
* Parolayla giriş açık/kapalı ve parola değiştirme
* Düşük stok seviyesi (bu seviyede veya altındaki ürünler `LOW` olarak
  işaretlenir)
* Varsayılan yedekleme sürücüsü

## Dosyalar

Tüm dosyalar düz metindir ve geçerli dizinde tutulur.

| Dosya | İçerik |
|-------|--------|
| `DATA.DAT`   | Ürün veritabanı, her satırda bir ürün: `BARKOD;ÜRÜN ADI;FİYAT;MİKTAR`. `;` ile başlayan satırlar açıklamadır. |
| `MARKET.CFG` | `ANAHTAR=DEĞER` satırları olarak ayarlar. İlk başlatmada varsayılan değerlerle oluşturulur. |
| `STOCK.TXT`  | Stok raporu; F7 ile (ve henüz yoksa F8 ile) oluşturulur. |

Örnek `DATA.DAT`:

```
; BARCODE;PRODUCT NAME;PRICE;QUANTITY
8690504000011;Ayran 1L;9.50;40
8690504000028;Beyaz ekmek;12.00;3
```

`DATA.DAT` önce `DATA.TMP` dosyasına yazılır, sonra yeniden adlandırılır;
böylece başarısız bir yazma eski veritabanını bozmaz. `MARKET.CFG`
içindeki parola ilk bakışta okunamayacak şekilde karıştırılır. Bu bir
şifreleme değildir.

## Sınırlar

* 1000 ürün. Barkodlar en fazla 20, adlar en fazla 30 karakter olabilir.
* Fiyat: 0.00 - 999999.99; stok miktarı: 0 - 65535.
* Satış başına 100 satır, satır başına en fazla 9999 adet.

## Gereksinimler

* IBM PC/XT veya uyumlusu (8088 veya üstü). Yalnızca 8086/8088 komutları
  kullanılır.
* Şu ekran kartlarından biri:
  * MDA: 80x25 tek renkli yazı modu (mod 7, B000h).
  * CGA, EGA veya VGA: 80x25 renkli yazı modu (mod 3, B800h), siyah üzeri
    yeşil. Program açılışta bu moda geçer.
* MS-DOS 3.0 veya üstü. Yaklaşık 130 KB boş bellek: program için 64 KB ve
  ürün kayıtları için 64 KB.

## Dağıtım

`bin` dizini, programı çalıştırmak için gereken her şeyi içerir:

| Dosya | İçerik |
|-------|--------|
| `bin/MARKET.COM`   | Program |
| `bin/READMETR.TXT` | Bu belgenin Türkçesi (kod sayfası 857) |
| `bin/READMEUS.TXT` | Bu belgenin Amerikan İngilizcesi |
| `bin/READMEUK.TXT` | Bu belgenin İngiliz İngilizcesi |

Metin dosyaları DOS (CR/LF) satır sonlarına ve en fazla 78 karakterlik
satırlara sahiptir; bu yüzden DOS'ta `TYPE` veya `MORE` ile okunabilir.
Türkçe karakterlerin doğru görünmesi için Türkçe kod sayfası 857
(`CHCP 857`) gerekir.

## Derleme

Program `bin` dizinine derlenir.

DOS'ta FASM ile:

```
BUILD.BAT
```

Linux'ta fasm ile:

```
./build.sh
```

Ya da doğrudan: `cd src`, ardından `fasm MARKET.ASM ../bin/MARKET.COM`.

Linux'ta `build.sh`, `tools/md2txt.py` ile `README_*.md` dosyalarından
`bin/README*.TXT` dosyalarını da oluşturur (bunun için Python 3 gerekir).

`src/MACROS.INC` tüm koşullu atlamaları kısa biçime zorlar. 8088'de yakın
(near) koşullu atlama yoktur; bu yüzden erim dışındaki bir atlama, 386 kodu
üretmek yerine derlemeyi bir hatayla durdurur.

## Kaynak düzeni

| Dosya | İçerik |
|-------|--------|
| `src/MARKET.ASM`   | Giriş noktası, başlatma, kesme işleyicileri |
| `src/CONST.INC`    | Sabitler ve ekran düzeni |
| `src/VIDEO.INC`    | Ekran kartı algılama, doğrudan görüntü belleği çıktısı, kutular, imleç |
| `src/KEYBOARD.INC` | Klavye, boşta çalışan işler, PC hoparlörü, ekran koruyucu |
| `src/STRING.INC`   | Dizgiler, sayı ayrıştırma/biçimlendirme, 48 bit aritmetik |
| `src/FILE.INC`     | Arabellekli metin dosyası G/Ç, dosya kopyalama |
| `src/DATABASE.INC` | Ürün kayıtları, `DATA.DAT` yükleme/kaydetme, arama |
| `src/CONFIG.INC`   | `MARKET.CFG` yükleme/kaydetme |
| `src/UI.INC`       | Çerçeve, durum satırı, giriş alanları, listeler, ürün formu |
| `src/SCREENS.INC`  | Ana menü ve işlevler |
| `src/DATA.INC`     | Metinler, tablolar ve değişkenler |

## DOSBox'ta deneme

MDA için:

```
[dosbox]
machine=hercules
[cpu]
cputype=8086
cycles=fixed 300
```

CGA, EGA veya VGA için `machine=cga`, `machine=ega` veya `machine=svga_s3`
kullanın.

`bin` dizinindeki `MARKET.COM` dosyasını bir dizine kopyalayın, dizini
bağlayın (mount) ve `MARKET` komutunu çalıştırın.

## Lisans

MIT Lisansı. Bkz. [LICENSE](LICENSE).

## Not

* IBM, IBM'in ticari markasıdır.
* Microsoft, Microsoft'un ticari markasıdır.
* MS-DOS, Microsoft'un ticari markasıdır.
* Windows, Microsoft'un ticari markasıdır.
