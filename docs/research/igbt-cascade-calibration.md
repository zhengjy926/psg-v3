# Pulse-output stage delay and current-sampling calibration (psg-v3, STM32G474)

> Board: psg-v3 (STM32G474QET6, HRTIM1 CHA1/CHA2 → `PLUSE_N` / `PLUSE_P`).
> Output stage under review: NCP81074ADR2G → 10 × HFBR-1412TZ LED string (one current) → fibre → (receiver, off-sheet) → 10 × IKQ75N120CH3 in series.
> Purpose: size the propagation delay, decide whether the v2 duration table can carry it, and fix the ADC sample-point and current-zero scheme against the interfaces already frozen for v3 (#12 / #13).
> Method: first-party data sheets (Broadcom, Infineon, onsemi) + the v1/v2 schematic + the as-built v2 source. Every number below is either quoted with page/file:line or explicitly marked **estimate**.

---

## 0. TL;DR

| Question | Answer |
|---|---|
| What is `HFBFR-1412TZ`? | Typo for **`HFBR-1412TZ`** — a Broadcom **820 nm fibre-optic transmitter**: a bare AlGaAs **LED** in an ST-port housing, *not* an optocoupler, *not* an SSR and *not* a driver (pins 2/6/7 = ANODE, 3 = CATHODE, 1/4/5/8 = NC). |
| Its propagation delay | **None is specified** — it is an LED. The delay belongs to the **receiver** on the HV side (natural mate HFBR-24x2Z: tPLH 65 ns / tPHL 49 ns typ; TX+1 m fibre+RX link: 72/46 ns typ). |
| IKQ75N120CH3 turn-on | `td(on)` 34 ns + `tr` 47 ns = **81 ns** @25 °C (33 + 48 = 81 ns @175 °C), VCC 600 V, IC 75 A, RG 6 Ω. |
| NCP81074ADR2G delay | **15 ns typ / 27 ns max** both edges, CL = 1.8 nF; tr/tf 4 ns typ / 7 ns max. |
| Total delay (μC pin → IGBT current) | **≈ 200 ns typ**; datasheet-bounded 165–240 ns, credible band **150–300 ns**; budget **300 ns** for the sample point. |
| Does the v2 width table fix the absolute t0? | **No.** It is a width table; an absolute delay cancels between the two edges of one pulse. It *does* already contain the asymmetry Δ = t_off − t_on, which is the quantity that breaks T-002. |
| Sample point for 200 ns pulses | A flat top **does** exist at the current (~185 ns wide), but only *after* the gate pulse; a period "tail" is required to make it schedulable. v2's fallback (`preset/2`) samples **before the current rises**. |
| ADC zero offset | #13 folds it into the ±0.2 A term; that is only safe if the measured zero is ≲ 2 LSB (46 mA). **Auto-zero at boot** (RAM only, no command, no flash) removes it without breaking #13. |
| 10 series IGBTs on one trigger | Sharing is **hardware** (matched gate networks + clamps). Firmware owns edge cleanliness, jitter, minimum width and minimum gap — it cannot trim per-device timing. |

---

## 1. Evidence base, and one correction to the premise

### 1.1 The schematic

Source: `docs/research/assets/op-pluse-schematic.png` (sheet "OP Pluse", 1252 × 825).
Read at 5–8× zoom around `FC1` and `U10`; pin names and part number are legible:

- `U9` / `U12` = `NFEB1PT472C1H9L` EMI filter, DC 20 V → `20V_B` / `20V_A`.
- `U10` = `NCP81074ADR2G` (channel N), `U11` = `NCP81074ADR2G` (channel P).
- Inputs `PLUSE_N` / `PLUSE_P` through `R17` / `R21` = 22 Ω; `R20` = 0 Ω to `GND_OP`.
- Outputs through `R18` / `R22` = **56 Ω 0.5 W** into a **series chain of ten** `HFBR-1412TZ` (`FC1…FC10` on N, `FC11…FC20` on P).
- Chain entry is pin 2 (`AN1`), exit is pin 3 (`CA`); the designer's note on the sheet is
  `I=(20−1.7*10)/56=55 mA`.
- The other pins of each device (`AN2` = 6, `AN3` = 7, `NC1…NC4` = 1/4/5/8) are unused.

So the ticket's "10 optos in series, one current, all trigger simultaneously" is **confirmed as a topology**, but the device is a transmitter, not an optocoupler.

### 1.2 Identity

The sheet annotation reads `HFBR-1412TZ` (the ticket's `HFBFR-` has an extra `F`). The pin-out on the sheet matches the Broadcom data sheet exactly — [HFBR-14xxZ/24xxZ Data Sheet, p.14]:

| Pin | Function (data sheet p.14) | Sheet label |
|---:|---|---|
| 1 | NC | NC1 |
| 2 | **ANODE** | AN1 |
| 3 | **CATHODE** | CA |
| 4 | NC | NC2 |
| 5 | NC | NC3 |
| 6 | **ANODE** | AN2 |
| 7 | **ANODE** | AN3 |
| 8 | NC | NC4 |

Notes on that page: *"PINS 1, 4, 5, AND 8 ARE ELECTRICALLY CONNECTED"* and *"PINS 2, 6, AND 7 ARE ELECTRICALLY CONNECTED TO THE HEADER"* — i.e. 2/6/7 are one electrical node (the LED anode) and 3 is the cathode. That is why the sheet can chain only two pins per package and still use ten devices.

**Consequence:** the HFBR-1412TZ is **only an LED**. It contains no driver, no phototransistor and no output. The 5 MBd link's timing is set by the receiver and is specified only as a *link* ([p.10]) or as *receiver* ([p.20]).

> Uncertainty flag: the schematic page shows **only the transmitters**. Ten fibres leave the sheet and the receivers plus the HV-side gate drives are on another assembly whose part numbers are not in this repository. The natural mate is `HFBR-24x2Z` (5 MBd TTL receiver, same family), but **this is an inference, not a fact**. All receiver numbers below are conditional on it.

---

## 2. Device data (first-party)

### 2.1 HFBR-1412TZ — Broadcom, 820 nm miniature-link transmitter

Family `HFBR-14xxZ` (TX) / `HFBR-24xxZ` (RX). Suffix decode: `1` = transmitter, `4` = 820 nm family, `1` = ST housed, `2` = standard power, `T` = threaded port, `Z` = RoHS ([p.2]).

| Parameter | Value | Conditions | Source |
|---|---|---|---|
| Forward voltage VF | 1.48 / **1.70** / 2.09 V (min/typ/max) | IF = 60 mA dc | p.15 |
| VF temp. coefficient | −0.22 mV/K | IF = 60 mA | p.15 |
| Peak wavelength | 792 / 820 / 865 nm | — | p.15 |
| Forward current IF dc / peak | 100 mA / 200 mA | peak ≤ 2 ns | p.14 |
| Reverse input voltage | 1.8–3.8 V | IF = −100 µA | p.15 |
| Diode capacitance | 55 pF | V = 0, 1 MHz | p.15 |
| Optical tr, tf (10→90 %) | **4.0 typ / 6.5 max ns** (3.0 typ for IF 10–100 mA) | no pre-bias, IF = 60 mA | p.16 |
| Optical pulse-width distortion | 0.5 ns typ | — | p.16 |
| **Propagation delay** | **not specified** (LED only) | — | p.14–16 |
| Link tPLH / tPHL (TX + 1 m fibre + RX) | **72 / 46 ns typ** | 25 °C, PR = −21 dBm, 5 MBd | p.10 |
| Link pulse-width distortion | 26 ns typ | same | p.10 |
| Receiver tPLHR / tPHLR (HFBR-24x2Z) | **65 / 49 ns typ** | 25 °C, PR = −21 dBm | p.20 |

Two consequences worth carrying forward:

1. **The 0.5–5 µs "standard optocoupler" intuition does not apply here.** The whole optical path is ~50–80 ns, and it is *not* the dominant term.
2. **The LED bias is marginal, and that spreads the delay.** Ten LEDs at VF(max) need 10 × 2.09 = **20.9 V**, more than the 20 V rail; at VF(typ) the string draws (20 − 17)/56 = 53.6 mA. With VF binning alone (±0.3 V/device, i.e. ±3 V per string) the current can swing from ~0 to ~70 mA. The receiver's delay is a function of received optical power (the data sheet plots delay vs transmitter current, Figure 6, p.12), so **part-to-part and temperature-dependent delay spread of tens of ns is inherent to this bias scheme**. Measure the actual string current before trusting any nominal delay.

### 2.2 IKQ75N120CH3 — Infineon, 1200 V / 75 A Highspeed3 IGBT (TO-247-3)

Data sheet V2.3, 2019-04-15.

| Parameter | Value | Conditions | Source |
|---|---|---|---|
| VCE | **1200 V** | Tvj ≥ 25 °C | p.3 |
| IC | 150 A @ Tc 25 °C / **75 A @ Tc 134 °C** | — | p.3 |
| ICpuls | 300 A | tp limited by Tvjmax | p.3 |
| **VCEsat** | **2.00 V typ / 2.35 V max @ 25 °C**, 2.50 V typ @ 175 °C | VGE = 15 V, IC = 75 A | p.4 |
| VGE(th) | 5.1 / 5.8 / 6.5 V | IC = 2.6 mA | p.4 |
| QG | 370 nC | VCC 960 V, IC 75 A, VGE 15 V | p.4 |
| Cies / Cres | 4856 / 290 pF | VCE 25 V, 1 MHz | p.4 |
| tSC | 10 µs | VGE 15 V, VCC ≤ 600 V, Tvj 175 °C | p.3 |

Switching, inductive load, **VCC = 600 V, IC = 75 A, VGE = 0/15 V, RG(on) = RG(off) = 6 Ω, Lσ = 90 nH, Cσ = 67 pF** (p.4):

| Symbol | 25 °C | 175 °C |
|---|---:|---:|
| td(on) | **34 ns** | **33 ns** |
| tr | **47 ns** | **48 ns** |
| **ton = td(on) + tr** | **81 ns** | **81 ns** |
| td(off) | **282 ns** | **388 ns** |
| tf | 29 ns | 66 ns |
| toff = td(off) + tf | 311 ns | 454 ns |
| Eon / Eoff | 6.40 / 2.80 mJ | 10.20 / 6.00 mJ |

Notes that matter for calibration:

- These are **typ only** — the data sheet gives no min/max. Device-to-device spread is therefore *unbounded by the data sheet*, which is exactly why a 10-device series stack needs external balancing (§7).
- `td(off)` (282–388 ns) is ~8–11× `td(on)`. **The current pulse is therefore several hundred ns longer than the gate pulse**, and that asymmetry is the single most important number for both the width table and the sample point.

### 2.3 NCP81074ADR2G — onsemi, single-channel 10 A low-side driver

NCP81074/D Rev. 5, November 2023 (the `A` suffix = fixed digital threshold; `DR2G` = SOIC-8, tape & reel, −40…+140 °C, p.2).

| Parameter | Typ | Max | Conditions | Source |
|---|---:|---:|---|---|
| td1, IN rising → OUT (low→high) | **15 ns** | **27 ns** | CL = 1.8 nF | p.4 |
| td2, IN falling → OUT (high→low) | **15 ns** | **27 ns** | CL = 1.8 nF | p.5 |
| tr | 4 ns | 7 ns | CL = 1.8 nF | p.5 |
| tf | 4 ns | 7 ns | CL = 1.8 nF | p.5 |
| VDD range | 4.5–20 V | — | — | p.1 |
| Peak source/sink | 10 A | — | 200 ns pulse | p.4 |

On this board the load is not 1.8 nF but a 56 Ω + 10-LED string, so `tr/tf` here are *current*-limited by the string, not by the driver: the LED current edge is a few ns (L/R with 56 Ω), and the 1.8 nF figures do **not** apply. The 15 ns propagation delay does.

---

## 3. Total delay from the HRTIM pin to IGBT current

### 3.1 Per-stage budget

Values marked **est** are engineering estimates, not data sheet numbers — flagged because they are the weakest links.

| # | Stage | Typ | Min | Max | Basis |
|---|---|---:|---:|---:|---|
| 1 | NCP81074 input → output | 15 | 15 | 27 | [NCP p.4–5] |
| 2 | Driver output edge into the LED string | 3 | 2 | 7 | **est**; resistive 56 Ω load |
| 3 | LED optical rise (to 90 % light) | 4.0 | 3.0 | 6.5 | [HFBR p.16] |
| 4 | Fibre transit, 1–3 m | 10 | 5 | 15 | n ≈ 1.5 ⇒ 5 ns/m |
| 5 | Receiver, tPLH (assumed HFBR-24x2Z) | 65 | 46 | 72 | [HFBR p.10, p.20] |
| 6 | HV-side gate driver (assumed NCP81074-class) | 20 | 15 | 27 | **est**; not on this sheet |
| 7 | IGBT td(on) + tr | **81** | 81 | 81 | [IKQ p.4] |
| | **Sum** | **≈ 198** | **≈ 167** | **≈ 236** | |

Add device spread (td(on) is typ-only; LED VF binning §2.1) and temperature: **use 150–300 ns, and budget 300 ns**.

Turn-off path, same arithmetic with `td(off) + tf`:

| | 25 °C | 175 °C |
|---|---:|---:|
| t_off (pin → current zero) | **≈ 412 ns** | **≈ 555 ns** |
| t_on (pin → current 90 %) | ≈ 198 ns | ≈ 198 ns |
| **Δ = t_off − t_on** | **≈ 215 ns** | **≈ 355 ns** |

### 3.2 Why not 10×

Correct as stated in the ticket: the ten LEDs are in **series on the drive side**, so one current lights all ten at once, and each device feeds its own fibre. The signal passes through **one** transmitter, **one** fibre and **one** receiver per channel. The ten IGBTs are in series on the *power* side and are driven in parallel from ten independent gate drives (inferred: ten fibres ⇒ ten receivers ⇒ ten gate drives) — they add **no** propagation delay in series, only spread (§7).

### 3.3 Cross-check against the v2 table — and a warning

The v2 table ([`PFA_PSG_V2.0/Modules/mod_pulse.c:105-152`]) is `preset_ns → actual_ns`. An endpoint fit of the 1500 V row gives

```
actual ≈ 1.0375 × preset + 74 ns      (200 ns…3000 ns)
```

Compare with the datasheet-derived prediction for the current pulse: `W_out = W_gate + Δ`, Δ ≈ 215 ns @25 °C. The two **disagree badly**:

- at preset 3000 the measured stretch is 187 ns — close to Δ;
- at preset 200 the measured stretch is 82 ns — far from Δ, and the 3.75 % slope has no digital explanation (the HRTIM tick is 2.94 ns, < 0.1 %).

So either (a) the v2 measurements were taken **upstream of the IGBTs** (gate or opto output) on hardware that is not this stack, or (b) the probe/threshold used for `actual_ns` distorts short pulses. Either way:

> **Do not inherit the v2 table as-is.** The v3 spec already says the first default table is a placeholder derived from v2 and is to be replaced on the bench ([`.tmp/psg-v3-spec.md:173`]), and this analysis is a second, independent reason to do so. The v2 table is fine as a *shape* (target→preset, monotone) but its values describe different hardware.

---

## 4. Width versus absolute t0 — what the table does and does not correct

### 4.1 The table measures widths, not timestamps

`CalibrationPoint_t` holds two *durations* ([`mod_pulse.c:56-59`]), and `InverseInterpolateSingle` inverts the `actual_ns` column ([`mod_pulse.c:327-380`]). A width is the distance between two edges of the same pulse. Any delay common to both edges cancels exactly. Therefore:

**The table can never observe or correct the absolute t0 shift.** It corrects `W_out` only.

### 4.2 What it *does* contain: the edge asymmetry Δ

The two edges do **not** have the same delay (§3.1: 198 ns vs 412 ns). The width therefore carries the *difference*:

```
W_out = W_gate + (t_off − t_on) = W_gate + Δ
```

The table's non-zero intercept (+74 ns in v2) is *that hardware's* measured Δ folded into the width axis. This is legitimate for T-001 — but it has a consequence the v2 code did not draw out, and §3.3 shows the v2 value does not match the IKQ stack.

### 4.3 Which v3 requirement is actually exposed

GLOSSARY definitions matter here:

- **脉冲间隔** = "silence at the start of the next repetition unit, ending at the first transition of that unit" — i.e. measured **from the previous pulse's trailing edge to the next pulse's leading edge** ([`GLOSSARY.md:85-86`]).
- **极性间间隔** = from the positive pulse's **second** transition to the negative pulse's **first** transition ([`GLOSSARY.md:81-82`]).
- **脉冲串间隔** = end of one train to start of the next ([`GLOSSARY.md:93-94`]).

All three are **cross-edge** quantities: one trailing edge plus one leading edge. A common delay cancels, but Δ does **not**:

| Req. | Quantity | Effect of Δ ≈ 215 ns | Budget | Verdict |
|---|---|---|---|---|
| T-001 | pulse width | already absorbed by the table (it *is* Δ) | ±(2 % + 20 ns) | table must be measured at the load |
| T-002 | pulse interval | interval **shrinks by Δ** | ±(1 % + 50 ns) → ±60 ns @1 µs | **must compensate**; 215 ns is 3.5× the budget |
| T-002 | polarity gap | shrinks by Δ **plus** pos/neg channel skew | ±(1 % + 50 ns) | compensate Δ and the skew |
| T-003 | train gap | shrinks by Δ, but gap ≥ 1 ms | ±(1 % + 10 µs) | Δ negligible |
| T-004 | group gap / trigger delay | ms scale | ±1 ms, ±1 % | irrelevant |

v2 already knew this and patched it with a blanket constant:

```c
/* Modules/mod_pulse.c:530-531 */
pls_sep = pCfg->pls_sep + 150U;
pol_gap = pCfg->pol_gap + 150U;
```

That +150 ns is the interval compensation for Δ — **uncalibrated** ("未标定", [`.tmp/v2-calib-findings.md:108-114`]) and **inconsistent with the table's own intercept** (+74 ns). The 76 ns disagreement alone can break T-002 at the 1 µs minimum interval. In v3 the two must come from **one** measurement: the same fit that builds the duration table also yields Δ, and Δ is what goes into the interval/gap compensation.

### 4.4 Per-channel skew does not cancel anywhere

The N and P channels are separate drivers (U10/U11), separate LED strings and separate receiver/gate-drive chains. Their delays differ by some tens of ns. Unlike Δ, this skew does **not** cancel in T-001 (each channel has its own table) but **does** appear directly in the polarity gap:

```
gap_err = Δ + (t_on_pos − t_on_neg)
```

so v3 should carry one `skew_ns = t_on_pos − t_on_neg` measured on the bench and add it to `pol_gap` (sign per the definition: pos→neg). If |skew| < ~20 ns it can be folded into the T-002 margin; above that it should be compensated explicitly.

### 4.5 Answer to Q5

- **No**, the width table does not and cannot correct the absolute t0 shift.
- **T-001/T-003/T-004 do not need** an absolute correction — they are all differences, and the ms-scale ones are immune anyway.
- **T-002 does need a correction**, but for Δ (trailing-vs-leading edge) and for channel skew, not for t0. v2's +150 ns is the right *idea* with the wrong *number*.
- **The sample point is the only place where absolute t0 matters** (§5) — which is why it must be a separate constant, not a table column.

---

## 5. The current sample point

### 5.1 When is the current actually flat?

From §3.1, referred to the HRTIM compare event (t = 0):

```
current rises       at t ≈ 151 ns (10 %)  →  198 ns (90 %)
current flat        from t ≈ 200 ns
gate turns off      at t ≈ 120 ns + W_gate
current starts down at t ≈ 120 ns + W_gate + td(off) ≈ W_out + 190 ns
current reaches 0   at ≈ W_out + 220 ns
```

So the flat top is approximately **[200 ns, W_out + 190 ns]** — and it exists even for a 200 ns output pulse, because the IGBT's storage time holds the current after the gate has gone. The width of that flat top is ≈ W_out.

### 5.2 Numbers for the protocol's width range

Using D = 200 ns (t_on to 90 %) and the flat-top end from §5.1:

| Configured width W_out | Flat top | Usable window |
|---:|---|---:|
| 200 ns | 200 → 390 ns | **190 ns** |
| 500 ns | 200 → 690 ns | 490 ns |
| 1 000 ns | 200 → 1 190 ns | 990 ns |
| 2 000 ns | 200 → 2 190 ns | 1 990 ns |
| 5 000 ns | 200 → 5 190 ns | 4 990 ns |

**For 200 ns the flat top exists but is only ~190 ns wide.** That is enough for one ADC sample only if the sample aperture is short (§5.5).

> If the HV-side current sensor is a Hall transducer or an isolated amplifier rather than a shunt + fast amp, its own delay may be µs-scale and may dominate everything above. The schematic does not show the sensor; **measure the sensor's step response before choosing any sample offset.**

### 5.3 The HRTIM period constraint, and the "tail" fix

In the v2 arrangement the pulse sits at the **end** of the timer period: `t0 = pls_sep`, `PER = t0 + W` ([`mod_pulse.c:537-546`]). A compare value must be `< PER`, so the ADC trigger can only be placed **inside the gate pulse**. With `W_preset = 134 ns` (the v2 inverse for a 200 ns target) and a required offset of 250 ns, that is impossible — the v2 code clamps the compare to `PER−1` ([`mod_pulse.c:483-495`]), i.e. it samples at the very end of the gate pulse, before the current has risen.

Clean fix, no change to the emitted waveform:

1. Define `TAIL` (e.g. **400 ns**), the room needed after the gate pulse.
2. Move the pulse earlier: `t0 = pls_sep − TAIL`, and set `PER = t0 + W_preset + TAIL` — which equals the old `pls_sep + W_preset`, so **the period and therefore every interval are unchanged**.
3. The sample compare `t0 + t_s` is now always `< PER` for `t_s < W_preset + TAIL`, i.e. for every width down to 134 ns with `t_s` up to ~350 ns (worst-case delay); the nominal is ~250 ns.
4. For very short pulses the sample lands a few tens of ns *after* the gate edge — still inside the current flat top, by design (§5.1).

This needs `pls_sep ≥ TAIL` (F-020 guarantees ≥ 1000 ns, so 400 ns is safe; clamp `t0` at a small positive minimum).

### 5.4 Two defects in the v2 sample code (do not port them)

```c
/* Modules/mod_pulse.c:458-474 */
static uint32_t SampleOffsetNs(uint32_t width_ns)
{
    uint32_t sample_delay = TURN_ON_DELAY_TIME + RISE_TIME;   /* 100 + 150 = 250 */
    ...
    } else {
        offset = width_ns / 2U;      /* <-- short-pulse fallback */
    }
```

1. **Wrong axis**: the call sites pass `pos_dur` / `neg_dur`, which are the *calibrated presets* ([`mod_pulse.c:524-529`], used at `:552-555`, `:572-574`, `:592-593`), not the target output width. For a 200 ns target the preset is ~134 ns, so the "width" used for the sample rule is 100 ns smaller than the pulse that actually appears.
2. **Wrong side of the delay**: when the preset is shorter than 250 ns the code samples at `preset/2` ≈ 67 ns — *before* the current has even started (t_on ≈ 154–200 ns). Any current reading for short pulses with the v2 rule is measuring the previous state, not the pulse.

### 5.5 Recommended rule

```
t_s(ch, tier) = t_on(ch) + rise(ch) + M            M = 50…100 ns
```

- `t_on`, `rise` are **per channel**, optionally **per voltage tier** (§5.6).
- `t_s` is computed **once per group** at plan freeze (it does not vary pulse to pulse), converted to ticks, and handed to `sample` with the plan.
- Hard constraint: `t_s < W_preset + TAIL` (guaranteed by TAIL = 400 ns for W ≥ 200 ns).
- Soft constraint: the ADC **sample phase** (sample time × 1/f_ADC) must fit inside the remaining flat top `W_out − (t_s − 200)`. For a 200 ns pulse with `t_s = 250` that leaves ~140 ns; choose the shortest sample time the source impedance allows (and/or use the ADC's hardware oversampling to average several samples inside the window).
- Diagnostic: if `W_target < t_on + rise + M`, log a `NO_FLAT_TOP` warning at plan freeze (do **not** reject — F-020 still allows 200 ns).

### 5.6 Per-tier, per-width or per-channel?

| Dimension | Needed? | Why |
|---|---|---|
| **Per channel** | **Yes** | different driver, LED string and receiver; skew shows up here (and in T-002, §4.4) |
| **Per voltage tier** | **Only if measured spread > ~20 ns** | the v2 width data already shows a 36 ns tier spread at 200 ns (281.75 ns vs 245.29 ns); if the delay tracks it, use a 6-entry table. Otherwise one constant. |
| **Per pulse width** | **No** | `t_on` and `rise` are edge properties, not width properties. Width enters only through the clamp/TAIL. |

---

## 6. ADC electrical zero offset (电流采样的偏移)

### 6.1 How to measure it

1. **Static zero, board powered, output stage off.** The IGBTs are off, so the primary current is exactly zero. Trigger the ADC in software (or run the HRTIM trigger with the outputs disabled) and average **≥ 1024** conversions per channel. That average, in raw LSB, is the whole-chain zero: ADC internal offset + op-amp Vos + reference + any bias network.
2. **Two-point check** to separate offset from gain: 0 A and a known current from a current source (e.g. 50 A). `offset = raw(0)`, `gain = (raw(50) − raw(0))/50`.
3. **Repeat warm and cold.** The op-amp offset drifts; the value captured at 25 °C is not guaranteed at −10 °C.
4. Convert to current: **1 LSB = K = 23.055 mA** ([`.tmp/res-13.md:13-17`]), so 1 LSB = 0.023 A.

### 6.2 Does #13 absorb it?

#13 says, verbatim, *"不做零点/偏置项：乘性误差下 `上报 / 真值` 是常数，零点折进 T-007 的 0.2 A 绝对项"* ([`.tmp/res-13.md:27`]).

That reasoning is correct **for gain** but not for offset: an offset adds a constant current, so `reported/true` is *not* constant — it diverges as the current falls. The offset is therefore not "absorbed" by the multiplicative model; it is **charged against the ±0.2 A absolute term** of T-007 ([`docs/需求规格说明书.md:166`]).

Budget check:

| Zero offset | In current | Share of the 0.2 A term | Verdict |
|---:|---:|---:|---|
| 1 LSB | 23 mA | 12 % | fine |
| 2 LSB | 46 mA | 23 % | acceptable |
| 5 LSB | 115 mA | 58 % | eats most of the low-current margin |
| 9 LSB | 207 mA | **104 %** | T-007 fails at low current by itself |

So #13 absorbs the offset **only if the measured zero is ≲ 2 LSB**. Whether it is depends on a front-end (`35 / 0.893 / 1.37`, provenance unknown — [`.tmp/res-13.md:21`]) that has not been characterised. **Measure first, then decide.**

### 6.3 Recommendation: boot-time auto-zero, RAM only

This removes the offset without adding a command, a flash record or a per-board production step — i.e. without touching #13's contract:

- At APPLICATION init (outputs disabled, no pulse yet), capture `raw_zero[ch]` over ≥ 1024 samples; sanity-check it (e.g. reject and fall back to a compile-time default if > 200 LSB, which would indicate a hardware fault).
- Keep `K` unchanged; do **not** fold the zero into `K` (it is additive, not multiplicative).
- At plan freeze, convert the under-current threshold with the zero included:
  `raw_thr_eff = round(A / K) + raw_zero[ch]` — then `sample` keeps comparing **unmodified raw averages** with `u16`, exactly as #13 requires ([`.tmp/res-13.md:44`]).
- At report time only: `A = (raw_avg − raw_zero[ch]) × K` (float, not on the hot path).
- Optional: re-capture at each IDLE→RUN transition if the ADC driver supports a software trigger; boot-only is sufficient for a first cut.

If measurement shows the zero is dominated by a systematic (design-wide) offset rather than per-board spread, a compile-time `raw_zero` in `calib_current_cfg.h` is enough and the boot capture can be dropped.

---

## 7. Ten series IKQ75N120CH3 on one trigger

### 7.1 What firmware can and cannot do

The ten devices share **one** gate signal (one LED string → ten fibres → ten receivers → ten gate drives). There is no per-device timing knob in firmware. Therefore:

- **Voltage sharing is a hardware property.** Static sharing follows VCEsat matching (2.00 V typ @25 °C, 2.50 V @175 °C) and leakage; note that the positive VCEsat temperature coefficient the data sheet advertises for *paralleling* works **against** series sharing (the device that sees more volts runs hotter and drops more). Dynamic sharing follows gate-delay matching, gate resistors, device capacitances and stray inductance, and normally needs snubbers / active clamps across each device.
- The data sheet gives switching times as **typ only** (no min/max, p.4–5), and the receivers likewise (65/49 ns typ, p.20). Neither spread is bounded by any first-party document — so the stack's skew budget cannot be closed from data sheets at all. It has to be measured.

### 7.2 What firmware must guarantee

1. **One clean edge per transition.** No retriggering, no double pulses, no PWM chattering — a second edge inside `td(off)` restarts the storage-time clock and unbalances the stack.
2. **Jitter ≪ transition time.** The stack's critical window is the ~50–100 ns current rise/fall; HRTIM jitter is ~3 ns, which is fine. Do not let communication load perturb the compare values (T-008 already forbids this).
3. **Minimum ON width.** The stack must reach full enhancement before turn-off. `ton = 81 ns` is the device term; add driver/receiver. A `PULSE_MIN_WIDTH` around **250–300 ns** is the physical floor — and note this collides with F-020's 200 ns minimum ([`docs/需求规格说明书.md:62`]), which the hardware cannot deliver (see §5.2 and the note below).
4. **Minimum OFF gap.** `toff = 311 ns @25 °C, 454 ns @175 °C`. F-020's minimum interval of 1000 ns covers this with ~2× margin — keep it; do not "optimise" it away.
5. **No high-frequency repetition.** Ten 1200 V devices in series switching at kHz rates with 6.4 mJ Eon each is a thermal problem; the 1 ms minimum train gap is doing real work.

> **The 200 ns floor is not achievable at the output.** With `W_out ≈ W_gate + Δ` and Δ ≈ 215 ns, a 200 ns output pulse needs a *negative* gate width. The v2 measurements agree in spirit: the smallest `actual_ns` anywhere in the table is **245.29 ns** ([`mod_pulse.c:148`]). v2 papered over this by extrapolating the inverse to `preset = 134…167 ns` ([`.tmp/v2-calib-findings.md:207-218`]), which cannot produce a narrower current pulse. v3 should (a) treat < ~250 ns as "below floor" in the diagnostic, and (b) discuss with the systems owner whether F-020's 200 ns minimum should be raised, or whether T-001's ±(2 % + 20 ns) is simply waived below the floor.

### 7.3 The receiver spread is part of the stack's skew

Because each IGBT has its **own** receiver and gate drive, the ten gate signals are not identical: their skew is the sum of ten receivers' part-to-part delay spread plus ten gate-drive spreads. Nothing in the firmware can trim it. If the measured skew across the ten channels exceeds ~10 % of the current rise time, the hardware needs matched receivers/drivers or per-device clamps — a firmware workaround does not exist.

---

## 8. Recommended v3 scheme, mapped onto the frozen interfaces

### 8.1 Principles

1. **Separate the three calibrations by *what they measure*.** Width (host-writable, per tier), delay (factory constant, per channel, affects only the sample point), zero (runtime, per channel). Do not overload the 0x44 record.
2. **Nothing host-writable changes**: `0x44/0x45` keeps `(target_ns, preset_ns)` and its A/B record exactly as frozen in #12 ([`.tmp/res-12.md:19-32, 88-100`]). `fmt_ver` stays 1; no `section 1`.
3. **The delay constants are compile-time**, in a new `calib_timing_cfg.h` next to `calib_current_cfg.h`. They are design/factory data, not production calibration data, so E-009's "非易失只保存…标定表" is untouched ([`docs/需求规格说明书.md:187`]).
4. **The zero is runtime RAM**, captured at boot; no storage, no command.

### 8.2 Where each piece lives

| Piece | Shape | Where | Host-writable? |
|---|---|---|---|
| Width table | 6 tiers × 2–32 × `(target_ns, preset_ns)` | `calib` A/B record (unchanged, #12) | **yes**, 0x44/0x45 |
| Delay `t_on[ch]`, `rise[ch]`, optional `[tier][ch]` | u16 ns | `calib_timing_cfg.h` (compile-time) | no |
| Sample margin `M`, tail `TAIL` | u16 ns | `calib_timing_cfg.h` | no |
| Zero `raw_zero[ch]` | u16 LSB | RAM, captured at boot | no |
| Current gain `K` | float | `calib_current_cfg.h` (#13, unchanged) | no |

### 8.3 Interface deltas (still within #3's ≤ 8 functions per service)

`service/calib` currently exposes exactly six functions ([`.tmp/res-12.md:74-79`], [`.tmp/psg-v3-spec.md:144`]). Add **two** — this keeps the service at #3's ≤ 8 and keeps the zero encapsulated where `K` already lives:

```c
/* HRTIM edge -> ADC trigger, per channel, clamped against the plan's width and
 * TAIL; returned in ns, converted to ticks by drivers/hrtim. */
uint16_t calib_sample_offset_ns(uint8_t channel, uint16_t voltage, uint16_t target_ns);

/* Install the boot-time zero (raw average taken with the output stage off).
 * Returns 0 on success, -ERANGE if the value is implausible. */
int calib_zero_set(uint8_t channel, uint16_t raw_avg);
```

No getter is needed, because the two existing current functions absorb it:
`calib_raw_threshold()` returns `round(A/K) + zero[ch]`, and `calib_current_a()` returns `(raw − zero[ch]) × K`. So `pulse`/`sample` still only see `preset_ns` and `raw_threshold`, exactly as #12 and #13 require ([`.tmp/res-12.md:83-84`], [`.tmp/res-13.md:31-33`]). The **sample compare ticks** travel in the pulse plan (`pulse.load(plan)`), because the HRTIM timer that carries the ADC trigger is owned by `pulse` ([`.tmp/psg-v3-spec.md:140-141`]).

> Note: #12's text says *"sample 收 raw 阈值"* and the plan is frozen at start. Adding the sample compare to the plan is consistent with that ("plan 内是已标定 preset") and keeps the run-time hot path free of calibration lookups.

### 8.4 The sample formula, in firmware terms

```c
/* at 0x30 accept time, once per group */
for (ch = 0; ch < 2; ch++) {
    uint16_t w = (ch == POS) ? plan->pos_target_ns : plan->neg_target_ns;
    uint16_t off = calib_sample_offset_ns(ch, plan->voltage[ch], w);
    plan->sample_cmp[ch] = hrtim_ns_to_ticks(plan->t0_ns + off);
    plan->raw_thr_eff[ch] = calib_raw_threshold(ch, plan->under_a[ch]);  /* K + zero */
}
```

and in `pulse.load(plan)`:

```c
pls_sep = plan->pls_sep + delta_ns;            /* Δ compensation, §4.3 */
pol_gap = plan->pol_gap + delta_ns + skew_ns;
t0      = pls_sep - TAIL;                      /* pulse moved earlier, period unchanged */
cmp     = t0 + W_preset;
per     = t0 + W_preset + TAIL;                /* == pls_sep + W_preset */
```

`delta_ns` and `skew_ns` are measured once (§9) and live in `calib_timing_cfg.h`, replacing v2's hard-coded `+150U`.

### 8.5 What must **not** go into the duration table

- **t0 / delay columns.** It would break the host contract (the point is `(target, preset)` of a width), the strict-monotone u16 axes, and the round-trip 0x45 → 0x44 property (N-007, [`.tmp/res-12.md:36`]).
- **Per-tier delay.** Six entries × two channels is timing data, not duration data; it changes only the sample point.
- **Zero offset.** It is a current quantity; the table has no current axis at all (this was the v2 lesson, [`.tmp/v2-calib-findings.md:9-30`]).

### 8.6 The five-line summary

1. Keep #12's width table as the **only** host-writable calibration; it fixes width, never t0.
2. Add a compile-time timing profile `(t_on, rise, M, TAIL, Δ, skew)` and compute the ADC compare per channel at plan freeze as `t_on + rise + M`.
3. Add a **400 ns post-pulse tail** to the timer period (compensated by moving `t0` earlier) so the sample is schedulable for 200 ns pulses; delete v2's `preset/2` fallback and its use of the preset as the width axis.
4. Replace v2's hard-coded `+150 ns` interval/gap patch with the measured **Δ = t_off − t_on** (from the same fit that builds the table) plus a measured pos/neg **skew**.
5. Auto-zero the ADC at boot into RAM; because `calib_raw_threshold` and `calib_current_a` absorb it, apply it at plan freeze as `round(A/K) + zero` and at report as `(raw − zero)·K`; measure |zero| first to see whether #13's 0.2 A term already covers it.

---

## 9. Bench calibration procedure (what to actually do)

**Rig:** scope or high-bandwidth logger; current probe on the load (Rogowski or Pearson, not a Hall unless its step response is checked); probe on `TP5`/`TP6` (HRTIM side) and `TP4`/`TP7` (driver output) per schematic.

| Step | Measurement | Output |
|---|---|---|
| 1 | LED string current at 25 °C (measure across `R18`/`R22`, 56 Ω) and the resulting optical link delay | confirms the marginal bias (§2.1) and the delay spread with IF |
| 2 | Zero: 1024-sample average per channel, outputs off; repeat hot/cold | `raw_zero[2]`; decide #13 vs zero-term |
| 3 | Step response of the current sensor alone (if separable) | sensor delay τ_sensor to add to t_on |
| 4 | t_on/t_off at 3 widths × 6 tiers × 2 polarities: 10 %/50 %/90 % of current, relative to the HRTIM edge | `t_on[ch]`, `rise[ch]`, per-tier spread, `Δ`, `skew` |
| 5 | Width sweep, 6 tiers × 13 points, measured **at the load** | new `(target, preset)` table; least-squares fit gives Δ as the intercept |
| 6 | Sweep the sample offset across one pulse and plot reported current vs offset | pick `M` in the middle of the plateau; verify the 200 ns case |
| 7 | Verification: T-001 (±(2 % + 20 ns)), T-002 (±(1 % + 50 ns)) at the 1 µs minimum interval, T-003, T-006 in the extrapolation zone | pass/fail per requirement |

Step 4 is the one that must not be skipped: it is the only way to separate **t_on**, **Δ** and **skew**, and each lands in a different constant.

---

## 10. Uncertainty register

| # | Uncertainty | Impact | How to close |
|---|---|---|---|
| U1 | Receiver part on the HV side is **not shown**; assumed HFBR-24x2Z | ±20 ns on the total delay | BOM / photograph of the HV gate board |
| U2 | HV-side gate driver unknown | ±15 ns | same |
| U3 | `t_on` is typ-only in both data sheets (NCP max 27 ns; IKQ no max) | ±50 % of the 200 ns estimate | step 4 |
| U4 | LED current depends on VF binning and rail tolerance; delay depends on LED current | tens of ns, temperature-dependent | step 1; consider specifying a current-regulated LED bias instead of 56 Ω from 20 V |
| U5 | Current-sensor type and bandwidth unknown | may dominate everything | schematic of the sense chain |
| U6 | v2 table's measurement node unknown; its +74 ns intercept disagrees with Δ ≈ 215 ns | the inherited default table may be off by > 200 ns at the load | step 5 |
| U7 | 200 ns output width is below the physical floor (§7.2) | F-020 vs T-001 conflict | systems decision |

---

## 11. References

1. Broadcom, *HFBR-14xxZ and HFBR-24xxZ Series — Low-Cost, 820 nm Miniature Link Fiber Optic Components with ST, SMA, SC, and FC Ports*, data sheet (25 pp.). Read via distributor mirror: <https://datasheet.lcsc.com/datasheet/pdf/7dea55694eafb00b440bb3b2bc857cc7.pdf?productCode=C692070> (product page <https://www.lcsc.com/product-detail/C692070.html>). Manufacturer page: <https://www.broadcom.com/products/fiber-optic-modules-components/industrial/industrial-control-general-purpose/820nm/hfbr-1412tz>. Pages cited: 2 (suffix decode), 10 (5 MBd link, tPLH/tPHL), 14 (pin-out, absolute maximum), 15 (VF, λp), 16 (optical tr/tf, PWD), 20 (receiver tPLHR/tPHLR).
2. Infineon, *IKQ75N120CH3 — High speed switching series third generation IGBT*, data sheet V2.3, 2019-04-15 (16 pp.): <https://www.infineon.com/assets/row/public/documents/60/49/infineon-ikq75n120ch3-datasheet-en.pdf?fileId=5546d4625bd71aa0015bd817a3920534>. Pages cited: 3 (maximum ratings, thermal), 4 (static/dynamic, switching @25 °C), 5 (switching @175 °C).
3. onsemi, *NCP81074A/D — Single Channel 10 A High Speed Low-Side MOSFET Driver*, NCP81074/D Rev. 5, November 2023 (14 pp.): <https://www.onsemi.com/pdf/datasheet/ncp81074-d.pdf>. The canonical URL returned HTTP 403 (Akamai) to automated fetches; the copy read here came from the manufacturer's own mirror <https://www.onsemi.cn/pub/Collateral/NCP81074-D.PDF> (same document number, Rev. 5, November 2023). Pages cited: 1 (features, 15 ns), 2 (ordering, NCP81074ADR2G), 4–5 (td1/td2/tr/tf, CL = 1.8 nF).
4. Schematic sheet **"OP Pluse"** (output stage supplied with the ticket): `docs/research/assets/op-pluse-schematic.png` (U10/U11 NCP81074ADR2G, R17/R21 22 Ω, R18/R22 56 Ω, FC1–FC20 HFBR-1412TZ, note `I=(20−1.7*10)/56=55 mA`).
5. v2 as-built firmware, `E:\Working Item\DSL\DSL_PFA\PFA_PSG_V2.0` (read-only): `Modules/mod_pulse.c:56-59` (`CalibrationPoint_t`), `:68-69` (`TURN_ON_DELAY_TIME` 100, `RISE_TIME` 150), `:76` (`SENSE_GAIN` 1.0f), `:82` (`OCD_DAC_SCALE`), `:105-152` (six tables), `:327-380` (inverse interpolation), `:389-431` (`CalibratedPreset`), `:483-495` (`SampleCmpTicks`), `:506-599` (`ModPulse_CalculateReg`, incl. `+150U` at `:530-531` and sample scheduling at `:552-555`), `:713-730` (raw → A).
6. v2 fact-check note: `.tmp/v2-calib-findings.md` (6 tiers × 70 points, `(preset, actual)`, inverse + extrapolation, single current constant, no 0x44).
7. v3 requirements, `docs/需求规格说明书.md`: F-020 `:62`, F-024/F-025/F-026 `:66-68`, F-035 `:87`, I-022 `:140`, T-001…T-007 `:160-166`, E-009 `:187`, E-011 `:189`, N-007 `:204`.
8. v3 design resolutions: `.tmp/res-12.md` (duration table + 0x44/0x45 contract, interfaces `:59-80`, storage `:88-106`, freeze semantics `:108-113`); `.tmp/res-13.md` (constant `K` `:13-17`, "no zero term" `:27`, integer compare `:44`); `.tmp/psg-v3-spec.md:140-144` (service interfaces), `:171-177` (calibration).
9. v3 glossary: `GLOSSARY.md:81-86` (极性间间隔, 脉冲间隔), `:93-94` (脉冲串间隔), `:45-47` (标定).

*Report written 2026-11-08 (Asia/Shanghai). Numbers marked **est** are engineering estimates, not data sheet values; everything else is traceable to the references above.*
