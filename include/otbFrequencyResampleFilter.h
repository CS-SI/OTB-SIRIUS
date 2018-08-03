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

#ifndef OTB_FREQUENCY_RESAMPLE_FILTER_H_
#define OTB_FREQUENCY_RESAMPLE_FILTER_H_

#include <sirius/filter.h>
#include <sirius/frequency_resampler_factory.h>
#include <sirius/image.h>
#include <sirius/types.h>

#include "itkImageToImageFilter.h"

#include "otbImage.h"

namespace otb {

/**
 * \brief Wrapper of Sirius IFrequencyZoom API
 */
template <class TInputImage, class TOutputImage = TInputImage>
class FrequencyResampleFilter
      : public itk::ImageToImageFilter<TInputImage, TOutputImage> {
  public:
    /** standard class typedefs */
    typedef TInputImage InputImageType;
    typedef TOutputImage OutputImageType;

    typedef FrequencyResampleFilter Self;
    typedef itk::ImageToImageFilter<InputImageType, OutputImageType> Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    typedef typename InputImageType::PixelType InputPixelType;
    typedef typename OutputImageType::PixelType OutputPixelType;
    typedef typename InputImageType::RegionType InputImageRegionType;
    typedef typename OutputImageType::RegionType OutputImageRegionType;
    typedef typename InputImageType::IndexType IndexType;
    typedef typename InputImageType::SizeType SizeType;

    void EnlargeOutputRequestedRegion(itk::DataObject* obj) override;

    /** Object factory management */
    itkNewMacro(Self);

    itkTypeMacro(FrequencyResampleFilter, ImageToImageFilter);

    /**
     * \brief Init the ImageToImageFilter with Sirius parameters
     * \param zoom_ratio zoom ratio
     * \param filter filter
     * \param image_decomposition requested image decomposition
     * \param zoom_strategy requested zoom strategy
     */
    void Init(const sirius::ZoomRatio& zoom_ratio, sirius::Filter&& filter,
              sirius::ImageDecompositionPolicies image_decomposition,
              sirius::FrequencyZoomStrategies zoom_strategy);

  protected:
    FrequencyResampleFilter() = default;
    ~FrequencyResampleFilter() override = default;

    void GenerateInputRequestedRegion() throw(
          itk::InvalidRequestedRegionError) override;
    void GenerateOutputInformation() override;
    void ThreadedGenerateData(
          const OutputImageRegionType& outputRegionForThread,
          itk::ThreadIdType threadId) override;

  private:
    FrequencyResampleFilter(const Self&) = delete;
    FrequencyResampleFilter& operator=(const Self&) = delete;

    InputImageRegionType GetInputRegion(const IndexType& idx,
                                        const SizeType& size,
                                        sirius::Padding& remaining_padding);

    void ResizeOutputRegion(OutputImageRegionType& output_region);

    /**
     * \brief Compute the padding needed for input_region to have the size
     *        of largest_input_region
     * \param input_region
     * \param largest_input_region
     */
    sirius::Padding GetRemainingPadding(
          const InputImageRegionType& input_region,
          const InputImageRegionType& largest_input_region);

    /**
     * \brief Generate an image from a region
     * \param region
     */
    sirius::Image GenerateImageFromRegion(const InputImageRegionType& region);

  private:
    sirius::ZoomRatio zoom_ratio_;
    sirius::Filter filter_;
    sirius::PaddingType padding_type_;
    sirius::IFrequencyResampler::UPtr frequency_resampler_;
};

}  // namespace otb

#ifndef OTB_MANUAL_INSTANTIATION
#include "otbFrequencyResampleFilter.txx"
#endif  // OTB_MANUAL_INSTANTIATION

#endif  // OTB_FREQUENCY_RESAMPLE_FILTER_H_
