# 🖥️ Arduino Mega ADK 3.2" TFT Weather & Smart Home Dashboard

Asztali automatizált 12-órás időjárás-előrejelző és okosotthon műszerfal **Arduino Mega ADK R3**, **TFT LCD Mega Shield v2.2** és **3.2" TFT_320QVT LCD kijelző (ILI9341)** felhasználásával.

---

## 🛠️ Hardver Mátrix

- **Mikrokontroller:** Eredeti Arduino Mega ADK R3 (ATmega2560 + MAX3421E USB Host chip)
- **Illesztő Kártya:** TFT LCD Mega Shield v2.2 (3.3V <-> 5V logikai jelszint illesztéssel)
- **Kijelző:** 3.2" TFT LCD (320x240 pixel, ILI9341 16-bites párhuzamos adatbusz)
- **Kapcsolódás:** Home Assistant szerver (`192.168.0.253`) & WeMos D1 Mini Wi-Fi bridge

---

## ⚙️ Fő Funkciók

1. **Élő Valós Idejű Óra & Dátum:** Folyamatos másodperc alapú digitális kijelzés.
2. **4-Zónás 12-Órás Időjárás Előrejelzés:** +3h, +6h, +9h és +12h meteorológiai kártyák hőmérséklettel, állapotikonokkal és páratartalom kiírással.
3. **Automatikus Óránkénti Szinkronizáció:** Minden egész órakor önműködően frissíti az előrejelzési kártyákat és az utolsó lekérési időbélyeget.
4. **Hardveres Betűrajzoló Motor:** Saját kiépítésű 5x7 ASCII bitképes betűkészlet közvetlen 16-bites buszrajzolással.

---

## 🚀 Összeállítás és Telepítés

```bash
cd /home/alphaws/Dev/Projects/Arduino-Mega-TFT-Dashboard
pio run --target upload --upload-port /dev/ttyACM0
```
