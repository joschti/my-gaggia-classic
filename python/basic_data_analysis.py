import pandas as pd
import matplotlib.pyplot as plt

data = pd.read_csv("logs/device-monitor-240416-initial-two-point-control.log", names=["T","active"])
ts = 0.5 # sampling interval in seconds

plt.figure()
plt.plot(ts * data.index, data["T"])
plt.ylabel("Temperature [°C]")
plt.xlabel("Time [s]")
plt.plot(ts * data.index, data["active"])
plt.ylabel("Heater active")
plt.xlabel("Time [s]")
plt.show()