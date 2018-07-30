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

set(DOCUMENTATION "Frequency Resample application (see also SIRIUS)")

otb_module(FrequencyResample
  DEPENDS
    OTBCommon
    OTBITK
    OTBImageBase
    OTBApplicationEngine

  TEST_DEPENDS
    OTBImageIO
    OTBTestKernel
    OTBCommandLine

  DESCRIPTION
    "${DOCUMENTATION}"
)
