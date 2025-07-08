import numpy as np
import matplotlib.pyplot as plt

raw = np.loadtxt('clean.csv',skiprows=2,usecols=[0,1],delimiter=',') #usecols [0,1] will allow us to ignore the trailing comma.  Have to skip the first two rows that have header information.


#but, I need to go and get some of that header info.... (for time data, etc)... directly read the file.

with open("clean.csv",'r') as f:
	line_1 = f.readline().strip().split(",")
	line_2 = f.readline().strip().split(",")



t_start = float(line_2[2])
t_span = float(line_2[3])


#index number, measurement time, measurement value
data = np.vstack((raw[:,0],raw[:,0]*t_span + t_start,raw[:,1])).T


#ok, now, for the output sake I need to come up with 2048 equally-spaced rescaled values.

#rescale:

max_ = np.max(data[:,2])
min_ = np.min(data[:,2])

#ok, so, we can't zero-shift.  The best we can do is to scale it so that the max or min (whichever's abs is larger) maxes out at +/- 1
scale = np.max([np.abs(max_),np.abs(min_)])
#print(max_,min_,scale)
rescaled = data[:,2]/scale

data = np.append(data,np.array([rescaled]).T,axis=1) #3rd column is the rescaled data...

print(data)
t_interpolate = np.linspace(data[0,1],data[-1,1],num=2048)
print(t_interpolate)
interpolated = np.interp(t_interpolate,data[:,1],data[:,3])
print(interpolated)

np.savetxt('interpolated.txt',interpolated,fmt='%1.6f') #save the data in the format that the DDS Signal Generator App is expecting, so we can upload as an airbitrary waveform
print("Set Frequency to:",1/(data[-1,1]-data[0,1]),"Hz")
print("Set Amplitude to:",np.max(data[:,2])-np.min(data[:,2])/2,"Volts")
print("Set Offset to: 0")

plt.figure()
plt.subplot(2,1,1)
plt.plot(data[:,1],data[:,2],'b',label="recorded")
plt.ylabel("Volts")
plt.xlabel("Seconds")
plt.grid()

plt.subplot(2,1,2)
plt.plot(rescaled)

plt.show()