import matplotlib.pyplot as plt
import numpy as np
import scipy
import pandas as pd
from tabulate import tabulate
import os
import argparse


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

    d['Measurment Duration'] = pd.to_timedelta(d['Duration']).dt.total_seconds()

    d.drop('Duration',axis=1)


    #the duration into the test that the measurement was taken.    
    d['Test Duration'] = d['Start Time'] - d['Start Time'][0]
    d['Test Duration'] = d['Test Duration'].dt.total_seconds()



    return d



def rev_D3():
    files = ["Rev_D3_Amazon_AA.csv","Rev_D3_Energizer_Max_partial.csv"]
    plt.figure()

    for idx,f in enumerate(files):
        d = load_sanitize_csv(f)

        plt.subplot(len(files),1,idx+1)

        def format_seconds_to_hours(seconds, pos):
            hours = int(seconds // 3600)
            minutes = int((seconds % 3600) // 60)
            # You can customize the format string as needed
            return f"{hours:d}:{minutes:02d}"

        plt.plot(d['Test Duration'],d['Sample V DC'])
        plt.gca().xaxis.set_major_formatter(format_seconds_to_hours)
        plt.title(f)




    plt.show(block=True)

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