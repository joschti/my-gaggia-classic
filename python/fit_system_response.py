import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import control
from scipy.optimize import minimize, least_squares, brute, fmin

# Derived system model in time domain by assumming step function input
# - Water inlet forced to 0
def sysmod_time_step(t, K, tau_boiler, t_d, u=0):
    t_d = max(0,t_d)
    tau_boiler = max(0,tau_boiler)
    return np.array([K*(1-np.exp(-(t-t_d)/tau_boiler)) if t >= t_d else 0 for t in t])

# Derived system model as transfer function
# - Water inlet forced to 0
def sysmod_laplace(t, K, tau_boiler, t_d, tau_heater, u):
    # # at least 0
    # t_d = max(0,t_d)
    # tau_boiler = max(0,tau_boiler)
    # tau_heater = max(0,tau_heater)
    # K = max(1, K)

    ## system model
    # no dead time in current system model
    # num, den = control.pade(t_d, 4)
    sys_dead_time = 1#control.tf(num, den)
    # simple first order system with gain and tau
    sys_boiler = control.tf([K], [tau_boiler, 1])
    # simple first order system with tau
    sys_heater = control.tf([1], [tau_heater, 1])
    sys = sys_dead_time * sys_boiler * sys_heater

    # run system
    t, y = control.forced_response(sys, t, u)

    return y

# Generic system model as transfer function
# - fixed number of zeros and poles
def sysmod_generic_laplace(t, X, u):
    xz = X[0:int(len(X)/2)]
    xp = X[int(len(X)/2):len(X)]

    # generic system model
    gen_list = lambda xx : [x for x in reversed(xx)] + [1]
    sys = control.tf(xz[0], gen_list(xp))

    # run system
    t, y = control.forced_response(sys, t, u)

    return y

# choose system model to use
system_model_func = sysmod_laplace

# load measurement data
data = pd.read_csv("logs/device-monitor-240416-initial-two-point-control.log", names=["T", "u"])
ts = 0.5 # sampling interval in seconds
tfull = ts * data.index
yfull = data["T"]
ufull = data["u"]

# cut and downsample data
tmax = 500
dsf = 10
ixmax = int(tmax / ts)
t = tfull[0:ixmax:dsf]
y = yfull[0:ixmax:dsf]
u = ufull[0:ixmax:dsf]

# plt.figure()
# plt.plot(t, y)
# plt.plot(t, u)
# plt.ylabel("Temperature [°C]")
# plt.xlabel("Time [s]")
# plt.show()

## preprocess measurement data
# need to remove initial temperature value as model does not care about this
y_offset = y[0]
y = y - y_offset
# upscale u to reflect heater temperature, we assume it reaches 150°C
# not good: we assume that heater temperature can jump from 150°C down to y_offset
#           that is not how it works... and might be a reason why fitting is not
#           working...
u = 150 * u - y_offset

## fit system
if system_model_func == sysmod_laplace:

    # initial values
    X = [0.95,120,0,10]
    K,tau_boiler,t_d,tau_heater = X

    ## works badly...
    # def err(X,t,y,u=0):
    #     K,tau_boiler,t_d,tau_heater = X
    #     t_d = 0 # don't use delay as it was already accounted for
    #     z = system_model_func(t,K,tau_boiler,t_d, tau_heater, u)
    #     # iae = sum(abs(z-y))*(max(t)-min(t))/len(t)
    #     iae = sum((z-y)**2)
    #     return iae
    # K,tau_boiler,t_d,tau_heater = minimize(
    #     err, X, args=(t,y,u), 
    #     method="L-BFGS-B",
    #     bounds=[(0, 1), (0, 10000), (0, 0), (0, 10000)],
    #     tol=1e-6,
    #     options={"disp": True}
    # ).x

    ## not really working...
    # def residual(X, t, y, u):
    #     z = system_model_func(t,K,tau_boiler,t_d, tau_heater, u)
    #     return (z - y)
    # K,tau_boiler,t_d,tau_heater = least_squares(
    #     residual,
    #     X,
    #     args=(t,y,u),
    #     gtol=None,
    #     verbose=2
    # ).x

    ## brute-force search
    rranges = (
        slice(1.8, 2, 0.05),
        slice(180, 200, 2),
        slice(190, 210, 2),
        # slice(0, 10, 2)
    )
    def err(X, t, y, u=0):
        K, tau_boiler, tau_heater = X
        t_d = 0
        z = system_model_func(t, K, tau_boiler, t_d, tau_heater, u)
        return sum((z-y)**2)
    resbrute = brute(
        err, rranges, args=(t,y,u), 
        finish=minimize,
        workers=-1, # fully parallelize
        disp=True,
        full_output=True,
    )
    K, tau_boiler, tau_heater = resbrute[0]
    print(f"model error: {resbrute[1]}")
    

    yfit = system_model_func(t, K, tau_boiler, t_d, tau_heater, u)

    print("K = {:.5f}".format(K))
    print("tau_boiler = {:.2f}".format(tau_boiler))
    print("t_d = {:.2f}".format(t_d))
    print("tau_heater = {:.2f}".format(tau_heater))

elif system_model_func == sysmod_generic_laplace:
    def err(X,t,y,u=0):
        z = system_model_func(t, X, u)
        iae = sum((z-y)**2)
        return iae

    # initial values
    X = 6 * [1] # must be even number of elements

    Xfit = minimize(err, X, args=(t,y,u), tol=1e-6, options={"disp": True}).x
    yfit = system_model_func(t, Xfit, u)

    print(f"X = {Xfit}")

## plot
plt.plot(t, y)
plt.plot(t, yfit)
plt.xlabel('Time [min]')
plt.ylabel('Temperature')
plt.legend(['Measurement','System model'])
plt.show()






# import pandas as pd
# import numpy as np
# import matplotlib.pyplot as plt
# import control
# from scipy.optimize import minimize

# # First Order Plus Time Delay (FOPTD)
# def foptd(t, K=1, tau=1, tau_d=0):
#     tau_d = max(0,tau_d)
#     tau = max(0,tau)
#     return np.array([K*(1-np.exp(-(t-tau_d)/tau)) if t >= tau_d else 0 for t in t])

# # Assumed system model
# def system_model(t, u, K=1, tau=1, tau_d=0):
#     # at least 0
#     tau_d = max(0,tau_d)
#     tau = max(0,tau)

#     # system model
#     # num, den = control.pade(tau_d, 4)
#     sys_dead_time = 1#control.tf(num, den)
#     sys_first_order = control.tf([K], [tau, 1])
#     sys = sys_dead_time * sys_first_order

#     # run system
#     t, y = control.forced_response(sys, t, u)

#     return y

# system_model_func = system_model

# # t = np.linspace(0,50,200)
# # tau = 10
# # tau_delay = 3
# # K = 2

# # y = foptd(t,K,tau,tau_delay)
# # plt.plot(t,y)
# # plt.xlabel('Time [min]')
# # plt.ylabel('Response')
# # plt.title('FOPTD Step Response')
# # plt.show()

# data = pd.read_csv("logs/device-monitor-240416-initial-two-point-control.log", names=["T", "u"])
# ts = 0.5 # sampling interval in seconds
# tfull = ts * data.index
# yfull = data["T"]
# ufull = data["u"]

# tmax = 500
# ixmax = int(tmax / ts)
# t = tfull[0:ixmax]
# y = yfull[0:ixmax]
# u = ufull[0:ixmax]

# plt.figure()
# plt.plot(t, y)
# plt.plot(t, u)
# plt.ylabel("Temperature [°C]")
# plt.xlabel("Time [s]")
# plt.show()


# # ts = t - t[0]
# y = (y - y[0])

# # plt.plot(ts, ys)
# # plt.title('Shifted and Scaled Data')
# # plt.show()

# # z = foptd(ts, 0.005, 10, 3)
# # plt.plot(ts,ys,ts,z)
# # plt.legend(['Experiment','FOPTD'])
# # plt.show()

# def err(X,t,y,u):
#     K,tau,tau_d = X
#     z = system_model_func(t,u,K,tau,tau_d)
#     # iae = sum(abs(z-y))*(max(t)-min(t))/len(t)
#     iae = sum((z-y)**2)
#     return iae

# X = [0.005,10,3]
# err(X,t,y,u)

# K,tau,tau_d = minimize(err,X,args=(t,y,u)).x

# print("K = {:.5f}".format(K))
# print("tau = {:.2f}".format(tau))
# print("tau_d = {:.2f}".format(tau_d))

# plt.plot(tfull, yfull)
# plt.plot(tfull, system_model_func(tfull,ufull,K,tau,tau_d))
# plt.xlabel('Time [min]')
# plt.ylabel('Temperature [scaled]')
# plt.legend(['Experiment','FOPTD'])
# plt.show()

# # z = foptd(ts,K,tau,tau_d)
# # ypred = y[0] + z

# # plt.plot(t,y,t,ypred)
# # plt.xlabel('Time [min]')
# # plt.ylabel('Temperature [scaled]')
# # plt.legend(['Experiment','FOPTD'])
# # plt.show()

# # K = 120
# # tau_d = 24.23
# # plt.plot(tfull, yfull)
# # plt.plot(tfull, foptd(tfull,K,90,tau_d))
# # plt.xlabel('Time [min]')
# # plt.ylabel('Temperature [scaled]')
# # plt.legend(['Experiment','FOPTD'])
# # plt.show()
