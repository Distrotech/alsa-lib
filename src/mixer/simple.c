/**
 * \file mixer/simple.c
 * \brief Mixer Simple Element Class Interface
 * \author Jaroslav Kysela <perex@perex.cz>
 * \author Abramo Bagnara <abramo@alsa-project.org>
 * \date 2001-2008
 *
 * Mixer simple element class interface.
 */
/*
 *  Mixer Interface - simple controls
 *  Copyright (c) 2000,2004 by Jaroslav Kysela <perex@perex.cz>
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <math.h>
#include "mixer_local.h"

#ifndef DOC_HIDDEN

#define CHECK_ENUM(xelem) \
	if (!(elem->sm.caps & (SM_CAP_PENUM|SM_CAP_CENUM))) \
		return -EINVAL;

#define COND_CAPS(xelem, what) \
	!!(elem->sm.caps & (what))

#define sm_elem(x)              (&(x)->sm)
#define sm_elem_ops(x)          ((x)->sm.ops)

#endif /* !DOC_HIDDEN */

#ifndef DOC_HIDDEN
int snd_amixer_compare_default(const snd_amixer_elem_t *c1, const snd_amixer_elem_t *c2)
{
	int d;
	
	d = c1->compare_weight - c2->compare_weight;
	if (d)
		return d;
	d = strcmp(c1->sm.id.name, c2->sm.id.name);
	if (d)
		return d;
	return c1->sm.id.index - c2->sm.id.index;
}
#endif
	
/**
 * \brief Find a mixer simple element
 * \param mixer Mixer handle
 * \param id Mixer simple element identifier
 * \return mixer simple element handle or NULL if not found
 */
snd_amixer_elem_t *snd_amixer_find_elem(snd_amixer_t *mixer,
				       const snd_amixer_elem_id_t *id)
{
	struct list_head *list;
	snd_amixer_elem_t *e;

	list_for_each(list, &mixer->elems) {
		e = list_entry(list, snd_amixer_elem_t, list);
		if (!strcmp(e->sm.id.name, id->name) && e->sm.id.index == id->index)
			return e;
	}
	return NULL;
}

/**
 * \brief Get mixer simple element identifier
 * \param elem Mixer simple element handle
 * \param id returned mixer simple element identifier
 */
void snd_amixer_elem_get_id(snd_amixer_elem_t *elem,
			    snd_amixer_elem_id_t *id)
{
	assert(id);
	*id = elem->sm.id;
}

/**
 * \brief Get name part of mixer simple element identifier
 * \param elem Mixer simple element handle
 * \return name part of simple element identifier
 */
const char *snd_amixer_elem_get_name(snd_amixer_elem_t *elem)
{
	return elem->sm.id.name;
}

/**
 * \brief Get index part of mixer simple element identifier
 * \param elem Mixer simple element handle
 * \return index part of simple element identifier
 */
unsigned int snd_amixer_elem_get_index(snd_amixer_elem_t *elem)
{
	return elem->sm.id.index;
}

/**
 * \brief Return true if mixer simple element has control for specified direction
 * \param elem Mixer simple element handle
 * \param dir Mixer direction
 * \return 0 false, 1 true
 */
int snd_amixer_elem_has_volume(snd_amixer_elem_t *elem, snd_amixer_dir_t dir)
{
	if (dir == SM_COMM)
		return COND_CAPS(elem, SM_CAP_GVOLUME);
	if (dir == SM_PLAY)
		return COND_CAPS(elem, SM_CAP_PVOLUME);
	if (dir == SM_CAPT)
		return COND_CAPS(elem, SM_CAP_CVOLUME);
	return 0;
}

/**
 * \brief Return info about volume control of a mixer simple element
 * \param elem Mixer simple element handle
 * \param dir Mixer direction
 * \return 0 if control is separated per channel, 1 if control acts on all channels together
 */
int snd_amixer_elem_has_volume_joined(snd_amixer_elem_t *elem, snd_amixer_dir_t dir)
{
	if (dir == SM_PLAY)
		return COND_CAPS(elem, SM_CAP_PVOLUME_JOIN);
	if (dir == SM_CAPT)
		return COND_CAPS(elem, SM_CAP_CVOLUME_JOIN);
	return 0;
}

/**
 * \brief Return true if mixer simple element has control for specified direction
 * \param elem Mixer simple element handle
 * \param dir Mixer direction
 * \return 0 false, 1 true
 */
int snd_amixer_elem_has_switch(snd_amixer_elem_t *elem, snd_amixer_dir_t dir)
{
	if (dir == SM_COMM)
		return COND_CAPS(elem, SM_CAP_GSWITCH);
	if (dir == SM_PLAY)
		return COND_CAPS(elem, SM_CAP_PSWITCH);
	if (dir == SM_CAPT)
		return COND_CAPS(elem, SM_CAP_CSWITCH);
	return 0;
}

/**
 * \brief Return info about switch control of a mixer simple element
 * \param elem Mixer simple element handle
 * \param dir Mixer direction
 * \return 0 if control is separated per channel, 1 if control acts on all channels together
 */
int snd_amixer_elem_has_switch_joined(snd_amixer_elem_t *elem, snd_amixer_dir_t dir)
{
	if (dir == SM_PLAY)
		return COND_CAPS(elem, SM_CAP_PSWITCH_JOIN);
	if (dir == SM_CAPT)
		return COND_CAPS(elem, SM_CAP_CSWITCH_JOIN);
	return 0;
}

/**
 * \brief Return name of mixer simple element channel
 * \param channel mixer simple element channel identifier
 * \return channel name
 */
const char *snd_amixer_elem_channel_name(snd_amixer_elem_channel_id_t channel)
{
	static const char *const array[SND_MIXER_SCHN_LAST + 1] = {
		[SND_MIXER_SCHN_FRONT_LEFT] = "Front Left",
		[SND_MIXER_SCHN_FRONT_RIGHT] = "Front Right",
		[SND_MIXER_SCHN_REAR_LEFT] = "Rear Left",
		[SND_MIXER_SCHN_REAR_RIGHT] = "Rear Right",
		[SND_MIXER_SCHN_FRONT_CENTER] = "Front Center",
		[SND_MIXER_SCHN_WOOFER] = "Woofer",
		[SND_MIXER_SCHN_SIDE_LEFT] = "Side Left",
		[SND_MIXER_SCHN_SIDE_RIGHT] = "Side Right",
		[SND_MIXER_SCHN_REAR_CENTER] = "Rear Center"
	};
	const char *p;
	assert(channel <= SND_MIXER_SCHN_LAST);
	p = array[channel];
	if (!p)
		return "?";
	return p;
}

/**
 * \brief Get info about the active state of a mixer simple element
 * \param elem Mixer simple element handle
 * \return 0 if not active, 1 if active
 */
int snd_amixer_elem_is_active(snd_amixer_elem_t *elem)
{
	return sm_elem_ops(elem)->is(elem, SM_PLAY, SM_OPS_IS_ACTIVE, 0);
}

/**
 * \brief Get info about channels of a mixer simple element
 * \param elem Mixer simple element handle
 * \param dir Mixer direction
 * \param channel Mixer simple element channel identifier
 * \return 0 if channel is not present, 1 if present
 */
int snd_amixer_elem_has_channel(snd_amixer_elem_t *elem, snd_amixer_dir_t dir, snd_amixer_elem_channel_id_t channel)
{
	return sm_elem_ops(elem)->is(elem, dir, SM_OPS_IS_CHANNEL, (int)channel);
}

/**
 * \brief Get count of valid channels
 * \param elem Mixer simple element handle
 * \param dir Mixer direction
 * \return 1 or more
 */
int snd_amixer_elem_get_channels(snd_amixer_elem_t *elem, snd_amixer_dir_t dir)
{
	return sm_elem_ops(elem)->get_channels(elem, dir);
}

/**
 * \brief Get range for volume of a mixer simple element
 * \param elem Mixer simple element handle
 * \param dir Mixer direction
 * \param min Pointer to returned minimum
 * \param max Pointer to returned maximum
 */
int snd_amixer_elem_get_volume_range(snd_amixer_elem_t *elem,
				     snd_amixer_dir_t dir,
				     long *min, long *max)
{
	if (!snd_amixer_elem_has_volume(elem, dir))
		return -EINVAL;
	return sm_elem_ops(elem)->get_range(elem, dir, min, max);
}

/**
 * \brief Get range in dB for volume of a mixer simple element
 * \param elem Mixer simple element handle
 * \param dir Mixer direction
 * \param min Pointer to returned minimum (dB * 100)
 * \param max Pointer to returned maximum (dB * 100)
 */
int snd_amixer_elem_get_dB_range(snd_amixer_elem_t *elem,
				 snd_amixer_dir_t dir,
				 long *min, long *max)
{
	if (!snd_amixer_elem_has_volume(elem, dir))
		return -EINVAL;
	return sm_elem_ops(elem)->get_dB_range(elem, dir, min, max);
}

/**
 * \brief Set range for volume of a mixer simple element
 * \param elem Mixer simple element handle
 * \param dir Mixer direction
 * \param min minimum volume value
 * \param max maximum volume value
 */
int snd_amixer_elem_set_volume_range(snd_amixer_elem_t *elem,
				     snd_amixer_dir_t dir,
				     long min, long max)
{
	assert(min < max);
	if (!snd_amixer_elem_has_volume(elem, dir))
		return -EINVAL;
	return sm_elem_ops(elem)->set_range(elem, dir, min, max);
}

/**
 * \brief Return corresponding dB value to an integer volume for a mixer simple element
 * \param elem Mixer simple element handle
 * \param dir Mixer direction
 * \param value value to be converted to dB range
 * \param dBvalue pointer to returned dB value
 * \return 0 on success otherwise a negative error code
 */
int snd_amixer_elem_ask_vol_dB(snd_amixer_elem_t *elem, snd_amixer_dir_t dir,
			       long value, long *dBvalue)
{
	if (!snd_amixer_elem_has_volume(elem, dir))
		return -EINVAL;
	return sm_elem_ops(elem)->ask_vol_dB(elem, dir, value, dBvalue);
}

/**
 * \brief Return corresponding integer volume for given dB value for a mixer simple element
 * \param elem Mixer simple element handle
 * \param dir Mixer direction
 * \param value value to be converted to dB range
 * \param xdir select direction (-1 = accurate or first bellow, 0 = accurate, 1 = accurate or first above)
 * \param dBvalue pointer to returned dB value
 * \return 0 on success otherwise a negative error code
 */
int snd_amixer_elem_ask_dB_vol(snd_amixer_elem_t *elem,
			       snd_amixer_dir_t dir,
			       long dBvalue, int xdir, long *value)
{
	if (!snd_amixer_elem_has_volume(elem, dir))
		return -EINVAL;
	return sm_elem_ops(elem)->ask_dB_vol(elem, dir, dBvalue, value, xdir);
}

/**
 * \brief Return value of playback volume control of a mixer simple element
 * \param elem Mixer simple element handle
 * \param dir Mixer direction
 * \param channel mixer simple element channel identifier
 * \param value pointer to returned value
 * \return 0 on success otherwise a negative error code
 */
int snd_amixer_elem_get_volume(snd_amixer_elem_t *elem, snd_amixer_dir_t dir,
			       snd_amixer_elem_channel_id_t channel,
			       long *value)
{
	if (!snd_amixer_elem_has_volume(elem, dir))
		return -EINVAL;
	if (snd_amixer_elem_has_volume_joined(elem, dir))
		channel = 0;
	return sm_elem_ops(elem)->get_volume(elem, dir, channel, value);
}

/**
 * \brief Return value of volume in dB control of a mixer simple element
 * \param elem Mixer simple element handle
 * \param dir Mixer direction
 * \param channel mixer simple element channel identifier
 * \param value pointer to returned value (dB * 100)
 * \return 0 on success otherwise a negative error code
 */
int snd_amixer_elem_get_dB(snd_amixer_elem_t *elem,
			   snd_amixer_dir_t dir,
			   snd_amixer_elem_channel_id_t channel, long *value)
{
	if (!snd_amixer_elem_has_volume(elem, dir))
		return -EINVAL;
	if (snd_amixer_elem_has_volume_joined(elem, dir))
		channel = 0;
	return sm_elem_ops(elem)->get_dB(elem, dir, channel, value);
}

/**
 * \brief Return value of playback switch control of a mixer simple element
 * \param elem Mixer simple element handle
 * \param dir Mixer direction
 * \param channel mixer simple element channel identifier
 * \param value pointer to returned value
 * \return 0 on success otherwise a negative error code
 */
int snd_amixer_elem_get_switch(snd_amixer_elem_t *elem, 
			       snd_amixer_dir_t dir,
			       snd_amixer_elem_channel_id_t channel,
			       int *value)
{
	if (!snd_amixer_elem_has_volume(elem, dir))
		return -EINVAL;
	if (snd_amixer_elem_has_volume_joined(elem, dir))
		channel = 0;
	return sm_elem_ops(elem)->get_switch(elem, dir, channel, value);
}

/**
 * \brief Set value of volume control of a mixer simple element
 * \param elem Mixer simple element handle
 * \param dir Mixer direction
 * \param channel mixer simple element channel identifier
 * \param value control value
 * \return 0 on success otherwise a negative error code
 */
int snd_amixer_elem_set_volume(snd_amixer_elem_t *elem,
			       snd_amixer_dir_t dir,
			       snd_amixer_elem_channel_id_t channel,
			       long value)
{
	if (!snd_amixer_elem_has_volume(elem, dir))
		return -EINVAL;
	if (snd_amixer_elem_has_volume_joined(elem, dir))
		channel = 0;
	return sm_elem_ops(elem)->set_volume(elem, dir, channel, value);
}

/**
 * \brief Set value in dB of playback volume control of a mixer simple element
 * \param elem Mixer simple element handle
 * \param dir Mixer direction
 * \param channel mixer simple element channel identifier
 * \param value control value in dB * 100
 * \param dir select direction (-1 = accurate or first bellow, 0 = accurate, 1 = accurate or first above)
 * \return 0 on success otherwise a negative error code
 */
int snd_amixer_elem_set_dB(snd_amixer_elem_t *elem,
			   snd_amixer_dir_t dir,
			   snd_amixer_elem_channel_id_t channel,
			   long value, int xdir)
{
	if (!snd_amixer_elem_has_volume(elem, dir))
		return -EINVAL;
	if (snd_amixer_elem_has_volume_joined(elem, dir))
		channel = 0;
	return sm_elem_ops(elem)->set_dB(elem, dir, channel, value, xdir);
}

/**
 * \brief Set value of volume control for all channels of a mixer simple element
 * \param elem Mixer simple element handle
 * \param dir Mixer direction
 * \param value control value
 * \return 0 on success otherwise a negative error code
 */
int snd_amixer_elem_set_volume_all(snd_amixer_elem_t *elem,
				   snd_amixer_dir_t dir, long value)
{
	snd_amixer_elem_channel_id_t chn;
	int err;

	for (chn = 0; chn <= SND_MIXER_SCHN_LAST; chn++) {
		if (!snd_amixer_elem_has_channel(elem, dir, chn))
			continue;
		err = snd_amixer_elem_set_volume(elem, dir, chn, value);
		if (err < 0)
			return err;
		if (chn == 0 && snd_amixer_elem_has_volume_joined(elem, dir))
			return 0;
	}
	return 0;
}

/**
 * \brief Set value in dB of volume control for all channels of a mixer simple element
 * \param elem Mixer simple element handle
 * \param dir Mixer direction
 * \param value control value in dB * 100
 * \param dir select direction (-1 = accurate or first bellow, 0 = accurate, 1 = accurate or first above)
 * \return 0 on success otherwise a negative error code
 */
int snd_amixer_elem_set_dB_all(snd_amixer_elem_t *elem, snd_amixer_dir_t dir,
			       long value, int xdir)
{
	snd_amixer_elem_channel_id_t chn;
	int err;

	for (chn = 0; chn <= SND_MIXER_SCHN_LAST; chn++) {
		if (!snd_amixer_elem_has_channel(elem, dir, chn))
			continue;
		err = snd_amixer_elem_set_dB(elem, dir, chn, value, xdir);
		if (err < 0)
			return err;
		if (chn == 0 && snd_amixer_elem_has_volume_joined(elem, dir))
			return 0;
	}
	return 0;
}

/**
 * \brief Set value of playback switch control of a mixer simple element
 * \param elem Mixer simple element handle
 * \param dir Mixer direction
 * \param channel mixer simple element channel identifier
 * \param value control value
 * \return 0 on success otherwise a negative error code
 */
int snd_amixer_elem_set_switch(snd_amixer_elem_t *elem,
			       snd_amixer_dir_t dir,
			       snd_amixer_elem_channel_id_t channel,
			       int value)
{
	if (!snd_amixer_elem_has_switch(elem, dir))
		return -EINVAL;
	if (snd_amixer_elem_has_switch_joined(elem, dir))
		channel = 0;
	return sm_elem_ops(elem)->set_switch(elem, dir, channel, value);
}

/**
 * \brief Set value of switch control for all channels of a mixer simple element
 * \param elem Mixer simple element handle
 * \param dir Mixer direction
 * \param value control value
 * \return 0 on success otherwise a negative error code
 */
int snd_amixer_elem_set_switch_all(snd_amixer_elem_t *elem,
				   snd_amixer_dir_t dir, int value)
{
	snd_amixer_elem_channel_id_t chn;
	int err;

	for (chn = 0; chn <= SND_MIXER_SCHN_LAST; chn++) {
		if (!snd_amixer_elem_has_channel(elem, dir, chn))
			continue;
		err = snd_amixer_elem_set_switch(elem, dir, chn, value);
		if (err < 0)
			return err;
		if (chn == 0 && snd_amixer_elem_has_switch_joined(elem, dir))
			return 0;
	}
	return 0;
}

/**
 * \brief Return info about switch control of a mixer simple element
 * \param elem Mixer simple element handle
 * \param dir Mixer direction (should be capture for now)
 * \return 0 if control is separated per element, 1 if control acts on other elements too (i.e. only one active at a time inside a group)
 */
int snd_amixer_elem_has_switch_exclusive(snd_amixer_elem_t *elem, snd_amixer_dir_t dir)
{
	if (dir == SM_CAPT)
		return COND_CAPS(elem, SM_CAP_CSWITCH_EXCL);
	return 0;
}

/**
 * \brief Return info about switch control of a mixer simple element
 * \param elem Mixer simple element handle
 * \param dir Mixer direction
 * \return group for switch exclusivity (see #snd_amixer_elem_has_switch_exclusive)
 */
int snd_amixer_elem_get_group(snd_amixer_elem_t *elem, snd_amixer_dir_t dir)
{
	sm_elem_t *s;
	if (dir != SM_CAPT)
		return -EINVAL;
	s = sm_elem(elem);
	if (! (s->caps & SM_CAP_CSWITCH_EXCL))
		return -EINVAL;
	return s->capture_group;
}

/**
 * \brief Return true if mixer simple enumerated element belongs to the direction
 * \param elem Mixer simple element handle
 * \param dir Mixer direction
 * \return 0 no playback direction, 1 playback direction
 */
int snd_amixer_elem_is_enum(snd_amixer_elem_t *elem, snd_amixer_dir_t dir)
{
	int res;

	if (!(elem->sm.caps & (SM_CAP_PENUM|SM_CAP_CENUM)))
		return 0;
	return sm_elem_ops(elem)->is(elem, dir, SM_OPS_IS_ENUMERATED, 0);
}

/**
 * \brief Return the number of enumerated items of the given mixer simple element
 * \param elem Mixer simple element handle
 * \return the number of enumerated items, otherwise a negative error code
 */
int snd_amixer_elem_get_enum_items(snd_amixer_elem_t *elem)
{
	CHECK_ENUM(elem);
	return sm_elem_ops(elem)->is(elem, SM_PLAY, SM_OPS_IS_ENUMCNT, 0);
}

/**
 * \brief get the enumerated item string for the given mixer simple element
 * \param elem Mixer simple element handle
 * \param item the index of the enumerated item to query
 * \param maxlen the maximal length to be stored
 * \param buf the buffer to store the name string
 * \return 0 if successful, otherwise a negative error code
 */
int snd_amixer_elem_get_enum_item_name(snd_amixer_elem_t *elem,
				       unsigned int item,
				       size_t maxlen, char *buf)
{
	CHECK_ENUM(elem);
	return sm_elem_ops(elem)->enum_item_name(elem, item, maxlen, buf);
}

/**
 * \brief get the current selected enumerated item for the given mixer simple element
 * \param elem Mixer simple element handle
 * \param channel mixer simple element channel identifier
 * \param itemp the pointer to store the index of the enumerated item
 * \return 0 if successful, otherwise a negative error code
 */
int snd_amixer_elem_get_enum_item(snd_amixer_elem_t *elem,
				  snd_amixer_elem_channel_id_t channel,
				  unsigned int *itemp)
{
	CHECK_ENUM(elem);
	return sm_elem_ops(elem)->get_enum_item(elem, channel, itemp);
}

/**
 * \brief set the current selected enumerated item for the given mixer simple element
 * \param elem Mixer simple element handle
 * \param channel mixer simple element channel identifier
 * \param item the enumerated item index
 * \return 0 if successful, otherwise a negative error code
 */
int snd_amixer_elem_set_enum_item(snd_amixer_elem_t *elem,
				  snd_amixer_elem_channel_id_t channel,
				  unsigned int item)
{
	CHECK_ENUM(elem);
	return sm_elem_ops(elem)->set_enum_item(elem, channel, item);
}

/**
 * \brief get size of #snd_amixer_elem_id_t
 * \return size in bytes
 */
size_t snd_amixer_elem_id_sizeof()
{
	return sizeof(snd_amixer_elem_id_t);
}

/**
 * \brief allocate an invalid #snd_amixer_elem_id_t using standard malloc
 * \param ptr returned pointer
 * \return 0 on success otherwise negative error code
 */
int snd_amixer_elem_id_malloc(snd_amixer_elem_id_t **ptr)
{
	assert(ptr);
	*ptr = calloc(1, sizeof(snd_amixer_elem_id_t));
	if (!*ptr)
		return -ENOMEM;
	return 0;
}

/**
 * \brief frees a previously allocated #snd_amixer_elem_id_t
 * \param obj pointer to object to free
 */
void snd_amixer_elem_id_free(snd_amixer_elem_id_t *obj)
{
	free(obj);
}

/**
 * \brief copy one #snd_amixer_elem_id_t to another
 * \param dst pointer to destination
 * \param src pointer to source
 */
void snd_amixer_elem_id_copy(snd_amixer_elem_id_t *dst, const snd_amixer_elem_id_t *src)
{
	assert(dst && src);
	*dst = *src;
}

/**
 * \brief Get name part of a mixer simple element identifier
 * \param obj Mixer simple element identifier
 * \return name part
 */
const char *snd_amixer_elem_id_get_name(const snd_amixer_elem_id_t *obj)
{
	assert(obj);
	return obj->name;
}

/**
 * \brief Get index part of a mixer simple element identifier
 * \param obj Mixer simple element identifier
 * \return index part
 */
unsigned int snd_amixer_elem_id_get_index(const snd_amixer_elem_id_t *obj)
{
	assert(obj);
	return obj->index;
}

/**
 * \brief Set name part of a mixer simple element identifier
 * \param obj Mixer simple element identifier
 * \param val name part
 */
void snd_amixer_elem_id_set_name(snd_amixer_elem_id_t *obj, const char *val)
{
	assert(obj);
	strncpy(obj->name, val, sizeof(obj->name));
	obj->name[sizeof(obj->name)-1] = '\0';
}

/**
 * \brief Set index part of a mixer simple element identifier
 * \param obj Mixer simple element identifier
 * \param val index part
 */
void snd_amixer_elem_id_set_index(snd_amixer_elem_id_t *obj, unsigned int val)
{
	assert(obj);
	obj->index = val;
}

/**
 * \brief Get simple mixer element abstraction structure
 * \param obj Mixer simple element identifier
 * \return sm_elem_t pointer
 */
sm_elem_t *snd_amixer_elem_get_sm(snd_amixer_elem_t *obj)
{
	assert(obj);
	return &obj->sm;
}

