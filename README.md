# OTB Frequency Resample Module

This standalone [OTB][OTB] module is a wrapper of the [Sirius library][Sirius].
It provides an image to image filter which resamples an image in the frequency domain.
This filter is packaged in the application *FrequencyResample*.

## Overview

* [Orfeo Toolbox][OTB]
* [Sirius library][Sirius]
* [Sirius documentation][Sirius Documentation]

## How to build

This module is using [CMake] to build its libraries and executables.

### Requirements

* C++14 compiler (GCC >5)
* [CMake] >=3.2
* [GDAL] development kit, >=2
* [FFTW] development kit, >=3
* [Sirius library][Sirius]

### Dependencies

OTB Frequency Resample Module depends on the [Sirius library][Sirius].
In order to find the library using [CMake] `find_package` helper, `SIRIUS_ROOT` **must** be set to a Sirius install directory.

### Build as standalone module

This OTB module can be built outside the OTB project as a standalone module. It only requires an OTB build tree (e.g. SuperBuild generation).

#### Options

* `CMAKE_BUILD_TYPE`: Debug, Release, RelWithDebInfo and MinSizeRel
* `SIRIUS_ROOT`: path to Sirius install directory
* `OTB_BUILD_MODULE_AS_STANDALONE`: set to `ON` to build `FrequencyResample` as a standalone module
* `OTB_DIR`: path to the OTB build tree [CMake] directory
* `BUILD_TESTING`: set to `ON` to enable tests

#### Example

```sh
git clone https://github.com/CS-SI/OTB-SIRIUS.git
mkdir .build
cd .build
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DSIRIUS_ROOT=/path/to/Sirius/install/directory \
         -DOTB_BUILD_MODULE_AS_STANDALONE=ON \
         -DOTB_DIR=/path/to/OTB/SDK/lib/cmake/OTB-x.y \
         -DBUILD_TESTING=ON
cmake --build . --target FrequencyResample-all
```

### Local build of the remote module

This module can also be built inside the OTB project (cf. [OTB documentation][OTB Local Build]).

#### Options

* `Module_FrequencyResample`: set to `ON` to activate FrequencyResample remote module
* `SIRIUS_ROOT`: path to Sirius install directory

#### Example

```sh
git clone https://github.com/CS-SI/OTB-SIRIUS.git FrequencyResample
cp -r FrequencyResample /path/to/OTB/Modules/Remote
cd /path/to/OTB/build/directory
cmake /path/to/OTB -DModule_FrequencyResample=ON -DSIRIUS_ROOT=/path/to/Sirius/install/directory
cmake --build . --target FrequencyResample-all
```

## How to use

### Host requirements

* [GDAL][GDAL] library
* [FFTW3][FFTW] library

### FrequencyResample application

The following example will zoom in `/path/to/input/image.tif` by 2, apply the filter `/path/to/filter/image.tif` to the resampled image and write the output into `/path/to/output/image.tif`.

```sh
# CWD is the OTB build directory
./bin/otbcli_FrequencyResample \
    -in /path/to/input/image.tif \
    -out /path/to/output/image.tif \
    -resampling.ratio 2:1 \
    -filter.path /path/to/filter/image.tif
```

### API

The following code is a boilerplate to create and initialize the `FrequencyResampleFilter` component.


```cpp
#include "otbFrequencyResampleFilter.h"

using FilterType = otb::FrequencyResampleFilter<DoubleImageType>;

// initialization parameters
sirius::ZoomRatio zoom_ratio(2, 1);

sirius::Image filter_image = {...};
sirius::Filter freq_filter = sirius::Filter::Create(filter_image, zoom_ratio);

sirius::ImageDecompositionPolicies image_decomposition =
      sirius::ImageDecompositionPolicies::kRegular;
sirius::FrequencyZoomStrategies zoom_strategy =
      sirius::FrequencyZoomStrategies::kPeriodization;

FilterType::Pointer i2i_filter = FilterType::New();

// init filter
i2i_filter->Init(zoom_ratio, std::move(freq_filter),
                 image_decomposition, zoom_strategy);

i2i_filter->SetInput(...);
```

[OTB]: https://www.orfeo-toolbox.org "Orfeo Toolbox"
[Sirius]: https://github.com/CS-SI/SIRIUS "Sirius library"
[Sirius Documentation]: https://CS-SI.github.io/SIRIUS/html/Sirius.html "Sirius documentation"
[CMake]: https://cmake.org/ "CMake"
[GDAL]: http://www.gdal.org/ "Geospatial Data Abstraction Library"
[FFTW]: http://www.fftw.org/ "Fastest Fourier Transform in the West"
[OTB Local Build]: https://wiki.orfeo-toolbox.org/index.php/How_to_write_a_remote_module#Including_a_remote_module_in_OTB "Local build of a remote module"


