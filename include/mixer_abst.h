/**
 * \file include/mixer_abst.h
 * \brief Mixer abstract implementation interface library for the ALSA library
 * \author Jaroslav Kysela <perex@perex.cz>
 * \date 2005-2008
 *
 * Mixer abstact implementation interface library for the ALSA library
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

#ifndef __ALSA_MIXER_ABST_H
#define __ALSA_MIXER_ABST_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 *  \defgroup Mixer_Abstract Mixer Abstact Module Interface
 *  The mixer abstact module interface.
 *  \{
 */

#define SM_CTL_COUNT		8

#define	SM_PLAY			SND_MIXER_DIR_PLAYBACK
#define SM_CAPT			SND_MIXER_DIR_CAPTURE
#define SM_COMM			SND_MIXER_DIR_COMMON

#define SM_CAP_GVOLUME		(1<<1)
#define SM_CAP_GSWITCH		(1<<2)
#define SM_CAP_PVOLUME		(1<<3)
#define SM_CAP_PVOLUME_JOIN	(1<<4)
#define SM_CAP_PSWITCH		(1<<5) 
#define SM_CAP_PSWITCH_JOIN	(1<<6) 
#define SM_CAP_CVOLUME		(1<<7) 
#define SM_CAP_CVOLUME_JOIN	(1<<8) 
#define SM_CAP_CSWITCH		(1<<9) 
#define SM_CAP_CSWITCH_JOIN	(1<<10)
#define SM_CAP_CSWITCH_EXCL	(1<<11)
#define SM_CAP_PENUM		(1<<12)
#define SM_CAP_CENUM		(1<<13)
/* SM_CAP_* 24-31 => private for module use */

#define SM_OPS_IS_ACTIVE	0
#define SM_OPS_IS_CHANNEL	1
#define SM_OPS_IS_ENUMERATED	2
#define SM_OPS_IS_ENUMCNT	3

typedef struct _sm_elem {
	snd_amixer_elem_id_t id;
	struct sm_elem_ops *ops;
	unsigned int caps;
	unsigned int capture_group;
} sm_elem_t;

struct sm_elem_ops {	
	int (*is)(snd_amixer_elem_t *elem, snd_amixer_dir_t dir, int cmd, int val);
	int (*get_channels)(snd_amixer_elem_t *elem, snd_amixer_dir_t dir);
	int (*get_range)(snd_amixer_elem_t *elem, snd_amixer_dir_t dir, long *min, long *max);
	int (*set_range)(snd_amixer_elem_t *elem, snd_amixer_dir_t dir, long min, long max);
	int (*get_dB_range)(snd_amixer_elem_t *elem, snd_amixer_dir_t dir, long *min, long *max);
	int (*ask_vol_dB)(snd_amixer_elem_t *elem, snd_amixer_dir_t dir, long value, long *dbValue);
	int (*ask_dB_vol)(snd_amixer_elem_t *elem, snd_amixer_dir_t dir, long dbValue, long *value, int xdir);
	int (*get_volume)(snd_amixer_elem_t *elem, snd_amixer_dir_t dir, snd_amixer_elem_channel_id_t channel, long *value);
	int (*get_dB)(snd_amixer_elem_t *elem, snd_amixer_dir_t dir, snd_amixer_elem_channel_id_t channel, long *value);
	int (*set_volume)(snd_amixer_elem_t *elem, snd_amixer_dir_t dir, snd_amixer_elem_channel_id_t channel, long value);
	int (*set_dB)(snd_amixer_elem_t *elem, snd_amixer_dir_t dir, snd_amixer_elem_channel_id_t channel, long value, int xdir);
	int (*get_switch)(snd_amixer_elem_t *elem, snd_amixer_dir_t dir, snd_amixer_elem_channel_id_t channel, int *value);
	int (*set_switch)(snd_amixer_elem_t *elem, snd_amixer_dir_t dir, snd_amixer_elem_channel_id_t channel, int value);
	int (*enum_item_name)(snd_amixer_elem_t *elem, unsigned int item, size_t maxlen, char *buf);
	int (*get_enum_item)(snd_amixer_elem_t *elem, snd_amixer_elem_channel_id_t channel, unsigned int *itemp);
	int (*set_enum_item)(snd_amixer_elem_t *elem, snd_amixer_elem_channel_id_t channel, unsigned int item);
};

struct sm_open {
	const char *name;
	snd_pcm_t *pcm_playback;
	snd_pcm_t *pcm_capture;
	int mode;
	snd_ctl_t *ctl[SM_CTL_COUNT];
};

sm_elem_t *snd_amixer_elem_get_sm(snd_amixer_elem_t *elem);

/** \} */

#ifdef __cplusplus
}
#endif

#endif /* __ALSA_MIXER_ABST_H */
