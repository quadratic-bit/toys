cowsay
------

Render an assembler cow, saying assembler things.
```
    ^__^
    (oo)\_______
    (__)\  asm  )\/\
        ||----w |
        ||     ||
```

### Assembly
Assemble `.obj` with
```
tasm /la cowsay.asm
```
Link with
```
tlink /t cowsay.obj
```

### Usage
To say things, run
```
cowsay.com your message
```
By default, it will say "Moov".

#### Custom borders
By default, `╔═╗║ ║╚═╝` borders are used. To specify your borders, pass with `/b`:
```
cowsay.com /b ┌-┐│ │└-┘ your message in a custom box
```
