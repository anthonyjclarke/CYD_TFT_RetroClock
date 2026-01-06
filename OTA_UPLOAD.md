# OTA (Over-The-Air) Firmware Updates

This guide explains how to upload firmware updates to your ESP32 CYD clock wirelessly, without needing a USB connection.

## Requirements

- Device must be connected to WiFi
- Device and computer must be on the same network
- Device must have OTA-enabled firmware already installed (v3.6+)

## Initial Setup (First Time Only)

**You must upload OTA-enabled firmware via USB first:**

```bash
pio run -t upload
```

After this initial upload, all future updates can be done wirelessly.

## OTA Upload Methods

### Method 1: Command Line (Recommended)

Once your device is connected to WiFi, note its IP address from:
- Serial monitor output
- TFT display (shown for 2.5 seconds at startup)
- Your router's DHCP client list

**Update the IP address in `platformio.ini`** (line 19):
```ini
upload_port = 192.168.1.123  ; Change to your device's IP
```

Then upload firmware wirelessly:

```bash
pio run -t upload
```

**Alternative:** Upload without changing `platformio.ini`:

```bash
pio run -t upload --upload-port 192.168.1.123
```

Replace `192.168.1.123` with your device's actual IP address.

### Method 2: VS Code Tasks (Easy)

The project includes custom VS Code tasks for convenient uploading:

1. Open Command Palette (`Cmd+Shift+P` / `Ctrl+Shift+P`)
2. Type "Tasks: Run Task"
3. Select one of:
   - **"PlatformIO: Upload (OTA)"** - Wireless upload using IP from `platformio.ini`
   - **"PlatformIO: Upload (USB)"** - USB upload fallback

These tasks are defined in `.vscode/tasks.json`.

### Method 3: Using mDNS Hostname

If your network supports mDNS (most do):

```bash
pio run -t upload --upload-port CYD-Clock.local
```

### ⚠️ VS Code Upload Icon Issue

**IMPORTANT:** The Upload icon (→) in the VS Code PlatformIO toolbar has a known issue with OTA uploads.

**Problem:** VS Code caches the last USB port used and automatically adds `--upload-port /dev/cu.usbserial-330` to the command, which overrides your OTA configuration in `platformio.ini`.

**Error you'll see:**
```
17:13:26 [ERROR]: Host /dev/cu.usbserial-330 Not Found
*** [upload] Error 1
```

**Workaround:** Don't use the Upload icon for OTA uploads. Instead, use:
- **Command line:** `pio run -t upload` (fastest)
- **VS Code task:** "PlatformIO: Upload (OTA)" (easiest)
- **USB fallback:** Use the "PlatformIO: Upload (USB)" task or upload icon

## Upload Process

During OTA upload, the device will:

1. Display **"OTA"** on the TFT screen
2. Show progress percentage (**"OTA 10%"**, **"OTA 20%"**, etc.)
3. Display **"OTA OK"** when complete
4. Automatically restart with the new firmware

**Note:** The upload typically takes 30-60 seconds depending on network speed.

## Troubleshooting

### "No Answer" Error

**Problem:** Device doesn't respond to OTA upload request

**Solutions:**
- Verify device is powered on and connected to WiFi
- Check IP address is correct (it may have changed)
- Ensure device and computer are on the same network
- Check firewall isn't blocking port 3232
- Try restarting the device

### "Authentication Failed" Error

**Problem:** Wrong OTA password

**Solution:** The default password is `CYD_OTA_2024`. If you changed it in the code, use your custom password.

### Upload Fails Partway Through

**Problem:** Connection lost during upload

**Solutions:**
- Move closer to WiFi router for better signal
- Ensure device has stable power supply
- Check network isn't congested
- Try again - OTA is resilient to interruptions

### Device Not Found by Hostname

**Problem:** `CYD-Clock.local` doesn't resolve

**Solutions:**
- Use IP address instead
- Check if mDNS is supported on your network
- On Windows, install [Bonjour Print Services](https://support.apple.com/kb/DL999)
- On Linux, ensure `avahi-daemon` is running

## Security Considerations

### Change Default Password

The default OTA password is `CYD_OTA_2024`. For better security:

1. Edit `src/cyd_tft_clock.cpp` line 1858:
   ```cpp
   ArduinoOTA.setPassword("YOUR_SECURE_PASSWORD");
   ```

2. Edit `platformio.ini` line 26:
   ```ini
   --auth=YOUR_SECURE_PASSWORD
   ```

3. Upload via USB or existing OTA connection

### Network Security

- OTA only works on your local network (WiFi)
- Password is required for all uploads
- Device hostname: `CYD-Clock`
- OTA port: 3232 (UDP)

## Monitoring OTA Updates

Watch the serial monitor during OTA upload:

```bash
pio device monitor
```

You'll see detailed progress:
```
OTA Update Start: sketch
OTA Progress: 10%
OTA Progress: 20%
...
OTA Progress: 100%
OTA Update Complete
```

## Reverting to USB Upload

If OTA isn't working, you can always fall back to USB:

```bash
pio run -t upload --upload-port /dev/cu.usbserial-330
```

(Adjust port for your system)

## Technical Details

- **Protocol:** ESP OTA (UDP port 3232)
- **Hostname:** CYD-Clock
- **Password:** CYD_OTA_2024 (default)
- **Flash Partition:** ~1.3MB available
- **RAM Overhead:** ~4KB
- **mDNS Support:** Yes

## Best Practices

1. **Always verify WiFi connection** before attempting OTA
2. **Don't interrupt power** during OTA upload
3. **Keep a backup** of working firmware
4. **Test on one device** before mass deployment
5. **Use USB for major updates** (changing partition scheme, etc.)
6. **Monitor serial output** during first OTA upload
7. **Note the IP address** or use static DHCP reservation

## Example Workflow

1. Make code changes on your computer
2. Save changes
3. Build firmware: `pio run`
4. Upload via OTA: `pio run -t upload --upload-port 192.168.1.123`
5. Watch TFT display show progress
6. Device restarts automatically
7. Verify new functionality

**No USB cable needed!** 🎉

## FAQ

**Q: Can I brick my device with OTA?**
A: Very unlikely. If OTA fails, the device keeps the old firmware. You can always recover via USB.

**Q: Does OTA work if WiFi credentials are wrong?**
A: No. Device must be connected to WiFi for OTA to work.

**Q: Can I update multiple devices at once?**
A: Yes, but do them sequentially, specifying each device's IP address.

**Q: What happens if power is lost during OTA?**
A: Device will boot with old firmware. Just upload again via USB or OTA.

**Q: Is OTA slower than USB?**
A: Yes, typically 30-60 seconds vs 10-20 seconds for USB. But the convenience is worth it!

## Summary

OTA updates make firmware development much faster by eliminating the need to physically connect to the device. After the initial USB upload, all future updates can be done wirelessly from across the room!

---

**Built with ❤️ by Anthony Clarke**
