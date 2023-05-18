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
 * \brief Implementation file for the ApiController helper class that
 *        demonstrates how to implement an asynchronous, continuous image
 *        acquisition with VmbC.
 */

#include <algorithm>

#include "ApiController.h"
#include "VmbException.h"

namespace VmbC
{
    namespace Examples
    {

        enum { NUM_FRAMES = 3, };

        ApiController::ApiController(MainWindow& mainWindow)
        /* 一个控制器类，用于管理相机、系统和接口的列表，并提供版本信息查询功能。 */
            : m_libraryLife {}
        {
        }

        namespace
        {

            template<typename InfoType, typename RetrievalFunctor>
            std::vector<std::unique_ptr<ModuleDataImpl<InfoType>>> ListModulesImpl(RetrievalFunctor retrievalFunction, const char* functionName)
            /* 函数是一个通用的模板函数，用于获取指定信息类型的模块列表。
            它接受一个检索函数和函数名称作为参数，使用检索函数获取模块列表并进行处理，返回一个包含ModuleDataImpl对象的向量。 */
            {
                VmbUint32_t count;

                VmbError_t error = retrievalFunction(nullptr, 0, &count, sizeof(InfoType));
                if (error != VmbErrorSuccess)
                {
                    throw VmbException::ForOperation(error, functionName);
                }

                std::vector<InfoType> buffer(count);

                VmbUint32_t filledCount;

                error = retrievalFunction(buffer.data(), count, &filledCount, sizeof(InfoType));

                // for simplicity we ignore the case where the list grows between calls
                if (error != VmbErrorSuccess && error != VmbErrorMoreData)
                {
                    throw VmbException::ForOperation(error, functionName);
                }

                if (filledCount < count)
                {
                    buffer.resize(filledCount);
                }

                std::vector<std::unique_ptr<ModuleDataImpl<InfoType>>> result(buffer.size());

                std::transform(buffer.begin(), buffer.end(), result.begin(), [](InfoType const& info)
                               {
                                   return std::unique_ptr<ModuleDataImpl<InfoType>>(new ModuleDataImpl<InfoType>(info));
                               });

                return result;
            }

            template<typename InfoType>
            std::vector<std::unique_ptr<ModuleDataImpl<InfoType>>> ListModules();

            template<>
            std::vector<std::unique_ptr<ModuleDataImpl<VmbTransportLayerInfo_t>>> ListModules<VmbTransportLayerInfo_t>()
            {
                return ListModulesImpl<VmbTransportLayerInfo_t>(VmbTransportLayersList, "VmbTransportLayersList");
            }

            template<>
            std::vector<std::unique_ptr<ModuleDataImpl<VmbInterfaceInfo_t>>> ListModules<VmbInterfaceInfo_t>()
            {
                return ListModulesImpl<VmbInterfaceInfo_t>(VmbInterfacesList, "VmbInterfacesList");
            }

            template<>
            std::vector<std::unique_ptr<ModuleDataImpl<VmbCameraInfo_t>>> ListModules<VmbCameraInfo_t>()
            {
                return ListModulesImpl<VmbCameraInfo_t>(VmbCamerasList, "VmbCamerasList");
            }

        };

        std::vector<std::unique_ptr<CameraData>> ApiController::GetCameraList()
        /* GetCameraList()函数，返回一个包含CameraData对象的向量，用于获取相机列表。 */
        {
            return ListModules<VmbCameraInfo_t>();
        }

        std::vector<std::unique_ptr<TlData>> ApiController::GetSystemList()
        /* 返回一个包含TlData对象的向量，用于获取系统列表 */
        {
            return ListModules<VmbTransportLayerInfo_t>();
        }

        std::vector<std::unique_ptr<InterfaceData>> ApiController::GetInterfaceList()
        /* 返回一个包含InterfaceData对象的向量，用于获取接口列表。 */
        {
            return ListModules<VmbInterfaceInfo_t>();
        }

        std::string ApiController::GetVersion() const
        /* 返回一个表示版本信息的字符串。 */
        {
            std::ostringstream os;

            VmbVersionInfo_t versionInfo;
            auto const error = VmbVersionQuery(&versionInfo, sizeof(versionInfo));

            if (error == VmbErrorSuccess)
            {
                os
                    << versionInfo.major << '.'
                    << versionInfo.minor << '.'
                    << versionInfo.patch;
            }

            return os.str();
        }

    } // namespace Examples
} // namespace VmbC
