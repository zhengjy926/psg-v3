# MOSFET CI58N330SM4 — Datasheet Research Report

> Project: psg-v3 (STM32G474 pulse-signal-generation board, six rod/electrode switches A–F).
> Purpose: verify two firmware timing decisions (10 µs turn-on wait; no explicit turn-off wait inside a ≥1 ms pulse-train gap) against the real datasheet.

## 0. Executive summary (tl;dr)

| Parameter | Value | Source (datasheet page) |
|---|---|---|
| **Vdss** | **3300 V** (3.3 kV), not 330 V | p.1, p.2, p.3 |
| **Id (continuous)** | **58 A** @ Tc=25 °C, Vgs=18 V (41 A @ Tc=100 °C) | p.2 |
| **Rds(on) max** | **60 mΩ** (50 mΩ typ) @ Vgs=18 V, Id=40 A, Tj=25 °C; 146 mΩ @ 175 °C | p.3 |
| **Vgs(th)** | **3.2 V typ / 4.0 V max** @ Id=10 mA, 25 °C (2.4 V typ @ 175 °C) | p.3 |
| **Qg / Qgs / Qgd** | **212 / 73 / 58 nC** (Vgs −5/+20 V, Vds=1000 V, Id=40 A) | p.3 |
| **td(on) / tr** | **37 ns / 27 ns** | p.4 |
| **td(off) / tf** | **51 ns / 9.6 ns** | p.4 |

**Three one-line answers:**

1. **Turn-on (10 µs wait):** `td(on)=37 ns` (+ `tr=27 ns` → ~64 ns total), so 10 µs is ~270× the real turn-on delay — the real `td(on)` is ~2.4 orders of magnitude smaller, and 10 µs is a very large (safe, conservative) margin.
2. **Turn-off (≥1 ms gap):** `td(off)+tf = 51 + 9.6 ≈ 61 ns`, which is ~16,500× smaller than 1 ms — turn-off is covered by the gap with an enormous margin.
3. **Anything wrong with current handling:** The timing is fine; the real risk is **gate-drive voltage**, not timing. `Vgs(th)` max is 4.0 V and `Rds(on)` is specified only at **Vgs = 18 V**, so this 3.3 kV SiC part **cannot be driven by a 3.3 V / 5 V GPIO directly** — it needs a gate driver supplying ~+18 V (datasheet recommended `Vgs(op)` = −4/+18 V). Confirm the board uses a proper +18 V gate driver (and ideally a negative off-bias, as the datasheet's switching times assume −5 V off).

---

## 1. Source identification (primary vs. secondary)

- **Manufacturer:** Guangzhou Tokmas Electronics Co., Ltd. (广州托克马斯电子有限公司). Official site: <https://www.tokmas.com> (site confirmed live; homepage title "广州托克马斯电子有限公司-首页").
- **Part:** `CI58N330SM4` — "3300V Planar SiC MOSFET", TO-247-4L package. The number decodes as **58 A / 3300 V** (not ~330 V as initially assumed in the background note).
- **Datasheet obtained:** the **manufacturer's own datasheet PDF** (every page is footed `www.tokmas.com` and titled "3300V Planar SiC MOSFET", exact part number `CI58N330SM4`), hosted on LCSC's (立创商城) CDN:
  - Direct PDF (signed URL; `Expires=4070880000` ≈ year 2099, effectively non-expiring): <https://atta.szlcsc.com/upload/public/pdf/source/20251117/A3E02BE6B0527BD353A8FD264220C6C2.pdf>
  - LCSC datasheet viewer page: <https://item.szlcsc.com/datasheet/CI58N330SM4/55854168.html>
  - LCSC product page (parameter summary, cross-check): <https://item.szlcsc.com/55854168.html>

**Primary-source caveat (stated explicitly, per request):** The PDF is the manufacturer's first-party datasheet (Tokmas logo/footer and part number present throughout), but I located it on a **distributor's CDN (LCSC)**, not on the manufacturer's own server. I attempted to navigate `www.tokmas.com` for a direct PDF link, but the site is a JS/PHP-heavy catalog and `web_fetch` only returns the shell/title, so I could not retrieve a same-site PDF URL. The datasheet content itself is first-party (manufacturer-authored); the *hosting* is a distributor mirror. This is flagged rather than silently treated as authoritative.

Secondary cross-checks (not used as authority, only for consistency):
- LCSC product page lists Vdss 3.3 kV, Id 58 A, Pd 500 W, Vgs(th) 4 V, Qg 212 nC, Rds(on) 60 mΩ — all consistent with the datasheet.
- JLCPCB part page: <https://jlcpcb.com/partdetail/CI58N330SM4/C52940991> (same part, C52940991).

---

## 2. Extracted parameters (with citations)

All values below are transcribed from the manufacturer datasheet PDF (10 pages). Page numbers refer to the PDF's internal pages; the datasheet has no printed page numbers, so I cite by section/page. Tables: p.2 "Maximum Ratings", p.3 "Static / Dynamic Characteristics", p.4 "Switching / Reverse Diode Characteristics".

### 2.1 Absolute maximum ratings (p.2)

| Symbol | Parameter | Conditions | Value |
|---|---|---|---|
| VDSmax | Drain–source voltage | Vgs=0 V, Id=100 µA | **3300 V** |
| VGSmax | Gate–source voltage | AC (f > 1 Hz) | **−8 / +22 V** |
| VGSop | Gate–source voltage (recommended operational) | — | **−4 / +18 V** |
| ID | Continuous drain current | Vgs=18 V, Tc=25 °C | **58 A** |
| ID | Continuous drain current | Vgs=18 V, Tc=100 °C | **41 A** |
| ID(pulse) | Pulsed drain current | tp limited by Tjmax | **230 A** |
| PD | Power dissipation | Tc=25 °C, Tj=175 °C | **500 W** |
| Tj / Tstg | Junction / storage temp | — | **−55 … +175 °C** |

Thermal: **Rth(j-c) = 0.3 °C/W** (p.2).

### 2.2 Static characteristics (p.3)

| Symbol | Parameter | Conditions | Min | Typ | Max |
|---|---|---|---|---|---|
| VDSS | Drain–source breakdown | Vgs=0 V, Id=100 µA | **3300 V** | — | — |
| **VGS(th)** | Gate threshold | Vds=Vgs, Id=10 mA, Tj=25 °C | **3.2 V** | — | **4.0 V** |
| VGS(th) | Gate threshold | Id=10 mA, Tj=175 °C | — | **2.4 V** | — |
| **RDS(on)** | On-resistance | Vgs=18 V, Id=40 A, Tj=25 °C | — | **50 mΩ** | **60 mΩ** |
| RDS(on) | On-resistance | Vgs=18 V, Id=40 A, Tj=175 °C | — | **146 mΩ** | — |
| Gfs | Transconductance | Vds=10 V, Id=40 A, Tj=25 °C | — | 19.1 S | — |

### 2.3 Dynamic / gate charge (p.3)

| Symbol | Parameter | Conditions | Value |
|---|---|---|---|
| Ciss / Coss / Crss | Capacitances | Vgs=0 V, Vds=1000 V, f=100 kHz, 25 °C | 5031 / 115 / 4.9 pF |
| **Qg** | Total gate charge | Vgs=−5/+20 V, Vds=1000 V, Id=40 A, 25 °C | **212 nC** |
| **Qgs** | Gate–source charge | (same) | **73 nC** |
| **Qgd** | Gate–drain charge | (same) | **58 nC** |
| Rg | Internal gate resistance | Vac=25 mV, f=1 MHz | 1.2 Ω |

### 2.4 Switching characteristics (p.4) — the numbers that matter for the timing decisions

Test conditions (single line, applies to all switching rows): **Vgs = −5 V / +20 V, Vdd = 1700 V, Id = 50 A, Rg(ext) = 3 Ω, L = 60 µH.**

| Symbol | Parameter | Value |
|---|---|---|
| **td(on)** | Turn-on delay time | **37 ns** |
| **tr** | Rise time | **27 ns** |
| **td(off)** | Turn-off delay time | **51 ns** |
| **tf** | Fall time | **9.6 ns** |
| Eon | Turn-on switching energy | 737 µJ |
| Eoff | Turn-off switching energy | 288 µJ |

### 2.5 Reverse (body) diode (p.4)

| Symbol | Parameter | Conditions | Value |
|---|---|---|---|
| VSD | Diode forward voltage | Vgs=−5 V, Isd=20 A, Tj=25 °C | 3.9 V (typ) |
| IS / IS(pulse) | Diode current | Vgs=−5 V | 43 A / 300 A |
| Trr / Qrr / Irrm | Reverse recovery | Vgs=−5 V, Isd=50 A, Vr=3300 V | 17.8 ns / 290 nC / 28 A |

> Minor datasheet transcription quirks (not relevant to timing, noted for completeness): p.1 says "Tape & Reel" for a TO-247-4L (LCSC lists 管装/tube), and several p.4 diode rows carry mislabeled parameter names (e.g. "VSD — Drain-source breakdown voltage", "Trr — On-resistance"). Values are unambiguous from context.

---

## 3. Answers to the three firmware questions

### 3.1 Is the 10 µs turn-on wait justified?

**Yes — justified as a large, conservative safety margin. The real `td(on)` is orders of magnitude smaller.**

- Datasheet `td(on) = 37 ns`, `tr = 27 ns` (p.4). Full turn-on (`td(on)+tr`) ≈ **64 ns**.
- 10 µs = 10,000 ns → **~270× `td(on)`** and ~156× the full turn-on time. That is ~2.4 orders of magnitude of margin.
- Even with a slower real-world gate driver than the datasheet's Rg(ext)=3 Ω, −5/+20 V condition, the delay would remain hundreds of ns at worst — still far below 10 µs. **Conclusion: 10 µs is more than enough; it is conservative, not wrong.** (If desired, it could be reduced by 1–2 orders of magnitude without risk.)

### 3.2 Is turn-off covered by the ≥1 ms pulse-train gap?

**Yes — by an enormous margin.**

- Datasheet `td(off) = 51 ns`, `tf = 9.6 ns` (p.4) → `td(off)+tf ≈ 60.6 ns`.
- 1 ms = 1,000,000 ns → the gap is **~16,500× the full turn-off time**. Turn-off is a non-issue at 1 ms. (The datasheet's 51 ns assumes a −5 V off-bias; even without negative bias, off time stays in the low-hundreds-of-ns range, still ≪ 1 ms.)

### 3.3 Any parameter that would make the current handling wrong?

The timing is fine. The genuine risk is **gate-drive voltage/level**, plus a secondary thermal note:

1. **Gate drive level (primary concern).** `Vgs(th)` max = **4.0 V** and `Rds(on)` is specified only at **Vgs = 18 V**; recommended operational range is **−4/+18 V** (p.2, p.3). A 3.3 V or 5 V logic drive is below/only just above `Vgs(th)` and would leave the device barely enhanced → very high `Rds(on)` (see "Rds(on) vs Vgs", Fig. 8), severe conduction heating, and possibly device damage at load current. **The STM32G474 cannot drive this gate directly; a +18 V gate driver (ideally with negative off-bias) is mandatory.** Verify the board has one before trusting the 10 µs assumption (the 10 µs wait only makes sense if the gate driver has actually charged Qg=212 nC to ~18 V).
2. **Switching test conditions differ from nominal rating.** The datasheet times are at Vdd=**1700 V** and a bipolar −5/+20 V gate (p.4). If the real bus is nearer 3300 V, or the driver is unipolar 0/+18 V, actual `td(on)/td(off)` shift somewhat — but they remain nanosecond-scale and cannot approach the µs/ms budgets above.
3. **Thermal/derating.** `Pd = 500 W`, `Rth(j-c) = 0.3 °C/W`, and `Id` drops 58 A→41 A from Tc=25 °C→100 °C (p.2). `Eon = 737 µJ` / `Eoff = 288 µJ` at Vdd=1700 V/Id=50 A (p.4) are non-trivial; in this rod-switching application switching happens only in the gap (infrequent), so average switching loss is low, but **conduction loss during each pulse train scales with on-time × current² × Rds(on)@Tj** — at high junction temperature Rds(on) roughly triples (146 mΩ @ 175 °C), which matters if the pulse train is long or current is high.

---

## 4. References

1. Tokmas, *"3300V Planar SiC MOSFET — CI58N330SM4"* datasheet (10 pp.), manufacturer site `www.tokmas.com`; PDF mirrored by LCSC: <https://atta.szlcsc.com/upload/public/pdf/source/20251117/A3E02BE6B0527BD353A8FD264220C6C2.pdf> (viewer: <https://item.szlcsc.com/datasheet/CI58N330SM4/55854168.html>).
2. LCSC (立创商城) product page CI58N330SM4 (C52940991), parameter summary: <https://item.szlcsc.com/55854168.html>.
3. JLCPCB part page CI58N330SM4 (C52940991): <https://jlcpcb.com/partdetail/CI58N330SM4/C52940991>.
4. Tokmas official website: <https://www.tokmas.com> (广州托克马斯电子有限公司).

*Report generated 2026-11-08 (Asia/Shanghai).*
