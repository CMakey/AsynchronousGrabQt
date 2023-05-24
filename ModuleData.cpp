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
 * \brief Implementation of ModuleData.
 */

#include "ModuleData.h"


/* 模块提供了一种通用的模块数据存储和访问机制，通过使用不同的访问者对象，可以对不同类型的模块信息进行处理和访问
 */
namespace VmbC
{
    namespace Examples
    {
        ModuleData* ModuleData::GetParent()
        /* 返回指向父模块数据的指针。 */
        {
            return m_parent;
        }

        ModuleData::ModuleData()
        /* 构造函数，初始化 m_parent 成员变量为 nullptr。 */
            : m_parent(nullptr)
        {
        }

        void ModuleData::Visitor::Visit(VmbCameraInfo_t const& data)
        /* 访问函数，用于处理 VmbCameraInfo_t 类型的数据。 */
        {
        }

        void ModuleData::Visitor::Visit(VmbInterfaceInfo_t const& data)
        {
        }

        void ModuleData::Visitor::Visit(VmbTransportLayerInfo_t const& data)
        {
        }
    } // namespace Examples
} // namespace VmbC
