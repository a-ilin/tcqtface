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

#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include "common.h"

class Semaphore
{
public:
  Semaphore(LONG initial = 1, LONG maximum = 1)
    : m_handle(CreateSemaphore(NULL, initial, maximum, NULL))
  {
    _assert(m_handle);
  }

  ~Semaphore()
  {
    CloseHandle(m_handle);
  }

  DWORD lock(DWORD dwMilliseconds = INFINITE)
  {
    return WaitForSingleObject(m_handle, dwMilliseconds);
  }

  LONG unlock()
  {
    LONG prev = 0;
    BOOL res = ReleaseSemaphore(m_handle, 1, &prev);
    _assert(res);
    return prev;
  }

private:
  Semaphore(const Semaphore&) = delete;
  Semaphore& operator = (const Semaphore&) = delete;

private:
  HANDLE m_handle;
};

class SemaphoreLocker
{
public:
  SemaphoreLocker(Semaphore* sem)
    : m_sem(sem)
  {
    _assert(m_sem);
    m_sem->lock();
  }

  ~SemaphoreLocker()
  {
    m_sem->unlock();
  }

private:
  SemaphoreLocker(const SemaphoreLocker&) = delete;
  SemaphoreLocker& operator = (const SemaphoreLocker&) = delete;

private:
  Semaphore* m_sem;
};

#endif // SEMAPHORE_H
