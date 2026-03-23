# Maze Generator & Solver

Proyek Tugas Besar mata kuliah **Algoritma & Pemrograman**. Sebuah visualisasi interaktif pembuatan dan penelusuran labirin menggunakan C++ dan SFML.

---

## Preview

> Labirin dibuat secara prosedural dengan metode **Recursive Backtracking**, lalu ditelusuri otomatis menggunakan algoritma **BFS**. Semua divisualisasikan secara *real-time* di layar.

| Warna | Keterangan |
|---|---|
| 🟥 Merah | Node sedang dikunjungi (*Visiting*) |
| ⬜ Abu-abu | Node sudah dikunjungi (*Visited*) |
| 🟩 Hijau | Jalur solusi (*Path*) |
| ⬛ Hitam | Dinding (*Wall*) |
| 🔲 Gelap | Belum dikunjungi (*Unvisited*) |

---

## Fitur

- **Maze Generation** menggunakan algoritma *Recursive Backtracker* (DFS berbasis stack)
- **Loop/Cycle Addition** — menambahkan jalur acak agar labirin tidak selalu memiliki solusi tunggal
- **Maze Solving** menggunakan algoritma **BFS (Breadth-First Search)** untuk menemukan jalur terpendek
- Visualisasi animasi real-time dengan **SFML**
- Kontrol kecepatan animasi dan fitur pause

---

## Kontrol

| Tombol | Fungsi |
|---|---|
| `Q` | Pause / Resume |
| `↑` Arrow Up | Percepat animasi (kurangi delay) |
| `↓` Arrow Down | Perlambat animasi (tambah delay) |
| `Esc` | Keluar program |
| *Tombol apa saja* saat pause | Lanjutkan ke tahap berikutnya |

---

## Dependensi

- **Compiler:** C++17 atau lebih baru (GCC / MinGW / MSVC)
- **Library:** [SFML 2.x](https://www.sfml-dev.org/) — Simple and Fast Multimedia Library
- **Font:** `ARIAL.TTF` — harus ditempatkan di direktori yang sama dengan executable

> **Catatan font:** Font Arial adalah milik Microsoft dan tidak dapat didistribusikan secara bebas. Kamu bisa mendapatkannya dari:
> - Windows: `C:\Windows\Fonts\arial.ttf`
> - Linux: install paket `ttf-mscorefonts-installer`
> - macOS: tersedia di `/Library/Fonts/Arial.ttf`
>
> Salin file `ARIAL.TTF` ke folder yang sama dengan file `.exe` / binary.

---

## Cara Kompilasi & Menjalankan

### Windows (MinGW + SFML)

```bash
g++ -o maze main.cpp -lsfml-graphics -lsfml-window -lsfml-system
./maze.exe
```

### Linux

```bash
g++ -o maze main.cpp -lsfml-graphics -lsfml-window -lsfml-system
./maze
```

> Pastikan SFML sudah terinstall. Di Ubuntu/Debian:
> ```bash
> sudo apt install libsfml-dev
> ```

---

## Struktur Proyek

```
maze-generator-solver/
├── main.cpp          # Source code utama
├── ARIAL.TTF         # Font (tidak termasuk di repo. Lihat catatan di atas)
├── .gitignore
└── README.md
```

---

## Algoritma yang Digunakan

### 1. Recursive Backtracking (Pembuatan Labirin)
Menggunakan **DFS berbasis stack** — dimulai dari titik awal, memilih arah secara acak, dan "mengukir" jalur hingga semua sel terkunjungi. Menghasilkan labirin *perfect* (setiap sel terhubung, tidak ada loop).

### 2. Loop Addition (Penambahan Siklus)
Setelah labirin perfect dibuat, sejumlah dinding acak dihancurkan untuk membuat *imperfect maze* — labirin dengan beberapa jalur alternatif.

### 3. BFS (Penelusuran Labirin)
**Breadth-First Search** digunakan untuk menemukan **jalur terpendek** dari sudut kiri-atas `(0,0)` ke sudut kanan-bawah `(63,31)`. BFS menjamin solusi optimal pada graf tak berbobot.

---

## Anggota Kelompok

| NIM | Nama |
|---|---|
| 2505551088 | [Ngakan Ketut Kutha Giri Prasetia](https://github.com/24girip) |
| 2505551163 | [Andika Septianantha](https://github.com/andikaseptianantha) |

---

## Lisensi

Proyek ini dibuat untuk keperluan akademis. Bebas digunakan sebagai referensi pembelajaran.
