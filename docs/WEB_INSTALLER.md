# Web installer

CYD_TFT_RetroClock installs from the browser at
[anthonyjclarke.github.io/CYD_TFT_RetroClock](https://anthonyjclarke.github.io/CYD_TFT_RetroClock/),
using the shared [cyd-web-installer](https://github.com/anthonyjclarke/cyd-web-installer)
tooling (reusable workflow, `tools/merge_bin.py`, vendored Improv 1.0.1).
Release images come only from CI on a `v*` tag on `main`.

---

## Project facts

| Item              | Value                                      |
|:------------------|:-------------------------------------------|
| Env               | `esp32-cyd` – CYD 2.8″ ESP32-2432S028R     |
| Display driver    | `ILI9341_2_DRIVER`                         |
| Platform          | `espressif32@6.12.0`                       |
| Partitions        | `partitions_custom.csv` (from 3.7.0)       |
| App size (3.7.0)  | 1,040,464 B – 56 % of the 1.79 MB slot     |
| Improv name       | `CYD_TFT_RetroClock` (`PROJECT_NAME`)      |
| Setup hotspot     | `CYD_Clock_Setup`                          |
| OTA               | ArduinoOTA (port 3232) – case 3 applies    |
| Settings in NVS   | None – only WiFi survives a reboot         |

---

## Smoke test (RUNBOOK 5a)

Pending.

---

## Tests owed

Smoke-tested only. Run these on the next real work on this project, or before
the next release, and tick them off with date and board MAC.

- [ ] Case 1 – fresh install, erased, on each remaining board
- [ ] Case 2 – Update on a provisioned board (settings kept)
- [ ] Case 3 – Update from `app1` (only if the project has OTA)
- Case 4 – not applicable: one env
- [ ] Upgrade from 3.6 – Install without erase keeps WiFi across the
      `default.csv` → `partitions_custom.csv` switch
