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
 * \brief Implementation of ::VmbC::Examples::VmbException
 */

#include <cassert>

#include "VmbException.h"

/* brief：可以在异常情况下创建和处理 VmbException 对象，并获取相关的错误代码和错误信息。 */
namespace VmbC
{
    namespace Examples
    {

        VmbException::VmbException(std::string const& message, VmbError_t exitCode)
            : m_exitCode(exitCode),
            m_errorMessage(message)
            /* 接受一个 std::string 类型的 message 和一个 VmbError_t 类型的 exitCode 参数，
            用于初始化 VmbException 对象的成员变量。
            构造函数会断言 exitCode 不等于 VmbErrorSuccess。 */
        {
            assert(exitCode != VmbErrorSuccess);
        }

        VmbException VmbException::ForOperation(VmbError_t exitCode, std::string const& operation)
        /* 接受一个 VmbError_t 类型的 exitCode 参数和一个 std::string 类型的 operation 参数，用于创建并返回一个 VmbException 对象。
        该函数会将 operation 和 exitCode 拼接成错误信息字符串，并用它来初始化 VmbException 对象的成员变量。 */
        {
            return VmbException(operation + " call unsuccessful; exit code " + std::to_string(exitCode), exitCode);
        }
    }
}
