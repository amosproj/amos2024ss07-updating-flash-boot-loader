/**
 * \file IfxStdIf_DPipe.h
 * \brief Standard interface: Data Pipe
 * \ingroup IfxStdIf
 *
 * \copyright Copyright (c) 2013 Infineon Technologies AG. All rights reserved.
 *
 * $Date: 2014-02-27 20:08:24 GMT$
 *
 *                                 IMPORTANT NOTICE
 *
 * Use of this file is subject to the terms of use agreed between (i) you or
 * the company in which ordinary course of business you are acting and (ii)
 * Infineon Technologies AG or its licensees. If and as long as no such terms
 * of use are agreed, use of this file is subject to following:
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer, must
 * be included in all copies of the Software, in whole or in part, and all
 * derivative works of the Software, unless such copies or derivative works are
 * solely in the form of machine-executable object code generated by a source
 * language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 *
 * \defgroup library_srvsw_stdif_dpipe Standard interface: Data Pipe
 * \ingroup library_srvsw_stdif
 *
 * The standard interafce data pipe (DPipe) abstract the hardware used for data transfer. It provide, after proper initialization an hardware independant way to write
 * and read data to/from as communciation channel.
 *
 * \par Porting StandardIo to DPipe
 * replace all
 *  - StandardIo type with IfxStdIf_DPipe
 *  - StandardIo_print with IfxStdIf_DPipe_print
 * delete StandardIo.c and StandardIo.h
 * Include "StdIf/IfxStdIf_DPipe.h" instead of "SysSe/Bsp/StandardIo.h"
 *
 * The following files are already ported: Ifx_Console, Ifx_Shell
 *
 */
#ifndef STDIF_DPIPE_H_
#define STDIF_DPIPE_H_ 1

#include "IfxStdIf.h"
//----------------------------------------------------------------------------------------
#ifndef ENDL
#    define ENDL       "\r\n"
#endif

/** \brief Forward declaration */
typedef struct IfxStdIf_DPipe_ IfxStdIf_DPipe;

typedef volatile boolean      *IfxStdIf_DPipe_WriteEvent;
typedef volatile boolean      *IfxStdIf_DPipe_ReadEvent;

/** \brief Size of the buffer allocated on the stack for the print function */
#define STDIF_DPIPE_MAX_PRINT_SIZE (255)

/** \brief Write binary data into the \ref IfxStdIf_DPipe.
 *
 * Initially the parameter 'count' specifies count of data to write.
 * After execution the data pointed by 'count' specifies the data actually written
 *
 * \param stdif Pointer to the interface driver object
 * \param data Pointer to the start of data
 * \param count Pointer to the count of data (in bytes).
 * \param timeout in system timer ticks
 *
 * \retval TRUE Returns TRUE if all items could be written
 * \retval FALSE Returns FALSE if not all the items could be written
 */
typedef boolean (*IfxStdIf_DPipe_Write)(IfxStdIf_InterfaceDriver stdIf, void *data, Ifx_SizeT *count, Ifx_TickTime timeout);

/** \brief Read data from the \ref IfxStdIf_DPipe object
 *
 * Initially the parameter 'count' specifies count of data to read.
 * After execution the data pointed by 'count' specifies the data actually read.
 *
 * \param stdif Pointer to the interface driver object
 * \param data Pointer to the start of data
 * \param count Pointer to the count of data (in bytes).
 * \param timeout in system timer ticks
 *
 * \retval TRUE Returns TRUE if all items could be read
 * \retval FALSE Returns FALSE if not all the items could be read
 */
typedef boolean (*IfxStdIf_DPipe_Read)(IfxStdIf_InterfaceDriver stdIf, void *data, Ifx_SizeT *count, Ifx_TickTime timeout);

/** \brief Returns the number of bytes in the rx buffer
 *
 * \param stdif Pointer to the interface driver object
 *
 * \return Returns the number of bytes in the rx buffer
 */
typedef sint32 (*IfxStdIf_DPipe_GetReadCount)(IfxStdIf_InterfaceDriver stdIf);

/** \brief Returns read event object
 *
 * \param stdif Pointer to the interface driver object
 *
 * \return Returns read event object
 */
typedef IfxStdIf_DPipe_ReadEvent (*IfxStdIf_DPipe_GetReadEvent)(IfxStdIf_InterfaceDriver stdIf);

/** \brief Returns number of bytes send
 *
 * \param stdif Pointer to the interface driver object
 *
 * \return Returns number of bytes send
 */
typedef uint32 (*IfxStdIf_DPipe_GetSendCount)(IfxStdIf_InterfaceDriver stdIf);

/** \brief Returns the time stamp of the last transmit data
 *
 * \param stdif Pointer to the interface driver object
 *
 * \return Returns the time stamp of the last transmit data
 */
typedef Ifx_TickTime (*IfxStdIf_DPipe_GetTxTimeStamp)(IfxStdIf_InterfaceDriver stdIf);

/** \brief Returns the number of free bytes (free space) in the tx buffer
 *
 * \param stdif Pointer to the interface driver object
 *
 * \return Returns the number of free bytes in the tx buffer
 */
typedef sint32 (*IfxStdIf_DPipe_GetWriteCount)(IfxStdIf_InterfaceDriver stdIf);

/** \brief Returns write event object
 *
 * \param stdif Pointer to the interface driver object
 *
 * \return Returns write event object
 */
typedef IfxStdIf_DPipe_WriteEvent (*IfxStdIf_DPipe_GetWriteEvent)(IfxStdIf_InterfaceDriver stdIf);

/** \brief Indicates if the required number of bytes are available for read in the buffer
 *
 * \param stdif Pointer to the interface driver object
 * \param count Pointer to the count of data (in bytes).
 * \param timeout in system timer ticks
 *
 * \return Returns TRUE if at least count bytes are available for read in the rx buffer,
 * if not the Event is armed to be set when the buffer count is bigger or equal to the requested count
 */
typedef boolean (*IfxStdIf_DPipe_CanReadCount)(IfxStdIf_InterfaceDriver stdIf, Ifx_SizeT count, Ifx_TickTime timeout);

/** \brief  Indicates if there is enough free space to write the data in the buffer
 *
 * \param stdif Pointer to the interface driver object
 * \param count Pointer to the count of data (in bytes).
 * \param timeout in system timer ticks
 *
 * \return Returns TRUE if at least count bytes can be written to the tx buffer,
 *  if not the Event is armed to be set when the buffer free count is bigger or equal to the requested count
 */
typedef boolean (*IfxStdIf_DPipe_CanWriteCount)(IfxStdIf_InterfaceDriver stdIf, Ifx_SizeT count, Ifx_TickTime timeout);

/** \brief Flush the transmit buffer by transmitting all data
 *
 * \param stdif Pointer to the interface driver object
 * \param timeout timeout for the flush operation
 *
 * \return Returns TRUE if the FIFO is empty
 */
typedef boolean (*IfxStdIf_DPipe_FlushTx)(IfxStdIf_InterfaceDriver stdIf, Ifx_TickTime timeout);

/** \brief Clears the RX buffer by removing all data
 *
 * \param stdif Pointer to the interface driver object
 * \return void
 */
typedef void (*IfxStdIf_DPipe_ClearRx)(IfxStdIf_InterfaceDriver stdIf);

/** \brief Clears the TX buffer by removing all data
 *
 * \param stdif Pointer to the interface driver object
 * \return void
 */
typedef void (*IfxStdIf_DPipe_ClearTx)(IfxStdIf_InterfaceDriver stdIf);

/** \brief handler called on reveive event
 *
 * \param stdif Pointer to the interface driver object
 *
 * \return none
 */
typedef void (*IfxStdIf_DPipe_OnReceive)(IfxStdIf_InterfaceDriver stdIf);
/** \brief handler called on transmit event
 *
 * \param stdif Pointer to the interface driver object
 *
 * \return none
 */
typedef void (*IfxStdIf_DPipe_OnTransmit)(IfxStdIf_InterfaceDriver stdIf);
/** \brief handler called on error event
 *
 * \param stdif Pointer to the interface driver object
 *
 * \return none
 */
typedef void (*IfxStdIf_DPipe_OnError)(IfxStdIf_InterfaceDriver stdIf);
/** \brief Reset the sendCount counter
 *
 * \param stdif Pointer to the interface driver object
 *
 * \return none
 */
typedef void (*IfxStdIf_DPipe_ResetSendCount)(IfxStdIf_InterfaceDriver stdIf);

/** \brief Standard interface object
 */
struct IfxStdIf_DPipe_
{
    IfxStdIf_InterfaceDriver driver;              /**< \brief Pointer to the specific driver object */
    boolean                  txDisabled;          /**< \brief If disabled is set to TRUE, the output is disabled, else enabled */

    /* Standard interface APIs */
    IfxStdIf_DPipe_Write          write;          /**< \brief \see IfxStdIf_DPipe_Write */
    IfxStdIf_DPipe_Read           read;           /**< \brief \see IfxStdIf_DPipe_Read */
    IfxStdIf_DPipe_GetReadCount   getReadCount;   /**< \brief \see IfxStdIf_DPipe_GetReadCount */
    IfxStdIf_DPipe_GetReadEvent   getReadEvent;   /**< \brief \see IfxStdIf_DPipe_GetReadEvent */
    IfxStdIf_DPipe_GetWriteCount  getWriteCount;  /**< \brief \see IfxStdIf_DPipe_GetWriteCount */
    IfxStdIf_DPipe_GetWriteEvent  getWriteEvent;  /**< \brief \see IfxStdIf_DPipe_GetWriteEvent */
    IfxStdIf_DPipe_CanReadCount   canReadCount;   /**< \brief \see IfxStdIf_DPipe_CanReadCount */
    IfxStdIf_DPipe_CanWriteCount  canWriteCount;  /**< \brief \see IfxStdIf_DPipe_CanWriteCount */
    IfxStdIf_DPipe_FlushTx        flushTx;        /**< \brief \see IfxStdIf_DPipe_FlushTx */
    IfxStdIf_DPipe_ClearTx        clearTx;        /**< \brief \see IfxStdIf_DPipe_ClearTx */
    IfxStdIf_DPipe_ClearRx        clearRx;        /**< \brief \see IfxStdIf_DPipe_ClearRx */
    IfxStdIf_DPipe_OnReceive      onReceive;      /**< \brief \see IfxStdIf_DPipe_OnReceive  */
    IfxStdIf_DPipe_OnTransmit     onTransmit;     /**< \brief \see IfxStdIf_DPipe_OnTransmit */
    IfxStdIf_DPipe_OnError        onError;        /**< \brief \see IfxStdIf_DPipe_OnError    */

    IfxStdIf_DPipe_GetSendCount   getSendCount;   /**< \brief \see IfxStdIf_DPipe_GetSendCount    */
    IfxStdIf_DPipe_GetTxTimeStamp getTxTimeStamp; /**< \brief \see IfxStdIf_DPipe_GetTxTimeStamp    */
    IfxStdIf_DPipe_ResetSendCount resetSendCount; /**< \brief \see IfxStdIf_DPipe_ResetSendCount    */
};
/** \addtogroup library_srvsw_stdif_dpipe
 * \{ */
/** \copydoc IfxStdIf_DPipe_Write
 */
IFX_INLINE boolean IfxStdIf_DPipe_write(IfxStdIf_DPipe *stdIf, void *data, Ifx_SizeT *count, Ifx_TickTime timeout)
{
    return stdIf->write(stdIf->driver, data, count, timeout);
}


/** \copydoc IfxStdIf_DPipe_Read
 */
IFX_INLINE boolean IfxStdIf_DPipe_read(IfxStdIf_DPipe *stdIf, void *data, Ifx_SizeT *count, Ifx_TickTime timeout)
{
    return stdIf->read(stdIf->driver, data, count, timeout);
}


/** \copydoc IfxStdIf_DPipe_GetReadCount
 */
IFX_INLINE sint32 IfxStdIf_DPipe_getReadCount(IfxStdIf_DPipe *stdIf)
{
    return stdIf->getReadCount(stdIf->driver);
}


/** \copydoc IfxStdIf_DPipe_GetWriteCount
 */
IFX_INLINE sint32 IfxStdIf_DPipe_getWriteCount(IfxStdIf_DPipe *stdIf)
{
    return stdIf->getWriteCount(stdIf->driver);
}


/** \copydoc IfxStdIf_DPipe_CanReadCount
 */
IFX_INLINE boolean IfxStdIf_DPipe_canReadCount(IfxStdIf_DPipe *stdIf, Ifx_SizeT count, Ifx_TickTime timeout)
{
    return stdIf->canReadCount(stdIf->driver, count, timeout);
}


/** \copydoc IfxStdIf_DPipe_CanWriteCount
 */
IFX_INLINE boolean IfxStdIf_DPipe_canWriteCount(IfxStdIf_DPipe *stdIf, Ifx_SizeT count, Ifx_TickTime timeout)
{
    return stdIf->canWriteCount(stdIf->driver, count, timeout);
}


/** \copydoc IfxStdIf_DPipe_GetReadEvent
 */
IFX_INLINE IfxStdIf_DPipe_ReadEvent IfxStdIf_DPipe_getReadEvent(IfxStdIf_DPipe *stdIf)
{
    return stdIf->getReadEvent(stdIf->driver);
}


/** \copydoc IfxStdIf_DPipe_GetWriteEvent
 */
IFX_INLINE IfxStdIf_DPipe_WriteEvent IfxStdIf_DPipe_getWriteEvent(IfxStdIf_DPipe *stdIf)
{
    return stdIf->getWriteEvent(stdIf->driver);
}


/** \copydoc IfxStdIf_DPipe_FlushTx
 */
IFX_INLINE boolean IfxStdIf_DPipe_flushTx(IfxStdIf_DPipe *stdIf, Ifx_TickTime timeout)
{
    return stdIf->flushTx(stdIf->driver, timeout);
}


/** \copydoc IfxStdIf_DPipe_ClearTx
 */
IFX_INLINE void IfxStdIf_DPipe_clearTx(IfxStdIf_DPipe *stdIf)
{
    stdIf->clearTx(stdIf->driver);
}


/** \copydoc IfxStdIf_DPipe_ClearRx
 */
IFX_INLINE void IfxStdIf_DPipe_clearRx(IfxStdIf_DPipe *stdIf)
{
    stdIf->clearRx(stdIf->driver);
}


/** \copydoc IfxStdIf_DPipe_OnReceive
 */
IFX_INLINE void IfxStdIf_DPipe_onReceive(IfxStdIf_DPipe *stdIf)
{
    stdIf->onReceive(stdIf->driver);
}


/** \copydoc IfxStdIf_DPipe_OnTransmit
 */
IFX_INLINE void IfxStdIf_DPipe_onTransmit(IfxStdIf_DPipe *stdIf)
{
    stdIf->onTransmit(stdIf->driver);
}


/** \copydoc IfxStdIf_DPipe_OnError
 */
IFX_INLINE void IfxStdIf_DPipe_onError(IfxStdIf_DPipe *stdIf)
{
    stdIf->onError(stdIf->driver);
}


/** \copydoc IfxStdIf_DPipe_GetSendCount
 */
IFX_INLINE uint32 IfxStdIf_DPipe_getSendCount(IfxStdIf_DPipe *stdIf)
{
    return stdIf->getSendCount(stdIf->driver);
}


/** \copydoc IfxStdIf_DPipe_GetTxTimeStamp
 */
IFX_INLINE Ifx_TickTime IfxStdIf_DPipe_getTxTimeStamp(IfxStdIf_DPipe *stdIf)
{
    return stdIf->getTxTimeStamp(stdIf->driver);
}


/** \copydoc IfxStdIf_DPipe_ResetSendCount
 */
IFX_INLINE void IfxStdIf_DPipe_resetSendCount(IfxStdIf_DPipe *stdIf)
{
    stdIf->resetSendCount(stdIf->driver);
}


IFX_EXTERN void IfxStdIf_DPipe_print(IfxStdIf_DPipe *stdIf, pchar format, ...);

/** \} */
//----------------------------------------------------------------------------------------

#endif /* STDIF_DPIPE_H_ */
