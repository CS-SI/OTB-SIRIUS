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

#include <sirius/filter.h>
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
              "* memory usage during processing is directly proportional to "
              "input resolution and image block size");
        SetDocAuthors("Sirius developers");
        SetDocSeeAlso(
              "https://github.com/CS-SI/SIRIUS\n"
              "https://github.com/CS-SI/OTB-SIRIUS");
        AddDocTag("Zoom");
        AddDocTag("Resample");
        AddDocTag(Tags::Geometry);

        AddParameter(ParameterType_InputImage, "in", "Input image");
        SetParameterDescription("in", "Path to the input image");
        AddParameter(ParameterType_OutputImage, "out", "Output Image");
        SetParameterDescription("out", "Path to the output image");

        AddParameter(ParameterType_String, "v", "Verbosity");
        SetParameterString("v", "info");
        SetParameterDescription(
              "v", "Verbosity: trace,debug,info,warn,err,critical,off");
        MandatoryOff("v");

        // resampling
        AddParameter(ParameterType_Group, "resampling", "Resampling options");
        SetParameterDescription("resampling", "Resampling options");

        AddParameter(ParameterType_String, "resampling.ratio",
                     "Resampling ratio");
        SetParameterDescription(
              "resampling.ratio",
              "Resampling ratio as input:output, allowed format: I "
              "(equivalent to I:1), I:O");
        SetParameterString("resampling.ratio", "1:1");

        AddParameter(ParameterType_Bool, "resampling.noimagedecomposition",
                     "Do not decompose the input image (default: periodic plus "
                     "smooth image decomposition)");
        SetParameterDescription(
              "resampling.noimagedecomposition",
              "Do not decompose the input image (default: periodic plus "
              "smooth image decomposition)");
        MandatoryOff("resampling.noimagedecomposition");

        AddParameter(ParameterType_Group, "resampling.upsample",
                     "Upsampling options");
        AddParameter(ParameterType_Bool, "resampling.upsample.periodization",
                     "Force periodization as upsampling algorithm");
        SetParameterDescription(
              "resampling.upsample.periodization",
              "Force periodization as upsampling algorithm (default "
              "algorithm if a filter is provided). A filter is required "
              "to use this algorithm");
        MandatoryOff("resampling.upsample.periodization");

        AddParameter(ParameterType_Bool, "resampling.upsample.zeropadding",
                     "Force zero padding as upsampling algorithm ");
        SetParameterDescription(
              "resampling.upsample.zeropadding",
              "Force zero padding as upsampling algorithm (default "
              "algorithm if no filter is provided)");
        MandatoryOff("resampling.upsample.zeropadding");

        // filter
        AddParameter(ParameterType_Group, "filter", "Filter options");
        SetParameterDescription("filter", "Filter options");
        AddParameter(ParameterType_InputImage, "filter.path",
                     "Path to the filter image to apply to the zoomed image");
        MandatoryOff("filter.path");

        AddParameter(ParameterType_Bool, "filter.normalize",
                     "Normalize filter coefficients");
        SetParameterDescription("filter.normalize",
                                "Normalize filter coefficients "
                                "(default: no normalization)");
        MandatoryOff("filter.normalize");

        AddParameter(ParameterType_Bool, "filter.zeropadrealedges",
                     "Force zero padding strategy on real input edges");
        SetParameterDescription(
              "filter.zeropadrealedges",
              "Force zero padding strategy on real input edges "
              "(default: mirror padding)");
        MandatoryOff("filter.zeropadrealedges");

        AddParameter(ParameterType_Group, "filter.hotpoint",
                     "Hot point filter options");
        SetParameterDescription("filter.hotpoint", "Hot point filter options");
        AddParameter(ParameterType_Int, "filter.hotpoint.x",
                     "Hot point x coordinate");
        SetParameterDescription("filter.hotpoint.x", "Hot point x coordinate");
        SetDefaultParameterInt("filter.hotpoint.x",
                               sirius::filter_default_hot_point.x);
        MandatoryOff("filter.hotpoint.x");

        AddParameter(ParameterType_Int, "filter.hotpoint.y",
                     "Hot point y coordinate");
        SetParameterDescription("filter.hotpoint.y", "Hot point y coordinate");
        SetDefaultParameterInt("filter.hotpoint.y",
                               sirius::filter_default_hot_point.y);
        MandatoryOff("filter.hotpoint.y");

        // upsampling
        SetDocExampleParameterValue("in", "lena.jpg");
        SetDocExampleParameterValue("out", "lena_z2.jpg");
        SetDocExampleParameterValue("resampling.ratio", "2:1");
    }

    void DoUpdateParameters() override {}

    void DoExecute() override {
        filter_ = FilterType::New();

        // sirius verbosity
        std::string verbosity = GetParameterAsString("v");
        sirius::utils::SetVerbosityLevel(verbosity);

        // resampling
        auto resampling_ratio = GetParameterAsString("resampling.ratio");
        auto zoom_ratio = sirius::ZoomRatio::Create(resampling_ratio);

        auto no_image_decomposition =
              IsParameterEnabled("resampling.noimagedecomposition");
        auto force_upsample_periodization =
              IsParameterEnabled("resampling.upsample.periodization");
        auto force_upsample_zero_padding =
              IsParameterEnabled("resampling.upsample.zeropadding");

        // frequency filter
        auto filter_normalize = IsParameterEnabled("filter.normalize");
        auto zero_pad_real_edges =
              IsParameterEnabled("filter.zeropadrealedges");

        sirius::Point hotpoint = sirius::filter_default_hot_point;
        hotpoint.x = GetParameterInt("filter.hotpoint.x");
        hotpoint.y = GetParameterInt("filter.hotpoint.y");

        sirius::PaddingType padding_type = sirius::PaddingType::kMirrorPadding;
        if (zero_pad_real_edges) {
            padding_type = sirius::PaddingType::kZeroPadding;
        }

        sirius::Filter frequency_filter;
        if (HasValue("filter.path")) {
            auto otb_filter_image = GetParameterDoubleImage("filter.path");
            auto filter_image = CreateFilterImage(otb_filter_image);
            sirius::Point hp(hotpoint.x, hotpoint.y);

            frequency_filter =
                  sirius::Filter::Create(std::move(filter_image), zoom_ratio,
                                         hp, padding_type, filter_normalize);
        }

        // resampling parameters
        sirius::ImageDecompositionPolicies image_decomposition_policy =
              sirius::ImageDecompositionPolicies::kPeriodicSmooth;
        sirius::FrequencyZoomStrategies zoom_strategy =
              sirius::FrequencyZoomStrategies::kPeriodization;

        if (no_image_decomposition) {
            LOG("sirius", info, "image decomposition: none");
            image_decomposition_policy =
                  sirius::ImageDecompositionPolicies::kRegular;
        } else {
            LOG("sirius", info, "image decomposition: periodic plus smooth");
        }

        if (zoom_ratio.ratio() > 1) {
            // choose the upsampling algorithm only if ratio > 1
            if (force_upsample_periodization && !frequency_filter.IsLoaded()) {
                LOG("sirius", error,
                    "filter is required with periodization upsampling");
                return;
            } else if (force_upsample_zero_padding ||
                       !frequency_filter.IsLoaded()) {
                LOG("sirius", info, "upsampling: zero padding");
                zoom_strategy = sirius::FrequencyZoomStrategies::kZeroPadding;
                if (frequency_filter.IsLoaded()) {
                    LOG("sirius", warn,
                        "filter will be used with zero padding upsampling");
                }
            } else {
                LOG("sirius", info, "upsampling: periodization");
                zoom_strategy = sirius::FrequencyZoomStrategies::kPeriodization;
            }
        }

        filter_->Init(zoom_ratio, std::move(frequency_filter),
                      image_decomposition_policy, zoom_strategy);

        filter_->SetInput(GetParameterDoubleImage("in"));
        SetParameterOutputImage("out", filter_->GetOutput());
    }

    sirius::Image CreateFilterImage(DoubleImageType* otb_image) {
        auto region = otb_image->GetLargestPossibleRegion();
        auto region_size = region.GetSize();
        sirius::Image filter_image({static_cast<int>(region_size[1]),
                                    static_cast<int>(region_size[0])});
        otb_image->Update();
        itk::ImageRegionConstIteratorWithIndex<
              std::remove_pointer_t<decltype(otb_image)>>
              it(otb_image, region);
        // copy pixels that could be read
        int image_index = 0;
        for (it.GoToBegin(); !it.IsAtEnd(); ++it) {
            filter_image.data[image_index] = static_cast<double>(it.Get());
            ++image_index;
        }
        return filter_image;
    }

  private:
    FilterType::Pointer filter_;
};

}  // namespace Wrapper
}  // namespace otb

OTB_APPLICATION_EXPORT(otb::Wrapper::FrequencyResample)
