# -*- coding: utf-8 -*-
"""
Created on Fri Aug 23 16:23:03 2019

This code works with 'kalman_step.ino'
  > Connect your arduino to Serial port
  > Upload the 'kalman_step.ino' to your arduino
  > Eventually modify the port name "COM7" below to match yours
  > Run this code while your arduino transmits data

To run this correctly you need the Python libraries Serial, Numpy and PyQtGraph

PyQtGraph allows for real time viewing.
If not installed, please use the '_matplotlib.py' code

@author: rfetick
"""

from serial import Serial
import pyqtgraph as pg
from pyqtgraph.Qt import QtGui, QtCore
import numpy as np
import re

port = "COM7"
baudrate = 57600

Nstate = 2
Nobs = 2

N = 500 # number of plotted measurements
M = 4000 # number of total measurements before stop
T = np.arange(N,dtype=float)

STATE = np.zeros((N,Nstate))
OBS = np.zeros((N,Nobs))
KAL = np.zeros((N,Nstate))


app = QtGui.QApplication([])

win = pg.GraphicsWindow(title="ARDUINO Kalman filter")
win.resize(1000,600)
win.setWindowTitle('ARDUINO Kalman filter')

p = win.addPlot(title="True (R), Data (G), Kalman (B)")
p.setLabel('left', "State_1")
p.setLabel('bottom', "Time", units='iteration')
p.setYRange(-.5,1.5)
p.showGrid(x=True, y=True)

c1 = p.plot(STATE[:,0], pen=pg.mkPen((250,0,0),width=6), name="True")
c2 = p.plot(OBS[:,0], pen=pg.mkPen((0,200,0),width=2,alpha=0.6), name="Data")
c3 = p.plot(KAL[:,0], pen=pg.mkPen((50,50,250),width=4), name="Kalman")

p2 = win.addPlot(title="True (R), Data (G), Kalman (B)")
p2.setLabel('left', "State_2")
p2.setLabel('bottom', "Time", units='iteration')
p2.setYRange(-.5,1.5)
p2.showGrid(x=True, y=True)

c21 = p2.plot(STATE[:,1], pen=pg.mkPen((250,0,0),width=6), name="True")
c22 = p2.plot(OBS[:,1], pen=pg.mkPen((0,200,0),width=2,alpha=0.6), name="Data")
c23 = p2.plot(KAL[:,1], pen=pg.mkPen((50,50,250),width=4), name="Kalman")

with Serial(port=port, baudrate=baudrate, timeout=1, writeTimeout=1) as port_serie:
    if port_serie.isOpen():
        port_serie.flush()
        for i in range(M):
            ligne = port_serie.readline()
            j = i * (i<N) + (N-1) * (i>=N)
            if i>=N:                
                T = np.roll(T,-1)
                STATE = np.roll(STATE,-1,axis=0)
                OBS = np.roll(OBS,-1,axis=0)
                KAL = np.roll(KAL,-1,axis=0)
            try:
                temp = str(ligne)[2:-5]
                temp = re.findall('[-.0-9]+',temp)
                temp = [float(t) for t in temp]
                #print(temp)
                STATE[j,:] = temp[0:Nstate]
                OBS[j,:] = temp[Nstate:Nstate+Nobs]
                KAL[j,:] = temp[Nstate+Nobs:]
                
                c1.setData(STATE[:,0])
                c2.setData(OBS[:,0])
                c3.setData(KAL[:,0])
                
                c21.setData(STATE[:,1])
                c22.setData(OBS[:,1])
                c23.setData(KAL[:,1])
                
                pg.QtGui.QApplication.processEvents()
                
            except:
                print("Exception: %s"%ligne)
        port_serie.close()
