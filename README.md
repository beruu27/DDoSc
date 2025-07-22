# 🛠️ DDoS Tools & Real-Time Server Monitor

> **fortis fortuna adiuvat** — Fortune favors the bold.

## 📂 Deskripsi

Repositori ini berisi dua alat utama:

1. **`ddos2.c`** — Alat uji penetrasi jaringan berbasis *SYN flood* dan *UDP flood*, yang dapat digunakan untuk menguji ketahanan server (hanya untuk keperluan legal & edukatif).
2. **`monitor.py`** — Monitor server real-time yang menampilkan info CPU, RAM, trafik jaringan, dan IP yang paling sering mengirim paket. Visualisasi menggunakan `rich`.

---

## ⚠️ Peringatan

> **Tools ini hanya boleh digunakan untuk _penetration testing_ dengan izin resmi!**
>
> Dilarang keras menggunakan script ini untuk aktivitas ilegal seperti serangan DDoS tanpa izin. Pelanggaran dapat dikenakan sanksi hukum sesuai UU ITE atau hukum internasional terkait keamanan siber.

---

## 📦 Requirements

### Untuk `monitor.py`:

Install dependensi dengan pip:

```bash
pip install psutil rich scapy
```

### Untuk `ddos2.c`:

Compilasi membutuhkan GCC dan akses root:

```bash
sudo apt update
sudo apt install build-essential
```

---

## 🔧 Instalasi & Kompilasi

### 🔹 Compile `ddos2.c`:

```bash
gcc ddos2.c -o ddos2 -lpthread
```

---

## 🚀 Penggunaan

### 1. 📡 **Menjalankan Monitor (Python)**

```bash
sudo python monitor.py
```

📍 Keluaran:
- Penggunaan CPU, RAM
- Total upload/download (MB)
- Jumlah koneksi aktif
- IP terbanyak mengirim paket (top 5)

### 2. 💥 **Menjalankan DDoS Tester (C)**

```bash
sudo ./ddos2 <target_domain_or_ip> <mode>
```

📌 Contoh:

```bash
sudo ./ddos2 example.com syn
sudo ./ddos2 example.com udp
sudo ./ddos2 example.com all
```

🧵 Menggunakan 30 thread secara default. Log hasil serangan disimpan di file `ddos.log`.

---

## 🖥️ (Monitor.py)

```
📡 Real-Time Server Monitor
────────────────────────────
⬆️ Upload (TX)        1.25 MB
⬇️ Download (RX)      3.10 MB
💻 CPU Usage          42%
🧠 RAM Usage          67%
📶 Total Connections  108
🔎 Top IPs
• 192.168.1.5         134 pkt
• 10.0.0.2            97 pkt
```

---

## 🤝 Kontribusi

Pull request & ide pengembangan selalu terbuka, selama mengikuti etika penggunaan dan batasan hukum.

---

## 📝 Lisensi

MIT License © 2025 `beru`

---

## 🧠 Catatan Tambahan

- **Pastikan** untuk menjalankan `monitor.py` dengan hak akses root/sudo agar modul `scapy` bisa menangkap paket.
- Script `ddos2.c` menggunakan `raw socket`, sehingga memerlukan izin superuser untuk berjalan dengan benar.

---
