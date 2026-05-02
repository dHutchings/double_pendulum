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
m1, m2 = 0.010, 0.010 #10 grams, approximately, revisit!  Need to account for the moment of inertia of the links!
L1, L2 = 2.75*25.4/1000, 2.75*25.4/1000 #2.75 inches
c1, c2 = 0.0001, 0.0001  # roller bearing coefficient of friction, straigt from google...

w = 0.5*25.4/1000 #width around the pendulum, for visualization only..  #Half Inch
d_weight = 1 * 25.4/1000 #the diameter of the weights at M1 / M2.  One inch (bottom weight)

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

    #put a magnet where in the center, at the top link
    magnet_loc = np.array([0,-1*L1])
    distance = np.linalg.norm(magnet_loc-linkage_loc)
    force = 0.00001

    magnetic_force = force/(distance**4)
    magnetic_force=np.clip(magnetic_force,-0.09,0.09) #clip it to a maximum value (just like in real life, the pendulum also has z seperation)
    #magnet force decreases 1/4th due to dipole effect (for now)

    fx,fy = magnetic_force*(magnet_loc-linkage_loc)/distance #break into the correct directions.  Make sure to normalize it by direction
    #print(t,magnetic_force,x1,y1,fx,fy)

    return fx,fy



#need this, because the only way we can make repulsive work is by making the thing stateful.
#for now, let's say it's the opporite of the attractive force
def repulsive_force(t,y):
    fx,fy = attractive_force(t,y)
    return -1*fx, -1*fy
    
def bottom_crossing_event(t,y,is_pushing): #is_pushing is not needed for this function, but must be taken due to solve_IVP syntax

    #gets the integer number of rotations...
    #num_rots = np.trunc(y[0]/(2*np.pi))
    #return_val = y[0] - 2*np.pi*num_rots

    '''
    if y[0] % (2*np.pi) <= np.pi:
        print(y[0], y[0] % (2*np.pi))
        return y[0] % (2*np.pi)
    else:
        print(y[0],-1*y[0] % (2*np.pi))
        return -1*y[0] % (2*np.pi)
    '''

    return_val = 1000*np.rad2deg(y[0]%(np.sign(y[0])*2*np.pi))

    #print("bot.  t=",t,y[0],return_val, end="\t")  #returns only if we are zero crossing any 2pi 

    return return_val
    #return y[0]

    #return y[0]%(2*np.pi) #we are finding zero crossings of theta, and, since we can do multiple rotations now, I need to %2pi
    #we have to find both the bottom zero crossing (push), and the TOP (since we switch the direction we expect the next crossing push to be..?)

def top_crossing_event(t,y,is_pushing): #is_pushing is not needed for this function, but must be taken due to solve_IVP syntax

    dist_from_top = y[0]-np.pi
    num_rots = np.trunc(dist_from_top/(2*np.pi))
    return_val = (dist_from_top) -  2*np.pi*num_rots

    #print("top",y[0],return_val)  #returns only if we are zero crossing any 2pi 

    return return_val


    return  #we are finding zero crossings of theta.
    #we have to find both the bottom zero crossing (push), and the TOP (since we switch the direction we expect the next crossing push to be..?)

def deriv(t, y, is_pushing): #is repulsing is given in by solve IVP, we are told if the magnet is repulsing or not.
    theta1, omega1, theta2, omega2 = y #y is [theta1 omega1 theta2 omega2]  #F comes from the external force function def.  we don't keep track of F since this function returns F dot, instead, we're going to have to re-solve after export.
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

    y0 = [np.deg2rad(30), 0, np.deg2rad(0), 0]


    t_sim = 10
    t_IVP_stopped = 0 
    timestep = 0.001
    pushing_time = 0.05
    starting_push_time = 0#1.25  #in my current simulation, only push as long as it takes to push the link fully out - don't let it come in or come to rest!

    t_meta_timestep = 0.05 #how long to run the numerical simulator without enforcing 2pi peroidicity.

    t_result = None
    y_result = None
    pushing_result = None


    direction = -1 #we expect the direction to be from positive to negative right now
    pendulum_pushing = False
    
    while t_IVP_stopped < t_sim: 

        #I have to incrimendtally solve untill the next change of conditions.
        #either till I stop pushing, or
        #if there is a crossing, OR
        #(minor point) I next enforce 2pi periodicity, then resume.
        #then, a new ODE untill conditions change aagain.


        if pendulum_pushing:
            t_eval = np.arange(t_IVP_stopped,t_IVP_stopped+pushing_time+1*timestep,timestep) #add one extra timestep so the bounds come out right and
            t_final = t_IVP_stopped+pushing_time+2*timestep #add some float on the end - for floating point timestep issues on numpy checking if things are in bounds - but keep it only a few timesteps overall
            #only evaluate for the pushing time
        else:

            t_eval = np.arange(t_IVP_stopped,np.min([t_IVP_stopped+t_meta_timestep+1*timestep,t_sim+1*timestep]),timestep) 
            t_final = np.min([t_IVP_stopped+t_meta_timestep+2*timestep,t_sim+2*timestep]) #again, for floating point timestep issues
            #only evaluate for some small timestep (so that I can enforce 2pi peroidicity) OR to the end of the simulation, whichever is smaller.

        

        bottom_crossing_event.terminal=True #solve only till the first zero-crossing
        top_crossing_event.terminal=True #solve only till the first zero-crossing



        if pendulum_pushing:            
            #no event, since the pendulum pushing is a fixed time.
            sol = solve_ivp(deriv, (t_IVP_stopped,t_final), y0, t_eval=t_eval, max_step=0.001,args=(pendulum_pushing,),rtol=1e-10, atol=1e-12)

        else:
            #events for crossing.  Need both top and bottom crossing events.            
            sol = solve_ivp(deriv, (t_IVP_stopped,t_final), y0, t_eval=t_eval, max_step=0.001,args=(pendulum_pushing,),events=(bottom_crossing_event, top_crossing_event),rtol=1e-10, atol=1e-12)




        t_IVP_stopped = sol.t[-1]


        #don't you dare do 2pi wrap-around, the solver really doensn't like that...
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




        if not pendulum_pushing and (sol.t_events[0].size > 0):
            #print("Bottom Event")
            pendulum_pushing = True #start pushing!
            y0[0] = y0[0] % (np.sign(y0[0])*2*np.pi)             #BUT - I need to enforce periodicity here, every chance I can get.  Zero crossing needs it, but, i can't do it natively inside solve IVP
            y0[2] = y0[2]%(np.sign(y0[2])*2*np.pi)


        elif not pendulum_pushing and (sol.t_events[1].size > 0):
            #print("Top Event")
            pendulum_pushing = False #No pushing here

            y0[0] = -1*y0[0]             #BUT - I need to enforce periodicity here.  No one elese is gonna do it for me.
                                         #it's a -1 since we're going from pi to -pi.
            y0[2] = y0[2]%(np.sign(y0[2])*2*np.pi)
        else:
            #print("End of push, or end of integration time")
            pendulum_pushing = False
            y0[0] = y0[0] % (np.sign(y0[0])*2*np.pi)             #BUT - I need to enforce periodicity here, every chance I can get.  Zero crossing needs it, but, i can't do it natively inside solve IVP
            y0[2] = y0[2]%(np.sign(y0[2])*2*np.pi)




        #print("_-----------------------")



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
        t[idx,0] = m1_tip*g*( y1[idx] +L1)  #mgh.  Not paying attention to linkage for now
        t[idx,1] = m2_tip*g*( y2[idx] +(L2+L1)) #mgh.  Not paying attention to linkage for now

        #V is complex.  It is both the kinetic energy of linkage 1 around the origin, linkage 2 about linkage 1, but also linkage 2 about the origin.
        #from https://physics.umd.edu/hep/drew/pendulum2.html
        #kind of like #1/2 I w^2, where I = m r^2.-->  1/2 m r^2 w^2, where r = linkage length.
        #but pay attention to more masses moving around


        v[idx,0] = 1/2 * (m1_tip+m2_tip) * (L1**2) * (omega1[idx]**2)  
        v[idx,1] = 1/2 * m2_tip * (L2**2) * (omega2[idx]**2)
        v[idx,2] = m2_tip*L1*L2*omega1[idx]*omega2[idx]*np.cos(theta1[idx] - theta2[idx])

    # -------------------
    # 🎥 Animation
    # -------------------
    fig,axd = plt.subplot_mosaic([["A","B"],["A","C"],["A","D"]],figsize=(11,6))
    ax = axd['A']
    ax2 = axd["B"]
    ax3 = axd["C"]
    ax3.sharex(ax2)
    ax4 = axd['D']
    ax4.sharex(ax2)
    ax.set_xlim(-1.1 * (L1+L2+d_weight/2),1.1 * (L1+L2+d_weight/2)) #just a little bit more than the link lengths
    ax.set_ylim(-1.1 * (L1+L2+d_weight/2),1.1 * (L1+L2+d_weight/2))
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

    force_arrow = ax.arrow(0,0,0,0,color="r",width=0.002,alpha=0.6,zorder=3)
    trace, = ax.plot([], [], '-', lw=1, alpha=0.6)
    time_label = ax.text(0.0, 0.9 * (L1+L2+d_weight/2), "t=0.000 [sec]",ha='center', va='center')

    #plot kinetic and potential energies
    #ax2.plot(t_eval,t[:,0],label="T for L1")
    #ax2.plot(t_eval,t[:,1],label="T for L2")
    ax2.plot(t_eval,np.sum(t,axis=1),label="T (both)")

    #ax2.plot(t_eval,v[:,0],label="V for L1")
    #ax2.plot(t_eval,v[:,1],label="V for L2")
    ax2.plot(t_eval,np.sum(v,axis=1),label="V (both)")

    ax2.plot(t_eval,np.sum(t,axis=1)+np.sum(v,axis=1),label="E total")


    ax2.legend()
    ax2.set_ylabel("Energy [Joules]")

    ax3.plot(t_eval,theta1,label="Theta1")
    ax3.plot(t_eval,theta2,label="Theta2")
    ax3.legend()

    ax4.plot(t_eval,pushing_result)
    ax4.set_xlabel("Time [sec]")




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