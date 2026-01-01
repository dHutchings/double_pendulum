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
        d[header] = pd.to_datetime(d[header])

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
    files = ["Rev_D3_Amazon_AA.csv","Rev_D3_Energizer_Max_partial.csv"]#,"Rev_D3_Duracell_first_mechanical.csv","Rev_D3_Duracell_add_spacer_untuned.csv","Rev_D3_Duracell_add_spacer_change_tuning.csv","Rev_D3_Energizer_Max_add_spacer_change_tunings.csv","Rev_D3_HDX_add_spacer_change_tunings.csv"]

    fig, axes = plt.subplots(len(files), 1, sharex=True, figsize=(8, 6))

    locator,formatter = get_xaxis_formatters()

    for f,ax in zip(files,axes):
        d = load_sanitize_csv(f)




        ax.xaxis.set_major_locator(locator)
        ax.xaxis.set_major_formatter(formatter)


        for data in ["Sample V DC","Average V DC","Min V DC","Max V DC"]:
            ax.plot(d['Test Duration'],d[data],label=data[:-5])


        d = detect_restarts(d)
        if 'Reset' in d.columns:
            ax.plot(d['Test Duration'],d['Reset'],label="Reset",marker="*")
        ax.set_title(f)

        ax.legend()




    plt.show(block=True)

def detect_restarts(d):
    #an ok test - inferring from battery voltage - whether I'm resetting or not.
    #not perfect, but, it works well enough.  Given that the fluke 289 only measures battery voltage, it's a good guess.
    d["Reset"] = np.logical_and( (d['Max V DC'] - d["Min V DC"] )> 1 , (d['Max V DC'] - d["Average V DC"] ) > 0.25)

    d["Ongoing Reset"] = False #new column

    def foo(row):
        #print(row)

        #Very fortunately, the CSV includes a "reading" column from the Fluke.
        #which is passed through.
        #that gives me an sample index to search for.
        #it starts the count at 1.

        idx = int(row['Reading']) - 1
        
        time = row['Test Duration'] #the time of this measurement

        indexes = (d['Test Duration'] > time) & (d['Test Duration'] < (time + 60/(24*60*60)) )#search for the next 60 seconds of data.  Remember that under the hood we are living in day territory here
        #this is a Giant array of true / false values, the length of which is identical to our original data array.

        if indexes.sum() == 0:
            df.at[idx,'Ongoing Reset'] = False
        else:
            df.at[idx,'Ongoing Reset'] = np.any(d["Reset"])



        #if ALL the 

        print(idx,indexes.sum())
        #print(d.iloc[idx,:])
        print("-------")

    d.apply(foo,axis=1)

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