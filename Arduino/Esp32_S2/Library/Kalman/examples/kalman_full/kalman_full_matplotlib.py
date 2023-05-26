# -*- coding: utf-8 -*-
"""
Created on Sat Aug 31 17:16:57 2019

This code works with 'kalman_full.ino'
  > Connect your arduino to Serial port
  > Upload the 'kalman_full.ino' to your arduino
  > Eventually modify the port name "COM7" below to match yours
  > Run this code while your arduino transmits data

To run this correctly you need the Python libraries Serial, Numpy and Matplotlib


@author: rfetick
"""


from serial import Serial
import matplotlib.pyplot as plt
import numpy as np
import re

port = "COM7"
baudrate = 57600

Nstate = 3
Nobs = 2

N = 300 # number of measurements
T = np.arange(N,dtype=float)
TT = np.concatenate((T,T[::-1]))

STATE = np.zeros((N,Nstate))
OBS = np.zeros((N,Nobs))
KAL = np.zeros((N,Nstate))
P = np.zeros((N,Nstate**2))

with Serial(port=port, baudrate=baudrate, timeout=1, writeTimeout=1) as port_serie:
    if port_serie.isOpen():
        port_serie.flush()
        for i in range(50):
            ligne = port_serie.readline() # remove first data
        for i in range(N):
            ligne = port_serie.readline()
            if((i%50)==0):
                print("Iteration %u / %u"%(i,N))
            try:
                temp = str(ligne)[2:-5]
                temp = re.findall('[-.0-9]+',temp)
                temp = [float(t) for t in temp]
                #print(temp)
                STATE[i,:] = temp[0:Nstate]
                OBS[i,:] = temp[Nstate:Nstate+Nobs]
                KAL[i,:] = temp[Nstate+Nobs:2*Nstate+Nobs]
                P[i,:] = temp[2*Nstate+Nobs:]
                
            except:
                print("Exception: %s"%ligne)
        port_serie.close()


#%% PLOT RESULTS

plt.figure(1)
plt.clf()

ax1 = plt.subplot(311)
PP = np.concatenate((KAL[:,0]+np.sqrt(P[:,0]),KAL[::-1,0]-np.sqrt(P[::-1,0])))
plt.fill(TT,PP,color="C1",alpha=0.3)
plt.plot(STATE[:,0],linewidth=4,label='True')
plt.scatter(T,OBS[:,0],alpha=0.8,color="C2",label='Obs')
plt.plot(KAL[:,0],linewidth=2,label='Kalman')
plt.legend()
plt.xlabel("Time [iter]")
plt.ylabel("Position")
plt.grid()
plt.xlim(0,N)
plt.ylim(-1.5,1.5)
plt.tight_layout()

plt.subplot(312, sharex=ax1)
PP = np.concatenate((KAL[:,1]+np.sqrt(P[:,4]),KAL[::-1,1]-np.sqrt(P[::-1,4])))
plt.fill(TT,PP,color="C1",alpha=0.3)
plt.plot(STATE[:,1],linewidth=4,label='True')
#plt.scatter(T,OBS[:,0],alpha=0.8,label='Obs')
plt.plot(KAL[:,1],linewidth=2,label='Kalman')
plt.legend()
plt.xlabel("Time [iter]")
plt.ylabel("Speed")
plt.grid()
plt.xlim(0,N)
plt.ylim(-5,5)

plt.subplot(313, sharex=ax1)
PP = np.concatenate((KAL[:,2]+np.sqrt(P[:,-1]),KAL[::-1,2]-np.sqrt(P[::-1,-1])))
plt.fill(TT,PP,color="C1",alpha=0.3)
plt.plot(STATE[:,2],linewidth=4,label='True')
plt.scatter(T,OBS[:,1],alpha=0.8,color="C2",label='Obs')
plt.plot(KAL[:,2],linewidth=2,label='Kalman')
plt.legend()
plt.xlabel("Time [iter]")
plt.ylabel("Acceleration")
plt.grid()
plt.xlim(0,N)
plt.ylim(-15,15)