# System model of boiler for temperature control

## References

[1] [MIT lecture notes: Process dynamics, operations and control](https://ocw.mit.edu/courses/10-450-process-dynamics-operations-and-control-spring-2006/dc573f23401eeb5822818fbaa177eaac_5_heated_tank.pdf)

[2] [Master thesis: Modeling and control of a heated tank system with variable liquid hold-up](http://etd.lib.metu.edu.tr/upload/12616167/index.pdf)

## Assumptions

- Constant mass flow into the tank
- Same outflow as inflow out of the tank
- Physicial properties of the liquid like density are constant

## Deriviation

### Mass balance

$$
\frac{d}{dt}(m_{boiler}) = w_{inlet} - w_{outlet}\\
0 = w_{inlet} - w_{outlet}
$$
- $m_{boiler}$ denotes the water mass carried by the boiler
- $w$ denotes the mass flow.

Simply
$$
w_{inlet} = w_{outlet} = w
$$ (1)

### Energy balance

$$
\frac{d}{dt}(E_{boiler}) = w_{inlet} \cdot C_{p} \cdot (T_{inlet} - T_{ref}) - w_{outlet} \cdot C_{p} \cdot (T_{boiler} - T_{ref})\\ + U_{heater} \cdot A_{heater} \cdot (T_{heater} - T_{boiler}) 
$$
- $E$ denotes heat energy
- $C_p$ denotes specific heat capacity of the water
- $T$ for temperature
- $U_{heater}$ for heat transfer coefficient
- $A_{heater}$ for the surface area of the heat element
- $T_{ref}$ denotes the theromodynamic reference

Let's break down $E_{boiler}$ that was mainly for readability
$$
\frac{d}{dt}(E_{boiler}) = \frac{d}{dt}(\rho \cdot V_{boiler} \cdot C_p \cdot (T_{boiler} - T_{ref}))
$$
- $\rho$ is the density of water
- $V_{boiler}$ denotes the volume of the boiler

$$
\frac{d}{dt}(\rho \cdot V_{boiler} \cdot C_p \cdot (T_{boiler} - T_{ref})) = w_{inlet} \cdot C_{p} \cdot (T_{inlet} - T_{ref}) - w_{outlet} \cdot C_{p} \cdot (T_{boiler} - T_{ref})\\
+ U_{heater} \cdot A_{heater} \cdot (T_{heater} - T_{boiler}) 
$$ (2)

### Combine mass and energy balance equations

Let's substitute (1) into (2):
$$
\frac{d}{dt}(\rho \cdot V_{boiler} \cdot C_p \cdot (T_{boiler} - T_{ref})) = w \cdot C_{p} \cdot (T_{inlet} - T_{ref}) - w \cdot C_{p} \cdot (T_{boiler} - T_{ref})\\
+ U_{heater} \cdot A_{heater} \cdot (T_{heater} - T_{boiler}) 
$$

Only $T_{boiler}$, $T_{inlet}$, $T_{outlet}$ and $T_{heater}$ are time dependent (I assume that $T_{ref}$ is a constant - and by differentiation 0)
$$
\rho \cdot V_{boiler} \cdot C_p \cdot  \frac{d}{dt}T_{boiler} = w \cdot C_{p} \cdot (T_{inlet} - T_{ref}) - w \cdot C_{p} \cdot (T_{boiler} - T_{ref})\\
+ U_{heater} \cdot A_{heater} \cdot (T_{heater} - T_{boiler}) 
$$

$T_{ref}$ cancels itself out on the right hand side:
$$
\rho \cdot V_{boiler} \cdot C_p \cdot  \frac{d}{dt}T_{boiler} = w \cdot C_{p} \cdot (T_{inlet} - T_{boiler}) + U_{heater} \cdot A_{heater} \cdot (T_{heater} - T_{boiler}) 
$$

Let's rearrange
$$
\rho \cdot V_{boiler} \cdot C_p \cdot  \frac{d}{dt}T_{boiler} = w \cdot C_{p} \cdot T_{inlet} - w \cdot C_{p} \cdot T_{boiler} + U_{heater} \cdot A_{heater} \cdot T_{heater} - U_{heater} \cdot A_{heater} \cdot T_{boiler}
$$
$$
\rho \cdot V_{boiler} \cdot C_p \cdot  \frac{d}{dt}T_{boiler} = - T_{boiler} \cdot (U_{heater} \cdot A_{heater}  + w \cdot C_{p}) + w \cdot C_{p} \cdot T_{inlet} + U_{heater} \cdot A_{heater} \cdot T_{heater}$$
$$
\rho \cdot V_{boiler} \cdot C_p \cdot  \frac{d}{dt}T_{boiler} + (U_{heater} \cdot A_{heater}  + w \cdot C_{p}) \cdot T_{boiler} = w \cdot C_{p} \cdot T_{inlet} + U_{heater} \cdot A_{heater} \cdot T_{heater}
$$
$$
\frac{\rho \cdot V_{boiler} \cdot C_p}{w \cdot C_{p} + U_{heater} \cdot A_{heater}} \cdot \frac{d}{dt}T_{boiler} + T_{boiler} = \frac{w \cdot C_{p}}{w \cdot C_{p} + U_{heater} \cdot A_{heater}} \cdot T_{inlet} + \frac{U_{heater} \cdot A_{heater}}{w \cdot C_{p} + U_{heater} \cdot A_{heater}} \cdot T_{heater}
$$

Let's use substitutes for these large terms:
$$
\tau_{boiler} = \frac{\rho \cdot V_{boiler} \cdot C_p}{w \cdot C_{p} + U_{heater} \cdot A_{heater}}
$$
$$
K_{inlet} = \frac{w \cdot C_{p}}{w \cdot C_{p} + U_{heater} \cdot A_{heater}}
$$
$$
K_{heater} = \frac{U_{heater} \cdot A_{heater}}{w \cdot C_{p} + U_{heater} \cdot A_{heater}}
$$

and get
$$
\tau_{boiler} \cdot \frac{d}{dt}T_{boiler} + T_{boiler} = K_{inlet} \cdot T_{inlet} + K_{heater} \cdot T_{heater}
$$ (3)

### Solve differential equation with Laplace transform

Take (3), apply Laplace transformation and solve for $T_{boiler}(s)$
$$
\tau_{boiler} \cdot \mathcal{L}(\frac{d}{dt}T_{boiler}) + \mathcal{L}(T_{boiler}) = K_{inlet} \cdot \mathcal{L}(T_{inlet}) + K_{heater} \cdot \mathcal{L}(T_{heater})
$$
$$
\tau_{boiler} \cdot (s \cdot T_{boiler}(s) - T_{boiler}(0)) + T_{boiler}(s) = K_{inlet} \cdot T_{inlet}(s) + K_{heater} \cdot T_{heater}(s)
$$
and finally get
$$
T_{boiler}(s) = \frac{K_{inlet}}{\tau_{boiler} \cdot s  + 1} \cdot T_{inlet}(s) + \frac{K_{heater}}{\tau_{boiler} \cdot s  + 1} \cdot T_{heater}(s) + \frac{\tau_{boiler}}{\tau_{boiler} \cdot s  + 1} \cdot T_{boiler}(0)
$$ (4)
where $T_{boiler}(0)$ is the inital temperature of the boiler.
> Unclear: Why is initial boiler temperature $T_{boiler}(0)$ low pass filtered?

### Delayed step response of boiler

> Heaviside function: $U(t - t_c) = \mathcal{L}^{-1}(\frac{1}{s} e^{-t_c s})$

In order to keep things simpler instead of temperatures, let's use delta temperature. This allows to neglect initial boiler temperature by choosing a room temperature as reference.

$$
\Delta T_{any} = T_{any} - 22°C
$$

Let's define $\Delta T_{inlet}$, $\Delta T_{heater}$ and $\Delta T_{boiler}(0)$:
$$
\Delta T_{inlet}(t) = \Delta T_{inlet,0} \cdot U(t - t_{inlet,0})
$$
$$
\Delta T_{inlet}(s) = \Delta T_{inlet,0} \cdot \frac{1}{s} e^{-t_{inlet,0} s}
$$
$$
\Delta T_{heater}(t) = \Delta T_{heater,0} \cdot U(t - t_{heater,0})
$$
$$
\Delta T_{heater}(s) = \Delta T_{heater,0} \cdot \frac{1}{s} e^{-t_{heater,0} s}
$$
$$
\Delta T_{boiler}(0) = 0
$$

and plug it in (4)
$$
\Delta T_{boiler}(s) = K_{inlet} \cdot \Delta T_{inlet,0} \cdot \frac{1}{s \cdot (\tau_{boiler} \cdot s  + 1)} \cdot e^{-t_{inlet,0} s} + K_{heater} \cdot \Delta T_{heater,0} \cdot \frac{1}{s \cdot (\tau_{boiler} \cdot s  + 1)} \cdot e^{-t_{heater,0} s}
$$

> Laplace table: $U(t - t_c) \cdot y(t - t_c) = \mathcal{L}^{-1}(e^{-t_c s} \cdot Y(s))$

> Laplace table: $1 - e^{-t / \tau} = \mathcal{L}^{-1}(\frac{1}{s \cdot (\tau \cdot s + 1)})$

$$
\Delta T_{boiler}(t) = K_{inlet} \cdot \Delta T_{inlet,0} \cdot U(t - t_{inlet,0}) \cdot (1 - e^{-\frac{t - t_{inlet,0}}{\tau_{boiler}}})\\ + K_{heater} \cdot \Delta T_{heater,0} \cdot U(t - t_{heater,0}) \cdot (1 - e^{-\frac{t - t_{heater,0}}{\tau_{boiler}}})
$$ (5)

Equation (5) allows us to simply fit real life measurements and to get a fully parameterized system model. The step function is something that can be simply measured from the coffee machine.