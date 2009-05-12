/**
 * \file include/mixer_old.h
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

#ifndef __ALSA_MIXER_OLD_H
#define __ALSA_MIXER_OLD_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 *  \defgroup HControl High level Control Interface
 *  \ingroup Control
 *  The high level control interface.
 *  See \ref hcontrol page for more details.
 *  \{
 */

/** HCTL element handle */
#define snd_hctl_elem_t snd_ctl_elem_t

/** HCTL handle */
#define snd_hctl_t snd_ctl_t

/**
 * \brief Compare function for sorting HCTL elements
 * \param e1 First element
 * \param e2 Second element
 * \return -1 if e1 < e2, 0 if e1 == e2, 1 if e1 > e2
 */
#define snd_hctl_compare_t snd_ctl_compare_t
#define snd_hctl_compare_fast snd_ctl_compare_fast

/** 
 * \brief HCTL callback function
 * \param hctl HCTL handle
 * \param mask event mask
 * \param elem related HCTL element (if any)
 * \return 0 on success otherwise a negative error code
 */
#define snd_hctl_callback_t snd_ctl_callback_t

/** 
 * \brief HCTL element callback function
 * \param elem HCTL element
 * \param mask event mask
 * \return 0 on success otherwise a negative error code
 */
#define snd_hctl_elem_callback_t snd_ctl_elem_callback_t

int snd_hctl_open(snd_hctl_t **hctl, const char *name, int mode);
int snd_hctl_open_ctl(snd_hctl_t **hctlp, snd_ctl_t *ctl);
int snd_hctl_close(snd_hctl_t *hctl);
int snd_hctl_nonblock(snd_hctl_t *hctl, int nonblock);
int snd_hctl_poll_descriptors_count(snd_hctl_t *hctl);
int snd_hctl_poll_descriptors(snd_hctl_t *hctl, struct pollfd *pfds, unsigned int space);
int snd_hctl_poll_descriptors_revents(snd_hctl_t *ctl, struct pollfd *pfds, unsigned int nfds, unsigned short *revents);
unsigned int snd_hctl_get_count(snd_hctl_t *hctl);
int snd_hctl_set_compare(snd_hctl_t *hctl, snd_hctl_compare_t hsort);
snd_hctl_elem_t *snd_hctl_first_elem(snd_hctl_t *hctl);
snd_hctl_elem_t *snd_hctl_last_elem(snd_hctl_t *hctl);
snd_hctl_elem_t *snd_hctl_find_elem(snd_hctl_t *hctl, const snd_ctl_elem_id_t *id);
void snd_hctl_set_callback(snd_hctl_t *hctl, snd_hctl_callback_t callback);
void snd_hctl_set_callback_private(snd_hctl_t *hctl, void *data);
void *snd_hctl_get_callback_private(snd_hctl_t *hctl);
int snd_hctl_load(snd_hctl_t *hctl);
int snd_hctl_free(snd_hctl_t *hctl);
int snd_hctl_handle_events(snd_hctl_t *hctl);
const char *snd_hctl_name(snd_hctl_t *hctl);
int snd_hctl_wait(snd_hctl_t *hctl, int timeout);
snd_ctl_t *snd_hctl_ctl(snd_hctl_t *hctl);

snd_hctl_elem_t *snd_hctl_elem_next(snd_hctl_elem_t *elem);
snd_hctl_elem_t *snd_hctl_elem_prev(snd_hctl_elem_t *elem);
int snd_hctl_elem_info(snd_hctl_elem_t *elem, snd_ctl_elem_info_t * info);
int snd_hctl_elem_read(snd_hctl_elem_t *elem, snd_ctl_elem_value_t * value);
int snd_hctl_elem_write(snd_hctl_elem_t *elem, snd_ctl_elem_value_t * value);
int snd_hctl_elem_tlv_read(snd_hctl_elem_t *elem, unsigned int *tlv, unsigned int tlv_size);
int snd_hctl_elem_tlv_write(snd_hctl_elem_t *elem, const unsigned int *tlv);
int snd_hctl_elem_tlv_command(snd_hctl_elem_t *elem, const unsigned int *tlv);

snd_hctl_t *snd_hctl_elem_get_hctl(snd_hctl_elem_t *elem);

void snd_hctl_elem_get_id(const snd_hctl_elem_t *obj, snd_ctl_elem_id_t *ptr);
unsigned int snd_hctl_elem_get_numid(const snd_hctl_elem_t *obj);
snd_ctl_elem_iface_t snd_hctl_elem_get_interface(const snd_hctl_elem_t *obj);
unsigned int snd_hctl_elem_get_device(const snd_hctl_elem_t *obj);
unsigned int snd_hctl_elem_get_subdevice(const snd_hctl_elem_t *obj);
const char *snd_hctl_elem_get_name(const snd_hctl_elem_t *obj);
unsigned int snd_hctl_elem_get_index(const snd_hctl_elem_t *obj);
void snd_hctl_elem_set_callback(snd_hctl_elem_t *obj, snd_hctl_elem_callback_t val);
void * snd_hctl_elem_get_callback_private(const snd_hctl_elem_t *obj);
void snd_hctl_elem_set_callback_private(snd_hctl_elem_t *obj, void * val);

/** \} */

/** \} */

/**
 *  \defgroup MixerOld Mixer Interface
 *  The mixer interface.
 *  \{
 */

/** Mixer handle */
typedef struct _snd_mixer snd_mixer_t;
/** Mixer element handle */
#define snd_mixer_elem_t snd_amixer_elem_t

/** 
 * \brief Mixer callback function
 * \param mixer Mixer handle
 * \param mask event mask
 * \param elem related mixer element (if any)
 * \return 0 on success otherwise a negative error code
 */
typedef int (*snd_mixer_callback_t)(snd_mixer_t *ctl,
				    unsigned int mask,
				    snd_mixer_elem_t *elem);

/** 
 * \brief Mixer element callback function
 * \param elem Mixer element
 * \param mask event mask
 * \return 0 on success otherwise a negative error code
 */
typedef int (*snd_mixer_elem_callback_t)(snd_mixer_elem_t *elem,
					 unsigned int mask);

/**
 * \brief Compare function for sorting mixer elements
 * \param e1 First element
 * \param e2 Second element
 * \return -1 if e1 < e2, 0 if e1 == e2, 1 if e1 > e2
 */
typedef int (*snd_mixer_compare_t)(const snd_mixer_elem_t *e1,
				   const snd_mixer_elem_t *e2);

/** Mixer element type */
typedef enum _snd_mixer_elem_type {
	/* Simple mixer elements */
	SND_MIXER_ELEM_SIMPLE,
	SND_MIXER_ELEM_LAST = SND_MIXER_ELEM_SIMPLE
} snd_mixer_elem_type_t;

int snd_mixer_open(snd_mixer_t **mixer, int mode);
int snd_mixer_close(snd_mixer_t *mixer);
snd_mixer_elem_t *snd_mixer_first_elem(snd_mixer_t *mixer);
snd_mixer_elem_t *snd_mixer_last_elem(snd_mixer_t *mixer);
int snd_mixer_handle_events(snd_mixer_t *mixer);
int snd_mixer_attach(snd_mixer_t *mixer, const char *name);
int snd_mixer_attach_hctl(snd_mixer_t *mixer, snd_hctl_t *hctl);
int snd_mixer_detach(snd_mixer_t *mixer, const char *name);
int snd_mixer_detach_hctl(snd_mixer_t *mixer, snd_hctl_t *hctl);
int snd_mixer_get_hctl(snd_mixer_t *mixer, const char *name, snd_hctl_t **hctl);
int snd_mixer_poll_descriptors_count(snd_mixer_t *mixer);
int snd_mixer_poll_descriptors(snd_mixer_t *mixer, struct pollfd *pfds, unsigned int space);
int snd_mixer_poll_descriptors_revents(snd_mixer_t *mixer, struct pollfd *pfds, unsigned int nfds, unsigned short *revents);
int snd_mixer_load(snd_mixer_t *mixer);
void snd_mixer_free(snd_mixer_t *mixer);
int snd_mixer_wait(snd_mixer_t *mixer, int timeout);
int snd_mixer_set_compare(snd_mixer_t *mixer, snd_mixer_compare_t msort);
void snd_mixer_set_callback(snd_mixer_t *obj, snd_mixer_callback_t val);
void * snd_mixer_get_callback_private(const snd_mixer_t *obj);
void snd_mixer_set_callback_private(snd_mixer_t *obj, void * val);
unsigned int snd_mixer_get_count(const snd_mixer_t *obj);

snd_mixer_elem_t *snd_mixer_elem_next(snd_mixer_elem_t *elem);
snd_mixer_elem_t *snd_mixer_elem_prev(snd_mixer_elem_t *elem);
void snd_mixer_elem_set_callback(snd_mixer_elem_t *obj, snd_mixer_elem_callback_t val);
void * snd_mixer_elem_get_callback_private(const snd_mixer_elem_t *obj);
void snd_mixer_elem_set_callback_private(snd_mixer_elem_t *obj, void * val);
snd_mixer_elem_type_t snd_mixer_elem_get_type(const snd_mixer_elem_t *obj);

/**
 *  \defgroup SimpleMixer Simple Mixer Interface
 *  \ingroup Mixer
 *  The simple mixer interface.
 *  \{
 */

/* Simple mixer elements API */

/** Mixer simple element channel identifier */
#define snd_mixer_selem_channel_id_t snd_amixer_elem_channel_id_t

/** Mixer simple element - register options - abstraction level */
enum snd_mixer_selem_regopt_abstract {
	/** no abstraction - try use all universal controls from driver */
	SND_MIXER_SABSTRACT_NONE = 0,
	/** basic abstraction - Master,PCM,CD,Aux,Record-Gain etc. */
	SND_MIXER_SABSTRACT_BASIC,
};

/** Mixer simple element - register options */
struct snd_mixer_selem_regopt {
	/** structure version */
	int ver;
	/** v1: abstract layer selection */
	enum snd_mixer_selem_regopt_abstract abstract;
	/** v1: device name (must be NULL when playback_pcm or capture_pcm != NULL) */
	const char *device;
	/** v1: playback PCM connected to mixer device (NULL == none) */
	snd_pcm_t *playback_pcm;
	/** v1: capture PCM connected to mixer device (NULL == none) */
	snd_pcm_t *capture_pcm;
};

/** Mixer simple element identifier */
#define snd_mixer_selem_id_t snd_amixer_elem_id_t

const char *snd_mixer_selem_channel_name(snd_mixer_selem_channel_id_t channel);

int snd_mixer_selem_register(snd_mixer_t *mixer,
			     struct snd_mixer_selem_regopt *options,
			     void **nothing);
void snd_mixer_selem_get_id(snd_mixer_elem_t *element,
			    snd_mixer_selem_id_t *id);
const char *snd_mixer_selem_get_name(snd_mixer_elem_t *elem);
unsigned int snd_mixer_selem_get_index(snd_mixer_elem_t *elem);
snd_mixer_elem_t *snd_mixer_find_selem(snd_mixer_t *mixer,
				       const snd_mixer_selem_id_t *id);

int snd_mixer_selem_is_active(snd_mixer_elem_t *elem);
int snd_mixer_selem_is_playback_mono(snd_mixer_elem_t *elem);
int snd_mixer_selem_has_playback_channel(snd_mixer_elem_t *obj, snd_mixer_selem_channel_id_t channel);
int snd_mixer_selem_is_capture_mono(snd_mixer_elem_t *elem);
int snd_mixer_selem_has_capture_channel(snd_mixer_elem_t *obj, snd_mixer_selem_channel_id_t channel);
int snd_mixer_selem_get_capture_group(snd_mixer_elem_t *elem);
int snd_mixer_selem_has_common_volume(snd_mixer_elem_t *elem);
int snd_mixer_selem_has_playback_volume(snd_mixer_elem_t *elem);
int snd_mixer_selem_has_playback_volume_joined(snd_mixer_elem_t *elem);
int snd_mixer_selem_has_capture_volume(snd_mixer_elem_t *elem);
int snd_mixer_selem_has_capture_volume_joined(snd_mixer_elem_t *elem);
int snd_mixer_selem_has_common_switch(snd_mixer_elem_t *elem);
int snd_mixer_selem_has_playback_switch(snd_mixer_elem_t *elem);
int snd_mixer_selem_has_playback_switch_joined(snd_mixer_elem_t *elem);
int snd_mixer_selem_has_capture_switch(snd_mixer_elem_t *elem);
int snd_mixer_selem_has_capture_switch_joined(snd_mixer_elem_t *elem);
int snd_mixer_selem_has_capture_switch_exclusive(snd_mixer_elem_t *elem);

int snd_mixer_selem_ask_playback_vol_dB(snd_mixer_elem_t *elem, long value, long *dBvalue);
int snd_mixer_selem_ask_capture_vol_dB(snd_mixer_elem_t *elem, long value, long *dBvalue);
int snd_mixer_selem_ask_playback_dB_vol(snd_mixer_elem_t *elem, long dBvalue, int dir, long *value);
int snd_mixer_selem_ask_capture_dB_vol(snd_mixer_elem_t *elem, long dBvalue, int dir, long *value);
int snd_mixer_selem_get_playback_volume(snd_mixer_elem_t *elem, snd_mixer_selem_channel_id_t channel, long *value);
int snd_mixer_selem_get_capture_volume(snd_mixer_elem_t *elem, snd_mixer_selem_channel_id_t channel, long *value);
int snd_mixer_selem_get_playback_dB(snd_mixer_elem_t *elem, snd_mixer_selem_channel_id_t channel, long *value);
int snd_mixer_selem_get_capture_dB(snd_mixer_elem_t *elem, snd_mixer_selem_channel_id_t channel, long *value);
int snd_mixer_selem_get_playback_switch(snd_mixer_elem_t *elem, snd_mixer_selem_channel_id_t channel, int *value);
int snd_mixer_selem_get_capture_switch(snd_mixer_elem_t *elem, snd_mixer_selem_channel_id_t channel, int *value);
int snd_mixer_selem_set_playback_volume(snd_mixer_elem_t *elem, snd_mixer_selem_channel_id_t channel, long value);
int snd_mixer_selem_set_capture_volume(snd_mixer_elem_t *elem, snd_mixer_selem_channel_id_t channel, long value);
int snd_mixer_selem_set_playback_dB(snd_mixer_elem_t *elem, snd_mixer_selem_channel_id_t channel, long value, int dir);
int snd_mixer_selem_set_capture_dB(snd_mixer_elem_t *elem, snd_mixer_selem_channel_id_t channel, long value, int dir);
int snd_mixer_selem_set_playback_volume_all(snd_mixer_elem_t *elem, long value);
int snd_mixer_selem_set_capture_volume_all(snd_mixer_elem_t *elem, long value);
int snd_mixer_selem_set_playback_dB_all(snd_mixer_elem_t *elem, long value, int dir);
int snd_mixer_selem_set_capture_dB_all(snd_mixer_elem_t *elem, long value, int dir);
int snd_mixer_selem_set_playback_switch(snd_mixer_elem_t *elem, snd_mixer_selem_channel_id_t channel, int value);
int snd_mixer_selem_set_capture_switch(snd_mixer_elem_t *elem, snd_mixer_selem_channel_id_t channel, int value);
int snd_mixer_selem_set_playback_switch_all(snd_mixer_elem_t *elem, int value);
int snd_mixer_selem_set_capture_switch_all(snd_mixer_elem_t *elem, int value);
int snd_mixer_selem_get_playback_volume_range(snd_mixer_elem_t *elem, 
					      long *min, long *max);
int snd_mixer_selem_get_playback_dB_range(snd_mixer_elem_t *elem, 
					  long *min, long *max);
int snd_mixer_selem_set_playback_volume_range(snd_mixer_elem_t *elem, 
					      long min, long max);
int snd_mixer_selem_get_capture_volume_range(snd_mixer_elem_t *elem, 
					     long *min, long *max);
int snd_mixer_selem_get_capture_dB_range(snd_mixer_elem_t *elem, 
					 long *min, long *max);
int snd_mixer_selem_set_capture_volume_range(snd_mixer_elem_t *elem, 
					     long min, long max);

int snd_mixer_selem_is_enumerated(snd_mixer_elem_t *elem);
int snd_mixer_selem_is_enum_playback(snd_mixer_elem_t *elem);
int snd_mixer_selem_is_enum_capture(snd_mixer_elem_t *elem);
int snd_mixer_selem_get_enum_items(snd_mixer_elem_t *elem);
int snd_mixer_selem_get_enum_item_name(snd_mixer_elem_t *elem, unsigned int idx, size_t maxlen, char *str);
int snd_mixer_selem_get_enum_item(snd_mixer_elem_t *elem, snd_mixer_selem_channel_id_t channel, unsigned int *idxp);
int snd_mixer_selem_set_enum_item(snd_mixer_elem_t *elem, snd_mixer_selem_channel_id_t channel, unsigned int idx);

size_t snd_mixer_selem_id_sizeof(void);
/** \hideinitializer
 * \brief allocate an invalid #snd_mixer_selem_id_t using standard alloca
 * \param ptr returned pointer
 */
#define snd_mixer_selem_id_alloca(ptr) __snd_alloca(ptr, snd_mixer_selem_id)
int snd_mixer_selem_id_malloc(snd_mixer_selem_id_t **ptr);
void snd_mixer_selem_id_free(snd_mixer_selem_id_t *obj);
void snd_mixer_selem_id_copy(snd_mixer_selem_id_t *dst, const snd_mixer_selem_id_t *src);
const char *snd_mixer_selem_id_get_name(const snd_mixer_selem_id_t *obj);
unsigned int snd_mixer_selem_id_get_index(const snd_mixer_selem_id_t *obj);
void snd_mixer_selem_id_set_name(snd_mixer_selem_id_t *obj, const char *val);
void snd_mixer_selem_id_set_index(snd_mixer_selem_id_t *obj, unsigned int val);

/** \} */

/** \} */

#ifdef __cplusplus
}
#endif

#endif /* __ALSA_MIXER_OLD_H */
