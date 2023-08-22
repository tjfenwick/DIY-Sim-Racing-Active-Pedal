import numpy as np
import matplotlib.pyplot as plt



# parameterize pedal geometry here
# see https://de.wikipedia.org/wiki/Kosinussatz
a = 200.0 # length of the loadcell rod
b = 120.0 # length between pedal pivots
c0 = 80.0 # vertical offset of rear pivot point from lower front pivot
c1 = 240.0 # horizontal offset of rear pivot point from lower front pivot



maxRpm = 5000 # maximum servo RPMs
spinglePitch_inMm = 5 # spindle pitch

eta = 0.83 #spindle efficiency
T = 1.1 # max motor torque: iSV57-t 130W 1.1 Nm
lengthTillPedal = b + 100 # length between lower pivot point and pedal face center
# see https://www.leadshine.com/downUeditor/image/20220609/26f4270682821c78ce90b12b5bc9e9af.JPG
# see https://www.leadshine.com/product-detail/iSV-B23130T-S21.html







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

    # calculate the max force at spindle
    # see https://tech.thk.com/de/products/pdf/de_b17_009.pdf
    F_a = 2 * np.pi * T / spinglePitch_inMm * eta * 1e3

    # calculate the max force at pedal plate
    # sin(phi) * F_p + F_lp = F_a
    # F_p: axial pedal force
    # F_lp: horizontal force at lower pivot point
    # F_lp = lengthTillPedal / b * F_p
    alpha0 = np.arcsin( c0 / c ) * 180 / np.pi
    phi = alpha + alpha0
    F_p = F_a / ( np.sin(phi * np.pi / 180) * lengthTillPedal / b  )

    # plot routine
    plt.subplot(3,1,1)
    plt.plot(t, alpha)
    plt.xlabel('t in s')
    plt.ylabel('pedal angle in °')

    plt.subplot(3, 1, 2)
    plt.plot(t[:-1], d_alpha)
    plt.xlabel('t in s')
    plt.ylabel('pedal angular velocity in °/s')

    plt.subplot(3,1,3)
    plt.plot(t, F_p)
    plt.xlabel('t in s')
    plt.ylabel('max pedal force in N')

    plt.tight_layout()
    plt.draw()
    plt.show(block=True)

