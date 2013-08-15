/*****************************************************************************
**
** OS abstraction layer for ARM's CMSIS.
** Copyright (C) 2013  Manuel Freiberger
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see http://www.gnu.org/licenses/gpl-3.0.
**
*****************************************************************************/

#ifndef OSL_CMSIS_SEMAPHORE_HPP
#define OSL_CMSIS_SEMAPHORE_HPP

#include "cmsis_os.h"

class semaphore : boost::noncopyable
{
public:
    explicit semaphore(int32_t count = 0)
        : m_id(0)
    {
        osSemaphoreDef_t semaphoreDef = { m_cmsisSemaphoreControlBlock };
        m_id = osSemaphoreCreate(&osSemaphoreDef, count);
    }

    ~semaphore()
    {
        if (m_id)
            osSemaphoreDelete(m_id);
    }

    void wait()
    {
        osSemaphoreWait(m_id, osWaitForever);
    }

    void release()
    {
        osSemaphoreRelease(m_id);
    }

private:
    uint32_t m_cmsisSemaphoreControlBlock[2];
    osSemaphoreId m_id;
};

#endif // OSL_CMSIS_SEMAPHORE_HPP
