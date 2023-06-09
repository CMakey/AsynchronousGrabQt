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
 * \brief Implementation of ::VmbC::Examples::VmbLibraryLifetime
 */

#include "VmbException.h"
#include "VmbLibraryLifetime.h"

namespace VmbC
{
    namespace Examples
    {

        VmbLibraryLifetime::VmbLibraryLifetime()
        /* 该构造函数用于初始化 VmbC 库。它调用 VmbStartup 函数，并将返回的错误码进行判断，
        如果不是 VmbErrorSuccess，则抛出一个 VmbException 异常，异常的错误信息包含了操作名称和错误码。 */
        {
            VmbError_t startupError = VmbStartup(nullptr);
            if (startupError != VmbErrorSuccess)
            {
                throw VmbException::ForOperation(startupError, "VmbStartup");
            }
        }

        VmbLibraryLifetime::~VmbLibraryLifetime()
        {
            VmbShutdown();
        }
    }
}
