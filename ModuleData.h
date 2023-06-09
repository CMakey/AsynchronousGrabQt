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
 * \brief Definition of base class for modules
 */

#ifndef ASYNCHRONOUSGRAB_C_MODULE_DATA_H
#define ASYNCHRONOUSGRAB_C_MODULE_DATA_H

#include <string>

#include <VmbC/VmbC.h>

class MainWindow;

namespace VmbC
{
    namespace Examples
    {
        class ApiController;

        class ModuleData
        {
        public:
            struct Visitor
            /* 访问者结构体，其中定义了纯虚函数 Visit，用于访问不同类型的数据。 */
            {
                virtual ~Visitor() = default;

                virtual void Visit(VmbCameraInfo_t const& data);

                virtual void Visit(VmbInterfaceInfo_t const& data);

                virtual void Visit(VmbTransportLayerInfo_t const& data);
            };

            virtual ~ModuleData() = default; //返回指向父模块数据的指针。

            ModuleData* GetParent();

            virtual void Accept(Visitor& visitor) const = 0;//纯虚函数，用于接受访问者对象。
        protected:
            ModuleData();//默认构造函数，初始化 m_parent 成员变量为 nullptr。
            ModuleData* m_parent;

        };

        /**
         * \brief a class holding the info about a single module
         * \tparam T the info type, e.g. VmbCameraInfo_t or VmbInterfaceInfo_t
         */
        template<typename T>
        class ModuleDataImpl : public ModuleData
        /* 
        InfoType 类型是模板参数，用于指定模块信息的类型（例如 VmbCameraInfo_t 或 VmbInterfaceInfo_t）。
         */
        {
        public:
            using InfoType = T;

            void Accept(Visitor& visitor) const override
            /* 实现了基类的虚函数，用于接受访问者对象并调用 visitor.Visit 方法。 */
            {
                visitor.Visit(m_info);
            }

            ModuleDataImpl(InfoType const& info)
                : m_info(info)
                /* 构造函数，使用传入的模块信息初始化 m_info 成员变量。 */
            {
            }

            InfoType const& GetInfo() const
            /* 返回模块信息的引用。 */
            {
                return m_info;
            }

            void SetParent(ModuleData* parent)
            /* 设置父模块数据 */
            {
                m_parent = parent;
            }
        private:

            /**
             * \brief Vmb C Api struct holding the info about the module
             */
            InfoType m_info;

        };
        //定义了别名
        using CameraData = ModuleDataImpl<VmbCameraInfo_t>;
        using InterfaceData = ModuleDataImpl<VmbInterfaceInfo_t>;
        using TlData = ModuleDataImpl<VmbTransportLayerInfo_t>;

    } // namespace Examples
} // namespace VmbC

#endif
