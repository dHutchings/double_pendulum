import matplotlib.pyplot as plt
import numpy as np
import scipy
import pandas as pd
from tabulate import tabulate
import os
import argparse

import matplotlib.dates as mdates

def get_xaxis_formatters():

    locator = mdates.AutoDateLocator()
    formatter = mdates.AutoDateFormatter(locator)


    def total_hours(x, pos=None):  #x is a number in total days!
        x *= 24 #multiply into hours.

        hours = int(x)
        min = int( (x - hours)*60)
        if(min < 10):
            label = str(hours) + ":0" + str(min)
        else:
            label = str(hours) + ":" + str(min)
        return label

    def total_hours_minutes(x, pos=None):
        x *= 24 #multiply into hours.

        hours = int(x)
        hours_label = str(hours)
        min = int( (x - hours)*60)
        sec = int( (x-hours)*60 - min)*60

        if min < 10:
            min_label = "0" + str(min)
        else:
            min_label = str(min)

        if sec < 10:
            sec_label = "0" + str(sec)
        else:
            sec_label = str(sec)

        return hours_label + ":" + min_label + ":" + sec_label

    formatter.scaled.pop(365)
    formatter.scaled[1] = "%d-%H" #if I'm showing days, do days (which will effectively be total days) / hours.
    formatter.scaled[1/24] = total_hours #if I'm showing hours, do hours:minutes
    formatter.scaled[1/24/60*15] = total_hours_minutes #if I'm showing 15 minutes, do hours:minutes:Min:Sec
    formatter.scaled[1/24/60] = "%M:%S" #zoom in and give me seconds.  This will mean
    formatter.scaled.pop(1/24/60/60) #don't go that far in
    formatter.scaled.pop(1/24/60/60/1000000) #don't go that far in
    formatter.scaled.pop(0.0006944444444444445 ) #I tried to overwrite this, but rounding issues got in the way.  Just manually do it.

    return locator,formatter

def load_sanitize_csv(csv_path):
    #the Fluke 289s default CSV is very info-dense.
    #and wierdly formatted.
    #this helps sanitize that stuff.

    #the first 9 rows are an info-dense header
    #the CSV also has a special windows encoding (at least when downloaded to doug's windows laptop)
    d = pd.read_csv(csv_path,skiprows=9,encoding='windows-1252')

    num_samples = np.max(d["Reading"]) #get the number of samples

    d = d.head(int(num_samples)) #throw away most of the data
    #this is because each data line shows up TWICE.
    #first in a line with the reading/min/max/duration, etc.
    #then, in many lines with the info broken out.
    #since we have pandas we can deal with many columns, so, throw away the indidual line readings.

    data_headers = ["Sample","Average","Min","Max"]


    #now, get rid of the strings and such.  ALso, get rid of the overloads.

    def sanitize_line(text):
        if text.find("OL") != -1: #there is an OL, overload.  we don't have valid data, it can be a +/- OL.
            return np.nan
        elif text.find(' V DC') != -1:
            text = text.replace(' V DC',"")
            #sometimes, the text is only whitespace.
            if text.isspace():
                return np.nan
            else:
                return float(text)
        else:
            return np.nan


    
    for header in data_headers:

        d[header] = d[header].apply(sanitize_line)


    #rename these headers with the units as the last step
    for header in data_headers:
        d = d.rename(columns={header:header+" V DC"})


    time_headers = ["Start Time","Max Time","Min Time","Stop Time"]
    for header in time_headers:
        #12/16/2025 12:14:15.3
        fmt = "%m/%d/%Y %H:%M:%S.%f" #promise datetime that we have this format.  It speeds up analysis - python doen't have to infer.
        #It's minorly wrpmg (we had to promise zero-padded days and months, wheras the data isn't zero-padded) but, it seems to work.
        #%f can support UP to microsecond precision, but, per https://stackoverflow.com/questions/50236669/converting-string-with-decimals-into-datetime, it zero-pads on the right.
        d[header] = pd.to_datetime(d[header],format=fmt) #https://pandas.pydata.org/pandas-docs/version/1.0.3/reference/api/pandas.to_datetime.html

    d['Measurment Duration'] = pd.to_timedelta(d['Duration']).dt.total_seconds()/(24*60*60)  #matplitlib dates API likes days, unfortunately.

    d.drop('Duration',axis=1)


    #the duration into the test that the measurement was taken.    
    d['Test Duration'] = d['Start Time'] - d['Start Time'][0]
    d['Test Duration'] = d['Test Duration'].dt.total_seconds()/(24*60*60) #matplitlib dates API likes days, unfortunately.



    return d


def format_seconds_to_hours(seconds, pos):
    hours = int(seconds // 3600)
    minutes = int((seconds % 3600) // 60)
    # You can customize the format string as needed
    return f"{hours:d}:{minutes:02d}"

def rev_C():
    files = ["Rev_C_Duracell.csv","Rev_C_Energizer_Max_partial.csv"]

    fig, axes = plt.subplots(len(files), 1, sharex=True, figsize=(8, 6))
    locator,formatter = get_xaxis_formatters()

    d_for_offset = load_sanitize_csv("Rev_C_Duracell.csv")

    for f,ax in zip(files,axes):
        d = load_sanitize_csv(f)



        if f != "Rev_C_Duracell.csv":
            #offset the time index of this dataset so it aligns with the voltage curve of Rev_C_Duracell.

            idx = np.where(d_for_offset['Sample V DC'] < d['Sample V DC'][0])[0] #Find the index in the duracell data where it first dips below the data in my file.
            t_offset = d_for_offset['Test Duration'][idx[0]]

            d["Test Duration"] = d["Test Duration"] + t_offset

        #an ok test - inferring from battery voltage - whether I'm resetting or not.
        #not perfect, but, it works well enough.  Given that the fluke 289 only measures battery voltage, it's a good guess.
        d["Reset"] = np.logical_and( (d['Max V DC'] - d["Min V DC"] )> 1 , (d['Max V DC'] - d["Average V DC"] ) > 0.25)




        for data in ["Sample V DC","Average V DC","Min V DC","Max V DC"]:
            ax.plot(d['Test Duration'],d[data],label=data[:-5])


        #ax.plot(d['Test Duration'],d['Reset'],label="Reset",marker="*")

        #ax.plot(d['Test Duration'],d['Sample V DC'])
        ax.xaxis.set_major_formatter(format_seconds_to_hours)
        ax.set_title(f)
        ax.legend()

    #can only reformat axes AFTER i do the offset, hence, why it's down here.
    for ax in axes:
        ax.xaxis.set_major_locator(locator)
        ax.xaxis.set_major_formatter(formatter)

    plt.gca().set_xlabel("Time (hours:min), conclusion: Duracel better than Energizer Max")

    plt.show(block=True)


def rev_D3():
    files = ["Rev_D3_Amazon_AA.csv","Rev_D3_Energizer_Max_partial.csv","Rev_D3_Duracell_first_mechanical.csv","Rev_D3_Duracell_add_spacer_untuned.csv","Rev_D3_Duracell_add_spacer_change_tuning.csv","Rev_D3_Energizer_Max_add_spacer_change_tunings.csv","Rev_D3_HDX_add_spacer_change_tunings.csv"]

    fig, axes = plt.subplots(len(files)+1, 2, sharex=True, figsize=(8, 6),gridspec_kw={'height_ratios': [ *[3]*len(files), 1],'width_ratios':[5,1]},layout='constrained')

    #for every axis in the bottom row
    for ax in axes[-1,:]:
        ax.axis("off") #turn axis off for cleaner look.   We will put the legend here.
    for ax in axes[:,-1]: #for the last column (table column)
        ax.axis("off") #turn axis off for cleaner look.   We will put the legend here.

    locator,formatter = get_xaxis_formatters()

    for idx,f in enumerate(files):
        
        d = load_sanitize_csv(f)

        plt_ax = axes[idx,0] #this is the main axis I want everything plotted on.
        table_ax = axes[idx,1] #the smaller axis off to the right: that's where the table goes
        print(f)

        #truncate the test anywhere where the average battery voltage is < 5 Volts
        #the boost converter probably (??) browns out.  Nothing good happens beyond that point.



        #d = d.iloc[:(end_idx+10)]

        d['Smoothed Average'] = simple_moving_average(d['Average V DC'],10)

        end_idx = d['Smoothed Average'] <= 6.5
        end_idx = end_idx.to_numpy()
        end_idx = end_idx[10:] #skip the first 10 data points. - sometimes for the first data point or two the multimeter has a low average
        end_idx = np.argwhere(end_idx == 1)[0][0]
        d.attrs["Battery Dead Index"] = end_idx+10
        d.attrs["Battery Dead Time (days)"] = d.loc[end_idx+10,'Test Duration']





        plt_ax.xaxis.set_major_locator(locator)
        plt_ax.xaxis.set_major_formatter(formatter)


        for data in ["Sample V DC","Average V DC","Min V DC","Max V DC"]:
            plt_ax.plot(d['Test Duration'],d[data],label=data[:-5])
        #Only plot this is we need to debug the test cutoff conditions.
        #plt_ax.plot(d['Test Duration'],d["Smoothed Average"],label="Smoothed Average") #
        plt_ax.axvline(x=d.attrs["Battery Dead Time (days)"],label="Battery Dead",color='red',alpha=0.5)

        
        d = detect_restarts(d)
        if 'Reset' in d.columns:
            plt_ax.plot(d['Test Duration'],d['Reset'],label="Reset",marker="*")
        if 'Ongoing Reset' in d.columns:
            plt_ax.plot(d['Test Duration'],d['Ongoing Reset'],label="Ongoing Reset",marker="*")
            #plt_ax.plot(d['Test Duration'],np.diff(d['Ongoing Reset'],prepend=0),label="Ongoing Reset DIFF",marker="*")


        plt_ax.set_title(f)

        #I don't want to see some things on the metadata table.  Far easier to just remove it.
        d.attrs.pop("Battery Dead Index")
        d.attrs["Battery Dead Time (hrs)"] = d.attrs["Battery Dead Time (days)"]*24
        d.attrs.pop("Battery Dead Time (days)")

        #break out some list comphrenension.
        #to trim to two decimals in case of floats.
        table_ax.table(cellText=[ [x] if x==int(x) else [f"{x:.2f}"] for x in d.attrs.values() ],rowLabels = [x for x in d.attrs.keys() ],loc='center',bbox=[0.5, 0.15, 0.5, 0.7])


        if f == files[-1]: #the last test I want to see
            plt_ax.xaxis.set_tick_params(labelbottom=True) #turn on the x=ticks again, by default they are off in sharex


    ax = axes[-1,0]
    #Pl;op the first plots legend on the last whitespace plot
    #assume all plots have the same legend
    handles, labels = axes[0,0].get_legend_handles_labels()
    by_label = dict(zip(labels, handles))
    ax.legend(by_label.values(), by_label.keys(),loc='upper center',ncol=3) #make the legend really wide




    plt.show(block=True)
    
#straight from google AI overview
def simple_moving_average(data, window_size):
    # Create a kernel (weights) where each element is 1/window_size
    weights = np.ones(window_size) / window_size
    # Convolve the data with the weights
    # 'valid' mode returns only the parts where the kernel fully overlaps the data
    sma = np.convolve(data, weights, mode='same')
    return sma

#calculate if I am in an ongoing continious chained reset situation, determine which restart is finally the last one / the 'successful one.'
#due to new tunings often the pendulum needs to attempt to restart more than once in close succession to successfully start for real.
#we call a sequence of restart attempts a "Continious Restart" because the pendulum is continually restarting untill its successful.
def detect_successful_restart(row):
    #Operates on only one row of data at a time.
    #it IS possible to pass in the full dataframe as an optional argument, but accessing each item out of there... what's the point?
    #instead, using the previous for-loop, do all the pre-processing that is needed.
    #so that this function can detect continious restarts by examining only the data in this line, nothing more.
    #much faster.

    idx = int(row['Reading']) - 1
    if row["Reset"]: #I am currently resetting
        row['Ongoing Reset'] = True
    elif row['Time from Previous Reset'] <= (60/(24*60*60)): #I Reset less than 1 minute ago, so i'm still in the energy-pump process.

        #the time difference between my previous reset and my next reset is less than 1 minute
        if (row['Time to Next Reset'] + row['Time from Previous Reset']) <= 60/(24*60*60):
            row['Ongoing Reset'] = True
        else:
            row['Ongoing Reset'] = False
    else:
        row['Ongoing Reset'] = False


    return row #must return the edited row



def detect_restarts(d):
    #an ok test - inferring from battery voltage - whether I'm resetting or not.
    #not perfect, but, it works well enough.  Given that the fluke 289 only measures battery voltage, it's a good guess.
    #d["Reset"] = np.logical_and( (d['Max V DC'] - d["Min V DC"] )> 1 , (d['Max V DC'] - d["Average V DC"] ) > 0.25)

    d["Reset"] = np.logical_and( np.logical_and( (d['Max V DC'] - d["Min V DC"] )> 1 , np.logical_or( (d['Max V DC'] - d["Average V DC"] ) > 0.25,(d['Average V DC'] - d["Min V DC"] ) > 0.25)), d['Reading'] <= d.attrs["Battery Dead Index"])

    d["Ongoing Reset"] = False #new column

    d["Reading of Next Reset"] = np.nan #new column.
    d["Time to Next Reset"] = np.nan #new column.
    reset_idxs = np.argwhere(d["Reset"].to_numpy())
    d.attrs["NumRestartAttempts"] = len(reset_idxs)

    #iterate through all reset indexes
    #for every datapoint calculate time between the previous and next reset.
    idx = 0
    prev_reset = np.nan

    for reset in reset_idxs:
        reset = reset[0] #have to pull it out of the numpy array

        d.loc[idx:reset,"Reading of Next Reset"] = reset 
        d.loc[idx:reset,"Time to Next Reset"] = d.loc[reset,"Test Duration"] -  d.loc[idx:reset,'Test Duration'] 
        if not np.isnan(prev_reset):
            d.loc[idx:reset,"Time from Previous Reset"] =  d.loc[idx:reset,'Test Duration'] - d.loc[prev_reset,"Test Duration"]

        idx = reset
        prev_reset = reset



    d = d.apply(detect_successful_restart,axis=1)

    #ok, now what?
    #now, I need to:
    #A) for each continious restart, how long is it?


    start_continious_idxs = np.argwhere( np.diff(d['Ongoing Reset'],prepend=0) == 1)  #this is - wierdly - the start
    d.attrs["NumSuccessfulRestarts"] = len(start_continious_idxs)
    d.attrs["NumAttemptsPerRestart"] =  d.attrs["NumRestartAttempts"] / d.attrs["NumSuccessfulRestarts"]
    for continious_restart_idx in start_continious_idxs: #For the start of each continious restart
        continious_restart_idx = continious_restart_idx[0] #have to pull it out of the numpy array
        #print(continious_restart_idx, d.loc[continious_restart_idx,"Test Duration"]*24)

    #C) what is the average time between succesful restarts?  How long can the pendulum run?
    #D) 

    d["Ongoing Reset"] = d["Ongoing Reset"]+1


    print(d.attrs)
    print()
    return d



if __name__=="__main__":
    #https://stackoverflow.com/questions/3061/calling-a-function-of-a-module-by-using-its-name-a-string
    parser = argparse.ArgumentParser(description = 'Central File for homeworks - pass in the function you want to run, or, when run, it will ask you which function you will like to run.  Functions --> part of the HW')
    parser.add_argument('fxn',default=None, type = str, nargs='?', help = 'Specify which function to run - must be typed exactly correct.')

    parsed = parser.parse_args()


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