# Pixel Repetition — BMP Window & Nearest-Neighbor Zoom

A C17 implementation of a pixel-repetition (nearest-neighbor) image scaler for uncompressed 24-bit BMP files. The program crops a rectangular window from an input bitmap and upscales it by an integer factor, with an optional SSE-optimized variant.

Built as the final project for **TUM GRA — Grundlagenpraktikum: Rechnerarchitektur** (Summer Semester 2023).

## Repository layout

| Path | Contents |
| --- | --- |
| [Implementierung/](Implementierung/) | C sources, `Makefile`, compiled `main` binary, sample BMPs in [Implementierung/Images/](Implementierung/Images/) |
| [Ausarbeitung/](Ausarbeitung/) | LaTeX write-up ([Ausarbeitung.tex](Ausarbeitung/Ausarbeitung.tex)) and rendered [Ausarbeitung.pdf](Ausarbeitung/Ausarbeitung.pdf) |
| [Vortrag/](Vortrag/) | Final presentation slides ([Vortrag.pdf](Vortrag/Vortrag.pdf)) |
| [Aufgabenstellung.pdf](Aufgabenstellung.pdf) | Original task description |

## Algorithm

Nearest-Neighbor upscaling in three passes over the scaled buffer:

1. **Place** each source pixel at its new `(row·f, col·f)` position; fill the rest with zeros.
2. **Expand** each placed pixel into its direct 8-neighborhood.
3. **Resolve** remaining gaps — unambiguous cells take the nearest source pixel; edge/ambiguous cells use a fixed fallback (pixel above on the bottom edge, pixel to the left on the right edge).

Asymptotic runtime for an `n × n` window and scale factor `f`: **O(n² · f²)**.

## Implementation

- [window.c](Implementierung/window.c) — scalar crop; handles both bottom-up and top-down BMPs and preserves 4-byte row stride.
- [window_v1.c](Implementierung/window_v1.c) — SSE2 variant using `_mm_loadu_si128` / `_mm_storeu_si128` to move 16-byte chunks (≈ 5⅓ pixels) per iteration.
- [zoom.c](Implementierung/zoom.c) — the 3-pass nearest-neighbor upscaler (scalar only; per-pixel index arithmetic does not vectorize).
- [main.c](Implementierung/main.c) — argv parsing (`getopt_long`), BMP header read/write, buffer allocation, benchmarking loop.

## Build

```sh
cd Implementierung
make
```

Requires `gcc` with C17 support. Compiles with `-O3 -Wall -Wextra -Wpedantic`. No inline assembly; no instructions beyond SSE4.2.

## Usage

```
./main [options] <input.bmp>
```

| Option | Description | Default |
| --- | --- | --- |
| `-V <n>` | Implementation: `0` scalar, `1` SSE | `0` |
| `-B [n]` | Benchmark mode; `n` repetitions | off / `1` |
| `-s <x>,<y>` | Window origin | `0,0` |
| `-w <n>` | Window width | image width |
| `-h <n>` | Window height | image height |
| `-f <n>` | Integer scale factor | `3` |
| `-o <file>` | Output `.bmp` | `output.bmp` |
| `-h`, `--help` | Print help | — |

### Examples

```sh
# Crop a 200×300 window starting at (170,170) from lena.bmp, upscale 3×
./main -s 170,170 -w 200 -h 300 -f 3 -o lena_zoom.bmp Images/lena.bmp

# Benchmark the SSE variant over 20 runs
./main -V 1 -B 20 -w 512 -h 512 -f 4 Images/lena.bmp
```

## Test images

[Implementierung/Images/](Implementierung/Images/) ships with `lena.bmp`, `johnmuirtrail.bmp`, `blackbuck.bmp`, `BMP.bmp`, `non.bmp`, and `LargeExample.bmp` covering top-down/bottom-up layouts and non-4-byte-aligned row widths.

## Authors

Arda Mat · Barış Özgün · Yiğit Odakır
