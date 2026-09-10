# Audio Mixer Desktop — Panduan Implementasi untuk AI Coding Agent

Versi spesifikasi: 1.7 — Pemrosesan audio wajib native, tanpa Web Audio  
Tanggal: 10 September 2026  
Bahasa penjelasan: Indonesia; identifier kode: English  
Status: spesifikasi pembangunan, bukan aplikasi yang sudah diimplementasikan atau diuji.

Revisi 1.7 mempertegas kontrak **native-only audio** pada bagian 3.2A. Seluruh capture, playback, mixing, DSP, monitoring, recording dan export berjalan pada engine/proses media native lokal. React/Electron hanya UI dan control IPC; tidak ada Web Audio processing atau fallback audio browser. Urutan 49 fase dan layout tidak berubah.

Revisi 1.6 merupakan audit konsistensi dan kelengkapan. **Bagian 8E** menetapkan urutan eksekusi tunggal, hubungan UI–engine, perilaku monitoring/transport/recording, serta empat fase integrasi akhir. Layout compact bagian 8D, 99 program, Harmony, UI-first dan batas kode 1.000 baris tetap berlaku. Revisi spesifikasi bukan bukti bahwa software sudah dibangun/diuji.

Revisi 1.5 menambahkan aturan wajib **modular UI dan native engine**, reusable control components serta batas **maksimal 1.000 baris fisik per file kode first-party**. Bagian **4A** berlaku sejak UI-00 dan ditegakkan otomatis pada verifikasi/CI. Target file jauh lebih kecil; jangan menunggu 1.000 baris untuk memecah tanggung jawab.

Revisi 1.4 memperkenalkan UI-first; urutan dependensi final sekarang 8E.1: **UI-00–04 terlebih dahulu**, mengikuti layout compact terakhir, lalu Phase 00–19 dan fase DSP terkait. Bagian **8D** menjadi acuan layout; bagian **8E.1** menetapkan urutan kerja final. Data simulasi dibolehkan hanya pada mode UI Preview yang ditandai jelas. Arahan lama engine-first dan kolom DSP besar di kanan tidak berlaku.

Revisi 1.3 mewajibkan tombol HARMONY ON/OFF pada strip channel vokal di Mixer utama, pengaturan key/scale/level melalui panel cepat, serta binding ke satu instance harmony native. Bagian 8C dan HARM-00–03 adalah pekerjaan wajib sebelum release; toggle visual tanpa DSP tidak memenuhi syarat.

Revisi 1.2 menambahkan dua unit FX A/FX B dengan bank 99 program yang dapat dipilih langsung pada Mixer utama, FX send A/B per channel, return independen, dan akses cepat insert melalui tombol FX. Bagian 8B tetap menjadi acuan bank program; layout terbarunya ditentukan bagian 8D; Vocal FX tetap workspace detail opsional.

Revisi 1.1 menambahkan Vocal FX rack native lengkap pada bagian 8A: pitch correction, pitch/formant shift, harmony, doubler, modulation, saturation, vocoder, preset, dan pengujian audio. Revisi ini menggantikan pengecualian pitch correction pada versi 1.0. Baca urutan eksekusi tunggal 8E.1 sebelum melanjutkan proyek existing.

## 1. Instruksi mulai untuk AI

Bangun aplikasi desktop audio mixer berdasarkan seluruh dokumen ini. Pengguna meminta **audio sistem dan mixer file multitrack**, dengan kontrol umum mixer hardware, termasuk pengaturan suara/vokal dalam dB. Kerjakan bertahap sampai fitur target selesai; jangan berhenti pada UI mockup atau MVP lalu menganggap seluruh proyek selesai.

Target awal adalah MacBook Pro M2, RAM 16 GB, macOS Apple Silicon. Produk berjalan lokal/offline; tidak ada layanan cloud audio, upload rekaman, atau remote backend. Koneksi internet hanya dibutuhkan saat mengambil dependency pengembangan. Nama kerja aplikasi: **Local Audio Mixer**; nama ini boleh diubah tanpa mengubah arsitektur.

Mulai dengan **UI-00 sampai UI-04 (bagian 8D)**: bangun layout dan interaksi tampilan lebih dahulu, verifikasi kesesuaiannya dengan mockup, kemudian lanjutkan engine. Jangan memulai pembangunan native engine/DSP sebelum gate UI terpenuhi. Baca instruksi repository yang sudah ada sebelum mengubah file. Dokumen ini merupakan sumber spesifikasi produk, bukan izin menghapus proyek lama atau mengubah pengaturan OS secara diam-diam.

### 1.1 Prosedur setiap tahap

1. Baca status fase sebelumnya, kode terkait, dan acceptance criteria.
2. Buat daftar perubahan konkret sesuai ruang lingkup fase aktif; tentukan owner module, public API dan reusable component yang dipakai (bagian 4A).
3. Implementasikan kode produksi beserta pengujian perilaku yang berisiko.
4. Jalankan build dan pemeriksaan yang relevan, termasuk `check:file-size` dan `check:architecture` setelah script tersedia; file >1.000 baris wajib dipecah sebelum fase dinyatakan selesai.
5. Perbaiki kegagalan sebelum melanjutkan fase yang bergantung padanya.
6. Catat hasil nyata di `docs/progress.md`, termasuk command, exit code, dan keterbatasan hardware.
7. Lanjutkan fase berikutnya menurut dependency plan 8E.1 jika tidak ada blocker; nomor Phase bukan satu-satunya urutan (Phase05 mendahului Phase04). Jangan meminta konfirmasi untuk pilihan kode rutin.
8. Jika terputus karena batas konteks, simpan checkpoint yang menjelaskan langkah selanjutnya secara presisi.

Dilarang menandai PASS untuk uji pendengaran, permission macOS, atau pengujian perangkat hanya karena unit test lulus. Jika lingkungan AI Linux, kerjakan bagian portable dan tandai pengujian Mac sebagai `BLOCKED_ENVIRONMENT`; jangan menyimulasikan hasil Core Audio.

### 1.2 Arti status pekerjaan

| Status | Makna |
| --- | --- |
| NOT_STARTED | Belum dikerjakan |
| IN_PROGRESS | Implementasi aktif |
| IMPLEMENTED_UNVERIFIED | Kode tersedia, validasi yang diwajibkan belum selesai |
| BLOCKED_ENVIRONMENT | Memerlukan OS/perangkat/izin yang tidak tersedia |
| FAILED | Validasi gagal dan belum diperbaiki |
| CAPABILITY_UNAVAILABLE | Fitur bersyarat telah diperiksa pada target dan memang tidak didukung; bukan kelulusan fitur wajib |
| UI_VERIFIED | Layout/interaksi lolos; engine audio belum dinyatakan berjalan |
| VERIFIED | Semua acceptance criteria fase terkait terpenuhi dengan bukti; sebutkan domain UI/engine/hardware |

Pada tahap UI-00–04, tombol boleh mengubah simulated state melalui PreviewAdapter untuk memvalidasi layout dan interaksi. Wajib tampilkan `UI PREVIEW · Audio engine belum terhubung`; jangan menyatakan suara sedang diproses atau file audio telah direkam. Pada mode Engine, tombol yang belum terintegrasi harus disabled dengan alasan. UI_VERIFIED bukan DSP VERIFIED.

## 2. Cakupan dan batas produk

### 2.1 Tiga mode penggunaan

- **System mixer:** suara browser, YouTube, QuickTime, dan aplikasi yang mengikuti routing OS masuk ke channel sistem, diproses, kemudian keluar ke output fisik.
- **Studio mixer:** beberapa file audio dan mikrofon/instrument input masuk ke channel terpisah; tersedia transport, recording, editing clip dasar, dan export mix/stems.
- **Hybrid:** playback file, audio sistem, dan mikrofon diproses bersamaan oleh satu engine dan satu clock render.

System mixer awal mengolah satu gabungan stereo dari aplikasi yang diarahkan ke virtual device. Itu **belum berarti kontrol volume terpisah per aplikasi**. Kontrol per aplikasi adalah deliverable lanjutan dengan capture backend tersendiri.

### 2.2 Definisi fitur lengkap untuk versi 1

| Area | Fitur wajib | Tahap utama |
| --- | --- | --- |
| Layout dahulu | Compact FX A/B, mixer, Channel Processing, Harmony, simulasi UI dan visual QA | UI-00–04 |
| Input | File mono/stereo, system stereo, mic/line, selector channel fisik | 03–06 |
| Channel strip | Trim, polarity, HPF/LPF, EQ 4 band, gate/expander, compressor, de-esser, fader, pan/balance, mute, solo, stereo link | 07–09 |
| Meter | Input/output peak, RMS, clip hold, gain reduction, master loudness/true peak | 07, 14 |
| Routing | Main stereo, 4 stereo subgroups, 4 stereo aux monitor, 2 dedicated FX send buses A/B + 2 stereo returns A/B, 4 DCA groups | 10, MIXFX-01 |
| Monitoring | PFL/AFL, monitor volume/mute/dim, physical output assignment | 10 |
| Effects | Room/Plate/Hall reverb, Slapback/Stereo/Ping-pong delay, ducking | 11, VFX-02 |
| Harmony satu tombol | HARMONY ON/OFF pada channel vokal, key/scale, interval, level, native engine binding | HARM-00–03 |
| DSP pada Mixer utama | Dua unit FX A/B, bank 99 program wet-only, quick parameters, per-channel send, independent return | MIXFX-00–05 |
| Vocal FX rack | Native effect catalog, insert ordering, bypass, wet/dry, A/B, presets | VFX-00–09 |
| Voice character | Chorus, doubler, pitch/formant shift, pitch correction, harmony, saturation, flanger/phaser, vocoder/robot | VFX-02–07 |
| Transport | Play/pause/stop/seek, loop, markers, metronome, count-in | 06, 12 |
| Editing | Move, trim non-destructive, split, fade in/out, clip gain, undo/redo | 12 |
| Recording | Master dan multitrack, pilihan dry/processed, arm dan input monitor terpisah | 13 |
| Export | Offline mixdown, stems, WAV 24-bit/32-bit float, FLAC; MP3 jika encoder valid | 14 |
| Session | Save/open, autosave, recovery, collect media, missing-media relink | 15 |
| Automation | Volume, pan, mute, send; read/touch/latch/write; MIDI learn | 16 |
| Integrasi | AU/VST3 hosting, isolated scan, state save, latency compensation | 17 |
| System lanjutan | Per-app capture jika API/perangkat mendukung, routing recovery | 18 |
| Delivery | App arm64, installer development, release signing/notarization bila tersedia | 19 |

Kapasitas desain versi 1: **32 input strips** yang berbagi pool untuk file, system, mic, dan app capture. Channel stereo dihitung satu strip, tetapi memakai dua kanal sample. Bus/FX return tidak mengambil slot input strip. Target kapasitas harus dibenchmark; bukan janji semua plugin dan 32 channel selalu berjalan dengan buffer paling kecil.

### 2.3 Batas yang harus jujur di UI

- Digital trim bukan analog preamp gain. Hardware clipping sebelum ADC tidak dapat diperbaiki dengan menurunkan fader.
- Phantom power 48V, pad analog, Hi-Z, preamp remote, dan hardware direct monitoring hanya dapat dikontrol jika interface menyediakan API resmi. Jangan buat toggle palsu.
- dBFS adalah level digital; dB SPL adalah tekanan suara fisik. Jangan tampilkan angka SPL tanpa kalibrasi/perangkat ukur.
- Mikrofon dan musik dalam satu file tidak otomatis terpisah. Fader vocal hanya mengendalikan channel vokal terpisah. Stem separation dan AI denoise bukan target versi 1. Pitch correction dan harmony monofonik menjadi target wajib revisi 1.1; keduanya membutuhkan sumber vokal terisolasi untuk hasil yang dapat diandalkan.
- Tidak semua app mengikuti default output; app dengan pilihan device sendiri perlu diarahkan secara eksplisit.
- Audio terproteksi atau capture yang dibatasi OS tidak dijamin dapat ditangkap.
- Satu output stereo tidak menyediakan beberapa headphone mix independen. Multi-monitor fisik membutuhkan jumlah output interface yang cukup.
- Bluetooth dapat memiliki latency tinggi dan perubahan mode/rate. Jalur benchmark low-latency memakai kabel atau interface USB.
- Plugin pihak ketiga dapat crash, mengalokasikan memori, atau menambah latency. Built-in DSP harus tetap dapat berjalan tanpa plugin.

## 3. Arsitektur yang dipilih

### 3.1 Stack

| Lapisan | Pilihan | Tanggung jawab |
| --- | --- | --- |
| Desktop shell | Electron, versi stabil kompatibel yang dipin | Window, lifecycle, dialog, native process supervisor |
| UI | React + TypeScript + Vite; CSS/Tailwind | Mixer strips, timeline, inspector, settings |
| Engine | C++20 + JUCE 8, exact tag/commit dipin | Audio devices, DSP, mixing, playback, recording, offline render |
| macOS adapter | Objective-C++ + Core Audio | Device UID, routing OS, taps backend, permission integration |
| Build native | CMake + Ninja + Apple Clang | Engine, tests, plugin scanner |
| Control IPC | Child process stdin/stdout, JSON Lines | Command, ACK, snapshot, throttled telemetry |
| Media conversion | JUCE readers; FFmpeg executable bila dibutuhkan | Decode/encode worker di luar callback |
| Storage | JSON schema versioned + file media lokal | Session, presets, preferences, recovery |
| Tests | Native CTest harness, Vitest, Playwright Electron | DSP, contracts, UI, integration |

Node development ditargetkan 24.15.0 sesuai lingkungan pengguna; Phase 00 harus mengecek kecocokan dependency. Electron membawa runtime Node sendiri, sehingga jangan menyamakan kedua versi tersebut. Python tidak menjadi engine realtime proyek ini; Python opsional hanya untuk membuat fixture/analisis pengujian.

JUCE dipilih untuk mengurangi pembangunan ulang infrastruktur audio native. Ini keputusan arsitektur proyek, bukan klaim bahwa seluruh fitur mixer tersedia otomatis dari JUCE. Periksa lisensi dependency yang dipin dan catat kebutuhan distribusinya sebelum release; jangan menganggap JUCE bebas kewajiban untuk semua model distribusi. [JUCE licence](https://juce.com/legal/juce-8-licence/).

### 3.2 Pemisahan control plane dan audio plane

- React mengirim parameter, bukan PCM.
- Electron main memvalidasi command dan berkomunikasi dengan engine executable.
- Engine memiliki satu authoritative session state; renderer memegang snapshot dan pending UI edits.
- DSP dan routing berjalan di native callback; playback/recording menggunakan worker dan buffer yang dialokasikan lebih dahulu.
- PCM tidak dikirim melalui JSON, IPC renderer, REST, WebSocket, atau Python.
- Audio engine tidak membuka port network. Stdout khusus protocol; log engine ke stderr. Development Vite UI boleh memakai localhost untuk HMR/visual QA; itu bukan audio backend. Packaged app memakai assets lokal dan tidak membutuhkan server Vite.
- UI freeze atau window minimize tidak boleh menghentikan suara.

Device manager JUCE menyediakan pengelolaan konfigurasi audio device dan callback; pemetaan kanal, recovery, dan aturan routing aplikasi tetap harus dibuat sendiri. [JUCE AudioDeviceManager](https://docs.juce.com/master/classjuce_1_1AudioDeviceManager.html).

### 3.2A Kontrak wajib: native-only audio

Ketentuan ini berlaku pada implementasi produksi, development, prototype dan fallback. UI-first berarti membangun tampilan dengan data simulasi yang ditandai Preview, bukan membuat engine audio sementara di browser.

- Seluruh capture/input, decode, playback, resampling, gain/pan, EQ, dynamics, DSP 99 program, FX A/B, Harmony, routing, metering audio, monitoring, recording dan offline export wajib dijalankan oleh engine C++20/JUCE/Core Audio atau native media worker yang ditetapkan. Decode/encode native boleh memakai FFmpeg lokal di luar callback; tidak melalui renderer.
- Dilarang menggunakan `AudioContext`, `webkitAudioContext`, `OfflineAudioContext`, `AudioWorklet`, `ScriptProcessorNode`, `getUserMedia`/`getDisplayMedia` untuk capture audio, `MediaRecorder`, atau elemen `<audio>`/`HTMLAudioElement` sebagai jalur audio mixer. Larangan juga mencakup library pembungkus Web Audio serta DSP WebAssembly yang dijalankan di browser/renderer.
- JavaScript/TypeScript di Electron main, preload, renderer maupun Web Worker tidak memproses buffer sampel audio. Worker frontend boleh mengolah data presentasi; audio decode, FFT/spectrum dan perhitungan level aktual menjadi tanggung jawab native worker/engine.
- UI boleh menggambar respons EQ dari parameter dan menampilkan meter, spectrum bins atau waveform min/max yang sudah diringkas oleh engine. Data visual ini tidak menjadi jalur playback atau pemrosesan suara; PCM mentah tidak dikirim ke renderer.
- Jika executable native gagal start/crash/tidak tersedia, tampilkan status engine unavailable dengan aksi recovery. Jangan beralih diam-diam ke browser audio. Mode UI Preview tetap tanpa output audio, file rekaman atau export palsu.
- Tambahkan aturan ini ke `AGENTS.md`, `.github/copilot-instructions.md` dan `check:architecture`. Guard memeriksa penggunaan API/import audio browser dan dependency pembungkus dalam source runtime first-party; referensi nama API dalam dokumentasi/fixture pemeriksaan tidak dihitung sebagai pelanggaran. Jangan hanya mengandalkan pencarian nama: review dependency dan jalur eksekusi untuk menghindari wrapper/alias yang lolos pemeriksaan.
- Acceptance native: buktikan playback/capture/DSP/output melalui proses native pada paket aplikasi; UI minimize atau renderer tidak responsif tidak menghentikan callback audio. Pengujian engine unavailable harus membuktikan tidak ada fallback Web Audio. `PRODUCT_VERIFIED` gagal bila salah satu fitur audio wajib ternyata diproses browser, meskipun suaranya terdengar benar.

### 3.3 Thread dan ownership

| Thread | Boleh dilakukan | Tidak boleh dilakukan |
| --- | --- | --- |
| Audio callback | DSP, buffer teralokasi, atomic scalar, konsumsi bounded event queue | malloc/new/free, mutex menunggu, disk/network, JSON, log, UI call |
| Engine control | Parse protocol, validasi, bangun graph/preset, device start/stop | Menulis langsung object DSP yang sedang diproses |
| Media workers | Decode, resample file, waveform cache, record writer, export | Mengubah graph aktif tanpa transaksi |
| Telemetry worker | Baca meter snapshot, serialize event | Membuat callback menunggu stdout |
| Electron main | Supervise engine, dialogs, IPC validation | DSP sample processing |
| Renderer | Render UI dan input user | Akses shell/filesystem bebas atau engine internals |

Graph edit: bangun graph baru di control thread, preallocate/prepare, lalu publish pada boundary block. Reclaim graph lama di control thread setelah acknowledgement audio thread. Jangan membiarkan destructor `shared_ptr` atau object besar berjalan pada callback. Single-producer queue untuk command disuplai hanya control thread; sumber command lain harus diserialisasi terlebih dahulu.

## 4. Struktur repository dan aturan kepemilikan

Buat struktur berikut. Jangan mengisi puluhan file kosong terlebih dahulu; buat saat fasenya tiba.

| Path | Isi |
| --- | --- |
| `README.md` | Setup, quick start, supported/unsupported capabilities |
| `AGENTS.md` | Aturan eksekusi dan batas realtime dari dokumen ini |
| `.github/copilot-instructions.md` | Ringkasan instruksi AI dengan rujukan spesifikasi |
| `docs/specs/AUDIO-MIXER-AI-IMPLEMENTATION.md` | Salinan spesifikasi ini |
| `docs/progress.md` | Status fase dan bukti validasi |
| `docs/decisions/` | ADR stack, routing, clock, plugin isolation |
| `docs/reports/` | Benchmark, hardware matrix, permission/package test |
| `apps/desktop/electron/` | Main, preload, engine supervisor, IPC |
| `apps/desktop/src/` | React feature folders dan UI state |
| `packages/contracts/` | Protocol schema, units, errors, fixtures |
| `native/engine/src/core/` | Engine state, graph, scheduler, parameter bridge |
| `native/engine/src/devices/` | Device abstraction dan channel maps |
| `native/engine/src/platform/macos/` | Core Audio, routing, taps, permissions |
| `native/engine/src/dsp/` | Channel/bus processors dan metering |
| `native/engine/src/media/` | Readers, stream cache, recording, export |
| `native/engine/src/session/` | Serialization, migration, command history |
| `native/plugin-scanner/` | Out-of-process plugin discovery |
| `native/tests/` | DSP, routing, protocol, offline tests |
| `tests/e2e/` | Electron journeys dengan fake dan real engine |
| `tests/fixtures/` | Generator dan fixture kecil berlisensi jelas |
| `scripts/` | Doctor, native build, verify, packaging |
| `third_party/` | Dependency pin/manifests, bukan download acak |

Engine tidak bergantung pada React atau Electron. Renderer tidak mengimpor native headers. Kontrak IPC tidak boleh mengimpor komponen UI. File platform macOS harus bisa dikecualikan dari portable DSP test build. Boleh memecah spesifikasi menjadi beberapa MD, tetapi pertahankan satu indeks, feature matrix, dan sumber parameter yang konsisten.


## 4A. Aturan wajib modularisasi dan ukuran file — revisi 1.5

### 4A.1 Prinsip dan batas tegas

Seluruh kode yang dibuat untuk proyek ini harus modular, dengan satu tanggung jawab utama per module. UI dibuat dari komponen kecil yang dapat digunakan ulang; native engine juga dipisahkan menurut fungsi dan ownership/thread. **Batas keras: tidak ada first-party code file yang melebihi 1.000 baris.** Tepat 1.000 masih memenuhi batas; 1.001 gagal. Target normal jauh lebih kecil.

“Komponen terkecil” berarti unit bermakna yang mandiri dan reusable, bukan memecah setiap `<span>` menjadi file. Ekstrak ketika mempunyai perilaku/aksesibilitas/style sendiri, dipakai ulang, atau memisahkan tanggung jawab yang jelas. Jangan membuat hierarchy wrapper kosong, generic component puluhan boolean, atau satu `AudioControl` raksasa untuk semua jenis kontrol.

| Jenis file | Target biasa, bukan batas minimum | Arah pemisahan |
| --- | --- | --- |
| UI primitive | 30–150 baris | Satu kontrol visual sederhana |
| Audio control | 60–250 baris | Satu pola interaksi/visual, unit terpisah |
| Composite/panel | 100–300 baris | Komposisi controls, sedikit presentation logic |
| Page/layout | 80–250 baris | Arrangement/layout, tanpa domain algorithm |
| Hook/store/adapter handler | 50–250 baris | Satu domain state atau lifecycle |
| Native header | 40–200 baris | API/contracts/ownership; minimal implementation |
| Native implementation | 100–400 baris | Satu service/processor atau cohesive algorithm stage |
| Test file | 80–350 baris | Satu module atau satu behavior family |

Ukuran bukan satu-satunya ukuran kualitas: file 300 baris dengan routing, DSP, JSON dan file I/O tetap harus dipisah. Jangan menunggu menyentuh batas keras. Pada >600 baris tinjau kemungkinan pemisahan; pada ≥800 line-count checker memberikan warning; pada >1.000 error dan exit nonzero.

### 4A.2 Definisi line-count dan cakupan

Hitung **baris fisik seluruh file**, termasuk baris kosong, komentar, imports dan embedded templates. File kosong=0; CRLF dianggap satu pemisah baris; final newline tidak menambah phantom line. Gunakan aturan setara `text.splitlines()` yang terdokumentasi dan tests boundary.

Berlaku untuk kode first-party authored maupun first-party generated yang dibundel sebagai sumber: TS/TSX/JS/JSX/MJS/CJS/CTS/MTS, C/C++/headers/HPP/HH/HXX/CC/CXX/IPP/TPP/inl, Objective-C/Objective-C++ (`.m`/`.mm`), Swift/Rust/Go/Python jika dipakai, shell, CSS/SCSS/Less, HTML/Vue/Svelte, SQL, CMakeLists.txt/`.cmake`, source build configs dan test/support scripts. Inline shaders/source templates juga tidak menjadi pengecualian. File kode tanpa ekstensi dalam `scripts/` ikut diperiksa berdasarkan shebang atau allowlisted source-path pattern.

Bukan kode aplikasi: Markdown/docs, gambar/audio/binary assets, lockfile otomatis, static program JSON data, serta build artifacts dan dependency eksternal unmodified. Batas 1.000 tidak diterapkan pada **dokumen spesifikasi MD ini**. Pengecualian untuk dependency eksternal tidak boleh dipakai menyembunyikan kode proyek: folder `third_party/` hanya boleh dikecualikan jika manifest mencatat origin/version/hash sebagai dependency upstream. Kode wrapper/modifikasi proyek tetap diperiksa. Generated project source harus dipecah oleh generator bila terlalu panjang; compiler output di build directory bukan source deliverable.

Bank 99 presets dapat disimpan sebagai data JSON berversi atau per-family files dengan index; jangan embed ribuan baris literal presets ke React component/C++ source. JSON schema/config first-party ikut quality review dan dianjurkan dipecah bila kompleks, meskipun static data bukan code-line limit.

Dilarang mengakali batas dengan menggabungkan banyak statement satu baris, minify source, menghapus komentar yang berguna, ekstensi palsu, `#include` fragmen `.inc` untuk melanjutkan satu class raksasa, atau `part1`/`part2` tanpa module API. Pemecahan harus menghasilkan batas tanggung jawab yang nyata.

### 4A.3 Hirarki UI reusable

Urutan dependensi: **page → feature panel/composite → audio control → UI primitive + pure utility**. Store/adapters/hooks disuplai melalui container/hooks tingkat feature; leaf controls menerima props/events dan tidak mengetahui device/session/native IPC.

| Layer | Contoh | Boleh mengetahui |
| --- | --- | --- |
| Primitives | Button, IconButton, ToggleButton, Label, Tooltip, Badge, Popover, Select, SearchField | Theme, disabled/focus state, accessibility |
| Audio controls | RotaryKnob, HorizontalSlider, VerticalFader, NumericParameter, LevelMeter, ClipIndicator, PanControl | Value/range/unit, interaction, display |
| Composites | LabeledKnob, SendControl, MeterWithClip, EqBandControls, FxProgramSelector, HarmonyVoiceControls | Kombinasi controls dan domain parameter mapping |
| Feature panels | ChannelStrip, CompactFxRow, ChannelProcessingPanel, ParametricEqPanel, CompressorPanel, HarmonyQuickPanel | Channel/unit ID, feature hooks, commands |
| Pages | MixerPage, VocalFxPage, TimelinePage, RoutingPage | Grid regions, selected entity/navigation |

Shared controls dibangun pada UI-01/UI-02 sebelum merakit panel. Nama boleh diselaraskan dengan repo existing; **jangan membuat `Knob`, `RotaryControl`, dan `RotaryKnob` berisi implementation duplikat**. Pilih satu nama canonical dan gunakan wrapper domain tipis bila perlu.

Inventory minimum:

- `RotaryKnob`: pointer capture/drag, keyboard, fine adjustment, min/max/step/default, disabled/pending, accessible slider role. Tidak memanggil engine.
- `HorizontalSlider`: dipakai untuk return, monitor dan Harmony Level melalui wrapper parameter; perilaku drag/keyboard tidak disalin.
- `VerticalFader`: audio taper/dB mapping via pure utility, ticks, numeric readout, explicit off state; bukan copy HorizontalSlider penuh. Shared hook gesture dan value-mapping digunakan bila tepat.
- `LevelMeter`: mono/stereo display, peak-hold mark, floor range, optional reduced-motion/static preview; hanya menerima level snapshot.
- `ClipIndicator`: latched state dan reset event; tidak menghitung clip di renderer ketika memakai native telemetry.
- `NumericParameter`: typed label/value/unit, edit/parse/validate/commit/cancel; formatting/parsing reusable untuk Hz, kHz, dB, ms, s, Q.
- `ToggleButton`: basis M/S/MON/REC/HARMONY dengan visual variant dan semantic label; jangan meng-hardcode state audio ke primitive.
- `EqResponseGraph`: visual graph/drag interaction; frequency-response math di pure module terpisah, dan engine DSP tetap modul native.
- `ProgramPicker`: search/filter/select registry; tidak mengandung 99 expanded preset objects inline.
- `SelectField`: Key/Scale/interval/output choice; source data diberikan lewat props.

Setiap control mendukung controlled value, typed callback, label/description dan disabled state. `onChange` memberi editing value; `onCommit`/gesture begin/end opsional untuk transaction/undo. Parent memutuskan apakah perubahan dikirim ke PreviewAdapter atau NativeAdapter. Tidak ada `window.api`, localStorage, global store import atau command string native di leaf controls.

### 4A.4 Pemisahan state, rendering, hooks dan styles

- `MixerPage.tsx` hanya menyusun header, compact rows, channel bank, processing inspector, Harmony tray dan footer. Jangan taruh engine lifecycle, registry data, filter math, IPC parser atau session reducers di file tersebut.
- `ChannelStrip.tsx` merakit section kecil: ChannelHeader, InputControls, ProcessingShortcuts, HarmonyShortcut, SendControls, FaderSection, ChannelActionButtons. Shared sections bisa dipakai Master/Group dengan explicit capability config; jangan memaksakan semua feature lewat puluhan boolean props.
- `CompactFxRow.tsx` memakai ProgramPicker, ParameterQuickControls, ReturnControl, LevelMeter dan status badges; registry loading/state di `useFxUnit`.
- `ParametricEqPanel.tsx` memakai EqResponseGraph dan EqBandControls. Drag mapping/frequency response/units di modul math/parameter sendiri. Jangan mencampur React lifecycle dengan DSP backend.
- `useChannelControls`, `useSelectedChannel`, `useFxUnit`, `useChannelHarmony` dipisah menurut domain. Satu universal `useMixerEverything` yang tumbuh besar dilarang.
- PreviewAdapter menyusun modules channel/fx/harmony/transport; fixture builders dan telemetry simulator terpisah. NativeAdapter menyusun transport/request tracking/domain clients; jangan satu adapter file menampung semua methods, parsing, store dan retries.
- State reducers/selectors dibagi channel, fx units, selection, transport, session. Shared authority tetap satu; pemecahan files bukan membuat dua source of truth.
- CSS memakai tokens global kecil dan scoped component/feature styles. Hindari satu `styles.css` >1.000 baris. Jangan membuat inline style object besar untuk semua regions di page.
- Icons memakai reusable SVG/Icon component atau library dipin; tidak menyisipkan SVG path panjang berulang di setiap control.

### 4A.5 Struktur UI yang disarankan

| Folder | Isi |
| --- | --- |
| `src/components/ui/` | Generic Button/Toggle/Badge/Popover/Select/Tooltip |
| `src/components/audio/rotary-knob/` | RotaryKnob.tsx, interaction hook bila bermakna, styles, tests |
| `src/components/audio/slider/` | HorizontalSlider dan shared slider gesture helpers |
| `src/components/audio/fader/` | VerticalFader, FaderScale, styles |
| `src/components/audio/meter/` | LevelMeter, MeterBar, PeakHoldMark, ClipIndicator |
| `src/components/audio/parameter/` | NumericParameter, ParameterLabel, formatting helpers |
| `src/features/mixer/components/` | ChannelBank, ChannelStrip dan section composites |
| `src/features/mixer/hooks/` | Selection dan channel command hooks |
| `src/features/processing/components/` | EQ/compressor/de-esser panels |
| `src/features/processing/math/` | UI frequency-response/axis helpers, bukan engine DSP |
| `src/features/fx/components/` | CompactFxRow, ProgramPicker, ReturnControl |
| `src/features/harmony/components/` | Shortcut/quick panel atau reuse folder feature existing |
| `src/state/` | Domain slices/selectors dan shared store assembly |
| `src/adapters/preview/` | Preview entrypoint dan domain command handlers |
| `src/adapters/native/` | Native entrypoint, request transport dan domain clients |
| `src/styles/` | Tokens, global reset, application layout rules |

Gunakan satu canonical file untuk tiap component. Jika folder existing berbeda, refactor/move imports dengan aman, bukan menyimpan old/new implementation sekaligus. Public feature entrypoint kecil dan explicit; hindari circular barrel re-exports. Leaf controls tidak mengimpor feature index.

### 4A.6 Modularisasi native engine

C++ realtime engine juga tunduk pada batas yang sama, termasuk `.cpp`, `.h`, `.mm`, templates dan tests. Pisahkan API header dari implementation; gunakan module-owned types, forward declaration ketika tepat, public headers minimal. Jangan membangun `AudioEngine.cpp` atau `AllEffects.cpp` yang menampung semua fungsi.

| Module | Tanggung jawab | Contoh file terpisah |
| --- | --- | --- |
| Bootstrap | Wiring lifecycle dependencies | EngineBootstrap, EngineLifecycle |
| Device | Enumerasi/konfigurasi/mapping | DeviceService, DeviceCatalog, ChannelMap |
| macOS platform | API Core Audio/permissions/routing | CoreAudioDevices.mm, SystemRouteController.mm, AudioTapCapture.mm |
| Graph | Validation/build/publication | GraphValidator, GraphBuilder, GraphPublisher, GraphRetirementQueue |
| Render | Callback dan prepared graph processing | AudioRenderCallback, RenderContext, PreparedGraph |
| Parameters | Metadata/queue/smoothing | ParameterRegistry, ParameterCommandQueue, GainSmoother |
| DSP strip | Satu processor per file/pasangan | GainProcessor, EqProcessor, GateProcessor, CompressorProcessor |
| FX | Satu effect dan adapter per module | ReverbEffect, DelayEffect, PitchBackendAdapter, HarmonyEffect, VocoderEffect |
| Pitch internals | Tahap analisis/generation yang cohesive | PitchDetector, ScaleMapper, HarmonyVoice, VoicingGate |
| Metering | Accumulator dan telemetry bridge | PeakMeter, RmsMeter, MeterSnapshotBridge |
| Media | Read/decode/cache terpisah writer | MediaReader, DecodeWorker, PlaybackBuffer, RecordingWriter |
| Session | Model/serialization/migration | SessionModel, SessionSerializer, SessionMigrationV2 |
| IPC | Framing/router/domain handlers | JsonLineReader, RequestRouter, FxCommandHandler, HarmonyCommandHandler |
| Export | Graph snapshot/render job/encoder | OfflineRenderJob, ExportWriter, ExportProgress |

AudioRenderCallback hanya mengakses prepared resources/control queue/DSP; tidak memanggil JSON serializer atau DeviceService mutation. RequestRouter mendelegasikan domain handlers; tidak menjadi switch ribuan baris. HarmonyEffect memakai PitchDetector/ScaleMapper/HarmonyVoice modules tanpa menggabungkan ulang semuanya di header.

Module interface tidak boleh menambah biaya yang merusak realtime: dependency injection/construction dilakukan di control thread; hindari allocation, mutex, unbounded queue, per-sample message dispatch atau virtual abstraction berlapis yang tidak diperlukan. Process block API boleh virtual untuk rack sesuai kontrak sebelumnya; **jangan** memecah setiap sample operation menjadi service call lintas module/thread.

Parameter state ownership, thread ownership dan buffer lifetime ditulis singkat pada public interface. Graph retirement masih off callback. Memecah file bukan alasan menggunakan shared mutable singleton lintas engine modules.

### 4A.7 Dependency rules dan pemeriksaan otomatis

UI lint/import rules minimal:

1. `components/ui` tidak mengimpor `features`, `state`, `adapters`, atau Electron.
2. `components/audio` hanya mengimpor UI primitives, pure shared math/units dan typed display contracts.
3. Feature panel tidak mengimpor internal file feature lain secara acak; gunakan public contract/component API.
4. Renderer tidak mengimpor Electron main/native filesystem. Main/preload tidak mengimpor React feature modules.
5. `packages/contracts` tidak mengimpor UI, Electron implementation atau native engine implementation.
6. Import cycle gagal `check:architecture`.

Native rules ditegakkan melalui CMake target dependency dan include audit: `dsp` tidak bergantung pada `ipc`, UI/Electron, `session` JSON atau macOS device control; `platform/macos` berada di adapter/device boundary; `ipc` boleh memakai application-service interfaces, bukan mutation pointer ke callback-owned processor. Pure DSP tests build tanpa Electron/macOS frameworks jika modulnya portable.

Buat scripts `npm run check:file-size` dan `npm run check:architecture` sejak UI-01. Tambahkan ke `verify:ui`, `verify`, dan CI. Guard dapat memakai Node standard library; jangan menambah dependency besar hanya untuk menghitung baris. CI tidak boleh skip check karena Mac/audio device tidak tersedia.

Line-count scanner harus:

- Memindai source tracked dan untracked non-ignored agar file baru diperiksa sebelum commit. Repo tanpa git tetap memiliki directory scan fallback.
- Mengecualikan hanya known build/dependency/binary paths; jangan skip seluruh `native`, `tests`, `scripts`, `generated-source` atau arbitrary folder.
- Mencetak path/count/status untuk warning ≥800 dan error >1.000; exit1 jika ada error.
- Menggunakan satu policy config checked-in dengan extension/source-path patterns dan explicit upstream exclusions; perubahan konfigurasi terlihat dalam diff.
- Menangani CRLF, UTF-8, empty file dan no-final-newline secara konsisten; tidak mengikuti symlink ke dependency tree tanpa kontrol.
- Gagal jelas pada first-party source file yang tidak terbaca; bukan silently skipping.

Tests scanner yang wajib dan bermakna: 999/1000/1001 lines; CRLF dan last-line tanpa newline; nested `.tsx`, `.mm`, `.hpp`, CSS, test dan script; untracked source; generated first-party source; build/vendor exclusion yang sah. Fixture panjang dibuat sementara oleh test, tidak perlu menyimpan ribuan baris dummy dalam repo.

### 4A.8 Workflow pemecahan dan acceptance

Saat module mendekati batas, lakukan langkah berikut dalam fase aktif:

1. Identifikasi tanggung jawab yang bercampur: rendering, state, math, I/O, domain commands atau algorithm stages.
2. Pilih module boundary dengan typed public API dan owner state yang jelas.
3. Pindahkan kode, perbarui imports/CMake targets dan hapus duplicate implementation lama.
4. Jalankan tests perilaku module, typecheck/build relevan, architecture guard dan line-count check.
5. Untuk UI, pastikan screenshot/interaction tidak berubah tanpa sengaja. Untuk native, ukur/test realtime hanya pada risiko terkait refactor, bukan mengulang semua benchmark tanpa alasan.
6. Catat module changes dan hasil check pada `docs/progress.md`.

Untuk repo existing dengan file >1.000 baris, inventarisasi dahulu dan refactor bertahap tanpa menghapus fitur. File touched yang melanggar harus diperbaiki pada fase aktif; seluruh first-party oversized source harus selesai sebelum overall handoff. Jangan menghapus kode hanya untuk lulus check atau mengubah threshold menjadi lebih besar.

Definition of Done tiap fase sejak UI-01:

- [ ] Reusable UI controls memakai implementation canonical, bukan copy-paste per channel/effect.
- [ ] Page/panel/service tidak mencampur tanggung jawab yang seharusnya dimiliki modul lain.
- [ ] File baru/diubah berada di bawah atau sama dengan 1.000 baris; target normal tetap jauh lebih kecil.
- [ ] check:file-size dan check:architecture lulus; tidak ada threshold/exclusion workaround.
- [ ] Native module contracts menjaga thread/buffer ownership dan realtime guarantees.
- [ ] Behavior/visual tests yang relevan tetap lulus setelah split.

Batas ini merupakan kriteria penyelesaian teknis, bukan kebutuhan persetujuan user untuk setiap pemecahan file. AI melakukan refactor rutin secara mandiri dalam scope tugas dan mempertahankan perubahan pengguna.

## 5. Kontrak audio dan parameter

### 5.1 Audio format

- Internal PCM: planar float32; accumulator tambahan boleh double jika profil menunjukkan kebutuhan.
- Project rate default: 48,000 Hz. Wajib mendukung 44,100 dan 48,000; 96,000 opsional setelah benchmark.
- Buffer request: 128/256/512/1024 frames; default 256. Selalu baca nilai aktual device.
- 256 frames pada 48 kHz = sekitar 5.33 ms per block, **bukan** total round-trip latency.
- Output device/aggregate menjadi master clock realtime. File diresample ke project rate oleh worker streaming.
- Gunakan sample-frame integer 64-bit untuk posisi timeline. Semua frame counter/offset pada JSON menggunakan decimal string sejak awal; renderer memakai BigInt pada boundaries dan Number hanya untuk rentang viewport yang aman.
- JSON tidak boleh memuat NaN, Infinity, atau `-Infinity`. Nilai meter silence dikirim `null`; UI menampilkan `−∞`.
- Jangan hard-clip setiap channel. Sisakan headroom internal float; limiter diletakkan pada output yang membutuhkan batas.

### 5.2 Parameter dasar

Rentang berikut adalah spesifikasi produk, bukan nilai yang diklaim aman untuk setiap sumber suara.

| Parameter | Rentang/default | Semantik |
| --- | --- | --- |
| Digital trim | -60 sampai +24 dB; default 0 | Sesudah input, sebelum filter |
| Channel fader | Off atau -90 sampai +10 dB; default 0 | Gain sesudah insert chain |
| Pan mono | -1 sampai +1; default 0 | Equal-power: tengah masing-masing -3.01 dB |
| Stereo balance | -1 sampai +1; default 0 | Tengah unity L/R; tidak dianggap pan mono |
| HPF | Off; 20–400 Hz, 12/24 dB/oct | Voice preset boleh 80 Hz |
| LPF | Off; 1–20 kHz, 12/24 dB/oct | Clamp di bawah Nyquist |
| EQ 4 band | Low shelf, 2 peaking, high shelf; ±18 dB | Frequency 20–20k dibatasi rate, Q 0.1–10 untuk peaking |
| Gate threshold | -80 sampai 0 dBFS; default -50 | Default gate bypass |
| Gate range | 0–80 dB; default 60 | Attenuation maksimum |
| Gate timing | Attack 0.1–50 ms, hold 0–500 ms, release 10–2000 ms | Hysteresis default 3 dB |
| Expander ratio | 1:1–10:1; default 2:1 | Alternatif mode gate |
| Compressor | Threshold -60–0; ratio 1–20; knee 0–12 dB | Default bypass; makeup manual 0 dB |
| Compressor timing | Attack 0.1–200 ms; release 10–3000 ms | Stereo detector linked |
| De-esser | 2–12 kHz; reduction limit 0–12 dB | Default bypass; detector band dan audition |
| Master limiter | Ceiling -1 dBFS; lookahead 3 ms default | Aktif; user bypass eksplisit, latency dilaporkan |
| Send level | Off atau -90 sampai +10 dB | Pre/post-fader selectable |
| Monitor dim | -20 dB default | Hanya monitor path |
| Delay | 1–2000 ms; feedback 0–0.9; wet 0–1 | Feedback internal effect dibatasi |
| Reverb | Decay 0.2–10 s; pre-delay 0–100 ms | FX return default wet 100% |

Semua parameter memiliki stable ID, unit, min/max, default, step, automation capability. Validasi terjadi di UI dan engine; engine tetap authoritative. Scalar invalid ditolak dengan error, bukan diam-diam mengubah sesi.

`linearGain = 10^(dB/20)`. Off direpresentasikan flag/enum, bukan angka JSON infinity. Gain ramp 5–20 ms; mute/unmute dan route transition juga harus diramp. EQ update jangan interpolasi koefisien secara sembarang jika berisiko tidak stabil; gunakan perubahan parameter terbatas atau crossfade dua filter prepared.

### 5.3 Signal path baku

Urutan channel: input → trim/polarity → HPF/LPF → gate/expander → EQ → compressor → de-esser → Vocal FX rack/user inserts → pre-fader tap → fader/DCA → pan/balance → post-fader tap → selected main/subgroup.

- Dry recording tap: sesudah channel mapping, sebelum digital trim/DSP.
- Processed recording tap: sesudah insert, sebelum fader; master recording diambil sesudah master chain sebelum monitor controls.
- Pre-fader send: sesudah insert, sebelum fader. Versi 1 selalu mengikuti channel mute; tidak mengikuti channel fader/DCA attenuation.
- Post-fader send: mengikuti fader, DCA, pan/balance, dan mute.
- Mute channel memutus main dan semua send; dry/processed armed recording tetap berjalan.
- DCA mengendalikan gain/mute channel terikat; DCA tidak membawa audio dan tidak membuat penjumlahan baru.
- Subgroup adalah bus penjumlahan audio. Channel default memilih main **atau** subgroup agar tidak tergandakan.
- PFL/AFL hanya menuju monitor bus, tidak mengubah main/record/export. Solo-in-place adalah mode terpisah dan harus ditampilkan jelas.
- PFL mono dipan tengah; stereo mempertahankan L/R. PFL mengabaikan channel fader/mute untuk audition; AFL mengambil hasil post-fader sebelum mute.
- Jika banyak PFL/AFL aktif, sum ke monitor dengan headroom dan monitor limiter. Headphone monitor jangan otomatis aktif saat mic ditambahkan.
- Master path: bus sum → master EQ/compressor/inserts → master fader → output limiter → master record/export tap.
- Monitor controls berada sesudah pemilihan main/PFL/AFL. Monitor volume/dim tidak masuk file hasil.

Graph bus harus DAG. Tolak cycle termasuk aux→return→aux yang membuat loop. Feedback delay internal hanya di dalam processor yang memiliki bound, bukan edge bebas pada routing graph.

## 6. Routing audio sistem di macOS

### 6.1 Backend awal: BlackHole dan Aggregate Device

BlackHole adalah virtual loopback device yang dapat mengirim audio antar aplikasi dan menyediakan build Apple Silicon. Gunakan instalasi terpisah sebagai prasyarat pengembangan; jangan otomatis membundel atau memodifikasi driver. [BlackHole repository](https://github.com/ExistentialAudio/BlackHole).

Desain routing aplikasi ini: **app sumber → BlackHole → engine DSP → output fisik**. Jangan membuat Multi-Output yang juga mengirim suara asli langsung ke speaker: pengguna akan mendengar dry dan processed bersamaan.

Gunakan satu Aggregate Device untuk menggabungkan BlackHole, physical output, dan mic/interface yang diperlukan. Pilih clock source yang tepat dan drift correction pada device non-master sesuai konfigurasi hardware. Urutan subdevice menentukan pemetaan kanal, sehingga aplikasi harus membaca channel map aktual. [Apple Aggregate Device](https://support.apple.com/en-us/102171).

Langkah setup development:

1. Instal BlackHole 2ch dari sumber resminya. Operasi installer/OS dilakukan di Mac pengguna.
2. Buka Audio MIDI Setup dan buat Aggregate Device bernama `LocalMixer Aggregate`.
3. Tambahkan physical output, BlackHole, dan input mic bila diperlukan. Samakan rate ke 48 kHz jika didukung.
4. Pilih hardware output/interface yang stabil sebagai clock source; atur drift correction untuk subdevice lain sesuai dokumentasi.
5. Di engine, buka aggregate yang sama sebagai input/output. Mapping input sistem menunjuk kanal input BlackHole; output master hanya menunjuk kanal physical output.
6. Set seluruh output aggregate yang tidak dipakai ke silence, terutama output menuju BlackHole.
7. Setelah engine siap, user dapat memilih BlackHole sebagai output sistem. Jangan memilih aggregate sebagai default system output untuk desain routing ini.
8. Putar file lokal lewat QuickTime; verifikasi fader master mixer memengaruhi satu-satunya suara terdengar.
9. Tambahkan browser; verifikasi dua app tergabung ke channel system yang sama.
10. Stop Engine/Disable System Routing/quit normal memulihkan route yang diubah aplikasi jika masih sesuai ownership policy di bawah. Tombol Stop transport tidak menghentikan system/mic mixing atau restore route (8E.3).

BlackHole 2ch adalah satu stream stereo; 32 strips aplikasi tidak menciptakan 32 sumber sistem independen. Pemilihan output harus berdasarkan device UID dan channel identity, bukan indeks/nama tetap.

### 6.2 Lifecycle routing

Untuk perubahan default output oleh aplikasi: simpan original output UID secara durable, set transaction state pending, pastikan engine siap, ubah route, verifikasi, lalu tandai active. Jika gagal, rollback.

Saat quit normal, restore hanya jika default output sekarang masih device yang dipasang aplikasi. Jika user telah mengganti output sendiri, jangan menimpa pilihannya. Jika original device hilang, tampilkan pilihan physical output yang tersedia; jangan memilih virtual input sebagai fallback.

Jika engine crash tetapi Electron main hidup, supervisor menghentikan mode routing dan mencoba restore. Jika seluruh aplikasi terkena SIGKILL atau listrik mati, restore instan tidak dijamin. Persist recovery marker untuk startup berikutnya dan sediakan petunjuk manual `System Settings → Sound → Output → speaker/headphone`. Jangan menjanjikan fail-open tanpa helper terpisah yang benar-benar diimplementasikan dan diuji.

### 6.3 Backend lanjutan: Core Audio taps

Apple menyediakan API/sample Core Audio taps untuk capture sistem. Phase 18 harus memakai dokumentasi dan SDK aktual sebagai dasar, termasuk availability dan permission; halaman ini tidak cukup untuk mengasumsikan semua versi OS memiliki perilaku identik. [Apple Core Audio taps](https://developer.apple.com/documentation/coreaudio/capturing-system-audio-with-core-audio-taps), [CATapDescription](https://developer.apple.com/documentation/coreaudio/catapdescription).

Implementasi wajib memverifikasi: mute original saat capture, pengecualian proses engine agar tidak menangkap dirinya sendiri, cleanup tap/aggregate, PID berubah, permission ditolak/dicabut, serta sinkronisasi clock. Jangan mengaktifkan BlackHole dan taps pada sumber yang sama sekaligus. Jangan memakai private API. Jika tap gagal, tampilkan backend unavailable dan pertahankan backend BlackHole yang sudah teruji.

## 7. Protocol IPC dan model data

### 7.1 Envelope

Protocol version `1`. UTF-8 JSON Lines, satu object per baris. Maksimum 1 MiB per pesan; import session besar memakai file handle/path yang tervalidasi. Request mempunyai ID unik dan timeout; command panjang mengembalikan job ID.

Contoh request:

```json
{"protocolVersion":1,"id":"req-42","type":"command","method":"parameter.set","params":{"channelId":"ch-voice","parameterId":"faderDb","value":-6,"expectedRevision":12}}
```

Contoh ACK:

```json
{"protocolVersion":1,"id":"req-42","type":"response","ok":true,"result":{"revision":13,"appliedAtFrame":"4096"}}
```

Contoh error:

```json
{"protocolVersion":1,"id":"req-42","type":"response","ok":false,"error":{"code":"PARAMETER_OUT_OF_RANGE","message":"faderDb must be between -90 and 10, or use off flag","retryable":false}}
```

Handshake mengembalikan engine version, protocol version, capabilities, sample rate/buffer aktual, device UID, dan session revision. Versi tidak cocok harus gagal dengan pesan yang bisa ditindaklanjuti.

### 7.2 Command wajib

| Kelompok | Method |
| --- | --- |
| Lifecycle | `engine.hello`, `engine.start`, `engine.stop`, `engine.shutdown`, `engine.status` |
| Device | `device.list`, `device.configure`, `routing.system.enable`, `routing.system.disable` |
| Channel | `channel.add`, `channel.remove`, `channel.rename`, `parameter.set`, `parameters.batch` |
| Graph | `routing.validate`, `routing.apply`, `group.assign`, `monitor.configure` |
| Media | `media.import`, `media.relink`, `job.cancel`, `job.status` |
| Transport | `transport.play`, `transport.pause`, `transport.stop`, `transport.seek`, `transport.loop` |
| Recording | `record.arm`, `record.start`, `record.stop` |
| Session | `session.new`, `session.open`, `session.save`, `session.snapshot`, `history.undo`, `history.redo` |
| Export | `export.start`, `export.cancel` |
| Automation | `automation.mode`, `automation.points.replace`, `midi.learn` |
| Plugins | `plugin.scan`, `plugin.add`, `plugin.remove`, `plugin.editor.open` |

Reliable responses dan lifecycle errors tidak boleh dijatuhkan. Meter event boleh coalesce/drop jika UI lambat, maksimum 30 Hz. stdout backpressure tidak pernah memblokir audio callback. Parameter automation berfrekuensi sample tidak dikirim realtime dari React; schedule disimpan di engine.

### 7.3 Model sesi

`Session` menyimpan `schemaVersion`, UUID, project rate, channel/bus definitions, routing edges, transport/loop/markers, media references, clips, automation lanes, plugin states, recording preferences. Device settings disimpan terpisah sebagai machine preferences dengan UID dan channel mapping; sesi portable tidak memaksa device yang tidak ada.

`Clip`: UUID, media UUID, timeline start frame, source offset frame, duration frames, source sample rate, gainDb, fadeInFrames, fadeOutFrames. Trim/split tidak menulis ulang source. Simpan media relative path, size/hash metadata; hash file besar di worker.

`Channel`: UUID, type, mono/stereo format, input assignment, ordered processors, parameters, fader off flag, mute, solo, destination, sends, DCA membership, record arm, record tap, input monitoring. Runtime PID tidak menjadi identitas persisten aplikasi.

Errors minimum: `DEVICE_NOT_FOUND`, `DEVICE_DISCONNECTED`, `UNSUPPORTED_FORMAT`, `PERMISSION_DENIED`, `ROUTING_CYCLE`, `FEEDBACK_ROUTE_REJECTED`, `REVISION_CONFLICT`, `QUEUE_FULL`, `MEDIA_MISSING`, `DISK_FULL`, `RECORD_OVERRUN`, `PLUGIN_FAILED`, `PROTOCOL_MISMATCH`.

## 8. Tahapan implementasi yang harus dieksekusi

### Phase 00 — Audit lingkungan dan keputusan dependency native

**Urutan revisi 1.4:** fase ini dijalankan setelah UI-04 berstatus UI_VERIFIED. Audit frontend ringan sudah dilakukan UI-00. Reuse dependency/lockfile yang sudah dipin; lanjutkan audit native SDK/JUCE, jangan restart proyek.

**Prasyarat:** repository target tersedia atau dibuat sebagai proyek baru.

**Pekerjaan:**

1. Baca `AGENTS.md`/instruksi existing dan periksa git status; jangan menimpa perubahan pengguna.
2. Deteksi OS, arm64, macOS version, Xcode SDK, Apple Clang, CMake, Ninja, Node, npm.
3. Tentukan deployment target awal macOS 14 atau lebih baru yang lolos SDK/toolchain; taps memakai runtime availability guard tersendiri.
4. Pin versi Electron/React/Vite/TypeScript/JUCE/CMake minimum yang benar-benar tersedia dan kompatibel. Simpan lockfile dan commit SHA; jangan gunakan floating `latest` pada build.
5. Buat ADR stack dan `docs/dependency-manifest.md` berisi versi, asal, lisensi, dan cara update.
6. Buat `scripts/doctor` yang hanya membaca lingkungan. Jika bukan Mac, tunjukkan bagian portable yang dapat dikerjakan.

Command inspeksi di Mac:

```bash
uname -m
sw_vers
xcode-select -p
xcrun --show-sdk-path
clang --version
cmake --version
ninja --version
node --version
npm --version
```

**Deliverable:** doctor report, dependency pins, ADR, initial progress matrix.  
**Acceptance:** tidak ada dependency version placeholder; tidak ada klaim Mac build jika belum dijalankan.

### Phase 01 — Tambahkan build native pada desktop shell yang sudah selesai

**Prasyarat:** UI-00–04 selesai dan Phase 00 audit native selesai. UI React/Electron sudah dibangun; jangan membuang atau menulis ulang layout approved.

**Pekerjaan:** pertahankan npm workspace, Electron main/preload dan React window hasil fase UI. Tambahkan CMake native executable serta DSP test target portable. Tambahkan scripts root `dev`, `dev:engine`, `build:native`, `build`, `typecheck`, `test:ui`, `test:native`, `test:e2e`, `verify`, `package:mac:unsigned`.

`dev:ui` tetap berjalan tanpa native binary. `dev:engine` memerlukan build native dan tidak boleh fallback diam-diam ke preview. `dev` menjadi alias `dev:ui` selama tahap UI; perpindahan ke mode engine harus eksplisit. Development Vite lifecycle dikelola dan dihentikan saat quit. Engine path memakai dev build path atau packaged resources path, bukan cwd asumsi.

Electron security: context isolation aktif, node integration renderer mati, sandbox aktif bila sesuai konfigurasi, CSP, preload API allowlist dan validasi sender. UI memakai asset lokal; jangan membuka arbitrary URL dengan hak native. [Electron security guide](https://www.electronjs.org/docs/latest/tutorial/security).

**Acceptance:** app membuka window, native binary arm64 dapat dieksekusi, seluruh proses berhenti saat quit, portable tests berjalan. Mixer UI lengkap sudah tersedia; pertahankan hasil visualnya dan verifikasi native integration tidak menggeser layout.

### Phase 02 — Protocol dan supervisor engine

**Pekerjaan:** implement JSONL parser bounded, command dispatcher, handshake, fake engine untuk tests, stderr logging, timeout/cancellation, unexpected EOF, native child lifecycle. Gunakan `spawn` dengan argument array dan `shell:false`.

Tambahkan engine states `STOPPED`, `STARTING`, `RUNNING`, `RECONFIGURING`, `RECOVERING`, `ERROR`. Audio ready harus berdasarkan event engine, bukan sukses spawn. Batasi auto-restart misalnya maksimum 3 kali/menit; jangan restart-loop tanpa batas. Reconnect mengambil snapshot baru dan membuang pending command lama secara jelas.

**Acceptance:** malformed/oversized message ditolak; engine crash tampil sebagai error; fake engine tidak digunakan diam-diam dalam production; UI hang tidak menjadi ketergantungan engine.

### Phase 03 — Audio device dan passthrough

**Prasyarat:** Phase02. Implement master gain/mute dan bounded sample-peak output limiter dasar di sini agar test tone/passthrough sudah punya output protection; Phase14 memperluasnya dengan mode/latency/true-peak, bukan membuat limiter pertama kali.

**Pekerjaan:** implement `DeviceService`, UID lookup, input/output channel labels, actual rate/block size, device listeners. Sediakan test tone default off pada -30 dBFS yang melewati monitor attenuation dan limiter.

Buat passthrough satu input ke physical output dengan mapping eksplisit. Input monitoring awal off. Preallocate untuk maksimum block size yang disepakati; bila device memberi block lebih besar, gunakan bounded chunk processing yang diuji atau stop/reconfigure aman, bukan realloc dalam callback.

**Acceptance:** physical device terdeteksi; mono/stereo mapping benar; actual rate dibandingkan project rate dan mismatch ditolak untuk stream project sesuai 8E.5, bukan diterima diam-diam; output tidak berisi uninitialized memory; mic permission ditolak menghasilkan state yang jelas. Lakukan uji kabel/headphone sebelum speaker untuk mic monitor.

### Phase 04 — System audio capture dan routing recovery

**Prasyarat:** Phase03 dan **Phase05**. Kerjakan multi-source/fader dasar dahulu agar uji routing/fader memiliki implementasi yang nyata.

**Pekerjaan:** implement BlackHole detection, aggregate channel mapping wizard, route validation, system route transaction/restore, callback output clearing. UI menunjukkan sumber system stereo tunggal. Jangan mengubah default output sebelum engine ready.

Buat panduan setup manual dari bagian 6.1. Tambahkan diagnostics input meter, selected input range, physical output range, dan alasan route ditolak. Jangan melakukan koreksi audio dengan restart paksa `coreaudiod`.

**Acceptance:** QuickTime dan browser terdengar melalui mixer; fader -6 dB mengubah level engine sesuai; mute menghasilkan silence; tidak ada jalur dry ganda; output virtual yang dapat kembali ke input ditolak; normal quit restore berhasil; crash/forced kill recovery diuji dan keterbatasannya didokumentasikan.

### Phase 05 — Multi-source engine dan channel strips dasar

**Prasyarat:** Phase03. Uji tahap ini memakai physical input dan injected native fixture sources; system capture baru dihubungkan Phase04, jadi tidak ada dependency melingkar.

**Pekerjaan:** buat pool 32 strips dengan create/remove/rename/color, mono/stereo assignment, trim, fader, pan/balance, mute, stereo link, input monitoring. Mic dan system bisa aktif bersamaan melalui aggregate. Channel link bukan dua processor yang saling menulis parameter tanpa ownership.

Graph update menggunakan prepare/publish/reclaim; parameter changes via ramp dan bounded queue. Sertakan channel input meter dan output meter yang terpisah.

**Acceptance:** source A tidak mengubah B; input mic tidak otomatis masuk speaker; remove/add channel saat playback tidak crash; channel stale ID ditolak; 32 silent strips tidak menimbulkan memory growth; channel sum diuji dengan fixture known amplitude.

### Phase 06 — Import file dan transport dasar

**Pekerjaan:** WAV/AIFF/FLAC melalui native reader yang dipin; MP3/M4A lewat decoder yang benar-benar tersedia atau bundled FFmpeg worker. Baca file secara streaming; jangan decode semua track panjang ke RAM.

Buat media import job dengan progress/cancel, cache normalized PCM bila perlu, waveform min/max pyramid di worker, play/pause/stop/seek/loop. Semua file dijadwalkan dengan satu timeline engine dan offset sample yang sama; jangan memakai satu browser audio element per track.

Definisikan stop kembali ke posisi awal playback; pause mempertahankan posisi. Seek mengosongkan buffer stale dan reset DSP yang memerlukan reset dengan fade pendek. Loop tanpa click memakai boundary fade yang terdokumentasi. Track stereo/mono dipetakan eksplisit.

**Acceptance:** fixture impulse pada dua track sejajar sample; pause/resume konsisten; mixed-rate import durasi/pitch benar; seek berulang tidak memutar audio lama; import cancelled tidak meninggalkan media reference invalid; file missing tidak crash.

### Phase 07 — Meter, gain staging, dan EQ

**Pekerjaan:** implement peak per block, RMS window sekitar 300 ms, UI decay, peak hold 1 s, clip latch manual reset, meter tap input/post-insert/post-fader. Peak/RMS dalam dBFS; RMS sine amplitude 1 adalah sekitar -3.01 dBFS pada definisi matematis yang dipakai.

Implement HPF/LPF dan EQ empat band dengan enable/bypass dan response curve dari parameter/filter aktual. Default EQ flat. Tambahkan polarity invert L/R dan linked controls. Clipping input hardware dan output digital ditampilkan berbeda.

**Acceptance:** sine amplitude 0.5 menunjukkan peak sekitar -6.02 dBFS; trim +6 dB memberi rasio amplitudo sesuai; flat EQ mendekati unity; filter cutoff/boost diverifikasi secara numerik; sweep parameter tidak menimbulkan NaN/unstable output. Meter silence tidak menjadi JSON invalid.

### Phase 08 — Gate, expander, dan compressor

**Pekerjaan:** gate dengan envelope, hysteresis, hold, range; expander mode; compressor threshold/ratio/knee/attack/release/manual makeup; linked stereo detector dan gain reduction meter. Implement soft knee dan makeup secara eksplisit jika processor dasar tidak mendukung.

JUCE compressor dasar mendokumentasikan threshold, ratio, attack, dan release; jangan mengarang setter knee/sidechain yang tidak ada. Perluas dengan implementation dan tests sendiri untuk spesifikasi tambahan. [JUCE Compressor](https://docs.juce.com/master/classjuce_1_1dsp_1_1Compressor.html).

**Acceptance:** signal steady -12 dBFS dengan threshold -24 dan ratio 4:1 mendekati -21 dBFS setelah settled pada hard knee dan makeup 0; silence tidak berkedip membuka gate; stereo image tidak bergeser; bypass mempertahankan latency alignment bila processor menambah latency.

### Phase 09 — Vocal channel dan de-esser

**Pekerjaan:** implement band detector de-esser, reduction cap, stereo linking, detector audition hanya di monitor. Buat preset `Voice Clean`, `Voice Warm`, `Voice Broadcast`, dan `Music Flat` sebagai starting values yang dapat diedit.

Contoh `Voice Clean`: HPF 80 Hz, EQ flat, compressor threshold -18 dBFS/ratio 3:1/attack 10 ms/release 120 ms, makeup 0; gate default bypass; de-esser detector 6 kHz, maximum reduction 6 dB, threshold harus disetel sesuai sumber. Label preset sebagai titik awal, bukan hasil otomatis yang selalu benar.

**Acceptance:** burst high-frequency berkurang saat detector melewati threshold; sinyal low-frequency di luar detector tidak direduksi berlebihan; detector audition tidak bocor ke master recording; fader Voice hanya memengaruhi sumber channel tersebut.

### Phase 10 — Bus, aux, DCA, dan monitoring

**Pekerjaan:** implement 4 subgroups, 4 aux monitor, 2 dedicated FX send buses A/B dan 2 FX returns A/B, 4 DCA; route editor; topological sort/cycle rejection; pre/post send; PFL/AFL; monitor output pair selector; dim/mute dan monitor limiter.

Simpan bus destination dan assignments sebagai UUID. Tampilkan channel name serta physical output label. Pada hardware dengan satu stereo output, monitor dan main memakai endpoint yang sama melalui monitor selector, bukan dua jalur penjumlahan yang menggandakan main.

**Acceptance:** pre send tidak berubah saat fader digerakkan; post send berubah; mute keduanya off; DCA mengubah effective gain tanpa mengubah saved channel fader; subgroup tidak menjumlahkan track dua kali; PFL/AFL tidak masuk export; cycle ditolak sebelum graph aktif berubah.

### Phase 11 — Reverb, delay, dan voice ducking

**Pekerjaan:** FX bus send-return dengan wet 100%; delay feedback capped dan tone filtering; reverb pre-delay/decay; ducking music bus dari detector voice dengan depth/attack/hold/release. Sidechain edge hanya membawa detector audio, bukan menambahkan suara voice ke music bus.

Voice ducking default off; sumber detector dapat pre-fader tetapi tetap mengikuti mute sesuai spesifikasi agar mic muted tidak menurunkan musik. FX return tidak boleh kembali ke send bus sendiri. Sediakan panic mute yang meramp semua physical outputs.

**Acceptance:** delay repeats menurun sesuai feedback; feedback tidak mencapai unity; tails dipertahankan saat transport pause sesuai policy yang ditampilkan; mute/panic segera efektif dengan ramp pendek; voice activity menurunkan music bus dan pulih tanpa pumping ekstrem pada fixture.

### Phase 12 — Timeline editing dan transport lengkap

**Pekerjaan:** clip move/trim/split, snap off/grid, markers, fades, clip gain, keyboard shortcuts, metronome/count-in, tempo sederhana tanpa tempo map. Edit non-destructive; command pattern untuk undo/redo.

Overlap clip pada channel yang sama dijumlahkan, dengan visual overlap yang jelas; jangan diam-diam mengganti clip. Metronome default hanya monitor dan tidak ikut master export; opsi print click harus eksplisit. Hilangkan drag state lama ketika project dibuka ulang.

**Acceptance:** split dan join tanpa processing perubahan menghasilkan posisi sample konsisten; undo/redo memulihkan clip references; edit selama playback aman; metronome tempo stabil; viewport scrolling tidak memengaruhi audio clock.

### Phase 13 — Recording master dan multitrack

**Pekerjaan:** implement recording state machine dan take/clip alignment 8E.4; writer thread dengan bounded preallocated ring buffer, master record serta arm per-channel, dry/processed tap selector, file naming collision-safe, progress/time, input monitor independen, free-space monitor di worker.

Gunakan WAV 24-bit atau 32-bit float, session rate. Untuk rekaman panjang implement RF64 yang didukung writer atau segmentasi sebelum batas WAV klasik 4 GiB. Simpan manifest segment dan timestamp agar dapat direkonstruksi.

Jika writer tertinggal, jangan block callback atau diam-diam membuang sample lalu melaporkan sukses. Stop recording dengan `RECORD_OVERRUN`, simpan data yang valid dan lokasi discontinuity. Pada disk penuh, finalize best-effort dan tandai file partial. Graceful stop flush writer sebelum menyatakan recording saved.

**Acceptance:** master file tidak berubah ketika monitor volume/dim digerakkan; dry file tidak berubah oleh fader/EQ; processed file berisi insert chain; test disk-full/slow-writer fault injection; user dapat menemukan file hasil; timestamp antar-track sejajar dan input latency alignment didokumentasikan.

### Phase 14 — Export, loudness, limiter, latency compensation

**Pekerjaan:** offline renderer memakai DSP graph yang sama dengan realtime, independent instance dan session snapshot; jangan membuka physical audio device untuk export. Live sources tidak dapat dirender ulang: export hanya file/recorded sources, dan laporkan sumber live yang diabaikan sebelum job dimulai.

Export mix/stems, sample rate conversion, pilihan 24-bit PCM/32-bit float WAV/FLAC; MP3 jika bundled encoder tersedia. Tambahkan TPDF dither pada final float→integer quantization sekali; tidak untuk float export. Normalization default off; jika on gunakan dua pass dan ceiling limiter.

Implement master LUFS integrated/short-term dan true-peak hanya dengan algoritme/dependency teruji, catat versi metodologi dan fixture referensi. Jangan melabeli RMS sebagai LUFS atau sample peak sebagai true peak. Target loudness adalah pilihan pengguna, bukan default wajib semua musik.

Built-in limiter harus punya bounded lookahead dan sample-peak ceiling test; true-peak mode merupakan mode terpisah dengan oversampling dan validasi. Graph menghitung latency tiap node dan mengompensasi jalur paralel; latency perubahan plugin membutuhkan graph rebuild. Monitor low-latency mode melewati processor latency tinggi dengan indikator jelas.

FX tail export: durasi tail eksplisit default 3 s, configurable 0–30 s; jika auto tail, gunakan threshold dan maximum bound. Export offline mempertahankan automation, bypass, pan law dan sample rate yang sama.

**Acceptance:** realtime capture vs offline render dengan built-in deterministic effects sesuai toleransi setelah alignment; stems sesuai tap yang dipilih; cancel meninggalkan file `.partial` yang dibersihkan/ditandai; sumber live tidak diam-diam dianggap file; parallel impulse alignment tepat; LUFS/true-peak cocok fixture referensi sebelum label fitur enabled.

### Phase 15 — Session, preset, autosave, recovery

**Pekerjaan:** JSON schema/migrations; atomic save temp+rename, previous-good backup; autosave debounce 5 s setelah edit dan checkpoint berkala saat recording; open/relink missing media; collect media portable folder; channel/FX/full-session presets; undo/redo integration.

Pisahkan saved session path, dirty state, autosave recovery path. Session open divalidasi menyeluruh lalu dipublish sebagai transaksi; schema lebih baru ditolak dengan pesan jelas. Path traversal/absolute path diperlakukan sesuai user file selection, bukan diambil bebas dari renderer.

**Acceptance:** save/open round-trip memulihkan routing/FX/automation; corrupt save tidak menimpa previous good; crash recovery menawarkan sesi pemulihan tanpa menimpa original; project dipindahkan tetap membaca media yang dikumpulkan; device missing membutuhkan remap tanpa menghilangkan data proyek.

### Phase 16 — Automation dan MIDI

**Pekerjaan:** implement native MIDI input service termasuk timestamp/channel/note on/off/CC dan injected-event tests; parameter lanes untuk fader/pan/mute/send, engine sample scheduling, read/touch/latch/write modes, MIDI learn fader/knob/mute, mapping persistence. Continuous values memakai interpolation, mute memakai discrete event. Rekam automation dari command timestamps yang dipetakan ke engine clock.

Touch kembali ke stored automation dengan ramp setelah release; latch mempertahankan nilai sampai transport stop; write menimpa lane selama playback dengan status yang mencolok. Automation read tidak menulis keyframe karena UI sedang memperbarui meter/fader.

**Acceptance:** offline dan realtime automation selaras; seek mendapatkan nilai pada posisi tujuan; undo mengembalikan satu gesture sebagai satu action; MIDI disconnect tidak menghapus mapping; tidak ada MIDI feedback loop; fader tidak melompat ketika pickup/soft takeover aktif.

### Phase 17 — AU/VST3 plugin hosting

**Pekerjaan:** isolated scanner executable, timeout, crash blacklist, architecture check, plugin list cache, insert chain, state save/restore, native editor window milik engine. JUCE AudioProcessor merupakan dasar lifecycle processing/latency/state, bukan jaminan plugin isolation. [JUCE AudioProcessor](https://docs.juce.com/master/classjuce_1_1AudioProcessor.html).

Scan plugin hanya dari lokasi resmi atau path pilihan user. Simpan plugin identifier/version/state; missing plugin diganti bypass placeholder dengan state tetap tersimpan. Audio format negotiation wajib; mono/stereo mismatch tidak boleh menyebabkan out-of-bounds.

Runtime hosting tahap pertama boleh in-process dalam engine native: crash plugin dapat menjatuhkan engine dan supervisor harus menginformasikannya. Isolated scanning **bukan** isolated realtime processing. Jangan menjanjikan runtime crash isolation sebelum separate worker audio bridge, added latency, timeout, dan recovery benar-benar diimplementasikan.

**Acceptance:** plugin test yang stabil dapat diproses dan editor dibuka; scanner crash tidak menutup app; plugin state round-trip benar; plugin latency dikompensasi; plugin missing tidak merusak session; native engine crash recovery routing teruji. Catat AU/VST3 yang diuji, bukan klaim semua plugin kompatibel.

### Phase 18 — Per-app system capture

**Pekerjaan:** spike Core Audio taps pada SDK/Mac target; enumerate supported process audio sources; explicit permission flow; pilih app→channel; mute original/exclude own output; track process lifecycle dan auxiliary processes browser; clock bridge dan discontinuity handling.

Pisahkan system-mix capture dan per-app mode secara eksklusif pada sumber yang overlap. Bila sumber memberi clock berbeda, gunakan bounded buffer dan adaptive rate matching yang diuji, atau native aggregate clock mechanism yang terbukti; fixed-rate resample saja tidak menyelesaikan clock drift.

**Acceptance:** dua app dapat diatur terpisah; mute app A tidak mematikan B; engine output tidak recaptured; original dry tidak terdengar; app restart pulih atau meminta reselection dengan jelas; permission revoke menghentikan capture aman. Jika OS tidak mendukung, status fitur unavailable, bukan checkbox VERIFIED.

### Phase 19 — Hardening, packaging, dan handoff

**Gate revisi 1.6:** INT-00–03 wajib lulus dan feature traceability lengkap; modularity/line-count gates bagian 4A wajib lulus; UI-00–04 harus UI_VERIFIED; VFX-00–09, MIXFX-00–05, dan HARM-00–03 juga harus memenuhi status validasinya. Phase 19 bukan penutup fitur sebelum ekspansi Vocal FX selesai.

**Pekerjaan:** benchmark, soak, output disconnect, sample-rate change, sleep/wake, suspend/restart renderer, safe cleanup, packaging arm64. Bundle engine/required converter di resources executable path, set executable permissions, hindari ketergantungan Homebrew pada end-user kecuali BlackHole yang didokumentasikan sebagai instalasi terpisah.

Uji `.app` yang dikemas, bukan hanya `npm run dev`: izin input/capture, engine launch, file dialogs, plugin editor, media path ber-spasi/Unicode, application quit. Sertakan usage descriptions dan entitlements yang benar untuk proses pemilik capture berdasarkan SDK/packaging aktual. Jangan menonaktifkan Gatekeeper atau protections global sebagai solusi packaging.

Buat unsigned development `.app`/archive. DMG opsional setelah app lulus. Release public memerlukan signing/notarization bila credential tersedia; jangan mengklaim signed release tanpa verifikasi. Siapkan notices dan dependency/license manifest. Jangan publish otomatis.

**Acceptance:** kriteria PRODUCT_VERIFIED 8E.12 terpenuhi; clean Mac development install dapat mengikuti README; app bekerja offline setelah dependency tersedia; test matrix berisi hasil nyata; seluruh feature matrix wajib punya status; open blockers terlihat; tidak ada stub enabled; pengguna memiliki source, build commands, app artifact jika build Mac tersedia, dan manual penggunaan.


## 8A. Ekspansi wajib revisi 1.1 — Vocal FX Rack

### 8A.1 Tujuan, dependensi, dan urutan eksekusi

Pengguna menginginkan jenis efek vokal yang dapat dipilih seperti prosesor vokal/mixer digital, bukan hanya EQ dan dynamics. Implementasikan katalog **efek native bawaan**. AU/VST3 tetap tambahan; meminta pengguna membeli plugin untuk memenuhi katalog native bukan penyelesaian tugas ini.

**Urutan kanonik ada di 8E.1**: UI-00–04 → Phase00–03 → Phase05 → Phase04 → Phase06–16 → VFX-00–08 → MIXFX-00–04 → HARM-00–02 → Phase17–18 → INT-00–02 → VFX-09 → MIXFX-05 → HARM-03 → INT-03 → Phase19. Nomor section historis tetap dipakai untuk pelacakan, bukan diurutkan numerik tanpa membaca dependensi.

Untuk repository yang sudah berjalan, audit file/progress dan pertahankan hasil yang sudah benar. Phase15/16 menyediakan persistence/MIDI/automation dasar sebelum VFX; VFX-08 menghubungkan schema/parameter efek baru, kemudian VFX-09 memverifikasi ulang integrasi final yang berubah. Tidak wajib mengulang seluruh fase dasar yang telah teruji.

Fokus input pitch: satu penyanyi, mono, vokal terpisah. Stereo vocal diproses hanya dengan kebijakan detector yang jelas; jangan menjumlahkan L/R antiphase tanpa deteksi. Full-mix tetap dapat diberi reverb/modulation, tetapi pitch correction/harmony tidak diklaim polyphonic.

### 8A.2 Katalog native dan hasil yang harus terdengar

| ID stabil | Nama UI | Hasil utama | Implementasi minimum |
| --- | --- | --- | --- |
| `reverb` | Reverb | Ruang, ekor gema | Room, Plate, Hall sebagai algoritme/preset yang benar-benar berbeda parameternya |
| `delay` | Delay / Echo | Pengulangan suara | Slapback, Stereo, Ping-pong, tempo sync |
| `chorus` | Chorus | Lapisan bergerak lembut | Modulated fractional delay stereo |
| `doubler` | Vocal Doubler | Duplikasi vokal melebar | Dua voice delay/detune independen, direct voice hanya sekali |
| `pitch_shift` | Pitch Shift | Transposisi pitch | -12 sampai +12 semitone, fine tuning |
| `formant_shift` | Formant Shift | Karakter resonansi berubah | Perubahan spectral envelope tanpa transposisi F0 yang disengaja |
| `pitch_correct` | Pitch Correction | Nada mendekati tangga nada pilihan | Monophonic F0 detector, note mapping, retune smoothing |
| `harmony` | Harmony | Dua suara harmoni tambahan | Fixed semitone atau diatonic third/fifth, voice gain/pan |
| `saturation` | Saturation / Drive | Warna harmonik/distorsi | Soft saturation, harder drive, telephone/megaphone preset |
| `flanger` | Flanger | Sapuan comb-filter | Fractional delay pendek termodulasi dan bounded feedback |
| `phaser` | Phaser | Sapuan fase | Modulated all-pass cascade |
| `vocoder` | Vocoder / Robot | Artikulasi vokal dengan warna synth | Filter-bank envelope modulator + internal carrier synth |

Gate, EQ, compressor dan de-esser tetap tersedia di section Channel Strip. Rack creative FX tidak otomatis membuat EQ/compressor kedua. Pengguna boleh menambah instance EQ/dynamics melalui katalog Utility setelah lifecycle rack teruji; jangan tampilkan duplicate instance tanpa sengaja.

Istilah Robot adalah preset vocoder yang nyata, bukan sekadar menurunkan pitch. Pitch Shift berbeda dari Formant Shift; kontrolnya tidak boleh menjadi dua label untuk fungsi DSP yang sama.

### 8A.3 Rentang parameter dan default aman

Seluruh efek baru ditambahkan dalam keadaan bypass sampai graph siap, lalu crossfade aktif. Default channel/session baru tidak memasang semua efek sekaligus.

| Efek | Parameter wajib | Default ketika ditambahkan |
| --- | --- | --- |
| Reverb | type Room/Plate/Hall; decay 0.2–10 s; predelay 0–100 ms; damping 500–18000 Hz; width 0–100%; mix 0–100% | Plate, 1.4 s, 20 ms, damping 6 kHz, mix 15% |
| Delay | mode; time 1–2000 ms; sync; note 1/1–1/32 termasuk dotted/triplet; feedback 0–90%; low/high cut; mix | Stereo, 250 ms, feedback 20%, mix 12% |
| Chorus | rate 0.05–5 Hz; depth 0–100%; centre delay 7–30 ms; feedback -70–70%; mix | 0.6 Hz, depth 25%, 12 ms, feedback 0, mix 20% |
| Doubler | 2 wet voices; delay 5–40 ms; detune -20–20 cents; pan -1–1; voice level -60–0 dB; mix | 15/23 ms, -6/+6 cents, pan -0.7/+0.7, voices -9 dB, mix 25% |
| Pitch Shift | semitones -12–12; fine -100–100 cents; formant preserve; mix | 0 st, 0 cent, preserve on, mix 100% |
| Formant Shift | semitone-equivalent -6–6; mix | 0 st, mix 100%, pitch fixed unity |
| Pitch Correction | key C–B; scale Chromatic/Major/Natural Minor/Custom; A4 430–450 Hz; retune 0–300 ms; amount 0–100%; tolerance 0–50 cents; confidence gate; vibrato preserve | C Major, A4 440, retune 80 ms, amount 70%, tolerance 10 cents |
| Harmony | Fixed/Diatonic; key/scale; 2 voices; fixed interval -12–12 st atau scale degree -7–7; level -60–0 dB; pan; formant preserve | Diatonic +2/+4 scale steps, voices -12/-15 dB, pan -0.5/+0.5 |
| Saturation | Soft/Hard; drive 0–24 dB; tone; output -24–6 dB; oversampling off/2x/4x; mix | Soft, drive 3 dB, output -3 dB, 2x, mix 20% |
| Flanger | rate 0.05–5 Hz; base delay 1–10 ms; depth; feedback -90–90%; mix | 0.25 Hz, 3 ms, depth 35%, feedback 25%, mix 15% |
| Phaser | rate 0.05–5 Hz; centre 100–4000 Hz; depth; feedback -80–80%; mix | 0.4 Hz, 900 Hz, depth 30%, feedback 20%, mix 20% |
| Vocoder | 16 bands; carrier Saw/Pulse/Noise; MIDI/Fixed note; fixed MIDI note 36–84; envelope attack 1–100 ms/release 10–500 ms; unvoiced amount; wet level | Saw, Fixed C3 (MIDI 48), attack 5 ms, release 100 ms, wet 100% |

Frequency ranges harus di-clamp di bawah Nyquist. Range delay sync yang melewati kapasitas buffer ditolak atau ditampilkan bounded; jangan wrap index tanpa pemeriksaan. Preset telephone: band-limit sekitar 300–3400 Hz dan drive ringan; bukan simulator model hardware tertentu.

Semua nilai ini keputusan produk awal, bukan jaminan cocok untuk semua vokal. Voice natural dan extreme effect dinilai dengan rubric yang berbeda.

### 8A.4 Kontrak processor, catalog dan rack

Buat modul berikut saat fasenya aktif:

| File/modul | Tanggung jawab |
| --- | --- |
| `dsp/fx/EffectProcessor.h` | Interface runtime tanpa ketergantungan UI/JSON |
| `dsp/fx/EffectRegistry.*` | ID, parameter descriptors, factory, capability |
| `dsp/fx/EffectRack.*` | Ordered graph, alignment, bypass, mix |
| `dsp/fx/DelayEffect.*`, `ReverbEffect.*`, `ModulationEffects.*` | Time/modulation processors |
| `dsp/fx/PitchBackend.*` | Backend abstraction dan fixed-block adapter |
| `dsp/fx/PitchDetector.*`, `PitchCorrectionEffect.*` | F0/confidence dan correction control |
| `dsp/fx/HarmonyEffect.*`, `FormantEffect.*` | Voice generation/envelope shift |
| `dsp/fx/VocoderEffect.*`, `SaturationEffect.*` | Character DSP |
| `session/RackState.*` | Schema/migration/preset serialization |
| `src/features/vocal-fx/` | Browser, RackList, EffectEditor, PitchDisplay, PresetBrowser |
| `native/tests/fx/` | Analytic, rendered, stress dan latency tests |

Interface konseptual C++ (AI harus mendefinisikan tipe yang dirujuk dan mengimplementasikannya):

```cpp
class EffectProcessor {
public:
    virtual ~EffectProcessor() = default;
    virtual void prepare(const ProcessSpec&) = 0; // control thread, sebelum publish
    virtual void reset() noexcept = 0;             // owner thread, boundary aman
    virtual void process(AudioBlockView&, const ProcessContext&) noexcept = 0;
    virtual void applyRealtimeParameter(ParameterId, float) noexcept = 0;
    virtual uint32_t latencySamples() const noexcept = 0;
    virtual uint64_t maximumTailSamples() const noexcept = 0;
};
```

`ProcessContext`: sampleRate, absoluteFrame, transport state, tempo, MIDI event span dan sidechain span yang teralokasi sebelumnya. Parameter structural (algorithm type, oversampling, channel format, block mode) tidak masuk `applyRealtimeParameter`; buat prepared replacement di control thread. Serialization/preset berada di layer state, bukan callback. DSP state internal filter/FFT tidak ditulis bersamaan oleh control thread.

Catalog item: `effectType`, `displayName`, `category`, `version`, channel formats, parameter schema, latency capability, tail policy, default mix law, automation support, availability reason. Runtime processor memakai `instanceId` UUID; jangan memakai index rack sebagai identitas automation.

Maksimum awal 8 native slots per channel, 4 per bus. AU/VST3 memakai slot pool terpisah yang bounded: 4 per channel dan 2 per bus; semua ikut graph latency/CPU budget. Urutan native rack lalu plugin slots adalah default; tidak membuat slot tak terbatas. Menambah slots di atas limit mengembalikan error. Jumlah slot bukan janji semua heavy effects dapat aktif di 32 strips tanpa batas CPU.

IPC tambahan:

- `fx.catalog`, `fx.add`, `fx.remove`, `fx.move`, `fx.bypass`, `fx.parameter.set`, `fx.mix.set`.
- `fx.rack.snapshot`, `fx.rack.replace`, `fx.preset.load`, `fx.preset.save`, `fx.compare.store`, `fx.compare.recall`.
- Command membawa channel/bus ID, instance ID, expected revision. ACK menyertakan actual applied revision.
- `fx.move` hanya mengubah creative rack, tidak memindahkan built-in strip processor secara terselubung.
- `fx.rack.replace` all-or-nothing. Jika satu factory gagal, rack lama tetap aktif.
- Telemetry pitch: voiced/unvoiced, F0, confidence, target note, cents deviation; throttled 20–30 Hz dan tidak dibaca sebagai clock audio.

### 8A.5 Wet/dry, bypass, reorder dan format kanal

1. Rack berjalan serial. Satu slot dapat memiliki wet voices paralel internal yang dijumlahkan hanya di slot itu.
2. Rack inlet mempertahankan format source: vokal mono tetap mono selama prefix pitch; processor widening mengubah hasil menjadi stereo sesuai graph negotiation. Setelah rack menghasilkan stereo, kontrol channel memakai stereo balance; jika hasil tetap mono, gunakan pan mono. Detector pitch mengambil source mono sebelum stereo spreading. Jangan meng-collapse hasil stereo dari chorus hanya untuk memasukkan harmony tanpa peringatan/negotiation.
3. Untuk v1, enforce pitch correction/shift/formant pada prefix mono; harmony menerima mono dan dapat menjadi node pertama yang menghasilkan stereo, lalu chorus/doubler/reverb/vocoder. Setelah harmony stereo, processor mono-only ditolak kecuali format negotiation eksplisit mendukungnya. UI menolak reorder incompatible dengan alasan. Slot yang benar-benar stereo-capable boleh beroperasi sesudah widening jika tests mendukungnya.
4. Dry path ditunda sebesar **algorithmic latency**, bukan intentional delay/reverb predelay. Delay 250 ms adalah efek musikal, jangan dikompensasi menjadi suara tanpa echo.
5. Pitch backend start delay, block adapter delay dan oversampling filter delay harus masuk graph latency report.
6. Mix linear `out=(1-mix)*alignedDry+mix*wet` menjadi default; identity wet pada 50% tidak naik 3 dB. Equal-power opsional untuk efek decorrelated dengan law ditulis di preset; tidak dipakai universal.
7. Pitch correction default wet 100%; pencampuran corrected+dry menjadi creative doubling, bukan cara mengatur correction amount. Amount mengatur besaran correction ratio.
8. Aux FX instance wajib wet 100%; dry main tidak masuk return kedua kali. UI menjelaskan mix locked dalam send mode.
9. Bypass crossfade 10–30 ms sambil mempertahankan latency compensation. CPU-saving remove/suspend adalah aksi berbeda yang dapat mengubah graph latency melalui transaksi.
10. Saat reorder/preset change, pre-roll replacement processor sejauh dibutuhkan pada buffer history bounded atau fade dry→wet setelah warmup. Graph lama tidak dihancurkan di callback.
11. Bypass reverb/delay pada v1 mem-fade output tail; opsi spillover bisa ditambahkan hanya setelah lifecycle/tail budget teruji. Jangan menjanjikan tail preservation otomatis saat instance dihapus.
12. Seed modulation/random behavior tersimpan untuk deterministic offline tests; initial phase reset policy jelas saat seek/export.

### 8A.6 Strategi algoritme pitch dan latency

Gunakan abstraction `PitchBackend`, jangan menyebar panggilan library ke UI/harmony/corrector. Backend awal yang konkret untuk prototype adalah **Rubber Band LiveShifter**, dipin setelah spike build dan pemeriksaan lisensi. Library menyediakan pitch shifting; itu bukan pitch detector atau pitch correction siap pakai. [Rubber Band](https://breakfastquay.com/rubberband/).

Dokumentasi LiveShifter menyebut fixed processing block size, API pitch/formant scale dan start delay; juga menyatakan delay dapat 50 ms atau lebih. Query ukuran/delay dari instance, adapt device callback dengan preallocated FIFO dan ukur tambahan latency adapter. Jangan menganggap kata “Live” berarti monitoring tanpa jeda. [LiveShifter API](https://breakfastquay.com/rubberband/code-doc/classRubberBand_1_1RubberBandLiveShifter.html).

Sebelum integrasi, pilih versi serta jalur lisensi development/distribusi yang sesuai. Jika backend tidak dapat digunakan, tulis blocker/ADR dan cari backend native yang memenuhi interface serta quality gates; jangan mengganti dengan resampling sederhana yang mengubah durasi atau menyatakan fitur selesai dengan stub. Tidak perlu pembelian otomatis untuk menulis kode adapter/prototype dan pengujian portable.

Dua profile:

- **Monitor Fast:** mempertahankan EQ/dynamics/efek ringan; heavy pitch/formant/harmony dapat dilewati secara eksplisit jika melewati budget. UI menandai slot `Skipped in Monitor Fast` dan menyebut bahwa efek itu tidak terdengar di monitor ini. Jika diperlukan monitor/main graph terpisah, gunakan instance terpisah, bukan mengubah graph master di belakang user.
- **Studio FX:** seluruh efek terpilih aktif dengan latency yang dilaporkan. Dapat dipakai untuk file playback, rendering, atau monitoring yang menerima jeda tersebut.

Target baseline ≤25 ms bagian 9 tetap untuk graph ringan yang disebutkan di sana. Target itu tidak berlaku otomatis pada Studio FX. Tampilkan graph-added latency yang dihitung dari engine terpisah dari hardware round-trip yang harus diukur; tidak perlu angka palsu pada mockup.

Pitch detector baseline native: monophonic YIN-family implementation dengan documented window/hop dan independent tests. Mulai window 4096, hop 256 pada 48 kHz, range default 70–1000 Hz, dengan implementasi difference-function efisien yang diprofilkan; window2048 hanya boleh untuk range minimum lebih tinggi yang lolos accuracy tests; expose analysis range tanpa menjanjikan semua bass rendah/acoustic mixtures terdeteksi. Detector menggunakan preallocated window; time-stamp estimasi di pusat window dan align correction control terhadap delayed audio. Uji ulang di 44.1 kHz.

Note target: `midi=69+12*log2(f0/A4)`. Cari allowed note terdekat dalam key/scale/custom mask. Gunakan previous-note hysteresis 20 cents dan debounce 2 analysis hops sebagai default awal; tie pada startup memilih note rendah, kemudian tie memilih previous target bila valid. `ratio=2^(correctionSemitones/12)`, dibatasi range backend.

Pada silence/low confidence: pitch correction ramp ratio ke unity, jangan mengunci note random; harmony voices fade out; original dry tetap menurut mix policy. Consonants/unvoiced harus lewat tanpa suara chirp berat. Confidence threshold numerik dipilih dari detector actual; jangan menyamakan skor antar-algoritme tanpa kalibrasi.

Retune 0 ms berarti correction smoothing minimal; **bukan latency 0 ms**. Vibrato preserve: default off hingga pitch contour decomposition tervalidasi; implement low-rate target-centre correction sambil mempertahankan komponen vibrato, lalu uji modulation depth. UI menonaktifkan toggle dengan alasan sampai implementasi nyata tersedia.

Harmony diatonic: definisikan scale step relatif terhadap detected allowed scale note; +2 berarti third diatonic, +4 fifth. Misalnya C major, E4 +2 → G4, bukan selalu menambah empat semitone. Key/scale manual, tidak auto-key detection. Fixed harmony tidak memerlukan note quantization, tetapi tetap memerlukan input/voicing policy.

### 8A.7 Tahap implementasi Vocal FX

#### VFX-00 — Audit gap, catalog, schema dan design contract

**Input:** UI-04 serta Phase00–16 yang menjadi dasar telah tersedia (termasuk persistence/MIDI/automation); ikuti 8E.1.

**Tugas:** inventaris DSP existing; pisahkan parameter descriptor dari GUI; definisikan catalog 12 efek di 8A.2; buat effect/rack state schema, version migration v1→v2 session bila schema berubah, IPC errors, mono/stereo format policy, latency budget document. Tambahkan feature flags `unavailable`, `implemented_unverified`, `verified` untuk developer report; production UI menampilkan availability yang benar-benar relevan.

**Output:** `docs/specs/vocal-fx.md` bila dipecah, registry metadata, protocol fixtures, ADR pitch backend.  
**Acceptance:** setiap effect ID punya owner module, parameter schema, implementation phase dan test plan; catalog bukan bukti DSP selesai.

#### VFX-01 — Rack runtime, dry alignment, bypass dan editor skeleton

**Tugas:** implement EffectProcessor, registry factory, slot ownership, 8-slot limit, graph publication, latency-aware dry/wet, atomic rack replacement, add/remove/reorder, A/B snapshots. Gunakan hanya test gain/delay processor untuk verifikasi plumbing, tidak ditampilkan sebagai efek produksi.

**Uji:** stale revision, factory failure rollback, move incompatible format, rapid bypass 100 kali, identity 50% mix unity, injected 1024-sample algorithmic latency. Audio callback bebas allocation/lock.  
**Acceptance:** setiap action UI/IPC mengubah engine atau mendapat error nyata; reorder aman saat playback; graph retirement tidak terjadi pada callback.

#### VFX-02 — Reverb, delay, chorus, flanger dan phaser

**Tugas:** adapt reverb/delay Phase 11 ke interface baru tanpa memprosesnya dua kali. Room/Plate/Hall harus punya karakter parameter/algorithm yang terdokumentasi; gunakan algorithmic approximation, jangan klaim convolution plate tanpa IR. Tambahkan tempo sync dari engine transport, fractional-delay interpolation, LFO phase handling, filter dalam feedback path.

JUCE chorus menyediakan rate/depth/centre-delay/feedback/mix; phaser menyediakan all-pass modulation dan kontrolnya. Bungkus API versi pin, bukan mengasumsikan rentang UI sama dengan library. [JUCE Chorus](https://docs.juce.com/master/classjuce_1_1dsp_1_1Chorus.html), [JUCE Phaser](https://docs.juce.com/master/classjuce_1_1dsp_1_1Phaser.html).

**Uji:** impulse repeat spacing, left/right ping-pong alternation, bounded feedback, rate perubahan tempo, reverb energy decay/tail limit, modulation sweep, silence stability, send return wet-only.  
**Acceptance:** kelima efek menghasilkan perbedaan suara yang terukur; bypass/automation tidak click; tail export benar.

#### VFX-03 — Saturation, telephone/megaphone dan doubler

**Tugas:** saturation waveshaper dengan normalized transfer, output trim, optional oversampling, DC blocker; telephone/megaphone preset memakai band limit + drive. Doubler gunakan dua micro-delay voices dengan independent slow modulation/detune; jangan sekadar copy identical signal yang menambah volume.

**Uji:** harmonics spectrum, aliasing comparison oversampling on/off, unity drive baseline, DC offset suppression, dry counted once, mono fold-down, delay bounds.  
**Acceptance:** doubler melebar namun tidak menghilangkan vokal saat mono; preset drive punya output level terkendali; tidak ada auto makeup yang tak terlihat.

#### VFX-04 — Pitch shift dan formant shift backend

**Tugas:** build backend di arm64; `prepare`, fixed-block adapter, dynamic ratio update dari owner thread, reset/drain, start-delay accounting. Implement pitch/formant controls terpisah, preserve option, stereo coherence policy. Rebuild ketika structural config berubah.

**Uji:** 220 Hz +12 st → 440 Hz dan -12 st →110 Hz setelah warmup; file duration tetap; ratio unity time-aligned; formant fixture harmonic source dengan spectral envelope; extreme sweep; variable callback sizes; seek/reset.  
**Acceptance:** pitch frequency error ≤10 cents pada steady fixture; formant-only menjaga F0 ≤10 cents dan envelope bergerak sesuai arah; actual latency dicatat; jangan menilai kualitas formant dengan sine tunggal.

#### VFX-05 — Pitch correction monofonik

**Tugas:** implement F0 detector, voiced confidence, key/scale/custom mask, target mapping/hysteresis, retune amount/tolerance, real-time pitch display. Align control timestamps dengan input yang sedang dikoreksi. Tidak membutuhkan model ML/network.

**Uji:** generated notes dengan detune ±15/35 cents, note slides, vibrato, silence, fricatives/noise, octave transitions, A4 tuning. Bandingkan pitch sebelum/sesudah dengan independent estimator atau known ground truth.  
**Acceptance:** pada steady high-confidence detuned note dan full amount, output settled ≤10 cents dari target; amount 0 memberi unity pitch; key change tidak crash; unvoiced tidak menghasilkan note palsu; hasil vokal nyata diaudisi. Retune transition test terpisah dari steady-state accuracy.

#### VFX-06 — Harmony dua voice

**Tugas:** implement dua shifter voices independent, fixed/diatonic interval, common detector, gain/pan, formant preserve, dry alignment, voicing gate. Main dry hanya sekali; don't cascade harmony voice 2 dari voice 1. Tampilkan dua voice rows dengan enable, interval, level, pan.

**Uji:** C/E/G pada major dan natural minor, octave boundary, chromatic out-of-scale mapping, sustained vowels dan consonants, dry/wet energy, pan, phase/latency alignment.  
**Acceptance:** interval benar secara teori dan terukur; harmony tidak dipasarkan sebagai intelligent chord-aware harmonizer; low confidence fade-out tidak mematikan original voice; dua voice tidak memicu xruns pada profile uji.

#### VFX-07 — Vocoder dan robot voice

**Tugas:** implement 16 log-spaced analysis/synthesis bands kira-kira 100–8000 Hz disesuaikan rate, per-band envelope follower, carrier polyphonic saw/pulse maksimal 4 notes dengan oscillator antialias sesuai kebutuhan, noise/unvoiced path, limiter/output trim. Carrier Internal Fixed/MIDI. MIDI note-off, all-notes-off, disconnect dan panic wajib ditangani.

Robot preset: internal fixed carrier, wet 100%, fixed note yang dapat diubah. Vocoder tidak sama dengan pitch correction. External carrier sidechain dapat menyusul; tidak menjadi prasyarat native internal carrier yang diwajibkan.

**Uji:** modulator silence→wet silence; carrier note mengubah warna/pitch synth; modulator spectral band memilih synthesis band; MIDI sustain release; stuck-note prevention; finite output pada noise dan full-scale fixtures.  
**Acceptance:** artikulasi vokal termodulasi terdengar; tidak ada carrier hum tak terkendali saat mic diam; CPU/tail bounded.

#### VFX-08 — UI final, preset dan automation hooks

**Tugas:** hubungkan UI hasil UI-00–04 ke native engine; lengkapi detail advanced layout 8A.9, effect search/categories, drag reorder dengan keyboard alternative, parameter controls/units, bypass, A/B, input/output meters, CPU dan added latency dari engine. Preset bawaan memuat daftar instance dan nilai lengkap; tombol preset tidak sekadar mengganti label.

Preset wajib: Clean Voice, Warm Broadcast, Studio Pop, Karaoke Hall, Slapback, Wide Double, Low Character, Bright Character, Hard Tune, Harmony Duo, Telephone, Robot. Simpan preset schema version, algorithm versions, seed, state, wet/dry dan output trim; jangan menyimpan device UID/record-arm/OS route ke preset efek.

**Uji:** load/save per-rack, undo/redo satu preset transaction, A/B same input segment, stale UI ACK, missing effect/version handling, keyboard focus, 1280×800 scrolling.  
**Acceptance:** semua kontrol berpengaruh pada processor; unavailable effect tidak dapat diaktifkan; setelah Phase 15/16 dilakukan integration rerun untuk session dan automation.

#### VFX-09 — Integrasi audio dan release gate

**Prasyarat:** VFX-00–08, Phase15/16, Phase17/18 telah diverifikasi sesuai capability, dan INT-02 dev-package integration selesai. Tidak menunggu Phase19 untuk mendapatkan paket uji.

**Tugas:** render fixture tiap efek dan semua preset, test automation per instance, save/open dengan rack, record dry vs processed, offline export equivalent, output disconnect/recovery, packaged app permissions, benchmark heavy chain. Buat `docs/reports/vocal-fx-quality.md` dan `docs/reports/vocal-fx-latency.md`.

**Acceptance:** semua quality gates 8A.10 terisi hasil nyata; tidak ada effect label menggantikan implementasi. Jika komputer AI tidak punya Mac/mic, tests portable boleh VERIFIED, tetapi hardware/ear tests tetap NOT_RUN/BLOCKED_ENVIRONMENT dan final completeness jujur.

### 8A.8 Preset starting points

Preset tidak perlu memakai semua efek. Defaults mengutamakan level yang terkendali dan efek yang dapat dibedakan.

| Preset | Chain dan arah parameter |
| --- | --- |
| Clean Voice | Channel strip HPF/EQ/compressor/de-esser; creative rack kosong |
| Warm Broadcast | Clean strip + saturation drive 3 dB/output -3 dB/mix 20%; no reverb |
| Studio Pop | Clean strip + doubler mix 15%; aux Plate 1.4 s send -18 dB |
| Karaoke Hall | Clean strip + aux Hall 2.3 s send -15 dB; Stereo delay 250 ms send -22 dB |
| Slapback | Delay 100 ms, feedback 0%, insert mix 15% |
| Wide Double | Doubler -6/+6 cents, 15/23 ms, pans -0.7/+0.7, mix 25% |
| Low Character | Pitch -3 st, preserved formants, lalu formant -2 st; Studio FX |
| Bright Character | Pitch +3 st, preserved formants, formant +2 st; Studio FX |
| Hard Tune | Pitch correction amount 100%, retune 0 ms, vibrato preserve off; user sets key/scale |
| Harmony Duo | Diatonic third/fifth, voice levels -12/-15 dB; user sets key/scale |
| Telephone | HPF sekitar 300 Hz, LPF 3400 Hz, drive 6 dB, compensating output trim |
| Robot | Vocoder internal Saw, Fixed C3, wet 100%, output -6 dB |

Preset dengan aux membutuhkan representasi **linked scene preset** terpisah dari rack-only preset. Rack-only Studio Pop/Karaoke harus membuat reverb/delay insert mix yang ekuivalen atau menjelaskan aux assignment yang belum dibuat; jangan diam-diam mengubah global bus yang dipakai channel lain. Default implementasi gunakan rack-only variants, dan tampilkan aux versions sebagai pilihan explicit scene preset setelah Phase 15 mendukung transaksi graph lintas bus.

Pitch/key values preset bukan deteksi otomatis lagu. Sebelum memilih Hard Tune/Harmony Duo, UI menampilkan key/scale saat ini dan menyediakan edit; tidak perlu modal konfirmasi berulang.

### 8A.9 Spesifikasi tampilan dan interaksi mockup

Tab **Vocal FX** tetap ada untuk editor luas. Revisi 1.2 menjadikan **Mixer** akses utama program DSP A/B serta quick insert editor melalui tombol FX channel (bagian 8B). Tab Vocal FX tidak wajib dikunjungi untuk memilih efek atau mengubah parameter cepat. Mockup hanya konsep visual; acceptance berasal dari engine dan tests.

Layout desktop lebar:

- Header: project, transport, selected channel `VOICE • USB Mic`, profile `Studio FX`/`Monitor Fast`, save preset dan A/B.
- Panel kiri: **Effect Library** dengan search dan kategori Pitch, Space, Modulation, Character, Utility. Daftar efek memberi label singkat dan tombol tambah; tidak semua aktif.
- Panel tengah: **Vocal FX Rack**, numbered slots dengan drag handle, enable/bypass, effect name, ringkasan parameter, wet/dry, slot output meter. Scroll setelah 6 row; maksimum 8 slot tetap jelas.
- Panel kanan: **Effect Editor**, satu efek terpilih. Pitch Correction menunjukkan Key/Scale, detected/target note, cents display, retune/amount/tolerance. Memilih EQ menampilkan curve; memilih Harmony menampilkan dua voice rows; memilih Vocoder menampilkan carrier/bands.
- Bagian bawah: compact mixer bank SYSTEM/VOICE/MUSIC/FX/MASTER agar fader tetap dapat diakses. VOICE highlighted konsisten dengan editor.
- Footer: physical output dan monitor volume; graph-added latency serta effect load dari engine, bukan angka dekoratif.

Rack contoh visual: Pitch Correction → Formant Shift (bypass) → Doubler → Plate Reverb → Stereo Delay → Saturation (bypass). Ini contoh chain yang dapat diedit, bukan urutan terbaik universal. Semua pitch-sensitive slots tetap di prefix mono sesuai negotiation policy; output stereo sesudah doubler.

Editor tiap effect wajib: title, actual enabled state, reset defaults, units, keyboard input, help singkat, output trim bila diperlukan. Tombol A/B membandingkan dua snapshot rack dan melakukan latency-aware transition; tidak hanya mengubah warna tombol. Semua meter di mockup merupakan ilustrasi. PreviewAdapter boleh menyediakan deterministic meter fixtures berlabel preview; mode engine hanya membaca telemetry nyata. Jangan memakai nilai ilustrasi sebagai telemetry produksi.

### 8A.10 Quality gate audio dan performance

- Buat fixtures synthetic plus 3 rekaman vokal dry pendek dengan izin/asal jelas: bicara, sustained vowel, melody. Tidak perlu mengirim audio user ke cloud.
- Simpan hasil `.wav` dry/wet untuk tiap effect dan preset; level-match untuk audition agar “lebih keras” tidak dianggap otomatis “lebih bagus”.
- Rubric listening: intelligibility, clicks, metallic warble, consonant preservation, stereo mono compatibility, musical target correctness, creative character. Catat natural-mode artifacts terpisah dari intended robot/distortion.
- Tests latency impulse untuk efek linear dan cross-correlation/envelope untuk pitch/nonlinear; jangan memaksa semua efek lolos sample null test yang tidak sesuai sifat algoritme.
- Export tetap frame count/durasi sesuai source setelah latency trim plus tail eksplisit. Creative delay tidak dihapus sebagai latency. Startup padding/warmup backend dicatat.
- Performance ringan: voice strip + chorus/reverb/delay; ikuti baseline wired 256 frames. Performance heavy: pitch correction + 2 harmony voices + formant pada satu vocal strip; laporkan actual graph latency, callback p99/max, xruns, CPU/memory pada 256 dan 512.
- Jangan mengklaim semua 32 channel dapat memakai heavy rack penuh berdasarkan benchmark satu vokal. Buat stress report terpisah; capacity warning harus berdasarkan engine metrics.
- Monitor Fast routing, Studio FX, dry recording, processed recording dan export diuji masing-masing; snapshot profile record/export eksplisit agar file hasil tidak berubah tanpa diketahui karena user mengganti monitoring mode.
- Record processed default memakai **main Studio graph yang dipilih untuk recording**, bukan headphone Monitor Fast graph. UI menampilkan `Record FX: Studio` jika kedua graph berbeda.
- Preset extreme tidak boleh menimbulkan NaN/Inf, output runaway, feedback di routing, atau crash. Limiter output tetap aktif sesuai master policy.
- Library/preset tanpa DSP nyata, tombol dummy, demo audio prerecorded, dan random meter tidak memenuhi acceptance.

### 8A.11 Prompt lanjutan khusus Vocal FX

```text
Baca dokumen AUDIO-MIXER-AI-IMPLEMENTATION.md revisi 1.6, terutama bagian 8D untuk gate UI dahulu lalu bagian 8A untuk engine.
Audit repository dan progress existing. Pertahankan mixer/audio engine yang sudah teruji.
Implementasikan native Vocal FX Rack VFX-00 sampai VFX-09 sesuai dependency pada 8A.1.
Katalog wajib: reverb, delay, chorus, doubler, pitch shift, formant shift, pitch correction,
harmony dua voice, saturation, flanger, phaser, dan vocoder/robot.
Buat backend DSP nyata, bukan hanya panel UI, preset label, atau placeholder plugin.
Penuhi contract processor, sample/latency alignment, wet/dry, bypass, format negotiation,
serialization, automation, recording, dan offline export yang konsisten.
Gunakan layout bagian 8D yang sudah UI_VERIFIED; jangan hardcode meter, pitch note, CPU atau latency pada mode engine. Preview fixtures hanya untuk mode UI Preview.
Build/test setelah setiap fase dan simpan bukti pada docs/progress.md.
Jika native Mac/hardware tidak tersedia, jalankan bagian portable dan tulis batas validasi.
Jangan menjanjikan monitoring tanpa jeda untuk pitch/harmony; ukur dan tampilkan actual latency.
Lanjutkan sampai semua fase yang tidak terblokir selesai; simpan next exact action saat terhenti.
```


## 8B. Revisi 1.2 — Mixer utama dengan 99 program DSP

### 8B.1 Keputusan produk dan batas arti 99 program

Bangun workflow menyerupai mixer digital dengan pemilih program efek pada halaman Mixer utama. Pengguna tidak perlu membuka tab Vocal FX untuk memilih program, mengaktifkan efek, mengatur dua parameter utamanya, atau menaikkan jumlah efek pada vokal.

**99 program adalah bank preset dari beberapa algoritme native, bukan 99 algoritme DSP berbeda.** Nomor/nama/parameter di bagian ini adalah katalog original proyek, bukan tiruan bank produk hardware tertentu. Jumlah 99 mencakup konfigurasi Room, Plate, Hall, Slapback, Stereo Delay, Ping-pong, Chorus, Phaser, dan kombinasi Delay + Plate. Program pitch correction/formant/harmony/vocoder tetap ada di katalog insert per-channel bagian 8A; tidak dipaksakan menjadi efek send global.

Dua unit independen:

- **FX A:** satu program aktif dari bank 01–99, parameter overrides, enabled state, wet output dan return level sendiri.
- **FX B:** bank identik, instance DSP/state berbeda; bisa memilih program yang sama atau berbeda.
- Mengganti program A tidak mengubah B atau insert channel.
- Tidak ada 99 processor hidup di background. Hanya program terpilih per unit yang disiapkan, dengan temporary replacement instance ketika crossfade.
- Empat aux monitor dari spesifikasi dasar tetap tersedia. FX send A/B adalah dua dedicated buses tambahan dan tidak mengurangi jumlah aux monitor; dua existing FX returns dipakai sebagai return A/B, bukan diduplikasi.

Default project baru: FX A program 12 `Vocal Plate`, FX B program 50 `Stereo 320`; keduanya OFF, semua FX sends OFF, return A/B -12 dB. Program dipilih tetapi tidak ada efek terdengar sampai unit diaktifkan dan channel dikirim ke sana. Restore session menggunakan state yang disimpan, bukan mengulang default tersebut.

### 8B.2 Jalur audio yang harus diimplementasikan

Untuk setiap channel, sinyal post-insert/post-fader/post-pan masuk main/subgroup sesuai route dan secara terpisah masuk send A serta send B dengan gain masing-masing. Default send post-fader, pilihan pre-fader di inspector tersedia; mute mengikuti kebijakan bagian 5.3.

Setiap bus menjumlahkan kontribusi semua channel → program FX wet-only → return fader/mute → main bus sebelum master chain. Return meter membaca post-return-fader sebelum main sum. Tidak ada monitoring langsung dari input bus FX ke output.

Larangan:

1. Return A/B tidak punya send kembali ke A/B pada versi 1.2; tolak return→FX edge lewat UI maupun protocol.
2. Aux monitor tidak boleh mengembalikan sinyal ke capture input atau membuat routing cycle.
3. **OFF pada unit return berarti wet output silence**, bukan meneruskan input bus sebagai dry bypass. Ini berbeda dengan bypass insert yang meneruskan aligned dry.
4. Send OFF direpresentasikan enabled flag + bounded stored dB, bukan JSON minus infinity.
5. Pitch correction insert default 100% wet; send reverb tetap wet-only. Channel direct vocal hanya sekali di main.

Routing sederhana yang harus berhasil: Mic VOICE send A -18 dB, MUSIC send A OFF, A=Vocal Plate ON, A return -6 dB. Hanya VOICE menerima efek Plate; MUSIC tetap dry. Menyalakan B=Stereo Delay dan menaikkan VOICE send B menambahkan delay tanpa mematikan A.

Gate FX input mute berhenti memberi input baru; tail lama boleh decay. Unit OFF mem-fade wet output menuju silence dalam 20 ms. ON melakukan warmup lalu fade-in. Label UI **FX ON/OFF**, jangan menyebut OFF sebagai dry bypass. Channel insert masih memakai label BYPASS dengan semantik berbeda.

### 8B.3 Kontrol permanen pada halaman Mixer

| Lokasi | Kontrol | Perilaku wajib |
| --- | --- | --- |
| Tiap source channel | FX A send knob + nilai dB | Kirim hanya channel ini ke FX A; minimum OFF |
| Tiap source channel | FX B send knob + nilai dB | Kirim hanya channel ini ke FX B |
| Tiap source channel | Tombol `INSERT FX` + jumlah insert aktif | Membuka quick insert editor channel ini tanpa berpindah tab |
| Unit FX A/B | Program display `12 · Vocal Plate` | Menampilkan nomor dan nama aktual dari engine |
| Unit FX A/B | Searchable dropdown, prev/next, klik nomor | Browse bank; keyboard increment/decrement dan numeric entry |
| Unit FX A/B | Search/dropdown program | Filter kategori/nama/nomor; tetap di Mixer |
| Unit FX A/B | `FX ON` | Wet enable per unit; OFF tidak menambah dry |
| Unit FX A/B | Dua parameter cepat P1/P2 | Label/unit berubah berdasarkan family; angka dapat diketik |
| Unit FX A/B | `Edit` | Inline popover/drawer parameter tambahan; tidak wajib pindah tab |
| Unit FX A/B | `Return` fader, mute, stereo meter | Volume hasil efek ke main, bukan jumlah send suatu channel |
| Unit FX A/B | `Reset` | Mengembalikan parameter program aktif; tidak mengubah sends/return |
| Footer | Panic output mute | Tetap tersedia, tidak mengubah saved program |

Range send A/B: OFF atau -90 sampai +10 dB; return OFF atau -90 sampai +10 dB. Internal float bus headroom tetap terjaga; meter FX input overload memberi petunjuk menurunkan sends bila nonlinear algorithm overload.

Program browsing: keyboard/prev-next memilih highlighted candidate lalu load setelah idle 150 ms; Enter/click program melakukan load segera. Tampilkan pending state sampai ACK. Coalesce rapid requests dan hanya latest desired program diproses; jangan menumpuk 99 graph allocations. Program boundaries clamp 01–99, tidak wrap 99→01 tanpa indikasi. Numeric input 0/100/non-integer ditolak.

Mengubah parameter memberi tanda `Modified`; nomor/nama factory program tetap terlihat. Memilih program baru memuat nilai factory baru tetapi **tidak** mengubah enable state, return gain, channel sends, source assignments, recording state, atau default OS output. Kembali ke program sebelumnya memuat factory default, kecuali user menyimpan user preset. User presets bank terpisah, tidak mengubah factory IDs 01–99.

### 8B.4 Tata letak final dan responsive behavior

Revisi 1.4 menggantikan layout DSP dock vertikal lama dengan acuan bagian 8D:

- FX A dan FX B menjadi **dua baris horizontal tipis** di bawah toolbar. Nomor/nama program memakai searchable dropdown; tidak ada display angka besar atau encoder program besar.
- Area utama kiri sekitar 60% untuk channel mixer, kanan sekitar 40% untuk **Channel Processing lengkap** yang selalu terlihat: grafik EQ, Freq/Gain/Q, HPF, compressor, de-esser dan SEND A terkait.
- Harmony berada di strip Vocal serta tray bawah bank kiri. Tidak menggantikan grafik EQ atau compressor.
- Source strip memiliki INSERT FX, SEND A/B, MON/REC; master/group tidak mendapat input monitor atau arm recorder palsu.
- Tombol INSERT FX membuka quick drawer di area kiri; inspector EQ kanan dan top FX rows tetap terlihat. Vocal FX tab tetap pilihan editor detail.
- Pada 1280×800 bank channel dapat horizontal scroll; master pinned; inspector dan top rows tetap terbaca. Detail ukuran/breakpoints ditentukan 8D.3.

Program A/B dan FX per-channel tetap berbeda secara fungsi. SEND A pada inspector adalah kontrol dari parameter SEND A channel terpilih yang sama, bukan send baru. Jika program A berubah, label mengikuti nama program aktual. Bank program dan engine contract bagian berikut tetap berlaku.

### 8B.5 Bank factory 01–99 yang wajib tersedia

Bank berikut spesifikasi numerik yang harus diwujudkan sebagai asset registry checked-in, bukan dibuat random ketika app dibuka. Setiap factory item harus mempunyai expanded parameter state lengkap. Nilai P1/P2 pada tabel diinterpretasikan dengan family schema berikut.

| Family | Program | P1 | P2 | Parameter tetap tambahan |
| --- | --- | --- | --- | --- |
| Room | 01–11 | Decay, seconds | Pre-delay, ms | Algorithm room, damping 6500 Hz, width 80%, wet 100% |
| Plate | 12–22 | Decay, seconds | Pre-delay, ms | Algorithm plate approximation, damping 6000 Hz, width 100%, wet 100% |
| Hall | 23–33 | Decay, seconds | Pre-delay, ms | Algorithm hall, damping 5000 Hz, width 100%, wet 100% |
| Slapback | 34–44 | Time, ms | Feedback, percent | Identical L/R delay, HPF 150 Hz, LPF 6000 Hz, wet 100%, sync off |
| Stereo Delay | 45–55 | Time, ms | Feedback, percent | Independent L/R same time, HPF 150 Hz, LPF 6000 Hz, wet 100%, sync off |
| Ping-pong | 56–66 | Time, ms | Feedback, percent | Alternating cross-feedback, HPF 180 Hz, LPF 5500 Hz, wet 100%, sync off |
| Chorus | 67–77 | Rate, Hz | Depth, percent | Centre 12 ms, feedback 0%, stereo phase offset 90°, wet 100% |
| Phaser | 78–88 | Rate, Hz | Depth, percent | Six-stage, centre 900 Hz, feedback 20%, wet 100% |
| Delay + Plate | 89–99 | Delay time, ms | Plate decay, seconds | Two parallel wet-only branches; delay feedback 20%; plate predelay 20 ms/damping 6000 Hz; branch gains each 0.5 linear, not equal-power |

Semua family memiliki output trim default 0 dB di processor; return gain terpisah. Rate/frequency ranges mengikuti bagian 8A.3. Program master schema: `factoryBankVersion=1`, `factoryProgramId` integer 1–99. Nomor tampilan memakai dua digit. Algoritme reverb yang sama dengan setting berbeda sah sebagai program berbeda; tidak boleh hanya mengganti nama tanpa parameter/suara berubah.

Untuk Slapback stereo inlet, sum mono dengan normalization 0.5(L+R) hanya pada program family ini dan jelaskan mono character di deskripsi; output duplikasi L/R. Stereo Delay mempertahankan input L/R independen. Ping-pong menginisialisasi first echo dari normalized mono input ke kiri, feedback bergantian ke kanan dan kiri, level conservation teruji. Default manual time dapat diganti sync melalui Edit; macro Time menjadi readout Note ketika sync aktif. Simpan original manual time supaya toggle sync off memulihkannya.

| No. | Nama program | Family | P1 | P2 |
| --- | --- | --- | --- | --- |
| 01 | Tiny Booth | Room | 0.2 s | 0 ms |
| 02 | Dry Studio | Room | 0.3 s | 2 ms |
| 03 | Small Room | Room | 0.4 s | 4 ms |
| 04 | Vocal Room | Room | 0.5 s | 6 ms |
| 05 | Warm Room | Room | 0.65 s | 8 ms |
| 06 | Bright Room | Room | 0.8 s | 10 ms |
| 07 | Wood Room | Room | 0.95 s | 12 ms |
| 08 | Drum Room | Room | 1.1 s | 14 ms |
| 09 | Medium Room | Room | 1.3 s | 16 ms |
| 10 | Wide Room | Room | 1.5 s | 18 ms |
| 11 | Large Room | Room | 1.8 s | 20 ms |
| 12 | Vocal Plate | Plate | 1.4 s | 20 ms |
| 13 | Short Plate | Plate | 0.6 s | 5 ms |
| 14 | Soft Plate | Plate | 0.8 s | 10 ms |
| 15 | Bright Plate | Plate | 1 s | 12 ms |
| 16 | Warm Plate | Plate | 1.2 s | 15 ms |
| 17 | Classic Plate | Plate | 1.6 s | 22 ms |
| 18 | Wide Plate | Plate | 1.8 s | 25 ms |
| 19 | Pop Plate | Plate | 2 s | 28 ms |
| 20 | Smooth Plate | Plate | 2.3 s | 30 ms |
| 21 | Long Plate | Plate | 2.8 s | 35 ms |
| 22 | Epic Plate | Plate | 3.5 s | 40 ms |
| 23 | Small Hall | Hall | 1.2 s | 10 ms |
| 24 | Vocal Hall | Hall | 1.6 s | 15 ms |
| 25 | Warm Hall | Hall | 1.9 s | 20 ms |
| 26 | Bright Hall | Hall | 2.2 s | 25 ms |
| 27 | Concert Hall | Hall | 2.5 s | 30 ms |
| 28 | Wide Hall | Hall | 2.8 s | 35 ms |
| 29 | Deep Hall | Hall | 3.2 s | 40 ms |
| 30 | Long Hall | Hall | 3.8 s | 45 ms |
| 31 | Grand Hall | Hall | 4.5 s | 50 ms |
| 32 | Cathedral | Hall | 5.5 s | 60 ms |
| 33 | Ambient Hall | Hall | 7 s | 70 ms |
| 34 | Micro Slap | Slapback | 40 ms | 0 % |
| 35 | Tight Slap | Slapback | 50 ms | 0 % |
| 36 | Short Slap | Slapback | 60 ms | 0 % |
| 37 | Vocal Slap | Slapback | 75 ms | 0 % |
| 38 | Vintage Slap | Slapback | 90 ms | 5 % |
| 39 | Rock Slap | Slapback | 100 ms | 8 % |
| 40 | Double Slap | Slapback | 115 ms | 10 % |
| 41 | Warm Slap | Slapback | 130 ms | 12 % |
| 42 | Wide Slap | Slapback | 145 ms | 15 % |
| 43 | Long Slap | Slapback | 160 ms | 18 % |
| 44 | Echo Slap | Slapback | 180 ms | 20 % |
| 45 | Stereo 80 | Stereo Delay | 80 ms | 10 % |
| 46 | Stereo 120 | Stereo Delay | 120 ms | 12 % |
| 47 | Stereo 160 | Stereo Delay | 160 ms | 15 % |
| 48 | Stereo 200 | Stereo Delay | 200 ms | 18 % |
| 49 | Stereo 250 | Stereo Delay | 250 ms | 20 % |
| 50 | Stereo 320 | Stereo Delay | 320 ms | 25 % |
| 51 | Stereo 400 | Stereo Delay | 400 ms | 30 % |
| 52 | Stereo 500 | Stereo Delay | 500 ms | 35 % |
| 53 | Stereo 650 | Stereo Delay | 650 ms | 40 % |
| 54 | Stereo 800 | Stereo Delay | 800 ms | 45 % |
| 55 | Stereo 1000 | Stereo Delay | 1000 ms | 50 % |
| 56 | Ping 100 | Ping-pong | 100 ms | 10 % |
| 57 | Ping 150 | Ping-pong | 150 ms | 12 % |
| 58 | Ping 200 | Ping-pong | 200 ms | 15 % |
| 59 | Ping 250 | Ping-pong | 250 ms | 18 % |
| 60 | Ping 320 | Ping-pong | 320 ms | 22 % |
| 61 | Ping 400 | Ping-pong | 400 ms | 26 % |
| 62 | Ping 500 | Ping-pong | 500 ms | 30 % |
| 63 | Ping 600 | Ping-pong | 600 ms | 35 % |
| 64 | Ping 750 | Ping-pong | 750 ms | 40 % |
| 65 | Ping 900 | Ping-pong | 900 ms | 45 % |
| 66 | Ping 1200 | Ping-pong | 1200 ms | 50 % |
| 67 | Subtle Chorus | Chorus | 0.1 Hz | 10 % |
| 68 | Slow Chorus | Chorus | 0.15 Hz | 12 % |
| 69 | Warm Chorus | Chorus | 0.2 Hz | 15 % |
| 70 | Vocal Chorus | Chorus | 0.3 Hz | 18 % |
| 71 | Wide Chorus | Chorus | 0.4 Hz | 22 % |
| 72 | Soft Motion | Chorus | 0.5 Hz | 25 % |
| 73 | Pop Chorus | Chorus | 0.6 Hz | 30 % |
| 74 | Bright Motion | Chorus | 0.8 Hz | 35 % |
| 75 | Deep Chorus | Chorus | 1 Hz | 40 % |
| 76 | Fast Chorus | Chorus | 1.5 Hz | 45 % |
| 77 | Liquid Chorus | Chorus | 2 Hz | 50 % |
| 78 | Subtle Phase | Phaser | 0.1 Hz | 10 % |
| 79 | Slow Phase | Phaser | 0.15 Hz | 15 % |
| 80 | Warm Phase | Phaser | 0.2 Hz | 20 % |
| 81 | Vocal Phase | Phaser | 0.3 Hz | 25 % |
| 82 | Wide Phase | Phaser | 0.4 Hz | 30 % |
| 83 | Soft Sweep | Phaser | 0.5 Hz | 35 % |
| 84 | Classic Phase | Phaser | 0.6 Hz | 40 % |
| 85 | Bright Sweep | Phaser | 0.8 Hz | 45 % |
| 86 | Deep Phase | Phaser | 1 Hz | 50 % |
| 87 | Fast Phase | Phaser | 1.5 Hz | 55 % |
| 88 | Liquid Phase | Phaser | 2 Hz | 60 % |
| 89 | Vocal Space | Delay + Plate | 100 ms | 0.6 s |
| 90 | Short Space | Delay + Plate | 150 ms | 0.8 s |
| 91 | Warm Space | Delay + Plate | 200 ms | 1 s |
| 92 | Pop Space | Delay + Plate | 250 ms | 1.2 s |
| 93 | Wide Space | Delay + Plate | 320 ms | 1.4 s |
| 94 | Ballad Space | Delay + Plate | 400 ms | 1.8 s |
| 95 | Dream Space | Delay + Plate | 500 ms | 2.2 s |
| 96 | Long Space | Delay + Plate | 600 ms | 2.8 s |
| 97 | Deep Space | Delay + Plate | 750 ms | 3.5 s |
| 98 | Ambient Space | Delay + Plate | 900 ms | 4.5 s |
| 99 | Infinite Mood | Delay + Plate | 1200 ms | 6 s |


Nama karakter seperti Warm/Bright pada bank awal adalah nama program, bukan klaim parameter tone berbeda jika parameter tabel hanya decay/time. AI boleh menambah per-program tone override yang terdokumentasi untuk mencocokkan karakter, tetapi harus menaikkan bankVersion serta mempertahankan migrasi sesi; jangan mengubah factory values secara diam-diam. `Infinite Mood` tetap bounded 6-second plate dan feedback 20%, bukan infinite feedback/freeze.

### 8B.6 Registry, state, migration dan protocol

Buat `packages/contracts/fx-program.schema.json` dan source registry `assets/fx-programs.v1.json` dengan tepat 99 entries. Schema minimal:

```json
{
  "bankVersion": 1,
  "id": 12,
  "name": "Vocal Plate",
  "family": "Plate",
  "routingMode": "sendReturn",
  "processorRecipe": "plate_wet_v1",
  "parameters": {
    "decaySeconds": 1.4,
    "preDelayMs": 20,
    "dampingHz": 6000,
    "width": 1,
    "wet": 1,
    "outputTrimDb": 0
  },
  "macros": ["decaySeconds", "preDelayMs"]
}
```

Gunakan template family untuk menulis expanded entries saat development; runtime jangan mengandalkan index modulo atau switch-case 99 cabang. Engine memvalidasi recipe/parameter sesuai versi. Built-in catalog harus bundled dan bekerja offline.

`FxUnitState`: UUID, unit A/B, enabled, bankVersion, programId, expanded current parameter snapshot, modified flag, return gain/off/mute, input/output meter taps. `ChannelState`: dua sends target UUID masing-masing gain/off/prePost. Program ID saja tidak cukup untuk session recall; snapshot values mencegah factory update mengubah suara sesi lama.

Session migration revisi 1.1: existing FX returns dengan stable IDs dipertahankan dan dipetakan A/B jika kompatibel. Jika existing custom chain tidak cocok bank, tampilkan `CUSTOM` di unit dengan full state dipertahankan, bukan memaksanya ke program 01. Memilih factory program baru mengganti custom chain hanya melalui aksi user; undo memulihkannya. Existing aux monitor tidak dialihkan diam-diam ke DSP. Migration dibuat di control thread sebelum publish, backup schema lama tersedia.

IPC tambahan:

- `fx.programs.list`: catalog metadata dan version.
- `fx.unit.selectProgram`: unit UUID, programId, expectedRevision; ACK actual program/state.
- `fx.unit.setEnabled`, `fx.unit.setMacro`, `fx.unit.setReturn`, `fx.unit.resetProgram`.
- `channel.fxSend.set`: source channel UUID, target A/B UUID, value/off, prePost.
- `fx.unit.snapshot`: authoritative state dan supported macros.

Scalar macros/send/return mendukung automation/MIDI. Program changes v1.2 bukan sample-accurate automation parameter; boleh user action scene recall dengan boundary transaction dan crossfade, tidak menjanjikan sample accuracy. Scene program change tidak menghasilkan automation points pada semua channel fader.

### 8B.7 Program switching tanpa click dan tanpa dry leak

Program request dipersiapkan pada control thread. Buat state/processor baru, preallocate, validate, warmup jika perlu, kemudian fade old wet turun sambil new wet naik selama 30–80 ms dengan linear complementary gains. Sesuaikan latency jalur A/B bila recipe mempunyai algorithmic delay. Parameter time delay adalah intentional effect timing, bukan graph latency yang harus dihilangkan.

V1.2 tidak menjanjikan full tail spillover: old tail dipotong dengan fade transisi, lalu instance dilepas off-thread. Jangan memelihara tail dari semua program yang dilewati selama browsing. Di tengah transisi hanya satu pending latest request; bounded maksimum dua active recipe instances per unit pada audio graph. Error preparation mempertahankan old valid program dan menampilkan pesan; OFF/ON state tidak tiba-tiba berubah.

Offline renderer membaca unit snapshot yang sama dan membangun return buses wet-only. Master mixdown mencakup kedua returns. Stem export default tetap dry/post-insert per-track; shared return tidak dibagikan acak ke tiap track. Tambahkan pilihan export `FX A Return` dan `FX B Return` sebagai stems tersendiri. Soloed per-source FX stem membutuhkan render terpisah dan berada di luar versi 1.2; jangan menjanjikan main mix dapat direkonstruksi dengan menggandakan shared FX pada setiap stem.

### 8B.8 Fase MIXFX yang harus dieksekusi

Tahap MIXFX di bawah adalah integrasi native setelah UI-first. Registry metadata 99 program, dropdown, macro controls dan rows dibangun pada UI-02/UI-03 menggunakan fixtures; native recipes baru diinstansiasi pada fase-fase berikut. Jangan menunda layout karena prerequisite DSP.

#### MIXFX-00 — Registry 99 program dan contracts

**Prasyarat:** VFX-00 dan VFX-02 tersedia, Phase 10/11 routing dasar teruji.

Buat expanded registry tepat dari tabel 8B.5, schema, validators, family recipe mapping, independent unit state dan protocol fixtures. Audit macro name/unit mappings. Test unique IDs 1–99, nama tidak kosong, parameter valid, recipe available, per-program expanded state berbeda, serialization stable.

**Acceptance:** semua 99 entry dapat diinstansiasi dan disiapkan oleh native engine; `fx.programs.list` mengembalikan metadata yang sama. Jangan menganggap render sound quality sudah lulus hanya karena registry valid.

#### MIXFX-01 — Dua FX buses dan per-channel sends

**Prasyarat:** MIXFX-00 dan VFX-01.

Bangun dedicated A/B input sums, processors wet-only dan returns menuju main, default state/off dan per-channel send controls. Preserve empat aux monitor. Input/output meters terpisah dan output channels clear tetap berlaku.

**Acceptance:** channel send A tidak memengaruhi B; MUSIC send OFF benar-benar tidak ada di return; A OFF menghasilkan wet silence bukan dry feed; feedback edges ditolak; main dry amplitude tidak bertambah pada unit OFF; selected A/B state independen.

#### MIXFX-02 — Integrasi native compact FX rows dan quick insert drawer

**Prasyarat:** MIXFX-01 dan VFX-08 UI/parameter components.

Reuse layout compact UI-00–04/8D; hubungkan two program dropdowns, search/numeric entry, macros, return faders/meters, FX sends A/B dan INSERT FX ke native contracts. Reuse existing parameter controls dan instance editor; jangan duplikasi sumber state antara Mixer dan Vocal FX tab.

**Acceptance:** dari tab Mixer user memilih 12 Vocal Plate, ON, menaikkan VOICE send A, mengubah decay dan return tanpa pindah tab; user membuka Pitch Correction insert dengan channel FX; kedua DSP panels tetap terlihat; keyboard dan 1280×800 layout lulus.

#### MIXFX-03 — Switching, macro overrides, preset recall

**Prasyarat:** MIXFX-02.

Implement debounce/coalescing dan ACK reconciliation, prepared recipe switch, fade policy, modified indicator, reset macros, Custom migration, undo/redo satu program switch sebagai satu action.

**Acceptance:** perubahan program tidak mengubah sends/return/enable/route; 100 rapid program requests tidak menghasilkan unbounded allocation atau click; failed prepare mempertahankan old graph; UI menampilkan actual ACK, bukan selected candidate seolah sudah aktif.

#### MIXFX-04 — Save/open, automation/MIDI dan export

**Prasyarat:** MIXFX-03, Phase 15/16, dan VFX-08.

Simpan actual unit state/expanded snapshot/send assignments, restore schema lama, MIDI mapping macro/return/send, automation parameters stable, offline bus rendering, separate A/B return stems.

**Acceptance:** session reopen menghasilkan mix identik sesuai tolerance deterministic graph; factory catalog update tidak mengubah snapshot; render FX returns hanya sekali; monitor volume tidak masuk export; tab switching tidak mereset program atau modulation phase.

#### MIXFX-05 — QA 99 program dan release gate

**Prasyarat:** MIXFX-04, VFX-09, dan INT-02; gunakan paket uji yang sudah tersedia.

Render impulse/sine/vocal fixture untuk setiap program, simpan measured peak, duration, finite-sample checks, native recipe version. Audisi semua 99 program secara batch yang ditandai, bukan hanya 9 contoh family; bandingkan referensi fixture yang sama dengan level matching. Uji 30 menit system+mic+file dengan kedua unit aktif dan perubahan program berkala, catat CPU/deadline/xruns. Hardware tests tak tersedia harus NOT_RUN, bukan PASS.

**Acceptance:** bank tepat 99, tidak ada silent/broken program ketika diberikan input yang sesuai; family effect signature berbeda sesuai recipe; wet-only invariant, transition, session/export tests lulus; bukti pengukuran/hardware dicatat di `docs/reports/mixer-dsp99.md`. Label prototype/unverified tetap jujur sampai gate selesai.

### 8B.9 Skenario uji pengguna yang wajib

1. Start project: Mixer terbuka, A/B terlihat, sources belum monitor otomatis.
2. Pilih VOICE dari mic; dengarkan dry melalui headphone dengan input monitor eksplisit.
3. A pilih program 12 Vocal Plate, ON, return -6 dB, VOICE send -18 dB: plate terdengar.
4. MUSIC send A tetap OFF: backing track tidak mendapat plate.
5. B pilih 50 Stereo 320, ON, VOICE send B -24 dB: delay ikut terdengar tanpa hilangnya plate.
6. Putar macro Decay A: tail berubah, B time tetap 320 ms.
7. Matikan A: plate fade out, dry vocal dan B delay tetap hidup.
8. Klik FX di VOICE: quick drawer menampilkan insert; edit correction amount tanpa pindah tab.
9. Save/open: nomor, override, sends, returns dan insert pulih.
10. Export master: plate/delay masuk sekali; export return stems terpisah tersedia.

### 8B.10 Prompt eksekusi revisi 1.2

```text
Baca AUDIO-MIXER-AI-IMPLEMENTATION.md revisi 1.6. Selesaikan UI-00–04 pada bagian 8D terlebih dahulu, kemudian integrasikan bagian 8B.
Ubah Mixer utama agar memiliki dua unit FX A/FX B permanen, masing-masing dapat memilih
99 factory programs yang didefinisikan lengkap pada tabel 8B.5. Jangan membuat 99 nama dummy.
Implementasikan registry native, two independent wet-only FX send/return buses, dua send knobs
per source channel, program selector, macro controls, return faders/meters dan FX ON/OFF.
OFF pada send effect harus silence wet return, bukan dry bypass.
Tambahkan tombol FX channel yang membuka quick insert editor tanpa pindah tab;
pitch correction/formant/harmony tetap per-channel insert. Vocal FX tab hanya editor detail opsional.
Pertahankan fitur mixer existing, 4 aux monitor, VFX native, recording dan offline export.
Ikuti MIXFX-00–05, prerequisite VFX/Phase yang tertulis, schema migration dan latency rules.
Program switch tidak boleh mengubah sends, return gain, enable state atau device routing.
Preview fixture hanya pada UI Preview. Jangan hardcode telemetry mode engine atau menandai DSP selesai hanya karena layout terlihat selesai.
Jalankan tests relevan, catat hasil nyata di docs/progress.md, dan lanjutkan pekerjaan yang tidak terblokir.
```


## 8C. Revisi 1.3 — Tombol Harmony pada Mixer utama

### 8C.1 Tujuan dan cakupan

User dapat menambahkan suara harmoni saat bernyanyi dengan **satu tombol HARMONY ON/OFF langsung pada channel vokal**, tanpa memasuki tab Vocal FX atau menambah processor secara manual. Tombol ini mengendalikan effect `harmony` yang sudah diwajibkan pada VFX-06; jangan membuat algoritme atau instance tambahan yang tidak sinkron.

Tetap pertahankan FX A/B dan 99-program bank. Harmony adalah processor channel tersebut, bukan program bersama A/B. Mengubah Harmony VOICE tidak mengubah MUSIC, sumber lain, atau program A/B. Harmony bukan auto-key detection; key/scale yang dipilih selalu terlihat pada panel pengaturan cepat.

Source channel mempunyai `contentRole: vocal | instrument | music | other`. Source microphone baru default vocal sebagai pilihan UI yang dapat diubah, bukan inferensi bahwa semua input hardware adalah penyanyi. Track file vocal dapat ditandai Vocal. Tombol hanya ditampilkan pada source strip ber-role Vocal; semua source yang sesuai format tetap dapat memasang harmony lewat insert catalog. Bus dan master tidak mendapat shortcut Harmony otomatis.

### 8C.2 Layout yang wajib dibuat, bukan sekadar opsi

1. Pada strip VOICE, tepat sesudah tombol **INSERT FX** dan sebelum PAN, tambahkan satu baris **HARMONY** beserta power indicator dan status **ON/OFF**. Di sebelah kanan baris sediakan ikon gear untuk pengaturan. Pada source non-vocal gunakan spacing konsisten; jangan memasang tombol Harmony pada MUSIC hanya demi simetri.
2. Tombol minimal hit area sekitar 32 px tinggi; label tetap terbaca. ON memakai aksen violet dengan teks ON dan ikon power; OFF redup dengan teks OFF. Warna bukan satu-satunya indikator.
3. Gear membuka tray **VOICE · HARMONY** di bawah bank channel pada tab Mixer yang sama. Gear tidak mengubah ON/OFF. Klik toggle tidak memaksa tray terbuka.
4. Tray menyediakan Key, Scale, Harmony mode, Voice 1 interval, Voice 2 interval, Harmony Level, dan status ON/OFF yang terikat ke engine. Tombol Advanced membuka editor detail opsional.
5. Nilai awal yang tampak: `Key C`, `Scale Major`, `Voice 1 +3rd`, `Voice 2 +5th`, `Harmony Level 0.0 dB`. Lead vocal tetap dipertahankan dan voice gains internal -12/-15 dB; knob level 0 dB berarti tidak mengubah kedua level dasar, bukan kedua voice masing-masing full level.
6. Default effect OFF pada project baru; mockup boleh menggambarkan ON sebagai contoh interaksi yang sudah dilakukan user. Key C Major adalah default manual, bukan hasil analisis lagu; helper text singkat “Sesuaikan key/scale dengan lagu”.
7. Dua compact FX A/B rows di atas dan main output controls tidak tertutup tray; Channel Processing kanan tetap terlihat. Pada 1280×800, tray dapat scroll/collapse dan channel bank horizontal scroll; HARMONY pada strip tetap terlihat walau tray ditutup.
8. Label tombol FX umum menjadi **INSERT FX**, dua send knob menjadi **SEND A** dan **SEND B** untuk membedakan dari Harmony channel dan DSP bersama.
9. Keyboard Space/Enter mengaktifkan toggle saat fokus; accessible name menyebut channel dan state. Role vocal berubah saat ada harmony tidak menghapus processor; hanya mengubah shortcut visibility dan menawarkan akses lewat insert.

Komponen yang dibuat/reuse:

- `ChannelHarmonyButton.tsx`: status authoritative dan pending, tooltip unavailable.
- `HarmonyQuickPanel.tsx`: key/scale/interval/level controls.
- `useChannelHarmony.ts`: command binding, snapshot reconciliation; bukan pemilik DSP state.
- `ChannelRoleControl.tsx`: source contentRole, persist dalam session.
- Reuse editor/key scale controls dari `features/vocal-fx`; jangan membuat dua set nilai default berbeda.

### 8C.3 Semantik satu tombol dan instance ownership

Setiap source channel maksimum satu **primary harmony instance** untuk shortcut, ditandai `primaryHarmonyInstanceId`. Instance ini mengambil satu slot dari batas 8 creative rack slots; bukan hidden ninth slot.

- Klik ON pertama: cari instance harmony yang sudah ada. Jika tepat satu, bind dan enable. Jika lebih dari satu, buka pilihan instance untuk primary; jangan menyalakan semuanya. Jika belum ada, engine membuat satu instance pada posisi yang valid sebelum reverb/delay/stereo widening dan mengembalikan ID-nya.
- Saat rack penuh, reject dengan `FX_RACK_FULL`, pertahankan audio/rack lama dan tampilkan instruksi melepas satu slot. Jangan mengganti efek lain otomatis.
- Jika format/reorder prefix tidak kompatibel, laporkan `HARMONY_FORMAT_UNSUPPORTED` dengan saran posisi; jangan mono-sum stereo input secara diam-diam.
- Double-click cepat/race dari tab lain tetap idempotent: native command memakai desired enabled state, bukan blind toggle. Revision conflict memicu snapshot refresh.
- Klik OFF mem-fade **harmony voices saja** menuju silence 20–40 ms. Lead vocal tetap mengalir sekali dengan level yang sama, fader/send/record arm tidak berubah.
- Pada loaded session, primary instance dipersiapkan sebelum engine ready. First-add saat audio aktif menjalankan prepared graph transition dengan latency alignment; status `Preparing` tetap OFF/pending sampai ACK ready, bukan indikator ON palsu.
- Saat ON→OFF, pertahankan algorithmic delay lead path dan graph latency supaya timing tidak melompat. Processor warm state boleh dipertahankan untuk toggle cepat; OFF bukan jaminan CPU usage menjadi nol.
- Remove/release instance lewat Advanced adalah tindakan terpisah, dapat mengubah latency lewat graph transaction. Jangan menyamakan OFF dengan remove.
- Unvoiced/low-confidence mengikuti VFX-06: harmony voices fade out, lead vocal tetap. Tidak ada harmony key otomatis yang diklaim dari backing track.

Semantik output primary harmony:

`output = alignedLead + enabledRamp * harmonyLevelGain * (voice1Gain * shiftedVoice1 + voice2Gain * shiftedVoice2)`

Setiap voice mencakup pan yang dinormalisasi dan shifter latency alignment. Mixer downstream melakukan format negotiation sesuai 8A.5. Lead tidak di-gain ulang oleh `Harmony Level`. Pada primary harmony, global insert mix dikunci pada format **Lead + Voices**; UI memakai voice levels/master harmony level, bukan wet/dry kedua yang dapat menggandakan lead. Rack editor dan shortcut menggunakan semantik enable yang sama, bukan standard insert crossfade yang membuat lead menghilang.

### 8C.4 Parameter cepat dan state

| Parameter | Rentang/default | Catatan |
| --- | --- | --- |
| Enabled | false pada project baru | Recall mengikuti session saved state |
| Key | C–B; default C | Manual, ditampilkan jelas |
| Scale | Major / Natural Minor / Chromatic / Custom; default Major | Diatonic third/fifth menggunakan Major/Minor/Custom; Chromatic memakai interval semitone Fixed agar makna third tidak ambigu |
| Mode | Diatonic / Fixed; default Diatonic | Mode change memvalidasi interval |
| Voice 1 | Diatonic +2 steps = +3rd; default -12 dB | Bukan selalu +4 semitone |
| Voice 2 | Diatonic +4 steps = +5th; default -15 dB | Bukan selalu +7 semitone di semua posisi scale |
| Harmony Level | OFF atau -30 sampai +6 dB; default 0 | Trim tambahan atas voice levels; tidak mengubah lead |
| Voice pan | -0.5 dan +0.5 default | Advanced dapat diubah |
| Formant preserve | true default | Sesuai backend capability dan quality tests |

Simpan role, primary instance ID, enabled state dan parameter di authoritative session schema. Key/scale di tray dan rack merupakan parameter **instance yang sama**, bukan mirrored mutable copies. Setiap channel vocal punya instance/key/scale/level sendiri. Jangan memaksa semua vocal mengikuti global song key tanpa opsi link yang benar-benar diimplementasikan.

Protocol tambahan:

```json
{"protocolVersion":1,"id":"h-1","type":"command","method":"channel.harmony.setEnabled","params":{"channelId":"ch-voice","enabled":true,"expectedRevision":42}}
```

ACK setelah prepared instance dipublish: instanceId, applied revision, applied sample frame, enabled dan latency samples. Tambahkan `channel.harmony.configure` untuk parameter batch tervalidasi, `channel.harmony.bindPrimary` untuk existing rack, serta `channel.harmony.snapshot` untuk reconciliation. Error: rack full, role/source unsupported, format unsupported, backend unavailable, revision conflict. Tidak ada optimistik “audio aktif” tanpa event engine.

Monitor Fast tidak boleh menampilkan ON seolah harmony terdengar jika heavy processor sedang dilewati. Tampilkan `Not in Monitor Fast` dan aksi mengganti profile; pertahankan status terpisah `requestedEnabled`, `effectiveInMonitor`, `effectiveInRecord`. Setting/record graph tetap mengikuti aturan 8A.10. Jangan diam-diam mengganti mode monitoring karena tombol ditekan.

### 8C.5 Tahapan HARM yang wajib masuk progress

Tahap HARM di bawah memverifikasi native binding setelah UI-first. Layout tombol, gear, dan quick tray dibuat dahulu pada UI-02/UI-03 tanpa menunggu VFX-06; statusnya saat itu UI-only.

#### HARM-00 — Audit instance binding dan state

**Prasyarat:** VFX-06 serta rack contracts, MIXFX-02 layout tersedia.

Implement contentRole, primary instance binding, desired-state native command, 8-slot limit handling, session migration dan parameter defaults tunggal. Existing single harmony instance dipakai ulang; existing multiple instances memerlukan pemilihan primary eksplisit.

**Acceptance:** 100 repeated desired ON requests menghasilkan satu instance, bukan 100; state berbeda antar-channel; program A/B tidak berubah; role switching tidak menghapus DSP atau media.

#### HARM-01 — Tombol strip dan quick panel

Reuse HARMONY button/power state dan gear hasil UI-00–04; hubungkan quick tray controls 8C.4 ke native engine, accessible keyboard, pending/failed/unsupported states. Reuse rack parameter widgets dan subscription; preserve compact rows A/B dan Channel Processing pada semua ukuran dukungan.

**Acceptance:** tanpa meninggalkan Mixer, user klik ON, membuka gear, mengubah key/scale dan level, lalu OFF. Close tray tidak menghapus state. Ganti selected channel tidak menyalakan Harmony channel lain. Mock engine tests dilabeli UI-only.

#### HARM-02 — Transisi suara dan sinkronisasi

Bind ke native harmony renderer, enable ramp wet voices, lead alignment dan latency policy, automation desired-state events, recording/offline snapshot dan undo/redo. Update rack editor untuk primary harmony Lead + Voices semantics.

**Acceptance:** OFF menghasilkan lead tetap terdengar sekali dan level stabil; ON menambahkan dua voice interval yang benar; toggles tidak click atau menyebabkan playback jump; Harmony Level tidak memengaruhi lead. Monitor Fast menunjukkan effective state yang jujur. Slot full/backend failure tidak merusak audio yang sedang berjalan.

#### HARM-03 — QA tampilan dan audio end-to-end

**Prasyarat:** HARM-00–02, VFX-09, MIXFX-05 dan INT-02; Phase15/16 sudah mendahului implementasi ini.

Uji note fixture known key, vocal recording nyata, transition audio, session save/open, primary instance delete/rebind, undo/redo, audio-system+music berjalan bersamaan, UI tab switching dan 1280×800. Catat hasil `docs/reports/harmony-button.md` serta screenshot Mixer desktop.

**Acceptance:** seluruh kriteria 8C.6 terpenuhi dengan hasil nyata. Jika hardware/pendengaran tidak tersedia, status NOT_RUN tetap tertulis dan jangan menandai overall VERIFIED.

### 8C.6 Checklist yang wajib diperiksa sebelum handoff

- [ ] Pada Mixer, strip VOICE mempunyai tombol berlabel HARMONY dengan ON/OFF dan gear.
- [ ] Tidak perlu membuka Vocal FX untuk mengaktifkan harmony atau mengubah key/scale/level.
- [ ] FX A/B masih terlihat dan berfungsi independen; 99-program bank tidak diganti.
- [ ] ON menambahkan dua suara, OFF hanya menghilangkan harmoni, lead tetap.
- [ ] Key/scale manual tampak, default tidak diklaim hasil auto-detect.
- [ ] Shortcut, rack dan quick tray menunjuk primary instance yang sama.
- [ ] Multiple clicks tidak menambah processor duplikat; rack capacity tetap dihormati.
- [ ] Harmony Level hanya memengaruhi tambahan suara harmoni.
- [ ] Lead alignment dan latency tidak berubah mendadak saat toggle biasa.
- [ ] Per-channel, session recall, undo/redo, record/export dan Monitor Fast teruji.
- [ ] Tombol tidak aktif palsu ketika engine/backend unavailable.
- [ ] Screenshot/mockup merupakan referensi tampilan; bukan bukti audio engine sudah berjalan.

### 8C.7 Prompt AI coding untuk penambahan Harmony

```text
Baca AUDIO-MIXER-AI-IMPLEMENTATION.md revisi 1.6: UI-00–04 pada bagian 8D dahulu, lalu native binding bagian 8C.
Tambahkan tombol HARMONY ON/OFF dan gear langsung di strip channel Vocal pada Mixer utama.
Gear membuka quick panel Key, Scale, Voice 1/2 interval dan Harmony Level tanpa pindah tab.
Pertahankan FX A/B, bank 99-program, SEND A/B dan fungsi INSERT FX.
Binding harus ke satu primary native harmony instance per channel; jangan membuat toggle UI dummy
atau instance ganda setiap diklik. ON menambah harmony voices; OFF fade voices saja, lead tetap.
Ikuti role/slot/format/latency policy, parameter defaults, engine ACK, session migration dan tests 8C.
Eksekusi HARM-00–03 berdasarkan prerequisite, update docs/progress.md, dan verifikasi layout serta audio.
Jika environment tidak dapat menjalankan audio Mac, laporkan batasnya tanpa mengarang hasil uji.
```


## 8D. Revisi 1.4 — Layout-first dan acuan visual final

### 8D.1 Prioritas dan arti selesai

**Arahan pengguna: pertama buat layout mirip gambar terakhir, kemudian kerjakan engine-engine audio.** Bagian ini menggantikan urutan engine-first dan tata letak versi lama bila ada pertentangan. Jangan menjalankan pekerjaan C++/Core Audio/DSP sebagai syarat untuk mulai menampilkan mixer.

Deliverable tahap pertama adalah aplikasi UI React yang dapat dijalankan, bergaya sesuai referensi, dengan interaksi simulasi yang koheren. Bukan gambar screenshot yang dijadikan background, bukan tombol HTML tanpa state, bukan wireframe generik, dan bukan laporan bahwa engine sudah selesai.

Urutan wajib:

1. **UI-00:** audit frontend ringan dan design contract.
2. **UI-01:** scaffold UI desktop/preview serta state/adapter.
3. **UI-02:** bangun semua bagian layout Mixer utama sesuai referensi.
4. **UI-03:** interaksi dan simulasi state lengkap, tanpa backend audio.
5. **UI-04:** screenshot, visual QA, layout/interaction gate → UI_VERIFIED.
6. **Native dan integrasi:** ikuti tabel 8E.1, termasuk Phase05 sebelum Phase04 serta Phase15/16 sebelum VFX.
7. **INT-00–03:** verifikasi semua alur aplikasi, paket uji dan kelengkapan fitur.
8. **Phase19:** final package/handoff setelah seluruh gate wajib terpenuhi.

Untuk repo existing, simpan engine yang sudah ada dan gunakan isolasi adapter. Larangan membangun engine sebelum UI gate bukan instruksi menghapus engine existing atau mengubah public contracts yang sudah dipakai. Audit kode secukupnya boleh agar UI contract tidak bertentangan.

Tidak membutuhkan persetujuan baru untuk setiap fase. Layout yang dijadikan referensi sudah disetujui dalam percakapan; gate di sini adalah pemeriksaan hasil kode terhadap desain. Setelah objektif UI gate lulus, lanjut ke native secara otomatis bila tugas implementasi penuh masih aktif.

### 8D.2 Referensi dan batas kesetiaan gambar

Referensi utama adalah mockup compact terakhir dengan Freq/Gain/Q, CLIP, MON/REC, Modified dan SEND A terkait. Nama file lokal asal pada percakapan: `exec-4a3c50a5-1261-4330-87a6-0579c31ec3d7.png`.

[Pratinjau referensi layout yang disetujui](sandbox:/workspace/scratch/7008f00b378e/generated_images/exec-4a3c50a5-1261-4330-87a6-0579c31ec3d7.png)

Saat dipakai di repository lain, path scratch ini tidak dijamin tersedia. Jika user menyediakan gambar, simpan sebagai `docs/design/approved-mixer-layout.png` dan gunakan untuk perbandingan; jangan membuat gambar baru lalu mengklaimnya reference approved. Jika file tidak tersedia, teruskan berdasarkan spesifikasi detail 8D.3–8D.6 dan catat `REFERENCE_IMAGE_UNAVAILABLE`; jangan berhenti pada permintaan attachment sebelum membuat UI yang dapat diperiksa.

Ikuti komposisi, hierarki dan karakter visualnya. Kesalahan kecil gambar AI tidak menjadi spesifikasi: misalnya MON/REC pada GROUP harus dihilangkan karena bukan input source; grafik EQ harus berasal dari nilai state UI, bukan menyalin kurva ilustrasi yang tidak konsisten. Shelf slope fixed tidak diganti dropdown bebas yang belum diimplementasikan. Prioritaskan semantik tertulis jika bertentangan dengan label gambar.

### 8D.3 Layout, ukuran, dan design tokens

Reference viewport: **1680×945 CSS pixels**, lalu validasi minimum **1280×800**. Gunakan CSS grid/flex dan minmax, bukan absolute positioning semua kontrol.

| Region | Susunan final |
| --- | --- |
| Header | Tinggi sekitar 52 px; title, Mixer/Vocal FX/Timeline/Routing tabs, transport, time, sample rate, engine/preview status |
| FX A row | Tinggi 42–46 px; amber badge, ON/OFF, searchable program dropdown, Modified, dua quick controls, return slider+meter, Edit |
| FX B row | Tinggi 42–46 px, struktur sama; cyan accent |
| Main workspace | Dua kolom kira-kira 60:40, gap 10–12 px; `min-height:0` agar internal scroll bekerja |
| Left bank | SYSTEM, VOICE, GUITAR, MUSIC, GROUP 1, MASTER; source strips scroll horizontal, master pinned |
| Right inspector | VOICE / Channel processing; EQ graph+controls, compressor, de-esser dan linked SEND A; tidak ditutup Harmony |
| Harmony tray | Bawah bank kiri; sekitar 80–100 px; ON, key/scale, 2 voices, level, Advanced |
| Footer | Sekitar 40–44 px; physical output picker, monitor volume, DIM/MUTE |

Desktop source strip sekitar 145–165 px; master sekitar 140–155 px. Pada 1280×800, inspector tetap sekitar 430–500 px, bank menampilkan lebih sedikit source strips lewat scroll; jangan shrink semua fader dan label sampai sulit dibaca. Dua FX rows tetap di atas; gunakan layout kolom compact, dropdown minimal sekitar 200 px, atau overflow Edit untuk kontrol sekunder. ON/program/return tetap terlihat dan kedua parameter cepat dapat diakses. Tinggi main memakai ruang tersisa; inspector dapat internal scroll vertikal pada minimum, namun EQ graph dan compressor tidak diganti tab tersembunyi. Footer tidak hilang.

Semua kontrol reusable mengikuti bagian 4A; jangan menulis knob/fader/meter baru di tiap panel. Design tokens awal: background #081217; panel #101F27; inset #0B171D; border #28414B; text #EAF2F5; muted #9DB2BA; cyan #18D6E7; amber #F3C842; harmony violet #B862F0; danger #F04E4E. Font Inter/system sans, body 12–14 px, heading 16–20 px; tabular numerals untuk dB/time. Radius kecil 4–8 px, shadow tipis. Hindari cards besar, neon berlebihan, gradient dekoratif atau font micro yang tidak terbaca.

Meter: green normal → yellow near peak, CLIP latch red; judul indikator jelas. Fader cap silver dengan track gelap dan tick scale; implement HTML/CSS/SVG nyata. Graph EQ boleh SVG/canvas dengan axis log frequency; bukan bitmap. Knob memakai pointer/wheel/keyboard serta numeric input alternatif. Semua controls perlu focus ring dan accessible labels.

### 8D.4 Detail kontrol yang tidak boleh hilang

**Source strip:** nama/source, trim, EQ/COMP/GATE/INSERT FX, pan, SEND A/B, fader dB, stereo/mono meter, CLIP reset, M/S, MON/REC. Voice punya tombol HARMONY dan gear terpisah sesudah INSERT FX. Group dan master tidak mendapat MON/REC input; master punya limiter indicator. Dalam preview, source type adalah label fixture; tidak mengklaim device tersebut benar-benar terdeteksi.

**EQ:** empat bands LOW shelf, MID 1 peaking, MID 2 peaking, HIGH shelf. Editable Freq/Gain di semua band; Q hanya kedua mid; label Shelf fixed untuk LOW/HIGH. Values fixture: LOW 100 Hz/+3 dB; MID 1 350 Hz/-2.5 dB/Q1.20; MID 2 2.5 kHz/+2 dB/Q1.00; HIGH 10 kHz/+4 dB; HPF on/80 Hz. Drag node horizontal→Freq, vertical→Gain; Q melalui numeric/keyboard. Curve preview berasal dari parameter/filter-response math UI, tidak memproses audio dan tidak menjadi engine kedua. Fixture curve harus konsisten dengan state, bukan hardcoded path.

**Compressor:** threshold -18 dB, ratio 3:1, attack 10 ms, release 120 ms, simulated reduction meter dengan penanda preview pada mode simulasi. De-esser 6 kHz tetap di bawah.

**Linked send inspector:** panel kanan bawah berjudul `SEND A · Vocal Plate`, nilai -18 dB sama dengan VOICE SEND A. Edit dari salah satu tempat mengubah state yang sama; ganti channel atau program A memperbarui judul/nilai. Bukan parameter `reverbSend` baru.

**Compact FX rows:** A=12 Vocal Plate; fixture override decay 1.8 s sehingga Modified tampil; predelay20 ms; return -6 dB. B=50 Stereo320; time320 ms/feedback25%; return -12 dB. Factory program A sendiri tetap decay1.4 s; fixture override tidak mengubah bank 99. Reset A memulihkan1.4 dan menghapus badge. Memilih program tidak mereset send/return/enable. Dropdown memuat semua 99 entries dengan search number/name/category.

**Harmony:** pada visual reference fixture ON/key C/Major/third+fifth/master level0 dB; session baru real mengikuti default OFF 8C. Role vocal mengontrol shortcut visibility. MON indicator selected VOICE `MONITOR ON` pada fixture; pada preview jangan mengklaim input sedang terdengar.

**CLIP/peak hold:** simulated fixture history deterministik, klik reset menghapus latch, peak hold menyatu meter. Selama native belum terhubung, label app Preview selalu tampak; fixture bukan pengukuran nyata.

### 8D.5 PreviewAdapter dan kontrak state sebelum engine

Buat `MixerControlPort` TypeScript interface untuk commands/snapshots/subscriptions. Implement `PreviewAdapter` sekarang dan `NativeAdapter` setelah UI gate. React tidak mengimpor fake data secara langsung di setiap component. State per channel/unit menggunakan stable IDs, parameter units dan boundaries yang sama dengan kontrak native.

Suggested modules:

- `features/mixer/MixerPage.tsx`, `ChannelBank.tsx`, `ChannelStrip.tsx`, `MasterStrip.tsx`.
- `features/fx/CompactFxRow.tsx`, `ProgramPicker.tsx`.
- `features/processing/ChannelProcessingPanel.tsx`, `ParametricEqPanel.tsx`, `CompressorPanel.tsx`, `LinkedSendControl.tsx`.
- Reuse `ChannelHarmonyButton.tsx`, `HarmonyQuickPanel.tsx` dari 8C.
- `components/audio/Fader.tsx`, `RotaryControl.tsx`, `LevelMeter.tsx`, `ClipIndicator.tsx`, `NumericParameter.tsx`.
- `adapters/MixerControlPort.ts`, `PreviewAdapter.ts`, lalu `NativeAdapter.ts` pada Phase02.
- `fixtures/approvedMixerSession.ts`, `fixtures/previewTelemetry.ts`; no random data in production component.

Preview mode wajib mempunyai badge **UI PREVIEW** serta tooltip/status `Audio engine belum terhubung`. Fixture rate48k boleh dilabeli session target, bukan actual negotiated hardware rate. Engine Ready pada gambar tidak disalin sebagai klaim engine ready saat preview. CPU/latency yang tidak diukur menampilkan `—`, bukan angka perkiraan.

Preview interactions boleh meniru ACK, validation, pending/error dan engine unavailable. Event berupa simulated-control state. Transport dapat menggerakkan clock demonstrasi; REC hanya record-arm preview, jangan membuat file palsu atau toast “Recording saved”. File picker dapat menampilkan metadata media untuk layout, tetapi jangan memulai decode/capture/pemrosesan sebelum fase audio. Tidak memasang BlackHole, tidak meminta mic permission, tidak mengubah default output OS pada UI phase.

`dev:ui` memilih PreviewAdapter secara eksplisit. Mode Native gagal jelas bila engine tidak ada; tidak fallback otomatis. Semua write ke UI state melalui adapter/store tunggal sehingga SEND A terkait, selected-channel inspector, Harmony dan preset Modified tidak divergen.

### 8D.6 Fase UI yang dikerjakan pertama

#### UI-00 — Audit frontend dan design contract

Baca existing repo instructions/status, audit Node/npm dan frontend versions. Tetapkan module boundaries dan reusable UI control inventory berdasarkan bagian 4A; jangan memulai dengan satu MixerPage monolitik. Pin frontend dependencies yang kompatibel Node24.15.0. Dokumentasikan final regions, tokens, interactions dan paths; jangan menunggu JUCE/CMake/Xcode tersedia. Buat progress berisi urutan UI-00–04 sebelum engine phases. Jika project existing, map components yang bisa dipakai ulang.

**Acceptance:** frontend setup jelas, acuan 8D dipilih sebagai final; old large DSP dock tidak masuk design; tidak ada dependency native yang menjadi syarat menjalankan preview.

#### UI-01 — Scaffold dan state adapter

Bangun workspace React/TS/Vite, Electron shell/main/preload bila belum ada. Sediakan browser preview untuk visual QA pada Linux serta Electron UI runner bila lingkungan mendukung; keduanya memakai component tree yang sama. Ini bukan penggantian target desktop menjadi website. Terapkan Electron security defaults dari Phase01 sekarang.

Scripts minimum: `dev:ui`, `dev:desktop:ui`, `typecheck`, `test:ui`, `build:ui`, `test:visual`, `check:file-size`, `check:architecture`. Implement line-count guard dan dependency rules bagian 4A sejak UI-01. `dev:ui` browser preview tidak butuh native executable; desktop UI entry memakai badge yang sama. Bangun MixerControlPort, fixture session dan PreviewAdapter. Header/toprows/main skeleton/footer dapat dirender.

**Acceptance:** UI berjalan dan resize; preview mode eksplisit, tidak ada Core Audio call atau native spawn failure yang memblokir layar.

#### UI-02 — Layout Mixer fidelity

Bangun reusable primitives dan audio controls dahulu, lalu susun semua region final 8D.3/8D.4 secara utuh. MixerPage hanya komposisi layout; state/adapters/math/fixtures di modul masing-masing. Prioritaskan Mixer page, bukan fitur audio atau editor advanced terlebih dahulu. Katalog program metadata99 boleh dibangun dari tabel8B saat ini; processor recipes cukup metadata sampai fase native. Implement SVG EQ graphic dan control labels, fader/meter component, Harmony tray, CLIP, MON/REC serta linked send panel.

Tabs selain Mixer boleh memperlihatkan struktur workspace berlabel preview atau disabled dengan penjelasan. Jangan habiskan fase ini pada full timeline editing sebelum Mixer utama sesuai.

**Acceptance:** semua elemen penting terlihat sesuai screenshot; tidak ada big FX dock kanan, EQ lengkap tidak hilang, Harmony tidak menggantikan INSERT FX, master pinned, source bank scroll.

#### UI-03 — Interaksi preview koheren

Implement select channel, knob/fader/numeric input, reset/defaults, EQ drag+Q, menu program99/search, ON/OFF/macro/return, Modified reset, Harmony toggle/gear/key/scale/level, MON/REC simulated state, CLIP reset, linked SEND A, Edit popover dan keyboard focus. Simulasi mode pending/error/revision untuk reconciliation; stable deterministic demo meter yang bisa freeze untuk screenshots.

**Acceptance:** perubahan VOICE tidak mengubah MUSIC; SEND A strip/inspector selalu sama; changing A preset tidak reset return/sends; Modified sesuai actual override; EQ controls/curve konsisten; power/arm controls menunjukkan state. Semua ini UI tests, bukan DSP correctness tests.

#### UI-04 — Visual QA dan gate sebelum engine

Render screenshot 1680×945 dan1280×800. Simpan di `docs/reports/ui/` dengan nama meaningful. Bandingkan terhadap approved image bila tersedia dan matriks 8D.7. Periksa hasil screenshot secara visual, bukan hanya berhasil menjalankan build. Perbaiki overlap, cropped controls, field terlalu kecil, missing labels, graph hilang dan unreadable text sebelum selesai.

Jalankan typecheck, UI build, behavioral tests yang relevan, screenshot script, `check:file-size`, dan `check:architecture`. Tidak ada first-party code file lebih dari 1.000 baris. Visual snapshot exact-pixel hanya dipakai pada fixture/font/viewport deterministik; jangan membuat automated expected screenshot dari output yang belum diinspeksi lalu menganggap desain tervalidasi.

**Acceptance:** seluruh UI gate lulus, hasil disimpan `docs/reports/ui-layout-review.md`, status **UI_VERIFIED**. Buat checklist mapping tiap component ke native endpoint yang akan dihubungkan. Setelah itu lanjut Phase00 native audit. Bila browser visual runner tersedia tetapi Electron Mac tidak, gate web-rendered UI bisa lulus dengan `Desktop Mac shell: NOT_RUN`; ini tidak boleh menjadi klaim native/hardware VERIFIED.

### 8D.7 UI gate checklist dan handoff ke native

- [ ] Dua FX rows compact selalu di atas; tidak ada right-side DSP modules besar.
- [ ] Mixer bank kiri, complete Channel Processing kanan, Harmony tray kiri bawah.
- [ ] EQ graph, HPF, Freq/Gain/Q, compressor, de-esser dan linked SEND A terlihat.
- [ ] HARMONY dan INSERT FX dua tombol berbeda pada channel Vocal.
- [ ] CLIP/peak hold, MON/REC, MONITOR ON dan Modified memiliki makna state yang benar.
- [ ] Simulasi UI berlabel; audio/capture/recording nyata tidak diklaim berjalan.
- [ ] Shared state linked send, per-channel settings dan per-unit FX isolation lulus tests.
- [ ] Keyboard/numeric input dapat dipakai tanpa drag; focus states terbaca.
- [ ] 1680×945 dan1280×800 diinspeksi; tidak ada overlap/halaman terpotong yang menyembunyikan kontrol penting.
- [ ] Graph/reference-image source tidak dibundel sebagai screenshot-background pengganti komponen.
- [ ] UI report menyebut apa yang verified, native belum started, dan next exact action.

Saat NativeAdapter ditambahkan, pertahankan layout. Perbedaan capability perangkat ditampilkan sebagai status/disabled control, bukan alasan merombak desain approved. Replace fixture telemetry dengan native subscription dan hapus badge Preview hanya sesudah real handshake berhasil. UI gate tidak menggantikan DSP/hardware gates pada fase berikut.


## 8E. Audit kelengkapan dan rencana penyelesaian kanonik — revisi 1.6

### 8E.1 Satu urutan eksekusi, dependency dan bukti

Dokumen ini berisi revisi historis; **tabel berikut adalah urutan/dependency final**. Layout tetap bagian 8D, modularisasi 4A, parameter/audio contracts sesuai bagian terkait dengan penegasan 8E. Nomor Phase tidak berarti harus selalu diurutkan numerik. Tidak ada dependensi ke fase yang lebih belakangan dalam tabel.

Saat UI-00, buat `docs/task-plan.json` dari tabel ini: id, dependsOn, status, domain, evidencePaths. Tambahkan `check:plan` ke verify/CI untuk memeriksa unique IDs, missing IDs, cycles, serta kesesuaian seluruh fase wajib dengan progress. Bukti capability-unavailable hanya boleh menutup cabang bersyarat yang ditentukan 8E.12; missing hardware bukan bukti fitur tidak didukung.

| Urut | Phase ID | Prasyarat langsung minimum | Hasil yang membuka fase berikut |
| --- | --- | --- | --- |
| 01 | UI-00 | — | Audit frontend dan desain final |
| 02 | UI-01 | UI-00 | Preview shell, contracts, modularity guards |
| 03 | UI-02 | UI-01 | Layout compact lengkap |
| 04 | UI-03 | UI-02 | Interaksi preview koheren |
| 05 | UI-04 | UI-03 | UI_VERIFIED dengan screenshot |
| 06 | Phase00 | UI-04 | Dependency native dipin |
| 07 | Phase01 | Phase00 | Native executable terhubung shell |
| 08 | Phase02 | Phase01 | Protocol/engine supervisor |
| 09 | Phase03 | Phase02 | Audio device, gain/mute/limiter dasar |
| 10 | Phase05 | Phase03 | Channel/fader/multi-source dasar |
| 11 | Phase04 | Phase05 | System routing nyata dan recovery |
| 12 | Phase06 | Phase04 | File streaming dan transport |
| 13 | Phase07 | Phase06 | Meter/filter/EQ |
| 14 | Phase08 | Phase07 | Dynamics |
| 15 | Phase09 | Phase08 | De-esser dan vocal strip |
| 16 | Phase10 | Phase09 | Bus/monitor routing |
| 17 | Phase11 | Phase10 | Reverb/delay/ducking dasar |
| 18 | Phase12 | Phase11 | Timeline/editing/count-in |
| 19 | Phase13 | Phase12 | Recording dan take alignment |
| 20 | Phase14 | Phase13 | Offline export/loudness/PDC |
| 21 | Phase15 | Phase14 | Persistence/recovery/preset schema |
| 22 | Phase16 | Phase15 | Automation dan native MIDI input |
| 23 | VFX-00 | Phase16 | Catalog/interfaces final |
| 24 | VFX-01 | VFX-00 | Rack runtime/alignment |
| 25 | VFX-02 | VFX-01 | Space/modulation DSP |
| 26 | VFX-03 | VFX-02 | Drive/doubler |
| 27 | VFX-04 | VFX-03 | Pitch/formant backend |
| 28 | VFX-05 | VFX-04 | Pitch correction |
| 29 | VFX-06 | VFX-05 | Harmony voices |
| 30 | VFX-07 | VFX-06 | Vocoder/MIDI |
| 31 | VFX-08 | VFX-07 | Advanced UI/presets/native binding |
| 32 | MIXFX-00 | VFX-08 | Registry99 + native recipes |
| 33 | MIXFX-01 | MIXFX-00 | FX A/B wet-only buses |
| 34 | MIXFX-02 | MIXFX-01 | Compact rows native binding |
| 35 | MIXFX-03 | MIXFX-02 | Program transitions/overrides |
| 36 | MIXFX-04 | MIXFX-03 | Persistence/automation/export A/B |
| 37 | HARM-00 | MIXFX-04 | Primary Harmony binding |
| 38 | HARM-01 | HARM-00 | One-button/native UI |
| 39 | HARM-02 | HARM-01 | Harmony transition/alignment |
| 40 | Phase17 | HARM-02 | Plugin hosting/runtime integration |
| 41 | Phase18 | Phase17 | Taps/capability audit |
| 42 | INT-00 | Phase18 | Semua UI menu/commands connected |
| 43 | INT-01 | INT-00 | Daily journeys + recording/replay |
| 44 | INT-02 | INT-01 | Dev .app/package + permissions teruji |
| 45 | VFX-09 | INT-02 | Final native FX/audio/latency evidence |
| 46 | MIXFX-05 | VFX-09 | 99 programs dan dual-unit QA |
| 47 | HARM-03 | MIXFX-05 | One-button harmony final QA |
| 48 | INT-03 | HARM-03 | Full traceability dan release checklist |
| 49 | Phase19 | INT-03 | Final clean package dan handoff |


Tabel sengaja menggunakan rantai minimum konservatif untuk eksekusi serial. Bila hardware blocker muncul, AI boleh mengerjakan unit/module independen yang input contract-nya tersedia, tetapi tidak boleh menandai integration gate sebagai lulus atau diam-diam melewati fitur. VFX-09/MIXFX-05/HARM-03 menggunakan paket uji INT-02; Phase19 adalah final packaging/handoff sehingga tidak ada lingkaran packaging→test→packaging.

### 8E.2 Audit gaps yang diperbaiki dan coverage fitur

| Temuan audit | Perbaikan wajib | Owner phase |
| --- | --- | --- |
| Routing test memakai fader sebelum channel engine ada | Phase05 sebelum04; physical/injected source untuk05 | 03/05/04 |
| Test tone membutuhkan limiter sebelum Phase14 | Output gain/mute/sample limiter dasar sejak03 | 03,14 |
| Fase FX meminta persistence/MIDI yang baru datang sesudahnya | Phase15/16 sebelum VFX; tests native MIDI lebih awal | 15/16,VFX |
| Paket diminta untuk FX QA sebelum final packaging | Dev package gate INT-02 sebelum final FX QA | INT-02,19 |
| MON/REC/Stop belum memiliki semantik lengkap | Source/monitor/recording state machine8E.3–4 | 05/06/13 |
| Frame domain dan perubahan rate bisa membuat clip bergeser | Project-frame source-frame distinction dan strict rate policy | 06/13,8E.5 |
| Window pitch awal kurang konservatif untuk range rendah | 4096 default, efficient detector, range-specific tests | VFX-04–06 |
| Knob cepat dapat bertabrakan di global revision | Coalescing, command IDs, scoped revisions dan ACK policy | 02,8E.7 |
| Layout Mixer selesai tetapi menu lain masih preview | Feature traceability dan INT-00 wajib | INT-00 |
| Stereo harmony/PDC bisa mengubah level lead | Format/lead pan normalization dan impulse/level tests | VFX-06,HARM-02 |
| UI input formats lebih luas dari codec yang dibundel | Format matrix dan packaged import tests | 06/14,INT-02 |
| Semua label VERIFIED belum menjamin aplikasi selesai | Explicit product acceptance8E.12 dan evidence per domain | INT-03,19 |

Buat `docs/feature-traceability.md`, satu row per fitur/control penting: requirementId, UI entrypoint, command/API, native owner, save field, unit test, integration test, manual evidence, status. Jangan mengisi seluruh kolom dengan satu file `engine.cpp` atau satu universal mock test.

Coverage minimum mencakup: setup/devices/permissions; add/remove/rename/source roles; import/transport/timeline/editing; strip/filter/dynamics; aux/subgroup/DCA/solo; 12 native effects;99 programs;Harmony button; record dry/processed/master; codecs/export/return stems;session/undo/autosave/relink;automation/MIDI;AU/VST3;per-app capture capability;status/error/recovery;packaging/offline use. Fitur mandatory yang belum diuji bukan `N/A`.

### 8E.3 Semantik source, MON, REC, transport dan solo

Pisahkan **device engine clock** yang terus berjalan dari **timeline playhead** yang dapat stop/pause. Menghentikan lagu tidak boleh mematikan mic/system processing.

| Aksi | Perilaku final |
| --- | --- |
| Play | Mulai/resume file timeline; live mic/system tetap berjalan |
| Pause | Freeze playhead dan hentikan pemberian sample file baru; live sources tetap; FX tails yang sudah ada decay |
| Stop transport | Stop file playback, kembali ke playback-start position; jika recording berjalan, finalize recording dahulu; live mic/system dan system route tetap |
| Stop Engine / Disable routing | Flush recording sesuai workflow, hentikan audio device dan restore system route yang dimiliki aplikasi |
| MON pada live source | Mengizinkan suara live source masuk jalur audition/main mix dan sends; OFF memutus kontribusi itu, tidak menghentikan capture/armed recording tap |
| REC pada live source | Arm track untuk recording; tidak otomatis mengaktifkan MON dan tidak mulai recording tanpa global Record |
| File track playback | Mengikuti transport, tidak bergantung MON; MON/REC live-input hanya tersedia jika explicit record-input assignment ada |
| Global Record | Mulai session recording; record mode menentukan master/armed tracks/both |
| Mute | Menutup channel mix/sends sesuai5.3; dry/processed armed recording tetap diambil |
| Footer monitor mute/dim | Mengubah headphone/monitor path saja; tidak memute recorded master |

**Penting:** MON OFF bisa berarti mic tidak ada di master recording karena main mix mic tidak diaktifkan, meskipun dry/processed armed track tetap direkam. UI tooltip menjelaskan perbedaan ini. Default mic MON OFF; system route dibuat oleh aksi user `Enable System Audio`, yang juga menawarkan/menyetel system strip MON ON agar audio sistem yang dialihkan terdengar. Jangan menyembunyikan perubahan tersebut.

Default file-only session tidak meminta mic permission. Sample preview VOICE MON ON bukan default semua real sessions. File-only strips tampil tanpa MON/REC atau disabled dengan alasan sampai record-input assignment dipilih; group/master tidak memiliki input MON/REC. Native state source kind lebih menentukan capability daripada nama channel.

Solo button S default PFL; setting monitor menawarkan PFL/AFL. PFL/AFL tidak mengubah main/export dan tidak mengubah MON/record-arm. Untuk mic MON OFF, PFL merupakan audition eksplisit ke monitor bus, tidak membuka main send. Solo-in-place tersedia sebagai mode terpisah: dim/mute non-solo channel contribution ke main dan FX sends, tetap pertahankan bus/return dependencies dari solo source; dry armed tracks tetap direkam. Simpan scopeMode pada session/preferences dengan badge agar user tahu bila master recording terpengaruh. Jangan membuat S kadang solo-in-place kadang PFL tanpa indikator.

DCA gain menjumlah dalam dB untuk memberships, mute efektif bila salah satu DCA mute; batasi jumlah memberships dan tampilkan effective gain. Pan mono equal-power; ketika Harmony mono→stereo, lead dipan center dengan -3.01 dB per side di dalam widening agar tombol ON tidak menambah3 dB lead. Semua returns/routes mempertahankan energy convention yang sama.

### 8E.4 Recording lengkap sampai dapat diputar ulang

Recording lifecycle: IDLE → PREPARING → RECORDING → STOPPING → SAVED atau PARTIAL/FAILED. Record button memiliki mode Master / Armed Tracks / Both. Untuk Armed Tracks tanpa arm, tampilkan error dan tidak membuat file kosong. Mute/solo/monitor state memengaruhi master sesuai graph, bukan dry-track capture.

1. Pilih destination folder writable dan record format sebelum mulai; persiapkan writer/buffer/file handles off callback. Global record baru aktif setelah ACK ready.
2. Default record bersamaan transport: jika stopped, start timeline dari cursor (setelah count-in bila aktif); jika playing, punch-in di boundary yang dicatat. Pause saat recording ditolak dengan pesan singkat pada versi ini; Stop recording/transport selalu finalize. Count-in tidak direkam pada dry/master kecuali explicit print-click.
3. Version1 tidak melakukan loop-take recording. Loop playback tetap ada; record-start dengan loop aktif meminta user mematikan loop atau menampilkan `LOOP_RECORD_UNSUPPORTED`; jangan menimpa take tiap putaran.
4. Simpan captureStartDeviceFrame, startProjectFrame, frameCount, tapPoint, channelMap, sampleRate, DSP latency dan user calibration offset. Semua writer memakai callback frame boundary yang sama. Processing/timestamp offsets bukan menambah atau menghapus sample diam-diam.
5. Recording ke timeline memakai device→project mapping yang ditetapkan pada start; latency input/output dan processed tap dikalibrasi lewat loopback lalu disimpan sebagai metadata yang bisa diperiksa. Jangan memakai waktu wall-clock sebagai posisi clip.
6. Saat selesai, register media UUID dan buat clip take pada track hasil yang terpisah sehingga existing source file tidak ditimpa. `Dry`/`Processed` pada nama/metadata jelas. Processed take di replay channel dengan inserts neutral secara default; jangan menerapkan Harmony/EQ yang sudah tercetak dua kali.
7. Untuk Both dry+processed dari source yang sama pada fitur lanjutan, record selections explicit; versi wajib cukup pilihan dry atau processed per armed source plus master opsional. Take source/raw media tidak dihapus oleh undo clip insertion; cleanup orphan memakai aksi terpisah yang dapat ditinjau.
8. Disk penuh/writer lag/engine disconnect menandai take PARTIAL dengan jumlah frame valid/discontinuity, tidak `Saved` palsu. Recording error notification tidak menghentikan audio callback dengan I/O.
9. Project close/open/quit saat recording mengikuti Stop-and-save workflow; jangan melepas graph/file lebih dahulu. Crash recovery memulihkan header file/segment manifest best-effort dan mempertahankan file mentah.
10. Sesudah capture sukses, user bisa tekan Play pada take, mendengar hasil, lalu export. Ini acceptance wajib, bukan sekadar existence file WAV.

Dry input/per-channel processed tap tidak memuat shared FX A/B returns; master record memuatnya sekali. Output stems merupakan per-tap exports, tidak dijanjikan menjumlah persis ke master nonlinear setelah compressor/limiter. Pilihan stem pre/post-fader wajib tertulis; jangan membiarkan default ambigu.

### 8E.5 Rate, streaming, timeline dan resource budget

- Realtime project rate default48k, supported44.1k/48k; request device rate yang sama lalu baca actual. Jika actual berbeda, jangan jalankan existing project pada rate baru diam-diam. Tampilkan error/remap; file-only offline export tetap tersedia tanpa device.
- User-initiated project-rate conversion adalah transaksi saat stopped/not recording: resample media mapping dan semua frame positions dengan rational mapping serta consistent rounding. Jika conversion UI belum diimplementasikan, buat project baru dengan rate yang didukung; jangan mengklaim conversion tersedia.
- Timeline start/duration/fades/automation memakai **project frame domain**; source offset memakai **source frame domain** dengan source rate tersimpan. Rate converter worker melakukan fractional position mapping; seek tidak menganggap offset project=offset source.
- File streaming per active source mempunyai bounded queue dan read-ahead; default start buffer sekitar250 ms, total PCM cache budget256 MiB, waveform cache terpisah64 MiB in RAM. Sesuaikan via measured profile/ADR jika perlu; jangan cache seluruh lagu panjang di RAM. 32-track stress memakai file disk nyata, bukan hanya silent nodes.
- Underflow playback: output silence untuk frame yang hilang, increment per-source counter dan tandai discontinuity. Jika terus-menerus, pause file transport dengan error; live device thread tetap tidak block. Jangan memperlambat seluruh project clock agar mengikuti disk.
- Waveform min/max peaks dihitung worker dan dikirim sebagai bounded preview data, bukan streaming PCM ke React. Peak cache keyed media fingerprint/source rate/channel mapping dan invalidated saat media berubah.
- Startup/shutdown media workers punya cancellation dan timeout; pending import/export queue bounded. Jangan menjalankan puluhan FFmpeg processes bersamaan; default decode concurrency2, export jobs1 dengan queue bounded.
- Native callback allocation checks dan timing instrumentation tidak boleh membuat benchmark menjadi blocking. Jangan memanggil debug logger/file writer untuk setiap sample/block; aggregate counters off-thread.
- Clip overlap, mono/stereo conversion, end-of-file, seek beyond duration, negative positions dan missing media mempunyai validation tests eksplisit. Posisi invalid ditolak/clamp sesuai documented command contract, bukan crash.

### 8E.6 DSP implementation decisions yang tidak boleh diserahkan ke label UI

**Reverb:** knob decay seconds adalah parameter product yang perlu algorithm/calibration. JUCE Reverb adalah implementasi reverb dengan parameters sendiri; jangan mengasumsikan ia mempunyai setter RT60 seconds. [JUCE Reverb reference](https://docs.juce.com/master/classjuce_1_1Reverb.html). Pilihan implementasi: measured mapping room-size/damping ke decay, atau native FDN dengan delay-line feedback berdasarkan target decay. Fase VFX-02 mencatat pilihan dan tests impulse decay per family; Plate label berarti approximation terdefinisi, bukan hanya rename Room dengan state identik.

**Pitch/harmony latency:** LiveShifter menyediakan fixed block dan start-delay; dokumentasinya menyebut delay50ms atau lebih sehingga backend ini tidak cukup untuk klaim vocal monitoring low-latency. [LiveShifter latency](https://breakfastquay.com/rubberband/code-doc/classRubberBand_1_1RubberBandLiveShifter.html). Studio FX boleh memakai backend tersebut; label `Real-time processing` tidak sama dengan `comfortable live monitoring`. Hasil uji bernyanyi actual wajib dicatat. Bila latency tidak cocok untuk tujuan user, jangan menandai live Harmony final selesai; profile low-latency membutuhkan backend lain yang memenuhi API/accuracy/performance, dengan tradeoff terdokumentasi. Jangan menghapus suara harmoni lewat Monitor Fast lalu mengklaim Harmony live sudah berfungsi.

**Detector:**2048 samples pada48k tidak otomatis cukup untuk seluruh70Hz target bila estimator butuh beberapa period/window. Default4096 diperiksa dengan70/80/100/220/440/880Hz fixtures serta unvoiced segments; implementation cost juga harus diprofilkan. Perhitungan input analysis timestamp, shifter warmup dan control delay dijumlahkan satu kali, tidak digandakan maupun disembunyikan.

**Dynamics/limiter:** define detector peak/RMS pada metadata dan tests; static compressor gain-law diuji dengan steady constant-envelope fixture setelah settled, sementara speech/sine transients diuji terpisah. Sample-peak limiter dasar dan true-peak limiter final punya label/capability berbeda. CLIP indikator digital tidak mengklaim dapat mendeteksi semua analog preamp clipping.

**Harmony format:** node menerima satu lead mono dan menghasilkan stereo lead+voices dengan energy-preserving center pan. Tidak boleh ada mono-only Formant setelah stereo Harmony tanpa negotiated conversion. Jika source stereo vocal, minta explicit mono channel extraction atau gunakan genuinely stereo-capable backend yang teruji; jangan silent downmix antiphase.

### 8E.7 Commands, revision, concurrency dan save consistency

Buat `packages/contracts/commands/` dengan schema tiap domain; implement TS validator/native validator terhadap fixtures yang sama. Tambahkan commands yang belum tercakup envelope awal: channel input/role/monitor assignment; clip add/move/trim/split/fade/gain; markers/metronome/count-in; parameter gesture begin/end; automation modes; save-as/collect-media; source device-map edits; clip/record job metadata; monitor panic/solo-mode. Nama exact dipin dalam command manifest pada Phase02 dan diperluas oleh owner phase, bukan diarang terpisah di React/native.

- `requestId` unik dalam engine session; retry structural/job-start request harus idempotent dengan dedupe cache bounded. Timeout berarti unknown outcome, bukan pasti gagal: query job/snapshot sebelum retry record/export/add instance.
- Initial `expectedRevision` semantics adalah scope entity revision untuk mutation (channel, FX unit, project structure), bukan global revision yang diincrement meter. Snapshot membawa global serial dan per-entity revisions. Semua examples memakai revision scope entity yang dimutasi.
- Continuous slider drag: maximaal satu request/batch in-flight per entity, coalesce pending latest value. ACK memperbarui revision sebelum berikutnya dikirim; pointer release flush nilai terakhir. Simultaneous channelA/channelB tidak saling membuat revision conflict. Automation engine runtime tidak mengubah configuration revision tiap sample.
- Scalar gesture dan program changes diproses deterministik per entity. Jika program berubah, obsolete parameter IDs dari prior recipe ditolak; UI reconcile ke current recipe. Undo satu gesture adalah satu entry; tidak ribuan sample-level edits.
- Control ACK `accepted` berbeda dari `appliedAtFrame`: callback mengirim small completion token via bounded queue, control thread serialize applied response. Jangan serialisasi JSON dari callback.
- Session save saat audio berjalan mengambil immutable **configuration snapshot**, termasuk automation lanes, bukan membaca mutable filter state langsung. Plugin getState dilakukan sesuai threading requirements plugin host, bukan blocking callback.
- Snapshot/file open uses validation→prepare→commit. Failure leaves previous session intact; no partial routing or overwritten media refs. Native invalid commands get typed errors, renderer pending state dibersihkan.
- Central engine lifecycle ownership: control thread mengatur graph/device; JUCE/macOS message-thread-dependent plugin editor/state operations dimarshall ke message loop yang benar. Native console executable tetap harus menyediakan message loop untuk plugin UI. Jangan membuka editor dari stdin reader thread.

### 8E.8 Semua entrypoint UI wajib terhubung

Tidak perlu menambah panel permanen yang merusak layout approved. Gunakan menu project, gear/settings, context menu channel, tabs existing dan popover untuk fungsi berikut.

| Entry point | Aksi nyata minimum | Gate |
| --- | --- | --- |
| Project menu | New/Open/Save/Save As/recent/collect media/recovery | Phase15, INT-00 |
| Add channel/source | Hardware/system/file source, role, mono/stereo map, rename/remove | Phase05/06 |
| Import / drop file | Job progress/cancel/error, clip di timeline, waveform, playback | Phase06/12 |
| Timeline | Play/seek/loop/markers/trim/split/fades/undo/redo | Phase12 |
| Channel Processing | EQ/HPF/LPF/gate/compressor/de-esser dan linked send | Phase07–09 |
| Routing tab | Aux/subgroup/DCA assignments, pre/post, physical outputs, cycle validation | Phase10 |
| Recording controls | Arm, MON, global record mode, folder/format, take creation/replay | Phase13 |
| Export dialog | Format/range/tail, stems/master, progress/cancel/result location | Phase14 |
| Preferences / gear | Devices/rate/block, permissions, route setup/restore, monitor/solo profile | Phase03/04/10 |
| Vocal FX/INSERT FX | Add/edit/move/remove/bypass/preset/A-B comparison | VFX-08 |
| MIDI/settings | Input device, learn/unlearn/pickup, disconnected mapping status | Phase16 |
| Plugins/settings | Scan/rescan/blacklist/editor/missing plugin restore | Phase17 |
| Diagnostics/help | Native version, source/device state, xrun counts, logs location, manual route recovery | INT-00/02 |

INT-00 harus mengecek seluruh tabel. UI preview tabs/disabled placeholders boleh selama UI phase, tetapi tidak boleh menjadi final implementation untuk fitur wajib. Transport controls tidak tergantung renderer animation timing; reflected state berasal dari engine clock.

### 8E.9 Codecs, paths, permissions dan packaging

Import minimum release lokal: WAV PCM/float, AIFF, FLAC, MP3 dan M4A AAC non-DRM. Probe actual decoder capability pada pinned native/FFmpeg build dan uji fixture masing-masing; extension saja tidak cukup. Export wajib WAV24-bit/WAV32-float dan FLAC24-bit; float→FLAC memerlukan quantization/dither yang jelas karena bukan float format. MP3 export hanya jika encoder yang dipilih benar-benar bundled/diuji dan dilaporkan supported. Jangan bergantung pada FFmpeg Homebrew untuk end-user.

Startup codec probe/missing binary harus memberi error yang actionable; working directory/resource path tidak diasumsikan. Spawn converter dengan argv/shell:false; filename ber-spasi/Unicode/leading dash diuji. Tidak menjalankan URL playlist/network input: import file lokal saja pada desain ini. Cache cleanup tidak menghapus source media/recording milik user.

macOS packaging wajib membuktikan identitas bundle/helper yang meminta permission microphone/capture. Ini harus diuji melalui `.app` yang dikemas, bukan hanya binary terminal. Permission denied/revoked/error ditangani; file-only mode tetap berjalan. Unit tests tidak dapat menggantikan TCC testing. Plugin editor memakai native window dan correct message loop; nested helper executable/dylib architecture/path/rpath diperiksa.

INT-02 membuat **dev package** dari source build sekarang menggunakan packaging scripts yang disiapkan, lalu menguji izin dan offline resources. Phase19 membangun final artifact dari commit/revision yang sama setelah bug fixes, verifies metadata/checksum dan memberi instruksi install. Jangan menunggu signing/notarization credential untuk menghasilkan local development build yang bisa ditinjau; public distribution tetap conditional dan tidak dipublish otomatis.

### 8E.10 Fase integrasi akhir yang tidak boleh dilewati

#### INT-00 — Connect seluruh produk, bukan hanya Mixer

**Prasyarat:** Phase18 serta UI/native feature phases sesuai tabel8E.1. Buat native-mode feature traceability per8E.8. Jalankan menu/file/device/settings/timeline/routing/MIDI/plugin flows dengan real adapter. Rekonsiliasi status sample source/real source agar screenshot fixtures tidak muncul sebagai device nyata.

**Acceptance:** semua entrypoint wajib mempunyai handler+native owner+test; native mode tidak memanggil PreviewAdapter, dummy waveform/meter/record file. Empty/error/loading states jelas; modularity/file-size guards lulus.

#### INT-01 — End-to-end penggunaan harian dan data integrity

**Prasyarat:** INT-00. Jalankan skenario8E.11 mulai session kosong: import backing, mic monitor, A/B effects, harmony, record, stop, playback take, save/open, export, offline playback hasil. Uji fault injection slow disk/disconnect/invalid file/cancel dengan output files dan logs yang diperiksa. Jangan memakai demo session seeded untuk melewati source setup.

**Acceptance:** audio masuk→diproses→terdengar→direkam→dibuka ulang→diekspor terbukti; no doubled lead/FX, positions/duration benar, critical failure tidak menghapus hasil rekaman. Kalau mic tidak tersedia, raw integration fixture test dicatat terpisah dari hardware flow yang belum lulus.

#### INT-02 — Paket development dan permission smoke test

**Prasyarat:** INT-01. Buat `.app` arm64 dan archive dev, jalankan dari lokasi install baru/working directory berbeda. Uji devices/permissions, native helper launch, bundled codec, presets99, save/open, plugin editor dan disconnect recovery. Matikan network setelah dependencies/install tersedia, lakukan import/record/export.

**Acceptance:** paket berjalan tanpa Vite/VSCode/Python/global FFmpeg; role/proc permission terdokumentasi; package tests benar-benar dijalankan pada Mac. Artefak dihasilkan sebelum VFX-09 agar tidak ada prerequisite melingkar. Report menyertakan exact build revision dan binary architecture.

#### INT-03 — Audit penerimaan keseluruhan

**Prasyarat:** VFX-09, MIXFX-05, HARM-03 dan INT-02. Cocokkan seluruh feature table 2.2 dengan traceability; verifikasi 99 catalog,12 effect families, semua UI modules, source formats dan failure flows. Ringkas mandatory pass, conditional unsupported, unresolved defects/performance, native tests dan manual evidence secara terpisah. Jangan memindah mandatory feature ke roadmap untuk menutup checklist.

**Acceptance:** hanya fitur conditional yang boleh CAPABILITY_UNAVAILABLE dengan bukti. Core functionality dan Harmony integration harus tested. Hasil audit `docs/reports/product-acceptance.md` menjadi input Phase19; blocker critical berarti belum PRODUCT_VERIFIED.

### 8E.11 Skenario final dan hasil yang diperiksa

| ID | Skenario dari session kosong | Bukti wajib |
| --- | --- | --- |
| E2E-01 | File-only: import MP3/WAV, play/seek, EQ/fader, export WAV | Heard output, duration/sample format, no mic prompt |
| E2E-02 | System: browser/QuickTime→BlackHole→engine→headphones | One processed path, mute/trim nyata, route restore |
| E2E-03 | Hybrid: mic+backing, vocal A Plate/B Delay, MUSIC sends OFF | Vocal effects terdengar, backing tidak ikut FX |
| E2E-04 | Harmony ON/OFF, key/scale, level, reopen panel/channel switch | Intervals benar, lead level stabil, no duplicate instance, measured delay |
| E2E-05 | Record armed dry dan master; MON/REC terpisah; stop/replay | Valid takes, alignment, master berisi FX sekali |
| E2E-06 | Processed vocal take diimpor ulang | No double Harmony/EQ, playback sesuai recorded processing |
| E2E-07 | Save/open/collect-media/move folder/relink | Actual audio/routing/preset state pulih; media tidak hilang |
| E2E-08 | Export master + FX A/B return stems | Valid files, tail policy, format/dither, monitor volume tidak tercetak |
| E2E-09 | Device unplug/sleep/wake/engine crash/full app kill | Recovery policy terbukti, tidak feedback, partial recording ditandai |
| E2E-10 | Automation+MIDI+plugin state | Values/time mapping benar, missing plugin tidak merusak project |
| E2E-11 | Clean packaged app offline | Native executable/codecs/assets lengkap, no dev server dependency |
| E2E-12 | 60-minute baseline +32-track stress dan program switches | CPU/memory/deadline/xruns/actual buffer terlapor, no hidden rate changes |

Audition user/manual diperlukan untuk naturalness Harmony, clicks dan monitoring comfort; kode AI tidak bisa mengklaimnya dari waveform fixture saja. Rekaman/screenshot/command output harus dirujuk sesuai actual environment.

### 8E.12 Arti project selesai dan batas bersyarat

**UI_VERIFIED:** layout/interaksi preview sesuai. **ENGINE_VERIFIED:** algoritme/commands diuji native. **PRODUCT_VERIFIED:** seluruh fitur mandatory dan E2E flows lulus pada target Mac dan paket nyata, critical defects nol, reports/artifact tersedia. **RELEASE_SIGNED:** status tambahan bila signing/notarization dilakukan. Jangan menukar istilah ini.

Wajib untuk complete local app: semua base channel/routing/file/record/export/session/automation/native DSP99/Harmony/UI features yang ditetapkan, AU/VST3 host diuji setidaknya pada plugin test kompatibel, codecs import di 8E.9, output wired dan device recovery. Partial app bisa diserahkan sebagai progress, tetapi tidak diberi label selesai penuh.

Conditional secara eksplisit: per-app taps pada OS/API yang mendukung (Phase18 tetap mengimplementasikan adapter dan availability check, bukan skip kode); multi-output physical bila interface punya channels; signed public release bila credentials tersedia; MP3 export bila encoder tersedia;96kHz optional. Missing test device di lingkungan AI berarti NOT_RUN/BLOCKED_ENVIRONMENT, bukan otomatis CAPABILITY_UNAVAILABLE. BlackHole setup di Mac tetap prasyarat system mode; tidak dibutuhkan file-only mode.

Performance target yang meleset harus terlihat dalam product report. Harmony yang hanya terdengar dengan delay mengganggu belum memenuhi klaim live singing nyaman; ini tetap risiko implementasi yang harus diukur/diselesaikan, bukan dijamin oleh dokumen. Jangan menghapus existing scope atau mengganti fixture pass sebagai human acceptance.

Handoff minimum: source+lockfiles+native dependency pins, runnable `.app`/archive pada Mac, README setup/system routing/recovery, command scripts, docs/progress+feature-traceability+task-plan, tests dan reports, contoh project kecil dengan generated media, known limitations, exact revision/checksum. Jika lingkungan tidak dapat membangun Mac artifact, katakan deliverable tersebut belum tersedia dan proyek belum memiliki final Mac acceptance.

### 8E.13 Hasil audit spesifikasi saat revisi ini

Audit ini memperbaiki dependensi dan menutup gap definisi proses, menu, data integrity, permission/build dan acceptance. Pemeriksaan dokumen meliputi IDs/dependency cycles, semua 99 program, semua 49 phase IDs dan prompt terakhir. **Belum ada hasil build/audio/hardware yang dapat disimpulkan dari audit Markdown ini.** Risiko backend pitch latency, device compatibility dan plugin runtime tetap harus dibuktikan saat implementasi.

## 9. Quality gates dan pengujian bermakna

Gunakan bukti UI, native-unit, native-integration, hardware, serta packaged-app secara terpisah sesuai 8E. Tidak ada status whole-product VERIFIED dari unit tests saja.

### 9.1 Fixture dan toleransi

Buat fixture generated: silence, impulse, mono/stereo sine, multi-tone, sweep, speech-like amplitude envelope, dual-tone sidechain. File pendek cukup untuk unit test; soak file dibuat saat test atau streaming generator.

| Test | Ekspektasi awal |
| --- | --- |
| Gain unity bypass | Max abs error ≤ 1e-6 untuk float passthrough tanpa resample |
| Gain dB | Amplitude ratio error ≤ 0.01 dB setelah ramp settled |
| Mono pan center | L/R masing-masing sekitar -3.0103 dB, tolerance 0.02 dB |
| EQ passband/band gain | Tolerance 0.2 dB pada titik yang terdefinisi, hindari transient |
| Compressor steady state | Tolerance 0.5 dB setelah settling, detector definition tetap |
| Routing | Tiap source sampai hanya ke destination yang diharapkan |
| Limiter sample peak | Ceiling tidak terlampaui di luar tolerance float pada adversarial fixture |
| Recording | Frame count sesuai interval, tidak ada gap yang tidak dilaporkan |
| Latency compensation | Impulse parallel paths sejajar ≤ 1 frame pada built-in graph |
| Protocol | Invalid type/range/revision ditolak; framing partial line benar |
| Session | Semantic round-trip; file corruption tidak merusak previous good |

Target toleransi dapat diperketat/diubah melalui ADR dengan alasan numerik, bukan dilonggarkan diam-diam agar test hijau. Jangan snapshot implementasi sebagai expected value; gunakan formula, analytical response, atau fixture referensi independen.

### 9.2 Benchmark pada M2 16 GB

Profil baseline: 48 kHz, actual block 256, wired output, system stereo + mic + 8 file stereo tracks, EQ/compressor per active strip, 1 reverb + 1 delay, plugin pihak ketiga off.

Target desain:

- Soak 60 menit tanpa xrun yang terdeteksi dan tanpa gap yang terdengar pada jalur uji; laporkan keduanya terpisah.
- Callback p99 < 50% block deadline pada baseline; laporkan maximum dan overrun count.
- Memory seluruh aplikasi steady-state ditargetkan < 1.5 GB pada baseline; cache dibatasi dan file panjang streaming.
- GUI meter 20–30 Hz tanpa mengirim PCM; drag fader responsif tanpa click audio.
- Round-trip wired ditargetkan ≤ 25 ms pada baseline tanpa high-latency effects; ukur loopback nyata. Ini target, bukan hasil yang sudah terbukti.
- Stress profile 32 strips dilaporkan terpisah pada 256/512 frames; jangan menyembunyikan gagal 256 dengan hanya menunjukkan hasil 512.

Jika deadline tidak tercapai, profil callback dan graph lebih dahulu. Hindari mengganti arsitektur besar tanpa bukti bottleneck. Menambah buffer dapat menjadi mode stabil yang ditampilkan jelas kepada user.

### 9.3 Manual acceptance matrix

| Skenario | Hasil yang dibuktikan |
| --- | --- |
| Browser + QuickTime | Satu system mix, control nyata, tidak double audio |
| Mic + backing track | Voice fader/EQ independen; input monitoring explicit |
| Dua app via taps | Fader per-app independen dan own-output exclusion |
| Output USB dicabut | Ramp/stop aman, state error dan remap tersedia |
| Sleep/wake | Device re-open terkendali, audio tidak feedback |
| Permission deny/revoke | Pesan actionable, file mode masih bisa dipakai |
| Engine crash | UI menunjukkan crash, route restore oleh supervisor bila memungkinkan |
| Full app force kill | Manual recovery dan recovery marker terbukti |
| Long recording | Segment/RF64, writer lag, disk-full behavior benar |
| Session moved | Media relink/collect berhasil |
| Packaged app | Native helper permission dan executable launch berhasil |
| UI busy/minimized | Engine terus berjalan; telemetry drop tidak block audio |

## 10. UX yang harus dibuat

**Acuan final adalah bagian 8D revisi 1.4.** Implementasikan seluruh layout utama lebih dahulu; referensi UX lama hanya berlaku bila konsisten dengan dua compact FX rows di atas dan inspector kanan.

Window utama minimal nyaman pada 1280×800, resizable:

- Top bar: project name, save/dirty indicator, transport, time, record, actual rate/buffer, engine status.
- Mixer area: channel bank horizontal scroll, color/name/source, input meter, trim, processing indicators, fader dB, mute/solo/arm/monitor, pan, output destination.
- Inspector: channel EQ/dynamics/sends/inserts; nilai angka dapat diketik, double-click reset, unit selalu terlihat. Tombol FX setiap channel membuka quick insert drawer tanpa berpindah tab; compact FX A/B rows tetap terlihat.
- Dua compact FX rows permanen pada Mixer: FX A/B masing-masing menampilkan program 01–99 lewat dropdown, Modified badge, parameter cepat, ON/OFF dan return/meter. Tidak ada DSP dock besar di kanan; kanan untuk Channel Processing.
- Master/monitor area: meter, limiter reduction, output selector, master fader, monitor volume/dim/mute.
- Timeline tab: track list, clips, markers, playhead, zoom, loop, waveform yang dihasilkan worker.
- Routing tab: tabel source/destination/pre-post untuk presisi; graph opsional untuk overview.
- Settings: audio backend/device/channel map, buffer/rate, permissions, media/cache paths, recording format, MIDI/plugins.

Warna meter saja tidak cukup: sertakan nilai/clip indicator. Tombol mute/solo/record mempunyai label dan keyboard focus. Meter tidak boleh memicu seluruh React tree render 30 kali/detik. UI optimistic parameter edit harus reconcile ACK/error; jangan menampilkan setting yang ditolak engine sebagai sudah aktif.

Mode setup pertama: pilih File Only atau System/Hybrid, pilih physical output, konfigurasi source, validasi route, baru start audio. Jangan memaksa BlackHole installation untuk pengguna yang hanya ingin mixer file.

## 11. Build/run contract untuk repository hasil

Script UI dibuat pada UI-00/UI-01 sebelum native build tersedia. Kontrak script native di bawah dibuat pada Phase 01 setelah UI_VERIFIED; ini bukan command yang sudah tersedia dari dokumen saja.

Mulai UI tanpa native engine:

```bash
npm ci
npm run dev:ui
```

Validasi UI:

```bash
npm run typecheck
npm run test:ui
npm run build:ui
npm run test:visual
```

```bash
npm ci
npm run doctor
npm run build:native
npm run typecheck
npm run test:native
npm run test:ui
npm run dev:engine
```

Verifikasi dan packaging di Mac:

```bash
npm run verify
npm run test:e2e
npm run build
npm run package:mac:unsigned
```

`verify`: check:plan + check:file-size + check:architecture + typecheck + portable/native relevant tests + build; hardware acceptance tetap terpisah dan tidak diberi status PASS otomatis. `test:e2e` membedakan mock-engine suite dan real-engine suite dalam output. Pin JUCE dependency sebelum CMake configure; cache download agar build berikutnya tidak memerlukan jaringan.

Jika toolchain belum ada, README harus memberi command instalasi yang sesuai Mac aktual beserta alasan. Jangan menjalankan installer privilege tinggi, mengganti global Node, atau mengubah system audio routing sebagai bagian tersembunyi dari `npm install`.

## 12. Isi instruksi AI repository

Buat `.github/copilot-instructions.md` dan ringkasan `AGENTS.md` yang konsisten dengan aturan berikut:

```text
Build Local Audio Mixer according to docs/specs/AUDIO-MIXER-AI-IMPLEMENTATION.md.
Use docs/progress.md and the canonical dependency plan in section 8E.1 to resume.
Track every UI control and required capability through command, native owner and test evidence.
Complete INT-00–03 and the actual packaged-app acceptance; never equate UI/build success with product completion.
Apply section 4A to UI, Electron, adapters, native engine, scripts and tests.
Build reusable small UI primitives/controls; compose panels/pages from them.
Keep each first-party code file at or below 1000 physical lines including comments/blanks.
Prefer focused 60–300 line UI controls/modules and focused native header/implementation pairs.
Run check:file-size and check:architecture; split responsibilities before accepting oversized files.
Never evade the limit with minification, massive includes, part1/part2 files, or unchecked exclusions.
FIRST complete UI-00–04 from section 8D against the latest approved compact layout.
Do not build native audio/DSP before the UI_VERIFIED gate. Preserve any existing engine code.
PreviewAdapter is allowed only in visibly labelled UI Preview mode; no audio/recording claims.
After UI_VERIFIED, continue native phases and replace the adapter without redesigning the UI.
Target macOS Apple Silicon. Keep Electron/React UI separate from the C++ native audio engine.
All audio stays local in the native C++/JUCE/Core Audio engine or approved native media workers.
Never use Web Audio, browser audio capture/playback/recording, renderer WASM DSP or a browser fallback.
Electron/React/JS workers must not process audio samples; UI Preview is silent.
Enforce section 3.2A in check:architecture and native integration acceptance.
Never send PCM through renderer IPC or a remote service.
The native audio callback must not allocate, free, block, serialize JSON, or perform I/O.
Preallocate resources, smooth audible parameter changes, and reclaim old graphs off the callback.
Validate all IPC and routing changes in the engine. Reject graph cycles and loopback output routes.
Preview UI state is allowed for UI tests; never present it as working audio. Do not claim hardware tests ran when unavailable.
Preserve user files and existing repository changes. Inspect instructions before editing.
Implement one phase at a time, verify meaningful acceptance criteria, update progress, and continue.
Record exact dependency pins and actual commands/results. Do not weaken tests to hide failures.
Keep default input monitoring off and do not silently alter OS audio routing or install drivers.
On blockers, save a precise checkpoint and continue independent authorized work where possible.
Finish the requested complete feature matrix; MVP completion is not full project completion.
```

Instruksi ini tidak mewajibkan multi-agent atau delegasi. Eksekusi serial bertahap cukup; dependensi antar-fase harus selalu dihormati.

## 13. Template checkpoint dan pelaporan AI

Gunakan format ini di `docs/progress.md`:

```markdown
## Phase XX — Nama
Status: IN_PROGRESS
Prerequisites: Phase YY VERIFIED
Changed files:
- path: perubahan dan alasan
Implemented behavior:
- perilaku yang benar-benar tersedia
Validation:
- command: ...
- environment: OS, arch, device, rate, actual block
- exit/result: ...
- hardware/manual: PASSED / FAILED / NOT_RUN, dengan alasan
Known limitations:
- batas yang relevan
Next exact action:
- fungsi/file/test yang harus dikerjakan berikutnya
```

Pada akhir turn AI, laporkan perubahan terpenting, pengujian yang dijalankan, blocker, dan fase selanjutnya. Jangan mengulang seluruh spesifikasi. Jika semua selesai, berikan cara menjalankan app dan lokasi artifact; jika ada gate belum lulus, gunakan `IMPLEMENTED_UNVERIFIED` atau status blocker yang tepat.

## 14. Prompt siap ditempel ke AI coding di VS Code

Simpan file ini pada `docs/specs/AUDIO-MIXER-AI-IMPLEMENTATION.md`. Bila gambar referensi tersedia, taruh di `docs/design/approved-mixer-layout.png`; gambar melengkapi ukuran/semantik tertulis bagian 8D.

```text
Baca seluruh docs/specs/AUDIO-MIXER-AI-IMPLEMENTATION.md revisi 1.6 dan instruksi repository.
Terapkan bagian 4A: kode UI maupun engine harus modular; knob, slider/fader, meter, numeric input
dan kontrol berulang wajib menjadi reusable components dengan satu sumber implementasi.
Tidak boleh ada file kode first-party lebih dari 1000 baris, termasuk tests/scripts/styles dan native code.
Utamakan file lebih kecil berdasarkan tanggung jawab; pasang check:file-size dan check:architecture sejak UI-01.
Prioritas pertama: buat layout desktop yang mirip mockup terakhir yang disetujui, SEBELUM engine.
Ikuti UI-00 sampai UI-04 pada bagian 8D; jangan memulai native DSP/Core Audio dahulu.
Buat React/Electron UI nyata dengan compact FX A/B rows di atas, mixer di kiri,
Channel Processing lengkap di kanan, dan tombol/tray Harmony. Lengkapi label Freq/Gain/Q,
CLIP/peak hold, MON/REC, Modified badge dan linked SEND A sesuai spesifikasi.
Gunakan PreviewAdapter dengan fixture deterministik dan badge UI PREVIEW; jangan mengaku audio berjalan.
Semua kontrol utama harus interaktif dalam preview: pilih channel, ubah nilai EQ, preset 99,
send/return, Harmony, monitor/arm state dan popup editor. Native audio belum diperlukan.
Render screenshot 1680x945 dan 1280x800, periksa visual, perbaiki overflow/overlap/label hilang,
dan catat UI_VERIFIED dengan bukti screenshot serta tests. Jangan berhenti pada wireframe atau image background.
Sesudah gate UI terpenuhi, ikuti **urutan kanonik bagian 8E.1**, bukan sekadar urut nomor Phase.
Selesaikan pula INT-00–03: semua menu tersambung, recording nyata, paket dev, dan audit penerimaan.
Gunakan feature-traceability agar fitur yang belum terintegrasi tidak terlewat saat handoff.
Integrasikan NativeAdapter tanpa menulis ulang layout. Pertahankan native code existing bila sudah ada.
Tidak perlu menunggu konfirmasi tambahan untuk melanjutkan setelah gate objektif UI lulus,
kecuali pengguna secara eksplisit membatasi pekerjaan hanya UI.
Catat status UI dan engine secara terpisah di docs/progress.md. Hardware tests tak dijalankan tetap NOT_RUN.
Bangun kode dan scripts nyata; jangan mengarang hasil audio, recording, capture atau pengukuran latency.
```

## 15. Referensi primer dan cara menggunakannya

Sumber di bawah dicek pada penyusunan dokumen. Desain kapasitas, parameter, fase, dan acceptance criteria adalah keputusan proyek ini; bukan salinan spesifikasi vendor. API detail dan availability harus diverifikasi kembali pada versi SDK/dependency yang dipin ketika implementasi.

- [BlackHole — repository resmi](https://github.com/ExistentialAudio/BlackHole): loopback device, instalasi, arsitektur yang didukung, ketentuan distribusi.
- [Apple — Aggregate Device](https://support.apple.com/en-us/102171): penggabungan perangkat, clock source, drift correction, pemetaan kanal.
- [Apple — Core Audio taps sample](https://developer.apple.com/documentation/coreaudio/capturing-system-audio-with-core-audio-taps): titik awal backend system/per-app capture.
- [Apple — CATapDescription](https://developer.apple.com/documentation/coreaudio/catapdescription): verifikasi parameter tap melalui SDK/dokumentasi aktual.
- [JUCE — AudioDeviceManager](https://docs.juce.com/master/classjuce_1_1AudioDeviceManager.html): lifecycle device/callback.
- [JUCE — Compressor](https://docs.juce.com/master/classjuce_1_1dsp_1_1Compressor.html): batas API compressor dasar.
- [JUCE — AudioProcessor](https://docs.juce.com/master/classjuce_1_1AudioProcessor.html): lifecycle processor, state, latency.
- [JUCE — licence](https://juce.com/legal/juce-8-licence/): rujukan dependency/distribusi.
- [Electron — security](https://www.electronjs.org/docs/latest/tutorial/security): pemisahan renderer/preload/main dan IPC.

## 16. Checklist selesai keseluruhan

- [ ] Seluruh fase mengikuti dependency plan 8E.1 dan tidak ada prerequisite test yang belum diimplementasikan.
- [ ] INT-00–03 selesai; semua kontrol/tabs di feature-traceability mempunyai native owner dan bukti uji.
- [ ] Stop/MON/REC/recording/solo mengikuti 8E.3–8E.4; file take dapat diputar ulang tanpa memproses efek dua kali.
- [ ] Mandatory codecs, sample rate, streaming underflow, preset mapping dan protocol consistency telah diverifikasi.
- [ ] Pengujian paket Mac nyata selesai; tidak ada PreviewAdapter atau mock telemetry pada release.

- [ ] Modular architecture bagian 4A diterapkan pada UI, Electron, native engine, scripts dan tests.
- [ ] Knob, slider/fader, meter, numeric input dan common controls reusable; panel tidak menyalin implementasi interaksi kontrol.
- [ ] Seluruh first-party code file ≤1.000 baris fisik; check:file-size lulus dan aktif di verify/CI.
- [ ] check:architecture lulus; tidak ada dependency cycle atau import UI↔engine yang dilarang.

- [ ] UI-00–04 dikerjakan pertama dan berstatus UI_VERIFIED sebelum pembangunan engine baru.
- [ ] Layout compact final sesuai 8D, screenshot kedua viewport tersedia, semua kontrol penting terlihat.
- [ ] Preview mode berlabel dan Native mode tidak fallback diam-diam ke fixture.

- [ ] Audio sistem benar-benar dapat diproses sebelum physical output.
- [ ] File multitrack dan mic dapat dicampur dalam satu engine tanpa clock timeline terpisah.
- [ ] Kontrol dB, EQ, dynamics, voice/de-esser, aux/bus/DCA, FX, dan monitoring berfungsi sesuai semantik.
- [ ] Record master/multitrack dan export mix/stems menghasilkan file valid.
- [ ] Editing, automation, MIDI, preset, session/recovery tersedia.
- [ ] VFX-00–09: rack, seluruh efek native, pitch/harmony/vocoder, preset, latency alignment, dan QA suara selesai.
- [ ] Mode monitoring cepat dan studio menampilkan latency aktual; efek yang dilewati terlihat jelas.
- [ ] Mockup Vocal FX dijadikan referensi desain, bukan bukti DSP berjalan.
- [ ] Mixer utama mempunyai FX A/B, bank tepat 99 program, parameter cepat, FX send A/B, dan return independen.
- [ ] Efek insert bisa diatur dari tombol FX channel tanpa pindah tab; layout tetap bekerja pada 1280×800.
- [ ] MIXFX-00–05 lulus, termasuk program switching tanpa dry leak, feedback, atau reset fader/route.
- [ ] HARM-00–03 lulus: tombol HARMONY terlihat pada strip Vocal, mengendalikan satu instance native, key/scale/level dapat diatur tanpa pindah tab.
- [ ] Harmony OFF mempertahankan lead vocal dan alignment; save/open serta rack dan shortcut menunjukkan state yang sama.
- [ ] AU/VST3 dan per-app capture memiliki hasil validasi serta capability status yang jujur.
- [ ] Tidak ada feedback route, double dry path, atau callback blocking yang diketahui pada skenario didukung.
- [ ] Hasil benchmark, hardware matrix, dan packaged-app test terdokumentasi.
- [ ] Build/run scripts reproducible dan dependency dipin.
- [ ] Tidak ada fitur wajib yang diam-diam diubah menjadi sekadar roadmap setelah MVP selesai.
- [ ] Keterbatasan hardware, permission, plugin, dan OS ditampilkan secara jelas.
- [ ] Source, dokumentasi, dan artifact build yang benar-benar tersedia diserahkan kepada pengguna.

- [ ] Kontrak native-only bagian 3.2A lulus: seluruh jalur audio native; tanpa Web Audio/browser fallback, termasuk saat engine gagal.
