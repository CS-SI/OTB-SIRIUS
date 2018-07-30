#
# Copyright (C) 2018 CS - Systemes d'Information (CS-SI)
#
# This file is part of OTB-SIRIUS
#
#     https://github.com/CS-SI/OTB-SIRIUS
#
# OTB-SIRIUS is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# OTB-SIRIUS is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with OTB-SIRIUS.  If not, see <https://www.gnu.org/licenses/>.
#

otb_module_test()

set(${otb-module}_Tests
    otbFrequencyResampleTestDriver.cxx
    otbFrequencyResampleTest.cxx
)

add_executable(otbFrequencyResampleTestDriver ${${otb-module}_Tests})
target_link_libraries(otbFrequencyResampleTestDriver
    ${${otb-module}-Test_LIBRARIES}
    libsirius-static)

otb_module_target_label(otbFrequencyResampleTestDriver)

otb_add_test(NAME TestFrequencyResample
             COMMAND otbFrequencyResampleTestDriver FrequencyResampleNew)
