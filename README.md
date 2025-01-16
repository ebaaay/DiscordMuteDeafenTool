# Discord Mute/Deafen Tool

This program is a tool for controlling mute and deafen states in Discord using the numpad keys, with an icon in the system tray. It is designed to make it easier to use Discord commands while utilizing the numpad, which can be useful during voice calls or streams.

## Functionality

- **NUMPAD0 key**:
  - Pressing NUMPAD0 briefly (less than 100 ms) simulates the key combination **Alt + NUMPAD9**, which will mute or unmute your microphone in Discord.
  - Holding NUMPAD0 for more than 100 ms simulates the key combination **Alt + NUMPAD8**, which activates the "Deafen" mode (disables both microphone and sound) in Discord.

- **System tray icon**:
  - The program adds an icon to the system tray that can be used to exit the program by right-clicking the icon and selecting "Exit".

- **Automatic startup**:
  - The program is set to run automatically when Windows starts.

## Requirements

To compile this program, you need to have **MinGW** installed and the necessary libraries to work with the Windows API.

## Compilation with MinGW

If you have **MinGW** installed, you can compile the program using the following command in the terminal:

```bash
g++ -o discord_hotkey.exe discord_hotkey.cpp -mwindows -municode -luser32 -lshell32 -ladvapi32
```

## Usage

1. **Compile the program** using the above command.
2. **Run the program**. An icon will appear in the system tray.
3. **Close the program** by right-clicking the system tray icon and selecting "Exit".

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for more details.
