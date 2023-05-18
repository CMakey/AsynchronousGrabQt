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
 * \brief Implementation of ::VmbC::Examples::ImageTranscoder
 */

#include <algorithm>
#include <limits>
#include <thread>
#include <type_traits>

#include "AcquisitionManager.h"
#include "Image.h"
#include "ImageTranscoder.h"
#include "VmbException.h"

#include <QImage>
#include <QPixmap>

#include <VmbC/VmbC.h>

namespace VmbC
{
    namespace Examples
    {
        ImageTranscoder::ImageTranscoder(AcquisitionManager& manager)
            : m_acquisitionManager(manager)
        /* 用于图像转码和处理。
        接受一个 AcquisitionManager 对象作为参数，并将其保存为成员变量。
        图像转码器，它接受图像帧并对其进行转码处理。
        转码器使用任务队列和线程来实现异步处理。
        转码器的启动和停止操作确保了正确的转码过程，并允许在需要时中止转码任务。
         */
        {
        }

        void ImageTranscoder::PostImage(VmbHandle_t const streamHandle, VmbFrameCallback callback, VmbFrame_t const* frame)
        /* 用于将图像提交给转码器进行处理。
        它接受流句柄、帧回调函数和帧对象作为参数。根据帧的接收状态和标志，决定是否对帧进行转码处理。
        如果需要转码处理，则创建一个 TransformationTask 对象来保存有关转码任务的信息，并将其放入任务队列中等待处理。
        如果不需要转码处理，则尝试重新将帧放回帧队列中。 */
        {
            bool notify = false;

            if (frame != nullptr)
            {
                if (frame->receiveStatus == VmbFrameStatusComplete
                    && (frame->receiveFlags & VmbFrameFlagsDimension) == VmbFrameFlagsDimension)
                {
                    auto message = std::unique_ptr<TransformationTask>(new TransformationTask(streamHandle, callback, *frame));

                    {
                        std::lock_guard<std::mutex> lock(m_inputMutex);
                        if (m_terminated)
                        {
                            message->m_canceled = true;
                        }
                        else
                        {
                            m_task = std::move(message);
                            notify = true;
                        }
                    }
                }
                else
                {
                    // try to renequeue the frame we won't pass to the image transformation
                    VmbCaptureFrameQueue(streamHandle, frame, callback);
                }
            }

            if (notify)
            {
                m_inputCondition.notify_one();
            }
        }

        void ImageTranscoder::Start()
        /* 用于启动转码器。
        它检查转码器是否已经在运行，如果是，则抛出异常。
        否则，将转码器标记为未终止状态，并启动一个新的线程来执行转码任务。 */
        {
            {
                std::lock_guard<std::mutex> lock(m_inputMutex);
                if (!m_terminated)
                {
                    throw VmbException("ImageTranscoder is still running");
                }
                m_terminated = false;
            }
            m_thread = std::thread(&ImageTranscoder::TranscodeLoop, std::ref(*this));
        }

        void ImageTranscoder::Stop() noexcept
        /* 用于停止转码器。
        它将转码器标记为已终止状态，并通知所有等待中的任务完成。
        然后，等待转码线程结束。
        */
        {
            {
                std::lock_guard<std::mutex> lock(m_inputMutex);
                if (m_terminated)
                {
                    return;
                }
                m_terminated = true;
                if (m_task)
                {
                    m_task->m_canceled;
                    m_task.reset();
                }
            }
            m_inputCondition.notify_all();
            m_thread.join();
        }

        void ImageTranscoder::SetOutputSize(QSize size)
        /* 用于设置输出图像的大小。
        它使用互斥锁保护共享的输出大小变量，将新的大小值存储在 m_outputSize 成员变量中。 
        */
        {
            std::lock_guard<std::mutex> lock(m_sizeMutex);
            m_outputSize = size;
        }

        ImageTranscoder::~ImageTranscoder()
        /* 
        如果转码器尚未终止，则调用 Stop() 方法停止转码器。
         */
        {
            // tell the thread about the shutdown
            if (!m_terminated)
            {
                Stop();
            }
        }

        void ImageTranscoder::TranscodeLoopMember()
        /* 是转码器的主要转码循环。它在一个循环中等待转码任务或终止信号。
        当接收到转码任务时，将任务从任务队列中取出并执行转码操作。
        转码操作通过调用 TranscodeImage() 函数实现。
        如果转码过程中捕获到 VmbException 或 std::bad_alloc 异常，暂时忽略它们。
        如果在转码过程中接收到终止信号，取消当前任务并终止转码循环。
         */
        {
            std::unique_lock<std::mutex> lock(m_inputMutex);

            while (true)
            {
                if (!m_terminated && !m_task)
                /* 
                使用互斥锁来保护共享变量，并在循环中等待转码任务或终止信号。
                如果接收到终止信号，则立即返回。
                如果有可用的转码任务，则取出任务并执行转码操作。
                转码操作完成后，再次获取互斥锁，并检查是否接收到终止信号。
                如果在转码过程中接收到终止信号，则将当前任务标记为已取消，并立即返回。
                 */
                {
                    m_inputCondition.wait(lock, [this]() { return m_terminated || m_task; }); // wait for frame/termination
                }

                if (m_terminated)
                {
                    return;
                }

                {
                    // get task
                    decltype(m_task) task;
                    std::swap(task, m_task);

                    lock.unlock();

                    if (task)
                    {
                        try
                        {
                            TranscodeImage(*task);
                        }
                        catch (VmbException const&)
                        {
                            // todo?
                        }
                        catch (std::bad_alloc&)
                        {
                            // todo?
                        }
                    }

                    lock.lock();

                    if (m_terminated)
                    {
                        // got terminated during conversion -> don't reenqueue frames
                        task->m_canceled = true;
                        return;
                    }
                }
            }

        }

        namespace
        {
            bool IsLittleEndian()
            /* 用于判断当前系统是否为小端字节序。
            它通过创建一个整数 one，并使用指针将其转换为字节序列。
            然后检查字节序列的第一个字节是否为1，来确定系统的字节序。 */
            {
                uint32_t const one = 1;
                auto oneBytes = reinterpret_cast<unsigned char const*>(&one);
                return oneBytes[0] == 1;
            }

            /**
             * \brief helper class for determining the image formats to use in the conversion 
             */
            class ImageFormats
            /* 用于确定在图像转换中使用的图像格式。 */
            {
            public:

                ImageFormats()
                /* 通过调用 IsLittleEndian() 函数来确定当前系统的字节序，并根据字节序设置适当的图像格式。 */
                    : ImageFormats(IsLittleEndian())
                {
                }

                QImage::Format const QtImageFormat;//用于存储用于转换的Qt图像格式（QImage::Format）
                VmbPixelFormat_t const VmbTransformFormat;//用于存储用于转换的Vimba图像格式（VmbPixelFormat_t）

            private:
                ImageFormats(bool littleEndian)
                    : QtImageFormat(littleEndian ? QImage::Format_RGB32 : QImage::Format_RGBX8888),
                    VmbTransformFormat(littleEndian ? VmbPixelFormatBgra8 : VmbPixelFormatRgba8)
                {
                }
            };

            static const ImageFormats ConversionFormats{};//用于保存转换所需的图像格式。它在编译时初始化，并使用默认构造函数 ImageFormats()
        }

        void ImageTranscoder::TranscodeImage(TransformationTask& task)
        //用于执行图像转码的操作
        {
            VmbFrame_t const& frame = task.m_frame;//获取任务中的帧信息 

            Image const source(task.m_frame);//使用帧信息创建一个 Image 对象 source，作为转码的源图像。

            // allocate new image, if necessary
            if (!m_transformTarget)
            {
                m_transformTarget.reset(new Image(ConversionFormats.VmbTransformFormat));
                /* 没有目标图像对象，就分配一个新的 Image 对象，并使用转换格式 ConversionFormats.VmbTransformFormat 进行初始化。 */
            }

            m_transformTarget->Convert(source);//将源图像转换为目标图像。

            /* 使用目标图像的数据、宽度、高度、每行字节数和Qt图像格式，创建一个 QImage 对象 qImage。 */
            QImage qImage(m_transformTarget->GetData(),
                          m_transformTarget->GetWidth(),
                          m_transformTarget->GetHeight(),
                          m_transformTarget->GetBytesPerLine(),
                          ConversionFormats.QtImageFormat);

            /* 使用 QPixmap::fromImage() 将 qImage 转换为 QPixmap 对象 pixmap，使用 Qt::ImageConversionFlag::ColorOnly 进行颜色转换。 */
            QPixmap pixmap = QPixmap::fromImage(qImage, Qt::ImageConversionFlag::ColorOnly);

            QSize size;
           /* 获取输出大小 size
           通过加锁访问 m_sizeMutex。 */
            {
                std::lock_guard<std::mutex> lock(m_sizeMutex);
                size = m_outputSize;
            }
            /* 将经过缩放后的 pixmap 作为参数传递给该函数，使用 Qt::AspectRatioMode::KeepAspectRatio 保持宽高比。 */
            m_acquisitionManager.ConvertedFrameReceived(pixmap.scaled(size, Qt::AspectRatioMode::KeepAspectRatio));
        }

        void ImageTranscoder::TranscodeLoop(ImageTranscoder& transcoder)
        /* 将给定的 ImageTranscoder 对象传递给 TranscodeLoopMember() 函数进行执行。 */
        {
            transcoder.TranscodeLoopMember();
        }

        ImageTranscoder::TransformationTask::TransformationTask(VmbHandle_t const streamHandle, VmbFrameCallback callback, VmbFrame_t const& frame)
            : m_streamHandle(streamHandle),
            m_callback(callback),
            m_frame(frame)
        /* 用于创建一个图像转码任务对象。
        它接收一个流处理句柄 streamHandle、回调函数 callback 和帧信息 frame 作为参数。
        在构造函数中，它将这些参数分别赋值给对应的成员变量 m_streamHandle、m_callback 和 m_frame */
        {
        }

        ImageTranscoder::TransformationTask::~TransformationTask()
        /* 
        负责清理图像转码任务对象。如果任务没有被取消（m_canceled 为 false），则调用 VmbCaptureFrameQueue 函数将帧重新排队到流处理中，以便后续的处理。
         */
        {
            if (!m_canceled)
            {
                VmbCaptureFrameQueue(m_streamHandle, &m_frame, m_callback);
            }
        }
    }
}
