import pandas as pd
import matplotlib.pyplot as plt

data = pd.read_csv("logs/device-monitor-240303-heating-up.log", names=["T"])
ts = 0.5 # sampling interval in seconds

plt.figure()
plt.plot(ts * data.index, data["T"])
plt.ylabel("Temperature [°C]")
plt.xlabel("Time [s]")
plt.show()