/**
 * Copyright (C) 2018 CS - Systemes d'Information (CS-SI)
 *
 * This file is part of OTB-SIRIUS
 *
 *     https://github.com/CS-SI/OTB-SIRIUS
 *
 * OTB-SIRIUS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OTB-SIRIUS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OTB-SIRIUS.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <sstream>

#include <sirius/frequency_zoom_factory.h>
#include <sirius/sirius.h>
#include <sirius/types.h>
#include <sirius/utils/log.h>

#include "otbWrapperApplication.h"
#include "otbWrapperApplicationFactory.h"

#include "otbFrequencyResampleFilter.h"

namespace otb {
namespace Wrapper {

class FrequencyResample : public Application {
  private:
    typedef otb::FrequencyResampleFilter<DoubleImageType> FilterType;

  public:
    typedef FrequencyResample Self;
    typedef Application Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    itkNewMacro(Self);
    itkTypeMacro(FrequencyResample, otb::Application);

  private:
    void DoInit() override {
        std::stringstream description;
        description
              << "This application is a wrapper of the Sirius library ("
              << sirius::kVersion << " - " << sirius::kGitCommit << ").\n"
              << "This library performs resampling in the frequency domain.";

        SetName("FrequencyResample");

        SetDescription("Resample images in the frequency domain");
        SetDocLongDescription(description.str());
        SetDocName("FrequencyResample");
        SetDocLimitations(
              "* resolution.input or resolution.output must be positive\n"
              "* memory usage during processing is directly proportional to "
              "input resolution and image block size");
        SetDocAuthors("Sirius developers");
        SetDocSeeAlso(
              "https://github.com/CS-SI/SIRIUS\n"
              "https://github.com/CS-SI/OTB-SIRIUS");
        AddDocTag("Zoom");
        AddDocTag("Resample");
        AddDocTag(Tags::Geometry);

        AddParameter(ParameterType_InputImage, "in", "Input Image");
        AddParameter(ParameterType_OutputImage, "out", "Output Image");

        AddParameter(ParameterType_String, "v",
                     "Verbosity: trace,debug,info,warn,err,critical,off");
        MandatoryOff("v");
        SetParameterString("v", "info");

        // resolution
        AddParameter(ParameterType_Group, "resolution",
                     "Resolution parameters");
        AddParameter(ParameterType_Int, "resolution.input",
                     "numerator of the zoom ratio");
        SetDefaultParameterInt("resolution.input", 1);
        SetMinimumParameterIntValue("resolution.input", 1);

        AddParameter(ParameterType_Int, "resolution.output",
                     "denominator of the zoom ratio");
        SetDefaultParameterInt("resolution.output", 1);
        SetMinimumParameterIntValue("resolution.output", 1);

        // filter
        AddParameter(ParameterType_Group, "filter", "filter parameters");
        AddParameter(ParameterType_String, "filter.path",
                     "Path to the filter image to apply to the zoomed image");
        MandatoryOff("filter.path");
        AddParameter(ParameterType_Empty, "filter.zeropadding",
                     "Use zero padding strategy to add filter margins to the "
                     "input data (default is mirror padding)");
        MandatoryOff("filter.zeropadding");

        // zoom
        AddParameter(ParameterType_Group, "zoom", "zoom parameters");
        AddParameter(ParameterType_Empty, "zoom.periodicsmooth",
                     "Use Periodic plus Smooth image decomposition (default is "
                     "regular image decomposition)");
        MandatoryOff("zoom.periodicsmooth");

        AddParameter(ParameterType_Empty, "zoom.zeropadding",
                     "Use zero padding zoom algorithm (default is "
                     "periodization zoom algorithm)");
        MandatoryOff("zoom.zeropadding");

        SetDocExampleParameterValue("in", "lena.jpg");
        SetDocExampleParameterValue("out", "lena_z2.jpg");
        SetDocExampleParameterValue("resolution.input", "2");
        SetDocExampleParameterValue("resolution.output", "1");
    }

    void DoUpdateParameters() override {}

    void DoExecute() override {
        filter_ = FilterType::New();

        // zoom
        int input_resolution = GetParameterInt("resolution.input");
        int output_resolution = GetParameterInt("resolution.output");
        bool periodic_smooth = GetParameterEmpty("zoom.periodicsmooth");
        bool zero_padding_zoom = GetParameterEmpty("zoom.zeropadding");

        // frequency filter
        std::string filter_path = GetParameterAsString("filter.path");
        bool filter_zero_padding = GetParameterEmpty("filter.zeropadding");

        std::string verbosity = GetParameterAsString("v");
        sirius::utils::SetVerbosityLevel(verbosity);

        sirius::ZoomRatio zoom_ratio(input_resolution, output_resolution);

        sirius::PaddingType filter_padding_type =
              sirius::PaddingType::kMirrorPadding;
        if (filter_zero_padding) {
            filter_padding_type = sirius::PaddingType::kZeroPadding;
        }

        sirius::ImageDecompositionPolicies image_decomposition =
              sirius::ImageDecompositionPolicies::kRegular;
        sirius::FrequencyZoomStrategies zoom_strategy =
              sirius::FrequencyZoomStrategies::kPeriodization;

        if (zero_padding_zoom) {
            zoom_strategy = sirius::FrequencyZoomStrategies::kZeroPadding;
        }
        if (periodic_smooth) {
            image_decomposition =
                  sirius::ImageDecompositionPolicies::kPeriodicSmooth;
        }

        filter_->Init(zoom_ratio, filter_path, filter_padding_type,
                      image_decomposition, zoom_strategy);

        filter_->SetInput(GetParameterDoubleImage("in"));
        SetParameterOutputImage("out", filter_->GetOutput());
    }

  private:
    FilterType::Pointer filter_;
};

}  // namespace Wrapper
}  // namespace otb

OTB_APPLICATION_EXPORT(otb::Wrapper::FrequencyResample)
