import numpy as np
import matplotlib.pyplot as plt
from scipy.integrate import solve_ivp
from matplotlib.animation import FuncAnimation

#chat GPT did this for me ... :(
#these prompts

#python double pendulum numerical simulation please
#can you please use a lagriangian energy formulation for this model
#symbolically derive the equations using SymPy
#dynamics simulator in python of a double pendulum with a force applied to the end of the first link
#please add damping 
#please add a matplotlib animation


# Parameters
g = 9.81
m1, m2 = 1.0, 1.0
L1, L2 = 1.0, 1.0
c1, c2 = 0.5, 0.3  # damping

# External force
def external_force(t):
    Fx = 2.0 * np.cos(2*t)
    Fy = 0.0
    return Fx, Fy

def deriv(t, y):
    theta1, omega1, theta2, omega2 = y
    delta = theta1 - theta2

    Fx, Fy = external_force(t)

    # Generalized force at joint 1
    Q1 = Fx * L1 * np.cos(theta1) + Fy * L1 * np.sin(theta1) - c1 * omega1
    Q2 = -c2 * omega2

    denom = (2*m1 + m2 - m2*np.cos(2*delta))

    dtheta1 = omega1
    dtheta2 = omega2

    domega1 = (
        -g*(2*m1 + m2)*np.sin(theta1)
        - m2*g*np.sin(theta1 - 2*theta2)
        - 2*np.sin(delta)*m2*(omega2**2 * L2 + omega1**2 * L1 * np.cos(delta))
    ) / (L1 * denom)

    domega2 = (
        2*np.sin(delta)*(
            omega1**2 * L1 * (m1 + m2)
            + g*(m1 + m2)*np.cos(theta1)
            + omega2**2 * L2 * m2 * np.cos(delta)
        )
    ) / (L2 * denom)

    # Add forces (approximate projection)
    domega1 += Q1 / ((m1 + m2) * L1**2)
    domega2 += Q2 / (m2 * L2**2)

    return [dtheta1, domega1, dtheta2, domega2]

# Initial conditions
y0 = [np.pi/2, 0, np.pi/2, 0]

t_span = (0, 30)
t_eval = np.linspace(*t_span, 3000)

sol = solve_ivp(deriv, t_span, y0, t_eval=t_eval)

theta1 = sol.y[0]
theta2 = sol.y[2]

# Convert to Cartesian coordinates
x1 = L1 * np.sin(theta1)
y1 = -L1 * np.cos(theta1)

x2 = x1 + L2 * np.sin(theta2)
y2 = y1 - L2 * np.cos(theta2)

# -------------------
# 🎥 Animation
# -------------------

fig, ax = plt.subplots(figsize=(6,6))
ax.set_xlim(-2.2, 2.2)
ax.set_ylim(-2.2, 2.2)
ax.set_aspect('equal')
ax.grid()

line, = ax.plot([], [], 'o-', lw=2)
trace, = ax.plot([], [], '-', lw=1, alpha=0.6)

trail_x, trail_y = [], []

def update(frame):
    # Pendulum line
    line.set_data(
        [0, x1[frame], x2[frame]],
        [0, y1[frame], y2[frame]]
    )

    # Trail of second mass
    trail_x.append(x2[frame])
    trail_y.append(y2[frame])

    # limit trail length
    if len(trail_x) > 200:
        trail_x.pop(0)
        trail_y.pop(0)

    trace.set_data(trail_x, trail_y)

    return line, trace

ani = FuncAnimation(
    fig,
    update,
    frames=len(t_eval),
    interval=20,
    blit=True
)

plt.title("Forced + Damped Double Pendulum Animation")
plt.show()



