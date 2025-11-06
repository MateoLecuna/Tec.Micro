import tkinter as tk
from tkinter import ttk, messagebox
import threading, time
from collections import deque
from serial import Serial, SerialException
from serial.tools import list_ports

import matplotlib
matplotlib.use("TkAgg")
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure

# ===================== Config =====================
BAUD = 115200
READ_TIMEOUT = 0.2
UI_UPDATE_MS = 50
WINDOW_S = 20          # Ventana fija de 5 segundos
MAX_FPS = 30

# ===================== Utilidades =====================
def listar_puertos():
    return [p.device for p in list_ports.comports()]

def parse_line(line: str):
    line = line.strip()
    if not line or line.startswith("P3C"):
        return None
    parts = line.split(',')
    if len(parts) < 4:
        return None
    try:
        ref = int(parts[0])   # pote (referencia, A0)
        act = int(parts[1])   # motor (medición, A1)
        pwm = int(parts[2])
        dir_str = parts[3]
        return ref, act, pwm, dir_str
    except:
        return None

# ===================== Hilo lector =====================
class SerialReader(threading.Thread):
    def __init__(self, on_sample, stop_event):
        super().__init__(daemon=True)
        self.on_sample = on_sample
        self.stop_event = stop_event
        self.ser = None
        self.buffer = b""

    def open(self, port):
        self.close()
        try:
            self.ser = Serial(port, BAUD, timeout=READ_TIMEOUT)
            return True
        except SerialException as e:
            self.ser = None
            messagebox.showerror("Serial", f"No se pudo abrir {port}\n{e}")
            return False

    def close(self):
        if self.ser:
            try: self.ser.close()
            except: pass
        self.ser = None

    def run(self):
        while not self.stop_event.is_set():
            if not self.ser:
                time.sleep(0.1)
                continue
            try:
                data = self.ser.read(256)
                if data:
                    self.buffer += data
                    while b"\n" in self.buffer:
                        line, self.buffer = self.buffer.split(b"\n", 1)
                        txt = line.decode(errors="ignore").strip()
                        rec = parse_line(txt)
                        if rec:
                            self.on_sample(rec)
            except:
                self.close()

# ===================== App =====================
class App(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("Telemetría P3C (pote/motor & pwm)")
        self.geometry("980x640")

        self.t0 = None
        self.t = deque()
        self.pote = deque()   # antes ref
        self.motor = deque()  # antes act
        self.pwm = deque()
        self.dir_last = "STOP"

        self.running = False
        self.stop_event = threading.Event()
        self.reader = SerialReader(self.on_sample, self.stop_event)
        self.reader.start()

        self._build_ui()
        self._setup_plot()
        self.after(UI_UPDATE_MS, self.update_plot)

    def _build_ui(self):
        top = ttk.Frame(self)
        top.pack(fill=tk.X, padx=8, pady=6)

        ttk.Label(top, text="Puerto:").pack(side=tk.LEFT)
        self.cmb_ports = ttk.Combobox(top, width=18, state="readonly")
        self.cmb_ports.pack(side=tk.LEFT)
        ttk.Button(top, text="Actualizar", command=self.refresh_ports).pack(side=tk.LEFT, padx=4)

        ttk.Button(top, text="Start", command=self.start).pack(side=tk.LEFT, padx=4)
        self.btn_stop = ttk.Button(top, text="Stop", command=self.stop, state="disabled")
        self.btn_stop.pack(side=tk.LEFT, padx=4)

        # (Se retiraron la barra de stats y el botón Guardar CSV)

        self.refresh_ports()

        self.frm_plot = ttk.Frame(self)
        self.frm_plot.pack(fill=tk.BOTH, expand=True, padx=8, pady=8)

    def _setup_plot(self):
        self.fig = Figure(figsize=(8, 5), dpi=100)

        # Gráfico superior: pote y motor
        self.ax_top = self.fig.add_subplot(211)
        self.ax_top.set_ylabel("pote / motor (0-1023)")
        self.ax_top.set_ylim(0, 1023)
        self.ax_top.grid(True, linestyle=":", linewidth=0.7)

        # Gráfico inferior: pwm
        self.ax_bottom = self.fig.add_subplot(212)
        self.ax_bottom.set_ylabel("PWM (0-255)")
        self.ax_bottom.set_ylim(0, 255)
        self.ax_bottom.set_xlabel("Tiempo (s)")
        self.ax_bottom.grid(True, linestyle=":", linewidth=0.7)

        (self.line_pote,)  = self.ax_top.plot([], [], label="pote", color="red")
        (self.line_motor,) = self.ax_top.plot([], [], label="motor", color="orange")
        (self.line_pwm,)   = self.ax_bottom.plot([], [], label="pwm", color="green")

        self.ax_top.legend(loc="upper left")
        self.ax_bottom.legend(loc="upper left")

        self.canvas = FigureCanvasTkAgg(self.fig, self.frm_plot)
        self.canvas.get_tk_widget().pack(fill=tk.BOTH, expand=True)
        self.fig.tight_layout()

    def refresh_ports(self):
        ports = listar_puertos()
        self.cmb_ports["values"] = ports
        if ports: self.cmb_ports.current(0)

    def start(self):
        if self.reader.open(self.cmb_ports.get()):
            self.running = True
            self.btn_stop.config(state="normal")
            self.reset_data()

    def stop(self):
        self.reader.close()
        self.running = False
        self.btn_stop.config(state="disabled")

    def reset_data(self):
        self.t0 = None
        self.t.clear(); self.pote.clear(); self.motor.clear(); self.pwm.clear()

    def on_sample(self, rec):
        ref, act, pwm, dir_str = rec
        if self.t0 is None: self.t0 = time.time()
        t = time.time() - self.t0

        self.t.append(t)
        self.pote.append(ref)    # map ref -> pote
        self.motor.append(act)   # map act -> motor
        self.pwm.append(pwm)
        self.dir_last = dir_str

        # Ventana fija
        tmin = t - WINDOW_S
        while self.t and self.t[0] < tmin:
            self.t.popleft(); self.pote.popleft(); self.motor.popleft(); self.pwm.popleft()

    def update_plot(self):
        if self.t:
            ts = list(self.t)
            self.line_pote.set_data(ts, list(self.pote))
            self.line_motor.set_data(ts, list(self.motor))
            self.line_pwm.set_data(ts, list(self.pwm))

            self.ax_top.set_xlim(ts[-1] - WINDOW_S, ts[-1])
            self.ax_bottom.set_xlim(ts[-1] - WINDOW_S, ts[-1])

            self.canvas.draw_idle()

        self.after(UI_UPDATE_MS, self.update_plot)

    def on_close(self):
        self.stop_event.set()
        self.reader.close()
        self.destroy()

if __name__ == "__main__":
    app = App()
    app.protocol("WM_DELETE_WINDOW", app.on_close)
    app.mainloop()
