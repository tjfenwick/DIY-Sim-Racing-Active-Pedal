# Kalman ![bdg](https://img.shields.io/github/license/rfetick/Kalman) ![bdg](https://img.shields.io/github/v/release/rfetick/Kalman) ![bdg](https://img.shields.io/github/commits-since/rfetick/Kalman/latest)

**Implement Kalman filter for your Arduino projects**

:information_source: Visit the github project's page https://github.com/rfetick/Kalman

:ballot_box_with_check: Tested successfully on _Arduino Uno_ and _Nano_ (ATmega 328P old bootloader) 

:arrows_counterclockwise: Any suggestion or issue? Please write to https://github.com/rfetick/Kalman/issues

## I. Long description

Other Kalman libraries already exist for Arduino, but so far I have only seen filters applied to independent scalars. The matricial implementation of this project allows to use the full power of the Kalman filter to coupled variables. It allows to merge measurements from multiple sensors such as accelerometers, GPS, ultrasound (distance) or pressure (altitude) sensors...

This library is adapted to your most sophisticated projects. In order to use it you need some knowledge about matrix formalism and be able to write (or find on internet) the actual state equations of your system.

## II. Using the Kalman library

### 1. Prerequisites

`BasicLinearAlgebra`: You might find it in the library manager of your Arduino IDE, or directly download it at https://github.com/tomstewart89/BasicLinearAlgebra

### 2. Downloading the Kalman library

This library is available in the official Arduino library manager. Just type `Kalman` and you should find it.

Other possibility is to download (or clone) this project from GITHUB and add it to your `Arduino/libraries/` folder.

### 3. Start using the library in your Arduino projects

See the [GETTING_STARTED](GETTING_STARTED.md) file.

See also the `examples\` folder.

## III. Possible issues

* The library `BLA::Matrix` seems to throw errors for matrices of size `<1,1>`. So the Kalman library will only work for `Nstate>1` and `Nobs>1`. For one-dimensional Kalman filters, please refer to other Arduino libraries.

* In case of issues with matrices computation please make sure to use the latest version of `BasicLinearAlgebra`. It works fine on my side with `BasicLinearAlgebra` version `3.2`, however compatibilities issues may occur with different versions.

* Size of matrices has to be relatively small due to the limited SRAM memory of Arduino. Effort has been made to reduce SRAM usage.

* An issue has been reported with Arduino Nano IoT 33: the program compiles but is not correctly loaded to Arduino Nano IoT 33. I might investigate this in the future.
