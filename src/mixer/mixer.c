/**
 * \file mixer/mixer.c
 * \brief Mixer Interface
 * \author Jaroslav Kysela <perex@perex.cz>
 * \author Abramo Bagnara <abramo@alsa-project.org>
 * \date 2001-2009
 *
 * Mixer interface is designed to access mixer elements.
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

/*! \page amixer Mixer interface

<P>Mixer interface is designed to access the abstracted amixer controls.

\section amixer_general_overview General overview

\section amixer_global Global mixer

The global mixer exposes basic or all (#SND_AMIXER_ALL) mixer related
controls to application.

\par Master
This control element defines playback master volume control for
whole card.

\section amixer_pcm PCM related mixer

This mixer works with PCM related controls with predefined abstractions.

\subsection amixer_pcm_playback Playback direction

Bellow mixer controls are available for playback PCM.

\par Master
Playback master volume control.

\par PCM
Playback PCM stream related volume control.

\subsection amixer_pcm_capture Capture direction

Note that none or any combination of controls might be present, but
at least Capture control should be implemented in alsa-lib.

\par Capture
Capture PCM stream related volume control.

\par Source
Capture Source (enum like Mic,CD,Line etc.).

\par [other] - like CD, Aux, Front Line etc.
These sources are mixed to PCM input. Both volume and switch might be
available.

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "mixer_local.h"
#include "mixer_abst.h"

static int ctl_event_handler(snd_ctl_t *ctl,
			     unsigned int mask,
			     snd_ctl_elem_t *elem);

static const char *const build_in_mixers[] = {
	"none", NULL
};

static int snd_amixer_open_conf(snd_amixer_t **amixerp,
				const char *name,
				snd_config_t *mixer_root,
				snd_config_t *mixer_conf,
				snd_pcm_t *pcm_playback,
				snd_pcm_t *pcm_capture,
				int mode)
{
	const char *str;
	char *buf = NULL, *buf1 = NULL;
	int err, idx;
	snd_config_t *conf, *type_conf = NULL;
	snd_config_iterator_t i, next;
	const char *id;
	const char *lib = NULL, *open_name = NULL;
	int (*open_func)(snd_amixer_t *amixer,
			 snd_config_t *root,
			 snd_config_t *conf,
			 struct sm_open *sm_open) = NULL;
#ifndef PIC
	extern void *snd_amixer_open_symbols(void);
#endif
	void *h = NULL;
	snd_amixer_t *amixer;
	if (snd_config_get_type(mixer_conf) != SND_CONFIG_TYPE_COMPOUND) {
		char *val;
		id = NULL;
		snd_config_get_id(mixer_conf, &id);
		val = NULL;
		snd_config_get_ascii(mixer_conf, &val);
		SNDERR("Invalid type for mixer %s%sdefinition (id: %s, value: %s)", name ? name : "", name ? " " : "", id, val);
		free(val);
		return -EINVAL;
	}
	err = snd_config_search(mixer_conf, "type", &conf);
	if (err < 0) {
		SNDERR("type is not defined");
		return err;
	}
	err = snd_config_get_id(conf, &id);
	if (err < 0) {
		SNDERR("unable to get id");
		return err;
	}
	err = snd_config_get_string(conf, &str);
	if (err < 0) {
		SNDERR("Invalid type for %s", id);
		return err;
	}
	err = snd_config_search_definition(mixer_root, "amixer_type", str, &type_conf);
	if (err >= 0) {
		if (snd_config_get_type(type_conf) != SND_CONFIG_TYPE_COMPOUND) {
			SNDERR("Invalid type for amixer type %s definition", str);
			goto _err;
		}
		snd_config_for_each(i, next, type_conf) {
			snd_config_t *n = snd_config_iterator_entry(i);
			const char *id;
			if (snd_config_get_id(n, &id) < 0)
				continue;
			if (strcmp(id, "comment") == 0)
				continue;
			if (strcmp(id, "lib") == 0) {
				err = snd_config_get_string(n, &lib);
				if (err < 0) {
					SNDERR("Invalid type for %s", id);
					goto _err;
				}
				continue;
			}
			if (strcmp(id, "open") == 0) {
				err = snd_config_get_string(n, &open_name);
				if (err < 0) {
					SNDERR("Invalid type for %s", id);
					goto _err;
				}
				continue;
			}
			SNDERR("Unknown field %s", id);
			err = -EINVAL;
			goto _err;
		}
	}
	if (!open_name) {
		buf = malloc(strlen(str) + 32);
		if (buf == NULL) {
			err = -ENOMEM;
			goto _err;
		}
		open_name = buf;
		sprintf(buf, "_snd_amixer_%s_open", str);
	}
	if (!lib) {
		const char *const *build_in = build_in_mixers;
		while (*build_in) {
			if (!strcmp(*build_in, str))
				break;
			build_in++;
		}
		if (*build_in == NULL) {
			buf1 = malloc(strlen(str) + sizeof(ALSA_PLUGIN_DIR) + 32);
			if (buf1 == NULL) {
				err = -ENOMEM;
				goto _err;
			}
			lib = buf1;
			sprintf(buf1, "%s/libasound_module_mixer_%s.so", ALSA_PLUGIN_DIR, str);
		}
	}
#ifndef PIC
	snd_mixer_open_symbols();	/* this call is for static linking only */
#endif
	open_func = snd_dlobj_cache_lookup(open_name);
	if (open_func) {
		err = 0;
		goto _err;
	}
	h = snd_dlopen(lib, RTLD_NOW);
	if (h)
		open_func = snd_dlsym(h, open_name, SND_DLSYM_VERSION(SND_AMIXER_DLSYM_VERSION));
	err = 0;
	if (!h) {
		SNDERR("Cannot open shared library %s",
		       lib ? lib : "[builtin]");
		err = -ENOENT;
	} else if (!open_func) {
		SNDERR("symbol %s is not defined inside %s", open_name,
		       lib ? lib : "[builtin]");
		snd_dlclose(h);
		err = -ENXIO;
	}
       _err:
	if (err >= 0) {
		amixer = calloc(1, sizeof(*amixer));
		if (amixer == NULL) {
			err = -ENOMEM;
			goto _err1;
		}
		INIT_LIST_HEAD(&amixer->elems);
		amixer->compare = snd_amixer_compare_default;
		amixer->sm_open.name = name;
		amixer->sm_open.pcm_playback = pcm_playback;
		amixer->sm_open.pcm_capture = pcm_capture;
		amixer->sm_open.mode = mode;
		err = open_func(amixer, mixer_root, mixer_conf, &amixer->sm_open);
		if (err < 0) {
			snd_amixer_close(amixer);
			goto _err1;
		}
		for (idx = 0; idx < SM_CTL_COUNT; idx++) {
			snd_ctl_t *ctl = amixer->sm_open.ctl[idx];
			if (ctl == NULL)
				continue;
			snd_ctl_set_callback(ctl, ctl_event_handler);
			snd_ctl_set_callback_private(ctl, amixer);
			err = snd_hctl_nonblock(ctl, 1);
			if (err >= 0)
				err = snd_ctl_subscribe_events(ctl, 1);
			if (err < 0) {
			        snd_amixer_close(amixer);
			        goto _err1;
                        }
		}
		*amixerp = amixer;
	       _err1:
		if (err >= 0) {
			if (h /*&& (mode & SND_PCM_KEEP_ALIVE)*/) {
				snd_dlobj_cache_add(open_name, h, open_func);
				h = NULL;
			}
			amixer->dl_handle = h;
			err = 0;
		} else {
			if (h)
				snd_dlclose(h);
		}
	}
	if (type_conf)
		snd_config_delete(type_conf);
	free(buf);
	free(buf1);
	return err;
}

static int snd_amixer_open_noupdate(snd_amixer_t **amixerp,
				    snd_config_t *root,
				    const char *name,
				    snd_pcm_t *pcm_playback,
				    snd_pcm_t *pcm_capture,
				    int mode,
				    int hop)
{
	int err, pcm = pcm_playback || pcm_capture;
	snd_config_t *mixer_conf;
	const char *str;

	err = snd_config_search_definition(root, pcm ? "amixer_pcm" : "amixer", name, &mixer_conf);
	if (err < 0) {
		SNDERR("Uknown amixer %s", name);
		return err;
	}
	if (snd_config_get_string(mixer_conf, &str) >= 0) {
		err = snd_amixer_open_noupdate(amixerp, root, name, pcm_playback, pcm_capture, mode, hop);
	} else {
		snd_config_set_hop(mixer_conf, hop);
		err = snd_amixer_open_conf(amixerp, name, root, mixer_conf, pcm_playback, pcm_capture, mode);
	}
	snd_config_delete(mixer_conf);
	return err;
}

/**
 * \brief Opens the global or PCM related mixer
 * \param amixerp Returned amixer handle
 * \param name ASCII identifier of the mixer handle
 * \param pcm_playback Playback PCM
 * \param pcm_capture Capture PCM
 * \param mode Open mode
 * \return 0 on success otherwise a negative error code
 *
 * If both pcm_playback and pcm_capture parameters are NULL, the global
 * mixer is opened.
 */
int snd_amixer_open(snd_amixer_t **amixerp,
		    const char *name,
		    snd_pcm_t *pcm_playback,
		    snd_pcm_t *pcm_capture,
		    int mode)
{
	int err;
	assert(amixerp);
	assert(name);
	err = snd_config_update();
	if (err < 0)
		return err;
	return snd_amixer_open_noupdate(amixerp, snd_config, name, pcm_playback, pcm_capture, mode, 0);
}

/**
 * \brief Opens the global or PCM related mixer using local configuration
 * \param amixerp Returned amixer handle
 * \param name ASCII identifier of the mixer handle
 * \param pcm_playback Playback PCM
 * \param pcm_capture Capture PCM
 * \param mode Open mode
 * \param lconf Local configuration
 * \return 0 on success otherwise a negative error code
 *
 * If both pcm_playback and pcm_capture parameters are NULL, the global
 * mixer is opened.
 */
int snd_amixer_open_lconf(snd_amixer_t **amixerp,
			  const char *name,
			  snd_pcm_t *pcm_playback,
			  snd_pcm_t *pcm_capture,
			  int mode,
			  snd_config_t *lconf)
{
	assert(amixerp);
	assert(name);
	assert(lconf);
	return snd_amixer_open_noupdate(amixerp, lconf, name, pcm_playback, pcm_capture, mode, 0);
}

/**
 * \brief Attach an CTL element to a amixer element
 * \param melem Mixer element
 * \param elem CTL element
 * \return 0 on success otherwise a negative error code
 *
 * For use by amixer element class specific code.
 */
int snd_amixer_elem_attach(snd_amixer_elem_t *melem,
			   snd_ctl_elem_t *elem)
{
	bag_t *bag = snd_ctl_elem_get_callback_private(elem);
	int err;
	err = bag_add(bag, melem);
	if (err < 0)
		return err;
	return bag_add(&melem->helems, elem);
}

/**
 * \brief Detach an CTL element from a amixer element
 * \param melem Mixer element
 * \param elem CTL element
 * \return 0 on success otherwise a negative error code
 *
 * For use by amixer element class specific code.
 */
int snd_amixer_elem_detach(snd_amixer_elem_t *melem,
			   snd_ctl_elem_t *elem)
{
	bag_t *bag = snd_ctl_elem_get_callback_private(elem);
	int err;
	err = bag_del(bag, melem);
	assert(err >= 0);
	err = bag_del(&melem->helems, elem);
	assert(err >= 0);
	return 0;
}

/**
 * \brief Return true if a amixer element does not contain any CTL elements
 * \param melem Mixer element
 * \return 0 if not empty, 1 if empty
 *
 * For use by amixer element class specific code.
 */
int snd_amixer_elem_is_empty(snd_amixer_elem_t *melem)
{
	return bag_empty(&melem->helems);
}

static int ctl_elem_event_handler(snd_ctl_elem_t *elem,
				  unsigned int mask)
{
	bag_t *bag = snd_ctl_elem_get_callback_private(elem);
	if (mask == SND_CTL_EVENT_MASK_REMOVE) {
		int res = 0;
		int err;
		bag_iterator_t i, n;
		bag_for_each_safe(i, n, bag) {
			snd_amixer_elem_t *melem = bag_iterator_entry(i);
			err = melem->amixer->event(melem->amixer, mask, elem, melem);
			if (err < 0)
				res = err;
		}
		assert(bag_empty(bag));
		bag_free(bag);
		return res;
	}
	if (mask & (SND_CTL_EVENT_MASK_VALUE | SND_CTL_EVENT_MASK_INFO)) {
		int err = 0;
		bag_iterator_t i, n;
		bag_for_each_safe(i, n, bag) {
			snd_amixer_elem_t *melem = bag_iterator_entry(i);
			err = melem->amixer->event(melem->amixer, mask, elem, melem);
			if (err < 0)
				return err;
		}
	}
	return 0;
}

static int ctl_event_handler(snd_ctl_t *ctl,
			     unsigned int mask,
			     snd_ctl_elem_t *elem)
{
	snd_amixer_t *amixer = snd_ctl_get_callback_private(ctl);
	int res = 0;
	if (mask & SND_CTL_EVENT_MASK_ADD) {
		bag_t *bag;
		int err = bag_new(&bag);
		if (err < 0)
			return err;
		snd_ctl_elem_set_callback(elem, ctl_elem_event_handler);
		snd_ctl_elem_set_callback_private(elem, bag);
		err = amixer->event(amixer, mask, elem, NULL);
		if (err < 0)
			return err;
	}
	return res;
}

static int snd_amixer_throw_event(snd_amixer_t *amixer,
				  unsigned int mask,
				  snd_amixer_elem_t *elem)
{
	amixer->events++;
	if (amixer->callback)
		return amixer->callback(amixer, mask, elem);
	return 0;
}

static int snd_amixer_elem_throw_event(snd_amixer_elem_t *elem,
				       unsigned int mask)
{
	elem->amixer->events++;
	if (elem->callback)
		return elem->callback(elem, mask);
	return 0;
}

static int _snd_amixer_find_elem(snd_amixer_t *amixer, snd_amixer_elem_t *elem, int *dir)
{
	unsigned int l, u;
	int c = 0;
	int idx = -1;
	assert(amixer && elem);
	assert(amixer->compare);
	l = 0;
	u = amixer->count;
	while (l < u) {
		idx = (l + u) / 2;
		c = amixer->compare(elem, amixer->pelems[idx]);
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

/**
 * \brief Get private data associated to give amixer element
 * \param elem Mixer element
 * \return private data
 */
void *snd_amixer_elem_get_private(const snd_amixer_elem_t *elem)
{
	return elem->private_data;
}

/**
 * \brief Allocate a new amixer element
 * \param amixer Mixer handle
 * \param elem Returned amixer element
 * \param id Element identificator
 * \param compare_weight Mixer element compare weight
 * \param private_data Private data
 * \param private_free Private data free callback
 * \return 0 on success otherwise a negative error code
 */
int snd_amixer_elem_new(snd_amixer_t *amixer,
			snd_amixer_elem_t **elem,
			snd_amixer_elem_id_t *id,
		        int compare_weight,
		        void *private_data,
		        void (*private_free)(snd_amixer_elem_t *elem))
{
	snd_amixer_elem_t *melem = calloc(1, sizeof(*melem));
	if (melem == NULL)
		return -ENOMEM;
	melem->amixer = amixer;
	melem->sm.id = *id;
	melem->compare_weight = compare_weight;
	melem->private_data = private_data;
	melem->private_free = private_free;
	INIT_LIST_HEAD(&melem->helems);
	*elem = melem;
	return 0;
}

/**
 * \brief Add an element for an amixer handle
 * \param amixer Mixer handle
 * \param elem Mixer element
 * \return 0 on success otherwise a negative error code
 */
int snd_amixer_elem_add(snd_amixer_t *amixer, snd_amixer_elem_t *elem)
{
	int dir, idx;

	if (amixer->count == amixer->alloc) {
		snd_amixer_elem_t **m;
		amixer->alloc += 32;
		m = realloc(amixer->pelems, sizeof(*m) * amixer->alloc);
		if (!m) {
			amixer->alloc -= 32;
			return -ENOMEM;
		}
		amixer->pelems = m;
	}
	if (amixer->count == 0) {
		list_add_tail(&elem->list, &amixer->elems);
		amixer->pelems[0] = elem;
	} else {
		idx = _snd_amixer_find_elem(amixer, elem, &dir);
		assert(dir != 0);
		if (dir > 0) {
			list_add(&elem->list, &amixer->pelems[idx]->list);
			idx++;
		} else {
			list_add_tail(&elem->list, &amixer->pelems[idx]->list);
		}
		memmove(amixer->pelems + idx + 1,
			amixer->pelems + idx,
			(amixer->count - idx) * sizeof(snd_amixer_elem_t *));
		amixer->pelems[idx] = elem;
	}
	amixer->count++;
	return snd_amixer_throw_event(amixer, SND_CTL_EVENT_MASK_ADD, elem);
}

/**
 * \brief Remove a amixer element
 * \param elem Mixer element
 * \return 0 on success otherwise a negative error code
 */
int snd_amixer_elem_remove(snd_amixer_elem_t *elem)
{
	snd_amixer_t *amixer = elem->amixer;
	bag_iterator_t i, n;
	int err, idx, dir;
	unsigned int m;
	assert(elem);
	assert(amixer->count);
	idx = _snd_amixer_find_elem(amixer, elem, &dir);
	if (dir != 0)
		return -EINVAL;
	bag_for_each_safe(i, n, &elem->helems) {
		snd_ctl_elem_t *helem = bag_iterator_entry(i);
		snd_amixer_elem_detach(elem, helem);
	}
	err = snd_amixer_elem_throw_event(elem, SND_CTL_EVENT_MASK_REMOVE);
	list_del(&elem->list);
	snd_amixer_elem_free(elem);
	amixer->count--;
	m = amixer->count - idx;
	if (m > 0)
		memmove(amixer->pelems + idx,
			amixer->pelems + idx + 1,
			m * sizeof(snd_amixer_elem_t *));
	return err;
}

/**
 * \brief Free a amixer element
 * \param elem Mixer element
 * \return 0 on success otherwise a negative error code
 */
void snd_amixer_elem_free(snd_amixer_elem_t *elem)
{
	if (elem->private_free)
		elem->private_free(elem);
	free(elem);
}

/**
 * \brief Mixer element informations are changed
 * \param elem Mixer element
 * \return 0 on success otherwise a negative error code
 */
int snd_amixer_elem_info(snd_amixer_elem_t *elem)
{
	return snd_amixer_elem_throw_event(elem, SND_CTL_EVENT_MASK_INFO);
}

/**
 * \brief Mixer element values is changed
 * \param elem Mixer element
 * \return 0 on success otherwise a negative error code
 */
int snd_amixer_elem_value(snd_amixer_elem_t *elem)
{
	return snd_amixer_elem_throw_event(elem, SND_CTL_EVENT_MASK_VALUE);
}

/**
 * \brief Close a amixer and free all related resources
 * \param amixer Mixer handle
 * \return 0 on success otherwise a negative error code
 */
int snd_amixer_close(snd_amixer_t *amixer)
{
	int res = 0, i;

	assert(amixer);
	for (i = 0; i < SM_CTL_COUNT; i++) {
		if (amixer->sm_open.ctl[i])
			res = snd_ctl_close(amixer->sm_open.ctl[i]);
		amixer->sm_open.ctl[i] = NULL;
	}
	assert(list_empty(&amixer->elems));
	if (amixer->pelems)
		free(amixer->pelems);
	if (amixer->dl_handle)
		snd_dlclose(amixer->dl_handle);
	free(amixer);
	return res;
}

static int amixer_compare(const void *a, const void *b)
{
	snd_amixer_t *amixer;

	amixer = (*((const snd_amixer_elem_t * const *)a))->amixer;
	return amixer->compare(*(const snd_amixer_elem_t * const *)a, *(const snd_amixer_elem_t * const *)b);
}

static int snd_amixer_sort(snd_amixer_t *amixer)
{
	unsigned int k;
	assert(amixer);
	assert(amixer->compare);
	INIT_LIST_HEAD(&amixer->elems);
	qsort(amixer->pelems, amixer->count, sizeof(snd_amixer_elem_t *), amixer_compare);
	for (k = 0; k < amixer->count; k++)
		list_add_tail(&amixer->pelems[k]->list, &amixer->elems);
	return 0;
}

/**
 * \brief Change amixer compare function and reorder elements
 * \param amixer Mixer handle
 * \param compare Element compare function
 * \return 0 on success otherwise a negative error code
 */
int snd_amixer_set_compare(snd_amixer_t *amixer, snd_amixer_compare_t compare)
{
	snd_amixer_compare_t compare_old;
	int err;

	assert(amixer);
	compare_old = amixer->compare;
	amixer->compare = compare == NULL ? snd_amixer_compare_default : compare;
	if ((err = snd_amixer_sort(amixer)) < 0) {
		amixer->compare = compare_old;
		return err;
	}
	return 0;
}

/**
 * \brief get count of poll descriptors for amixer handle
 * \param amixer Mixer handle
 * \return count of poll descriptors
 */
int snd_amixer_poll_descriptors_count(snd_amixer_t *amixer)
{
	assert(amixer);
	int i, c = 0, n;
	for (i = 0; i < SM_CTL_COUNT; i++) {
		if (amixer->sm_open.ctl[i] == NULL)
			continue;
		n = snd_ctl_poll_descriptors_count(amixer->sm_open.ctl[i]);
		if (n < 0)
			return n;
		c += n;
	}
	return c;
}

/**
 * \brief get poll descriptors
 * \param amixer Mixer handle
 * \param pfds array of poll descriptors
 * \param space space in the poll descriptor array
 * \return count of filled descriptors
 */
int snd_amixer_poll_descriptors(snd_amixer_t *amixer, struct pollfd *pfds, unsigned int space)
{
	int i, count = 0, n;

	assert(amixer);
	for (i = 0; i < SM_CTL_COUNT; i++) {
		if (amixer->sm_open.ctl[i] == NULL)
			continue;
		n = snd_ctl_poll_descriptors(amixer->sm_open.ctl[i], pfds, space);
		if (n < 0)
			return n;
		if (space >= (unsigned int)n) {
			count += n;
			space -= n;
			pfds += n;
		} else {
			space = 0;
		}
	}
	return count;
}

/**
 * \brief get returned events from poll descriptors
 * \param amixer Mixer handle
 * \param pfds array of poll descriptors
 * \param nfds count of poll descriptors
 * \param revents returned events
 * \return zero if success, otherwise a negative error code
 */
int snd_amixer_poll_descriptors_revents(snd_amixer_t *amixer, struct pollfd *pfds, unsigned int nfds, unsigned short *revents)
{
	unsigned int idx;
	unsigned short res;
        assert(amixer && pfds && revents);
	if (nfds == 0)
		return -EINVAL;
	res = 0;
	for (idx = 0; idx < nfds; idx++)
		res |= pfds->revents & (POLLIN|POLLERR|POLLNVAL);
	*revents = res;
	return 0;
}

/**
 * \brief Wait for a amixer to become ready (i.e. at least one event pending)
 * \param amixer Mixer handle
 * \param timeout maximum time in milliseconds to wait
 * \return 0 otherwise a negative error code on failure
 */
int snd_amixer_wait(snd_amixer_t *amixer, int timeout)
{
	struct pollfd spfds[16];
	struct pollfd *pfds = spfds;
	int err;
	int count;
	count = snd_amixer_poll_descriptors(amixer, pfds, sizeof(spfds) / sizeof(spfds[0]));
	if (count < 0)
		return count;
	if ((unsigned int) count > sizeof(spfds) / sizeof(spfds[0])) {
		pfds = malloc(count * sizeof(*pfds));
		if (!pfds)
			return -ENOMEM;
		err = snd_amixer_poll_descriptors(amixer, pfds, 
						 (unsigned int) count);
		assert(err == count);
	}
	err = poll(pfds, (unsigned int) count, timeout);
	if (err < 0)
		return -errno;
	return 0;
}

/**
 * \brief get first element for a amixer
 * \param amixer Mixer handle
 * \return pointer to first element
 */
snd_amixer_elem_t *snd_amixer_first_elem(snd_amixer_t *amixer)
{
	assert(amixer);
	if (list_empty(&amixer->elems))
		return NULL;
	return list_entry(amixer->elems.next, snd_amixer_elem_t, list);
}

/**
 * \brief get last element for a amixer
 * \param amixer Mixer handle
 * \return pointer to last element
 */
snd_amixer_elem_t *snd_amixer_last_elem(snd_amixer_t *amixer)
{
	assert(amixer);
	if (list_empty(&amixer->elems))
		return NULL;
	return list_entry(amixer->elems.prev, snd_amixer_elem_t, list);
}

/**
 * \brief get next amixer element
 * \param elem amixer element
 * \return pointer to next element
 */
snd_amixer_elem_t *snd_amixer_elem_next(snd_amixer_elem_t *elem)
{
	assert(elem);
	if (elem->list.next == &elem->amixer->elems)
		return NULL;
	return list_entry(elem->list.next, snd_amixer_elem_t, list);
}

/**
 * \brief get previous amixer element
 * \param elem amixer element
 * \return pointer to previous element
 */
snd_amixer_elem_t *snd_amixer_elem_prev(snd_amixer_elem_t *elem)
{
	assert(elem);
	if (elem->list.prev == &elem->amixer->elems)
		return NULL;
	return list_entry(elem->list.prev, snd_amixer_elem_t, list);
}

/**
 * \brief Handle pending amixer events invoking callbacks
 * \param amixer Mixer handle
 * \return Number of events that occured on success, otherwise a negative error code on failure
 */
int snd_amixer_handle_events(snd_amixer_t *amixer)
{
	int err, i;

	assert(amixer);
	amixer->events = 0;
	for (i = 0; i < SM_CTL_COUNT; i++) {
		if (amixer->sm_open.ctl[i] == NULL)
			continue;
		err = snd_ctl_handle_events(amixer->sm_open.ctl[i]);
		if (err < 0)
			return err;
	}
	return amixer->events;
}

/**
 * \brief Set event callback function for a amixer
 * \param obj amixer handle
 * \param val callback function
 *
 * This function is used in the mixer implementation. Use callback
 * functions to watch events.
 */
void snd_amixer_set_event(snd_amixer_t *obj, snd_amixer_event_t val)
{
	assert(obj);
	obj->event = val;
}

/**
 * \brief Set callback function for a amixer
 * \param obj amixer handle
 * \param val callback function
 */
void snd_amixer_set_callback(snd_amixer_t *obj, snd_amixer_callback_t val)
{
	assert(obj);
	obj->callback = val;
}

/**
 * \brief Set callback private value for a amixer
 * \param obj amixer handle
 * \param val callback private value
 */
void snd_amixer_set_callback_private(snd_amixer_t *obj, void * val)
{
	assert(obj);
	obj->callback_private = val;
}

/**
 * \brief Get callback private value for a amixer
 * \param amixer amixer handle
 * \return callback private value
 */
void * snd_amixer_get_callback_private(const snd_amixer_t *amixer)
{
	assert(amixer);
	return amixer->callback_private;
}

/**
 * \brief Get elements count for a amixer
 * \param amixer amixer handle
 * \return elements count
 */
unsigned int snd_amixer_get_count(const snd_amixer_t *amixer)
{
	assert(amixer);
	return amixer->count;
}

/**
 * \brief Set callback function for a amixer element
 * \param amixer amixer element
 * \param val callback function
 */
void snd_amixer_elem_set_callback(snd_amixer_elem_t *amixer, snd_amixer_elem_callback_t val)
{
	assert(amixer);
	amixer->callback = val;
}

/**
 * \brief Set callback private value for a amixer element
 * \param amixer amixer element
 * \param val callback private value
 */
void snd_amixer_elem_set_callback_private(snd_amixer_elem_t *amixer, void * val)
{
	assert(amixer);
	amixer->callback_private = val;
}

/**
 * \brief Get callback private value for a amixer element
 * \param amixer amixer element
 * \return callback private value
 */
void * snd_amixer_elem_get_callback_private(const snd_amixer_elem_t *amixer)
{
	assert(amixer);
	return amixer->callback_private;
}

/**
 * \brief Check if ID is generic
 * \param id string
 * \return 1 if success, otherwise 0
 */
int snd_amixer_conf_generic_id(const char *id)
{
	static const char ids[3][8] = { "comment", "type", "hint" };
	unsigned int k;
	for (k = 0; k < sizeof(ids) / sizeof(ids[0]); ++k) {
		if (strcmp(id, ids[k]) == 0)
			return 1;
	}
	return 0;
}
