# -*- coding: utf-8 -*-
"""
Created on Sat Aug 31 16:16:49 2019

This code works with 'kalman_step.ino'
  > Connect your arduino to Serial port
  > Upload the 'kalman_step.ino' to your arduino
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

Nstate = 2
Nobs = 2

N = 500 # number of measurements
T = np.arange(N,dtype=float)

STATE = np.zeros((N,Nstate))
OBS = np.zeros((N,Nobs))
KAL = np.zeros((N,Nstate))

with Serial(port=port, baudrate=baudrate, timeout=1, writeTimeout=1) as port_serie:
    if port_serie.isOpen():
        port_serie.flush()
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
                KAL[i,:] = temp[Nstate+Nobs:]
                
            except:
                print("Exception: %s"%ligne)
        port_serie.close()


#%% PLOT RESULTS

plt.figure(1)
plt.clf()

for i in range(2):
    plt.subplot(1,2,i+1)
    plt.plot(STATE[:,i],linewidth=4,label='True')
    plt.scatter(T,OBS[:,i],alpha=0.8,label='Obs')
    plt.plot(KAL[:,i],linewidth=2,label='Kalman')
    plt.legend()
    plt.xlabel("Time [iter]")
    plt.ylabel("Data")
    plt.grid()
    plt.xlim(0,N)
    plt.ylim(-1,2)
