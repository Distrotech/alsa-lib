/**
 * \file control/hcontrol.c
 * \brief CTL Interface - High Level Cached Control Elements
 * \author Jaroslav Kysela <perex@perex.cz>
 * \author Abramo Bagnara <abramo@alsa-project.org>
 * \date 2000
 *
 * CTL interface is designed to access preloaded and sorted primitive controls.
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

/*! \page hcontrol High level cached control interface

<P> High level control interface is designed to access preloaded and sorted primitive controls.

\section hcontrol_general_overview General overview

<P> High level control interface caches the accesses to primitive controls
to reduce overhead accessing the real controls in kernel drivers.

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#ifndef DOC_HIDDEN
#define __USE_GNU
#endif
#include "control_local.h"
#ifdef HAVE_LIBPTHREAD
#include <pthread.h>
#endif

#ifndef DOC_HIDDEN
#define NOT_FOUND 1000000000
#endif

static int snd_ctl_compare_default(const snd_ctl_elem_t *c1,
				   const snd_ctl_elem_t *c2);

static int snd_ctl_throw_event(snd_ctl_t *ctl, unsigned int mask,
			       snd_ctl_elem_t *elem)
{
	if (ctl->callback)
		return ctl->callback(ctl, mask, elem);
	return 0;
}

static int snd_ctl_elem_throw_event(snd_ctl_elem_t *elem,
				    unsigned int mask)
{
	if (elem->callback)
		return elem->callback(elem, mask);
	return 0;
}

static int snd_ctl_compare_mixer_priority_lookup(const char **name, const char * const *names, int coef)
{
	int res;

	for (res = 0; *names; names++, res += coef) {
		if (!strncmp(*name, *names, strlen(*names))) {
			*name += strlen(*names);
			if (**name == ' ')
				(*name)++;
			return res+1;
		}
	}
	return NOT_FOUND;
}

static int get_compare_weight(const snd_ctl_elem_id_t *id)
{
	static const char *const names[] = {
		"Master",
		"Hardware Master",
		"Headphone",
		"Tone Control",
		"3D Control",
		"PCM",
		"Front",
		"Surround",
		"Center",
		"LFE",
		"Synth",
		"FM",
		"Wave",
		"Music",
		"DSP",
		"Line",
		"CD",
		"Mic",
		"Phone",
		"Video",
		"Zoom Video",
		"PC Speaker",
		"Aux",
		"Mono",
		"ADC",
		"Capture Source",
		"Capture",
		"Playback",
		"Loopback",
		"Analog Loopback",
		"Digital Loopback",
		"I2S",
		"IEC958",
		NULL
	};
	static const char *const names1[] = {
		"Switch",
		"Volume",
		"Playback",
		"Capture",
		"Bypass",
		"Mono",
		"Front",
		"Rear",
		"Pan",
		"Output",
		"-",
		NULL
	};
	static const char *const names2[] = {
		"Switch",
		"Volume",
		"Bypass",
		"Depth",
		"Wide",
		"Space",
		"Level",
		"Center",
		NULL
	};
	const char *name = (char *)id->name, *name1;
	int res, res1;
	
	if ((res = snd_ctl_compare_mixer_priority_lookup((const char **)&name, names, 1000000)) == NOT_FOUND)
		return NOT_FOUND;
	if (*name == '\0')
		return res;
	for (name1 = name; *name1 != '\0'; name1++);
	for (name1--; name1 != name && *name1 != ' '; name1--);
	while (name1 != name && *name1 == ' ')
		name1--;
	if (name1 != name) {
		for (; name1 != name && *name1 != ' '; name1--);
		name = name1;
		if ((res1 = snd_ctl_compare_mixer_priority_lookup((const char **)&name, names1, 1000)) == NOT_FOUND)
			return res;
		res += res1;
	} else {
		name = name1;
	}
	if ((res1 = snd_ctl_compare_mixer_priority_lookup((const char **)&name, names2, 1)) == NOT_FOUND)
		return res;
	return res + res1;
}

static int _snd_ctl_find_elem(snd_ctl_t *ctl, const snd_ctl_elem_id_t *id, int *dir)
{
	unsigned int l, u;
	snd_ctl_elem_t el;
	int c = 0;
	int idx = -1;
	assert(ctl && id);
	assert(ctl->compare);
	el.id = *id;
	el.compare_weight = get_compare_weight(id);
	l = 0;
	u = ctl->count;
	while (l < u) {
		idx = (l + u) / 2;
		c = ctl->compare(&el, ctl->pelems[idx]);
		if (c < 0)
			u = idx;
		else if (c > 0)
			l = idx + 1;
		else
			break;
	}
	*dir = c;
	return idx;
}

static int snd_ctl_elem_add(snd_ctl_t *ctl, snd_ctl_elem_t *elem)
{
	int dir;
	int idx; 
	elem->compare_weight = get_compare_weight(&elem->id);
	if (ctl->count == ctl->alloc) {
		snd_ctl_elem_t **h;
		ctl->alloc += 32;
		h = realloc(ctl->pelems, sizeof(*h) * ctl->alloc);
		if (!h) {
			ctl->alloc -= 32;
			return -ENOMEM;
		}
		ctl->pelems = h;
	}
	if (ctl->count == 0) {
		list_add_tail(&elem->list, &ctl->elems);
		ctl->pelems[0] = elem;
	} else {
		idx = _snd_ctl_find_elem(ctl, &elem->id, &dir);
		assert(dir != 0);
		if (dir > 0) {
			list_add(&elem->list, &ctl->pelems[idx]->list);
			idx++;
		} else {
			list_add_tail(&elem->list, &ctl->pelems[idx]->list);
		}
		memmove(ctl->pelems + idx + 1,
			ctl->pelems + idx,
			(ctl->count - idx) * sizeof(snd_ctl_elem_t *));
		ctl->pelems[idx] = elem;
	}
	ctl->count++;
	return snd_ctl_throw_event(ctl, SNDRV_CTL_EVENT_MASK_ADD, elem);
}

static void snd_ctl_celem_remove(snd_ctl_t *ctl, unsigned int idx)
{
	snd_ctl_elem_t *elem = ctl->pelems[idx];
	unsigned int m;
	snd_ctl_elem_throw_event(elem, SNDRV_CTL_EVENT_MASK_REMOVE);
	list_del(&elem->list);
	free(elem);
	ctl->count--;
	m = ctl->count - idx;
	if (m > 0)
		memmove(ctl->pelems + idx,
			ctl->pelems + idx + 1,
			m * sizeof(snd_ctl_elem_t *));
}

#ifndef DOC_HIDDEN
/**
 * \brief free cached elements
 * \param ctl CTL handle
 * \return 0 on success otherwise a negative error code
 */
int snd_ctl_cache_free(snd_ctl_t *ctl)
{
	while (ctl->count > 0)
		snd_ctl_celem_remove(ctl, ctl->count - 1);
	if (ctl->pelems)
		free(ctl->pelems);
	ctl->pelems = 0;
	ctl->alloc = 0;
	INIT_LIST_HEAD(&ctl->elems);
	return 0;
}
#endif

static snd_ctl_t *compare_ctl;
static int ctl_compare(const void *a, const void *b) {
	return compare_ctl->compare(*(const snd_ctl_elem_t * const *) a,
				    *(const snd_ctl_elem_t * const *) b);
}

static void snd_ctl_sort(snd_ctl_t *ctl)
{
	unsigned int k;
#ifdef HAVE_LIBPTHREAD
	static pthread_mutex_t sync_lock = PTHREAD_MUTEX_INITIALIZER;
#endif

	assert(ctl);
	assert(ctl->compare);
	INIT_LIST_HEAD(&ctl->elems);

#ifdef HAVE_LIBPTHREAD
	pthread_mutex_lock(&sync_lock);
#endif
	compare_ctl = ctl;
	qsort(ctl->pelems, ctl->count, sizeof(*ctl->pelems), ctl_compare);
#ifdef HAVE_LIBPTHREAD
	pthread_mutex_unlock(&sync_lock);
#endif
	for (k = 0; k < ctl->count; k++)
		list_add_tail(&ctl->pelems[k]->list, &ctl->elems);
}

/**
 * \brief Change CTL compare function and reorder elements
 * \param ctl CTL handle
 * \param compare Element compare function
 * \return 0 on success otherwise a negative error code
 */
int snd_ctl_set_compare(snd_ctl_t *ctl, snd_ctl_compare_t compare)
{
	assert(ctl);
	ctl->compare = compare == NULL ? snd_ctl_compare_default : compare;
	snd_ctl_sort(ctl);
	return 0;
}

/**
 * \brief A "don't care" fast compare functions that may be used with #snd_ctl_set_compare
 * \param c1 First CTL element
 * \param c2 Second CTL element
 * \return -1 if c1 < c2, 0 if c1 == c2, 1 if c1 > c2
 */
int snd_ctl_compare_fast(const snd_ctl_elem_t *c1,
			 const snd_ctl_elem_t *c2)
{
	return c1->id.numid - c2->id.numid;
}

static int snd_ctl_compare_default(const snd_ctl_elem_t *c1,
				    const snd_ctl_elem_t *c2)
{
	int res;
	int d = c1->id.iface - c2->id.iface;
	if (d != 0)
		return d;
	if (c1->id.iface == SNDRV_CTL_ELEM_IFACE_MIXER) {
		d = c1->compare_weight - c2->compare_weight;
		if (d != 0)
			return d;
	}
	res = strcmp((const char *)c1->id.name, (const char *)c2->id.name);
	if (res != 0)
		return res;
	d = c1->id.index - c2->id.index;
	return d;
}

/**
 * \brief get first element for an CTL
 * \param ctl CTL handle
 * \return pointer to first element
 */
snd_ctl_elem_t *snd_ctl_first_elem(snd_ctl_t *ctl)
{
	assert(ctl);
	if (list_empty(&ctl->elems))
		return NULL;
	return list_entry(ctl->elems.next, snd_ctl_elem_t, list);
}

/**
 * \brief get last element for an CTL
 * \param ctl CTL handle
 * \return pointer to last element
 */
snd_ctl_elem_t *snd_ctl_last_elem(snd_ctl_t *ctl)
{
	assert(ctl);
	if (list_empty(&ctl->elems))
		return NULL;
	return list_entry(ctl->elems.prev, snd_ctl_elem_t, list);
}

/**
 * \brief get next CTL element
 * \param elem CTL element
 * \return pointer to next element
 */
snd_ctl_elem_t *snd_ctl_elem_next(snd_ctl_elem_t *elem)
{
	assert(elem);
	if (elem->list.next == &elem->ctl->elems)
		return NULL;
	return list_entry(elem->list.next, snd_ctl_elem_t, list);
}

/**
 * \brief get previous CTL element
 * \param elem CTL element
 * \return pointer to previous element
 */
snd_ctl_elem_t *snd_ctl_elem_prev(snd_ctl_elem_t *elem)
{
	assert(elem);
	if (elem->list.prev == &elem->ctl->elems)
		return NULL;
	return list_entry(elem->list.prev, snd_ctl_elem_t, list);
}

/**
 * \brief Search an CTL element
 * \param ctl CTL handle
 * \param id Element identifier
 * \return pointer to found CTL element or NULL if it does not exists
 */
snd_ctl_elem_t *snd_ctl_find_elem(snd_ctl_t *ctl, const snd_ctl_elem_id_t *id)
{
	int dir;
	int res = _snd_ctl_find_elem(ctl, id, &dir);
	if (res < 0 || dir != 0)
		return NULL;
	return ctl->pelems[res];
}

#ifndef DOC_HIDDEN
/**
 * \brief Load an CTL with all elements and sort them
 * \param ctl CTL handle
 * \return 0 on success otherwise a negative error code
 */
int snd_ctl_cache_load(snd_ctl_t *ctl)
{
	snd_ctl_elem_list_t list;
	int err = 0;
	unsigned int idx;

	assert(ctl);
	assert(ctl->count == 0);
	assert(list_empty(&ctl->elems));
	memset(&list, 0, sizeof(list));
	if ((err = snd_ctl_elem_list(ctl, &list)) < 0)
		goto _end;
	while (list.count != list.used) {
		if (list.space)
			snd_ctl_elem_list_free_space(&list);
		err = snd_ctl_elem_list_alloc_space(&list, list.count);
		if (err < 0)
			goto _end;
		if ((err = snd_ctl_elem_list(ctl, &list)) < 0)
			goto _end;
	}
	if (ctl->alloc < list.count) {
		ctl->alloc = list.count;
		free(ctl->pelems);
		ctl->pelems = malloc(ctl->alloc * sizeof(*ctl->pelems));
		if (!ctl->pelems) {
			err = -ENOMEM;
			goto _end;
		}
	}
	for (idx = 0; idx < list.count; idx++) {
		snd_ctl_elem_t *elem;
		elem = calloc(1, sizeof(snd_ctl_elem_t));
		if (elem == NULL) {
			snd_ctl_cache_free(ctl);
			err = -ENOMEM;
			goto _end;
		}
		elem->id = list.pids[idx];
		elem->ctl = ctl;
		elem->compare_weight = get_compare_weight(&elem->id);
		ctl->pelems[idx] = elem;
		list_add_tail(&elem->list, &ctl->elems);
		ctl->count++;
	}
	if (!ctl->compare)
		ctl->compare = snd_ctl_compare_default;
	snd_ctl_sort(ctl);
	for (idx = 0; idx < ctl->count; idx++) {
		int res = snd_ctl_throw_event(ctl, SNDRV_CTL_EVENT_MASK_ADD,
					       ctl->pelems[idx]);
		if (res < 0)
			return res;
	}
 _end:
 	snd_ctl_elem_list_free_space(&list);
	return err;
}
#endif

/**
 * \brief Set callback function for an CTL
 * \param ctl CTL handle
 * \param callback callback function
 */
void snd_ctl_set_callback(snd_ctl_t *ctl, snd_ctl_callback_t callback)
{
	assert(ctl);
	ctl->callback = callback;
}

/**
 * \brief Set callback private value for an CTL
 * \param ctl CTL handle
 * \param callback_private callback private value
 */
void snd_ctl_set_callback_private(snd_ctl_t *ctl, void *callback_private)
{
	assert(ctl);
	ctl->callback_private = callback_private;
}

/**
 * \brief Get callback private value for an CTL
 * \param ctl CTL handle
 * \return callback private value
 */
void *snd_ctl_get_callback_private(snd_ctl_t *ctl)
{
	assert(ctl);
	return ctl->callback_private;
}

/**
 * \brief Get number of loaded elements for an CTL
 * \param ctl CTL handle
 * \return elements count
 */
unsigned int snd_ctl_get_count(snd_ctl_t *ctl)
{
	return ctl->count;
}

static int snd_ctl_handle_event(snd_ctl_t *ctl, snd_ctl_event_t *event)
{
	snd_ctl_elem_t *elem;
	int res;

	assert(ctl);
	switch (event->type) {
	case SND_CTL_EVENT_ELEM:
		break;
	default:
		return 0;
	}
	if (event->data.elem.mask == SNDRV_CTL_EVENT_MASK_REMOVE) {
		int dir;
		res = _snd_ctl_find_elem(ctl, &event->data.elem.id, &dir);
		assert(res >= 0 && dir == 0);
		if (res < 0 || dir != 0)
			return -ENOENT;
		snd_ctl_celem_remove(ctl, (unsigned int) res);
		return 0;
	}
	if (event->data.elem.mask & SNDRV_CTL_EVENT_MASK_ADD) {
		elem = calloc(1, sizeof(snd_ctl_elem_t));
		if (elem == NULL)
			return -ENOMEM;
		elem->id = event->data.elem.id;
		elem->ctl = ctl;
		res = snd_ctl_elem_add(ctl, elem);
		if (res < 0)
			return res;
	}
	if (event->data.elem.mask & (SNDRV_CTL_EVENT_MASK_VALUE |
				     SNDRV_CTL_EVENT_MASK_INFO)) {
		elem = snd_ctl_find_elem(ctl, &event->data.elem.id);
		assert(elem);
		if (!elem)
			return -ENOENT;
		res = snd_ctl_elem_throw_event(elem, event->data.elem.mask &
						(SNDRV_CTL_EVENT_MASK_VALUE |
						 SNDRV_CTL_EVENT_MASK_INFO));
		if (res < 0)
			return res;
	}
	return 0;
}

/**
 * \brief Handle pending CTL events invoking callbacks
 * \param ctl CTL handle
 * \return 0 otherwise a negative error code on failure
 */
int snd_ctl_handle_events(snd_ctl_t *ctl)
{
	snd_ctl_event_t event;
	int res;
	unsigned int count = 0;
	
	assert(ctl);
	while ((res = snd_ctl_read(ctl, &event)) != 0 &&
	       res != -EAGAIN) {
		if (res < 0)
			return res;
		res = snd_ctl_handle_event(ctl, &event);
		if (res < 0)
			return res;
		count++;
	}
	return count;
}

/**
 * \brief Get information for an CTL element
 * \param elem CTL element
 * \param info CTL element information
 * \return 0 otherwise a negative error code on failure
 */
int snd_ctl_celem_info(snd_ctl_elem_t *elem, snd_ctl_elem_info_t *info)
{
	assert(elem);
	assert(elem->ctl);
	assert(info);
	info->id = elem->id;
	return snd_ctl_elem_info(elem->ctl, info);
}

/**
 * \brief Get value for an CTL element
 * \param elem CTL element
 * \param value CTL element value
 * \return 0 otherwise a negative error code on failure
 */
int snd_ctl_celem_read(snd_ctl_elem_t *elem, snd_ctl_elem_value_t * value)
{
	assert(elem);
	assert(elem->ctl);
	assert(value);
	value->id = elem->id;
	return snd_ctl_elem_read(elem->ctl, value);
}

/**
 * \brief Set value for an CTL element
 * \param elem CTL element
 * \param value CTL element value
 * \retval 0 on success
 * \retval >1 on success when value was changed
 * \retval <0 a negative error code on failure
 */
int snd_ctl_celem_write(snd_ctl_elem_t *elem, snd_ctl_elem_value_t * value)
{
	assert(elem);
	assert(elem->ctl);
	assert(value);
	value->id = elem->id;
	return snd_ctl_elem_write(elem->ctl, value);
}

/**
 * \brief Get TLV value for an CTL element
 * \param elem CTL element
 * \param tlv TLV array for value
 * \param tlv_size size of TLV array in bytes
 * \return 0 otherwise a negative error code on failure
 */
int snd_ctl_celem_tlv_read(snd_ctl_elem_t *elem, unsigned int *tlv, unsigned int tlv_size)
{
	assert(elem);
	assert(tlv);
	assert(tlv_size >= 12);
	return snd_ctl_elem_tlv_read(elem->ctl, &elem->id, tlv, tlv_size);
}

/**
 * \brief Set TLV value for an CTL element
 * \param elem CTL element
 * \param tlv TLV array for value
 * \retval 0 on success
 * \retval >1 on success when value was changed
 * \retval <0 a negative error code on failure
 */
int snd_ctl_celem_tlv_write(snd_ctl_elem_t *elem, const unsigned int *tlv)
{
	assert(elem);
	assert(tlv);
	assert(tlv[1] >= 4);
	return snd_ctl_elem_tlv_write(elem->ctl, &elem->id, tlv);
}

/**
 * \brief Set TLV value for an CTL element
 * \param elem CTL element
 * \param tlv TLV array for value
 * \retval 0 on success
 * \retval >1 on success when value was changed
 * \retval <0 a negative error code on failure
 */
int snd_ctl_celem_tlv_command(snd_ctl_elem_t *elem, const unsigned int *tlv)
{
	assert(elem);
	assert(tlv);
	assert(tlv[1] >= 4);
	return snd_ctl_elem_tlv_command(elem->ctl, &elem->id, tlv);
}

/**
 * \brief Get CTL handle for an CTL element
 * \param elem CTL element
 * \return CTL handle
 */
snd_ctl_t *snd_ctl_elem_get_ctl(snd_ctl_elem_t *elem)
{
	assert(elem);
	return elem->ctl;
}

/**
 * \brief Get CTL element identifier of a CTL element id/value
 * \param obj CTL element id/value
 * \param ptr Pointer to returned CTL element identifier
 */
void snd_ctl_elem_get_id(const snd_ctl_elem_t *obj, snd_ctl_elem_id_t *ptr)
{
	assert(obj && ptr);
	*ptr = obj->id;
}

/**
 * \brief Get element numeric identifier of a CTL element id/value
 * \param obj CTL element id/value
 * \return element numeric identifier
 */
unsigned int snd_ctl_elem_get_numid(const snd_ctl_elem_t *obj)
{
	assert(obj);
	return obj->id.numid;
}

/**
 * \brief Get interface part of CTL element identifier of a CTL element id/value
 * \param obj CTL element id/value
 * \return interface part of element identifier
 */
snd_ctl_elem_iface_t snd_ctl_elem_get_interface(const snd_ctl_elem_t *obj)
{
	assert(obj);
	return obj->id.iface;
}

/**
 * \brief Get device part of CTL element identifier of a CTL element id/value
 * \param obj CTL element id/value
 * \return device part of element identifier
 */
unsigned int snd_ctl_elem_get_device(const snd_ctl_elem_t *obj)
{
	assert(obj);
	return obj->id.device;
}

/**
 * \brief Get subdevice part of CTL element identifier of a CTL element id/value
 * \param obj CTL element id/value
 * \return subdevice part of element identifier
 */
unsigned int snd_ctl_elem_get_subdevice(const snd_ctl_elem_t *obj)
{
	assert(obj);
	return obj->id.subdevice;
}

/**
 * \brief Get name part of CTL element identifier of a CTL element id/value
 * \param obj CTL element id/value
 * \return name part of element identifier
 */
const char *snd_ctl_elem_get_name(const snd_ctl_elem_t *obj)
{
	assert(obj);
	return (const char *)obj->id.name;
}

/**
 * \brief Get index part of CTL element identifier of a CTL element id/value
 * \param obj CTL element id/value
 * \return index part of element identifier
 */
unsigned int snd_ctl_elem_get_index(const snd_ctl_elem_t *obj)
{
	assert(obj);
	return obj->id.index;
}

/**
 * \brief Set callback function for an CTL element
 * \param obj CTL element
 * \param val callback function
 */
void snd_ctl_elem_set_callback(snd_ctl_elem_t *obj, snd_ctl_elem_callback_t val)
{
	assert(obj);
	obj->callback = val;
}

/**
 * \brief Set callback private value for an CTL element
 * \param obj CTL element
 * \param val callback private value
 */
void snd_ctl_elem_set_callback_private(snd_ctl_elem_t *obj, void * val)
{
	assert(obj);
	obj->callback_private = val;
}

/**
 * \brief Get callback private value for an CTL element
 * \param obj CTL element
 * \return callback private value
 */
void * snd_ctl_elem_get_callback_private(const snd_ctl_elem_t *obj)
{
	assert(obj);
	return obj->callback_private;
}

