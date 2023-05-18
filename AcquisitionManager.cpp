/**
 * \copyright Allied Vision Technologies 2021 - 2022.  All Rights Reserved.
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
 * \brief Implementation of ::VmbC::Examples::AcquisitionManager
 */

#include <cstdlib>
#include <cstring>
#include <limits>
#include <string>

#include "AcquisitionManager.h"
#include "VmbException.h"

#include "UI/MainWindow.h"

namespace VmbC
{
    namespace Examples
    {

        namespace
        {
            /**
             * \brief feature name of custom command for choosing the packet size provided by AVT GigE cameras
             * 简介：用于选择AVT GigE摄像机提供的数据包大小的自定义命令的功能名称
             * 这段代码是使用匿名命名空间定义的一部分代码。让我逐步解释其中的内容：

                1. namespace { ... };: 这是一个匿名命名空间，用于限定代码的作用域。它可以包含函数、变量、结构等定义，这些定义在命名空间内部是可见的，但在命名空间外部是不可见的。在这个例子中，所有的定义都位于这个匿名命名空间内。
                2. constexpr char const* AdjustPackageSizeCommand = "GVSPAdjustPacketSize";: 这是一个常量表达式，用于定义一个指向字符常量的指针。该常量指针被命名为AdjustPackageSizeCommand，并赋值为字符串"GVSPAdjustPacketSize"。这个常量可能在代码的其他地方使用，用于表示自定义命令的特征名称，该命令用于选择由AVT GigE相机提供的数据包大小。
                3.struct AcquisitionContext { ... };: 这是一个名为AcquisitionContext的结构体的定义。它具有以下成员：
                    3.1 AcquisitionManager* m_acquisitionManager;: 这是一个指向AcquisitionManager对象的指针成员变量。
                    3.2 AcquisitionContext(AcquisitionManager* acquisitionManager) noexcept: 这是一个构造函数，它接受一个指向AcquisitionManager对象的指针，并在构造时初始化m_acquisitionManager成员变量。
                    3.3 AcquisitionContext(VmbFrame_t const& frame) noexcept: 这是另一个构造函数，它接受一个VmbFrame_t类型的常量引用，并从中提取出AcquisitionManager对象指针，并将其存储在m_acquisitionManager成员变量中。
                4. void FillFrame(VmbFrame_t& frame) const noexcept: 这是一个成员函数，用于填充VmbFrame_t类型的帧对象。它将m_acquisitionManager指针存储在帧对象的context数组的第一个元素中。
                这段代码定义了一个命名空间，并在其中定义了一个常量和一个结构体。这些定义可能在项目的其他部分使用，用于实现与图像采集相关的功能。请注意，由于这是一个片段代码，缺少上下文信息，可能需要结合其他代码来理解其完整的含义和功能。
             * 
             */
            constexpr char const* AdjustPackageSizeCommand = "GVSPAdjustPacketSize";

            struct AcquisitionContext
            {
                AcquisitionManager* m_acquisitionManager;

                AcquisitionContext(AcquisitionManager* acquisitionManager) noexcept
                    : m_acquisitionManager(acquisitionManager)
                {
                }

                AcquisitionContext(VmbFrame_t const& frame) noexcept
                    : m_acquisitionManager(static_cast<AcquisitionManager*>(frame.context[0]))
                {
                }

                void FillFrame(VmbFrame_t& frame) const noexcept
                {
                    frame.context[0] = m_acquisitionManager;
                }

            };
        };

        void AcquisitionManager::StartAcquisition(VmbCameraInfo_t const& cameraInfo)
        /* 
        brief：开始图像采集
        这是AcquisitionManager类的成员函数，用于开始图像采集。它执行以下操作：
        1.调用StopAcquisition()函数，如果摄像机已经打开，则先关闭它。
        2.创建一个名为CameraAccessLifetime的对象，并传递cameraInfo和当前的AcquisitionManager对象(*this)给它。然后，通过m_openCamera成员变量持有这个对象。
        3.调用m_imageTranscoder对象的Start()函数，开始图像转码。 
        */
        {
            StopAcquisition(); // if a camera is open, close it first
            m_openCamera.reset(new CameraAccessLifetime(cameraInfo, *this));
            m_imageTranscoder.Start();
        }

        void AcquisitionManager::StopAcquisition() noexcept
        /*
        brief：停止图像采集
        这是AcquisitionManager类的成员函数，用于停止图像采集。它执行以下操作：
        1.调用m_imageTranscoder对象的Stop()函数，停止图像转码。
        2.通过调用m_openCamera的reset()函数，将其重置为空指针。 
         */
        {
            m_imageTranscoder.Stop();
            m_openCamera.reset();
        }

        AcquisitionManager::AcquisitionManager(MainWindow& renderWindow)
        /*
        brief：AcquisitionManager的类构造函数
        这是AcquisitionManager类的构造函数，接受一个MainWindow对象的引用作为参数。它执行以下操作：
        初始化m_renderWindow成员变量为传入的renderWindow对象。
        初始化m_imageTranscoder对象，将当前的AcquisitionManager对象(*this)作为参数传递给它。 
         */
            : m_renderWindow(renderWindow),
            m_imageTranscoder(*this)
        {
        }

        AcquisitionManager::~AcquisitionManager()
        /* 
        brief：析构函数，解除资源占用
        这是AcquisitionManager类的析构函数。它执行以下操作：
        1.调用StopAcquisition()函数，以确保停止图像采集。
        2.通过调用m_openCamera的reset()函数，将其重置为空指针。
         */
        {
            StopAcquisition();
            m_openCamera.reset();
        }

        void AcquisitionManager::ConvertedFrameReceived(QPixmap image)
        /* 
        brief：转换接受到的帧
        这是AcquisitionManager类的成员函数，用于接收转换后的图像帧。它执行以下操作：
        1. 将接收到的image参数传递给m_renderWindow对象的RenderImage()函数，以在窗口中渲染图像。
         */
        {
            m_renderWindow.RenderImage(image);
        }

        void AcquisitionManager::SetOutputSize(QSize size)
        /* 
        brief：设定输出图像大小
        这是AcquisitionManager类的成员函数，用于设置输出图像的大小。它执行以下操作：
        1. 将指定的size参数传递给m_imageTranscoder对象的SetOutputSize()函数，以设置输出图像的尺寸。

         */
        {
            m_imageTranscoder.SetOutputSize(size);
        }

        void VMB_CALL AcquisitionManager::FrameCallback(VmbHandle_t /* cameraHandle */, VmbHandle_t const streamHandle, VmbFrame_t* frame)
       /* 
       brief:帧回调函数
        这是AcquisitionManager类的静态成员函数，作为帧回调函数被调用。它执行以下操作：
        1. 首先，检查frame是否为非空指针。
        2. 创建一个名为context的AcquisitionContext对象，将*frame作为参数传递给它。通过这样做，从frame中提取出与采集相关的上下文信息，并将其存储在context对象中。
        3. 检查context.m_acquisitionManager是否为非空指针。如果不为空，表示成功提取出与AcquisitionManager相关的上下文信息。
        4. 调用context.m_acquisitionManager的FrameReceived()函数，传递streamHandle和frame作为参数。通过这样做，将帧数据传递给AcquisitionManager对象的FrameReceived()函数进行处理。
        */
        {
            if (frame != nullptr)
            {
                AcquisitionContext context(*frame);
                if (context.m_acquisitionManager != nullptr)
                {
                    context.m_acquisitionManager->FrameReceived(streamHandle, frame);
                }
            }
        }

        void AcquisitionManager::FrameReceived(VmbHandle_t const streamHandle, VmbFrame_t const* frame)
        /* 
        brief：接受帧数据
        这是AcquisitionManager类的成员函数，用于接收帧数据。它执行以下操作：
        调用m_imageTranscoder对象的PostImage()函数，传递streamHandle、&AcquisitionManager::FrameCallback（函数指针）和frame作为参数。通过这样做，将帧数据提交给m_imageTranscoder对象进行处理。
         */
        {
            m_imageTranscoder.PostImage(streamHandle, &AcquisitionManager::FrameCallback, frame);
        }

        AcquisitionManager::CameraAccessLifetime::CameraAccessLifetime(VmbCameraInfo_t const& camInfo, AcquisitionManager& acquisitionManager)
        /*
        该构造函数的目的是打开相机、刷新相机信息、执行一些命令和检查，并创建相应的对象。
        它在CameraAccessLifetime对象的构造过程中执行这些操作，用于管理相机访问的生命周期。 
         */
        {
            /* 
            这行代码打开一个相机设备，使用camInfo.cameraIdString表示要打开的相机的标识符，并指定访问模式为VmbAccessModeFull。
            打开成功后，将相机的句柄存储在m_cameraHandle成员变量中。
             */
            VmbError_t error = VmbCameraOpen(camInfo.cameraIdString, VmbAccessModeFull, &m_cameraHandle);
            if (error != VmbErrorSuccess)
            {
                throw VmbException::ForOperation(error, "VmbCameraOpen");
            }

            // refresh camera info to get streams
            VmbCameraInfo_t refreshedCameraInfo;
            bool errorHappened = false;
            VmbException ex;
            error = VmbCameraInfoQueryByHandle(m_cameraHandle, &refreshedCameraInfo, sizeof(refreshedCameraInfo));
            /*
            根据刷新后的相机信息进行一系列的错误检查和操作，包括验证相机的本地设备句柄是否为空，检查相机是否提供了流等。 
             */
            if (error != VmbErrorSuccess)
            {
                errorHappened = true;
                ex = VmbException::ForOperation(error, "VmbCameraOpen");
            }

            if (!errorHappened)
            {
                errorHappened = true;
                if (refreshedCameraInfo.localDeviceHandle == nullptr)
                {
                    ex = VmbException("The id could not be used to query the info of the correct camera");
                }
                else if (refreshedCameraInfo.streamCount == 0)
                {
                    ex = VmbException("The camera does not provide a stream");
                }
                else
                {
                    errorHappened = false;
                }
            }

            if (!errorHappened)
            {
                // execute packet size adjustment, if this is a AVT GigE camera
                /* 
                这行代码执行一个名为GVSPAdjustPacketSize的自定义命令，用于调整数据包大小。
                如果执行成功，则进入相关的命令完成检查和数据包大小获取的逻辑。
                 */
                if (VmbErrorSuccess == VmbFeatureCommandRun(refreshedCameraInfo.streamHandles[0], AdjustPackageSizeCommand))
                {
                    VmbBool_t isCommandDone = VmbBoolFalse;
                    do
                    {
                        if (VmbErrorSuccess != VmbFeatureCommandIsDone(refreshedCameraInfo.streamHandles[0],
                            AdjustPackageSizeCommand,
                            &isCommandDone))
                        {
                            break;
                        }
                    } while (VmbBoolFalse == isCommandDone);
                    VmbInt64_t packetSize = 0;
                    VmbFeatureIntGet(refreshedCameraInfo.streamHandles[0], "GVSPPacketSize", &packetSize);
                    printf("GVSPAdjustPacketSize: %lld\n", packetSize);
                }

                try
                {
                    /*
                    brief:
                    这行代码创建一个名为StreamLifetime的对象，并传递refreshedCameraInfo.streamHandles[0]（流句柄）、
                    m_cameraHandle（相机句柄）和acquisitionManager给它。然后，通过m_streamLife成员变量持有这个对象。 
                     */
                    m_streamLife.reset(new StreamLifetime(refreshedCameraInfo.streamHandles[0], m_cameraHandle, acquisitionManager));
                }

                /* 
                如果在上述操作中出现错误，会进行相应的清理操作，并抛出相关的异常。
                 */
                catch (...)
                {
                    VmbCameraClose(m_cameraHandle);
                    throw;
                }
            }
            else
            {
                VmbCameraClose(m_cameraHandle);
                throw ex;
            }
        }

        AcquisitionManager::CameraAccessLifetime::~CameraAccessLifetime()
        /* 
        brief：解除系统占用，先关闭相机流再关闭相机接口
        首先关闭相机的流，然后关闭相机本身。这样可以确保在对象销毁时释放相关资源并保持系统的正确状态。        
        1. m_streamLife.reset();: 这行代码通过调用reset()函数将m_streamLife指针重置为空指针，从而关闭相机的流。
        2. VmbCameraClose(m_cameraHandle);: 这行代码关闭相机，使用m_cameraHandle参数表示要关闭的相机的句柄。
         */
        {
            m_streamLife.reset(); // close stream first
            VmbCameraClose(m_cameraHandle);
        }

        AcquisitionManager::StreamLifetime::StreamLifetime(VmbHandle_t const streamHandle, VmbHandle_t const cameraHandle, AcquisitionManager& acquisitionManager)
        {
            VmbUint32_t value;
            /* 
            使用VmbPayloadSizeGet()函数获取给定流句柄的有效负载大小，并将结果存储在变量value中。
             */
            VmbError_t error = VmbPayloadSizeGet(streamHandle, &value);
            if (error != VmbErrorSuccess)
            /* 
            检查VmbPayloadSizeGet()的返回值是否为VmbErrorSuccess，如果不是，则抛出相应的异常。
             */
            {
                throw VmbException::ForOperation(error, "VmbPayloadSizeGet");
            }

            if (value == 0)
            /* 
            检查负载大小是否为零，如果是，则抛出相应的异常。
             */
            {
                throw VmbException("Non-zero payload size required");
            }
            // Evaluate required alignment for frame buffer in case announce frame method is used
            /* 
            1. 获取帧缓冲区的所需对齐方式（如果使用announce frame方法）。
            2. 使用VmbFeatureIntGet()函数获取属性名为"StreamBufferAlignment"的属性值，并将结果存储在变量nStreamBufferAlignment中。
            3. 如果获取失败，则将对齐方式设置为1。
             */
            VmbInt64_t nStreamBufferAlignment = 1;  // Required alignment of the frame buffer
            if (VmbErrorSuccess != VmbFeatureIntGet(streamHandle, "StreamBufferAlignment", &nStreamBufferAlignment))
                nStreamBufferAlignment = 1;
            /* 
            如果对齐方式小于1，则将其设置为1。
             */
            if (nStreamBufferAlignment < 1)
                nStreamBufferAlignment = 1;
            
            /* 
            将有效负载大小和缓冲区对齐方式存储在成员变量m_payloadSize和bufferAlignment中。
             */
            m_payloadSize = static_cast<size_t>(value);
            size_t bufferAlignment = static_cast<size_t>(nStreamBufferAlignment);
            /* 
            1. 创建一个名为AcquisitionLifetime的对象，并通过m_acquisitionLife成员变量持有它。
            2. 传递cameraHandle、m_payloadSize、bufferAlignment和acquisitionManager给构造函数，以初始化采集的生命周期。
             */
            m_acquisitionLife.reset(new AcquisitionLifetime(cameraHandle, m_payloadSize, bufferAlignment, acquisitionManager));
        }

        AcquisitionManager::StreamLifetime::~StreamLifetime()
        /* 
        析构函数
         */
        {
        }

        namespace
        {
            void RunCommand(VmbHandle_t const camHandle, std::string const& command)
            /* 这是一个用于执行相机命令的函数。它接受相机句柄camHandle和命令字符串command作为参数。 */
            {
                auto error = VmbFeatureCommandRun(camHandle, command.c_str());

                if (error != VmbErrorSuccess)
                {
                    throw VmbException::ForOperation(error, "VmbFeatureCommandRun");
                }

                VmbBool_t done = false;
                while (!done)
                {
                    error = VmbFeatureCommandIsDone(camHandle, command.c_str(), &done);
                    /* 
                    调用VmbFeatureCommandIsDone函数来检查命令是否执行完成。
                    它将相机句柄、命令字符串和指向done变量的指针作为参数传递给该函数，并将返回值存储在error变量中。 
                    */
                    if (error != VmbErrorSuccess)
                    {
                        throw VmbException::ForOperation(error, "VmbFeatureCommandIsDone");
                    }
                }
            }
        }

        AcquisitionManager::AcquisitionLifetime::AcquisitionLifetime(VmbHandle_t const camHandle, size_t payloadSize, size_t nBufferAlignment, AcquisitionManager& acquisitionManager)
            : m_camHandle(camHandle)
        {
            m_frames.reserve(BufferCount);
            for (size_t count = BufferCount; count > 0; --count)
            {
                auto frame = std::unique_ptr<Frame>(new Frame(payloadSize, nBufferAlignment));
                m_frames.emplace_back(std::move(frame));
            }

            VmbError_t error = VmbErrorSuccess;
            for (auto& frame : m_frames)
            {
                AcquisitionContext context(&acquisitionManager);
                context.FillFrame(frame->m_frame);

                error = VmbFrameAnnounce(camHandle, &(frame->m_frame), sizeof(frame->m_frame));
                if (error != VmbErrorSuccess)
                {
                    break;
                }
            }

            if (error != VmbErrorSuccess)
            {
                VmbFrameRevokeAll(camHandle); // error ignored on purpose
                throw VmbException::ForOperation(error, "VmbFrameAnnounce");
            }

            error = VmbCaptureStart(camHandle);
            if (error != VmbErrorSuccess)
            {
                throw VmbException::ForOperation(error, "VmbCaptureStart");
            }

            size_t numberEnqueued = 0;

            for (auto& frame : m_frames)
            {
                error = VmbCaptureFrameQueue(camHandle, &(frame->m_frame), &AcquisitionManager::FrameCallback);
                if (error == VmbErrorSuccess)
                {
                    ++numberEnqueued;
                }
            }

            if (numberEnqueued == 0)
            {
                VmbCaptureEnd(camHandle);
                throw VmbException("Non of the frames could be enqueued using VmbCaptureFrameQueue", error);
            }

            try
            {
                RunCommand(camHandle, "AcquisitionStart");
            }
            catch (VmbException const&)
            {
                VmbCaptureEnd(camHandle);
                throw;
            }
        }

        AcquisitionManager::AcquisitionLifetime::~AcquisitionLifetime()
        {
            try
            {
                RunCommand(m_camHandle, "AcquisitionStop");
            }
            catch (VmbException const&)
            {
            }

            VmbCaptureEnd(m_camHandle);
            VmbCaptureQueueFlush(m_camHandle);
            VmbFrameRevokeAll(m_camHandle);
        }

        AcquisitionManager::Frame::Frame(size_t payloadSize, size_t bufferAlignment)
        {
            if (payloadSize > (std::numeric_limits<VmbUint32_t>::max)())
            {
                throw VmbException("payload size outside of allowed range");
            }
#ifdef _WIN32
            m_frame.buffer = (unsigned char*)_aligned_malloc(payloadSize,bufferAlignment);
#else
            m_frame.buffer = (unsigned char*)aligned_alloc(bufferAlignment, payloadSize);
#endif
            if (m_frame.buffer == nullptr)
            {
                throw VmbException("Unable to allocate memory for frame", VmbErrorResources);
            }
            m_frame.bufferSize = static_cast<VmbUint32_t>(payloadSize);
        }

        AcquisitionManager::Frame::~Frame()
        {
#ifdef _WIN32
            _aligned_free(m_frame.buffer);
#else
            std::free(m_frame.buffer);
#endif
        }

    }
}
