/*!\file cppQueue.cpp
** \author SMFSW
** \copyright BSD 3-Clause License (c) 2017-2020, SMFSW
** \brief cppQueue handling library (designed on Arduino)
** \details cppQueue handling library (designed on Arduino)
**			This library was designed for Arduino, yet may be compiled without change with gcc for other purposes/targets
**/
/****************************************************************/
#include <string.h>
#include <stdlib.h>

#include "cppQueue.h"
/****************************************************************/


/*!	\brief Increment index
**	\details Increment buffer index \b pIdx rolling back to \b start when limit \b end is reached
**	\param [in,out] pIdx - pointer to index value
**	\param [in] end - counter upper limit value
**	\param [in] start - counter lower limit value
**/
static inline void __attribute__((nonnull, always_inline)) inc_idx(uint16_t * const pIdx, const uint16_t end, const uint16_t start)
{
	if (*pIdx < end - 1)	{ (*pIdx)++; }
	else					{ *pIdx = start; }
}

/*!	\brief Decrement index
**	\details Decrement buffer index \b pIdx rolling back to \b end when limit \b start is reached
**	\param [in,out] pIdx - pointer to index value
**	\param [in] end - counter upper limit value
**	\param [in] start - counter lower limit value
**/
static inline void __attribute__((nonnull, always_inline)) dec_idx(uint16_t * const pIdx, const uint16_t end, const uint16_t start)
{
	if (*pIdx > start)		{ (*pIdx)--; }
	else					{ *pIdx = end - 1; }
}


cppQueue::cppQueue(const uint16_t size_rec, const uint16_t nb_recs, const cppQueueType type, const bool overwrite)
{
	const uint32_t size = nb_recs * size_rec;

	rec_nb = nb_recs;
	rec_sz = size_rec;
	impl = type;
	ovw = overwrite;

	init = 0;

	//if (queue)	{ free(queue); }	// Free existing data (if any)
	queue = (uint8_t *) malloc(size);

	if (queue == NULL)	{ queue_sz = 0; return; }	// Return here if cppQueue not allocated
	else				{ queue_sz = size; }

	init = QUEUE_INITIALIZED;
	flush();
}

cppQueue::~cppQueue()
{
	if (init == QUEUE_INITIALIZED)	free(queue);
}


void cppQueue::flush(void)
{
	in = 0;
	out = 0;
	cnt = 0;
}


bool __attribute__((nonnull)) cppQueue::push(const void * const record)
{
	if ((!ovw) && isFull())	{ return false; }

	uint8_t * const pStart = queue + (rec_sz * in);
	memcpy(pStart, record, rec_sz);

	inc_idx(&in, rec_nb, 0);

	if (!isFull())	{ cnt++; }	// Increase records count
	else if (ovw)				// cppQueue is full and overwrite is allowed
	{
		if (impl == FIFO)			{ inc_idx(&out, rec_nb, 0); }	// as oldest record is overwritten, increment out
		//else if (impl == LIFO)	{}								// Nothing to do in this case
	}

	return true;
}

bool __attribute__((nonnull)) cppQueue::pop(void * const record)
{
	const uint8_t * pStart;

	if (isEmpty())	{ return false; }	// No more records

	if (impl == FIFO)
	{
		pStart = queue + (rec_sz * out);
		inc_idx(&out, rec_nb, 0);
	}
	else if (impl == LIFO)
	{
		dec_idx(&in, rec_nb, 0);
		pStart = queue + (rec_sz * in);
	}
	else	{ return false; }

	memcpy(record, pStart, rec_sz);
	cnt--;	// Decrease records count
	return true;
}


bool __attribute__((nonnull)) cppQueue::peek(void * const record)
{
	const uint8_t *	pStart;

	if (isEmpty())	{ return false; }	// No more records

	if (impl == FIFO)
	{
		pStart = queue + (rec_sz * out);
		// No change on out var as it's just a peek
	}
	else if (impl == LIFO)
	{
		uint16_t rec = in;	// Temporary var for peek (no change on in with dec_idx)
		dec_idx(&rec, rec_nb, 0);
		pStart = queue + (rec_sz * rec);
	}
	else	{ return false; }

	memcpy(record, pStart, rec_sz);
	return true;
}


bool cppQueue::drop(void)
{
	if (isEmpty())			{ return false; }	// No more records

	if (impl == FIFO)		{ inc_idx(&out, rec_nb, 0); }
	else if (impl == LIFO)	{ dec_idx(&in, rec_nb, 0); }
	else					{ return false; }

	cnt--;	// Decrease records count
	return true;
}


bool cppQueue::peekIdx(void * const record, const uint16_t idx)
{
	const uint8_t * pStart;

	if (idx + 1 > getCount())	{ return false; }	// Index out of range

	if (impl == FIFO)
	{
		pStart = queue + (rec_sz * ((out + idx) % rec_nb));
	}
	else if (impl == LIFO)
	{
		pStart = queue + (rec_sz * idx);
	}
	else	{ return false; }

	memcpy(record, pStart, rec_sz);
	return true;
}

