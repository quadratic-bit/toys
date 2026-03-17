shepherd
--------

Resident register monitor with an assembler cow.

`shepherd.com` installs itself as a TSR by hooking interrupt `09h` (keyboard IRQ).
Pressing `F12` toggles an on-screen overlay (somewhere).

When enabled, it draws:
* a framed register monitor with current register values
* a cow under the frame
* a surprise under the cow

The cow orientation is also randomized.

The overlay is drawn on top of the existing text screen. When hidden, the saved screen area is restored.

### Assembly
Assemble `.obj` with
```
tasm /la shepherd.asm
```
Link with
```
tlink /t shepherd.obj
```

### Verification
Assemble `bogus.asm` the same way, and run it. It will load bogus registers to
demonstrate that registers drawn in a register box are correct. Press `ESC`
to exit.

### What it looks like

Register box:

* `AX`, `BX`
* `CX`, `DX`
* `SI`, `DI`
* `BP`, `SP`
* `CS`, `DS`
* `ES`, `SS`
* `IP`, `FL`

Cow:

```
\   ^__^
 \  (oo)\_______
    (__)\  asm  )\/\
        ||----w |
        ||     ||
```

### Usage

Load into memory:

```
.\shepherd.com
```

Then press `F12`:

* first press: show overlay
* second press: hide overlay and restore the saved screen area

### Behavior

* Hooks the `09h` interrupt vector
* Draws directly into text video memory at `B800h`
* Saves and restores the covered screen rectangle

### Notes

* Intended for 80x25 color text mode
* Stays resident after launch
* Trigger key is `F12`
