/*
 * \file include/mixer.h
 * \brief Application interface library for the ALSA driver
 * \author Jaroslav Kysela <perex@perex.cz>
 * \author Abramo Bagnara <abramo@alsa-project.org>
 * \author Takashi Iwai <tiwai@suse.de>
 * \date 1998-2009
 *
 * Application interface library for the ALSA driver
 */
/*
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

#ifndef __ALSA_MIXER_H
#define __ALSA_MIXER_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 *  \defgroup Mixer Mixer Interface
 *  The amixer interface.
 *  \{
 */

/* AMixer elements API */

/** dlsym version for interface entry callback */
#define SND_AMIXER_DLSYM_VERSION	_dlsym_amixer_001

/** AMixer element operation identifier */
typedef enum _snd_amixer_elem_type {
	/** Playback */
	SND_MIXER_DIR_PLAYBACK = 0,
	/** Capture */
	SND_MIXER_DIR_CAPTURE = 1,
	/** Common - playback and capture directions are identical */
	SND_MIXER_DIR_COMMON = 2,
} snd_amixer_dir_t;

/** AMixer element channel identifier */
typedef enum _snd_amixer_elem_channel_id {
	/** Unknown */
	SND_MIXER_SCHN_UNKNOWN = -1,
	/** Front left */
	SND_MIXER_SCHN_FRONT_LEFT = 0,
	/** Front right */
	SND_MIXER_SCHN_FRONT_RIGHT,
	/** Rear left */
	SND_MIXER_SCHN_REAR_LEFT,
	/** Rear right */
	SND_MIXER_SCHN_REAR_RIGHT,
	/** Front center */
	SND_MIXER_SCHN_FRONT_CENTER,
	/** Woofer */
	SND_MIXER_SCHN_WOOFER,
	/** Side Left */
	SND_MIXER_SCHN_SIDE_LEFT,
	/** Side Right */
	SND_MIXER_SCHN_SIDE_RIGHT,
	/** Rear Center */
	SND_MIXER_SCHN_REAR_CENTER,
	SND_MIXER_SCHN_LAST = 31,
	/** Mono (Front left alias) */
	SND_MIXER_SCHN_MONO = SND_MIXER_SCHN_FRONT_LEFT
} snd_amixer_elem_channel_id_t;

/** Mixer handle */
typedef struct _snd_amixer snd_amixer_t;
/** Mixer element handle */
typedef struct _snd_amixer_elem snd_amixer_elem_t;
/** Mixer simple element identifier */
typedef struct _snd_amixer_elem_id {
	char name[60];
	unsigned int index;
} snd_amixer_elem_id_t;

/** 
 * \brief Mixer callback function
 * \param amixer Mixer handle
 * \param mask event mask
 * \param elem related amixer element (if any)
 * \return 0 on success otherwise a negative error code
 */
typedef int (*snd_amixer_callback_t)(snd_amixer_t *ctl,
				    unsigned int mask,
				    snd_amixer_elem_t *elem);

/** 
 * \brief Mixer element callback function
 * \param elem Mixer element
 * \param mask event mask
 * \return 0 on success otherwise a negative error code
 */
typedef int (*snd_amixer_elem_callback_t)(snd_amixer_elem_t *elem,
					 unsigned int mask);

/**
 * \brief Compare function for sorting amixer elements
 * \param e1 First element
 * \param e2 Second element
 * \return -1 if e1 < e2, 0 if e1 == e2, 1 if e1 > e2
 */
typedef int (*snd_amixer_compare_t)(const snd_amixer_elem_t *e1,
				    const snd_amixer_elem_t *e2);

/**
 * \brief Event callback for the amixer class
 * \param amixer mixer handle
 * \param mask Event mask (SND_CTL_EVENT_*)
 * \param celem CTL element which invoked the event
 * \param melem Mixer element associated to CTL element
 * \return zero if success, otherwise a negative error value
 */
typedef int (*snd_amixer_event_t)(snd_amixer_t *amixer,
				  unsigned int mask,
				  snd_ctl_elem_t *celem,
				  snd_amixer_elem_t *melem);

/** Expose all mixer controls (flag for open mode) \hideinitializer */
#define SND_AMIXER_ALL		0x00000002
/** Compatibility mode for older selem API (flag for open mode) \hideinitializer */
#define SND_AMIXER_COMPAT1	0x40000000

int snd_amixer_open(snd_amixer_t **amixer, const char *name,
		    snd_pcm_t *playback_pcm, snd_pcm_t *capture_pcm,
		    int mode);
int snd_amixer_close(snd_amixer_t *amixer);
int snd_amixer_handle_events(snd_amixer_t *amixer);
int snd_amixer_poll_descriptors_count(snd_amixer_t *amixer);
int snd_amixer_poll_descriptors(snd_amixer_t *amixer, struct pollfd *pfds, unsigned int space);
int snd_amixer_poll_descriptors_revents(snd_amixer_t *amixer, struct pollfd *pfds, unsigned int nfds, unsigned short *revents);
int snd_amixer_wait(snd_amixer_t *amixer, int timeout);
int snd_amixer_set_compare(snd_amixer_t *amixer, snd_amixer_compare_t msort);
void snd_amixer_set_callback(snd_amixer_t *obj, snd_amixer_callback_t val);
void *snd_amixer_get_callback_private(const snd_amixer_t *obj);
void snd_amixer_set_callback_private(snd_amixer_t *obj, void *val);
void snd_amixer_set_event(snd_amixer_t *amixer, snd_amixer_event_t event);
snd_amixer_event_t snd_amixer_get_event(const snd_amixer_t *amixer);
unsigned int snd_amixer_get_count(const snd_amixer_t *obj);
void snd_amixer_set_private(const snd_amixer_t *amixer, void *private_data);
void snd_amixer_set_private_free(const snd_amixer_t *amixer, void (*private_free)(snd_amixer_t *amixer));
void *snd_amixer_get_private(const snd_amixer_t *amixer);
int snd_amixer_conf_generic_id(const char *id);

int snd_amixer_compare_default(const snd_amixer_elem_t *c1, const snd_amixer_elem_t *c2);

snd_amixer_elem_t *snd_amixer_first_elem(snd_amixer_t *amixer);
snd_amixer_elem_t *snd_amixer_last_elem(snd_amixer_t *amixer);
snd_amixer_elem_t *snd_amixer_elem_next(snd_amixer_elem_t *elem);
snd_amixer_elem_t *snd_amixer_elem_prev(snd_amixer_elem_t *elem);
void snd_amixer_elem_set_callback(snd_amixer_elem_t *obj, snd_amixer_elem_callback_t val);
void * snd_amixer_elem_get_callback_private(const snd_amixer_elem_t *obj);
void snd_amixer_elem_set_callback_private(snd_amixer_elem_t *obj, void * val);

int snd_amixer_add_elem(snd_amixer_t *amixer, snd_amixer_elem_t *elem);
int snd_amixer_remove_elem(snd_amixer_t *amixer, snd_amixer_elem_t *elem);
int snd_amixer_elem_new(snd_amixer_t *amixer,
			snd_amixer_elem_t **elem,
			snd_amixer_elem_id_t *id,
			int compare_weight,
			void *private_data,
			void (*private_free)(snd_amixer_elem_t *elem));
int snd_amixer_elem_add(snd_amixer_t *amixer, snd_amixer_elem_t *elem);
int snd_amixer_elem_remove(snd_amixer_elem_t *elem);
void snd_amixer_elem_free(snd_amixer_elem_t *elem);
int snd_amixer_elem_info(snd_amixer_elem_t *elem);
int snd_amixer_elem_value(snd_amixer_elem_t *elem);
int snd_amixer_elem_attach(snd_amixer_elem_t *melem, snd_ctl_elem_t *helem);
int snd_amixer_elem_detach(snd_amixer_elem_t *melem, snd_ctl_elem_t *helem);
int snd_amixer_elem_is_empty(snd_amixer_elem_t *melem);
void *snd_amixer_elem_get_private(const snd_amixer_elem_t *melem);

const char *snd_amixer_elem_channel_name(snd_amixer_elem_channel_id_t channel);

void snd_amixer_elem_get_id(snd_amixer_elem_t *element,
			    snd_amixer_elem_id_t *id);
const char *snd_amixer_elem_get_name(snd_amixer_elem_t *elem);
unsigned int snd_amixer_elem_get_index(snd_amixer_elem_t *elem);
snd_amixer_elem_t *snd_amixer_find_elem(snd_amixer_t *amixer,
					const snd_amixer_elem_id_t *id);

int snd_amixer_elem_is_active(snd_amixer_elem_t *elem);
int snd_amixer_elem_has_channel(snd_amixer_elem_t *obj, snd_amixer_dir_t dir, snd_amixer_elem_channel_id_t channel);
int snd_amixer_elem_has_volume(snd_amixer_elem_t *elem, snd_amixer_dir_t dir);
int snd_amixer_elem_has_volume_joined(snd_amixer_elem_t *elem, snd_amixer_dir_t dir);
int snd_amixer_elem_has_switch(snd_amixer_elem_t *elem, snd_amixer_dir_t dir);
int snd_amixer_elem_has_switch_joined(snd_amixer_elem_t *elem, snd_amixer_dir_t dir);

int snd_amixer_elem_get_group(snd_amixer_elem_t *elem, snd_amixer_dir_t dir);
int snd_amixer_elem_has_switch_exclusive(snd_amixer_elem_t *elem, snd_amixer_dir_t dir);

int snd_amixer_elem_ask_vol_dB(snd_amixer_elem_t *elem, snd_amixer_dir_t dir, long value, long *dBvalue);
int snd_amixer_elem_ask_dB_vol(snd_amixer_elem_t *elem, snd_amixer_dir_t dir, long dBvalue, int xdir, long *value);
int snd_amixer_elem_get_channels(snd_amixer_elem_t *elem, snd_amixer_dir_t dir);
int snd_amixer_elem_get_volume(snd_amixer_elem_t *elem, snd_amixer_dir_t dir, snd_amixer_elem_channel_id_t channel, long *value);
int snd_amixer_elem_get_dB(snd_amixer_elem_t *elem, snd_amixer_dir_t dir, snd_amixer_elem_channel_id_t channel, long *value);
int snd_amixer_elem_get_switch(snd_amixer_elem_t *elem, snd_amixer_dir_t dir, snd_amixer_elem_channel_id_t channel, int *value);
int snd_amixer_elem_get_volume_range(snd_amixer_elem_t *elem, snd_amixer_dir_t dir, long *min, long *max);
int snd_amixer_elem_get_dB_range(snd_amixer_elem_t *elem, snd_amixer_dir_t dir, long *min, long *max);
int snd_amixer_elem_set_volume(snd_amixer_elem_t *elem, snd_amixer_dir_t dir, snd_amixer_elem_channel_id_t channel, long value);
int snd_amixer_elem_set_dB(snd_amixer_elem_t *elem, snd_amixer_dir_t dir, snd_amixer_elem_channel_id_t channel, long value, int xdir);
int snd_amixer_elem_set_volume_all(snd_amixer_elem_t *elem, snd_amixer_dir_t dir, long value);
int snd_amixer_elem_set_dB_all(snd_amixer_elem_t *elem, snd_amixer_dir_t dir, long value, int xdir);
int snd_amixer_elem_set_switch(snd_amixer_elem_t *elem, snd_amixer_dir_t dir, snd_amixer_elem_channel_id_t channel, int value);
int snd_amixer_elem_set_switch_all(snd_amixer_elem_t *elem, snd_amixer_dir_t dir, int value);
int snd_amixer_elem_set_volume_range(snd_amixer_elem_t *elem, snd_amixer_dir_t dir, long min, long max);

int snd_amixer_elem_is_enum(snd_amixer_elem_t *elem, snd_amixer_dir_t dir);
int snd_amixer_elem_get_enum_items(snd_amixer_elem_t *elem);
int snd_amixer_elem_get_enum_item_name(snd_amixer_elem_t *elem, unsigned int idx, size_t maxlen, char *str);
int snd_amixer_elem_get_enum_item(snd_amixer_elem_t *elem, snd_amixer_elem_channel_id_t channel, unsigned int *idxp);
int snd_amixer_elem_set_enum_item(snd_amixer_elem_t *elem, snd_amixer_elem_channel_id_t channel, unsigned int idx);

size_t snd_amixer_elem_id_sizeof(void);
/** \hideinitializer
 * \brief allocate an invalid #snd_amixer_elem_id_t using standard alloca
 * \param ptr returned pointer
 */
#define snd_amixer_elem_id_alloca(ptr) __snd_alloca(ptr, snd_amixer_elem_id)
int snd_amixer_elem_id_malloc(snd_amixer_elem_id_t **ptr);
void snd_amixer_elem_id_free(snd_amixer_elem_id_t *obj);
void snd_amixer_elem_id_copy(snd_amixer_elem_id_t *dst, const snd_amixer_elem_id_t *src);
const char *snd_amixer_elem_id_get_name(const snd_amixer_elem_id_t *obj);
unsigned int snd_amixer_elem_id_get_index(const snd_amixer_elem_id_t *obj);
void snd_amixer_elem_id_set_name(snd_amixer_elem_id_t *obj, const char *val);
void snd_amixer_elem_id_set_index(snd_amixer_elem_id_t *obj, unsigned int val);

/** \} */

#ifdef __cplusplus
}
#endif

#endif /* __ALSA_MIXER_H */

