/**
 * \file <alsa/timer.h>
 * \brief Application interface library for the ALSA driver
 * \author Jaroslav Kysela <perex@suse.cz>
 * \author Abramo Bagnara <abramo@alsa-project.org>
 * \author Takashi Iwai <tiwai@suse.de>
 * \date 1998-2001
 *
 * Application interface library for the ALSA driver
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

#ifndef __ALSA_TIMER_H
#define __ALSA_TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 *  \defgroup Timer Timer Interface
 *  Timer Interface
 *  \{
 */

/** dlsym version for interface entry callback */
#define SND_TIMER_DLSYM_VERSION		_dlsym_timer_001
/** dlsym version for interface entry callback */
#define SND_TIMER_QUERY_DLSYM_VERSION	_dlsym_timer_query_001

/** timer identification structure */
typedef struct _snd_timer_id snd_timer_id_t;
/** timer info structure */
typedef struct _snd_timer_info snd_timer_info_t;
/** timer params structure */
typedef struct _snd_timer_params snd_timer_params_t;
/** timer status structure */
typedef struct _snd_timer_status snd_timer_status_t;
/** timer read structure */
typedef struct _snd_timer_read {
	unsigned int resolution;	/**< tick resolution in nanoseconds */
        unsigned int ticks;		/**< count of happened ticks */
} snd_timer_read_t;

/** timer master class */
typedef enum _snd_timer_class {
	SND_TIMER_CLASS_NONE = -1,	/**< invalid */
	SND_TIMER_CLASS_SLAVE = 0,	/**< slave timer */
	SND_TIMER_CLASS_GLOBAL,		/**< global timer */
	SND_TIMER_CLASS_CARD,		/**< card timer */
	SND_TIMER_CLASS_PCM,		/**< PCM timer */
	SND_TIMER_CLASS_LAST = SND_TIMER_CLASS_PCM	/**< last timer */
} snd_timer_class_t;

/** timer slave class */
typedef enum _snd_timer_slave_class {
	SND_TIMER_SCLASS_NONE = 0,		/**< none */
	SND_TIMER_SCLASS_APPLICATION,		/**< for internal use */
	SND_TIMER_SCLASS_SEQUENCER,		/**< sequencer timer */
	SND_TIMER_SCLASS_OSS_SEQUENCER,		/**< OSS sequencer timer */
	SND_TIMER_SCLASS_LAST = SND_TIMER_SCLASS_OSS_SEQUENCER	/**< last slave timer */
} snd_timer_slave_class_t;

/** global timer - system */
#define SND_TIMER_GLOBAL_SYSTEM 0
/** global timer - RTC */
#define SND_TIMER_GLOBAL_RTC 	1

/** timer open mode flag - nonblocking behaviour */
#define SND_TIMER_OPEN_NONBLOCK		0x0001

/** timer handle type */
typedef enum _snd_timer_type {
	/** Kernel level HwDep */
	SND_TIMER_TYPE_HW = 0,
	/** Shared memory client timer (not yet implemented) */
	SND_TIMER_TYPE_SHM,
	/** INET client timer (not yet implemented) */
	SND_TIMER_TYPE_INET
} snd_timer_type_t;

/** timer query handle */
typedef struct _snd_timer_query snd_timer_query_t;
/** timer handle */
typedef struct _snd_timer snd_timer_t;


int snd_timer_query_open(snd_timer_query_t **handle, const char *name, int mode);
int snd_timer_query_open_lconf(snd_timer_query_t **handle, const char *name, int mode, snd_config_t *lconf);
int snd_timer_query_close(snd_timer_query_t *handle);
int snd_timer_query_next_device(snd_timer_query_t *handle, snd_timer_id_t *tid);

int snd_timer_open(snd_timer_t **handle, const char *name, int mode);
int snd_timer_open_lconf(snd_timer_t **handle, const char *name, int mode, snd_config_t *lconf);
int snd_timer_close(snd_timer_t *handle);
int snd_timer_poll_descriptors_count(snd_timer_t *handle);
int snd_timer_poll_descriptors(snd_timer_t *handle, struct pollfd *pfds, unsigned int space);
int snd_timer_poll_descriptors_revents(snd_timer_t *timer, struct pollfd *pfds, unsigned int nfds, unsigned short *revents);
int snd_timer_info(snd_timer_t *handle, snd_timer_info_t *timer);
int snd_timer_params(snd_timer_t *handle, snd_timer_params_t *params);
int snd_timer_status(snd_timer_t *handle, snd_timer_status_t *status);
int snd_timer_start(snd_timer_t *handle);
int snd_timer_stop(snd_timer_t *handle);
int snd_timer_continue(snd_timer_t *handle);
ssize_t snd_timer_read(snd_timer_t *handle, void *buffer, size_t size);

size_t snd_timer_id_sizeof(void);
/** allocate #snd_timer_id_t container on stack */
#define snd_timer_id_alloca(ptr) do { assert(ptr); *ptr = (snd_timer_id_t *) alloca(snd_timer_id_sizeof()); memset(*ptr, 0, snd_timer_id_sizeof()); } while (0)
int snd_timer_id_malloc(snd_timer_id_t **ptr);
void snd_timer_id_free(snd_timer_id_t *obj);
void snd_timer_id_copy(snd_timer_id_t *dst, const snd_timer_id_t *src);

void snd_timer_id_set_class(snd_timer_id_t *id, int dev_class);
int snd_timer_id_get_class(snd_timer_id_t *id);
void snd_timer_id_set_sclass(snd_timer_id_t *id, int dev_sclass);
int snd_timer_id_get_sclass(snd_timer_id_t *id);
void snd_timer_id_set_card(snd_timer_id_t *id, int card);
int snd_timer_id_get_card(snd_timer_id_t *id);
void snd_timer_id_set_device(snd_timer_id_t *id, int device);
int snd_timer_id_get_device(snd_timer_id_t *id);
void snd_timer_id_set_subdevice(snd_timer_id_t *id, int subdevice);
int snd_timer_id_get_subdevice(snd_timer_id_t *id);

size_t snd_timer_info_sizeof(void);
/** allocate #snd_timer_info_t container on stack */
#define snd_timer_info_alloca(ptr) do { assert(ptr); *ptr = (snd_timer_info_t *) alloca(snd_timer_info_sizeof()); memset(*ptr, 0, snd_timer_info_sizeof()); } while (0)
int snd_timer_info_malloc(snd_timer_info_t **ptr);
void snd_timer_info_free(snd_timer_info_t *obj);
void snd_timer_info_copy(snd_timer_info_t *dst, const snd_timer_info_t *src);

int snd_timer_info_is_slave(snd_timer_info_t * info);
int snd_timer_info_get_card(snd_timer_info_t * info);
const char *snd_timer_info_get_id(snd_timer_info_t * info);
const char *snd_timer_info_get_name(snd_timer_info_t * info);
long snd_timer_info_get_ticks(snd_timer_info_t * info);
long snd_timer_info_get_resolution(snd_timer_info_t * info);

size_t snd_timer_params_sizeof(void);
/** allocate #snd_timer_params_t container on stack */
#define snd_timer_params_alloca(ptr) do { assert(ptr); *ptr = (snd_timer_params_t *) alloca(snd_timer_params_sizeof()); memset(*ptr, 0, snd_timer_params_sizeof()); } while (0)
int snd_timer_params_malloc(snd_timer_params_t **ptr);
void snd_timer_params_free(snd_timer_params_t *obj);
void snd_timer_params_copy(snd_timer_params_t *dst, const snd_timer_params_t *src);

void snd_timer_params_set_auto_start(snd_timer_params_t * params, int auto_start);
void snd_timer_params_set_ticks(snd_timer_params_t * params, long ticks);
long snd_timer_params_get_ticks(snd_timer_params_t * params);
void snd_timer_params_set_queue_size(snd_timer_params_t * params, long queue_size);
long snd_timer_params_get_queue_size(snd_timer_params_t * params);

size_t snd_timer_status_sizeof(void);
/** allocate #snd_timer_status_t container on stack */
#define snd_timer_status_alloca(ptr) do { assert(ptr); *ptr = (snd_timer_status_t *) alloca(snd_timer_status_sizeof()); memset(*ptr, 0, snd_timer_status_sizeof()); } while (0)
int snd_timer_status_malloc(snd_timer_status_t **ptr);
void snd_timer_status_free(snd_timer_status_t *obj);
void snd_timer_status_copy(snd_timer_status_t *dst, const snd_timer_status_t *src);

struct timeval snd_timer_status_get_timestamp(snd_timer_status_t * status);
long snd_timer_status_get_resolution(snd_timer_status_t * status);
long snd_timer_status_get_lost(snd_timer_status_t * status);
long snd_timer_status_get_overrun(snd_timer_status_t * status);
long snd_timer_status_get_queue(snd_timer_status_t * status);

/** \} */

#ifdef __cplusplus
}
#endif

#endif /** __ALSA_TIMER_H */

