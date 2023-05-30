
from pandas import *
import matplotlib.pyplot as plt

# rolling window length to smooth the velocity signal. Adjust as needed
velocityWindowLength = 3

fileName = 'C:/Users/chris/Downloads/digital.csv'


if __name__ == '__main__':

    data = read_csv(fileName)

    # compute rising edges of step signal
    data['step_edge_rising'] = data['Channel 2'].diff() > 0.5
    data['step_edge_falling'] = data['Channel 2'].diff() < 0.5

    # compute position
    data['movement_dir'] = data['Channel 0'] * 2 - 1
    data['movement'] = data['movement_dir'] * data['step_edge_rising']
    data['position'] = data['movement'].cumsum()

    # compute velocity & acceleration
    data['velocity_p'] = data['position'].diff().rolling(velocityWindowLength).max()
    data['velocity_n'] = data['position'].diff().rolling(velocityWindowLength).min()
    data['velocity'] = data['velocity_p'] + data['velocity_n']
    data['acceleration'] = data['velocity'].diff()




    # plot routine
    fig, ax = plt.subplots(3, 2, sharex=True, sharey=False)


    plt.subplot(321)
    plt.plot(data['Time [s]'], data['Channel 2'])
    plt.xlabel('time in s')
    plt.ylabel('Step+')
    plt.grid('on')

    plt.subplot(323)
    plt.plot(data['Time [s]'], data['Channel 0'])
    plt.xlabel('time in s')
    plt.ylabel('Dir+')
    plt.grid('on')

    plt.subplot(325)
    plt.plot(data['Time [s]'], data['Channel 1'])
    plt.xlabel('time in s')
    plt.ylabel('Debug')
    plt.grid('on')

    plt.subplot(322)
    plt.plot( data['Time [s]'], data['position'] )
    plt.xlabel('time in s')
    plt.ylabel('position in steps')
    plt.grid('on')

    plt.subplot(324)
    plt.plot(data['Time [s]'], data['velocity'])
    plt.xlabel('time in s')
    plt.ylabel('velocity in steps/s')
    plt.grid('on')

    plt.subplot(326)
    plt.plot(data['Time [s]'], data['acceleration'])
    plt.xlabel('time in s')
    plt.ylabel('acceleration in steps/s^2')
    plt.grid('on')

    plt.tight_layout()

    plt.draw()
    plt.show(block=True)



