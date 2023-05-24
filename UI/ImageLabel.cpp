/*=============================================================================
  Copyright (C) 2012 - 2021 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        ImageLabel.cpp

  Description: Implementation of ImageLabel.

-------------------------------------------------------------------------------

  THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED
  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF TITLE,
  NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR  PURPOSE ARE
  DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
  AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=============================================================================*/

#include <QResizeEvent>

#include "UI/ImageLabel.h"

ImageLabel::ImageLabel(QWidget* parent, Qt::WindowFlags flags)
    : QLabel(parent, flags)
    /* 接受一个父窗口指针 parent 和窗口标志 flags，并通过调用 QLabel 的构造函数进行初始化。 */
{
}


void ImageLabel::resizeEvent(QResizeEvent* event)
/* 是一个重写的事件处理函数，用于处理控件的大小调整事件。
在函数内部，它首先调用了 QLabel 的 resizeEvent 函数，以确保基类的默认行为得以执行。
然后，通过发射 sizeChanged 信号，将调整后的大小信息作为参数发送出去，以通知其他对象或部件进行相应的处理。 */
{
    QLabel::resizeEvent(event);
    emit sizeChanged(event->size());
}
