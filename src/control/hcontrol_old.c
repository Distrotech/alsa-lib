/**
 * \file control/hcontrol_old.c
 * \brief HCTL Interface - High Level CTL
 * \author Jaroslav Kysela <perex@perex.cz>
 * \author Abramo Bagnara <abramo@alsa-project.org>
 * \date 2000
 *
 * HCTL interface is designed to access preloaded and sorted primitive controls.
 * Callbacks may be used for event handling.
 * See \ref hcontrol page for more details.
 */
/*
 *  Control Interface - high level API
 *  Copyright (c) 2000 by Jaroslav Kysela <perex@perex.cz>
 *  Copyright (c) 2001 by Abramo Bagnara <abramo@alsa-project.org>
 *
 *
 *   This library is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as
 *   published by the Free Software Foundation; either version 2.1 of
 *   the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

/*! \page hcontrol High level control interface

<P> High level control interface is designed to access preloaded and sorted primitive controls.

\section hcontrol_general_overview General overview

<P> High level control interface caches the accesses to primitive controls
to reduce overhead accessing the real controls in kernel drivers.

*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/poll.h>
#include "control_local.h"
#include "mixer_old.h"

/**
 * \brief Opens an HCTL
 * \param hctlp Returned HCTL handle
 * \param name ASCII identifier of the underlying CTL handle
 * \param mode Open mode (see #SND_CTL_NONBLOCK, #SND_CTL_ASYNC)
 * \return 0 on success otherwise a negative error code
 */
int snd_hctl_open(snd_hctl_t **hctlp, const char *name, int mode ATTRIBUTE_UNUSED)
{
	return snd_ctl_open(hctlp, name, SND_CTL_CACHE);
}

/**
 * \brief Opens an HCTL
 * \param hctlp Returned HCTL handle
 * \param ctl underlying CTL handle
 * \return 0 on success otherwise a negative error code
 */
int snd_hctl_open_ctl(snd_hctl_t **hctlp, snd_ctl_t *ctl)
{
	assert(hctlp);
	assert(ctl);
	*hctlp = ctl;
	ctl->mode = SND_CTL_CACHE;
	return 0;
}

/**
 * \brief close HCTL handle
 * \param hctl HCTL handle
 * \return 0 on success otherwise a negative error code
 *
 * Closes the specified HCTL handle and frees all associated
 * resources.
 */
int snd_hctl_close(snd_hctl_t *hctl)
{
	return snd_ctl_close(hctl);
}

/**
 * \brief get identifier of HCTL handle
 * \param hctl HCTL handle
 * \return ascii identifier of HCTL handle
 *
 * Returns the ASCII identifier of given HCTL handle. It's the same
 * identifier specified in snd_hctl_open().
 */
const char *snd_hctl_name(snd_hctl_t *hctl)
{
	assert(hctl);
	return snd_ctl_name(hctl);
}

/**
 * \brief set nonblock mode
 * \param hctl HCTL handle
 * \param nonblock 0 = block, 1 = nonblock mode
 * \return 0 on success otherwise a negative error code
 */
int snd_hctl_nonblock(snd_hctl_t *hctl, int nonblock)
{
	assert(hctl);
	return snd_ctl_nonblock(hctl, nonblock);
}

/**
 * \brief set async mode
 * \param hctl HCTL handle
 * \param sig Signal to raise: < 0 disable, 0 default (SIGIO)
 * \param pid Process ID to signal: 0 current
 * \return 0 on success otherwise a negative error code
 *
 * A signal is raised when a change happens.
 */
int snd_hctl_async(snd_hctl_t *hctl, int sig, pid_t pid)
{
	assert(hctl);
	return snd_ctl_async(hctl, sig, pid);
}

/**
 * \brief get count of poll descriptors for HCTL handle
 * \param hctl HCTL handle
 * \return count of poll descriptors
 */
int snd_hctl_poll_descriptors_count(snd_hctl_t *hctl)
{
	assert(hctl);
	return snd_ctl_poll_descriptors_count(hctl);
}

/**
 * \brief get poll descriptors
 * \param hctl HCTL handle
 * \param pfds array of poll descriptors
 * \param space space in the poll descriptor array
 * \return count of filled descriptors
 */
int snd_hctl_poll_descriptors(snd_hctl_t *hctl, struct pollfd *pfds, unsigned int space)
{
	assert(hctl);
	return snd_ctl_poll_descriptors(hctl, pfds, space);
}

/**
 * \brief get returned events from poll descriptors
 * \param hctl HCTL handle
 * \param pfds array of poll descriptors
 * \param nfds count of poll descriptors
 * \param revents returned events
 * \return zero if success, otherwise a negative error code
 */
int snd_hctl_poll_descriptors_revents(snd_hctl_t *hctl, struct pollfd *pfds, unsigned int nfds, unsigned short *revents)
{
	assert(hctl);
	return snd_ctl_poll_descriptors_revents(hctl, pfds, nfds, revents);
}

/**
 * \brief free HCTL loaded elements
 * \param hctl HCTL handle
 * \return 0 on success otherwise a negative error code
 */
int snd_hctl_free(snd_hctl_t *hctl)
{
	return snd_ctl_cache_free(hctl);
}

/**
 * \brief Change HCTL compare function and reorder elements
 * \param hctl HCTL handle
 * \param compare Element compare function
 * \return 0 on success otherwise a negative error code
 */
int snd_hctl_set_compare(snd_hctl_t *hctl, snd_hctl_compare_t compare)
{
	return snd_ctl_set_compare(hctl, compare);
}

/**
 * \brief get first element for an HCTL
 * \param hctl HCTL handle
 * \return pointer to first element
 */
snd_hctl_elem_t *snd_hctl_first_elem(snd_hctl_t *hctl)
{
	return snd_ctl_first_elem(hctl);
}

/**
 * \brief get last element for an HCTL
 * \param hctl HCTL handle
 * \return pointer to last element
 */
snd_hctl_elem_t *snd_hctl_last_elem(snd_hctl_t *hctl)
{
	return snd_ctl_last_elem(hctl);
}

/**
 * \brief get next HCTL element
 * \param elem HCTL element
 * \return pointer to next element
 */
snd_hctl_elem_t *snd_hctl_elem_next(snd_hctl_elem_t *elem)
{
	return snd_ctl_elem_next(elem);
}

/**
 * \brief get previous HCTL element
 * \param elem HCTL element
 * \return pointer to previous element
 */
snd_hctl_elem_t *snd_hctl_elem_prev(snd_hctl_elem_t *elem)
{
	return snd_ctl_elem_prev(elem);
}

/**
 * \brief Search an HCTL element
 * \param hctl HCTL handle
 * \param id Element identifier
 * \return pointer to found HCTL element or NULL if it does not exists
 */
snd_hctl_elem_t *snd_hctl_find_elem(snd_hctl_t *hctl, const snd_ctl_elem_id_t *id)
{
	return snd_ctl_find_elem(hctl, id);
}

/**
 * \brief Load an HCTL with all elements and sort them
 * \param hctl HCTL handle
 * \return 0 on success otherwise a negative error code
 */
int snd_hctl_load(snd_hctl_t *hctl)
{
	return snd_ctl_subscribe_events(hctl, 1);
}

/**
 * \brief Set callback function for an HCTL
 * \param hctl HCTL handle
 * \param callback callback function
 */
void snd_hctl_set_callback(snd_hctl_t *hctl, snd_hctl_callback_t callback)
{
	snd_ctl_set_callback(hctl, callback);
}

/**
 * \brief Set callback private value for an HCTL
 * \param hctl HCTL handle
 * \param callback_private callback private value
 */
void snd_hctl_set_callback_private(snd_hctl_t *hctl, void *callback_private)
{
	snd_ctl_set_callback_private(hctl, callback_private);
}

/**
 * \brief Get callback private value for an HCTL
 * \param hctl HCTL handle
 * \return callback private value
 */
void *snd_hctl_get_callback_private(snd_hctl_t *hctl)
{
	return snd_ctl_get_callback_private(hctl);
}

/**
 * \brief Get number of loaded elements for an HCTL
 * \param hctl HCTL handle
 * \return elements count
 */
unsigned int snd_hctl_get_count(snd_hctl_t *hctl)
{
	return snd_ctl_get_count(hctl);
}

/**
 * \brief Wait for a HCTL to become ready (i.e. at least one event pending)
 * \param hctl HCTL handle
 * \param timeout maximum time in milliseconds to wait
 * \return a positive value on success otherwise a negative error code
 * \retval 0 timeout occurred
 * \retval 1 an event is pending
 */
int snd_hctl_wait(snd_hctl_t *hctl, int timeout)
{
	return snd_ctl_wait(hctl, timeout);
}

/**
 * \brief Get a ctl handle associated to the given hctl handle
 * \param hctl HCTL handle
 * \return a ctl handle otherwise NULL
 */
snd_ctl_t *snd_hctl_ctl(snd_hctl_t *hctl)
{
	return hctl;
}

/**
 * \brief Handle pending HCTL events invoking callbacks
 * \param hctl HCTL handle
 * \return 0 otherwise a negative error code on failure
 */
int snd_hctl_handle_events(snd_hctl_t *hctl)
{
	return snd_ctl_handle_events(hctl);
}

/**
 * \brief Get information for an HCTL element
 * \param elem HCTL element
 * \param info HCTL element information
 * \return 0 otherwise a negative error code on failure
 */
int snd_hctl_elem_info(snd_hctl_elem_t *elem, snd_ctl_elem_info_t *info)
{
	return snd_ctl_celem_info(elem, info);
}

/**
 * \brief Get value for an HCTL element
 * \param elem HCTL element
 * \param value HCTL element value
 * \return 0 otherwise a negative error code on failure
 */
int snd_hctl_elem_read(snd_hctl_elem_t *elem, snd_ctl_elem_value_t * value)
{
	return snd_ctl_celem_read(elem, value);
}

/**
 * \brief Set value for an HCTL element
 * \param elem HCTL element
 * \param value HCTL element value
 * \retval 0 on success
 * \retval >1 on success when value was changed
 * \retval <0 a negative error code on failure
 */
int snd_hctl_elem_write(snd_hctl_elem_t *elem, snd_ctl_elem_value_t * value)
{
	return snd_ctl_celem_write(elem, value);
}

/**
 * \brief Get TLV value for an HCTL element
 * \param elem HCTL element
 * \param tlv TLV array for value
 * \param tlv_size size of TLV array in bytes
 * \return 0 otherwise a negative error code on failure
 */
int snd_hctl_elem_tlv_read(snd_hctl_elem_t *elem, unsigned int *tlv, unsigned int tlv_size)
{
	return snd_ctl_celem_tlv_read(elem, tlv, tlv_size);
}

/**
 * \brief Set TLV value for an HCTL element
 * \param elem HCTL element
 * \param tlv TLV array for value
 * \retval 0 on success
 * \retval >1 on success when value was changed
 * \retval <0 a negative error code on failure
 */
int snd_hctl_elem_tlv_write(snd_hctl_elem_t *elem, const unsigned int *tlv)
{
	return snd_ctl_celem_tlv_write(elem, tlv);
}

/**
 * \brief Set TLV value for an HCTL element
 * \param elem HCTL element
 * \param tlv TLV array for value
 * \retval 0 on success
 * \retval >1 on success when value was changed
 * \retval <0 a negative error code on failure
 */
int snd_hctl_elem_tlv_command(snd_hctl_elem_t *elem, const unsigned int *tlv)
{
	return snd_ctl_celem_tlv_command(elem, tlv);
}

/**
 * \brief Get HCTL handle for an HCTL element
 * \param elem HCTL element
 * \return HCTL handle
 */
snd_hctl_t *snd_hctl_elem_get_hctl(snd_hctl_elem_t *elem)
{
	return snd_ctl_elem_get_ctl(elem);
}

/**
 * \brief Get CTL element identifier of a CTL element id/value
 * \param obj CTL element id/value
 * \param ptr Pointer to returned CTL element identifier
 */
void snd_hctl_elem_get_id(const snd_hctl_elem_t *obj, snd_ctl_elem_id_t *ptr)
{
	snd_ctl_elem_get_id(obj, ptr);
}

/**
 * \brief Get element numeric identifier of a CTL element id/value
 * \param obj CTL element id/value
 * \return element numeric identifier
 */
unsigned int snd_hctl_elem_get_numid(const snd_hctl_elem_t *obj)
{
	return snd_ctl_elem_get_numid(obj);
}

/**
 * \brief Get interface part of CTL element identifier of a CTL element id/value
 * \param obj CTL element id/value
 * \return interface part of element identifier
 */
snd_ctl_elem_iface_t snd_hctl_elem_get_interface(const snd_hctl_elem_t *obj)
{
return snd_ctl_elem_get_interface(obj);
}

/**
 * \brief Get device part of CTL element identifier of a CTL element id/value
 * \param obj CTL element id/value
 * \return device part of element identifier
 */
unsigned int snd_hctl_elem_get_device(const snd_hctl_elem_t *obj)
{
	return snd_ctl_elem_get_device(obj);
}

/**
 * \brief Get subdevice part of CTL element identifier of a CTL element id/value
 * \param obj CTL element id/value
 * \return subdevice part of element identifier
 */
unsigned int snd_hctl_elem_get_subdevice(const snd_hctl_elem_t *obj)
{
	return snd_ctl_elem_get_subdevice(obj);
}

/**
 * \brief Get name part of CTL element identifier of a CTL element id/value
 * \param obj CTL element id/value
 * \return name part of element identifier
 */
const char *snd_hctl_elem_get_name(const snd_hctl_elem_t *obj)
{
	return snd_ctl_elem_get_name(obj);
}

/**
 * \brief Get index part of CTL element identifier of a CTL element id/value
 * \param obj CTL element id/value
 * \return index part of element identifier
 */
unsigned int snd_hctl_elem_get_index(const snd_hctl_elem_t *obj)
{
	return snd_ctl_elem_get_index(obj);
}

/**
 * \brief Set callback function for an HCTL element
 * \param obj HCTL element
 * \param val callback function
 */
void snd_hctl_elem_set_callback(snd_hctl_elem_t *obj, snd_hctl_elem_callback_t val)
{
	return snd_ctl_elem_set_callback(obj, val);
}

/**
 * \brief Set callback private value for an HCTL element
 * \param obj HCTL element
 * \param val callback private value
 */
void snd_hctl_elem_set_callback_private(snd_hctl_elem_t *obj, void * val)
{
	return snd_ctl_elem_set_callback_private(obj, val);
}

/**
 * \brief Get callback private value for an HCTL element
 * \param obj HCTL element
 * \return callback private value
 */
void * snd_hctl_elem_get_callback_private(const snd_hctl_elem_t *obj)
{
	return snd_ctl_elem_get_callback_private(obj);
}

