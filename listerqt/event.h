/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2015-2019 by Aleksei Ilin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/

#ifndef EVENT_H
#define EVENT_H

#include  "common.h"

class Event
{
public:
  Event(BOOL bManualReset = FALSE, BOOL bInitialSignaled = FALSE)
    : m_handle(CreateEvent(NULL, bManualReset, bInitialSignaled, NULL))
  {
    _assert(m_handle);
  }

  ~Event()
  {
    CloseHandle(m_handle);
  }

  void set()
  {
    SetEvent(m_handle);
  }

  void reset()
  {
    ResetEvent(m_handle);
  }

  DWORD wait(DWORD dwMilliseconds = INFINITE)
  {
    return WaitForSingleObject(m_handle, dwMilliseconds);
  }

private:
  Event(const Event&) = delete;
  Event& operator= (const Event&) = delete;

private:
  HANDLE m_handle;
};

#endif // EVENT_H
