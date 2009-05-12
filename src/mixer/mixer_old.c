/**
 * \file mixer/mixer.c
 * \brief Mixer Interface
 * \author Jaroslav Kysela <perex@perex.cz>
 * \author Abramo Bagnara <abramo@alsa-project.org>
 * \date 2001
 *
 * Old (v1) mixer interface is designed to access mixer elements.
 * Callbacks may be used for event handling.
 */
/*
 *  Mixer Interface - main file
 *  Copyright (c) 1998/1999/2000 by Jaroslav Kysela <perex@perex.cz>
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

/*! \page mixer Mixer interface

<P>Mixer interface is designed to access the abstracted mixer controls.
This is an abstraction layer over the hcontrol layer.

\section mixer_general_overview General overview

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "mixer_old_local.h"

/**
 * \brief Opens an empty mixer
 * \param mixerp Returned mixer handle
 * \param mode Open mode
 * \return 0 on success otherwise a negative error code
 */
int snd_mixer_open(snd_mixer_t **mixerp, int mode ATTRIBUTE_UNUSED)
{
	snd_mixer_t *mixer;
	assert(mixerp);
	mixer = calloc(1, sizeof(*mixer));
	if (mixer == NULL)
		return -ENOMEM;
	*mixerp = mixer;
	return 0;
}

/**
 * \brief Attach a HCTL to midxer
 * \param mixer Mixer handle
 * \param name the HCTL device name
 * \return 0 on success otherwise a negative error code
 */
int snd_mixer_attach(snd_mixer_t *mixer, const char *name)
{
	assert(mixer);
	assert(name);
	if (mixer->amixer)
		return -EBUSY;
	return snd_amixer_open(&mixer->amixer, name, NULL, NULL, SND_AMIXER_COMPAT1);
}

/**
 * \brief Attach an HCTL to an opened mixer
 * \param mixer Mixer handle
 * \param hctl the HCTL to be attached
 * \return 0 on success otherwise a negative error code
 */
int snd_mixer_attach_hctl(snd_mixer_t *mixer, snd_hctl_t *hctl)
{
	return -ENXIO;
}

/**
 * \brief Detach a previously attached HCTL to an opened mixer freeing all related resources
 * \param mixer Mixer handle
 * \param name HCTL previously attached
 * \return 0 on success otherwise a negative error code
 */
int snd_mixer_detach(snd_mixer_t *mixer, const char *name)
{
	return -ENXIO;
}

/**
 * \brief Detach a previously attached HCTL to an opened mixer freeing all related resources
 * \param mixer Mixer handle
 * \param hctl HCTL previously attached
 * \return 0 on success otherwise a negative error code
 *
 * Note: The hctl handle is not closed!
 */
int snd_mixer_detach_hctl(snd_mixer_t *mixer, snd_hctl_t *hctl)
{
	return -ENXIO;
}

/**
 * \brief Obtain a HCTL pointer associated to given name
 * \param mixer Mixer handle
 * \param name HCTL previously attached
 * \param hctl HCTL pointer
 * \return 0 on success otherwise a negative error code
 */
int snd_mixer_get_hctl(snd_mixer_t *mixer, const char *name, snd_hctl_t **hctl)
{
	return -ENXIO;
}

/**
 * \brief Get private data associated to give mixer element
 * \param elem Mixer element
 * \return private data
 *
 * For use by mixer element class specific code.
 */
void *snd_mixer_elem_get_private(const snd_mixer_elem_t *elem)
{
	return snd_amixer_elem_get_private(elem);
}

/**
 * \brief Load a mixer elements
 * \param mixer Mixer handle
 * \return 0 on success otherwise a negative error code
 */
int snd_mixer_load(snd_mixer_t *mixer)
{
	return 0;
}

/**
 * \brief Unload all mixer elements and free all related resources
 * \param mixer Mixer handle
 */
void snd_mixer_free(snd_mixer_t *mixer)
{
}

/**
 * \brief Close a mixer and free all related resources
 * \param mixer Mixer handle
 * \return 0 on success otherwise a negative error code
 */
int snd_mixer_close(snd_mixer_t *mixer)
{
	int res;
	
	if (mixer->amixer)
		res = snd_amixer_close(mixer->amixer);
	free(mixer);
	return 0;
}

/**
 * \brief Change mixer compare function and reorder elements
 * \param mixer Mixer handle
 * \param compare Element compare function
 * \return 0 on success otherwise a negative error code
 */
int snd_mixer_set_compare(snd_mixer_t *mixer, snd_mixer_compare_t compare)
{
	return snd_amixer_set_compare(mixer->amixer, compare);
}

/**
 * \brief get count of poll descriptors for mixer handle
 * \param mixer Mixer handle
 * \return count of poll descriptors
 */
int snd_mixer_poll_descriptors_count(snd_mixer_t *mixer)
{
	return snd_amixer_poll_descriptors_count(mixer->amixer);
}

/**
 * \brief get poll descriptors
 * \param mixer Mixer handle
 * \param pfds array of poll descriptors
 * \param space space in the poll descriptor array
 * \return count of filled descriptors
 */
int snd_mixer_poll_descriptors(snd_mixer_t *mixer, struct pollfd *pfds, unsigned int space)
{
	return snd_amixer_poll_descriptors(mixer->amixer, pfds, space);
}

/**
 * \brief get returned events from poll descriptors
 * \param mixer Mixer handle
 * \param pfds array of poll descriptors
 * \param nfds count of poll descriptors
 * \param revents returned events
 * \return zero if success, otherwise a negative error code
 */
int snd_mixer_poll_descriptors_revents(snd_mixer_t *mixer, struct pollfd *pfds, unsigned int nfds, unsigned short *revents)
{
	return snd_amixer_poll_descriptors_revents(mixer->amixer, pfds, nfds, revents);
}

/**
 * \brief Wait for a mixer to become ready (i.e. at least one event pending)
 * \param mixer Mixer handle
 * \param timeout maximum time in milliseconds to wait
 * \return 0 otherwise a negative error code on failure
 */
int snd_mixer_wait(snd_mixer_t *mixer, int timeout)
{
	return snd_amixer_wait(mixer->amixer, timeout);
}

/**
 * \brief get first element for a mixer
 * \param mixer Mixer handle
 * \return pointer to first element
 */
snd_mixer_elem_t *snd_mixer_first_elem(snd_mixer_t *mixer)
{
	return snd_amixer_first_elem(mixer->amixer);
}

/**
 * \brief get last element for a mixer
 * \param mixer Mixer handle
 * \return pointer to last element
 */
snd_mixer_elem_t *snd_mixer_last_elem(snd_mixer_t *mixer)
{
	return snd_amixer_last_elem(mixer->amixer);
}

/**
 * \brief get next mixer element
 * \param elem mixer element
 * \return pointer to next element
 */
snd_mixer_elem_t *snd_mixer_elem_next(snd_mixer_elem_t *elem)
{
	return snd_amixer_elem_next(elem);
}

/**
 * \brief get previous mixer element
 * \param elem mixer element
 * \return pointer to previous element
 */
snd_mixer_elem_t *snd_mixer_elem_prev(snd_mixer_elem_t *elem)
{
	return snd_amixer_elem_prev(elem);
}

/**
 * \brief Handle pending mixer events invoking callbacks
 * \param mixer Mixer handle
 * \return Number of events that occured on success, otherwise a negative error code on failure
 */
int snd_mixer_handle_events(snd_mixer_t *mixer)
{
	return snd_amixer_handle_events(mixer->amixer);
}

static int snd_mixer_default_callback(snd_amixer_t *mixer,
				      unsigned int mask,
				      snd_mixer_elem_t *elem)
{
	snd_mixer_t *old = snd_amixer_get_callback_private(mixer);
	return old->callback(old, mask, elem);
}

/**
 * \brief Set callback function for a mixer
 * \param obj mixer handle
 * \param val callback function
 */
void snd_mixer_set_callback(snd_mixer_t *obj, snd_mixer_callback_t val)
{
	assert(obj);
	obj->callback = val;
	snd_amixer_set_callback(obj->amixer, snd_mixer_default_callback);
	snd_amixer_set_callback_private(obj->amixer, obj);
}

/**
 * \brief Set callback private value for a mixer
 * \param mixer mixer handle
 * \param val callback private value
 */
void snd_mixer_set_callback_private(snd_mixer_t *mixer, void * val)
{
	assert(mixer);
	mixer->callback_private = val;
}

/**
 * \brief Get callback private value for a mixer
 * \param mixer mixer handle
 * \return callback private value
 */
void * snd_mixer_get_callback_private(const snd_mixer_t *mixer)
{
	assert(mixer);
	return mixer->callback_private;
}

/**
 * \brief Get elements count for a mixer
 * \param mixer mixer handle
 * \return elements count
 */
unsigned int snd_mixer_get_count(const snd_mixer_t *mixer)
{
	return snd_amixer_get_count(mixer->amixer);
}

/**
 * \brief Set callback function for a mixer element
 * \param mixer mixer element
 * \param val callback function
 */
void snd_mixer_elem_set_callback(snd_mixer_elem_t *mixer, snd_mixer_elem_callback_t val)
{
	return snd_amixer_elem_set_callback(mixer, val);
}

/**
 * \brief Set callback private value for a mixer element
 * \param elem mixer element
 * \param val callback private value
 */
void snd_mixer_elem_set_callback_private(snd_mixer_elem_t *elem, void * val)
{
	return snd_amixer_elem_set_callback_private(elem, val);
}

/**
 * \brief Get callback private value for a mixer element
 * \param elem mixer element
 * \return callback private value
 */
void * snd_mixer_elem_get_callback_private(const snd_mixer_elem_t *elem)
{
	return snd_amixer_elem_get_callback_private(elem);
}

/**
 * \brief Get type for a mixer element
 * \param mixer mixer element
 * \return mixer element type
 */
snd_mixer_elem_type_t snd_mixer_elem_get_type(const snd_mixer_elem_t *mixer)
{
	assert(mixer);
	return SND_MIXER_ELEM_SIMPLE;
}


