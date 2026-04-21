import numpy as np
import matplotlib.pyplot as plt
from scipy.integrate import solve_ivp
from matplotlib.animation import FuncAnimation
import argparse

#chat GPT did this for me ... :(
#these prompts

#python double pendulum numerical simulation please
#can you please use a lagriangian energy formulation for this model
#symbolically derive the equations using SymPy
#dynamics simulator in python of a double pendulum with a force applied to the end of the first link
#please add damping 
#please add a matplotlib animation
#
# then started doing my own thing:
# added the standard homework function selector
# added a visualizer for the force direction
# added a time label.
# 
# next up: The iterative "depending on physics" push or pull
# big deal: we have to use events because the solve IVP may step backwards in time.


# Parameters
g = 9.81
m1, m2 = 1.0, 1.0
L1, L2 = 1.0, 1.0
c1, c2 = 0.1, 0.1  # damping

# External force
def external_force(t,y):
    '''
    Fx = 2.0 * np.cos(2*t)
    Fy = 0.0
    return Fx, Fy
    '''
    fx_a,fy_a = attractive_force(t,y)

    return fx_a,fy_a

def attractive_force(t,y): #does not depend on time, only the pendulum position
    theta1,*_  = y #y is [theta1 omega1 theta2 omega2 Fx Fy]  #F comes from the external force function def; we need to save it but the previous F doesn't matter for this F


    x1 = L1 * np.sin(theta1)
    y1 = -L1 * np.cos(theta1)
    linkage_loc = np.array([x1,y1])

    #put a magnet at 0,-1
    magnet_loc = np.array([0,-1])
    distance = np.linalg.norm(magnet_loc-linkage_loc)
    force = 0.2

    magnetic_force = force/(distance**4)
    magnetic_force=np.clip(magnetic_force,-2,2) #clip it to a maximum value (just like in real life, the pendulum also has z seperation)
    #magnet force decreases 1/4th due to dipole effect (for now)

    fx,fy = magnetic_force*(magnet_loc-linkage_loc)/distance #break into the correct directions.  Make sure to normalize it by direction
    #print(t,magnetic_force,x1,y1,fx,fy)

    return fx,fy



#need this, because the only way we can make repulsive work is by making the thing stateful.

def repulsive_force(t,y):

    pass


def deriv(t, y):
    theta1, omega1, theta2, omega2 = y #y is [theta1 omega1 theta2 omega2 Fx Fy]  #F comes from the external force function def.  we don't keep track of F since this function returns F dot, instead, we're going to have to re-solve after export.
    delta = theta1 - theta2

    Fx, Fy = external_force(t,y) #pass in the whole state vector as well as time

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

def run_sim():

    # Initial conditions
    y0 = [np.pi/2, 0, np.pi/2, 0]

    t_span = (0, 30)
    t_eval = np.linspace(*t_span, 3000)

    sol = solve_ivp(deriv, t_span, y0, t_eval=t_eval, max_step=0.01)

    theta1 = sol.y[0]
    theta2 = sol.y[2]

    #now, I have to re-calculate the force since I wasn't able to export them out of the ODE solver.

    fx = np.zeros(t_eval.shape)
    fy = np.zeros(t_eval.shape)
    for idx,t in enumerate(t_eval):
        fx_calc,fy_calc = external_force(t,sol.y[:,idx])
        fx[idx] = fx_calc
        fy[idx] = fy_calc

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

    linkage_lines, = ax.plot([], [], 'o-', lw=2)
    force_arrow = ax.arrow(0,0,0,0,color="r",width=0.02,alpha=0.6)
    trace, = ax.plot([], [], '-', lw=1, alpha=0.6)
    time_label = ax.text(0.0, 2.0gi, "t=0.000 [sec]",ha='center', va='center')


    trail_x, trail_y = [], []

    def update(frame):
        # Pendulum line
        linkage_lines.set_data(
            [0, x1[frame], x2[frame]], #x data of the three pivots
            [0, y1[frame], y2[frame]] #y data of the three pivots
        )

        # Trail of second mass
        trail_x.append(x2[frame])
        trail_y.append(y2[frame])

        # limit trail length
        if len(trail_x) > 200:
            trail_x.pop(0)
            trail_y.pop(0)

        #visualize where the force vector is pointing.
        force_arrow.set_data(x=x1[frame],y=y1[frame],dx=fx[frame],dy=fy[frame])
        #construct the visualization of the force vector
        trace.set_data(trail_x, trail_y)
        time_label.set_text(f"t={t_eval[frame]:2.3f} [sec]")

        return linkage_lines, force_arrow, trace, time_label

    ani = FuncAnimation(
        fig,
        update,
        frames=len(t_eval),
        interval=20,
        blit=True
    )

    plt.title("Forced + Damped Double Pendulum Animation")
    plt.show()



if __name__=="__main__":
    #https://stackoverflow.com/questions/3061/calling-a-function-of-a-module-by-using-its-name-a-string
    parser = argparse.ArgumentParser(description = 'Central File for homeworks - pass in the function you want to run, or, when run, it will ask you which function you will like to run.  Functions --> part of the HW')
    parser.add_argument('fxn',default=None, type = str, nargs='?', help = 'Specify which function to run - must be typed exactly correct.')

    parsed = parser.parse_args()

    print(parsed)


    if parsed.fxn == None:


        #improvement - this will check the type of each local, and only add things that are functions that I define,.
        all_var = list(locals().keys())
        all_functions = []
        for v in all_var:
            if callable(locals()[v]):
                all_functions.append(v)


        print(all_functions)

        #heavily borrow from https://stackoverflow.com/questions/52143468/python-tab-autocompletion-in-script so that we can actually do tab completion on our input.
        try:
            import readline
        except ImportError:
            import pyreadline as readline

        def completer(text, state):
            options = [cmd for cmd in all_functions if cmd.startswith(text)]
            if state < len(options):
                return options[state]
            else:
                return None

        readline.parse_and_bind("tab: complete")
        readline.set_completer(completer)

        parsed.fxn = input("Which question to run?: ")
    assert parsed.fxn in locals().keys(), "Did not select a function I've defined"

    locals()[parsed.fxn]() #yes, that's the crazy syntax to get the right function out of the locals() and then call it.