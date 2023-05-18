/**
 * \date 2021
 * \copyright Allied Vision Technologies.  All Rights Reserved.
 *
 * \copyright Redistribution of this file, in original or modified form, without
 *            prior written consent of Allied Vision Technologies is prohibited.
 *
 * \warning THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF TITLE,
 * NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR  PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * \brief Implementation of ::VmbC::Examples::Image
 */

#include <cstdlib>

#include <VmbImageTransform/VmbTransform.h>

#include "Image.h"
#include "VmbException.h"

namespace VmbC
{
    namespace Examples
    {

        Image::Image(VmbPixelFormat_t pixelFormat) noexcept
            : m_pixelFormat(pixelFormat)
        /* 用于创建一个空的图像对象。它接受一个像素格式作为参数，并初始化图像对象的相关属性。 
        这段代码实现了一个用于处理图像数据的 Image 类。
        它提供了创建空图像、根据帧对象创建图像、释放图像内存和图像转换等功能。
        这些功能对于图像处理和相机应用程序非常有用*/
        {
            m_image.Size = sizeof(m_image);
            m_image.Data = nullptr;
        }

        Image::Image(VmbFrame_t const& frame)
            : m_dataOwned(false),
            m_pixelFormat(frame.pixelFormat)
        /* 根据给定的帧对象创建一个图像对象。
        它接受一个帧对象作为参数，并根据帧的像素格式、宽度和高度设置图像对象的属性。 */
        {
            m_image.Size = sizeof(m_image);
            m_image.Data = frame.imageData;
            
            auto error = VmbSetImageInfoFromPixelFormat(frame.pixelFormat, frame.width, frame.height, &m_image);
            if (error != VmbErrorSuccess)
            {
                throw VmbException::ForOperation(error, "VmbSetImageInfoFromPixelFormat");
            }
        }

        Image::~Image()
        /* 释放图像对象所占用的内存。
        如果图像对象拥有自己的数据（即 m_dataOwned 为 true），则释放图像数据的内存。 */
        {
            if (m_dataOwned)
            {
                std::free(m_image.Data);
            }
        }

        void Image::Convert(Image const& conversionSource)
        /* 用于将当前图像对象转换为另一个图像对象。
        它接受另一个图像对象作为参数，并将当前图像对象转换为与参数图像对象相同的像素格式和尺寸。
        转换过程使用 Vimba API 提供的函数进行图像转换和重新分配内存。 */
        {
            if (&conversionSource == this)
            {
                return;
            }
            auto error = VmbSetImageInfoFromPixelFormat(m_pixelFormat, conversionSource.GetWidth(), conversionSource.GetHeight(), &m_image);
            if (error != VmbErrorSuccess)
            {
                throw VmbException::ForOperation(error, "VmbSetImageInfoFromPixelFormat");
            }

            size_t requiredCapacity = GetBytesPerLine() * GetHeight();
            if (requiredCapacity > m_capacity)
            {
                void* newData;
                if (m_image.Data == nullptr)
                {
                    newData = std::malloc(requiredCapacity);
                }
                else
                {
                    newData = std::realloc(m_image.Data, requiredCapacity);
                }

                if (newData == nullptr)
                {
                    throw std::bad_alloc();
                }

                m_image.Data = newData;
                m_capacity = requiredCapacity;
            }

            error = VmbImageTransform(&conversionSource.m_image, &m_image, nullptr, 0);
            if (error != VmbErrorSuccess)
            {
                throw VmbException::ForOperation(error, "VmbImageTransform");
            }
        }
    }
}