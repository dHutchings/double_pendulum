import numpy as np
import matplotlib.pyplot as plt
from scipy.integrate import solve_ivp
from matplotlib.animation import FuncAnimation
import matplotlib.patches as patches
import math

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

w = 0.1 #width around the pendulum, for visualization only.
d_weight = 0.3 #the diameter of the weights at M1 / M2.

# External force
def external_force(t,y,is_pushing):
    '''
    Fx = 2.0 * np.cos(2*t)
    Fy = 0.0
    return Fx, Fy
    '''
    if is_pushing:
        fx_a,fy_a = repulsive_force(t,y)
    else:
        fx_a,fy_a = attractive_force(t,y)

    return fx_a,fy_a

def attractive_force(t,y): #does not depend on time, only the pendulum position
    theta1,*_  = y #y is [theta1 omega1 theta2 omega2]  #F comes from the external force function def; we need to save it but the previous F doesn't matter for this F


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
#for now, let's say it's the opporite of the attractive force
def repulsive_force(t,y):
    fx,fy = attractive_force(t,y)
    return -1*fx, -1*fy
    
def crossing_event(t,y,is_pushing): #is_pushing is not needed for this function, but must be taken due to solve_IVP syntax
    return y[0] #we are finding zero crossings of theta

def deriv(t, y, is_pushing): #is repulsing is given in by solve IVP, we are told if the magnet is repulsing or not.
    theta1, omega1, theta2, omega2 = y #y is [theta1 omega1 theta2 omega2 Fx Fy]  #F comes from the external force function def.  we don't keep track of F since this function returns F dot, instead, we're going to have to re-solve after export.
    delta = theta1 - theta2

    Fx, Fy = external_force(t,y,is_pushing) #pass in the whole state vector as well as time

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

#from chat GPT
#given two line endpoints,
#calcualte the 4 corners of a rectangle around that line of width W
def rectangle_corners(x1, y1, x2, y2):
    dx = x2 - x1
    dy = y2 - y1

    length = math.hypot(dx, dy)
    if length == 0:
        raise ValueError("Endpoints must be different")

    # Unit direction vector
    dir_x = dx / length
    dir_y = dy / length

    # Unit perpendicular vector
    perp_x = -dy / length
    perp_y = dx / length

    half_w = w / 2

    # Offsets
    offset_dir_x = dir_x * half_w
    offset_dir_y = dir_y * half_w

    offset_perp_x = perp_x * half_w
    offset_perp_y = perp_y * half_w

    # Four corners
    p1 = (x1 - offset_dir_x + offset_perp_x, y1 - offset_dir_y + offset_perp_y)
    p2 = (x1 - offset_dir_x - offset_perp_x, y1 - offset_dir_y - offset_perp_y)
    p3 = (x2 + offset_dir_x - offset_perp_x, y2 + offset_dir_y - offset_perp_y)
    p4 = (x2 + offset_dir_x + offset_perp_x, y2 + offset_dir_y + offset_perp_y)


    return [p1, p2, p3, p4]

def run_sim():

    # Initial conditions
    #y0 = [np.pi/2, 0, np.pi/2, 0]

    y0 = [np.deg2rad(0.002), 0, 0, 0]


    t_sim = 30
    t_IVP_stopped = 0 
    timestep = 0.01
    pushing_time = 0.1
    starting_push_time = 1.25  #in my current simulation, only push as long as it takes to push the link fully out - don't let it come in or come to rest!

    t_result = None
    y_result = None
    pushing_result = None


    direction = -1 #we expect the direction to be from positive to negative right now
    pendulum_pushing = False
    
    while t_IVP_stopped < t_sim: 

        if t_IVP_stopped == 0: #I"m starting up!
            t_eval = np.arange(t_IVP_stopped,t_IVP_stopped+starting_push_time+0.05,timestep) #add 0.05, again, for floating point timestep issues
            t_final = starting_push_time
            pendulum_pushing = True

        elif pendulum_pushing:
            t_eval = np.arange(t_IVP_stopped,t_IVP_stopped+pushing_time+0.05,timestep) #add 0.05, again, for floating point timestep issues
            t_final = t_IVP_stopped+pushing_time
        else:
            t_eval = np.arange(t_IVP_stopped,t_sim+0.05,timestep) #add 0.05, again, for floating point timestep issues
            t_final = t_sim

        

        crossing_event.terminal=True #solve only till the first zero-crossing
        crossing_event.direction = direction #we need to detect the stopping in only one direction - if we do either, we will hang forever at the zero crossing.
        
        #add 0.1 seconds to the final time, to deal with floating point time resolution issues, to ensure that all desired time points are in bounds.
        sol = solve_ivp(deriv, (t_IVP_stopped,t_final+0.1), y0, t_eval=t_eval, max_step=0.01,args=(pendulum_pushing,),events=crossing_event) #going in the negative direction due to initial conditions

        t_IVP_stopped = sol.t[-1]
        #print("t_IVP_stopped:",t_IVP_stopped)

        y0 = sol.y[:,-1] #update the Initial values for the next time through the loop.  Get the final result
        #print("y0:",y0)

        if t_result is None:
            t_result = sol.t
            y_result = sol.y
            pushing_result = pendulum_pushing*np.ones(sol.t.shape).astype(bool)
        else:
            t_result = np.concatenate([t_result,sol.t[1:]]) #throw away the first timestep, since, that's a repeat
            y_result = np.concatenate([y_result,sol.y[:,1:]],axis=1)
            pushing_result = np.concatenate([pushing_result,pendulum_pushing*np.ones(sol.t[1:].shape).astype(bool)])


        if sol.message == "A termination event occurred.": #then, we just crossed zero    
            direction = direction*-1 #next time the crossing will be in the other direction.  but, we could also be pushing for a limited time and stopping for other reasons.
            pendulum_pushing = True #start pushing!
        elif sol.message == "The solver successfully reached the end of the integration interval.": #we either finished the sim or finished the small push time    
            pendulum_pushing = False

        #print("_-----------------------")



    '''
    t_eval = sol.t #the actual tevals may be slightly different than the nominal ones due to solve IVP details
    theta1 = sol.y[0]
    omega1 = sol.y[1]
    theta2 = sol.y[2]
    omega2 = sol.y[3]
    '''
    t_eval = t_result
    theta1 = y_result[0]
    omega1 = y_result[1]
    theta2 = y_result[2]
    omega2 = y_result[3]


    #now, I have to re-calculate the force since I wasn't able to export them out of the ODE solver.
    fx = np.zeros(t_eval.shape)
    fy = np.zeros(t_eval.shape)
    for idx,time_val in enumerate(t_eval):
        is_pushing = pushing_result[idx]
        fx_calc,fy_calc = external_force(time_val,y_result[:,idx],is_pushing)
        fx[idx] = fx_calc
        fy[idx] = fy_calc


    # Convert to Cartesian coordinates
    x1 = L1 * np.sin(theta1)
    y1 = -L1 * np.cos(theta1)

    x2 = x1 + L2 * np.sin(theta2)
    y2 = y1 - L2 * np.cos(theta2)

    #also calculate the kinetic & potential energies, because, why not.
    #easiest to do this once we got the cartesian coors
    
    t = np.zeros((len(t_eval),2))
    v = np.zeros((len(t_eval),3)) #three because of the extra term, see URL below.
    #y -> [theta1 omega1 theta2 omega2]
    for idx,time_val in enumerate(t_eval):
        #offset both of these so that 0 is there lowest possible position.
        t[idx,0] = m1*g*( y1[idx] +L1)  #mgh.  Not paying attention to linkage for now
        t[idx,1] = m2*g*( y2[idx] +(L2+L1)) #mgh.  Not paying attention to linkage for now

        #V is complex.  It is both the kinetic energy of linkage 1 around the origin, linkage 2 about linkage 1, but also linkage 2 about the origin.
        #from https://physics.umd.edu/hep/drew/pendulum2.html
        #kind of like #1/2 I w^2, where I = m r^2.-->  1/2 m r^2 w^2, where r = linkage length.
        #but pay attention to more masses moving around


        v[idx,0] = 1/2 * (m1+m2) * (L1**2) * (omega1[idx]**2)  
        v[idx,1] = 1/2 * m2 * (L2**2) * (omega2[idx]**2)
        v[idx,2] = m2*L1*L2*omega1[idx]*omega2[idx]*np.cos(theta1[idx] - theta2[idx])

    # -------------------
    # 🎥 Animation
    # -------------------

    fig, (ax,ax2) = plt.subplots(2,1,figsize=(6,10),gridspec_kw={'height_ratios': [3, 1]})
    ax.set_xlim(-2.2, 2.2)
    ax.set_ylim(-2.2, 2.2)
    ax.set_aspect('equal')
    ax.grid()

    linkage_lines, = ax.plot([], [], '-', lw=2)
    first_weight = patches.Circle((0,-1),radius=d_weight/2)
    ax.add_artist(first_weight)
    second_weight = patches.Circle((0,-2),radius=d_weight/2)
    ax.add_artist(second_weight)

    first_linkage = patches.Polygon(rectangle_corners(0,0,0,-1),closed=True,alpha=0.5)
    ax.add_artist(first_linkage)

    second_linkage = patches.Polygon(rectangle_corners(0,-1,0,-2),closed=True,alpha=0.5)
    ax.add_artist(second_linkage)

    force_arrow = ax.arrow(0,0,0,0,color="r",width=0.02,alpha=0.6,zorder=3)
    trace, = ax.plot([], [], '-', lw=1, alpha=0.6)
    time_label = ax.text(0.0, 2.0, "t=0.000 [sec]",ha='center', va='center')

    #plot kinetic and potential energies
    #ax2.plot(t_eval,t[:,0],label="T for L1")
    #ax2.plot(t_eval,t[:,1],label="T for L2")
    ax2.plot(t_eval,np.sum(t,axis=1),label="T (both)")

    #ax2.plot(t_eval,v[:,0],label="V for L1")
    #ax2.plot(t_eval,v[:,1],label="V for L2")
    ax2.plot(t_eval,np.sum(v,axis=1),label="V (both)")

    ax2.plot(t_eval,np.sum(t,axis=1)+np.sum(v,axis=1),label="E total")


    ax2.legend()
    ax2.set_xlabel("Time [sec]")
    ax2.set_ylabel("Energy [Joules]")



    trail_x, trail_y = [], []

    def update(frame):
        # Pendulum line
        linkage_lines.set_data(
            [0, x1[frame], x2[frame]], #x data of the three pivots
            [0, y1[frame], y2[frame]] #y data of the three pivots
        )

        first_weight.set_center((x1[frame],y1[frame]))
        second_weight.set_center((x2[frame],y2[frame]))

        first_linkage.set_xy(rectangle_corners(0,0,x1[frame],y1[frame]))
        second_linkage.set_xy(rectangle_corners(x1[frame],y1[frame],x2[frame],y2[frame]))


        # Trail of second mass
        trail_x.append(x2[frame])
        trail_y.append(y2[frame])

        # limit trail length
        if len(trail_x) > 200:
            trail_x.pop(0)
            trail_y.pop(0)

        #visualize where the force vector is pointing.
        force_arrow.set_data(x=x1[frame],y=y1[frame],dx=fx[frame],dy=fy[frame])

        if pushing_result[frame]:
            force_arrow.set_facecolor((1,0,0))
            force_arrow.set_edgecolor((1,0,0))
        else:
            force_arrow.set_facecolor((0,1,0))
            force_arrow.set_edgecolor((0,1,0))

        #construct the visualization of the force vector
        trace.set_data(trail_x, trail_y)
        time_label.set_text(f"t={t_eval[frame]:2.3f} [sec]")


        return linkage_lines, force_arrow, trace, time_label, first_weight, second_weight, first_linkage, second_linkage

    ani = FuncAnimation(
        fig,
        update,
        frames=len(t_eval),
        interval=20,
        blit=True
    )

    ax.set_title("Forced + Damped Double Pendulum Animation")
    ax2.set_title("Pendulum Energy over time\n(excluding energy stored in magnetic attration)")
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