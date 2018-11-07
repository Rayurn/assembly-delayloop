# assembly-delayloop

This tool was made to avoid the tedious calculation of assembler delay loops by hand.
It takes a time and clock frequency or a number of clock cycles as input and generates a loop that delays for exactly the
required time.

## Usage

There are several commandline flags available:
```
-c/--cycles     To use a number of clock cycles as input. Don't use together with -t or -f.
-t/--time       To specify which argument is the time, by default it is the left one.
-f/--frequency  To specify which argument is the frequency, by default it is the right one.
-s/--short      Gives only the loop parameters as output. the last element is the number of additional nop instructions.
-r/--register   To choose which registers to use, default is 16.
-h/--help       Display usage, and information on the commandline flags.
```
When using time and frequency, units are required while SI-prefixes are optional. Units for time are seconds(s), minutes(min),
hours(h) and days(d); frequency is in Hz. SI-prefixes are available from femto to Peta (for micro, instead of 'μ' use 'u').

## Building

```
git clone https://github.com/Rayurn/assembly-delayloop.git
cd assembly-delayloop
make
sudo ./install.sh
```
`sudo ./install.sh` is not required, but enables you to run this tool from anywhere on your system. 

## Examples

```
delayloop 500ms 16MHz
```
returns
```
asm volatile (
    "   ldi  r16, 41    \n"
    "   ldi  r17, 150   \n"
    "   ldi  r18, 128   \n"
    "L: dec  r18        \n"
    "   brne L          \n"
    "   dec  r17        \n"
    "   brne L          \n"
    "   dec  r16        \n"
    "   brne L          \n"
);
```

```
delayloop 500ms 16MHz -s
```
returns
```
[41, 150, 128, 0]
```

```
delayloop -c 16000000
```
returns
```
asm volatile (
    "   ldi  r16, 82    \n"
    "   ldi  r17, 44    \n"
    "   ldi  r18, 0 \n"
    "L: dec  r18        \n"
    "   brne L          \n"
    "   dec  r17        \n"
    "   brne L          \n"
    "   dec  r16        \n"
    "   brne L          \n"
    "   nop             \n"
    "   nop             \n"
);
```

```
delayloop 16MHz -t 500ms -r 20
```
returns
```
asm volatile (
    "   ldi  r20, 41    \n"
    "   ldi  r21, 150   \n"
    "   ldi  r22, 128   \n"
    "L: dec  r22        \n"
    "   brne L          \n"
    "   dec  r21        \n"
    "   brne L          \n"
    "   dec  r20        \n"
    "   brne L          \n"
);
```

## License

This project is licensed under the GNU General Public License v3.0 - see the [LICENSE](LICENSE) file for details
