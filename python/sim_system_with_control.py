import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import control
from scipy.optimize import minimize

# First Order Plus Time Delay (FOPTD)
def foptd(t, K=1, tau=1, tau_d=0):
    tau_d = max(0,tau_d)
    tau = max(0,tau)
    return np.array([K*(1-np.exp(-(t-tau_d)/tau)) if t >= tau_d else 0 for t in t])

data = pd.read_csv("logs/device-monitor-240303-heating-up.log", names=["T"])
ts = 0.5 # sampling interval in seconds
tfull = ts * data.index
yfull = data["T"]

K = 1
tau = 90.0
tau_d = 24.23
# plt.plot(tfull, yfull)
# plt.plot(tfull, foptd(tfull,K,tau,tau_d))
# plt.xlabel('Time [min]')
# plt.ylabel('Temperature [scaled]')
# plt.legend(['Experiment','FOPTD'])
# plt.show()

num, den = control.pade(tau_d, 4)
sys_dead_time = control.tf(num, den)
sys_first_order = control.tf([K], [tau, 1])
sys_heater = sys_dead_time * sys_first_order
print(sys_dead_time)
print(sys_first_order)
print(sys_heater)

plt.subplot(211)
t, y = control.step_response(sys_heater, tfull)
plt.plot(t, y)

sys_pcontroller = 1000000
sys_openloop = sys_pcontroller * sys_heater
sys_closedloop = sys_openloop / (1 + sys_openloop)

input = np.zeros(tfull.shape)
input[30:180] = 300
# input[90:120] = 120
t, y = control.forced_response(sys_heater, tfull, input)
plt.subplot(212)
plt.plot(t, input)
plt.plot(t, y)
plt.show()