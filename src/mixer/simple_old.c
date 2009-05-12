/**
 * \file mixer/simple.c
 * \brief Mixer Simple Element Class Interface
 * \author Jaroslav Kysela <perex@perex.cz>
 * \author Abramo Bagnara <abramo@alsa-project.org>
 * \date 2001-2004
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
#include "mixer_old_local.h"

#define SM_PLAY SND_MIXER_DIR_PLAYBACK
#define SM_CAPT SND_MIXER_DIR_CAPTURE
#define SM_COMM SND_MIXER_DIR_COMMON

/**
 * \brief Register mixer simple element class
 * \param mixer Mixer handle
 * \param options Options container
 * \param nothing Pass NULL
 * \return zero on success, otherwise a negative error code
 */
int snd_mixer_selem_register(snd_mixer_t *mixer,
			     struct snd_mixer_selem_regopt *options,
			     void **nothing ATTRIBUTE_UNUSED)
{
	int err;

	if (options && options->ver == 1) {
		if (options->device != NULL &&
		    (options->playback_pcm != NULL ||
		     options->capture_pcm != NULL))
			return -EINVAL;
		if (options->device == NULL &&
		    options->playback_pcm == NULL &&
		    options->capture_pcm == NULL)
			return -EINVAL;
	}
	if (options == NULL ||
	    (options->ver == 1 && options->abstract == SND_MIXER_SABSTRACT_NONE)) {
		if (options != NULL) {
			err = snd_mixer_attach(mixer, options->device);
			if (err < 0)
				return err;
		}
		return 0;
	} else if (options->ver == 1) {
		if (options->abstract == SND_MIXER_SABSTRACT_BASIC)
			return snd_mixer_attach(mixer, options->device);
	}
	return -ENXIO;
}

/**
 * \brief Find a mixer simple element
 * \param mixer Mixer handle
 * \param id Mixer simple element identifier
 * \return mixer simple element handle or NULL if not found
 */
snd_mixer_elem_t *snd_mixer_find_selem(snd_mixer_t *mixer,
				       const snd_mixer_selem_id_t *id)
{
	return snd_amixer_find_elem(mixer->amixer, id);
}

/**
 * \brief Get mixer simple element identifier
 * \param elem Mixer simple element handle
 * \param id returned mixer simple element identifier
 */
void snd_mixer_selem_get_id(snd_mixer_elem_t *elem,
			    snd_mixer_selem_id_t *id)
{
	return snd_amixer_elem_get_id(elem, id);
}

/**
 * \brief Get name part of mixer simple element identifier
 * \param elem Mixer simple element handle
 * \return name part of simple element identifier
 */
const char *snd_mixer_selem_get_name(snd_mixer_elem_t *elem)
{
	return snd_amixer_elem_get_name(elem);
}

/**
 * \brief Get index part of mixer simple element identifier
 * \param elem Mixer simple element handle
 * \return index part of simple element identifier
 */
unsigned int snd_mixer_selem_get_index(snd_mixer_elem_t *elem)
{
	return snd_amixer_elem_get_index(elem);
}

/**
 * \brief Return true if mixer simple element has only one volume control for both playback and capture
 * \param elem Mixer simple element handle
 * \return 0 separated control, 1 common control
 */
int snd_mixer_selem_has_common_volume(snd_mixer_elem_t *elem)
{
	return snd_amixer_elem_has_volume(elem, SM_COMM);
}

/**
 * \brief Return true if mixer simple element has only one switch control for both playback and capture
 * \param elem Mixer simple element handle
 * \return 0 separated control, 1 common control
 */
int snd_mixer_selem_has_common_switch(snd_mixer_elem_t *elem)
{
	return snd_amixer_elem_has_switch(elem, SM_COMM);
}

/**
 * \brief Return name of mixer simple element channel
 * \param channel mixer simple element channel identifier
 * \return channel name
 */
const char *snd_mixer_selem_channel_name(snd_mixer_selem_channel_id_t channel)
{
	return snd_amixer_elem_channel_name(channel);
}

/**
 * \brief Get info about the active state of a mixer simple element
 * \param elem Mixer simple element handle
 * \return 0 if not active, 1 if active
 */
int snd_mixer_selem_is_active(snd_mixer_elem_t *elem)
{
	return snd_amixer_elem_is_active(elem);
}

/**
 * \brief Get info about channels of playback stream of a mixer simple element
 * \param elem Mixer simple element handle
 * \return 0 if not mono, 1 if mono
 */
int snd_mixer_selem_is_playback_mono(snd_mixer_elem_t *elem)
{
	return snd_amixer_elem_get_channels(elem, SM_PLAY) == 1;
}

/**
 * \brief Get info about channels of playback stream of a mixer simple element
 * \param elem Mixer simple element handle
 * \param channel Mixer simple element channel identifier
 * \return 0 if channel is not present, 1 if present
 */
int snd_mixer_selem_has_playback_channel(snd_mixer_elem_t *elem, snd_mixer_selem_channel_id_t channel)
{
	return snd_amixer_elem_has_channel(elem, SM_PLAY, channel);
}

/**
 * \brief Get range for playback volume of a mixer simple element
 * \param elem Mixer simple element handle
 * \param min Pointer to returned minimum
 * \param max Pointer to returned maximum
 */
int snd_mixer_selem_get_playback_volume_range(snd_mixer_elem_t *elem,
					       long *min, long *max)
{
	return snd_amixer_elem_get_volume_range(elem, SM_PLAY, min, max);
}

/**
 * \brief Get range in dB for playback volume of a mixer simple element
 * \param elem Mixer simple element handle
 * \param min Pointer to returned minimum (dB * 100)
 * \param max Pointer to returned maximum (dB * 100)
 */
int snd_mixer_selem_get_playback_dB_range(snd_mixer_elem_t *elem,
					  long *min, long *max)
{
	return snd_amixer_elem_get_dB_range(elem, SM_PLAY, min, max);
}

/**
 * \brief Set range for playback volume of a mixer simple element
 * \param elem Mixer simple element handle
 * \param min minimum volume value
 * \param max maximum volume value
 */
int snd_mixer_selem_set_playback_volume_range(snd_mixer_elem_t *elem, 
					      long min, long max)
{
	return snd_amixer_elem_set_volume_range(elem, SM_PLAY, min, max);
}

/**
 * \brief Return info about playback volume control of a mixer simple element
 * \param elem Mixer simple element handle
 * \return 0 if no control is present, 1 if it's present
 */
int snd_mixer_selem_has_playback_volume(snd_mixer_elem_t *elem)
{
	return snd_amixer_elem_has_volume(elem, SM_PLAY);
}

/**
 * \brief Return info about playback volume control of a mixer simple element
 * \param elem Mixer simple element handle
 * \return 0 if control is separated per channel, 1 if control acts on all channels together
 */
int snd_mixer_selem_has_playback_volume_joined(snd_mixer_elem_t *elem)
{
	return snd_amixer_elem_has_volume_joined(elem, SM_PLAY);
}

/**
 * \brief Return info about playback switch control existence of a mixer simple element
 * \param elem Mixer simple element handle
 * \return 0 if no control is present, 1 if it's present
 */
int snd_mixer_selem_has_playback_switch(snd_mixer_elem_t *elem)
{
	return snd_amixer_elem_has_switch(elem, SM_PLAY);
}

/**
 * \brief Return info about playback switch control of a mixer simple element
 * \param elem Mixer simple element handle
 * \return 0 if control is separated per channel, 1 if control acts on all channels together
 */
int snd_mixer_selem_has_playback_switch_joined(snd_mixer_elem_t *elem)
{
	return snd_amixer_elem_has_switch_joined(elem, SM_PLAY);
}

/**
 * \brief Return corresponding dB value to an integer playback volume for a mixer simple element
 * \param elem Mixer simple element handle
 * \param value value to be converted to dB range
 * \param dBvalue pointer to returned dB value
 * \return 0 on success otherwise a negative error code
 */
int snd_mixer_selem_ask_playback_vol_dB(snd_mixer_elem_t *elem, long value, long *dBvalue)
{
	return snd_amixer_elem_ask_vol_dB(elem, SM_PLAY, value, dBvalue);
}

/**
 * \brief Return corresponding integer playback volume for given dB value for a mixer simple element
 * \param elem Mixer simple element handle
 * \param value value to be converted to dB range
 * \param dir select direction (-1 = accurate or first bellow, 0 = accurate, 1 = accurate or first above)
 * \param dBvalue pointer to returned dB value
 * \return 0 on success otherwise a negative error code
 */
int snd_mixer_selem_ask_playback_dB_vol(snd_mixer_elem_t *elem, long dBvalue, int dir, long *value)
{
	return snd_amixer_elem_ask_dB_vol(elem, SM_PLAY, dBvalue, dir, value);
}

/**
 * \brief Return value of playback volume control of a mixer simple element
 * \param elem Mixer simple element handle
 * \param channel mixer simple element channel identifier
 * \param value pointer to returned value
 * \return 0 on success otherwise a negative error code
 */
int snd_mixer_selem_get_playback_volume(snd_mixer_elem_t *elem, snd_mixer_selem_channel_id_t channel, long *value)
{
	return snd_amixer_elem_get_volume(elem, SM_PLAY, channel, value);
}

/**
 * \brief Return value of playback volume in dB control of a mixer simple element
 * \param elem Mixer simple element handle
 * \param channel mixer simple element channel identifier
 * \param value pointer to returned value (dB * 100)
 * \return 0 on success otherwise a negative error code
 */
int snd_mixer_selem_get_playback_dB(snd_mixer_elem_t *elem, snd_mixer_selem_channel_id_t channel, long *value)
{
	return snd_amixer_elem_get_dB(elem, SM_PLAY, channel, value);
}

/**
 * \brief Return value of playback switch control of a mixer simple element
 * \param elem Mixer simple element handle
 * \param channel mixer simple element channel identifier
 * \param value pointer to returned value
 * \return 0 on success otherwise a negative error code
 */
int snd_mixer_selem_get_playback_switch(snd_mixer_elem_t *elem, snd_mixer_selem_channel_id_t channel, int *value)
{
	return snd_amixer_elem_get_switch(elem, SM_PLAY, channel, value);
}

/**
 * \brief Set value of playback volume control of a mixer simple element
 * \param elem Mixer simple element handle
 * \param channel mixer simple element channel identifier
 * \param value control value
 * \return 0 on success otherwise a negative error code
 */
int snd_mixer_selem_set_playback_volume(snd_mixer_elem_t *elem, snd_mixer_selem_channel_id_t channel, long value)
{
	return snd_amixer_elem_set_volume(elem, SM_PLAY, channel, value);
}

/**
 * \brief Set value in dB of playback volume control of a mixer simple element
 * \param elem Mixer simple element handle
 * \param channel mixer simple element channel identifier
 * \param value control value in dB * 100
 * \param dir select direction (-1 = accurate or first bellow, 0 = accurate, 1 = accurate or first above)
 * \return 0 on success otherwise a negative error code
 */
int snd_mixer_selem_set_playback_dB(snd_mixer_elem_t *elem, snd_mixer_selem_channel_id_t channel, long value, int dir)
{
	return snd_amixer_elem_set_dB(elem, SM_PLAY, channel, value, dir);
}

/**
 * \brief Set value of playback volume control for all channels of a mixer simple element
 * \param elem Mixer simple element handle
 * \param value control value
 * \return 0 on success otherwise a negative error code
 */
int snd_mixer_selem_set_playback_volume_all(snd_mixer_elem_t *elem, long value)
{
	return snd_amixer_elem_set_volume_all(elem, SM_PLAY, value);
}

/**
 * \brief Set value in dB of playback volume control for all channels of a mixer simple element
 * \param elem Mixer simple element handle
 * \param value control value in dB * 100
 * \param dir select direction (-1 = accurate or first bellow, 0 = accurate, 1 = accurate or first above)
 * \return 0 on success otherwise a negative error code
 */
int snd_mixer_selem_set_playback_dB_all(snd_mixer_elem_t *elem, long value, int dir)
{
	return snd_amixer_elem_set_dB_all(elem, SM_PLAY, value, dir);
}

/**
 * \brief Set value of playback switch control of a mixer simple element
 * \param elem Mixer simple element handle
 * \param channel mixer simple element channel identifier
 * \param value control value
 * \return 0 on success otherwise a negative error code
 */
int snd_mixer_selem_set_playback_switch(snd_mixer_elem_t *elem, snd_mixer_selem_channel_id_t channel, int value)
{
	return snd_amixer_elem_set_switch(elem, SM_PLAY, channel, value);
}

/**
 * \brief Set value of playback switch control for all channels of a mixer simple element
 * \param elem Mixer simple element handle
 * \param value control value
 * \return 0 on success otherwise a negative error code
 */
int snd_mixer_selem_set_playback_switch_all(snd_mixer_elem_t *elem, int value)
{
	return snd_amixer_elem_set_switch_all(elem, SM_PLAY, value);
}

/**
 * \brief Get info about channels of capture stream of a mixer simple element
 * \param elem Mixer simple element handle
 * \return 0 if not mono, 1 if mono
 */
int snd_mixer_selem_is_capture_mono(snd_mixer_elem_t *elem)
{
	return snd_amixer_elem_get_channels(elem, SM_CAPT) == 1;
}

/**
 * \brief Get info about channels of capture stream of a mixer simple element
 * \param elem Mixer simple element handle
 * \param channel Mixer simple element channel identifier
 * \return 0 if channel is not present, 1 if present
 */
int snd_mixer_selem_has_capture_channel(snd_mixer_elem_t *elem, snd_mixer_selem_channel_id_t channel)
{
	return snd_amixer_elem_has_channel(elem, SM_CAPT, channel);
}

/**
 * \brief Get range for capture volume of a mixer simple element
 * \param elem Mixer simple element handle
 * \param min Pointer to returned minimum
 * \param max Pointer to returned maximum
 */
int snd_mixer_selem_get_capture_volume_range(snd_mixer_elem_t *elem,
					     long *min, long *max)
{
	return snd_amixer_elem_get_volume_range(elem, SM_CAPT, min, max);
}

/**
 * \brief Get range in dB for capture volume of a mixer simple element
 * \param elem Mixer simple element handle
 * \param min Pointer to returned minimum (dB * 100)
 * \param max Pointer to returned maximum (dB * 100)
 */
int snd_mixer_selem_get_capture_dB_range(snd_mixer_elem_t *elem,
					 long *min, long *max)
{
	return snd_amixer_elem_get_dB_range(elem, SM_CAPT, min, max);
}

/**
 * \brief Set range for capture volume of a mixer simple element
 * \param elem Mixer simple element handle
 * \param min minimum volume value
 * \param max maximum volume value
 */
int snd_mixer_selem_set_capture_volume_range(snd_mixer_elem_t *elem, 
					     long min, long max)
{
	return snd_amixer_elem_set_volume_range(elem, SM_CAPT, min, max);
}

/**
 * \brief Return info about capture volume control of a mixer simple element
 * \param elem Mixer simple element handle
 * \return 0 if no control is present, 1 if it's present
 */
int snd_mixer_selem_has_capture_volume(snd_mixer_elem_t *elem)
{
	return snd_amixer_elem_has_volume(elem, SM_CAPT);
}

/**
 * \brief Return info about capture volume control of a mixer simple element
 * \param elem Mixer simple element handle
 * \return 0 if control is separated per channel, 1 if control acts on all channels together
 */
int snd_mixer_selem_has_capture_volume_joined(snd_mixer_elem_t *elem)
{
	return snd_amixer_elem_has_volume_joined(elem, SM_CAPT);
}

/**
 * \brief Return info about capture switch control existence of a mixer simple element
 * \param elem Mixer simple element handle
 * \return 0 if no control is present, 1 if it's present
 */
int snd_mixer_selem_has_capture_switch(snd_mixer_elem_t *elem)
{
	return snd_amixer_elem_has_switch(elem, SM_CAPT);
}

/**
 * \brief Return info about capture switch control of a mixer simple element
 * \param elem Mixer simple element handle
 * \return 0 if control is separated per channel, 1 if control acts on all channels together
 */
int snd_mixer_selem_has_capture_switch_joined(snd_mixer_elem_t *elem)
{
	return snd_amixer_elem_has_switch_joined(elem, SM_CAPT);
}

/**
 * \brief Return info about capture switch control of a mixer simple element
 * \param elem Mixer simple element handle
 * \return 0 if control is separated per element, 1 if control acts on other elements too (i.e. only one active at a time inside a group)
 */
int snd_mixer_selem_has_capture_switch_exclusive(snd_mixer_elem_t *elem)
{
	return snd_amixer_elem_has_switch_exclusive(elem, SM_CAPT);
}

/**
 * \brief Return info about capture switch control of a mixer simple element
 * \param elem Mixer simple element handle
 * \return group for switch exclusivity (see #snd_mixer_selem_has_capture_switch_exclusive)
 */
int snd_mixer_selem_get_capture_group(snd_mixer_elem_t *elem)
{
	return snd_amixer_elem_get_group(elem, SM_CAPT);
}

/**
 * \brief Return corresponding dB value to an integer capture volume for a mixer simple element
 * \param elem Mixer simple element handle
 * \param value value to be converted to dB range
 * \param dBvalue pointer to returned dB value
 * \return 0 on success otherwise a negative error code
 */
int snd_mixer_selem_ask_capture_vol_dB(snd_mixer_elem_t *elem, long value, long *dBvalue)
{
	return snd_amixer_elem_ask_vol_dB(elem, SM_CAPT, value, dBvalue);
}

/**
 * \brief Return corresponding integer capture volume for given dB value for a mixer simple element
 * \param elem Mixer simple element handle
 * \param dBvalue dB value to be converted to integer range
 * \param value pointer to returned integer value
 * \param dir select direction (-1 = accurate or first bellow, 0 = accurate, 1 = accurate or first above)
 * \return 0 on success otherwise a negative error code
 */
int snd_mixer_selem_ask_capture_dB_vol(snd_mixer_elem_t *elem, long dBvalue, int dir, long *value)
{
	return snd_amixer_elem_ask_dB_vol(elem, SM_CAPT, dBvalue, dir, value);
}

/**
 * \brief Return value of capture volume control of a mixer simple element
 * \param elem Mixer simple element handle
 * \param channel mixer simple element channel identifier
 * \param value pointer to returned value
 * \return 0 on success otherwise a negative error code
 */
int snd_mixer_selem_get_capture_volume(snd_mixer_elem_t *elem, snd_mixer_selem_channel_id_t channel, long *value)
{
	return snd_amixer_elem_get_volume(elem, SM_CAPT, channel, value);
}

/**
 * \brief Return value of capture volume in dB control of a mixer simple element
 * \param elem Mixer simple element handle
 * \param channel mixer simple element channel identifier
 * \param value pointer to returned value (dB * 100)
 * \return 0 on success otherwise a negative error code
 */
int snd_mixer_selem_get_capture_dB(snd_mixer_elem_t *elem, snd_mixer_selem_channel_id_t channel, long *value)
{
	return snd_amixer_elem_get_dB(elem, SM_CAPT, channel, value);
}

/**
 * \brief Return value of capture switch control of a mixer simple element
 * \param elem Mixer simple element handle
 * \param channel mixer simple element channel identifier
 * \param value pointer to returned value
 * \return 0 on success otherwise a negative error code
 */
int snd_mixer_selem_get_capture_switch(snd_mixer_elem_t *elem, snd_mixer_selem_channel_id_t channel, int *value)
{
	return snd_amixer_elem_get_switch(elem, SM_CAPT, channel, value);
}

/**
 * \brief Set value of capture volume control of a mixer simple element
 * \param elem Mixer simple element handle
 * \param channel mixer simple element channel identifier
 * \param value control value
 * \return 0 on success otherwise a negative error code
 */
int snd_mixer_selem_set_capture_volume(snd_mixer_elem_t *elem, snd_mixer_selem_channel_id_t channel, long value)
{
	return snd_amixer_elem_set_volume(elem, SM_CAPT, channel, value);
}

/**
 * \brief Set value in dB of capture volume control of a mixer simple element
 * \param elem Mixer simple element handle
 * \param channel mixer simple element channel identifier
 * \param value control value in dB * 100
 * \param dir select direction (-1 = accurate or first bellow, 0 = accurate, 1 = accurate or first above)
 * \return 0 on success otherwise a negative error code
 */
int snd_mixer_selem_set_capture_dB(snd_mixer_elem_t *elem, snd_mixer_selem_channel_id_t channel, long value, int dir)
{
	return snd_amixer_elem_set_dB(elem, SM_CAPT, channel, value, dir);
}

/**
 * \brief Set value of capture volume control for all channels of a mixer simple element
 * \param elem Mixer simple element handle
 * \param value control value
 * \return 0 on success otherwise a negative error code
 */
int snd_mixer_selem_set_capture_volume_all(snd_mixer_elem_t *elem, long value)
{
	return snd_amixer_elem_set_volume_all(elem, SM_CAPT, value);
}

/**
 * \brief Set value in dB of capture volume control for all channels of a mixer simple element
 * \param elem Mixer simple element handle
 * \param value control value in dB * 100
 * \param dir select direction (-1 = accurate or first bellow, 0 = accurate, 1 = accurate or first above)
 * \return 0 on success otherwise a negative error code
 */
int snd_mixer_selem_set_capture_dB_all(snd_mixer_elem_t *elem, long value, int dir)
{
	return snd_amixer_elem_set_dB_all(elem, SM_CAPT, value, dir);
}

/**
 * \brief Set value of capture switch control of a mixer simple element
 * \param elem Mixer simple element handle
 * \param channel mixer simple element channel identifier
 * \param value control value
 * \return 0 on success otherwise a negative error code
 */
int snd_mixer_selem_set_capture_switch(snd_mixer_elem_t *elem, snd_mixer_selem_channel_id_t channel, int value)
{
	return snd_amixer_elem_set_switch(elem, SM_CAPT, channel, value);
}

/**
 * \brief Set value of capture switch control for all channels of a mixer simple element
 * \param elem Mixer simple element handle
 * \param value control value
 * \return 0 on success otherwise a negative error code
 */
int snd_mixer_selem_set_capture_switch_all(snd_mixer_elem_t *elem, int value)
{
	return snd_amixer_elem_set_switch_all(elem, SM_CAPT, value);
}

/**
 * \brief Return true if mixer simple element is an enumerated control
 * \param elem Mixer simple element handle
 * \return 0 normal volume/switch control, 1 enumerated control
 */
int snd_mixer_selem_is_enumerated(snd_mixer_elem_t *elem)
{
	return snd_amixer_elem_is_enum(elem, SM_COMM) ||
	       snd_amixer_elem_is_enum(elem, SM_PLAY) ||
	       snd_amixer_elem_is_enum(elem, SM_CAPT);
}

/**
 * \brief Return true if mixer simple enumerated element belongs to the playback direction
 * \param elem Mixer simple element handle
 * \return 0 no playback direction, 1 playback direction
 */
int snd_mixer_selem_is_enum_playback(snd_mixer_elem_t *elem)
{
	return snd_amixer_elem_is_enum(elem, SM_PLAY);
}

/**
 * \brief Return true if mixer simple enumerated element belongs to the capture direction
 * \param elem Mixer simple element handle
 * \return 0 no capture direction, 1 capture direction
 */
int snd_mixer_selem_is_enum_capture(snd_mixer_elem_t *elem)
{
	return snd_amixer_elem_is_enum(elem, SM_CAPT);
}

/**
 * \brief Return the number of enumerated items of the given mixer simple element
 * \param elem Mixer simple element handle
 * \return the number of enumerated items, otherwise a negative error code
 */
int snd_mixer_selem_get_enum_items(snd_mixer_elem_t *elem)
{
	return snd_amixer_elem_get_enum_items(elem);
}

/**
 * \brief get the enumerated item string for the given mixer simple element
 * \param elem Mixer simple element handle
 * \param item the index of the enumerated item to query
 * \param maxlen the maximal length to be stored
 * \param buf the buffer to store the name string
 * \return 0 if successful, otherwise a negative error code
 */
int snd_mixer_selem_get_enum_item_name(snd_mixer_elem_t *elem,
				       unsigned int item,
				       size_t maxlen, char *buf)
{
	return snd_amixer_elem_get_enum_item_name(elem, item, maxlen, buf);
}

/**
 * \brief get the current selected enumerated item for the given mixer simple element
 * \param elem Mixer simple element handle
 * \param channel mixer simple element channel identifier
 * \param itemp the pointer to store the index of the enumerated item
 * \return 0 if successful, otherwise a negative error code
 */
int snd_mixer_selem_get_enum_item(snd_mixer_elem_t *elem,
				  snd_mixer_selem_channel_id_t channel,
				  unsigned int *itemp)
{
	return snd_amixer_elem_get_enum_item(elem, channel, itemp);
}

/**
 * \brief set the current selected enumerated item for the given mixer simple element
 * \param elem Mixer simple element handle
 * \param channel mixer simple element channel identifier
 * \param item the enumerated item index
 * \return 0 if successful, otherwise a negative error code
 */
int snd_mixer_selem_set_enum_item(snd_mixer_elem_t *elem,
				  snd_mixer_selem_channel_id_t channel,
				  unsigned int item)
{
	return snd_amixer_elem_set_enum_item(elem, channel, item);
}

/**
 * \brief get size of #snd_mixer_selem_id_t
 * \return size in bytes
 */
size_t snd_mixer_selem_id_sizeof(void)
{
	return snd_amixer_elem_id_sizeof();
}

/**
 * \brief allocate an invalid #snd_mixer_selem_id_t using standard malloc
 * \param ptr returned pointer
 * \return 0 on success otherwise negative error code
 */
int snd_mixer_selem_id_malloc(snd_mixer_selem_id_t **ptr)
{
	return snd_amixer_elem_id_malloc(ptr);
}

/**
 * \brief frees a previously allocated #snd_mixer_selem_id_t
 * \param obj pointer to object to free
 */
void snd_mixer_selem_id_free(snd_mixer_selem_id_t *obj)
{
	return snd_amixer_elem_id_free(obj);
}

/**
 * \brief copy one #snd_mixer_selem_id_t to another
 * \param dst pointer to destination
 * \param src pointer to source
 */
void snd_mixer_selem_id_copy(snd_mixer_selem_id_t *dst, const snd_mixer_selem_id_t *src)
{
	return snd_amixer_elem_id_copy(dst, src);
}

/**
 * \brief Get name part of a mixer simple element identifier
 * \param obj Mixer simple element identifier
 * \return name part
 */
const char *snd_mixer_selem_id_get_name(const snd_mixer_selem_id_t *obj)
{
	return snd_amixer_elem_id_get_name(obj);
}

/**
 * \brief Get index part of a mixer simple element identifier
 * \param obj Mixer simple element identifier
 * \return index part
 */
unsigned int snd_mixer_selem_id_get_index(const snd_mixer_selem_id_t *obj)
{
	return snd_amixer_elem_id_get_index(obj);
}

/**
 * \brief Set name part of a mixer simple element identifier
 * \param obj Mixer simple element identifier
 * \param val name part
 */
void snd_mixer_selem_id_set_name(snd_mixer_selem_id_t *obj, const char *val)
{
	return snd_amixer_elem_id_set_name(obj, val);
}

/**
 * \brief Set index part of a mixer simple element identifier
 * \param obj Mixer simple element identifier
 * \param val index part
 */
void snd_mixer_selem_id_set_index(snd_mixer_selem_id_t *obj, unsigned int val)
{
	return snd_amixer_elem_id_set_index(obj, val);
}
