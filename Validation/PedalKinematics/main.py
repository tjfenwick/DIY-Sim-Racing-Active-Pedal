import numpy as np
import matplotlib.pyplot as plt



# parameterize pedal geometry here
# see https://de.wikipedia.org/wiki/Kosinussatz
a = 200.0 # length of the loadcell rod
b = 200.0 # length between pedal pivots
c0 = 80.0 # vertical offset of rear pivot point from lower front pivot
c1 = 240.0 # horizontal offset of rear pivot point from lower front pivot

maxRpm = 5000 # maximum servo RPMs
spinglePitch_inMm = 5 # spindle pitch








if __name__ == '__main__':
    # calculation
    v_sled = spinglePitch_inMm * (maxRpm / 60)  # mm/s
    max_T = 100 / v_sled  # max travel / sled velocity

    # define time vector
    t = np.linspace(0, max_T, 1000)
    delta_T = t[1] - t[0]

    delta_c = v_sled * t # sled offset for each timestep
    c = np.sqrt( c0**2 + (c1+delta_c)**2) # calculate distance between lower front pivot point to rear pivot point

    # caclulate pedal angle
    nom = b**2 + c**2 - a**2
    den = 2 * b * c
    alpha = np.arccos(nom / den) * 180 / np.pi

    # numerically calculate angular velocity
    d_alpha = np.diff(alpha) / delta_T

    # plot routine
    plt.subplot(2,1,1)
    plt.plot(t, alpha)
    plt.xlabel('t')
    plt.ylabel('pedal angle in °')

    plt.subplot(2, 1, 2)
    plt.plot(t[:-1], d_alpha)
    plt.xlabel('t')
    plt.ylabel('pedal angular velocity in °/s')

    plt.draw()
    plt.show(block=True)

