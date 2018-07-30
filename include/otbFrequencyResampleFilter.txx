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

#ifndef OTB_FREQUENCY_RESAMPLE_FILTER_TXX_
#define OTB_FREQUENCY_RESAMPLE_FILTER_TXX_

#include "otbFrequencyResampleFilter.h"

#include <cmath>

#include "itkImageLinearConstIteratorWithIndex.h"
#include "itkImageRegionConstIteratorWithIndex.h"
#include "itkImageRegionIterator.h"

namespace otb {

template <class TInputImage, class TOutputImage>
void FrequencyResampleFilter<TInputImage, TOutputImage>::
      GenerateInputRequestedRegion() throw(itk::InvalidRequestedRegionError) {
    // call the superclass' implementation of this method
    Superclass::GenerateInputRequestedRegion();

    // get pointers to the input and output
    auto input_ptr = const_cast<InputImageType*>(this->GetInput());
    auto output_ptr = this->GetOutput();

    auto output_idx = output_ptr->GetRequestedRegion().GetIndex();
    auto output_size = output_ptr->GetRequestedRegion().GetSize();

    // Set input index and size with padding
    sirius::Padding remaining_padding;
    auto input_reg = GetInputRegion(output_idx, output_size, remaining_padding);

    // crop the region if it is partly out of the source image
    bool crop_success = input_reg.Crop(input_ptr->GetLargestPossibleRegion());
    assert(crop_success);
    if (!crop_success) {
        // store what we tried to request (prior to trying to crop)
        input_ptr->SetRequestedRegion(input_reg);

        // throw an exception
        itk::InvalidRequestedRegionError e(__FILE__, __LINE__);
        e.SetLocation(ITK_LOCATION);
        e.SetDescription(
              "Requested region is outside the largest possible region.");
        e.SetDataObject(input_ptr);
        throw e;
    }

    input_ptr->SetRequestedRegion(input_reg);
}

template <class TInputImage, class TOutputImage>
void FrequencyResampleFilter<TInputImage,
                             TOutputImage>::GenerateOutputInformation() {
    Superclass::GenerateOutputInformation();

    auto input_ptr = this->GetInput();
    auto output_ptr = this->GetOutput();

    double ratio = static_cast<double>(zoom_ratio_.input_resolution()) /
                   zoom_ratio_.output_resolution();

    auto input_largest_size = input_ptr->GetLargestPossibleRegion().GetSize();
    SizeType output_largest_size = {
          static_cast<typename SizeType::SizeValueType>(
                std::ceil(input_largest_size[0] * ratio)),
          static_cast<typename SizeType::SizeValueType>(
                std::ceil(input_largest_size[1] * ratio))};

    OutputImageRegionType output_largest_reg;
    output_largest_reg.SetSize(output_largest_size);
    output_largest_reg.SetIndex(
          input_ptr->GetLargestPossibleRegion().GetIndex());

    output_ptr->SetLargestPossibleRegion(output_largest_reg);
    // TODO ME
    // check spacing
    output_ptr->SetSignedSpacing(
          input_ptr->GetSignedSpacing() * zoom_ratio_.output_resolution() /
          static_cast<double>(zoom_ratio_.input_resolution()));
}

template <class TInputImage, class TOutputImage>
void FrequencyResampleFilter<TInputImage, TOutputImage>::
      EnlargeOutputRequestedRegion(itk::DataObject* obj) {
    auto output_reg = this->GetOutput()->GetRequestedRegion();
    ResizeOutputRegion(output_reg);

    static_cast<OutputImageType*>(obj)->SetRequestedRegion(output_reg);
}

template <class TInputImage, class TOutputImage>
void FrequencyResampleFilter<TInputImage, TOutputImage>::ThreadedGenerateData(
      const OutputImageRegionType& outputRegionForThread, itk::ThreadIdType) {
    sirius::Padding remaining_zero_padding;

    // copy region and update its size to comply with sirius
    auto output_region = outputRegionForThread;
    ResizeOutputRegion(output_region);

    auto input_region =
          GetInputRegion(output_region.GetIndex(), output_region.GetSize(),
                         remaining_zero_padding);

    auto input_block = GenerateImageFromRegion(input_region);
    auto zoomed_block = frequency_zoom_->Compute(
          zoom_ratio_, input_block, remaining_zero_padding, filter_);

    itk::ImageRegionIterator<OutputImageType> oit(this->GetOutput(),
                                                  output_region);
    auto zoomed_block_it = zoomed_block.data.cbegin();
    auto zoomed_block_end_it = zoomed_block.data.cend();
    for (oit.GoToBegin();
         !oit.IsAtEnd() && zoomed_block_it != zoomed_block_end_it;
         ++oit, ++zoomed_block_it) {
        oit.Set(static_cast<OutputPixelType>(*zoomed_block_it));
    }
}

template <class TInputImage, class TOutputImage>
typename TInputImage::RegionType
FrequencyResampleFilter<TInputImage, TOutputImage>::GetInputRegion(
      const IndexType& idx, const SizeType& size,
      sirius::Padding& remaining_padding) {
    auto res_in = zoom_ratio_.input_resolution();
    auto res_out = zoom_ratio_.output_resolution();
    auto filter_padding_size = filter_.padding_size();

    IndexType in_idx = {
          {static_cast<typename IndexType::IndexValueType>(
                 ((idx[0] * res_out) / static_cast<float>(res_in)) -
                 filter_padding_size.col),
           static_cast<typename IndexType::IndexValueType>(
                 ((idx[1] * res_out) / static_cast<float>(res_in)) -
                 filter_padding_size.row)}};

    // add this truncature to the input region size so we have the minimal
    // region needed to compute the requested output
    SizeType in_size = {
          static_cast<typename SizeType::SizeValueType>(
                ((size[0] * res_out) / static_cast<float>(res_in)) +
                2 * filter_padding_size.col),
          static_cast<typename SizeType::SizeValueType>(
                ((size[1] * res_out) / static_cast<float>(res_in)) +
                2 * filter_padding_size.row)};

    InputImageRegionType input_region;
    input_region.SetIndex(in_idx);
    input_region.SetSize(in_size);

    auto largest_region = this->GetInput()->GetLargestPossibleRegion();
    remaining_padding = GetRemainingPadding(input_region, largest_region);

    // crop the region if it is partly out of the source image
    bool crop_success = input_region.Crop(largest_region);
    assert(crop_success);
    if (!crop_success) {
        // throw an exception
        itk::InvalidRequestedRegionError e(__FILE__, __LINE__);
        e.SetLocation(ITK_LOCATION);
        e.SetDescription(
              "Requested region is outside the largest possible region.");
        e.SetDataObject(const_cast<TInputImage*>(this->GetInput()));
        throw e;
    }

    return input_region;
}

template <class TInputImage, class TOutputImage>
sirius::Image
FrequencyResampleFilter<TInputImage, TOutputImage>::GenerateImageFromRegion(
      const InputImageRegionType& region) {
    auto region_size = region.GetSize();
    sirius::Image input_image(
          {static_cast<int>(region_size[1]), static_cast<int>(region_size[0])});

    itk::ImageRegionConstIteratorWithIndex<TInputImage> it(this->GetInput(),
                                                           region);
    // copy pixels that could be read
    int image_index = 0;
    for (it.GoToBegin(); !it.IsAtEnd(); ++it) {
        input_image.data[image_index] = static_cast<double>(it.Get());
        ++image_index;
    }
    return input_image;
}

template <class TInputImage, class TOutputImage>
sirius::Padding
FrequencyResampleFilter<TInputImage, TOutputImage>::GetRemainingPadding(
      const InputImageRegionType& input_region,
      const InputImageRegionType& largest_input_region) {
    sirius::Padding remaining_padding;
    remaining_padding.type = padding_type_;

    auto idx = input_region.GetIndex();
    auto size = input_region.GetSize();
    auto filter_padding = filter_.padding_size();

    remaining_padding.left = (idx[0] < 0) ? -idx[0] : 0;
    remaining_padding.top = (idx[1] < 0) ? -idx[1] : 0;

    auto largest_region_size = largest_input_region.GetSize();

    if (idx[0] + size[0] + filter_padding.col > largest_region_size[0]) {
        remaining_padding.right =
              (idx[0] + size[0] + filter_padding.col) - largest_region_size[0];
    } else {
        remaining_padding.right = 0;
    }

    if (idx[1] + size[1] + filter_padding.row > largest_region_size[1]) {
        remaining_padding.bottom =
              (idx[1] + size[1] + filter_padding.row) - largest_region_size[1];
    } else {
        remaining_padding.bottom = 0;
    }

    return remaining_padding;
}

template <class TInputImage, class TOutputImage>
void FrequencyResampleFilter<TInputImage, TOutputImage>::ResizeOutputRegion(
      OutputImageRegionType& output_region) {
    auto res_in = zoom_ratio_.input_resolution();
    auto res_out = zoom_ratio_.output_resolution();

    auto output_ptr = this->GetOutput();
    OutputImageRegionType new_region;
    IndexType new_index = output_region.GetIndex();
    SizeType new_size = output_region.GetSize();

    auto largest_output_region = output_ptr->GetLargestPossibleRegion();
    auto largest_output_region_size = largest_output_region.GetSize();

    if (output_region != largest_output_region) {
        while (floor((new_index[1] * res_out) / static_cast<double>(res_in)) !=
               ceil((new_index[1] * res_out) / static_cast<double>(res_in))) {
            new_index[1]--;
            new_size[1]++;
        }

        while (floor((new_index[0] * res_out) / static_cast<double>(res_in)) !=
               ceil((new_index[0] * res_out) / static_cast<double>(res_in))) {
            new_index[0]--;
            new_size[0]++;
        }

        if (new_size[0] <= largest_output_region_size[0]) {
            while (
                  floor((new_size[0] * res_out) /
                        static_cast<double>(res_in)) !=
                  ceil((new_size[0] * res_out) / static_cast<double>(res_in))) {
                if (new_index[0] + new_size[0] + 1 <=
                    largest_output_region_size[0]) {
                    new_size[0]++;
                } else {
                    break;
                }
            }
        }

        if (new_size[1] <= largest_output_region_size[1]) {
            while (
                  floor((new_size[1] * res_out) /
                        static_cast<double>(res_in)) !=
                  ceil((new_size[1] * res_out) / static_cast<double>(res_in))) {
                if (new_index[1] + new_size[1] + 1 <=
                    largest_output_region_size[1]) {
                    new_size[1]++;
                } else {
                    break;
                }
            }
        }
    }
    output_region.SetSize(new_size);
    output_region.SetIndex(new_index);
}

template <class TInputImage, class TOutputImage>
void FrequencyResampleFilter<TInputImage, TOutputImage>::Init(
      const sirius::ZoomRatio& zoom_ratio, const std::string& filter_path,
      sirius::PaddingType padding_type,
      sirius::ImageDecompositionPolicies image_decomposition,
      sirius::FrequencyZoomStrategies zoom_strategy) {
    zoom_ratio_ = zoom_ratio;
    if (!filter_path.empty()) {
        padding_type_ = padding_type;
        filter_ =
              sirius::Filter::Create(filter_path, zoom_ratio_, padding_type_);
    }
    frequency_zoom_ = sirius::FrequencyZoomFactory::Create(image_decomposition,
                                                           zoom_strategy);
}

}  // namespace otb

#endif  // OTB_FREQUENCY_RESAMPLE_FILTER_TXX_
