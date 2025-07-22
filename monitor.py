
import time
import psutil
from rich.live import Live
from rich.table import Table
from rich import box
from scapy.all import sniff, IP
from collections import Counter

ip_counter = Counter()

def packet_callback(pkt):
    if IP in pkt:
        src_ip = pkt[IP].src
        ip_counter[src_ip] += 1

def get_monitor_table():
    net_io = psutil.net_io_counters()
    cpu_percent = psutil.cpu_percent()
    mem = psutil.virtual_memory()

    table = Table(title="📡 Real-Time Server Monitor", box=box.SIMPLE_HEAVY)
    table.add_column("Komponen", style="cyan", justify="right")
    table.add_column("Status", style="magenta", justify="left")

    table.add_row("⬆️ Upload (TX)", f"{net_io.bytes_sent / (1024*1024):.2f} MB")
    table.add_row("⬇️ Download (RX)", f"{net_io.bytes_recv / (1024*1024):.2f} MB")
    table.add_row("💻 CPU Usage", f"{cpu_percent}%")
    table.add_row("🧠 RAM Usage", f"{mem.percent}%")
    table.add_row("📶 Total Connections", f"{len(psutil.net_connections())}")
    
    top_ips = ip_counter.most_common(5)
    if top_ips:
        table.add_row("🔎 Top IPs", "")
        for ip, count in top_ips:
            table.add_row(f"• {ip}", f"{count} pkt")

    return table

def run_monitor():
    from threading import Thread
    sniff_thread = Thread(target=sniff, kwargs={'prn': packet_callback, 'store': False})
    sniff_thread.daemon = True
    sniff_thread.start()

    with Live(get_monitor_table(), refresh_per_second=1) as live:
        while True:
            time.sleep(1)
            live.update(get_monitor_table())

if __name__ == "__main__":
    run_monitor()
